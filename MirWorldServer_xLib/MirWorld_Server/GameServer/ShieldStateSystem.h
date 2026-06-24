#pragma once

#include "ECS.h"
#include "ShieldStateComponent.h"
#include "EcsWorld.h"
#include <unordered_map>

class CHumanPlayer;

/**
 *  ShieldStateSystem — 护盾状态组件管理器
 */
class ShieldStateSystem : public xSingletonClass<ShieldStateSystem>
{
public:
	ShieldStateSystem()  = default;
	~ShieldStateSystem() = default;

	ShieldStateComponent* GetShieldState(CHumanPlayer* pPlayer);
	VOID SetShieldCount(CHumanPlayer* pPlayer, int nCount);
	VOID OnPlayerLogout(UINT id);

private:
	entity_t GetOrCreateEntity(UINT objId);

	std::unordered_map<UINT, entity_t> m_idToEntity;
};
