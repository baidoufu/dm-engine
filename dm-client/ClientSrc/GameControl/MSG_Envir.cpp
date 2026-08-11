#include "Global/Global.h"
#include "GameControl.h"
#include "GameData\MagicDefine.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameData.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/GameGlobal.h"
#include "GameMap/GameMap.h"
#include "GameData/OtherData.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameAI/AIMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameAI/AIPromptMgr.h"
#include "GameAI/AIAutoMgr.h"
#include "GameData/NpcData.h"
#include "WndClass/GameWnd/GMagicManager.h"
#include "WndClass/GameWnd/GameManager.h"
#include "GameAI/AIConfigMgr.h"
#include "GameAI/AutoKillMonsterMgr.h"
#include "GameData/DirtyWords.h"

void CGameControl::MSG_Monster_Name(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen-12);

	DWORD id = Conv_DWORD(msg);
	if(id == SELF.GetID())
	{
		SELF.SetName(buf.c_str());

		if(msg[7] != 0x01)
			SELF.SetNameColor(msg[6]);

		return;
	}

	CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
	if(!pChar) return;
	if(msg[7] != 0x01)
		pChar->SetNameColor(msg[6]);

	///////////////////////////////////////////////////////////////
	////豹子的留言 豹语泡泡
	//if ( ((atemp->GetLooks() >> 16) & 0x000000FF) == 166 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 167 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 168 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 185 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 186 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 187 ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 20  ||
	//	((atemp->GetLooks() >> 16) & 0x000000FF) == 2 
	//	)
	//{
	//	std::string strTemp;
	//	std::string strTemp1 ;
	//	strTemp.assign(temp,iLen-12);
	//	int i1 = strTemp.find(")-");
	//	if(i1>0 )
	//	{
	//		int i2 = strTemp.find(':',i1+2);
	//		if(i2>=0 && i2< iLen-12)
	//		{
	//			strTemp1.assign(temp+i2+1,iLen-i2-13);
	//			atemp->SetPetLeaveWords(strTemp1.c_str());
	//		}
	//		else
	//		{
	//			atemp->GetPetLeaveWords().clear();
	//		}
	//	}
	//}


	pChar->SetName(buf.c_str());

	//if(g_bEnableWaigua)
	//{
	//	if(atemp->IsPet())
	//	{
	//		g_pAssTools->GetRemainder().CharacterToPet(atemp->GetID());
	//	}
	//}


}


void CGameControl::MSG_Other_Player_Info(const char * msg,int iLen)
{
	g_OtherData.OtherPlayer().FromBuffer(msg,iLen);

	const char * name = g_OtherData.OtherPlayer().GetName();
	CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter((char*)name);

	if (!pChar)
		return;

	g_pControl->PopupWindow(MSG_CTRL_OTHER_PLAYER_INFO,OPER_RECREATE);

	g_OtherData.OtherPlayer().GetLooks().Player.wBody = pChar->GetBody();
}

// 这里的刷新游戏问题
void CGameControl::MSG_Monster_Appearance(const char * msg,int iLen)
{
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	DWORD id = lpPacketMsg->stDefMsg.nRecog;
	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	BYTE bDir = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	BYTE bGender = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	output_debug("MSG_Monster_Appearance MsgSize %d MsgBodySize %d\n", iLen, iLen-12);
	if(id == SELF.GetID())
	{
		//output_debug("MSG_Monster_Appearance MsgSize Self Size %d !!! \n", iLen);
	}else if( g_pGameData->FindSimpleCharacter(id) != NULL){
		//output_debug("MSG_Monster_Appearance MsgSize Monster&NPC Size %d !!! \n", iLen);
	}

	if(id == SELF.GetID())
	{
		__int64 looks = Conv_INT64(msg + 12);
		SELF.SetLooks(looks);
		return;
	}

	//12+12 24 NPC 状态刷新
	if(iLen == 24){
		CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(id);
		if(pChar)
		{
			pChar->SetDir(bDir);
			if(pChar->IsLeopard())
			{
				int iTempState =  *((char *)(msg + 16));
				if(iTempState==1)
				{
					pChar->PushSNextAction(ACTION_ATTACK6,bDir);//过渡
					pChar->PushSNextAction(ACTION_ATTACK5,bDir);//打哈欠
					pChar->PushSNextAction(ACTION_ATTACK4,bDir);//趴下
				}
				else
				{
					if(pChar->GetRaceNo() != 301 && pChar->GetRaceNo() != 302)	//凤凰起来不用打哈气了
					{
						pChar->PushSNextAction(ACTION_ATTACK5,bDir);//打哈欠
					}

					if(pChar->GetAction().wAction != ACTION_ADDACTION2)
					{
						pChar->PushSNextAction(ACTION_ADDACTION2,bDir);//过渡到站立
					}
				}
			}else{
				if(pChar->GetRaceNo() == 115)//大刀
					pChar->PushSNextAction(ACTION_STAND,bDir);
				else
				{
					SNextAction* pNextAction = pChar->PushSNextAction(ACTION_APPEAR,bDir);
					if(pNextAction)
					{
						pNextAction->iData = 20;
						//pNextAction->cDir = cDir;
					}
				}
			}
		}
	}else if(iLen > 24)
	{		
		//出现新的NPC
		CSimpleCharacterNode* pChar = g_pGameData->AddSimpleCharacter(x,y,id);
		if(pChar == NULL)
			return;

		pChar->FromBuffer(msg,iLen);
		pChar->SetDead(false);

		BYTE byMonsterType = msg[11];
		if (byMonsterType & 0x10)//可挖的怪物,死亡后放一个特效
		{
			pChar->SetCanDig(true);
		}

		////五行玄关怪物身上特效
		if (id == g_OtherData.GetWuXingFlag().dwFlagInMonstID)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_WUXING_FALG_MONSTER_STATE,0,SELF.GetID(),id);
		}

		int iMonster = pChar->GetRaceNo();
		__int64 looks = Conv_DWORD(msg+12);

		// 圣诞树(17)的音乐
		if((looks & 0x000000FF) == 0x00000032 && (looks & 0x00FF0000) == 0x00110000)
		{
			DWORD dwTemp = Conv_DWORD(msg + 6);
			int iSelfX,iSelfY,iOx,iOy;

			SELF.SetReserveData(7,dwTemp);
			SELF.GetXY(iSelfX,iSelfY);
			SELF.GetOffset(iOx,iOy);
			TRY_BEGIN
				g_pAudio->PlayMidi(1,true,0,0,(iSelfX - x) * 64 + iOx,(iSelfY - y) * 32 + iOy);
			TRY_END_DO(g_pAudio->EnableMidi(false);)
		}

		//怪物出现时音乐
		UINT lSoundID = (UINT)(((looks&0xFFFF0000)>>13) + 16);
		int iSelfX,iSelfY;  
		
		if(GetTickCount() - m_dwJumpTime>4000 && pChar->IsMonster())
		{
			SELF.GetXY(iSelfX,iSelfY);
			g_pAudio->PlayEx(EAT_MONSTER,lSoundID, g_pAudio->GetRand()++,iSelfX,x,iSelfY,y); 
		}

		if(pChar->GetMagicState() & MS_ANIMAPPEAR)
		{
			pChar->InitAction(ACTION_APPEAR);

			switch(iMonster)
			{
			case 110:       //神兽
				g_pMagicCtrl->CreateMagic(MAGICID_CALL_JOSS,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				pChar->InitAction(ACTION_STAND);
				break;
			case  223:  //强化骷髅
			case  207:
				g_pMagicCtrl->CreateMagic(MAGICID_ENCALL_MONSTER,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				pChar->InitAction(ACTION_STAND);
				break;
				//case 113://心魔神兽
				//	g_pMagicCtrl->CreateMagic(MAGICID_DEMON_CALL_JOSS,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				//	break;
				//case 117://心魔强化骷髅
				//	g_pMagicCtrl->CreateMagic(MAGICID_DEMON_ENCALL_MONSTER,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				//	break;
			case 221://暗域之王出现效果
				if(pChar->GetDir() < 4)
					g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_APPEAR1,0,pChar->GetID(),0,2*pChar->GetDir());
				else
					g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_APPEAR2,0,pChar->GetID(),0,2*pChar->GetDir());

				break;
			case 242://寒荒海兽出现效果
				g_pMagicCtrl->CreateMagic(MAGICID_MON_OCEAN_APPEAR,0,pChar->GetID(),0);
				break;
			case 56://门
			case 57:
				{
					pChar->SetDir(0);//关门
					int iSelfX,iSelfY;
					SELF.GetXY(iSelfX,iSelfY);
					g_pAudio->PlayEx(EAT_OTHER,903,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y,false);
				}
				break;
			}

			pChar->RemoveMagicState(MS_ANIMAPPEAR);
		}


		if (pChar->IsMonster())//防止同编号npc出现怪物的特效
		{
			switch( iMonster)
			{
			case 480://豹子出现效果
				g_pMagicCtrl->CreateMagic(MAGICID_LEOPARD_EFFECT,0,pChar->GetID(),0,0);
				break;
			case 451://混沌女王出现效果
				{
					g_pMagicCtrl->CreateMagic(MAGICID_QUEEN_EFFECT,0,pChar->GetID(),0,0);

					if (pChar->GetID() == g_OtherData.GetLockHunDunNvWangLockID())//被锁住的混沌女王
					{
						DWORD dwLock = g_OtherData.GetHunDunNvWangLock();

						Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_JIN,-1,pChar->GetID());
						if ((dwLock & 0x00000001) == 0)//金
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_JIN,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_MU,-1,pChar->GetID());
						if ((dwLock & 0x00000002) == 0)//木
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_MU,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_TU,-1,pChar->GetID());
						if ((dwLock & 0x00000004) == 0)//土
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_TU,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_SHUI,-1,pChar->GetID());
						if ((dwLock & 0x00000008) == 0)//水
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_SHUI,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_HUO,-1,pChar->GetID());
						if ((dwLock & 0x00000010) == 0)//火
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_HUO,0,0,pChar->GetID());
						}
					}
				}
				break;

			case 321://护塔机关2
				g_pMagicCtrl->CreateMagic(MAGICID_ELECTRIC_1,0,pChar->GetID(),0,0);
				break;
			case 322://护塔机关3	
				g_pMagicCtrl->CreateMagic(MAGICID_ELECTRIC_2,0,pChar->GetID(),0,0);
				break;
			case 276://箭塔
				g_pMagicCtrl->CreateMagic(MAGICID_TUTENG_EFFECT_TOWER,0,pChar->GetID(),0,0);
				break;

			case 260://反射图腾
			case 261://隐身图腾
			case 262://重力图腾
			case 263://生命图腾
			case 264://诅咒图腾
			case 265://巫毒图腾
			case 266://巫术图腾
			case 267://魔力图腾
			case 268://仇恨图腾
			case 269://箭塔图腾
			case 270://陷阱图腾
				g_pMagicCtrl->CreateMagic(MAGICID_TUTENG_EFFECT,0,pChar->GetID(),0,iMonster - 260);//用方向表示是什么图腾		
				break;

			case  69: pChar->SetShowHP( 60); break;	// 尸霸
			case  77: pChar->SetShowHP(120); break;	// 机关巨兽
			case  88: pChar->SetShowHP( 80); break;	// 逆魔		
			case 192:pChar->SetShowHP(150);break; 
			case 235://修罗王神转向
			case 236:// 禁地魔王
				pChar->SetShowHP(150); 
				pChar->SetDir(4);
				break;	
			case 198: // 通天教主
				pChar->SetShowHP( 80);
				g_pMagicCtrl->CreateMagic(MAGICID_BIANSHEN_EFFECT,0,id);
				break;	
			case 199: pChar->SetShowHP(100); break;	// 通天教主乘骑
			case 209: //迷仙阵石碑怪物
				if(strnicmp(g_pGameMap->GetMapName(),"MAZE",4) == 0)
				{
					//g_pMagicCtrl->CreateMagic(MGGICID_MAZE_SHIBEI,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(51,37));//固定坐标
					g_pMagicCtrl->CreateMagic(MGGICID_MAZE_SHIBEI,0,pChar->GetID(),0,0);//固定坐标
					g_pMagicCtrl->FinishMagicByID(MAGICID_MAZE_SHIBEI_DEADNODELAY);
				}
				break;
			case 218: pChar->SetShowHP(80); break; // 投石巨魔
			case 220: pChar->SetShowHP(80); break; // 幽影武士
			case 221: pChar->SetShowHP(100); break; // 暗域之王
			case 228: pChar->SetShowHP(80);  break; // 蛇妖王
			case 229: pChar->SetShowHP(80);  break; // 逆魔之王
			case 230: pChar->SetShowHP(60);  break; // 尸霸将军
			case 242: pChar->SetShowHP(150); break; // 寒荒海兽
			case 249: pChar->SetShowHP(100); break; // 无相天魔
			case 355:
			case 356:
				{
					_TimeOut timeout("",GetTickCount(),5 * 1000,40001,1,5,pChar->GetID());
					g_OtherData.AddTimeOut(timeout);
				}
				break;
			case 421://法宝骑兵
				{
					pChar->SetFaBaoType(1,9);
				}
				break;
			case 434://法宝骑兵
				{
					pChar->SetFaBaoType(6,9);
				}
				break;
			case 484://木灵虎
				g_pMagicCtrl->CreateMagic(MAGICID_Tiger_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 481://水麒麟
				g_pMagicCtrl->CreateMagic(MAGICID_QILIN_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 483://土岩狮
				g_pMagicCtrl->CreateMagic(MAGICID_TULION_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 482://凤凰
				g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			}
		}
		g_AIPromptMgr.DoCharacterAppear(pChar);

		g_AutoKillMonsterMgr.AddMonster(pChar->GetID());

		//记录丛林豹的ID
		if(pChar->GetRaceType() == CHARACTER_MONSTER && !pChar->IsDead() && pChar->IsPet()) //活的宠物
		{
			//丛林豹
			if (pChar->IsYourPet(SELF.GetName()))
			{
				switch(iMonster)
				{
				case 2:
				case 20:
				case 38:
				case 300:
					{
						SELF.SetMyPetID(pChar->GetID());
						SELF.SetMyOldPet(true);
					}
					break;
				case 39:
				case 40:
				case 41:
				case 166:
				case 167:
				case 168:
				case 185:
				case 186:
				case 187:
				case 301:
				case 302:
				case 303:
					{
						if(iMonster == 301 || iMonster == 302)
						{
							pChar->InitAction(ACTION_ADDACTION2);
						}

						SELF.SetMyPetID(pChar->GetID());
						SELF.SetMyOldPet(false);
					}
					break;
				default:
					break;

				}
			}
		}

	}

	//无双处理
	return;
	if(iLen == 28) //人物转方向
	{
		//DWORD id = Conv_DWORD(msg);

		if(id == SELF.GetID())
		{
			SELF.SetDir(bDir);
			//SELF.SetAttackSpeed(Conv_WORD(msg+16));
		}
		else
		{
			CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(id);
			if(pChar)
			{
				pChar->SetDir(msg[10]);
				pChar->SetAttackSpeed(Conv_WORD(msg+16));

				BYTE cDir = msg[10];

				if(pChar->IsLeopard())
				{
					int iTempState =  *((char *)(msg + 16));
					if(iTempState==1)
					{
						pChar->PushSNextAction(ACTION_ATTACK6,cDir);//过渡
						pChar->PushSNextAction(ACTION_ATTACK5,cDir);//打哈欠
						pChar->PushSNextAction(ACTION_ATTACK4,cDir);//趴下
					}
					else
					{
						if(pChar->GetRaceNo() != 301 && pChar->GetRaceNo() != 302)	//凤凰起来不用打哈气了
						{
							pChar->PushSNextAction(ACTION_ATTACK5,cDir);//打哈欠
						}

						if(pChar->GetAction().wAction != ACTION_ADDACTION2)
						{
							pChar->PushSNextAction(ACTION_ADDACTION2,cDir);//过渡到站立
						}
					}
				}
				else
				{
					if(pChar->GetRaceNo() == 115)//大刀
						pChar->PushSNextAction(ACTION_STAND,cDir);
					else
					{
						SNextAction* pNextAction = pChar->PushSNextAction(ACTION_APPEAR,cDir);
						if(pNextAction)
						{
							pNextAction->iData = 20;
							//pNextAction->cDir = cDir;
						}
					}
				}

			}
		}
		return;
	}
	else if(iLen > 28)
	{
		DWORD id = Conv_DWORD(msg);
		int   x  = Conv_WORD(msg+6);	//7-8	nParam1
		int   y  = Conv_WORD(msg+8);	//9-10	nParam2

		if(id == SELF.GetID())
		{
			SELF.SetLooks(Conv_INT64(msg+12));
			return;
		}

		CSimpleCharacterNode* pChar = g_pGameData->AddSimpleCharacter(x,y,id);
		if(pChar == NULL)
			return;

		pChar->FromBuffer(msg,iLen);
		pChar->SetDead(false);

		BYTE byMonsterType = msg[11];		//wParam
		if (byMonsterType & 0x10)//可挖的怪物,死亡后放一个特效
		{
			pChar->SetCanDig(true);
		}

		////五行玄关怪物身上特效
		if (id == g_OtherData.GetWuXingFlag().dwFlagInMonstID)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_WUXING_FALG_MONSTER_STATE,0,SELF.GetID(),id);
		}

		int iMonster = pChar->GetRaceNo();

		__int64 looks = Conv_DWORD(msg+12);

		// 圣诞树(17)的音乐
		if((looks & 0x000000FF) == 0x00000032 && (looks & 0x00FF0000) == 0x00110000)
		{
			DWORD dwTemp = Conv_DWORD(msg + 6);
			int iSelfX,iSelfY,iOx,iOy;

			SELF.SetReserveData(7,dwTemp);
			SELF.GetXY(iSelfX,iSelfY);
			SELF.GetOffset(iOx,iOy);
			TRY_BEGIN
				g_pAudio->PlayMidi(1,true,0,0,(iSelfX - x) * 64 + iOx,(iSelfY - y) * 32 + iOy);
			TRY_END_DO(g_pAudio->EnableMidi(false);)
		}

		//怪物出现时音乐
		UINT lSoundID = (UINT)(((looks&0xFFFF0000)>>13) + 16);
		int iSelfX,iSelfY;  
		
		if(GetTickCount() - m_dwJumpTime>4000 && pChar->IsMonster())
		{
			SELF.GetXY(iSelfX,iSelfY);
			g_pAudio->PlayEx(EAT_MONSTER,lSoundID, g_pAudio->GetRand()++,iSelfX,x,iSelfY,y); 
		}

		if(pChar->GetMagicState() & MS_ANIMAPPEAR)
		{
			pChar->InitAction(ACTION_APPEAR);

			switch(iMonster)
			{
			case 110:       //神兽
				g_pMagicCtrl->CreateMagic(MAGICID_CALL_JOSS,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				pChar->InitAction(ACTION_STAND);
				break;
			case  223:  //强化骷髅
			case  207:
				g_pMagicCtrl->CreateMagic(MAGICID_ENCALL_MONSTER,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
				pChar->InitAction(ACTION_STAND);
				break;
			//case 113://心魔神兽
			//	g_pMagicCtrl->CreateMagic(MAGICID_DEMON_CALL_JOSS,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
			//	break;
			//case 117://心魔强化骷髅
			//	g_pMagicCtrl->CreateMagic(MAGICID_DEMON_ENCALL_MONSTER,EMP_MAGIC_SERVER_ADVISE,pChar->GetID(),0);
			//	break;
			case 221://暗域之王出现效果
				if(pChar->GetDir() < 4)
					g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_APPEAR1,0,pChar->GetID(),0,2*pChar->GetDir());
				else
					g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_APPEAR2,0,pChar->GetID(),0,2*pChar->GetDir());

				break;
			case 242://寒荒海兽出现效果
				g_pMagicCtrl->CreateMagic(MAGICID_MON_OCEAN_APPEAR,0,pChar->GetID(),0);
				break;
			case 56://门
			case 57:
				{
					pChar->SetDir(0);//关门
					int iSelfX,iSelfY;
					SELF.GetXY(iSelfX,iSelfY);
					g_pAudio->PlayEx(EAT_OTHER,903,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y,false);
				}
				break;
			}
			
			pChar->RemoveMagicState(MS_ANIMAPPEAR);
		}


		if (pChar->IsMonster())//防止同编号npc出现怪物的特效
		{
			switch( iMonster)
			{
			case 480://豹子出现效果
				g_pMagicCtrl->CreateMagic(MAGICID_LEOPARD_EFFECT,0,pChar->GetID(),0,0);
				break;
			case 451://混沌女王出现效果
				{
					g_pMagicCtrl->CreateMagic(MAGICID_QUEEN_EFFECT,0,pChar->GetID(),0,0);

					if (pChar->GetID() == g_OtherData.GetLockHunDunNvWangLockID())//被锁住的混沌女王
					{
						DWORD dwLock = g_OtherData.GetHunDunNvWangLock();

						Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_JIN,-1,pChar->GetID());
						if ((dwLock & 0x00000001) == 0)//金
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_JIN,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_MU,-1,pChar->GetID());
						if ((dwLock & 0x00000002) == 0)//木
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_MU,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_TU,-1,pChar->GetID());
						if ((dwLock & 0x00000004) == 0)//土
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_TU,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_SHUI,-1,pChar->GetID());
						if ((dwLock & 0x00000008) == 0)//水
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_SHUI,0,0,pChar->GetID());
						}

						ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_HUO,-1,pChar->GetID());
						if ((dwLock & 0x00000010) == 0)//火
						{
							if (ms)
							{
								g_pMagicCtrl->CreateMagic(ms);
								g_pMagicCtrl->FinishMagic(ms);
							}
						}
						else  if (!ms)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_HUO,0,0,pChar->GetID());
						}
					}
				}
				break;

			case 321://护塔机关2
				g_pMagicCtrl->CreateMagic(MAGICID_ELECTRIC_1,0,pChar->GetID(),0,0);
				break;
			case 322://护塔机关3	
				g_pMagicCtrl->CreateMagic(MAGICID_ELECTRIC_2,0,pChar->GetID(),0,0);
				break;
			case 276://箭塔
				g_pMagicCtrl->CreateMagic(MAGICID_TUTENG_EFFECT_TOWER,0,pChar->GetID(),0,0);
				break;

			case 260://反射图腾
			case 261://隐身图腾
			case 262://重力图腾
			case 263://生命图腾
			case 264://诅咒图腾
			case 265://巫毒图腾
			case 266://巫术图腾
			case 267://魔力图腾
			case 268://仇恨图腾
			case 269://箭塔图腾
			case 270://陷阱图腾
				g_pMagicCtrl->CreateMagic(MAGICID_TUTENG_EFFECT,0,pChar->GetID(),0,iMonster - 260);//用方向表示是什么图腾		
				break;

			case  69: pChar->SetShowHP( 60); break;	// 尸霸
			case  77: pChar->SetShowHP(120); break;	// 机关巨兽
			case  88: pChar->SetShowHP( 80); break;	// 逆魔		
			case 192:pChar->SetShowHP(150);break; 
			case 235://修罗王神转向
			case 236:// 禁地魔王
				pChar->SetShowHP(150); 
				pChar->SetDir(4);
				break;	
			case 198: // 通天教主
				pChar->SetShowHP( 80);
				g_pMagicCtrl->CreateMagic(MAGICID_BIANSHEN_EFFECT,0,id);
				break;	
			case 199: pChar->SetShowHP(100); break;	// 通天教主乘骑
			case 209: //迷仙阵石碑怪物
				if(strnicmp(g_pGameMap->GetMapName(),"MAZE",4) == 0)
				{
					//g_pMagicCtrl->CreateMagic(MGGICID_MAZE_SHIBEI,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(51,37));//固定坐标
					g_pMagicCtrl->CreateMagic(MGGICID_MAZE_SHIBEI,0,pChar->GetID(),0,0);//固定坐标
					g_pMagicCtrl->FinishMagicByID(MAGICID_MAZE_SHIBEI_DEADNODELAY);
				}
				break;
			case 218: pChar->SetShowHP(80); break; // 投石巨魔
			case 220: pChar->SetShowHP(80); break; // 幽影武士
			case 221: pChar->SetShowHP(100); break; // 暗域之王
			case 228: pChar->SetShowHP(80);  break; // 蛇妖王
			case 229: pChar->SetShowHP(80);  break; // 逆魔之王
			case 230: pChar->SetShowHP(60);  break; // 尸霸将军
			case 242: pChar->SetShowHP(150); break; // 寒荒海兽
			case 249: pChar->SetShowHP(100); break; // 无相天魔
			case 355:
			case 356:
				{
					_TimeOut timeout("",GetTickCount(),5 * 1000,40001,1,5,pChar->GetID());
					g_OtherData.AddTimeOut(timeout);
				}
				break;
			case 421://法宝骑兵
				{
					pChar->SetFaBaoType(1,9);
				}
				break;
			case 434://法宝骑兵
				{
					pChar->SetFaBaoType(6,9);
				}
				break;
			case 484://木灵虎
				g_pMagicCtrl->CreateMagic(MAGICID_Tiger_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 481://水麒麟
				g_pMagicCtrl->CreateMagic(MAGICID_QILIN_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 483://土岩狮
				g_pMagicCtrl->CreateMagic(MAGICID_TULION_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			case 482://凤凰
				g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_SELF_EFFECT,0,pChar->GetID(),0,0,-1,0,0,0,false);
				break;
			}
		}
		g_AIPromptMgr.DoCharacterAppear(pChar);

		g_AutoKillMonsterMgr.AddMonster(pChar->GetID());

		//记录丛林豹的ID
		if(pChar->GetRaceType() == CHARACTER_MONSTER && !pChar->IsDead() && pChar->IsPet()) //活的宠物
		{
			//丛林豹
			if (pChar->IsYourPet(SELF.GetName()))
			{
				switch(iMonster)
				{
				case 2:
				case 20:
				case 38:
				case 300:
					{
						SELF.SetMyPetID(pChar->GetID());
						SELF.SetMyOldPet(true);
					}
					break;
				case 39:
				case 40:
				case 41:
				case 166:
				case 167:
				case 168:
				case 185:
				case 186:
				case 187:
				case 301:
				case 302:
				case 303:
					{
						if(iMonster == 301 || iMonster == 302)
						{
							pChar->InitAction(ACTION_ADDACTION2);
						}

						SELF.SetMyPetID(pChar->GetID());
						SELF.SetMyOldPet(false);
					}
					break;
				default:
					break;

				}
			}
		}
	}
}

