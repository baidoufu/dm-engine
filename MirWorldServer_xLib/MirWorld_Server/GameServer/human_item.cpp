#include "StdAfx.h"
#include "humanplayer.h"
#include "logicmapmgr.h"
#include "gameworld.h"
#include "guildex.h"
#include "sandcity.h"
#include "logicmap.h"
#include "SystemScript.h"
#include "math.h"
#include "fireburnevent.h"
#include "monstermanagerex.h"
#include "monsterex.h"
#include "gsclientobj.h"
#include "SpecialEquipmentManager.h"
#include "visibleevent.h"
#include "eventmanager.h"
#include "itemmanager.h"

BOOL CHumanPlayer::OnEquipItem(int pos, ITEM& item)
{
	if (item.baseitem.btStdMode == 33 && item.baseitem.btMinDef != 0)
	{
		if (IsEquipedHorse())
		{
			m_Equipments.SetErrorMsg(getstrings(SD_YOUALREADYEQUIPAHORSE));
			return FALSE;
		}
		if (m_pHorse != nullptr || m_bRideHorse)
		{
			m_Equipments.SetErrorMsg(getstrings(SD_YOUALREADYHAVEAHORSE));
			return FALSE;
		}
		if (m_pMap == nullptr)return FALSE;
		if (!m_pMap->IsFlagSeted(MF_RIDEHORSE))
		{
			m_Equipments.SetErrorMsg(getstrings(SD_MAPNOTALLOWRIDE));
			return FALSE;
		}
		//马牌1 212枣红马
		//马牌2 209雪龙
		//马牌3 206乌骓
		//马牌4 187黄金宝马
		//马牌5 211迎亲宝马
		//马牌6 184高头大马
		if (item.baseitem.btMinDef == 206)
		{
			CGuildEx* pGuild = GetGuild();
			if (!pGuild || !pGuild->IsMaster(this))
			{
				m_Equipments.SetErrorMsg("乌骓马牌只能行会会长穿戴!");
				return FALSE;
			}
		}
		if (item.baseitem.btMinDef == 209)
		{
			CGuildEx* pGuild = GetGuild();
			if (!pGuild || !pGuild->IsMaster(this) || pGuild != CSandCity::GetInstance()->GetOwnerGuild())
			{
				m_Equipments.SetErrorMsg("雪龙马牌只能沙城行会会长穿戴!");
				return FALSE;
			}
		}
		char szName[12];
		o_strncpy(szName, (char*)&item.baseitem.btMinDef, 10);
		szName[10] = '\0';
		strcat(szName, "1");
		if (!SummonPet(szName))
		{
			SaySystem(getstrings(SD_HORSENOTSETED));
			return FALSE;
		}
		else
			SendOutPetInfo(&item, 2);//发送马的宠物信息
	}
	return TRUE;
}

BOOL CHumanPlayer::SummonPet(const char* pszName, BOOL bSetOwner, int x, int y)
{
	if (m_pMap == nullptr) return FALSE;
	
	// 检查是否为丛林豹
	auto IsBaoziPet = [](const char* name) -> BOOL {
		if (name == nullptr) return FALSE;
		if (strncmp(name, "丛林豹", 6) != 0) return FALSE;
		char level = name[6];
		return level >= '0' && level <= '7';
	};
	
	if (IsBaoziPet(pszName))
	{
		//如果第二次召唤就清空变量
		if (ISzhaohuan && m_pPet != nullptr)
		{
			//这里对豹子进行了释放操作
			CGameWorld::GetInstance()->RemoveMapObject(m_pPet);//从游戏世界移除这个对象
			CMonsterManagerEx::GetInstance()->DeleteMonster(m_pPet);//从怪物列表移除这怪物
			this->DelPet(m_pPet);//删除豹子时, 这个变量要清空
			return TRUE;
		}
	}
	if (x == -1 || y == -1)
	{
		GetFrontPosition(x, y);
		if (m_pMap->IsBlocked(x, y))
		{
			POINT pt;
			if (!m_pMap->GetValidPoint(getX(), getY(), &pt, 1))
				return FALSE;
			x = pt.x;
			y = pt.y;
		}
	}
	CMonsterEx* p = CMonsterManagerEx::GetInstance()->CreateMonster(pszName, this->GetMapId(), x, y);
	if (p == nullptr) return FALSE;
	if (bSetOwner) p->SetOwner(this);
	int petDir = GETBACKDIR(GetDirection());
	p->SetDirection((e_direction)petDir);
	if (!CGameWorld::GetInstance()->AddMapObject(p))
	{
		CMonsterManagerEx::GetInstance()->DeleteMonsterImm(p);
		return FALSE;
	}
	if (!AddPet(p)) return FALSE;
	if (IsBaoziPet(pszName))
	{
		ISzhaohuan = TRUE;
		SetPetName(pszName);
		char* pszBornScript = m_pPet->GetDesc()->pszBornScript;
		if (pszBornScript)
			CSystemScript::GetInstance()->Execute(this->GetScriptTarget(), m_pPet->GetDesc()->pszBornScript, FALSE);
	}
	int pMonId = p->GetId();
	SendMsg(GetId(), 0x9610, 0, 0, 0, &pMonId, sizeof(pMonId));
	return TRUE;
}

CMonsterEx* CHumanPlayer::SummonMonster(const char* pszName, BOOL bSetOwner, int x, int y)
{
	if (m_pMap == nullptr)return nullptr;
	if (x == -1 || y == -1)
	{
		GetFrontPosition(x, y);
		if (m_pMap->IsBlocked(x, y))
		{
			POINT	pt;
			if (!m_pMap->GetValidPoint(getX(), getY(), &pt, 1))
				return nullptr;
			x = pt.x;
			y = pt.y;
		}
	}
	CMonsterEx* p = CMonsterManagerEx::GetInstance()->CreateMonster(pszName, this->GetMapId(), x, y, nullptr);
	if (p == nullptr)
		return nullptr;
	if (!CGameWorld::GetInstance()->AddMapObject(p))
	{
		CMonsterManagerEx::GetInstance()->DeleteMonster(p);
		return nullptr;
	}
	if (bSetOwner)
		p->SetOwner(this);
	return p;
}

BOOL CHumanPlayer::OnUnEquipItem(int pos, ITEM& item)
{
	if (item.baseitem.btStdMode == 33 && item.baseitem.btMinDef != 0)
	{
		if (m_pHorse)
		{
			if (m_bRideHorse)
			{
				CMonsterManagerEx::GetInstance()->DeleteMonster(m_pHorse);
				m_bRideHorse = FALSE;
				m_pHorse = nullptr;
				SendFeatureChanged();
			}
			else
			{
				CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
				m_pHorse = nullptr;
			}
		}
	}
	return TRUE;
}

VOID CHumanPlayer::OnPetDie(CAliveObject* pPet, CAliveObject* pKiller)
{
	if (pPet && pPet == m_pHorse) // 如果是马
	{
		ITEM* pItem = GetEquipment(_U_CHARM);
		if (pItem && pItem->baseitem.btStdMode == 33)
		{
			ITEM item;
			if (m_Equipments.UnEquipItem(_U_CHARM, item))
			{
				this->SendTakeBagItem(&item);
			}
		}
		return;
	}
	if (pPet && pPet == m_pPet) // 如果是豹子
	{
		return;
	}
}

ITEM* CHumanPlayer::GetEquipedHorseItem()
{
	ITEM* pItem = GetEquipment(_U_POISON);
	if (pItem && pItem->baseitem.btStdMode == 33)
		return pItem;
	pItem = GetEquipment(_U_CHARM);
	if (pItem && pItem->baseitem.btStdMode == 33)
		return pItem;
	return nullptr;
}

VOID CHumanPlayer::RefreshSpecialEquipment()
{
	ITEM* pItem = GetEquipedHorseItem();
	if (pItem && pItem->baseitem.btMinDef != 0 && m_pHorse == nullptr)
	{
		if (m_pMap && m_pMap->IsFlagSeted(MF_RIDEHORSE))
		{
			char szName[32];
			o_strncpy(szName, (char*)&pItem->baseitem.btMinDef, 10);
			strcat(szName, "1");
			if (!SummonPet(szName))
			{
				SaySystem(getstrings(SD_HORSENOTSETED));
			}
			else
				SendOutPetInfo(pItem, 2);
		}
	}
	else if (!m_bRideHorse && m_pHorse != nullptr && m_pHorse->GetMap() == nullptr)
	{
		if (m_pMap && m_pMap->IsFlagSeted(MF_RIDEHORSE))
		{
			int x, y;
			GetFrontPosition(x, y);
			if (m_pMap->IsBlocked(x, y))
			{
				POINT pt;
				if (!m_pMap->GetValidPoint(getX(), getY(), &pt, 1))
					return;
				x = pt.x;
				y = pt.y;
			}
			m_pHorse->setXY(x, y);
			m_pHorse->SetMapId(GetMapId());
			CGameWorld::GetInstance()->AddMapObject(m_pHorse);
			if (pItem) SendOutPetInfo(pItem, 2);
		}
	}
}

