#include "StdAfx.h"
#include ".\scriptnpc.h"
#include ".\gameworld.h"
#include "humanplayer.h"
#include "cmdproc.h"
#include "npcmanager.h"
#include "itemmanager.h"
#include "sandcity.h"
#include "GuildEx.h"
#include "Server.h"
#include "scriptobject.h"
#include "scriptelement.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <string>  
#include <cctype>
#include <array>

CScriptNpc::CScriptNpc(VOID)
{
	m_szName.fill(0);
	Clean();
}

CScriptNpc::~CScriptNpc(VOID)
{
}

VOID CScriptNpc::Clean()
{
	CAliveObject::Clean();
	auto* mc = GetNpcMerchant();
	if (mc && mc->pSellGoodsList)
	{
		NpcGoodsList* pNext = nullptr;
		while (mc->pSellGoodsList)
		{
			pNext = mc->pSellGoodsList->pNext;

			NpcGoodsItemList* pNextItem = nullptr;
			while (mc->pSellGoodsList->pItemList)
			{
				pNextItem = mc->pSellGoodsList->pItemList->pNext;

				CNpcManager::GetInstance()->FreeGoodsItemList(mc->pSellGoodsList->pItemList);
				mc->pSellGoodsList->pItemList = pNextItem;
			}

			CNpcManager::GetInstance()->FreeGoodsList(mc->pSellGoodsList);
			mc->pSellGoodsList = pNext;
		}
		mc->pSellGoodsList = nullptr;
	}
}

BOOL CScriptNpc::Init(UINT dbid, const char* pszName, int view, int x, int y, DWORD mapid, CScriptObject* pScriptObject)
{
	auto* st = GetNpcState();
	auto* mc = GetNpcMerchant();
	mc->pSellGoodsList = nullptr;
	m_pScriptObject = pScriptObject;
	setXY(x, y);
	SetMapId(mapid);
	SetDirection((e_direction)(5 + Getrand(3)));
	if (st) st->nView = view;
	if (st) st->StoreId = dbid | 0x70000000;
	if (*pszName == '*')
	{
		pszName++;
		if (st) st->fSandCityMerchant = TRUE;
	}
	else
		if (st) st->fSandCityMerchant = FALSE;
	o_strncpy(m_szName.data(), pszName, 31);
	o_strncpy(m_szLongName.data(), m_szName.data(), 31);
	mc->fChanged = FALSE;
	Goods* pGoodList = nullptr;
	if (m_pScriptObject && m_pScriptObject->getGoodsList() && m_pScriptObject->getGoodsList()->getList())
	{
		pGoodList = m_pScriptObject->getGoodsList()->getList();
	}
	if (pGoodList != nullptr)
	{
		if (!InitGoods(pGoodList))
			return FALSE;
		LoadItems();
	}
	m_tmrUpdateItem.Savetime();
	mc->dwTimeOut = 0;
	if (st) st->bIsNpc = TRUE;
	return TRUE;
}

BOOL CScriptNpc::InitGoods(tagGoods* pGoodsList)
{
	tagGoods* p = pGoodsList;
	if (p == nullptr) return FALSE;

	NpcGoodsList* pGoodsListTail = nullptr;
	ITEMCLASS* pItemClass = nullptr;

	for (p = pGoodsList; p != nullptr; p = p->pNext)
	{
		auto* mc = GetNpcMerchant();
		if (mc->pSellGoodsList == nullptr)
		{
			mc->pSellGoodsList = CNpcManager::GetInstance()->AllocGoodsList();
			assert(mc->pSellGoodsList != nullptr);
			pGoodsListTail = mc->pSellGoodsList;
		}
		else
		{
			if (pGoodsListTail)
			{
				pGoodsListTail->pNext = CNpcManager::GetInstance()->AllocGoodsList();
				pGoodsListTail = pGoodsListTail->pNext;
			}
		}

		if (pGoodsListTail)
		{
			pGoodsListTail->defaultcount = p->wCount;
			pGoodsListTail->refreshtime = p->wRefreshTime;
			o_strncpy(pGoodsListTail->szTemplate.data(), p->szName.data(), 31);

			pItemClass = CItemManager::GetInstance()->GetItemClassByName(pGoodsListTail->szTemplate.data());
			if (pItemClass == nullptr)
				continue;

			pGoodsListTail->dwTemplatePrice = ROUND(mc->fBuyPercent * pItemClass->nPrice);
		}
	}
	return TRUE;
}

VOID CScriptNpc::QueryTalk(CHumanPlayer* pPlayer)
{
	auto* st = GetNpcState();
	if (!st || !st->bTalk) return;
	QuerySelectLink(pPlayer, nullptr);
}

