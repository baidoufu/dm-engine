#include "GameControl.h"
#include "GameData\MagicDefine.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameData.h"
#include "GameMap/GameMap.h"
#include "Global/StringUtil.h"
#include "GameData/Character.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameData/OtherData.h"
#include "GameData/TalkMgr.h"
#include "Global/Interface/AudioInterface.h"
#include "GameAI/AIMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameAI/EffectMsgFilter.h"
#include "GameData/FindGood.h"
#include "GameData/PetData.h"
#include "GameData/TaskData.h"
#include "GameData/NpcData.h"
#include "GameData/BoothData.h"
#include "GameAI/AIAutoEatMgr.h"
#include "BaseClass/Control/GoodGrid.h"
#include "GameAI/AIAutoFightMgr.h"
#include "GameAI/AIAutoMgr.h"
#include "GameData/ActionStruct.h"
#include "GameData/LoginData.h"
#include "GameData/GuildData.h"
#include "GameData/MsgBoxMgr.h"
#include "Global/StringUtil.h"
#include "BaseClass/Compress/Compr.h"
#include "WndClass/GameWnd/GameManager.h"
#include "GameAI/AIConfigMgr.h"
#include "GameClient/SDOAInterface.h"
#include "GameData/XmlCfg.h"
#include "GameAI/AIGoodMgr.h"
#include "WndClass/GameWnd/NpcQuickWnd.h"
#include "BaseClass/Misc/AntiWaiGua.h"


// 收到的消息处理
void CGameControl::MSG_Player_Success(const char * msg, int iLen)
{
	SELF.SetWaitServer(false,true);

	int iX = Conv_WORD(msg+6);
	int iY = Conv_WORD(msg+8);

	//int iOldX,iOldY;
	//SELF.GetRealXY(iOldX,iOldY);
	//output_debug("MSG_Player_Success:OldX:%d,OldY:%d,iX:%d,iY:%d,wAction:%d\r\n",iOldX,iOldY,iX,iY,SELF.GetAction().wAction);


	SELF.SetRealXY(iX,iY);

	switch(g_AIMgr.GetAI_Action())
	{
	case CAIMgr::AI_ACTION_MOVE:
		{
			g_AutoPickMgr.MoveSuccess();
			//g_AIMgr.UpdateMoveTime(TRUE);
		}
		break;
	case CAIMgr::AI_ACTION_ATTACK:
		{
			//g_AIMgr.UpdateAttackTime(TRUE);
		}
		break;
	case CAIMgr::AI_ACTION_MAGIC:
		{
			//g_AIMgr.UpdateMagicTime(0,TRUE);
		}
		break;
	}
}

void CGameControl::MSG_Player_Fail(const char * msg, int iLen)
{
	int iX = Conv_WORD(msg + 6);
	int iY = Conv_WORD(msg + 8);
	DWORD dwFailType =  Conv_DWORD(msg);
	WORD  wMagicID = Conv_WORD(msg + 10);


	int iOldX,iOldY;
	SELF.GetRealXY(iOldX,iOldY);
	output_debug("MSG_Player_Fail:OldX:%d,OldY:%d,iX:%d,iY:%d,wAction:%d\r\n",iOldX,iOldY,iX,iY,SELF.GetAction().wAction);

	SELF.SetWaitServer(false,false);
	if(SELF.IsDead() == false && SELF.GetNextActionNum() == 0)
	{
		SNextAction &pNextAction = SELF.GetCNextAction();
		SAction &actionNow = SELF.GetAction();
		if((pNextAction.wAction==ACTION_ATTACK1 || actionNow.wAction ==  ACTION_ATTACK1 )&& g_AIMgr.HaveChopEquip())	// 挖矿时不需要停下来
			return;	

	}

	SELF.SetXY(iX,iY);
	SELF.SetRealXY(iX,iY);
	SELF.SetOffset(0,0);
	SELF.SetDrawOffset(0,0);
	g_AIMgr.SetReserveTime(plyLastFailTime,GetTickCount());

	switch(dwFailType)
	{
	case ACTION_WALK:
	case ACTION_RUN:
		{
			g_AutoPickMgr.MoveFailed(iX,iY);
		}
		break;
	case ACTION_ATTACK1:
		{
		}
		break;
	case ACTION_MAGIC:
		{
			switch(wMagicID)
			{
			case MAGICID_ZHI_YAN:
				g_AIMgr.SetReserveTime(plyLastZhiYanTime,0);
				break;
			case MAGICID_SWT:
			case MAGICID_LSJ:
			case MAGICID_JYL:
				g_AIMgr.SetReserveTime(plyLastTaoZhuangTime,0);
				break;
			case MAGICID_COLDSTROM:
				g_AIMgr.SetReserveTime(plyLastColdstrom,0);
				break;
			case MAGICID_ENCHANTER_WILD:
			case MAGICID_SHIFT_TRANSPORT:
			case MAGICID_ESCAPE_QUICK:
				g_AutoPickMgr.MoveFailed(iX,iY);
				g_AutoPickMgr.BatFlyFinish(iX,iY);
				break;
			default:
				break;
			}

		}
		break;
	}

	SELF.InitAction(ACTION_STAND);
}

void CGameControl::MSG_Isvalid_Action(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);

	if(id == 0xFFFFFFFF)		// FAIL
	{
		//SELF.SetWaitServer(false,false);

		if(SELF.IsDead() == false && SELF.GetNextActionNum() == 0)
		{
			SNextAction &pNextAction = SELF.GetCNextAction();
			if(pNextAction.wAction==ACTION_ATTACK1 && g_AIMgr.HaveChopEquip())	// 挖矿时不需要停下来
				return;		
			else
			{
				pNextAction.ClearTarget();
			}

			SELF.SetOffset(0,0);
			SELF.SetDrawOffset(0,0);
			SELF.InitAction(ACTION_STAND);
			SELF.SetXY(x,y);
			SELF.SetRealXY(x,y);
		}
	}
	else							// GOOD
	{
		SELF.SetWaitServer(false,true);
	}
}

void CGameControl::MSG_Player_Position(const char * msg,int iLen)
{
	SELF.SetID(Conv_DWORD(msg));
	MapArray.Clear();

	int x = Conv_WORD(msg+6);
	int y = Conv_WORD(msg+8);

	SELF.SetXY(x,y);
	SELF.SetRealXY(x,y);
	SELF.SetOffset(0,0);
	SELF.SetDrawOffset(0,0);

	SELF.InitAction(ACTION_STAND);
	if(g_AIAutoMgr.IsEnalbeWaiGua())
		g_AutoPickMgr.ItemPickup(x,y);

	//载入地图
	std::string strMapName;
	strMapName.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
	if(strMapName.compare(g_pGameMap->GetMapName()) != 0)
	{
		g_pGameData->ChangeMapClear(strMapName.c_str());//切换了地图
		g_pMagicCtrl->FinishAllMagic(true);
		g_pGameMap->LoadMap(strMapName.c_str());//载入新的地图
		g_pGameControl->DoOverMapFindPath();
		g_pGameControl->SEND_Open_MapView();

		SELF.InitAction(ACTION_STAND);
		SELF.SetWaitServer(false,true);
	}
}

void CGameControl::MSG_Player_Refresh(const char * msg,int iLen)
{
	// 角色外观更新
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	SELF.SetXY(x,y);
	SELF.SetRealXY(x,y);
	SELF.SetDir(LOBYTE(lpPacketMsg->stDefMsg.wSeries));
	SELF.SetSex(HIBYTE(lpPacketMsg->stDefMsg.wSeries));

	//外观信息
	MESSAGEBODYWL stMsgBodyWl;
	memcpy(&stMsgBodyWl, lpPacketMsg->szEncodeData, sizeof(MESSAGEBODYWL));
	SELF.SetLooks(stMsgBodyWl.lParam1);
	SELF.SetStatus(stMsgBodyWl.lParam2);

	//组队
	g_pGameData->EnableTroop(stMsgBodyWl.nTag1 != 0);
	g_AutoPickMgr.ItemPickup(x,y);

	/*
	int x = Conv_WORD(msg + 6);
	int y = Conv_WORD(msg + 8);

	SELF.SetXY(x,y);
	SELF.SetRealXY(x,y);
	SELF.SetDir(msg[10]);
	SELF.SetLooks(Conv_INT64(msg+12));
	SELF.SetStatus(Conv_WORD(msg+22));

	g_pGameData->EnableTroop((msg[20] != 0));
	g_AutoPickMgr.ItemPickup(x,y);
	*/
}

void CGameControl::MSG_Player_Attribute(const char * msg,int iLen)
{
	SELF.FromBuffer(msg,iLen);
}

void CGameControl::MSG_Player_Info2(const char * msg,int iLen)
{
	SELF.SetMagicDef(msg[0]); // 魔法躲避
    SELF.SetFame(msg[1]); // 声望
	SELF.SetPrecision(msg[6]); // 命中
	SELF.SetSmartness(msg[7]); // 躲避
	SELF.SetPoisonDef(msg[8]); // 中毒恢复
	SELF.SetMagicRes(msg[10]); // 魔法恢复
	SELF.SetLifeRes(msg[11]);  // 生命恢复

	if(iLen > 12)
	{
		SELF.SetEnergy(Conv_WORD(msg+12)); // 活力值
		SELF.SetEnergyMax(Conv_WORD(msg+14)); // 最大活力值
		SELF.SetEnergyEnable(msg[16]); // 是否开启活力条
	}
	if(iLen >= 21)
	{
		SELF.SetBestItemValue(Conv_DWORD(msg + 17)); // 极品修炼值
	}
	if(iLen >= 23)
	{
		SELF.SetLuckyValue(*(BYTE*)(msg + 21)); // 幸运
		SELF.SetCurseValue(*(BYTE*)(msg + 22)); // 诅咒
	}
	if (iLen >= 25)
	{
		SELF.SetMagicRate(*(BYTE*)(msg + 23)); // 魔法命中
		SELF.SetPoisonRate(*(BYTE*)(msg + 24)); // 中毒命中
	}
	//[协议要修改]
	//if (iLen >= 26)
	//{
	//	SELF.SetRestoreSpiritPower(*(BYTE*)(msg + 25));
	//}
	if(iLen > 35)
	{
		// 五行 攻
		SELF.SetJinAttack((BYTE)msg[26]);
		SELF.SetMuAttack((BYTE)msg[27]);
		SELF.SetTuAttack((BYTE)msg[28]);
		SELF.SetShuiAttack((BYTE)msg[29]);
		SELF.SetHuoAttack((BYTE)msg[30]);
		// 五行 防御 百分比
		SELF.SetJinDef((BYTE)msg[31]);
		SELF.SetMuDef((BYTE)msg[32]);
		SELF.SetTuDef((BYTE)msg[33]);
		SELF.SetShuiDef((BYTE)msg[34]);
		SELF.SetHuoDef((BYTE)msg[35]);
	}

	if(iLen >= 49)
	{
		SELF.SetMeritoriousnessLevel((BYTE)msg[36]); // 当前功勋等级
		SELF.SetMeritoriousnessValue(Conv_DWORD(msg + 37)); // 功勋值
		SELF.SetMeritoriousnessAddSpeed(Conv_DWORD(msg + 41)); // 当前等级获取功勋值上限：XXX\小时
		SELF.SetMeritoriousnessExp(Conv_DWORD(msg + 45)); // 当前可兑换经验
	}

	if(iLen >= 59)
	{
		// 五行 攻
		SELF.SetJinAttack(Conv_WORD(msg + 49));
		SELF.SetMuAttack(Conv_WORD(msg + 51));
		SELF.SetTuAttack(Conv_WORD(msg + 53));
		SELF.SetShuiAttack(Conv_WORD(msg + 55));
		SELF.SetHuoAttack(Conv_WORD(msg + 57));
	}

	if (iLen >= 70)
	{
		SELF.SetCruelAttack(Conv_WORD(msg + 59)); // 暴击
		SELF.SetAbsordBlood(Conv_WORD(msg + 61)); // 吸血
		SELF.SetReboundHurt(Conv_WORD(msg + 63)); // 反弹伤害
		SELF.SetAbsDefend(Conv_WORD(msg + 65)); // 绝对防御
		SELF.SetDestroyDefend(Conv_WORD(msg + 67)); // 破防
		SELF.SetBloody((BYTE)msg[69]); // 嗜血 百分数
	}

	if (iLen >= 80)
	{
		// 五行 攻 下限
		SELF.SetJinAttack_Low(Conv_WORD(msg + 70));
		SELF.SetMuAttack_Low(Conv_WORD(msg + 72));
		SELF.SetTuAttack_Low(Conv_WORD(msg + 74));
		SELF.SetShuiAttack_Low(Conv_WORD(msg + 76));
		SELF.SetHuoAttack_Low(Conv_WORD(msg + 78));
	}

	//int iPos = 70;
	//bool bHaveHolyWing = false;
	//if (iLen > iPos && msg[iPos] > 0)
	//{
	//	bHaveHolyWing = true;
	//	iPos ++;
	//}

	//if (iLen >= iPos + 38)
	//{		
	//	SELF.SetHolyWingLevelUpNeedLevel(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingPartLevelUpExpMax(Conv_DWORD(msg + iPos));
	//	iPos += 4;
	//	SELF.SetHolyWingAddHp(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingAddMp(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingAttack(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingDefend(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingMagicDefend(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingCruelAttack(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingReboundHurt(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingAbsordBlood(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingMagicDefend(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	SELF.SetHolyWingAttackAvoid(Conv_WORD(msg + iPos));
	//	iPos += 2;
	//	for (int i = 0; i < MAX_HOLYWING_PARTS; i++)
	//	{
	//		SELF.SetHolyWingAttrType(i,BYTE(msg[iPos]));
	//		iPos += 1;
	//		SELF.SetHolyWingAttrValue(i,Conv_WORD(msg + iPos));
	//		iPos += 2;
	//	}
	//}


}

// 玩家身上装备
//ii, btCompress,  iii, SizeOf(TClientItem2));
// ii 个数,  btCompress 是否压缩, iii 总大小, 结构大小
void CGameControl::MSG_Player_Arm_Info(const char * msg,int iLen)
{
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;

	string buf;
	int iCount = lpPacketMsg->stDefMsg.nRecog;		//个数 ii
	int iSize = lpPacketMsg->stDefMsg.wTag;			//总大小
	WORD byGoodLen = lpPacketMsg->stDefMsg.wSeries;		//单个物品长度
	WORD wHasenc = lpPacketMsg->stDefMsg.wParam; // 是否hash加密数据

	//BYTE byGoodLen = BYTE(msg[7]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	//没有物品返回
	if(iCount == 0){
		output_debug("角色无穿戴装备\n");
		return;
	}

	//(msg[6] == 1)		//消息体进过压缩
	if(wHasenc == 1 && iCount > 0)
	{
		DWORD size = 128 * 1024;
		Uncompress((byte *)g_cBuf,&size,(byte *)(msg + CMD_SIZE),iLen - CMD_SIZE);
		buf.assign(g_cBuf,size);		

		//DWORD size = 128 * 1024;
		//Uncompress((byte *)g_cBuf,&size,(byte *)lpPacketMsg->szEncodeData,iSize);
		//buf.assign(g_cBuf,size);		
	}
	else
	{
		buf.assign(msg + CMD_SIZE,iLen - CMD_SIZE);
		//buf.assign(lpPacketMsg->szEncodeData,iSize);
	}	

	int i = 0;
	while(i < buf.size())
	{
		char j = *(buf.c_str() + i);

		CGood* pGood = NULL;
		if (j == 16)
		{
			pGood = SELF.EquipGood().GetPtr(ITEM_POS_WENPEI);
		}
		else if (j == 17)
		{
			pGood = SELF.EquipGood().GetPtr(ITEM_POS_WING);
		}
		else
		{
			pGood = SELF.EquipGood().GetPtr(j);
		}

		
		if(pGood == NULL)
		{
#ifdef _DEBUG
			output_debug("服务器装备数据有误，装备位置在 %d \n", j);
#endif
			i += (2 + byGoodLen);
			continue;
		}

		// 偏移两个字节因为有个ID
		pGood->FromBuffer(buf.c_str() + i + 2,false,byGoodLen);
		i+= (2 + byGoodLen);
	}

	CGood& shield = SELF.EquipGood().Get(ITEM_POS_SHIELD);
	if(shield.GetID() != 0 && shield.GetShape() > 0)
	{
		SELF.SetShield(shield.GetShape());
	}
	else
	{
		SELF.SetShield(0);	
	}
	
	CMagicData *pMagic = SELF.MagicArray().FindMagic(MAGICID_SWT + SELF.GetCareer());
	if(pMagic)
	{
		pMagic->SetDisable(!IsEquipXWSkillSuit());
	}
}

void CGameControl::MSG_Player_Info1(const char * msg,int iLen)
{
	SELF.SetPackageWeight(Conv_WORD(msg));
	SELF.SetWeight(BYTE(msg[6]));
	SELF.SetWrist(BYTE(msg[8]));
}