BOOL CHumanPlayer::CanEnterMap(CLogicMap* pMap)
{
	DWORD dwParam;
	if (pMap->IsFlagSeted(MF_LEVELABOVE, dwParam))
	{
		if (GetPropValue(PI_LEVEL) <= LOWORD(dwParam))
		{
			SaySystem(getstrings(SD_MAPLIMITED_LOWLEVEL));
			return FALSE;
		}
	}
	if (pMap->IsFlagSeted(MF_LEVELBELOW, dwParam))
	{
		if (GetPropValue(PI_LEVEL) >= LOWORD(dwParam))
		{
			SaySystem(getstrings(SD_MAPLIMITED_HILEVEL));
			return FALSE;
		}
	}
	if (pMap->IsFlagSeted(MF_LIMITJOB, dwParam))
	{
		if (m_Humandesc.dbinfo.btClass != LOWORD(dwParam))
		{
			if (HIWORD(dwParam) > 0)
			{
				if (m_Humandesc.dbinfo.btClass != HIWORD(dwParam))
				{
					SaySystem(getstrings(SD_MAPLIMITED_WRONGJOB));
					return FALSE;
				}
			}
			else
			{
				SaySystem(getstrings(SD_MAPLIMITED_WRONGJOB));
				return FALSE;
			}
		}
	}
	if (pMap->IsFlagSeted(MF_PKPOINTABOVE, dwParam))
	{
		if (GetPkValue() <= LOWORD(dwParam))
		{
			SaySystem(getstrings(SD_MAPLIMITED_LOWPKPOINT));
			return FALSE;
		}
	}
	if (pMap->IsFlagSeted(MF_PKPOINTBELOW, dwParam))
	{
		if (GetPkValue() >= LOWORD(dwParam))
		{
			SaySystem(getstrings(SD_MAPLIMITED_HIPKPOINT));
			return FALSE;
		}
	}
	if (!pMap->IsFlagSeted(MF_RIDEHORSE))
	{
		if (m_bRideHorse)
		{
			RideHorse();
		}
		if (m_pHorse)
			CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
	}
	return TRUE;
}

BOOL CHumanPlayer::DoUpgradeWeapon()
{
	WORD wDc1 = 0, wDc2 = 0;
	WORD wMc1 = 0, wMc2 = 0;
	WORD wSc1 = 0, wSc2 = 0;
	WORD wHitrate = 0;
	DWORD dwDura = 0;
	int count = m_ItemBox.GetCount();

	ITEM* pWeapon = GetEquipment(_U_WEAPON);
	if (pWeapon == nullptr || pWeapon->baseitem.btStdMode == 6 || pWeapon->baseitem.bNeedIdentify ||
		CItemManager::GetInstance()->ItemLimited(*pWeapon, IL_NOUPGRADE))
	{
		SaySystem(getstrings(SD_NOFITABLEWEAPONTOUPGRADE));
		return FALSE;
	}

	if (GetUpgradeItemComp()->Item.dwMakeIndex != 0)
	{
		SaySystem(getstrings(SD_ONEWEAPONISUPGRADING));
		return FALSE;
	}

#ifdef _DEBUG
	SaySystem("武器升级次数: %u 次", pWeapon->baseitem.btUpgradeTimes);
#endif
	static thread_local std::array<ITEM, 100> s_arrMaterial{};
	DWORD arrMaterialCount = 0;

	for (int i = 0; i < count; i++)
	{
		ITEM* p = m_ItemBox.GetItem(i);
		if (p)
		{
			if ((p->baseitem.btStdMode == ISM_RING1 ||
				p->baseitem.btStdMode == ISM_RING0 ||
				p->baseitem.btStdMode == ISM_NECKLACE0 ||
				p->baseitem.btStdMode == ISM_NECKLACE1 ||
				p->baseitem.btStdMode == ISM_NECKLACE2 ||
				p->baseitem.btStdMode == ISM_BRACELET0 ||
				p->baseitem.btStdMode == ISM_BRACELET1))
			{
				if (arrMaterialCount < 100)
				{
					s_arrMaterial[arrMaterialCount++] = *p;
					wDc1 += p->baseitem.btMinAtk;
					wDc2 += p->baseitem.btMaxAtk;
					wMc1 += p->baseitem.btMinMagAtk;
					wMc2 += p->baseitem.btMaxMagAtk;
					wSc1 += p->baseitem.btMinSouAtk;
					wSc2 += p->baseitem.btMaxSouAtk;
					if (p->baseitem.btStdMode == ISM_NECKLACE1 || p->baseitem.btStdMode == ISM_BRACELET0)
					{
						wHitrate += p->baseitem.btMaxDef;
					}
				}
			}
			else if (p->baseitem.btStdMode == ISM_MINE && strncmp(p->baseitem.szName, CGameWorld::GetInstance()->GetName(ENI_UPGRADEMINESTONE), p->baseitem.btNameLength) == 0 /* p->baseitem.btShape == CGameWorld::GetInstance()->GetVar( EVI_SHAPE_BLACKSTONE )*/)		//	黑铁矿
			{
				if (arrMaterialCount < 100)
				{
					s_arrMaterial[arrMaterialCount++] = *p;
					dwDura += p->wCurDura > p->wMaxDura ? (p->wCurDura - p->wMaxDura) * 2 : 1;
				}
			}
		}
	}
	if (arrMaterialCount == 0)
	{
		SaySystem(getstrings(SD_WEAPONUPGRADENOENOUGHMATRIAL));
		return FALSE;
	}
#ifdef _DEBUG
	SaySystem("攻击: %u-%u 魔法: %u-%u 道术: %u-%u HITRATE: %u 持久: %u", wDc1, wDc2, wMc1, wMc2, wSc1, wSc2, wHitrate, dwDura);
#endif

	//	确定最后该加什么属性
	WORD wParam[7] = { wDc1, wDc2, wMc1, wMc2, wSc1, wSc2, wHitrate };
	int top = -1;
	WORD wTop = 0;
	for (int i = 0; i < 7; i++)
	{
		if (wParam[i] > wTop)
		{
			top = i;
			wTop = wParam[i];
		}
	}

	UPGRADEADDMASK mask;

	if (top != -1)
	{
		mask.addtype1 = top;
		mask.addvalue1 = 1;
		if (wTop > 100)
		{
			mask.addvalue1++;
		}
	}

	if (dwDura > 0)
	{
		mask.badddura = 1;
		mask.adddura = 1;
		//	ADD VALUE	0-3								2 BITS
		if (Getrand(100) > 80)
		{
			mask.adddura++;
		}
	}
	else
	{
		if (Getrand(100) > 80)
		{
			mask.adddura = 1;
			mask.badddura = 0;
		}
	}
	//	set mask and upgrade
	pWeapon->dwParam[0] = mask.dwValue;
	pWeapon->baseitem.bNeedIdentify = 1;
	//	如果是临时物品, 就等更新物品的时候更新到数据库,否则, 更新到数据库
	if (!m_Equipments.UnEquipItem(_U_WEAPON, GetUpgradeItemComp()->Item))
	{
		pWeapon->dwParam[0] = 0;
		pWeapon->baseitem.bNeedIdentify = 0;
		return FALSE;
	}

	if (!CItemManager::GetInstance()->UpgradeItem(GetUpgradeItemComp()->Item))
	{
		pWeapon->dwParam[0] = 0;
		pWeapon->baseitem.bNeedIdentify = 0;
		m_Equipments.EquipItem(_U_WEAPON, GetUpgradeItemComp()->Item, GetUpgradeItemComp()->Item, TRUE);
		return FALSE;
	}

	for (DWORD i = 0; i < arrMaterialCount; i++)
	{
		if (s_arrMaterial[i].dwMakeIndex != 0)
		{
			DeleteBagItem(&s_arrMaterial[i]);
			SendTakeBagItem(&s_arrMaterial[i]);
			CItemManager::GetInstance()->DeleteItem(s_arrMaterial[i].dwMakeIndex);
		}
	}
	SendTakeBagItem(&GetUpgradeItemComp()->Item);
	return TRUE;
}

ITEM* CHumanPlayer::FindEquipmentWithShape(BYTE btShape, int& pos)
{
	for (pos = 0; pos < _U_MAX; pos++)
	{
		ITEM* pEquipment = this->GetEquipment(pos);
		if (pEquipment && pEquipment->baseitem.btShape == btShape)
		{
			return pEquipment;
		}
	}
	return nullptr;
}

VOID CHumanPlayer::ChangeHair(BYTE	btHair)
{
	this->m_Humandesc.dbinfo.btHair = btHair;
	SendFeatureChanged();
}

VOID CHumanPlayer::ChangeWeaponView(BYTE btView)
{
	ITEM* p = GetEquipment(_U_WEAPON);
	if (p)
	{
		p->baseitem.btShape = btView;
		SendFeatureChanged();
	}
}

