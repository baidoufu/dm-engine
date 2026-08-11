#include "StdAfx.h"
#include "humanplayer.h"
#include "gameworld.h"
#include "sandcity.h"
#include "LogicMap.h"
#include "guildex.h"
#include "SystemScript.h"
#include "ItemManager.h"
#include "ConSkill.h"

static std::array<char, 65536> g_szTempString{};
static std::array<ITEM, 100> g_items{};
typedef struct tagDropEquipment
{
	ITEM* pItem;
	int	pos;
}DropEquipment;

VOID CHumanPlayer::OnDeath(DWORD dwKiller)
{
	CleanPets();
	ClearAllConSkillBuffs(this); // 死亡时清除连击Buff
	CSystemScript::GetInstance()->Execute(GetScriptTarget(), "PlayerEnv.OnDeath", FALSE); // 执行死亡触发脚本
	CAliveObject* pObj = CGameWorld::GetInstance()->GetAliveObjectById(dwKiller);
	BOOL bPersonKill = TRUE;
	if (m_pMap && m_pMap->IsFlagSeted(MF_FIGHTMAP))
		bPersonKill = FALSE;
	if (bPersonKill)
	{
		xPacketPool::ScopedPacket packet(65536);
		int bagitemcount = 0;
		int dropcount = 0;
		std::array<ITEM*, 100> Items{};
		ITEM item;
		char szText[1024];
		std::array<DropEquipment, 20> equipments{};
		int equipmentcount = 0;
		//	获取所有合法物品的指针
		for (int i = 0; i < m_ItemBox.GetCount(); i++)
		{
			Items[bagitemcount] = m_ItemBox.GetItem(i);
			if (Items[bagitemcount])
			{
				if (CItemManager::GetInstance()->ItemLimited(*Items[bagitemcount], IL_NODEADDROP))
					continue;
				bagitemcount++;
			}
		}
		for (int i = 0; i < _U_MAX; i++)
		{
			equipments[equipmentcount].pItem = m_Equipments.GetEquipment(i);
			if (equipments[equipmentcount].pItem)
			{
				if (CItemManager::GetInstance()->ItemLimited(*equipments[equipmentcount].pItem, IL_NODEADDROP))
					continue;
				equipments[equipmentcount].pos = i;
				equipmentcount++;
			}
		}
		//	获取掉罗个数
		int dropbagtotal = 0;
		int dropequipmenttotal = 0;
		if (GetPkValue() < CGameWorld::GetInstance()->GetVar(EVI_YELLOWPKPOINT))
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0 && Getrand(10) == 0)dropequipmenttotal = 1;
		}
		else if (GetPkValue() < CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0 && Getrand(5) == 0)dropequipmenttotal = 1;
		}
		else
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0)dropequipmenttotal = 1;
		}
		//	乱续
		for (int i = 0; i < bagitemcount; i++)
		{
			int idx1 = Getrand(bagitemcount);
			int idx2 = Getrand(bagitemcount);
			if (idx1 == idx2)continue;
			ITEM* p = Items[idx1];
			Items[idx1] = Items[idx2];
			Items[idx2] = p;
		}
		//	对必掉和消失的下手
		for (int i = 0; i < bagitemcount; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(*Items[i], IL_DEADDELETE))
			{
				item = *Items[i];
				CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
				SendTakeBagItem(&item);
			}
			else if (CItemManager::GetInstance()->ItemLimited(*Items[i], IL_DEADDROP))
			{
				if (!DropItem(*Items[i]))continue;
				item = *Items[i];
			}
			else
				continue;
			this->m_ItemBox.RemoveItem(Items[i]->dwMakeIndex);
			o_strncpy(szText, item.baseitem.szName, 14);
			snprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), "/%u/", item.dwMakeIndex);
			packet->push((LPVOID)szText, (int)strlen(szText));
			dropcount++;
			dropbagtotal--;
			Items[i] = nullptr;
		}
		//	第二轮掉落
		if (dropbagtotal > 0)
		{
			for (int i = 0; i < bagitemcount; i++)
			{
				if (Items[i] && DropItem(*Items[i]))
				{
					item = *Items[i];
					this->m_ItemBox.RemoveItem(Items[i]->dwMakeIndex);
					o_strncpy(szText, item.baseitem.szName, 14);
					snprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), "/%u/", item.dwMakeIndex);
					packet->push((LPVOID)szText, (int)strlen(szText));
					dropcount++;
					Items[i] = nullptr;
					dropbagtotal--;
					if (dropbagtotal <= 0)break;
				}
			}
		}
		//	获取有装备的位置
		for (int i = 0; i < equipmentcount; i++)
		{
			int idx1 = Getrand(equipmentcount);
			int idx2 = Getrand(equipmentcount);
			if (idx1 == idx2)continue;
			DropEquipment a = equipments[idx1];
			equipments[idx1] = equipments[idx2];
			equipments[idx2] = a;
		}
		//	第一轮掉落
		int operation = 0;	//	drop
		for (int i = 0; i < equipmentcount; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(*equipments[i].pItem, IL_EQUDEADDELETE))
				operation = 1;	//	delete
			else if (CItemManager::GetInstance()->ItemLimited(*equipments[i].pItem, IL_DEADDROP))
				operation = 0;	//	drop
			else
				continue;
			if (m_Equipments.UnEquipItem(equipments[i].pos, item))
			{
				if (operation == 0)
					DropItem(item);
				else
				{
					CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
					SendTakeBagItem(&item);
				}
				o_strncpy(szText, item.baseitem.szName, 14);
				snprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), "/%u/", item.dwMakeIndex);
				packet->push((LPVOID)szText, (int)strlen(szText));
				equipments[i].pItem = nullptr;
				dropcount++;
				dropequipmenttotal--;
			}
		}
		//	第二轮掉落
		if (dropequipmenttotal > 0)
		{
			for (int i = 0; i < equipmentcount; i++)
			{
				if (equipments[i].pItem == nullptr)
					continue;
				if (m_Equipments.UnEquipItem(equipments[i].pos, item))
				{
					DropItem(item);
					o_strncpy(szText, item.baseitem.szName, 14);
					snprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), "/%u/", item.dwMakeIndex);
					packet->push((LPVOID)szText, (int)strlen(szText));
					equipments[i].pItem = nullptr;
					dropcount++;
					dropequipmenttotal--;
					if (dropequipmenttotal <= 0)break;
				}
			}
		}
		//	发送消息
		if (packet->getsize() > 0)
			SendMsg(0, 0x2c5, 0, 0, dropcount, (LPVOID)packet->getbuf(), packet->getsize());
	}
	if (pObj && pObj->GetType() == OBJ_PET)
		pObj = pObj->GetOwner();
	if (pObj)
		pObj->OnKillTarget(this);
	if (pObj && pObj->GetType() == OBJ_PLAYER)
	{
		CHumanPlayer* pKiller = (CHumanPlayer*)pObj;
		if (pKiller->GetGuild() && pKiller->GetGuild()->IsKillGuildMember(this))
		{
			//GetTickCount64();
		}
		else
		{
			if (!this->NoLawProtect())
			{
				if (!(CSandCity::GetInstance()->IsWarStarted() && InWarArea()))
				{
					((CHumanPlayer*)pObj)->AddPkPoint(CGameWorld::GetInstance()->GetVar(EVI_ONCEPKPOINT));
					ITEM* pWeapon = ((CHumanPlayer*)pObj)->GetEquipment(_U_WEAPON);
					if (pWeapon && Getrand(100) < static_cast<int>(CGameWorld::GetInstance()->GetVar(EVI_PKCURSERATE)))
					{
						if (pWeapon->baseitem.Ac1 > 0)
						{
							pWeapon->baseitem.Ac1--;
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪, 武器被诅咒了");
							CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
							((CHumanPlayer*)pObj)->SendUpdateItem(*pWeapon);
							((CHumanPlayer*)pObj)->m_Equipments.ResetPropCache();
							((CHumanPlayer*)pObj)->UpdateProp();
							((CHumanPlayer*)pObj)->UpdateSubProp();
						}
						else if (pWeapon->baseitem.Mac1 < 10)
						{
							pWeapon->baseitem.Mac1++;
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪, 武器被诅咒了");
							CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
							((CHumanPlayer*)pObj)->SendUpdateItem(*pWeapon);
							((CHumanPlayer*)pObj)->m_Equipments.ResetPropCache();
							((CHumanPlayer*)pObj)->UpdateProp();
							((CHumanPlayer*)pObj)->UpdateSubProp();
						}
						else
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪");
					}
					else
						((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪");
				}
			}
			else
				((CHumanPlayer*)pObj)->SaySystem("你实施了正当防卫");
		}
		SaySystem("您被 %s 杀死了!", pObj->GetName());
	}
}

VOID CHumanPlayer::OnSystemFlagCleared(int index, DWORD dwParam)
{
	if (index == SF_SPECIALNAMECOLOR) // 聊天名字变色
		SendChangeName();
	else if (index == SF_BANED)
		SaySystem("您的聊天功能被恢复!");
	else if (index == SF_GODBLESS) // 附体效果
	{
		SaySystem("上古神魔的力量慢慢弱去, 你的身体恢复正常.");
		SendSpecialStatusChanged();
	}
	else if (index == SF_WINDSHIELD) // 风影盾
	{
		USERMAGIC* pMagic = GetMagic(70);
		if (pMagic == nullptr) return;
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		DecProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value3);//恢复躲避
		DecProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value4);//恢复魔法躲避
		DecProp(PI_POISONESCAPE, magicskill.skills[pMagic->magic.btLevel].value5);//恢复中毒躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
		SaySystemAttrib(CC_GREENS, "你的风影盾已经消散!");
	}
	else if (index == SF_STRONGSHIELD) // 金刚护体
	{
		USERMAGIC* pMagic = GetMagic(61);
		if (pMagic == nullptr) return;
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		DecProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//恢复躲避
		DecProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//恢复魔法躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
		SaySystemAttrib(CC_GREENS, "你的金刚护体已经被击碎!");
	}
	else if (index == SF_KUANGNUWIND) // 狂怒旋风
	{
		USERMAGIC* pMagic = GetMagic(107);
		if (pMagic == nullptr) return;
		//Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		//DecProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//恢复躲避
		//DecProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//恢复魔法躲避
		//UpdateProp();
		//UpdateSubProp();
		SendSpecialStatusChanged();
		SaySystemAttrib(CC_GREENS, "你的狂怒旋风已经被击碎!");
	}
	else if (index == SF_TRANSFORMED)
	{
		SendAroundMsg(GetId(), 0x532c, 0x40, 0, 0);
		SendMsg(GetId(), 0x532c, 0x40, 0, 0);
	}
}

