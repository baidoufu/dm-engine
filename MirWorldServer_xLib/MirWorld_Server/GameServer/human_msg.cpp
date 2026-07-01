#include "stdafx.h"
#include "FengHaoGrowManager.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "scriptshell.h"
#include "scriptview.h"
#include "logicmap.h"
#include "timesystem.h"

VOID CHumanPlayer::SendUpdateItem(ITEM& item)
{
	ItemToClient(item);
	ITEMCLIENT clientItem;
	memcpy(&clientItem, &item, sizeof(ITEMCLIENT));
	SendMsg(GetId(), 0xcb, 0, 0, 1, &clientItem, sizeof(ITEMCLIENT));
}

VOID CHumanPlayer::SendWeaponBroken()
{
	SaySystem("您的武器已破碎!");
	SendAroundMsg(GetId(), 0x44e, 0, 0, 0);
	SendMsg(GetId(), 0x44e, 0, 0, 0);
}

VOID CHumanPlayer::SendAddCommunity(WORD wType, const char* pszName)
{
	SendMsg(0, 0x1c2, wType, 0, 0, (LPVOID)pszName);
}

VOID CHumanPlayer::SendDeleteCommunity(WORD wType, const char* pszName)
{
	SendMsg(0, 0x1c3, wType, 0, 0, (LPVOID)pszName);
}

VOID CHumanPlayer::SendSetPetBag(UINT nSize)
{
	SendMsg(GetId(), 0x9601, 0, 0, nSize);
}

VOID CHumanPlayer::SendPetBag(ITEMCLIENT* pItems, UINT nCount)
{
	if (nCount > 0 && pItems != nullptr)
		SendMsg(GetId(), 0x9602, 0, 0, nCount, (LPVOID)pItems, sizeof(ITEMCLIENT) * nCount);
	else
	{
		nCount = 0;
		SendMsg(GetId(), 0x9602, 0, 0, nCount);
	}
}

VOID CHumanPlayer::SendCloseScriptPage(UINT nId)
{
	SendMsg(nId, 0x284, 0, 0, 0, nullptr);
}

VOID CHumanPlayer::SendPage(CScriptShell* pShell, CScriptView* pView)
{
	UINT id = pShell == nullptr ? 0xffffffff : pShell->GetTitleId();
	SendMsg(id, 0x283, static_cast<WORD>(pView->GetParam()), 0, 1, (LPVOID)pView->getPacket().getbuf(), pView->getPacket().getsize());
}

VOID CHumanPlayer::SendClosePage(CScriptShell* pShell)
{
	SendCloseScriptPage(pShell == nullptr ? 0xffffffff : pShell->GetTitleId());
}

VOID CHumanPlayer::PostMsg(const char* pszMsg, int length)
{
	if (m_pClientObj == nullptr || pszMsg == nullptr) return;
	if (length == 0) length = (int)strlen(pszMsg);
	m_pClientObj->PostMsg(pszMsg, length);
}

VOID CHumanPlayer::SendTimeWeatherChanged()
{
	if (m_pMap == nullptr) return;
	WORD wTime = 0xffff;
	if (!m_pMap->IsFlagSeted(MF_DAY) && !m_pMap->IsFlagSeted(MF_NIGHT))
		wTime = CTimeSystem::GetInstance()->GetCurrentlyTime();
	WORD wWeather = m_pMap->GetWeather().wWeatherIndex;
	WORD wFlag = m_pMap->GetWeather().wFlag;
	DWORD dwWeatherColor = m_pMap->GetWeather().dwWeatherColor;
	if (!m_pMap->IsFlagSeted(MF_WEATHER))
	{
		wWeather = 0;
		dwWeatherColor = 0xffffffff;
	}
	SendMsg(m_pMap->GetWeather().dwBGColor, SM_SETGAMEDATETIME, wTime, wWeather, wFlag, &dwWeatherColor, sizeof(DWORD));
}