// 魔法师的兽化飞行，1.80道士的遁地，战士的移形换位，骑战的翱风斩，纵雷诀
void  CGameControl::MSG_Monster_Magic_Fly(const char * msg,int iLen)
{
	if(iLen < 30 )
		return;

	DWORD uID = Conv_DWORD(msg);
	WORD wx = Conv_WORD(msg + 6);
	WORD wy = Conv_WORD(msg + 8);
	BYTE cDir = msg[10];
	char sex = msg[11];
	char cGrid = msg[32];

	int iMagicAction = ACTION_MAGIC;

	WORD wMagicID = Conv_WORD(msg+34);

	bool bHorseFghSkill = false;
	if(wMagicID == MAGICID_AOFENGCUT_ATTACK || wMagicID == MAGICID_ZHONGLEI_ATTACK)
	{										
		if(wMagicID == MAGICID_ZHONGLEI_ATTACK)			
			iMagicAction = ACTION_LFMAGICRAB;
		else if(wMagicID == MAGICID_AOFENGCUT_ATTACK)	
			iMagicAction = ACTION_LFMAGICFGH;

		bHorseFghSkill = true;
		if(uID == SELF.GetID())
			g_AIMgr.UpdateRideFightTime();
	}
	else if(wMagicID != MAGICID_ESCAPE_QUICK && wMagicID != MAGICID_SHIFT_TRANSPORT)
	{
		wMagicID = MAGICID_ENCHANTER_WILD;
	}

	int ox = Conv_WORD(msg + 28);
	int oy = Conv_WORD(msg + 30);
	int iDir = GetDir8(wx,wy,ox,oy);

	//战士骑战技能翱风斩在方向0和4时重新计算方向
	if(wMagicID == MAGICID_ZHONGLEI_ATTACK &&(ox != wx || oy != wy))
	{
		if(iDir == 0 || iDir == 4)
		{
			if(abs(wx-ox) > 1)
			{		
				 if(wx>ox && wy < oy) iDir = 1;
				 else if(wx > ox && wy > oy) iDir = 3;
				 else if(wx < ox && wy > oy) iDir = 5;
				 else if(wx < ox && wy < oy) iDir = 7;
			}
		}
	}

	CSimpleCharacterNode* pNode = MapArray.FindSimpleCharacter(wx,wy);
	if(uID == SELF.GetID())		// 主角自己的变身飞行
	{
		if (SELF.IsOnPhenix() && wMagicID == MAGICID_AOFENGCUT_ATTACK)
		{
			wMagicID = MAGICID_AOFENGCUT_ATTACK_ONPHENIX;
		}

		if(SELF.GetLevel() >= 130 && wMagicID == MAGICID_SHIFT_TRANSPORT)//移形换影70级之后每升5级，移形换影的使用CD减少5秒，直至最终无CD。并且在无CD时，表现去除最后还原的特效。
		{
			wMagicID = MAGICID_QUICK_SHIFT_TRANSPORT;
		}

		g_AutoPickMgr.BatFlyFinish(wx,wy);
		SNextAction* pNext = SELF.PushSNextAction(iMagicAction,iDir,wx,wy);
		if(pNext)
		{
			pNext->iData1 = wMagicID;
			pNext->uData1 = EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE;
			pNext->uData2 = uID;
			pNext->uData3 = MAKELONG(wx,wy);

			pNext->uFlag |= CREATE_MAGIC;

			if (wMagicID == MAGICID_AOFENGCUT_ATTACK || wMagicID == MAGICID_AOFENGCUT_ATTACK_ONPHENIX)
			{
				pNext->uData = cDir;
			}
		}

		SELF.SetRealXY(wx,wy); 

	}
	else// 其他玩家的变身飞行
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(uID);

		if(pChar)											// 玩家本来就在屏幕内
		{
			pChar->SetSex(sex);
			SNextAction* pNext = pChar->PushSNextAction(iMagicAction,iDir,wx,wy);
			if(pNext)
			{
				pNext->iData1 = wMagicID;
				pNext->uData1 = EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE;
				pNext->uData2 = uID;
				pNext->uData3 = MAKELONG(wx,wy);

				pNext->uFlag |= CREATE_MAGIC;
			}

			pChar->SetOffset(0,0); 
			pChar->SetDrawOffset(0,0);
			pChar->SetRealXY(wx,wy); 
		}
		else// 玩家从屏幕外移动到屏幕内
		{
			if(!bHorseFghSkill)
				pChar = g_pGameData->AddSimpleCharacter(wx,wy,uID);
			else
				pChar = g_pGameData->AddSimpleCharacter(ox,oy,uID);
			if(!pChar)
				return;

			pChar->SetSex(sex);
			pChar->SetLooks(Conv_INT64(msg + 12));
			pChar->SetAttackSpeed(msg[20]);
			pChar->SetStatus(Conv_WORD(msg+22));
			pChar->SetHP(Conv_WORD(msg + 24));
			pChar->SetHPMax(100);

			if(msg[27]&0x01)
				pChar->SetReserveData(pubHasWings,1);

			pChar->SetRealXY(wx,wy);
			pChar->SetXY(ox,oy);
			pChar->InitAction(ACTION_STAND);
			pChar->SetNameColor(msg[33]);

			//名字 + '\0' + 第二元神等级(1字节) + 骑跑几格的豹子(2位) + 盾牌类型(2位) + 行会旗帜(4位) + 扩展10字节(1978版本);
			std::string strTemp2;
			strTemp2.append(msg + 36,iLen - 36);
			std::string strTemp3;
			int first =strTemp2.find('\0');
			if( first != string::npos  )
			{
				pChar->SetName(strTemp2.substr(0,first).c_str());	

				const BYTE * tempbuf = (const BYTE *)(strTemp2.c_str() + first + 1);
				//第二元神等级(
				BYTE byTemp = tempbuf[0];
				//pChar->SetSecondYuanShen(byTemp);
				//骑跑几格的豹子
				byTemp = tempbuf[1] & 0x3;
				if(byTemp == 1)	//骑战且豹子跑两格
				{
					pChar->SetFightOnLeopard(true);
					pChar->SetRunStepOnLeopard(2);
				}
				else if(byTemp == 3)	//骑战且豹子跑三格
				{
					pChar->SetFightOnLeopard(true);
					pChar->SetRunStepOnLeopard(3);					    
				}
				else
				{
					pChar->SetFightOnLeopard(false);
					pChar->SetRunStepOnLeopard(3);					  
				}
				//第2字节低高6bit表示带了什么盾牌,发过来的是盾牌的shape: 战士:1,2,3,4;法师:5,6,7,8;道士:9,10,11,12,虎王盾:13,14,15,16
				byTemp = (tempbuf[1] >> 2) & 0x3F;
				if (byTemp > 0)
				{
					pChar->SetShield(byTemp);					
				}
				else
				{
					pChar->SetShield(0);
				}
				//第三字节第一个bit表示是否托管,非零表示已经托管
// 				BYTE byTrusteeshipFlag = tempbuf[2] & 0x01;
// 				if (byTrusteeshipFlag > 0)
// 				{
// 					if (g_TrusteeshipData.FindMemberPosByID(pChar->GetID()) >= 0)
// 					{
// 						pChar->SetTrusteeshipFlag(1);
// 					}
// 					else
// 					{
// 						pChar->SetTrusteeshipFlag(2);
// 					}
// 				}

				pChar->SetVipTradeLevel(tempbuf[3]);

				//
				//这里是传世飞升相关数据,传奇世界已经不要了
				//
				pChar->SetSanWeiFireLevel(tempbuf[4]);

				//神翼等级 + 1,0表示没有神翼
				if (tempbuf[8] == 0)
				{
					pChar->SetCharHolyWingLevel(-1);
					//强化等级
					pChar->SetCharHolyWingStrongLevel(-1);
				}
				else
				{
					pChar->SetCharHolyWingLevel((BYTE)tempbuf[8] - 1);
					//强化等级
					pChar->SetCharHolyWingStrongLevel((BYTE)tempbuf[9]);

					//夺宝1.28内容
					//BYTE byWingColor = tempbuf[8];
					//pChar->SetCharHolyWingStrongLevel( (int)(byWingColor & 0xF) );
					//pChar->SetCharHolyWingLevel( (int)((byWingColor >> 4) & 0xF) );

				}

				//生存令牌数量
				pChar->SetLiveTokenNumber((BYTE)tempbuf[10]);
				//buffer圈圈
				pChar->SetTypeBufferQQ((BYTE)tempbuf[11]);
				//法宝类型,等级
				pChar->SetFaBaoType((BYTE)tempbuf[12],(BYTE)tempbuf[13]);

				if (iLen > 41 + first)
				{
					BYTE daodun = (BYTE)tempbuf[14];
					if (daodun == 0)
					{
						pChar->Set8DunState(false);
						pChar->SetDaoZunJiangLinState(false);
					}
					else if (daodun == 1)
					{
						pChar->Set8DunState(true);
						pChar->SetDaoZunJiangLinState(false);
					}
					else if (daodun == 2)
					{
						pChar->Set8DunState(false);
						pChar->SetDaoZunJiangLinState(true);
					}

				//	BYTE leftFabao = tempbuf[16];
				//	pChar->SetLeftFaBaoType(leftFabao);
				//	BYTE rightFabao = tempbuf[17];
				//	pChar->SetRightFaBaoType(rightFabao);

				//	pChar->StartFaZhen(tempbuf[15],tempbuf[16],tempbuf[17]);

				//	pChar->Set12GongTitle((BYTE)msg[18]);
				}


				if(pChar->IsHuman())
				{
					//第5位低四位表示神魔属性
					//[协议要修改]
					//pChar->SetFlyType(tempbuf[4] & 0x0F);
					//pChar->SetFlyLevel(tempbuf[5]);
					pChar->SetGreateMagicState(*((WORD*)(tempbuf + 6)));
				}
			}

			if(pChar && pChar->GetLevel() > 130 && wMagicID == MAGICID_SHIFT_TRANSPORT)//移形换影70级之后每升5级，移形换影的使用CD减少5秒，直至最终无CD。并且在无CD时，表现去除最后还原的特效。
			{
				wMagicID = MAGICID_QUICK_SHIFT_TRANSPORT;
			}

			if (pChar && pChar->IsOnPhenix() && wMagicID == MAGICID_AOFENGCUT_ATTACK)
			{
				wMagicID = MAGICID_AOFENGCUT_ATTACK_ONPHENIX;
			}

			if((wMagicID == MAGICID_AOFENGCUT_ATTACK || wMagicID == MAGICID_AOFENGCUT_ATTACK_ONPHENIX) && pNode)
				g_pMagicCtrl->CreateMagic( wMagicID,EMP_MAGIC_SERVER_ADVISE,uID,pNode->GetID(),iDir*2);
			else
				g_pMagicCtrl->CreateMagic( wMagicID,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,uID,MAKELONG(wx,wy),iDir*2);
		}

		if(pChar && bHorseFghSkill)
		{
			int iDis = (wx-ox)*(wx-ox) + (wy-oy)*(wy-oy);
			SNextAction* pNext = pChar->PushSNextAction(iMagicAction,iDir,wx,wy);
			if (pNext)
			{
				pNext->iData = iDis;
				if(wMagicID == MAGICID_AOFENGCUT_ATTACK || wMagicID == MAGICID_AOFENGCUT_ATTACK_ONPHENIX)
					pNext->uData = cDir;
			}
		}
	}
}

void CGameControl::MSG_Monster_Walk(const char * msg,int iLen)
{
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	DWORD id = lpPacketMsg->stDefMsg.nRecog;
	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	BYTE bDir = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	//BYTE bGender = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	TCHARDESC	stCharDesc;
	memcpy(&stCharDesc, lpPacketMsg->szEncodeData, sizeof(TCHARDESC));

	//DWORD id = Conv_DWORD(msg);
	//WORD x = Conv_WORD(msg + 6);
	//WORD y = Conv_WORD(msg + 8);

	CCharacterAttr *pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar )
		return;
	
	//pChar->SetAttackSpeed(Conv_WORD(msg+20));
	pChar->SetStatus(stCharDesc.nStatus);
	pChar->SetRealXY(x,y);

	SNextAction * pNext = pChar->PushSNextAction(ACTION_WALK,bDir,x,y);
	if(pNext)
	{
		pNext->iLooks = Conv_INT64(msg+12);
		pNext->uFlag |= SERVER_ACTION;
	}
}

void CGameControl::MSG_Monster_Run(const char * msg,int iLen)
{
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	DWORD id = lpPacketMsg->stDefMsg.nRecog;
	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	BYTE bDir = LOBYTE(lpPacketMsg->stDefMsg.wSeries);

	TCHARDESC stCharDesc;
	memcpy(&stCharDesc, lpPacketMsg->szEncodeData, sizeof(TCHARDESC));

	//DWORD id = Conv_DWORD(msg);
	//WORD x = Conv_WORD(msg + 6);
	//WORD y = Conv_WORD(msg + 8);

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if(!pChar)
		return;

	pChar->SetStatus(stCharDesc.nStatus);
	pChar->SetRealXY(x,y);

	SNextAction* pNext = pChar->PushSNextAction(ACTION_RUN,bDir,x,y);
	if(pNext)
	{
		pNext->iLooks = Conv_INT64(msg + 12);
		pNext->uFlag |= SERVER_ACTION;
	}
} 

