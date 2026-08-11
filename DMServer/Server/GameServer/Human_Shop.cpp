#include "StdAfx.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "gsclientobj.h"

// ==================== 个人摆摊系统 ====================

BOOL CHumanPlayer::SetPrivateShop(int iCount, PRIVATESHOPQUERY* pQuery)
{
	if (GetActionType() != AT_PRIVATESHOP && !CanDoAction(AT_PRIVATESHOP))return FALSE;
	if (iCount > 10)return FALSE;
	o_strncpy(m_szPrivateShopName.data(), pQuery->szName, 52);
	m_iPrivateShopItemCount = 0;
	for (int i = 0; i < iCount; i++)
	{
		m_PrivateShopCache[m_iPrivateShopItemCount].pItem = m_ItemBox.FindItem(pQuery->items[i].dwMakeIndex);
		if (m_PrivateShopCache[m_iPrivateShopItemCount].pItem == nullptr)continue;
		if (CItemManager::GetInstance()->ItemLimited(*m_PrivateShopCache[m_iPrivateShopItemCount].pItem, IL_NOPRIVATESHOP))
		{
			SaySystem("摊位中有不能出售的物品!");
			return FALSE;
		}
		m_PrivateShopCache[m_iPrivateShopItemCount].dwPrice = pQuery->items[i].dwPrice;
		m_PrivateShopCache[m_iPrivateShopItemCount].pricetype = (money_type)(pQuery->items[i].wPriceType & 1);
		m_iPrivateShopItemCount++;
	}
	if (m_iPrivateShopItemCount == 0)return FALSE;
	int olddir = (int)GetDirection();
	int newdir = olddir;
	if ((newdir & 1) == 0)
	{
		if (Getrand(2) == 0)
			newdir++;
		else
			newdir--;
	}
	newdir = (newdir + 8) % 8;
	SetDirection((e_direction)newdir);
	if (!SetAction(AT_PRIVATESHOP, GetDirection(), getX(), getY(), 0xffffffff))
	{
		SetDirection((e_direction)olddir);
		return FALSE;
	}
	return TRUE;
}

BOOL CHumanPlayer::StopPrivateShop()
{
	SetAction(AT_STAND, GetDirection(), getX(), getY(), 0);
	return TRUE;
}

BOOL CHumanPlayer::SendPrivateShopPage(CHumanPlayer* pQueryer, WORD wFlag)
{
	if (pQueryer == nullptr)return FALSE;
	if (this->m_ActionType != AT_PRIVATESHOP)return FALSE;
	PRIVATESHOPSHOW	psshow;
	this->GetPrivateShopView(psshow.header);
	psshow.header.wCount = static_cast<WORD>(m_iPrivateShopItemCount);
	psshow.header.w2 = static_cast<BYTE>(wFlag);
	if (wFlag == 1)
		pQueryer->SetCurPrivateShopView(this);
	o_strncpy(psshow.header.szName, m_szPrivateShopName.data(), 51);
	int ptr = 0;
	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem == nullptr)continue;
		psshow.items[ptr] = *(ITEMCLIENT*)m_PrivateShopCache[i].pItem;
		psshow.items[ptr].baseitem.btPriceType = static_cast<BYTE>(m_PrivateShopCache[i].pricetype);
		psshow.items[ptr].baseitem.nPrice = m_PrivateShopCache[i].dwPrice;
		ptr++;
	}
	psshow.header.wCount = (WORD)ptr;
	if (ptr == 0)return FALSE;
	pQueryer->SendMsg(GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psshow, sizeof(PRIVATESHOPHEADER) + sizeof(ITEMCLIENT) * ptr);
	return TRUE;
}

static thread_local std::array<char, 65536> g_szCodedMsgBuffer{};
VOID CHumanPlayer::UpdatePrivateShopToAround()
{
	PRIVATESHOPHEADER psheader{};
	this->GetPrivateShopView(psheader);
	psheader.w2 = 0;
	DWORD* pdwParam = (DWORD*)&psheader.w1;
	SendMsg(GetId(), 0x80d7, getX(), getY(), (WORD)GetDirection(), (LPVOID)pdwParam, sizeof(DWORD) * 2);

	if (m_xVisibleObjectList.getCount() == 0) return;

	PRIVATESHOPSHOW	psshow;
	psshow.header.wCount = static_cast<WORD>(m_iPrivateShopItemCount);
	psshow.header.w2 = 2;
	o_strncpy(psshow.header.szName, m_szPrivateShopName.data(), 51);

	int ptr = 0;
	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem == nullptr)continue;
		psshow.items[ptr] = *(ITEMCLIENT*)m_PrivateShopCache[i].pItem;
		psshow.items[ptr].baseitem.btPriceType = static_cast<BYTE>(m_PrivateShopCache[i].pricetype);
		psshow.items[ptr].baseitem.nPrice = m_PrivateShopCache[i].dwPrice;
		ptr++;
	}
	psshow.header.wCount = (WORD)ptr;

	if (ptr == 0) return;

	int size = EncodeMsg(g_szCodedMsgBuffer.data(), GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psshow, sizeof(PRIVATESHOPHEADER) + sizeof(ITEMCLIENT) * ptr);

	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObject = pNode->getObject()->pObject;
		if (pObject && pObject->GetType() == OBJ_PLAYER && ((CHumanPlayer*)pObject)->GetCurPrivateShopView() == this)
			pObject->OnAroundMsg(this, g_szCodedMsgBuffer.data(), size);
		pNode = pNode->getNext();
	}
}

BOOL CHumanPlayer::BuyPrivateShopItem(CHumanPlayer* pBuyer, DWORD dwItemId, const char* pszName)
{
	CItemBox& box = pBuyer->GetBag();
	if (box.GetFree() <= 0)
	{
		pBuyer->SaySystem("背包满了, 无法购买物品!");
		return FALSE;
	}

	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem)
		{
			if (m_PrivateShopCache[i].pItem->dwMakeIndex == dwItemId)
			{
				DWORD dwMoney = pBuyer->GetMoney(m_PrivateShopCache[i].pricetype);
				if (dwMoney < m_PrivateShopCache[i].dwPrice)
				{
					pBuyer->SaySystem("没有足够的金钱来购买此物品!");
					return FALSE;
				}
				if (!TestAddMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice))
				{
					char szName[20];
					o_strncpy(szName, m_PrivateShopCache[i].pItem->baseitem.szName, 16);
					SaySystem("%s要购买您的%s物品, 但是你已经无法再携带更多的钱了.",
						pBuyer->GetName(), szName);
					pBuyer->SaySystem("卖家不能再携带更多的钱了!");
					return FALSE;
				}
				if (pBuyer->AddBagItem(*m_PrivateShopCache[i].pItem, FALSE, TRUE))
				{
					pBuyer->CostMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice);
					AddMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice);
					m_iPrivateShopItemCount--;
					ITEM item = *m_PrivateShopCache[i].pItem;
					if (m_iPrivateShopItemCount > 0)
					{
						for (int j = i; j < m_iPrivateShopItemCount; j++)
						{
							m_PrivateShopCache[j] = m_PrivateShopCache[j + 1];
						}
					}
					m_ItemBox.RemoveItem(dwItemId);
					SendTakeBagItem(&item);
					SendWeightChanged();
					return TRUE;
				}
				else
					return FALSE;
			}
		}
	}
	return FALSE;
}
