#pragma once

#include "ECS.h"
#include "ECSRateLimit.h"
#include "ECSShieldState.h"
#include "ECSSpecialEquip.h"
#include "ECSTimer.h"
#include "TimerSystem.h"
#include <unordered_map>
#include <array>

class CAliveObject;

/**
 *  ECS 生物组件管理器
 *
 *  职责: 管理 CAliveObject 共享组件的 ECS 注册表、实体映射和生命周期。
 *        当前管理的组件: TimerComponent (TMR_ACTION / TMR_HP_RECOVER / TMR_MP_RECOVER)
 */
class EcsAliveManager : public xSingletonClass<EcsAliveManager>
{
public:
	EcsAliveManager();
	~EcsAliveManager();

	// ========== 定时器 (CAliveObject 共享) ==========
	// 批量创建所有定时器实体
	VOID CreateAliveTimers(CAliveObject* pObj);
	// 检查定时器是否到期 (替代原 CServerTimer::IsTimeOut)，到期自动重置
	BOOL  CheckTimer(UINT objId, TimerType type, DWORD intervalMs);
	// 重置定时器 (替代原 CServerTimer::Savetime)
	VOID ResetTimer(UINT objId, TimerType type);

	// ========== 生命周期 ==========
	VOID OnObjectDestroy(UINT objId);

private:
	// 将 TimerType 映射到本地数组索引 (0..2)
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

	// === 统一 ECS 注册表 ===
	ECSRegistry  m_world;

	// === 共享定时器系统 ===
	static constexpr size_t ALIVE_TMR_COUNT = 3;
	TimerSystem<ALIVE_TMR_COUNT> m_timerSystem;
};
