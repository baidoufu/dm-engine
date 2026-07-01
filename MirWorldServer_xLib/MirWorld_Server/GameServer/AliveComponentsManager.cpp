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
	SWLock lock(world.m_mutex);

	entity_t e = ecsWorld->CreateEntity(id);
	if (e == INVALID_ENTITY) return;

	// 缓存 ECS 实体句柄到 OOP 对象, 后续热路径绕过 map 查找
	pObj->SetECSEntity(e);

	if (world.has_nolock<TimerComponent>(e)) return;

	int now = CFrameTime::GetFrameTime();
	auto& at = world.emplace<TimerComponent>(e);
	at.lastTickMs.fill(now);

	// 同时创建技能/状态免疫组件 (玩家+怪物共享)
	world.emplace<AliveImmunityComponent>(e);
}

BOOL AliveComponentsManager::CheckAliveTimer(entity_t e, TimerType type, DWORD intervalMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return FALSE;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	// 共享锁读取: 检查定时器是否到期
	{
		SRLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) < (int)intervalMs)
			return FALSE;
	}
	// 到期: 升级为独占锁写入新时间戳
	{
		SWLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs)
		{
			tc->lastTickMs[idx] += (int)intervalMs;
			return TRUE;
		}
		return FALSE;
	}
}

VOID AliveComponentsManager::ResetAliveTimer(entity_t e, TimerType type)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* tc = world.get_nolock<TimerComponent>(e);
	if (tc) tc->lastTickMs[idx] = CFrameTime::GetFrameTime();
}

// ========== 技能免疫 ==========
BOOL AliveComponentsManager::CheckImmunityTimer(entity_t e, int wMagicId)
{
	const int idx = ImmunitySkillToIdx(wMagicId);
	if (idx < 0 || e == INVALID_ENTITY) return TRUE; // 未知技能默认视为免疫已过期

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SRLock lock(world.m_mutex);

	auto* ic = world.get_nolock<AliveImmunityComponent>(e);
	if (!ic) return TRUE;

	if (ic->skillDurationMs[idx] == 0) return TRUE;
	int now = CFrameTime::GetFrameTime();
	return GetTimeToTime(ic->skillLastTickMs[idx], now) >= (int)ic->skillDurationMs[idx];
}

VOID AliveComponentsManager::SetImmunityTimer(entity_t e, int wMagicId, DWORD nTime)
{
	const int idx = ImmunitySkillToIdx(wMagicId);
	if (idx < 0 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* ic = world.get_nolock<AliveImmunityComponent>(e);
	if (!ic) return;

	ic->skillDurationMs[idx] = nTime;
	ic->skillLastTickMs[idx] = CFrameTime::GetFrameTime();
}

// ========== 状态免疫 (麻痹 SI_PALSY=26) ==========
BOOL AliveComponentsManager::CheckStatusImmunity(entity_t e, int index)
{
	if (index != 26 || e == INVALID_ENTITY) return TRUE; // 仅处理麻痹, 其他状态视为无免疫

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SRLock lock(world.m_mutex);

	auto* ic = world.get_nolock<AliveImmunityComponent>(e);
	if (!ic) return TRUE;

	if (ic->statusDurationMs == 0) return TRUE;
	int now = CFrameTime::GetFrameTime();
	return GetTimeToTime(ic->statusLastTickMs, now) >= (int)ic->statusDurationMs;
}

VOID AliveComponentsManager::SetStatusImmunity(entity_t e, int index, DWORD nTime)
{
	if (index != 26 || e == INVALID_ENTITY) return; // 仅处理麻痹

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* ic = world.get_nolock<AliveImmunityComponent>(e);
	if (!ic) return;

	ic->statusDurationMs = nTime;
	ic->statusLastTickMs = CFrameTime::GetFrameTime();
}
