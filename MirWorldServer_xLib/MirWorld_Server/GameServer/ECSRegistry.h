#pragma once

#include "ECSDefines.h"
#include "ECSComponentPool.h"
#include <vector>
#include <memory>
#include <mutex>
#include <concurrencysal.h>

// ==================================================
//  IECSPool —— 类型擦除的组件池基类
//  (仅用于 ECSRegistry 内部存储)
// ==================================================
struct IECSPool
{
    virtual ~IECSPool() = default;
    virtual bool            has(entity_t e) const noexcept = 0;
    virtual void            remove(entity_t e) noexcept     = 0;
    virtual void            clear() noexcept                = 0;
    virtual size_t          size() const noexcept           = 0;
    virtual const entity_t* entities() const noexcept       = 0;
};

template<typename T>
class ECSPoolWrapper final : public IECSPool
{
public:
    ComponentPool<T> pool;

    bool   has(entity_t e) const noexcept override     { return pool.has(e); }
    void   remove(entity_t e) noexcept override        { pool.remove(e); }
    void   clear() noexcept override                   { pool.clear(); }
    size_t size() const noexcept override              { return pool.size(); }
    const entity_t* entities() const noexcept override { return pool.entities(); }
};

// ==================================================
//  ECSRegistry —— 中心世界
// ==================================================
class ECSRegistry
{
public:
    // ==========================================
    //  多线程支持 — 满足 BasicLockable 概念
    //  外部可通过 std::lock_guard<ECSRegistry> 持有锁，
    //  内部方法递归获取同一把 recursive_mutex。
    // ==========================================
    _Acquires_lock_(this->m_mutex) void lock()     const { m_mutex.lock(); }
    _Releases_lock_(this->m_mutex) void unlock()   const { m_mutex.unlock(); }
    _When_(return != 0, _Acquires_lock_(this->m_mutex)) bool try_lock() const { return m_mutex.try_lock(); }

    ECSRegistry()
    {
        entityGenerations_.resize(INITIAL_GEN_CAPACITY, 0);
        freeList_.reserve(4096);
    }

    ~ECSRegistry() { clear_all(); }

    ECSRegistry(const ECSRegistry&) = delete;
    ECSRegistry& operator=(const ECSRegistry&) = delete;
    ECSRegistry(ECSRegistry&&) = default;
    ECSRegistry& operator=(ECSRegistry&&) = default;

    // ==========================================
    //  实体生命周期
    // ==========================================

    entity_t create()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        entity_idx idx;
        if (!freeList_.empty())
        {
            idx = freeList_.back();
            freeList_.pop_back();
        }
        else
        {
            idx = nextEntity_;
            if (idx >= MAX_ENTITIES) return INVALID_ENTITY;
            ++nextEntity_;
            if (idx >= entityGenerations_.size())
                entityGenerations_.resize(entityGenerations_.size() * 2, 0);
        }
        return make_entity(idx, entityGenerations_[idx]);
    }

    void destroy(entity_t e)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (e == INVALID_ENTITY) return;
        const entity_idx idx = entity_to_index(e);
        if (idx >= entityGenerations_.size()) return;
        if (entity_to_gen(e) != entityGenerations_[idx]) return;

        for (auto& pool : pools_)
            if (pool) pool->remove(e);

        ++entityGenerations_[idx];
        if (entityGenerations_[idx] == 0)
            entityGenerations_[idx] = 1;

        freeList_.push_back(idx);
    }

    bool valid(entity_t e) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (e == INVALID_ENTITY) return false;
        const entity_idx idx = entity_to_index(e);
        return idx < entityGenerations_.size() &&
               entity_to_gen(e) == entityGenerations_[idx];
    }

    size_t entity_count() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return nextEntity_ - freeList_.size();
    }

    // ==========================================
    //  组件操作
    // ==========================================

    template<typename T, typename... Args>
    T& emplace(entity_t e, Args&&... args)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return ensure_pool<T>().emplace(e, std::forward<Args>(args)...);
    }

    template<typename T>
    T& get_or_emplace(entity_t e)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return ensure_pool<T>().get_or_emplace(e);
    }

    template<typename T>
    T* get(entity_t e) noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto* p = get_pool<T>();
        return p ? p->get(e) : nullptr;
    }

    template<typename T>
    const T* get(entity_t e) const noexcept
    {
        return const_cast<ECSRegistry*>(this)->get<T>(e);
    }

    template<typename T>
    bool has(entity_t e) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        const auto* p = get_pool<T>();
        return p && p->has(e);
    }

    template<typename T>
    void remove(entity_t e) noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto* p = get_pool<T>();
        if (p) p->remove(e);
    }

    // ==========================================
    //  池访问 (供 View 构造使用)
    // ==========================================

    template<typename T>
    ComponentPool<T>& ensure_pool()
    {
        const uint32_t tid = get_component_type_id<T>();
        if (tid >= pools_.size())
            pools_.resize(tid + 1);
        if (!pools_[tid])
            pools_[tid] = std::make_unique<ECSPoolWrapper<T>>();
        return static_cast<ECSPoolWrapper<T>*>(pools_[tid].get())->pool;
    }

    template<typename T>
    ComponentPool<T>* get_pool() noexcept
    {
        const uint32_t tid = get_component_type_id<T>();
        if (tid >= pools_.size() || !pools_[tid]) return nullptr;
        return &static_cast<ECSPoolWrapper<T>*>(pools_[tid].get())->pool;
    }

    template<typename T>
    const ComponentPool<T>* get_pool() const noexcept
    {
        return const_cast<ECSRegistry*>(this)->get_pool<T>();
    }

    // ==========================================
    //  全局操作
    // ==========================================

    void clear_all() noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (auto& p : pools_)
            if (p) p->clear();
        nextEntity_ = 0;
        freeList_.clear();
        entityGenerations_.clear();
        entityGenerations_.resize(INITIAL_GEN_CAPACITY, 0);
    }

    template<typename T>
    void reserve(size_t n)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        ensure_pool<T>().reserve(n);
    }

private:
    static constexpr size_t INITIAL_GEN_CAPACITY = 16384;

    mutable std::recursive_mutex m_mutex;

    std::vector<std::unique_ptr<IECSPool>> pools_;
    std::vector<entity_gen>  entityGenerations_;
    std::vector<entity_idx>  freeList_;
    entity_idx               nextEntity_ = 0;
};
