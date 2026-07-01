#pragma once

#include "ECSDefines.h"
#include "ECSComponentPool.h"
#include <vector>
#include <memory>
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
//
//  锁模型 (两级):
//    全局 m_mutex (srw_mutex) 保护:
//      - pools_ 向量 (组件池注册表)
//      - entityGenerations_ / freeList_ / nextEntity_ (实体生命周期)
//    每个 ComponentPool<T> 持有独立的 srw_mutex 保护组件数据。
//    锁顺序: 始终先获取全局锁，再获取池锁。
//
//  锁契约 (修复后明确化):
//    - 实体生命周期操作 (create/destroy/valid/entity_count):
//        调用者必须持全局独占锁 (SWLock)
//    - ensure_pool<T>():
//        调用者必须持全局独占锁 (SWLock) — 会修改 pools_
//    - get_pool<T>():
//        调用者必须持全局共享锁 (SRLock) 或独占锁 (SWLock)
//    - 组件操作 emplace/get_or_emplace (需 ensure_pool):
//        调用者必须持全局独占锁 (SWLock)
//    - 组件操作 get/has/remove (仅 get_pool):
//        调用者必须持全局共享锁 (SRLock) 或独占锁 (SWLock)
//    - clear_all():
//        调用者必须持全局独占锁 (SWLock)
//    - ECSView 迭代 (each/each_entity/collect):
//        调用者必须持全局共享锁 (SRLock) 或独占锁 (SWLock),
//        且迭代期间不得释放锁, 否则另一线程 ensure_pool 可能导致 pools_ realloc
//
//  注意: SRWLOCK 不可重入, 同一线程不可重复加同种锁。
//        若调用者已持独占锁, 内部方法不再加锁 (避免死锁)。
// ==================================================
class ECSRegistry
{
public:
	// ==========================================
	//  多线程支持
	//  - 外部调用者通过 SRLock(m_mutex) / SWLock(m_mutex) 加锁
	//  - 内部方法不加锁，避免双重加锁
	// ==========================================
	mutable SRWLOCK m_mutex = SRWLOCK_INIT;

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
	//  锁要求: 调用者必须持全局独占锁 (SWLock)
	// ==========================================

	entity_t create()
	{
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
		if (e == INVALID_ENTITY) return;
		const entity_idx idx = entity_to_index(e);
		if (idx >= entityGenerations_.size()) return;
		if (entity_to_gen(e) != entityGenerations_[idx]) return;

		for (auto& pool : pools_)
			if (pool) pool->remove(e);

		// 世代号在 entity_t 中仅占 ENTITY_GENERATION_BITS(12) 位，
		// 必须按 12 位掩码递增回绕。否则存储值超过 4095 后，make_entity
		// 编码会截断高位，导致 create() 复用该 idx 返回的句柄立即
		// valid()==false（entity_to_gen(句柄) != entityGenerations_[idx]）。
		// 原实现仅判断 ==0（针对 uint16_t 全范围溢出），与 12 位语义不符。
		entityGenerations_[idx] = (entityGenerations_[idx] + 1) & ((1u << ENTITY_GENERATION_BITS) - 1u);
		if (entityGenerations_[idx] == 0)
			entityGenerations_[idx] = 1;

		freeList_.push_back(idx);
	}

	bool valid(entity_t e) const noexcept
	{
		if (e == INVALID_ENTITY) return false;
		const entity_idx idx = entity_to_index(e);
		return idx < entityGenerations_.size() &&
			   entity_to_gen(e) == entityGenerations_[idx];
	}

	size_t entity_count() const noexcept
	{
		return nextEntity_ - freeList_.size();
	}

	// ==========================================
	//  组件操作
	//  锁要求:
	//    - emplace/get_or_emplace: 调用者必须持全局独占锁 (SWLock) — 因需 ensure_pool
	//    - get/has/remove: 调用者必须持全局共享锁 (SRLock) 或独占锁 (SWLock)
	// ==========================================

	template<typename T, typename... Args>
	T& emplace(entity_t e, Args&&... args)
	{
		return ensure_pool<T>().emplace(e, std::forward<Args>(args)...);
	}

	template<typename T>
	T& get_or_emplace(entity_t e)
	{
		return ensure_pool<T>().get_or_emplace(e);
	}

	template<typename T>
	T* get(entity_t e) noexcept
	{
		auto* p = get_pool<T>();
		return p ? p->get(e) : nullptr;
	}

	template<typename T>
	const T* get(entity_t e) const noexcept
	{
		return const_cast<ECSRegistry*>(this)->get<T>(e);
	}

	// ---- 无锁快速路径 (调用者必须持全局锁, 跳过池级锁避免冗余开销) ----

	template<typename T>
	T* get_nolock(entity_t e) noexcept
	{
		auto* p = get_pool<T>();
		return p ? p->get_nolock(e) : nullptr;
	}

	template<typename T>
	const T* get_nolock(entity_t e) const noexcept
	{
		return const_cast<ECSRegistry*>(this)->get_nolock<T>(e);
	}

	template<typename T>
	bool has_nolock(entity_t e) const noexcept
	{
		const auto* p = get_pool<T>();
		return p && p->has_nolock(e);
	}

	template<typename T>
	bool has(entity_t e) const noexcept
	{
		const auto* p = get_pool<T>();
		return p && p->has(e);
	}

	template<typename T>
	void remove(entity_t e) noexcept
	{
		auto* p = get_pool<T>();
		if (p) p->remove(e);
	}

	// ==========================================
	//  池访问 (供 View 构造使用)
	//  锁要求:
	//    - ensure_pool<T>(): 调用者必须持全局独占锁 (SWLock) — 会修改 pools_
	//    - get_pool<T>(): 调用者必须持全局共享锁 (SRLock) 或独占锁 (SWLock)
	//  注意: ensure_pool 可能触发 pools_ vector realloc, 若其他线程无锁读取 pools_
	//        会导致悬空指针。ECSView 构造时通过 get_pool 获取池指针, 必须持锁。
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
	//  锁要求:
	//    - clear_all(): 调用者必须持全局独占锁 (SWLock) — 遍历并修改 pools_ 及实体状态
	//    - reserve<T>(): 调用者必须持全局独占锁 (SWLock) — 因需 ensure_pool
	// ==========================================

	// clear_all 遍历 pools_ 并修改实体状态, 必须在全局独占锁下调用
	// 修复: 原实现无锁说明, 并发 ensure_pool 会导致 pools_ 迭代器失效 → 崩溃
	void clear_all() noexcept
	{
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
		ensure_pool<T>().reserve(n);
	}

private:
	static constexpr size_t INITIAL_GEN_CAPACITY = 16384;

	std::vector<std::unique_ptr<IECSPool>> pools_;
	std::vector<entity_gen>  entityGenerations_;
	std::vector<entity_idx>  freeList_;
	entity_idx               nextEntity_ = 0;
};
