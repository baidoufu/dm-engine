#include "StdAfx.h"
#include ".\itemmanager.h"
#include "server.h"
#include "localsupport.h"
#include "GameWorld.h"

CItemManager::CItemManager(VOID)
{
	m_nTempItemCount = 0;
	m_nCreateItemCount = 0;
	m_nDeleteTempItemCount = 0;
	m_nIdentCount = 0;
}

CItemManager::~CItemManager(VOID)
{
}

VOID CItemManager::Load(const char* pszfile)
{
	CStringFile sfItemClass(pszfile);
	for (int i = 1; i < sfItemClass.GetLineCount(); i++)
	{
		AddItemClassString(sfItemClass[i]);
	}
}

VOID CItemManager::ClearItemData()
{
	// 先清理 hash 中的引用
	m_xItemClassPool.forEach([this](ITEMCLASS* pObject) {
		if (pObject != nullptr)
		{
			char szFullName[32];
			o_strncpy(szFullName, pObject->szFullName, 16);
			m_ItemClassHash.HDel(szFullName);
		}
		return true; // 继续遍历
	});
	// 清空对象池
	m_xItemClassPool.clearAll([](ITEMCLASS*) {}); // 不需要额外清理
	ClearStringPool();
}

WORD CItemManager::AddStringToPool(const char* pszString)
{
	if (pszString == nullptr || *pszString == '\0') return 0;

	WORD wLength = (WORD)strlen(pszString);
	WORD wOffset = (WORD)m_vStringData.size();

	m_vStringData.insert(m_vStringData.end(), pszString, pszString + wLength);
	m_vStringData.push_back('\0');

	m_vStringIndex.emplace_back();
	auto& entry = m_vStringIndex.back();
	entry.wOffset = wOffset;
	entry.wLength = wLength;

	return (WORD)m_vStringIndex.size(); // 返回ID（从1开始）
}

const char* CItemManager::GetStringFromPool(WORD wStringId)
{
	if (wStringId == 0 || wStringId > m_vStringIndex.size())
		return "";

	const StringPoolEntry& entry = m_vStringIndex[wStringId - 1];
	return &m_vStringData[entry.wOffset];
}

VOID CItemManager::ClearStringPool()
{
	m_vStringIndex.clear();
	m_vStringData.clear();
}

VOID CItemManager::LoadLimit(const char* pszfile)
{
	CStringFile sf(pszfile);
	char* pLine = nullptr;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		WORD wLimit = 0;
		xStringsExtracter<40> limit(pLine, "=|", " \t");
		if (limit.getCount() <= 1)continue;
		ITEMCLASS* pClass = GetItemClassByName(limit[0]);
		if (pClass == nullptr)continue;
		for (UINT n = 0; n < limit.getCount() - 1; n++)
		{
			item_limit l = GetItemLimit(limit[n + 1]);
			if (l == IL_MAX)continue;
			wLimit |= (1 << (int)l);
		}
		pClass->wLimit = wLimit;
	}
}

VOID CItemManager::LoadScriptLink(const char* pszfile)
{
	CStringFile sf(pszfile);
	char* pLine = nullptr;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		xStringsExtracter<2> ss(pLine, "=", " \t");
		if (ss.getCount() < 2)continue;
		ITEMCLASS* pClass = GetItemClassByName(ss[0]);
		if (pClass == nullptr)continue;
		xStringsExtracter<3> sss(ss[1], ",", " \t");
		if (sss.getCount() < 1)continue;
		if (sss.getCount() > 0 && sss[0][0] != '\0') // 捡物品脚本
		{
			pClass->wPickupPageId = AddStringToPool(sss[0]);
			PRINT(WARN_YELLOW, "物品 %s 加载捡物品脚本: %s\n", ss[0], sss[0]);
		}
		if (sss.getCount() > 1 && sss[1][0] != '\0') // 扔物品脚本
		{
			xStringsExtracter<3> bb(sss[1], "|", " \t");

			if (bb.getCount() == 1)
			{
				pClass->wDropPageId = AddStringToPool(bb[0]);
				pClass->dwDropPageDelay = 0;
				pClass->dwDropPageExecuteTimes = 1;
				PRINT(WARN_YELLOW, "物品 %s 加载扔物品脚本: %s\n", ss[0], bb[0]);
			}
			else if (bb.getCount() == 2)
			{
				pClass->wDropPageId = AddStringToPool(bb[1]);
				pClass->dwDropPageDelay = (DWORD)StringToInteger(bb[0]);
				pClass->dwDropPageExecuteTimes = 1;
				PRINT(WARN_YELLOW, "物品 %s 加载扔物品脚本: %s, 时间间隔%s秒\n", ss[0], bb[1], bb[0]);
			}
			else if (bb.getCount() > 2)
			{
				pClass->wDropPageId = AddStringToPool(bb[2]);
				pClass->dwDropPageDelay = (DWORD)StringToInteger(bb[1]);
				pClass->dwDropPageExecuteTimes = (DWORD)StringToInteger(bb[0]);
				PRINT(WARN_YELLOW, "物品 %s 加载扔物品脚本: %s, 时间间隔%s秒, 执行%s次\n", ss[0], bb[2], bb[1], bb[0]);
			}
		}
		if (sss.getCount() > 2 && sss[2][0] != '\0') // 使用脚本
		{
			pClass->wPageId = AddStringToPool(sss[2]);
			PRINT(WARN_YELLOW, "物品 %s 加载使用脚本: %s\n", ss[0], sss[2]);
		}
	}
}