void CGameControl::MSG_Monster_Attack1(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	BYTE cActionType = msg[11];

	CSimpleCharacter * pChar = g_pGameData->FindCharacterByID(id);
	if(!pChar)
		return;

	//g_NPC只改变方向
	if(pChar->IsNpc())
	{
		//SAction& saction = pChar->GetAction();
		//saction.SetNpcRandStand(pChar->GetRaceNo());
		pChar->SetDir(dir);
		return;
	}

	SNextAction * pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if (!pNext)
		return;

	pNext->uFlag |= SERVER_ACTION;


	if(pChar->GetRaceNo() == 249)//无相天魔的攻击方式
	{
		switch(msg[11])
		{
		case 3:
		case 4: pNext->wAction = ACTION_ATTACK2;break;
		case 5:	pNext->wAction = ACTION_ATTACK4;break;
		}
	}
	else if (pChar->GetRaceNo() == 389)
	{
		switch(msg[11])
		{
		case 1:	pNext->wAction = ACTION_ATTACK2; break;
		case 2:	pNext->wAction = ACTION_ATTACK3; break;//(豹子)扑食物  (卫士 弓箭守卫  站立)
		case 3: pNext->wAction = ACTION_ATTACK4; break;//    (豹子)趴下    (卫士 弓箭守卫 下跪不动)
		case 4: pNext->wAction = ACTION_ATTACK5; break;//    (豹子)打哈欠
		case 5: pNext->wAction = ACTION_ATTACK6; break;//    (豹子)过渡站立
		case 6: pNext->wAction = ACTION_ADDACTION1; break;// (豹子)喂食2
		case 7: pNext->wAction = ACTION_ADDACTION2; break;// 单个(豹子)吼叫 产生喷气
		}
	}
	else
	{
		switch(msg[11])
		{
		case 1:	pNext->wAction = ACTION_ATTACK2; break;
		case 2:	pNext->wAction = ACTION_ATTACK3; break;//(豹子)扑食物  (卫士 弓箭守卫  站立)
		case 3: pNext->wAction = ACTION_ATTACK4; break;//    (豹子)趴下    (卫士 弓箭守卫 下跪不动)
		case 4: pNext->wAction = ACTION_ATTACK5; break;//    (豹子)打哈欠
		case 5: pNext->wAction = ACTION_ATTACK6; break;//    (豹子)过渡站立
		case 6: pNext->wAction = ACTION_ADDACTION1; break;// (豹子)喂食2
		case 7: pNext->wAction = ACTION_ATTACK3; break;// 单个(豹子)吼叫 产生喷气
		}
	}
	
	//动作伴随位移
	if(pChar->GetRaceNo() == 382 && pNext->wAction == ACTION_ATTACK2)
	{
		pNext->uFlag |= MOVE_ACTION;
		pChar->PushSNextAction(ACTION_ATTACK3,dir,x,y);
	}
	else if (pChar->GetRaceNo() == 385 && pNext->wAction == ACTION_ATTACK2)
	{
		pNext->uFlag |= MOVE_ACTION;
	}
	else if (pChar->GetRaceNo() == 387 && pNext->wAction == ACTION_ATTACK2)
	{
		pNext->uFlag |= MOVE_ACTION;
		pChar->PushSNextAction(ACTION_ATTACK3,dir,x,y);
	}

	if (pNext->uFlag & MOVE_ACTION)
	{
		pChar->SetRealXY(x,y);
	}



	//卫士 弓箭守卫下跪
	if(pChar->GetLooks().Char.wShape == 115 || pChar->GetLooks().Char.wShape == 116)
	{
		if(msg[11] == 1)
		{
			if(pChar->GetMonsterArmLevel() == 0)
			{
				pChar->SetMonsterArmLevel(1);
				pChar->PushSNextAction(ACTION_ATTACK4,dir,x,y);//下跪不动
			}
		}
		else if(msg[11] == 2)
		{
			if(pChar->GetMonsterArmLevel() > 0)
			{
				pChar->SetMonsterArmLevel(0);

				SNextAction pSecond;
				SAction& pActionNow = pChar->GetAction();
				if(pActionNow.wAction == ACTION_ATTACK1 && pActionNow.iData > 0)
				{
					pChar->InitAction(ACTION_STAND);
				}
			}
		}
		else if(msg[11] == 0)
		{
			SAction& pActionNow = pChar->GetAction();
			if(pActionNow.wAction == ACTION_ATTACK1 && pActionNow.iData > 0)
			{
				pChar->InitAction(ACTION_STAND);
			}
		}
	}


	Magic_Show_s* ms = NULL;
	switch(pChar->GetRaceNo())
	{
	case 226: //噩梦铠魔
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_BIG_SWORD_ATTACKED,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 228: //蛇妖王
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_ICE_SWORD_ATTACKED,EMP_MAGIC_NOTARGET,id);
			else if (cActionType == 1)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_ICE_BLOCK_ATTACKED,EMP_MAGIC_NOTARGET,id);

			break;
		}
	case 232: //暗影修罗的攻击效果
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_ANYING_ATTACK_EFFECT,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 233: //魔神修罗的攻击效果
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_MOSHEN_ATTACK_EFFECT,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}

	case 235:// 阿修罗神得攻击效果 , 自身特效包
		g_pMagicCtrl->CreateMagic(MAGICID_MON_SELF_EFFECT,EMP_MAGIC_NOTARGET,id);
		break;
	case 236:// 阿修罗神变身后得攻击效果 , 自身特效包
		g_pMagicCtrl->CreateMagic(MAGICID_MON_LIGHT_EFFECT_BEGIN,EMP_MAGIC_NOTARGET,id);
		for(int i=0; i<10; i++)
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_LIGHT_EFFECT,EMP_MAGIC_NOTARGET,id,0,-1,0xFFFFFFFF,(512 - (rand()%1024) -200 - (TILE_WIDTH/2)),(384 - (rand()%768) - (TILE_HEIGHT/2)));
			if(ms)
			{
				ms->byAlpha = (i%2 == 0) ? RM_ADD1 : RM_ADD2;
				ms->iCycles = rand()%90;
			}
		}
		break;
	case 242: // 海兽
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_HAISHOU_ATTACK_EFFECT,EMP_MAGIC_NOTARGET,id);
			else
				ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_HAISHOU_ATTACK2_EFFECT,EMP_MAGIC_NOTARGET,id);

			break;
		}
	case 249:// 无相天魔鬼
		if(cActionType == 1)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_MON_LIGHT_EFFECT_BEGIN,EMP_MAGIC_NOTARGET,id);
			for(int i=0; i<10; i++)
			{
				ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_LIGHT_EFFECT,EMP_MAGIC_NOTARGET,id,0,-1,0xFFFFFFFF,(512 - (rand()%1024)-200 - (TILE_WIDTH/2)),(384 - (rand()%768) - (TILE_HEIGHT/2)));
				if(ms)
				{
					ms->byAlpha = (i%2 == 0) ? RM_ADD1 : RM_ADD2;
					ms->iCycles = rand()%90;
				}
			}
		}
		else if(cActionType == 3)//召唤
		{
			g_pMagicCtrl->CreateMagic(MAGICID_MON_DEMON_CALL_UP,EMP_MAGIC_NOTARGET,id);
		}
		else if(cActionType == 4)//无相天魔吞噬效果1
		{
			g_pMagicCtrl->CreateMagic(MAGICID_MON_DEMON_LICK_UP1,EMP_MAGIC_NOTARGET,id);
		}
		break;
	case 451: //混沌女王的攻击效果
		{
			if (cActionType == 1)//远程全屏攻击
			{
				Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_QUEEN_ATTACK,EMP_MAGIC_NOTARGET,id);
			}
			else if (cActionType == 0)//近身单体攻击
			{
				g_pMagicCtrl->CreateMagic(MAGICID_HDNW_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);
			}
			break;
		}
	case 381: //长枪兵
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_CHANGQIANG_ATTACK1,EMP_MAGIC_NOTARGET,id,0,dir*2);
			else
				ms = g_pMagicCtrl->CreateMagic(MAGICID_CHANGQIANG_ATTACK2,EMP_MAGIC_NOTARGET,id,0,dir*2);

			break;
		}
	case 383: //盾兵
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_DUNBING_ATTACK1,EMP_MAGIC_NOTARGET,id,0,dir*2);
			else
				ms = g_pMagicCtrl->CreateMagic(MAGICID_DUNBING_ATTACK2,EMP_MAGIC_NOTARGET,id,0,dir*2);

			break;
		}
	case 385: //骑兵
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_QIBING_ATTACK1,EMP_MAGIC_NOTARGET,id,0,dir*2);
			else
				ms = g_pMagicCtrl->CreateMagic(MAGICID_QIBING_ATTACK2,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 387: //双刀兵
		{
			if (cActionType == 0)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_SHUANGDAOBING_ATTACK1,EMP_MAGIC_NOTARGET,id,0,dir*2);
			else
				ms = g_pMagicCtrl->CreateMagic(MAGICID_SHUANGDAOBING_ATTACK2,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 388: //将军状态1
		{
			if (cActionType == 4)
				g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUN_ZAJIAN,EMP_MAGIC_NOTARGET,id);
			break;
		}
	case 389: //将军状态2
		{
			if (cActionType == 4)
				g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUN_ZAJIAN,EMP_MAGIC_NOTARGET,id);
			else if (cActionType == 2)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUNBOSS_BS_AOE,EMP_MAGIC_NOTARGET,id,0,dir*2);
			else if (cActionType == 3)
				ms = g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUNBOSS_BS_SPECIAL,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 442: //巨猿的攻击效果
		{
			if (cActionType == 0)//物理攻击
			{
			}
			else if (cActionType == 1)//魔法攻击
			{				
				g_pMagicCtrl->CreateMagic(MAGICID_APE_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);
			}
			break;
		}
	case 446: // 狼怪人形
		{
			g_pMagicCtrl->CreateMagic(MAGICID_WOLF_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 484: // 木灵虎
		{
			g_pMagicCtrl->CreateMagic(MAGICID_Tiger_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);//单手攻击
			break;

		}
	case 481: // 水麒麟
		{
			g_pMagicCtrl->CreateMagic(MAGICID_QILIN_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);//单手攻击
			break;
		}
	case 448: // 狼怪兽型
		{
			g_pMagicCtrl->CreateMagic(MAGICID_WOLF_ANIMAL_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);//单手攻击
			break;
		}
	case 482: // 凤凰
		{
			g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_ATTACK,EMP_MAGIC_NOTARGET,id,0,dir*2);//单手攻击
			break;
		}
	case 447: // 独角兽
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_DUJIAOSHOU_ZHUAZI,EMP_MAGIC_NOTARGET,id,0,dir*2);//单手攻击
			break;
		}
	case 480: //金刚豹
		{
			//近身单体攻击
			g_pMagicCtrl->CreateMagic(MAGICID_JINGANGBAO_ZHUAZI,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 483: //土岩狮
		{
			//近身单体攻击
			g_pMagicCtrl->CreateMagic(MAGICID_TULION_ZHUAZI,EMP_MAGIC_NOTARGET,id,0,dir*2);
			break;
		}
	case 441: //撼天力士,犀牛
		{
			if (cActionType == 0)//头顶
			{
			}
			else if (cActionType == 1)//锤子
			{
				g_pMagicCtrl->CreateMagic(MAGICID_XINIU_ZHENDI,EMP_MAGIC_NOTARGET,id,0,dir*2);
			}
			break;
		}
	}
}

void CGameControl::MSG_Monster_Attack2(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if(!pChar)
		return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if (pNext)
	{
		pNext->uFlag |= SERVER_ACTION;
	}

	if(iLen == 14)			// 挖矿的时候有矿渣
	{
		WORD wTemp = Conv_WORD(msg + 12);
		if(wTemp == 1)
			pChar->AddMagicState(MS_VALIDDIG);
	}
}

void CGameControl::MSG_Monster_Attack3(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) 
		return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK3,dir,x,y);
	if (pNext)
	{
		pNext->uFlag |= SERVER_ACTION;
	}
}

void CGameControl::MSG_Monster_Attack_Kill(const char * msg,int iLen)
{

	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	char byMagicLevel = msg[11];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if(pChar) 
	{
		if(iLen >= 20)//第16~19字节(下标)表示击中的目标ID,播放强化攻杀击中特效
		{
			DWORD targeId = Conv_DWORD(msg + 16);
			g_pMagicCtrl->CreateMagic(MAGICID_SUPER_SKILL_ATTACKED,0,targeId,0,(byMagicLevel - 1)*2);
		}

		SNextAction * pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
		if (pNext)
		{
			pNext->uFlag   |= MAGIC_ACTION;
			pNext->uFlag |= SERVER_ACTION;
			if(byMagicLevel == 0)
				pNext->iData    = MAGICID_ATTACK_KILL;
			else
				pNext->iData    = MAGICID_SUPER_ATTACK_KILL;

			if (iLen >= 16)
			{
				DWORD dwColor = Conv_DWORD(msg + 12);
				if(dwColor != 0)
					pNext->dwColor = dwColor;
			}

			if (byMagicLevel >= 10)
			{
				/*BYTE byFlyType = pChar->GetFlyType();
				if (byFlyType == 0)
				{
				byFlyType = byMagicLevel / 10;
				}*/
				pNext->iData = GetGreateMagicID(MAGICID_ATTACK_KILL,1,byMagicLevel % 10,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
			}	
		}
	}	
}

void CGameControl::MSG_Monster_Attack_Stick(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	char byMagicLevel = msg[11];// > 10表示大师级技能

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if(pNext)
	{
		pNext->uFlag   |= MAGIC_ACTION;
		pNext->uFlag |= SERVER_ACTION;
		if (byMagicLevel >= 10)
		{
			/*BYTE byFlyType = pChar->GetFlyType();
			if (byFlyType == 0)
			{
			byFlyType = byMagicLevel / 10;
			}*/
			pNext->iData = GetGreateMagicID(MAGICID_ATTACK_STICK,1,byMagicLevel % 10,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
		}	

		if(iLen >= 16)
		{
			DWORD dwColor = Conv_DWORD(msg + 12);
			if(dwColor != 0)
				pNext->dwColor = dwColor;
		}
	}
}

void CGameControl::MSG_Monster_Attack_Moon(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];

	char byMagicLevel = msg[11];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if(pNext)
	{
		pNext->uFlag   |= MAGIC_ACTION;
		pNext->uFlag |= SERVER_ACTION;
		pNext->iData    = MAGICID_ATTACK_MOON;

		if(iLen >= 16)
		{
			DWORD dwColor = Conv_DWORD(msg + 12);
			if(dwColor != 0)
				pNext->dwColor = dwColor;
		}

		if (byMagicLevel >= 10)
		{
			/*BYTE byFlyType = pChar->GetFlyType();
			if (byFlyType == 0)
			{
			byFlyType = byMagicLevel / 10;
			}*/
			pNext->iData = GetGreateMagicID(MAGICID_ATTACK_MOON,1,byMagicLevel % 10,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
		}
	}
}

void CGameControl::MSG_Monster_Attack_Fire(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	char byMagicLevel = msg[11];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if(pNext)
	{
		pNext->uFlag   |= MAGIC_ACTION;
		pNext->uFlag |= SERVER_ACTION;
		pNext->iData    = MAGICID_ATTACK_FIRE;

		if(iLen >= 16)
		{
			DWORD dwColor = Conv_DWORD(msg + 12);
			if(dwColor != 0)
				pNext->dwColor = dwColor;
		
			if (byMagicLevel > 10)
			{
				/*BYTE byFlyType = pChar->GetFlyType();
				if (byFlyType == 0)
				{
				byFlyType = byMagicLevel / 10;
				}*/
				pNext->iData = GetGreateMagicID(MAGICID_ATTACK_FIRE,1,byMagicLevel%10,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
			}
		}
	}
}

void CGameControl::MSG_Monster_Cut(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];

	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) 
		return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_CUT,dir,x,y);
	if (pNext)
	{
		pNext->uFlag |= SERVER_ACTION;
	}
}