BOOL CHumanPlayer::CheckEquipment(int pos, int stdmode, int image, int& posout)
{
	ITEM* p = nullptr;
	if (pos < 0 || pos >= _U_MAX)
	{
		for (int i = 0; i < _U_MAX; i++)
		{
			if (p = GetEquipment(i))
			{
				if (stdmode != -1)
				{
					if (p->baseitem.btStdMode != (BYTE)stdmode)
						continue;
				}
				if (image == -1)
					return TRUE;
				if (p->baseitem.wImageIndex != (WORD)image)
					continue;
				posout = i;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		p = GetEquipment(pos);
		if (p)
		{
			if (stdmode != -1)
			{
				if (p->baseitem.btStdMode != (BYTE)stdmode)
					return FALSE;
			}
			if (image != -1)
			{
				if (p->baseitem.wImageIndex != (WORD)image)
					return FALSE;
			}
			posout = pos;
			return TRUE;
		}
	}
	return FALSE;
}

//BOOL	CHumanPlayer::IsSpecialEquipmentFunctionOn( special_equipment_func func )
//{
//	DWORD	dwFlag = 1<<(int)func;
//	if( dwFlag & this->m_dwSpecialEquipmentFunctionFlag )
//	{
//		return TRUE;
//	}
//	return FALSE;
//}

VOID CHumanPlayer::ProcSpecialEquipmentFunctionOff()
{
	for (int i = 0; i < SEF_MAX; i++)
	{
		if (IsSpecialEquipmentFunctionOn((special_equipment_func)i))
		{
			DWORD dwPosFlag = 0;
			if (!CSpecialEquipmentManager::GetInstance()->MatchFunction(this, (special_equipment_func)i, dwPosFlag))
				SetSpecialEquipmentFunctionOff((special_equipment_func)i);
			else
				SetSpecialEquipmentFunctionOn((special_equipment_func)i, dwPosFlag); // 更新生效物品的位置标记!
		}
	}
}

VOID CHumanPlayer::ProcSpecialEquipmentFunctionOn()
{
	for (int i = 0; i < SEF_MAX; i++)
	{
		if (!IsSpecialEquipmentFunctionOn((special_equipment_func)i))
		{
			DWORD dwPosFlag = 0;
			if (CSpecialEquipmentManager::GetInstance()->MatchFunction(this, (special_equipment_func)i, dwPosFlag))
				SetSpecialEquipmentFunctionOn((special_equipment_func)i, dwPosFlag);
		}
	}
}

VOID CHumanPlayer::OnSpecialEquipmentFunctionOn(special_equipment_func func)
{
	if (IsGameMaster())
	{
		if (func >= 0 && func < SEF_MAX)
			SaySystem("%s %s", g_special_equipment_table[func], getstrings(SD_SPECIALFUNCTIONTURNON));
	}
	switch (func)
	{
	case SEF_RELIVE:
	{
		DWORD dwTime = CFrameTime::GetFrameTime();
		if (dwTime < 2 * 60 * 1000)
			dwTime = 0;
		else
			dwTime -= 2 * 60 * 1000;
		SetRateLimitInterval(RateLimitComponent::ACT_RELIVE, dwTime);
	}
	break;
	case SEF_CLOAK:
	{
		if (IsStatusSet(SI_CLOAK)) ClrStatus(SI_CLOAK);
		SetStatus(SI_CLOAK, 0, 0xffffffff);
	}
	break;
	case SEF_BAGOVERLOAD:
	case SEF_OVERLOAD:
	{
		UpdateProp();
	}
	break;
	case SEF_HEALHALO:
	{
		BYTE btClass = GetPro() % 3;
		int	addvalue = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HEALHALO, btClass);
		AddProp(PI_MAXHP, addvalue);
		char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_HEALHALO);
		if (p[0] == 0)p = nullptr;
		SendAroundMsg(GetId(), 0x532c, 0x10, 1, 0);
		SendMsg(GetId(), 0x532c, 0x10, 1, 0, (LPVOID)p, p ? strlen(p) + 1 : 0);
	}
	break;
	case SEF_MIRAGE:
	{
		char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_MIRAGE);
		if (p[0] == 0)p = nullptr;
		SendAroundMsg(GetId(), 0x532c, 0x10, 2, 0);
		SendMsg(GetId(), 0x532c, 0x10, 2, 0, (LPVOID)p, p ? strlen(p) + 1 : 0);
	}
	break;
	case SEF_DEFENCEUP:
	{
		int addac = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_DEFENCEUP, 0);
		int addmac = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_DEFENCEUP, 1);
		AddProp(PI_MAXAC, addac);
		AddProp(PI_MAXMAC, addmac);
	}
	break;
	case SEF_SHENWUALL:
	case SEF_SHENWUHALF:
	case SEF_SHIXUEALL:
	case SEF_SHIXUEHALF:
	{
		int addhprecover = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(func, 0);
		AddProp(PI_HPRECOVER, addhprecover);
	}
	break;
	case SEF_HUANMOALL:
	case SEF_HUANMOHALF:
	case SEF_HUANMO01:
	case SEF_HUANMO02:
	case SEF_HUANMO03:
	case SEF_HUANMO04:
	case SEF_HUANMO05:
	case SEF_HUANMO06:
	case SEF_HUANMO07:
	case SEF_HUANMO08:
	case SEF_HUANMO09:
	case SEF_HUANMO10:
	case SEF_MOXUEALL:
	case SEF_MOXUEHALF:
	case SEF_MOXUE01:
	case SEF_MOXUE02:
	case SEF_MOXUE03:
	case SEF_MOXUE04:
	case SEF_MOXUE05:
	{
		int addmaxhp = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(func, 0);
		AddProp(PI_MAXHP, addmaxhp);
	}
	}
}

VOID CHumanPlayer::OnSpecialEquipmentFunctionOff(special_equipment_func func)
{
	switch (func)
	{
	case SEF_CLOAK:
	{
		if (IsStatusSet(SI_CLOAK)) ClrStatus(SI_CLOAK);
	}
	break;
	case SEF_BAGOVERLOAD:
	case SEF_OVERLOAD:
	{
		UpdateProp();
	}
	break;
	case SEF_HEALHALO:
	{
		BYTE btClass = GetPro() % 3;
		int	addvalue = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HEALHALO, btClass);
		AddProp(PI_MAXHP, -1 * addvalue);
		SendAroundMsg(GetId(), 0x532c, 0x10, 0, 0);
		SendMsg(GetId(), 0x532c, 0x10, 0, 0);
	}
	break;
	case SEF_MIRAGE:
	{
		SendAroundMsg(GetId(), 0x532c, 0x10, 0, 0);
		SendMsg(GetId(), 0x532c, 0x10, 0, 0);
	}
	break;
	case SEF_DEFENCEUP:
	{
		int addac = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_DEFENCEUP, 0);
		int addmac = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_DEFENCEUP, 1);
		AddProp(PI_MAXAC, -1 * addac);
		AddProp(PI_MAXMAC, -1 * addmac);
	}
	break;
	case SEF_SHENWUALL:
	case SEF_SHENWUHALF:
	case SEF_SHIXUEALL:
	case SEF_SHIXUEHALF:
	{
		int addhprecover = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(func, 0);
		AddProp(PI_HPRECOVER, -1 * addhprecover);
	}
	break;
	case SEF_HUANMOALL:
	case SEF_HUANMOHALF:
	case SEF_HUANMO01:
	case SEF_HUANMO02:
	case SEF_HUANMO03:
	case SEF_HUANMO04:
	case SEF_HUANMO05:
	case SEF_HUANMO06:
	case SEF_HUANMO07:
	case SEF_HUANMO08:
	case SEF_HUANMO09:
	case SEF_HUANMO10:
	case SEF_MOXUEALL:
	case SEF_MOXUEHALF:
	case SEF_MOXUE01:
	case SEF_MOXUE02:
	case SEF_MOXUE03:
	case SEF_MOXUE04:
	case SEF_MOXUE05:
	{
		int addmaxhp = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(func, 0);
		AddProp(PI_MAXHP, -1 * addmaxhp);
	}
	break;
	}
}

VOID CHumanPlayer::OnEquipmentOn(int pos, ITEM& item)
{
	this->ProcSpecialEquipmentFunctionOn(); // 开启特殊穿戴属性
}

VOID CHumanPlayer::OnEquipmentOff(int pos, ITEM& item)
{
	this->ProcSpecialEquipmentFunctionOff(); // 取消特殊穿戴属性
}

VOID CHumanPlayer::OnDoAction(actiontype action)
{
	switch (action)
	{
	case AT_ATTACK:
	{
		if (IsStatusSet(SI_CLOAK)) ClrStatus(SI_CLOAK);
	}
	break;
	}
}

BOOL CHumanPlayer::WillDie()
{
	if (IsSpecialEquipmentFunctionOn(SEF_RELIVE))
	{
		auto* rl = GetRateLimit();
		if (rl && rl->TryExecute(RateLimitComponent::ACT_RELIVE, CFrameTime::GetFrameTime()))
		{
			AddPropValue(PI_CURHP, GetPropValue(PI_MAXHP));
			//SendHpMpChanged();
			DamageSpecialEquipment(SEF_RELIVE, 1000);
			char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_RELIVE);
			if (p != nullptr && p[0] != 0)
				SaySystem(p);
			return FALSE;
		}
	}
	return TRUE;
}

VOID CHumanPlayer::DamageSpecialEquipment(special_equipment_func func, int iDamage)
{
	DWORD dwPosFlag;
	if (!IsSpecialEquipmentFunctionOn(func))return;
	for (int i = 0; i < _U_MAX; i++)
	{
		dwPosFlag = 1 << i;
		if (GetSpecialEquipmentFlag(func) & dwPosFlag)
		{
			DamageDura(i, iDamage);
		}
	}
}

WORD CHumanPlayer::GetBodyEffect()
{
	WORD wFlag = 0;
	if (IsSpecialEquipmentFunctionOn(SEF_HEALHALO))
		wFlag |= 1;
	if (IsSpecialEquipmentFunctionOn(SEF_MIRAGE))
		wFlag |= 2;
	return wFlag;
}

VOID CHumanPlayer::SendBodyEffectChanged()
{
	WORD wFlag = GetBodyEffect();
	SendAroundMsg(GetId(), 0x532c, 0x10, wFlag, 0);
	SendMsg(GetId(), 0x532c, 0x10, wFlag, 0);
}

BOOL CHumanPlayer::DoMine(int dir, int x, int y)
{
	CLogicMap* pMap = GetMap();
	if (pMap == nullptr) return FALSE;
	if (!CanDoAction(AT_MINE)) return FALSE;
	if (!SetAction(AT_MINE, (e_direction)dir, getX(), getY(), 800)) return FALSE;
	DWORD dwParam; //矿洞地图返回参数
	//	如果可以挖矿, 那么就显示矿渣, 并且判断是否得到矿石
	if (pMap->IsFlagSeted(MF_MINE, dwParam))
	{
		_mineCounter()++;
		UpdateMineEffect();
		if ((_mineCounter() % (4)) == 0)
		{
			if (m_pClientObj) m_pClientObj->PostMsg("#+DIG!", 6);
		}
		if (_mineCounter() >= (dwParam / 2 + Getrand(dwParam + 1)))
		{
			pMap->GotMineItem(this);
			_mineCounter() = 0;
		}
	}
	else
		return FALSE;
	SendAroundMsg(GetId(), 0xf, x, y, dir);
	return TRUE;
}

VOID CHumanPlayer::UpdateMineEffect()
{
	if (m_pMap == nullptr)return;
	CVisibleEvent* pEvent = (CVisibleEvent*)m_pMap->FindEventObject(getX(), getY(), VE_MINESTONE);
	if (pEvent == nullptr)
		pEvent = CEventManager::GetInstance()->NewVisibleEvent(m_pMap, getX(), getY(), VE_MINESTONE, 1000, 100000, nullptr, 0, 0);
	if (pEvent)
	{
		DWORD dwParam1 = pEvent->GetParam1();
		if (dwParam1 < 5)
			pEvent->SetParam(dwParam1 + 1, pEvent->GetParam2());
	}
}

