#include "StdAfx.h"
#include "AliveComponentsManager.h"
#include "AliveObject.h"

VOID AliveComponentsManager::CreateAliveComponents(CAliveObject* pObj)
{
	if (!pObj) return;
	UINT id = pObj->GetId();
	if (id == 0) return;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->CreateEntity(id);
	if (e == INVALID_ENTITY) return;

	if (world.has<AliveTimerComponent>(e)) return;

	int now = CFrameTime::GetFrameTime();
	auto& at = world.emplace<AliveTimerComponent>(e);
	at.lastTickMs.fill(now);
}

BOOL AliveComponentsManager::CheckAliveTimer(UINT objId, TimerType type, DWORD intervalMs)
{
	const int idx = AliveTimerTypeToIdx(type);
	if (idx < 0) return FALSE;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(objId);
	if (e == INVALID_ENTITY) return FALSE;
	auto* tc = world.get<AliveTimerComponent>(e);
	if (!tc) return FALSE;

	int now = CFrameTime::GetFrameTime();
	if (GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs)
	{
		tc->lastTickMs[idx] += (int)intervalMs;
		return TRUE;
	}
	return FALSE;
}

VOID AliveComponentsManager::ResetAliveTimer(UINT objId, TimerType type)
{
	const int idx = AliveTimerTypeToIdx(type);
	if (idx < 0) return;

	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);

	entity_t e = ecsWorld->GetEntity(objId);
	if (e == INVALID_ENTITY) return;
	auto* tc = world.get<AliveTimerComponent>(e);
	if (tc) tc->lastTickMs[idx] = CFrameTime::GetFrameTime();
}