VOID CItemManager::LoadPetINI(const char* pszfile)
{
	CSettingFile sfGuild;
	if (!sfGuild.Open(pszfile)) return;
	PetInfolist.clear();
	for (int i = 0; i < 3; i++)
	{
		m_PetInfo info;
		char sectionName[64];
		sprintf(sectionName, "PetExp%d", i);
		info.lv0 = sfGuild.GetInteger(sectionName, "lv0", 0);
		info.lv1 = sfGuild.GetInteger(sectionName, "lv1", 0);
		info.lv2 = sfGuild.GetInteger(sectionName, "lv2", 0);
		info.lv3 = sfGuild.GetInteger(sectionName, "lv3", 0);
		info.lv4 = sfGuild.GetInteger(sectionName, "lv4", 0);
		info.lv5 = sfGuild.GetInteger(sectionName, "lv5", 0);
		info.lv6 = sfGuild.GetInteger(sectionName, "lv6", 0);
		info.lv7 = sfGuild.GetInteger(sectionName, "lv7", 0);
		PetInfolist.push_back(info);
	}
	sfGuild.Close();
}

BOOL CItemManager::ItemLimited(ITEM& item, item_limit limit)
{
	WORD	wFlag = (1 << (int)limit);
	UPGRADEADDMASK mask;
	mask.dwValue = item.dwParam[0];
	if (wFlag & mask.wItemLimit)
		return TRUE;
	return FALSE;
}

VOID CItemManager::AddItemModifyFlag(ITEM& item, BYTE btFlag)
{
	item.baseitem.wFeature = (item.baseitem.wFeature & 0xff0f) | (btFlag & 0xf0);
}

