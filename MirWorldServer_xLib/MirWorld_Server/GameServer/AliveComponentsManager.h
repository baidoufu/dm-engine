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

	BOOL  CheckAliveTimer(UINT objId, TimerType type, DWORD intervalMs);
	VOID  ResetAliveTimer(UINT objId, TimerType type);
};
