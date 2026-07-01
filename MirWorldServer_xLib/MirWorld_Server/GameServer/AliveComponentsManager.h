#pragma once

#include "ECSWorld.h"
#include "TimerComponent.h"
#include "StatusComponent.h"
#include "PotionRecoverComponent.h"

class CAliveObject;
/// <summary>
/// 独立管理 CAliveObject 使用的组件
/// </summary>
class AliveComponentsManager : public xSingletonClass<AliveComponentsManager>
{
public:
	AliveComponentsManager()  = default;
	~AliveComponentsManager() = default;

	VOID CreateAliveComponents(CAliveObject* pObj);

	BOOL  CheckAliveTimer(entity_t e, TimerType type, DWORD intervalMs);
	VOID  ResetAliveTimer(entity_t e, TimerType type);

	// 统一定时器批量预计算 (单次 ecs_view<TimerComponent> 扫描全部 23 槽)
	VOID  BatchPrecomputeTimers(int frameTime);

	// 状态过期批量预计算 (ecs_view<StatusComponent> 扫描, 预填 statusExpiredMask)
	VOID  BatchPrecomputeStatusExpire(int frameTime);

	// 技能/状态免疫 (管理 AliveImmunityComponent)
	BOOL  CheckImmunityTimer(entity_t e, int wMagicId);
	VOID  SetImmunityTimer(entity_t e, int wMagicId, DWORD nTime);
	BOOL  CheckStatusImmunity(entity_t e, int index);
	VOID  SetStatusImmunity(entity_t e, int index, DWORD nTime);
};