BOOL CItemManager::UpdateItemPos(BYTE btFlag, BAGITEMPOS* pItemPos, WORD wCount)
{
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);

	if (pObj != nullptr)
	{
		pObj->SendUpdateItemPos(btFlag, pItemPos, wCount);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::UpdateItems(DWORD dwOwnerId, BYTE btFlag, DBITEM* pItemArray, int count)
{
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);

	if (pObj != nullptr)
	{
		pObj->SendUpdateItems(dwOwnerId, btFlag, pItemArray, count);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::CreateItem(const char* pszName, DWORD dwId, DWORD dwKey, DWORD dwOwner, WORD wPos, BYTE btFlag, BOOL IsBind, BOOL bRandomUpgrade)
{
	CClientObj* pObj = (CClientObj*)CServer::GetInstance()->GetClientObject(dwId);
	ITEM item;
	if (pObj && CreateTempItem(pszName, item))
	{
		m_nCreateItemCount++;
		if (IsBind)
			item.SetBind(TRUE);
		pObj->OnCreateItem(item, wPos, btFlag);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::CreateTempItem(const char* pszName, ITEM& item, BOOL bRandomUpgrade, int ratefix)
{
	if (bRandomUpgrade)
	{
		if (!MakeupRandomUpgradeItemTemplate(pszName, item, ratefix))return FALSE;
	}
	else if (!MakeupTemplateItem(pszName, item))return FALSE;
	m_nTempItemCount++;
	item.dwMakeIndex = (m_xTempItemIdAllocor.allocid() | 0x80000000);
	return TRUE;
}

VOID CItemManager::IdentItem(ITEM& item)
{
	m_nIdentCount++;
	item.dwMakeIndex = (m_xTempItemIdAllocor.allocid() | 0x80000000);
}

BOOL CItemManager::UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos)
{
	if (IsTempItem(dwItemIndex))return TRUE;
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);
	ITEM item;
	if (pObj != nullptr)
	{
		pObj->SendUpdateItemPos(dwItemIndex, btFlag, wPos);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::UpdateItemOwner(DWORD dwOwnerId, DWORD dwItemIndex, BYTE	btFlag, WORD wPos)
{
	if (IsTempItem(dwItemIndex))return TRUE;
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);
	ITEM item;
	if (pObj != nullptr)
	{
		pObj->SendUpdateItemOwner(dwItemIndex, dwOwnerId, btFlag, wPos);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::DeleteItem(DWORD dwMakeIndex)
{
	if (dwMakeIndex == 0)
	{
		return FALSE;
	}
	if (dwMakeIndex & 0x80000000)
	{
		m_nDeleteTempItemCount++;
		m_xTempItemIdAllocor.freeid(dwMakeIndex & 0x7fffffff);
		return TRUE;
	}

	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);
	if (pObj != nullptr)
	{
		pObj->SendDeleteItem(dwMakeIndex, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemManager::AddItemClassString(char* pszItemClassDesc)
{
	char* Params[28];
	int nParam = SearchParam(pszItemClassDesc, Params, 28, ",");
	if (*Params[0] == '#') return TRUE;
	if (nParam < 22)
	{
		m_strErrorMsg = "BaseItem.csv物品库数量不对";
		return FALSE;
	}
	ITEMCLASS	itemclass;
	ITEMCLASS* pClass = &itemclass;//new ITEMCLASS;
	//if (strcmp(Params[0], "马牌1") == 0)
	//	PRINT(ERROR_RED, pszItemClassDesc);
	o_strncpy(pClass->szFullName, Params[0], 16);
	FilterItemShowName(Params[0]);
	o_strncpy(pClass->szName, Params[0], 14);
	pClass->stdMode = (BYTE)StringToInteger(Params[1]);
	pClass->btShape = (BYTE)StringToInteger(Params[2]);
	pClass->wImage = (WORD)StringToInteger(Params[3]);
	pClass->specialpower = (char)StringToInteger(Params[4]);
	pClass->ac[0] = (BYTE)StringToInteger(Params[5]);
	pClass->ac[1] = (BYTE)StringToInteger(Params[6]);
	pClass->mac[0] = (BYTE)StringToInteger(Params[7]);
	pClass->mac[1] = (BYTE)StringToInteger(Params[8]);
	pClass->dc[0] = (BYTE)StringToInteger(Params[9]);
	pClass->dc[1] = (BYTE)StringToInteger(Params[10]);
	pClass->mc[0] = (BYTE)StringToInteger(Params[11]);
	pClass->mc[1] = (BYTE)StringToInteger(Params[12]);
	pClass->sc[0] = (BYTE)StringToInteger(Params[13]);
	pClass->sc[1] = (BYTE)StringToInteger(Params[14]);
	pClass->btMagicNicety = (BYTE)StringToInteger(Params[15]);
	pClass->btPoisonNicety = (BYTE)StringToInteger(Params[16]);
	pClass->btWeight = (BYTE)StringToInteger(Params[17]);

	char* pstar = strchr(Params[18], '*');
	if (pstar)
	{
		*pstar++ = 0;
		pClass->duratimes = (WORD)StringToInteger(pstar);
		if (pClass->duratimes == 0)
			pClass->duratimes = 1000;
	}
	else
		pClass->duratimes = 1000;
	pClass->maxdura = (WORD)StringToInteger(Params[18]);
	pClass->nPrice = StringToInteger(Params[19]);
	pClass->needtype = (BYTE)StringToInteger(Params[20]);
	pClass->needlevel = (BYTE)StringToInteger(Params[21]);
	pClass->wPageId = 0;
	if (nParam > 22)
	{
		if (nParam > 22 && *Params[22] == '#')
			pClass->wPageId = AddStringToPool(Params[22] + 1);
		if (nParam > 23 && *Params[23] == '#')
			pClass->wPageId = AddStringToPool(Params[23] + 1);
		if (nParam > 24 && *Params[24] == '#')
			pClass->wPageId = AddStringToPool(Params[24] + 1);
		if (nParam > 25 && *Params[25] == '#')
			pClass->wPageId = AddStringToPool(Params[25] + 1);
		if (nParam > 26 && *Params[26] == '#')
			pClass->wPageId = AddStringToPool(Params[26] + 1);
		if (nParam > 27 && *Params[27] == '#')
			pClass->wPageId = AddStringToPool(Params[27] + 1);
	}
	if (!AddItemClass(pClass))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CItemManager::AddItemClass(ITEMCLASS* pItemclass)
{
	char szFullName[32];
	o_strncpy(szFullName, pItemclass->szFullName, 16); // 使用完整名字作为哈希值

	ITEMCLASS* prItemclass = m_xItemClassPool.newObject();
	*prItemclass = *pItemclass;

	if (!m_ItemClassHash.HAdd(szFullName, (LPVOID)prItemclass))
	{
		m_xItemClassPool.deleteObject(prItemclass);

		ITEMCLASS* p = (ITEMCLASS*)m_ItemClassHash.HGet(szFullName);

		if (p == nullptr)
		{
			m_strErrorMsg = "加入到名字表中失败!";
			return FALSE;
		}

		p->dwDropPageDelay = pItemclass->dwDropPageDelay;
		p->dwDropPageExecuteTimes = pItemclass->dwDropPageExecuteTimes;
		p->wLimit = pItemclass->wLimit;
		p->wPageId = pItemclass->wPageId;
		p->wDropPageId = pItemclass->wDropPageId;
		p->wPickupPageId = pItemclass->wPickupPageId;
		if (memcmp(p, pItemclass, sizeof(ITEMCLASS)) != 0)
		{
			*p = *pItemclass;
			PRINT(WARN_YELLOW, "更新物品类数据 %s\n", szFullName);
		}
	}
	return TRUE;
}

ITEMCLASS* CItemManager::GetItemClassByName(const char* pszName)
{
	return (ITEMCLASS*)m_ItemClassHash.HGet(pszName);
}

BOOL CItemManager::MakeupTemplateItem(const char* pszName, ITEM& item)
{
	ITEMCLASS* pClass = GetItemClassByName(pszName);
	if (pClass == nullptr)
	{
		if (_stricmp(CGameWorld::GetInstance()->GetName(ENI_GOLD), pszName) == 0)
		{
			o_strncpy(item.baseitem.szName, pszName, 14);
			item.baseitem.btNameLength = (BYTE)strlen(pszName);
			if (item.baseitem.btNameLength > 14)item.baseitem.btNameLength = 14;
			item.baseitem.btStdMode = 255;
			item.baseitem.btShape = 0;
			item.baseitem.wImageIndex = 0;
			return TRUE;
		}
		return FALSE;
	}

	memset(&item, 0, sizeof(item));
	o_strncpy(item.baseitem.szName, pClass->szName, 14);
	item.baseitem.btNameLength = (BYTE)strlen(item.baseitem.szName);

	item.baseitem.btStdMode = pClass->stdMode;
	item.baseitem.btShape = pClass->btShape;
	item.baseitem.wImageIndex = pClass->wImage;
	item.baseitem.btWeight = pClass->btWeight;
	item.baseitem.wMaxDura = pClass->stdMode == ISM_BOOK ? pClass->maxdura : (pClass->duratimes * pClass->maxdura);
	if (pClass->stdMode == 10 || pClass->stdMode == 11)
	{
		DWORD dwDressColor = CGameWorld::GetInstance()->GetVar(EVI_INITDRESSCOLOR);
		if(dwDressColor == 0) dwDressColor = static_cast<DWORD>(Getrand(16));
		item.baseitem.wFeature = (item.baseitem.wFeature & 0xfff0) | (dwDressColor & 0xf);
	}
	else if (pClass->stdMode == 5 || pClass->stdMode == 6)
	{
		item.baseitem.btPriceType = pClass->btShape;
	}
	item.baseitem.needtype = pClass->needtype;
	item.baseitem.needvalue = pClass->needlevel;

	item.baseitem.btSpecialpower = (BYTE)pClass->specialpower;

	BYTE pValues[10] = { 0 };
	pValues[0] = pClass->ac[0];  // 最小物理防御
	pValues[1] = pClass->ac[1];  // 最大物理防御
	pValues[2] = pClass->mac[0]; // 最小魔法防御
	pValues[3] = pClass->mac[1]; // 最大魔法防御
	pValues[4] = pClass->dc[0];  // 最小物理攻击
	pValues[5] = pClass->dc[1];  // 最大物理攻击
	pValues[6] = pClass->mc[0];  // 最小魔法攻击
	pValues[7] = pClass->mc[1];  // 最大魔法攻击
	pValues[8] = pClass->sc[0];  // 最小道术攻击
	pValues[9] = pClass->sc[1];  // 最大道术攻击
	memcpy(&item.baseitem.Ac1, pValues, 10);

	item.baseitem.nPrice = pClass->nPrice;

	item.wCurDura = item.wMaxDura = item.baseitem.wMaxDura;

	item.SetLen(); // 设置物品长度

	UPGRADEADDMASK mask;
	mask.dwValue = item.dwParam[0];
	mask.wItemLimit = pClass->wLimit;
	item.dwParam[0] = mask.dwValue; // ItemLimit.txt中配置的限制物品ID
	o_strncpy(item.szFullName, pClass->szFullName, 16);
	SetItemClassPtr(item, pClass);
	return TRUE;
}

BOOL CItemManager::MakeupRandomUpgradeItemTemplate(const char* pszName, ITEM& item, int ratefix)
{
	ITEMCLASS* pClass = GetItemClassByName(pszName);

	if (pClass == nullptr)
	{
		if (_stricmp(CGameWorld::GetInstance()->GetName(ENI_GOLD), pszName) == 0)
		{
			o_strncpy(item.baseitem.szName, pszName, 14);
			item.baseitem.btNameLength = (BYTE)strlen(pszName);
			if (item.baseitem.btNameLength > 14)item.baseitem.btNameLength = 14;
			item.baseitem.btStdMode = 255;
			item.baseitem.btShape = 0;
			item.baseitem.wImageIndex = 0;
			return TRUE;
		}
		return FALSE;
	}

	memset(&item, 0, sizeof(item));
	o_strncpy(item.baseitem.szName, pClass->szName, 14);
	item.baseitem.btNameLength = (BYTE)strlen(item.baseitem.szName);
	item.baseitem.btStdMode = pClass->stdMode;
	item.baseitem.btShape = pClass->btShape;
	item.baseitem.wImageIndex = pClass->wImage;
	item.baseitem.btWeight = pClass->btWeight;
	item.baseitem.wMaxDura = pClass->stdMode == ISM_BOOK ? pClass->maxdura : (pClass->duratimes * pClass->maxdura);

	if (pClass->stdMode == 10 || pClass->stdMode == 11)
	{
		DWORD dwDressColor = Getrand(16);
		item.baseitem.wFeature = (item.baseitem.wFeature & 0xfff0) | (dwDressColor & 0xf);
	}
	else if (pClass->stdMode == 5 || pClass->stdMode == 6)
	{
		item.baseitem.btPriceType = pClass->btShape;
	}
	item.baseitem.needtype = pClass->needtype;
	item.baseitem.needvalue = pClass->needlevel;

	item.baseitem.btSpecialpower = (BYTE)pClass->specialpower;
	
	BYTE pValues[10] = { 0 };
	pValues[0] = pClass->ac[0];  // 最小物理防御
	pValues[1] = pClass->ac[1];  // 最大物理防御
	pValues[2] = pClass->mac[0]; // 最小魔法防御
	pValues[3] = pClass->mac[1]; // 最大魔法防御
	pValues[4] = pClass->dc[0];  // 最小物理攻击
	pValues[5] = pClass->dc[1];  // 最大物理攻击
	pValues[6] = pClass->mc[0];  // 最小魔法攻击
	pValues[7] = pClass->mc[1];  // 最大魔法攻击
	pValues[8] = pClass->sc[0];  // 最小道术攻击
	pValues[9] = pClass->sc[1];  // 最大道术攻击

	WORD wDura = item.baseitem.wMaxDura;
	WORD wMaxDura = wDura;
	GetUpgradeValues(pClass->stdMode, pClass->btShape, ratefix, wDura, wMaxDura, pValues);
	memcpy(&item.baseitem.Ac1, pValues, 10);

	item.baseitem.nPrice = pClass->nPrice;

	item.wMaxDura = wMaxDura;
	item.wCurDura = wDura;

	item.SetLen(); // 设置物品长度

	UPGRADEADDMASK mask;
	mask.dwValue = item.dwParam[0];
	mask.wItemLimit = pClass->wLimit;
	item.dwParam[0] = mask.dwValue; // ItemLimit.txt中配置的限制物品ID
	o_strncpy(item.szFullName, pClass->szFullName, 16);
	SetItemClassPtr(item, pClass);
	return TRUE;
}

static BYTE GetUpgradeValue(int count, int rate)
{
	BYTE btValue = 0;
	for (int i = 0; i < count; i++)
	{
		if (Getrand(rate) == 0)
			btValue++;
		else
			break;
	}
	return btValue;
}

static BYTE GetUpgradeValueEx(BYTE oldValue, int count, int rate)
{
	if (oldValue == 0)
		if (Getrand(10) != 0)return 0;
	if (Getrand(oldValue + 1) == 0)
		return GetUpgradeValue(count, rate);
	return 0;
}

static VOID GetUpgradeDura(int count, int rate, WORD& wDura, WORD& wMaxDura)
{
	WORD wAddDura = GetUpgradeValue(count, rate) * 1000;
	if (wAddDura > 0)
	{
		if (wMaxDura < 64000)
		{
			if (64000 - wMaxDura < wAddDura)
				wMaxDura = 64000;
			else
				wMaxDura += wAddDura;
		}
	}
	wDura = Getrand(wMaxDura / 2) + wMaxDura / 2;
}

VOID CItemManager::GetUpgradeValues(BYTE btStdMode, BYTE btShape, int ratefix, WORD& wDura, WORD& wMaxDura, BYTE* pValues)
{
	//	dc1 dc2 mc1 mc2 sc1 sc2 ac1 ac2 mac1 mac2 strong lucky damn something
	int rate = CGameWorld::GetInstance()->GetVar(EVI_RANDOMUPGRADEITEMRATE);
	if (ratefix > rate / 2)
		rate = rate / 2;
	else
		rate -= ratefix;
	if (rate < 2)rate = 2;

	switch (btStdMode)
	{
	case	22://戒指
	case	26://手镯
	case	10://男衣服
	case	11://女衣服
	case	15://头盔
	case	30://勋章
	case	59://宝石
	case	60:
	case	61:
	case	ISM_SHOES://靴子
	case	ISM_BELT://腰带
	{
		pValues[NPI_AC1] += GetUpgradeValueEx(pValues[NPI_AC1], 5, rate);
		pValues[NPI_AC2] += GetUpgradeValueEx(pValues[NPI_AC2], 30, rate);

		pValues[NPI_MAC1] += GetUpgradeValueEx(pValues[NPI_MAC1], 5, rate);
		pValues[NPI_MAC2] += GetUpgradeValueEx(pValues[NPI_MAC2], 30, rate);

		pValues[NPI_DC1] += GetUpgradeValueEx(pValues[NPI_DC1], 5, rate);
		pValues[NPI_DC2] += GetUpgradeValueEx(pValues[NPI_DC2], 30, rate);

		pValues[NPI_MC1] += GetUpgradeValueEx(pValues[NPI_MC1], 5, rate);
		pValues[NPI_MC2] += GetUpgradeValueEx(pValues[NPI_MC2], 30, rate);

		pValues[NPI_SC1] += GetUpgradeValueEx(pValues[NPI_SC1], 5, rate);
		pValues[NPI_SC2] += GetUpgradeValueEx(pValues[NPI_SC2], 30, rate);
		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;
	case	ISM_WEAPON1://武器2
	case	ISM_WEAPON0://武器1
	{
		pValues[WPI_LUCKY] += GetUpgradeValueEx(pValues[WPI_LUCKY], 7, rate);
		//if( pValues[WPI_LUCKY] == 0 )[WPI_DAWN] += GetUpgradeValueEx( pValues[WPI_DAWN], 3, rate );
		pValues[WPI_HITRATE] += GetUpgradeValueEx(pValues[WPI_HITRATE], 10, rate);

		pValues[WPI_DC1] += GetUpgradeValueEx(pValues[WPI_DC1], 5, rate);
		pValues[WPI_DC2] += GetUpgradeValueEx(pValues[WPI_DC2], 30, rate);

		pValues[WPI_MC1] += GetUpgradeValueEx(pValues[WPI_MC1], 5, rate);
		pValues[WPI_MC2] += GetUpgradeValueEx(pValues[WPI_MC2], 30, rate);

		pValues[WPI_SC1] += GetUpgradeValueEx(pValues[WPI_SC1], 5, rate);
		pValues[WPI_SC2] += GetUpgradeValueEx(pValues[WPI_SC2], 30, rate);

		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;
	case 19://魔法躲避的项链
	{
		//S19_MAGESCAPE,
		//S19_DAWN,
		//S19_LUCKY,

		pValues[S19_LUCKY] += GetUpgradeValueEx(pValues[S19_LUCKY], 7, rate);
		//pValues[S19_DAWN] += GetUpgradeValueEx( pValues[S19_DAWN], 10, rate );
		pValues[S19_MAGESCAPE] += GetUpgradeValueEx(pValues[S19_MAGESCAPE], 9, rate);
		pValues[S19_UNKNOWN] += 0;

		pValues[S19_DC1] += GetUpgradeValueEx(pValues[S19_DC1], 5, rate);
		pValues[S19_DC2] += GetUpgradeValueEx(pValues[S19_DC2], 30, rate);

		pValues[S19_MC1] += GetUpgradeValueEx(pValues[S19_MC1], 5, rate);
		pValues[S19_MC2] += GetUpgradeValueEx(pValues[S19_MC2], 30, rate);

		pValues[S19_SC1] += GetUpgradeValueEx(pValues[S19_SC1], 5, rate);
		pValues[S19_SC2] += GetUpgradeValueEx(pValues[S19_SC2], 30, rate);

		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;
	case	24://手镯
	case	20://项链
	{
		pValues[S20_HITRATE] += GetUpgradeValueEx(pValues[S20_HITRATE], 10, rate);
		pValues[S20_ESCAPE] += GetUpgradeValueEx(pValues[S20_ESCAPE], 10, rate);

		pValues[S20_DC1] += GetUpgradeValueEx(pValues[S20_DC1], 5, rate);
		pValues[S20_DC2] += GetUpgradeValueEx(pValues[S20_DC2], 30, rate);

		pValues[S20_MC1] += GetUpgradeValueEx(pValues[S20_MC1], 5, rate);
		pValues[S20_MC2] += GetUpgradeValueEx(pValues[S20_MC2], 30, rate);

		pValues[S20_SC1] += GetUpgradeValueEx(pValues[S20_SC1], 5, rate);
		pValues[S20_SC2] += GetUpgradeValueEx(pValues[S20_SC2], 30, rate);

		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;
	case	21://攻击、加速的项链
	{
		//S21_ATTACKSPEED,
		//S21_HPRECOVER,
		//S21_ATTACKSPEEDDEC,
		//S21_MPRECOVER,

		pValues[S21_ATTACKSPEED] += GetUpgradeValueEx(pValues[S21_ATTACKSPEED], 10, rate);
		if (pValues[S21_ATTACKSPEED] == 0)pValues[S21_ATTACKSPEEDDEC] += GetUpgradeValueEx(pValues[S21_ATTACKSPEEDDEC], 10, rate);
		pValues[S21_HPRECOVER] += GetUpgradeValueEx(pValues[S21_HPRECOVER], 10, rate);
		pValues[S21_MPRECOVER] += GetUpgradeValueEx(pValues[S21_MPRECOVER], 10, rate);

		pValues[S21_DC1] += GetUpgradeValueEx(pValues[S21_DC1], 5, rate);
		pValues[S21_DC2] += GetUpgradeValueEx(pValues[S21_DC2], 30, rate);

		pValues[S21_MC1] += GetUpgradeValueEx(pValues[S21_MC1], 5, rate);
		pValues[S21_MC2] += GetUpgradeValueEx(pValues[S21_MC2], 30, rate);

		pValues[S21_SC1] += GetUpgradeValueEx(pValues[S21_SC1], 5, rate);
		pValues[S21_SC2] += GetUpgradeValueEx(pValues[S21_SC2], 30, rate);
		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;

	//ac1p = PI_MINAC, ac2p = PI_MAXAC, mac1p = PI_MINMAC, mac2p = PI_MAXMAC;
	//return TRUE;
	case	23://特殊戒指
	{
		//S23_ATTACKSPEED,
		//S23_POISONESCAPE,
		//S23_ATTACKSPEEDDEC,
		//S23_POISONRECOVER,
		pValues[S23_ATTACKSPEED] += GetUpgradeValueEx(pValues[S23_ATTACKSPEED], 10, rate);
		if (pValues[S23_ATTACKSPEED] == 0)pValues[S23_ATTACKSPEEDDEC] += GetUpgradeValueEx(pValues[S23_ATTACKSPEEDDEC], 10, rate);

		pValues[S23_POISONESCAPE] += GetUpgradeValueEx(pValues[S23_POISONESCAPE], 10, rate);
		pValues[S23_POISONRECOVER] += GetUpgradeValueEx(pValues[S23_POISONRECOVER], 10, rate);

		pValues[S23_DC1] += GetUpgradeValueEx(pValues[S23_DC1], 5, rate);
		pValues[S23_DC2] += GetUpgradeValueEx(pValues[S23_DC2], 30, rate);

		pValues[S23_MC1] += GetUpgradeValueEx(pValues[S23_MC1], 5, rate);
		pValues[S23_MC2] += GetUpgradeValueEx(pValues[S23_MC2], 30, rate);

		pValues[S23_SC1] += GetUpgradeValueEx(pValues[S23_SC1], 5, rate);
		pValues[S23_SC2] += GetUpgradeValueEx(pValues[S23_SC2], 30, rate);

		GetUpgradeDura(5, 5, wDura, wMaxDura);
	}
	break;
	case 40: //肉类
	{
		wDura = ratefix * 1000;
	}
	break;
	}
}

BOOL CItemManager::UpgradeItem(ITEM& item)
{
	if (item.dwMakeIndex & 0x80000000)
	{
		return TRUE;
	}
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_ITEMDB);
	if (pObj != nullptr)
	{
		pObj->SendUpgradeItem(item.dwMakeIndex, item.dwParam[0]);
		return TRUE;
	}
	return FALSE;
}

VOID CItemManager::DamageDura(ITEM& item, WORD wDura)
{
	ITEMCLASS* pClass = this->GetItemClassPtr(item);
	if (pClass != 0)
	{
		wDura *= pClass->duratimes;
	}
	if (wDura >= item.wCurDura)
	{
		item.wCurDura = 0;
		item.dwParam[3] = UR_DELETED;
	}
	else
	{ 
		item.wCurDura -= wDura;
		item.dwParam[3] = UR_UPDATED;
	}
	this->AddItemModifyFlag(item, ITEMMODIFY_DURACHANGED);
}

ITEMCLASS* CItemManager::GetItemClass(ITEM& item)
{
	return this->GetItemClassPtr(item);
}

BOOL CItemManager::CheckDura(ITEM& item, WORD wDura)
{
	ITEMCLASS* pClass = GetItemClass(item);
	if (pClass)
		wDura *= pClass->duratimes;
	return (item.wCurDura >= wDura);
}

BOOL CItemManager::UpgradeItem(ITEM& item, const char* pszUpgradeString)
{
	char szTempBuffer[1024];
	o_strncpy(szTempBuffer, pszUpgradeString, 1020);
	xStringsExtracter<50> mstr(szTempBuffer, ",", " \t");
	for (UINT i = 0; i < mstr.getCount(); i++)
	{
		xStringsExtracter<2> exps(mstr[i], "=", " \t");
		if (exps.getCount() < 2)
			continue;
		int opr = 0;

		char* pValue = exps[1];

		if (pValue[0] == '-')
		{
			opr = 1;
			pValue = TrimEx(pValue + 1);
		}
		else if (pValue[0] == '+')
		{
			opr = 2;
			pValue = TrimEx(pValue + 1);
		}
		// 属性修改器映射表
		using ItemModifier = std::function<void(ITEM*, int opr, int val)>;
		static const SmallFlatMap<std::string, ItemModifier, 32> modifiers =
		{
			{"stdmode", [](ITEM* p, int o, int v) { p->baseitem.btStdMode   = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.btStdMode - v : p->baseitem.btStdMode + v); }},
			{"shape",   [](ITEM* p, int o, int v) { p->baseitem.btShape     = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.btShape - v : p->baseitem.btShape + v); }},
			{"weight",  [](ITEM* p, int o, int v) { p->baseitem.btWeight    = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.btWeight - v : p->baseitem.btWeight + v); }},
			{"defdura", [](ITEM* p, int o, int v) { p->baseitem.wMaxDura    = (WORD)(o == 0 ? v : o == 1 ? p->baseitem.wMaxDura - v : p->baseitem.wMaxDura + v); }},
			{"curdura", [](ITEM* p, int o, int v) { p->wCurDura             = (WORD)(o == 0 ? v : o == 1 ? p->wCurDura - v : p->wCurDura + v); }},
			{"maxdura", [](ITEM* p, int o, int v) { p->wMaxDura             = (WORD)(o == 0 ? v : o == 1 ? p->wMaxDura - v : p->wMaxDura + v); }},
			{"ac1",     [](ITEM* p, int o, int v) { p->baseitem.Ac1         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Ac1 - v : p->baseitem.Ac1 + v); }},
			{"ac2",     [](ITEM* p, int o, int v) { p->baseitem.Ac2         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Ac2 - v : p->baseitem.Ac2 + v); }},
			{"mac1",    [](ITEM* p, int o, int v) { p->baseitem.Mac1        = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Mac1 - v : p->baseitem.Mac1 + v); }},
			{"mac2",    [](ITEM* p, int o, int v) { p->baseitem.Mac2        = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Mac2 - v : p->baseitem.Mac2 + v); }},
			{"dc1",     [](ITEM* p, int o, int v) { p->baseitem.Dc1         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Dc1 - v : p->baseitem.Dc1 + v); }},
			{"dc2",     [](ITEM* p, int o, int v) { p->baseitem.Dc2         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Dc2 - v : p->baseitem.Dc2 + v); }},
			{"mc1",     [](ITEM* p, int o, int v) { p->baseitem.Mc1         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Mc1 - v : p->baseitem.Mc1 + v); }},
			{"mc2",     [](ITEM* p, int o, int v) { p->baseitem.Mc2         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Mc2 - v : p->baseitem.Mc2 + v); }},
			{"sc1",     [](ITEM* p, int o, int v) { p->baseitem.Sc1         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Sc1 - v : p->baseitem.Sc1 + v); }},
			{"sc2",     [](ITEM* p, int o, int v) { p->baseitem.Sc2         = (BYTE)(o == 0 ? v : o == 1 ? p->baseitem.Sc2 - v : p->baseitem.Sc2 + v); }},
			{"price",   [](ITEM* p, int o, int v) { p->baseitem.nPrice      = (o == 0 ? v : o == 1 ? p->baseitem.nPrice - v : p->baseitem.nPrice + v); }},
			{"image",   [](ITEM* p, int o, int v) { p->baseitem.wImageIndex = (WORD)(o == 0 ? v : o == 1 ? p->baseitem.wImageIndex - v : p->baseitem.wImageIndex + v); }},
		};
		// 通用属性修改
		auto it = modifiers.find(exps[0]);
		if (it != modifiers.end())
		{
			int val = StringToInteger(pValue);
			it->second(&item, opr, val);
		}
		// limit: 特殊bitmask操作
		else if (_stricmp(exps[0], "limit") == 0)
		{
			WORD wLimit = 0;
			xStringsExtracter<40> limit(pValue, "|", " \t");
			if (limit.getCount() == 0) continue;

			for (UINT n = 0; n < limit.getCount(); n++)
			{
				item_limit l = GetItemLimit(limit[n]);
				if (l == IL_MAX) continue;
				wLimit |= (1 << (int)l);
			}
			UPGRADEADDMASK mask;
			mask.dwValue = item.dwParam[0];
			if (opr == 0)
				mask.wItemLimit = wLimit;
			else if (opr == 1)
				mask.wItemLimit ^= (mask.wItemLimit & wLimit);
			else if (opr == 2)
				mask.wItemLimit |= wLimit;
			item.dwParam[0] = mask.dwValue;
		}
		// name: 特殊字符串操作
		else if (_stricmp(exps[0], "name") == 0)
		{
			BYTE btLen = static_cast<BYTE>(strlen(pValue));
			if (btLen > 14)
			{
				pValue[14] = 0;
				btLen = 14;
			}
			memcpy(item.baseitem.szName, pValue, 14);
			item.baseitem.btNameLength = btLen;
		}
	}
	return TRUE;
}

