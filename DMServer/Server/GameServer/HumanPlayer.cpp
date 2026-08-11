#include "StdAfx.h"
#include "humanplayer.h"
#include "SpecialEquipmentManager.h"
#include "StringListManager.h"
#include "gsclientobj.h"
#include "logicmap.h"
#include "server.h"
#include "downitemmgr.h"
#include "equipment.h"
#include "itemmanager.h"
#include "ConSkill.h"
#include "gameworld.h"
#include "npcmanager.h"
#include "scriptnpc.h"
#include "math.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "ExchangeObjectMgr.h"
#include "humanplayermgr.h"
#include "groupobjectmgr.h"
#include "groupobject.h"
#include "titlemanager.h"
#include "bundlemanager.h"
#include "logicmapmgr.h"
#include "FireBurnEvent.h"
#include "guildmanagerex.h"
#include "guildex.h"
#include "sandcity.h"
#include "topmanager.h"
#include "systemscript.h"
#include "scriptvariable.h"
#include "ScriptDef.h"

extern DWORD g_dwActionDelay[AT_MAX];
CHumanPlayer::CHumanPlayer(VOID) :m_pClientObj(nullptr), m_Equipments(this), m_ScriptTarget(this)
{
	m_ItemBox.Create(BIGBAG_SLOT); // 玩家背包大小
	m_ItemBank.Create(STOREAGE_SLOT); // 玩家仓库大小
	m_ItemPetBag.Create(PETBAG_SLOT); // 豹子背包大小
	m_iHuoli = 0;
	m_nRecalcHit = 0;
	m_nRecalcSpeed = 0;
	m_pHorse = 0;
	m_bRideHorse = FALSE;
	ISzhaohuan = FALSE;
	m_bEnterMap = FALSE;
	m_bFirstLogin = FALSE;
	petname.fill(0);
	m_nMaterialBagPos = -1;
	boPoison = FALSE;
	Clean();
}

CHumanPlayer::~CHumanPlayer(VOID)
{
}

VOID CHumanPlayer::Clean()
{
	memset(&m_TaskInfo, 0, sizeof(m_TaskInfo));
	memset(&m_xAbilityShellRef, 0, sizeof(m_xAbilityShellRef));
	ClearParam();
	this->m_ScriptTarget.Clean();
	m_vMagic.clear();
	m_fMagicLoaded = FALSE;
	ClearAllConSkillBuffs(this);
	m_iPetCount = 0;
	CAliveObject::Clean();
	m_DBTimer.Savetime();
	m_StaminaTimer.Savetime();
	m_tmrSpecialAttackSkill.Savetime();
	m_tmrMine.Savetime();
	m_tmrGameTime.Savetime();
	m_tmrUseItem.Savetime();
	m_tmrPickupItem.Savetime();
	m_tmrDropItem.Savetime();
	m_tmrEquipChange.Savetime();
	m_pExchangeObj = nullptr;
	m_pGroupObject = nullptr;
	m_dwStartPointIndex = 0;
	m_bFirstEnterMap = TRUE;
	m_szCurrentTitle.fill(0);
	m_iCurrentTitleIndex = 0;
	for (auto& param : m_sParam) param.fill(0);
	m_vParam.fill(0);
	m_ItemBox.Clean();
	m_ItemBank.Clean();
	m_ItemPetBag.Clean();
	m_pAutoMagic.fill(nullptr);
	m_iAutoMagicCount = 0;
	m_fExpFactor = 1.0f;
	m_pExpMagic = nullptr;
	m_szCurWisperTarget[0] = 0;
	memset(m_bChatChannelDisabled.data(), 0, sizeof(m_bChatChannelDisabled));
	SetGuild(nullptr);
	m_szGuildTitle.fill(0);
	m_iGuildTitleLevel = 0;
	m_pAddToGuildRequester = nullptr;
	m_pHorse = nullptr;
	m_bRideHorse = FALSE;
	m_HorseTimer.Savetime();
	m_dwPkValue = 0;
	m_bJustPk = FALSE;
	m_pSeizedObject = nullptr;
	m_iSeizedTimes = 0;
	m_szGuildTitle.fill(0);

	m_iPrivateShopItemCount = 0;
	SetCurPrivateShopView(nullptr);
	m_sMaster[0] = 0;
	m_sWife[0] = 0;
	for (auto& student : m_sStudents) student.fill(0);
	for (auto& friendName : m_sFriends) friendName.fill(0);

	m_bRefuseAddFriend = FALSE;
	m_pTimeOutDeActiveMagic = nullptr;

	m_dwSpecialEquipmentFunctionFlags.fill(0);
	m_dwMineCounter = 0;
	
	memset(&m_UpgradeItem, 0, sizeof(m_UpgradeItem));

	m_bHorseRest = FALSE;
	m_pUsingItem = nullptr;
	m_pPackItem = nullptr;
	m_pPutItem = nullptr;
	m_nCutMonsterId = 0;
	m_wPrivateShopStyle = 0;
	m_wPrivateShopFlags = 0;
	m_wPrivateShopSign = 0;
	m_btChatColor = 1;
	m_pPet = nullptr;
	m_dwPetKey = 0;
	m_wPetSkill = 0;
	m_baozhiID = 0;
	m_dwZhenBaoExpMax = 0;
	m_dwZhenBaoStar = 0;
	m_wYuanQi = 2000; // 临时把它默认最大，就不会一直发元气值更新，因为客户端现在不支持元气值
	m_bYuanQi = FALSE;
	m_wStamina = 0;
	m_wMaxStamina = 2000;
}

BOOL CHumanPlayer::Init(CREATEHUMANDESC& desc)
{
	//对刚出生的角色进行初始化
	m_pClientObj = desc.pClientObj;

	int tx, ty, tmapid;
	HUMANDATADESC* pHumanData = nullptr;
	FIRSTLOGIN_INFO* pFirstLoginInfo = nullptr;
	if (desc.dbinfo.wLevel == 0) //如果DBINFO的等级为0
	{
		m_bFirstLogin = TRUE;
		pFirstLoginInfo = CGameWorld::GetInstance()->GetFirstLoginInfo();
		pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(desc.dbinfo.btClass, pFirstLoginInfo->level); // 加载角色基础数据
	}
	else
		pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(desc.dbinfo.btClass, desc.dbinfo.wLevel); // 加载角色基础数据
	if (pHumanData == nullptr) return FALSE;
	m_pHumanDataDesc = pHumanData;
	START_POINT* pt = nullptr;
	if (m_bFirstLogin)
	{
		desc.dbinfo.wLevel = pFirstLoginInfo->level;
		BOOL bRet = CGameWorld::GetInstance()->GetBornPoint(desc.dbinfo.btClass, tmapid, tx, ty, desc.dbinfo.szStartPoint);
		assert(bRet);
		if (!bRet) return FALSE;
		desc.dbinfo.x = tx;
		desc.dbinfo.y = ty;
		desc.dbinfo.mapid = tmapid;
		desc.dbinfo.hp = pHumanData->wHp;
		desc.dbinfo.mp = pHumanData->wMp;
		desc.dbinfo.maxhp = pHumanData->wHp;
		desc.dbinfo.maxmp = pHumanData->wMp;
		desc.dbinfo.btAttackMode = HAM_PEACE;
		desc.dbinfo.dwGold = pFirstLoginInfo->dwGold;
		desc.dbinfo.dwYuanbao = pFirstLoginInfo->dwYuanBao;
		desc.dbinfo.dwCurExp = 0;
		desc.dbinfo.handweight = pHumanData->handweight;
		desc.dbinfo.weight = pHumanData->bagweight;
		desc.dbinfo.bodyweight = pHumanData->bodyweight;
		desc.dbinfo.maxac = pHumanData->maxac;
		desc.dbinfo.minac = pHumanData->minac;
		desc.dbinfo.maxmac = pHumanData->maxmac;
		desc.dbinfo.minmac = pHumanData->minmac;
		desc.dbinfo.maxmc = pHumanData->maxmc;
		desc.dbinfo.minmc = pHumanData->minmc;
		desc.dbinfo.maxsc = pHumanData->maxsc;
		desc.dbinfo.minsc = pHumanData->minsc;
		desc.dbinfo.maxdc = pHumanData->maxdc;
		desc.dbinfo.mindc = pHumanData->mindc;

		desc.dbinfo.btAttackMode = HAM_PEACE;
		desc.dbinfo.nGameTime = pFirstLoginInfo->nGameTime;
	}
	else
	{
		desc.dbinfo.maxhp = pHumanData->wHp;
		desc.dbinfo.maxmp = pHumanData->wMp;
		if (desc.dbinfo.hp > desc.dbinfo.maxhp)
			desc.dbinfo.hp = desc.dbinfo.maxhp;
		if (desc.dbinfo.mp > desc.dbinfo.maxmp)
			desc.dbinfo.mp = desc.dbinfo.maxmp;
		desc.dbinfo.handweight = pHumanData->handweight;
		desc.dbinfo.weight = pHumanData->bagweight;
		desc.dbinfo.bodyweight = pHumanData->bodyweight;
		desc.dbinfo.maxac = pHumanData->maxac;
		desc.dbinfo.minac = pHumanData->minac;
		desc.dbinfo.maxmac = pHumanData->maxmac;
		desc.dbinfo.minmac = pHumanData->minmac;
		desc.dbinfo.maxmc = pHumanData->maxmc;
		desc.dbinfo.minmc = pHumanData->minmc;
		desc.dbinfo.maxsc = pHumanData->maxsc;
		desc.dbinfo.minsc = pHumanData->minsc;
		desc.dbinfo.maxdc = pHumanData->maxdc;
		desc.dbinfo.mindc = pHumanData->mindc;
	}

	if (desc.dbinfo.hp == 0)
	{
		desc.dbinfo.hp = GetRangeRand(1, 10);
		BOOL bRet = CGameWorld::GetInstance()->GetStartPoint(desc.dbinfo.szStartPoint, tmapid, tx, ty);
		if (!bRet)
			bRet = CGameWorld::GetInstance()->GetBornPoint(desc.dbinfo.btClass, tmapid, tx, ty, desc.dbinfo.szStartPoint);
		if (!bRet) return FALSE;
		desc.dbinfo.x = tx;
		desc.dbinfo.y = ty;
		desc.dbinfo.mapid = tmapid;
	}

	START_POINT* p = CGameWorld::GetInstance()->GetStartPoint(desc.dbinfo.szStartPoint);
	if (p != nullptr)
		this->m_dwStartPointIndex = p->index;
	m_Humandesc = desc;
	setXY(desc.dbinfo.x, desc.dbinfo.y);
	SetMapId(desc.dbinfo.mapid);
	SetDirection((e_direction)desc.dbinfo.dir);
	int iBagCount = 0;
	if (desc.dbinfo.bBigBag)
		iBagCount = BIGBAG_SLOT;
	else
		iBagCount = SMALLBAG_SLOT;
	m_ItemBox.SetCountLimit(iBagCount);
	this->m_ItemBank.SetCountLimit(STOREAGE_SLOT);
	m_Equipments.Clean();
	m_iHuoli = 0;
	m_nVisibleObjectFlag = 0;
	//添加玩家可以看到的对象类型
	for (int i = 0; i < OBJ_MAX; i++)
	{
		e_object_type objTye = (e_object_type)i;
		if (objTye == OBJ_EVENT) continue;
		AddVisibleObjectType(objTye);
	}

	Sendfirstdlg(CGameWorld::GetInstance()->GetNotice());//658 发送登录窗口提示
	SendMsg(0, 0x9591, 0, 0, 0, "9, 9, 9, 9"); // 发送版本号
	SendMsg(0, 0x328, 1, 0, 0); // 服务器通知客户端是否使用动态加密算法, 以及动态加密数据的长度设置
	SendMsg(0, 0x9594, 0, iBagCount, MAXBOOT_SLOT);//38292 发送背包大小
	SendMsg(GetId(), 0x9593, 1, 0, desc.dbinfo.wPersonCode, (LPVOID)desc.dbinfo.szPersonSign);//38291 设置个性化签名
	SendMsg(GetId(), 0x9593, 2, 0, 0, (LPVOID)desc.dbinfo.szTempRank);//38291 设置临时称号
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(desc.dbinfo.mapid);
	if (pMap == nullptr) return FALSE;
	SendMsg(GetId(), SM_SETMAP, m_wX, m_wY, 0, (LPVOID)pMap->GetName());//人物在地图的位置
	UpdateProp(); // 52人物信息
	RecalcHitSpeed(); // 计算命中、躲避
	UpdateSubProp(); // 752附加属性
	SendStatusChanged(); // 657发送状态改变
	SaySystemAttrib(CC_GREEN, "[%s攻击模式]", g_pAttackModeDesc[m_Humandesc.dbinfo.btAttackMode]);//攻击模式
	SaySystemAttrib(CC_GREEN, getstrings(SD_CHANGEATTACKMODE)); // 发送更改攻击模式, CTR-H
	SendTimeWeatherChanged(); // 发送天气时间
	CheckAndUpgradeTitle(); // 发送窗口标题
	SendGroupMode(); // 发送组队状态
	SendMoneyChanged(MT_YUANBAO); // 发送元宝数量
	if (m_Humandesc.dbinfo.szGuildName[0] != 0)
	{
		CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(m_Humandesc.dbinfo.szGuildName);
		if (pGuild)
			pGuild->MemberLogon(this);
		else
			PRINT(ERROR_RED, "%s 找不到自己所在的行会 %s, 请检查是否是数据转移的问题!\n", m_Humandesc.dbinfo.szName, m_Humandesc.dbinfo.szGuildName);
	}
	ChangeChatChannel(CCH_NORMAL);
	SaySystemAttrib( CC_GREEN, "更改频道 CTRL+S 查看频道信息 @ccinfo" );
	m_tmrPkTimer.Savetime();
	m_dwPkValue = m_Humandesc.dbinfo.dwFlag[1];
	UpdateViewName();
	m_LoginTime = CSystemTime();
	return TRUE;
}

