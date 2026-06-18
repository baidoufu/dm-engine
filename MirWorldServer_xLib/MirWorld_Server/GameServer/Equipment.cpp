#include "StdAfx.h"
#include ".\equipment.h"
#include "ItemManager.h"
#include "humanplayer.h"
#include "sandcity.h"

CEquipment::CEquipment(CHumanPlayer* pPlayer)
{
	assert(pPlayer != nullptr);
	m_pPlayer = pPlayer;
	Clean();
}

CEquipment::~CEquipment(VOID)
{
}

BOOL CEquipment::EquipItem(int pos, ITEM& inItem, ITEM& outItem, BOOL bForced, BOOL bNoticePlayer)
{
	if (pos < 0 || pos > _U_MAX)return FALSE;
	if (!bForced) // 检查条件是否满足
	{
		if (!CheckFitable(pos, inItem))return FALSE;
	}
	if (bNoticePlayer)
	{
		if (!m_pPlayer->OnEquipItem(pos, inItem)) return FALSE;
	}
	//如果存在旧装备, 则卸下它
	if (m_Equipments[pos].dwMakeIndex != 0)
	{
		outItem.dwMakeIndex = 0;
		if (!UnEquipItem(pos, outItem)) return FALSE;
	}
	//装备新物品
	m_Equipments[pos] = inItem;
	AddPropCache(inItem);
	m_pPlayer->OnEquipmentOn(pos, m_Equipments[pos]);
	return TRUE;
}

BOOL CEquipment::UnEquipItem(int pos, ITEM& outItem)
{
	if (pos < 0 || pos > _U_MAX)return FALSE;

	if (m_Equipments[pos].dwMakeIndex == 0) return FALSE;
	if (!m_pPlayer->OnUnEquipItem(pos, m_Equipments[pos])) return FALSE;
	outItem = m_Equipments[pos];
	memset(&m_Equipments[pos], 0, sizeof(ITEM));
	RemovePropCache(outItem);
	m_pPlayer->OnEquipmentOff(pos, m_Equipments[pos]);
	return TRUE;
}

VOID CEquipment::Clean()
{
	m_Equipments.fill({});
	m_PropMap.fill(0);
}

BOOL CEquipment::CheckFitable(int pos, ITEM& inItem)
{
	m_strErrorMsg = "错误的装备位置!";
	if (!VerifyPos(pos))return FALSE;
	if (!CheckStdModeFitPos(inItem.baseitem.btStdMode, pos))return FALSE;
	if (!CheckNeedInfo(inItem.baseitem.needtype, inItem.baseitem.needvalue))return FALSE;
	m_strErrorMsg = "太重了!";
	if (pos == _U_WEAPON)
	{
		if (inItem.baseitem.btWeight > m_pPlayer->GetPropValue(PI_MAXHANDWEIGHT))
			return FALSE;
	}
	else
	{
		int weight = CalcEquipmentsWeight(pos);
		int maxweight = (int)m_pPlayer->GetPropValue(PI_MAXBODYWEIGHT);
		if (weight >= maxweight)return FALSE;
		if (inItem.baseitem.btWeight + weight > maxweight)return FALSE;
	}
	return TRUE;
}