void CGameControl::MSG_Monster_Attack_Magic(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	string buf;
	buf.assign(msg+12,iLen-12);
	int i = 0;
	WORD  iMagicID = StringUtil::toUInt(buf,i);
	int iLevel = StringUtil::toUInt(buf,i);	
	//int iFlyType = StringUtil::toUInt(buf,i);


	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if( !pChar ) 
		return;

	//iMagicID = GetGreateMagicID(iMagicID,iFlyType==0?pChar->GetFlyType():iFlyType,iLevel);
	iMagicID = GetGreateMagicID(iMagicID,1,iLevel,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
	int wx = Conv_WORD(msg+6);
	int wy = Conv_WORD(msg+8);

	if(iMagicID == MAGICID_WILD_COLLIDE || iMagicID == MAGICID_SUDDEN_KILL || iMagicID == MAGICID_AOFENGCUT_ATTACK || iMagicID == MAGICID_ZHONGLEI_ATTACK)
		return;// 没有魔法效果

	
	SNextAction * pNext = NULL;
	if(iMagicID >= MAGICID_SWT && iMagicID <= MAGICID_JYL)
	{
		//这几个动作在,自己和其它玩家都在MSG_Magic_Effectived里面创建
		//pNext = pChar->PushSNextAction(ACTION_SWT + iMagicID - MAGICID_SWT,DIR_NO,wx,wy);
	}
 	else if (iMagicID == MAGICID_SHADOWKILL8)
 	{
 		//自己和其它玩家都在MSG_ShadowKill8里面创建
 	}
	else
	{
		pNext = pChar->PushSNextAction(ACTION_MAGIC,DIR_NO,wx,wy);
	}

	if(pNext) 
	{
		pNext->uFlag |= SERVER_ACTION;
		pNext->iData	= iMagicID;					// MagicID
	}
}

void CGameControl::MSG_Magic_NoEffectived(const char * msg,int iLen)
{
	//int iMagicID = Conv_DWORD(msg);

	////去掉等待魔法属性
	//Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(iMagicID,SELF.GetID(),EMP_MAGIC_WAIT_SERVER);
	//if (ms)
	//{
	//	g_pMagicCtrl->FinishMagic(ms);
	//}
	SELF.SetWaitServer(false,false);
}

void CGameControl::MSG_Magic_Effectived(const char * msg,int iLen)	//使用魔法成功
{
	UINT oid = Conv_DWORD(msg);
	UINT tid = Conv_DWORD(msg + 12);
	WORD wx = Conv_WORD(msg + 6);
	WORD wy = Conv_WORD(msg + 8);
	DWORD dwColor = 0;
	int iDir = -1;
	if(iLen >= 20)
		dwColor = Conv_DWORD(msg + 16);
	WORD wMagicID = Conv_WORD(msg+10);
	BYTE byMagicLevel = 0;
	if (iLen > 20)
		byMagicLevel = msg[20];
	//BYTE byFlyType = 0;

	CCharacterAttr * pNode = NULL;
	int iX = 0,iY = 0;

	if(oid)
	{
		pNode = g_pGameData->FindCharacterByID(oid);
	}

	////法阵攻击
	//if (wMagicID == MAGICID_FAZHEN_CARRYON && iLen >= 20)
	//{	
	//	BYTE zhuFabao = Conv_BYTE(msg + 16);
	//	BYTE leftFabao = Conv_BYTE(msg + 17);
	//	BYTE rightFabao = Conv_BYTE(msg + 18);

	//	char fazhenName[9] = {0};
	//	memcpy(fazhenName,msg + 20,iLen - 20);

	//	char temp[16] = {0};

	//	memcpy(temp,fazhenName,strlen(fazhenName));

	//	char cleanedWord[64] = {0};
	//	g_DirtyWords.ClearWords(temp,cleanedWord);

	//	pNode->SetMagicSlogan(cleanedWord);

	//	g_pMagicCtrl->CreateMagic(MAGICID_SHOWTEXT_ONHEAD,EMP_MAGIC_SERVER_ADVISE,oid);

	//	//关闭法阵
	//	if (pNode->IsUsingFaZhen())
	//	{
	//		pNode->UseFaZhen(false);
	//	}

	//	//发出法阵
	//	g_pMagicCtrl->CreateMagic(MAGICID_FAZHEN_ATTACK_START + zhuFabao*2-1,0,oid,tid,-1,-1,0,-60);
	//	g_pMagicCtrl->CreateMagic(MAGICID_FAZHEN_ATTACK_START + leftFabao*2,0,oid,tid,-1,-1,-60,80);
	//	g_pMagicCtrl->CreateMagic(MAGICID_FAZHEN_ATTACK_START + rightFabao*2,0,oid,tid,-1,-1,60,80);


	//	return;
	//}


	if (pNode)
	{
		pNode->GetXY(iX,iY);
		iDir = GetDir8(wx,wy,iX,iY);

		//之前的wmagicid很多地方传的是db里的wMagicEffectType,后面加上真正的magicid及技能等级
		if (iLen >= 25)
		{
			wMagicID = Conv_WORD(msg + 21);
			byMagicLevel = msg[23];
			//[协议要修改]
			//byFlyType = msg[24];

			////怕服务器可能有些地方不传flytype
			//if (byFlyType == 0)
			//{
			//	byFlyType = pNode->GetFlyType();
			//}			
		}
		wMagicID = GetGreateMagicID(wMagicID,1,byMagicLevel,pNode->IsInDaoZunJiangLinState(),pNode->GetFightOnLeopard());
	}

	if(pNode && pNode->GetFightOnLeopard())
	{
		wMagicID = GetFightOnLeopardMagicID(wMagicID);		
	}


	if (pNode && wMagicID >= MAGICID_SWT && wMagicID <= MAGICID_JYL)
	{
		SNextAction* pNext = pNode->PushSNextAction(ACTION_SWT + wMagicID - MAGICID_SWT,iDir,wx,wy);	
		if (pNext)
		{
			pNext->uFlag |= SERVER_ACTION;
			pNext->iData = wMagicID;
		}
	}

	if(pNode && wMagicID == MAGICID_DULINGBO_ATTACK)
	{
		iDir *= 2;
		if(oid == SELF.GetID())
		{
			g_AIMgr.UpdateRideFightTime();
		}
	}

	if (pNode && oid == SELF.GetID())
	{
		g_AutoKillMonsterMgr.OnMagicEffectived(wMagicID);
	}

// 	if (pNode && oid == SELF.GetID())
// 	{
// 		TneupMember *pMember = g_TrusteeshipData.FindMemberByID(oid);
// 		if (pMember)
// 		{
// 			switch (wMagicID)
// 			{
// 			case MAGICID_ENCALL_MONSTER:
// 				pMember->aPet[ePetSuperSkull] = 2;
// 				break;
// 			case MAGICID_CONTROL_CORPSE:
// 				pMember->aPet[ePetCorpus] = 1;
// 				break;
// 			case MAGICID_CALL_JOSS:
// 				pMember->aPet[ePetSupernalBeast] = 1;
// 				break;
// 			case MAGICID_CALL_MONSTER:
// 				pMember->aPet[ePetSkull] = 1;
// 				break;
// 			default:
// 				break;
// 			}
// 		}
// 	}

	//////////////////
	Magic_Show_s* ms = NULL;

	if(oid && wMagicID == MAGICID_DULINGBO_ATTACK)
		ms = g_pMagicCtrl->FindMagicByAll(MAGICID_DULINGBO_ATTACK,oid,-1);
	else
		ms = g_pMagicCtrl->FindMagicByAll(wMagicID,oid,-1,EMP_MAGIC_WAIT_SERVER);

	if(ms)
	{
		if(ms->ref)
		{
			if(tid != 0)
			{
				ms->ref->uTargetID = tid;
				ms->ref->attr &= ~EMP_MAGIC_NOTARGET;
			}

			ms->ref->wTargetTileX = wx;
			ms->ref->wTargetTileY = wy;
			ms->attr &= ~EMP_MAGIC_WAIT_SERVER;

			if(dwColor != 0)
				ms->dwColor = dwColor;

			if(iLen >= 21)//强化技能的魔法等级
				ms->ref->wData[1] = msg[20];
		}
	}
	else////创建新魔法
	{
		if(oid == 0) //没有攻击者
		{			
			ms = g_pMagicCtrl->CreateMagic(wMagicID,0,oid,tid,iDir);			
			if(ms && ms->ref)
			{
				ms->ref->wTargetTileX = wx;
				ms->ref->wTargetTileY = wy;
				ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				if(dwColor != 0)
					ms->dwColor = dwColor;

				if(iLen >= 21)//强化技能的魔法等级
					ms->ref->wData[1] = msg[20];
			}
		}
		else//有攻击者的情况
		{
			CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(oid);
			if(pChar) 
			{
				SNextAction* pNext = pChar->GetNextAction();//PushSNextAction(ACTION_MAGIC);
				WORD wAction = ACTION_MAGIC;
				if(wMagicID >= MAGICID_SWT && wMagicID <= MAGICID_JYL)
				{
					wAction = ACTION_SWT + wMagicID - MAGICID_SWT;
				}

				if(pNext && pNext->wAction == wAction)
				{
					pNext->uFlag |= SERVER_ACTION;
					pNext->uData = 5;//创建魔法的如果遇到uData == 5;因为这个时候服务器已经返回成功了，所以创建的时候要去掉EMP_MAGIC_WAIT_SERVER标记
					pNext->uTarget = tid;
					pNext->iAimX   = wx;
					pNext->iAimY   = wy;
					if(dwColor != 0)
						pNext->dwColor = dwColor;

					if(iLen >= 21)//强化技能的魔法等级
						pNext->iData2 = msg[20];
				}
			}
		}
	}

	//寒冰风暴,判断施法者处于吟唱状态状态
	if (wMagicID == MAGICID_COLDSTROM)
	{	
		CSimpleCharacter * pChar = g_pGameData->FindCharacterByID(oid);
		if (pChar->IsSinging())
		{
			DWORD dwMagicType  = EMP_MAGIC_SERVER_ADVISE;
			dwMagicType |= EMP_MAGIC_NOTARGET;
			DWORD dwTargetID = MAKELONG(wx,wy);

			g_pMagicCtrl->CreateMagic(MAGICID_HANBINGFENGBAO_QISHOUJIDA,dwMagicType,pChar->GetID(),dwTargetID);
		}
	}
}

void CGameControl::MSG_Magic_Position_Success(const char * msg,int iLen)	//使用冲撞成功
{
	DWORD id = Conv_DWORD(msg);
	int   x = Conv_WORD(msg + 6);
	int   y = Conv_WORD(msg + 8);
	BYTE  dir = msg[10];
	char  extraState = msg[11];
	__int64 looks = Conv_INT64(msg+12);

	SNextAction * pNext = NULL;

	if(id == SELF.GetID())		// 主角跑的动作
	{
		SELF.SetRealXY(x,y);
		pNext = SELF.PushSNextAction(ACTION_RUN,dir,x,y);
		if (pNext)
		{
			pNext->iLooks		= looks;
			pNext->uFlag |= COLLIDE_ACTION;   

			if(SELF.GetAction().wAction == ACTION_STAND)
				g_pGameMgr->PlayerDoNextStep();
		}

		WORD state = SELF.GetExtraState();
		if(extraState == 2)
		{		
			state |= 0x04;	
			SELF.SetReserveData(plySuddenKillFrame,0);
		}
		else if (extraState == 3)
		{
			state &= ~0x04;
			pNext->uFlag |= COLLIDE_SHEN;
		}
		else if (extraState == 4)
		{
			state &= ~0x04;
			pNext->uFlag |= COLLIDE_MO;
		}
		else
		{
			state &= ~0x04;
		}
		SELF.SetExtraState(state);
	}
	else
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(id);
		if( !pChar)
			return;

		pChar->SetRealXY(x,y);
		pNext = pChar->PushSNextAction(ACTION_RUN,dir,x,y);
		if(pNext)
		{
			pNext->iLooks = looks;
			pNext->uFlag |= COLLIDE_ACTION;

			if(pChar->GetAction().wAction == ACTION_STAND || pChar->GetRaceNo() == 304)
				g_pGameMgr->CharDoNextStep(pChar);
		}


		WORD state = pChar->GetExtraState();
        if(extraState == 2)
		{		
			state |= 0x04;	
			pChar->SetReserveData(plySuddenKillFrame,0);
		}
		else if (extraState == 3)
		{
			state &= ~0x04;
			pNext->uFlag |= COLLIDE_SHEN;
		}
		else if (extraState == 4)
		{
			state &= ~0x04;
			pNext->uFlag |= COLLIDE_MO;
		}
		else
		{
			state &= ~0x04;
		}
		pChar->SetExtraState(state);
	}
}
void CGameControl::MSG_Magic_Position_Failed(const char * msg,int iLen)	//使用冲撞失败
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y= Conv_WORD(msg + 8);
	BYTE dir = msg[10];
	__int64 looks = Conv_INT64(msg + 12);
	SNextAction * pNext;

	if(id == SELF.GetID())		// 主角跑的动作
	{
		pNext = SELF.PushSNextAction(ACTION_RUN,dir,x,y);
		if(pNext)
		{
			pNext->iLooks		= looks;
			pNext->uFlag |= COLLIDE_ACTION;  
			pNext->iData = 1;

			if(SELF.GetAction().wAction == ACTION_STAND)
				g_pGameMgr->PlayerDoNextStep();
		}
	}
	else
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(id);
		if( !pChar)
			return;

		pNext = pChar->PushSNextAction(ACTION_RUN,dir,x,y);
		if(pNext)
		{
			pNext->iLooks		= looks;
			pNext->uFlag |= COLLIDE_ACTION; 
			pNext->iData = 1;

			if(pChar->GetAction().wAction == ACTION_STAND)
				g_pGameMgr->CharDoNextStep(pChar);
		}
	}
}

void CGameControl::MSG_Magic_Monster1_Attack(const char * msg,int iLen)
{
	DWORD oid = Conv_DWORD(msg);
	WORD  x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	BYTE cActionType = msg[11];
	WORD tx = Conv_WORD(msg + 12);
	WORD ty = Conv_WORD(msg + 14);
	DWORD tid = Conv_DWORD(msg + 16);

	CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(oid);
	if(!pChar)
		return;

	if( pChar->IsNpc())
	{
		//SAction& saction = pChar->GetAction();
		//saction.SetNpcRandStand(pChar->GetRaceNo());

		pChar->SetDir(dir);
		return;
	}

	WORD wAction = ACTION_ATTACK1;
	switch(cActionType)
	{
	case 0:	wAction = ACTION_ATTACK1; break;
	case 1:	wAction = ACTION_ATTACK2; break;
	case 2:	wAction = ACTION_ATTACK3; break;
	case 3: wAction = ACTION_ATTACK4; break;
	case 4: wAction = ACTION_ATTACK5; break;
	case 5: wAction = ACTION_ATTACK6; break;
	case 6: wAction = ACTION_ADDACTION1; break;
	case 7: wAction = ACTION_ATTACK3; break;
	}


	SNextAction * pNext = pChar->PushSNextAction(wAction,dir,tx,ty);
	if (pNext)
		pNext->uData = tid;

	switch(pChar->GetRaceNo())
	{
	case 218: //投石巨魔
		{
			g_pMagicCtrl->CreateMagic(MAGICID_BIG_STONE_ATTACKED,0,oid,tid);
			break;
		}
	case 228: //玄冰蛇妖
		{
			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_ICE_ARROW,0,oid,tid);
			if(ms)
			{
				g_pMagicCtrl->CreateMagic(ms);
				g_pMagicCtrl->FinishMagic(ms);
			}
			break;
		}
	case 229: //逆魔之王
		{
			if (pNext)
				pNext->wAction = ACTION_ATTACK2;

			break;
		}
	case 234: //黑袍修罗的攻击效果
		{
			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_HEIPAO_ATTACK_EFFECT,0,oid,tid);
			break;
		}
	case 243://寒荒小怪
	case 244:
	case 245:
		{
			if(!g_pMagicCtrl->CreateMagic(MAGICID_MON_XIAOGUAI_ATTACK_EFFECT,EMP_MAGIC_NOTARGET,oid,MAKELONG(tx,ty)))
			{
				pChar->SetXY(tx,ty);
				break;
			}
			break;
		}
	case 384: //弓箭
		{
			if (cActionType == 0)
				g_pMagicCtrl->CreateMagic(MAGICID_GONGJIAN_HUANGSE_PUTONG,0,oid,tid);
			else
				g_pMagicCtrl->CreateMagic(MAGICID_GONGJIAN_HUANGSE_JIAQIANG,0,oid,tid);

			break;
		}
	case 386: //轻骑兵
		{
			if (cActionType == 0)
				g_pMagicCtrl->CreateMagic(MAGICID_GONGJIAN_LANSE_PUTONG,0,oid,tid);
			else
				g_pMagicCtrl->CreateMagic(MAGICID_GONGJIAN_LANSE_JIAQIANG,0,oid,tid);

			break;
		}
	case 388: //将军状态1
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUNBOSS_PT_FLYINGBALL,0,oid,tid);
			break;
		}
	case 389: //将军状态2
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_JIANGJUNBOSS_BS_MAGIC,0,oid,tid);
			break;
		}
	case 421: //法宝骑兵
	case 434:
		{

			CSimpleCharacter * pOwner = g_pGameData->FindCharacterByID(oid);

			CSimpleCharacter * pTarget = g_pGameData->FindCharacterByID(tid);
			if (pOwner == NULL || pTarget == NULL)
			{
				return;
			}
			int ownX,ownY;
			int tarX,tarY;

			pOwner->GetXY(ownX,ownY);
			pTarget->GetXY(tarX,tarY);

			int distant = max(abs(ownX - tarX),abs(ownY - tarY));

			BYTE wFaBaoType = pOwner->GetFaBaoType()->dwCurFaBaoType;

			BYTE byQianghuaLevel = pOwner->GetFaBaoType()->iFaBaoLevel;
			//强化等级0~5:白色 6~8:蓝色 9:金
			int EffColor = byQianghuaLevel == 9?3:(byQianghuaLevel >= 6?2:1);

			if ( distant > 4 /*|| (rand()%10) < 1*/)//一定用远距离的
			{
				g_pMagicCtrl->CreateMagic(wFaBaoType*2 + MAGICID_FABAO_START,0,oid,tid);
				g_pMagicCtrl->CreateMagic(MAGICID_FABAOTRACK_SHUXING_jin_YUAN,0,oid,tid);
				g_pMagicCtrl->CreateMagic((wFaBaoType - 1)*6 + MAGICID_FABAO_ATTACK_EFFECT_START + EffColor + 3,0,oid,tid);
			}
			else 
			{
				g_pMagicCtrl->CreateMagic(wFaBaoType*2 - 1 + MAGICID_FABAO_START,0,oid,tid);
				g_pMagicCtrl->CreateMagic(MAGICID_FABAOTRACK_SHUXING_jin,0,oid,tid);
				g_pMagicCtrl->CreateMagic((wFaBaoType - 1)*6 + MAGICID_FABAO_ATTACK_EFFECT_START + EffColor,0,oid,tid);
			}
			break;
		}
	case 435://骷髅法师攻击
		g_pMagicCtrl->CreateMagic(MAGICID_DEATH_RABBI_ATTACK,0,oid,tid);
		break;
	case 111://召唤神兽
		g_pMagicCtrl->CreateMagic(MAGICID_SHENSHOU_SHANXING,EMP_MAGIC_NOTARGET,oid,MAKELONG(tx,ty));
		break;
	case 480: //豹子的攻击效果
		{
			if (cActionType == 1)//远程单体攻击
			{
				Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_LEOPARD_ATTACK,0,oid,tid);
			}
			else if (cActionType == 2)//远程范围攻击
			{
				//Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_LEOPARD_ATTACK2,EMP_MAGIC_NOTARGET,oid,MAKELONG(tx,ty));
			}
			break;
		}
	case 450: //火怪的攻击效果
		{
			g_pMagicCtrl->CreateMagic(MAGICID_FIREMONSTER_ATTACK,0,oid,tid);
			break;
		}
	case 446: //狼怪人形
		{
			g_pMagicCtrl->CreateMagic(MAGICID_WOLF_FLYINGBALL,0,oid,tid);
			break;
		}
	case 484: //老虎
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_Tiger_FLYINGBALL,0,oid,tid);
			break;
		}
	case 481: //麒麟
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_QILIN_FLYINGBALL,0,oid,tid);
			break;
		}
	case 482: //凤凰
		{
			if (cActionType == 1)
				g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_FLYINGBALL,0,oid,tid);
			else if (cActionType == 2)
				g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_COOLATTACK_START,0,oid,tid);
			break;
		}
	case 449: //美女蛇
		{
			g_pMagicCtrl->CreateMagic(MAGICID_MEINVSNAKE_FLYINGBALL,0,oid,tid);
			break;
		}
	case 443: //树妖1
	case 444: //树妖2
		{
			g_pMagicCtrl->CreateMagic(MAGICID_SHUYAO_FLYINGBALL,0,oid,tid);
			break;
		}
	case 447: // 独角兽
		{
			if(cActionType == 0)
				g_pMagicCtrl->CreateMagic(MAGICID_DUJIAOSHOU_TOUDING,0,oid,tid);//单手攻击
			break;
		}
	default:
		break;

	}
}

void CGameControl::MSG_Magic_Monster2_Attack(const char * msg,int iLen)
{
	DWORD oid = Conv_DWORD(msg);
	WORD  x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];

	CSimpleCharacterNode * pChar = g_pGameData->AddSimpleCharacter(x,y,oid);
    if(!pChar) return;

	if(pChar->IsNpc())
	{
		//SAction& saction = pChar->GetAction();
		//saction.SetNpcRandStand(pChar->GetRaceNo());

		pChar->SetDir(dir);
		return;
	}
	pChar->PushSNextAction(ACTION_ATTACK1,dir);

}

void CGameControl::MSG_Monster_Back(const char * msg,int iLen)		// 被其他玩家使用了野蛮冲撞等
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	BYTE type = msg[11];
	__int64 looks = Conv_INT64(msg+12);
	SNextAction * pNext = NULL;
	
	if(id == SELF.GetID())		
	{	
		SELF.SetRealXY(x,y);
		SELF.SetLooks(looks);

		pNext = SELF.PushSNextAction(ACTION_WALK,dir,x,y);
		if(pNext)
		{
			pNext->uFlag |= BACK_ACTION;
			pNext->iData = 1;

			if(SELF.GetAction().wAction == ACTION_STAND)
				g_pGameMgr->PlayerDoNextStep();
			//g_AIMgr.DelayMoveTime();
		}

		if (type == 1)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_BIANBING_ATTACK2,0,SELF.GetID(),SELF.GetID());
		}
	}
	else
	{
		CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar)
			return;

		pChar->SetRealXY(x,y);
		pNext = pChar->PushSNextAction(ACTION_WALK,dir,x,y);

		if(pNext)
		{
			pNext->iLooks = looks;
			pNext->uFlag |= BACK_ACTION;
			pNext->iData = 1;

			if(pChar->GetAction().wAction == ACTION_STAND)
				g_pGameMgr->CharDoNextStep(pChar);
		}

		if (type == 1)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_BIANBING_ATTACK2,0,pChar->GetID(),pChar->GetID());
		}
	}

}

