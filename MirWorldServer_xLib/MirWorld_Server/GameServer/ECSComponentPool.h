#pragma once

#include "ECSDefines.h"
#include <vector>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <new>

// ==================================================
//  LockedComponentRef<T> —— 持锁的组件引用包装
//
//  用途: 消除 GetFromPool 快速路径的 TOCTOU 竞态。
//  构造时获取池级 SRLock，析构时释放，保证引用有效期内
//  组件不会被并发 remove/emplace 破坏。
//
//  用法:
//    auto ref = GetFromPool(m_pool, e);
//    if (ref) { ref->DoSomething(); }  // 锁持有期间安全
//    // ref 析构时自动释放锁
//
//  注意: 仅支持移动，不支持拷贝。
// ==================================================
template<typename T>
class LockedComponentRef
{
public:
	LockedComponentRef() noexcept : m_lock(nullptr), m_ptr(nullptr) {}

	LockedComponentRef(SRWLOCK& lock, T* ptr) noexcept
		: m_lock(&lock), m_ptr(ptr)
	{
		if (m_ptr)
			AcquireSRWLockShared(m_lock);
	}

	~LockedComponentRef()
	{
		if (m_ptr && m_lock)
			ReleaseSRWLockShared(m_lock);
	}

	// 仅移动
	LockedComponentRef(LockedComponentRef&& other) noexcept
		: m_lock(other.m_lock), m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;  // 转移锁所有权
	}

	LockedComponentRef& operator=(LockedComponentRef&& other) noexcept
	{
		if (this != &other)
		{
			if (m_ptr && m_lock)
				ReleaseSRWLockShared(m_lock);
			m_lock = other.m_lock;
			m_ptr  = other.m_ptr;
			other.m_ptr = nullptr;
		}
		return *this;
	}

	LockedComponentRef(const LockedComponentRef&) = delete;
	LockedComponentRef& operator=(const LockedComponentRef&) = delete;

	T* operator->() noexcept       { return m_ptr; }
	T& operator*()  noexcept       { return *m_ptr; }
	const T* operator->() const noexcept { return m_ptr; }
	const T& operator*()  const noexcept { return *m_ptr; }

	explicit operator bool() const noexcept { return m_ptr != nullptr; }
	T* get() noexcept { return m_ptr; }
	const T* get() const noexcept { return m_ptr; }

private:
	SRWLOCK* m_lock;
	T*       m_ptr;
};

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
 *
 *  线程安全:
 *    每个池持有独立的 srw_mutex。
 *    读方法(has, get, size, empty, entities)获取共享锁；
 *    写方法(emplace, remove, clear, reserve)获取独占锁。
 *    不同组件池之间可完全并发操作，互不阻塞。
 */
template<typename T>
class ComponentPool
{
	// 只要求可构造、可移动、可析构——即任何常规值类型
	static_assert(std::is_default_constructible_v<T>, "ComponentPool<T>: T must be default constructible");
	static_assert(std::is_move_constructible_v<T>, "ComponentPool<T>: T must be move constructible");
	static_assert(std::is_move_assignable_v<T>, "ComponentPool<T>: T must be move assignable");

public:
	using value_type = T;
	using iterator       = T*;
	using const_iterator = const T*;

	ComponentPool()
	{
		sparse_.resize(INITIAL_SPARSE_CAP, INVALID_SPARSE);
	}

	// ---- 容量 ----
	size_t size()     const noexcept { SRLock lock(m_mutex); return count_; }
	size_t capacity() const noexcept { SRLock lock(m_mutex); return dense_.capacity(); }
	bool   empty()    const noexcept { SRLock lock(m_mutex); return count_ == 0; }

	// ---- 锁访问 (供 LockedComponentRef 等外部同步设施使用) ----
	SRWLOCK& mutex() const { return m_mutex; }

	// ---- 迭代器 (遍历稠密组件数组) ----
	//  警告: 这些方法返回原始指针，调用者必须在持有 m_mutex 或全局锁期间使用。
	//  在锁外使用可能导致 UAF (emplace realloc) 或脏读 (swap-with-last)。
	//  推荐: 使用 ECSView (需全局锁) 或 LockedComponentRef 包装。
	iterator       begin()       noexcept { return dense_.data(); }
	const_iterator begin() const noexcept { return dense_.data(); }
	iterator       end()         noexcept { return dense_.data() + count_; }
	const_iterator end()   const noexcept { return dense_.data() + count_; }

	// ---- 原始数据访问 ----
	//  警告: 返回原始指针，调用者必须在锁持有期间使用 (同上)。
	T*       data()     noexcept { return dense_.data(); }
	const T* data() const noexcept { return dense_.data(); }
	const entity_t* entities() const noexcept { return entity_.data(); }

	// ---- 单组件操作 ----

	bool has(entity_t e) const noexcept
	{
		SRLock lock(m_mutex);
		const entity_idx idx = entity_to_index(e);
		if (idx >= sparse_.size()) return false;
		const entity_idx denseIdx = sparse_[idx];
		if (denseIdx >= count_) return false;
		return entity_[denseIdx] == e;  // 世代验证
	}