void CGameControl::MSG_Player_Money(const char * msg,int iLen)
{
	DWORD money = Conv_DWORD(msg);

	WORD wType = Conv_WORD(msg + 6);

	if (wType == 0)
	{
		DWORD origin = SELF.GetGold();
		if(origin < money)
		{
			string str = StringUtil::format("%d",money-origin);

			g_FindGood.AddMessage(str.c_str(),2);
			g_pAudio->Play(EAT_OTHER,13,g_pAudio->GetRand()++);
		} 

		SELF.SetGold(money);
	}
	else if (wType == 1)
	{
		DWORD origin = SELF.GetBindGold();
		if(origin < money)
		{
			string str = StringUtil::format("%d",money-origin);

			g_FindGood.AddMessage(str.c_str(),2);
			g_pAudio->Play(EAT_OTHER,13,g_pAudio->GetRand()++);
		} 
		SELF.SetBindGold(money);
	}
	else if (wType == 2)
	{
		SELF.SetBindYuanBao(money);
	}
}

void CGameControl::MSG_Player_Life(const char * msg,int iLen)
{
	DWORD id =  Conv_DWORD(msg);

	CCharacterAttr* pChar = g_pGameData->FindCharacterByID(id);
	if(pChar)
	{
		WORD wTemp = Conv_WORD(msg + 10);

		if(id == SELF.GetID())
		{
			int iHp = pChar->GetHP();
			if (iHp > Conv_WORD(msg + 6))
			{
				g_pGameControl->Send_Player_Prompt_Status(23,1);
			}

			int iMp = pChar->GetMP();
			if (iMp > Conv_WORD(msg + 8))
			{
				g_pGameControl->Send_Player_Prompt_Status(24,1);
			}

			pChar->SetHPMax(wTemp);
			//[协议要修改]
			//SELF.SetSpiritPower(Conv_WORD(msg + 12));
			//SELF.SetSpiritPowerMax(Conv_WORD(msg + 14));
// 			if (g_TrusteeshipData.IsTrusteeship())//微操或附身时的血量变化不要广播给其它人,本地客户端的数据的也会在该队员广播时设置
// 			{
// 				TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 				if ((g_pSelf == &ORIGINALSELF) || (pSelf && pSelf->byOffLineMode))
// 				{
// 					Send_Trusteeship_HPMP(SELF.GetSelfTrusteeshipPos(),pChar->GetHPMax(), pChar->GetMPMax(), pChar->GetHP(), pChar->GetMP());
// 					TneupMember * pMember = g_TrusteeshipData.GetSelf();
// 					if (pMember)
// 					{
// 						pMember->wMaxHP = pChar->GetHPMax();
// 						pMember->wHP = pChar->GetHP();
// 						pMember->wMaxMP = pChar->GetMPMax();
// 						pMember->wMP = pChar->GetMP();
// 					}
// 				}
// 			}
		}
		else
		{
			pChar->SetHPMax(wTemp);
		}

		pChar->SetHP(Conv_WORD(msg + 6));
		pChar->SetMP(Conv_WORD(msg + 8));

		if (iLen >= 21 && g_AICfgMgr.IsShowHpChangeEffect())
		{
			int iAddHp = Conv_INT(msg + 16);
			if (iAddHp != 0)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(MAGICID_POPUP_PAOPAO,0,id,0,-1,iAddHp > 0?0xFF00FF00:0xFFFF0000);
				if (ms)
				{
					ms->byRev1 = 2;
					if (iLen >= 21)
					{
						ms->iRev1 = msg[20];//1:暴击
					}
					ms->iData1 = iAddHp;
					int iTexW,iTexH;
					int iTexOffX = 0,iTexOffY = 0;
					pChar->GetTexWH(iTexW,iTexH);
					pChar->SetTexOffXY(iTexOffX,iTexOffY);
					ms->iOffX = iTexW / 2 + iTexOffX;
					ms->iOffY = iTexH + iTexOffY - 132;//怪物绘制非透明部分起始位置y - 132 + iTexOffY + iTexH = 怪物脚底y,所有 怪物绘制非透明部分起始位置y = 怪物脚底y - (iTexOffY + iTexH - 132)
				}
			}
		}
	}
}

void CGameControl::MSG_Player_Experience_Up(const char * msg,int iLen)
{
	DWORD dwExpUp = Conv_DWORD(msg + 6);//增加的经验
	UINT64 iExp = Conv_INT64(msg + 12);

	int iBefore = 0,iFinish = 0;

	if(SELF.GetLevelUpExp() > 0) //增加的经验
	{
		iBefore = (int)((double)(SELF.GetExp())*100.0f / (double)SELF.GetLevelUpExp());
		iFinish = (int)((double)iExp * 100.0f /(double)SELF.GetLevelUpExp());

		if(iFinish - iBefore > 0)
		{
			g_OtherData.SetExpUpTime(GetTickCount());
		}
	}

	SELF.SetExp(iExp); //增加以后的经验

	if(msg[10] == 0)
	{
		std::string strTmp = StringUtil::format("%u经验值增加",dwExpUp);
		g_TalkMgr.PushSysTalk(strTmp.c_str());
	}

	//如果经验有1%的增长，向服务器发送更新自身数据
	int iLevel = SELF.GetLevel();
	if(iLevel>=30 && iLevel <35)	//30-35涨10%
	{
		if(iFinish - iBefore > 0 && iFinish % 10 == 0 )
			g_pGameControl->Send_Range_Sign_Up(SELF.GetID(),1);

	}
	else if(iLevel>=35 && iLevel <40)	//35-40涨5%
	{
		if(iFinish - iBefore > 0 && iFinish % 5 == 0 )
			g_pGameControl->Send_Range_Sign_Up(SELF.GetID(),1);
	}
	else if(iLevel>=40)	//40-涨1%
	{
		if(iFinish - iBefore > 0)
			g_pGameControl->Send_Range_Sign_Up(SELF.GetID(),1);
	}

	return;
}

void CGameControl::MSG_Player_Rank_Up(const char * msg,int iLen)
{
	WORD level = Conv_WORD(msg+6);
	UINT64 exp = Conv_INT64(msg+12);
	DWORD uID = Conv_DWORD(msg+8);

	if(uID == 0 || uID == SELF.GetID())
	{
		if(level > SELF.GetLevel() || SELF.GetLevel() >= 71)
		{
			g_FindGood.AddMessage("升级了!",3);

			g_pMagicCtrl->CreateMagic(MAGICID_RANKUP_EFFECTIVE,0,uID);

			// 升级后更新可接任务列表
			g_PromptInfoMgr.UpdateAvaTask(level);
		}
		int iOldLevel = SELF.GetLevel();
		SELF.SetExp(exp);
		SELF.SetLevel(level);

		g_pGameMgr->OnRankUpLevel(iOldLevel,SELF.GetLevel());

		if (g_Login.GetLoginInExpType() == 1)
		{
			//如果原来20级一下，现在升级为20级
			if(SELF.GetLevel() >= 20 && iOldLevel < 20)
			{
				g_pControl->PopupWindow(MSG_CTRL_BINDPTWND_WND,OPER_CREATE);
			}
		}

		//如果原来22级一下，现在升级为22级
		if(SELF.GetLevel() >= MAX_LEVEL && iOldLevel < MAX_LEVEL)
		{
			g_pControl->PopupWindow(MSG_CTRL_CHARWND,OPER_RECREATE);
		}
		//将弹出帮助页面提高到玩家25级
		if(level <= 20 || level == 30 || level == 50)
		{			
			g_pControl->MsgToWnd(MSG_CTRL_HANDYMAP_WND,MSG_CTRL_HANDYMAP_WND,2);
		}

		if (iOldLevel < 42 && level == 42)
		{
			g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,7);//SanWeiFireWnd 按钮闪烁
		}

		g_pAudio->Play(EAT_OTHER,30,g_pAudio->GetRand()++);

		//如果级别增长，向服务器发送更新自身数据
		if(SELF.GetLevel() > iOldLevel)
		{
			g_pGameControl->Send_Range_Sign_Up(SELF.GetID(),1);
		}

		if(g_pSDOAInterface)
			g_pSDOAInterface->OnLevelUp(level);
	}
	else
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(uID);
		if(pChar)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_RANKUP_EFFECTIVE,0,uID);

			int iSelfX,iSelfY,iOtherX,iOtherY;
			SELF.GetXY(iSelfX,iSelfY);
			pChar->GetXY(iOtherX,iOtherY);
			g_pAudio->PlayEx(EAT_OTHER,30,g_pAudio->GetRand()++,iSelfX,iOtherX,iSelfY,iOtherY);
		}	
	}
}


void CGameControl::MSG_Package_All_Info(const char * msg, int iLen)
{
	string buf;
	BYTE byGoodLen = BYTE(msg[7]);//单个物品的长度	wParam
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	if(msg[6] == 1)		//消息体进过压缩
	{
		DWORD size = 128 * 1024;
		Uncompress((byte *)g_cBuf,&size,(byte *)(msg + CMD_SIZE),iLen - CMD_SIZE);
		buf.assign(g_cBuf,size);		
	}
	else
	{
		buf.assign(msg + CMD_SIZE,iLen - CMD_SIZE);
	}	

	int iSize = buf.size();
	SELF.PackageGood().Clear();

	for(int i = 0,j = 0; i < iSize; i += byGoodLen,j++)
	{
		SELF.PackageGood().Add(buf.c_str() + i,byGoodLen);
	}

	SELF.GetUsingTemp().Clear();
	CGoodGrid::ClearGoodFrom();
	SELF.GetEquipTemp().clear();
}


//　从包裹到装备栏
void CGameControl::SEND_Package_Object_Arm(CGood & m,int nPos)
{
	if(m.GetID() == 0) return;

	int i = 0;

	if (g_AIGoodMgr.IsFaBao(m))
	{
		SET_COMMAND(CS_FABAO_ZHUANGBEI,12);
		ASSIGN_DWORD(6,m.GetID());
		ASSIGN_BYTE(10,nPos == ITEM_POS_FABAO/*?0:nPos == ITEM_POS_FABAOLEFT?1:2*/);
		i = SEND_GAME_SERVER();
	}
	else
	{
		SET_COMMAND(CS_PACKAGE_OBJECT_ARM,255);
		ASSIGN_ID(m.GetID());
		ASSIGN_WORD(6,nPos);
		ADD_STR(m.GetName());
		i = SEND_GAME_SERVER();
	}


	if (i > 0)
	{
		SELF.SetEquipTempPos(nPos); //装备栏上的位置
		SELF.GetEquipTemp().Assign(m);
		m.SetID(0);
	}
}

//　从装备栏到包裹
void CGameControl::SEND_Package_Object_Unarm(CGood & m,int nPos)
{
	if(m.GetID() == 0)	return;

	int i = 0;
	//法宝
	if (g_AIGoodMgr.IsFaBao(m))
	{
		SET_COMMAND(CS_FABAO_ZHUANGBEI,12);
		//ASSIGN_ID(m.GetID());
		ASSIGN_DWORD(6,m.GetID());
		ASSIGN_BYTE(10,nPos == ITEM_POS_FABAO/*?0:nPos == ITEM_POS_FABAOLEFT?1:2*/);
		i = SEND_GAME_SERVER();
	}
	else
	{
		SET_COMMAND(CS_PACKAGE_OBJECT_UNARM,255);
		ASSIGN_ID(m.GetID());
		ASSIGN_WORD(6,nPos);
		ADD_STR(m.GetName());
		i = SEND_GAME_SERVER();
	}

	if (i > 0)
	{
		SELF.SetEquipTempPos(nPos);
		SELF.GetEquipTemp().Assign(m);
		m.SetID(0);
	}
}

//装备物品
void CGameControl::MSG_Player_Arm_Refresh(const char * msg,int iLen)
{
    SELF.SetLooks(Conv_INT64(msg+12));

    CGood& tmp = SELF.GetEquipTemp();
	g_BoothData.CleanItem(tmp.GetID()); //清除掉

    if(tmp.GetID() != 0)    //判断装备是否存在
    {
        int iPos = SELF.GetEquipTempPos();
		if(iPos >= 0)
		{
			SELF.GetEquipGood(iPos).Assign(tmp);
			SELF.GetEquipTemp().SetID(0);

			CMagicData *pMagic = SELF.MagicArray().FindMagic(MAGICID_SWT + SELF.GetCareer());
			if(pMagic)
			{
				pMagic->SetDisable(!IsEquipXWSkillSuit());
			}
		}
    }
	//else
	//{
	//	SELF.SetIBodyEx(16);
	//}

}

//装备失败
void CGameControl::MSG_Player_Arm_Fail(const char * msg,int iLen)
{
	CGood& tmp = SELF.GetEquipTemp();
	if(tmp.GetID() == 0) //没有装备物品
		return;

	int pos = tmp.GetPos();
	CGood* pGood = SELF.PackageGood().GetPtr(pos);
	if(pGood && pGood->GetID() == 0)
	{
		pGood->Assign(tmp);
	}
	else
	{
		pGood = SELF.PackageGood().FindGood(0); //找到空位
		if(pGood)
		{
			pGood->Assign(tmp);
		}
	}

	tmp.SetID(0);
}

void CGameControl::MSG_Player_Unarm_Refresh(const char * msg,int iLen)
{
	SELF.SetLooks(Conv_INT64(msg+12));
	SELF.GetEquipTemp().SetID(0);

	CMagicData *pMagic = SELF.MagicArray().FindMagic(MAGICID_SWT + SELF.GetCareer());
	if(pMagic)
	{
		pMagic->SetDisable(!IsEquipXWSkillSuit());
	}
}

void CGameControl::MSG_Player_Unarm_Fail(const char * msg,int iLen)
{
	int iEquipPos = SELF.GetEquipTempPos();
	if(iEquipPos < 0 || iEquipPos >= MAX_EQUIPMENT)
		return;

	SELF.GetEquipGood(iEquipPos) = SELF.GetEquipTemp();
	SELF.GetEquipTemp().SetID(0);
}

void CGameControl::MSG_Train_Success(const char * msg,int iLen)
{
	SNextAction &pAction = SELF.GetCNextAction();
	pAction.ClearTarget();

	SELF.InitAction(ACTION_STAND);
	SELF.SetReserveData(plyAttackLockID,0);
	g_PetData.GetMyHorse().bStatus = 1;
}


// 要发送的消息处理
bool CGameControl::SEND_Player_Walk(int x,int y)
{
	SET_COMMAND(CS_PLAYER_WALK,256);
	ASSIGN_WORD(0,x);
	ASSIGN_WORD(2,y);
	ASSIGN_BYTE(8,SELF.GetDir());

	//SELF.SetWaitServer(false);
	g_AIMgr.UpdateMoveTime();
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_PLAYER_WALK, MAKELONG(x, y), 0, SELF.GetDir(), 0);
	//g_pNet->SendPacket(&DefMsg, NULL);

	g_AutoPickMgr.ItemPickup(x,y);

	output_debug("SEND_Player_Walk:iArmX:%d,iArmY:%d,Time:%u\r\n",x,y,GetTickCount());

	return true;
}

bool CGameControl::SEND_Player_Run(int x,int y)
{
	SET_COMMAND(CS_PLAYER_RUN,256);
	ASSIGN_WORD(0,x);
	ASSIGN_WORD(2,y);
	ASSIGN_BYTE(8,SELF.GetDir());

	//SELF.SetWaitServer(false);
	g_AIMgr.UpdateMoveTime();
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_PLAYER_RUN, MAKELONG(x, y), 0, SELF.GetDir(), 0);
	//g_pNet->SendPacket(&DefMsg, NULL);

	g_AutoPickMgr.ItemPickup(x,y);

	output_debug("SEND_Player_Run:iArmX:%d,iArmY:%d,Time:%u\r\n",x,y,GetTickCount());

	return true;
}

void CGameControl::SEND_Player_Turn(char dir)
{
	SET_COMMAND(CS_PLAYER_TURN,CMD_SIZE);
	ASSIGN_WORD(0,SELF.GetX());
	ASSIGN_WORD(2,SELF.GetY());
	ASSIGN_BYTE(8,dir);
	SELF.SetReserveData(plyLastTurnTime,GetTickCount());
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_PLAYER_TURN, MAKELONG(SELF.GetX(), SELF.GetY()), 0, dir, 0);
	//g_pNet->SendPacket(&DefMsg, NULL);
}

void CGameControl::SEND_Player_Squat(char dir)
{
	SET_COMMAND(CS_PLAYER_SQUAT,CMD_SIZE);
	ASSIGN_WORD(0,SELF.GetX());
	ASSIGN_WORD(2,SELF.GetY());
	ASSIGN_BYTE(8,dir);
	SELF.SetWaitServer(false);
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_PLAYER_SQUAT, MAKELONG(SELF.GetX(), SELF.GetY()), 0, dir, 0);
	//g_pNet->SendPacket(&DefMsg, NULL);
}

void CGameControl::SEND_Player_Force_Attack(char dir)
{
	SET_COMMAND(CS_PLAYER_FORCE_ATTACK,CMD_SIZE);
	ASSIGN_WORD(0,SELF.GetX());
	ASSIGN_WORD(2,SELF.GetY());
	ASSIGN_BYTE(8,dir);

	//SELF.SetWaitServer(true);
    g_AIMgr.UpdateAttackTime();
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_PLAYER_FORCE_ATTACK, MAKELONG(SELF.GetX(), SELF.GetY()), 0, dir, 0);
	//g_pNet->SendPacket(&DefMsg, NULL);
}

