#include "StdAfx.h"
#include <string.h>
#include "gsclientobj.h"
#include "LogicMap.h"
#include "server.h"
#include "humanplayermgr.h"
#include "gameworld.h"
#include "scriptnpc.h"
#include "npcmanager.h"
#include "guildex.h"
#include "topmanager.h"
#include "scripttarget.h"
#include "marketmanager.h"
#include "systemscript.h"
#include "BossTJ.h"
#include "TimeAchieve.h"
#include "GameStage.h"

VOID CClientObj::HandleRideHorse(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->RideHorse())
		m_pPlayer->SendMsg(1, 0xcd, 0, 0, 0);
	else
		m_pPlayer->SendMsg(0, 0xcd, 0, 0, 0);
}

VOID CClientObj::HandleAddFriend(PMIRMSG pMsg, int datasize)
{
	if (CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data))
		pPlayer->PostAddFriendRequest(m_pPlayer);
	else
		m_pPlayer->SendFriendSystemError(FE_ADD_OFFONLINE, pMsg->data);
}

VOID CClientObj::HandleDeleteFriend(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->DeleteFriend(pMsg->data);
}

VOID CClientObj::HandleConfirmFriend(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->ReplyAddFriendRequest(pMsg->dwFlag, pMsg->data);
}

VOID CClientObj::HandleSetItemPosition(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->SetBagItemPos((BAGITEMPOS*)pMsg->data, pMsg->wParam[2]);
}

VOID CClientObj::HandleViewEquipment(PMIRMSG pMsg, int datasize)
{
	CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyId(pMsg->dwFlag);
	if (pPlayer != nullptr)
	{
		xPacketPool::ScopedPacket packet(65535);
		pPlayer->GetViewDetail(*packet);
		m_pPlayer->SendMsg(pPlayer->GetId(), 0x2ef, 0, MAKEWORD(0, _U_MAX), MAKEWORD(0, 1), (LPVOID)packet->getbuf(), packet->getsize());
	}
}

VOID CClientObj::HandleLeaveServer(PMIRMSG pMsg, int datasize)
{
	m_bCompetlyQuit = TRUE;
}

VOID CClientObj::HandleRecordHomeStone(PMIRMSG pMsg, int datasize)
{
	int nMapId = HIBYTE(pMsg->wParam[0]);
	char* szName{};
	switch (nMapId)
	{
	case 0:
		szName = "落霞岛";
		break;
	case 1:
		szName = "中州";
		break;
	case 2:
		szName = "热砂荒漠";
		break;
	case 3:
		szName = "禁地";
		break;
	default:
		szName = "落霞岛";
		break;
	}
	CItemBox& box = m_pPlayer->GetBag();
	ITEM* pItem = box.FindItem(pMsg->dwFlag);
	if (pItem == nullptr) return; // 防止空指针解引用
	m_pPlayer->SetUsingItem(pItem);
	m_pPlayer->RecordHomeXY(szName);
	m_pPlayer->SendUpdateItem(*pItem);
}

VOID CClientObj::HandleMasterApprentice(PMIRMSG pMsg, int datasize)
{
	CAliveObject* pAliveObj = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
	if (pAliveObj == nullptr || pAliveObj->GetType() != OBJ_PLAYER) return;
	CHumanPlayer* pObject = (CHumanPlayer*)pAliveObj;
	switch (pMsg->wParam[0])
	{
	case 1:
	{
		if (m_pPlayer->GetStudentCount() == 3)
			m_pPlayer->SaySystem("你已收满徒弟!");
		else if (m_pPlayer->GetPropValue(PI_LEVEL) < 28)
			m_pPlayer->SaySystem("你的等级不够, 不能收徒!");
		else if (pObject->GetPropValue(PI_LEVEL) >= 28)
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x310, 258, 12, 0, "对方等级太高");
		else if (pObject->HasMaster())
			m_pPlayer->SaySystem("对方已经有师傅!");
		else
			pObject->SendMsg(m_pPlayer->GetId(), 0x310, 4, 0, 0);
	}
	break;
	case 2:
	{
		if (pObject->GetStudentCount() == 3)
			m_pPlayer->SaySystem("对方已收满徒弟!");
		else if (pObject->GetPropValue(PI_LEVEL) < 28)
			m_pPlayer->SaySystem("对方等级不够!");
		else if (m_pPlayer->GetPropValue(PI_LEVEL) >= 28)
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x310, 258, 12, 0, "你的等级太高");
		else if (m_pPlayer->HasMaster())
			m_pPlayer->SaySystem("你已经有师傅!");
		else
			pObject->SendMsg(m_pPlayer->GetId(), 0x310, 3, 0, 0);
	}
	break;
	case 3:
	{
		if (m_pPlayer->GetStudentCount() < 3 && m_pPlayer->GetPropValue(PI_LEVEL) >= 28 && pObject->GetPropValue(PI_LEVEL) < 28)
		{
			if (m_pPlayer->AddStudent(pObject))
				pObject->SetMaster(m_pPlayer);
		}
	}
	break;
	case 4:
	{
		if (pObject->GetStudentCount() < 3 && pObject->GetPropValue(PI_LEVEL) >= 28 && m_pPlayer->GetPropValue(PI_LEVEL) < 28)
		{
			if (pObject->AddStudent(m_pPlayer))
				m_pPlayer->SetMaster(pObject);
		}
	}
	break;
	case 259:
		pObject->SendMsg(m_pPlayer->GetId(), 0x312, 258, 16, 0, "对方拒绝收你为徒");
		break;
	case 260:
		pObject->SendMsg(m_pPlayer->GetId(), 0x312, 258, 18, 0, "对方拒绝拜你为师傅");
		break;
	}
}

