#pragma once

#include "ECS.h"
#include "TimerComponent.h"
#include "EcsWorld.h"
#include "TimerSystem.h"
#include <unordered_map>
#include <array>

class CAliveObject;

/**
 *  AliveTimerSystem — 生物对象定时器管理器
 *
 *  职责: 管理 CAliveObject 的 3 个共享 TimerComponent (TMR_ACTION / TMR_HP_RECOVER / TMR_MP_RECOVER)。
 *  拥有独立的 ECSRegistry (不与其他组件共享)。
 */
class AliveTimerSystem : public xSingletonClass<AliveTimerSystem>
{
public:
	AliveTimerSystem();
	~AliveTimerSystem() = default;

	// 批量创建所有定时器实体
	VOID CreateAliveTimers(CAliveObject* pObj);

	// 定时器操作
	BOOL  CheckTimer(UINT objId, TimerType type, DWORD intervalMs);
	VOID  ResetTimer(UINT objId, TimerType type);

	// 对象销毁时清理
	VOID OnObjectDestroy(UINT objId);

private:
	static inline int AliveTimerTypeToIdx(TimerType t)
	{
		switch (t)
		{
		case TimerType::TMR_ACTION:     return 0;
		case TimerType::TMR_HP_RECOVER: return 1;
		case TimerType::TMR_MP_RECOVER: return 2;
		default: return -1;
		}
	}

	static constexpr size_t ALIVE_TMR_COUNT = 3;
	TimerSystem<ALIVE_TMR_COUNT> m_timerSystem;
};