BOOL CItemManager::UpgradePetItem(ITEM& item, const char* pszUpgradeString)
{
	BOOL b = FALSE;
	char szTempBuffer[1024];
	o_strncpy(szTempBuffer, pszUpgradeString, 1020);
	xStringsExtracter<50> mstr(szTempBuffer, ",", " \t");
	for (UINT i = 0; i < mstr.getCount(); i++)
	{
		xStringsExtracter<2> exps(mstr[i], "=", " \t");
		if (exps.getCount() < 2)
			continue;
		int opr = 0;

		char* pValue = exps[1];

		if (pValue[0] == '-')
		{
			opr = 1;
			pValue = TrimEx(pValue + 1);
		}
		else if (pValue[0] == '+')
		{
			opr = 2;
			pValue = TrimEx(pValue + 1);
		}

		if (_stricmp(exps[0], "mc") == 0)
		{
			int temp = GetPetLevelInfo(item.baseitem.wDc, 0);
			WORD btemp = (WORD)temp;
			WORD MC = item.baseitem.wMc;
			MC += (WORD)StringToInteger(pValue);
			if (MC >= btemp)
			{
				if (item.baseitem.wDc >= 7)
					item.baseitem.wMc = (WORD)StringToInteger(pValue);
				else
				{
					b = TRUE;
					item.baseitem.wDc++;
					item.baseitem.wMc = MC - btemp;
				}
			}
			else
				item.baseitem.wMc += StringToInteger(pValue);
		}
	}
	return b;
}