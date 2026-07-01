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
///
/// 线程安全:
///   GetXxx 方法返回 T* 裸指针, 池级 SRLock 在返回时释放。
///   调用者必须在持有全局 SRLock 或确保实体不被并发销毁的前提下使用。
///   推荐: 对于跨线程访问, 使用 LockedComponentRef<T> 包装 (见 ECSComponentPool.h)。
/// </summary>
class PlayerComponentManager : public xSingletonClass<PlayerComponentManager>
{
public:
	PlayerComponentManager()  = default;
	~PlayerComponentManager() = default;

	// 初始化池指针缓存 (必须在所有组件类型注册后、首帧 Update 前调用一次)
	// 线程安全: 应在单线程启动阶段调用, 或调用者持有全局 SWLock。
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

	// 辅助: 从 entity 获取组件 (池级 SRLock, 无全局锁)
	// 注意: 返回 T* 后锁释放, 指针仅在调用者持有外部锁（全局 SRLock）或
	//        确保实体不被并发销毁的前提下有效。
	// 跨线程安全访问请用 WithComponentLocked 模式。
	template<typename T>
	T* GetFromPool(ComponentPool<T>* pool, entity_t e)
	{
		return pool ? pool->get(e) : nullptr;
	}

	// 持锁访问: 在整个 lambda 执行期间持有池级 SRLock, 消除 TOCTOU。
	template<typename T, typename Func>
	decltype(auto) WithComponentLocked(ComponentPool<T>* pool, entity_t e, Func&& func)
	{
		if (!pool) return func(static_cast<T*>(nullptr));
		SRLock lock(pool->mutex());
		T* ptr = pool->get_nolock(e);
		return func(ptr);
	}
};
