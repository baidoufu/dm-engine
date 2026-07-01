#pragma once

#include "ECS.h"
#include <unordered_map>

/**
 *  ECSWorld — 共享 ECS 注册表单例
 *
 *  职责:
 *    1. 持有唯一的 ECSRegistry, 供所有 System 共享访问。
 *    2. 维护 ownerId → entity_t 映射表, 实现「一个游戏对象 = 一个 ECS 实体」。
 *
 *  线程安全:
 *    本类方法自身不加锁。m_entityMap 与 m_world 的实体状态共用同一把锁
 *    (m_world.m_mutex), 避免双锁嵌套导致的死锁。
 *
 *  锁契约:
 *    - 写操作 (CreateEntity/DestroyEntity): 调用者必须持 m_world.m_mutex 独占锁 (SWLock)
 *      原因: 会修改 m_entityMap 和 m_world 实体生命周期
 *    - 读操作 (GetEntity/HasEntity): 调用者必须持 m_world.m_mutex 共享锁 (SRLock) 或独占锁 (SWLock)
 *      原因: 读取 m_entityMap
 *    - GetWorld(): 无锁要求 (返回引用, 但后续操作需按 ECSRegistry 锁契约加锁)
 *
 *  注意:
 *    1. SRWLOCK 不可重入, 同一线程不可重复加同种锁。
 *    2. m_entityMap 没有独立锁, 完全依赖 m_world.m_mutex 保护。
 *       若未来需要解耦, 可为 m_entityMap 增加独立 mutex, 但需注意锁顺序。
 *    3. CreateEntity 的 find→create→insert 链在独占锁下是原子的。
 */
class ECSWorld : public xSingletonClass<ECSWorld>
{
public:
	ECSWorld()  = default;
	~ECSWorld() = default;

	// 获取 ECSRegistry 对象
	ECSRegistry& GetWorld() { return m_world; }
	const ECSRegistry& GetWorld() const { return m_world; }

	// ========== 实体映射表(幂等接口) ==========
	// 锁要求: 所有方法调用者必须持 m_world.m_mutex (写操作用 SWLock, 读操作用 SRLock)

	/**
	 *  创建空实体并登记到映射表。
	 *  幂等: 若 ownerId 已存在, 直接返回既有实体, 不重复创建。
	 *  调用方负责在新建实体上 emplace 所需组件(通过 world.has<T> 判断是否新建)。
	 *  返回 INVALID_ENTITY 表示创建失败(实体数达上限)。
	 *  锁要求: 调用者必须持 m_world.m_mutex 独占锁 (SWLock)
	 */
	entity_t CreateEntity(UINT ownerId)
	{
		auto it = m_entityMap.find(ownerId);
		if (it != m_entityMap.end())
			return it->second;
		entity_t e = m_world.create();
		if (e == INVALID_ENTITY)
			return INVALID_ENTITY;
		m_entityMap[ownerId] = e;
		return e;
	}

	/**
	 *  获取 ownerId 对应的实体。
	 *  不存在时返回 INVALID_ENTITY。调用方应处理空指针/无效实体情形。
	 *  锁要求: 调用者必须持 m_world.m_mutex 共享锁 (SRLock) 或独占锁 (SWLock)
	 */
	entity_t GetEntity(UINT ownerId)
	{
		auto it = m_entityMap.find(ownerId);
		return (it != m_entityMap.end()) ? it->second : INVALID_ENTITY;
	}

	/**
	 *  销毁 ownerId 对应的实体并从映射表移除。
	 *  幂等: 不存在时安全无操作。
	 *  ECSRegistry::destroy 会自动移除该实体上的所有组件。
	 *  锁要求: 调用者必须持 m_world.m_mutex 独占锁 (SWLock)
	 */
	void DestroyEntity(UINT ownerId)
	{
		auto it = m_entityMap.find(ownerId);
		if (it == m_entityMap.end())
			return;
		if (m_world.valid(it->second))
			m_world.destroy(it->second);
		m_entityMap.erase(it);
	}

	/**
	 *  判断 ownerId 是否已登记实体。
	 *  锁要求: 调用者必须持 m_world.m_mutex 共享锁 (SRLock) 或独占锁 (SWLock)
	 */
	bool HasEntity(UINT ownerId)
	{
		return m_entityMap.find(ownerId) != m_entityMap.end();
	}

private:
	ECSRegistry m_world;
	std::unordered_map<UINT, entity_t> m_entityMap;
};