void CGameControl::SEND_Player_Normal_Attack(char dir,UINT uID)
{
	SET_COMMAND(CS_PLAYER_NORMAL_ATTACK,256);
	if(uID == 0)
	{	
		ASSIGN_WORD(0,SELF.GetX());
		ASSIGN_WORD(2,SELF.GetY());
	}
	else
	{
		ASSIGN_ID(uID);
	}
	ASSIGN_BYTE(8,dir);

	//SELF.SetWaitServer(true);	
	SELF.SetReserveData(plyLastVerifyAction,ACTION_ATTACK1);
    g_AIMgr.UpdateAttackTime();

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Player_Attack_Sword(char dir,WORD wMagicID)
{
	WORD Cmd = 0;
	if(wMagicID == MAGICID_ATTACK_MOON)
		Cmd = CS_PLAYER_ATTACK_MOON;
	else if(wMagicID == MAGICID_ATTACK_FIRE)
		Cmd = CS_PLAYER_ATTACK_FIRE;
	else if(wMagicID == MAGICID_ATTACK_STICK)
		Cmd = CS_PLAYER_ATTACK_STICK;
	else if(wMagicID == MAGICID_ATTACK_KILL)
		Cmd = CS_PLAYER_ATTACK_KILL;
	else
		Cmd = CS_PLAYER_ATTACK_NEW_FENCE;

	if(wMagicID == MAGICID_ATTACK_FIRE || Cmd == MAGICID_THUNDER_FIRE)
		g_AutoFightMgr.SetAutoFire(0);

	SET_COMMAND(Cmd,256);

	ASSIGN_WORD(0,SELF.GetX());
	ASSIGN_WORD(2,SELF.GetY());
	ASSIGN_BYTE(8,dir);
	ASSIGN_DWORD(10,wMagicID);

	//SELF.SetWaitServer(true);    

    g_AIMgr.UpdateAttackTime();

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Player_Attack_Magic(DWORD aimID,WORD aimX,WORD aimY,WORD magID)
{
	SET_COMMAND(CS_PLAYER_ATTACK_MAGIC,CMD_SIZE);
	ASSIGN_WORD(0,aimX);
	ASSIGN_WORD(2,aimY);

	ASSIGN_WORD(6,LOWORD(aimID));
	ASSIGN_WORD(8,magID);
	ASSIGN_WORD(10,HIWORD(aimID));

	//SELF.SetWaitServer(true);

	g_AIMgr.UpdateMagicTime(magID);
	SEND_GAME_SERVER();

	if(g_AIMgr.IsFlyMagic(magID))
		g_AutoPickMgr.ItemPickup(aimX,aimY);
}

void CGameControl::SEND_Player_Use_Magic_Reel(DWORD aimID,WORD aimX,WORD aimY,WORD magID,DWORD dwBookID)
{
	SET_COMMAND(CS_PLAYER_USE_MAGIC_REEL,256);

	WORD wAimIDLow = LOWORD(aimID);
	WORD wAimIDHigh = HIWORD(aimID);

	ASSIGN_WORD(0,aimX);
	ASSIGN_WORD(2,aimY);
	ASSIGN_WORD(6,wAimIDLow);
	ASSIGN_WORD(8,magID);
	ASSIGN_WORD(10,wAimIDHigh);
	ADD_DWORD(dwBookID);

	//SELF.SetWaitServer(true);
	g_AIMgr.UpdateMagicTime(magID);

	SEND_GAME_SERVER();
}

// 骑马
void CGameControl::SEND_Player_Ride_Horse(int iType,DWORD dwID)
{
	if(SELF.IsDead())
		return;

	if(SELF.HasStatus(CS_LULL) || (SELF.GetExtraState() & ES_FROST))
	{
		g_TalkMgr.PushSysTalk("已经被麻痹或冰冻,无法动弹。");
		return;
	}

	SAction& pAction = SELF.GetAction();
	if( pAction.wAction!=ACTION_STAND)
		return;

	if(SELF.GetBoothState())
	{
		g_TalkMgr.PushSysTalk("摆摊时不能骑乘骑兽。");
		return;
	}
	//int flag = false;
	//if( !SELF.IsOnHorse() )
	//{
	//	int x,y,x1,y1,x2,y2;
	//	CSimpleCharacterNode * pChar;

	//	SELF.GetXY(x,y);
	//	x1 = x - 2;
	//	y1 = y - 2;
	//	x2 = x + 2;
	//	y2 = y + 2;
	//	if( x1 < 0 ) x1 = 0;
	//	if( y1 < 0 ) y1 = 0;
	//	if( x2 >= g_pGameMap->GetWidth()  ) x2 = g_pGameMap->GetWidth()  - 1;
	//	if( y2 >= g_pGameMap->GetHeight() ) y2 = g_pGameMap->GetHeight() - 1;

	//	for(x=x1;x<=x2;x++)
	//	{
	//		for(y=y1;y<=y2;y++)
	//		{
	//			pChar = MapArray.GetSimpleCharacterHead(x,y);
	//			while(pChar)
	//			{
	//				if( pChar->GetRaceType()==CHARACTER_MONSTER && !pChar->IsDead() && pChar->IsMyPet())
	//				{

	//					DWORD iMonster = pChar->GetRaceNo();
	//					//if(iMonster == 166 || iMonster == 167 || iMonster == 168 || iMonster == 185 || iMonster == 186 || iMonster == 187 || iMonster == 4 || iMonster == 5 || iMonster == 6 )
	//					if(IsCanRidePet(iMonster))
	//						//////////////////////////////////
	//					{
	//						__time32_t tMyPetTime  = SELF.GetMyPetTime();//豹子饿的时间的前一天
	//						//计算饱或饿的状态
	//						__time32_t t_time;
	//						_time32(&t_time);
	//						t_time += g_dwServerTime;
	//						int iDays = (int)( (t_time - tMyPetTime) / (24*3600));
	//						//大于一天说明豹子饿了
	//						if(iDays>=2 && iDays<=10 )
	//						{
	//							g_TalkMgr.PushSysTalk("你的灵兽处饱食状态才能骑乘。");
	//							return;
	//						}
	//					}

	//					flag = true;
	//					break;
	//				}
	//				pChar = pChar->m_MapNext;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	flag = true;
	//}


	//if( !flag ) 
	//{
	//	g_TalkMgr.PushSysTalk("你可骑乘的马匹不在身边");
	//	return;
	//}

	SET_COMMAND(CS_RIDE_HORSE,12);
	temp[7] = iType;
	*((DWORD*)(temp + 8)) = dwID;
	SEND_GAME_SERVER();
}

// 驯马
void CGameControl::SEND_Player_Train_Horse(int x,int y)
{
	SET_COMMAND(CS_TRAIN_HORSE,12);
	ASSIGN_WORD(0,x);
	ASSIGN_WORD(2,y);
	ASSIGN_BYTE(8,SELF.GetDir());

	//SELF.SetWaitServer(true);
    g_AIMgr.UpdateAttackTime();

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Objects_Position()
{
	if(SELF.GetID() == 0)
		return;

	SET_COMMAND(CS_OBJECTS_POSITION,512);
	CGoodGrid::ReleaseDrop();

	BYTE num = 0;
	int iSize = SELF.PackageGood().Size();
	for(int i = 0; i < iSize; i++)
	{
		DWORD id = SELF.GetPackageGood(i).GetID();
		if(id == 0)
			continue;

		ADD_DWORD(id);
		ADD_WORD(i);
		num++;
	}

	iSize = SELF.GetUsingTemp().Size();
	for(int i = 0; i < iSize; i++)
	{
		DWORD id = SELF.GetUsingTemp().Get(i).GetID();
		if(id == 0)
			continue;

		ADD_DWORD(id);
		ADD_WORD(SELF.GetUsingTemp().Get(i).GetPos());
		num++;
	}

	temp[10] = (char)num;
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Objects_Position(const char * msg,int iLen)
{
	int iCount = msg[10];
	int num = (BYTE)((iLen - 12) / 6);
	iCount = min(iCount,num);
	if(iCount == 0)
		return;

	if(iCount > MAX_PACKAGE_ELEMENT)
		iCount = MAX_PACKAGE_ELEMENT;

	GoodPos_t* PosArray = new GoodPos_t[iCount];

	for(int iNum = 0; iNum < iCount; iNum++)						// 接受服务器端的位置信息		
	{
		PosArray[iNum].dwID = Conv_DWORD(msg+12+iNum*6);
		PosArray[iNum].iPos = Conv_WORD(msg+16+iNum*6);
		PosArray[iNum].bDeal = false;
	}

	SELF.ApplyPackageGoodPos(PosArray,iCount);

	SAFE_DELETE_ARRAY(PosArray);
}

void CGameControl::SEND_Player_StopGo()
{
	SET_COMMAND(CS_PLAYER_STOPGO,12);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Monster_StopGo(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	CSimpleCharacter * pChar = g_pGameData->FindSimpleCharacter(id);
	if(pChar)
	{
		pChar->SetStopGo(true);
	}
}

void CGameControl::MSG_Monster_MovePosition(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	BYTE dir = msg[10];
	__int64 looks = Conv_INT64(msg + 12);

	WORD wAction = 0;
	SAction* pAction = NULL;
    BYTE byRideStatus = 0;
    if(iLen > 20)
        memcpy(&byRideStatus,msg+20,1);

	int iType = APT_NORMAL;

	if (id == SELF.GetID())
	{
		SELF.SetXY(x,y);
		SELF.SetRealXY(x,y);
		SELF.SetDir(dir);
		SELF.SetLooks(looks);

        if(byRideStatus == 1)	//骑战
        {
           // g_TalkMgr.PushSysTalk("你骑乘的灵兽已进入了骑战状态，可发动骑战技能");
            SELF.SetFightOnLeopard(true);
            SELF.SetRunStepOnLeopard(2);
			iType = APT_RIDE_ATTACK_LEAPOARD;
        }
        else if(byRideStatus == 2)	//骑战加跑三格的豹子
        {
            SELF.SetFightOnLeopard(true);
            SELF.SetRunStepOnLeopard(3);
			iType = APT_RIDE_ATTACK_LEAPOARD;
        }
        else		//不是骑战
        {
            SELF.SetFightOnLeopard(false);
            SELF.SetRunStepOnLeopard(3);
        }

		wAction = SELF.GetAction().wAction;
		pAction	= &(SELF.GetAction());
		g_AIMgr.MasterMoved();

		if(iType == APT_NORMAL)
		{
			if(SELF.IsOnLepoard())
			{
				iType = APT_RIDE_LEPOARD;
			}
			else if(SELF.IsOnHorse())
			{
				iType = APT_RIDE_HORSE;
			}
			
		}
	}
	else
	{
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar) return;

        if(byRideStatus == 1)	//骑战
        {
            pChar->SetFightOnLeopard(true);
            pChar->SetRunStepOnLeopard(2);
			iType = APT_RIDE_ATTACK_LEAPOARD;
        }
        else if(byRideStatus == 2)//////////骑战加跑三格的豹子
        {
            pChar->SetFightOnLeopard(true);
            pChar->SetRunStepOnLeopard(3);
			iType = APT_RIDE_ATTACK_LEAPOARD;
        }
        else		//不是骑战
        {
            pChar->SetFightOnLeopard(false);
            pChar->SetRunStepOnLeopard(3);
        }

        pChar->SetXY(x,y);
		pChar->SetRealXY(x,y);
		pChar->SetDir(dir);
		pChar->SetLooks(looks);
		g_pGameData->AddSimpleCharacter(x,y,id);

		wAction = pChar->GetAction().wAction;
		pAction	= &(pChar->GetAction());

		if(iType == APT_NORMAL)
		{
			if(pChar->IsOnLepoard())
			{
				iType = APT_RIDE_LEPOARD;
			}
			else if(pChar->IsOnHorse())
			{
				iType = APT_RIDE_HORSE;
			}			
		}
	}

	if(wAction == ACTION_STAND && pAction)
	{
		pAction->wDrawAction = g_ActionInf.ConvertAction(wAction,iType);//进行转换
	}
}

void CGameControl::MSG_Player_RideEnd(const char * msg,int iLen)
{
	//SELF.SetMagicBody(0);
}

void CGameControl::MSG_Player_Salute(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	BYTE  dir = msg[10];

	CSimpleCharacter* pChar = g_pGameData->FindSimpleCharacter(id);
	if(pChar)
	{
		if(pChar->IsOnLepoard()) //6-3 豹子吼叫其他玩家转方向
			pChar->SetDir(dir);

		int iAction = ACTION_SALUTE;
		if (pChar->GetFightOnLeopard())
		{
			iAction = ACTION_LSALUTE;
		}
		pChar->PushSNextAction(iAction,dir);
	}
}

void CGameControl::SEND_Player_Salute()
{
	SET_COMMAND(CS_PLAYER_SALUTE,12);
	ASSIGN_BYTE(8,SELF.GetDir());
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Player_Auto_Arm(const char * msg,int iLen)
{
	int pos = msg[6];
	DWORD id = Conv_DWORD(msg);

	if(pos > MAX_EQUIPMENT)
		return;

	CGood* pGood = SELF.PackageGood().FindGood(id);
	if(pGood)
	{
		SELF.GetEquipGood(pos).Assign(*pGood);
		pGood->SetID(0);
	}
	g_BoothData.CleanItem(id);
}

void CGameControl::MSG_Open_Remain_Shade(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_REMAINSHADE);
}

void CGameControl::MSG_Close_Remain_Shade(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_REMAINSHADE);
}

void CGameControl::MSG_Open_Blood_Shade(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_BLOODSHADE);
}

void CGameControl::MSG_Close_Blood_Shade(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_BLOODSHADE);
}

void CGameControl::MSG_Open_Whole_Moon(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_WHOOLEMOON);
}

void CGameControl::MSG_Close_Whole_Noon(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_WHOOLEMOON);
}

void CGameControl::MSG_Open_Thunder_Fire(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_THUNDERFIRE);
	g_AutoFightMgr.SetAutoFire(1);
}

void CGameControl::MSG_Close_Thunder_Fire(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_THUNDERFIRE);
	g_AutoFightMgr.SetAutoFire(0);
}

void CGameControl::MSG_Open_Destroy_Weapon(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_DESTROYWEAPON);
}

void CGameControl::MSG_Close_Destroy_Weapon(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_DESTROYWEAPON);
}

void CGameControl::MSG_Open_Destroy_Shell(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_DESTROYSHELL);
}

void CGameControl::MSG_Close_Destroy_Shell(const char *strMsg, int iLen)
{
	SELF.RemoveMagicState(MS_DESTROYSHELL);	
}  

void CGameControl::MSG_Open_Destroy_Shield(const char * strMsg,int iLen)
{
	SELF.AddMagicState(MS_DESTROYSHIELD);
}

void CGameControl::MSG_Close_Destroy_Shield(const char * strMsg,int iLen)
{
	SELF.RemoveMagicState(MS_DESTROYSHIELD);
}

void CGameControl::MSG_Player_Sword_Switch(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg + 6);
	BYTE bClose = msg[8];

	DWORD iMState = 0;

	switch(wMagicID)
	{
	case MAGICID_ATTACK_STICK:					// 刺杀剑术
		iMState = MS_ATTACKSTICK;
		break;
	case MAGICID_ATTACK_MOON:					// 半月弯刀
		iMState = MS_ATTACKMOON;
		break;
	case MAGICID_WHOLE_MOON:					// 园月弯刀
		iMState = MS_WHOOLEMOON;
		break;
	case MAGICID_ATTACK_FIRE:					// 烈火剑法
		iMState = MS_ATTACKFIRE;
		break;
    case MAGICID_THUNDER_FIRE:					// 雷烈剑法
		iMState = MS_THUNDERFIRE;
		break;
	case MAGICID_DESTROY_WEAPON:				// 击破
		iMState = MS_DESTROYWEAPON;
		break;
	}    

	if(bClose)
		SELF.RemoveMagicState(iMState);
	else
		SELF.AddMagicState(iMState);
}

void CGameControl::MSG_Player_Use_Sword(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg+6);

	int iMState = 0;
	switch(wMagicID)
	{
	case MAGICID_ATTACK_KILL:					// 攻杀剑术
		iMState = MS_ATTACKKILL;
		break;
	case MAGICID_REMAIN_SHADE:					// 残影刀法
		iMState = MS_REMAINSHADE;
		break;
	case MAGICID_BLOOD_SHADE:					// 血影刀法
		iMState = MS_BLOODSHADE;
		break;
	}
	SELF.AddMagicState(iMState);
}

void CGameControl::MSG_Player_Dig_Break(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_VALIDDIG); 
}