VOID CClientObj::HandleTradeRequest(PMIRMSG pMsg, int datasize)
{
	CAliveObject* pAliveObj = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
	if (pAliveObj == nullptr || pAliveObj->GetType() != OBJ_PLAYER) return;
	CHumanPlayer* pObject = (CHumanPlayer*)pAliveObj;
	switch (pMsg->wParam[0])
	{
	case 0:
	{
		if (pObject->GetAttackMode() != HAM_PEACE)
		{
			m_pPlayer->SaySystem("%s 不是和平模式", pObject->GetName());
			break;
		}
		m_pPlayer->ChangeAttackMode(HAM_PEACE);
		char szContent[64];
		sprintf_s(szContent, "%s 申请和你交易", m_pPlayer->GetName());
		pObject->SendMsg(m_pPlayer->GetId(), 0x312, 1, 20, 0, szContent);
	}
	break;
	case 1:
	{
		m_pPlayer->ChangeAttackMode(HAM_PEACE);
		if (!m_pPlayer->Trade(pObject))
			m_pPlayer->CancelTrade();
	}
	break;
	default:
		pObject->SendMsg(m_pPlayer->GetId(), 0x312, 256, 16, 0, "对方拒绝与你交易");
		break;
	}
}

VOID CClientObj::HandlePetFeed(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->PetEatItem(pMsg->wParam[0], pMsg->dwFlag);
}

VOID CClientObj::HandleGuildMemberList(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild)
		pGuild->SendDurationMemberList(m_pPlayer);
}

VOID CClientObj::HandleDropItem(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->CanDropItem()) return;
	if (m_pPlayer->DropBagItem(pMsg->dwFlag))
	{
		m_pPlayer->SendMsg(pMsg->dwFlag, 0x258, 0, 0, 0);
		m_pPlayer->SendWeightChanged();
	}
	else
		m_pPlayer->SendMsg(pMsg->dwFlag, 0x259, 0, 0, 0);
}

VOID CClientObj::HandlePickupItem(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->GetActionType() != AT_STAND) return;
	if (!m_pPlayer->CanPickupItem()) return;
	if (m_pPlayer->getX() == pMsg->wParam[0] && m_pPlayer->getY() == pMsg->wParam[1])
	{
		if (m_pPlayer->PickupItem())
		{
			m_pPlayer->SendWeightChanged();
		}
	}
}

VOID CClientObj::HandleTakeOnItem(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->CanEquipChange()) return;
	BOOL bResult = m_pPlayer->EquipItem((int)pMsg->wParam[0], pMsg->dwFlag);
	SendEquipItemResult(bResult, (int)pMsg->wParam[0], pMsg->dwFlag);
}

VOID CClientObj::HandleTakeOffItem(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->CanEquipChange()) return;
	SendUnEquipItemResult(m_pPlayer->UnEquipItem((int)pMsg->wParam[0], pMsg->dwFlag), (int)pMsg->wParam[0], pMsg->dwFlag);
}

VOID CClientObj::HandleUseItem(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->CanUseItem())
	{
		m_pPlayer->SendEatFail();
		return;
	}
	m_pPlayer->UseItem(pMsg->dwFlag, static_cast<DWORD>(MAKELONG(pMsg->wParam[0], pMsg->wParam[1])));
}

VOID CClientObj::HandleDigCorpse(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->CutBody(pMsg->dwFlag, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]))
		m_pPlayer->SendAroundMsg(m_pPlayer->GetId(), 0x27d, m_pPlayer->getX(), m_pPlayer->getY(), pMsg->wParam[2]);
}

VOID CClientObj::HandleSkillShortcut(PMIRMSG pMsg, int datasize)
{
	USERMAGIC* pMagic = m_pPlayer->GetMagicByKey((BYTE)pMsg->wParam[0]);
	if (pMagic != nullptr)pMagic->magic.btKey = 0;
	pMagic = m_pPlayer->GetMagic(pMsg->dwFlag);
	if (pMagic != nullptr)pMagic->magic.btKey = (BYTE)pMsg->wParam[0];
}

VOID CClientObj::HandleSoftClose(PMIRMSG pMsg, int datasize)
{
	CSCClientObj* pObj = CServer::GetInstance()->GetSCConnection();
	if (pObj != nullptr)
	{
		pObj->SendMsgAcrossServer(0, MAS_RESTARTGAME, MST_SINGLE, (WORD)m_EnterInfo.dwSelectCharServerId, (char*)&m_EnterInfo, sizeof(m_EnterInfo));
		m_bCompetlyQuit = TRUE;
	}
	m_pPlayer->UpdateToDB();
}

VOID CClientObj::HandleClickNpcOrShop(PMIRMSG pMsg, int datasize)
{
	CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
	if (pObject == nullptr)return;
	if (pObject->GetType() == OBJ_NPC)
	{
		CScriptNpc* pNpc = (CScriptNpc*)pObject;
		pNpc->QueryTalk(m_pPlayer);
	}
	else if (pObject->GetType() == OBJ_PLAYER)
	{
		CHumanPlayer* pTarget = (CHumanPlayer*)pObject;
		if (pTarget != m_pPlayer) // 不能查看自己的私人商店
			pTarget->SendPrivateShopPage(m_pPlayer, 1);
	}
}

VOID CClientObj::HandleNpcClick(PMIRMSG pMsg, int datasize)
{
	if (pMsg->dwFlag == 0xffffffff)
	{
		if (!CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), pMsg->data))
			m_pPlayer->SendCloseScriptPage(0xffffffff);
		return;
	}
	if (pMsg->dwFlag == 0xfffffff0)
	{
		char szPage[256];
		sprintf_s(szPage, sizeof(szPage), "变强攻略.%s", (const char*)pMsg->data);
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), szPage);
		return;
	}

	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	if (pNpc != nullptr)
		pNpc->QuerySelectLink(m_pPlayer, pMsg->data);
}

VOID CClientObj::HandleGetItemPrice(PMIRMSG pMsg, int datasize)
{
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	if (pNpc != nullptr)
	{
		ITEM* pItem = m_pPlayer->FindBagItem(*(DWORD*)&pMsg->wParam[0]);
		if (pItem)
			m_pPlayer->SendMsg(pNpc->GetItemSellPrice(*pItem), 0x287, 0, 0, 0);
		else
			m_pPlayer->SendMsg(0, 0x287, 0, 0, 0);
	}
}