void CGameControl::MSG_Monster_Back2(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	__int64 looks = Conv_INT64(msg+12);
	WORD wFlag = Conv_WORD(msg+24);

	SNextAction * pNext = NULL;
	bool bPosionWave = true;
	WORD wMagicID = MAGICID_DULINGBO_ATTACK;

	if(id == SELF.GetID())		
	{
		SELF.SetRealXY(x,y);
		pNext = SELF.PushSNextAction(ACTION_WALK,dir,x,y);
		if(pNext)
		{
			pNext->uData = wFlag + 1;
			pNext->uFlag |= BACK_ACTION;

			Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAll(94,-1,SELF.GetID());
			if(ms)
			{
				pNext->uFlag |= DELAY_ACTION;
			}

			pNext->iData = 1;

			DWORD dwID = SELF.GetID();

			//Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(wMagicID,dwID,EMP_MAGIC_SERVER_ADVISE);
			//if(!ms)
			//{
			//	DWORD dwTargetID = MAKELONG(x,y);					
			//	ms = g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,0,dwTargetID);
			//}

			if(SELF.GetAction().wAction == ACTION_STAND)
				g_pGameMgr->PlayerDoNextStep();
		}
	}
	else
	{
		CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar)
			return;

		pChar->SetRealXY(x,y);
		pNext = pChar->PushSNextAction(ACTION_WALK,dir,x,y);
		if (pNext)
		{
			pNext->iLooks = looks;
			pNext->uFlag |= BACK_ACTION;
			pNext->uFlag |= DELAY_ACTION;
			pNext->uData = wFlag + 1;
			pNext->iData = 1;

			DWORD dwID = pChar->GetID();

			//Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(wMagicID,dwID,EMP_MAGIC_SERVER_ADVISE);
			//if(!ms)
			//{
			//	ms = g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,dwID,dwID);
			//}

			if(pChar->GetAction().wAction == ACTION_STAND)
				g_pGameMgr->CharDoNextStep(pChar);
		}
	}
}

void CGameControl::MSG_Monster_Back1(const char * msg,int iLen)		// 被其他玩家使用了狮子吼
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	__int64 looks = Conv_INT64(msg+12);
	WORD wFlag = Conv_WORD(msg+24);

	SNextAction * pNext = NULL;
	bool bPosionWave = true;
	WORD wMagicID = MAGICID_LION_CALLED;
	
	if(id == SELF.GetID())		
	{
		SELF.SetRealXY(x,y);
		pNext = SELF.PushSNextAction(ACTION_WALK,dir,x,y);
		if(pNext)
		{
			pNext->uData = wFlag + 1;
			pNext->uFlag |= BACK_ACTION;
			pNext->iData = 1;

			DWORD dwID = SELF.GetID();

			Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(wMagicID,dwID,EMP_MAGIC_SERVER_ADVISE);
			if(!ms)
			{
				if(wMagicID == MAGICID_DULINGBO_ATTACK)
				{
					DWORD dwTargetID = MAKELONG(x,y);					
					ms = g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,0,dwTargetID,dir*2);
				}
				else
					ms = g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,dwID,dwID,dir*2);//
			}

			if(SELF.GetAction().wAction == ACTION_STAND)
				g_pGameMgr->PlayerDoNextStep();
		}
	}
	else
	{
		CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar)
			return;

		pChar->SetRealXY(x,y);
		pNext = pChar->PushSNextAction(ACTION_WALK,dir,x,y);
		if (pNext)
		{
			pNext->iLooks = looks;
			pNext->uFlag |= BACK_ACTION;
			pNext->uData = wFlag + 1;
			pNext->iData = 1;
			
			DWORD dwID = pChar->GetID();

			Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(wMagicID,dwID,EMP_MAGIC_SERVER_ADVISE);
			if(!ms)
			{
				ms = g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,dwID,dwID,dir*2);
			}

			if(pChar->GetAction().wAction == ACTION_STAND)
				g_pGameMgr->CharDoNextStep(pChar);
		}
	}
}

void CGameControl::MSG_Monster_NameColor(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	char cColor = msg[6];

	CCharacterAttr* pChar = g_pGameData->FindCharacterByID(id);
	if(pChar)
	{
		pChar->SetNameColor(cColor);
	}
}

//护身真气,风行术开关
void CGameControl::MSG_Monster_ExtraState(const char *msg, int iLen)
{

	DWORD id = Conv_DWORD(msg);
	WORD state = Conv_WORD(msg + 6);

	//判断此ID是否存在
	CCharacterAttr * pChar = g_pGameData->FindCharacterByID(id);
	if(pChar == NULL)
		return;

	WORD cExtraState = pChar->GetExtraState(); //原来身上的状态

	if(state & 0x10)// 特殊的套装
	{
		//time == 1 --> 幻彩系列__time == 2 --> 幻影系列
		WORD wTime = Conv_WORD(msg + 8);
		pChar->SetReserveData(pubHuanCaiCloth,wTime);
		if(iLen > 12)
		{
			g_OtherData.GetSetMsg().assign(msg + 12,iLen - 12);
			g_OtherData.SetSetMsgStartTime(GetTickCount());
		}
		return;
	}
	else if(state & 0x08) // 有封号
	{
		std::string strMask;
		strMask.assign(msg+12,iLen -12);

		//下标10作为封号高字节
		WORD wMaskLevel = (BYTE)msg[7];
		wMaskLevel |= ((WORD)msg[10] << 8);

		pChar->SetMaskName(wMaskLevel,strMask.c_str());
		pChar->SetReserveData(pubHasWings,(BYTE)wMaskLevel > 0?1:0);

		//if(id != SELF.GetID())
		//{
		//	((CSimpleCharacter*)pChar)->SetMerShapeType((BYTE)msg[8]);
		//	((CSimpleCharacter*)pChar)->SetOtherLevel((BYTE)msg[9]);
		//}

		pChar->SetVipCardType((BYTE)(msg[11]));
		return;
	}
	

	if(state == 0x40) //变身
	{
		DWORD dwLooks = Conv_DWORD(msg+8);
		pChar->SetBianShenLooks(dwLooks);
		if(dwLooks != 0)
		{
			if (pChar->GetBianShenLooks().Char.wShape == 198)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_BIANSHEN_EFFECT,0,id);
			}
		}
		else
		{
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_BIANSHEN_EFFECT,id));
		}

		if (id == SELF.GetID())
		{
			SELF.SetReserveData(plyAttackLockID,0);
			SELF.SetReserveData(plyMagicLockID,0);
		}

		return;
	}
	else if(state & ES_ZHENYUAN)//真元攻击
	{
		WORD dwContinueTime = *((WORD*)(msg + 8));
		if(dwContinueTime != 0)
		{
			//设置真元攻击
			pChar->SetReserveData(plyZhenYuanStart,GetTickCount());
			pChar->SetReserveData(plyZhenYuanConTm,dwContinueTime*1000);
			return;
		}		
	}
	
	if (state & ES_GOLDENOXSUIT)//金牛套装
	{
		cExtraState |= 	ES_GOLDENOXSUIT;
	}
	else
	{
		cExtraState &= ~ES_GOLDENOXSUIT;
	}

	if(state & 0x01)//金刚护体
	{
		cExtraState |=  ES_STEELPROTECT;
		if(!g_pMagicCtrl->FindMagicByAll(MAGICID_STEEL_PROTECT,id))
			g_pMagicCtrl->CreateMagic(MAGICID_STEEL_PROTECT,EMP_MAGIC_SERVER_ADVISE,id,id);
	}
	else
	{
		cExtraState &= ~ES_STEELPROTECT;
		g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_STEEL_PROTECT,id));
	}

	if(state & 0x02)//风影盾
		cExtraState |=  ES_DODGEPROTECT;
	else
		cExtraState &= ~ES_DODGEPROTECT;

	if(state & 0x20 || state & ES_FROST2) //被冰冻
		cExtraState |= ES_FROST;
	else
	{
		cExtraState &= ~ES_FROST;
	}

	//狂怒旋风
	if((state & ES_KUANGNUWIND) && !(cExtraState & ES_KUANGNUWIND))
	{
		cExtraState |= ES_KUANGNUWIND;

		WORD wMagicID = MAGICID_KUANGNU_XUANFENG;
		Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(wMagicID,id);
		if (!pShow)	
			g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,id,id,-1);		
	}
	else if((cExtraState & ES_KUANGNUWIND) && !(state & ES_KUANGNUWIND))	// 狂怒风暴的消失(需要延时设置标示位)
	{
		cExtraState &= ~ES_KUANGNUWIND;
		Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(MAGICID_KUANGNU_XUANFENG,id);
		g_pMagicCtrl->CreateMagic(pShow);
		g_pMagicCtrl->FinishMagic(pShow);
	}

	//心灵护体
	if((state & ES_DIANLINGFUTI) && !(cExtraState & ES_DIANLINGFUTI))
	{
		cExtraState |= ES_DIANLINGFUTI;

		WORD wMagicID = MAGICID_XINLINGHUTI_XUNHUAN_JIESHU;
		Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(wMagicID,id);
		if (!pShow)	
			g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,id,id,-1);		
	}
	else if((cExtraState & ES_DIANLINGFUTI) && !(state & ES_DIANLINGFUTI))	// 心灵护体的消失(需要延时设置标示位)
	{
		cExtraState &= ~ES_DIANLINGFUTI;
		Magic_Show_s* ms = NULL;
		vector<Magic_Show_s*> VMs;
		while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_XINLINGHUTI_XUNHUAN_JIESHU,id,-1,-1,-1,ms))
		{
			VMs.push_back(ms);
		}

		for (size_t i = 0; i < VMs.size(); i++)
		{
			g_pMagicCtrl->CreateMagic(VMs[i]);//放到m_ShowList头部
			g_pMagicCtrl->FinishMagic(VMs[i]);
		}
	}

	//寒冰风暴
	if((state & ES_COOLICESTORM) && !(cExtraState & ES_COOLICESTORM))
	{
		cExtraState |= ES_COOLICESTORM;

		WORD wMagicID = MAGICID_COLDSTROM;
		Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(wMagicID,id);
		if (!pShow)	
			g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,id,id,-1);


		if (id == SELF.GetID())
		{
			g_pGameData->SetSingingTimeStart(GetTickCount());
			g_pGameData->SetSingingTechID(MAGICID_COLDSTROM);	
			SELF.AddUsingMagicType(UMT_NOACTION);	
		}		
	}
	else if((cExtraState & ES_COOLICESTORM) && !(state & ES_COOLICESTORM))	// 寒冰风暴的消失(需要延时设置标示位)
	{
		cExtraState &= ~ES_COOLICESTORM;
		Magic_Show_s* ms = NULL;
		vector<Magic_Show_s*> VMs;
		while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_COLDSTROM,id,-1,-1,-1,ms))
		{
			VMs.push_back(ms);
		}

		for (size_t i = 0; i < VMs.size(); i++)
		{
			g_pMagicCtrl->CreateMagic(VMs[i]);//放到m_ShowList头部
			g_pMagicCtrl->FinishMagic(VMs[i]);
		}

		if (id == SELF.GetID())
		{
			g_pGameData->SetSingingTimeStart(0);
			g_pGameData->SetSingingTechID(0);	
			SELF.RemoveUsingMagicType(UMT_NOACTION);
		}
	}


	if(state & 0x80) //是否有泡泡
		cExtraState |= ES_PAOPAO;
	else
		cExtraState &= ~ES_PAOPAO;

	if(state & ES_BRAST)
		cExtraState |= ES_BRAST;
	else
		cExtraState &= ~ES_BRAST;

	if (cExtraState & ES_SHENYOU)   //原来有神佑效果
	{
		if((!(state & ES_SHENYOU) && state != 12) || ((state & ES_SHENYOU) && (state == 12)))   // 单独的附身
		{
			Magic_Show_s * ms = g_pMagicCtrl->FindMagicByAll(MAGICID_PLAYER_MAGIC_EFFECT,pChar->GetID());
			if(ms)
			{
				g_pMagicCtrl->FinishMagic(ms);
			}
			cExtraState &= ~ES_SHENYOU;

			if(state == 12) // 重新吃了附身
			{
				if(g_pMagicCtrl->CreateMagic(MAGICID_PLAYER_MAGIC_EFFECT,0,pChar->GetID()))
					pChar->SetReserveData(plyShenYouStartTime,GetTickCount());	
				cExtraState |= ES_SHENYOU;
			}
		}	
	}
	else  // 没有神佑效果
	{
		if((state & ES_SHENYOU))   //神佑效果出现
		{
			if(g_pMagicCtrl->CreateMagic(MAGICID_PLAYER_MAGIC_EFFECT,0,pChar->GetID()))
				pChar->SetReserveData(plyShenYouStartTime,GetTickCount());
			cExtraState |= ES_SHENYOU;
		}
	}

	DWORD dwTime = Conv_WORD(msg + 8) * 1000;
	if(id == SELF.GetID() && dwTime > 0 && (state & ES_SHENYOU))
	{
		SELF.SetReserveData(plyShenYouLastTime,dwTime);
	}

	pChar->SetExtraState(cExtraState);
}

void CGameControl::MSG_Monster_State(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	if(id == SELF.GetID())
	{		

		WORD wState = Conv_WORD(msg+8);

		if (wState != 0)
		{
			SELF.SetGreateMagicState(Conv_WORD(msg+6));
		}
		SELF.SetStatus(wState);
		if (wState == 0)
		{
			SELF.SetGreateMagicState(Conv_WORD(msg+6));
		}

		SELF.SetAttackSpeed(*((short*)(msg+10)));
	}
	else
	{
		CSimpleCharacter* pChar = g_pGameData->FindSimpleCharacter(id);
		if(pChar)
		{			

			WORD wState = Conv_WORD(msg+8);

			if (wState != 0)
			{
				pChar->SetGreateMagicState(Conv_WORD(msg+6));
			}
			WORD wOldState = pChar->GetStatus();

			pChar->SetStatus(wState);
			if (wState == 0)
			{
				pChar->SetGreateMagicState(Conv_WORD(msg+6));
			}

			if(!(wOldState & CS_SOULWALL) && (wState & CS_SOULWALL))// 灵魂之墙出现
			{
				pChar->SetReserveData(pubSoulWallState,3);
				pChar->SetReserveData(pubSoulWallFrame,0);
			}

			pChar->SetAttackSpeed(Conv_WORD(msg+10));
		}
	}


}

void CGameControl::MSG_Monster_Dead(const char * msg,int iLen)
{
    DWORD id = Conv_DWORD(msg);
    WORD x = Conv_WORD(msg + 6);
    WORD y = Conv_WORD(msg + 8);
    __int64 looks = Conv_INT64(msg+12);

	BYTE byFaBaoWuXing = 0;//0:不是法宝打死的,1:无属性法宝,2,3,4,5,6:金,木,土,水,火属性法宝打死
	if (iLen >= 24)
	{
		byFaBaoWuXing = (BYTE)(msg[23]);
	}

	CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
	if (id == SELF.GetReserveData(plyAttackLockID))
	{
		if (pChar)
		{
			if (pChar->GetRaceNo() == 180)
			{
				g_pGameControl->Send_Player_Prompt_Status(28,1);
				g_pGameControl->Send_Player_Prompt_Status(27,1);
			}
			else g_pGameControl->Send_Player_Prompt_Status(27,1);
		}

		SELF.SetReserveData(plyAttackLockID,0);
	}

	if (id == SELF.GetReserveData(plyMagicLockID))
	{
		if (pChar)
		{
			if (pChar->GetRaceNo() == 180)
			{
				g_pGameControl->Send_Player_Prompt_Status(28,1);
				g_pGameControl->Send_Player_Prompt_Status(27,1);
			}
			else g_pGameControl->Send_Player_Prompt_Status(27,1);
		}

		SELF.SetReserveData(plyMagicLockID,0);
	}

    //if(g_OtherData.GetWuXingFlag().dwFlagInMonstID == id)
    //{
    //    g_OtherData.GetWuXingFlag().dwFlagInMonstID = 0;
    //}

	if(id == SELF.GetMyPetID())
	{
		g_AutoPickMgr.PetPickupClear();
		SELF.SetMyPetID(0);
	}
    if(SELF.GetID() == id)
    {		
		SELF.PushSNextAction(ACTION_DEATH,msg[10],x,y);
		SELF.SetDead(true);
		SELF.SetFaBaoType(0,0,1);//收法宝

		//死亡时的音乐
		if(SELF.IsMale())//男 
		{	
			g_pAudio->Play(EAT_OTHER,16,g_pAudio->GetRand()++);
		}
		else
		{
			g_pAudio->Play(EAT_OTHER,15,g_pAudio->GetRand()++);	
		}

        g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
        g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
// 		g_pControl->PopupWindow(MSG_CTRL_TNEUP_WND,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);

		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);

		if((msg[11] & 0xF0) != 0)//高四位表示还魂神符的价格
			g_OtherData.SetReLivePrice((msg[11] & 0xF0)>>4);

		g_dwDeathTime = GetTickCount(); //死亡的时间
		bool bIsMazeMap = (strnicmp(g_pGameMap->GetMapName(),"MAZE",4) == 0);

		if((msg[11] & 0x03) != 0 || bIsMazeMap)  //低2位通知在副本地图可以直接使用魂神神符
		{
			int iPos = SELF.PackageGood().FindGoodByStdmode(3,28);

			if(iPos < 0)//没有还魂神符弹出用元宝复活的界面
			{
				g_pControl->PopupWindow(MSG_CTRL_RELIVEWITHYUANBAO_WND,OPER_CREATE);//副本地图弹出使用元宝复活界面
			}
			else//有还魂神符弹出用还魂神符复活的界面
			{
				g_pControl->PopupWindow(MSG_CTRL_RELIVE_WND,OPER_CREATE);
			}
		}
		else
		{
			g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CREATE);
		}
    }
    else
    {
        if(!pChar) return;

		int iSelfX,iSelfY;
		SELF.GetXY(iSelfX,iSelfY);

		pChar->SetFaBaoType(0,0,1);//收法宝

		//他人死亡时音乐
		if(pChar->IsHuman())
		{
			if(pChar->IsMale())
				g_pAudio->PlayEx(EAT_OTHER,26,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);
			else
				g_pAudio->PlayEx(EAT_OTHER,25,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);
		}
		else if(pChar->IsMonster())
		{
			if (pChar->IsCanDig())//可挖的怪物,死亡后放一个特效
			{
				g_pMagicCtrl->CreateMagic(MAGICID_DIGMONSTER_DEAD,0,pChar->GetID(),pChar->GetID()); 
				
				if(!g_pGameData->HasPaoPaoStatus(EP_First_CutZhangShu_PaoPao) && strcmp(pChar->GetName(),"樟树") == 0)
				{
					g_pControl->PopupArrowTip(MSG_CTRL_GAMEWND,EP_First_CutZhangShu_PaoPao,x,y,XAL_TOPLEFT,false,XAL_TOPLEFT,1,pChar->GetID());
				}
			}
			
			//五行死亡
			if (byFaBaoWuXing >= 2 && byFaBaoWuXing <= 6)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_MONSTER_JIN_DEAD + byFaBaoWuXing - 2,0,pChar->GetID(),pChar->GetID()); 
			}

			UINT lSoundID = pChar->GetRaceNo() * 8 + 19;
			g_pAudio->PlayEx(EAT_MONSTER,lSoundID,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);	
		}

		//结束五行玄关怪物身上特效
		if (id == g_OtherData.GetWuXingFlag().dwFlagInMonstID)
		{
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_WUXING_FALG_MONSTER_STATE,-1,id));
		}

		switch (pChar->GetRaceNo())
		{
		case 249://无相天魔死亡特效
			g_pMagicCtrl->CreateMagic(MAGICID_MON_TIANMO_DIE_EFFECT,0,id);
			break;
		case 208://迷仙阵机关死亡特效
			if(strnicmp(g_pGameMap->GetMapName(),"MAZE",4) == 0)
			{
				int iX,iY;
				pChar->GetXY(iX,iY);
				g_pMagicCtrl->CreateMagic(MAGICID_JIGUAN_EXPLODE,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(iX,iY));//固定坐标 
			}
			break;
		case 209:////关闭迷仙阵石碑特效
			if(strnicmp(g_pGameMap->GetMapName(),"MAZE",4) == 0)
			{
				int iX,iY;
				pChar->GetXY(iX,iY);
				g_pMagicCtrl->FinishMagicByID(MGGICID_MAZE_SHIBEI);
				g_pMagicCtrl->CreateMagic(MAGICID_MAZE_SHIBEI_DEAD,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(iX,iY));//固定坐标
			}
			break;

		case 276://箭塔
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_TUTENG_EFFECT_TOWER,pChar->GetID()));
			break;

		case 260://反射图腾
		case 261://隐身图腾
		case 262://重力图腾
		case 263://生命图腾
		case 264://诅咒图腾
		case 265://巫毒图腾
		case 266://巫术图腾
		case 267://魔力图腾
		case 268://仇恨图腾
		case 269://箭塔图腾
		case 270://陷阱图腾
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_TUTENG_EFFECT,pChar->GetID()));
			break;
		case 316:
		case 317:
		case 320://护塔机关,慢慢消失		
			{
				SNextAction *pNext = pChar->PushSNextAction(ACTION_APPEAR);
				if (pNext)
				{
					pNext->iData = 5;	
				}
				return;
			}
			break;
		case 310:
		case 311:
		case 323:
		case 324:
		case 325:
		case 326:
			MapArray.DeleteCharacter(pChar->GetID());
			return;
			break;
		case 355:
		case 356:
			{
				int iX,iY;
				pChar->GetXY(iX,iY);
				g_pMagicCtrl->CreateMagic(MAGICID_BOMB_BLAST,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(iX,iY));//固定坐标

				int iR = 1;
				if (pChar->GetRaceNo() == 356)
				{
					iR = 2;
				}
				for (int i = -1 * iR; i <= iR; i++)
				{
					for (int j = -1 * iR; j <= iR; j++)
					{
						if (i == 0 && j == 0)
						{
							continue;
						}

				        g_pMagicCtrl->CreateMagic(MAGICID_BOMB_MONSTER_FIRE,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(iX + i,iY + j));//固定坐标
					}
				}
			}
			break;
		case 480://豹子
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_LEOPARD_EFFECT,pChar->GetID()));
			break;
		case 484://老虎
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_Tiger_SELF_EFFECT,pChar->GetID()));
			break;
		case 481://麒麟
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_QILIN_SELF_EFFECT,pChar->GetID()));
			break;
		case 483://狮子
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_TULION_SELF_EFFECT,pChar->GetID()));
			break;
		case 482://凤凰
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_PHOENIX_SELF_EFFECT,pChar->GetID()));
			break;
		}


		pChar->SetDead(true);
        SNextAction *pNext =  pChar->PushSNextAction(ACTION_DEATH,msg[10],x,y);
		//pChar->SetFightOnLeopard(false);

		if(g_AIAutoMgr.IsEnalbeWaiGua())
		{
			g_AIPromptMgr.DoCharacterDead(id);
			if(id == SELF.GetMyPetID())
			{
				g_AutoPickMgr.PetPickupClear();
				SELF.SetMyPetID(0);
			}
		}

		g_AutoKillMonsterMgr.DelMonster(pChar->GetID());

		if(pNext && iLen > 21 && msg[21] == 1)
			pNext->wDrawAction = ACTION_DEATH2;
    }

	Magic_Show_s *ms = NULL;
	while((ms = g_pMagicCtrl->FindMagicByAll(0,id,-1,EMP_MAGIC_STATE,-1,ms)))
	{
		Magic_Show_s *msTemp = ms->pNext;
		g_pMagicCtrl->FinishMagic(ms);
		ms = msTemp;
		if(!ms)
			break;
	}
}

