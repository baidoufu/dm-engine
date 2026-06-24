#pragma once

#include "ECSDefines.h"
#include <vector>
#include <cstring>
#include <cassert>
#include <algorithm>

/**
 *  ComponentPool<T> —— 单组件类型的稀疏集存储
 *
 *  核心数据结构:
 *    dense[]   —— 稠密组件数组 (连续内存, 缓存友好迭代)
 *    entities[] —— dense[i] 对应的实体 (双向映射)
 *    sparse[]  —— 实体索引 → dense 下标 (O(1) 查找, INVALID_SPARSE 表示不存在)
 *
 *  性能特征:
 *    - 迭代: O(n) 线性扫描稠密数组, 预取友好
 *    - 添加: O(1) 追加到稠密数组末尾
 *    - 移除: O(1) swap-with-last + pop
 *    - 查找: O(1) 通过稀疏数组索引
 *    - 单线程使用; 多线程场景请在外部加锁
 */
template<typename T>
class ComponentPool
{
    static_assert(std::is_trivially_copyable_v<T>, "ComponentPool<T>: T must be trivially copyable");
    static_assert(std::is_default_constructible_v<T>, "ComponentPool<T>: T must be default constructible");

public:
    using value_type = T;
    using iterator       = T*;
    using const_iterator = const T*;

    ComponentPool()
    {
        sparse_.resize(INITIAL_SPARSE_CAP, INVALID_SPARSE);
    }

    // ---- 容量 ----
    size_t size()     const noexcept { return count_; }
    size_t capacity() const noexcept { return dense_.capacity(); }
    bool   empty()    const noexcept { return count_ == 0; }

    // ---- 迭代器 (遍历稠密组件数组) ----
    iterator       begin()       noexcept { return dense_.data(); }
    const_iterator begin() const noexcept { return dense_.data(); }
    iterator       end()         noexcept { return dense_.data() + count_; }
    const_iterator end()   const noexcept { return dense_.data() + count_; }

    // ---- 原始数据访问 ----
    T*       data()     noexcept { return dense_.data(); }
    const T* data() const noexcept { return dense_.data(); }
    const entity_t* entities() const noexcept { return entity_.data(); }

    // ---- 单组件操作 ----

    bool has(entity_t e) const noexcept
    {
        const entity_idx idx = entity_to_index(e);
        if (idx >= sparse_.size()) return false;
        const entity_idx denseIdx = sparse_[idx];
        if (denseIdx >= count_) return false;
        return entity_[denseIdx] == e;  // 世代验证
    }

    T* get(entity_t e) noexcept
    {
        const entity_idx idx = entity_to_index(e);
        if (idx >= sparse_.size()) return nullptr;
        const entity_idx denseIdx = sparse_[idx];
        if (denseIdx >= count_) return nullptr;
        if (entity_[denseIdx] != e) return nullptr;
        return &dense_[denseIdx];
    }

    const T* get(entity_t e) const noexcept
    {
        return const_cast<ComponentPool*>(this)->get(e);
    }

    // 获取或创建: 存在则返回已有组件, 否则默认构造一个
    T& get_or_emplace(entity_t e)
    {
        T* ptr = get(e);
        if (ptr) return *ptr;
        return emplace(e, T{});
    }

    template<typename... Args>
    T& emplace(entity_t e, Args&&... args)
    {
        const entity_idx idx = entity_to_index(e);
        assert(idx < MAX_ENTITIES);

        // 如需扩容稀疏数组
        if (idx >= sparse_.size())
            sparse_.resize((std::max<size_t>)(idx + 1, sparse_.size() * 2), INVALID_SPARSE);

        // 检查是否已存在
        const entity_idx oldDense = sparse_[idx];
        if (oldDense < count_ && entity_[oldDense] == e)
        {
            // 已存在: 原地替换
            dense_[oldDense] = T{std::forward<Args>(args)...};
            return dense_[oldDense];
        }

        // 追加到稠密数组末尾
        const entity_idx newDense = count_;
        if (newDense >= dense_.size() || newDense >= entity_.size())
        {
            // 任一 vector 容量不足时统一 push_back，保证 dense_ 和 entity_ 大小始终同步
            dense_.push_back(T{std::forward<Args>(args)...});
            entity_.push_back(e);
        }
        else
        {
            dense_[newDense] = T{std::forward<Args>(args)...};
            entity_[newDense] = e;
        }
        sparse_[idx] = newDense;
        ++count_;
        return dense_[newDense];
    }

    void remove(entity_t e) noexcept
    {
        const entity_idx idx = entity_to_index(e);
        if (idx >= sparse_.size()) return;

        const entity_idx denseIdx = sparse_[idx];
        if (denseIdx >= count_ || entity_[denseIdx] != e) return;

        // swap-with-last 然后 pop (O(1) 移除)
        const entity_idx lastIdx = count_ - 1;
        if (denseIdx != lastIdx)
        {
            dense_[denseIdx]  = std::move(dense_[lastIdx]);
            entity_[denseIdx] = entity_[lastIdx];
            sparse_[entity_to_index(entity_[lastIdx])] = denseIdx;
        }
        sparse_[idx] = INVALID_SPARSE;
        --count_;
    }

    // 批量移除
    void remove_batch(const std::vector<entity_t>& entities) noexcept
    {
        for (entity_t e : entities)
            remove(e);
    }

    // 清空所有组件 (保留容量)
    void clear() noexcept
    {
        count_ = 0;
    }

    // 收缩内存到实际大小
    void shrink_to_fit()
    {
        dense_.resize(count_);
        dense_.shrink_to_fit();
        entity_.resize(count_);
        entity_.shrink_to_fit();
    }

    // 预分配
    void reserve(size_t n)
    {
        dense_.reserve(n);
        entity_.reserve(n);
    }

private:
    static constexpr entity_idx INVALID_SPARSE = 0xFFFFFFFFu;
    static constexpr size_t     INITIAL_SPARSE_CAP = 4096;

    std::vector<T>         dense_;    // 稠密组件数组
    std::vector<entity_t>  entity_;   // dense[i] 对应的实体
    std::vector<entity_idx> sparse_;  // 实体索引 → dense 下标
    entity_idx             count_ = 0;
};