BOOL CEquipment::GetStdModeMeaning(BYTE stdMode, int& dc1p, int& dc2p, int& mc1p, int& mc2p, int& sc1p, int& sc2p, int& ac1p, int& ac2p, int& mac1p, int& mac2p)
{
	ac1p = -1, ac2p = -1, mac1p = -1, mac2p = -1;
	dc1p = PI_MINDC, dc2p = PI_MAXDC, mc1p = PI_MINMC, mc2p = PI_MAXMC, sc1p = PI_MINSC, sc2p = PI_MAXSC;
	switch (stdMode)
	{
	case 5: // 武器
	case 6: // 特殊武器
		ac1p = PI_LUCKY, ac2p = PI_HITRATE, mac1p = PI_DAWN;
		mac2p = PI_ATTACKSPEED * -1;
		return TRUE;
	case 10: // 衣服（男）
	case 11: // 衣服（女）
	case 15: // 头盔
	case 30: // 勋章
	case 59: // 宝石
	case 60:
	case 61:
	case ISM_SHOES: // 靴子
	case ISM_BELT: // 腰带
		ac1p = PI_MINAC, ac2p = PI_MAXAC, mac1p = PI_MINMAC, mac2p = PI_MAXMAC;
		return FALSE;
	case 19: // 项链
		ac2p = PI_MAGESCAPE,			//	魔法躲避+300%	maxdef*10
			mac2p = PI_LUCKY,			//	幸运+50		maxmagdef
			mac1p = PI_DAWN;			//	诅咒+40		minmagdef
		return FALSE;
	case 20: // 项链
		ac2p = PI_HITRATE,				//	命中+30		maxdef
			mac2p = PI_ESCAPE;			//	躲避+50		maxmagdef
		return FALSE;
	case 21: // 特殊项链
		ac2p = PI_HPRECOVER,			//体力恢复+30	maxdef
			ac1p = PI_ATTACKSPEED,		//攻击速度+20	mindef
			mac2p = PI_MPRECOVER,		//魔法恢复+50	maxmagdef
			mac1p = PI_ATTACKSPEED * -1;//攻击速度-40	minmagdef
		return TRUE;					//强度+33		specialpower
	case 22: // 戒指
	case 26: // 手镯
		ac1p = PI_MINAC, ac2p = PI_MAXAC, mac1p = PI_MINMAC, mac2p = PI_MAXMAC;
		return TRUE;
	case 23: // 戒指
		ac2p = PI_POISONESCAPE,			//毒物躲避+300%	maxdef*10
			ac1p = PI_ATTACKSPEED,		//攻击速度+20	mindef
			mac2p = PI_POISONRECOVER,	//中毒恢复+500%	maxmagdef*10
			mac1p = PI_ATTACKSPEED * -1;//攻击速度-40	minmagdef
		return TRUE;					//强度+33		specialpower
	case 24: // 手镯
		ac2p = PI_HITRATE,				//命中+30		maxdef
			mac2p = PI_ESCAPE;			//躲避+50		maxmagdef
		return TRUE;					//强度+33		specialpower
	case 25:
	case 34:
		dc1p = -1, dc2p = -1, mc1p = -1, mc2p = -1, sc1p = -1, sc2p = -1;
		return FALSE;
	}
	dc1p = -1, dc2p = -1, mc1p = -1, mc2p = -1, sc1p = -1, sc2p = -1;
	return FALSE;
}

VOID CEquipment::AddPropCache(ITEM& item)
{
	int	dc1p, dc2p, mc1p, mc2p, sc1p, sc2p, ac1p = PI_MINAC, ac2p = PI_MAXAC, mac1p = PI_MINMAC, mac2p = PI_MAXMAC;
	BOOL bAddSpecialPower = GetStdModeMeaning(item.baseitem.btStdMode, dc1p, dc2p, mc1p, mc2p, sc1p, sc2p, ac1p, ac2p, mac1p, mac2p);
	if (dc1p != -1)
		m_PropMap[dc1p] += item.baseitem.btMinAtk;
	if (dc2p != -1)
		m_PropMap[dc2p] += item.baseitem.btMaxAtk;
	if (mc1p != -1)
		m_PropMap[mc1p] += item.baseitem.btMinMagAtk;
	if (mc2p != -1)
		m_PropMap[mc2p] += item.baseitem.btMaxMagAtk;
	if (sc1p != -1)
		m_PropMap[sc1p] += item.baseitem.btMinSouAtk;
	if (sc2p != -1)
		m_PropMap[sc2p] += item.baseitem.btMaxSouAtk;
	if (ac1p != -1)
	{
		if (ac1p < 0)
			m_PropMap[ac1p * -1] -= item.baseitem.btMinDef;
		else
			m_PropMap[ac1p] += item.baseitem.btMinDef;
	}
	if (ac2p != -1)
	{
		int nMaxDef = item.baseitem.btMaxDef;
		if (item.baseitem.btStdMode == 19 || item.baseitem.btStdMode == 23) // 如果是项链、戒指, 要乘以10
		{
			nMaxDef *= 10; // 魔法躲避 或者 毒物躲避
		}
		if (ac2p < 0)
			m_PropMap[ac2p * -1] -= nMaxDef;
		else
			m_PropMap[ac2p] += nMaxDef;
	}
	if (mac1p != -1)
	{
		if (mac1p < 0)
			m_PropMap[mac1p * -1] -= item.baseitem.btMinMagDef;
		else
			m_PropMap[mac1p] += item.baseitem.btMinMagDef;
	}
	if (mac2p != -1)
	{
		int nMaxMagDef = item.baseitem.btMaxMagDef;
		if (item.baseitem.btStdMode == 23) // 如果是戒指, 要乘以10
		{
			nMaxMagDef *= 10; // 中毒恢复
		}
		if (mac2p < 0)
			m_PropMap[mac2p * -1] -= nMaxMagDef;
		else
			m_PropMap[mac2p] += nMaxMagDef;
	}
	if (bAddSpecialPower)
	{
		char value = (char)item.baseitem.btSpecialpower;
		if (value > 0)
			m_PropMap[PI_HARD] += value;
		else
			m_PropMap[PI_HOLLY] += value * -1;
	}
	ITEMCLASS* pItemClass = CItemManager::GetInstance()->GetItemClassPtr(item);
	if (pItemClass == nullptr) return;
	int btMagicNicety = pItemClass->btMagicNicety;
	if (btMagicNicety > 0)
	{
		m_PropMap[PI_MAGHITRATE] += btMagicNicety;
	}
	int btPoisonNicety = pItemClass->btPoisonNicety;
	if (btPoisonNicety > 0)
	{
		m_PropMap[PI_POISONHITRATE] += btPoisonNicety;
	}
}

