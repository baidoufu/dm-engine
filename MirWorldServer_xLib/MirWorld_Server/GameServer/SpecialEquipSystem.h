#pragma once

#include "ECS.h"
#include "SpecialEquipComponent.h"
#include "EcsWorld.h"
#include <unordered_map>

class CHumanPlayer;

/**
 *  SpecialEquipSystem — 特殊装备组件管理器
 */
class SpecialEquipSystem : public xSingletonClass<SpecialEquipSystem>
{
public:
	SpecialEquipSystem()  = default;
	~SpecialEquipSystem() = default;

	SpecialEquipComponent* GetSpecialEquip(CHumanPlayer* pPlayer);
	DWORD GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func);
	VOID OnPlayerLogout(UINT id);

private:
	entity_t GetOrCreateEntity(UINT objId);

	std::unordered_map<UINT, entity_t> m_idToEntity;
};
