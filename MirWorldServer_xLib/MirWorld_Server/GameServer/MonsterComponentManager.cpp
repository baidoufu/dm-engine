#include "StdAfx.h"
#include "MonsterComponentManager.h"
#include "AliveComponentsManager.h"

VOID MonsterComponentManager::CreateMonsterComponents(CMonsterEx* pObj)
{
	if (!pObj) return;
	AliveComponentsManager::GetInstance()->CreateAliveComponents(pObj);
}

VOID MonsterComponentManager::DestroyMonsterComponents(UINT objId)
{
	ECSWorld::GetInstance()->DestroyEntity(objId);
}