VOID CEquipment::RemovePropCache(ITEM& item)
{
	int	dc1p, dc2p, mc1p, mc2p, sc1p, sc2p, ac1p = PI_MINAC, ac2p = PI_MAXAC, mac1p = PI_MINMAC, mac2p = PI_MAXMAC;
	BOOL bAddSpecialPower = GetStdModeMeaning(item.baseitem.btStdMode, dc1p, dc2p, mc1p, mc2p, sc1p, sc2p, ac1p, ac2p, mac1p, mac2p);
	if (dc1p != -1)
		m_PropMap[dc1p] -= item.baseitem.btMinAtk;
	if (dc2p != -1)
		m_PropMap[dc2p] -= item.baseitem.btMaxAtk;
	if (mc1p != -1)
		m_PropMap[mc1p] -= item.baseitem.btMinMagAtk;
	if (mc2p != -1)
		m_PropMap[mc2p] -= item.baseitem.btMaxMagAtk;
	if (sc1p != -1)
		m_PropMap[sc1p] -= item.baseitem.btMinSouAtk;
	if (sc2p != -1)
		m_PropMap[sc2p] -= item.baseitem.btMaxSouAtk;
	if (ac1p != -1)
	{
		if (ac1p < 0)
			m_PropMap[ac1p * -1] += item.baseitem.btMinDef;
		else
			m_PropMap[ac1p] -= item.baseitem.btMinDef;
	}
	if (ac2p != -1)
	{
		int nMaxDef = item.baseitem.btMaxDef;
		if (item.baseitem.btStdMode == 19 || item.baseitem.btStdMode == 23) // 如果是项链、戒指, 要乘以10
		{
			nMaxDef *= 10; // 魔法躲避 或者 毒物躲避
		}
		if (ac2p < 0)
			m_PropMap[ac2p * -1] += nMaxDef;
		else
			m_PropMap[ac2p] -= nMaxDef;
	}
	if (mac1p != -1)
	{
		if (mac1p < 0)
			m_PropMap[mac1p * -1] += item.baseitem.btMinMagDef;
		else
			m_PropMap[mac1p] -= item.baseitem.btMinMagDef;
	}
	if (mac2p != -1)
	{
		int nMaxMagDef = item.baseitem.btMaxMagDef;
		if (item.baseitem.btStdMode == 23) // 如果是戒指, 要乘以10
		{
			nMaxMagDef *= 10; // 中毒恢复
		}
		if (mac2p < 0)
			m_PropMap[mac2p * -1] += nMaxMagDef;
		else
			m_PropMap[mac2p] -= nMaxMagDef;
	}
	if (bAddSpecialPower)
	{
		char value = (char)item.baseitem.btSpecialpower;
		if (value > 0)
			m_PropMap[PI_HARD] -= value;
		else
			m_PropMap[PI_HOLLY] -= value * -1;
	}
	ITEMCLASS* pItemClass = CItemManager::GetInstance()->GetItemClassPtr(item);
	if (pItemClass == nullptr) return;
	int btMagicNicety = pItemClass->btMagicNicety;
	if (btMagicNicety > 0)
	{
		m_PropMap[PI_MAGHITRATE] -= btMagicNicety;
	}
	int btPoisonNicety = pItemClass->btPoisonNicety;
	if (btPoisonNicety > 0)
	{
		m_PropMap[PI_POISONHITRATE] -= btPoisonNicety;
	}
}

