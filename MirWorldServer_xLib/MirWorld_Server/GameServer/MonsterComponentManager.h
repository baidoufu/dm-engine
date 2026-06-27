#pragma once

#include "ECSWorld.h"
#include "MonsterEx.h"

class CMonsterEx;
/// <summary>
/// 独立管理 CMonsterEx 使用的组件
/// </summary>
class MonsterComponentManager : public xSingletonClass<MonsterComponentManager>
{
public:
	MonsterComponentManager()  = default;
	~MonsterComponentManager() = default;

	VOID CreateMonsterComponents(CMonsterEx* pObj);
	VOID DestroyMonsterComponents(UINT objId);
};