VOID CClientObj::HandleNpcSellItem(PMIRMSG pMsg, int datasize)
{
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	if (pNpc != nullptr)
	{
		ITEM* pItem = m_pPlayer->FindBagItem(*(DWORD*)&pMsg->wParam[0]);
		if (pItem && pNpc->SellItem(m_pPlayer, *pItem))
		{
			m_pPlayer->SendMsg(m_pPlayer->GetMoney(MT_GOLD), 0x288, 0, 0, 0);
			m_pPlayer->SendWeightChanged();
			return;
		}
	}
	m_pPlayer->SendMsg(-1, 0x289, 0, 0, 0);
}

VOID CClientObj::HandleBuyShopItem(PMIRMSG pMsg, int datasize)
{
	CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
	if (pObject == nullptr)return;
	if (pObject->GetType() == OBJ_NPC)
	{
		CScriptNpc* pNpc = (CScriptNpc*)pObject;
		DWORD dwError = 0;
		if (pNpc != nullptr && pNpc->BuyItem(m_pPlayer, (char*)&pMsg->data[4], *(DWORD*)&pMsg->wParam[0], dwError))
			m_pPlayer->SendMsg(m_pPlayer->GetMoney(MT_GOLD), 0x28a, pMsg->wParam[0], pMsg->wParam[1], 0);
		else
			m_pPlayer->SendMsg(dwError, 0x28b, 0, 0, 0);
	}
	else if (pObject->GetType() == OBJ_PLAYER)
	{
		if (((CHumanPlayer*)pObject)->BuyPrivateShopItem(m_pPlayer, *(DWORD*)&pMsg->wParam[0], pMsg->data))
		{
			if (((CHumanPlayer*)pObject)->GetPrivateShopItemCount() > 0)
				((CHumanPlayer*)pObject)->UpdatePrivateShopToAround();
			else
			{
				((CHumanPlayer*)pObject)->StopPrivateShop();
				((CHumanPlayer*)pObject)->SendStopPrivateShop();
			}
		}
	}
}

VOID CClientObj::HandleNpcSellList(PMIRMSG pMsg, int datasize)
{
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	if (pNpc != nullptr)
	{
		if (!pNpc->QueryItemList(m_pPlayer, pMsg->data, pMsg->wParam[0]))
			m_pPlayer->SendMsg(-1, 0x028B, 0, 0, 0);
	}
}

VOID CClientObj::HandleDropGold(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->DropGold(pMsg->dwFlag);
}

VOID CClientObj::HandleConfirmFirstDialog(PMIRMSG pMsg, int datasize)
{
	CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pDI)
	{
		pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_EQUIPMENT, 20);
		pDI->QueryCommunity(getId(), m_dwClientKey, m_pPlayer->GetDBId());
	}
	else
	{
		m_pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
		Disconnect(3000);
		return;
	}
	if (!CGameWorld::GetInstance()->AddMapObject(m_pPlayer))
	{
		m_pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"进入地图失败!");
		Disconnect(3000);
		LG2("玩家 %s 进入地图失败\n", m_pPlayer->GetName());
		return;
	}
	if (m_pPlayer->IsFirstLogin())
		m_pPlayer->AddProcess(EP_FIRSTLOGINPROCESS);
	else
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "LoginEnv.Login", FALSE);
	m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9609, 0, 0, 0, (LPVOID)m_pPlayer->GetName());
	m_pPlayer->SendClientPluginInfo();
	m_pPlayer->SendClientKeyConfig();
	CBossTJ::GetInstance()->SendBossList(m_pPlayer);
	m_State = GSUM_VERIFIED;
}

VOID CClientObj::HandleChangeGroupMode(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->SetGroupMode((BOOL)pMsg->wParam[0]);
}

VOID CClientObj::HandleAddGroupMember(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->AddGroupMember(pMsg->data);
}

VOID CClientObj::HandleDeleteGroupMember(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->RemoveGroupMember(pMsg->data);
}

VOID CClientObj::HandleRepairItem(PMIRMSG pMsg, int datasize)
{
	UINT npcid = pMsg->dwFlag;
	UINT nItemId = *(UINT*)&pMsg->wParam[0];
	ITEM* pItem = m_pPlayer->FindBagItem(nItemId);
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(npcid);
	if (pItem != nullptr && pNpc != nullptr && pNpc->RepairItem(m_pPlayer, *pItem))
	{
		if (m_pPlayer->IsSystemFlagSeted(SF_SPECIALREPAIR))
			pNpc->QuerySelectLink(m_pPlayer, "@~s_repair");
		else
			pNpc->QuerySelectLink(m_pPlayer, "@~repair");
	}
	else
		m_pPlayer->SendMsg(0, 0x029E, 0, 0, 0);
}

VOID CClientObj::HandlePutRepairItem(PMIRMSG pMsg, int datasize)
{
	UINT npcid = pMsg->dwFlag;
	UINT nItemId = *(UINT*)&pMsg->wParam[0];
	ITEM* pItem = m_pPlayer->FindBagItem(nItemId);
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(npcid);
	if (pItem != nullptr && pNpc != nullptr)
	{
		DWORD dwMoney = pNpc->GetItemRepairPrice(m_pPlayer, *pItem);
		m_pPlayer->SendMsg(dwMoney, 0x29f, 0, 0, 0, nullptr);
	}
}

VOID CClientObj::HandleQueryTrade(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->Trade();
}

VOID CClientObj::HandlePutTradeItem(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->PutTradeItem(pMsg->dwFlag))
		m_pPlayer->SendMsg(0, SM_PUTTRADEITEMOK, 0, 0, 0, 0);
	else
		m_pPlayer->SendMsg(0, SM_PUTTRADEITEMFAIL, 0, 0, 0, 0);
}

VOID CClientObj::HandleCancelTrade(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->CancelTrade();
}