VOID CScriptNpc::QuerySelectLink(CHumanPlayer* pPlayer, const char* pLink, BOOL bHumanQuery)
{
	if (pLink != nullptr)
	{
		if (_stricmp(pLink, "@exit") == 0)
		{
			SendClosePage(pPlayer);
			return;
		}
		else if (_strnicmp(pLink, "@AttackRequestPage", 18) == 0)
		{
			std::array<char, 2096> szbuffer{};
			UINT id = StringToInteger(pLink + 18);
			sprintf(szbuffer.data(), "%s/", m_szName.data());
			int len = static_cast<int>(strlen(szbuffer.data()));
			int length = CSandCity::GetInstance()->PrePareAttackRequestPage(id, szbuffer.data() + len);
			pPlayer->SendMsg(GetId(), 0x283, 0, 0, 0, (LPVOID)szbuffer.data(), length + len);
			return;
		}
	}
	if (this->m_pScriptObject && this->m_pScriptObject->IsBigBox())
	{
		CScriptObject* pLeftPage = this->m_pScriptObject->GetLeftPage();
		if (pLink == nullptr)
		{
			if (!Execute(pPlayer->GetScriptTarget(), pLink, 1, 0x10))
			{
				SendClosePage(pPlayer);
				return;
			}
			CSe_Page* pPage = pLeftPage != nullptr ? pLeftPage->GetPage("@main") : nullptr;
			if (pPage)
			{
				if (!Execute(pPlayer->GetScriptTarget(), pPage, 0, 0x11))
				{
					SendClosePage(pPlayer);
					return;
				}
			}
		}
		else
		{
			CSe_Page* pPage = m_pScriptObject->GetPage(pLink);
			if (pPage)
			{
				if (!Execute(pPlayer->GetScriptTarget(), pPage, 0, 0x10))
				{
					SendClosePage(pPlayer);
					return;
				}
			}
			else
			{
				pPage = pLeftPage != nullptr ? pLeftPage->GetPage(pLink) : nullptr;
				if (pPage)
				{
					if (!Execute(pPlayer->GetScriptTarget(), pPage, 0, 0x11))
					{
						SendClosePage(pPlayer);
						return;
					}
				}
				else
				{
					if (!Execute(pPlayer->GetScriptTarget(), pLink, 0, 0x11))
					{
						SendClosePage(pPlayer);
						return;
					}
				}
			}
		}
	}
	else
	{
		if (!Execute(pPlayer->GetScriptTarget(), pLink, bHumanQuery))
		{
			SendClosePage(pPlayer);
			return;
		}
	}

	if (pLink != nullptr)
	{
		if (_stricmp(pLink, "@s_repair") == 0)
		{
			pPlayer->SetSystemFlag(SF_SPECIALREPAIR, TRUE);
			pPlayer->SendMsg(GetId(), 0x29c, 0, 0, 0, nullptr);
		}
		else if (_stricmp(pLink, "@repair") == 0)
		{
			pPlayer->SetSystemFlag(SF_SPECIALREPAIR, FALSE);
			pPlayer->SendMsg(GetId(), 0x29c, 0, 0, 0, nullptr);
		}
		else if (_stricmp(pLink, "@buy") == 0)
			SendGoodsList(pPlayer);
		else if (_stricmp(pLink, "@sell") == 0)
			pPlayer->SendMsg(GetId(), 0x286, 0, 0, 0);
		else if (_stricmp(pLink, "@getback") == 0)
			pPlayer->SendBank(GetId());
		else if (_stricmp(pLink, "@storage") == 0)
			pPlayer->SendMsg(GetId(), 0x2bc, 0, 0, 0);
	}
}

//关闭NPC对话框
VOID CScriptNpc::SendClosePage(CHumanPlayer* pPlayer)
{
	pPlayer->SendMsg(GetId(), 0x284, 0, 0, 0, nullptr);
}

NpcGoodsList* CScriptNpc::FindGoodsList(const char* pszName)
{
	NpcGoodsList* pList = nullptr;
	int namelen = (int)strlen(pszName);
	auto* mc = GetNpcMerchant();
	for (pList = mc->pSellGoodsList; pList != nullptr; pList = pList->pNext)
	{
		if (strcmp(pList->szTemplate.data(), pszName) == 0)
			return pList;
	}
	return nullptr;
}

NpcGoodsList* CScriptNpc::FindGoodsList(ITEM& item)
{
	NpcGoodsList* pList = nullptr;
	std::array<char, 30> szName{};
	o_strncpy(szName.data(), item.baseitem.szName, 14);
	auto* mc = GetNpcMerchant();
	for (pList = mc->pSellGoodsList; pList != nullptr; pList = pList->pNext)
	{
		if (strcmp(szName.data(), pList->szTemplate.data()) == 0)
			return pList;
	}
	return nullptr;
}