void CGameControl::SEND_Player_Attack_Position(char cPosition)
{
	SET_COMMAND(CS_PLAYER_ATTACK_POSTION,256);
	ASSIGN_BYTE(6,cPosition);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Player_Pet_State()
{
	SET_COMMAND(CS_PLAYER_PET_STATE,256);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Player_Pet_State(const char * msg,int iLen)
{
	g_PetData.GetPetStatus().clear();

	PET_STATUS temp{};
	char *p = (char*)(msg + 12);
	int iPets = (iLen - 12)/24;
	if(iPets > 0)
	{
		for(int i = 0; i < iPets; i++)
		{
			temp.cPetLevel =p[i * 24];
			memcpy(&temp.cPetName	,p + i * 24 + 1,14);
			temp.wHp = Conv_WORD(p + i * 24 + 16);
			temp.wHpMax	= Conv_WORD(p + i * 24 + 18);
			temp.cDc = p[i * 24 + 20];
			temp.cDcMax	= p[i * 24 + 21];
			temp.cAc =	p[i * 24 + 22];
			temp.cAcMax	= p[i * 24 + 23];
			temp.cPetName[14] = 0;
			g_PetData.GetPetStatus().push_back(temp);
		}
	}

}

void  CGameControl::MSG_Play_Attacked_Sound(const char * msg, int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD  x  = Conv_WORD(msg+6);
	WORD  y  = Conv_WORD(msg+8);
	WORD type = Conv_WORD(msg+10);

	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	int iSoundID = 0;

	if(type == 1)
		iSoundID = MAGICID_DESTROY_SHELL*4+18;
	else if(type == 2)
		iSoundID = MAGICID_DESTROY_SHIELD *4+18;

	g_pAudio->PlayEx(EAT_MAGIC,iSoundID,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y,true);
}

void CGameControl::MSG_Player_Magic_Color(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg + 6);
	CMagicData *pMagicData = SELF.FindMagic(wMagicID);
	if(pMagicData)
	{
		pMagicData->SetColor(Conv_DWORD(msg));
	}
}

void CGameControl::MSG_Player_Task_Info(const char* msg,int iLen)
{
	//新任务，做出改动，传世前传
	if (iLen > 32768) return;//太长了，不接受
	
	DWORD  dwThemeID = 0;
	WORD  wTaskID = (WORD)(*((DWORD*)(msg)));
	BYTE  byOperType = *((BYTE*)(msg + 10));//1是增加或修改，2是删除,0是概要信息
	BOOL  bMainTask = false;

	if (*(msg + 6) == 1 || *(msg + 6) == 2)
	{
		DWORD dwParam = MAKELONG(*(msg + 6),3);
		g_pControl->PopupWindow(MSG_CTRL_TASK_WND,OPER_RECREATE,dwParam);

		return;
	}

	if (byOperType == 2)
	{//删除任务	
		bMainTask = true;
		if(!g_TaskData.DeleteTask(wTaskID,true))
		{
			bMainTask = false;
			g_TaskData.DeleteTask(wTaskID,false);			
		}
		
		CControl* pControl = (CControl*)(bMainTask?2:3);		

		g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(wTaskID,1));		
		g_pControl->MsgToWnd(MSG_CTRL_TASK_WND,MSG_CTRL_TASK_WND,wTaskID,pControl);//更新任务面版数据


		if (wTaskID >= 7000 + 6101 && wTaskID <= 7000 + 6499)//纹配任务,获得任务中的当前完全数量以及总共需要完成的数量,用来显示完成百分比,并改变按钮状态
		{
			WORD wLooks = wTaskID - 7000;

			sWenPeiTask *pWenPeiTask = g_XmlCfg.GetWenPeiTask(wLooks);
			if (pWenPeiTask)
			{
				pWenPeiTask->clear();
				g_pControl->MsgToWnd(MSG_CTRL_WENPEI_WND,MSG_CTRL_WENPEI_WND,9);
			}
		}

		return;
	}

	//增加或修改
	bool bRet;

	std::string str,strSub,strResult;
	str.assign(msg + 12,iLen - 12);	

	if (byOperType == 0)
	{
		//最大任务数		
		strSub = "MaxTaskCount=";
		bRet = g_TaskData.GetMarkedString(str,strSub,' ',strResult);
		if (bRet) g_TaskData.SetMaxTaskNum(atoi(strResult.c_str()));
		
		//已经完成任务数
		strSub = "TotalTaskCount=";
		bRet = g_TaskData.GetMarkedString(str,strSub,' ',strResult);
		if (bRet) g_TaskData.SetFinishNum(atoi(strResult.c_str()));


		//跑环任务数
		strSub = "LoopNum=";
		bRet = g_TaskData.GetMarkedString(str,strSub,' ',strResult);
		if (bRet) g_TaskData.SetLoopNum(atoi(strResult.c_str()));

		//积分
		strSub = "Achievement=";
		bRet = g_TaskData.GetMarkedString(str,strSub,' ',strResult);
		if (bRet) g_TaskData.SetAchievement(atoi(strResult.c_str()));
	}
	else if (byOperType == 1)
	{
		bool bNewTask = false;
		strSub = "#MBI";			
		bRet = g_TaskData.GetMarkedString(str,strSub,'#',strResult);
		if (bRet)
		{
			DWORD dwMain;
			sscanf(strResult.c_str(),"%d,%d",&dwThemeID,&dwMain);
			if (dwMain == 0) bMainTask = TRUE;
		}

		_Task task;
		task.bMain = bMainTask;
		_Task* pTask = g_TaskData.FindTask((WORD)dwThemeID,wTaskID,bMainTask);

		bool bFirstTask = false;
		if(!pTask) 
		{
			bNewTask = true;
			pTask = g_TaskData.AppendTask((WORD)dwThemeID,wTaskID,task,bMainTask);

			////第一次接任务的泡泡	
			if(!g_pGameData->HasPaoPaoStatus(EP_FirstGetNewTask_PaoPao))
			{
				bFirstTask = true;
			}



// 			//如果是第1次发言/屏蔽发言任务,全程泡泡提示
 			if (wTaskID == 10002 && !g_pGameData->HasPaoPaoStatus(EP_First_FaYan))
 			{
				g_pControl->PopupArrowTip(MSG_CTRL_TALKVIEW_WND,EP_First_FaYan_Step1,10,149,XAL_TOPLEFT,false,XAL_BOTTOMRIGHT);
 			}
 			else if (wTaskID == 10003 && !g_pGameData->HasPaoPaoStatus(EP_First_Team) && !g_pGameData->HasPaoPaoStatus(EP_First_Team_TeamWnd))
 			{
 				if (!g_pControl->FindWindowByName("TeamWnd"))
 				{
					if (g_EutUiType == EUT_CLASSIC)
					{
						g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_Team_Step1,557,78,XAL_TOPLEFT,false,XAL_BOTTOMLEFT);
					} 
					else
					{
						g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_Team_Step1,604,74,XAL_TOPLEFT,false,XAL_BOTTOMLEFT);
					}
					
 					//组队按钮闪烁
 					g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,4);
 				}
 				else
 				{
 					g_pControl->PopupArrowTip(MSG_CTRL_TEAMWND,EP_First_Team_TeamWnd,32,260,XAL_TOPLEFT,false,XAL_TOPLEFT);
 				}
 			}

		}

		if (!pTask)
			return;

		_ThemeTask* pTheme = g_TaskData.FindThemeTask((WORD)dwThemeID,bMainTask);
		if (!pTheme) return;

		strSub = "#BTL";			
		g_TaskData.GetMarkedString(str,strSub,'#',pTheme->strTaskTile);

		strSub = "#STL";
		g_TaskData.GetMarkedString(str,strSub,'#',pTask->strTaskName);

		strSub = "#PIC";
		bRet = g_TaskData.GetMarkedString(str,strSub,'#',strResult);
		if (bRet) sscanf(strResult.c_str(),"%d",&pTask->dwTaskPic);

		strSub = "#TDN";
		g_TaskData.GetMarkedString(str,strSub,'#',pTask->strTaskAim);

		strSub = "#DSP";
		g_TaskData.GetMarkedString(str,strSub,'#',pTask->strTaskDesc);

		if (wTaskID >= 7000 + 6101 && wTaskID <= 7000 + 6499)//纹配任务,获得任务中的当前完全数量以及总共需要完成的数量,用来显示完成百分比,并改变按钮状态
		{
			WORD wLooks = wTaskID - 7000;

			sWenPeiTask *pWenPeiTask = g_XmlCfg.GetWenPeiTask(wLooks);
			if (pWenPeiTask)
			{
				int iFinishNum = 0,iMaxNum = 1;

				int iPos = pTask->strTaskDesc.find('[',0);
				if (iPos != string::npos)
				{
					int iEndPos = pTask->strTaskDesc.find(']',iPos + 1);
					if(iEndPos != string::npos) 
					{
						iMaxNum = StringUtil::toInt(pTask->strTaskDesc.substr(iPos + 1,iEndPos - (iPos + 1)));
						pTask->strTaskDesc.erase(iPos,iEndPos - iPos + 1);
					}
				}

				iPos = pTask->strTaskDesc.find('[',0);
				if (iPos != string::npos)
				{
					int iEndPos = pTask->strTaskDesc.find(']',iPos + 1);
					if(iEndPos != string::npos) 
					{
						int iNum = StringUtil::toInt(pTask->strTaskDesc.substr(iPos + 1,iEndPos - (iPos + 1)));
						iFinishNum = iMaxNum - iNum;
						pTask->strTaskDesc.erase(iPos,iEndPos - iPos + 1);
					}
				}

				if (iMaxNum <= 0)
				{
					iMaxNum = 1;
				}
				if (iFinishNum > iMaxNum)
				{
					iFinishNum = iMaxNum;
				}

				pWenPeiTask->iMaxNum = iMaxNum;
				pWenPeiTask->iFinishNum = iFinishNum;
				
				if (iFinishNum >= iMaxNum)
				{
					pWenPeiTask->taskstate = 2;

					if(g_pControl->GetGameState() == MSG_CTRL_GAMEWND)
					{
						g_pMagicCtrl->CreateMagic(MAGICID_WENPEITASK,0,SELF.GetID(),0);
					}
				}
				else
				{
					pWenPeiTask->taskstate = 1;
				}

				g_pControl->MsgToWnd(MSG_CTRL_WENPEI_WND,MSG_CTRL_WENPEI_WND,9);
			}
		}


		if(g_pControl->GetGameState() != MSG_CTRL_GAMEWND)
			return;

		if (!bNewTask)
		{
			//不是新任务，需要更新
			g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(wTaskID,4));
		}
		else
		{
			//新接到任务,任务按钮闪烁
			g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,3);
			//现在艾婕要求不弹窗口了
			if (g_pControl->FindWindowByName("TaskWnd"))
			{
				g_pControl->PopupWindow(MSG_CTRL_TASK_WND,OPER_RECREATE,MAKELONG(wTaskID,0));
			}

			//自动追踪
			if (g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(6,6)))
			{
				pTask->bTrack = true;
				g_pControl->MsgToWnd("TaskWnd",MSG_CTRL_TASK_WND,0xFFFFFFFD,(CControl*)(wTaskID));

				//游戏窗口增加踪踪信息
				g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(wTaskID,0));
			}
		}

		if (bFirstTask)
		{
			int iSelfX = 0,iSelfY = 0;
			SELF.GetScrXY(iSelfX,iSelfY);
			g_pControl->PopupArrowTip(MSG_CTRL_GAMEWND,EP_FirstInGmae,iSelfX + 84,iSelfY - 52,XAL_TOPLEFT,false,XAL_BOTTOMLEFT);
		
			//在任务按钮上不冒泡了
			g_pGameData->AddPaoPaoStatus_Closed(EP_FirstGetNewTask_PaoPao);
			g_pGameControl->Send_Player_Prompt_Status(EP_FirstGetNewTask_PaoPao);	
		
			//if (g_EutUiType == EUT_CLASSIC)
			//{
			//	g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_FirstGetNewTask_PaoPao,166+186,35+28,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
			//} 
			//else
			//{
			//	g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_FirstGetNewTask_PaoPao,166+152,35+28,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
			//}			
		}
	}

}

void CGameControl::SEND_Player_Abandon_Task(WORD wTaskID)
{
	SET_COMMAND(CS_PLAYER_ABANDON_TASK,12);
	ASSIGN_ID(wTaskID);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Item_Effect_Switch(const char* msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD  wItemType = Conv_WORD(msg + 10);
	WORD  iX = Conv_WORD(msg + 6);
	WORD  iY = Conv_WORD(msg + 8);

	DWORD dwColor = 0xFFFFFFFF;
	if(iLen > 12)
	{
		dwColor = Conv_DWORD(msg + 12);
	}

	g_EffectMsgFilter.DoFilter(id,wItemType,iX,iY,dwColor);
}

void CGameControl::MSG_Self_Character_Action(const char* msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	if(msg[10] == 1)      //使玩家复活
	{
		if(id == SELF.GetID())  //主角
		{
			if(!SELF.IsDead()) //还活着呢
				return;

			SELF.SetDead(false);
			SELF.SetOffset(0,0);
			SELF.SetDrawOffset(0,0);
			SELF.InitAction(ACTION_STAND);

			g_pControl->PopupWindow(MSG_CTRL_RELIVEWITHYUANBAO_WND,OPER_CLOSE);
			g_pControl->PopupWindow(MSG_CTRL_QUIT_WND,OPER_CLOSE);   
			g_pControl->PopupWindow(MSG_CTRL_RELIVE_WND,OPER_CLOSE);
		}
		else
		{
			CSimpleCharacter* pChar = g_pGameData->FindSimpleCharacter(id);
			if(!pChar || !pChar->IsDead()) //还活着
				return;

			pChar->SetDead(false);
			pChar->SetOffset(0,0);
			pChar->SetDrawOffset(0,0);

			pChar->InitAction(ACTION_STAND);
		}
	}
}


//1.907新增协议
void CGameControl::MSG_Object_Extern_Info(const char * msg,int iLen)
{
	DWORD dwPlayerId = *((DWORD*)msg);
	WORD wNum = Conv_WORD(msg+10);

	int  j = 12;

	for(int i = 0; i < wNum && j+4 < iLen; i++)
	{
		DWORD dwGoodID = Conv_DWORD(msg+j);
		string strText = msg+j + 4;
		j += 5 + strText.size();

		CGood* pGood = NULL;

		if(dwPlayerId == SELF.GetID()) //玩家自己
		{
			g_pGameData->InsertExternString(dwGoodID,strText);
			int iFromWnd = NO_Wnd;
			pGood = g_pGameData->SearchGoodByID(dwGoodID,iFromWnd);
		}
		else
			pGood = g_pGameData->SearchOtherGoodByID(dwGoodID);

		if(pGood)
		{
			pGood->SetExternString(strText);
		}
	}
}

//第7位BYTE是物品类型　1是新手怪物召唤卡
void CGameControl::MSG_Set_Use_Object_Para(const char * msg,int iLen)
{
	DWORD dwObjID =	Conv_DWORD(msg);
	BYTE byType = msg[6];

	if(byType == 1)//新手怪物召唤卡
	{
		g_pControl->PopupWindow(MSG_CTRL_USE_OBJECT_WND,OPER_CREATE,5);
		g_pControl->PopupWindow(MSG_CTRL_USE_OBJECT_WND,OPER_CREATE,dwObjID);	

		char strTemp[1024]={0};
		memcpy(strTemp,msg + 12,iLen - 12);
		g_pControl->Msg(MSG_CTRL_USE_OBJECT_WND,1,(CControl*)(&strTemp));
			
	}
}

// 拜师与招徒相关请求
//对方ID	10	03	X	Y	Dir
//0x0310,x[0]=1师傅请求收徒弟 
//x[0]=2徒弟请求拜师
//x[0]=3师傅应答GS传过来的玩家拜师的请求。X[1]=0 同意，x[1]=1拒绝
//x[0]=4徒弟应答GS传过来的玩家收徒的请求。X[1]=0 同意，x[1]=1拒绝
void  CGameControl::SEND_Master_Prentice_Request(DWORD dwId,BYTE byType,BYTE byData)
{
	if(byType == 1 && GetTickCount() - g_AIMgr.GetReserveTime(plyLastTakeinPrenticeTime) < 5000)
	{
		g_TalkMgr.PushSysTalk("每5秒才能请求一次收徒");
		return;
	}
	else if(byType == 2 && GetTickCount() -g_AIMgr.GetReserveTime(plyLastTakeinMasterTime) < 5000)
	{
		g_TalkMgr.PushSysTalk("每5秒才能请求一次拜师");
		return;
	}



	SET_COMMAND(CS_MASTER_PRENTICE_REQUIRE,12);
	ASSIGN_DWORD(0,dwId);
	ASSIGN_BYTE(6,byType);
	if(byType == 3 || byType == 4)
	{
		ASSIGN_BYTE(7,byData);
	}
	int i = SEND_GAME_SERVER();


	if (i > 0)
	{
		if(byType == 1)
		{
			g_TalkMgr.PushSysTalk("已经发出收徒请求，请耐心等待回应");
			g_AIMgr.SetReserveTime(plyLastTakeinPrenticeTime,GetTickCount());
		}
		else if(byType == 2)
		{
			g_TalkMgr.PushSysTalk("已经发出拜师请求，请耐心等待回应");
			g_AIMgr.SetReserveTime(plyLastTakeinMasterTime,GetTickCount());
		}
	}
}

//拜师与招徒相关信息
//对方ID	10	03	X	Y	Dir	提示内容
//0x0310,
//x[0]=1系统应答师傅请求收徒弟，x[1]成功与否标记：0成功，其他失败，y应答提示内容长度，
//x[0]=2系统应答徒弟请求拜师，x[1]成功与否标记：0成功，其他失败，y应答提示内容长度
//x[0]=3 告诉师傅有玩家拜师的请求。
//x[0]=4 告诉徒弟有玩家收徒的请求。
void  CGameControl::MSG_Master_Prentice(const char* msg,int iLen)
{
	DWORD dwID = *((DWORD*)msg);
	BYTE byType = *((BYTE*)(msg + 6));
	BYTE byResult = *((BYTE*)(msg + 7));

	if(byType == 3)//请求拜师
	{

		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(dwID);
		if(p)
		{
			S_CommonConfirm CommonConfirm;

			CommonConfirm.iType = 1;
			CommonConfirm.dwLastTime = 30000;
			CommonConfirm.dwRequestPersonID = dwID;
			CommonConfirm.strMsg = CommonConfirm.strMsg + p->GetName() + "想拜你为师。";
			g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);

			g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
		}

	}
	else if(byType == 4)//请求收徒
	{
		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(dwID);
		if(p)
		{	
			S_CommonConfirm CommonConfirm;

			CommonConfirm.iType = 2;
			CommonConfirm.dwLastTime = 30000;
			CommonConfirm.dwRequestPersonID = dwID;
			CommonConfirm.strMsg = CommonConfirm.strMsg + p->GetName() + "想招收你为徒弟。";
			g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);

			g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
		}	
	}
	else if(byType == 1)//回复拜师请求
	{
		//if(byResult == 0)//成功
		//{
		//}
		//else//失败
		//{
		//}
		BYTE byMsgLen = *((BYTE*)(msg + 8));
		char strTemp[128]={0};
		memcpy(strTemp,msg+12,byMsgLen);
		g_TalkMgr.PushSysTalk(strTemp);
	}
	else if(byType == 2)//回复收徒请求
	{
		//if(byResult == 0)//成功
		//{
		//}
		//else//失败
		//{
		//}
		BYTE byMsgLen = *((BYTE*)(msg + 8));
		char strTemp[128]={0};
		memcpy(strTemp,msg+12,byMsgLen);
		g_TalkMgr.PushSysTalk(strTemp);

	}

}