VOID CClientObj::HandlePutTradeGold(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->PutTradeMoney((money_type)pMsg->wParam[0], pMsg->dwFlag))
	{
		DWORD dwCurMoney = m_pPlayer->GetMoney((money_type)pMsg->wParam[0]);
		m_pPlayer->SendMsg(pMsg->dwFlag, SM_PUTTRADEGOLDOK, dwCurMoney & 0xffff, (dwCurMoney >> 16), pMsg->wParam[0]);
	}
	else
		m_pPlayer->SendMsg(0, SM_PUTTRADEGOLDFAIL, 0, 0, 0);
}

VOID CClientObj::HandleQueryTradeEnd(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->ConfirmTrade();
}

VOID CClientObj::HandlePutToBank(PMIRMSG pMsg, int datasize)
{
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	DWORD dwItemId = *(DWORD*)&pMsg->wParam[0];
	if (pNpc != nullptr)
	{
		if (m_pPlayer->PutBankItem(dwItemId))
			m_pPlayer->SendMsg(dwItemId, 0x2bd, 0, 0, 0);
		else
			m_pPlayer->SendMsg(dwItemId, 0x2be, 0, 0, 0);
	}
}

VOID CClientObj::HandleTakeFromBank(PMIRMSG pMsg, int datasize)
{
	CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
	DWORD dwItemId = *(DWORD*)&pMsg->wParam[0];
	if (pNpc != nullptr)
	{
		if (m_pPlayer->TakeBankItem(dwItemId))
			m_pPlayer->SendMsg(dwItemId, 0x2c1, 0, 0, 0);
		else
			m_pPlayer->SendMsg(dwItemId, 0x2c2, 0, 0, 0);
	}
}

VOID CClientObj::HandleWalkToMapLink(PMIRMSG pMsg, int datasize)
{
	CLogicMap* pMap = m_pPlayer->GetMap();
	int minimap = 0;
	if (pMap)
	{
		minimap = pMap->GetMiniMap();
	}
	m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x2c6, minimap & 0xffff, 0, 0, 0, 0);
}

VOID CClientObj::HandleViewGuildInfo(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild == nullptr)
		m_pPlayer->SendMsg(0, 0x2f2, 0, 0, 0);
	else
	{
		pGuild->SendFirstPage(m_pPlayer);
		pGuild->SendGroupFengHaoData(m_pPlayer);
		pGuild->SendGuildTowerInfo(m_pPlayer);
	}
}

VOID CClientObj::HandleJoinGuild(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(m_pPlayer))
	{
		CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data);
		if (pMember)
		{
			if (pMember->GetGuild() != nullptr)
				m_pPlayer->SaySystem(getstrings(SD_ADDTOGUILD_ALREADYOTHERGUILDMEMBER));
			else
				pMember->PostAddToGuildRequest(m_pPlayer);
		}
	}
}

VOID CClientObj::HandleRemoveGuildMember(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(m_pPlayer))
	{
		const char* pszCharName = pMsg->data;
		if (_stricmp(pszCharName, m_pPlayer->GetName()) == 0)
		{
			m_pPlayer->SaySystem("掌门人不能被踢出.");
			return;
		}
		pGuild->RemoveMember(pszCharName);
		pGuild->SendDurationMemberList();
		CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pszCharName);
		if (pPlayer) pPlayer->UpdateViewName();
	}
}

VOID CClientObj::HandleEditGuildNotice(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(m_pPlayer))
	{
		pMsg->data[datasize++] = '\r';
		pMsg->data[datasize] = 0;
		m_pPlayer->GetGuild()->SetNotice(pMsg->data);
		char buffer[4096];
		int size = m_pPlayer->GetGuildFrontPage(buffer, 4096);
		m_pPlayer->SendMsg(0, 0x2f1, 0, 0, 0, (LPVOID)buffer, size);
		pGuild->SendFirstPage(m_pPlayer);
	}
	else
		m_pPlayer->SaySystem("你不是掌门人，不能编辑行会公告!");
}

VOID CClientObj::HandleViewGuildExp(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->GetGuild())
		m_pPlayer->GetGuild()->SendExp(m_pPlayer);
}

VOID CClientObj::HandleRequestGroupPos(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->UpdateGroupPosition();
}

VOID CClientObj::HandlePetBackExp(PMIRMSG pMsg, int datasize)
{
	CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "灵兽.BackExp");
}

VOID CClientObj::HandleTimeAchieve(PMIRMSG pMsg, int datasize)
{
	CTimeAchieve::GetInstance()->SendAchieveData(m_pPlayer);
}

VOID CClientObj::HandleShortcutKey(PMIRMSG pMsg, int datasize)
{
	if (pMsg->wParam[0] == 1)
		return;
	m_pPlayer->SendClientKeyConfig();
}

VOID CClientObj::HandleNewMail(PMIRMSG pMsg, int datasize)
{
	SendClientNewMail(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]);
}

VOID CClientObj::HandleSocialInfo(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wParam[0])
	{
	case 6:
	{
		m_pPlayer->UpdateCommunityInfoToClient();
	}
	break;
	}
}

VOID CClientObj::HandleFengHao(PMIRMSG pMsg, int datasize)
{
	if (pMsg->wParam[0] == 0)
		m_pPlayer->SendFengHaoGrowInfo();
	else
		m_pPlayer->SendFengHaoEquip(pMsg->wParam[1]);
}