DWORD CHumanPlayer::GetFeather()
{
	BYTE btShape = m_Equipments[_U_WEAPON].baseitem.btShape;
	if (m_Equipments[_U_WEAPON].baseitem.btStdMode == 5 && btShape == 40) // 马鞭外观值, 重写
		btShape--;
	if (m_Equipments[_U_WEAPON].baseitem.btStdMode == 6 && btShape == 19) // 鹤嘴锄外观值, 重写
		btShape = btShape + 6;

	BYTE btHair = m_Equipments[_U_HELMET].baseitem.btShape;
	//四个字节分别是：bGender | (bWeapon << 8) | (bHair << 16) | (bDress << 24)
	return MAKEFEATHER(
		GetShape(), // 衣服外观
		(m_Equipments[_U_HELMET].dwMakeIndex != 0 ? btHair : m_Humandesc.dbinfo.btHair), // 头发外观
		(m_Equipments[_U_WEAPON].dwMakeIndex != 0 ? btShape : 0), // 武器外观
		(m_bRideHorse && m_pHorse) ? ((m_pHorse->GetFeather() & 0xff0000) >> 16) + 0x40 : 0
	);
}

DWORD CHumanPlayer::GetHealth()
{
	return 0x00640064;
}

DWORD CHumanPlayer::GetStatus()
{
	return m_Status.GetFlag();
}

BYTE CHumanPlayer::GetShape()
{
	BYTE temp = m_Equipments[_U_DRESS].baseitem.btShape > 0 ? m_Equipments[_U_DRESS].baseitem.btShape : 0;
	return temp;
}

BOOL CHumanPlayer::GetDBInfo(CHARDBINFO& info)
{
	info = m_Humandesc.dbinfo;
	info.x = m_wX;
	info.y = m_wY;
	info.mapid = m_Mapid;
	info.dir = m_Direction;
	info.dwFlag[1] = GetPkValue(); // PK值
	return TRUE;
}

BOOL CHumanPlayer::AddBagItem(ITEM& item, BOOL bSilence, BOOL bUpdateDB, BOOL bUpdateWeight)
{
	// 检查重量限制
	if (!bSilence && !CanBearItem(item))
	{
		SaySystem("负重满了，不能放到背包!");
		return FALSE;
	}
	if (m_ItemBox.AddItem(item))
	{
		if (!bSilence && m_pClientObj != nullptr)
			m_pClientObj->SendAddItem(item);
		if (bUpdateWeight)
			SendWeightChanged();
		if (bUpdateDB)
			CItemManager::GetInstance()->UpdateItemOwner(GetDBId(), item.dwMakeIndex, IDF_BAG, 0);
		return TRUE;
	}
	return FALSE;
}

//子类实现虚函数, 豹子拾取物品到主人背包
BOOL CHumanPlayer::PetAddBagItem(ITEM& item, BOOL bSilence, BOOL bUpdateDB, BOOL bUpdateWeight)
{
	return AddBagItem(item, bSilence, bUpdateDB, bUpdateWeight);
}

BOOL CHumanPlayer::DropItem(ITEM& item)
{
	if (item.IsBind())
	{
		SaySystem("%s 是绑定物品, 不能丢弃!", item.baseitem.szName);
		return FALSE;
	}
	POINT pt;
	BOOL bFlag = FALSE;
	if (m_pMap == nullptr) return FALSE;
	int count = m_pMap->GetDropItemPoint(getX(), getY(), &pt, 1);
	if (count > 0)
	{
		bFlag = CDownItemMgr::GetInstance()->DropItem(m_pMap, item, static_cast<WORD>(pt.x), static_cast<WORD>(pt.y), FALSE, this);
		if (bFlag)
			OnDropItem(item, pt.x, pt.y);
	}
	return bFlag;
}

BOOL CHumanPlayer::PickupItem()
{
	if (m_pMap == nullptr) return FALSE;
	CDownItemObject* pObj = (CDownItemObject*)m_pMap->FindObject(m_wX, m_wY, OBJ_DOWNITEM);
	if (pObj == nullptr)return FALSE;
	if (!pObj->UpdateValid()) return FALSE;
	if (!pObj->IsGold())
	{
		if (m_ItemBox.GetFree() == 0)
		{
			SaySystem(getstrings(SD_PICKUP_BAGFULL));
			return FALSE;
		}
		if (!CanBearItem(pObj->GetItem()))
		{
			SaySystem(getstrings(SD_PICKUP_WEIGHTFULL));
			return FALSE;
		}
	}
	DWORD dwOwnerId = pObj->GetOwnerId();
	if (dwOwnerId == 0 || dwOwnerId == this->GetDBId() || (m_pGroupObject != nullptr && m_pGroupObject->IsMemberDBId(dwOwnerId)))
		return CDownItemMgr::GetInstance()->PickupItem(m_pMap, pObj, this);
	SaySystem(getstrings(SD_PICKUP_NOTYOURS));
	return FALSE;
}

BOOL CHumanPlayer::DropBagItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem == nullptr)return FALSE;
	if (CItemManager::GetInstance()->ItemLimited(*pItem, IL_NODROP))
	{
		SaySystem(getstrings(SD_ITEMLIMIT_NODROP));
		return FALSE;
	}
	if (!DropItem(*pItem))return FALSE;
	m_ItemBox.RemoveItem(dwMakeIndex);
	return TRUE;
}

BYTE CHumanPlayer::GetShape(DWORD dwMakeIndex)
{
	BYTE S = 0;
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem != nullptr)
		S = pItem->baseitem.btShape;
	return S;
}

BOOL CHumanPlayer::EquipItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	switch (pItem->baseitem.btStdMode)
	{
	case 10: case 11:
		return EquipItem(_U_DRESS, *pItem);
	case 5: case 6:
		return EquipItem(_U_WEAPON, *pItem);
	case 30: case 32:
		return EquipItem(_U_CHARM, *pItem);
	case 19: case 20: case 21:
		return EquipItem(_U_NECKLACE, *pItem);
	case 15:
		return EquipItem(_U_HELMET, *pItem);
	case 24: case 26:
	{
		ITEM* item = m_Equipments.GetEquipment(_U_ARMRINGR);
		if (item == nullptr)
			return EquipItem(_U_ARMRINGR, *pItem);
		else
			return EquipItem(_U_ARMRINGL, *pItem);
	}
	case 22: case 23:
	{
		ITEM* item = m_Equipments.GetEquipment(_U_RINGR);
		if (item == nullptr)
			return EquipItem(_U_RINGR, *pItem);
		else
			return EquipItem(_U_RINGL, *pItem);
	}
	case 81:
		return EquipItem(_U_SHOE, *pItem);
	case 58:
		return EquipItem(_U_BELT, *pItem);
	case 59: case 60: case 61:
		return EquipItem(_U_STONE, *pItem);
	case 25: case 34: // 毒和道符
	{
		ITEM* item = m_Equipments.GetEquipment(_U_ARMRINGR);
		if (item == nullptr)
			return EquipItem(_U_ARMRINGR, *pItem);
		else
			return EquipItem(_U_POISON, *pItem);
	}
	case 33: // 马牌
	{
		ITEM* item = m_Equipments.GetEquipment(_U_CHARM);
		if (item == nullptr)
			return EquipItem(_U_CHARM, *pItem);
		else
			return EquipItem(_U_POISON, *pItem);
	}
	}
	return FALSE;
}

BOOL CHumanPlayer::EquipItem(int pos, DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem == nullptr) return FALSE;
	ITEM itemout;
	if (m_Equipments.EquipItem(pos, *pItem, itemout))
	{
		CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_EQUIPMENT, (WORD)pos);
		m_ItemBox.RemoveItem(dwMakeIndex);
		if (itemout.dwMakeIndex != 0)
		{
			AddBagItem(itemout);
			CItemManager::GetInstance()->UpdateItemPos(itemout.dwMakeIndex, IDF_BAG, 0);
		}
		return TRUE;
	}
	SaySystem(m_Equipments.GetErrorMsg());
	return FALSE;
}

BOOL CHumanPlayer::EquipItem(int pos, ITEM& item, BOOL bForced, BOOL bNoticePlayer)
{
	ITEM itemout;
	if (m_Equipments.EquipItem(pos, item, itemout, bForced, bNoticePlayer))
	{
		item = itemout;
		return TRUE;
	}
	SaySystem(m_Equipments.GetErrorMsg());
	return FALSE;
}

BOOL CHumanPlayer::UnEquipItem(int pos, DWORD dwMakeIndex)
{
	if (dwMakeIndex == 0 || m_Equipments[pos].dwMakeIndex != dwMakeIndex)
		return FALSE;
	BOOL bAddToBag = TRUE;
	if (m_Equipments[pos].wCurDura == 0)
		bAddToBag = FALSE;
	else if (CItemManager::GetInstance()->ItemLimited(m_Equipments[pos], IL_NOTAKEOFF))
	{
		SaySystem(getstrings(SD_ITEMLIMIT_NOTAKEOFF));
		return FALSE;
	}
	ITEM item;
	if (bAddToBag && m_ItemBox.GetFree() < 1)
		return FALSE;
	if (bAddToBag && !CanBearItem(m_Equipments[pos]))
	{
		SaySystem("负重满了，无法取下装备放到背包!");
		return FALSE;
	}
	if (!m_Equipments.UnEquipItem(pos, item))
		return FALSE;
	if (bAddToBag)
		AddBagItem(item, FALSE, TRUE);
	else
	{
		CItemManager::GetInstance()->DeleteItem(dwMakeIndex);
		this->SendTakeBagItem(&item);
	}
	return TRUE;
}

BOOL CHumanPlayer::AddGroupMember(CHumanPlayer* pObject)
{
	return AddGroupMember(pObject->GetName());
}

BOOL CHumanPlayer::RemoveGroupMember(CHumanPlayer* pObject)
{
	return RemoveGroupMember(pObject->GetName());
}