BOOL CHumanPlayer::IsGodBlessEffectivable(special_godbless type, CAliveObject* pObject)
{
	if (pObject->GetType() != OBJ_MONSTER ||
		((CMonsterEx*)pObject)->GetDesc() == nullptr ||
		((CMonsterEx*)pObject)->GetDesc()->base.btRace == MR_BOSS ||
		((CMonsterEx*)pObject)->GetDesc()->base.btRace == MR_HUMAN ||
		(GetOwner() != nullptr && GetOwner()->GetType() == OBJ_PLAYER))
		return FALSE;
	switch (type)
	{
	case SG_SHIELD:
	{
		if (this->IsSpecialEquipmentFunctionOn(SEF_SHIELD))
		{
			DWORD dwParam = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_SHIELD, 0);
			char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_SHIELD);
			if (Getrand(100) < (int)dwParam)
			{
				if (p && p[0] != 0)
					SaySystem(p);
				return TRUE;
			}
			if (IsSystemFlagSeted(SF_GODBLESS))
				return IsGodBlessEffective(SG_SHIELD);
		}
	}
	break;
	case SG_IMMDIE:
	{
		if (this->IsSpecialEquipmentFunctionOn(SEF_IMMDIE))
		{
			DWORD dwParam = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_IMMDIE, 0);
			char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_IMMDIE);
			if (Getrand(100) < (int)dwParam)
			{
				if (p && p[0] != 0)
					SaySystem(p);
				return TRUE;
			}
			if (IsSystemFlagSeted(SF_GODBLESS))
				return IsGodBlessEffective(SG_IMMDIE);
		}
	}
	break;
	case SG_BIGHIT:
	{
		if (this->IsSpecialEquipmentFunctionOn(SEF_BIGHIT))
		{
			DWORD dwParam = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_BIGHIT, 0);
			char* p = CSpecialEquipmentManager::GetInstance()->GetFunctionString(SEF_BIGHIT);
			if (Getrand(100) < (int)dwParam)
			{
				if (p && p[0] != 0)
					SaySystem(p);
				return TRUE;
			}
			if (IsSystemFlagSeted(SF_GODBLESS))
				return IsGodBlessEffective(SG_BIGHIT);
		}
	}
	break;
	case SG_DOUBLEEXP:
	{
		if (IsSystemFlagSeted(SF_GODBLESS))
			return IsGodBlessEffective(SG_DOUBLEEXP);
	}
	break;
	}
	return FALSE;
}

BOOL CHumanPlayer::Damage(DWORD dwHitter, int value)
{
	if (this->IsSpecialEquipmentFunctionOn(SEF_PROTECT))
	{
		DecPropValue(PI_CURHP, value);
		if (GetPropValue(PI_CURHP) == 0)
		{
			if (IsNoDead() || !WillDie()) return FALSE;
			AddProcess(EP_DEAD, dwHitter, 0, 0, 0, 2);
		}
	}
	else
		return CAliveObject::Damage(dwHitter, value);
	return FALSE;
}

VOID CHumanPlayer::UpgradeWeapon()
{
	ITEM* pWeapon = GetEquipment(_U_WEAPON);
	if (pWeapon)
	{
		if (pWeapon->baseitem.bNeedIdentify)
		{
			int maxupgradetimes = CGameWorld::GetInstance()->GetVar(EVI_MAXUPGRADETIMES);
			if (pWeapon->baseitem.btUpgradeTimes >= maxupgradetimes || Getrand(maxupgradetimes * 10) < pWeapon->baseitem.btUpgradeTimes * 10 + 5)
			{
				ITEM item;
				if (m_Equipments.UnEquipItem(_U_WEAPON, item))
				{
					SendTakeBagItem(&item);
					SendWeaponBroken();
					CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
					UpdateProp();
					UpdateSubProp();
				}
				else
				{
					pWeapon->baseitem.bNeedIdentify = 0;
					pWeapon->dwParam[0] = 0;
					pWeapon->baseitem.btUpgradeTimes++;
					CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
					SendUpdateItem(*pWeapon);
				}
			}
			else
			{
				UPGRADEADDMASK mask;
				mask.dwValue = pWeapon->dwParam[0];
				pWeapon->dwParam[0] = 0;
				pWeapon->baseitem.bNeedIdentify = 0;
				pWeapon->baseitem.btUpgradeTimes++;
				switch (mask.addtype1)
				{
				case 0:
					pWeapon->baseitem.btMinAtk = static_cast<BYTE>(pWeapon->baseitem.btMinAtk + mask.addvalue1);
					break;
				case 1:
					pWeapon->baseitem.btMaxAtk = static_cast<BYTE>(pWeapon->baseitem.btMaxAtk + mask.addvalue1);
					break;
				case 2:
					pWeapon->baseitem.btMinMagAtk = static_cast<BYTE>(pWeapon->baseitem.btMinMagAtk + mask.addvalue1);
					break;
				case 3:
					pWeapon->baseitem.btMaxMagAtk = static_cast<BYTE>(pWeapon->baseitem.btMaxMagAtk + mask.addvalue1);
					break;
				case 4:
					pWeapon->baseitem.btMinSouAtk = static_cast<BYTE>(pWeapon->baseitem.btMinSouAtk + mask.addvalue1);
					break;
				case 5:
					pWeapon->baseitem.btMaxSouAtk = static_cast<BYTE>(pWeapon->baseitem.btMaxSouAtk + mask.addvalue1);
					break;
				case 6:
					pWeapon->baseitem.btMaxDef = static_cast<BYTE>(pWeapon->baseitem.btMaxDef + mask.addvalue1);
					break;
				}
				if (mask.adddura > 0)
				{
					if (mask.badddura)
						pWeapon->wMaxDura = static_cast<WORD>(pWeapon->wMaxDura + mask.adddura * 1000);
					else
					{
						if (pWeapon->wMaxDura > mask.adddura * 1000)
							pWeapon->wMaxDura = static_cast<WORD>(pWeapon->wMaxDura - mask.adddura * 1000);
						else
							pWeapon->wMaxDura = 0;
					}
				}
				pWeapon->wCurDura = pWeapon->wMaxDura;
				CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
				SendUpdateItem(*pWeapon);
				m_Equipments.ResetPropCache();
				UpdateProp();
				UpdateSubProp();
				SaySystemAttrib(CC_GREEN, getstrings(SD_UPGRADEWEAPONSUCCESS));
			}
		}
	}
}

VOID CHumanPlayer::OnDamageTarget(CAliveObject* pTarget, int nDamage, damage_type type)
{
	if (type == DT_PHYSICS)
	{
		int weapondamagerate = CGameWorld::GetInstance()->GetVar(EVI_WEAPONDAMAGERATE);
		if (weapondamagerate > 0)
		{
			int nWeaponDamage = (nDamage + weapondamagerate - 1) / weapondamagerate - GetPropValue(PI_HARD);
			if (nWeaponDamage > 0)
				DamageDura(_U_WEAPON, nWeaponDamage);
		}
		ITEM* pWeapon = GetEquipment(_U_WEAPON);
		if (pWeapon && pWeapon->baseitem.bNeedIdentify) // 鉴定武器
			UpgradeWeapon();
	}

	if (pTarget != nullptr)
	{
		CheckPk(pTarget);
		SetPetTarget(pTarget);
	}
	//	首先检测地图
	//	其次检测目标
	//	如果目标不是褐色和红色的人, 那么自己就算是恶意PK
	//	如果目标是怪物, 所有者是白色或者黄色的人, 那么自己也就算恶意PK
	int addhp = 0;
	//特殊装备-吸血
	static constexpr special_equipment_func huamoFlags[] = {
		SEF_SHENWU01, SEF_SHENWU02, SEF_SHENWU03, SEF_SHENWU04, SEF_SHENWU05, SEF_SHENWU06, SEF_SHENWU07, SEF_SHENWU08, SEF_SHENWU09, SEF_SHENWU10, //神武特殊装备
		SEF_SHIXUE01, SEF_SHIXUE02, SEF_SHIXUE03, SEF_SHIXUE04, SEF_SHIXUE05 //嗜血装备
	};
	for (special_equipment_func flag : huamoFlags)
	{
		if (this->IsSpecialEquipmentFunctionOn(flag))
		{
			if (Getrand(100) < (int)CSpecialEquipmentManager::GetInstance()->GetFunctionParam(flag, 0))
				addhp += ROUND(nDamage * CSpecialEquipmentManager::GetInstance()->GetFunctionParam(flag, 1) / 100);
		}
	}
	if (addhp > 0)
	{
		AddPropValue(PI_CURHP, addhp);
		SendHpMpChanged(-addhp, 13); // 吸血
	}
	CAliveObject::OnDamageTarget(pTarget, nDamage, type);
}

BOOL CHumanPlayer::TakeUpgradeWeapon()
{
	if (GetUpgradeItemComp()->Item.dwMakeIndex == 0)
	{
		SaySystem(getstrings(SD_NOUPGRADEWEAPON));
		return FALSE;
	}
	if (AddBagItem(GetUpgradeItemComp()->Item, FALSE, TRUE))
	{
		memset(&GetUpgradeItemComp()->Item, 0, sizeof(ITEM));
	}
	return TRUE;
}

