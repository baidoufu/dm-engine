#include "stdafx.h"
#include "AliveTimerSystem.h"

AliveTimerSystem::AliveTimerSystem()
	: m_timerSystem(ECSWorld::GetInstance()->GetWorld(), AliveTimerTypeToIdx)
{
}

VOID AliveTimerSystem::CreateAliveTimers(CAliveObject* pObj)
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

BOOL AliveTimerSystem::CheckTimer(UINT objId, TimerType type, DWORD intervalMs)
{
	return m_timerSystem.CheckTimer(objId, type, intervalMs);
}

VOID AliveTimerSystem::ResetTimer(UINT objId, TimerType type)
{
	m_timerSystem.ResetTimer(objId, type);
}

VOID AliveTimerSystem::OnObjectDestroy(UINT objId)
{
	m_timerSystem.Cleanup(objId);
}
