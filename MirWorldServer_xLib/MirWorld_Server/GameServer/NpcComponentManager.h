#pragma once

#include "ECSWorld.h"
#include "NpcStateComponent.h"

class CScriptNpc;

/// <summary>
/// 独立管理 CScriptNpc 使用的组件
/// </summary>
class NpcComponentManager : public xSingletonClass<NpcComponentManager>
{
public:
	NpcComponentManager()  = default;
	~NpcComponentManager() = default;

	// === 生命周期 ===
	VOID CreateNpcComponents(CScriptNpc* pObj);
	VOID DestroyNpcComponents(UINT objId);

	// === 商人组件延迟创建（首次设置货物时调用，幂等） ===
	VOID EnsureMerchantComponent(CScriptNpc* pObj);

	// === 组件访问（热路径优化：绕过全局 m_mutex，走池级锁） ===
	NpcStateComponent*    GetNpcState(CScriptNpc* pObj);
	NpcMerchantComponent* GetNpcMerchant(CScriptNpc* pObj);

private:
	ComponentPool<NpcStateComponent>*    m_npcStatePool    = nullptr;
	ComponentPool<NpcMerchantComponent>* m_npcMerchantPool = nullptr;
	BOOL m_bPoolCacheInited = FALSE;

	VOID InitPoolCache();
};
