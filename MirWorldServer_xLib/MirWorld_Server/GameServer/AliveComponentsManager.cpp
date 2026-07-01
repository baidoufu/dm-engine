#include "StdAfx.h"
#include "AliveComponentsManager.h"
#include "AliveObject.h"
#include "GameWorld.h"

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

	// 统一定时器组件
	if (!world.has<TimerComponent>(e))
	{
		int now = CFrameTime::GetFrameTime();
		auto& tc = world.emplace<TimerComponent>(e);
		tc.lastTickMs.fill(now);
		tc.ownerId = id;
	}

	// 状态过期组件 (预计算 statusExpiredMask / systemFlagExpiredMask)
	if (!world.has<StatusComponent>(e))
	{
		auto& sc = world.emplace<StatusComponent>(e);
		sc.ownerId = id;
	}

	// 同时创建技能/状态免疫组件 (玩家+怪物共享)
	world.emplace<AliveImmunityComponent>(e);

	// 吃药恢复组件 (HP/MP 递增, PotionRecoverSystem 消费)
	if (!world.has<PotionRecoverComponent>(e))
	{
		auto& pc = world.emplace<PotionRecoverComponent>(e);
		pc.ownerId = id;
	}
}

BOOL AliveComponentsManager::CheckAliveTimer(entity_t e, TimerType type, DWORD intervalMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return FALSE;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	// 路径 0 (超快路径): 批量预计算已置位 firedMask → 零锁返回
	{
		SRLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		if (InterlockedAnd((volatile LONG*)&tc->firedMask, ~(1u << idx)) & (1u << idx))
			return TRUE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) < (int)intervalMs)
			return FALSE;
	}
	// 冷路径: 独占锁写入
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

// ========== 统一定时器批量预计算 (单次扫描全部 23 槽) ==========
VOID AliveComponentsManager::BatchPrecomputeTimers(int frameTime)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	CGameWorld* pGameWorld = CGameWorld::GetInstance();
	DWORD justPkInterval  = pGameWorld->GetVar(EVI_GRAYNAMETIME) * 1000;
	DWORD pkPointInterval = pGameWorld->GetVar(EVI_ONEPKPOINTTIME) * 1000;
	DWORD dbSaveInterval = pGameWorld->GetVar(EVI_CHARINFOBACKUPTIME) * 60 * 1000;

	ecs_view<TimerComponent>(world).each([=](TimerComponent& tc) {
		uint32_t mask = 0;

		// === 玩家定时器 (位 0~16) ===

		// TMR_DB_SAVE (0): 动态间隔 = dbSaveBaseInterval + per-player 错峰偏移
		if (dbSaveInterval > 0)
		{
			DWORD dbSaveOffset = (tc.ownerId % 30) * 10 * 1000;
			DWORD totalInterval = dbSaveInterval + dbSaveOffset;
			if (GetTimeToTime(tc.lastTickMs[0], frameTime) >= (int)totalInterval)
			{
				mask |= (1u << 0);
				tc.lastTickMs[0] += (int)totalInterval;
			}
		}

		if (GetTimeToTime(tc.lastTickMs[1], frameTime) >= 6 * 60 * 1000)
		{
			mask |= (1u << 1);
			tc.lastTickMs[1] += 6 * 60 * 1000;
		}
		if (GetTimeToTime(tc.lastTickMs[2], frameTime) >= 1000)
		{
			mask |= (1u << 2);
			tc.lastTickMs[2] += 1000;
		}
		if (justPkInterval > 0 && GetTimeToTime(tc.lastTickMs[4], frameTime) >= (int)justPkInterval)
		{
			mask |= (1u << 4);
			tc.lastTickMs[4] += (int)justPkInterval;
		}
		if (pkPointInterval > 0 && GetTimeToTime(tc.lastTickMs[5], frameTime) >= (int)pkPointInterval)
		{
			mask |= (1u << 5);
			tc.lastTickMs[5] += (int)pkPointInterval;
		}
		if (GetTimeToTime(tc.lastTickMs[6], frameTime) >= 60 * 1000)
		{
			mask |= (1u << 6);
			tc.lastTickMs[6] += 60 * 1000;
		}
		if (GetTimeToTime(tc.lastTickMs[7], frameTime) >= 3000)
		{
			mask |= (1u << 7);
			tc.lastTickMs[7] += 3000;
		}
		if (GetTimeToTime(tc.lastTickMs[8], frameTime) >= 60 * 1000)
		{
			mask |= (1u << 8);
			tc.lastTickMs[8] += 60 * 1000;
		}

		// === 活体定时器 (位 17~19) ===
		if (GetTimeToTime(tc.lastTickMs[18], frameTime) >= 3000)
		{
			mask |= (1u << 18);
			tc.lastTickMs[18] += 3000;
		}
		if (GetTimeToTime(tc.lastTickMs[19], frameTime) >= 3000)
		{
			mask |= (1u << 19);
			tc.lastTickMs[19] += 3000;
		}

		tc.firedMask |= mask;
	});
}

// ========== 状态过期批量预计算 ==========
// 扫描 StatusComponent (SoA), 对比 lastTickMs/durationMs 计算过期的状态位
VOID AliveComponentsManager::BatchPrecomputeStatusExpire(int frameTime)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	ecs_view<StatusComponent>(world).each([frameTime](StatusComponent& sc) {
		uint32_t statusMask = 0;
		uint32_t sysFlagMask = 0;
		uint64_t active = sc.activeSlotMask;
		if (!active) return;  // 无激活槽位, 直接跳过
		// m_Status 槽位 (索引 0~31): 检查 active 低32位
		{
			uint32_t activeStatus = (uint32_t)(active & 0xFFFFFFFFull);
			while (activeStatus)
			{
				unsigned long i;
				_BitScanForward(&i, activeStatus);
				activeStatus &= ~(1u << i);
				DWORD dur = sc.durationMs[i];
				if (dur != 0 && dur != 0xFFFFFFFF &&
					GetTimeToTime(sc.lastTickMs[i], frameTime) >= (int)dur)
					statusMask |= (1u << i);
			}
		}
		// m_SystemFlag 槽位 (索引 32~63): 检查 active 高32位
		{
			uint32_t activeSysFlag = (uint32_t)(active >> 32);
			while (activeSysFlag)
			{
				unsigned long i;
				_BitScanForward(&i, activeSysFlag);
				activeSysFlag &= ~(1u << i);
				int ci = i + 32;
				DWORD dur = sc.durationMs[ci];
				if (dur != 0 && dur != 0xFFFFFFFF &&
					GetTimeToTime(sc.lastTickMs[ci], frameTime) >= (int)dur)
					sysFlagMask |= (1u << i);
			}
		}

		if (statusMask)
			sc.statusExpiredMask |= statusMask;
		if (sysFlagMask)
			sc.systemFlagExpiredMask |= sysFlagMask;
	});
}

// ========== 技能免疫 ==========
BOOL AliveComponentsManager::CheckImmunityTimer(entity_t e, int wMagicId)
{
	const int idx = ImmunitySkillToIdx(wMagicId);
	if (idx < 0 || e == INVALID_ENTITY) return TRUE;

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

BOOL AliveComponentsManager::CheckStatusImmunity(entity_t e, int index)
{
	if (index != 26 || e == INVALID_ENTITY) return TRUE;

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
	if (index != 26 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* ic = world.get_nolock<AliveImmunityComponent>(e);
	if (!ic) return;

	ic->statusDurationMs = nTime;
	ic->statusLastTickMs = CFrameTime::GetFrameTime();
}
