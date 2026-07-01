#pragma once

#include "ECSWorld.h"
#include "TimerComponent.h"

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

	// 技能/状态免疫 (管理 AliveImmunityComponent)
	BOOL  CheckImmunityTimer(entity_t e, int wMagicId);
	VOID  SetImmunityTimer(entity_t e, int wMagicId, DWORD nTime);
	BOOL  CheckStatusImmunity(entity_t e, int index);
	VOID  SetStatusImmunity(entity_t e, int index, DWORD nTime);
};