VOID CClientObj::HandleFuncCollection(PMIRMSG pMsg, int datasize)
{
	switch (str_hash(pMsg->data))  // 注意：是运行时哈希
	{
	case "GameTimeMgr"_hash: {
		GameTimeMgr* pGameTimeMgr = (GameTimeMgr*)pMsg->data;
		switch (pGameTimeMgr->btCode)
		{
		case 1:
		{
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Open");
		}
		break;
		case 2:
		{
			m_pPlayer->setVParam(0, pGameTimeMgr->nValue[2]);
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Refill");
		}
		break;
		case 3:
		{
			m_pPlayer->setVParam(0, pGameTimeMgr->nValue[2]);
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Renewal");
		}
		break;
		}
		break;
	}
	case "guildmgr"_hash: {
		Guildmgr* pGuildmgr = (Guildmgr*)pMsg->data;
		switch (pGuildmgr->btCode)
		{
		case 0:
		{
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "行会.Help");
		}
		break;
		case 1:
		{
			xPacketPool::ScopedPacket packet(65535);
			const char* s1C = "guildmgr";
			packet->push(s1C);
			packet->push(1);
			int nValue = 1;
			packet->push((LPVOID)&nValue, 1);
			nValue = 0x0B;
			packet->push((LPVOID)&nValue, 4);
			std::string szRecruitStateList = CGuildManagerEx::GetInstance()->GetRecruitStateList();
			if (szRecruitStateList == "") break;
			packet->push(szRecruitStateList.c_str());
			packet->push(1);
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
		}
		break;
		case 2:
		{
			if (m_pPlayer->GetPropValue(PI_LEVEL) < 11)
			{
				m_pPlayer->SaySystem("你的等级还未超过10级, 能力还不够, 无法报名应征行会.");
				break;
			}
			const char* pszGuildName = pMsg->data + sizeof(Guildmgr);
			CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(pszGuildName);
			if (pGuild)
			{
				if (pGuild->AddApplyPlayer(m_pPlayer))
					m_pPlayer->SaySystem("你成功申请了加入行会, 会长将会择优录取.");
				else
					m_pPlayer->SaySystem("你已经申请过该行会了, 请耐心等候.");
			}
		}
		break;
		case 3:
		{
			CGuildEx* pGuild = m_pPlayer->GetGuild();
			if (!pGuild) break;
			if (!pGuild->IsMaster(m_pPlayer))break;
			pGuild->SendApplyList(m_pPlayer);
		}
		break;
		case 4:
		{
			CGuildEx* pGuild = m_pPlayer->GetGuild();
			if (!pGuild)
			{
				m_pPlayer->SaySystem("你还没有加入任何行会, 不能招募会员.");
				break;
			}
			if (!pGuild->IsMaster(m_pPlayer))
			{
				m_pPlayer->SaySystem("你不是行会会长, 不能招募行会成员, 前往皇宫找寻国王可创建行会.");
				break;
			}
			if (pGuild->IsFull())
			{
				m_pPlayer->SaySystem("你当前行会成员已经达到上限, 前往皇宫找寻国王可提高上限成员.");
				break;
			}
			const char* pszCharName = pMsg->data + sizeof(Guildmgr);
			int* boAllow = (int*)(pszCharName + strlen(pszCharName) + 1);
			pGuild->DelApplyPlayer(pszCharName);
			if (*boAllow == 0)
			{
				pGuild->SendDurationMemberList();
				CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pszCharName);
				if (pPlayer)
				{
					if (pPlayer->GetGuild() != nullptr)
						m_pPlayer->SaySystem("%s 已经加入了其他行会, 招募失败.", pszCharName);
					else if (pPlayer->GetPropValue(PI_LEVEL) > 0 && pGuild->AddMember(pPlayer))
					{
						m_pPlayer->SaySystem("成功招募了行会成员 %s.", pszCharName);
						pGuild->SendDurationMemberList();
						pPlayer->UpdateViewName();
					}
					else
						m_pPlayer->SaySystem("%s 等级不足, 招募失败.", pszCharName);
				}
				else
				{
					if (pGuild->AddMember(pszCharName))
						m_pPlayer->SaySystem("成功招募了行会成员 %s.", pszCharName);
				}
			}
			else
				m_pPlayer->SaySystem("你拒绝了 %s 的行会申请.", pszCharName);
		}
		break;
		case 5:
		{
			int* nSelect = (int*)(pMsg->data + sizeof(Guildmgr));
			int* boRecruitState = (int*)((char*)nSelect + 4);
			CGuildEx* pGuild = m_pPlayer->GetGuild();
			if (*nSelect == 1)
				pGuild->SetRecruitState(*boRecruitState);
			else
			{
				xPacketPool::ScopedPacket packet;
				const char* s1C = "guildmgr";
				packet->push(s1C);
				packet->push(1);
				int nValue = 0x05;
				packet->push((LPVOID)&nValue, 4);
				packet->push(1);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				nValue = (pGuild && pGuild->GetRecruitState()) ? 0x01 : 0x00;
				packet->push((LPVOID)&nValue, 4);
				packet->push(12);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				nValue = (pGuild && pGuild->GetRecruitState()) ? 0x01 : 0x00;
				packet->push((LPVOID)&nValue, 4);
				packet->push(4);
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
			}
		}
		break;
		case 6:
		{
			CGuildEx* pGuild = m_pPlayer->GetGuild();
			if (!pGuild) break;
			if (!pGuild->IsMaster(m_pPlayer))break;
			if (pGuildmgr->nType == 0)
			{
				xPacketPool::ScopedPacket packet(65535);
				const char* s1C = "guildmgr";
				packet->push(s1C);
				packet->push(1);
				int nValue = 0x06;
				packet->push((LPVOID)&nValue, 1);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				std::string szRecruitStateList = pGuild->GetDeclarationList(TRUE);
				packet->push(szRecruitStateList.c_str());
				packet->push(1);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				packet->push(12);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				nValue = 0x01;
				packet->push((LPVOID)&nValue, 4);
				nValue = 0x00;
				packet->push((LPVOID)&nValue, 4);
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
			}
			else
			{
				if (pGuild == nullptr) break;
				const char* pszDeclaration = pMsg->data + sizeof(Guildmgr);
				pGuild->SetDeclarationList(pszDeclaration);
			}
		}
		break;
		}
		break;
	}
	case "expback2020"_hash: {
		ExpBack* pExpBack = (ExpBack*)pMsg->data;
		switch (pExpBack->btCode)
		{
		case 0:
		{
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "每日经验.Main");
		}
		break;
		case 2:
		{
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "每日经验.Help");
		}
		break;
		}
		break;
	}
	case "ActivityScore2014"_hash: {
		ActivityScore2014* pActivityScore2014 = (ActivityScore2014*)pMsg->data;
		switch (pActivityScore2014->btCode)
		{
		case 1:
		{
			if (m_pPlayer->GetPropValue(PI_LEVEL) >= 7)
			{
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "活动日历.Main");
			}
		}
		break;
		}
		break;
	}
	case "CheckPlayerMapJump"_hash: {
		MapJump* pMapJump = (MapJump*)pMsg->data;
		switch (pMapJump->btCode)
		{
		case 0x62:
		{
			CGameStage::GetInstance()->SendPlayerMapJumpPage(m_pPlayer, pMapJump->szPage);
		}
		break;
		case 0x63:
		{
			CGameStage::GetInstance()->SendPlayerMapJumpHome(m_pPlayer);
		}
		break;
		}
		break;
	}
	case "LianYu18"_hash: {
		BossTJ* pBossTJ = (BossTJ*)pMsg->data;
		if (pBossTJ->btCode == 2 && pBossTJ->nNum == 1)
			CBossTJ::GetInstance()->SendBoss(m_pPlayer, pBossTJ->sName);
		break;
	}
	case "chatglog2023"_hash: {

		break;
	}
	default:
	{
		char szPage[64];
		snprintf(szPage, sizeof(szPage), "自定义界面.%s", pMsg->data);

		const BYTE* p = (const BYTE*)pMsg->data;
		int pos = 0;
		int maxPos = datasize; // 数据边界
		auto ReadInt = [&](int& n) -> BOOL
			{
				if (pos + 4 > maxPos) return FALSE;
				memcpy(&n, p + pos, 4);
				pos += 4;
				return TRUE;
			};
		auto ReadByte = [&](BYTE& b) -> BOOL
			{
				if (pos + 1 > maxPos) return FALSE;
				memcpy(&b, p + pos, 1);
				pos += 1;
				return TRUE;
			};
		auto ReadString = [&](const char*& pszStr) -> BOOL
			{
				if (pos >= maxPos) return FALSE;
				pszStr = (const char*)(p + pos);
				int len = (int)strlen(pszStr);
				if (pos + len + 1 > maxPos) return FALSE;
				pos += len + 1;
				return TRUE;
			};
		const char* pszActName = nullptr;
		if (!ReadString(pszActName)) return;
		m_pPlayer->GetScriptTarget()->SetVariableValue("CustomActName", (char*)pszActName);
		int nActType;
		char szActType[16];
		if (!ReadInt(nActType)) return;
		sprintf(szActType, "%d", nActType);
		m_pPlayer->GetScriptTarget()->SetVariableValue("CustomActType", szActType);
		BYTE byHaveStrData = 0;
		if (!ReadByte(byHaveStrData)) return;
		if (byHaveStrData)
		{
			int nStrCount;
			char szStrCount[16];
			if (!ReadInt(nStrCount)) return;
			if (nStrCount < 0 || nStrCount > 100) return;
			sprintf(szStrCount, "%d", nStrCount);
			m_pPlayer->GetScriptTarget()->SetVariableValue("CustomStrCount", szStrCount);
			char szVar[16];
			for (int i = 0; i < nStrCount; i++)
			{
				const char* pszStr = nullptr;
				if (!ReadString(pszStr)) return;
				sprintf(szVar, "CustomS%d", i + 1);
				m_pPlayer->GetScriptTarget()->SetVariableValue(szVar, (char*)pszStr);
			}
		}
		BYTE byHaveIntData = 0;
		if (!ReadByte(byHaveIntData)) return;
		if (byHaveIntData)
		{
			int nIntCount;
			char szIntCount[16];
			if (!ReadInt(nIntCount)) return;
			if (nIntCount < 0 || nIntCount > 100) return;
			sprintf(szIntCount, "%d", nIntCount);
			m_pPlayer->GetScriptTarget()->SetVariableValue("CustomIntCount", szIntCount);
			char szVar[16], szVal[16];
			int nVal;
			for (int i = 0; i < nIntCount; i++)
			{
				if (!ReadInt(nVal)) return;
				sprintf(szVar, "CustomN%d", i + 1);
				sprintf(szVal, "%d", nVal);
				m_pPlayer->GetScriptTarget()->SetVariableValue(szVar, szVal);
			}
		}
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), szPage);
	}
	}
}

