#include "StdAfx.h"
#include "EcsPlayerManager.h"
#include "HumanPlayer.h"

EcsPlayerManager::EcsPlayerManager()
	: m_timerSystem(m_world, PlayerTimerTypeToIdx)
{
}

EcsPlayerManager::~EcsPlayerManager()
{
	m_world.clear_all();
	m_idToRateEntity.clear();
	m_idToShieldEntity.clear();
	m_idToSpecialEquipEntity.clear();
}

// ==================================================
//  频率限制
// ==================================================

RateLimitComponent* EcsPlayerManager::GetRateLimit(CHumanPlayer* pPlayer)
{
	entity_t e = GetOrCreateEntity<RateLimitComponent>(pPlayer->GetId(), m_idToRateEntity);
	return m_world.get<RateLimitComponent>(e);
}

BOOL EcsPlayerManager::TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act)
{
	entity_t e = GetOrCreateEntity<RateLimitComponent>(pPlayer->GetId(), m_idToRateEntity);
	auto* rl = m_world.get<RateLimitComponent>(e);
	if (!rl) return TRUE;
	if (act == RateLimitComponent::ACT_SPECIAL_ATTACK) // 战士技能攻击速度根据攻速调整
		rl->SetInterval(RateLimitComponent::ACT_SPECIAL_ATTACK, rl->intervals[RateLimitComponent::ACT_SPECIAL_ATTACK] - 80 * pPlayer->GetPropValue(PI_ATTACKSPEED));
	return rl->TryExecute(act, CFrameTime::GetFrameTime());
}

VOID EcsPlayerManager::GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms)
{
	entity_t e = GetOrCreateEntity<RateLimitComponent>(pPlayer->GetId(), m_idToRateEntity);
	auto* rl = m_world.get<RateLimitComponent>(e);
	if (rl) rl->SetInterval(act, ms);
}

// ==================================================
//  护盾状态
// ==================================================

ShieldStateComponent* EcsPlayerManager::GetShieldState(CHumanPlayer* pPlayer)
{
	entity_t e = GetOrCreateEntity<ShieldStateComponent>(pPlayer->GetId(), m_idToShieldEntity);
	return m_world.get<ShieldStateComponent>(e);
}

VOID EcsPlayerManager::SetShieldCount(CHumanPlayer* pPlayer, int nCount)
{
	entity_t e = GetOrCreateEntity<ShieldStateComponent>(pPlayer->GetId(), m_idToShieldEntity);
	auto* ss = m_world.get<ShieldStateComponent>(e);
	if (ss) ss->magShieldResCount = nCount;
}

// ==================================================
//  特殊装备
// ==================================================

EcsSpecialEquip* EcsPlayerManager::GetSpecialEquip(CHumanPlayer* pPlayer)
{
	entity_t e = GetOrCreateEntity<EcsSpecialEquip>(pPlayer->GetId(), m_idToSpecialEquipEntity);
	return m_world.get<EcsSpecialEquip>(e);
}

DWORD EcsPlayerManager::GetSpecialEquipFlag(CHumanPlayer* pPlayer, special_equipment_func func)
{
	entity_t e = GetOrCreateEntity<EcsSpecialEquip>(pPlayer->GetId(), m_idToSpecialEquipEntity);
	return m_world.get<EcsSpecialEquip>(e)->GetPosFlag(func);
}

// ==================================================
//  定时器 (委托到 TimerSystem)
// ==================================================

VOID EcsPlayerManager::CreatePlayerTimers(CHumanPlayer* pPlayer)
{
	if (!pPlayer) return;
	UINT id = pPlayer->GetId();
	if (id == 0) return;
	// 创建 CHumanPlayer 专属的 17 个定时器实体
	static constexpr TimerType s_playerTimerTypes[PLAYER_TMR_COUNT] = {
		TimerType::TMR_DB_SAVE,
		TimerType::TMR_STAMINA,
		TimerType::TMR_GAME_TIME,
		TimerType::TMR_PK,
		TimerType::TMR_JUST_PK,
		TimerType::TMR_PK_POINT,
		TimerType::TMR_FENGHAO,
		TimerType::TMR_HORSE,
		TimerType::TMR_ADD_TO_GUILD,
		TimerType::TMR_CHAT_NORMAL,
		TimerType::TMR_CHAT_CRY,
		TimerType::TMR_CHAT_WISPER,
		TimerType::TMR_CHAT_GROUP,
		TimerType::TMR_CHAT_GUILD,
		TimerType::TMR_CHAT_COUPLE,
		TimerType::TMR_CHAT_GM,
		TimerType::TMR_CHAT_FRIEND,
	};

	for (TimerType t : s_playerTimerTypes)
		m_timerSystem.GetOrCreateTimerEntity(id, t);
}

BOOL EcsPlayerManager::CheckTimer(UINT ownerId, TimerType type, DWORD intervalMs)
{
	return m_timerSystem.CheckTimer(ownerId, type, intervalMs);
}

BOOL EcsPlayerManager::CheckTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs)
{
	return m_timerSystem.CheckTimerNoReset(ownerId, type, intervalMs, outLastTickMs);
}

VOID EcsPlayerManager::ResetTimer(UINT ownerId, TimerType type)
{
	m_timerSystem.ResetTimer(ownerId, type);
}

VOID EcsPlayerManager::OffsetTimer(UINT ownerId, TimerType type, int offsetMs)
{
	m_timerSystem.OffsetTimer(ownerId, type, offsetMs);
}

int EcsPlayerManager::GetTimerLastTick(UINT ownerId, TimerType type)
{
	return m_timerSystem.GetTimerLastTick(ownerId, type);
}

// ==================================================
//  资源清理
// ==================================================

VOID EcsPlayerManager::CleanupEntityMap(UINT id, std::unordered_map<UINT, entity_t>& map)
{
	auto it = map.find(id);
	if (it != map.end())
	{
		m_world.destroy(it->second);
		map.erase(it);
	}
}

VOID EcsPlayerManager::OnPlayerLogout(CHumanPlayer* pPlayer)
{
	UINT id = pPlayer->GetId();

	CleanupEntityMap(id, m_idToRateEntity);
	CleanupEntityMap(id, m_idToShieldEntity);
	CleanupEntityMap(id, m_idToSpecialEquipEntity);

	m_timerSystem.Cleanup(id);
}