BOOL CEquipment::CheckStdModeFitPos(BYTE stdMode, int pos)
{
	m_strErrorMsg = "错误的装备位置!";
	BOOL bRet = FALSE;
	switch (pos)
	{
	case _U_CHARM:
		bRet =(stdMode == 30/*勋章*/ || stdMode == 32 || stdMode == 33/*马牌*/);
		break;
	case _U_WEAPON:
		bRet = (stdMode == 5 || stdMode == 6);
		break;
	case _U_DRESS:
		bRet = ((m_pPlayer->GetSex() + 10) == stdMode);
		break;
	case _U_NECKLACE:
		bRet = (stdMode == 19 || stdMode == 20 || stdMode == 21);
		break;
	case _U_RINGL:
	case _U_RINGR:
		bRet = (stdMode == 22 || stdMode == 23);
		break;
	case _U_ARMRINGL:
		bRet = (stdMode == 24 || stdMode == 26);
		break;
	case _U_ARMRINGR:
		bRet = (stdMode == 24 || stdMode == 26 || stdMode == 25 || stdMode == 34);
		break;
	case _U_HELMET:
		bRet =(stdMode == 15);
		break;
	case _U_SHOE:
		bRet = (stdMode == ISM_SHOES);
		break;
	case _U_BELT:
		bRet = (stdMode == ISM_BELT);
		break;
	case _U_STONE:
		bRet = (stdMode == 59 || stdMode == 60 || stdMode == 61);
		break;
	case _U_POISON:
		bRet = (stdMode == 25 || stdMode == 34 || stdMode == 33);
		break;
	default:
		break;
	}
	if (!bRet && pos == _U_DRESS)
	{
		m_strErrorMsg = "性别不符, 不能装备!";
	}
	return bRet;
}

BOOL CEquipment::CheckItemInfo(int pos, BYTE stdMode, BYTE btShape)
{
	ITEM* item = GetEquipment(pos);
	if (item && item->baseitem.btStdMode == stdMode && item->baseitem.btShape == btShape)
		return TRUE;
	return FALSE;
}

BOOL CEquipment::CheckNeedInfo(BYTE needtype, BYTE needlevel)
{
	//NeedType表示限制种类.
	//NeedLevel表示了(NeedType)限制的具体数量.
	switch (needtype)
	{
	case INT_LEVEL:
		m_strErrorMsg = "等级不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetPropValue(PI_LEVEL) >= needlevel);
	case INT_DC:
		m_strErrorMsg = "攻击力不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetPropValue(PI_MAXDC) >= needlevel);
	case INT_MC:
		m_strErrorMsg = "魔力不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetPropValue(PI_MAXMC) >= needlevel);
	case INT_SC:
		m_strErrorMsg = "道术不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetPropValue(PI_MAXSC) >= needlevel);
	case INT_PKVALUE:
		m_strErrorMsg = "PK值不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetPkValue() >= needlevel);
	case INT_CREDIT:
		m_strErrorMsg = "声望不够!";
		return (m_pPlayer != nullptr && m_pPlayer->GetCredit() >= needlevel);
	case INT_SABUKOWNER:
		m_strErrorMsg = "您不是沙城城主!";
		return (m_pPlayer != nullptr && m_pPlayer->GetGuild() != nullptr && m_pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild());
	}
	return TRUE;
}

VOID CEquipment::ResetPropCache()
{
	m_PropMap.fill(0);
	for (int i = 0; i < _U_MAX; i++)
	{
		if (m_Equipments[i].dwMakeIndex != 0)
		{
			AddPropCache(m_Equipments[i]);
		}
	}
}