VOID CClientObj::HandleAvatarFrame(PMIRMSG pMsg, int datasize)
{
	if (pMsg->wParam[0] == 0)
	{
		xPacketPool::ScopedPacket packet;
		int nValue = 1;
		packet->push((LPVOID)&nValue, 4);
		nValue = 0;
		packet->push((LPVOID)&nValue, 4);
		nValue = 0;
		packet->push((LPVOID)&nValue, 4);
		nValue = 1;
		packet->push((LPVOID)&nValue, 4);
		nValue = 0;
		packet->push((LPVOID)&nValue, 4);
		nValue = 1;
		packet->push((LPVOID)&nValue, 4);
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa06, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
	}
	else
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa06, pMsg->wParam[0], 0, 0);
}

VOID CClientObj::HandleGuildFengHao(PMIRMSG pMsg, int datasize)
{
	CGuildEx* pGuild = m_pPlayer->GetGuild();
	if (pGuild == nullptr || !pGuild->IsMaster(m_pPlayer))
	{
		m_pPlayer->SaySystem("你不是掌门人，不能编辑行会封号!");
		return;
	}
	switch (pMsg->wParam[0])
	{
	case 1:
	{
		CGuildGroupEx* pGroup = pGuild->NewGroup("成员");
		pGuild->AddGroupToList(pGroup);
		pGuild->Save();
		pGuild->SendGroupFengHaoData();
	}
	break;
	case 3:
	{
		WORD nLevel = pMsg->wParam[1];
		if (nLevel == 0 || nLevel > 99)
		{
			m_pPlayer->SendMsg(-7, 0x2f9, 0, 0, 0);
			break;
		}
		char* pszGroupName = (char*)pMsg->data;
		if (!pGuild->IsProperName(pszGroupName))
		{
			m_pPlayer->SendMsg(-15, 0x2f9, 0, 0, 0);
			break;
		}
		if (strlen(pszGroupName) > 16)
		{
			pszGroupName[16] = '\0';
		}
		CGuildGroupEx* pGroup = pGuild->GetGroupByLevel(nLevel);
		pGroup->SetName(pszGroupName);
		pGuild->Save();
		pGuild->SendGroupFengHaoData();
		pGuild->SendDurationMemberList(m_pPlayer);
	}
	break;
	case 4:
	{
		char* pszCharName = (char*)pMsg->data;
		if (strcmp(m_pPlayer->GetName(), pszCharName) == 0)
		{
			m_pPlayer->SaySystem("不能对自己进行免职和任职操作!");
			break;
		}
		if (strcmp(pGuild->GetFirstOwnerName(), pszCharName) == 0)
		{
			m_pPlayer->SaySystem("不能对掌门人进行免职和任职操作!");
			break;
		}
		WORD nLevel = pMsg->wParam[2];
		WORD nOldLevel = pMsg->wParam[1];
		if (pGuild->ChangeGroup(pszCharName, nLevel, nOldLevel))
			pGuild->SendDurationMemberList();
		else
			m_pPlayer->SaySystem("免职或任职操作失败!");
	}
	break;
	}
}

