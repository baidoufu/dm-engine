#pragma once

#include "ECSWorld.h"
#include "TimerComponent.h"
#include "MonsterStateComponent.h"
#include "PetComponent.h"

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

	// 宠物组件延迟创建（首次设置主人时调用，幂等）
	VOID EnsurePetComponent(CMonsterEx* pObj);

	// 怪物专属定时器（管理 TimerComponent 怪物槽位）
	BOOL  CheckMonsterTimer(entity_t e, TimerType type, DWORD intervalMs);
	VOID  ResetMonsterTimer(entity_t e, TimerType type);

	// 组件访问（热路径优化：绕过全局m_mutex，走池级锁）
	MonsterStateComponent* GetMonsterState(CMonsterEx* pObj);
	PetComponent*          GetPet(CMonsterEx* pObj);

private:
	ComponentPool<MonsterStateComponent>* m_monsterStatePool = nullptr;
	ComponentPool<PetComponent>*          m_petPool          = nullptr;
	BOOL m_bPoolCacheInited = FALSE;

	VOID InitPoolCache();
};