//第7位BYTE是物品类型　0　是回城神石,1是新手怪物召唤卡
//第8位BYTE是选择结果 回神石为选择的地点:　0　1　2　3 4　5
void CGameControl::SEND_Set_Use_Object_Para(BYTE byObjType,DWORD dwObjID,BYTE byData,const char * pStrData)//使用物品的参数设置，如回城神石
{
	SET_COMMAND(CS_SET_USE_OBJECT_PARA,128);
	ASSIGN_DWORD(0,dwObjID);
	ASSIGN_BYTE(6,byObjType);
	ASSIGN_BYTE(7,byData);

	if( (byObjType == 1) && pStrData)
	{
		ADD_STR(pStrData);
	}

	SEND_GAME_SERVER();
}

void  CGameControl::MSG_Player_SkillPower(const char* msg,int iLen)
{
	WORD wSkillPower = *(WORD*)(msg + 6);
	WORD wSkillPowerMax = *(WORD*)(msg + 8);

	SELF.SetSkillPower(wSkillPower);
	SELF.SetSkillPowerMax(wSkillPowerMax);
}

void CGameControl::SEND_Leopard_Action(int x,int y)
{
	SET_COMMAND(CS_LEPOARD_ACTION,12);
	ASSIGN_WORD(0,x);
	ASSIGN_WORD(2,y);
	ASSIGN_WORD(6,0x0001);
	SEND_GAME_SERVER();
}

//iType == 1 告诉服务器创建数据
//iType == 2 扩充宝盒
//iType == 3 强化
//iType == 4 开启宝盒
//iType == 5 拿出
//iType == 6 放入
//iType == 7 免费区和收费区交换
//dwGoodID 为取出拿出的物品ID
void CGameControl::SEND_BOX_INFO(int iType,DWORD dwGoodID,BYTE byArea)
{
	SET_COMMAND(CS_SPIRT_INFO,CMD_SIZE); 

	if(iType == 1 || iType == 2 || iType == 3 || iType == 4)
	{
		;
	}
	else
	{
		ASSIGN_DWORD(0,dwGoodID);
	}	

	ASSIGN_BYTE(6,iType);

	if(iType == 3)
	{
		int iNum = 0;
		for (int i=0;i < 6;i++)
		{
			if(g_NPC.GetGemGood(i).GetID() != 0)
			{
				iNum++;
				ADD_DWORD(g_NPC.GetGemGood(i).GetID());
			}
		}
		ASSIGN_BYTE(7,iNum);
	}
	else if(iType == 5 || iType == 6 || iType == 7)
	{
		ASSIGN_BYTE(10,byArea);
	}

	SEND_GAME_SERVER();
}

//大宝箱，可能出现大宝箱的协议
void CGameControl::MSG_MAZE_INFO(const char* msg,int iLen)
{
	int iNum = msg[6];
	if (iNum < 0 || iNum > 48) return;	

	_MazeMap& MazeMap = g_OtherData.GetMazeMap();	
	g_OtherData.GetMazeMap().clear();
	for (int i = 0;i < iNum;i++)
	{
		MazeMap.maze[msg[12+i]].byType = 0x08;
	}
    
	return;
}

void CGameControl::MSG_BOX_INFO(const char* msg,int iLen)
{
	int iType = msg[6];
	BYTE byGoodLen = BYTE(msg[7]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	stBoxInfo& boxInfo = g_pGameData->GetBoxInfo();

	if(iType == 0)
	{
		boxInfo.bOpen = true;
		CGood LingXiBox;
		LingXiBox.FromBuffer(msg + 12,false,byGoodLen);
		
		boxInfo.iFreeGoodCount = LOBYTE((WORD)LingXiBox.GetDura());
		boxInfo.iPayGoodCount = HIBYTE((WORD)LingXiBox.GetDura());
		boxInfo.iFreeCapacity = LOBYTE(LingXiBox.GetDuraMax());
		boxInfo.iPayCapacity = HIBYTE(LingXiBox.GetDuraMax());
		boxInfo.freegoods.ReSize(boxInfo.iFreeCapacity);
		boxInfo.paygoods.ReSize(boxInfo.iPayCapacity);

// 		if(LingXiBox.GetRecordTime(false) == 0)
// 		{
// 			sprintf(boxInfo.szExpireTime,"灵犀宝盒强化成功，永不消耗灵力");
// 			boxInfo.dwTime = 0;
// 		}
// 		else
		{
			DWORD time = LingXiBox.GetRecordTime(false);
			DWORD dwTimeNow = g_Timer.GetCurTime() + g_dwServerTime;

			if (time < dwTimeNow)//没有开启或者已经过期
			{
				sprintf(boxInfo.szExpireTime,"至尊宝未租赁");
			}
			else
			{
				boxInfo.dwTime = time;
				char* btmp = g_Timer.GetDateTime("%Y年%m月%d日 ",time);
				sprintf(boxInfo.szExpireTime,"至尊宝租赁到期时间： %s",btmp);
			}
		}

		g_pControl->Msg(MSG_CTRL_LINGXIBOX_WND,OPER_CUSTOM);
	}
	else if(iType == 1 && boxInfo.bOpen)
	{
		boxInfo.freegoods.Clear();
		boxInfo.paygoods.Clear();
		int iNum = msg[8];
		const char * buf = msg + 12;
		for(int i = 0; i < iNum; i++)
		{
			BYTE byArea = *buf;
			buf += 1;
			if (byArea == 0)//免费区
			{
				boxInfo.freegoods.Add(buf,byGoodLen);	
			}
			else//收费区
			{
				boxInfo.paygoods.Add(buf,byGoodLen);	
			}
			buf += byGoodLen;
		}
	}
	else if(iType == 2 && boxInfo.bOpen)	//取出物品
	{
		DWORD goodID = Conv_DWORD(msg);
		CGood * pGood = SELF.GetUsingTemp().FindGood(goodID);
		if(pGood)
		{			
			pGood->SetID(0);
		}
	}
	else if((iType == 3 || iType == 7) && boxInfo.bOpen)	//3:放入物品,//7:免费区和收费区物品交换
	{
		DWORD goodID = Conv_DWORD(msg);
		CGood * pGood = SELF.GetUsingTemp().FindGood(goodID);
		if(pGood)
		{
			int eToWnd = pGood->GetToWnd();
			pGood->SetToWnd(0);

			if (eToWnd == LingXiBox_Free_Wnd)
			{
				boxInfo.freegoods.Add(*pGood);
			}
			else if (eToWnd == LingXiBox_Pay_Wnd)
			{
				boxInfo.paygoods.Add(*pGood);
			}
			pGood->SetID(0);
		}
	}
// 	else if(iType == 4)		//强化宝盒
// 	{
// 		DWORD dwResult = Conv_DWORD(msg);
// 
// 		if(dwResult == 0)
// 		{
// 			if(g_NPC.IsCondensing())
// 			{
// 				g_MsgBoxMgr.PopSimpleAlert("\n\n对不起,系统正忙,请稍后再试。",0,0);
// 				return;
// 			}
// 
// 			g_pControl->PopupWindow(MSG_CTRL_COMMONCONDENSE_WND,OPER_CLOSE);
// 			g_pControl->PopupWindow(MSG_CTRL_SMALL_GEM_CON_WND,OPER_CLOSE);
// 			g_pControl->PopupWindow(MSG_CTRL_EQUIP_LEVEL_UP_WND,OPER_CLOSE);
// 			g_pControl->PopupWindow(MSG_CTRL_GEMCONDENSE_WND,OPER_RECREATE,7);
// 		}
// 		else if(dwResult == 1 || dwResult == 2)
// 		{
// 			g_NPC.SetIsCondensing(false);
// 
// 			for (int i=0;i < 6;i++)
// 			{
// 				g_NPC.GetGemGood(i).SetID(0);				
// 			}
// 
// 			g_pControl->Msg(MSG_CTRL_GEMCONDENSE_WND,OPER_CUSTOM + dwResult);
// 		}		
// 	}
 	//else if(iType == 7)		//扩宝盒需要灵力数
 	//{
 	//	DWORD dwResult = Conv_DWORD(msg);
 
 	//	string strTemp = StringUtil::format("\\你当前已启用了<color=green %d个格子>，你可以再用<color=green %d点灵力>将宝盒格子再扩充<color=green 2格>。你确\\定要扩充宝盒吗？",boxInfo.iCapacity,dwResult);
 	//	g_MsgBoxMgr.PopTagOkCancelBox(strTemp.c_str(),MSG_CTRL_ENLARGE_BOX,0);
 	//}

	
}

void CGameControl::Msg_Guild_Status_Switch(const char* msg,int iLen)
{//行会状态开关
	WORD wTemp = *((WORD*)(msg + 6));
	g_GuildData.SetGuildBufferSwitch(wTemp==1);
	wTemp = *((WORD*)(msg + 8));
	if(wTemp == 1)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM+1);
	}
	wTemp = *((WORD*)(msg + 10));
	g_GuildData.SetSelfGuildBufferSwitch(wTemp==1);
}

void CGameControl::Msg_Guild_Ini_Info(const char* msg,int iLen)
{
//　行会初始化协议	
	g_GuildData.SetGuildTael(*((DWORD*)msg));//自己的行会银两
	g_GuildData.SetGuildTowerLevel(*((WORD*)(msg+6)));
	g_GuildData.SetGuildFunc(*((DWORD*)(msg+8)));//权限
	//Buffer加成
	CGuildData::_GuildBuffer& guildBuff = g_GuildData.GetGuildBuffer();
	//g_GuildData.GetGuildTower().wTowerX = *((WORD*)(msg+12));
	//g_GuildData.GetGuildTower().wTowerY = *((WORD*)(msg+14));
	g_GuildData.SetGuildBufferSwitch(msg[16] == 1);//是开启行会塔
	BYTE* pBuff = (BYTE*)(msg + 17);
//主体Buffer加成
	guildBuff.m_byAttackFgh[0] = *pBuff;
	guildBuff.m_byAttackFgh[1] = *(pBuff + 1);
	guildBuff.m_byAttackRab[0] = *(pBuff + 2);
	guildBuff.m_byAttackRab[1] = *(pBuff + 3);
	guildBuff.m_byAttackDao[0] = *(pBuff + 4);
	guildBuff.m_byAttackDao[1] = *(pBuff + 5);
	guildBuff.m_byPhyDef[0]	   = *(pBuff + 6);
	guildBuff.m_byPhyDef[1]	   = *(pBuff + 7);
	guildBuff.m_byMagicDef[0]  = *(pBuff + 8);
	guildBuff.m_byMagicDef[1]  = *(pBuff + 9);
	guildBuff.m_byMagicHit     = *(pBuff + 10);
	guildBuff.m_byPhyHit       = *(pBuff + 11);
	guildBuff.m_byMagicAvoid   = *(pBuff + 12);	
	guildBuff.m_byPhyAvoid     = *(pBuff + 13);
////元神Buffer加成
//	guildBuff.m_byMAttackFgh[0] = *(pBuff + 14);
//	guildBuff.m_byMAttackFgh[1] = *(pBuff + 15);
//	guildBuff.m_byMAttackRab[0] = *(pBuff + 16);
//	guildBuff.m_byMAttackRab[1] = *(pBuff + 17);
//	guildBuff.m_byMAttackDao[0] = *(pBuff + 18);
//	guildBuff.m_byMAttackDao[1] = *(pBuff + 19);
//	guildBuff.m_byMPhyDef[0]    = *(pBuff + 20);
//	guildBuff.m_byMPhyDef[1]    = *(pBuff + 21);
//	guildBuff.m_byMMagicDef[0]  = *(pBuff + 22);
//	guildBuff.m_byMMagicDef[1]  = *(pBuff + 23);
//	guildBuff.m_byMMagicHit     = *(pBuff + 24);	
//	guildBuff.m_byMPhyHit       = *(pBuff + 25);
//	guildBuff.m_byMMagicAvoid   = *(pBuff + 26);
//	guildBuff.m_byMPhyAvoid     = *(pBuff + 27);
	guildBuff.m_byJinDefend = *(pBuff + 14);
	guildBuff.m_byMuDefend = *(pBuff + 15);
	guildBuff.m_byTuDefend = *(pBuff + 16);
	guildBuff.m_byShuiDefend = *(pBuff + 17);
	guildBuff.m_byHuoDefend = *(pBuff + 18);
	guildBuff.m_wHP = Conv_WORD(pBuff + 19);
	guildBuff.m_wMP    = Conv_WORD(pBuff + 21);
	//guildBuff.m_byMPhyDef[1]    = *(pBuff + 21);
	//guildBuff.m_byMMagicDef[0]  = *(pBuff + 22);
	//guildBuff.m_byMMagicDef[1]  = *(pBuff + 23);
	//guildBuff.m_byMMagicHit     = *(pBuff + 24);	
	//guildBuff.m_byMPhyHit       = *(pBuff + 25);
	//guildBuff.m_byMMagicAvoid   = *(pBuff + 26);
	//guildBuff.m_byMPhyAvoid     = *(pBuff + 27);

	g_GuildData.SetSelfGuildBufferSwitch(*((BYTE*)(msg + 45)) == 1);

	//填充官职映射表
	std::map<int,CGuildData::sOfficer>& offMap = g_GuildData.GetGuildOfficerMap();
	offMap.clear();
	int iBegin = 0;
	typedef pair<int,CGuildData::sOfficer> Int_Struct;
	Int_Struct sTemp;
	std::string strChairMan = StringUtil::toStr(msg+46,iBegin);
	g_GuildData.SetChairMan(strChairMan);
	while(iBegin < iLen - 46)
	{
		sTemp.first = StringUtil::toUInt(msg + 46,iBegin);//coreNum
		sTemp.second.wNum = StringUtil::toUInt(msg+46,iBegin);//数目
		sTemp.second.wTotalNum = StringUtil::toUInt(msg + 46,iBegin);//此封号最多人数
		sTemp.second.bFav = (StringUtil::toInt(msg + 46,iBegin) != 0);///////////////////是否有亲信
		sTemp.second.dwTael = StringUtil::toUInt(msg + 46,iBegin);
		sTemp.second.strTile = StringUtil::toStr(msg + 46,iBegin);//封号名
		offMap.insert(sTemp);
	}
}

void CGameControl::Msg_Guild_Off_Appoint_Dis(const char* msg,int iLen)
{
	//行会官员任罢免协议
	if (iLen <= 12) return;
		
	CGuildData::sOfficeCost& guildCost = g_GuildData.GetOfficeCost();
	guildCost.dwCost = *((DWORD*)msg);//银两
	guildCost.dwID = *((DWORD*)(msg + 8));//任命者ID
	guildCost.wJob = *((WORD*)(msg + 6));
	guildCost.strName.assign(msg+12,iLen - 12);

	//弹出对话框
	char ctemp[256] = {0};
	std::map<int ,CGuildData::sOfficer>::iterator it = g_GuildData.GetGuildOfficerMap().find(guildCost.wJob);
	if (it != g_GuildData.GetGuildOfficerMap().end())
	{
		S_CommonConfirm CommonConfirm;
		CommonConfirm.iType = 6;
		CommonConfirm.dwLastTime = 30000;
		CommonConfirm.dwRequestPersonID = guildCost.dwID;
		std::string::size_type iPos = it->second.strTile.find("亲信");
		if (iPos != std::string::npos)
		{
			it--;			
			sprintf(ctemp,"%s(职位)%s想邀请你成为他的亲信",it->second.strTile.c_str(),guildCost.strName.c_str());
		}
		else
		{
			sprintf(ctemp,"行会会长邀请你胜任%s职位",(it->second).strTile.c_str());
		}
		CommonConfirm.strMsg = ctemp;
		g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);
		g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
	}
}

