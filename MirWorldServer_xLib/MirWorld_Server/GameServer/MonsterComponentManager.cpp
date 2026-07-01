#include "StdAfx.h"
#include "MonsterComponentManager.h"
#include "AliveComponentsManager.h"
#include "MonsterEx.h"

// 池缓存一次性初始化 (线程安全)
static std::once_flag s_monsterPoolCacheInitFlag;

// ========== 初始化池指针缓存 ==========
VOID MonsterComponentManager::InitPoolCache()
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	m_monsterStatePool = world.get_pool<MonsterStateComponent>();
	m_petPool          = world.get_pool<PetComponent>();
}

VOID MonsterComponentManager::CreateMonsterComponents(CMonsterEx* pObj)
{
	if (!pObj) return;
	UINT id = pObj->GetId();
	if (id == 0) return;
	// 先创建公共活体组件（含实体创建 + TimerComponent）
	AliveComponentsManager::GetInstance()->CreateAliveComponents(pObj);
	// 再创建专属组件
	{
		auto& world = ECSWorld::GetInstance()->GetWorld();
		SWLock lock(world.m_mutex);

		entity_t e = pObj->GetECSEntity();
		if (e == INVALID_ENTITY) return;

		if (!world.has<MonsterStateComponent>(e))
		{
			auto& st = world.emplace<MonsterStateComponent>(e);
			st.wCurHp = 0;
			st.wCurMp = 0;
		}

		if (!world.has<TimerComponent>(e))
		{
			auto& mt = world.emplace<TimerComponent>(e);
			int now = CFrameTime::GetFrameTime();
			mt.lastTickMs.fill(now);
		}
	}

	if (!m_bPoolCacheInited)
	{
		std::call_once(s_monsterPoolCacheInitFlag, [this]() { InitPoolCache(); });
		m_bPoolCacheInited = TRUE;
	}
}

VOID MonsterComponentManager::DestroyMonsterComponents(UINT objId)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	SWLock lock(world.m_mutex);
	ecsWorld->DestroyEntity(objId);
}

VOID MonsterComponentManager::EnsurePetComponent(CMonsterEx* pObj)
{
	if (!pObj) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return;

	if (!world.has<PetComponent>(e))
		world.emplace<PetComponent>(e);
}

// ========== 怪物专属定时器 ==========
// 与 AliveComponentsManager::CheckAliveTimer 模式一致
// 双层锁: SRLock 快速路径 + SWLock 冷路径
BOOL MonsterComponentManager::CheckMonsterTimer(entity_t e, TimerType type, DWORD intervalMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return FALSE;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	{
		SRLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) < (int)intervalMs)
			return FALSE;
	}
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

VOID MonsterComponentManager::ResetMonsterTimer(entity_t e, TimerType type)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* tc = world.get_nolock<TimerComponent>(e);
	if (tc) tc->lastTickMs[idx] = CFrameTime::GetFrameTime();
}

MonsterStateComponent* MonsterComponentManager::GetMonsterState(CMonsterEx* pObj)
{
	if (!pObj || !m_monsterStatePool) return nullptr;
	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return m_monsterStatePool->get(e);
}

PetComponent* MonsterComponentManager::GetPet(CMonsterEx* pObj)
{
	if (!pObj || !m_petPool) return nullptr;
	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return m_petPool->get(e);
}