VOID CHumanPlayer::OnSystemFlagSeted(int index, DWORD dwParam)
{
	if (index == SF_GODBLESS)
	{
		int type = m_SystemFlag.GetParam(index) & 0xffff;
		DWORD dwSecond = m_SystemFlag.GetTimeOut(index) / 1000;
		switch (type)
		{
		case 1:
		{
			SaySystem("上古的神魔赋予了你一股神奇的力量,");
			SaySystem("你获得了秒杀的神力,");
			SaySystem("神力将持续%u分钟……", dwSecond / 60);
		}
		break;
		case 2:
		{
			SaySystem("上古的神魔赋予了你一股神奇的力量,");
			SaySystem("你获得了重击的神力,");
			SaySystem("神力将持续%u分钟…… ", dwSecond / 60);
		}
		break;
		case 3:
		{
			SaySystem("来自上古的神魔赐予了你一股神奇的力量,");
			SaySystem("你获得了神御的神力,");
			SaySystem("神力将持续%u分钟……", dwSecond / 60);
		}
		break;
		case 4:
		{
			SaySystem("来自上古的神魔赐予了你一股神奇的力量");
			SaySystem("你获得了神佑的神力");
			SaySystem("神魔的眷顾可能让你在%u分钟内获得双倍的经验", dwSecond / 60);
			SaySystem("进入炼狱更有可能获得三倍……");
		}
		break;
		}
		SendSpecialStatusChanged();
	}
	else if (index == SF_WINDSHIELD)
	{
		USERMAGIC* pMagic = GetMagic(70);
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		AddProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value3);//增加躲避
		AddProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value4);//增加魔法躲避
		AddProp(PI_POISONESCAPE, magicskill.skills[pMagic->magic.btLevel].value5);//增加中毒躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
	}
	else if (index == SF_STRONGSHIELD)
	{
		USERMAGIC* pMagic = GetMagic(61);
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		AddProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//增加躲避
		AddProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//增加魔法躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
	}
	else if (index == SF_KUANGNUWIND)
	{
		USERMAGIC* pMagic = GetMagic(61);
		//Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		//AddProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//增加躲避
		//AddProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//增加魔法躲避
		//UpdateProp();
		//UpdateSubProp();
		SendSpecialStatusChanged();
	}
	else if (index == SF_TRANSFORMED)
	{
		SendAroundMsg(GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff);
		SendMsg(GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff);
	}
}