BOOL CScriptNpc::AddItem(ITEM& item)
{
	auto* mc = GetNpcMerchant();
	NpcGoodsItemList* pItemList = CNpcManager::GetInstance()->AllocGoodsItemList();
	if (pItemList == nullptr)return FALSE;
	pItemList->item = item;
	pItemList->dwPutTime = CFrameTime::GetFrameTime();
	NpcGoodsList* pList = FindGoodsList(item);
	if (pList && pList->defaultcount == 0)
		return CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
	if (pList == nullptr)
	{
		pList = CNpcManager::GetInstance()->AllocGoodsList();
		if (pList == nullptr)
		{
			CNpcManager::GetInstance()->FreeGoodsItemList(pItemList);
			return FALSE;
		}
		o_strncpy(pList->szTemplate.data(), item.baseitem.szName, 14);
#ifdef	_DEBUG
		PRINT(SUCCESS_GREEN, "szTemplate的名字:%s\n", pList->szTemplate);
#endif
		pList->dwTemplatePrice = ROUND(mc->fBuyPercent * item.baseitem.nPrice);
		pList->refreshtime = 10;
		pList->defaultcount = 1; // 1表示不是NPC本身卖的物品 0表示是买的时候创建的.
		pList->dwLastRefreshTime = pItemList->dwPutTime;
		if (mc->pSellGoodsList)
			pList->pNext = mc->pSellGoodsList;
		mc->pSellGoodsList = pList;
	}
	if (pList->pItemList == nullptr)
		pList->pItemList = pItemList;
	else
	{
		pItemList->pNext = pList->pItemList;
		pList->pItemList = pItemList;
	}
	pList->currentcount++;
	return TRUE;
}

VOID CScriptNpc::SendGoodsList(CHumanPlayer* pPlayer)
{
	std::array<char, 4096> szBuffer{};
	char* p = szBuffer.data();
	int count = 0;
	auto* mc = GetNpcMerchant();
	for (NpcGoodsList* pList = mc->pSellGoodsList; pList != nullptr; pList = pList->pNext)
	{
		if (pList->dwTemplatePrice > 100000 || pList->dwTemplatePrice == 0)
			continue;
		size_t len = strlen(pList->szTemplate.data());
		if (len < 4) continue;
		if (containschar(pList->szTemplate.data())) continue;
		sprintf(p, "%s/%u/%u/%u/", pList->szTemplate.data(), pList->defaultcount != 0, pList->dwTemplatePrice, pList->currentcount);
		p += strlen(p);
		count++;
	}
	*p = 0;
	pPlayer->SendMsg(GetId(), 0x285, count, 0, 0, (LPVOID)szBuffer.data(), (int)(p - szBuffer.data()));
}

bool CScriptNpc::containschar(const std::string& str)
{
	for (char c : str) {
		if (c == '€')
			return true; // 找到非标准字符, 返回true  
	}
	return false; // 未找到非标准字符, 返回false  
}

VOID CScriptNpc::Update()
{
	DWORD dwCurTime = CFrameTime::GetFrameTime();
	auto* mc = GetNpcMerchant();
	if (m_tmrUpdateItem.IsTimeOut(mc->dwTimeOut))
	{
		if (mc->dwTimeOut == 0) mc->dwTimeOut = Getrand(9000) + 1000;
		m_tmrUpdateItem.Savetime();

		NpcGoodsList* pList = mc->pSellGoodsList;
		while (pList)
		{
			if (pList->defaultcount > 1)
			{
				if (pList->currentcount < pList->defaultcount)
				{
					ITEM item;
					if (CItemManager::GetInstance()->MakeupTemplateItem(pList->szTemplate.data(), item))
					{
						int count = pList->defaultcount - pList->currentcount;
						for (int i = 0; i < count; i++)
						{
							CItemManager::GetInstance()->IdentItem(item);
							NpcGoodsItemList* pItemNode = CNpcManager::GetInstance()->AllocGoodsItemList();
							if (pItemNode == nullptr)
							{
								CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
							}
							else
							{
								pItemNode->dwPutTime = CFrameTime::GetFrameTime();
								pItemNode->item = item;
								pItemNode->pNext = pList->pItemList;
								pList->pItemList = pItemNode;
								pList->currentcount++;
								if (!mc->fChanged) mc->fChanged = TRUE;
							}
						}
					}
				}
			}
			pList = pList->pNext;
		}
		if (mc->fChanged)
			SaveItems();
	}
	CAliveObject::Update();
}

BOOL CScriptNpc::BeAttack(CAliveObject* pAttacker, int nDamage)
{
	Say("%s 伤害 %d !", pAttacker->GetName(), nDamage);
	return FALSE;
}