int CHumanPlayer::GetPropValue(PROP_INDEX index)
{
	int	sRet = 0;
	switch (index)
	{
	case PI_MINAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minac; break;
	case PI_MAXAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxac + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddAc); break;

	case PI_MINMAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minmac; break;
	case PI_MAXMAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxmac + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddMac); break;

	case PI_MINDC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.mindc; break;
	case PI_MAXDC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxdc; break;

	case PI_MINMC:
	{
		sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minmc;
		sRet += CAliveObject::GetPropValue(index);
		if (!IsSpecialEquipmentFunctionOn(SEF_HUANMOALL) && !IsSpecialEquipmentFunctionOn(SEF_HUANMOHALF))
		{
			static constexpr special_equipment_func huamoFlags[] = {
				SEF_HUANMO01, SEF_HUANMO02, SEF_HUANMO03, SEF_HUANMO04, SEF_HUANMO05, SEF_HUANMO06, SEF_HUANMO07, SEF_HUANMO08, SEF_HUANMO09, SEF_HUANMO10
			};
			for (special_equipment_func flag : huamoFlags)
			{
				if (IsSpecialEquipmentFunctionOn(flag))
					sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(flag, 1);
			}
		}
		else if (!IsSpecialEquipmentFunctionOn(SEF_MOXUEALL) && !IsSpecialEquipmentFunctionOn(SEF_MOXUEHALF))
		{
			static constexpr special_equipment_func moxueFlags[] = {
				SEF_MOXUE01, SEF_MOXUE02, SEF_MOXUE03, SEF_MOXUE04, SEF_MOXUE05
			};
			for (special_equipment_func flag : moxueFlags)
			{
				if (IsSpecialEquipmentFunctionOn(flag))
					sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(flag, 1);
			}
		}
		if (sRet <= 0)sRet = 0;
	}
	break;
	case PI_MAXMC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxmc; break;

	case PI_MINSC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minsc; break;
	case PI_MAXSC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxsc; break;

	case PI_HITRATE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->hitrate); break;
	case PI_ESCAPE:			sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->escape); break;
	case PI_MAGESCAPE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magescape); break;
	case PI_MAGHITRATE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magicnicety); break;
	case PI_POISONESCAPE:	sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->poisonescape); break;
	case PI_POISONHITRATE:	sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->poisonnicety); break;

	case PI_ATTACKSPEED:	sRet = m_Equipments.GetProp(index); break;
	case PI_LUCKY:			sRet = m_Equipments.GetProp(index); break;
	case PI_DAWN:			sRet = m_Equipments.GetProp(index); break;

	case PI_HPRECOVER:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->hprecover); break;
	case PI_MPRECOVER:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magicrecover); break;
	case PI_POISONRECOVER:	sRet = m_Equipments.GetProp(index); break;

	case PI_HARD:			sRet = m_Equipments.GetProp(index); break;
	case PI_HOLLY:			sRet = m_Equipments.GetProp(index); break;

	case PI_LEVEL:			sRet = m_Humandesc.dbinfo.wLevel; break;

	case PI_CURBAGWEIGHT:	sRet = m_ItemBox.CalcWeight(); break;
	case PI_MAXBAGWEIGHT:	sRet = IsSpecialEquipmentFunctionOn(SEF_BAGOVERLOAD) ? m_Humandesc.dbinfo.weight * 2 : m_Humandesc.dbinfo.weight; break;

	case PI_CURHANDWEIGHT:	sRet = m_Equipments[_U_WEAPON].baseitem.btWeight; break;
	case PI_MAXHANDWEIGHT:	sRet = m_Humandesc.dbinfo.handweight; break;

	case PI_CURBODYWEIGHT:	sRet = m_Equipments.CalcEquipmentsWeight(-1); break;
	case PI_MAXBODYWEIGHT:	sRet = IsSpecialEquipmentFunctionOn(SEF_OVERLOAD) ? m_Humandesc.dbinfo.bodyweight * 2 : m_Humandesc.dbinfo.bodyweight; break;

	case PI_CURHP:			sRet = m_Humandesc.dbinfo.hp; break;
	case PI_CURMP:			sRet = m_Humandesc.dbinfo.mp; break;
	case PI_MAXHP:			sRet = m_Humandesc.dbinfo.maxhp + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddHp); break;
	case PI_MAXMP:			sRet = m_Humandesc.dbinfo.maxmp + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddMp); break;
	case PI_HPPERCENT:		sRet = ROUND(m_Humandesc.dbinfo.hp / GetPropValue(PI_MAXHP));break;
	case PI_EXP:			sRet = m_Humandesc.dbinfo.dwCurExp; break;
	case PI_LEVELUPEXP:		sRet = (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->dwLevelupExp);break;
	case PI_EXPPERCENT:		sRet = (m_pHumanDataDesc == nullptr || m_pHumanDataDesc->dwLevelupExp == 0) ? 0 : ROUND(m_Humandesc.dbinfo.dwCurExp / m_pHumanDataDesc->dwLevelupExp);break;
	}
	return (sRet + CAliveObject::GetPropValue(index));
}

VOID CHumanPlayer::DecPropValue(PROP_INDEX index, int value)
{
	switch (index)
	{
	case PI_CURHP:
	{
		if (IsSpecialEquipmentFunctionOn(SEF_PROTECT))
		{
			WORD wDecMp = static_cast<WORD>(value);
			WORD wCurMp = static_cast<WORD>(GetPropValue(PI_CURMP));
			DecPropValue(PI_CURMP, wDecMp);
			if (wCurMp < wDecMp)
				value = wDecMp - wCurMp;
			else
				return;
		}

		WORD wDecHp = (WORD)value;
		if (wDecHp > m_Humandesc.dbinfo.hp)
			m_Humandesc.dbinfo.hp = 0;
		else
			m_Humandesc.dbinfo.hp -= wDecHp;
		if (IsStatusSet(SI_GREENPOISON))//绿毒状态
		{
			// 改为走EP_DEAD延迟队列统一死亡路径，避免直接调用ToDeath导致与Damage()中的EP_DEAD重复触发
			if (m_Humandesc.dbinfo.hp <= 0 && !IsDeath()) AddProcess(EP_DEAD, GetSetter(SI_GREENPOISON), 0, 0, 0, 2);
		}
	}
	break;
	case PI_CURMP:
	{
		WORD wDecMp = (WORD)value;
		if (wDecMp > m_Humandesc.dbinfo.mp)
			m_Humandesc.dbinfo.mp = 0;
		else
			m_Humandesc.dbinfo.mp -= wDecMp;
	}
	break;
	}
}

VOID CHumanPlayer::AddPropValue(PROP_INDEX index, int value)
{
	WORD wAddValue = static_cast<WORD>(value);
	switch (index)
	{
	case PI_CURHP:
	{
		WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
		if (wMaxHp - m_Humandesc.dbinfo.hp > wAddValue)
			m_Humandesc.dbinfo.hp += wAddValue;
		else
			m_Humandesc.dbinfo.hp = wMaxHp;
	}
	break;
	case PI_CURMP:
	{
		WORD wMaxMp = static_cast<WORD>(GetPropValue(PI_MAXMP));
		if (wMaxMp - m_Humandesc.dbinfo.mp > wAddValue)
			m_Humandesc.dbinfo.mp += wAddValue;
		else
			m_Humandesc.dbinfo.mp = wMaxMp;
	}
	break;
	}
}

