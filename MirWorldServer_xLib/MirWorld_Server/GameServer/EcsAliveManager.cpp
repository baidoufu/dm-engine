#include "stdafx.h"
#include "EcsAliveManager.h"

EcsAliveManager::EcsAliveManager()
	: m_timerSystem(m_world, AliveTimerTypeToIdx)
{
}

EcsAliveManager::~EcsAliveManager()
{
	m_world.clear_all();
}

// ==================================================
//  定时器 (委托到 TimerSystem)
// ==================================================

VOID EcsAliveManager::CreateAliveTimers(CAliveObject* pObj)
{
	if (!pObj) return;
	UINT id = pObj->GetId();
	if (id == 0) return;
	static constexpr TimerType s_aliveTimerTypes[ALIVE_TMR_COUNT] = {
		TimerType::TMR_ACTION,
		TimerType::TMR_HP_RECOVER,
		TimerType::TMR_MP_RECOVER,
	};

	for (TimerType t : s_aliveTimerTypes)
		m_timerSystem.GetOrCreateTimerEntity(id, t);
}

BOOL EcsAliveManager::CheckTimer(UINT objId, TimerType type, DWORD intervalMs)
{
	return m_timerSystem.CheckTimer(objId, type, intervalMs);
}

VOID EcsAliveManager::ResetTimer(UINT objId, TimerType type)
{
	m_timerSystem.ResetTimer(objId, type);
}

// ==================================================
//  生命周期
// ==================================================

VOID EcsAliveManager::OnObjectDestroy(UINT objId)
{
	m_timerSystem.Cleanup(objId);
}
