#pragma once

#include "ECSRegistry.h"

// ==================================================
//  ECSView<Include...> —— 编译期视图
//
//  迭代策略: 以第一个非 NullComponent 组件池为驱动,
//  遍历其所有实体, 对其他池通过 O(1) sparse lookup 验证。
//  热路径上无虚函数调用, 无类型擦除开销。
//
//  用法:
//    ecs_view<Position, Velocity>(registry).each([](Position& p, Velocity& v) {
//        p.x += v.dx;
//    });
//
//    // 需要实体句柄时:
//    ecs_view<Health>(registry).each_entity([](entity_t e, Health& h) {
//        if (h.hp <= 0) { /* 入队销毁 */ }
//    });
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

// ==================================================
//  ecs_exclude 排除标记
// ==================================================
template<typename... Exclude>
struct ecs_exclude_t {};

template<typename... Exclude>
inline constexpr ecs_exclude_t<Exclude...> ecs_exclude{};
