#include "StdAfx.h"
#include "NpcComponentManager.h"
#include "AliveComponentsManager.h"
#include "ScriptNpc.h"

// 池缓存一次性初始化 (线程安全)
static std::once_flag s_npcPoolCacheInitFlag;

// ========== 初始化池指针缓存 ==========
VOID NpcComponentManager::InitPoolCache()
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	m_npcStatePool    = world.get_pool<NpcStateComponent>();
	m_npcMerchantPool = world.get_pool<NpcMerchantComponent>();
}

// ========== 创建 NPC 专属 ECS 组件 ==========
VOID NpcComponentManager::CreateNpcComponents(CScriptNpc* pObj)
{
	if (!pObj) return;
	UINT id = pObj->GetId();
	if (id == 0) return;

	// 先创建公共活体组件（含实体创建 + TimerComponent + AliveImmunityComponent）
	AliveComponentsManager::GetInstance()->CreateAliveComponents(pObj);

	// 再创建 NPC 专属组件
	{
		auto& world = ECSWorld::GetInstance()->GetWorld();
		SWLock lock(world.m_mutex);

		entity_t e = pObj->GetECSEntity();
		if (e == INVALID_ENTITY) return;

		if (!world.has<NpcStateComponent>(e))
		{
			auto& st = world.emplace<NpcStateComponent>(e);
			st.nView    = pObj->GetView();
			st.StoreId  = pObj->GetStoreId();
			st.bTalk    = pObj->IsTalk();
			st.bIsNpc   = pObj->IsNPC();
			st.nDistance = pObj->GetDistance();
			st.fSandCityMerchant = pObj->IsSandCityMerchant();
		}

		// 始终创建商人组件 (所有NPC都有买卖百分比默认值)
		if (!world.has<NpcMerchantComponent>(e))
		{
			auto& mc = world.emplace<NpcMerchantComponent>(e);
			mc.fBuyPercent  = pObj->GetBuyPercent();
			mc.fSellPercent = pObj->GetSellPercent();
		}
	}

	if (!m_bPoolCacheInited)
	{
		std::call_once(s_npcPoolCacheInitFlag, [this]() { InitPoolCache(); });
		m_bPoolCacheInited = TRUE;
	}
}

// ========== 销毁 NPC 专属 ECS 组件 ==========
VOID NpcComponentManager::DestroyNpcComponents(UINT objId)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	SWLock lock(world.m_mutex);
	ecsWorld->DestroyEntity(objId);
}

// ========== 延迟创建商人组件（幂等） ==========
VOID NpcComponentManager::EnsureMerchantComponent(CScriptNpc* pObj)
{
	if (!pObj) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return;

	if (!world.has<NpcMerchantComponent>(e))
		world.emplace<NpcMerchantComponent>(e);

	if (!m_bPoolCacheInited)
	{
		std::call_once(s_npcPoolCacheInitFlag, [this]() { InitPoolCache(); });
		m_bPoolCacheInited = TRUE;
	}
}

// ========== 组件访问器 ==========
NpcStateComponent* NpcComponentManager::GetNpcState(CScriptNpc* pObj)
{
	if (!pObj || !m_npcStatePool) return nullptr;
	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return m_npcStatePool->get(e);
}

NpcMerchantComponent* NpcComponentManager::GetNpcMerchant(CScriptNpc* pObj)
{
	if (!pObj || !m_npcMerchantPool) return nullptr;
	entity_t e = pObj->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return m_npcMerchantPool->get(e);
}
