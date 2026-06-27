#include "StdAfx.h"
#include "PlayerComponentManager.h"
#include "HumanPlayer.h"
#include "AliveComponentsManager.h"

VOID PlayerComponentManager::CreatePlayerComponents(CHumanPlayer* pPlayer)
{
	if (!pPlayer) return;
	UINT id = pPlayer->GetId();
	if (id == 0) return;

	{
		auto* ecsWorld = ECSWorld::GetInstance();
		auto& world = ecsWorld->GetWorld();
		std::lock_guard<ECSRegistry> lock(world);

		entity_t e = ecsWorld->CreateEntity(id);
		if (e == INVALID_ENTITY) return;

		if (world.has<PlayerTimerComponent>(e)) return;

		int now = CFrameTime::GetFrameTime();

		auto& pt = world.emplace<PlayerTimerComponent>(e);
		pt.lastTickMs.fill(now);

		world.emplace<RateLimitComponent>(e);
		world.emplace<ShieldStateComponent>(e);
		world.emplace<SpecialEquipComponent>(e);
	}

	AliveComponentsManager::GetInstance()->CreateAliveComponents(pPlayer);
}

VOID PlayerComponentManager::DestroyPlayerComponents(UINT ownerId)
{
	ECSWorld::GetInstance()->DestroyEntity(ownerId);
}

BOOL PlayerComponentManager::CheckPlayerTimer(UINT ownerId, TimerType type, DWORD intervalMs)
{
	const int idx = PlayerTimerTypeToIdx(type);
	if (idx < 0) return FALSE;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(ownerId);
	if (e == INVALID_ENTITY) return FALSE;
	auto* tc = world.get<PlayerTimerComponent>(e);
	if (!tc) return FALSE;

	int now = CFrameTime::GetFrameTime();
	if (GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs)
	{
		tc->lastTickMs[idx] += (int)intervalMs;
		return TRUE;
	}
	return FALSE;
}

BOOL PlayerComponentManager::CheckPlayerTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs)
{
	const int idx = PlayerTimerTypeToIdx(type);
	if (idx < 0) { outLastTickMs = 0; return FALSE; }

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(ownerId);
	if (e == INVALID_ENTITY) { outLastTickMs = 0; return FALSE; }
	auto* tc = world.get<PlayerTimerComponent>(e);
	if (!tc) { outLastTickMs = 0; return FALSE; }

	outLastTickMs = tc->lastTickMs[idx];
	int now = CFrameTime::GetFrameTime();
	return GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs;
}

VOID PlayerComponentManager::ResetPlayerTimer(UINT ownerId, TimerType type)
{
	const int idx = PlayerTimerTypeToIdx(type);
	if (idx < 0) return;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(ownerId);
	if (e == INVALID_ENTITY) return;
	auto* tc = world.get<PlayerTimerComponent>(e);
	if (tc) tc->lastTickMs[idx] = CFrameTime::GetFrameTime();
}

VOID PlayerComponentManager::OffsetPlayerTimer(UINT ownerId, TimerType type, int offsetMs)
{
	const int idx = PlayerTimerTypeToIdx(type);
	if (idx < 0) return;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(ownerId);
	if (e == INVALID_ENTITY) return;
	auto* tc = world.get<PlayerTimerComponent>(e);
	if (tc) tc->lastTickMs[idx] -= offsetMs;
}

int PlayerComponentManager::GetPlayerTimerLastTick(UINT ownerId, TimerType type)
{
	const int idx = PlayerTimerTypeToIdx(type);
	if (idx < 0) return 0;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(ownerId);
	if (e == INVALID_ENTITY) return 0;
	auto* tc = world.get<PlayerTimerComponent>(e);
	return tc ? tc->lastTickMs[idx] : 0;
}

RateLimitComponent* PlayerComponentManager::GetRateLimit(CHumanPlayer* pPlayer)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(pPlayer->GetId());
	if (e == INVALID_ENTITY) return nullptr;
	return world.get<RateLimitComponent>(e);
}

BOOL PlayerComponentManager::TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act)
{
	auto* rl = GetRateLimit(pPlayer);
	if (!rl) return TRUE;
	if (act == RateLimitComponent::ACT_SPECIAL_ATTACK)
		rl->SetInterval(RateLimitComponent::ACT_SPECIAL_ATTACK,
			rl->intervals[RateLimitComponent::ACT_SPECIAL_ATTACK] - 80 * pPlayer->GetPropValue(PI_ATTACKSPEED));
	return rl->TryExecute(act, CFrameTime::GetFrameTime());
}

VOID PlayerComponentManager::GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms)
{
	auto* rl = GetRateLimit(pPlayer);
	if (rl) rl->SetInterval(act, ms);
}

ShieldStateComponent* PlayerComponentManager::GetShieldState(CHumanPlayer* pPlayer)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(pPlayer->GetId());
	if (e == INVALID_ENTITY) return nullptr;
	return world.get<ShieldStateComponent>(e);
}

SpecialEquipComponent* PlayerComponentManager::GetSpecialEquip(CHumanPlayer* pPlayer)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(pPlayer->GetId());
	if (e == INVALID_ENTITY) return nullptr;
	return world.get<SpecialEquipComponent>(e);
}

DWORD PlayerComponentManager::GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func)
{
	auto* se = GetSpecialEquip(pPlayer);
	return se ? se->GetPosFlag(func) : 0;
}
