#include "StdAfx.h"
#include "PlayerComponentManager.h"
#include "HumanPlayer.h"
#include "AliveComponentsManager.h"

VOID PlayerComponentManager::InitPoolCache()
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	m_rateLimitPool    = world.get_pool<RateLimitComponent>();
	m_shieldStatePool  = world.get_pool<ShieldStateComponent>();
	m_specialEquipPool = world.get_pool<SpecialEquipComponent>();
	m_staminaPool      = world.get_pool<StaminaComponent>();
	m_taskPool         = world.get_pool<TaskComponent>();
	m_fenghaoPool      = world.get_pool<FenghaoComponent>();
	m_achievementPool  = world.get_pool<AchievementComponent>();
	m_socialPool       = world.get_pool<SocialComponent>();
	m_chatPool         = world.get_pool<ChatComponent>();
	m_pkPool           = world.get_pool<PkComponent>();
	m_marketPool       = world.get_pool<MarketComponent>();
	m_titlePool        = world.get_pool<TitleComponent>();
	m_scriptVarPool    = world.get_pool<ScriptVarComponent>();
	m_miscStatePool    = world.get_pool<MiscStateComponent>();
	m_zhenBaoPool      = world.get_pool<ZhenBaoComponent>();
	m_recalcCachePool  = world.get_pool<RecalcCacheComponent>();
	m_upgradeItemPool  = world.get_pool<UpgradeItemComponent>();
}

VOID PlayerComponentManager::CreatePlayerComponents(CHumanPlayer* pPlayer)
{
	if (!pPlayer) return;
	UINT id = pPlayer->GetId();
	if (id == 0) return;
	// 创建公共活体组件（含实体创建 + TimerComponent）
	AliveComponentsManager::GetInstance()->CreateAliveComponents(pPlayer);
	// 再创建专属组件
	{
		auto* ecsWorld = ECSWorld::GetInstance();
		auto& world = ecsWorld->GetWorld();
		SWLock lock(world.m_mutex);

		entity_t e = ecsWorld->CreateEntity(id);
		if (e == INVALID_ENTITY) return;

		// 缓存 ECS 实体句柄到 OOP 对象, 后续热路径绕过 map 查找
		pPlayer->SetECSEntity(e);

		// TimerComponent 已由 CreateAliveComponents 创建
		if (!world.has<TimerComponent>(e))
		{
			int now = CFrameTime::GetFrameTime();
			auto& pt = world.emplace<TimerComponent>(e);
			pt.lastTickMs.fill(now);
			pt.ownerId = id;
		}

		// 添加相关组件
		world.emplace<RateLimitComponent>(e);
		world.emplace<ShieldStateComponent>(e);
		world.emplace<SpecialEquipComponent>(e);
		world.emplace<StaminaComponent>(e);
		world.emplace<TaskComponent>(e);
		world.emplace<FenghaoComponent>(e);
		world.emplace<AchievementComponent>(e);
		world.emplace<SocialComponent>(e);
		world.emplace<ChatComponent>(e);
		world.emplace<PkComponent>(e);
		world.emplace<MarketComponent>(e);
		world.emplace<TitleComponent>(e);
		world.emplace<ScriptVarComponent>(e);
		world.emplace<MiscStateComponent>(e);
		world.emplace<ZhenBaoComponent>(e);
		world.emplace<RecalcCacheComponent>(e);
		world.emplace<UpgradeItemComponent>(e);
	}

	// 首次创建时缓存所有池指针 (后续 GetXxx 不再需要全局锁)
	static bool s_poolCacheInited = false;
	if (!s_poolCacheInited)
	{
		InitPoolCache();
		s_poolCacheInited = true;
	}
}

VOID PlayerComponentManager::DestroyPlayerComponents(UINT ownerId)
{
	auto* ecsWorld = ECSWorld::GetInstance();
	auto& world = ecsWorld->GetWorld();
	SWLock lock(world.m_mutex);
	ecsWorld->DestroyEntity(ownerId);
}

BOOL PlayerComponentManager::CheckPlayerTimer(entity_t e, TimerType type, DWORD intervalMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return FALSE;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	// 路径 0 (超快路径): 批量预计算已置位 firedMask → 零锁返回
	{
		SRLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		if (InterlockedAnd((volatile LONG*)&tc->firedMask, ~(1u << idx)) & (1u << idx))
			return TRUE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) < (int)intervalMs)
			return FALSE;
	}
	// 冷路径: 独占锁写入
	{
		SWLock lock(world.m_mutex);
		auto* tc = world.get_nolock<TimerComponent>(e);
		if (!tc) return FALSE;
		int now = CFrameTime::GetFrameTime();
		if (GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs)
		{
			tc->lastTickMs[idx] += (int)intervalMs;
			return TRUE;
		}
		return FALSE;
	}
}

