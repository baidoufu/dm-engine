#include "StdAfx.h"
#include "ShieldStateSystem.h"
#include "HumanPlayer.h"

entity_t ShieldStateSystem::GetOrCreateEntity(UINT objId)
{
	// 调用方必须已持有 ECSRegistry 锁
	auto& world = ECSWorld::GetInstance()->GetWorld();
	auto it = m_idToEntity.find(objId);
	if (it != m_idToEntity.end()) return it->second;

	entity_t e = world.create();
	world.emplace<ShieldStateComponent>(e);
	m_idToEntity[objId] = e;
	return e;
}

ShieldStateComponent* ShieldStateSystem::GetShieldState(CHumanPlayer* pPlayer)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	return world.get<ShieldStateComponent>(e);
}

VOID ShieldStateSystem::SetShieldCount(CHumanPlayer* pPlayer, int nCount)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	auto* ss = world.get<ShieldStateComponent>(e);
	if (ss) ss->magShieldResCount = nCount;
}

VOID ShieldStateSystem::OnPlayerLogout(UINT id)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	auto it = m_idToEntity.find(id);
	if (it != m_idToEntity.end())
	{
		world.destroy(it->second);
		m_idToEntity.erase(it);
	}
}