VOID CHumanPlayer::OnStatusSet(int index, DWORD dwParam)
{
	CAliveObject::OnStatusSet(index, dwParam);
	UpdateProp();
}

VOID CHumanPlayer::OnStatusClr(int index, DWORD dwParam)
{
	CAliveObject::OnStatusClr(index, dwParam);
	UpdateProp();
}

VOID CHumanPlayer::OnLeaveMap(CLogicMap* pMap)
{
	if (pMap->IsFlagSeted(MF_SABUKPALACE) && CSandCity::GetInstance()->IsWarStarted())
		CSandCity::GetInstance()->OnLeavePalace(this);
	CAliveObject::OnLeaveMap(pMap);
	m_bEnterMap = FALSE;
}

VOID CHumanPlayer::OnEnterMap(CLogicMap* pMap)
{
	SendMsg(GetId(), SM_SETMAP, m_wX, m_wY, 0, (LPVOID)pMap->GetName());//人物在地图的位置
	const std::array<DWORD, 4> dwParam = { GetFeather(), GetStatus(), GetGroupObject() != nullptr, 0 };
	SendMsg(GetId(), SM_SETPLAYER, m_wX, m_wY, GetSex() << 8 | GetDirection(), (LPVOID)dwParam.data(), sizeof(DWORD) * 4);//人物外观更新
	CAliveObject::OnEnterMap(pMap);
	SendTimeWeatherChanged();
	UpdateViewName();
	if (GetStatus() > 0)
		SendStatusChanged();
	if (IsSystemFlagSeted(SF_COMMUNITYLOADED))
		UpdateCommunityInfoToClient(FALSE);
	BOOL bSendSpecialStatus = FALSE;
	if (IsSystemFlagSeted(SF_GODBLESS))
	{
		bSendSpecialStatus = TRUE;
		if (!IsSystemFlagSeted(SF_WINDSHIELD) && !IsSystemFlagSeted(SF_STRONGSHIELD))
			SendMsg(GetId(), 0x532c, 0, 0, 0);
	}
	if (IsSystemFlagSeted(SF_WINDSHIELD) || IsSystemFlagSeted(SF_STRONGSHIELD))
		bSendSpecialStatus = TRUE;
	if (bSendSpecialStatus)
		SendSpecialStatusChanged(FALSE);
	//	附身
	//if( IsSystemFlagSeted(SF_GODBLESS) )
	//{
	//	SendMsg( GetId(), 0x532c, 0, 0, 0 );
	//	SendMsg( GetId(), 0x532c, 4, m_SystemFlag.GetTimeOut(SF_GODBLESS)/1000, 0 );
	//}
	//	如果地图是沙城宫殿, 并且攻城战已经开始, 那么就调用进入宫殿的事件
	if (pMap->IsFlagSeted(MF_SABUKPALACE) && CSandCity::GetInstance()->IsWarStarted())
		CSandCity::GetInstance()->OnEnterPalace(this);
	RefreshSpecialEquipment();
	m_bEnterMap = TRUE;
}

