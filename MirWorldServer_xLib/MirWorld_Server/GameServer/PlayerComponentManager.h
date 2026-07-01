#pragma once

#include "ECSWorld.h"
#include "TimerComponent.h"
#include "RateLimitComponent.h"
#include "ShieldStateComponent.h"
#include "SpecialEquipComponent.h"
#include "StaminaComponent.h"
#include "TaskComponent.h"
#include "FenghaoComponent.h"
#include "AchievementComponent.h"
#include "SocialComponent.h"
#include "ChatComponent.h"
#include "PkComponent.h"
#include "MarketComponent.h"
#include "TitleComponent.h"
#include "ScriptVarComponent.h"
#include "MiscStateComponent.h"
#include "ZhenBaoComponent.h"
#include "RecalcCacheComponent.h"
#include "UpgradeItemComponent.h"

class CHumanPlayer;
/// <summary>
/// 独立管理 CHumanPlayer 使用的组件
/// </summary>
class PlayerComponentManager : public xSingletonClass<PlayerComponentManager>
{
public:
	PlayerComponentManager()  = default;
	~PlayerComponentManager() = default;

	// 初始化池指针缓存 (必须在所有组件类型注册后、首帧 Update 前调用一次)
	// 之后所有 GetXxx 方法走池级锁, 不再获取全局 m_mutex
	VOID InitPoolCache();

	VOID CreatePlayerComponents(CHumanPlayer* pPlayer);
	VOID DestroyPlayerComponents(UINT ownerId);

	BOOL  CheckPlayerTimer(entity_t e, TimerType type, DWORD intervalMs);
	BOOL  CheckPlayerTimerNoReset(entity_t e, TimerType type, DWORD intervalMs, int& outLastTickMs);

	VOID  ResetPlayerTimer(entity_t e, TimerType type);
	VOID  OffsetPlayerTimer(entity_t e, TimerType type, int offsetMs);
	int   GetPlayerTimerLastTick(entity_t e, TimerType type);

	RateLimitComponent* GetRateLimit(CHumanPlayer* pPlayer);
	BOOL  TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act);
	VOID  GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms);

	ShieldStateComponent* GetShieldState(CHumanPlayer* pPlayer);

	SpecialEquipComponent* GetSpecialEquip(CHumanPlayer* pPlayer);
	DWORD GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func);

	StaminaComponent* GetStamina(CHumanPlayer* pPlayer);
	TaskComponent* GetTask(CHumanPlayer* pPlayer);
	FenghaoComponent* GetFenghao(CHumanPlayer* pPlayer);
	AchievementComponent* GetAchievement(CHumanPlayer* pPlayer);
	SocialComponent* GetSocial(CHumanPlayer* pPlayer);
	ChatComponent* GetChat(CHumanPlayer* pPlayer);
	PkComponent* GetPk(CHumanPlayer* pPlayer);
	MarketComponent* GetMarket(CHumanPlayer* pPlayer);
	TitleComponent* GetTitle(CHumanPlayer* pPlayer);
	ScriptVarComponent* GetScriptVar(CHumanPlayer* pPlayer);
	MiscStateComponent* GetMiscState(CHumanPlayer* pPlayer);
	ZhenBaoComponent* GetZhenBao(CHumanPlayer* pPlayer);
	RecalcCacheComponent* GetRecalcCache(CHumanPlayer* pPlayer);
	UpgradeItemComponent* GetUpgradeItem(CHumanPlayer* pPlayer);

private:
	// ==== 池指针缓存 (InitPoolCache 后稳定, 绕过全局 m_mutex) ====
	ComponentPool<RateLimitComponent>*    m_rateLimitPool   = nullptr;
	ComponentPool<ShieldStateComponent>*  m_shieldStatePool = nullptr;
	ComponentPool<SpecialEquipComponent>* m_specialEquipPool = nullptr;
	ComponentPool<StaminaComponent>*      m_staminaPool     = nullptr;
	ComponentPool<TaskComponent>*         m_taskPool        = nullptr;
	ComponentPool<FenghaoComponent>*      m_fenghaoPool     = nullptr;
	ComponentPool<AchievementComponent>*  m_achievementPool = nullptr;
	ComponentPool<SocialComponent>*       m_socialPool      = nullptr;
	ComponentPool<ChatComponent>*         m_chatPool        = nullptr;
	ComponentPool<PkComponent>*           m_pkPool          = nullptr;
	ComponentPool<MarketComponent>*       m_marketPool      = nullptr;
	ComponentPool<TitleComponent>*        m_titlePool       = nullptr;
	ComponentPool<ScriptVarComponent>*    m_scriptVarPool   = nullptr;
	ComponentPool<MiscStateComponent>*    m_miscStatePool   = nullptr;
	ComponentPool<ZhenBaoComponent>*      m_zhenBaoPool     = nullptr;
	ComponentPool<RecalcCacheComponent>*  m_recalcCachePool = nullptr;
	ComponentPool<UpgradeItemComponent>*  m_upgradeItemPool = nullptr;

	// 辅助: 从 entity 获取组件 (无全局锁, 直接走池级 SRLock)
	template<typename T>
	T* GetFromPool(ComponentPool<T>* pool, entity_t e)
	{
		return pool ? pool->get(e) : nullptr;
	}
};