BOOL PlayerComponentManager::CheckPlayerTimerNoReset(entity_t e, TimerType type, DWORD intervalMs, int& outLastTickMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) { outLastTickMs = 0; return FALSE; }

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SRLock lock(world.m_mutex);
	auto* tc = world.get_nolock<TimerComponent>(e);
	if (!tc) { outLastTickMs = 0; return FALSE; }

	outLastTickMs = tc->lastTickMs[idx];
	int now = CFrameTime::GetFrameTime();
	return GetTimeToTime(tc->lastTickMs[idx], now) >= (int)intervalMs;
}

VOID PlayerComponentManager::ResetPlayerTimer(entity_t e, TimerType type)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* tc = world.get_nolock<TimerComponent>(e);
	if (tc) tc->lastTickMs[idx] = CFrameTime::GetFrameTime();
}

VOID PlayerComponentManager::OffsetPlayerTimer(entity_t e, TimerType type, int offsetMs)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SWLock lock(world.m_mutex);

	auto* tc = world.get_nolock<TimerComponent>(e);
	if (tc) tc->lastTickMs[idx] -= offsetMs;
}

int PlayerComponentManager::GetPlayerTimerLastTick(entity_t e, TimerType type)
{
	const int idx = TimerTypeToIdx(type);
	if (idx < 0 || e == INVALID_ENTITY) return 0;

	auto& world = ECSWorld::GetInstance()->GetWorld();
	SRLock lock(world.m_mutex);

	auto* tc = world.get_nolock<TimerComponent>(e);
	return tc ? tc->lastTickMs[idx] : 0;
}

RateLimitComponent* PlayerComponentManager::GetRateLimit(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_rateLimitPool, e);
}

BOOL PlayerComponentManager::TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act)
{
	auto* rl = GetRateLimit(pPlayer);
	if (!rl) return TRUE;
	if (act == RateLimitComponent::ACT_SPECIAL_ATTACK)
		rl->SetInterval(RateLimitComponent::ACT_SPECIAL_ATTACK,
			rl->intervals[RateLimitComponent::ACT_SPECIAL_ATTACK] - 80 * pPlayer->GetPropValue(PI_ATTACKSPEED));
	return rl->TryExecute(act, CFrameTime::GetFrameTime());
}

VOID PlayerComponentManager::GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms)
{
	auto* rl = GetRateLimit(pPlayer);
	if (rl) rl->SetInterval(act, ms);
}

ShieldStateComponent* PlayerComponentManager::GetShieldState(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_shieldStatePool, e);
}

SpecialEquipComponent* PlayerComponentManager::GetSpecialEquip(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_specialEquipPool, e);
}

DWORD PlayerComponentManager::GetSpecialEquipFlag(CHumanPlayer* pPlayer, int func)
{
	auto* se = GetSpecialEquip(pPlayer);
	return se ? se->GetPosFlag(func) : 0;
}

StaminaComponent* PlayerComponentManager::GetStamina(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_staminaPool, e);
}

TaskComponent* PlayerComponentManager::GetTask(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_taskPool, e);
}

FenghaoComponent* PlayerComponentManager::GetFenghao(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_fenghaoPool, e);
}

AchievementComponent* PlayerComponentManager::GetAchievement(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_achievementPool, e);
}

SocialComponent* PlayerComponentManager::GetSocial(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_socialPool, e);
}

ChatComponent* PlayerComponentManager::GetChat(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_chatPool, e);
}

PkComponent* PlayerComponentManager::GetPk(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_pkPool, e);
}

MarketComponent* PlayerComponentManager::GetMarket(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_marketPool, e);
}

TitleComponent* PlayerComponentManager::GetTitle(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_titlePool, e);
}

ScriptVarComponent* PlayerComponentManager::GetScriptVar(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_scriptVarPool, e);
}

MiscStateComponent* PlayerComponentManager::GetMiscState(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_miscStatePool, e);
}

ZhenBaoComponent* PlayerComponentManager::GetZhenBao(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_zhenBaoPool, e);
}

RecalcCacheComponent* PlayerComponentManager::GetRecalcCache(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_recalcCachePool, e);
}

UpgradeItemComponent* PlayerComponentManager::GetUpgradeItem(CHumanPlayer* pPlayer)
{
	entity_t e = pPlayer->GetECSEntity();
	if (e == INVALID_ENTITY) return nullptr;
	return GetFromPool(m_upgradeItemPool, e);
}
