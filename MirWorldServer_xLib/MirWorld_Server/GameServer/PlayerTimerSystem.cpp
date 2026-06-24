#include "StdAfx.h"
#include "PlayerTimerSystem.h"
#include "HumanPlayer.h"

PlayerTimerSystem::PlayerTimerSystem()
	: m_timerSystem(ECSWorld::GetInstance()->GetWorld(), PlayerTimerTypeToIdx)
{
}

VOID PlayerTimerSystem::CreatePlayerTimers(CHumanPlayer* pPlayer)
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
	//初始化
	AliveTimerSystem::GetInstance()->CreateAliveTimers(pPlayer);
}

BOOL PlayerTimerSystem::CheckTimer(UINT ownerId, TimerType type, DWORD intervalMs)
{
	return m_timerSystem.CheckTimer(ownerId, type, intervalMs);
}

BOOL PlayerTimerSystem::CheckTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs)
{
	return m_timerSystem.CheckTimerNoReset(ownerId, type, intervalMs, outLastTickMs);
}

VOID PlayerTimerSystem::ResetTimer(UINT ownerId, TimerType type)
{
	m_timerSystem.ResetTimer(ownerId, type);
}

VOID PlayerTimerSystem::OffsetTimer(UINT ownerId, TimerType type, int offsetMs)
{
	m_timerSystem.OffsetTimer(ownerId, type, offsetMs);
}

int PlayerTimerSystem::GetTimerLastTick(UINT ownerId, TimerType type)
{
	return m_timerSystem.GetTimerLastTick(ownerId, type);
}

VOID PlayerTimerSystem::OnPlayerLogout(UINT ownerId)
{
	m_timerSystem.Cleanup(ownerId);
	//释放
	AliveTimerSystem::GetInstance()->OnObjectDestroy(ownerId);
}