DWORD CScriptNpc::GetItemSellPrice(ITEM& item)
{
	//判断是否此物品可用在此NPC出售 在NPC脚本头部的 +btStdMode 值。例如：+40 这是肉类
	if (this->m_pScriptObject == nullptr || !m_pScriptObject->IsItemTradeble(item.baseitem.btStdMode))
		return 0;
	DWORD n10 = item.baseitem.nPrice;
	if (n10 <= 0) return 0;
	// 需要增加某些物品有纯度的物品
	if (item.baseitem.btStdMode == 40) //肉
	{
		if (item.wMaxDura / item.wCurDura > 2)
		{
			int n20 = ((n10 * item.wCurDura) / item.wMaxDura) / 2;
			n10 = MAX(2, ROUND(n10 - n20));
		}
		else
			n10 = n10 + ROUND((n10 * 2.0 * item.wCurDura) / item.wMaxDura);
		if (n10 > 0)
			return static_cast<DWORD>(GetSellPercent() * n10);
		else
			return 0;
	}
	else if (item.baseitem.btStdMode == 43) //铁矿
	{
		if (item.wMaxDura / item.wCurDura > 2)
		{
			int n20 = ((n10 * item.wCurDura) / item.wMaxDura) / 2;
			n10 = MAX(2, ROUND(n10 - n20));
		}
		else
			n10 = n10 + ROUND((n10 * 1.3 * item.wCurDura) / item.wMaxDura);
		if (n10 > 0)
			return static_cast<DWORD>(GetSellPercent() * n10);
		else
			return 0;
	}
	else if (item.baseitem.btStdMode > 4)
	{
		int n14 = 0;
		int nC = 0;
		while (true)
		{
			//武器根据 0攻击、1魔法、2道术、3幸运、4诅咒、5命中、6攻击速度、7强度 值进行计算价格
			if (item.baseitem.btStdMode == 5 || item.baseitem.btStdMode == 6) // 武器物品
			{
				if (nC != 4) // 4是诅咒
				{
					if (nC == 6)
					{
						if (item.btItemExt[nC] > 10)
							n14 += (item.btItemExt[nC] - 10) * 2;
					}
					else
						n14 += item.btItemExt[nC];
				}
			}
			else
				n14 += item.btItemExt[nC];
			++nC;
			if (nC >= 8) break;
		}
		if (n14 > 0)
			n10 = n10 / 5 * n14;
		if (item.wMaxDura / item.wCurDura > 2)
		{
			int n20 = ((n10 * item.wCurDura) / item.wMaxDura) / 2;
			n10 = MAX(2, ROUND(n10 - n20));
		}
		else
			n10 = n10 + ROUND((n10 * 1.3 * item.wCurDura) / item.wMaxDura);
		return static_cast<DWORD>(GetSellPercent() * n10);
	}
	else
		return static_cast<DWORD>(ROUND(GetSellPercent() * item.baseitem.nPrice)); //	使用标准的物品价格计算公式~
}

DWORD CScriptNpc::GetItemBuyPrice(ITEM& item)
{
	std::array<char, 32> szName{};
	o_strncpy(szName.data(), item.baseitem.szName, item.baseitem.btNameLength);
	NpcGoodsList* pList = FindGoodsList(szName.data());
	if (pList)
		return ROUND(pList->dwTemplatePrice);
	else
		return ROUND(GetBuyPercent() * item.baseitem.nPrice);
}

DWORD CScriptNpc::GetItemRepairPrice(CHumanPlayer* pPlayer, ITEM& item)
{
	if (CItemManager::GetInstance()->ItemLimited(item, IL_NOREPAIR))return 0xffffffff;
	if (m_pScriptObject == nullptr)return 0xffffffff;
	if (!m_pScriptObject->IsItemTradeble(item.baseitem.btStdMode))
		return 0xffffffff;
	DWORD dwMoney = GetItemBuyPrice(item);
	if (item.baseitem.wMaxDura == 0)
		item.baseitem.wMaxDura = item.wMaxDura;
	if (item.wMaxDura == 0)
		return 0xffffffff;
	if (item.wCurDura > item.wMaxDura)
		return 0xffffffff;
	FLOAT frate = (FLOAT)(item.wMaxDura - item.wCurDura) / item.baseitem.wMaxDura;
	if (pPlayer->IsSystemFlagSeted(SF_SPECIALREPAIR))
		dwMoney = ROUND(dwMoney * frate);
	else
		dwMoney = ROUND(dwMoney * frate) / 2;
	return dwMoney;
}

BOOL CScriptNpc::RepairItem(CHumanPlayer* pPlayer, ITEM& item)
{
	if (m_pScriptObject == nullptr || !m_pScriptObject->IsItemTradeble(item.baseitem.btStdMode))
		return FALSE;
	DWORD dwMoney = GetItemRepairPrice(pPlayer, item);
	if (dwMoney == 0xffffffff || dwMoney == 0)return FALSE;
	if (dwMoney > pPlayer->GetMoney(MT_GOLD))
	{
		pPlayer->SaySystem("钱不够,无法修理!");
		return FALSE;
	}
	pPlayer->CostMoney(MT_GOLD, dwMoney, FALSE);
	if (this->IsSandCityMerchant())
	{
		CSandCity::GetInstance()->AddIncoming(dwMoney);
	}
	if (!pPlayer->IsSystemFlagSeted(SF_SPECIALREPAIR))
	{
		FLOAT frate = (FLOAT)(item.wMaxDura - item.wCurDura) / item.wMaxDura;
		WORD wDecDura = static_cast<WORD>(1000 * frate);
		if (wDecDura > item.wMaxDura)item.wMaxDura = 1;
		item.wMaxDura -= wDecDura;
	}
	item.wCurDura = item.wMaxDura;
	CItemManager::GetInstance()->AddItemModifyFlag(item, ITEMMODIFY_DURACHANGED);
	pPlayer->SendMsg(pPlayer->GetMoney(MT_GOLD), SM_REPAIROK, item.wCurDura, item.wMaxDura, 0);
	return TRUE;
}