void CGameControl::SEND_General_Reply_Confirm(bool bAccept,BYTE byType)
{	
	SET_COMMAND(CS_GENERAL_REPLY_CONFIRM,CMD_SIZE);
	ASSIGN_BYTE(0,byType);
	ASSIGN_BYTE(1,(bAccept?1:0));
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Off_Appoint_Confirm(bool bAccept)
{
	//任职确认协议
	CGuildData::sOfficeCost& guildCost = g_GuildData.GetOfficeCost();
	SET_COMMAND(CS_GUILD_OFF_ACCEPTORNOT,CMD_SIZE);
	ASSIGN_DWORD(0,guildCost.dwID);//任命者ID
	ASSIGN_WORD(6,guildCost.wJob);
	ASSIGN_BYTE(8,(bAccept?1:0));
	ADD_STR(guildCost.strName.c_str());
	SEND_GAME_SERVER();
}

//行会会长分配银两协议
void CGameControl::SEND_Guild_Assign_Tael(DWORD dwTael,const char* pName)
{
	SET_COMMAND(CS_GUILD_ASSIGN_TAEL,128);
	ASSIGN_DWORD(0,dwTael);//分配的银两数值
	ADD_STR(pName);
	SEND_GAME_SERVER();
}

//捐赠协议
void CGameControl::SEND_Guild_Contribute(DWORD dwID1,DWORD dwID2,DWORD dwID3)
{
	//捐赠协议 
	SET_COMMAND(CS_GUILD_RESOURCE_CONTRIBUTE,CMD_SIZE);
	if (dwID1 + dwID2 + dwID3 == 0)
	{
		ASSIGN_DWORD(0,0);
	}
	else
	{
		ASSIGN_DWORD(0,1);
		ADD_DWORD(dwID1);
		ADD_DWORD(dwID2);
		ADD_DWORD(dwID3);
	}
	//物品ID（物品ID==0时，申请NPC界面的内容）
	SEND_GAME_SERVER();
}


void CGameControl::SEND_GUILD_TASK_INFO(BYTE byType)
{
	SET_COMMAND(CS_GUILD_TASK_INFO,CMD_SIZE);
	ASSIGN_BYTE(0,byType);
	SEND_GAME_SERVER();
}
//0x0353 图腾效果,6,7字节的一个word的16位每位表示一种种图腾效果的有无,值为0表示没有这种效果,值为1表示有这种效果,
//第0位:隐身图腾, 第1位:反射图腾, 第2位:生命图腾,第3位:重力图腾,第4位:诅咒图腾,第5位:巫毒图腾,第6位:巫术图腾,第7位:魔力图腾,
void CGameControl::Msg_TuTeng_State(const char* msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD wState = Conv_WORD(msg + 6);
	CCharacterAttr *pChar = g_pGameData->FindCharacterByID(id);
	pChar->SetTuTengState(wState);
}

void CGameControl::MSG_JINGLI_VALUE(const char * msg,int iLen)
{
	char str[256] = "";
	WORD wJingLi = SELF.GetJingLi();
	if(Conv_WORD(msg + 6) > wJingLi)
	{
		sprintf(str,"精力值增加%d",Conv_WORD(msg + 6) - wJingLi);
		g_TalkMgr.PushSysTalk(str);
	}
	else if(Conv_WORD(msg + 6) < wJingLi)
	{
		sprintf(str,"精力值减少%d",wJingLi - Conv_WORD(msg + 6));
		g_TalkMgr.PushSysTalk(str);
	}

	SELF.SetJingLi(Conv_WORD(msg + 6));
	SELF.SetJingLiMax(Conv_WORD(msg));
}

// void CGameControl::MSG_LIANHUA_VALUE(const char * msg,int iLen)
// {
// 	SELF.SetLianHuaValue(Conv_DWORD(msg));
// }
// 
// void CGameControl::MSG_LIANHUA_LEVEL(const char * msg,int iLen)
// {
// 	SELF.SetLianHuaValue(Conv_DWORD(msg));
// 	SELF.SetLianHuaLevel((BYTE)msg[6]);
// }

void CGameControl::Msg_Player_Prompt_Info(const char* msg,int iLen)
{		
	if (iLen < 12 + 8 + 16) return;

	DWORD dwWndType;
	if (*(msg + 6) == 4 || *(msg + 6) == 5)
	{//军机令4//长是人千里5
		dwWndType = MAKELONG(*(msg + 6) - 3,3);
		g_pControl->PopupWindow(MSG_CTRL_TASK_WND,OPER_CREATE,dwWndType);
	}

	unsigned __int64* pInt64 = (unsigned __int64*)(msg + 12);
	g_pGameData->SetPaoPaoStatus(*pInt64);
	g_pGameData->SetPaoPaoStatus_Closed(*pInt64);
	
	BYTE* pNewHandStatus = (BYTE*)(msg + 20);
	g_pGameData->SetStatusInfo(pNewHandStatus);
}

//在线领道具提醒 下标6  0=没有领了   1=有领
void CGameControl::Msg_GiveOnlinePrize(const char* msg,int iLen)
{	
	SELF.SetHaveOnlinePrize(msg[6] == 1);
}

void CGameControl::Send_Player_Prompt_Status(int i,int iType)
{
	if (iType == 0)
	{
		if (i < 0 || i > 63) return;

		unsigned __int64 ui64 = g_pGameData->GetPaoPaoStatus();
		ui64 |= ((unsigned __int64)1 << i);
		g_pGameData->SetPaoPaoStatus(ui64);
	}
	else
	{
		if (!g_AICfgMgr.IsShowNewHandPrompt()) return;

		if ( i < 0 || i > 128) return;

		if(g_pGameData->HasStatus(i)) return;

		_sPromptInfo* promInfo = g_PromptInfoMgr.FindNewHandProInfo(i);
		if (!promInfo) return;

		g_pGameData->AddCareForInfo(promInfo);
		g_pControl->Msg(MSG_CTRL_UPDATE_INSTANCE_INFO,0);

		g_pGameData->SetStatusInfo(i);
		
	}

	SET_COMMAND(CS_WARM_HEART_PROMPT_INFO,255);
	
	unsigned __int64* pInt64 = (unsigned __int64*)(temp + 12);
	*pInt64 = g_pGameData->GetPaoPaoStatus();

	const BYTE* pStatus = g_pGameData->GetStatusInfo();
	memcpy(temp+20,pStatus,16);
	iBLen += 24;
	SEND_GAME_SERVER();
}

//void CGameControl::Send_Player_Prompt_Status(ePaoPaoPromptStatus paopao)
//{
//	unsigned __int64 ui64 = g_pGameData->GetPaoPaoStatus();
//	ui64 |= ((unsigned __int64)1 << paopao);
//	g_pGameData->SetPaoPaoStatus(ui64);
//
//	SET_COMMAND(CS_WARM_HEART_PROMPT_INFO,32);
//	ASSIGN_BYTE(6,3);
//	unsigned __int64* pInt64 = (unsigned __int64*)(temp + 12);
//	*pInt64 = ui64;
//	iBLen += 8;
//	SEND_GAME_SERVER();
//}

void CGameControl::Send_Produce_Info(DWORD dwID,BYTE byType)
{
	SET_COMMAND(CS_PRODUCE_INFO,32);
	
	ASSIGN_DWORD(0,dwID);
	ASSIGN_BYTE(6,byType);

	SEND_GAME_SERVER();
}

//请求托管,邀请托管(pName==NULL,为请求托管)(byMode==2时为退出,自己主动退出时pName==NULL,队长
//剔出时pName是需要的)byMode = 0为区域模式 byMode = 1为自由模式
// void CGameControl::Send_Trusteeship(const char* pName,BYTE byMode)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP,32);
// 	ASSIGN_BYTE(6,byMode);
// 	if (pName)
// 	{
// 		ADD_STR(pName);
// 	}
// 	SEND_GAME_SERVER();
// }

//成功地加入一个托管成员
//删除一个成员
//解散托管
// void CGameControl::MSG_Trustship(const char* msg,int iLen)
// {
// 	BYTE byFlag = *((BYTE*)(msg + 7));//0:区域托管, 1:自由托管, 2:剔除成员,  3:解散
// 	if (byFlag == 2)
// 	{
// 		if (strlen(msg+12) > 14) return;	//非法名称
// 
// 		for (int i=0; i< MAX_TRUSTEESHIP_NUM;i++)
// 		{
// 			TneupMember& tmp = g_TrusteeshipData.GetTneupMember(i);
// 			if (strcmp(tmp.szName, msg+12) == 0)
// 			{
// 				if (i == SELF.GetSelfTrusteeshipPos())
// 				{
// 					SELF.SetSelfTrusteeshipPos(-1);
// 					g_TrusteeshipData.SetTrusteeshipType(eTrusteeshipNone);
// 					g_pControl->PopupWindow(MSG_CTRL_QUITTNEUP_WND, OPER_CLOSE);
// 				}
// 				
// 				//不可能是队长，队长应该是解散，故不需要去做
// 				tmp.Clear();
// 			}
// 		}
// 	}
// 	else if (byFlag == 3)
// 	{
// 		g_pControl->PopupWindow(MSG_CTRL_TNEUPCHART_WND, OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_TNEUPCTRLPANEL_WND, OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_QUITTNEUP_WND, OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_PACKAGEWND, OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_TNEUP_WND,OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_TRUSTEESHIPCONFIG_WND,OPER_CLOSE);
// 		g_TrusteeshipData.Clear();
// 	}
// 	else if ((byFlag == 0 || byFlag == 1) && iLen > 14)
// 	{
// 		BYTE byPos = BYTE(msg[12 + 2]);//托管成员位置
// 		if (byPos >= 0 && byPos < MAX_TRUSTEESHIP_NUM)
// 		{
// 			TneupMember& tmp = g_TrusteeshipData.GetTneupMember(byPos);			
// 			tmp.FromeBuffer(msg + 12);
// 			
// 			if (tmp.dwID == SELF.GetID())
// 			{				
// 				g_pControl->PopupWindow(MSG_CTRL_QUITTNEUP_WND);
// 				SELF.SetSelfTrusteeshipPos(byPos);
// 			}
// 
// 			if (g_TrusteeshipData.IsCaptain())
// 			{
// 				Send_TrusteeshipMode(tmp.szName, eTrusteeshipModeFight);
// 			}
// 		}
// 	}
// }

//队长发送某一被托管对象的托管模式(战斗，跟随，驻扎...)
// void CGameControl::Send_TrusteeshipMode(const char* pName,DWORD dwMode, DWORD dwParam)
// {
// 	if (!pName) return;
// 
// 	SET_COMMAND(CS_TRUSTEESHIP_MODE,12);
// 	ASSIGN_DWORD(6,dwMode);
// 	if (dwMode == eTrusteeshipModeAttack || dwMode == eTrusteeshipModeGarrsion)
// 	{
// 		ASSIGN_DWORD(0, dwParam);
// 	}
// 	ADD_STR(pName);
// 	SEND_GAME_SERVER();
// }
//托管模式响应(战斗，跟随，驻扎...)
// void CGameControl::MSG_TrusteeshipMode(const char* msg,int iLen)
// {
// 	const char* pName = msg + 12;
// 
//  	TneupMember *pMember = g_TrusteeshipData.FindMemberByName(pName);
// 	if (pMember)
// 	{
// 		pMember->byAttackMode = (BYTE)Conv_DWORD(msg+6);
// 		DWORD dwParam = Conv_DWORD(msg);
// 		if (pMember->byAttackMode == eTrusteeshipModeAttack)
// 		{
// 			pMember->dwForceAttackID = dwParam;
// 		}
// 		else if (pMember->byAttackMode == eTrusteeshipModeGarrsion)
// 		{
// 			pMember->dwGarrsionXY = dwParam;
// 		}
// 	}
// }

//托管开关(区域，自由)
// void CGameControl::Send_TrusteeshipSwitch(BYTE byMode,bool bValue,bool bAddExp)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP_SWITCH,12);
// 	ASSIGN_BYTE(6,byMode);
// 	BYTE byValue = bValue?1:0;
// 	ASSIGN_BYTE(7,byValue);
// 
// 	if(bAddExp)
// 	{
// 		ASSIGN_BYTE(8,1);
// 	}
// 
// 	SEND_GAME_SERVER();
// }
//托管开关响应(区域，自由)//|0x01允许自由模式,|0x02允许区域模式
//void CGameControl::MSG_TrusteeshipSwitch(const char* msg,int iLen)
//{
// 	SELF.SetFreeTrusteeship(msg[6]);
// 	SELF.SetAreaTrusteeship(msg[7]);
// 
// 	g_pControl->MsgToWnd(MSG_CTRL_TEAMWND,MSG_CTRL_NOTIFY_TRUSTEESHIP_MODE);
//}

// void CGameControl::MSG_TrusteeshipHeadMode(const char* msg,int iLen)
// {
// 	g_TrusteeshipData.SetCapatinAttackMode(PLAYER_ATTACK_TYPE(Conv_DWORD(msg + 6)));
// 
// 	//换成队长攻击模式
// 	char ctemp[52] = {0};
// 	sprintf(ctemp,"@AttackMode %d",Conv_DWORD(msg + 6));
// 	if (!g_TrusteeshipData.IsCaptain())
// 		g_pGameControl->SEND_Guild_Ally(ctemp);
// }

// void CGameControl::MSG_TrusteeshipHeadPos(const char* msg,int iLen)
// {
// 	TneupMember* pCaptian = g_TrusteeshipData.GetCaptain();
// 	if (pCaptian)
// 	{
// 		pCaptian->wX = Conv_WORD(msg + 6);//HIWORD为Y,LOWORD为X
// 		pCaptian->wY = Conv_WORD(msg + 8);
// 	}
// }
// 
// void CGameControl::Send_TrusteeshipRespond(bool b, BYTE byMode, const char* pName)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP_INVITE_ASK,32);
// 	BYTE byValue = b?1:0;
// 	ASSIGN_BYTE(6,byMode);
// 	ASSIGN_BYTE(7,byValue);
// 	if (NULL != pName)
// 	{
// 		ADD_STR(pName);
// 	}
// 
// 	SEND_GAME_SERVER();
// }

// void CGameControl::Send_TrusteeshipHeadAttackTarget(DWORD dwID)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP_HEAD_TARGET,CMD_SIZE);
// 	ASSIGN_DWORD(0,dwID);
// 	SEND_GAME_SERVER();
// }

//void CGameControl::MSG_TrusteeshipAsk(const char* msg,int iLen)
//{
// 	BYTE byMode = *(msg + 6);
// 	BYTE byJob = *(msg + 10);
// 	WORD wLevel = Conv_WORD(msg + 8);
// 	bool bHead = (*(msg + 7) == 1);
// 
// 	char ctemp[256] = {0};
// 	string str[3] = {"战士","法师","道士"};
// 	string str2[2] = {"区域","自由"};
// 	if (bHead)
// 	{
// 		//队长邀请
// 		sprintf(ctemp,"队长邀请你进入%s托管模式，你是否同意？",str2[byMode].c_str());
// 		g_MsgBoxMgr.PopOkCancelBox(ctemp,MSG_CTRL_TRUSTEESHIP_RESPONSE,MAKELONG(MAKEWORD(byMode,1),1), NULL, MSG_CTRL_TRUSTEESHIP_RESPONSE,MAKELONG(MAKEWORD(byMode,0),1));
// 	}
// 	else
// 	{
// 		//成员申请
// 		char* pName = new char[strlen(msg + 12) + 1];
// 		memset(pName,0,strlen(msg + 12) + 1);
// 		strcpy(pName, msg+12);
// 		sprintf(ctemp,"%d级%s%s请求加入%s托管模式，你是否同意？",wLevel,pName,str[byJob].c_str(),str2[byMode].c_str());
// 		g_MsgBoxMgr.PopOkCancelBox(ctemp,MSG_CTRL_TRUSTEESHIP_RESPONSE,MAKELONG(MAKEWORD(byMode,1),0),(CControl*)pName,MSG_CTRL_TRUSTEESHIP_RESPONSE,MAKELONG(MAKEWORD(byMode,0),0),(CControl*)pName);
// 	}
//}

// void CGameControl::MSG_TrusteeshipHeadAttackTarget(const char* msg,int iLen)
// {
// 	DWORD dwCapAttackID = Conv_DWORD(msg);
// 	for (int i = 0; i < MAX_TRUSTEESHIP_NUM; i++)
// 	{
// 		g_TrusteeshipData.GetTneupMember(i).SetCapAttackID(dwCapAttackID);
// 	}
// }

// void CGameControl::MSG_TneupMember(const char* msg, int iLen)
// {
// 	g_TrusteeshipData.Clear();
// 
// 	BYTE byCount = Conv_BYTE(msg + 6);
// 	BYTE byTneupSize = Conv_BYTE(msg + 7);
// 
// 	//  [2/8/2010 dujun]
// 	TrusteeshipType temp = TrusteeshipType(Conv_DWORD(msg + 8));
// 	g_TrusteeshipData.SetTrusteeshipType(temp);//获取队长阵谱类型
// 	
// 	msg += 12;
// 
// 	for (BYTE i = 0; i < byTneupSize; i++)
// 	{
// 		g_TrusteeshipData.SetTneupJob(i, Conv_BYTE(msg));
// 		msg++;
// 	}
// 
// 	for (BYTE i=0; i<byCount; i++)
// 	{
// 		BYTE byPos = Conv_BYTE(msg+2);
// 		TneupMember &member = g_TrusteeshipData.GetTneupMember(byPos);
// 		member.FromeBuffer(msg);
// 
// 		if (member.dwID == SELF.GetID())
// 		{
// 			SELF.SetSelfTrusteeshipPos(byPos);
// 
// 			//复制自己的喝药设置到群英模式的喝药设置
// 			for (int j = 0; j < MAX_MEDICA_NUMBER; j++)
// 			{
// 				WOOOLMEDICAEAT *pHPMPMedicaInfo = g_AutoEatMgr.GetHPMPMedicaInfo(j);
// 				memcpy(&(member.eatConfig.medica[i]),pHPMPMedicaInfo,sizeof(WOOOLMEDICA));
// 			}
// 
// 			member.eatConfig.bUsePTSuperHP = g_AICfgMgr.IsUsePTSuperHP();
// 			member.eatConfig.bUsePTSuperMP = g_AICfgMgr.IsUsePTSuperMP();
// 		}
// 
// 		if (i == 0)	
// 		{
// 			g_TrusteeshipData.SetCaptainPos(byPos);//队长
// 		}
// 
// 		msg += 38;
// 	}
// 	
// 	g_pControl->PopupWindow(MSG_CTRL_TNEUPCHART_WND);
// 	if (g_TrusteeshipData.IsCaptain())//自己是队长
// 	{
// 		g_pControl->PopupWindow(MSG_CTRL_TNEUPCTRLPANEL_WND);
// 
// 		if (g_pControl->FindWindowByName("PackageWnd"))
// 		{
// 			g_pControl->PopupWindow("PackageWnd",OPER_RECREATE);
// 		}
// 	}
// }

// void CGameControl::MSG_Trusteeship_NeedJob(const char *msg, int iLen)
// {
// 	BYTE byType = Conv_BYTE(msg+6);
// 	BYTE byNum  = Conv_BYTE(msg+7);
// 
// 	for (int i=0; i<byNum; i++)
// 	{
// 		g_TrusteeshipData.SetTneupJob(i, Conv_BYTE(msg+12+i));
// 	}
// }

// void CGameControl::Send_Trusteeship_NeedJob(DWORD dwGoodID)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP_NEEDJOB,32);
// 	ASSIGN_DWORD(0,dwGoodID);
// 	SEND_GAME_SERVER();
// }

// void CGameControl::MSG_Trusteeship_HPMP(const char *msg, int iLen)
// {
// 	BYTE byPos = Conv_BYTE(msg+6);
// 	if (byPos < 0 || byPos >= MAX_TRUSTEESHIP_NUM)
// 		return;
// 	TneupMember& member = g_TrusteeshipData.GetTneupMember(byPos);
// 	if (member.szName[0] == 0)
// 		return;
// 	member.wMaxHP = Conv_WORD(msg);
// 	member.wHP    = Conv_WORD(msg+7);
// 	member.wMaxMP = Conv_WORD(msg+2);
// 	member.wMP    = Conv_WORD(msg+9);
// }

// void CGameControl::Send_Trusteeship_HPMP(BYTE byPos, WORD wMaxHP, WORD wMaxMP, WORD wHP, WORD wMP)
// {
// 	SET_COMMAND(CS_TRUSTEESHIP_HPMP,CMD_SIZE);
// 	ASSIGN_BYTE(6, byPos);
// 	ASSIGN_WORD(0, wMaxHP);
// 	ASSIGN_WORD(2, wMaxMP);
// 	ASSIGN_WORD(7, wHP);
// 	ASSIGN_WORD(9, wMP);
// 	SEND_GAME_SERVER();
// }

// void CGameControl::Send_QunYing_Reports()
// {
// 	g_OtherData.GetQunYingReports().clear();
// 	g_OtherData.SetHaveRequestQunYingReport(true);
// 
// 	SET_COMMAND(CS_QUNYING_REPORT,CMD_SIZE);
// 	SEND_GAME_SERVER();
// 	
// }

// void CGameControl::MSG_QunYing_Reports(const char* msg, int iLen)
// {
// 
// 	_QunYingReportStruct lreport;	
// 	lreport.iKills = Conv_DWORD(msg + 6);
// 	lreport.iExp = Conv_DWORD(msg+10);
// 	lreport.iPkKills = Conv_DWORD(msg+14);
// 	lreport.iDeaths = Conv_DWORD(msg+18);
// 	if (iLen > 22)
// 	{
// 		lreport.strName.assign(msg + 22,iLen - 22);
// 	}
// 
// 	if (lreport.strName != "")
// 	{
// 		g_OtherData.GetQunYingReports().push_back(lreport);
// 		g_pControl->Msg(MSG_CTRL_QUNYING_REPORT_WND,10,NULL);
// 	}
// }

//pos:0：升级整片翅膀,1－4：升级单片翅膀 
void CGameControl::Send_LevelUpWing(int pos, DWORD dwGoodId)
{
	SET_COMMAND(CS_LEVELUPWING,CMD_SIZE);
	ASSIGN_DWORD(0,dwGoodId);
	ASSIGN_WORD(6, pos);
	SEND_GAME_SERVER();	
}

void CGameControl::MSG_Full_ZhenBao_Left_Time(const char* msg, int iLen)
{
	DWORD dwLeftTime = Conv_DWORD(msg);
	if (dwLeftTime == 0)
	{
		SELF.SetFullZhenBaoTime(0);
	}
	else
	{
		SELF.SetFullZhenBaoTime(GetTickCount() + dwLeftTime * 1000);
	}
}

void CGameControl::MSG_ZhenBao_Value(const char* msg, int iLen)
{
	WORD wType = Conv_WORD(msg + 6);
	if (wType == 0)
	{
		SELF.SetZhenBaoValue(Conv_DWORD(msg));
	}
	else if (wType == 1 && iLen >= 12 + MAX_ZHENBAO_LEVEL * 4)
	{
		for (int i = 0; i < MAX_ZHENBAO_LEVEL; i++)
		{
			SELF.SetZhenBaoLevelMaxExp(i,Conv_DWORD(msg + 12 + i * 4));
		}
	}
}
void CGameControl::MSG_Honor_Value(const char* msg, int iLen)
{
	DWORD HonorValue = Conv_DWORD(msg);
	SELF.SetHonorValue(HonorValue);
}

void CGameControl::MSG_WenPei_List(const char * msg,int iLen)
{
	for (int i = 0; i < 4; ++i)
	{
		g_OtherData.GetSubWenPei(i).clear();
	}

	SubWenPei sub;
	int index = 0;
	int j = 12;
	while (j < iLen)
	{		
		sub.nums = Conv_BYTE(msg+j);
		if (sub.nums > 0)
		{
			//纹佩looks
			if (index >= 0 && index < 31)
			{
				sub.id = index + 1;
				sub.looks = CGood::GetSubWenPeiLooks(1,sub.id);
				g_OtherData.GetSubWenPei(0).push_back(sub);
			}
			else if (index >= 31 && index < 31 * 2)
			{
				sub.id = index - 31 + 1;
				sub.looks = CGood::GetSubWenPeiLooks(2,sub.id);
				g_OtherData.GetSubWenPei(1).push_back(sub);
			}
			else if (index >= 31 * 2 && index < 31 * 3)
			{
				sub.id = index - 31 * 2 + 1;
				sub.looks = CGood::GetSubWenPeiLooks(3,sub.id);
				g_OtherData.GetSubWenPei(2).push_back(sub);
			}
			else if (index >= 31 * 3 && index < 31 * 4)
			{
				sub.id = index - 31 * 3 + 1;
				sub.looks = CGood::GetSubWenPeiLooks(4,sub.id);
				g_OtherData.GetSubWenPei(3).push_back(sub);
			}			
			else if (index >= 31 * 4 && index < 31 * 4 + 16)
			{
				sub.id = index - 31 * 4 + 1 + 31;
				sub.looks = CGood::GetSubWenPeiLooks(1,sub.id);
				g_OtherData.GetSubWenPei(0).push_back(sub);
			}
			else if (index >= 31 * 4 + 16 && index < 31 * 4 + 16 * 2)
			{
				sub.id = index - 31 * 4 - 16 + 1 + 31;
				sub.looks = CGood::GetSubWenPeiLooks(2,sub.id);
				g_OtherData.GetSubWenPei(1).push_back(sub);
			}
			else if (index >= 31 * 4 + 16 * 2 && index < 31 * 4 + 16 * 3)
			{
				sub.id = index - 31 * 4 - 16 * 2 + 1 + 31;
				sub.looks = CGood::GetSubWenPeiLooks(3,sub.id);
				g_OtherData.GetSubWenPei(2).push_back(sub);
			}
			else if (index >= 31 * 4 + 16 * 3 && index < 31 * 4 + 16 * 4)
			{
				sub.id = index - 31 * 4 - 16 * 3 + 1 + 31;
				sub.looks = CGood::GetSubWenPeiLooks(4,sub.id);
				g_OtherData.GetSubWenPei(3).push_back(sub);
			}
		}

		index++;
		j ++;
	}
}

//合成纹佩  0－3：分别表示合成的4个部件编号
void CGameControl::SEND_WenPei_Compound(int id1, int id2, int id3, int id4)
{
	SET_COMMAND(CS_WENPEI_COMPOUND,CMD_SIZE);
	ASSIGN_BYTE(0,id1);
	ASSIGN_BYTE(1, id2);
	ASSIGN_BYTE(2, id3);
	ASSIGN_BYTE(3, id4);
	SEND_GAME_SERVER();	
}

//拆分纹佩部件  0－3：纹佩部件编号  百位表示部位，十位个位表示编号，例如：301表示第四个部分的1号
void CGameControl::SEND_WenPeiSub_Decompound(int iSubId, int dwId)
{
	int isend = iSubId * 100 + dwId;

	SET_COMMAND(CS_WENPEISUB_DECOMPOUND,CMD_SIZE);
	ASSIGN_DWORD(0,isend);
	ASSIGN_WORD(6,0);
	SEND_GAME_SERVER();	
}

//拆卸纹佩   0－3：纹佩指针
void CGameControl::SEND_WenPei_Decompound(DWORD dwId)
{
	SET_COMMAND(CS_WENPEI_DECOMPOUND,CMD_SIZE);
	ASSIGN_DWORD(0,dwId);
	SEND_GAME_SERVER();	
}

//强化纹佩   0－3：纹佩指针， 8－11：材料指针  协议体头4字节：幸运符
void CGameControl::SEND_WenPei_Streng(DWORD dwGoodId1, DWORD dwGoodId2, DWORD dwFu)
{
	SET_COMMAND(CS_WENPEI_STRENG,32);
	ASSIGN_DWORD(0,dwGoodId2);
	ASSIGN_DWORD(8,dwGoodId1);
	if (dwFu != 0)
	{
		ADD_DWORD(dwFu);
	}
	SEND_GAME_SERVER();	
}

void CGameControl::SEND_WenPei_RongHe(DWORD dwLeftGoodId1, DWORD dwRightGoodId2, DWORD dwMdGoodId)
{
	SET_COMMAND(CS_WENPEI_RONGHE,32);
	ASSIGN_DWORD(0,dwLeftGoodId1);
	ASSIGN_DWORD(8,dwRightGoodId2);
	if (dwMdGoodId != 0)
	{
		ADD_DWORD(dwMdGoodId);
	}
	SEND_GAME_SERVER();	
}

//纹佩部件提取  0－3：纹佩部件编号  百位表示部位，十位个位表示编号，例如：301表示第四个部分的1号
void CGameControl::SEND_WenPeiSub_TiQu(int iSubId, int dwId)
{
	int isend = iSubId * 100 + dwId;

	SET_COMMAND(CS_WENPEISUB_DECOMPOUND,CMD_SIZE);
	ASSIGN_DWORD(0,isend);
	ASSIGN_WORD(6,1);
	SEND_GAME_SERVER();	
}

//void CGameControl::SEND_Mac_To_GS()
//{
//	SET_COMMAND(CS_MAC,64);
//
//	ADD_STR(g_strMac);
//
//	SEND_GAME_SERVER();	
//}

void CGameControl::SEND_HasWaiGua(bool bHas)
{
	SET_COMMAND(CS_HASWAIGUA,CMD_SIZE);

	ASSIGN_BYTE(6,bHas);

	SEND_GAME_SERVER();	
}

void CGameControl::MSG_HasWaiGua(const char* msg, int iLen)
{
	std::string str/*,strClassName,strTitle*/;
	str.assign(msg + 12,iLen - 12);	

	//int iPos = str.find("/");
	//if (iPos >= 0)
	//{
	//	strClassName = str.substr(0,iPos);
	//	strTitle = str.substr(iPos + 1,str.length() - (iPos + 1));
	//}
	//else
	//{
	//	strClassName = str;
	//}


	//const char *pStrClassName = NULL;
	//if(!strClassName.empty())
	//	pStrClassName = strClassName.c_str();

	//const char *pStrTitle = NULL;
	//if(!strTitle.empty())
	//	pStrTitle = strTitle.c_str();

	////找到外挂窗口
	//if(FindWindowEx(NULL,NULL,pStrClassName,pStrTitle))
	//{
	//	SEND_HasWaiGua(1);
	//}

	//if(str.size() > 0)
	//	g_AntiWaiGua.FindClientsNumber(str.c_str());
}

void CGameControl::MSG_Player_ConSkillBuff(const char * msg,int iLen)
{
	DWORD wMagicID = Conv_DWORD(msg);
	WORD iTime = Conv_WORD(msg+6);
	if (iTime > 0)
	{
		g_AIMgr.AddConSubSkillBuff((WORD)wMagicID, iTime*1000);
	}
	else
	{
		g_AIMgr.RemoveConSubSkillBuff((WORD)wMagicID,true);
	}
}

//将仓库物品位置发送
void CGameControl::SEND_Objects_Storage_Position()
{
	SET_COMMAND(CS_OBJECT_STORAGE_POSITION,1024);

	std::vector<CGood> vGood = g_NPC.GetNpcQuickVector();
	DWORD* posArray = CNpcQuickWnd::GetGoodPos(); 

	//for (std::vector<CGood>::iterator itr = vGood.begin(); itr != vGood.end(); ++itr)
	//{
	//	DWORD id = (*itr).GetID();
	//	if(id == 0)
	//		continue;
	//	for (std::vector<GoodPos_t>::iterator itrPos = posArray.begin(); itr != posArray.end(); ++itr)
	//	{
	//		if ((*itrPos).dwID == id)
	//		{
	//			ADD_DWORD(id);
	//			ADD_BYTE((*itrPos).iPos);
	//		}
	//	}	
	//}
	ASSIGN_WORD(10,vGood.size());
	for (std::vector<CGood>::iterator itr = vGood.begin(); itr != vGood.end(); ++itr)
	{
		for (int ii = 0; ii != MAXNUMBERSTORAGE; ++ii)
		{
			if (posArray[ii] == (*itr).GetID())
			{
				ADD_DWORD(posArray[ii]);
				ADD_BYTE(ii);
				break;
			}
		}
	}

	

	SEND_GAME_SERVER();
}
//itype : 0:购买VIP,1:请求VIP商城信息,2:勾选/取消自动续费,3:卖物品,4:取回Vip商城物品 ,5:买vip商城物品,6:开宝箱
void CGameControl::SEND_Player_Vip_Option(int iType,DWORD dwData1,DWORD dwData2,DWORD dwData3,const char *szData)
{
	SET_COMMAND(CS_PLAYER_VIP_OPTION,128);

	ASSIGN_BYTE(6,(BYTE)iType);

	switch(iType)
	{
	case 0:
		ASSIGN_BYTE(8,(BYTE)dwData1);
		break;
	case 1:
		ASSIGN_DWORD(8,dwData2);//请求第几页的数据
		break;
	case 2:
		ASSIGN_BYTE(8,(BYTE)dwData1);
		break;
	case 3:
		ASSIGN_ID(dwData1);
		ASSIGN_BYTE(7,(dwData3 > 0)?1:0);//元宝:1,金币:0
		ASSIGN_DWORD(8,(dwData3 > 0)?dwData3:dwData2);
		break;
	case 4:
		ASSIGN_ID(dwData1);
		break;
	case 5:
		ASSIGN_ID(dwData1);
		ADD_STR(szData);
		break;
	case 6:
		ASSIGN_BYTE(7,(BYTE)dwData1);
		break;
		
	default:
		return;
		break;
	}

	SEND_GAME_SERVER();	

}

void CGameControl::MSG_Player_VipInfo(const char * msg,int iLen)
{
	S_VipTrade &vip = g_OtherData.GetVipTradeStruct();

	vip.tEndTime = Conv_DWORD(msg);
	vip.byEffected = BYTE(msg [6]); 
	vip.byVipType = BYTE(msg[7]);
	vip.byVipLevel = BYTE(msg[8]);
	vip.bAutoExtern = msg[9] > 0?true:false;
	vip.byBoxOpened = msg[10];

	vip.iVipExp = Conv_DWORD(msg + 12);

	if(vip.byVipType > 0 && vip.byEffected)
	{
		SELF.SetVipTradeLevel(vip.byVipLevel);
	}
	else
	{
		SELF.SetVipTradeLevel(0);
	}

	if (g_pControl->FindWindowByName("VipCardWnd"))
	{
		g_pControl->PopupWindow(MSG_CTRL_VIPCARD_WND,OPER_RECREATE);
		vip.bNeedOpenWndWhenReceiveData = false;
	}
}


void CGameControl::MSG_VipStoreInfo(const char * msg,int iLen)
{
	string buf;

	DWORD dwStartIndex = Conv_DWORD(msg) * PERONS_PER_PAGE_VIPTRADEWND;
	BYTE byGoodLen = BYTE(msg[6]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	if(msg[10] == 1)		//消息体进过压缩
	{
		DWORD size = 128 * 1024;
		Uncompress((byte *)g_cBuf,&size,(byte *)(msg + CMD_SIZE),iLen - CMD_SIZE);
		buf.assign(g_cBuf,size);		
	}
	else
	{
		buf.assign(msg + CMD_SIZE,iLen - CMD_SIZE);
	}	

	int iTotalNameCount = Conv_WORD(msg + 8);
	int iSize = buf.size();
	int iNameCount = (BYTE)(msg[11]);

	SVipTradGood &VipTradeGood = g_NPC.GetVipTradeGood();
	VipTradeGood.bHaveRequireData = true;

	VipTradeGood.iVipPersons = iTotalNameCount;

	BYTE bySelType = 0;
	int iPrice = 0;
	for (int i = 0,iPos = 0; i < iNameCount && iPos < iSize; i++)
	{
		SVipTradGood_OnewPerson vogood;
		vogood.strOwner.assign(buf.c_str() + iPos,14);
		iPos += 14;

		int iGoodCount = *((int *)(buf.c_str() + iPos));
		iPos += 4;
		vogood.iVipExp = *((int *)(buf.c_str() + iPos));
		iPos += 4;

		for (int j = 0; j < iGoodCount && iPos < iSize; j++)
		{
			CGood good;

			bySelType = (BYTE)(buf.c_str()[iPos]);
			iPos += 1;
			iPrice = *((int *)(buf.c_str() + iPos));
			iPos += 4;

			good.FromBuffer(buf.c_str() + iPos,false,byGoodLen);
			iPos += byGoodLen;

			good.SetPayType(bySelType);
			good.SetPrice(iPrice);

			vogood.VGoods.push_back(good);
		}

		VipTradeGood.mMVipGood[dwStartIndex + i] = vogood;
	}

	g_pControl->PopupWindow(MSG_CTRL_FIRM_WND,MSG_CTRL_FIRM_WND,1);

}


// 	if (wType == 0)  //购买VIP
// 	{
// 		if(pPlayer->m_pVipInfo)
// 			return;
// 		BYTE byVipType = pBuf[8];
// 		gpVipMgr->BuyVip(pPlayer,byVipType);
// 	}
// 	else if (wType == 1) //请求VIP商城信息
// 	{
// 		pPlayer->SendData(CPacketBuilder::VipStoreInfo());
// 	}
// 	else if (wType == 2)  //勾选/取消自动续费
// 	{
// 		if(!pPlayer->m_pVipInfo)
// 			return;
// 		pPlayer->m_pVipInfo->SetAutoExtern(pBuf[8] == 1);
// 	}
	//else if (wType == 3)   //卖物品
	//{
	//	if(!pPlayer->m_pVipInfo)
	//		return;	
	//	int iPrice = (*(int*)(pBuf+6));
	//	CItem * pSellItem = (CItem*)(*((DWORD*)pBuf));
	//	BYTE bySellType = pBuf[10];
	//	gpVIPMgr->VipStoreSellItem(pPlayer,pSellItem,bySellType,iPrice);
	//}
// 	else if(wType == 4)   //取回所得收入
// 	{
// 		if(!pPlayer->m_pVipInfo)
// 			return;	
// 		gpVipMgr->GetBackSaledMoney(pPlayer);
// 	}
// 	else if(wType == 5)   //取回Vip商城物品
// 	{
// 		if(!pPlayer->m_pVipInfo)
// 			return;	
// 		CItem * pSellItem = (CItem*)(*((DWORD*)pBuf));
// 		gpVIPMgr->GetBackVipSaleItem(pPlayer,pSellItem);
// 	}
// 	else if(wType == 6)   //买vip商城物品
// 	{
// 		CItem * pSellItem = (CItem*)(*((DWORD*)pBuf));
// 		if(!pSellItem || nLen < 13)
// 			return;
// #ifdef NOENCODE
// 		std: tring strRecvBuf(&pBuf[12], nLen - 12);
// #else
// 		--nLen;			//去除尾部的\0
// 		std: tring strRecvBuf = Decode(pBuf + 12, nLen - 12);
// #endif
// 		gpVIPMgr->BuyItemFromVipStore(pPlayer,strRecvBuf,pSellItem);
// 	}
// 	else
// 		return;

/*
0-1 (放置:兵卒类型, 移动和撤回:兵卒编号)
2-3 操作类型(0:请求信息, 1: 放置, 2: 移动, 3: 撤回)
4-5 协议号
6-7 放置或移动坐标X
8-9 放置或移动坐标Y
10-11 殿内还是殿外,0外，1内
*/
void CGameControl::SEND_EditBing(WORD wEditType, BYTE byType, WORD wInOut, WORD wBingID, int x, int y)
{
	if (wEditType >= 1 && wEditType <= 3)
	{
		SET_COMMAND(CS_EDIT_BING,CMD_SIZE);

		switch (wEditType)
		{	
		case 1:
			ASSIGN_WORD(0,(WORD)byType);
			break;
		case 2:
		case 3:
			ASSIGN_WORD(0,wBingID);
			break;
		}

		ASSIGN_WORD(2,wEditType);
		ASSIGN_WORD(6,(WORD)x);
		ASSIGN_WORD(8,(WORD)y);
		ASSIGN_WORD(10,wInOut);

		SEND_GAME_SERVER();	
	}	
}

void CGameControl::SEND_EditBingAdd(BYTE byType, WORD wInOut, int x, int y)
{
	SEND_EditBing(1,byType,wInOut,0,x,y);
}

void CGameControl::SEND_EditBingDelete(WORD wInOut, WORD wBingID)
{
	SEND_EditBing(3,0,wInOut,wBingID,0,0);
}

void CGameControl::SEND_EditBingMove(WORD wInOut, WORD wBingID, int newx, int newy)
{
	SEND_EditBing(2,0,wInOut,wBingID,newx,newy);
}

void CGameControl::SEND_GetBingInfo()
{
	SET_COMMAND(CS_EDIT_BING,CMD_SIZE);
	ASSIGN_WORD(2,0);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_BingInfo(const char * msg,int iLen)
{
	WORD infonum = Conv_WORD(msg + 6);//兵种数量信息
	WORD num = Conv_WORD(msg + 8);//已出战兵卒信息

	BingList& lBings = g_OtherData.GetBings();
	lBings.clear();
	if (infonum > 0)
		lBings.reserve(infonum);
	

	const char * buf = msg + 12;
	for (int i = 0; i < infonum; ++i)
	{
		S_Bing bing;
		memset(&bing.info,0,sizeof(bing.info));
		memcpy(&bing.info,buf,sizeof(bing.info));
		buf += sizeof(bing.info);
		lBings.push_back(bing);
	}

	for (int i = 0; i < num; ++i)
	{
		S_BingNet bingnet;
		memset(&bingnet,0,sizeof(bingnet));
		memcpy(&bingnet,buf,sizeof(bingnet));
		buf += sizeof(bingnet);

		S_Bing* bing = g_OtherData.GetBingByType(bingnet.byType);
		if (bing)
		{
			bing->iCurNum++;
		}
	}

	g_pControl->MsgToWnd("Defend12GongWnd",MSG_CTRL_DEFEND12GONG_WND,10);
}

void CGameControl::MSG_OpenBingCtrlWnd(const char * msg,int iLen)
{
	g_pControl->PopupWindow(MSG_CTRL_DEFEND12CTRL_WND,OPER_CREATE);
}

void CGameControl::SEND_BingControl(WORD type, DWORD dwTargetID)
{
	SET_COMMAND(CS_CONTROL_BING,CMD_SIZE);
	ASSIGN_WORD(6,type);
	if (dwTargetID != 0)
		ASSIGN_DWORD(0,dwTargetID);
	SEND_GAME_SERVER();
}

/*
0x04B2
0－3异火等级
12－19：异火经验
12－19：异火经验
20－24：五行攻击加成
24－25：幸运
26－27：攻击上限
28－29：攻击下限
30－31：命中
32－33：暴击
34－35：无视防御
要加个契合度
无视防御与快捷键中间加个契合度【float，4字节】
36： 快捷键

后面：名字
*/
//void CGameControl::MSG_YIHUO_REQUEST(const char * msg,int iLen)
//{
//	SELF.GetYIHUOINFO().dwLevel = Conv_DWORD(msg);
//	SELF.GetYIHUOINFO().i64CurExp = Conv_INT64(msg + 12);
//	SELF.GetYIHUOINFO().i64CurMaxExp = Conv_INT64(msg + 20);
//
//	SELF.GetYIHUOINFO().AttackAddJin = Conv_BYTE(msg + 28);
//	SELF.GetYIHUOINFO().AttackAddMu = Conv_BYTE(msg + 29);
//	SELF.GetYIHUOINFO().AttackAddTu = Conv_BYTE(msg + 30);
//	SELF.GetYIHUOINFO().AttackAddShui = Conv_BYTE(msg + 31);
//	SELF.GetYIHUOINFO().AttackAddHuo = Conv_BYTE(msg + 32);
//
//	SELF.GetYIHUOINFO().wLucy = Conv_WORD(msg + 33);
//	SELF.GetYIHUOINFO().wAttackUpper = Conv_WORD(msg + 35);
//	SELF.GetYIHUOINFO().wAttackLower = Conv_WORD(msg + 37);
//	SELF.GetYIHUOINFO().wHit = Conv_WORD(msg + 39);
//	SELF.GetYIHUOINFO().wCruelAttack = Conv_WORD(msg + 41);
//	SELF.GetYIHUOINFO().wAbsDefend  = Conv_WORD(msg + 43);
//	SELF.GetYIHUOINFO().fQiHeDu = *((float*)(msg + 45));
//	SELF.GetYIHUOINFO().byShortCut =  Conv_BYTE(msg + 49);
//	memcpy(SELF.GetYIHUOINFO().strName,msg + 50,iLen - 50);
//}
/*
c-s 04b2 协议号后一个byte 快捷键,消息头跟一个字符串	
*/
void CGameControl::SEND_YIHUO_NAME_SHORTCUT(BYTE shortcut, const char* pName)
{
	SET_COMMAND(CS_YIHUO_SET_NAME_SHORTCUT,128);
	ASSIGN_BYTE(6,shortcut);
	ADD_STR(pName);
	SEND_GAME_SERVER();
}

/**
八卦盾状态通知
offset = 0, 4字节表示玩家ID
offset = 4, 2字节表示消息号0xA46
offset = 6, 2字节表示当前畜力值
offset = 8, 2字节表示最大畜力值
offset = 10, 1字节表示是否闪烁畜力条,0闪烁，非0不闪烁
offset = 11, 1字节表示盾的显示状态，0显示盾，非0不显示
*/
void CGameControl::MSG_Fire_Shield(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD curProgress = Conv_WORD(msg+6);
	WORD maxProgress = Conv_WORD(msg+8);
	BYTE byShan = Conv_BYTE(msg+10);
	BYTE byDun = Conv_BYTE(msg+11);

	//判断此ID是否存在
	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(uID);
	if(pChar == NULL)
		return;

	pChar->Set8DunCurProgress((int)curProgress);
	pChar->Set8DunMaxProgress((int)maxProgress);
	pChar->Set8DunShan(byShan == 0);
	pChar->Set8DunState(byDun == 0);
}

/**
无量五行诀
offset = 0, 4字节表示玩家ID
offset = 4, 2字节表示消息号0xA47
offset = 6, 2字节表示状态：0，表示处于无量五行诀状态，其他表示无量五行诀状态消失
offset = 8, 2字节表示此状态的剩余时间
*/
void CGameControl::MSG_DaoZun_JiangLin(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD daoZunState = Conv_WORD(msg+6);
	WORD time = Conv_WORD(msg+8);

	//判断此ID是否存在
	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(uID);
	if(pChar == NULL)
		return;

	pChar->SetDaoZunJiangLinState(daoZunState == 0);
}

/**
八方分影斩
offset = 0, 4字节表示玩家ID
offset = 4, 2字节表示消息号0xA48
offset = 6, 2字节表示玩家瞬移的目标点X
offset = 8, 2字节表示玩家瞬移的目标点Y
offset = 10, 1字节表示幻影数量
offset = 11, 1字节表示玩家方向
*/
void CGameControl::MSG_ShadowKill8(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD tX = Conv_WORD(msg+6);
	WORD tY = Conv_WORD(msg+8);
	BYTE byShadowNum = Conv_BYTE(msg+10);
	BYTE dir = Conv_BYTE(msg+11);

	if (byShadowNum > 7)
		byShadowNum = 7;
	

	int iMagicAction = ACTION_MAGIC;
	WORD wMagicID = MAGICID_SHADOWKILL8;
	
	if(uID == SELF.GetID())	
	{		
		SELF.SetDir(dir);
		SELF.InitShadowNum(byShadowNum);
		SELF.SetShadowKill8Dir(dir);

		SNextAction* pNext = SELF.PushSNextAction(iMagicAction,dir,tX,tY);
		if(pNext)
		{
			pNext->iData1 = wMagicID;
			pNext->uData1 = EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE;
			pNext->uData2 = uID;
			pNext->uData3 = MAKELONG(tX,tY);

			pNext->uFlag |= CREATE_MAGIC;
		}

		SELF.SetRealXY(tX,tY); 

	}
	else
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(uID);

		if(pChar)											// 玩家本来就在屏幕内
		{			
			pChar->SetDir(dir);
			pChar->InitShadowNum(byShadowNum);
			pChar->SetShadowKill8Dir(dir);

			SNextAction* pNext = pChar->PushSNextAction(iMagicAction,dir,tX,tY);
			if(pNext)
			{
				pNext->iData1 = wMagicID;
				pNext->uData1 = EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE;
				pNext->uData2 = uID;
				pNext->uData3 = MAKELONG(tX,tY);

				pNext->uFlag |= CREATE_MAGIC;
			}

			pChar->SetOffset(0,0); 
			pChar->SetDrawOffset(0,0);
			pChar->SetRealXY(tX,tY); 
		}
		else// 玩家从屏幕外移动到屏幕内
		{
			pChar = g_pGameData->AddSimpleCharacter(tX,tY,uID);
			if(!pChar)
				return;	
			pChar->SetDir(dir);
			pChar->SetRealXY(tX,tY);
			pChar->SetXY(tX,tY);
			pChar->InitAction(ACTION_STAND);
		}
	}
}

void CGameControl::SEND_Enlarge_Package_Info_Req()
{
	SET_COMMAND(CS_ENLARGE_PACKAGE_INFO_REQ,CMD_SIZE);
	ASSIGN_ID(SELF.GetID());

	SEND_GAME_SERVER();
}

//offset = 0, 4字节表示玩家ID
//offset = 6, 2字节表示扩金币上限所需元宝
//offset = 8, 4字节表示扩金币上限所需金币
//offset = 12, 4字节表示玩家当前最大可携带金币
//offset = 16, 4字节表示玩家扩充后最大可携带金币
void CGameControl::MSG_Enlarge_Package_Info_Ack(const char * msg,int iLen)
{
	WORD wNeedYuanBao = Conv_WORD(msg + 6);
	DWORD dwNeedMonery = Conv_DWORD(msg + 8);
	DWORD dwMaxMonery = Conv_DWORD(msg + 16);

	char szNeedMonery[32] = {0};
	char szMaxMonery[32] = {0};
	ToCommaStr(szNeedMonery,dwNeedMonery);
	ToCommaStr(szMaxMonery,dwMaxMonery);


	char szMsg[256] = {0};
	sprintf(szMsg,"你现在可以将钱袋扩充到存放%s金币，但要消耗%d个元宝和%s金币。你要扩钱袋吗？",szMaxMonery,wNeedYuanBao,szNeedMonery);

	g_MsgBoxMgr.PopOkCancelBox(szMsg,MSG_CTRL_ENLARGE_MONEYBAG,0);

}