void CGameControl::MSG_Monster_Hurted(const char * msg,int iLen)
{
    DWORD id = Conv_DWORD(msg);
    WORD hp = Conv_WORD(msg + 6);
    WORD hpMax = Conv_WORD(msg + 8);     
    WORD wHurt = Conv_WORD(msg + 10);
    __int64 looks = Conv_INT64(msg + 12);
    DWORD oid = Conv_DWORD(msg + 24);
    DWORD dwTime = Conv_DWORD(msg + 28);

	CSimpleCharacter * pChar = NULL;
	Magic_Show_s * pMagic = NULL;
	//更新血HP值
	if (id == SELF.GetID())
	{
		pChar = &(SELF);
		SELF.SetHP(hp);
		SELF.SetHPMax(hpMax); 

// 		if (g_TrusteeshipData.IsTrusteeship())//微操或附身时的血量变化不要广播给其它人,本地客户端的数据的也会在该队员广播时设置
// 		{
// 			TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 			if ((g_pSelf == &ORIGINALSELF) || (pSelf && pSelf->byOffLineMode))
// 			{
// 				Send_Trusteeship_HPMP(SELF.GetSelfTrusteeshipPos(),SELF.GetHPMax(), SELF.GetMPMax(), SELF.GetHP(), SELF.GetMP());
// 				TneupMember * pMember = g_TrusteeshipData.GetSelf();
// 				if (pMember)
// 				{
// 					pMember->wMaxHP = SELF.GetHPMax();
// 					pMember->wHP = SELF.GetHP();
// 					pMember->wMaxMP = SELF.GetMPMax();
// 					pMember->wMP = SELF.GetMP();
// 				}
// 			}
// 		}
		//神力或魔力
		//[协议要修改]
		//if(iLen >= 32)
		//{
		//	SELF.SetSpiritPower(Conv_WORD(msg+28));
		//	SELF.SetSpiritPowerMax(Conv_WORD(msg+30));
		//}

		g_AutoKillMonsterMgr.DealWithSelfHurted(oid);
		g_AIPromptMgr.SetAttackerID(oid);
		g_AIAutoMgr.DealWithHurted(hp,hpMax);



		if (!g_pGameData->HasPaoPaoStatus(EP_First_FirstHurt_PaoPao))
		{
			int iPos = SELF.PackageGood().FindGoodByName("金创药(小量)",MAX_PACKAGE_ELEMENT - 6);
			if (iPos > 0)
			{
				iPos = iPos - (MAX_PACKAGE_ELEMENT - 6);
				if (g_EutUiType == EUT_CLASSIC)
				{
					g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_FirstHurt_PaoPao,131+18 + iPos * 41,102-26,XAL_TOPLEFT,false,XAL_BOTTOMLEFT,0,0,10000);
				} 
				else
				{
					g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_FirstHurt_PaoPao,96+18 + iPos * 41,102-26,XAL_TOPLEFT,false,XAL_BOTTOMLEFT,0,0,10000);
				}
			}
		}
	}
	else
	{
		pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar || pChar->IsDead()) 
			return;

		DWORD dwHurtedTime = pChar->GetReserveData(pubHurtedTime);

		if(dwHurtedTime == 0 || dwTime > dwHurtedTime)
		{
			pChar->SetHP(hp);
			pChar->SetHPMax(100);
			pChar->SetReserveData(pubHurtedTime,dwTime);
		}
	}

	if (iLen >= 38 && g_AICfgMgr.IsShowHpChangeEffect())
	{
		int iAddHp = Conv_WORD(msg + 36);
		if (iAddHp != 0)
		{
			Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(MAGICID_POPUP_PAOPAO,0,id,0,-1,0xFFFF0000);
			if (ms)
			{
				ms->byRev1 = 2;
				if (iLen >= 39)
				{
					ms->iRev1 = msg[38];//1:暴击
				}
				ms->iData1 = -iAddHp;
				int iTexW,iTexH;
				int iTexOffX = 0,iTexOffY = 0;
				pChar->GetTexWH(iTexW,iTexH);
				pChar->SetTexOffXY(iTexOffX,iTexOffY);
				ms->iOffX = iTexW / 2 + iTexOffX;
				ms->iOffY = iTexH + iTexOffY - 132;//怪物绘制非透明部分起始位置y - 132 + iTexOffY + iTexH = 怪物脚底y,所有 怪物绘制非透明部分起始位置y = 怪物脚底y - (iTexOffY + iTexH - 132)
			}
		}
	}

	if (oid != SELF.GetID())// 攻击者为CHAR
	{	

		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(oid);
		if( !p || p->IsDead() ) 
		{
			// 没有攻击者，马上让受害者做出被攻击动作
			AddAttacked(id,oid);
			return;
		}

		int iRaceNo = p->GetRaceNo();
// 		if (id == SELF.GetID())
// 		{
// 			TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 			if (pSelf)
// 			{
// 				switch(iRaceNo)
// 				{
// 				case 77:
// 				case 192:
// 				case 198:
// 				case 227:
// 				case 229:
// 				case 197:
// 				case 219:
// 				case 206:
// 				case 233:
// 				case 116:
// 					pSelf->AddAttackObject(oid,true,false);
// 					break;
// 				default:
// 					pSelf->AddAttackObject(oid,false,false);
// 					break;
// 				}
// 			}
// 		}

		switch(iRaceNo)
		{
		case 77:			// 机关巨兽
			{
				int iX,iY;
				if (pChar) pChar->GetXY(iX,iY);
				else SELF.GetXY(iX,iY);
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_STONE_ATTACKED,EMP_MAGIC_NOOWNER,MAKELONG(iX,iY - 6),id);
				if(pMagic)
					pMagic->ref->uOwnerID = oid;
				break;
			}
		case 192:			// 禁地魔王
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_JINDI_FLY_ATTACK,0,oid,id);
				return;
			}
		case 198:			// 通天教主
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_ATTACKED_BY_TONGTIAN,0,id);
				break;
			}
		case 227:         // 魔眼
		case 229:
			{
				return;
			}
		case 197:			// 烈焰使
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_FIREBALL,0,oid,id);//同小火球，不要起手效果
				if (pMagic)
				{
					g_pMagicCtrl->CreateMagic(pMagic);
					g_pMagicCtrl->FinishMagic(pMagic);
				}
				break;
			}

		case 219: // 炎魔的攻击附带效果
		case 206:
			{
				//g_pControl->Msg(MSG_CTRL_STONEATTACKED+0x10000001,id);
				break;
			}
		case 233: //魔神修罗的击中效果
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_MOSHEN_ATTACKED,0,oid,id);
				break;
			}
		case 378://兔女郎击中特效
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_ATTACKED_BY_RABBIT,0,id);
				break;
			}
		}
	}

    if(strnicmp(g_pGameMap->GetMapName(),"wxxg",4) == 0)        //五行玄关怪物受攻击播放特效
    {
        g_pMagicCtrl->CreateMagic(MAGICID_WXXG_MONSTER_HURT,0,0,id);
    }
    else
    {
	    /////////////////////////////如果存在ownerid == oid的魔法，把id加到这个魔法的攻击目标当中,被攻击动作在这个魔法的合适时候(配了EMP_MAGIC_POST_ATTACKED的结点)或是魔法结束时做
	    if(!g_pMagicMgr->AddMagicTarget(id,0,oid))
		    AddAttacked(id,oid);//添加被攻击动作
    }
}

void CGameControl::MSG_Monster_Hurted2(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD hp = Conv_WORD(msg+6);
	WORD hpMax = Conv_WORD(msg+8);
	WORD wHurt = Conv_WORD(msg+10);
	__int64 looks = Conv_INT64(msg+12);
	DWORD dwTime = Conv_DWORD(msg+20);
	DWORD oid = Conv_DWORD(msg+24);
	WORD wTemp = Conv_WORD(msg + 30);//1:被烈火攻击 2:被兽化攻击 3:风火轮 4:被通天教主攻击2,6:被破甲击中 破击  7:被破盾击中,8被强化破击剑法攻击
	CSimpleCharacter * pChar = NULL;

	if(id == SELF.GetID())
	{
		pChar = &(SELF);
		SELF.SetHP(hp);
		SELF.SetHPMax(hpMax); 
		SELF.SetLooks(looks);

// 		if (g_TrusteeshipData.IsTrusteeship())//微操或附身时的血量变化不要广播给其它人,本地客户端的数据的也会在该队员广播时设置
// 		{
// 			TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 			if ((g_pSelf == &ORIGINALSELF) || (pSelf && pSelf->byOffLineMode))
// 			{
// 				Send_Trusteeship_HPMP(SELF.GetSelfTrusteeshipPos(),SELF.GetHPMax(), SELF.GetMPMax(), SELF.GetHP(), SELF.GetMP());
// 				TneupMember * pMember = g_TrusteeshipData.GetSelf();
// 				if (pMember)
// 				{
// 					pMember->wMaxHP = SELF.GetHPMax();
// 					pMember->wHP = SELF.GetHP();
// 					pMember->wMaxMP = SELF.GetMPMax();
// 					pMember->wMP = SELF.GetMP();
// 				}
// 			}
// 		}
		//神力或魔力
		//[协议要修改]
		//if(iLen >= 34)
		//{
		//	SELF.SetSpiritPower(Conv_WORD(msg+32));
		//	SELF.SetSpiritPowerMax(Conv_WORD(msg+34));
		//}

		g_AutoKillMonsterMgr.DealWithSelfHurted(oid);
		g_AIPromptMgr.SetAttackerID(oid);
		g_AIAutoMgr.DealWithHurted(hp,hpMax);

// 		if (id == SELF.GetID())
// 		{
// 			TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 			if (pSelf)
// 			{
// 				switch(wTemp)
// 				{
// 				case 1:
// 				case 2:
// 				case 3:
// 				case 4:
// 					pSelf->AddAttackObject(oid,true,false);
// 					break;
// 				default:
// 					pSelf->AddAttackObject(oid,false,false);
// 					break;
// 				}
// 			}
// 		}
	}
	else
	{
		// 攻击者为CHAR
		pChar = g_pGameData->FindSimpleCharacter(id);
		if(!pChar || pChar->IsDead())
			return;

		pChar->SetHP(hp);
		pChar->SetHPMax(100);
		pChar->SetLooks(looks);
	}


	if (iLen >= 38 && g_AICfgMgr.IsShowHpChangeEffect())
	{
		int iAddHp = Conv_WORD(msg + 36);
		if (iAddHp != 0)
		{
			Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(MAGICID_POPUP_PAOPAO,0,id,0,-1,0xFFFF0000);
			if (ms)
			{
				ms->byRev1 = 2;
				ms->iData1 = -iAddHp;
				if (iLen >= 39)
				{
					ms->iRev1 = msg[38];//1:暴击
				}
				int iTexW,iTexH;
				int iTexOffX = 0,iTexOffY = 0;
				pChar->GetTexWH(iTexW,iTexH);
				pChar->SetTexOffXY(iTexOffX,iTexOffY);
				ms->iOffX = iTexW / 2 + iTexOffX;
				ms->iOffY = iTexH + iTexOffY - 132;//怪物绘制非透明部分起始位置y - 132 + iTexOffY + iTexH = 怪物脚底y,所有 怪物绘制非透明部分起始位置y = 怪物脚底y - (iTexOffY + iTexH - 132)
			}
		}
	}


	//添加被攻击动作
	int iData = 0;
	switch(wTemp)
	{
	case 1:
		iData = MAGICID_ATTACKED_BY_ATTACK_FIRE;
		break;
	case 2:
		iData = MAGICID_ATTACKED_BY_WILD_ATTACK;
		break;
	case 3:
		{
			g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_ROTATE_FIRE,oid));
			g_pMagicCtrl->CreateMagic(MAGICID_ROTATE_FIRE_ATTACKED,0,id);
		}
		return;
	case 4:
		iData = MAGICID_ATTACKED_BY_TONGTIAN2;
		break;
	case 6:
		iData = MAGICID_ATTACKED_BY_DESTROY_SHELL;
		break;
	case 7:
		iData = MAGICID_ATTACKED_BY_DESTROY_SHIELD;
		break;

	default:
		break;
	}

	g_pMagicCtrl->CreateMagic(iData,0,id);
	AddAttacked(id,oid,iData);
}

void CGameControl::MSG_Monster_Disappearance(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	if (id == SELF.GetReserveData(plyAttackLockID))
		SELF.SetReserveData(plyAttackLockID,0);

	if (id == SELF.GetReserveData(plyMagicLockID))
		SELF.SetReserveData(plyMagicLockID,0);

    CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
	if(pChar == NULL)
		return;

	if(id == g_OtherData.GetExploreID())
	{
		g_OtherData.SetExploreID(0);
		return;
	}

	//丛林豹消失
	if(g_AIAutoMgr.IsEnalbeWaiGua())
	{
		g_AIPromptMgr.DoCharacterDisappear(id);
		if(id == SELF.GetMyPetID())
		{
			g_AutoPickMgr.PetPickupClear();
			SELF.SetMyPetID(0);
		}
	}

	g_AutoKillMonsterMgr.DelMonster(pChar->GetID());

	if(pChar && pChar->GetRaceType() == CHARACTER_NPC && (pChar->GetLooksValue() & 0x00FF0000) == 0x00110000)
	{
		SELF.SetReserveData(pubChristmasTreePos,0);
		g_pAudio->StopMidi(1);
	}

    if(!g_pMagicMgr->AddMagicTarget(id,1))
	    MapArray.DeleteCharacter(id);

	if(id == g_NPC.GetID())
	{
		g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
        g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
        g_pControl->PopupWindow(MSG_CTRL_HUANHUA_WND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
	}
	//结束五行玄关怪物身上特效
	if (id == g_OtherData.GetWuXingFlag().dwFlagInMonstID)
	{
		g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_WUXING_FALG_MONSTER_STATE,-1,id));
	}

}

void CGameControl::MSG_Monster_Bone(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD wx = Conv_WORD(msg+6);
	WORD wy = Conv_WORD(msg+8);
	char cDir = msg[10];
	__int64 uLooks = Conv_INT64(msg+12);

	CSimpleCharacterNode * pChar = g_pGameData->AddSimpleCharacter(wx,wy,uID);
	if(pChar)
	{
		g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_DIGMONSTER_DEAD,uID));

		pChar->SetDead(true);
		pChar->SetLooks(uLooks);
		pChar->SetReserveData(pubBoneState,1);
		SNextAction * pNext = pChar->PushSNextAction(ACTION_DEATH,cDir);
		if(pNext && pChar->GetRaceNo() == 236)
			pNext->cDir = 4;

		if(iLen > 28)		// 24字节后是名字以及名字的颜色,具体和000A一样
		{
			string nametemp;
			nametemp.assign(msg+28,iLen-28);
			int i = 0;
			string name;
			name = StringUtil::toStr(nametemp,i);
			pChar->SetName(name.c_str());
			char cColor = nametemp.at(i);
			pChar->SetNameColor(cColor);
		}
	}
}

void CGameControl::MSG_Monster_Corpus(const char * msg,int iLen)
{
	string temp;
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	BYTE bySexAndDigTag = (BYTE)msg[11];
	std::string strtemp;
	int iLenTemp=24;

	CSimpleCharacterNode * pChar = g_pGameData->AddSimpleCharacter(x,y,Conv_DWORD(msg));
	if(!pChar)
		return;
	pChar->SetXY(x,y);
	pChar->SetDir(msg[10]);
	pChar->SetSex(bySexAndDigTag & 0x0F);

	__int64 looks = Conv_INT64(msg + 12);
	pChar->SetLooks(looks);
	pChar->SetStatus(Conv_WORD(msg+22));
	pChar->SetDead(true);
	if(pChar->GetRaceNo() == 236 )
		pChar->SetDir(4);

	pChar->InitAction(ACTION_DEATH);
    SAction& actionNow = pChar->GetAction();
	actionNow.tFrameStart = g_pGfx->GetFrameCount() - 500;//停到最后一帧

	if( iLen > iLenTemp )
	{
		temp.assign(msg + 28,iLen - iLenTemp);
		int i = 0;
		string nametemp;
		nametemp = StringUtil::toStr(temp,i);

		pChar->SetName(nametemp.c_str());
		char cColor = (char)(StringUtil::toInt(temp,i));
		pChar->SetNameColor(cColor);
	}

	if (bySexAndDigTag & 0x10)
	{
		pChar->SetCanDig(true);
	}

	if (pChar->IsCanDig() && !g_pMagicCtrl->FindMagicByAll(MAGICID_DIGMONSTER_DEAD,pChar->GetID()))//可挖的怪物,死亡后放一个特效
	{
		g_pMagicCtrl->CreateMagic(MAGICID_DIGMONSTER_DEAD,0,pChar->GetID(),pChar->GetID()); 
	
		if(!g_pGameData->HasPaoPaoStatus(EP_First_CutZhangShu_PaoPao) && strcmp(pChar->GetName(),"樟树") == 0)
		{
			g_pControl->PopupArrowTip(MSG_CTRL_GAMEWND,EP_First_CutZhangShu_PaoPao,x,y,XAL_TOPLEFT,false,XAL_TOPLEFT,1,pChar->GetID());
		}
	}

}