	T* get(entity_t e) noexcept
	{
		SRLock lock(m_mutex);
		const entity_idx idx = entity_to_index(e);
		if (idx >= sparse_.size()) return nullptr;
		const entity_idx denseIdx = sparse_[idx];
		if (denseIdx >= count_) return nullptr;
		if (entity_[denseIdx] != e) return nullptr;
		return &dense_[denseIdx];
	}

	// const 版本直接实现, 避免 const_cast (修复形式上 UB)
	const T* get(entity_t e) const noexcept
	{
		SRLock lock(m_mutex);
		const entity_idx idx = entity_to_index(e);
		if (idx >= sparse_.size()) return nullptr;
		const entity_idx denseIdx = sparse_[idx];
		if (denseIdx >= count_) return nullptr;
		if (entity_[denseIdx] != e) return nullptr;
		return &dense_[denseIdx];
	}

	// 获取或创建: 存在则返回已有组件, 否则默认构造一个
	T& get_or_emplace(entity_t e)
	{
		SWLock lock(m_mutex);
		T* ptr = get_nolock(e);
		if (ptr) return *ptr;
		return emplace_nolock(e, T{});
	}

	template<typename... Args>
	T& emplace(entity_t e, Args&&... args)
	{
		SWLock lock(m_mutex);
		return emplace_nolock(e, std::forward<Args>(args)...);
	}

	void remove(entity_t e) noexcept
	{
		SWLock lock(m_mutex);
		remove_nolock(e);
	}

	// 批量移除 (调用者负责外部加锁或通过此方法内部加锁)
	// noexcept 规格跟随 T 的移动赋值异常规格 (修复过强 noexcept 导致 std::terminate 风险)
	void remove_batch(const std::vector<entity_t>& entities) noexcept(std::is_nothrow_move_assignable_v<T>)
	{
		SWLock lock(m_mutex);
		for (entity_t e : entities)
			remove_nolock(e);
	}

	// 清空所有组件 (保留容量)
	void clear() noexcept(std::is_nothrow_destructible_v<T>)
	{
		SWLock lock(m_mutex);
		// vector::clear() 自动析构 [0, size()) 范围所有存活对象 (含 moved-from)
		dense_.clear();
		entity_.clear();
		// 重置 sparse 为"不存在"
		std::fill(sparse_.begin(), sparse_.end(),
			static_cast<entity_idx>(INVALID_SPARSE));
		count_ = 0;
	}

	// 收缩内存到实际大小 (析构 [count_, dense_.size()) 范围的 moved-from 对象并释放内存)
	void shrink_to_fit()
	{
		SWLock lock(m_mutex);
		dense_.resize(count_);
		dense_.shrink_to_fit();
		entity_.resize(count_);
		entity_.shrink_to_fit();
	}

	// 预分配
	void reserve(size_t n)
	{
		SWLock lock(m_mutex);
		dense_.reserve(n);
		entity_.reserve(n);
	}

	// ---- 无锁内部方法 (供 ECSRegistry 在已持有全局锁后批量操作时使用) ----

	bool has_nolock(entity_t e) const noexcept
	{
		const entity_idx idx = entity_to_index(e);
		if (idx >= sparse_.size()) return false;
		const entity_idx denseIdx = sparse_[idx];
		if (denseIdx >= count_) return false;
		return entity_[denseIdx] == e;
	}

	T* get_nolock(entity_t e) noexcept
	{
		const entity_idx idx = entity_to_index(e);
		if (idx >= sparse_.size()) return nullptr;
		const entity_idx denseIdx = sparse_[idx];
		if (denseIdx >= count_) return nullptr;
		if (entity_[denseIdx] != e) return nullptr;
		return &dense_[denseIdx];
	}

	void remove_nolock(entity_t e) noexcept
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

	/**
	 *  emplace 的无锁实现 (调用者必须持有 m_mutex 独占锁)
	 */
	template<typename... Args>
	T& emplace_nolock(entity_t e, Args&&... args)
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
			T* ptr = dense_.data() + oldDense;
			ptr->~T();
			new (ptr) T(std::forward<Args>(args)...);
			return *ptr;
		}

		// 追加到稠密数组末尾
		const entity_idx newDense = count_;
		if (newDense < dense_.size() && newDense < entity_.size())
		{
			// 槽位已构造 (remove 后遗留的 moved-from 存活对象)
			T* ptr = dense_.data() + newDense;
			ptr->~T();
			new (ptr) T(std::forward<Args>(args)...);
			entity_[newDense] = e;
		}
		else
		{
			dense_.emplace_back(std::forward<Args>(args)...);
			entity_.push_back(e);
		}
		sparse_[idx] = newDense;
		++count_;
		return dense_[newDense];
	}

private:
	static constexpr entity_idx INVALID_SPARSE = 0xFFFFFFFFu;
	static constexpr size_t     INITIAL_SPARSE_CAP = 4096;

	mutable SRWLOCK m_mutex = SRWLOCK_INIT;  // 每个池独立的读写锁

	std::vector<T>         dense_;    // 稠密组件数组
	std::vector<entity_t>  entity_;   // dense[i] 对应的实体
	std::vector<entity_idx> sparse_;  // 实体索引 → dense 下标
	entity_idx             count_ = 0;
};
