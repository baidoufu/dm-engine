#include "StdAfx.h"
#include "SpecialEquipSystem.h"
#include "HumanPlayer.h"

entity_t SpecialEquipSystem::GetOrCreateEntity(UINT objId)
{
	// 调用方必须已持有 ECSRegistry 锁
	auto& world = ECSWorld::GetInstance()->GetWorld();
	auto it = m_idToEntity.find(objId);
	if (it != m_idToEntity.end()) return it->second;

	entity_t e = world.create();
	world.emplace<SpecialEquipComponent>(e);
	m_idToEntity[objId] = e;
	return e;
}

SpecialEquipComponent* SpecialEquipSystem::GetSpecialEquip(CHumanPlayer* pPlayer)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	return world.get<SpecialEquipComponent>(e);
}

DWORD SpecialEquipSystem::GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	return world.get<SpecialEquipComponent>(e)->GetPosFlag(func);
}

VOID SpecialEquipSystem::OnPlayerLogout(UINT id)
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
