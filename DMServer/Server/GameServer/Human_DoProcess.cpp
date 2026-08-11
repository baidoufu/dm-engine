#include "stdafx.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "NpcManager.h"
#include "FireBurnEvent.h"
#include "SystemScript.h"
#include "LogicMap.h"
#include "SandCity.h"
#include "GameWorld.h"

VOID CHumanPlayer::DoProcess(OBJECTPROCESS* pProcess)
{
	if (pProcess == nullptr) return;
	switch (pProcess->ident)
	{
	case EP_PUTITEM:
	{
		SendMsg(0, 0x8850, 0, 0, 0, (LPVOID)pProcess->pszParam);
	}
	break;
	case EP_EXCHANGEBOX:
	{
		xPacketPool::ScopedPacket packet;
		DWORD nLooks = pProcess->dwParam[0];
		packet->push(&nLooks, 2);
		packet->push(pProcess->pszParam);
		packet->push(1);
		SendMsg(GetId(), 0x273, MAKEWORD(6, pProcess->dwParam[1]), 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
		SaySystem("恭喜 %s 宝箱开启, 获得 %s 物品!", GetName(), pProcess->pszParam);
		CreateBagItem(pProcess->pszParam);
	}
	break;
	case EP_MAPTELEPORT:
	{
		DWORD dwMapId = GetMapId();
		if (dwMapId >= pProcess->dwParam[0] &&
			dwMapId <= pProcess->dwParam[1])
		{
			if (pProcess->dwParam[3] == 0)
				RandomTeleport(pProcess->dwParam[2]);
			else
				FlyTo(pProcess->dwParam[2], pProcess->dwParam[3] & 0xffff, (pProcess->dwParam[3] & 0xffff0000) >> 16);
		}
	}
	break;
	case EP_WEATHERCHANGED:
	{
		if (pProcess->dwParam[0] == GetMapId()) SendTimeWeatherChanged();
	}
	break;
	case EP_TIMECHANGED:
	{
		// 取得当前地图的天气, 然后结合自己的时间
		SendTimeWeatherChanged();
	}
	break;
	case EP_CATCHHORSE:
	{
		if (m_pHorse)
		{
			ITEM item;
			if (CItemManager::GetInstance()->CreateTempItem("马牌", item))
			{
				o_strncpy((char*)&item.baseitem.btMinDef, m_pHorse->GetName(), 10);
				ITEM old = item;
				if (EquipItem(_U_CHARM, item, FALSE, FALSE))
				{
					if (item.dwMakeIndex != 0) AddBagItem(item);
					SendMsg(GetId(), SM_ADDBAGITEM, 0, 0, 0, &old, sizeof(old));
					SendMsg(old.dwMakeIndex, 0x23, _U_CHARM, 0, 0);
				}
			}
		}
	}
	break;
	case EP_GODBLESS:
	{
		SendAroundMsg(GetId(), 0x44d, static_cast<WORD>(pProcess->dwParam[0] >> 16), static_cast<WORD>(pProcess->dwParam[0] & 0xffff), static_cast<WORD>(pProcess->dwParam[1]));
		SendMsg(GetId(), 0x44d, static_cast<WORD>(pProcess->dwParam[0] >> 16), static_cast<WORD>(pProcess->dwParam[0] & 0xffff), static_cast<WORD>(pProcess->dwParam[1]));
	}
	break;
	case EP_SPECIALPOTION:
	{
		DWORD Idx = pProcess->dwParam[0]; // 索引序号
		DWORD nPower = pProcess->dwParam[1]; // 效果值
		DWORD nTime = pProcess->dwParam[2] / 1000; // 持续时间
		static constexpr std::array<SpecialPotionInfo, 6> specialPotionInfo = { {
			{PI_MAXDC,       "攻击力增加%d秒"},
			{PI_MAXMC,       "魔法力增加%d秒"},
			{PI_MAXSC,       "道术增加%d秒"},
			{PI_ATTACKSPEED, "攻击速度增加%d秒"},
			{PI_MAXHP,       "生命值增加%d秒"},
			{PI_MAXMP,       "魔法值增加%d秒"},
		} };
		if (pProcess->repeattimes == 1)
			DecProp(specialPotionInfo[Idx].propIndex, nPower);
		else
		{
			AddProp(specialPotionInfo[Idx].propIndex, nPower);
			SaySystemAttrib(CC_GREEN, specialPotionInfo[Idx].szMsg, nTime);
		}
		UpdateProp();
	}
	break;
	case EP_APPEAR:
	{
		Show();
	}
	break;
	case EP_WARSTART:
	{
		if (InWarArea())
		{
			SendMsg(4, 0x2c4, 0, 0, 0);
			UpdateViewName();
		}
		else if (!InCityArea())
			SendChangeName();
	}
	break;
	case EP_WAREND:
	{
		if (InWarArea())
		{
			SendMsg(0, 0x2c4, 0, 0, 0);
			UpdateViewName();
		}
		else if (!InCityArea())
			SendChangeName();
	}
	break;
	case EP_CREATEBAGITEM:
	{
		if (pProcess->dwParam[0] > 0)
		{
			for (DWORD i = 0; i < pProcess->dwParam[0]; i++)
			{
				CreateBagItem(pProcess->pszParam);
			}
		}
		else
			CreateBagItem(pProcess->pszParam);
	}
	break;
	case EP_RANDOMTELEPORT:
	{
		if (pProcess->dwParam[0] == 0)
			RandomTeleport();
		else
			RandomTeleport(pProcess->dwParam[0]);
	}
	break;
	case EP_HOME:
	{
		Home();
	}
	break;
	case EP_SABUKHOME:
	{
		CSandCity::GetInstance()->Home(this);
	}
	break;
	case EP_SENDCODEDTEXT:
	{
		if (m_pClientObj != nullptr)
			m_pClientObj->PostMsg(pProcess->pszParam, (int)strlen(pProcess->pszParam));
	}
	break;
	case EP_SCROLLTEXT:
	{
		SendScrollText(pProcess->pszParam);
	}
	break;
	case EP_SECNONDTIMEOUT:
	{
		SendMsg(pProcess->dwParam[0], 0x0339, static_cast<WORD>(pProcess->dwParam[1]), static_cast<WORD>(pProcess->dwParam[2]), 0, pProcess->pszParam);
	}
	break;
	case EP_RECOVERHP:
	{
		WORD curhp = static_cast<WORD>(GetPropValue(PI_CURHP));
		WORD maxhp = static_cast<WORD>(GetPropValue(PI_MAXHP));
		if (IsDeath())
		{
			pProcess->repeattimes = 0;
			SendMsg(GetId(), 510, 0, 0, 1);
			break;
		}
		if (pProcess->repeattimes == 1)
			SendMsg(GetId(), 510, 0, 0, 1);
		if (curhp == maxhp) break;
		WORD addHp = (WORD)pProcess->dwParam[0];
		if (addHp >= maxhp - curhp)
			curhp = maxhp;
		else
			curhp += addHp;
		m_Humandesc.dbinfo.hp = curhp;
		SendHpMpChanged(-addHp);
	}
	break;
	case EP_RECOVERMP:
	{
		WORD curmp = GetPropValue(PI_CURMP);
		WORD maxmp = GetPropValue(PI_MAXMP);
		if (IsDeath())
		{
			pProcess->repeattimes = 0;
			SendMsg(GetId(), 510, 0, 0, 2);
			break;
		}
		if (pProcess->repeattimes == 1)
			SendMsg(GetId(), 510, 0, 0, 2);
		if (curmp == maxmp) break;
		WORD addMp = (WORD)pProcess->dwParam[0];
		if (addMp >= maxmp - curmp)
			curmp = maxmp;
		else
			curmp += addMp;
		m_Humandesc.dbinfo.mp = curmp;
		SendHpMpChanged();
	}
	break;
	case EP_FIRSTLOGINPROCESS:
	{
		//赠送物品
		FIRSTLOGIN_INFO* pInfo = CGameWorld::GetInstance()->GetFirstLoginInfo();
		FIRSTLOGIN_ITEM* pItem = pInfo->pItems;
		while (pItem)
		{
			for (int i = 0; i < pItem->nCount; i++)
			{
				if (pItem->btJob != 99 && pItem->btJob != GetPro())
					continue;
				if (pItem->btSex != 99 && pItem->btSex != GetSex())
					continue;
				CreateBagItem(pItem->szItem, pItem->boBind);
			}
			pItem = pItem->pNext;
		}
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "FirstEnv.First", FALSE);
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "LoginEnv.Login", FALSE);
		//清除首次登录记录
		ClearFirstLogin();
	}
	break;
	case EP_CLOSEPAGE:
	{
		SendMsg(pProcess->dwParam[0], 0x284, 0, 0, 0, nullptr);
	}
	break;
	case EP_OPENSCRIPTPAGE:
	{
		if (pProcess->dwParam[0] == 0xffffffff)
		{
			if (!CSystemScript::GetInstance()->Execute(GetScriptTarget(), pProcess->pszParam, FALSE))
				SendCloseScriptPage(0xffffffff);
			break;
		}
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pProcess->dwParam[0]);
		if (pNpc != nullptr)
			pNpc->QuerySelectLink(this, pProcess->pszParam, FALSE);
	}
	break;
	case EP_FIREBURN: // 火墙
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		CFireBurnEvent::Create(this, x, y, pProcess->dwParam[1], pProcess->dwParam[2], pProcess->dwParam[3] * 1000);
	}
	break;
	case EP_POISON: // 施毒术
	{
		BOOL bSuccess = TRUE;
		DWORD nTarget = pProcess->dwParam[0];
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(nTarget); // 攻击目标
		DWORD needitem = pProcess->dwParam[1];
		DWORD du = pProcess->dwParam[2];
		int hongdu = du & 0xffff; // 低位
		int lvdu = (du & 0xffff0000) >> 16; // 高位
		int nPower = pProcess->dwParam[3];
		if (!pObject->IsSkillTime(6)) break;
		nPower = nPower * (1 - pObject->GetPropValue(PI_POISONRECOVER));
		if (needitem == SNI_REDPOISON) // 红毒30
		{
			pObject->SetStatus(SI_REDPOISON, hongdu, nPower * 1000);
			pObject->SetSetter(SI_REDPOISON, GetId()); // 设置施毒者
		}
		else if (needitem == SNI_GREENPOISON) // 绿毒31
		{
			pObject->SetStatus(SI_GREENPOISON, lvdu, nPower * 1000);
			pObject->SetSetter(SI_GREENPOISON, GetId()); // 设置施毒者
		}
		else
			bSuccess = FALSE;
		int escape = pObject->GetPropValue(PI_POISONESCAPE);
		if (escape > 0)
		{
			if (GetPropValue(PI_POISONHITRATE) < Getrand(escape))
				bSuccess = FALSE;
		}
		if (nPower <= 0)
			bSuccess = FALSE;
		//	防止隐身毒
		if (bSuccess)
		{
			if (pObject->IsProperTarget(this))
				pObject->SetTarget(this);
			CheckPk(pObject);
			SetPetTarget(pObject);
		}
	}
	break;
	case EP_STRAW: // 诅咒术
	{
		BOOL bSuccess = TRUE;
		DWORD nTarget = pProcess->dwParam[0];
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(nTarget); // 攻击目标
		DWORD needitem = pProcess->dwParam[1];
		DWORD du = pProcess->dwParam[2];
		int value2 = du & 0xffff; // 低位
		int value1 = (du & 0xffff0000) >> 16; // 高位
		int nPower = pProcess->dwParam[3];
		if (!pObject->IsSkillTime(45)) break;
		nPower = nPower * (1 - pObject->GetPropValue(PI_POISONRECOVER));
		if (needitem == SNI_STRAWMAN)
			pObject->SetStatus(SI_STRAWMAN, value2, nPower * 1000);
		else if (needitem == SNI_STRAWWOMAN)
			pObject->SetStatus(SI_STRAWWOMAN, value1, nPower * 1000);
		else
			bSuccess = FALSE;
		int escape = pObject->GetPropValue(PI_POISONESCAPE);
		if (escape > 0)
		{
			if (GetPropValue(PI_POISONHITRATE) < Getrand(escape))
				bSuccess = FALSE;
		}
		if (nPower <= 0)
			bSuccess = FALSE;
		//	防止隐身毒
		if (bSuccess)
		{
			if (pObject->IsProperTarget(this))
				pObject->SetTarget(this);
			CheckPk(pObject);
			SetPetTarget(pObject);
		}
	}
	break;
	case EP_DEFENCEUP: // 幽灵盾
	case EP_MAGDEFENCEUP: // 神圣战甲术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		MagMakeDefenceArea(x, y, pProcess->dwParam[1], pProcess->dwParam[2], pProcess->dwParam[3]);
	}
	break;
	case EP_GROUPCLOAK: // 群体隐身术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		for (int i = 0; i < 9; i++)
		{
			int px = x;
			int py = y;
			if (i < 8) GETNEXTPOS(px, py, i);
			CMapCellInfo* pInfo = m_pMap != nullptr ? m_pMap->GetMapCellInfoShared(px, py) : nullptr;
			if (pInfo)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObject = pNode->getObject();
					pNode = pNode->getNext();
					if (pObject)
					{
						if (pObject->GetClassType() == CLS_ALIVEOBJECT && IsProperFriend((CAliveObject*)pObject))
						{
							DWORD dwParam = (pObject->getX() << 16) | pObject->getY();
							if (!((CAliveObject*)pObject)->IsStatusSet(SI_CLOAK))
								((CAliveObject*)pObject)->AddProcess(EP_SETSTATUS, SI_CLOAK, dwParam, pProcess->dwParam[1] * 1000);
						}
					}
				}
			}
		}
	}
	break;
	case EP_GROUPMAGHEALING: // 群体治愈术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		int nRange = pProcess->dwParam[1]; // 范围
		int nHP = pProcess->dwParam[2]; // 回血总量
		int nTimeHp = pProcess->dwParam[3]; //每次加血量
		for (int i = 0; i < nRange; i++)
		{
			int px = x;
			int py = y;
			CMapCellInfo* pInfo = m_pMap != nullptr ? m_pMap->GetMapCellInfoShared(px, py) : nullptr;
			if (i < 8) GETNEXTPOS(px, py, i);
			if (pInfo)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObject = pNode->getObject();
					pNode = pNode->getNext();
					if (pObject)
					{
						if (pObject->GetClassType() == CLS_ALIVEOBJECT && IsProperFriend((CAliveObject*)pObject))
							((CAliveObject*)pObject)->AddProcess(EP_MAGHEALING, nHP, nTimeHp, 0, 0, 600);
					}
				}
			}
		}
	}
	break;
	case EP_SKILLFLY:
	{
		if (m_pMap)
		{
			m_pMap->MoveObject(this, pProcess->dwParam[0], pProcess->dwParam[1]);
			m_wActionX = static_cast<WORD>(pProcess->dwParam[0]);
			m_wActionY = static_cast<WORD>(pProcess->dwParam[1]);
		}
	}
	break;
	default:
	{
		CAliveObject::DoProcess(pProcess);
	}
	break;
	}
}