VOID CHumanPlayer::UpdateProp()
{
	HUMANPROP prop;
	prop.wLevel = m_Humandesc.dbinfo.wLevel;

	prop.wLevelUpCount = static_cast<WORD>(MIN(m_Humandesc.dbinfo.dwCurExp, static_cast<int>(USHRT_MAX)));
	prop.wNextLevelUpCount = static_cast<WORD>(MIN(m_pHumanDataDesc->dwLevelupExp, static_cast<int>(USHRT_MAX)));
	prop.dwCurexp = m_Humandesc.dbinfo.dwCurExp;
	prop.dwMaxexp = m_pHumanDataDesc->dwLevelupExp;
	//生命值、魔法值
	int iCurHp = GetPropValue(PI_CURHP);
	int iMaxHp = GetPropValue(PI_MAXHP);
	int iCurMp = GetPropValue(PI_CURMP);
	int iMaxMp = GetPropValue(PI_MAXMP);
	prop.wCurHp = static_cast<WORD>(MIN(iCurHp, static_cast<int>(USHRT_MAX)));
	prop.wMaxHp = static_cast<WORD>(MIN(iMaxHp, static_cast<int>(USHRT_MAX)));
	prop.wCurMp = static_cast<WORD>(MIN(iCurMp, static_cast<int>(USHRT_MAX)));
	prop.wMaxMp = static_cast<WORD>(MIN(iMaxMp, static_cast<int>(USHRT_MAX)));
	//属性值
	int iMinAc = GetPropValue(PI_MINAC);
	int iMaxAc = GetPropValue(PI_MAXAC);
	int iMinMagicDef = GetPropValue(PI_MINMAC);
	int iMaxMagicDef = GetPropValue(PI_MAXMAC);
	int iMinAtk = GetPropValue(PI_MINDC);
	int iMaxAtk = GetPropValue(PI_MAXDC);
	int iMinMagAtk = GetPropValue(PI_MINMC);
	int iMaxMagAtk = GetPropValue(PI_MAXMC);
	int iMinSprAtk = GetPropValue(PI_MINSC);
	int iMaxSprAtk = GetPropValue(PI_MAXSC);
	prop.btMinDef = static_cast<WORD>(MIN(iMinAc, static_cast<int>(USHRT_MAX)));
	prop.btMaxDef = static_cast<WORD>(MIN(iMaxAc, static_cast<int>(USHRT_MAX)));
	prop.btMinMagicDef = static_cast<WORD>(MIN(iMinMagicDef, static_cast<int>(USHRT_MAX)));
	prop.btMaxMagicDef = static_cast<WORD>(MIN(iMaxMagicDef, static_cast<int>(USHRT_MAX)));
	prop.btMinAtk = static_cast<WORD>(MIN(iMinAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxAtk = static_cast<WORD>(MIN(iMaxAtk, static_cast<int>(USHRT_MAX)));
	prop.btMinMagAtk = static_cast<WORD>(MIN(iMinMagAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxMagAtk = static_cast<WORD>(MIN(iMaxMagAtk, static_cast<int>(USHRT_MAX)));
	prop.btMinSprAtk = static_cast<WORD>(MIN(iMinSprAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxSprAtk = static_cast<WORD>(MIN(iMaxSprAtk, static_cast<int>(USHRT_MAX)));

	prop.wCurBagWeight = GetPropValue(PI_CURBAGWEIGHT);
	prop.wMaxBagWeight = GetPropValue(PI_MAXBAGWEIGHT);
	prop.btCurBodyWeight = GetPropValue(PI_CURBODYWEIGHT);
	prop.btCurHandWeight = GetPropValue(PI_CURHANDWEIGHT);
	prop.btMaxBodyWeight = GetPropValue(PI_MAXBODYWEIGHT);
	prop.btMaxHandWeight = GetPropValue(PI_MAXHANDWEIGHT);

	prop.wStamina = m_wStamina;
	prop.wMaxStamina = m_wMaxStamina;

	WORD w1 = m_Humandesc.dbinfo.btClass; // 职业
	SendMsg(m_Humandesc.dbinfo.dwGold, SM_UPDATEPROP, w1, 0, 0, &prop, sizeof(prop)); // 发送人物信息
}

VOID CHumanPlayer::UpdateSubProp()
{
	DWORD dwArr = ((m_Humandesc.dbinfo.dwFlag[0] & 0xffff) << 8) | (GetPropValue(PI_MAGESCAPE) & 0xff); // 声望 | 魔法躲避/10
	WORD w1 = GetPropValue(PI_ESCAPE) << 8 | GetPropValue(PI_HITRATE); // 躲避 | 命中
	WORD w2 = GetPropValue(PI_POISONESCAPE) << 8 | GetPropValue(PI_POISONRECOVER); // 客户端没有(中毒躲避)/10 | 中毒恢复
	WORD w3 = GetPropValue(PI_HPRECOVER) << 8 | GetPropValue(PI_MPRECOVER); // 生命恢复 | 魔法恢复
	HUMANSUPROP suprop;
	suprop.wHuoli = m_iHuoli;
	suprop.wHuoliMax = 600;
	suprop.bHuoli = 1; // 开启活力条
	suprop.dwForgePoint = m_Humandesc.dbinfo.dwForgePoint;
	suprop.bLucky = (BYTE)GetPropValue(PI_LUCKY);
	suprop.bDawn = (BYTE)GetPropValue(PI_DAWN);
	suprop.btMagicNicety = GetPropValue(PI_MAGHITRATE);
	suprop.btPoisonNicety = GetPropValue(PI_POISONHITRATE);
	SendMsg(dwArr, SM_UPDATESUBPROP, w1, w2, w3, &suprop, sizeof(suprop));
}

VOID CHumanPlayer::GetViewDetail(xPacket& packet)
{
	//基本信息
	VIEWDETAIL detail;
	o_strncpy(detail.szName, GetName(), 15);
	detail.btNamelen = (BYTE)strlen(detail.szName);
	if (m_pGuild)
	{
		detail.btGuildNamelen = (BYTE)strlen(m_pGuild->GetName());
		o_strncpy(detail.szGuildName, m_pGuild->GetName(), 14);
		if (detail.btGuildNamelen > 14)detail.btGuildNamelen = 14;
		o_strncpy(detail.szTitleName, m_szGuildTitle.data(), 15);
		detail.btTitleNameLen = (BYTE)strlen(m_szGuildTitle.data());
		if (detail.btTitleNameLen > 15)
			detail.btTitleNameLen = 15;
	}
	detail.dwFeature = GetFeather();
	detail.dwStatus = GetStatus();
	detail.dwNameColor = GetNameColor(this);
	detail.btSex = GetSex();
	for (int i = 0; i < _U_MAX; i++)
	{
		detail.items[i] = *(ITEMCLIENT*)&m_Equipments[i];
		detail.items[i].SetLen(); // 设置物品长度
	}
	//扩展信息
	VIEWDETAIL_EX detailEx;
	detailEx.btJob = GetPro(); // 玩家职业
	//push数据
	packet.push(&detail, sizeof(VIEWDETAIL));
	int nValue = (BYTE)strlen(m_Humandesc.dbinfo.szPersonSign);
	packet.push(&nValue, 1); // 个性签名长度
	const char* pszPersonSign = m_Humandesc.dbinfo.szPersonSign;
	packet.push(pszPersonSign);
	packet.push(&detailEx, sizeof(VIEWDETAIL_EX));
}

BOOL CHumanPlayer::CreateBagItem(const char* pszName, BOOL IsBing)
{
	if (m_pClientObj == nullptr)return FALSE;
	return CItemManager::GetInstance()->CreateItem(pszName, m_pClientObj->getId(), m_pClientObj->GetKey(), GetDBId(), 0, IDF_BAG, IsBing);
}

int CHumanPlayer::GetEquipments(EQUIPMENT* pEquipments)
{
	int count = 0;
	for (int i = 0; i < _U_MAX; i++)
	{
		if (m_Equipments[i].dwMakeIndex != 0)
		{
			pEquipments[count].item = *(ITEMCLIENT*)&m_Equipments[i];
			pEquipments[count].pos = i;
			count++;
		}
	}
	return count;
}

int CHumanPlayer::GetVarValue(const char* pszVar) const
{
	if (_stricmp(pszVar, "job") == 0)
		return m_Humandesc.dbinfo.btClass;
	if (_stricmp(pszVar, "sex") == 0)
		return m_Humandesc.dbinfo.btSex;
	if (_stricmp(pszVar, "gold") == 0)
		return m_Humandesc.dbinfo.dwGold;
	if (_stricmp(pszVar, "yuanbao") == 0)
		return m_Humandesc.dbinfo.dwYuanbao;
	if (_stricmp(pszVar, "level") == 0)
		return m_Humandesc.dbinfo.wLevel;
	if (_stricmp(pszVar, "pkpoint") == 0)
		return m_dwPkValue;
	if (_stricmp(pszVar, "credit") == 0)
		return (m_Humandesc.dbinfo.dwFlag[0] & 0xffff);
	return -1;
}

VOID CHumanPlayer::Update()
{
	//分帧更新
	const DWORD dwUpdateKey = GetUpdateKey();
	DWORD dwkey = (dwUpdateKey % 2);
	switch (dwkey)
	{
	case 0:
	{
		if (m_bJustPk)// 检测PK
		{
			if (m_tmrJustPk.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_GRAYNAMETIME) * 1000))
			{
				m_bJustPk = FALSE;
				SendChangeName();
			}
		}
		if (m_dwPkValue > 0) // PK值大于0
		{
			if (m_PkPointTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_ONEPKPOINTTIME) * 1000))
			{
				BYTE btColor = GetNameColor(this);
				m_dwPkValue--;
				m_PkPointTimer.Savetime();
				if (btColor != GetNameColor(this))
					SendChangeName();
			}
		}
		// 6分钟检查
		if (m_Humandesc.dbinfo.wLevel >= 7 && m_StaminaTimer.IsTimeOut(6 * 60 * 1000))
		{
			int expFactor = (m_pMap != nullptr) ? (int)ceilf(m_pMap->GetExpFactor()) : 1;
			if (expFactor > 1)
				m_wStamina += expFactor;
			else
				m_wStamina++;
			if (m_wStamina <= m_wMaxStamina)
			{
				SendJingLiZhi(m_wStamina);
				UpdateProp();
			}
			else
				m_wStamina = m_wMaxStamina;
			m_StaminaTimer.Savetime();
		}
		// 30分钟保存数据 —— 按playerId错开存档时间
		// 偏移 = (playerId % 30) * 10 秒，分成 30 个时间片，避免所有玩家同时冲击DB
		// 每次保存后撤回偏移，保证后续间隔统一为固定30分钟
		DWORD dwSaveInterval = CGameWorld::GetInstance()->GetVar(EVI_CHARINFOBACKUPTIME) * 60 * 1000;
		DWORD dwSaveOffset = (GetId() % 30) * 10 * 1000;
		if (m_DBTimer.IsTimeOut(dwSaveInterval + dwSaveOffset))
		{
			if (CGameWorld::GetInstance()->CanSaveToDB())
			{
				CGameWorld::GetInstance()->UpdateDBUpdateTimer();
				m_DBTimer.Savetime();
				m_DBTimer.SetSavedTime(m_DBTimer.GetSavedTime() - dwSaveOffset);
				UpdateToDB();
			}
		}
		// 1秒检查
		if (m_tmrGameTime.IsTimeOut(1000))
		{
			m_tmrGameTime.Savetime();
			if (m_bYuanQi == FALSE && m_wYuanQi < 2000) // 元气值
			{
				m_wYuanQi++;
				SendMsg(GetId(), 0x9611, m_wYuanQi, 2000, 0);
				if (m_wYuanQi >= 2000)
					m_bYuanQi = TRUE;
			}
		}
	}
	break;
	case 1:
	{
		if (this->IsSpecialEquipmentFunctionOn(SEF_CLOAK))// 检测隐身
		{
			if (!IsStatusSet(SI_CLOAK))
			{
				if (CanDoAction(AT_ATTACK))
					SetStatus(SI_CLOAK, 0, 0xffffffff);
			}
		}
		if (m_pAddToGuildRequester != nullptr) // 请求加入行会人
		{
			if (m_AddToGuildTimer.IsTimeOut(60 * 1000))
				ReplyAddToGuildRequest(FALSE);
		}
		// 检测烈火剑法、雷霆剑
		if (m_pTimeOutDeActiveMagic != nullptr && (m_pTimeOutDeActiveMagic->dwFlag & USERMAGICFLAG_ACTIVED))
		{
			if (m_pTimeOutDeActiveMagic->useTimer.IsTimeOut())
			{
				m_pTimeOutDeActiveMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
				if (m_pTimeOutDeActiveMagic->magic.wId == 26)
				{
					SaySystem("精神火球消失");
					if (m_pClientObj)m_pClientObj->PostMsg("#+UFIR!", 7);
				}
				else if (m_pTimeOutDeActiveMagic->magic.wId == 44)
				{
					SaySystem("雷电力量消失");
					if (m_pClientObj)m_pClientObj->PostMsg("#+UTHU!", 7);
				}
			}
		}
		if (m_pMap) // 检测进入地图
		{
			DWORD dwParam = 0;
			if (m_pMap->IsFlagSeted(MF_LEVELABOVE, dwParam))
			{
				if (GetPropValue(PI_LEVEL) <= LOWORD(dwParam))
				{
					SaySystem("您等级不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_LEVELBELOW, dwParam))
			{
				if (GetPropValue(PI_LEVEL) >= LOWORD(dwParam))
				{
					SaySystem("您等级不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_PKPOINTABOVE, dwParam))
			{
				if (GetPkValue() <= LOWORD(dwParam))
				{
					SaySystem("您的PK值不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_PKPOINTBELOW, dwParam))
			{
				if (GetPkValue() >= LOWORD(dwParam))
				{
					SaySystem("您的PK值不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
		}
	}
	break;
	}
	if (m_pExchangeObj != nullptr && !m_pExchangeObj->GetFastExchange())//如果不是快速交易, 就检测双方占位
	{
		if (!CheckTradeOtherSideOk(m_pExchangeObj->GetOtherSidePlayer(this)))
			m_pExchangeObj->End(this, ET_CANCEL);
	}
	CAliveObject::Update();
	UpdateConSkillBuffs(this);//更新连击技能BUFF
}

VOID CHumanPlayer::AddExp(DWORD dwExp, int level, DWORD dwId)
{
	//	以后要根据level来算出经验, 还有组队的因素
	if (m_pGroupObject)
		m_pGroupObject->AdjustGroupExp(this, dwExp, dwId);
	else
		WinExp(dwExp, FALSE, dwId);
}

VOID CHumanPlayer::WinExp(DWORD dwExp, BOOL bNoBonus, DWORD dwId)
{
	if (!bNoBonus)
	{
		DWORD dwAddExp = 0;
		if (m_pExpMagic)
		{
			m_pExpMagic->nAddPower--;
			if (m_pExpMagic->nAddPower <= 0)
			{
				dwAddExp = dwExp;
				m_pExpMagic->nAddPower = (7 - m_pExpMagic->magic.btLevel);
				m_pExpMagic->nAddPower -= Getrand(m_pExpMagic->nAddPower);
				if (m_pExpMagic->pClass->szSpecial[0] != 0)SaySystem(m_pExpMagic->pClass->szSpecial);
				TrainMagic(m_pExpMagic);
			}
		}
		//World配置倍数 + 地图倍数
		float factor = CGameWorld::GetInstance()->GetExpFactor() + (m_pMap != nullptr ? m_pMap->GetExpFactor() : 0.0f);
		if (dwId > 0)
		{
			if (IsGodBlessEffective(SG_DOUBLEEXP))
			{
				factor += 1.0f;
				AddProcess(EP_GODBLESS, dwId, 8);
			}
		}
		dwExp = ROUND(factor * dwExp) + dwAddExp;
		dwExp = ROUND(GetExpFactor() * dwExp);
	}
	m_Humandesc.dbinfo.dwCurExp += dwExp;
	UINT64 i64TmpExp = m_Humandesc.dbinfo.dwCurExp;  // DWORD → UINT64，高位自动补 0
	SendMsg(0, SM_GETEXP, dwExp & 0xffff, (dwExp & 0xffff0000) >> 16, 0, (LPVOID)&i64TmpExp, sizeof(UINT64));
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] && !m_pPets[i]->IsDeath())
		{
			dwExp /= 10;
			if (dwExp == 0)dwExp = 1;
			((CMonsterEx*)m_pPets[i])->WinExp(dwExp);
		}
	}
	if (m_Humandesc.dbinfo.dwCurExp >= m_pHumanDataDesc->dwLevelupExp)
	{
		m_Humandesc.dbinfo.dwCurExp -= m_pHumanDataDesc->dwLevelupExp;
		LevelUp(m_Humandesc.dbinfo.wLevel + 1);
	}
	CheckAndUpgradeTitle();
}

VOID CHumanPlayer::SetExp(DWORD dwExp)
{
	m_Humandesc.dbinfo.dwCurExp = dwExp;
}

VOID CHumanPlayer::LevelUp(int level)
{
	if (level == m_Humandesc.dbinfo.wLevel)return;
	HUMANDATADESC* pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(m_Humandesc.dbinfo.btClass, level);
	if (pHumanData == nullptr)return;
	m_Humandesc.dbinfo.wLevel = level;
	m_Humandesc.dbinfo.hp = pHumanData->wHp;
	m_Humandesc.dbinfo.mp = pHumanData->wMp;
	m_Humandesc.dbinfo.maxhp = pHumanData->wHp;
	m_Humandesc.dbinfo.maxmp = pHumanData->wMp;
	m_Humandesc.dbinfo.handweight = pHumanData->handweight;
	m_Humandesc.dbinfo.weight = pHumanData->bagweight;
	m_Humandesc.dbinfo.bodyweight = pHumanData->bodyweight;
	m_Humandesc.dbinfo.maxac = pHumanData->maxac;
	m_Humandesc.dbinfo.minac = pHumanData->minac;
	m_Humandesc.dbinfo.maxmac = pHumanData->maxmac;
	m_Humandesc.dbinfo.minmac = pHumanData->minmac;
	m_Humandesc.dbinfo.maxmc = pHumanData->maxmc;
	m_Humandesc.dbinfo.minmc = pHumanData->minmc;
	m_Humandesc.dbinfo.maxsc = pHumanData->maxsc;
	m_Humandesc.dbinfo.minsc = pHumanData->minsc;
	m_Humandesc.dbinfo.maxdc = pHumanData->maxdc;
	m_Humandesc.dbinfo.mindc = pHumanData->mindc;
	m_pHumanDataDesc = pHumanData;
	OnLevelUp(level);
	DWORD dwId = GetId();
	UINT64 i64TmpExp = m_Humandesc.dbinfo.dwCurExp;  // DWORD → UINT64，高位自动补 0
	SendMsg(0, SM_LEVELUP, level, dwId & 0xffff, (dwId >> 16) & 0xffff, (LPVOID)&i64TmpExp, sizeof(UINT64)); //发送升级特效
	UpdateProp();
	UpdateSubProp();
}

BOOL CHumanPlayer::AddGold(DWORD dwCount, BOOL bUpdateClient)
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwCount > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	m_Humandesc.dbinfo.dwGold += dwCount;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::PetAddgold(DWORD dwgold, BOOL bUpdateClient)
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwgold > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	m_Humandesc.dbinfo.dwGold += dwgold;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::TakeGold(DWORD dwCount, BOOL bUpdateClient)
{
	if (m_Humandesc.dbinfo.dwGold < dwCount)
		return FALSE;
	m_Humandesc.dbinfo.dwGold -= dwCount;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::TestAddGold(DWORD dwCount)const
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwCount > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	return TRUE;
}

BOOL CHumanPlayer::TestAddMoney(money_type type, DWORD dwCount)const
{
	DWORD dwMax = CGameWorld::GetInstance()->GetVar(type == MT_GOLD ? EVI_MAXGOLD : EVI_MAXYUANBAO);
	DWORD dwCur = GetMoney(type);
	if (dwCur > dwMax)return FALSE;
	if (dwCount > (dwMax - dwCur))return FALSE;
	return TRUE;
}

VOID CHumanPlayer::DropGold(DWORD dwCount)
{
	if (dwCount > m_Humandesc.dbinfo.dwGold)
	{
		SaySystem("您没有那么多钱可以扔!");
		return;
	}
	POINT pt;
	if (m_pMap == nullptr) return;
	int count = m_pMap->GetDropItemPoint(getX(), getY(), &pt, 1);
	if (count > 0)
	{
		m_Humandesc.dbinfo.dwGold -= dwCount;
		CAliveObject::DropGold(dwCount, pt.x, pt.y);
		SendGoldChanged();
	}
}

VOID CHumanPlayer::UpdateToDB()
{
	if (this->m_pClientObj == nullptr)return;
	CHARDBINFO info;
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pObj == nullptr)return;

	if (GetDBInfo(info))
	{
		if (!IsDeath())
		{
			if (info.hp == 0)
				info.hp = info.maxhp;
		}
		else
			info.hp = 0;
		pObj->SendPutDbInfo(GetId(), m_pClientObj->GetKey(), info);
	}
	if (IsSystemFlagSeted(SF_COMMUNITYLOADED))
	{
		char szCommunityText[4096];
		int length = GetCommunityInfo(szCommunityText, 4096);
		pObj->UpdateCommunity(GetDBId(), szCommunityText);
	}
	//if( IsSystemFlagSeted( SF_BANKLOADED ) )
	//{
	//	BAGITEMPOS	itempos[STOREAGE_SLOT];
	//	int count = m_ItemBank.GetItemPos( itempos, STOREAGE_SLOT );
	//	if( count > 0 )pObj->SendUpdateItemPos( IDF_BANK, itempos, count );
	//}
	if (this->m_fMagicLoaded)
	{
		std::array<MAGICDB, 255> array{};
		int	count = 0;
		for (auto& up : m_vMagic)
			array[count++] = up->magic;
		if (count > 0)
			pObj->SendUpdateMagic(GetDBId(), count, array.data());
	}
}

VOID CHumanPlayer::SetMagic(MAGICDB* pArray, int count)
{
	USERMAGIC* pMagic = nullptr;
	for (int i = 0; i < count; i++)
	{
		pMagic = GetMagic(pArray[i].wId);
		if (pMagic == nullptr)
		{
			auto up = std::make_unique<USERMAGIC>();
			up->magic = pArray[i];
			up->pClass = CMagicManager::GetInstance()->GetClassById(pArray[i].wId);
			if (up->pClass == nullptr)
				continue;
			up->useTimer.SetTimeOut(0);
			pMagic = up.get();
			m_vMagic.push_back(std::move(up));
			OnAddMagic(pMagic);
		}
		else
		{
			pMagic->magic = pArray[i];
			pMagic->useTimer.SetTimeOut(0);
		}
	}
	RecalcHitSpeed();
	UpdateSubProp();
	m_fMagicLoaded = TRUE;
	UpdateConSkillActive(this);
}

BOOL CHumanPlayer::AddMagic(WORD wId, BYTE btLevel)
{
	if (!m_fMagicLoaded)return FALSE;
	USERMAGIC* pUserMagic = GetMagic(wId);
	if (pUserMagic)return FALSE;
	MAGICCLASS* pMagicClass = CMagicManager::GetInstance()->GetClassById(wId);
	if (pMagicClass == nullptr)return FALSE;

	auto upMagic = std::make_unique<USERMAGIC>();
	upMagic->magic.btKey = 0;
	upMagic->magic.btLevel = btLevel;
	upMagic->magic.dwCurTrain = 0;
	upMagic->magic.wId = wId;
	upMagic->pClass = pMagicClass;

	MAGIC magic;
	if (CMagicManager::GetInstance()->CreateMagic((UINT)wId, magic))
	{
		magic.btLevel = btLevel;
		SendMsg(0, 0xd2, 0, 0, 1, &magic, static_cast<WORD>(sizeof(magic)));
		SaySystem("您学会了 %s 技能!", pMagicClass->szName);
		USERMAGIC* pRaw = upMagic.get();
		m_vMagic.push_back(std::move(upMagic));
		OnAddMagic(pRaw);
		RecalcHitSpeed();
		UpdateSubProp();
		UpdateConSkillActive(this);
	}
	return TRUE;
}

BOOL CHumanPlayer::AddMagic(const char* pszName, BYTE btLevel)
{
	if (!m_fMagicLoaded)return FALSE;
	MAGICCLASS* pMagicClass = CMagicManager::GetInstance()->GetClassByName(pszName);
	if (pMagicClass == nullptr)return FALSE;
	USERMAGIC* pUserMagic = GetMagic(pMagicClass->id);
	if (pUserMagic)return FALSE;

	auto upMagic = std::make_unique<USERMAGIC>();
	upMagic->magic.btKey = 0;
	upMagic->magic.btLevel = btLevel;
	upMagic->magic.dwCurTrain = 0;
	upMagic->magic.wId = static_cast<WORD>(pMagicClass->id);
	upMagic->pClass = pMagicClass;

	MAGIC magic;
	if (CMagicManager::GetInstance()->CreateMagic((UINT)pMagicClass->id, magic))
	{
		magic.btLevel = btLevel;
		SendMsg(0, 0xd2, 0, 0, 1, &magic, static_cast<WORD>(sizeof(magic)));
		SaySystem("您学会了 %s 技能!", pszName);
		USERMAGIC* pRaw = upMagic.get();
		m_vMagic.push_back(std::move(upMagic));
		OnAddMagic(pRaw);
		RecalcHitSpeed();
		UpdateSubProp();
		UpdateConSkillActive(this);
	}
	return TRUE;
}

const char* CHumanPlayer::GetAccount()
{
	if (m_pClientObj)return m_pClientObj->GetAccount(); return "<nullptr>";
}

BOOL CHumanPlayer::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	BOOL bRet = CAliveObject::GetViewmsg(pszMsg, length, pViewer);
	if (!bRet) return FALSE;
	if (m_szCurrentTitle[0] != 0)
	{
		WORD wFlag = 8 | ((m_iCurrentTitleIndex + 1) << 8);
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0x532c, wFlag, 0, 0, (LPVOID)m_szCurrentTitle.data());
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	WORD wFlag = 0;
	WORD wSecond = 0;
	if (IsSystemFlagSeted(SF_WINDSHIELD))
		wFlag |= 2;
	if (IsSystemFlagSeted(SF_STRONGSHIELD))
		wFlag |= 1;
	if (IsSystemFlagSeted(SF_KUANGNUWIND))
		wFlag |= 8192;
	if (IsSystemFlagSeted(SF_GODBLESS))
	{
		wFlag |= 4;
		wSecond = static_cast<WORD>(m_SystemFlag.GetTimeOut(SF_GODBLESS) / 1000);
	}
	//SendMsg( GetId(), 0x532c, wFlag, wSecond, 0 );
	if (wFlag > 0)
	{
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0x532c, wFlag, wSecond, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	if (IsSpecialEquipmentFunctionOn(SEF_HEALHALO))
	{
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0x532c, 0x10, 1, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	else if (IsSpecialEquipmentFunctionOn(SEF_MIRAGE))
	{
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0x532c, 0x10, 2, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	if (GetActionType() == AT_PRIVATESHOP)
	{
		PRIVATESHOPHEADER psheader;
		o_strncpy(psheader.szName, m_szPrivateShopName.data(), 51);
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
			&psheader, sizeof(psheader));
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	DWORD dwParam = 0;
	if (IsSystemFlagSeted(SF_TRANSFORMED))
	{
		dwParam = GetSystemFlagParam(SF_TRANSFORMED);
		char szTempBuffer[1024];
		int tempSize = EncodeMsg(szTempBuffer, GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff, nullptr);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	return TRUE;
}

BOOL CHumanPlayer::AddPet(CAliveObject* pObject)
{
	if (pObject == nullptr) return FALSE;
	CMonsterEx* pPet = (CMonsterEx*)pObject;
	MonsterClass* pDesc = pPet->GetDesc();
	if (pDesc && pDesc->petset.Type == APT_RIDE)
	{
		SetHorse(pPet);
		return TRUE;
	}
	else if (pDesc && pDesc->petset.Type == APT_PET)
	{
		m_pPet = pPet;
		return TRUE;
	}
	//如果有列表里有,则直接返回
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] == pObject) return TRUE;
	}
	if (m_iPetCount >= 5) return FALSE;
	m_pPets[m_iPetCount++] = pPet;
	return TRUE;
}

BOOL CHumanPlayer::DelPet(CAliveObject* pObject)
{
	if (pObject == nullptr) return FALSE;
	CMonsterEx* pPet = (CMonsterEx*)pObject;
	MonsterClass* pDesc = pPet->GetDesc();
	if (pDesc && pDesc->petset.Type == APT_RIDE) // 处理坐骑
	{
		if (m_pHorse == nullptr) return FALSE;
		SetHorse(nullptr);
		return TRUE;
	}
	else if (pDesc && pDesc->petset.Type == APT_PET) // 处理豹子宠物
	{
		if (m_pPet == nullptr) return FALSE;
		m_pPet->SetOwner(nullptr);//设置豹子的主人为空
		petname.fill(0);
		ISzhaohuan = FALSE;
		m_baozhiID = 0;
		m_pPet = nullptr;
		return TRUE;
	}
	// 处理普通宠物列表
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] == pPet)
		{
			for (int j = i; j < m_iPetCount - 1; j++)
			{
				m_pPets[j] = m_pPets[j + 1];
			}
			m_iPetCount--;
			m_pPets[m_iPetCount] = nullptr;
			break;
		}
	}
	return TRUE;
}

DWORD CHumanPlayer::GetMoney(money_type type)const
{
	if (type == MT_GOLD)
		return m_Humandesc.dbinfo.dwGold;
	else if (type == MT_YUANBAO)
		return m_Humandesc.dbinfo.dwYuanbao;
	return 0;
}

BOOL CHumanPlayer::AddGroupMember(const char* pszName)
{
	CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
	if (pMember == nullptr)
	{
		SaySystem("无法邀请 %s , 可能已经下线了!", pszName);
		return FALSE;
	}
	if (!pMember->IsGroupEnabled())
	{
		SaySystem("对方拒绝编组!");
		return FALSE;
	}
	if (m_pGroupObject == nullptr)
	{
		if (CGroupObjectMgr::GetInstance()->CreateGroup(this, pMember))
			return TRUE;
		return FALSE;
	}
	else
	{
		if (m_pGroupObject->IsLeader(this))
		{
			if (m_pGroupObject->IsMember(pMember))
				SaySystem("已经是队员～!");
			else
				return m_pGroupObject->AddMember(pMember);
		}
		else
			SaySystem("对不起, 您不是队长, 没有权限!");
	}
	return FALSE;
}

BOOL CHumanPlayer::RemoveGroupMember(const char* pszName)
{
	if (m_pGroupObject)
	{
		CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
		if (pMember == nullptr)
		{
			SaySystem(" %s 不在线!", pszName);
			return FALSE;
		}
		if (m_pGroupObject->IsLeader(this))
		{
			m_pGroupObject->DelMember(pMember);
			return TRUE;
		}
		else
			SaySystem("您不是队长, 没有删除队员的权限!");
	}
	else
		SaySystem("您还没有建立队伍!");
	return FALSE;
}

VOID CHumanPlayer::UpdateGroupPosition()
{
	if (m_pGroupObject != nullptr)
		m_pGroupObject->UpdateMemberPosition(this);
}

VOID CHumanPlayer::SetStartPointIndex(DWORD dwStartPointIndex)
{
	m_dwStartPointIndex = dwStartPointIndex;
	START_POINT* pStartPoint = CGameWorld::GetInstance()->GetStartPoint(dwStartPointIndex);
	if (pStartPoint == nullptr)
	{
		pStartPoint = CGameWorld::GetInstance()->GetStartPoint((DWORD)0);
		if (pStartPoint == nullptr)return;
		m_dwStartPointIndex = 0;
		o_strncpy(m_Humandesc.dbinfo.szStartPoint, pStartPoint->szName, 31);
	}
	else
		o_strncpy(m_Humandesc.dbinfo.szStartPoint, pStartPoint->szName, 31);
}

VOID CHumanPlayer::Home()
{
	int	mapid, x, y;
	if (CGameWorld::GetInstance()->GetStartPoint(m_Humandesc.dbinfo.szStartPoint, mapid, x, y))
		FlyTo(mapid, x, y);
}

ITEM* CHumanPlayer::FindBagItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	return pItem;
}

BOOL CHumanPlayer::DeleteBagItem(DWORD dwMakeIndex)
{
	return m_ItemBox.RemoveItem(dwMakeIndex);
}

BOOL CHumanPlayer::DeleteBagItem(ITEM* pItem)
{
	return m_ItemBox.RemoveItem(pItem);
}

BOOL CHumanPlayer::IsGroupEnabled()const
{
	if (m_Humandesc.dbinfo.dwFlag[0] & 0x80000000)
		return TRUE;
	return FALSE;
}

VOID CHumanPlayer::SetGroupMode(BOOL bEnabled)
{
	if (bEnabled)
		m_Humandesc.dbinfo.dwFlag[0] |= 0x80000000;
	else
	{
		if (IsGroupEnabled())
			m_Humandesc.dbinfo.dwFlag[0] ^= 0x80000000;
	}
	if (!bEnabled)
	{
		if (m_pGroupObject)
			m_pGroupObject->LeaveMember(this);
	}
	SendGroupMode();
}

BOOL CHumanPlayer::RandomTeleport(int nMapId)
{
	if (m_pMap == nullptr)return FALSE;
	//	检查是否允许随机移动
	CLogicMap* pMap = nullptr;
	if (nMapId == -1)
	{
		if (m_pMap->IsFlagSeted(MF_NORANDOMMOVE))
		{
			SaySystemAttrib(CC_RED, getstrings(SD_MAPLIMITED_NORANDOMMOVE));
			return FALSE;
		}
		nMapId = GetMapId();
		pMap = m_pMap;
	}
	else
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
		if (pMap == nullptr)return FALSE;
	}
	int width = pMap->GetWidth();
	int height = pMap->GetHeight();
	int px = Getrand(width);
	int py = Getrand(height);
	POINT pt;
	while (pMap->IsBlocked(px, py))
	{
		if (pMap->GetValidPoint(px, py, &pt, 1))
		{
			px = pt.x, py = pt.y;
			break;
		}
		px = Getrand(width);
		py = Getrand(height);
	}
	return FlyTo(pMap, px, py);
}

VOID CHumanPlayer::CheckAndUpgradeTitle()
{
	int iTitleIndex = 0;
	if (CTitleManager::GetInstance()->GetTitle(this, m_szCurrentTitle.data(), iTitleIndex))
	{
		if ((iTitleIndex + 1) != m_iCurrentTitleIndex)
		{
			m_iCurrentTitleIndex = iTitleIndex + 1;
			SendTitleChanged();
		}
	}
}

int CHumanPlayer::GetAutoRecoverHp()
{
	int hp = GetPropValue(PI_HPRECOVER);
	if (IsStatusSet(SI_GREENPOISON))//绿毒状态
		hp = -static_cast<int>(GetStatusParam(SI_GREENPOISON));
	return hp;
}

int CHumanPlayer::GetAutoRecoverMp()
{
	int mp = GetPropValue(PI_MPRECOVER);
	if (IsStatusSet(SI_STRAWMAN)) // 诅咒术(男)红咒
		mp = -static_cast<int>(GetStatusParam(SI_STRAWMAN));
	return mp;
}

BOOL CHumanPlayer::CostMoney(money_type type, DWORD dwCount, BOOL bUpdateClient)
{
	DWORD& dwMoney = (type == MT_GOLD) ? (m_Humandesc.dbinfo.dwGold) : (m_Humandesc.dbinfo.dwYuanbao);
	if (dwMoney < dwCount)return FALSE;
	dwMoney -= dwCount;
	if (bUpdateClient)SendMoneyChanged(type);
	return TRUE;
}

BOOL CHumanPlayer::AddMoney(money_type type, DWORD dwCount, BOOL bUpdateClient)
{
	DWORD& dwMoney = (type == MT_GOLD) ? (m_Humandesc.dbinfo.dwGold) : (m_Humandesc.dbinfo.dwYuanbao);
	DWORD dwMaxGold = 0;
	if (m_Humandesc.dbinfo.bBigGold)
		dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXBIGGOLD);
	else
		dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	DWORD dwMaxYuanBao = CGameWorld::GetInstance()->GetVar(EVI_MAXYUANBAO);
	DWORD dwMaxMoney = (type == MT_GOLD) ? dwMaxGold : dwMaxYuanBao;
	if (dwMoney + dwCount >= dwMaxMoney)
		return FALSE;
	dwMoney += dwCount;
	if (bUpdateClient)SendMoneyChanged(type);
	return TRUE;
}

BOOL CHumanPlayer::TrainMagic(USERMAGIC* pMagic, int exp)
{
	if (pMagic->magic.btLevel < 9)
	{
		if (exp == 0) exp = (Getrand(3) + 1);
		if (IsSpecialEquipmentFunctionOn(SEF_TRAINSKILL))exp *= 2;
		if (exp != 0)
		{
			pMagic->magic.dwCurTrain += exp;
			if (pMagic->magic.dwCurTrain >= pMagic->pClass->dwNeedExp[pMagic->magic.btLevel])
			{
				pMagic->magic.btLevel++;
				OnMagicLevelup(pMagic);
				pMagic->magic.dwCurTrain = 0;
			}
			SendMagicExpChg(pMagic);
			return TRUE;
		}
	}
	return FALSE;
}

VOID CHumanPlayer::DamageDura(int pos, int nDura, int nDuraRate)
{
	ITEM* pItem = m_Equipments.GetEquipment(pos);
	if (pItem == nullptr)return;
	if (pItem->wCurDura == 0)
	{
		ITEM offitem;
		if (m_Equipments.UnEquipItem(pos, offitem))
		{
			CItemManager::GetInstance()->DeleteItem(offitem.dwMakeIndex);
			SendTakeBagItem(&offitem);
			if (pos == _U_WEAPON) SendWeaponBroken(); // 发送武器破碎
		}
		SendFeatureChanged();
		UpdateProp();
		UpdateSubProp();
		SendStatusChanged();
	}
	else
	{
		int t = pItem->wCurDura / nDuraRate;
		if (pItem->wCurDura < nDura)
			pItem->wCurDura = 0;
		else
			pItem->wCurDura -= nDura;
		if (t != pItem->wCurDura / nDuraRate)
			SendDuraChanged(pos, pItem->wCurDura, pItem->wMaxDura);
		CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
	}
}

VOID CHumanPlayer::DamageMaterialDura(int pos, int nDura)
{
	ITEM* pItem = m_Equipments.GetEquipment(pos);
	if (pItem == nullptr)return;
	CItemManager::GetInstance()->DamageDura(*pItem, nDura);
	SendDuraChanged(pos, pItem->wCurDura, pItem->wMaxDura);
	if (pItem->wCurDura == 0)
	{
		ITEM offitem;
		if (m_Equipments.UnEquipItem(pos, offitem))
		{
			CItemManager::GetInstance()->DeleteItem(offitem.dwMakeIndex);
			SendTakeBagItem(&offitem);
		}
	}
}

VOID CHumanPlayer::OnAddMagic(USERMAGIC* pMagic)
{
	pMagic->nAddPower = (7 - pMagic->magic.btLevel);
	pMagic->nAddPower -= Getrand(pMagic->nAddPower);
	if (((pMagic->pClass->dwFlag & MAGICFLAG_FORCED) != 0 && (pMagic->pClass->dwFlag & MAGICFLAG_ACTIVED) == 0)) //增加属性的被动技能
	{
		if (m_iAutoMagicCount >= 8)
			PRINT(ERROR_RED, "被动技能太多～～\n");
		else
			m_pAutoMagic[m_iAutoMagicCount++] = pMagic;
	}
	else if (pMagic->pClass->dwFlag & MAGICFLAG_FORCED_EXP)
		m_pExpMagic = pMagic;
}

VOID CHumanPlayer::UpdateAutoMagic()
{
	for (int i = 0; i < m_iAutoMagicCount; i++)
	{
		if (m_pAutoMagic[i] && (--m_pAutoMagic[i]->nAddPower) <= 0)
		{
			m_pAutoMagic[i]->nAddPower = 7 - m_pAutoMagic[i]->magic.btLevel;
			m_pAutoMagic[i]->nAddPower -= Getrand(m_pAutoMagic[i]->nAddPower);
			TrainMagic(m_pAutoMagic[i]);
			if (m_pAutoMagic[i]->pClass->dwFlag & MAGICFLAG_FORCED)
			{
				switch (m_pAutoMagic[i]->magic.wId)
				{
				case 7:	// 攻杀剑法
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+PWR!", 6);
				}
				break;
				case 40: // 残影
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+VIS!", 6);
				}
				break;
				case 41: // 血影 
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+SHAD!", 7);
				}
				break;
				}
			}
		}
	}
}

VOID CHumanPlayer::RecalcHitSpeed()
{
	if (m_vMagic.empty()) return;
	DecProp(PI_HITRATE, m_nRecalcHit);// 先把增加的命中减掉
	//DecProp(PI_ESCAPE, m_nRecalcSpeed);// 先把增加的躲避减掉
	m_nRecalcHit = 0;
	//m_nRecalcSpeed = 0;
	for (auto& up : m_vMagic)
	{
		auto* pMagic = up.get();
		switch (pMagic->magic.wId)
		{
		case 3: // 初级剑法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 4: // 精神战法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 7: // 攻杀剑法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 40: // 残影刀法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 41: // 血影刀法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value2;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 74: // 擒龙手
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value1;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		}
	}
}

BOOL CHumanPlayer::MagMakeDefenceArea(int x, int y, int nRange, int nSec, int nState)
{
	if (m_pMap == nullptr) return FALSE;
	BOOL bFlag = FALSE;
	int nStartX = x - nRange;
	int nStartY = y - nRange;
	int nEndX = x + nRange;
	int nEndY = y + nRange;

	for (x = nStartX; x <= nEndX; x++)
	{
		for (y = nStartY; y <= nEndY; y++)
		{
			CMapCellInfo* pInfo = m_pMap->GetMapCellInfoShared(x, y);
			if (pInfo == nullptr)continue;
			xListHelper<CMapObject> objlist(&pInfo->m_xObjectList);
			for (CMapObject* pObject = objlist.first(); pObject != nullptr; pObject = objlist.next())
			{
				if (pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperFriend((CAliveObject*)pObject))
				{
					((CAliveObject*)pObject)->DefenceUp(nState, nSec);
					if (pObject->GetType() == OBJ_PLAYER)
					{
						((CHumanPlayer*)pObject)->SaySystemAttrib(CC_GREENS, nState == 22 ? "防御力增加%d秒" : "抗魔法力增加%d秒", nSec);
					}
					bFlag = TRUE;
				}
			}
		}
	}
	return bFlag;
}

VOID CHumanPlayer::ChangeAttackMode(int mode)
{
	int itemp = m_Humandesc.dbinfo.btAttackMode;
	if (mode == -1)
	{
		if (itemp >= HAM_MAX - 1)
			itemp = 0;
		else
			itemp++;
	}
	else
		itemp = mode;
	m_Humandesc.dbinfo.btAttackMode = itemp;
	SaySystemAttrib(CC_GREEN, "[%s攻击模式]", g_pAttackModeDesc[m_Humandesc.dbinfo.btAttackMode]);
}

BOOL CHumanPlayer::IsProperFriend(CAliveObject* pObject)
{
	if (pObject == nullptr || (pObject && pObject->IsDeath())) return FALSE;
	if (pObject == this) return TRUE;
	e_object_type type = pObject->GetType();
	if (type == OBJ_NPC) return FALSE;
	if (type == OBJ_MONSTER) return FALSE;
	if (type == OBJ_PET)
	{
		if (pObject->GetOwner())
			return IsProperFriend(pObject->GetOwner());
		else
			return FALSE;
	}
	else if (type == OBJ_PLAYER)
	{
		e_humanattackmode mode = GetAttackMode();
		switch (mode)
		{
		case HAM_ALL:
			return TRUE;
		case HAM_PEACE:
			return FALSE;
		case HAM_GROUP:
			return (m_pGroupObject != nullptr && m_pGroupObject->IsMember((CHumanPlayer*)pObject));
		case HAM_GUILD:
			return (m_pGuild != nullptr && m_pGuild == ((CHumanPlayer*)pObject)->GetGuild());
		break;
		case HAM_COUPLE:
			return (m_sWife[0] != 0 && strcmp(m_sWife.data(), pObject->GetName()) == 0);
		break;
		case HAM_CRIME:
			return (!((CHumanPlayer*)pObject)->NoLawProtect());
		break;
		case HAM_MASTER:
		{
			if (m_sMaster[0] != 0 && strcmp(m_sMaster.data(), pObject->GetName()) == 0)
				return TRUE;
			if (GetStudentCount() > 0 && IsMyStudent(pObject->GetName()) == 0)
				return TRUE;
		}
		break;
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::IsProperTarget(CAliveObject* pObject)
{
	if (!pObject) return FALSE;
	if (pObject == this) return FALSE;
	e_object_type type = pObject->GetType();
	if (type == OBJ_NPC) return FALSE;
	if (type == OBJ_MONSTER) return TRUE;
	if (!pObject->IsStatusSet(SI_ITEMTRACED))
	{
		if (InSafeArea() || pObject->InSafeArea())
			return FALSE;
	}
	e_humanattackmode mode = GetAttackMode();
	if (type == OBJ_PET && mode != HAM_ALL)
	{
		if (pObject->GetOwner())
			return IsProperTarget(pObject->GetOwner());
		else
			return FALSE;
	}
	switch (mode)
	{
	case HAM_CRIME: // 善恶
	{
		if (((CHumanPlayer*)pObject)->NoLawProtect()) return TRUE;
		return FALSE;
	}
	break;
	case HAM_ALL: // 全体
		return TRUE;
	case HAM_PEACE: // 和平
		return FALSE;
	case HAM_GROUP: // 组队
	{
		if (m_pGroupObject == nullptr) return TRUE;
		return (!m_pGroupObject->IsMember((CHumanPlayer*)pObject));
	}
	break;
	case HAM_GUILD: // 行会
	{
		if (m_pGuild == nullptr) return TRUE;
		if (m_pGuild != ((CHumanPlayer*)pObject)->GetGuild()) return TRUE;
		return FALSE;
	}
	break;
	case HAM_MASTER: // 师徒
	{
		if (m_sMaster[0] != 0 && strcmp(m_sMaster.data(), pObject->GetName()) == 0)
			return FALSE;
		if (GetStudentCount() > 0 && IsMyStudent(pObject->GetName()) == 0)
			return FALSE;
	}
	break;
	case HAM_COUPLE: // 夫妻
	{
		if (m_sWife[0] != 0 && strcmp(m_sWife.data(), pObject->GetName()) == 0)
			return FALSE;
	}
	break;
	}
	return FALSE;
}

VOID CHumanPlayer::SetGuild(CGuildEx* pGuild, const char* pszTitle, int iLevel)
{
	m_pGuild = pGuild;
	if (pGuild == nullptr)
	{
		m_szGuildTitle.fill(0);
		m_iGuildTitleLevel = 0;
	}
	else
	{
		if (pszTitle == nullptr)
		{
			m_pGuild->MemberLogon(this);
			return;
		}
		else
		{
			o_strncpy(m_szGuildTitle.data(), pszTitle, 63);
			m_iGuildTitleLevel = iLevel;
			m_pGuild = pGuild;
		}
	}
	char GuildTitle[200] = "";
	if (m_pGuild)
	{
		snprintf(GuildTitle, sizeof(GuildTitle), "%s/%s", m_pGuild->GetName(), m_szGuildTitle.data());
		o_strncpy(m_Humandesc.dbinfo.szGuildName, m_pGuild->GetName(), 31);
	}
	else
		m_Humandesc.dbinfo.szGuildName[0] = 0;
	SendMsg(0, 0x2ee, 0, 0, 0, (LPVOID)GuildTitle);
}

int	CHumanPlayer::GetGuildFrontPage(char* pszBuffer, int buffersize)
{
	return 0;
}

BOOL CHumanPlayer::IsMyFriend(CHumanPlayer* pPlayer)const
{
	if (pPlayer == nullptr)return FALSE;
	char* pName = (char*)pPlayer->GetName();
	for (const auto& friendName : m_sFriends)
	{
		if (friendName[0] == 0)continue;
		if (strcmp(friendName.data(), pName) == 0)return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::IsMyFriend(const char* pszName)const
{
	if (pszName == nullptr)return FALSE;
	for (const auto& friendName : m_sFriends)
	{
		if (friendName[0] == 0)continue;
		if (strcmp(friendName.data(), pszName) == 0)return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::PostAddFriendRequest(CHumanPlayer* poster)
{
	if (poster == nullptr)return FALSE;
	if (IsMyFriend(poster) || poster->IsMyFriend(this))
	{
		poster->SendFriendSystemError(FE_ADD_ALREADYFRIEND, GetName());
		return FALSE;
	}
	if (m_bRefuseAddFriend)
	{
		poster->SendFriendSystemError(FE_ADD_TARGETNOTALOW, GetName());
		return FALSE;
	}
	int level = CGameWorld::GetInstance()->GetVar(EVI_ADDFRIENDLEVEL);
	if (poster->GetPropValue(PI_LEVEL) < level)
	{
		poster->SendFriendSystemError(FE_ADD_LOWLEVEL, "");
		return FALSE;
	}
	else if (GetPropValue(PI_LEVEL) < level)
	{
		poster->SendFriendSystemError(FE_ADD_TARGETLOWLEVEL, GetName());
		return FALSE;
	}
	SendMsg(0, 0x1c6, 0, 0, 0, (LPVOID)poster->GetName());
	return TRUE;
}

VOID CHumanPlayer::ReplyAddFriendRequest(BOOL bAccept, const char* pszName)
{
	CHumanPlayer* pRequester = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
	if (pRequester == nullptr)return;
	if (bAccept > 1)bAccept = FALSE;
	if (bAccept)
		pRequester->AcceptAddFriendRequest(this);
	else
		pRequester->SendFriendSystemError(FE_ADD_REFUSED, GetName());
}

VOID CHumanPlayer::AcceptAddFriendRequest(CHumanPlayer* pFriend)
{
	AddFriend(pFriend);
}

BOOL CHumanPlayer::PostAddToGuildRequest(CHumanPlayer* poster)
{
	if (m_pAddToGuildRequester != nullptr)
	{
		if (m_pAddToGuildRequester == poster)
			poster->SaySystem("此人正在决定你的加入行会请求!");
		else
			poster->SaySystem("此人正在决定别人的加入行会请求!");
		return FALSE;
	}
	m_pAddToGuildRequester = poster;
	if (poster != 0)
	{
		m_dwAddToGuildRequesterInstanceKey = poster->GetInstanceKey();
		SendMsg(0, 0xc7ff, 0, 0, 0, (LPVOID)poster->GetName());
		m_AddToGuildTimer.Savetime();
	}
	return TRUE;
}

VOID CHumanPlayer::ReplyAddToGuildRequest(BOOL bAccept)
{
	if (bAccept)
	{
		if (m_pAddToGuildRequester == nullptr)
			return;
		if (m_pAddToGuildRequester->GetInstanceKey() != m_dwAddToGuildRequesterInstanceKey)
		{
			SaySystem("请求者下线, 本次加入操作作废!");
			return;
		}
		m_pAddToGuildRequester->AcceptAddToGuildRequest(this);
	}
	else
	{
		if (m_pAddToGuildRequester == nullptr)
			return;
		if (m_pAddToGuildRequester->GetInstanceKey() == m_dwAddToGuildRequesterInstanceKey)
			m_pAddToGuildRequester->SaySystem("对方拒绝你的加入行会请求");
	}
	m_pAddToGuildRequester = nullptr;
	m_dwAddToGuildRequesterInstanceKey = 0;
}

VOID CHumanPlayer::AcceptAddToGuildRequest(CHumanPlayer* pMember)
{
	CGuildEx* pGuild = pMember->GetGuild();
	if (pGuild != nullptr) return;
	pGuild = GetGuild();
	if (pGuild->AddMember(pMember))
	{
		pMember->UpdateViewName();
		SaySystemAttrib(CC_GREEN, "加入行会成功");
		pMember->SaySystemAttrib(CC_GREEN, "加入行会成功");
	}
}

ITEM* CHumanPlayer::GetWeapon()
{
	return m_Equipments.GetEquipment(_U_WEAPON);
}

ITEM* CHumanPlayer::GetDress()
{
	return m_Equipments.GetEquipment(_U_DRESS);
}

ITEM* CHumanPlayer::GetEquipment(int pos)
{
	return m_Equipments.GetEquipment(pos);
}

BOOL CHumanPlayer::RideHorse()
{
	if (m_pHorse != nullptr)
	{
		if (m_bRideHorse)
		{
			if (m_pMap == nullptr) return FALSE;
			POINT	pt;
			int x = getX();
			int y = getY();
			if (m_pMap->GetValidPoint(x, y, &pt, 1))
			{
				m_bRideHorse = FALSE;
				ForceMove(pt.x, pt.y);
				m_pHorse->setXY(x, y);
				m_pHorse->SetMapId(GetMapId());
				CGameWorld::GetInstance()->AddMapObject((CMapObject*)m_pHorse);
				m_HorseTimer.Savetime();
				return TRUE;
			}
		}
		else
		{
			//马牌1 212枣红马
			//马牌2 209雪龙
			//马牌3 206乌骓
			//马牌4 187黄金宝马
			//马牌5 211迎亲宝马
			//马牌6 184高头大马
			MonsterClass* pDesc = m_pHorse->GetDesc();
			char* szHorseName = pDesc->base.szClassName;
			if (szHorseName && (szHorseName[0] & 0xff) == 206)
			{
				CGuildEx* pGuild = GetGuild();
				if (!pGuild || !pGuild->IsMaster(this))
				{
					SaySystem("乌骓只能行会会长骑!");
					return FALSE;
				}
			}
			if (szHorseName && (szHorseName[0] & 0xff) == 209)
			{
				CGuildEx* pGuild = GetGuild();
				if (!pGuild || !pGuild->IsMaster(this) || pGuild != CSandCity::GetInstance()->GetOwnerGuild())
				{
					SaySystem("雪龙只能沙城行会会长骑!");
					return FALSE;
				}
			}
			if (m_ActionType != AT_STAND || IsSystemFlagSeted(SF_TRANSFORMED))
			{
				SaySystem("您现在的状态不能骑马!");
				return FALSE;
			}
			if (m_pHorse->GetMap() != this->GetMap() ||
				DISTANCE(getX(), getY(), m_pHorse->getX(), m_pHorse->getY()) > 2)
			{
				SaySystem("您的马不在身边!");
				return FALSE;
			}
			if (!m_HorseTimer.IsTimeOut(3000))
			{
				SaySystem("三秒钟之后才能骑马!");
				return FALSE;
			}
			m_HorseTimer.Savetime();
			int x = m_pHorse->getX();
			int y = m_pHorse->getY();
			//m_pHorse->GetMap()->RemoveObject(m_pHorse);
			CGameWorld::GetInstance()->RemoveMapObject((CMapObject*)m_pHorse);
			m_bRideHorse = TRUE;
			ForceMove(x, y);
			return TRUE;
		}
	}
	return FALSE;
}

VOID CHumanPlayer::CleanPets()
{
	if (m_iPetCount > 0)
	{
		std::array<CAliveObject*, 5> pPets{};
		memcpy(pPets.data(), m_pPets.data(), sizeof(m_pPets));
		int petcount = m_iPetCount;
		for (int i = 0; i < petcount; i++)
		{
			// 改为通过EP_DEAD延迟队列，避免主线程直接操作工作线程拥有的宠物对象
			pPets[i]->AddProcess(EP_DEAD, 0, 0, 0, 0, 2);
		}
		m_iPetCount = 0;
	}
	if (m_pHorse)
	{
		if (m_bRideHorse)
		{
			CMonsterManagerEx::GetInstance()->DeleteMonsterImm(m_pHorse);
			m_bRideHorse = FALSE;
		}
		else
		{
			ITEM* pItem = nullptr;
			if ((pItem = GetEquipment(_U_CHARM)) && pItem->baseitem.btStdMode == 33)
			{
				CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
				CMonsterManagerEx::GetInstance()->DeleteMonster(m_pHorse);
			}
			else if ((pItem = GetEquipment(_U_POISON)) && pItem->baseitem.btStdMode == 33)
			{
				CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
				CMonsterManagerEx::GetInstance()->DeleteMonster(m_pHorse);
			}
			else
				// 改为通过EP_DEAD延迟队列，避免主线程直接操作工作线程拥有的坐骑对象
				m_pHorse->AddProcess(EP_DEAD, 0, 0, 0, 0, 2);
		}
		m_pHorse = nullptr;
	}
	if (ISzhaohuan && m_pPet != nullptr)
	{
		//这里对豹子进行了释放操作
		CGameWorld::GetInstance()->RemoveMapObject(m_pPet);//从游戏世界移除这个对象
		CMonsterManagerEx::GetInstance()->DeleteMonster(m_pPet);//从怪物列表移除这怪物
		DelPet(m_pPet);
	}
}

VOID CHumanPlayer::SetPetTarget(CAliveObject* pObject)
{
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] && !m_pPets[i]->IsDeath() && m_pPets[i]->GetTarget() == nullptr)
			m_pPets[i]->SetTarget(pObject);
	}
}

VOID CHumanPlayer::UpdateViewName()
{
	o_strncpy(m_szLongName.data(), GetName(), m_szLongName.size() - 1);
	char szTemp[256];
	size_t nUsed = strlen(m_szLongName.data());
	size_t nRemain = (nUsed < m_szLongName.size() - 1) ? (m_szLongName.size() - 1 - nUsed) : 0;
	if (m_pGuild != nullptr) //	行会
	{
		const char* pGuildName = m_pGuild->GetName();
		if (m_pGuild == CSandCity::GetInstance()->GetOwnerGuild())
		{
			snprintf(szTemp, sizeof(szTemp), "\\%s(%s)", pGuildName, CSandCity::GetInstance()->GetName());
			strncat(m_szLongName.data(), szTemp, nRemain);
		}
		else if (CSandCity::GetInstance()->IsWarStarted() && InWarArea())
		{
			snprintf(szTemp, sizeof(szTemp), "\\%s(战争)", pGuildName);
			strncat(m_szLongName.data(), szTemp, nRemain);
		}
		else
		{
			CGuildMemberEx* pGuildMember = m_pGuild->GetMember(this);
			if (pGuildMember != nullptr)
			{
				snprintf(szTemp, sizeof(szTemp), "\\%s(%s)", pGuildName, pGuildMember->GetGroup()->GetName());
				strncat(m_szLongName.data(), szTemp, nRemain);
			}
			else
			{
				snprintf(szTemp, sizeof(szTemp), "\\%s", pGuildName);
				strncat(m_szLongName.data(), szTemp, nRemain);
			}
		}
		nUsed = strlen(m_szLongName.data());
		nRemain = (nUsed < m_szLongName.size() - 1) ? (m_szLongName.size() - 1 - nUsed) : 0;
	}
	if (m_sMaster[0] != 0 && nRemain > 0) //	师徒 
	{
		snprintf(szTemp, sizeof(szTemp), "\\(%s的徒弟)", m_sMaster.data());
		strncat(m_szLongName.data(), szTemp, nRemain);
		nUsed = strlen(m_szLongName.data());
		nRemain = (nUsed < m_szLongName.size() - 1) ? (m_szLongName.size() - 1 - nUsed) : 0;
	}
	if (m_sWife[0] != 0 && nRemain > 0) //	夫妻
	{
		if (m_Humandesc.dbinfo.btSex == 0)
			snprintf(szTemp, sizeof(szTemp), "\\(%s的丈夫)", m_sWife.data());
		else
			snprintf(szTemp, sizeof(szTemp), "\\(%s的妻子)", m_sWife.data());
		strncat(m_szLongName.data(), szTemp, nRemain);
		nUsed = strlen(m_szLongName.data());
		nRemain = (nUsed < m_szLongName.size() - 1) ? (m_szLongName.size() - 1 - nUsed) : 0;
	}
	m_szLongName[m_szLongName.size() - 1] = '\0';
	SendChangeName();
}

const char* CHumanPlayer::GetScriptVarValue(const char* pszName)
{
	//<$USERNAME> 人物名称 
	GetVariableStruct var;
	if (CScriptVariableManager::GetInstance()->GetVariable(pszName, this, var))
	{
		if (var.nType == 0)
		{
			snprintf(m_szTempScriptVarValue.data(), m_szTempScriptVarValue.size(), "%u", var.nValue);
			return m_szTempScriptVarValue.data();
		}
		else
			return var.pszValue;
	}
	else
		return "";
}

BOOL CHumanPlayer::EscapeMap()
{
	int	mapid, x, y;
	if (m_pMap == nullptr)return FALSE;
	if (m_pMap->IsFlagSeted(MF_NOESCAPE))
		return FALSE;
	if (CGameWorld::GetInstance()->GetStartPoint(m_Humandesc.dbinfo.szStartPoint, mapid, x, y))
		AddProcess(EP_RANDOMTELEPORT, mapid, 0, 0, 0, 1);
	else
		AddProcess(EP_RANDOMTELEPORT, 0, 0, 0, 0, 1);
	return TRUE;
}

BOOL CHumanPlayer::CanDoAction(actiontype action)
{
	if (m_bRideHorse)
	{
		if (action != AT_RUN && action != AT_WALK) // 骑马的时候不能作其他动作, 只能走路～!
			return FALSE;
	}
	else if (IsSystemFlagSeted(SF_TRANSFORMED))
	{
		if (action != AT_WALK && action != AT_PRIVATESHOP)
			return FALSE;
	}
	if (action == AT_SPECIALHIT)
	{
		return m_tmrSpecialAttackSkill.IsTimeOut(g_dwActionDelay[AT_ATTACK] - 80 * GetPropValue(PI_ATTACKSPEED));
	}
	return CAliveObject::CanDoAction(action);
}

BOOL CHumanPlayer::CanBearItem(ITEM& item)
{
	int maxWeight = GetPropValue(PI_MAXBAGWEIGHT);
	int curWeight = GetPropValue(PI_CURBAGWEIGHT);
	if (curWeight >= maxWeight)
		return FALSE;
	if (maxWeight - curWeight < item.baseitem.btWeight)
		return FALSE;
	return TRUE;
}

BOOL CHumanPlayer::SetMagicLevel(USERMAGIC* pMagic, int level)
{
	if (level < 0) level = 0;
	if (level > 3) level = 2;
	pMagic->magic.btLevel = level;
	pMagic->magic.dwCurTrain = 0;
	SendMagicExpChg(pMagic);
	return TRUE;
}

BOOL CHumanPlayer::SetMagicLevel(const char* pszName, int level)
{
	USERMAGIC* pMagic = this->GetMagicByName(pszName);
	if (pMagic == nullptr) return FALSE;
	if (level < 0) level = 0;
	if (level > 3) level = 2;
	pMagic->magic.btLevel = level;
	pMagic->magic.dwCurTrain = 0;
	SendMagicExpChg(pMagic);
	return TRUE;
}

VOID CHumanPlayer::SaveVars()
{
	char szPath[1024];
	_makepath(szPath, nullptr, ".\\Data\\Charvars", GetName(), "txt");
	if (GetScriptTarget())
		GetScriptTarget()->SaveVars(szPath);
}

VOID CHumanPlayer::LoadVars()
{
	char szPath[1024];
	_makepath(szPath, nullptr, ".\\Data\\Charvars", GetName(), "txt");
	if (GetScriptTarget())
		GetScriptTarget()->LoadVars(szPath);
}

VOID CHumanPlayer::InitMyConSkill()
{
	UpdateConSkillActive(this);
}

VOID CHumanPlayer::UpdateMyConSkillBuffs()
{
	UpdateConSkillBuffs(this);
}

VOID CHumanPlayer::ClearMyConSkillBuffs()
{
	ClearAllConSkillBuffs(this);
}

VOID CHumanPlayer::OnMyConSkillMagicSuccess(WORD wMagicID)
{
	OnConSkillMagicSuccess(this, wMagicID);
}