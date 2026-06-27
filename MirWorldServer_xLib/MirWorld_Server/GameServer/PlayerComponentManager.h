#pragma once

#include "ECSWorld.h"
#include "TimerComponent.h"
#include "RateLimitComponent.h"
#include "ShieldStateComponent.h"
#include "SpecialEquipComponent.h"

class CHumanPlayer;
/// <summary>
/// 独立管理 CHumanPlayer 使用的组件
/// </summary>
class PlayerComponentManager : public xSingletonClass<PlayerComponentManager>
{
public:
	PlayerComponentManager()  = default;
	~PlayerComponentManager() = default;

	VOID CreatePlayerComponents(CHumanPlayer* pPlayer);
	VOID DestroyPlayerComponents(UINT ownerId);

	BOOL  CheckPlayerTimer(UINT ownerId, TimerType type, DWORD intervalMs);
	BOOL  CheckPlayerTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs);
	VOID  ResetPlayerTimer(UINT ownerId, TimerType type);
	VOID  OffsetPlayerTimer(UINT ownerId, TimerType type, int offsetMs);
	int   GetPlayerTimerLastTick(UINT ownerId, TimerType type);

	RateLimitComponent* GetRateLimit(CHumanPlayer* pPlayer);
	BOOL  TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act);
	VOID  GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms);

	ShieldStateComponent* GetShieldState(CHumanPlayer* pPlayer);

	SpecialEquipComponent* GetSpecialEquip(CHumanPlayer* pPlayer);
	DWORD GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func);
};