static thread_local std::array<DBITEM, 100> s_dbPacketDst{};
static thread_local std::array<ITEM, 100> s_dbPacketItems{};
static thread_local std::array<BAGITEMPOS, 100> s_dbPacketPos{};
VOID CHumanPlayer::GetDBInfoPacket(xPacket& packet)
{
	CHARDBINFO info;
	int length = 0;
	packet.clear();
	// 角色属性数据
	if (GetDBInfo(info))
	{
		if (!IsDeath())
		{
			if (info.hp == 0)
				info.hp = info.maxhp;
		}
		else
			info.hp = 0;
		length = EncodeMsg((char*)packet.getfreebuf(), 0, DM_PUTCHARDBINFO, 0, 0, 0, &info, sizeof(info));
		packet.addsize(length);
	}
	// 社交数据
	if (IsSystemFlagSeted(SF_COMMUNITYLOADED))
	{
		char szCommunityText[4096];
		length = GetCommunityInfo(szCommunityText, 4096);
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATECOMMUNITY, 0, 0, 0, (LPVOID)szCommunityText);
		packet.addsize(length);
	}
	// 已学习技能数据
	if (this->m_fMagicLoaded)
	{
		std::array<MAGICDB, 255> array{};
		USERMAGIC* pMagic = m_pMagic;
		int	count = 0;
		while (pMagic)
		{
			if (count < 255)
				array[count++] = pMagic->magic;
			pMagic = pMagic->pNext;
		}
		if (count > 0)
		{
			length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEMAGIC, 0, 0, count, (LPVOID)array.data(), sizeof(MAGICDB) * count);
			packet.addsize(length);
		}
	}
	// 任务信息
	length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATETASKINFO, 0, 0, 0, (LPVOID)&_taskInfo(), sizeof(_taskInfo()));
	packet.addsize(length);
	// 时长封号信息
	length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEFENGHAO, 0, 0, 0, (LPVOID)&_fenghaoInfo(), sizeof(_fenghaoInfo()));
	packet.addsize(length);
	// 背包数据
	int count = 0;
	count = m_ItemBox.GetItems(s_dbPacketItems.data(), 100);
	int updatecount = 0;
	int uposcount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbPacketItems[i].dwMakeIndex & 0x80000000)
		{
			if (CItemManager::GetInstance()->ItemLimited(s_dbPacketItems[i], IL_NOUPDATETODB))continue;
			s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
			s_dbPacketDst[updatecount].btFlag = 0;
			s_dbPacketDst[updatecount].pos = static_cast<WORD>(s_dbPacketItems[i].dwParam[2]);
			updatecount++;
		}
		else
		{
			BYTE btFlag = static_cast<BYTE>(s_dbPacketItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
				s_dbPacketDst[updatecount].btFlag = btFlag;
				s_dbPacketDst[updatecount].pos = static_cast<WORD>(s_dbPacketItems[i].dwParam[2]);
				updatecount++;
			}
			else
			{
				s_dbPacketPos[uposcount].ItemId = s_dbPacketItems[i].dwMakeIndex;
				s_dbPacketPos[uposcount].wPos = static_cast<WORD>(s_dbPacketItems[i].dwParam[2]);
				uposcount++;
			}
		}
	}
	if (updatecount > 0)
	{
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEITEMS, updatecount, IDF_BAG, 0, (LPVOID)s_dbPacketDst.data(), sizeof(DBITEM) * updatecount);
		packet.addsize(length);
	}
	// 背包的其他物品的位置要更新
	if (uposcount > 0)
	{
		length = EncodeMsg((char*)packet.getfreebuf(), 0, DM_UPDATEITEMPOSEX, uposcount, IDF_BAG, 0, (LPVOID)s_dbPacketPos.data(), sizeof(BAGITEMPOS) * uposcount);
		packet.addsize(length);
	}
	// 仓库物品数据
	count = m_ItemBank.GetItems(s_dbPacketItems.data(), 100);
	updatecount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbPacketItems[i].dwMakeIndex & 0x80000000)
		{
			if (CItemManager::GetInstance()->ItemLimited(s_dbPacketItems[i], IL_NOUPDATETODB))continue;
			s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
			s_dbPacketDst[updatecount].btFlag = 0;
			s_dbPacketDst[updatecount].pos = i;
			updatecount++;
		}
		else
		{
			BYTE btFlag = (s_dbPacketItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
				s_dbPacketDst[updatecount].btFlag = btFlag;
				s_dbPacketDst[updatecount].pos = i;
				updatecount++;
			}
		}
	}
	if (updatecount > 0)
	{
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEITEMS, updatecount, IDF_BANK, 0, (LPVOID)s_dbPacketDst.data(), sizeof(DBITEM) * updatecount);
		packet.addsize(length);
	}
	// 宠物背包数据
	count = m_ItemPetBag.GetItems(s_dbPacketItems.data(), 100);
	updatecount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbPacketItems[i].dwMakeIndex & 0x80000000)
		{
			if (CItemManager::GetInstance()->ItemLimited(s_dbPacketItems[i], IL_NOUPDATETODB))continue;
			s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
			s_dbPacketDst[updatecount].btFlag = 0;
			s_dbPacketDst[updatecount].pos = i;
			updatecount++;
		}
		else
		{
			BYTE btFlag = (s_dbPacketItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbPacketDst[updatecount].item = s_dbPacketItems[i];
				s_dbPacketDst[updatecount].btFlag = btFlag;
				s_dbPacketDst[updatecount].pos = i;
				updatecount++;
			}
		}
	}
	if (updatecount > 0)
	{
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEITEMS, updatecount, IDF_PETBANK, 0, (LPVOID)s_dbPacketDst.data(), sizeof(DBITEM) * updatecount);
		packet.addsize(length);
	}
	// 装备数据
	ITEM* pEquipment = nullptr;
	updatecount = 0;
	for (int i = 0; i < _U_MAX; i++)
	{
		pEquipment = m_Equipments.GetEquipment(i);
		if (pEquipment)
		{
			if (pEquipment->dwMakeIndex & 0x80000000)
			{
				if (CItemManager::GetInstance()->ItemLimited(*pEquipment, IL_NOUPDATETODB))continue;
				s_dbPacketDst[updatecount].item = *pEquipment;
				s_dbPacketDst[updatecount].btFlag = 0;
				s_dbPacketDst[updatecount].pos = i;
				updatecount++;
			}
			else
			{
				BYTE btFlag = (pEquipment->baseitem.wFeature & 0x00f0);
				if (btFlag != 0)
				{
					s_dbPacketDst[updatecount].item = *pEquipment;
					s_dbPacketDst[updatecount].btFlag = btFlag;
					s_dbPacketDst[updatecount].pos = i;
					updatecount++;
				}
			}
		}
	}
	if (updatecount > 0)
	{
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEITEMS, updatecount, IDF_EQUIPMENT, 0, (LPVOID)s_dbPacketDst.data(), sizeof(DBITEM) * updatecount);
		packet.addsize(length);
	}
	// 存储升级临时武器的物品
	if ((this->GetUpgradeItemComp()->Item.dwMakeIndex & 0x80000000) && !CItemManager::GetInstance()->ItemLimited(this->GetUpgradeItemComp()->Item, IL_NOUPDATETODB))
	{
		updatecount = 0;
		s_dbPacketDst[updatecount].item = GetUpgradeItemComp()->Item;
		s_dbPacketDst[updatecount].btFlag = 0;
		s_dbPacketDst[updatecount].pos = 0;
		updatecount++;
		length = EncodeMsg((char*)packet.getfreebuf(), info.dwDBId, DM_UPDATEITEMS, updatecount, IDF_UPGRADE, 0, (LPVOID)s_dbPacketDst.data(), sizeof(DBITEM) * updatecount);
		packet.addsize(length);
	}
}