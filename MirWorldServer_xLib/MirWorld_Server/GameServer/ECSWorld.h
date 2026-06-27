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
 *    所有方法通过 ECSRegistry 的 recursive_mutex 加锁, 支持同线程递归持锁。
 *    System 方法已持有锁时调用本类方法, 会递归获取同一把锁, 无死锁风险。
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

	/**
	 *  创建空实体并登记到映射表。
	 *  幂等: 若 ownerId 已存在, 直接返回既有实体, 不重复创建。
	 *  调用方负责在新建实体上 emplace 所需组件(通过 world.has<T> 判断是否新建)。
	 *  返回 INVALID_ENTITY 表示创建失败(实体数达上限)。
	 */
	entity_t CreateEntity(UINT ownerId)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
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
	 */
	entity_t GetEntity(UINT ownerId)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		auto it = m_entityMap.find(ownerId);
		return (it != m_entityMap.end()) ? it->second : INVALID_ENTITY;
	}

	/**
	 *  销毁 ownerId 对应的实体并从映射表移除。
	 *  幂等: 不存在时安全无操作。
	 *  ECSRegistry::destroy 会自动移除该实体上的所有组件。
	 */
	void DestroyEntity(UINT ownerId)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		auto it = m_entityMap.find(ownerId);
		if (it == m_entityMap.end())
			return;
		if (m_world.valid(it->second))
			m_world.destroy(it->second);
		m_entityMap.erase(it);
	}

	/**
	 *  判断 ownerId 是否已登记实体。
	 */
	bool HasEntity(UINT ownerId)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		return m_entityMap.find(ownerId) != m_entityMap.end();
	}

private:
	ECSRegistry m_world;
	std::unordered_map<UINT, entity_t> m_entityMap;
};