VOID CClientObj::HandleReplyGuild(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->ReplyAddToGuildRequest((BOOL)pMsg->wParam[0]);
}

VOID CClientObj::HandleTrainHorse(PMIRMSG pMsg, int datasize)
{
	int	x = pMsg->dwFlag & 0xffff;
	int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
	int dir = pMsg->wParam[1] & 0xff;
	SendActionResult(m_pPlayer->DoTrainHorse(dir, x, y));
}

VOID CClientObj::HandleTurn(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->IsEnterMap()) return;
	SendActionResult(m_pPlayer->Turn(pMsg->wParam[1] & 0xff));
}

VOID CClientObj::HandleWalk(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->IsEnterMap()) return;
	int	x = pMsg->dwFlag & 0xffff;
	int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
	int dir = pMsg->wParam[1] & 0xff;
	BOOL bSuccess = FALSE;
	if (m_pPlayer->WalkXY(x, y, dir))
	{
		bSuccess = TRUE;
		CScriptTargetForPlayer* pSTarget = m_pPlayer->GetScriptTarget();
		if (pSTarget) pSTarget->CheckExceedDistance();
	}
	SendActionResult(bSuccess);
}

VOID CClientObj::HandleGetMeal(PMIRMSG pMsg, int datasize)
{
	SendActionResult(m_pPlayer->GetMeal(pMsg->wParam[1] & 0xff));
}

VOID CClientObj::HandleRun(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->IsEnterMap()) return;
	int	x = pMsg->dwFlag & 0xffff;
	int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
	int dir = pMsg->wParam[1] & 0xff;
	BOOL bSuccess = FALSE;
	if (m_pPlayer->RunXY(x, y, dir))
	{
		bSuccess = TRUE;
		CScriptTargetForPlayer* pSTarget = m_pPlayer->GetScriptTarget();
		if (pSTarget) pSTarget->CheckExceedDistance();
	}
	SendActionResult(bSuccess);
}

VOID CClientObj::HandleAttack(PMIRMSG pMsg, int datasize)
{
	damage_type curAttackType = DT_PHYSICS;
	if (m_pPlayer->CheckItemInfo(_U_WEAPON, 5, 7))
		curAttackType = DT_CUTTREE;
	e_humanattackmode mode = m_pPlayer->GetAttackMode();
	SendActionResult(m_pPlayer->Attack(pMsg->wParam[1] & 0xff, 0, mode, curAttackType));
}

VOID CClientObj::HandleMine(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer->CanmMine()) return;
	int	x = pMsg->dwFlag & 0xffff;
	int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
	int dir = pMsg->wParam[1] & 0xff;
	BOOL bSuccess = FALSE;
	if (m_pPlayer->DoMine(dir, x, y))
	{
		bSuccess = TRUE;
	}
	SendActionResult(bSuccess);
}

VOID CClientObj::HandleSpellSkill(PMIRMSG pMsg, int datasize)
{
	DWORD dwId = MAKEDWORD2W(pMsg->wParam[0], pMsg->wParam[2]);
	int x, y;
	x = pMsg->dwFlag & 0xffff;
	y = (pMsg->dwFlag & 0xffff0000) >> 16;
	SendActionResult(m_pPlayer->SpellCast(x, y, dwId, pMsg->wParam[1]));
}

VOID CClientObj::HandleSpecialHit(PMIRMSG pMsg, int datasize)
{
	SendActionResult(m_pPlayer->SpecialHit(pMsg->wParam[1], pMsg->wParam[2]));
}

VOID CClientObj::HandleStop(PMIRMSG pMsg, int datasize)
{
	SendActionResult(TRUE);
	m_pPlayer->Stop();
}

VOID CClientObj::HandleBowAction(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->ChangeChatChannel();
}

