#pragma once

#include "ECSRegistry.h"

// ==================================================
//  ECSView<Include...> —— 编译期视图
//
//  迭代策略: 以第一个非 NullComponent 组件池为驱动,
//  遍历其所有实体, 对其他池通过 O(1) sparse lookup 验证。
//  热路径上无虚函数调用, 无类型擦除开销。
//
//  线程安全:
//    ECSView 的构造与迭代 (each/each_entity/collect) 要求调用者持有
//    ECSRegistry::m_mutex 的独占锁 (SWLock), 且在整个迭代期间不得释放锁。
//    原因:
//    1. 构造时通过 get_pool 获取池指针, 若无锁另一线程 ensure_pool 可能 realloc pools_ → 悬空
//    2. 迭代时通过 primary->entities() 获取裸指针 (无锁), 若无锁另一线程 emplace/remove 可能
//       导致 vector realloc → UAF, 或 swap-with-last → 迭代顺序错乱/重复/遗漏
//    3. get_ref 通过 pool->get(e) 获取组件引用, 锁在 get 返回时释放, 回调期间另一线程
//       remove(e) 会导致引用指向 moved-from 对象 → UAF/脏读
//    必须使用独占锁 (SWLock) 而非共享锁 (SRLock), 因为:
//    - 全局 SRLock 可阻止 ensure_pool (需 SWLock), 但无法阻止通过池级锁的并发写入
//    - 池级 SWLock (emplace/remove) 与全局 SRLock 不互斥
//    - 只有全局 SWLock 能同时阻止 pools_ realloc 和所有池级写入
//
//  用法:
//    // 迭代期间必须持独占锁:
//    SWLock lock(registry.m_mutex);
//    ecs_view<Position, Velocity>(registry).each([](Position& p, Velocity& v) {
//        p.x += v.dx;
//    });
//
//    // 需要实体句柄时:
//    ecs_view<Health>(registry).each_entity([](entity_t e, Health& h) {
//        if (h.hp <= 0) { /* 入队销毁, 不得在此直接 destroy (会破坏迭代) */ }
//    });
//
//  注意: 迭代回调中不得直接 emplace/remove/destroy 当前迭代池的实体,
//        会触发 vector realloc 或 swap-with-last 导致迭代失效。
//        应将待操作实体入队, 迭代结束后再处理。
// ==================================================
template<typename... Include>
class ECSView
{
    static_assert(sizeof...(Include) > 0, "ECSView requires at least one Include component type");

    using PoolTuple = std::tuple<ComponentPool<Include>*...>;

public:
    explicit ECSView(ECSRegistry* registry) noexcept
        : registry_(registry)
    {
        init_pools(std::index_sequence_for<Include...>{});
    }

    /**
     *  遍历所有匹配实体, 回调 func(Include&...)
     */
    template<typename Func>
    void each(Func&& func)
    {
        auto* primary = get_driver_pool();
        if (!primary || primary->empty()) return;

        const entity_t* ents = primary->entities();
        const size_t n = primary->size();

        decltype(auto) fn = static_cast<Func&&>(func);
        for (size_t i = 0; i < n; ++i)
        {
            const entity_t e = ents[i];
            if (has_all(e))
                invoke(fn, e, std::index_sequence_for<Include...>{});
        }
    }

    /**
     *  遍历所有匹配实体, 回调 func(entity_t, Include&...)
     */
    template<typename Func>
    void each_entity(Func&& func)
    {
        auto* primary = get_driver_pool();
        if (!primary || primary->empty()) return;

        const entity_t* ents = primary->entities();
        const size_t n = primary->size();

        decltype(auto) fn = static_cast<Func&&>(func);
        for (size_t i = 0; i < n; ++i)
        {
            const entity_t e = ents[i];
            if (has_all(e))
                invoke_entity(fn, e, std::index_sequence_for<Include...>{});
        }
    }

    /**
     *  收集所有匹配的实体 ID 到 vector
     */
    std::vector<entity_t> collect() const
    {
        std::vector<entity_t> result;
        auto* primary = get_driver_pool();
        if (!primary || primary->empty()) return result;

        const entity_t* ents = primary->entities();
        const size_t n = primary->size();
        result.reserve(n);

        for (size_t i = 0; i < n; ++i)
        {
            const entity_t e = ents[i];
            if (has_all(e))
                result.push_back(e);
        }
        return result;
    }

    size_t size() const noexcept
    {
        auto* p = get_driver_pool();
        return p ? p->size() : 0;
    }

private:
    template<size_t... Is>
    void init_pools(std::index_sequence<Is...>)
    {
        ((std::get<Is>(pools_) = registry_->template get_pool<Include>(), void()), ...);
    }

    // 取第一个非 NullComponent 池作为驱动池
    auto* get_driver_pool() const noexcept
    {
        return get_driver_impl<0>();
    }

    template<size_t I>
    auto* get_driver_impl() const noexcept
    {
        if constexpr (I >= sizeof...(Include))
            return static_cast<ComponentPool<NullComponent>*>(nullptr);
        else
        {
            using T = std::tuple_element_t<I, std::tuple<Include...>>;
            if constexpr (std::is_same_v<T, NullComponent>)
                return get_driver_impl<I + 1>();
            else
                return std::get<I>(pools_);
        }
    }

    bool has_all(entity_t e) const noexcept
    {
        return has_all_impl<0>(e);
    }

    template<size_t I>
    bool has_all_impl(entity_t e) const noexcept
    {
        if constexpr (I >= sizeof...(Include))
            return true;
        else
        {
            using T = std::tuple_element_t<I, std::tuple<Include...>>;
            if constexpr (std::is_same_v<T, NullComponent>)
                return has_all_impl<I + 1>(e);
            else
            {
                auto* p = std::get<I>(pools_);
                if (!p || !p->has(e)) return false;
                return has_all_impl<I + 1>(e);
            }
        }
    }

    // func(Include&...)
    template<typename Func, size_t... Is>
    void invoke(Func&& func, entity_t e, std::index_sequence<Is...>)
    {
        std::forward<Func>(func)(get_ref<Is>(e)...);
    }

    // func(entity_t, Include&...)
    template<typename Func, size_t... Is>
    void invoke_entity(Func&& func, entity_t e, std::index_sequence<Is...>)
    {
        std::forward<Func>(func)(e, get_ref<Is>(e)...);
    }

    // 获取组件引用 (回调期间使用)
    // 注意: pool->get(e) 内部加共享锁返回 T*, 锁在返回时释放。
    // 回调 func(T&) 执行期间, 若另一线程 remove(e) 会导致引用指向 moved-from 对象。
    // 因此调用者必须持全局锁 (共享或独占) 以阻止并发修改。
    // NullComponent 分支: C++11 起 static 局部变量初始化是线程安全的(magic statics),
    // NullComponent 为只读占位对象, 无需 thread_local 隔离, 改用 static 消除TLS开销
    template<size_t I>
    auto& get_ref(entity_t e)
    {
        using T = std::tuple_element_t<I, std::tuple<Include...>>;
        if constexpr (std::is_same_v<T, NullComponent>)
        {
            static NullComponent dummy;
            return dummy;
        }
        else
        {
            return *std::get<I>(pools_)->get(e);
        }
    }

    ECSRegistry* registry_;
    PoolTuple pools_;
};

// ==================================================
//  ecs_view —— 视图构造自由函数
// ==================================================
template<typename... Include>
ECSView<Include...> ecs_view(ECSRegistry& registry)
{
    return ECSView<Include...>(&registry);
}