VOID CHumanPlayer::OnLevelUp(int level)
{
	if (level > 45) // 46级开始才有封号判断
		CheckAndUpgradeTitle();
	CSystemScript::GetInstance()->Execute(GetScriptTarget(), "LevelUpEnv.LevelUp", FALSE);
}

VOID CHumanPlayer::OnMagicLevelup(USERMAGIC* pMagic)
{
	if (pMagic->pClass->dwFlag & MAGICFLAG_FORCED)
	{
		switch (pMagic->magic.wId)
		{
		case 74://	擒龙手
		case 7:	//	攻杀剑法
		case 4:	//	精神战法
		case 3:	//	初级剑法
		{
			RecalcHitSpeed();
			UpdateSubProp();
		}
		break;
		}
	}
}

#ifdef _DEBUG
//static char szDecodeBuffer[65536];
#endif
VOID CHumanPlayer::OnAroundMsg(CMapObject* pSender, const char* pszCodedMsg, int size)
{
	if (m_pClientObj == nullptr) return;
	assert(m_pClientObj != nullptr);
#ifdef _DEBUG // 打印发出去的日志
	//if (pszCodedMsg && size > 0)
	//{
	//	if (pszCodedMsg[0] == '#')
	//	{
	//		char* pStart = (char*)pszCodedMsg + 1;
	//		if (*pStart >= '0' && *pStart <= '9') pStart++;
	//		int length = _UnGameCode(pStart, (BYTE*)szDecodeBuffer);
	//		CServer::GetInstance()->OnUnknownMsg((PMIRMSG)szDecodeBuffer, length, 2);
	//	}
	//}
#endif
	m_pClientObj->PostMsg(pszCodedMsg, size);
}