VOID CClientObj::HandleSay(PMIRMSG pMsg, int datasize)
{
	if (m_pPlayer->IsSystemFlagSeted(SF_BANED))
	{
		m_pPlayer->SaySystem(getstrings(SD_YOUAREBANED));
		return;
	}
	switch (pMsg->wParam[0])
	{
	case 0: case 1:
	{
		switch (pMsg->data[0])
		{
		case '@':
		{
			OnCommand(pMsg->data + 1);
		}
		break;
		case '/':
		{
			char* p = pMsg->data + 1;
			while (*p && *p != ' ' && *p != '\t')
				p++;
			if (*p != 0)
			{
				*p = 0;
				p++;
			}
			m_pPlayer->ChannelSay(CCH_WISPER, pMsg->data + 1, p);
		}
		break;
		case '!':
		{
			switch (pMsg->data[1])
			{
			case '!':
				m_pPlayer->ChannelSay(CCH_GROUP, nullptr, pMsg->data + 2);
				break;
			case '~':
				m_pPlayer->ChannelSay(CCH_GUILD, nullptr, pMsg->data + 2);
				break;
			default:
				m_pPlayer->ChannelSay(CCH_CRY, nullptr, pMsg->data + 1);
				break;
			}
		}
		break;
		default:
		{
			if (m_pPlayer->IsSystemFlagSeted(SF_SCROLLTEXTMODE))
				CGameWorld::GetInstance()->AddGlobeProcess(EP_SCROLLTEXT, 0, 0, 0, 0, 50, 1, pMsg->data);
			else if (m_pPlayer->IsSystemFlagSeted(SF_NOTICEMODE))
				CGameWorld::GetInstance()->PostSystemMessage(pMsg->data);
			else
				m_pPlayer->ChannelSay(m_pPlayer->GetChatChannel(), nullptr, pMsg->data);
		}
		break;
		}
	}
	break;
	case 2:
	case 3:
	{
		char* p = pMsg->data;
		while (*p && *p != ' ')
			p++;
		if (*p != 0)
		{
			*p = 0;
			p++;
		}
		CHumanPlayer* player = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data);
		if (player)
		{
			char szBuff[248];
			sprintf_s(szBuff, sizeof(szBuff), "%s: %s", m_pPlayer->GetName(), p);
			szBuff[sizeof(szBuff) - 1] = '\0';
			WORD nType = pMsg->wParam[0];
			player->SendMsg(m_pPlayer->GetId(), 0x67, 0, 0, nType, szBuff);
		}
		else
			m_pPlayer->SaySystem("%s 玩家不在线, 无法收到信息!", pMsg->data);
	}
	break;
	}
}

VOID CClientObj::HandleClientTime(PMIRMSG pMsg, int datasize)
{
	SendMsg(GetUnixTimeSec(), 0x9600, 0, 0, 0);
}

VOID CClientObj::HandlePetPickup(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->SetPetFindItem(pMsg->wParam[0], pMsg->wParam[1]);
}

VOID CClientObj::HandlePutToPetBag(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->PutItemToPetBag(pMsg->dwFlag);
}

VOID CClientObj::HandleGetFromPetBag(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->GetItemFromPetBag(pMsg->dwFlag);
}

VOID CClientObj::HandleOpenMarket(PMIRMSG pMsg, int datasize)
{
	CMarketManager::GetInstance()->OnClientMsg(m_pPlayer, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
}

VOID CClientObj::HandlePrivateShop(PMIRMSG pMsg, int datasize)
{
	PRIVATESHOPQUERY* pQuery = (PRIVATESHOPQUERY*)pMsg->data;
	BOOL bStarted = FALSE;
	if (pMsg->wParam[0] > 0)
	{
		if (m_pPlayer->GetActionType() == AT_PRIVATESHOP)
			bStarted = TRUE;
		if (m_pPlayer->SetPrivateShop(pMsg->wParam[0], pQuery))
		{
			if (bStarted)
				m_pPlayer->UpdatePrivateShopToAround();
			else
				m_pPlayer->SendStartPrivateShop();
		}
		else
		{
			m_pPlayer->StopPrivateShop();
			m_pPlayer->SendStopPrivateShop();
		}
	}
	else
	{
		m_pPlayer->StopPrivateShop();
		m_pPlayer->SendStopPrivateShop();
	}
}

VOID CClientObj::HandleDeleteTask(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->DeleteTask(static_cast<WORD>(pMsg->dwFlag));
}

VOID CClientObj::HandleRequestPrivateShop(PMIRMSG pMsg, int datasize)
{
	if (pMsg->wParam[0] == 5)
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x80D5, 5, 0, 0);
}

VOID CClientObj::HandleCreateGuild(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->GetScriptTarget()->OnInputConfirm(pMsg->data);
}

VOID CClientObj::HandlePersonSetting(PMIRMSG pMsg, int datasize)
{
	WORD wPersonCode = pMsg->wParam[1];
	if (wPersonCode > 0)
	{
		m_pPlayer->SetPersonCode(wPersonCode);
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 3, 1, 0);
	}
	PersonSetting* pPersonSetting = (PersonSetting*)pMsg->data;
	if (pPersonSetting->szPersonSign[0] != 0)
	{
		m_pPlayer->SetPersonSign(pPersonSetting->szPersonSign);
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 1, 0, wPersonCode, pPersonSetting->szPersonSign);
	}
	if (pPersonSetting->szTempBan[0] != 0)
	{
		m_pPlayer->SetTempRank(pPersonSetting->szTempBan);
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 2, 0, 0, pPersonSetting->szTempBan);
	}
}

VOID CClientObj::HandlePutItem(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->OnPutItem(pMsg->dwFlag, *(UINT*)&pMsg->wParam[0]);
}

VOID CClientObj::HandleShowPetInfo(PMIRMSG pMsg, int datasize)
{
	m_pPlayer->ShowPetInfo();
}

VOID CClientObj::HandlePetWindow(PMIRMSG pMsg, int datasize)
{
	CItemBox& box = m_pPlayer->GetBag();
	ITEM* pPetStones[5];
	int nCount = box.FindItems("豹魔石", pPetStones, 5, TRUE);
	for (int i = 0; i < nCount; i++)
		m_pPlayer->SendOutPetInfo(pPetStones[i], 1);
	ITEM* pItem = m_pPlayer->GetEquipedHorseItem();
	if (pItem) m_pPlayer->SendOutPetInfo(pItem, 2);
}

VOID CClientObj::HandleHeroRank(PMIRMSG pMsg, int datasize)
{
	if (pMsg->wParam[0] == 0)
	{
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9702, 35, 0, 0);
	}
}

VOID CClientObj::HandleQueryHeroRank(PMIRMSG pMsg, int datasize)
{
	CTopManager::GetInstance()->SendRank(m_pPlayer, pMsg->wParam[0], pMsg->wParam[2]);
}
