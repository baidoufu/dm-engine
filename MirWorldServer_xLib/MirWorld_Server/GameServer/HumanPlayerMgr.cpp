#include "StdAfx.h"
#include ".\humanplayermgr.h"
#include ".\gameworld.h"
#include "BotManager.h"
#include "AliveTimerSystem.h"
#include "PlayerTimerSystem.h"

CHumanPlayerMgr::CHumanPlayerMgr(VOID)
{
	m_HumanPlayers.Create(1024);
	VMP_PROTECT_BEGIN("CHumanPlayerMgr-boTest");
	m_boTest = FALSE;
	VMP_PROTECT_END();
}

CHumanPlayerMgr::~CHumanPlayerMgr(VOID)
{
}

CHumanPlayer* CHumanPlayerMgr::FindbyName(const char* pszName)
{
	return (CHumanPlayer*)m_PlayerNameHash.HGet(pszName);
}

CHumanPlayer* CHumanPlayerMgr::FindbyId(UINT id)
{
	UINT rawId = id;
	if (((id & 0xff000000) >> 24) == OBJ_PLAYER)
		rawId = id & 0xffffff;
	CHumanPlayer* pPlayer = m_HumanPlayers.Get(rawId);
	if (pPlayer != nullptr)
		return pPlayer;
	CBotManager* pBotMgr = CBotManager::GetInstance();
	if (pBotMgr != nullptr)
	{
		CBotPlayer* pBot = pBotMgr->FindBotById(id);
		if (pBot != nullptr)
			return pBot;
	}
	return nullptr;
}

BOOL CHumanPlayerMgr::AddPlayerNameList(CHumanPlayer* pPlayer, const char* pszName)
{
	return m_PlayerNameHash.HAdd(pszName, (LPVOID)pPlayer);
}

VOID CHumanPlayerMgr::RemovePlayerNameList(const char* pszName)
{
	m_PlayerNameHash.HDel(pszName);
}

BOOL CHumanPlayerMgr::RegisterBotPlayer(CHumanPlayer* pPlayer)
{
	const char* pszName = pPlayer->GetName();
	if (pszName == nullptr || pszName[0] == '\0')
		return FALSE;
	// 检查是否已存在同名玩家
	if (m_PlayerNameHash.HGet(pszName) != nullptr)
	{
		LG2("机器人注册: 名字 [%s] 已存在\n", pszName);
		return FALSE;
	}
	RegEcs(pPlayer);
	return m_PlayerNameHash.HAdd(pszName, (LPVOID)pPlayer);
}

VOID CHumanPlayerMgr::UnregisterBotPlayer(CHumanPlayer* pPlayer)
{
	const char* pszName = pPlayer->GetName();
	if (pszName == nullptr || pszName[0] == '\0')
		return;
	m_PlayerNameHash.HDel(pszName);
	UINT rawId = pPlayer->GetId();
	UnregEcs(rawId);
}

CHumanPlayer* CHumanPlayerMgr::NewPlayer()
{
	VMP_PROTECT_BEGIN("NewPlayer-boTest");
	if (IsTestMode())
	{
		if (m_HumanPlayers.GetCount() >= 5)
			return nullptr;
	}
	VMP_PROTECT_END();
	CHumanPlayer* pPlayer = nullptr;
	UINT id = 0;
	id = m_HumanPlayers.New(&pPlayer);
	if (id == 0 || pPlayer == nullptr) return nullptr;
	id |= (OBJ_PLAYER << 24);
	pPlayer->SetId(id);
	RegEcs(pPlayer);
	return pPlayer;
}

BOOL CHumanPlayerMgr::DeletePlayer(CHumanPlayer* pPlayer)
{
	UINT rawId = pPlayer->GetId();
	UINT id = rawId & 0xffffff;
	m_PlayerNameHash.HDel(pPlayer->GetName());
	pPlayer->Clean();
	UnregEcs(rawId);
	return m_HumanPlayers.Del(id);
}

VOID CHumanPlayerMgr::RegEcs(CHumanPlayer* pPlayer)
{
	PlayerTimerSystem::GetInstance()->CreatePlayerTimers(pPlayer);
}

VOID CHumanPlayerMgr::UnregEcs(UINT id)
{
	RateLimitSystem::GetInstance()->OnPlayerLogout(id);
	ShieldStateSystem::GetInstance()->OnPlayerLogout(id);
	SpecialEquipSystem::GetInstance()->OnPlayerLogout(id);
	PlayerTimerSystem::GetInstance()->OnPlayerLogout(id);
}