//地面持续魔法，火墙等，开始
void CGameControl::MSG_Magic_Continue_Begin(const char * msg,int iLen)
{
	DWORD dwMagicID = Conv_DWORD(msg);
	WORD wTemp = Conv_WORD(msg+6);
	int x = Conv_WORD(msg+8);
	int y = Conv_WORD(msg+10);
	DWORD dwColor = 0xFFFFFFFF;
	WORD wType = Conv_WORD(msg+14);
	
	SELF.SetReserveData(plyJumpMap,0);

	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);
	Magic_Show_s *ms = NULL;

	if(iLen >= 20)
		dwColor = *((DWORD*)(msg + 16));
	if(dwColor == 0)
		dwColor = 0xFFFFFFFF;

	switch(wTemp)
	{
	case 21: // 暗域之王 魔焰攻击
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			//g_pGameData->GetOtherSimpleData().SetFireMagicData(x,y,dwID,wTemp,rand()%10);
			if(GetTickCount() - SELF.GetReserveData(plyFireWallTime) >1000)
			{
				SELF.SetReserveData(plyFireWallTime,GetTickCount());
				g_pAudio->PlayEx(EAT_MAGIC,106,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);
			}

			break;
		}
	case 5:// 火墙
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_FIREWALL_LAST,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			if(GetTickCount() - SELF.GetReserveData(plyFireWallTime) >1000)
			{
				SELF.SetReserveData(plyFireWallTime,GetTickCount());
				g_pAudio->PlayEx(EAT_MAGIC,106,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);
			}
			break;
		}
	case 25://火墙(神)
		{
			break;
		}
	case 26://火墙(魔)
		{
			break;
		}
	case 3:				// 矿渣
		ms = g_pMagicCtrl->CreateMagic(MAGICID_SCORIA,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),msg[12],-1,0,0,g_OtherData.GetFireShowList(x,y));//不同形状的矿渣用方向来表示
		break;
	case 1:				// 僵尸2洞
		ms = g_pMagicCtrl->CreateMagic(MAGICID_CORPSE_HOLE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),msg[12],-1,0,0,g_OtherData.GetFireShowList(x,y));//不同形状的洞用方向来表示
		break;
	case 6://逆 魔 序 列
		ms = g_pMagicCtrl->CreateMagic(MAGICID_NIMO_SHELL,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),msg[12]-1,-1,0,0,g_OtherData.GetFireShowList(x,y));//不同形状的用方向来表示
		break;
	case 7://心形玫瑰阵
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_HEART_FLOWER_BLOSSOM,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		}
		break;
	case 8:	// 花烛
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_HAPPY_CANDLE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		}
		break;
	case 9: //雪人
	case 10://骷髅
	case 11://战将
	case 12://大刀
	case 13://弓箭
	case 14://青龙旗
	case 15://白虎旗
	case 16://玄武旗
	case 17://朱雀旗
	case 18:
	case 19:
		{
			ms = g_pMagicCtrl->CreateMagic(2063 + wTemp,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 93://1.908 商城道具	盛宴蛋糕
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 94://寒冰风暴
		{
			//DWORD dwMagicType  = EMP_MAGIC_SERVER_ADVISE;
			//dwMagicType |= EMP_MAGIC_NOTARGET;
			//DWORD dwTargetID = MAKELONG(x,y);
			if (wType == 3)//3x3
			{
				ms = g_pMagicCtrl->CreateMagic(MAGICID_HANBINGFENGBAO_3X3,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
				
			} 
			else//5x5
			{
				ms = g_pMagicCtrl->CreateMagic(MAGICID_HANBINGFENGBAO_5X5,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
				
			}
			break;	
		}
	case 95://警报
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_JINGBAO,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		}
		break;
	case 96://警报2
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_JINGBAO2,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		}
		break;
	case 24://冰红茶
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_ICE_TEA,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 4:// 困魔
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_ONE_LOCK_MOSTER,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			if(GetTickCount() - SELF.GetReserveData(plyLockMonsterTime) >1000)
			{
				SELF.SetReserveData(plyLockMonsterTime,GetTickCount());
				g_pAudio->PlayEx(EAT_MAGIC,82,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y);
			}
			break;
		}
	case 20:// 鞭炮爆炸
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_FIREWORK_BLAST,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 22://1.80版，流星火雨，地面上的持续效果，暂时用火墙
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_FIRE_RAIN_LAST,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 70:
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MOONCAKE_EFFECT,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 71://烽火台特效
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_SIGNAL_FIRE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y),-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
			break;
		}
	case 23://火毒攻心剑
		{
			int iSelfX,iSelfY;
			SELF.GetXY(iSelfX,iSelfY);
			WORD wMagicID = 0;
			if( iSelfX < x ) //目标在主角右边
				wMagicID = MAGICID_DAO_FIGHTER_POISON_RIGHT;
			else if(iSelfX > x )
				wMagicID = MAGICID_DAO_FIGHTER_POISON_LEFT;
			else if(iSelfX == x )
				wMagicID = MAGICID_DAO_FIGHTER_POISON_MIDDLE;

			ms = g_pMagicCtrl->FindMagicByAll(wMagicID,-1,-1,EMP_MAGIC_WAIT_SERVER);
			if (!ms)
			{
				ms = g_pMagicCtrl->FindMagicByAll(wMagicID,-1,-1);
			}

			if(!ms)
			{
				ms = g_pMagicCtrl->CreateMagic(MAGICID_DAO_FIGHTER_POISON_MIDDLE,EMP_MAGIC_SERVER_ADVISE | EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y));
			}
			
			if(ms)
			{
				ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				ms->ref->wOriginTileX = x;
				ms->ref->wOriginTileY = y;
				ms->ref->wTargetTileX = x;
				ms->ref->wTargetTileY = y;
			}

		}
	case 97://剧毒沼泽,没有处理特效
		{
			;
		}
		break;
	default:
		
		//g_pGameData->GetOtherSimpleData().SetFireMagicData(x,y,dwID,wTemp,0);
		break; 
	}

	if(ms)
	{
		ms->ref->create_id = dwMagicID;	
		ms->dwColor = dwColor;
	}
}

//地面持续魔法，火墙等，结束
void CGameControl::MSG_Magic_Continue_End(const char * msg,int iLen)
{
	DWORD dwID = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 8);
	WORD y = Conv_WORD(msg + 10);

	if(!g_OtherData.ClearFireShowList(x,y))
	{
		Magic_Show_s *ms = NULL;
		while((ms = g_pMagicCtrl->FindMagicByPos(x,y,false,dwID,-1,ms)))
		{
			if((ms->attr & EMP_MAGIC_LOOP) || ms->iCycles >= 1)
			{
				ms->attr &= ~EMP_MAGIC_LOOP;
				ms->iCycles = 0;
				ms->iCurCycle = 0;
			}
			else
			{
				Magic_Show_s *msTemp = ms->pNext;
				g_pMagicCtrl->FinishMagic(ms);
				ms = msTemp;
				if(!ms)
					break;
			}
		}
	}


}

//队长死亡后变成灵珠
void CGameControl::MSG_Monster_Change_Ball(const char * msg,int iLen)
{
	DWORD dwID = Conv_DWORD(msg);
	WORD wState = Conv_WORD(msg + 6);


	CSimpleCharacter* pChar = g_pGameData->FindCharacterByID(dwID);
	if(pChar)
	{
// 		if(wState == 1)
// 		{
// 			if (dwID == SELF.GetID())
// 			{
// 				//弹出选新队长窗口
// 				DWORD dType = 0x0200 | SELF.GetEquipGood(ITEM_TNEUPCHART).GetAC();
// 				g_pControl->PopupWindow(MSG_CTRL_TNEUP_WND,OPER_CREATE,dType);
// 				SELF.GetCNextAction().Clear();
// 			}
// 
// 			g_pMagicCtrl->CreateMagic(MAGICID_MONSTER_CHANGEBALL,0,dwID,0);
// 			pChar->SetBianShenLooks(0x0000000064000B);//黄
// 		}
// 		else
		{
			pChar->SetBianShenLooks(0);
		}
	}
}

// 怪物的动画出现或者战士的兽化
void CGameControl::MSG_Monster_Animate_Appear(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD wx = Conv_WORD(msg+6);
	WORD wy = Conv_WORD(msg+8);
	char cDir = msg[10];
	__int64 iLook = Conv_INT64(msg+12);
	WORD wJob = Conv_WORD(msg+28);		// 职业必须为1(战士)
	WORD wState = Conv_WORD(msg+30);	// 兽化状态0,1

	int monster_look  = (int)(( iLook >> 16 ) & 0x000000000000FFFF);



	if(wJob == MAGICID_FIGHTER_WILD || wJob == MAGICID_TAOIST_WILD)		// 兽化,1:战士 3:道士
	{	
		if(uID == SELF.GetID())
		{
			SNextAction * pNext = SELF.PushSNextAction(ACTION_MAGIC,cDir,wx,wy);
			if (pNext)
			{
				pNext->iData = wJob;
				pNext->iLooks = iLook;

				if(SELF.GetAction().wAction == ACTION_STAND)
					g_pGameMgr->PlayerDoNextStep();
			}			
		}
		else
		{
			CSimpleCharacterNode * pChar = g_pGameData->AddSimpleCharacter(wx,wy,uID);
			if(!pChar)
			{
				return;
			}
			SNextAction * pNext = pChar->PushSNextAction(ACTION_MAGIC,cDir,wx,wy);
			if (pNext)
			{
				pNext->iData = wJob;
				pNext->iLooks = iLook;
			}

		}
	}
	else							// 怪物的动画出现
	{
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(uID);
		if(pChar)
		{
			SNextAction * pNext = pChar->PushSNextAction(ACTION_APPEAR);
			if( !pNext ) 
				return;	
			pNext->iData = 10;
			pNext->iLooks = iLook;
			pChar = g_pGameData->AddSimpleCharacter(wx,wy,uID);
		}
		else
		{
			pChar = g_pGameData->AddSimpleCharacter(wx,wy,uID);

			//....................  幽影武士出现效果 ........................
			if(monster_look == 220)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_SHADOW_MONSTER_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			//....................  幽影武士2 出现效果 ........................
			else if (monster_look == 225)
			{
				if(cDir == 3)
					g_pMagicCtrl->CreateMagic(MAGICID_SHADOW_MONSTER2_DIR3_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				else
					g_pMagicCtrl->CreateMagic(MAGICID_SHADOW_MONSTER2_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			//.................... 修罗王神 出现效果 ........................
			else if (monster_look == 235)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_XIULUO_KING_MONSTER_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			//禁地魔王出现效果
			else if (monster_look == 236)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_JINDI_MONSTER_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			//.................... 元神怪物 出现效果 ........................
			else if (monster_look == 247)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_YUANSHEN_MONSTER_APPEAR,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			else if (monster_look == 71)//327 道士套装技能召唤出来的宝宝碧血魔,但协议是用一个byte表示,所以变成71了,71铜人正好没有这个属性,先这样用了,以后改协议
			{
				g_pMagicCtrl->CreateMagic(MAGICID_JYL_MONSTER,0,uID);
				return;
			}
			//....................  圣殿 骑兵 出现效果 ........................

			else if (monster_look == 385)
			{
				if(cDir == 3)
					g_pMagicCtrl->CreateMagic(MAGICID_QIBING_APPEAR_LEFT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				else
					g_pMagicCtrl->CreateMagic(MAGICID_QIBING_APPEAR_RIGHT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
				return;
			}
			else if (monster_look == 400 || monster_look == 402 || monster_look == 404)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_LUOJIAN_BIG_LEFT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
			}
			else if (monster_look == 401 || monster_look == 403 || monster_look == 405)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_LUOJIAN_BIG_RIGHT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
			}
			else if (monster_look == 406 || monster_look == 408)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_LUOJIAN_SMALL_LEFT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
			}
			else if (monster_look == 407 || monster_look == 409)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_LUOJIAN_SMALL_RIGHT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,uID,MAKELONG(wx,wy));
			}

			if(pChar)
			{
				pChar->SetLooks(iLook);

				pChar->SetXY(wx,wy);
				pChar->AddMagicState(MS_ANIMAPPEAR);

				pChar->InitAction(ACTION_APPEAR);
			}
		}

		// 设置血条长度
		switch(pChar->GetRaceNo())
		{
			case 88:  pChar->SetShowHP(80); break;	// 逆魔
			case 132://逆魔出现特效
				g_pMagicCtrl->CreateMagic(MAGICID_NIMO_BREAK,0,pChar->GetID());
				break;
			case 133:
				g_pMagicCtrl->CreateMagic(MAGICID_NIMO_BREAK2,0,pChar->GetID());
				break;
			case 229: pChar->SetShowHP(80); break;  // 逆魔之王
			case 220: pChar->SetShowHP(80); break; // 幽影武士
			case 221: pChar->SetShowHP(100);break; // 暗域之王
			case 249: pChar->SetShowHP(100);break; // 无相天魔
		}
        
		UINT lSoundID = (UINT)(((iLook & 0x00FF0000) >> 13) + 16);
		int iSelfX,iSelfY;
		SELF.GetXY(iSelfX,iSelfY);
		
		if(pChar && pChar->IsMonster())
		{
			g_pAudio->PlayEx(EAT_MONSTER,lSoundID,g_pAudio->GetRand()++,iSelfX,wx,iSelfY,wy);
		}
	}
}

// 怪物的动画消失
void CGameControl::MSG_Monster_Animate_Disappear(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(id);
	if(pChar)
	{
		int iAction = ACTION_APPEAR;
		int iRaceNo = pChar->GetRaceNo();

		SNextAction *pNext = pChar->PushSNextAction(iAction);
		__int64 iLook = pChar->GetLooksValue();
		//特殊怪特殊处理
		switch((iLook & 0x00FF0000)>>16)
		{
		case 0x6F:								// 大麒麟变为小麒麟
			{
				if (iRaceNo == 132)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_NIMO_BREAK,0,pChar->GetID());
				}
				else if (iRaceNo == 133)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_NIMO_BREAK2,0,pChar->GetID());
				}
				pChar->SetLooks((iLook&0xFF00FFFF)|0x006E0000);
				break;
			}
		default:// 怪物动画消失,如食人花装入地下
			{
				if (pNext)
				{
					pNext->iData = 5;	
				}
				break;
			}
		}
	}
	else
		MapArray.DeleteCharacter(id);

}

// NPC怪物刷新
void CGameControl::MSG_Monster_Refresh(const char * msg,int iLen)
{
	// 传世解包
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	DWORD id  = lpPacketMsg->stDefMsg.nRecog;
	//int m_nState = MAKELONG(lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag);

	FEATURE stFeature;
	LONG nFeature	  = MAKELONG(lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag);
	memcpy(&stFeature, &nFeature, sizeof(LONG));

	bool bIsOnHorse = true;
	SAction* pAction = NULL;
	if(SELF.GetID() == id){		//自己
		SELF.SetLooks(nFeature);
		SELF.SetSex(HIBYTE(lpPacketMsg->stDefMsg.wSeries));
		SELF.SetDir(LOBYTE(lpPacketMsg->stDefMsg.wSeries));
		if (!SELF.IsOnHorse())
		{
			SELF.SetFightOnLeopard(false);
			SELF.SetRunStepOnLeopard(3);
			bIsOnHorse = false;
		}

		pAction = &(SELF.GetAction());
	}else
	{
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
		if(pChar == NULL)
			return;
		pChar->SetLooks(nFeature);
		pChar->SetSex(HIBYTE(lpPacketMsg->stDefMsg.wSeries));
		pChar->SetDir(LOBYTE(lpPacketMsg->stDefMsg.wSeries));
		if (!pChar->IsOnHorse())
		{
			pChar->SetFightOnLeopard(false);
			pChar->SetRunStepOnLeopard(3);	
			bIsOnHorse = false;
		}
		pAction = &(pChar->GetAction());
	}

	/* 无双错误的解包
	DWORD id = Conv_DWORD(msg);	//4	Integer(ProcessMsg.BaseObject)
	char sex = msg[11];		//
	__int64 iLooks = Conv_INT64(msg + 12);		
	SAction* pAction = NULL;
	bool bIsOnHorse = true;
	//int iPutOnShield = *((BYTE*)(msg + 21)) & 0x3;
	//int iGuildFlag = (*((byte*)(msg + 21)) >> 2) & 0xf;		//服务器无法发送自己的行会旗帜
	BYTE byTemp = (BYTE)msg[21];//盾牌类型

	if(SELF.GetID() == id)
	{
		SELF.SetLooks(iLooks);
		if (!SELF.IsOnHorse())
		{
			SELF.SetFightOnLeopard(false);
			SELF.SetRunStepOnLeopard(3);
			bIsOnHorse = false;
		}

		//SELF.SetPutOnShield(iPutOnShield);
		if (byTemp > 0)
		{
			SELF.SetShield(byTemp);
		}
		else
		{
			SELF.SetShield(0);
		}

		pAction = &(SELF.GetAction());
		
		//[协议要修改]
		//if(iLen >= 24)
		//{
		//	int iOldFlyType = SELF.GetFlyType();
		//	SELF.SetFlyType((BYTE)msg[22]);
		SELF.SetSanWeiFireLevel((BYTE)msg[22]);
		//	SELF.SetFlyLevel((BYTE)msg[23]);
		//	if(iOldFlyType != SELF.GetFlyType() && SELF.GetFlyType() != 0)
		//	{
		//		g_pControl->Msg(MSG_CTRL_FLYGOD,0);
		//		if(g_pControl->FindWindowByName("AvatarWnd"))
		//			g_pControl->Msg(MSG_CTRL_CHARWND,OPER_RECREATE);
		//	}
		//}	
		if (iLen >= 25)
		{
// 			//第一个bit表示是否托管,非零表示已经托管
// 			BYTE byTrusteeshipFlag = ((BYTE)msg[24]) & 0x01;
// 			if (byTrusteeshipFlag > 0)
// 			{
// 				SELF.SetTrusteeshipFlag(1);
// 			}
// 			else
// 			{
// 				SELF.SetTrusteeshipFlag(0);
// 			}
		}
		
		if (iLen >= 26)
		{
			SELF.SetLiveTokenNumber((BYTE)msg[25]);
		}
		if (iLen >= 27)
		{
			SELF.SetTypeBufferQQ((BYTE)msg[26]);
		}
		if (iLen >= 29)//法宝类型
		{			
			SELF.SetFaBaoType((BYTE)msg[27],(BYTE)msg[28]);
		}
		if (iLen >= 30)//vip商行等级
		{			
			SELF.SetVipTradeLevel((BYTE)msg[29]);
		}
		if (iLen >= 31)
		{
			BYTE daodun = (BYTE)msg[30];
			if (daodun == 0)
			{
				SELF.Set8DunState(false);
				SELF.SetDaoZunJiangLinState(false);
			}
			else if (daodun == 1)
			{
				SELF.Set8DunState(true);
				SELF.SetDaoZunJiangLinState(false);
			}
			else if (daodun == 2)
			{
				SELF.Set8DunState(false);
				SELF.SetDaoZunJiangLinState(true);
			}
		}
		//if (iLen >= 34)//法阵
		//{		
		//	BYTE leftFabao = (BYTE)msg[32];
		//	SELF.SetLeftFaBaoType(leftFabao);
		//	BYTE rightFabao = (BYTE)msg[33];
		//	SELF.SetRightFaBaoType(rightFabao);

		//	SELF.StartFaZhen(msg[31],msg[32],msg[33]);
		//}

		//if (iLen >= 35)
		//{
		//	BYTE byWingColor = (BYTE)msg[34];
		//	if (byWingColor > 0)
		//	{
		//		SELF.SetCharHolyWingStrongLevel( (int)(byWingColor & 0xF) );
		//		SELF.SetCharHolyWingLevel( (int)((byWingColor >> 4) & 0xF) );
		//	}
		//	else
		//	{
		//		SELF.SetCharHolyWingLevel( -1 );
		//		SELF.SetCharHolyWingStrongLevel( -1 );
		//	}
		//}

		//if (iLen >= 36)
		//{
		//	SELF.Set12GongTitle((BYTE)msg[35]);
		//}
	}
	else
	{
		// 这里有错误,所有的NPC全部刷成了怪物
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
		if(pChar == NULL)
			return;

		pChar->SetSex(sex);
		pChar->SetLooks(iLooks);
		if (!pChar->IsOnHorse())
		{
			pChar->SetFightOnLeopard(false);
			pChar->SetRunStepOnLeopard(3);	
			bIsOnHorse = false;
		}

		//pChar->SetPutOnShield(iPutOnShield);
		//pChar->SetGuildFlag(iGuildFlag);
		if (byTemp > 0)
		{
			pChar->SetShield(byTemp);
		}
		else
		{
			pChar->SetShield(0);
		}

		pAction = &(pChar->GetAction());
		//[协议要修改]
		//if(iLen >= 24)
		//{
		//	pChar->SetFlyType((BYTE)msg[22]);
		pChar->SetSanWeiFireLevel((BYTE)msg[22]);
		//	pChar->SetFlyLevel((BYTE)msg[23]);
		//}	
		//第一个bit表示是否托管,非零表示已经托管

		if (iLen >= 25)
		{
// 			BYTE byTrusteeshipFlag = ((BYTE)msg[24]) & 0x01;
// 			if (byTrusteeshipFlag > 0)
// 			{
// 				if (g_TrusteeshipData.FindMemberPosByID(pChar->GetID()) >= 0)
// 				{
// 					pChar->SetTrusteeshipFlag(1);
// 				}
// 				else
// 				{
// 					pChar->SetTrusteeshipFlag(2);
// 				}
// 			}
// 			else
// 			{
// 				pChar->SetTrusteeshipFlag(0);
// 			}

		}
		if (iLen >= 26)
		{
			pChar->SetLiveTokenNumber((BYTE)msg[25]);
		}
		if (iLen >= 27)
		{
			pChar->SetTypeBufferQQ((BYTE)msg[26]);
		}
		if (iLen >= 29)//法宝类型,等级
		{			
			pChar->SetFaBaoType((BYTE)msg[27],(BYTE)msg[28]);
		}
		if (iLen >= 30)//vip商行等级
		{			
			pChar->SetVipTradeLevel((BYTE)msg[29]);
		}
		if (iLen >= 31)
		{
			BYTE daodun = (BYTE)msg[30];
			if (daodun == 0)
			{
				pChar->Set8DunState(false);
				pChar->SetDaoZunJiangLinState(false);
			}
			else if (daodun == 1)
			{
				pChar->Set8DunState(true);
				pChar->SetDaoZunJiangLinState(false);
			}
			else if (daodun == 2)
			{
				pChar->Set8DunState(false);
				pChar->SetDaoZunJiangLinState(true);
			}
		}

		//if (iLen >= 34)//法阵
		//{			
		//	BYTE zhuFabao = pChar->GetFaBaoType()->dwCurFaBaoType;
		//	BYTE leftFabao = (BYTE)msg[32];
		//	BYTE rightFabao = (BYTE)msg[33];

		//	pChar->StartFaZhen(msg[31],msg[32],msg[33]);
		//}

		//if (iLen >= 35)
		//{
		//	BYTE byWingColor = (BYTE)msg[34];
		//	if (byWingColor > 0)
		//	{
		//		pChar->SetCharHolyWingStrongLevel( (int)(byWingColor & 0xF) );
		//		pChar->SetCharHolyWingLevel( (int)((byWingColor >> 4) & 0xF) );
		//	}
		//	else
		//	{
		//		pChar->SetCharHolyWingLevel( -1 );
		//		pChar->SetCharHolyWingStrongLevel( -1 );
		//	}
		//}

		//if (iLen >= 36)
		//{
		//	pChar->Set12GongTitle((BYTE)msg[35]);
		//}
	}
	*/
	
	if(pAction->wAction == ACTION_STAND && !bIsOnHorse)
	{
		pAction->wDrawAction = g_ActionInf.ConvertAction(pAction->wAction,APT_NORMAL);
	}

}