BOOL CScriptNpc::SellItem(CHumanPlayer* pPlayer, ITEM& item)
{
	ITEM bitem = item;
	DWORD dwPrice = GetItemSellPrice(item);
	if (item.IsBind())
	{
		pPlayer->SaySystem("该物品已绑定不能被卖出!");
		return FALSE;
	}
	if (CItemManager::GetInstance()->ItemLimited(item, IL_NOSELL))
	{
		pPlayer->SaySystem("该物品不能被卖出!");
		return FALSE;
	}
	if (m_pScriptObject == nullptr || !m_pScriptObject->IsItemTradeble(item.baseitem.btStdMode))
	{
		pPlayer->SaySystem("本店不收该物品!");
		return FALSE;
	}
	if (this->IsSandCityMerchant() && pPlayer->GetGuild() != nullptr && pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
	{
		dwPrice = ROUND(dwPrice * ((FLOAT)CSandCity::GetInstance()->GetTexRate() / 100));
	}
	if (pPlayer->TestAddMoney(MT_GOLD, dwPrice))
	{
		//	删除背包物品
		if (pPlayer->DeleteBagItem(bitem.dwMakeIndex))
		{
			//	把物品加入到NPC的物品表中
			if (AddItem(bitem))
			{
				CItemManager::GetInstance()->UpdateItemOwner(0, bitem.dwMakeIndex, IDF_NPC, 0);
				pPlayer->AddGold(dwPrice, FALSE);
				auto* mc = GetNpcMerchant();
				mc->fChanged = TRUE;
				return TRUE;
			}
			else
				pPlayer->AddBagItem(bitem, TRUE, FALSE, FALSE);
		}
		else
			pPlayer->SaySystem("该物品比较特殊, 无法卖掉!");
	}
	else
		pPlayer->SaySystem("您无法再携带更多的钱了!");
	return FALSE;
}

BOOL CScriptNpc::QueryItemList(CHumanPlayer* pPlayer, const char* pszItemName, int ptr)
{
	NpcGoodsList* pList = FindGoodsList(pszItemName);
	if (pList == nullptr || pList->defaultcount == 0)return FALSE;
	NpcGoodsItemList* pItemList = pList->pItemList;
	if (pItemList == nullptr)return FALSE;
	int count = 0;
	ITEMCLIENT items[10];
	int scount = 0;
	if (ptr >= pList->currentcount)
		ptr = 0;
	while (pItemList)
	{
		count++;
		if (count > ptr)
		{
			items[scount] = *(ITEMCLIENT*)&pItemList->item;
			items[scount].baseitem.nPrice = GetItemValue(pItemList->item, pList->dwTemplatePrice);
			scount++;
			if (scount >= 10)
				break;
		}
		pItemList = pItemList->pNext;
	}
	if (scount == 0)return FALSE;
	pPlayer->SendMsg(GetId(), 0x28c, scount, ptr, 0, (LPVOID)items, sizeof(ITEMCLIENT) * scount);
	return TRUE;
}

DWORD CScriptNpc::GetItemValue(ITEM& item, DWORD dwBasePrice)
{
	if (item.baseitem.wMaxDura == 0)return dwBasePrice;
	if (item.wMaxDura == 0)return 0;
	dwBasePrice = (item.wMaxDura == item.baseitem.wMaxDura) ? dwBasePrice : ROUND(dwBasePrice * ((FLOAT)item.wMaxDura / item.baseitem.wMaxDura));
	dwBasePrice = (item.wCurDura == item.wMaxDura) ? dwBasePrice : ROUND(dwBasePrice * ((FLOAT)item.wCurDura / item.wMaxDura));
	return dwBasePrice;
}

BOOL CScriptNpc::BuyItem(CHumanPlayer* pPlayer, const char* pszName, DWORD dwMakeIndex, DWORD& dwErrorCode)
{
	CItemBox& box = pPlayer->GetBag();
	dwErrorCode = 0;
	if (box.GetFree() <= 0)
	{
		pPlayer->SaySystem( "背包满了, 无法购买物品!" );
		dwErrorCode = 2;
		return FALSE;
	}
	else
	{
		ITEM item;
		NpcGoodsList* pList = FindGoodsList(pszName);
		if (pList == nullptr)return FALSE;
		if (pList->defaultcount == 0)
		{
			if (CItemManager::GetInstance()->MakeupTemplateItem(pszName, item))
			{
				DWORD dwPrice = GetItemValue(item, pList->dwTemplatePrice);
				if (this->IsSandCityMerchant() && pPlayer->GetGuild() != nullptr && pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
				{
					dwPrice = ROUND(dwPrice * ((FLOAT)CSandCity::GetInstance()->GetRebate() / 100));
				}
				if (dwPrice > pPlayer->GetMoney(MT_GOLD))
				{
					pPlayer->SaySystem( "对不起, 您没有足够的钱来购买这个物品!" );
					dwErrorCode = 3;
					return FALSE;
				}
				else
				{
					if (!pPlayer->CanBearItem(item))
					{
						pPlayer->SaySystem( "对不起, 您的负重已经满了, 无法再购买更多的东西!" );
						dwErrorCode = 2;
						return FALSE;
					}
					CItemManager::GetInstance()->IdentItem(item);
					if (!pPlayer->AddBagItem(item, FALSE, TRUE))
					{
						pPlayer->SaySystem( "对不起, 您的背包无法装更多的东西了!" );
						CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
						dwErrorCode = 2;
						return FALSE;
					}
					pPlayer->CostMoney(MT_GOLD, dwPrice, TRUE);
					if (this->IsSandCityMerchant())
					{
						CSandCity::GetInstance()->AddIncoming(dwPrice);
					}
					return TRUE;
				}
			}
			else
			{
				pPlayer->SaySystem( "对不起, 该物品是样品, 尚无存货!" );
				dwErrorCode = 0;
			}
		}
		else
		{
			NpcGoodsItemList* pItemList = pList->pItemList;
			NpcGoodsItemList* pLastItemList = pItemList;
			while (pItemList)
			{
				if (pItemList->item.dwMakeIndex == dwMakeIndex)
				{
					break;
				}
				pLastItemList = pItemList;
				pItemList = pItemList->pNext;
			}
			if (pItemList == nullptr)
			{
				dwErrorCode = 1;
				return FALSE;
			}
			if (pItemList != nullptr)
			{
				if (!pPlayer->CanBearItem(pItemList->item))
				{
					pPlayer->SaySystem( "对不起, 您的负重已经满了, 无法再购买更多的东西!" );
					dwErrorCode = 2;
					return FALSE;
				}
				DWORD dwPrice = GetItemValue(pItemList->item, pList->dwTemplatePrice);
				if (this->IsSandCityMerchant() && pPlayer->GetGuild() != nullptr && pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
				{
					dwPrice = ROUND(dwPrice * ((FLOAT)CSandCity::GetInstance()->GetRebate() / 100));
				}
				if (dwPrice > pPlayer->GetMoney(MT_GOLD))
				{
					pPlayer->SaySystem( "对不起, 您没有足够的钱来购买这个物品!" );
					dwErrorCode = 3;
					return FALSE;
				}
				else
				{
					if (!pPlayer->AddBagItem(pItemList->item, FALSE, TRUE))
					{
						pPlayer->SaySystem( "对不起, 您的背包无法装更多的东西了!" );
						dwErrorCode = 2;
						return FALSE;
					}
					pPlayer->CostMoney(MT_GOLD, dwPrice, FALSE);
					if (this->IsSandCityMerchant())
					{
						CSandCity::GetInstance()->AddIncoming(dwPrice);
					}
					DeleteNpcGoodsItemList(pList, pItemList);
					if (pList->pItemList == nullptr)
					{
						//	如果是玩家卖进去的, 就删除这个结点～～
						if (pList->defaultcount == 1)
						{
							DeleteNpcGoodsList(pList);
						}
						else
						{
							ITEM item;
							if (CItemManager::GetInstance()->MakeupTemplateItem(pList->szTemplate.data(), item))
							{
								for (int i = 0; i < pList->defaultcount; i++)
								{
									CItemManager::GetInstance()->IdentItem(item);
									NpcGoodsItemList* pItemNode = CNpcManager::GetInstance()->AllocGoodsItemList();
									if (pItemNode == nullptr)
									{
										CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
									}
									else
									{
										pItemNode->dwPutTime = CFrameTime::GetFrameTime();
										pItemNode->item = item;
										pItemNode->pNext = pList->pItemList;
										pList->pItemList = pItemNode;
										pList->currentcount++;
									}
								}
							}
						}
						pPlayer->SendMsg(GetId(), 0x28c, 0, 0, 0, nullptr, 0);
					}
					auto* mc = GetNpcMerchant();
					mc->fChanged = TRUE;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

VOID CScriptNpc::DeleteNpcGoodsList(NpcGoodsList* pList)
{
	auto* mc = GetNpcMerchant();
	NpcGoodsList* pList1 = nullptr;
	if (pList == mc->pSellGoodsList)
	{
		mc->pSellGoodsList = pList->pNext;
		CNpcManager::GetInstance()->FreeGoodsList(pList);
		return;
	}
	for (pList1 = mc->pSellGoodsList; pList1 != nullptr; pList1 = pList1->pNext)
	{
		if (pList1->pNext == pList)
		{
			pList1->pNext = pList->pNext;
			CNpcManager::GetInstance()->FreeGoodsList(pList);
			return;
		}
	}
}

VOID CScriptNpc::DeleteNpcGoodsItemList(NpcGoodsList* pList, NpcGoodsItemList* pItemList)
{
	if (pItemList == pList->pItemList)
	{
		pList->pItemList = pItemList->pNext;
		CNpcManager::GetInstance()->FreeGoodsItemList(pItemList);
		pList->currentcount--;
		return;
	}
	NpcGoodsItemList* pItemList1 = nullptr;
	for (pItemList1 = pList->pItemList; pItemList1 != nullptr; pItemList1 = pItemList1->pNext)
	{
		if (pItemList1->pNext == pItemList)
		{
			pItemList1->pNext = pItemList->pNext;
			CNpcManager::GetInstance()->FreeGoodsItemList(pItemList);
			pList->currentcount--;
			return;
		}
	}
}

VOID CScriptNpc::SaveItems()
{
	auto* mc = GetNpcMerchant();
	if (!mc->fChanged) return;
	char szFilename[1024];
	sprintf(szFilename, ".\\Data\\Market_Save\\Market_%08x.dat", GetStoreId());
	FILE* fp = fopen(szFilename, "wb");
	if (fp == nullptr)
	{
		PRINT(ERROR_RED, "存储 %s 商店信息出错~\n", szFilename);
		return;
	}
	xPacketPool::ScopedPacket packet(65536);
	NpcGoodsList* pList = mc->pSellGoodsList;
	while (pList)
	{
		if (pList->defaultcount != 0)
		{
			NpcGoodsItemList* pItemList = pList->pItemList;
			while (pItemList)
			{
				if (!packet->push((LPVOID)&pItemList->item, sizeof(ITEM)))
				{
					fwrite((LPVOID)packet->getbuf(), packet->getsize(), 1, fp);
					packet->clear();
					packet->push((LPVOID)&pItemList->item, sizeof(ITEM));
				}
				pItemList = pItemList->pNext;
			}
		}
		pList = pList->pNext;
	}
	if (packet->getsize() > 0)
		fwrite((LPVOID)packet->getbuf(), packet->getsize(), 1, fp);
	fclose(fp);
	mc->fChanged = FALSE;
}

static ITEM	items_t[100];
VOID CScriptNpc::LoadItems()
{
	char szFilename[1024] = { 0 };
	//market_%08x.dat其中%08x表示以十六进制形式输出一个 8 位宽度的数值, 
	//sprintf( szFilename, ".\\data\\Market_Save\\market_%08x.dat", m_StoreId );
	//PRINT(ERROR_RED, "读取 %s 商店信息~\n", szFilename);
	FILE* fp = fopen(szFilename, "rb");
	if (fp == nullptr) return;
	int size = _GetFileSize(fp);
	size /= sizeof(ITEM);
	while (size > 100)
	{
		size -= 100;
		fread((VOID*)items_t, sizeof(ITEM) * 100, 1, fp);
		for (int i = 0; i < 100; i++)
		{
			if (CItemManager::GetInstance()->IsTempItem(items_t[i].dwMakeIndex))
				CItemManager::GetInstance()->IdentItem(items_t[i]);
			AddItem(items_t[i]);
		}
	}
	if (size > 0)
	{
		fread((VOID*)items_t, sizeof(ITEM) * size, 1, fp);
		for (int i = 0; i < size; i++)
		{
			if (CItemManager::GetInstance()->IsTempItem(items_t[i].dwMakeIndex))
				CItemManager::GetInstance()->IdentItem(items_t[i]);
			AddItem(items_t[i]);
		}
	}
	fclose(fp);
}

VOID CScriptNpc::OnEnterMap(CLogicMap* pMap)
{
	CAliveObject::OnEnterMap(pMap);
}

using namespace rapidjson;
VOID CScriptNpc::SendMerChantJsonMsg(CScriptTarget* pTarget, const char* pWords, UINT nType)
{
	CScriptTargetForPlayer* pPTarget = (CScriptTargetForPlayer*)pTarget;
	if (!pPTarget) return;
	CHumanPlayer* pPlayer = pPTarget->GetOwner();
	if (!pPlayer) return;
	switch (nType)
	{
	case 0:
		SendDayExpMain(pPlayer, pWords); // 每日经验-主界面
	break;
	case 1:
		SendDayExpHelp(pPlayer, pWords); // 每日经验-帮助
	break;
	case 2:
		SendActivityMain(pPlayer, pWords); // 活动主页
	break;
	case 3:
		SendCreateGuildHelp(pPlayer, pWords); //创建行会帮助
	break;
	case 100:
		SendCustomUIWnd(pPlayer, pWords); //发送自定义UI消息
	}
}

VOID CScriptNpc::SendDayExpMain(CHumanPlayer* pPlayer, const char* pWords)
{
	Document doc;
	doc.Parse(pWords);
	if (doc.HasParseError()) return;

	xPacketPool::ScopedPacket packet;
	const char* s1C = "expback2020";
	packet->push(s1C);
	packet->push(2);
	int nValue = 0x01;
	packet->push((LPVOID)&nValue, 4);
	packet->push(doc["注意事项"].GetString());
	packet->push(1);
	nValue = 0x07;
	packet->push((LPVOID)&nValue, 4);
	nValue = doc["今日可以获得经验下限"].GetInt();
	packet->push((LPVOID)&nValue, 4);
	nValue = doc["今日可以获得经验上限"].GetInt();
	packet->push((LPVOID)&nValue, 4);
	nValue = doc["今日可追赶经验"].GetInt();
	packet->push((LPVOID)&nValue, 4);
	nValue = doc["累积可追赶经验"].GetInt();
	packet->push((LPVOID)&nValue, 4);
	packet->push(8);
	nValue = doc["开服天数"].GetInt();
	packet->push((LPVOID)&nValue, 4);
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
}

VOID CScriptNpc::SendDayExpHelp(CHumanPlayer* pPlayer, const char* pWords)
{
	Document doc;
	doc.Parse(pWords);
	if (doc.HasParseError()) return;

	xPacketPool::ScopedPacket packet;
	const char* s1C = "expback2020";
	packet->push(s1C);
	packet->push(1);
	int nValue = 0x02;
	packet->push((LPVOID)&nValue, 1);
	nValue = 0x01;
	packet->push((LPVOID)&nValue, 4);
	packet->push(doc["帮助说明"].GetString());
	packet->push(1);
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
}

VOID CScriptNpc::SendActivityMain(CHumanPlayer* pPlayer, const char* pWords)
{
	Document doc;
	doc.Parse(pWords);
	if (doc.HasParseError()) return;

	xPacketPool::ScopedPacket packet(65535);
	int nValue = 0x00;
	packet->push((LPVOID)&nValue, 2);

	int nDayActivity = doc["今日活跃度"].GetInt();
	packet->push((LPVOID)&nDayActivity, 4);

	int nWeekActivity = doc["本周累计活跃度"].GetInt();
	packet->push((LPVOID)&nWeekActivity, 4);

	int nBoxCount = doc["宝箱"].Size();
	packet->push((LPVOID)&nBoxCount, 4);

	for (int i = 0; i < nBoxCount; i++)
	{
		const rapidjson::Value& box = doc["宝箱"][i];
		const char* szReward = box["奖励"].GetString();
		packet->push(szReward);
		packet->push(1);
	}
	packet->push(4);
	packet->push((LPVOID)&nBoxCount, 4);
	for (int i = 0; i < nBoxCount; i++)
	{
		const rapidjson::Value& box = doc["宝箱"][i];
		int nStatus = box["状态"].GetInt();
		packet->push((LPVOID)&nStatus, 4);
	}
	packet->push((LPVOID)&nBoxCount, 4);
	for (int i = 0; i < nBoxCount; i++)
	{
		const rapidjson::Value& box = doc["宝箱"][i];
		int nRequire = box["需求"].GetInt();
		packet->push((LPVOID)&nRequire, 4);
	}

	int nTaskCount = doc["任务"].Size();
	packet->push((LPVOID)&nTaskCount, 4);

	for (int i = 0; i < nTaskCount; i++)
	{
		const rapidjson::Value& task = doc["任务"][i];
		const char* szName = task["名称"].GetString();
		packet->push(szName);
		packet->push(1);
	}
	packet->push((LPVOID)&nTaskCount, 4);
	for (int i = 0; i < nTaskCount; i++)
	{
		const rapidjson::Value& task = doc["任务"][i];
		int nCount = task["次数"].GetInt();
		packet->push((LPVOID)&nCount, 4);
	}

	pPlayer->SendMsg(pPlayer->GetId(), 0x836, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
}

VOID CScriptNpc::SendCreateGuildHelp(CHumanPlayer* pPlayer, const char* pWords)
{
	xPacketPool::ScopedPacket packet;
	const char* s1C = "guildmgr";
	packet->push(s1C);
	packet->push(1);
	packet->push(1);
	int nValue = 0x01;
	packet->push((LPVOID)&nValue, 4);
	packet->push(pWords);
	packet->push(21);
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
}

VOID CScriptNpc::SendCustomUIWnd(CHumanPlayer* pPlayer, const char* pWords)
{
	Document doc;
	doc.Parse(pWords);
	if (doc.HasParseError()) return;

	xPacketPool::ScopedPacket packet(65535);
	const char* pszWndName = doc["WndName"].GetString();
	packet->push(pszWndName);
	packet->push(1);
	int pMsgType = doc["MsgType"].GetInt();
	packet->push((LPVOID)&pMsgType, 1);

	int nListStrCount = doc.HasMember("StrList") ? doc["StrList"].Size() : 0;
	packet->push((LPVOID)&nListStrCount, 4);
	std::array<char, 512> szFinal{};
	for (int i = 0; i < nListStrCount; i++)
	{
		const rapidjson::Value& task = doc["StrList"][i];
		const char* pszStr = task.GetString();
		ProcFmtText(pszStr, szFinal.data(), 512, pPlayer->GetScriptTarget());
		packet->push(szFinal.data());
		packet->push(1);
	}

	int nListIntCount = doc.HasMember("NumList") ? doc["NumList"].Size() : 0;
	packet->push((LPVOID)&nListIntCount, 4);
	for (int i = 0; i < nListIntCount; i++)
	{
		const rapidjson::Value& task = doc["NumList"][i];
		int pInt = task.GetInt();
		packet->push(&pInt, 4);
	}
	packet->push(12);
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
}