#include "BundleManager.h"
#include "magicmanager.h"
VOID CHumanPlayer::UseItem(DWORD dwItemIndex, DWORD dwPackIndex)//吃物品
{
	ITEM* pItem = this->m_ItemBox.FindItem(dwItemIndex);
	if (pItem == nullptr)//如果物品的指针为空发送吃物品失败
	{
		SendEatFail();
		return;
	}
	ITEMCLASS* pItemClass = CItemManager::GetInstance()->GetItemClassPtr(*pItem);
	if (pItemClass == nullptr)
	{
		SendEatFail();
		return;
	}
	char* szFullName = pItemClass->szFullName;
	DWORD dwInterFlag = 0;
	std::vector<std::string> szExtraParams;
	if (GetMap() != nullptr && GetMap()->IsFlagSeted(MF_NOUSEITEM, dwInterFlag, szExtraParams))
	{
		for (size_t i = 0; i < szExtraParams.size(); i++)
		{
			if (szFullName == szExtraParams[i])
			{
				SaySystem("此地图禁止使用 %s", pItem->baseitem.szName);
				SendEatFail();
				return;
			}
		}
	}

	if (dwPackIndex != 0)
		m_pPackItem = this->m_ItemBox.FindItem(dwPackIndex);
	m_pUsingItem = pItem;
	BOOL bSuccess = TRUE;
	BOOL bUpdateItem = FALSE;

	char szItemName[20];
	o_strncpy(szItemName, pItem->baseitem.szName, 14);
	szItemName[14] = 0;
	if (_stricmp(szItemName, "豹魔石") == 0)
		m_baozhiID = dwItemIndex;
	ITEMCLASS* pClass = pItemClass; // 复用已查询的pItemClass, 避免双重查询不一致
	if (pClass && pClass->wPageId != 0)
	{
		const char* pszPage = CItemManager::GetInstance()->GetStringFromPool(pClass->wPageId);
		if (pszPage[0] != 0)
		{
			DWORD saveindex = pItem->dwMakeIndex;
			pItem->dwParam[3] = UR_NORESULT;
			CSystemScript::GetInstance()->Execute(this->GetScriptTarget(), pszPage, FALSE);
			// 如果当前使用的物品已经被清空
			if (pItem->dwParam[3] == UR_DELETED)
			{
				ITEM itemSnapshot = *m_pUsingItem;
				DeleteBagItem(m_pUsingItem);
				CItemManager::GetInstance()->DeleteItem(saveindex);
				SendTakeBagItem(&itemSnapshot);
				SendEatOk();
			}
			else
			{
				SendEatFail();
				if (pItem->dwParam[3] == UR_UPDATED)
					SendUpdateItem(*pItem);
			}
			m_pUsingItem = nullptr;
			return;
		}
	}
	switch (pItem->baseitem.btStdMode)
	{
	case ISM_BUNDLE:
	case ISM_OTHERBUNDLE:
	{
		char szName[20];
		o_strncpy(szName, pItem->baseitem.szName, pItem->baseitem.btNameLength);
		szName[pItem->baseitem.btNameLength] = 0;
		char	szExtractName[20] = { 0 };
		int iExtractCount = 0;
		if (CBundleManager::GetInstance()->GetBundleInfo(szName, szExtractName, iExtractCount))
		{
			if (iExtractCount > (m_ItemBox.GetFree() + 1))
			{
				SaySystem(getstrings(SD_NOSPACETOEXPANDBUNDLE));
				bSuccess = FALSE;
			}
			else
			{
				AddProcess(EP_CREATEBAGITEM, iExtractCount, 0, 0, 0, 0, 1, szExtractName);
			}
		}
		else
		{
			SaySystem(getstrings(SD_THISITEMCANNOTUNBUNDLE));
			bSuccess = FALSE;
		}
	}
	break;
	case ISM_USABLEITEM:
	{
		switch (pItem->baseitem.btShape)
		{
		case 1:	//	地牢
		{
			if (!EscapeMap())
			{
				bSuccess = FALSE;
				SaySystem(getstrings(SD_MAPLIMITED_NOESCAPE));
			}
		}
		break;
		case 2:	//	随机卷
		{
			if (m_pMap == nullptr || m_pMap->IsFlagSeted(MF_NORANDOMMOVE))
			{
				bSuccess = FALSE;
				SaySystem(getstrings(SD_MAPLIMITED_NORANDOMMOVE));
			}
			else
				AddProcess(EP_RANDOMTELEPORT, 0, 0, 0, 0, 100, 0, nullptr);
		}
		break;
		case 3:	//	回城卷
		{
			if (m_pMap == nullptr || m_pMap->IsFlagSeted(MF_NOHOME))
			{
				bSuccess = FALSE;
				SaySystem(getstrings(SD_MAPLIMITED_NOHOME));
			}
			else
				AddProcess(EP_HOME, 0, 0, 0, 0, 100, 0, nullptr);
		}
		break;
		case 4://祝福神油
		{
			ITEM* pWeapon = GetEquipment(_U_WEAPON);
			if (pWeapon)
			{
				//1- 选择加幸运 2- 选择加诅咒
				BYTE& b1 = pWeapon->baseitem.Ac1;
				BYTE& b2 = pWeapon->baseitem.Mac1;
				BOOL bChanged = FALSE;
				if (b1 >= 9 || b2 >= 9)
			{
				SaySystemAttrib(CC_RED, getstrings(SD_YOURWEAPONGOTFAIL));
				bSuccess = FALSE; // 已达上限不应消耗物品
				break;
			}
				int nRand = Getrand(100);
				if (nRand < 8) //诅咒计算
				{
					if (b1 > 0)
					{
						b1--;
						bChanged = TRUE;
					}
					else if (Getrand(100) > b2 * 10)
					{
						b2++;
						bChanged = TRUE;
					}
					if (bChanged)
						SaySystemAttrib(CC_GREEN, getstrings(SD_YOURWEAPONGOTEVIL));
				}
				else if(nRand < 10) //幸运计算
				{
					if (b2 > 0)
					{
						b2--;
						bChanged = TRUE;
					}
					else if (Getrand(100) > b1 * 10)
					{
						b1++;
						bChanged = TRUE;
					}
					if (bChanged)
						SaySystemAttrib(CC_GREEN, getstrings(SD_YOURWEAPONGOTLUCKY));
				}
				else
					SaySystemAttrib(CC_RED, getstrings(SD_YOURWEAPONGOTFAIL));
				if (bSuccess && bChanged)
				{
					CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
					SendUpdateItem(*pWeapon);
					m_Equipments.ResetPropCache();
					UpdateProp();
					UpdateSubProp();
				}
			}
			else
			{
				bSuccess = FALSE;
				SaySystem("你没有穿戴武器, 无法使用 祝福神油!");
			}
		}
		break;
		case 5://沙城回城卷
		{
			if (m_pGuild == nullptr || m_pGuild != CSandCity::GetInstance()->GetOwnerGuild() ||
				m_pMap == nullptr || m_pMap->IsFlagSeted(MF_NOSANDCITYHOME))
			{
				SaySystem(getstrings(SD_CANNOTUSESANDCITYHOME));//你不能使用沙城回城卷
				bSuccess = FALSE;
			}
			else
				AddProcess(EP_SABUKHOME, 0, 0, 0, 0, 2);//能够使用沙城回城卷
		}
		break;
		case 10://战士神油
		{
			ITEM* pEquipment = m_Equipments.GetEquipment(_U_WEAPON); //获取身上穿戴武器
			if (pEquipment)
			{
				if (pEquipment->wCurDura != pEquipment->wMaxDura)
				{
					pEquipment->wCurDura = pEquipment->wMaxDura;
					SendUpdateItem(*pEquipment);
					SendMsg(pEquipment->wCurDura, 0x282, _U_WEAPON, pEquipment->wMaxDura, 0);
				}
				SaySystemAttrib(CC_GREEN, "您的武器经过战士神油的修复,已经焕然一新");
				SendMsg(GetId(), 0x1fe, 1, 0, 3); // 身上特效
			}
			else
			{
				SaySystem("你没有穿戴武器, 无法使用 战士神油!");
				bSuccess = FALSE;
			}
		}
		break;
		case 12://特效神水
		{
			DWORD nTime = pItem->baseitem.Mac2 * 1000;
			switch (pItem->baseitem.wImageIndex)
			{
			case 218: //攻击神水
				AddProcess(EP_SPECIALPOTION, 0, pItem->baseitem.Dc1, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			case 219: //魔力神水
				AddProcess(EP_SPECIALPOTION, 1, pItem->baseitem.Mc1, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			case 220: //精神神水
				AddProcess(EP_SPECIALPOTION, 2, pItem->baseitem.Sc1, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			case 221: //极速神水
				AddProcess(EP_SPECIALPOTION, 3, pItem->baseitem.Ac2, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			case 207: //体力强效神水
				AddProcess(EP_SPECIALPOTION, 4, pItem->baseitem.Ac1, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			case 208: //魔力强效神水
				AddProcess(EP_SPECIALPOTION, 5, pItem->baseitem.Mac1, nTime, 0, nTime, 2, nullptr, TRUE);
			break;
			default: // 未知类型不消耗物品
				bSuccess = FALSE;
				break;
			}
		}
		break;
		default:
			bSuccess = FALSE;
		break;
		}
	}
	break;
	case ISM_DRUG:
	{
		//吃药
		if (pItem->baseitem.btShape == 0)
		{
			if (pItem->baseitem.btMinDef)
				SetAddHp(pItem->baseitem.btMinDef, 10);
			if (pItem->baseitem.btMinMagDef)
				SetAddMp(pItem->baseitem.btMinMagDef, 10);
		}
		else if (pItem->baseitem.btShape == 1)
		{
			if (pItem->baseitem.btMinDef)
				AddPropValue(PI_CURHP, pItem->baseitem.btMinDef);
			if (pItem->baseitem.btMinMagDef)
				AddPropValue(PI_CURMP, pItem->baseitem.btMinMagDef);
			SendHpMpChanged(-pItem->baseitem.btMinDef);
		}
		else if (pItem->baseitem.btShape == 5)
		{
			if (pItem->baseitem.wImageIndex == 751 && pItem->baseitem.wMc > 0) // 强化技能灵丹
			{
				USERMAGIC* pMagic = GetMagic(dwPackIndex); // dwPackIndex这是技能序号
				if (pMagic && this->TrainMagic(pMagic, pItem->baseitem.wMc))
					SaySystem("%s 技能在强化技能灵丹作用下增加了%d点经验!", pMagic->pClass->szName, pItem->baseitem.wMc);
				else
					bSuccess = FALSE; // 技能无效或训练失败不消耗物品
			}
			else
			{
				bSuccess = FALSE; // 非强化灵丹或配置异常不消耗物品
			}
		}
		else if (pItem->baseitem.btShape == 200)
		{
			//天山雪莲
			if (pItem->baseitem.wImageIndex == 793 && pItem->baseitem.btMinDef > 0 && pItem->baseitem.wDc > 0)
			{
				if (pItem->wCurDura > 0)
				{
					if (IsProcess(EP_RECOVERHP))
					{
						bSuccess = FALSE;
						break;
					}
					//每次加btMinDef点,总共加wDc秒
					AddProcess(EP_RECOVERHP, pItem->baseitem.btMinDef, 0, 0, 0, 1000, pItem->baseitem.wDc);
				SendMsg(GetId(), 510, 1, 1, 1);
				pItem->wCurDura--;
				if (pItem->wCurDura == 0)
					bSuccess = TRUE;
				else
				{
					bSuccess = FALSE;
					bUpdateItem = TRUE;
				}
				}
			}
			//深海灵礁
			if (pItem->baseitem.wImageIndex == 794 && pItem->baseitem.btMinMagDef > 0 && pItem->baseitem.wDc > 0)
			{
				if (pItem->wCurDura > 0)
				{
					if (IsProcess(EP_RECOVERMP))
					{
						bSuccess = FALSE;
						break;
					}
					//每次加btMinMagDef点,总共加wMac秒
					AddProcess(EP_RECOVERMP, pItem->baseitem.btMinMagDef, 0, 0, 0, 1000, pItem->baseitem.wDc);
				SendMsg(GetId(), 510, 1, 1, 2);
				pItem->wCurDura--;
				if (pItem->wCurDura == 0)
					bSuccess = TRUE;
				else
				{
					bSuccess = FALSE;
					bUpdateItem = TRUE;
				}
				}
			}
		}
		else if (pItem->baseitem.btShape == 201) // 治疗神水
		{
			if (pItem->wCurDura > 0)
			{
				if (pItem->baseitem.btMinDef)
					AddPropValue(PI_CURHP, pItem->baseitem.btMinDef);
				if (pItem->baseitem.btMinMagDef)
					AddPropValue(PI_CURMP, pItem->baseitem.btMinMagDef);
				SendHpMpChanged(-pItem->baseitem.btMinDef);
				SendMsg(GetId(), 0x1fe, 1, 0, 3); // 发送身上特效
				pItem->wCurDura--;
				if (pItem->wCurDura == 0)
					bSuccess = TRUE;
				else
				{
					bSuccess = FALSE;
					bUpdateItem = TRUE;
				}
			}
		}
	}
	break;
	case ISM_BOOK://技能书
	{
		if (pItem->baseitem.btShape != m_Humandesc.dbinfo.btClass)
		{
			SaySystem(getstrings(SD_STUDYSKILL_WRONGJOB));
			bSuccess = FALSE;
		}
		else if (pItem->baseitem.wMaxDura > GetPropValue(PI_LEVEL))
		{
			SaySystem(getstrings(SD_STUDYSKILL_LOWLEVEL));
			bSuccess = FALSE;
		}
		else
		{
			char szName[20];
			USERMAGIC* pMagic = nullptr;
			o_strncpy(szName, pItem->baseitem.szName, 14);
			MAGICCLASS* pClass = CMagicManager::GetInstance()->GetClassByName(szName);
			if (pClass == nullptr)
			{
				SaySystem("你的物品不正常!");
				bSuccess = FALSE;
			}
			else
			{
				char szNeedMagic[60] = "";
				MAGICCLASS* pCurClass = nullptr;
				for (int i = 0; i < 3; i++)
				{
					if (pClass->wNeedMagic[i] == 0)continue;
					pMagic = GetMagic(pClass->wNeedMagic[i]);
					if (pMagic == nullptr)
					{
						pCurClass = CMagicManager::GetInstance()->GetClassById(pClass->wNeedMagic[i]);
						if (pCurClass == nullptr)continue;
						if (szNeedMagic[0] == 0)
						{
							strncpy(szNeedMagic, pCurClass->szName, sizeof(szNeedMagic) - 1);
							szNeedMagic[sizeof(szNeedMagic) - 1] = '\0';
						}
						else
						{
							size_t curLen = strlen(szNeedMagic);
							size_t nameLen = strlen(pCurClass->szName);
							if (curLen + nameLen < sizeof(szNeedMagic) - 1)
								strcat(szNeedMagic, pCurClass->szName);
							else
								strncat(szNeedMagic, pCurClass->szName, sizeof(szNeedMagic) - 1 - curLen);
						}
						bSuccess = FALSE;
					}
					else
					{
						bSuccess = TRUE;
						break;
					}
				}
				if (bSuccess)
				{
					for (int i = 0; i < 3; i++)
					{
						pMagic = GetMagic(pClass->wMutexMagic[i]);
						if (pMagic != nullptr)
						{
							SaySystem("你已经学了%s, 不能再学习%s", pMagic->pClass->szName, pClass->szName);
							bSuccess = FALSE;
							break;
						}
					}
					if (bSuccess && GetMagic(pClass->id) != nullptr)
					{
						SaySystem("你已经学了%s, 不能重复学习", pClass->szName);
						bSuccess = FALSE;
					}
					if (bSuccess && !AddMagic(szName))
						bSuccess = FALSE;
				}
				else
					SaySystem("你不能学习%s, 在此之前你必须先学习%s", pClass->szName, szNeedMagic);
			}
		}
		break;
	}
	break;
	case 32:
	{
		ITEM* pMapai = GetEquipedHorseItem();//空马牌
		if (pMapai)
		{
			SaySystem(getstrings(SD_YOUALREADYEQUIPAHORSE));
			bSuccess = FALSE;
		}
		else if (m_pHorse == nullptr)
		{
			SaySystem(getstrings(SD_TRAINHORSE_NOHORSE));
			bSuccess = FALSE;
		}
		else
		{
			if (!AddProcess(EP_CATCHHORSE))
				bSuccess = FALSE;
			else
				bSuccess = TRUE;
		}
	}
	break;
	default:
		bSuccess = FALSE;
		break;
	}
	if (bSuccess)
	{
		ITEM itemSnapshot = *m_pUsingItem;
		DeleteBagItem(m_pUsingItem);
		CItemManager::GetInstance()->DeleteItem(dwItemIndex);
		SendTakeBagItem(&itemSnapshot);
		SendEatOk();
		pItem = nullptr; // 防止UAF, DeleteBagItem已释放pItem内存
	}
	else
		SendEatFail();
	if (bUpdateItem && pItem != nullptr) // 增加判空, 防止bSuccess时UAF
	{
		SendUpdateItem(*pItem); // 更新物品
		CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
	}
	m_pUsingItem = nullptr;
	m_pPackItem = nullptr; // 清理关联物品指针, 防止悬挂
}

BOOL CHumanPlayer::DoTrainHorse(int dir, int x, int y)
{
	if (!CanDoAction(AT_ATTACK) || m_pHorse != nullptr) return FALSE;
	ITEM* pWeapon = GetEquipment(_U_WEAPON);
	if (pWeapon == nullptr || pWeapon->baseitem.btShape != 40)	//	判断马鞭
		return FALSE;
	SetDirection((e_direction)(dir % 8));
	CMonsterEx* pHorse = (CMonsterEx*)m_pMap->FindObject(x, y, OBJ_MONSTER);
	if (pHorse == nullptr || pHorse->GetDesc() == nullptr || (pHorse->GetDesc()->sprop.pFlag & SF_CANBECALLED) != 0 || pHorse->GetDesc()->petset.Type != APT_RIDE)
	{
		return FALSE;
	}
	if (pHorse == this->m_pSeizedObject)
		this->m_iSeizedTimes++;
	else
	{
		m_pSeizedObject = pHorse;
		m_iSeizedTimes = 1;
	}
	if (pHorse->TrainOk(this, m_iSeizedTimes))
	{
		AddPet(pHorse);
		pHorse->SetOwner(this);
		SendMsg(0, 0x3e, 0, 0, 0); // 驯马成功
	}
	else
		pHorse->OnDamage(this, 0, DT_PHYSICS);
	m_tmrAttack.Savetime();
	return TRUE;
}

BOOL CHumanPlayer::IsEquipedHorse()
{
	ITEM* pItem = GetEquipment(_U_POISON);
	if (pItem && pItem->baseitem.btStdMode == 33)
		return TRUE;
	pItem = GetEquipment(_U_CHARM);
	if (pItem && pItem->baseitem.btStdMode == 33)
		return TRUE;
	return FALSE;
}

BOOL CHumanPlayer::TakeEquipment(const char* pszEquipment, ITEM& item)
{
	int pos = 0;
	ITEM* pItem = GetEquipment(pszEquipment, pos);
	if (pItem == nullptr)return FALSE;
	return TakeEquipment(pos, item);
}

BOOL CHumanPlayer::TakeEquipment(int pos, ITEM& item)
{
	if (m_Equipments[pos].dwMakeIndex == 0)return FALSE;
	if (!m_Equipments.UnEquipItem(pos, item))return FALSE;
	UpdateProp();
	UpdateSubProp();
	SendWeightChanged();
	return TRUE;
}

ITEM* CHumanPlayer::GetEquipment(const char* pszName, int& posout)
{
	for (int i = 0; i < _U_MAX; i++)
	{
		if (m_Equipments[i].dwMakeIndex != 0 && m_Equipments[i] == pszName)
		{
			posout = i;
			return &m_Equipments[i];
		}
	}
	return nullptr;
}

#include "humanplayermgr.h"
BOOL CHumanPlayer::AddTeacherCredit(UINT nCount)
{
	if (this->_master()[0] == 0)return FALSE;
	CHumanPlayer* pMaster = CHumanPlayerMgr::GetInstance()->FindbyName(_master().data());
	if (pMaster)
		return pMaster->GiveCredit(nCount);
	else
	{
		CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
		if (pObj)
			pObj->SendAddCredit(_master().data(), nCount);
	}
	return TRUE;
}

VOID CHumanPlayer::ShowPetInfo()
{
	PETINFO info[6] = { 0 };
	int count = this->m_iPetCount > 5 ? 5 : this->m_iPetCount;
	char szFullName[64] = { 0 };
	int i = 0;
	for (i = 0; i < count; i++)
	{
		if (this->m_pPets[i])
		{
			o_strncpy(szFullName, m_pPets[i]->GetName(), 14);
			char* Params[2];
			int nParam = SearchParam(szFullName, Params, 2, "级");
			if (nParam >= 2)
			{
				info[i].btLevel = StringToInteger(Params[0]);
				o_strncpy(info[i].sName, Params[1], 14);
			}
			info[i].wCurHp = m_pPets[i]->GetPropValue(PI_CURHP);
			info[i].wMaxHp = m_pPets[i]->GetPropValue(PI_MAXHP);
			info[i].dc1 = m_pPets[i]->GetPropValue(PI_MINDC);
			info[i].dc2 = m_pPets[i]->GetPropValue(PI_MAXDC);
			info[i].ac = m_pPets[i]->GetPropValue(PI_MAXAC);
			info[i].mac = m_pPets[i]->GetPropValue(PI_MAXMAC);
			info[i].flag = 0x2f;
		}
	}
	if (m_pPet && count < 6)
	{
		i = count;
		o_strncpy(szFullName, m_pPet->GetName(), 14);
		char* Params[2];
		int nParam = SearchParam(szFullName, Params, 2, "级");
		if (nParam >= 2)
		{
			info[i].btLevel = StringToInteger(Params[0]);
			o_strncpy(info[i].sName, Params[1], 14);
		}
		info[i].wCurHp = m_pPet->GetPropValue(PI_CURHP);
		info[i].wMaxHp = m_pPet->GetPropValue(PI_MAXHP);
		info[i].dc1 = m_pPet->GetPropValue(PI_MINDC);
		info[i].dc2 = m_pPet->GetPropValue(PI_MAXDC);
		info[i].ac = m_pPet->GetPropValue(PI_MAXAC);
		info[i].mac = m_pPet->GetPropValue(PI_MAXMAC);
		info[i].flag = 0x2f;
		count++;
	}
	SendMsg(0, 0x6858, 0, 0, 0, (LPVOID)info, sizeof(PETINFO) * count);
}

VOID CHumanPlayer::TransformInto(WORD wRace, WORD wImage, DWORD dwTime)
{
	DWORD dwParam = (wRace << 16) | wImage;
	if (IsSystemFlagSeted(SF_TRANSFORMED))
	{
		if (dwParam == GetSystemFlagParam(SF_TRANSFORMED))
		{
			SetSystemFlag(SF_TRANSFORMED, FALSE);
			SendStatusChanged();
			SendTitleChanged();
		}
		else
		{
			SendAroundMsg(GetId(), 0x532c, 8, 0, 0);
			SendMsg(GetId(), 0x532c, 8, 0, 0);
			SetSystemFlag(SF_TRANSFORMED, TRUE, dwParam, dwTime);
		}
	}
	else
	{
		if (m_bRideHorse)
			return;
		SendAroundMsg(GetId(), 0x532c, 8, 0, 0);
		SendMsg(GetId(), 0x532c, 8, 0, 0);
		SetSystemFlag(SF_TRANSFORMED, TRUE, dwParam, dwTime);
	}
}

BOOL CHumanPlayer::CutBody(UINT nMonsterId, WORD x, WORD y, WORD dir)
{
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetMonsterById(nMonsterId);
	if (this->GetActionType() != AT_STAND)return FALSE;
	if (pMonster == nullptr)return FALSE;
	if (!pMonster->IsDeath() || pMonster->IsSystemFlagSeted(SF_BONE))return FALSE;
	if (x != pMonster->getX() || y != pMonster->getY())return FALSE;

	if (nMonsterId != this->_cutMonsterId())
	{
		this->_cutMonsterId() = nMonsterId;
		this->_cutTimes() = 1;
	}
	else
		this->_cutTimes()++;

	UINT nReachTimes = pMonster->GetPropValue(PI_LEVEL) / 5 + 2;
	if (this->_cutTimes() >= nReachTimes)
	{
		this->_cutMonsterId() = 0;
		this->_cutTimes() = 0;
		pMonster->OnCuted(this);
	}
	return TRUE;
}

BOOL CHumanPlayer::CheckMaterial(BYTE stdMode, BYTE shape, BYTE special, int nCount)
{
	BOOL bOk = FALSE;
	ITEM* pEquipment = m_Equipments.GetEquipment(_U_ARMRINGR);
	if (pEquipment)
	{
		if (pEquipment->baseitem.btStdMode == stdMode && pEquipment->baseitem.btShape == shape &&
		pEquipment->baseitem.btSpecialpower == special)
			bOk = CItemManager::GetInstance()->CheckDura(*pEquipment, nCount);
	}
	if (bOk) return TRUE;
	pEquipment = m_Equipments.GetEquipment(_U_POISON);
	if (pEquipment)
	{
		if (pEquipment->baseitem.btStdMode == stdMode && pEquipment->baseitem.btShape == shape &&
		pEquipment->baseitem.btSpecialpower == special)
			bOk = CItemManager::GetInstance()->CheckDura(*pEquipment, nCount);
	}
	if (bOk) return TRUE;
	// 如果装备中没有找到, 则从背包中查找
	int count = m_ItemBox.GetCount();
	for (int i = 0; i < count; i++)
	{
		ITEM* pItem = m_ItemBox.GetItem(i);
		if (pItem && pItem->baseitem.btStdMode == stdMode && pItem->baseitem.btShape == shape &&
			pItem->baseitem.btSpecialpower == special)
		{
			if (CItemManager::GetInstance()->CheckDura(*pItem, nCount))
			{
				_materialBagPos() = i;
				return TRUE;
			}
		}
	}
	return bOk;
}

VOID CHumanPlayer::TakeMaterial(BYTE stdMode, BYTE shape, BYTE special, int nCount)
{
	BOOL bOk = FALSE;
	int pos = _U_ARMRINGR;
	ITEM* pEquipment = m_Equipments.GetEquipment(_U_ARMRINGR);
	if (pEquipment)
	{
		if (pEquipment->baseitem.btStdMode == stdMode &&
			pEquipment->baseitem.btShape == shape &&
			pEquipment->baseitem.btSpecialpower == special)
			bOk = TRUE;
	}

	if (!bOk)
	{
		pos = _U_POISON;
		pEquipment = m_Equipments.GetEquipment(_U_POISON);
		if (pEquipment)
		{
			if (pEquipment->baseitem.btStdMode == stdMode &&
				pEquipment->baseitem.btShape == shape &&
				pEquipment->baseitem.btSpecialpower == special)
				bOk = TRUE;
		}
	}
	if (bOk)
		DamageMaterialDura(pos, nCount);
	else if (_materialBagPos() >= 0) // 检查是否是从背包中使用材料
	{
		ITEM* pItem = m_ItemBox.GetItem(_materialBagPos());
		if (pItem && pItem->baseitem.btStdMode == stdMode && pItem->baseitem.btShape == shape &&
			pItem->baseitem.btSpecialpower == special)
		{
			CItemManager::GetInstance()->DamageDura(*pItem, nCount);
			if (pItem->dwParam[3] == UR_DELETED)
			{
				CItemManager::GetInstance()->DeleteItem(pItem->dwMakeIndex);
				SendTakeBagItem(pItem);
			}
			if (pItem->dwParam[3] == UR_UPDATED)
				SendUpdateItem(*pItem);
			_materialBagPos() = -1; // 重置位置标记
		}
	}
}

VOID CHumanPlayer::GetPrivateShopView(PRIVATESHOPHEADER& header)
{
	o_strncpy(header.szName, _shopName().data(), 50);
	header.dw1 = this->_shopSign();
	header.w1 = this->_shopStyle();
	header.w2 = 0;
	header.btFlag = static_cast<BYTE>(this->_shopFlags());
}

VOID CHumanPlayer::SendStartPrivateShop()
{
	PRIVATESHOPHEADER psheader;
	this->GetPrivateShopView(psheader);
	psheader.w2 = 0;
	DWORD* pdwParam = (DWORD*)&psheader.w1;
	SendAroundMsg(GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psheader, sizeof(psheader));
	SendMsg(GetId(), 0x80d7, getX(), getY(), (WORD)GetDirection(),
		(LPVOID)pdwParam, sizeof(DWORD) * 2);
}

VOID CHumanPlayer::SendStopPrivateShop()
{
	PRIVATESHOPHEADER psheader;
	this->GetPrivateShopView(psheader);
	psheader.w2 = 0xff;
	DWORD* pdwParam = (DWORD*)&psheader.w1;
	SendAroundMsg(GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psheader, sizeof(psheader));
	SendMsg(GetId(), 0x80d7, getX(), getY(), (WORD)GetDirection(),
		(LPVOID)pdwParam, sizeof(DWORD) * 2);
}

VOID CHumanPlayer::OnPutItem(DWORD dwShellId, DWORD dwMakeIndex)
{
	m_pUsingItem = m_ItemBox.FindItem(dwMakeIndex);
	if (m_pUsingItem == nullptr)return;
	m_pUsingItem->dwParam[3] = UR_NORESULT;
	this->m_ScriptTarget.OnPutItem(dwShellId, dwMakeIndex);
	if (m_pUsingItem->dwParam[3] == UR_DELETED)
	{
		ITEM tempitem = *m_pUsingItem;
		DeleteBagItem(m_pUsingItem);
		CItemManager::GetInstance()->DeleteItem(tempitem.dwMakeIndex);
		SendTakeBagItem(m_pUsingItem);
		SendMsg(1, 0x9590, 0, 0, 0);
	}
	else if (m_pUsingItem->dwParam[3] == UR_UPDATED)
		SendUpdateItem(*m_pUsingItem);
	SendMsg(1, 0x9590, 0xff, 0, 0);
	m_pUsingItem = nullptr;
}

VOID CHumanPlayer::SetBagItemPos(BAGITEMPOS* pPosArray, int count)
{
	ITEM* pItem = nullptr;
	for (int i = 0; i < count; i++)
	{
		pItem = m_ItemBox.FindItem(pPosArray[i].ItemId);
		if (pItem != nullptr)
			pItem->dwParam[2] = pPosArray[i].wPos;
	}
}

static thread_local std::array<ITEM, 100> s_dbSyncItems{};
WORD CHumanPlayer::GetBagItemPos(DWORD dwMakeIndex)
{
	int count = 0;
	count = m_ItemBox.GetItems(s_dbSyncItems.data(), 100);
	ITEM* pItem = nullptr;
	for(int i = 0;i < count;i ++ )
	{
		if(s_dbSyncItems[i].dwMakeIndex == dwMakeIndex )
		{
			pItem = m_ItemBox.FindItem(s_dbSyncItems[i].dwMakeIndex);
			return static_cast<WORD>(pItem->dwParam[2]);
		}
	}
	return 0;
}

static thread_local std::array<DBITEM, 100> s_dbSyncDst{};
static thread_local std::array<BAGITEMPOS, 100> s_dbSyncPos{};
VOID CHumanPlayer::UpdateItemsToDB()
{
	int count = 0;
	count = m_ItemBox.GetItems(s_dbSyncItems.data(), 100);
	int updatecount = 0;
	int uposcount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbSyncItems[i].dwMakeIndex & 0x80000000)
		{
			CItemManager::GetInstance()->DeleteItem(s_dbSyncItems[i].dwMakeIndex);
			if (CItemManager::GetInstance()->ItemLimited(s_dbSyncItems[i], IL_NOUPDATETODB))continue;
			if (CItemManager::GetInstance()->ItemLimited(s_dbSyncItems[i], IL_TRACEDITEM))
			{
				DropItem(s_dbSyncItems[i]);
				continue;
			}
			s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
			s_dbSyncDst[updatecount].btFlag = 0;
			s_dbSyncDst[updatecount].pos = static_cast<WORD>(s_dbSyncItems[i].dwParam[2]);
			updatecount++;
		}
		else
		{
			BYTE btFlag = static_cast<BYTE>(s_dbSyncItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
				s_dbSyncDst[updatecount].btFlag = btFlag;
				s_dbSyncDst[updatecount].pos = static_cast<WORD>(s_dbSyncItems[i].dwParam[2]);
				updatecount++;
			}
			else
			{
				s_dbSyncPos[uposcount].ItemId = s_dbSyncItems[i].dwMakeIndex;
				s_dbSyncPos[uposcount].wPos = static_cast<WORD>(s_dbSyncItems[i].dwParam[2]);
				uposcount++;
			}
		}
	}
	if (updatecount > 0)
		CItemManager::GetInstance()->UpdateItems(GetDBId(), IDF_BAG, s_dbSyncDst.data(), updatecount);

	//背包的其他物品的位置要更新～
	if (uposcount > 0)
		CItemManager::GetInstance()->UpdateItemPos(IDF_BAG, s_dbSyncPos.data(), uposcount);
	//仓库数据
	count = m_ItemBank.GetItems(s_dbSyncItems.data(), 100);
	updatecount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbSyncItems[i].dwMakeIndex & 0x80000000)
		{
			CItemManager::GetInstance()->DeleteItem(s_dbSyncItems[i].dwMakeIndex);
			if (CItemManager::GetInstance()->ItemLimited(s_dbSyncItems[i], IL_NOUPDATETODB))continue;
			s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
			s_dbSyncDst[updatecount].btFlag = 0;
			s_dbSyncDst[updatecount].pos = i;
			updatecount++;
		}
		else
		{
			BYTE btFlag = (s_dbSyncItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
				s_dbSyncDst[updatecount].btFlag = btFlag;
				s_dbSyncDst[updatecount].pos = i;
				updatecount++;
			}
		}
	}
	if (updatecount > 0)
		CItemManager::GetInstance()->UpdateItems(GetDBId(), IDF_BANK, s_dbSyncDst.data(), updatecount);
	//	宠物背包
	count = m_ItemPetBag.GetItems(s_dbSyncItems.data(), 100);
	updatecount = 0;
	for (int i = 0; i < count; i++)
	{
		if (s_dbSyncItems[i].dwMakeIndex & 0x80000000)
		{
			CItemManager::GetInstance()->DeleteItem(s_dbSyncItems[i].dwMakeIndex);
			if (CItemManager::GetInstance()->ItemLimited(s_dbSyncItems[i], IL_NOUPDATETODB))continue;
			s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
			s_dbSyncDst[updatecount].btFlag = 0;
			s_dbSyncDst[updatecount].pos = i;
			updatecount++;
		}
		else
		{
			BYTE btFlag = (s_dbSyncItems[i].baseitem.wFeature & 0x00f0);
			if (btFlag != 0)
			{
				s_dbSyncDst[updatecount].item = s_dbSyncItems[i];
				s_dbSyncDst[updatecount].btFlag = btFlag;
				s_dbSyncDst[updatecount].pos = i;
				updatecount++;
			}
		}
	}
	if (updatecount > 0)
		CItemManager::GetInstance()->UpdateItems(GetDBId(), IDF_PETBANK, s_dbSyncDst.data(), updatecount);
	//穿戴装备数据
	ITEM* pEquipment = nullptr;
	updatecount = 0;
	for (int i = 0; i < _U_MAX; i++)
	{
		pEquipment = m_Equipments.GetEquipment(i);
		if (pEquipment)
		{
			if (pEquipment->dwMakeIndex & 0x80000000)
			{
				CItemManager::GetInstance()->DeleteItem(pEquipment->dwMakeIndex);
				if (CItemManager::GetInstance()->ItemLimited(*pEquipment, IL_NOUPDATETODB))continue;
				s_dbSyncDst[updatecount].item = *pEquipment;
				s_dbSyncDst[updatecount].btFlag = 0;
				s_dbSyncDst[updatecount].pos = i;
				updatecount++;
			}
			else
			{
				BYTE btFlag = (pEquipment->baseitem.wFeature & 0x00f0);
				if (btFlag != 0)
				{
					s_dbSyncDst[updatecount].item = *pEquipment;
					s_dbSyncDst[updatecount].btFlag = btFlag;
					s_dbSyncDst[updatecount].pos = i;
					updatecount++;
				}
			}
		}
	}
	if (updatecount > 0)
		CItemManager::GetInstance()->UpdateItems(GetDBId(), IDF_EQUIPMENT, s_dbSyncDst.data(), updatecount);
	//	存储升级临时武器的物品
	if ((this->GetUpgradeItemComp()->Item.dwMakeIndex & 0x80000000) &&
		!CItemManager::GetInstance()->ItemLimited(this->GetUpgradeItemComp()->Item, IL_NOUPDATETODB))
	{
		updatecount = 0;
		s_dbSyncDst[updatecount].item = GetUpgradeItemComp()->Item;
		s_dbSyncDst[updatecount].btFlag = 0;
		s_dbSyncDst[updatecount].pos = 0;
		updatecount++;
		CItemManager::GetInstance()->UpdateItems(GetDBId(), IDF_UPGRADE, s_dbSyncDst.data(), updatecount);
	}
}

VOID CHumanPlayer::OnPetBank(DBITEM* pItems, UINT nCount)
{
	if (nCount > 10)nCount = 10;
	if (nCount > 0)
	{
		for (UINT i = 0; i < nCount; i++)
		{
			m_ItemPetBag.AddItem(pItems[i].item);
		}
	}
	this->SetSystemFlag(SF_PETBANKLOADED, TRUE);
}

BOOL CHumanPlayer::RepairEquipment(UINT pos, UINT nCount)
{
	ITEM* pEquipment = nullptr;
	BOOL IsOK = FALSE;
	SendMsg(GetId(), 0x1fe, 228, 238, 3); // 使用冰泉圣水音效
	for (UINT i = pos; i < pos + nCount; i++)
	{
		pEquipment = m_Equipments.GetEquipment(i);
		if (pEquipment)
		{
			if (pEquipment->wCurDura != pEquipment->wMaxDura)
			{
				pEquipment->wCurDura = pEquipment->wMaxDura;
				SendUpdateItem(*pEquipment);
				SendMsg(pEquipment->wCurDura, 0x282, i, pEquipment->wMaxDura, 0);
			}
			IsOK = TRUE;
		}
	}
	if (IsOK)
	{
		SaySystem("经过冰泉圣水的洗礼,您的装备已经焕然一新");
		SendMsg(GetId(), 0x1fe, 1, 0, 3); // 身上特效
	}
	return IsOK;
}

BOOL CHumanPlayer::RecordHomeXY(const char* pszName)
{
	ITEM* pItem = nullptr;
	if ((pItem = GetUsingItem()) == nullptr)return FALSE;
	START_POINT* pStartPoint = CGameWorld::GetInstance()->GetStartPoint(pszName);
	if (pStartPoint)
	{
		CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(pStartPoint->mapid);
		if (pMap == nullptr)return 0;
		if (pMap->GetPhysicsMap() == nullptr)return 0;
		memcpy(&pItem->baseitem.Dc1, (VOID*)pMap->GetPhysicsMap()->GetName(), 8);
		pItem->baseitem.wAc = pStartPoint->x;
		pItem->baseitem.wMac = pStartPoint->y;
		pItem->baseitem.wMapId = pStartPoint->mapid;
		pItem->dwParam[3] = UR_UPDATED;
		CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
		return TRUE;
	}
	return FALSE;
}