// 僵尸复活
void CGameControl::MSG_Corpse_Relive(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg+6);
	WORD y = Conv_WORD(msg+8);
	__int64 iLook = Conv_INT64(msg+12);

	CSimpleCharacterNode * pChar = g_pGameData->AddSimpleCharacter(x,y,uID);
	if(pChar)
	{
		pChar->SetDead(false);
		pChar->SetLooks(iLook);
		pChar->SetStatus(Conv_WORD(msg + 22));
		

		if(iLen == 24)
		{
			WORD wTemp = Conv_WORD(msg+24);
			pChar->SetHP(wTemp);
			wTemp = Conv_WORD(msg+26);
			pChar->SetHPMax(100);
		}

		pChar->InitAction(ACTION_APPEAR);
	}
}

void CGameControl::MSG_Monster_Attack_Sword(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);
	WORD y = Conv_WORD(msg + 8);
	char dir = msg[10];
	char byMagicLevel = msg[11];

	WORD wMagicID = Conv_WORD(msg+12);

	CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(id);
	if(!pChar)
		return;

	SNextAction* pNext = pChar->PushSNextAction(ACTION_ATTACK1,dir,x,y);
	if(pNext)
	{
		pNext->uFlag       |= MAGIC_ACTION;
		pNext->iData        = wMagicID;

		if(iLen >= 18)
		{
			DWORD dwColor = Conv_DWORD(msg + 14);
			if(dwColor != 0)
				pNext->dwColor = dwColor;
		}

		pNext->iData = wMagicID;
		if (byMagicLevel > 10)
		{
			/*BYTE byFlyType = pChar->GetFlyType();
			if (byFlyType == 0)
			{
			byFlyType = byMagicLevel / 10;
			}*/
			pNext->iData = GetGreateMagicID(wMagicID,1,byMagicLevel % 10,pChar->IsInDaoZunJiangLinState(),pChar->GetFightOnLeopard());
		}
	}
}

void CGameControl::MSG_Monster_Protector_Color(const char * msg,int iLen)
{
	DWORD dwColor = g_pGameData->GetMirColor(msg[10]);

	CCharacterAttr* p = g_pGameData->FindCharacterByID(Conv_DWORD(msg));
	if(p)
	{
		p->SetReserveData(pubHushenColor,dwColor);
		p->SetReserveData(pubHushenHP,Conv_DWORD(msg+6));
		WORD wMagicID = MAGICID_PROTECT_SKIN;

		Magic_Show_s* pshow = g_pMagicCtrl->FindMagicByAll(wMagicID,p->GetID());
		if (pshow)
			pshow->dwColor = dwColor;
	}
}

void CGameControl::MSG_Monster_SoulWall_State(const char * msg,int iLen)
{
	UINT uID;
	char cTemp;

	memcpy(&uID,msg,4);
	memcpy(&cTemp,msg+6,1);

	CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(uID);
	if(pChar)
	{
		WORD wState = pChar->GetStatus();
		if(cTemp)	// 消失
		{ 
			wState &= ~CS_SOULWALL;
			pChar->SetStatus(wState);
			pChar->SetReserveData(pubSoulWallState,2);
			pChar->SetReserveData(pubSoulWallFrame,0);
		}
        else	// 出现
		{
			pChar->SetStatus(wState | CS_SOULWALL);
			if(iLen >= 16)
			{
				DWORD dwColor = *(DWORD*)(msg + 12);
			}

			pChar->SetReserveData(pubSoulWallState,1);
			pChar->SetReserveData(pubSoulWallFrame,0);
			if(iLen >= 16)
				pChar->SetReserveData(pubSoulWallColor,*(DWORD*)(msg + 12));
		}
	}

}

//怪物使用的魔法，如怪物使用治疗等，并且做出施法的动作。。。
void CGameControl::MSG_Monster_Magic_Effect(const char* msg,int iLen)
{
	if(iLen<22)
		return;

	DWORD oid = 0;
	DWORD tid = 0;
	WORD  dir = 0;
	WORD  tx = 0,ty = 0;
	WORD  wMagicID = 0;

	memcpy(&oid,msg,4);
	memcpy(&tid,msg+12,4);
	memcpy(&dir,msg+10,2);
	memcpy(&tx,msg+16,2);
	memcpy(&ty,msg+18,2);
	memcpy(&wMagicID,msg+20,2);


	int ox = 0,oy = 0;
	int offx = 0,offy = 0;
	WORD wAction = ACTION_MAGIC;
	if(wMagicID >= MAGICID_SWT && wMagicID <= MAGICID_JYL)
	{
		wAction = ACTION_SWT + wMagicID - MAGICID_SWT;
	}

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(oid);
	//自己ID
	if(oid == SELF.GetID())
	{
		SELF.InitAction(wAction);
		SAction& sAction = SELF.GetAction();

		sAction.iAimX = tx;
		sAction.iAimY = ty;
		sAction.uData = tid;

		SELF.GetXY(ox,oy);
		SELF.GetOffset(offx,offy);
		SELF.SetDir((BYTE)dir);
	}
	else
	{
		if( !p ) return;
		if( p->IsNpc())
		{
			//SAction& saction = p->GetAction();
			//saction.SetNpcRandStand(p->GetRaceNo());
			
			p->SetDir((BYTE)dir);
			return;
		}

		if(!p->IsHuman())
			wAction = ACTION_ATTACK1;

		SNextAction* pNextAction = p->InsertSNextAction(wAction,(BYTE)dir,tx,ty);
		if( !pNextAction )
			return;

		pNextAction->uData = tid;
		p->GetXY(ox,oy);
	}

// 	if (tid == SELF.GetID())
// 	{
// 		//被施加魔法对象是自己
// 		if (p && p->IsHuman())
// 		{
// 			if (g_TrusteeshipData.IsAttackSkill(p->GetCareer(),wMagicID))
// 			{//魔法攻击				
// 				TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 				if (pSelf)
// 				{
// 					pSelf->AddAttackObject(oid,true,false);
// 				}
// 			}
// 		}
// 	}

	if(wMagicID == 4670 || wMagicID == 4671)
	{
		offx = offy = 0;
	}

	Magic_Show_s* ms = 	g_pMagicCtrl->CreateMagic(wMagicID,EMP_MAGIC_SERVER_ADVISE,oid,tid,-1,0xFFFFFFFF,offx,offy);
	if(ms)
	{
        ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
		ms->ref->wMagicID = wMagicID;
		ms->ref->uOwnerID = oid;
		if(tid != 0)
		{
			ms->ref->uTargetID = tid;
		}

		ms->ref->wOriginTileX = ox;
		ms->ref->wOriginTileY = oy;
		ms->ref->wTargetTileX = tx;
		ms->ref->wTargetTileY = ty;
	}
}


void CGameControl::MSG_Object_Appearance(const char * msg,int iLen)
{
	DWORD id= Conv_DWORD(msg);
	int   x	= Conv_WORD(msg + 6);
	int   y	= Conv_WORD(msg + 8);
	int iSelfX,iSelfY; 
	SELF.GetXY(iSelfX,iSelfY);

	CSimpleGood * pGood = MapArray.AddGood(x,y,id);
	if(pGood)
	{
		pGood->FromBuffer(msg,iLen);
		g_AutoPickMgr.ItemAppear(pGood);

		if(GetTickCount() - g_OtherData.GetLastMovePosTime() > 3000 && abs(iSelfX - x) <= 8 && abs(iSelfY - y) <= 9)
		{
			Magic_Show_s * ms = g_pMagicCtrl->CreateMagic(MAGICID_FALL_OBJECT,EMP_MAGIC_NOTARGET|EMP_MAGIC_NOOWNER,id);
			if(ms)
			{
				ms->tFrameStart     = GetTickCount() + rand()%300;
				ms->iData2			= x;
				ms->iData3			= y;
				ms->ref->uOwnerID   = id;
				ms->ref->uTargetID  = pGood->GetLooks();
				pGood->SetLooks(0);
				if(y < iSelfY)
				{
					ms->attr |= EMP_MAGIC_FLOOR;
					ms->attr &= ~EMP_MAGIC_AIR;
				}
				else
				{
					ms->attr |= EMP_MAGIC_AIR;
					ms->attr &= ~EMP_MAGIC_FLOOR;
				}
			}
		}

		if(!g_pGameData->HasPaoPaoStatus(EP_First_WatchGood_Panel_PaoPao))
		{
			g_pControl->PopupArrowTip(MSG_CTRL_GAMEWND,EP_First_WatchGood_Panel_PaoPao,x,y,XAL_TOPLEFT,false,XAL_TOPLEFT,2,pGood->GetID(),5000);
		}
	}
}

void CGameControl::MSG_Object_Disappearance(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	g_AutoPickMgr.ItemDisappear(id);
	MapArray.DeleteGood(id);
}

void CGameControl::MSG_HeiYanMo_Refraction(const char * msg,int iLen)
{
	DWORD oid= Conv_DWORD(msg);
	WORD wCount = Conv_WORD(msg + 6);


	Magic_Show_s *pNew = g_pMagicCtrl->CreateMagic(MAGICID_HEIYANMO_REFRACTION,0,oid,Conv_DWORD(msg + 12));
	if(pNew)
	{
		if( wCount > 1)
		{
			char *pDynData = new char[(wCount-1)*4 + 2];    
			memcpy(pDynData + 2,msg + 12 + 4,(wCount-1)*4);

			pDynData[0] = 0;
			pDynData[1] = (char)wCount - 1;

			if(pNew->ref)
				pNew->ref->pDynData = pDynData;
			else
				SAFE_DELETE_ARRAY(pDynData);
		}
	}
}
//0-3 玩家指针,4-5 协议号,6-7 数目,8-9 魔法编号,10-11 玩家方向,怪物指针
void CGameControl::MSG_Quick_TG_Monster_List(const char * msg,int iLen)
{
	DWORD dwOwnerID = Conv_DWORD(msg);
	WORD wCount = Conv_WORD(msg + 6);
	if (wCount > 200)
	{
		throw exception("MSG_Quick_TG_Monster_List,怪物数量错误");
		return;
	}

	if (iLen - 12 != wCount * sizeof(DWORD))
	{
		return;
	}

	//自己ID
	if(dwOwnerID == SELF.GetID())
	{
		SELF.SetDir(msg[10]);
		SELF.InitAction(ACTION_MAGIC);
		SAction& sAction = SELF.GetAction();
	}
	else
	{
		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(dwOwnerID);
		if( !p )
			return;

		p->SetDir(msg[10]);
		p->InitAction(ACTION_MAGIC);
	}

	DWORD utid = 0;
	for (int i = 0; i < wCount; i++)
	{
		utid = Conv_DWORD(msg + 12 + i * 4);
		Magic_Show_s* ms = 	g_pMagicCtrl->CreateMagic(MAGICID_PROTECT_SYMBOL,EMP_MAGIC_SERVER_ADVISE,dwOwnerID,utid);
		if(ms)
		{
			ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
			ms->ref->wMagicID = MAGICID_PROTECT_SYMBOL;
			ms->ref->uOwnerID = dwOwnerID;
			if(utid != 0)
			{
				ms->ref->uTargetID = utid;
			}
		}

		//g_pMagicCtrl->CreateMagic(MAGICID_QUICK_TG_MAGIC,0,dwOwnerID,Conv_DWORD(msg + 12 + i * 4));
	}

}

void CGameControl::MSG_NPC_LOOK_CHANGE(const char * msg,int iLen)
{
	DWORD dwNpcID = Conv_DWORD(msg);
	CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(dwNpcID);
	if(pChar)
	{
		WORD wOld = pChar->GetLooks().Char.wReserve;
		pChar->GetLooks().Char.wReserve = ((wOld & 0xFF00))  | ((WORD)(Conv_BYTE(msg + 6)));
	}
}
//从第一个bit开始分别表示金木土水火链是否连接
void CGameControl::MSG_Monster_Hdnw_Lock(const char * msg,int iLen)
{
	DWORD dwMonsterID = Conv_DWORD(msg);
	g_OtherData.SetLockHunDunNvWangLockID(dwMonsterID);

	DWORD dwLock = Conv_DWORD(msg + 6);
	g_OtherData.SetHunDunNvWangLock(dwLock);


	CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(dwMonsterID);
	if(pChar)
	{
		vector<Magic_Show_s*> VMs;
		Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_JIN,-1,dwMonsterID);
		if ((dwLock & 0x00000001) == 0)//金
		{
			VMs.push_back(ms);
			while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_JIN,-1,dwMonsterID,-1,-1,ms))
			{
				VMs.push_back(ms);
			}

			for (size_t i = 0; i < VMs.size(); i++)
			{
				g_pMagicCtrl->CreateMagic(VMs[i]);
				g_pMagicCtrl->FinishMagic(VMs[i]);
			}

			VMs.clear();
		}
		else  if (!ms)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_JIN,0,0,dwMonsterID);
		}

		ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_MU,-1,dwMonsterID);
		if ((dwLock & 0x00000002) == 0)//木
		{
			VMs.push_back(ms);
			while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_MU,-1,dwMonsterID,-1,-1,ms))
			{
				VMs.push_back(ms);
			}

			for (size_t i = 0; i < VMs.size(); i++)
			{
				g_pMagicCtrl->CreateMagic(VMs[i]);
				g_pMagicCtrl->FinishMagic(VMs[i]);
			}

			VMs.clear();
		}
		else  if (!ms)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_MU,0,0,dwMonsterID);
		}

		ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_TU,-1,dwMonsterID);
		if ((dwLock & 0x00000004) == 0)//土
		{
			VMs.push_back(ms);
			while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_TU,-1,dwMonsterID,-1,-1,ms))
			{
				VMs.push_back(ms);
			}

			for (size_t i = 0; i < VMs.size(); i++)
			{
				g_pMagicCtrl->CreateMagic(VMs[i]);
				g_pMagicCtrl->FinishMagic(VMs[i]);
			}

			VMs.clear();

		}
		else  if (!ms)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_TU,0,0,dwMonsterID);
		}

		ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_SHUI,-1,dwMonsterID);
		if ((dwLock & 0x00000008) == 0)//水
		{
			VMs.push_back(ms);
			while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_SHUI,-1,dwMonsterID,-1,-1,ms))
			{
				VMs.push_back(ms);
			}

			for (size_t i = 0; i < VMs.size(); i++)
			{
				g_pMagicCtrl->CreateMagic(VMs[i]);
				g_pMagicCtrl->FinishMagic(VMs[i]);
			}

			VMs.clear();
		}
		else  if (!ms)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_SHUI,0,0,dwMonsterID);
		}

		ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_HUO,-1,dwMonsterID);
		if ((dwLock & 0x00000010) == 0)//火
		{
			VMs.push_back(ms);
			while (ms = g_pMagicCtrl->FindMagicByAll(MAGICID_HDNW_LOCK_HUO,-1,dwMonsterID,-1,-1,ms))
			{
				VMs.push_back(ms);
			}

			for (size_t i = 0; i < VMs.size(); i++)
			{
				g_pMagicCtrl->CreateMagic(VMs[i]);
				g_pMagicCtrl->FinishMagic(VMs[i]);
			}

			VMs.clear();
		}
		else  if (!ms)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_HDNW_LOCK_HUO,0,0,dwMonsterID);
		}
	}

}
