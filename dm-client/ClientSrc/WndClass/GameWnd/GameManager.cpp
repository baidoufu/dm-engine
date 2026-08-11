#include "GameManager.h"
#include "GameData/GameData.h" 
#include "GameData/MapArray.h"
#include "BaseClass/Misc/Input.h"
#include "BaseClass/Control/Control.h"
#include "GSceneDefine.h"
#include "GameControl/GameControl.h"
#include "GameMap/GameMap.h"
#include "GameData/GameGlobal.h"
#include "GameData/MagicDefine.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameAI/AIMgr.h"
#include "GameAI/AIConfigMgr.h"
#include "GameAI/AIGoodMgr.h"
#include "GameData/TalkMgr.h"
#include "GameData/OtherData.h"
#include "Global/Interface/AudioInterface.h"
#include "BaseClass/Control/GoodGrid.h"
#include "Baseclass/Control/ParserTip.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameAI/AIAutoMgr.h"
#include "GameAI/AIAutoFightMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameData/RangeStruct.h"
#include "GameData/NpcData.h"
#include "GameData/TaskData.h"
#include "GameData/ConfigData.h"
#include "GameAI/AutoKillMonsterMgr.h"

#include "GSceneManager.h"

CGameManager* g_pGameMgr = NULL; //游戏控制管理器

CGameManager::CGameManager(void)
{
	g_pGameMgr = this;
	m_dwFrameCount = 0;	
	m_bMouseDClick = false;
	m_bCannotMove = false;
	m_dwAutoDigID = 0;
	m_dwActionStartTime = 0;
}

CGameManager::~CGameManager(void)
{
	g_pGameMgr = NULL;
}

void CGameManager::Update(bool bIsFocus)
{
	m_dwFrameCount = g_pGfx->GetFrameCount();

	g_pControl->GetMouseXY(m_iMouseX,m_iMouseY);	// 得到鼠标坐标

	if(bIsFocus)
	{
		m_bLeftDown = g_pInput->IsLeftButton();
		m_bRightDown = g_pInput->IsRightButton();
	}
	else
	{
		m_bLeftDown = false;
		m_bRightDown = false;
	}


// 	bool bOldEnableMagic = g_Config.IsEnableMagic();
// 
// 	CCharacter *pOldSelf = g_pSelf;
// 
	//离线托管玩家,微操对象,微身对象没有微操其它对象时,都在队长的客户端执行相关AI, 非离线托管模式下,附身且微操时,附身对象的AI在原来的队员的客户端执行
// 	if (g_TrusteeshipData.IsTrusteeship() && g_TrusteeshipData.IsCaptain())
// 	{
// 		for (int i = 0; i < MAX_TRUSTEESHIP_NUM; i++)
// 		{
// 			TneupMember& member = g_TrusteeshipData.GetTneupMember(i);
// 			if (member.dwID != 0 && member.byPos == i && member.pCharacter &&
// 				(member.byOffLineMode || member.byPos == g_TrusteeshipData.GetMicroControlPos() || (member.byPos == g_TrusteeshipData.GetCaptionSubstitutePos() && g_TrusteeshipData.GetMicroControlPos() < 0))
// 				)
// 			{
// 				g_pSelf = member.pCharacter;
// 
// 				if(g_pSelf != g_TrusteeshipData.GetMicroControledCharacter())//微操模式下对于非微操对象收到的协议产生的魔法都不应该创建,否则会看到两个魔法,因为其它人使用魔法微操对象和自己都会收到同样的协议,而且不创建动作,不处理动作,就像传世的元神
// 				{
// 					g_Config.SetEnableMagic(false);
// 				}
// 				else
// 				{
// 					g_Config.SetEnableMagic(bOldEnableMagic);
// 				}
// 				
// 				SELF.GetPlayerXY(m_iPlayerX,m_iPlayerY);
// 				SELF.ReSetOppBlock(m_iPlayerX,m_iPlayerY);
// 				//微操或者附身而又没有微操其它角色时,周围的怪物要处理,其它情况不要处理,因为也没有动作,只要设置阻挡点即可
// 				if (member.byPos == g_TrusteeshipData.GetMicroControlPos() || (member.byPos == g_TrusteeshipData.GetCaptionSubstitutePos() && g_TrusteeshipData.GetMicroControlPos() < 0))
// 				{
// 					DealCharacter();
// 				}
// 				else
// 				{
// 					AddCharacterBlock();
// 				}
// 
// 				if (SELF.IsOnPhenix())
// 				{
// 					g_pGameMap->AddPassBlockType(AT_PHENIX_PASS_BLOCK);
// 				}
// 				else
// 				{
// 					g_pGameMap->DelPassBlockType(AT_PHENIX_PASS_BLOCK);
// 				}
// 
// 				DealPlayer();
// 			}
// 		}
// 
// 		//微操模式下对于非微操对象收到的协议产生的魔法都不应该创建
// 		if (g_TrusteeshipData.GetMicroControlPos() >= 0 || g_TrusteeshipData.GetCaptionSubstitutePos() >= 0)
// 		{
// 			g_Config.SetEnableMagic(false);
// 		}
// 		else
// 		{
// 			g_Config.SetEnableMagic(bOldEnableMagic);
// 		}
// 
// 	}
// 
// 
	//没有附身,处理自己
// 	if (g_TrusteeshipData.GetCaptionSubstitutePos() < 0)
// 	{
// 		g_pSelf = &ORIGINALSELF;

		if (SELF.IsOnPhenix())
		{
			g_pGameMap->AddPassBlockType(AT_PHENIX_PASS_BLOCK);
		}
		else
		{
			g_pGameMap->DelPassBlockType(AT_PHENIX_PASS_BLOCK);
		}

		SELF.GetPlayerXY(m_iPlayerX,m_iPlayerY);
		SELF.ReSetOppBlock(m_iPlayerX,m_iPlayerY);
		DealCharacter();
		DealPlayer();
// 	}
// 
// 	g_Config.SetEnableMagic(bOldEnableMagic);
// 	g_pSelf = pOldSelf;
// 	//如果有微操,重新设置m_iPlayerX,m_iPlayerY,不然事件里用的m_iPlayerX,m_iPlayerY不对
// 	if (g_TrusteeshipData.GetMicroControlPos() >= 0)
// 	{
// 		CCharacter *pMicroControlSelf = g_TrusteeshipData.GetMicroControledCharacter();
// 		if (pMicroControlSelf)
// 		{
// 			pMicroControlSelf->GetPlayerXY(m_iPlayerX,m_iPlayerY);
// 		}
// 	}
}

void CGameManager::DealPlayer()
{
	SAction& ActionNow = SELF.GetAction();

	//八方分影斩	
	if (SELF.IsInShadowKill8())
	{
		if(ActionNow.uFlag & SHADOWKILL_ACTION)
		{
			int iFrame = m_dwFrameCount - SELF.GetReserveData(pubShadowKill8FrameStart);//此处的iFrame是连续的
			int iStepFrame = SELF.GetShadowStepFrame();
			int maxShadowNum = SELF.GetMaxShadowNum();

			if (iFrame == iStepFrame * maxShadowNum + 1)
			{
				SELF.SetUsingMagicType(0);			

				SELF.InitAction(ACTION_ATTACK1);
				SAction& sAction = SELF.GetAction();
				sAction.uFlag |= SHADOWKILL_ACTION;
				sAction.uData = 0;
				sAction.cDir = SELF.GetShadowKill8Dir();
			}	
			else if (iFrame == iStepFrame * maxShadowNum + 28)
			{
				SELF.SetUsingMagicType(1);

				int ax,ay;
				GetDirStep(SELF.GetShadowKill8Dir(),ax,ay);
				int iChaX,iChaY;
				SELF.GetXY(iChaX,iChaY);
				int cmx = iChaX + 2*ax;
				int cmy = iChaY + 2*ay;
				g_pMagicCtrl->CreateMagic(MAGICID_SHADOWKILL8_HIT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(cmx,cmy));//固定坐标 
			}	
			else if (iFrame >= iStepFrame * maxShadowNum + 28 + 25)
			{
				SELF.SetUsingMagicType(0);
				SELF.SetInShadowKill8(false);
			}
		}
		else
		{
			SELF.SetUsingMagicType(0);
			SELF.SetInShadowKill8(false);
		}
	}


	if(SELF.GetUsingMagicType() & UMT_NOACTION)// 不可以主动做任何动作
	{
		return;
	}

	
	int iFrameNow = ActionNow.GetFrame(m_dwFrameCount);//把m_dwFrameCount传到ActionNow

TRY_BEGIN
	if(ActionNow.IsStand())
	{
// 		if(g_TrusteeshipData.IsNeedDoTrusteeshipAction())
// 			DealPlayerActionStandTrust();
// 		else 
			DealPlayerActionStand();
	}
	else if(!ActionNow.IsEnd(m_dwFrameCount))//其他动作,还没有结束
	{
		DealPlayerActionRunning();
	}
	else //其他动作已经结束了
	{
		//结束前面的动作
		switch(ActionNow.wAction)
		{
		case ACTION_DEATH:
			{
				//死亡时始终停留
				return;
			}
		case ACTION_WALK:
		case ACTION_RUN:
		case ACTION_LFMAGICFGH:
		case ACTION_LFMAGICRAB:
		case ACTION_LFMAGICTAO:
			{	
				if(ActionNow.wAction == ACTION_LFMAGICRAB)
				{			
					Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(MAGICID_ZHONGLEI_ATTACK,SELF.GetID());
					g_pMagicCtrl->CreateMagic(pShow);
					g_pMagicCtrl->FinishMagic(pShow);
					SELF.SetNumForZhongLeiHit(0);
				}

				if(!SELF.IsWaitServer() || ActionNow.iData != 0)
				{
					SELF.SetOffset(0,0);					

					if(SELF.IsSuccess() || ActionNow.iData != 0 || (ActionNow.uFlag & SERVER_ACTION))
					{
						//如MSG_Magic_Position_Failed发来的冲撞失败不要设置
						if((ActionNow.wAction == ACTION_RUN) && (ActionNow.uFlag & SERVER_ACTION) && ActionNow.iData == 1) 
						{
						}
						else
						{
							SELF.SetXY(ActionNow.iAimX,ActionNow.iAimY);
							SELF.SetRealXY(ActionNow.iAimX,ActionNow.iAimY);
						}
					}
				}

				UpdateAlarm(SELF.IsWaitServer());

				if (ActionNow.wAction == ACTION_LFMAGICFGH || ActionNow.wAction == ACTION_LFMAGICRAB)
				{
					SELF.SetOffset(0,0);
				}
				break;
			}
		case ACTION_MAGIC:
			{
				int iPastFrame = m_dwFrameCount - ActionNow.tFrameStart;
				if(iPastFrame < 47)//魔法动作如何后面没有动作或是站立,魔法动作要停一会,如果是攻击动作也要一直停到下个攻击动作可以使用
				{
					SNextAction* pNextAction = SELF.GetNextAction();
					if(pNextAction == NULL ||  pNextAction->wAction == ACTION_STAND || pNextAction->wAction == ACTION_MAGIC)
					{
						ActionNow.iFrameNow = 0;
						return;
					}
					else if(pNextAction && pNextAction->wAction == ACTION_ATTACK1)
					{
						if(!g_AIMgr.IsCanAttack())
						{
							ActionNow.iFrameNow = 0;
							return;
						}
					}
				}


				if(ActionNow.iData == MAGICID_CALL_MONSTER_WXXG && ActionNow.uData != 0)		//召唤封元印守卫
				{
					//服务器要求做的动作,只做动作不发协议
					if ((ActionNow.uFlag & SERVER_ACTION) == 0)
					{
						g_pGameControl->SEND_WuXing_Apply_Monster(ActionNow.uData,ActionNow.iAimX,ActionNow.iAimY);
					}

					ActionNow.uData = 0;
				}
			}
		case ACTION_ATTACK1:
		case ACTION_CUT:
			{
				UpdateAlarm(SELF.IsWaitServer());
				break;
			}
		}

// 		if(g_TrusteeshipData.IsNeedDoTrusteeshipAction())
// 			DealPlayerActionEndTrust();
// 		else 
			DealPlayerActionEnd();
	}
TRY_END
}


void CGameManager::DealPlayerActionStand()
{
	SAction& ActionNow = SELF.GetAction();

	//外挂有优先要处理的动作,比如魔法盾之类,后面的动作延后
	if(g_AutoKillMonsterMgr.DoLoop())
	{//下一步动作		
		PlayerDoNextStep();								//有新的动作做新的动作
	}	
	else if(g_AutoFightMgr.DealAutoFight())
	{//下一步动作		
		PlayerDoNextStep();								//有新的动作做新的动作
	}
	else if(SELF.HasNextAction() == ENS_NO_ACTION)
	{
		if(SELF.GetMapPathFinder().IsOnRoute())
		{ 
			if(!g_AIMgr.NeedFailDelay())
				DealAutoRun();
		}
		else
		{
			ActionNow.GetFrame(m_dwFrameCount);
			if(ActionNow.IsEnd(m_dwFrameCount))			//呆机动作重复从头开始,
			{
				ActionNow.tFrameStart = m_dwFrameCount;
				ActionNow.iFrameNow = 0;
			}

			AutoNextAction();							//尝试生成新的动作

			if(SELF.HasNextAction() != ENS_NO_ACTION)	//有新的动作生成
				 PlayerDoNextStep();
			else g_AutoFightMgr.AutoMagicAttack();
		}
	}
	else
	{//下一步动作		
		PlayerDoNextStep();								//有新的动作做新的动作
	}
}

// void CGameManager::DealPlayerActionStandTrust()
// {
// 	SAction& ActionNow = SELF.GetAction();
//  	if(g_AITrusteeshipMgr.TrusteeshipAI())
//  	{//下一步动作		
//  		PlayerDoNextStep();								//有新的动作做新的动作
//  	}	
//  	else if(SELF.HasNextAction() == ENS_NO_ACTION)
// 	{
// 		if(SELF.GetMapPathFinder().IsOnRoute())
// 		{ 
// 			if(!g_AIMgr.NeedFailDelay())
// 				DealAutoRun();
// 		}
// 		else if(ActionNow.IsEnd(m_dwFrameCount))				//待机动作重复从头开始,
// 		{
// 			ActionNow.tFrameStart = m_dwFrameCount;
// 			ActionNow.iFrameNow = 0;
// 		}
// 		else ActionNow.GetFrame(m_dwFrameCount);
// 	}
// 	else
// 	{//下一步动作		
// 		PlayerDoNextStep();								//有新的动作做新的动作
// 	}
// }

void CGameManager::DealPlayerActionRunning()
{
	SAction& ActionNow = SELF.GetAction();
	int iFrameNow = ActionNow.GetFrame(m_dwFrameCount);

	if(ActionNow.wAction == ACTION_LFMAGICFGH)//战士攻击的最后6帧不移动位置,翱风斩
	{
		if(ActionNow.iFrameCount - ActionNow.iFrameNow < 7)
		{
			if((ActionNow.uFlag & HIGH_ACTION) ==0)
			{
				ActionNow.cDir = ActionNow.uData;
				SELF.SetDir(ActionNow.cDir);
				ActionNow.uFlag = HIGH_ACTION;
				Magic_Show_s*  pShow =g_pMagicCtrl->FindMagicByAll(MAGICID_AOFENGCUT_ATTACK,SELF.GetID());
				if(pShow)
				{
					pShow->iDir = ActionNow.uData*2;//翱风斩的最后一跃将根据被攻击者的位置进行调整方向
				}
			}
		}

		ActionNow.iFrameCount -= 7;
	}

	float fFrameRate = ActionNow.GetRate(m_dwFrameCount);
	int iDealTimesCurFrame = ActionNow.iDealTimesCurFrame;

	if(ActionNow.wAction == ACTION_LFMAGICFGH)//战士翱风斩攻击的最后6帧不移动位置
	{
		ActionNow.iFrameCount += 7;
	}

	switch(ActionNow.wAction)
	{
	case ACTION_WALK:
	case ACTION_RUN:
	case ACTION_LFMAGICFGH:
	case ACTION_LFMAGICRAB:
	case ACTION_LFMAGICTAO:
		{
			int iOffX = (int)(ActionNow.iMoveX * fFrameRate);
			int iOffY = (int)(ActionNow.iMoveY * fFrameRate);
			int iOldOffX,iOldOffY;	
			int iX,iY;
			SELF.GetXY(iX,iY);
			SELF.GetOffset(iOldOffX,iOldOffY);
			if(ActionNow.wAction == ACTION_LFMAGICRAB)
			{
				if(abs((iOffX+4)/TILE_WIDTH - (iOldOffX+4)/TILE_WIDTH) > 0 || abs((iOffY+2)/TILE_HEIGHT - (iOldOffY+2)/TILE_HEIGHT) >0)
				{
					SELF.SetNumForZhongLeiHit(SELF.GetNumForZhongLeiHit() + 1);
					iX += (iOldOffX + 4)/TILE_WIDTH;
					iY += (iOldOffY + 2)/TILE_HEIGHT;

					UINT uTargetID = MAKELONG(iX,iY);

					Magic_Show_s* pMagShow = g_pMagicCtrl->CreateMagic(MAGICID_ZHONGLEI_EFFECT_HIT,EMP_MAGIC_NOTARGET,SELF.GetID(),uTargetID,0);
					if(pMagShow)
						pMagShow->iCycles = SELF.GetNumForZhongLeiHit()*2;
				}
			}

			SELF.SetOffset(iOffX,iOffY);

			// 设置圣诞树的位置音乐
			if(SELF.GetReserveData(pubChristmasTreePos))
			{
				WORD wSx,wSy;
				int iSelfX,iSelfY;
				wSx = LOWORD(SELF.GetReserveData(pubChristmasTreePos));
				wSy = HIWORD(SELF.GetReserveData(pubChristmasTreePos));
				SELF.GetXY(iSelfX,iSelfY);
				if(abs(iSelfX - wSx) > 15 || abs(iSelfY - wSy) > 15)
				{
					g_pAudio->StopMidi(1);
					SELF.SetReserveData(pubChristmasTreePos,0);
				}
				else
				{
					g_pAudio->SetPositionMidi(1,0,0,(iSelfX - wSx) * 64 + iOffX,(iSelfY - wSy) * 32 + iOffY);
				}
			}
			break;
		}
	case ACTION_ATTACK1:
	case ACTION_MAGIC:
	case ACTION_SWT:
	case ACTION_LSJ:
	case ACTION_JYL:
		{
			if(fFrameRate > 0.7f && !SELF.IsWaitServer() && SELF.IsSuccess())
			{
				if(ActionNow.pTarget)
				{
					EffectAttacked(ActionNow.pTarget);
				}
			}
			if(iFrameNow == 3 && iDealTimesCurFrame == 1 && ActionNow.wAction == ACTION_ATTACK1)//第3帧第一次绘制
			{
				if (SELF.HasMagicState(MS_VALIDDIG))
				{  
					g_pMagicCtrl->CreateMagic(MAGICID_DIG_SPARK,0,SELF.GetID(),0,2*SELF.GetDir());
					SELF.RemoveMagicState(MS_VALIDDIG);
				}
				else if (SELF.HasMagicState(MS_WEAPONBREAK))
				{	
					g_pMagicCtrl->CreateMagic(MAGICID_ARM_REFRESH_FAILED,0,SELF.GetID(),0,2*SELF.GetDir());	
					SELF.RemoveMagicState(MS_WEAPONBREAK);
				}

			}
			if(iFrameNow >= 3 && ActionNow.iData == MAGICID_CALL_MONSTER_WXXG && ActionNow.uData != 0)		//召唤封元印守卫
			{
				g_pGameControl->SEND_WuXing_Apply_Monster(ActionNow.uData,ActionNow.iAimX,ActionNow.iAimY);
				ActionNow.uData = 0;
			}
			break;
		}
	case ACTION_ATTACKED:		// 被攻击动作
		{
			if(ActionNow.iData != 0 && iFrameNow == 1 && iDealTimesCurFrame == 1)
			{
				g_pMagicCtrl->CreateMagic(ActionNow.iData,0,SELF.GetID());
			}

			break;
		}
	}
}

void CGameManager::DealPlayerActionEnd()
{
	//外挂有优先要处理的动作,比如魔法盾之类,后面的动作延后

	if(g_AutoKillMonsterMgr.DoLoop())
	{
	}	
	else if(g_AutoFightMgr.DealAutoFight())
	{
	}

	if(SELF.GetMapPathFinder().IsOnRoute())
	{
		if(SELF.HasNextAction() != ENS_NO_ACTION)
			PlayerDoNextStep();
		else if(!g_AIMgr.NeedFailDelay())
			DealAutoRun();

		return;
	}

	//根据鼠标键盘状态，持续动作
	if(SELF.HasNextAction() == ENS_NO_ACTION)
	{
		AutoNextAction();
	}

	//下一步动作
	PlayerDoNextStep(); //有新的动作做新的动作
}

// void CGameManager::DealPlayerActionEndTrust()
// {
//  	g_AITrusteeshipMgr.TrusteeshipAI();
// 
// 	//下一步动作
// 	PlayerDoNextStep(); //有新的动作做新的动作
// }

void CGameManager::AddCharacterBlock()
{
	MiniChar& MChars = MapArray.GetMiniChar();
	if(MChars.size() == 0)
		return;

	for(MiniChar::iterator itr = MChars.begin(); itr != MChars.end(); ++itr)
	{
		CSimpleCharacterNode *pChar = itr->second;
		while(pChar)
		{
			//设置阻挡信息
			if(!pChar->IsDead())
			{
				SetCharBlock(pChar);
			}

			pChar = pChar->m_MapNext;
		}
	}
}

void CGameManager::DealCharacter()
{
	MiniChar& MChars = MapArray.GetMiniChar();
	if(MChars.size() == 0)
		return;

	for(MiniChar::iterator itr = MChars.begin(); itr != MChars.end(); ++itr)
	{
		CSimpleCharacterNode *pChar = itr->second;

		while(pChar)
		{
			SAction& ActionNow = pChar->GetAction();
			int iFrameNow = ActionNow.GetFrame(m_dwFrameCount);

			if( pChar->IsDeal() )
			{
				pChar->SetDeal(false); // 这个物体已经被处理过了，在处理的时候可能会改变位置,就会导致在MapArray.GetMiniChar 性表示是在前面移动的物体
				pChar = pChar->m_MapNext;
				continue;
			}

			//设置阻挡信息
			if(!pChar->IsDead())
			{
				SetCharBlock(pChar);
			}

			//八方分影斩
			if (pChar->IsInShadowKill8())
			{
				if(ActionNow.uFlag & SHADOWKILL_ACTION)
				{
					int iFrame = m_dwFrameCount - pChar->GetReserveData(pubShadowKill8FrameStart);//此处的iFrame是连续的
					int iStepFrame = pChar->GetShadowStepFrame();			
					int maxShadowNum = pChar->GetMaxShadowNum();

					if (iFrame == iStepFrame * maxShadowNum + 1)
					{
						pChar->SetUsingMagicType(0);

						pChar->InitAction(ACTION_ATTACK1);
						SAction& sAction = pChar->GetAction();
						sAction.uFlag |= SHADOWKILL_ACTION;
						sAction.uData = 0;
						sAction.cDir = pChar->GetShadowKill8Dir();
					}	
					else if (iFrame == iStepFrame * maxShadowNum + 28)
					{
						pChar->SetUsingMagicType(1);

						int ax,ay;
						GetDirStep(pChar->GetShadowKill8Dir(),ax,ay);
						int iChaX,iChaY;
						pChar->GetXY(iChaX,iChaY);
						int cmx = iChaX + 2*ax;
						int cmy = iChaY + 2*ay;
						g_pMagicCtrl->CreateMagic(MAGICID_SHADOWKILL8_HIT,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(cmx,cmy));//固定坐标
					}
					else if (iFrame >= iStepFrame * maxShadowNum + 28 + 25)
					{
						pChar->SetUsingMagicType(0);
						pChar->SetInShadowKill8(false);
					}

					//刀砍下去停留半秒
					if((m_dwFrameCount - ActionNow.tFrameStart) >= 28)
					{
						ActionNow.iFrameNow = ActionNow.iFrameCount - 1;
						iFrameNow = ActionNow.iFrameNow;
						pChar = pChar->m_MapNext;
						continue;
					}
				}
				else
				{
					pChar->SetUsingMagicType(0);
					pChar->SetInShadowKill8(false);
				}				
			}


			if(ActionNow.IsStand())
			{
				if(pChar->HasNextAction() == ENS_NO_ACTION) //没有动作
				{
					if(ActionNow.IsEnd(m_dwFrameCount))
					{
						if (pChar->IsNpc())
						{
							if (GetTickCount() - pChar->GetNpcActionTm() >= 15 * 1000)
							{
								ActionNow.SetNpcRandStand(pChar->GetRaceNo());
								if (pChar->GetRaceNo() != 6 || ActionNow.uData >= 3)
									pChar->SetNpcActionTm(GetTickCount());

								ActionNow.uData++;
							}
							else 
							{
								ActionNow.uData = 0;
								ActionNow.wAction = ACTION_STAND;
								ActionNow.wDrawAction = ACTION_STAND;
								ActionNow.iFrameCount = 6;
								ActionNow.iFrameSpeed = 16;
							}
						}

						ActionNow.tFrameStart = m_dwFrameCount;
						ActionNow.iFrameNow = 0;						
					}
				}
				else
				{
					CharDoNextStep(pChar);
				}
			}
			else if(!ActionNow.IsEnd(m_dwFrameCount))//其他动作,还没有结束
			{
				int iSelfX,iSelfY,iX,iY;
				SELF.GetXY(iSelfX,iSelfY);
				pChar->GetXY(iX,iY);
				UINT lSoundID = 0;				
				int iMonster = pChar->GetRaceNo();
				int iDealTimesCurFrame = ActionNow.iDealTimesCurFrame;


				if(ActionNow.wAction == ACTION_LFMAGICFGH)//战士攻击的最后6帧不移动位置
				{
					if(ActionNow.iFrameCount - iFrameNow < 7)
					{
						if((ActionNow.uFlag & HIGH_ACTION) ==0)
						{
							ActionNow.cDir = ActionNow.uData;
							pChar->SetDir(ActionNow.cDir);
							ActionNow.uFlag = HIGH_ACTION;

							Magic_Show_s*  pShow =g_pMagicCtrl->FindMagicByAll(MAGICID_AOFENGCUT_ATTACK,pChar->GetID());
							if(pShow)
								pShow->iDir = ActionNow.uData*2;
						}
					}
					ActionNow.iFrameCount -= 7;
				}

				float fFrameRate = ActionNow.GetRate(m_dwFrameCount);

				if(ActionNow.wAction == ACTION_LFMAGICFGH)//战士攻击的最后6帧不移动位置
				{
					ActionNow.iFrameCount += 7;
				}

				switch(ActionNow.wAction)
				{
				case ACTION_STAND:
					{
						if((iMonster == 215 || iMonster  == 216 || iMonster  == 217) && iFrameNow == 0 && iDealTimesCurFrame == 1) // 地火兽弓骑士效果
							g_pMagicCtrl->CreateMagic(MAGICID_FLOOR_MONSTER_STAND,0,pChar->GetID(),0,2*pChar->GetDir());
					}
					break;
				case ACTION_WALK:
					{	
						lSoundID = (iMonster * 8) + 21;
						if((iMonster == 215 || iMonster  == 216 || iMonster  == 217) && iFrameNow == 0 && iDealTimesCurFrame == 1) // 地火兽弓骑士效果
							g_pMagicCtrl->CreateMagic(MAGICID_FLOOR_MONSTER_WALK,0,pChar->GetID(),0,2*pChar->GetDir());
					}
					break;
				case ACTION_RUN:
					{
						lSoundID = (iMonster * 8) + 21;

						if(iMonster == 306)		//双头暴熊
						{
							lSoundID = 2470;
						}
					}
					break;

				case ACTION_LFMAGICFGH:
				case ACTION_LFMAGICRAB:
				case ACTION_LFMAGICTAO:
					{
						int iOffX = (int)(ActionNow.iMoveX * fFrameRate);
						int iOffY = (int)(ActionNow.iMoveY * fFrameRate);

						if(ActionNow.wAction == ACTION_LFMAGICRAB)
						{
							int iOldOffX,iOldOffY;
							int iX,iY;
							pChar->GetXY(iX,iY);
							pChar->GetOffset(iOldOffX,iOldOffY);

							if(abs((iOffX + 4)/TILE_WIDTH - (iOldOffX + 4)/TILE_WIDTH) > 0 || abs((iOffY + 2)/TILE_HEIGHT - (iOldOffY + 2)/TILE_HEIGHT) >0)
							{
								iX += (iOldOffX + 4)/TILE_WIDTH;
								iY += (iOldOffY + 2)/TILE_HEIGHT;
								UINT uTargetID = MAKELONG(iX,iY);

								pChar->SetNumForZhongLeiHit(pChar->GetNumForZhongLeiHit() + 1);

								Magic_Show_s* pMagShow = g_pMagicCtrl->CreateMagic(MAGICID_ZHONGLEI_EFFECT_HIT,EMP_MAGIC_NOTARGET,pChar->GetID(),uTargetID);
								if(pMagShow)
									pMagShow->iCycles = pChar->GetNumForZhongLeiHit()*2;
							}
						}
						pChar->SetOffset(iOffX,iOffY);
						int iPlayAudio = 2606+ActionNow.wAction - ACTION_LFMAGICFGH;
						g_pAudio->PlayEx(EAT_MAGIC,iPlayAudio,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY);
					}

					break;

				case ACTION_ATTACKED:		// 被攻击动作
					{
						lSoundID = (iMonster * 8) + 18;
						if (ActionNow.iFrameNow == 0 && ActionNow.iDealTimesCurFrame == 1)
						{
							g_pAudio->PlayEx(EAT_OTHER,127,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY,false);	
						}

						if(iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							switch (iMonster)
							{
							case 215:
							case 216:
							case 217:
								g_pMagicCtrl->CreateMagic(MAGICID_FLOOR_MONSTER_ATTACKED,0,pChar->GetID(),0,2*pChar->GetDir());
								break;
							case 316://离火柱
								g_pMagicCtrl->CreateMagic(MAGICID_FIRE_ATTACKED,0,pChar->GetID());
								break;
							case 317://玄冰柱
								g_pMagicCtrl->CreateMagic(MAGICID_ICE_ATTACKED,0,pChar->GetID());
								break;
							case 320://护塔机关			
							case 321://护塔机关2	
							case 322://护塔机关3	
								g_pMagicCtrl->CreateMagic(MAGICID_TOWER_ATTACKED,0,pChar->GetID());
								break;
							}
						}

						if(ActionNow.iData != 0 && iFrameNow == 1 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(ActionNow.iData,0,SELF.GetID());
						}
						if (pChar->GetRaceNo() == 111 || pChar->GetRaceNo() == 113)
						{
							g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByAll(MAGICID_SHENSHOU_ATTACK,pChar->GetID()));
						}

						if(iMonster == 314 || iMonster == 315)		//阴阳妖灵50%发出受伤声音
						{
							if(rand() % 2 == 0)
							{
								lSoundID = 0;
							}
						}

						break;
					}
				case ACTION_ATTACK1:
					{
						lSoundID = (iMonster * 8) + 17;
						if(iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							if(iMonster == 179)// 铁翼魔的攻击
								g_pMagicCtrl->CreateMagic(MAGICID_IRON_WING_MONSTR_ATTACK,0,pChar->GetID(),0,2*pChar->GetDir());
							else if(iMonster == 198)//通天教主攻击飞行效果
								g_pMagicCtrl->CreateMagic(MAGICID_TONGTIAN_FLY_ATTACK,0,pChar->GetID());
							else if (iMonster == 199)//通天教主1攻击效果
								g_pMagicCtrl->CreateMagic(MAGICID_SHENSHOU_ATTACK,0,pChar->GetID(),0,2*pChar->GetDir());
							else if(iMonster == 206 || pChar->GetRaceNo() == 219) // 炎魔攻击效果
								g_pMagicCtrl->CreateMagic(MAGICID_YAN_MONSTER_ATTACK_EFFECT,0,pChar->GetID(),0,2*pChar->GetDir());
							else if(iMonster == 215 || pChar->GetRaceNo()  == 216 || pChar->GetRaceNo()  == 217) // 地火兽弓骑士效果
								g_pMagicCtrl->CreateMagic(MAGICID_FLOOR_MONSTER_ATTACK1,0,pChar->GetID(),0,2*pChar->GetDir());
							else if(iMonster == 249) //无相天魔的攻击特效
								g_pMagicCtrl->CreateMagic(MAGICID_MON_TIANMO_ATTACK1,0,pChar->GetID(),0,2*pChar->GetDir());		
							else if(iMonster == 306)
								g_pMagicCtrl->CreateMagic(MAGICID_BEAR_Claw_Light,0,pChar->GetID(),0,pChar->GetDir() * 2);	
							//else if(iMonster == 378)//兔女郎
							//	g_pMagicCtrl->CreateMagic(MAGICID_RABBIT_ATTACK,0,pChar->GetID(),0,pChar->GetDir() * 2);	

		
						}
						else if(iFrameNow == 3 && iDealTimesCurFrame == 1)//第3帧
						{
							if (pChar->HasMagicState(MS_VALIDDIG))
							{  
								g_pMagicCtrl->CreateMagic(MAGICID_DIG_SPARK,0,pChar->GetID(),0,2*pChar->GetDir());
								pChar->RemoveMagicState(MS_VALIDDIG);
							}
							else if (pChar->HasMagicState(MS_WEAPONBREAK))
							{	
								g_pMagicCtrl->CreateMagic(MAGICID_ARM_REFRESH_FAILED,0,pChar->GetID(),0,2*pChar->GetDir());	
								pChar->RemoveMagicState(MS_WEAPONBREAK);
							}
							else if(iMonster == 305)						
							{
								g_pMagicCtrl->CreateMagic(MAGICID_Sword_Fire,0,pChar->GetID(),0,pChar->GetDir() * 2);
								g_pMagicCtrl->CreateMagic(MAGICID_Ground_Break,0,pChar->GetID(),0,pChar->GetDir() * 2);
							}

						}
						else if(iFrameNow == 5 && iDealTimesCurFrame == 1)//第5帧
						{
							switch(iMonster)
							{
							case 111://麒麟(神兽)的攻击
							case 113://心魔召唤的神兽的攻击
								g_pMagicCtrl->CreateMagic(MAGICID_SHENSHOU_ATTACK,0,pChar->GetID(),0,2*pChar->GetDir());
								break;	
							case 313://马怪
								g_pMagicCtrl->CreateMagic(MAGICID_FIRE_BALL_HORSE_ATTACK,0,pChar->GetID(),ActionNow.uData);
								break;
							case 307://驭冰游神
								g_pMagicCtrl->CreateMagic(MAGICID_ICE_ATTACK1,0,pChar->GetID(),ActionNow.uData);
								break;															
							case 327://道士套装技能召唤出来的碧血魔
								g_pMagicCtrl->CreateMagic(MAGICID_DAO_BLOOD_MONSTER_ATTACK,0,pChar->GetID(),ActionNow.uData);
								break;							
							case 420://圣武连弩
								g_pMagicCtrl->CreateMagic(MAGICID_SHENGWUGONG_ATTACK,0,pChar->GetID(),ActionNow.uData);
								break;
							}
						}
						else if(iFrameNow == 8 && iDealTimesCurFrame == 1)//第8帧
						{
							switch(iMonster)
							{
							case 314:
								if(ActionNow.uData)
								{
									g_pMagicCtrl->CreateMagic(MAGICID_FIRE_BOSS_ATTACK1,0,pChar->GetID(),ActionNow.uData);
								}
								break;
							case 315:
								if(ActionNow.uData)
								{
									g_pMagicCtrl->CreateMagic(MAGICID_ICE_BOSS_ATTACK1,0,pChar->GetID(),ActionNow.uData);
								}
								break;	
							case 445:
								if(ActionNow.uData)
								{
									g_pMagicCtrl->CreateMagic(MAGICID_FIREMONSTER_THROWATTACK,EMP_MAGIC_OBJECT,pChar->GetID(),ActionNow.uData);
								}
								break;
							}
						}

						break;
					}
				case ACTION_ATTACK2:
					{
						if(iMonster == 2 || iMonster == 166 || iMonster == 168 || iMonster == 170 || iMonster == 185 || iMonster == 186 || iMonster == 187)//踏云豹 丛林豹 扑食物  打哈欠  吃食物2 
							lSoundID = 1352;//扑食物
						else if(iMonster == 301 || iMonster == 302)
							lSoundID = 2436;
						else if(iMonster == 249)
							lSoundID = 236 * 8 + 17;
						else if(iMonster == 306)	//双头暴熊
							lSoundID = 2471;
						else if(iMonster == 307)	//驭冰游神
							lSoundID = 2478;
						else if(iMonster == 314)	//火BOSS
							lSoundID = 2534;
						else if(iMonster == 315)	//冰BOSS
							lSoundID = 2542;
						else if(iMonster == 351 || iMonster == 348)
							lSoundID = 2544;

						// 暗域之王
						if(iMonster == 221 && iFrameNow == 0 && iDealTimesCurFrame == 1)
							g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK2,0,pChar->GetID());
						if(iMonster == 229 && iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							g_pAudio->PlayEx(EAT_MONSTER,22,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY,false);
						}
						else if (iFrameNow == 0 && iDealTimesCurFrame == 1) //无相天魔的攻击特效
						{
							if(iMonster == 249)
								g_pMagicCtrl->CreateMagic(MAGICID_MON_TIANMO_ATTACK2,0,pChar->GetID());								
						}
						else if (iFrameNow == 3 && iDealTimesCurFrame == 1) 
						{					
							if(iMonster == 315)	//冰Boss的攻击
							{
								for(int i = 0;i < 8;i++)
								{
									g_pMagicCtrl->CreateMagic(MAGICID_ICE_BOSS_ATTACK2,0,pChar->GetID(),0,2 * i);
								}
							}
							else if(iMonster == 314)	//火BOSS
							{
								for(int i = 0;i < 8;i++)
								{
									g_pMagicCtrl->CreateMagic(MAGICID_FIRE_BOSS_ATTACK2,0,pChar->GetID(),0,2 * i);
								}
							}
							else if(iMonster == 352)
							{
								g_pMagicCtrl->CreateMagic(MAGICID_HUMANTIGER_ROAR,0,pChar->GetID(),0,pChar->GetDir() * 2);
							}
							else if(iMonster == 351)
							{
								g_pMagicCtrl->CreateMagic(MAGICID_TIGER_SPRAY,0,pChar->GetID());
							}
						}
						else if(iFrameNow == 5 && iDealTimesCurFrame == 1)	//驭冰游神
						{
							switch (iMonster)
							{
							case 307:
								g_pMagicCtrl->CreateMagic(MAGICID_ICE_ATTACK2,0,pChar->GetID());
								break;							
							case 420://圣武连弩
								{
									int mdir = pChar->GetDir()*2;
									for (int i = -3; i <= 3; ++i)
									{
										int tdir = ModifyDir(mdir + i, 16);										
										g_pMagicCtrl->CreateMagic(MAGICID_SHENGWUGONG_ATTACK,0,pChar->GetID(),0,tdir);
									}
								}
								break;
							}
						}
						else if(iMonster == 483 && iFrameNow == 8 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_TULION_EARTH_CI,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));
						}
					}
					break;
				case ACTION_ATTACK3:
					{
						if(iMonster == 2 )//丛林豹
							lSoundID = 32;//吃食物2
						else if (iMonster == 166 || iMonster == 168 || iMonster == 170)
							lSoundID = 1353;//吃食物2  吼叫
						else if (iMonster == 185 || iMonster == 186 || iMonster == 187)
							lSoundID = 1360;//吃食物2  吼叫
						else if (iMonster == 39 || iMonster == 40 || iMonster == 41)
							lSoundID = 1361;		
						else if(iMonster == 301 || iMonster == 302)
							lSoundID = 2430;
						else if(iMonster == 314)	//火BOSS
							lSoundID = 2535;
						else if(iMonster == 315)	//冰BOSS
							lSoundID = 2543;

						// 暗域之王
						if(pChar->GetRaceNo() == 221 && iFrameNow == 0 && iDealTimesCurFrame == 1)
							g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK3,0,pChar->GetID(),0,2 * pChar->GetDir());
						else if(iMonster == 314 && iFrameNow == 8 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_FIRE_BOSS_ATTACK3,0,pChar->GetID(),ActionNow.uData);
						}						
						else if(iMonster == 419 && iFrameNow == 5 && iDealTimesCurFrame == 1)
						{//圣武斧卫
							g_pMagicCtrl->CreateMagic(MAGICID_SHENGWUFU_ATTACK,0,pChar->GetID(),ActionNow.uData);							
						}
						else if (iMonster == 436 && iFrameNow == 6 && iDealTimesCurFrame == 1)//持鞭骑兵
						{
							g_pMagicCtrl->CreateMagic(MAGICID_BIANBING_ATTACK2,0,pChar->GetID(),ActionNow.uData);
						}
						else if (iMonster == 480 && iFrameNow == 8 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_LEOPARD_ATTACK2,EMP_MAGIC_NOTARGET,
								pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));	
						}
						else if(iMonster == 484 && iFrameNow == 8 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_Tiger_COOLATTACK,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));
						}
						else if (iMonster == 481 && iFrameNow == 5 && iDealTimesCurFrame == 1)	//水麒麟大招
						{
							g_pMagicCtrl->CreateMagic(MAGICID_QILIN_EARTH_ICE,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));
						}
						else if (iMonster == 483 && iFrameNow == 5 && iDealTimesCurFrame == 1)	//土狮子大招
						{
							g_pMagicCtrl->CreateMagic(MAGICID_TULION_ATTACKED_TARGET,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));
						}
						else if (iMonster == 482 && iFrameNow == 5 && iDealTimesCurFrame == 1)	//凤凰大招
						{
							g_pMagicCtrl->CreateMagic(MAGICID_PHOENIX_COOLATTACK,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(ActionNow.iAimX,ActionNow.iAimY));
						}
						break;
					}
				case ACTION_ATTACK4:
					{
						// 暗域之王
						if(iMonster == 221 && iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							g_pAudio->PlayEx(EAT_MONSTER,6,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY,false);
							g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK4,0,pChar->GetID(),0,2 * pChar->GetDir());
						}
						break;
					}
				case ACTION_ATTACK5:
					{
						if(iMonster == 166 || iMonster == 168 || iMonster == 170 )//踏云豹 
							lSoundID = 1351;//打哈欠
						else if (iMonster == 185 || iMonster == 186 || iMonster == 187)
							lSoundID = 1496;//打哈欠
						else if(iMonster == 301 || iMonster == 302)
							lSoundID = 2431;

						// 暗域之王
						if(iMonster == 221 && iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							g_AIMgr.StartShake(3000);
							g_pAudio->PlayEx(EAT_MONSTER,7,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY,false);
							if (pChar->GetDir() < 4)
								g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK5_DIR4,0,pChar->GetID(),0,2 * pChar->GetDir());
							else
								g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK5_DIR8,0,pChar->GetID(),0,2 * pChar->GetDir());
						}
						break;
					}
				case ACTION_ATTACK6:
					{
						// 暗域之王
						if(iMonster == 221 && iFrameNow == 0 && iDealTimesCurFrame == 1)
						{
							g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_ATTACK6,0,pChar->GetID(),0,2 * pChar->GetDir());
						}
						break;
					}
				case ACTION_ADDACTION1:
					{
						if (iMonster == 185 || iMonster == 186 || iMonster == 187)
							lSoundID = 1360;//吃食物2  吼叫
						else if(iMonster == 301 || iMonster == 302)
							lSoundID = 2428;
					}
					break;
				default:
					break;
				}

				if(ActionNow.IsMove()|| ActionNow.uFlag & MOVE_ACTION)
				{
					int iOffX = (int)(ActionNow.iMoveX * fFrameRate);
					int iOffY = (int)(ActionNow.iMoveY * fFrameRate);
					pChar->SetOffset(iOffX,iOffY);
				}	

				if(ActionNow.iFrameNow == 0 && ActionNow.iDealTimesCurFrame == 1 && lSoundID > 0 && pChar->IsMonster())
				{
					g_pAudio->PlayEx(EAT_MONSTER,lSoundID,g_pAudio->GetRand()++,iSelfX,iX,iSelfY,iY);
				}
			}
			else //其他动作已经结束了
			{
				if(ActionNow.wAction == ACTION_APPEAR && (ActionNow.iData == 5 || ActionNow.iData == 6))
				{
					MapArray.DeleteCharacter(pChar->GetID());
					return;
				}

				//野蛮冲撞或突斩失败处理
				if((ActionNow.wAction == ACTION_RUN) && (ActionNow.uFlag & SERVER_ACTION)
					&& (ActionNow.uFlag & COLLIDE_ACTION) && (ActionNow.iData == 1))
				{
					int iX,iY;
					pChar->GetRealXY(iX,iY);
					pChar->SetXY(iX,iY);
					pChar->SetOffset(0,0);
					g_pGameData->AddSimpleCharacter(iX,iY,pChar->GetID());
				} 
				else if(ActionNow.IsMove() || (ActionNow.uFlag & MOVE_ACTION) || ActionNow.wAction == ACTION_LFMAGICFGH || ActionNow.wAction == ACTION_LFMAGICRAB || ActionNow.wAction == ACTION_LFMAGICTAO)
				{
					int cx,cy;
					pChar->GetXY(cx,cy);
					pChar->SetOffset(0,0);
					pChar->SetDir(ActionNow.cDir);

					//坐标发生改变
					if(cx != ActionNow.iAimX || cy != ActionNow.iAimY)
					{
						cx = ActionNow.iAimX;
						cy = ActionNow.iAimY;

						g_pGameData->AddSimpleCharacter(cx,cy,pChar->GetID());
						pChar->SetXY(cx,cy);
					}
				}

				switch(ActionNow.wAction)
				{
				case ACTION_ATTACK1:
					{
						if (!pChar->IsMonster())
							break;

						switch(pChar->GetRaceNo())
						{
						case 47:// 掷枪尸卫
							g_pMagicCtrl->CreateMagic(MAGICID_THROW_MONSTER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 85:// 地火兽弓骑士
							g_pMagicCtrl->CreateMagic(MAGICID_FLOORFIRE_CAVALIER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 115://大刀，回到原来的地方
							{
								int cx,cy;
								pChar->GetXY(cx,cy);

								if(cx != ActionNow.iAimX || cy != ActionNow.iAimY)
								{
									cx = ActionNow.iAimX;
									cy = ActionNow.iAimY;
									g_pGameData->AddSimpleCharacter(cx,cy,pChar->GetID());
									pChar->SetXY(cx,cy);
									pChar->SetDeal(true);
								}
							}
							break;
						case 116://弓箭手护卫的攻击
							g_pMagicCtrl->CreateMagic(MAGICID_GUILD_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 276://箭塔图腾
							g_pMagicCtrl->CreateMagic(MAGICID_ARROW_TOWER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 123:// 仙人掌
							g_pMagicCtrl->CreateMagic(MAGICID_CACTI_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 173:////血魔
							g_pMagicCtrl->CreateMagic(MAGICID_BLOOD_MONSTER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 205://铜人攻击
							g_pMagicCtrl->CreateMagic(MAGICID_COPPER_MONSTER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 215://////逆魔弓箭手
							g_pMagicCtrl->CreateMagic(MAGICID_BOW_MONSTER_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;
						case 216: //地火兽投矛手
							g_pMagicCtrl->CreateMagic(MAGICID_FLOORFIRE_SPEAR_FLY_ATTACK,0,pChar->GetID(),ActionNow.uData);
							break;						
						}

						break;
					}
				case ACTION_ATTACK2:
					if(pChar->GetRaceNo() == 229)//逆魔之王在攻击动作最后一帧时放出火球
						g_pMagicCtrl->CreateMagic(MAGICID_FIREBALL,EMP_MAGIC_NOTARGET,pChar->GetID());
					break;
				case ACTION_ATTACK4:
					if(pChar->GetRaceNo() == 249)//无相天魔的攻击方式
					{
						int iX,iY;
						pChar->GetXY(iX,iY);
						g_pMagicCtrl->CreateMagic(MAGICID_MON_DEMON_GROUND,EMP_MAGIC_NOTARGET,pChar->GetID(),MAKELONG(iX,iY));
						g_AIMgr.StartShake(3000);//震动屏幕三秒
					}

					break;
				case ACTION_LFMAGICRAB:
					{
						Magic_Show_s* pShow = g_pMagicCtrl->FindMagicByAll(MAGICID_ZHONGLEI_ATTACK,pChar->GetID());
						g_pMagicCtrl->CreateMagic(pShow);
						g_pMagicCtrl->FinishMagic(pShow);
					}
					break;
				}

				bool bNeedDoNextStep = true;
				if(pChar->IsHuman() && pChar->HasNextAction() == ENS_NO_ACTION)//如果后面服务器的动作还没有来,停留在最后一帧停一会
				{
					if(ActionNow.wAction == ACTION_ATTACK1 || ActionNow.wAction == ACTION_MAGIC || ActionNow.wAction == ACTION_RUN || ActionNow.wAction == ACTION_WALK)
					{
						int iAddFrame = 24;
						if(ActionNow.wAction == ACTION_RUN || ActionNow.wAction == ACTION_WALK)
							iAddFrame = 6;

						if(m_dwFrameCount < ActionNow.tFrameStart + ActionNow.iFrameCount*ActionNow.iFrameSpeed + iAddFrame)
						{
							if(ActionNow.wAction == ACTION_RUN || ActionNow.wAction == ACTION_WALK)
								ActionNow.iFrameNow = ActionNow.iFrameCount - 1;
							else
								ActionNow.iFrameNow = 0;

							bNeedDoNextStep = false;
						}
					}
				}

				if(!ActionNow.IsDeath() && bNeedDoNextStep)
				{
					CharDoNextStep(pChar);
				}
			}
			pChar = pChar->m_MapNext;
		}
	}
}

void CGameManager::PlayerDoNextStep()
{
	E_NEXT_STATE next_state = SELF.HasNextAction();
	if(next_state == ENS_NO_ACTION) //没有下一个动作
	{
		if(SELF.GetMapPathFinder().IsOnRoute())
		{ 
			if(!g_AIMgr.NeedFailDelay())
				DealAutoRun();
		}
		else if(!SELF.IsWaitServer() && !SELF.IsSinging())
		{
			SELF.InitAction(ACTION_STAND);
		}
		return;
	}

	if(next_state != ENS_SERVER_ACTION && (SELF.HasStatus(CS_LULL) || (SELF.GetExtraState() & ES_FROST)))
	{
		SELF.InitAction(ACTION_STAND);
		return;
	}

	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	int ix,iy; //临时

	//下一个动作变成当前动作
	SAction& ActionNow = SELF.GetAction();
	WORD wOldAction = ActionNow.wAction;
	SNextAction Next;
	SNextAction* pNextAction = NULL;
	pNextAction = SELF.GetNextAction();
TRY_BEGIN
	if(pNextAction)//道士骑战技能动作毒凌波
	{
		if((pNextAction->uFlag & BACK_ACTION) && (pNextAction->wAction == ACTION_WALK) && (pNextAction->uFlag & DELAY_ACTION))
		{
			ActionNow.tFrameStart = m_dwFrameCount;
			ActionNow.iFrameNow = 0;
			return;
		}		
	}

	if(!SELF.PopNextAction(&Next)) //弹出最后一个动作
	{
		SELF.InitAction(ACTION_STAND);
		return;
	}

	if(Next.cDir == DIR_NO) //动作没有设置方向
		Next.cDir = SELF.GetDir();

	m_dwActionStartTime = GetTickCount();
TRY_END

TRY_BEGIN
	switch(Next.wAction)
	{
	case ACTION_WALK:
	case ACTION_RUN:
	case ACTION_LFMAGICFGH:
	case ACTION_LFMAGICRAB:
	case ACTION_LFMAGICTAO:
		{
			if(Next.uFlag & SERVER_ACTION) //服务器发送来的动作,只做动作不发协议
			{
				if(g_pGameMap->IsBlock(Next.iAimX,Next.iAimY) && !(Next.uFlag & COLLIDE_ACTION))
				{
					SELF.InitAction(ACTION_STAND);
					return;
				}

				if(Next.wAction != ACTION_LFMAGICRAB)
					Next.cDir = GetDir8(Next.iAimX,Next.iAimY,iSelfX,iSelfY);

				if((Next.uFlag & BACK_ACTION) && (Next.wAction == ACTION_WALK)) //倒退着走，被冲撞或被狮子吼或被毒凌波击中
				{			
					if(Next.uData > 0 && Next.uData != 1)//被狮子吼后(Next.uData > 0)下一个动作不再是后退(Next.uData != 1)
					{
						Magic_Show_s* ms2 = NULL;
						Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(MAGICID_LION_CALLED,SELF.GetID(),EMP_MAGIC_SERVER_ADVISE);
						if(!ms)
						{
							ms2 =  g_pMagicCtrl->FindMagicByUnitID(4694,SELF.GetID());
							if(ms2)
							{
								g_pMagicCtrl->CreateMagic(ms2);
								g_pMagicCtrl->FinishMagic(ms2);
							}
						}

						g_pMagicCtrl->FinishMagic(ms);
						Next.cDir = SELF.GetDir();
					}
					else
					{
						Next.cDir = ((Next.cDir + 4) % 8);
					}
				}

				//计算目的地
				SELF.InitAction(&Next);

				////加快野蛮冲撞和突斩的人物移动速度
				if(Next.uFlag & COLLIDE_ACTION)
				{
					ActionNow.iFrameSpeed /= 2;
					SELF.SetDir(Next.cDir);
					SELF.SetOffset(0,0);

					if(Next.uFlag & COLLIDE_SHEN)
						ActionNow.uFlag |= COLLIDE_SHEN;
					else if (Next.uFlag & COLLIDE_MO)
						ActionNow.uFlag |= COLLIDE_MO;		
				}

				if((Next.uFlag & BACK_ACTION) && (Next.wAction == ACTION_WALK)) //倒退着走，被冲撞或被狮子吼
					ActionNow.iFrameSpeed /= 2;

				ActionNow.iStartX	= iSelfX;
				ActionNow.iStartY	= iSelfY;
				ActionNow.iMoveX	= (Next.iAimX - iSelfX) * TILE_WIDTH;
				ActionNow.iMoveY	= (Next.iAimY - iSelfY) * TILE_HEIGHT;
			}
			else
			{
				if(!PlayerMoveImmediate(Next,iSelfX,iSelfY))
				{
					if(!ActionNow.IsStand())
					{
						if(m_dwFrameCount >= ActionNow.tFrameStart + ActionNow.iFrameCount*ActionNow.iFrameSpeed + 2)//等2帧还不能走的话就变成站立
						{
							SELF.InitAction(ACTION_STAND);
							SELF.GetCNextAction().Clear();
						}
					}
					else if(ActionNow.IsEnd(m_dwFrameCount))//待机动作重复从头开始
					{					
						ActionNow.tFrameStart = m_dwFrameCount;
						ActionNow.iFrameNow = 0;
					}

					return;
				}
			}

			//翱风斩和纵雷诀特效
			if(Next.uFlag &  CREATE_MAGIC)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(Next.iData1,Next.uData1,Next.uData2,Next.uData3,Next.cDir*2);
				if(ms)
				{
					ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				}
			}

			break;
		}
	case ACTION_ATTACK1:
		{
			CSimpleCharacterNode *	pChar = NULL;
			CSimpleGoodNode	*		pGood = NULL;

			if(Next.uFlag & FORCE_ACTION)		// 强制攻击(按住shift)
			{
				pChar	= g_pGameData->FindSimpleCharacter(Next.uTarget);
				if(pChar)						// 有目标
				{
					pChar->GetRealXY(ix,iy);
				}
				else							// 没目标
				{
					GetDirStep(Next.cDir,ix,iy);
					ix += iSelfX;
					iy += iSelfY;
				}
			}
			else if(Next.uTarget)				// 一般攻击(点中目标)
			{
				pChar	= g_pGameData->FindSimpleCharacter(Next.uTarget);
				if(pChar && !pChar->IsDead())
				{
					pChar->GetRealXY(ix,iy);
				}
			}

			if(((pChar && !pChar->IsDead())|| (Next.uFlag & FORCE_ACTION)) && _closePoint(iSelfX,iSelfY,ix,iy,1))	//强行攻击和有目标攻击
			{
				if(!(Next.uFlag & SERVER_ACTION))//在追杀模式下,一直有一个客户端的攻击动作没有清除,跑到了以后要立即尝试使用突斩或冲撞,否则等做完了这个普通攻击动作可能人走了突不到
				{
					if( (SELF.GetCareer() == JOB_ZHANSHI && g_AutoFightMgr.AutoWildCollide()) ||
						(SELF.GetCareer() == JOB_FASHI && g_AutoFightMgr.AutoDisputeFire())   ||
						(SELF.GetCareer() == JOB_DAO && g_AutoFightMgr.AutoLionCall())           )
					{
						break;
					}
				}

				Next.cDir = GetDir8Attack(ix,iy,iSelfX,iSelfY);
				Next.iAimX = ix;
				Next.iAimY = iy;

				//服务器要求做的动作,只做动作不发协议
				if ((Next.uFlag & SERVER_ACTION) == 0)
				{
					PlayerAttack(Next,iSelfX,iSelfY);		
				}
			}
			else if(pChar && !pChar->IsDead() )
			{
				if(g_pInput->IsShift() || g_AICfgMgr.IsRunAttack() || Next.iData == 0)
				{
					Next.iAimX = ix;
					Next.iAimY = iy;
					Next.cDir = GetDir8Attack(ix,iy,iSelfX,iSelfY);
					Next.wAction = ACTION_RUN;

					if(!PlayerMoveImmediate(Next,iSelfX,iSelfY))
					{
						if(ActionNow.wAction != ACTION_STAND)
						{
							SELF.InitAction(ACTION_STAND);
						}
						else if(ActionNow.IsEnd(m_dwFrameCount))
						{
							ActionNow.tFrameStart = m_dwFrameCount;
							ActionNow.iFrameNow = 0;
						}
					}
				}
				else
				{
					SELF.InitAction(ACTION_STAND);
				}

				//不清除下一个动作
				return; 
			}
			else if(g_AIMgr.HaveChopEquip())
			{
				//服务器要求做的动作,只做动作不发协议
				if ((Next.uFlag & SERVER_ACTION) == 0)
				{
					PlayerAttack(Next,iSelfX,iSelfY);	
				}
				//不清除下一个动作
				return; 
			}
			else
			{
				SELF.InitAction(ACTION_STAND);
			}
			break;
		}
	case ACTION_MAGIC:
	case ACTION_SWT:
	case ACTION_LSJ:
	case ACTION_JYL:
		{
			if(Next.uFlag &  CREATE_MAGIC)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(Next.iData1,Next.uData1,Next.uData2,Next.uData3,Next.cDir*2);
				if(ms)
				{
					ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				}
			}
			else
			{
				//服务器要求做的动作,只做动作不发协议
				if ((Next.uFlag & SERVER_ACTION) == 0)
				{
					PlayerMagic(Next,iSelfX,iSelfY);	
				}
			}

			break;
		}
	case ACTION_CUT:
		{
			if (SELF.IsOnHorse())
			{
				SELF.InitAction(ACTION_STAND);
				break;
			}

			CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(Next.uTarget);
			if (pChar)
			{
				int iX,iY;
				pChar->GetXY(iX,iY);
				Next.cDir = (char)GetDir8(iX,iY,iSelfX,iSelfY);

				//if(g_AIMgr.CanDig(pChar))
				if(pChar->IsCanDig())
				{
					if(_closePoint(iSelfX,iSelfY,iX,iY,1))
					{
						//服务器要求做的动作,只做动作不发协议
						if ((Next.uFlag & SERVER_ACTION) == 0)
						{
							g_pGameControl->SEND_Player_Dig_Corpse(Next.uTarget,iX,iY,Next.cDir);
						}

						SELF.InitAction(&Next);
					}
					else
					{
						//服务器要求做的动作,只做动作不发协议
						if ((Next.uFlag & SERVER_ACTION) == 0)
						{
							g_pGameControl->SEND_Player_Squat(Next.cDir);
						}

						SELF.InitAction(&Next);
					}
				}
				else
				{
					//服务器要求做的动作,只做动作不发协议
					if ((Next.uFlag & SERVER_ACTION) == 0)
					{
						g_pGameControl->SEND_Player_Squat(Next.cDir);
					}

					SELF.InitAction(&Next);
				}
			}
			else
			{
				//服务器要求做的动作,只做动作不发协议
				if ((Next.uFlag & SERVER_ACTION) == 0)
				{
					g_pGameControl->SEND_Player_Squat(Next.cDir);
				}

				SELF.InitAction(&Next);
			}
			break;
		}
	case ACTION_ATTACKED:
		{
			SELF.InitAction(&Next);
			break;
		}
	case ACTION_SALUTE:
	case ACTION_LSALUTE:
		{
			/*if (SELF.IsOnLepoard() && !SELF.GetFightOnLeopard())
			Next.wAction = ACTION_LSALUTE;
			else*/
			//服务器要求做的动作,只做动作不发协议
			if ((Next.uFlag & SERVER_ACTION) == 0)
			{
				g_pGameControl->SEND_Player_Salute();
			}

			Next.cDir = SELF.GetDir();
			SELF.InitAction(&Next);
			break;
		}
	default:
		{
			SELF.InitAction(&Next);
			break;
		}
	}
TRY_END

	SELF.GetCNextAction().Clear();

}

void CGameManager::CharDoNextStep(CSimpleCharacterNode *pChar)
{
	if(!pChar)
		return;

	SAction& ActionNow = pChar->GetAction();
	WORD wOldAction = ActionNow.wAction;
	if(pChar->HasNextAction() == ENS_NO_ACTION || pChar->IsStopGo())
	{
		if(ActionNow.wAction == ACTION_ATTACK4)//(豹子)趴下    (卫士 弓箭守卫 下跪不动)
		{
			switch(pChar->GetRaceNo())
			{
			case 115://卫士
			case 116://弓箭守卫
				//case 2://豹子
			case 166://豹子
			case 167://豹子
			case 168://豹子
				//case 20://狮子
			case 185://狮子
			case 186://狮子
			case 187://狮子
				//case 38://小麒麟
			case 39://青麒麟
			case 40://黄麒麟
			case 41://红麒麟
			case 301://红凤凰
			case 302://紫凤凰
				pChar->InitAction(ACTION_ATTACK4); 
				return;
			}
		}

		pChar->SetSpeedup(0);
		pChar->InitAction(ACTION_STAND); //恢复待机
		return;
	}

	//计算加速的倍数
	int iNextNum = pChar->GetNextActionNum();
	if(iNextNum > 3)
		pChar->SetSpeedup(2);
	else if(iNextNum > 1)
		pChar->SetSpeedup(1);
	else
		pChar->SetSpeedup(0);

	SNextAction  Next;
	SNextAction* pNextAction = NULL;
	pNextAction = pChar->GetNextAction();
	if(pNextAction)
	{
		if((pNextAction->uFlag & BACK_ACTION) && (pNextAction->wAction == ACTION_WALK) && (pNextAction->uFlag & DELAY_ACTION))
		{
			ActionNow.tFrameStart = m_dwFrameCount;
			ActionNow.iFrameNow = 0;
			return;
		}		
	}
	if(!pChar->PopSNextAction(&Next)) //弹出最后一个动作
	{
		if(!pChar->IsSinging())
		{
			pChar->InitAction(ACTION_STAND);
		}
		return;
	}

	if(Next.cDir == DIR_NO) //动作没有设置方向
		Next.cDir = pChar->GetDir();

	int iX,iY;
	pChar->GetXY(iX,iY);

// 	if (Next.uFlag & MOVE_ACTION)
// 	{
// 		Next.cDir = GetDir8(Next.iAimX,Next.iAimY,iX,iY);
// 	}

	switch(Next.wAction)
	{
	case ACTION_WALK:
	case ACTION_RUN:
	case ACTION_LFMAGICFGH:
	case ACTION_LFMAGICRAB:
	case ACTION_LFMAGICTAO:
		{
			if(Next.wAction != ACTION_LFMAGICRAB)
				Next.cDir = GetDir8(Next.iAimX,Next.iAimY,iX,iY);

			//倒着走，被冲撞或被狮子吼
			if((Next.uFlag & BACK_ACTION) && (Next.wAction == ACTION_WALK))
			{
				if(Next.uData > 0 && Next.uData != 1)//被狮子吼后(Next.uData > 0)下一个动作不再是后退(Next.uData != 1)
				{
					Magic_Show_s* ms2 = NULL;
					Magic_Show_s* ms = g_pMagicCtrl->FindMagicByAttr(MAGICID_LION_CALLED,pChar->GetID(),EMP_MAGIC_SERVER_ADVISE);
					if(!ms)
					{
						ms2 = g_pMagicCtrl->FindMagicByUnitID(4694,pChar->GetID());
						if(ms2)
						{
							g_pMagicCtrl->CreateMagic(ms2);
							g_pMagicCtrl->FinishMagic(ms2);
						}
					}

					g_pMagicCtrl->FinishMagic(ms);
					Next.cDir = pChar->GetDir();
				}
				else
				{
					Next.cDir = (Next.cDir + 4) % 8;
				}
			}

			pChar->InitAction(&Next);

			////加快野蛮冲撞和突斩的人物移动速度
			//if(Next.uFlag & COLLIDE_ACTION)
			//    ActionNow.iFrameSpeed /= 2;

			//倒着走，被冲撞或被狮子吼
			//if((Next.uFlag & BACK_ACTION) && (Next.wAction == ACTION_WALK))
			//    ActionNow.iFrameSpeed /= 2;
			if(Next.uFlag & COLLIDE_ACTION)
			{
				if(Next.uFlag & COLLIDE_SHEN)
					ActionNow.uFlag |= COLLIDE_SHEN;
				else if (Next.uFlag & COLLIDE_MO)
					ActionNow.uFlag |= COLLIDE_MO;
			}

			ActionNow.iStartX = iX;
			ActionNow.iStartY = iY;
			ActionNow.iMoveX = (Next.iAimX - iX) * TILE_WIDTH;
			ActionNow.iMoveY = (Next.iAimY - iY) * TILE_HEIGHT;

			//翱风斩和纵雷诀特效
			if(Next.uFlag &  CREATE_MAGIC)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(Next.iData1,Next.uData1,Next.uData2,Next.uData3,Next.cDir*2);
				if(ms)
				{
					ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				}
			}

			break;
		}
	case ACTION_DEATH:
		{
			int x,y;
			pChar->GetXY(x,y);
			if( x!=Next.iAimX || y!=Next.iAimY )
			{
				pChar->SetXY(Next.iAimX,Next.iAimY);
				g_pGameData->AddSimpleCharacter(Next.iAimX,Next.iAimY,pChar->GetID());
			}

			if(pChar->IsMonster())
			{
				switch(pChar->GetRaceNo())
				{
				case 56:
				case 57:
				case 333:
				case 334:
				case 335:
				case 336:
					//开门
					{
						int iSelfX,iSelfY;
						SELF.GetXY(iSelfX,iSelfY);
						Next.cDir = 0;
						g_pAudio->PlayEx(EAT_OTHER,902,g_pAudio->GetRand()++,iSelfX,x,iSelfY,y,false);	
					}
					break;
				case 69:// 尸霸
					g_pMagicCtrl->CreateMagic(MAGICID_SHIBA_DEATH,0,pChar->GetID());
					break;
				case 198:// 通天教主
					g_pMagicCtrl->CreateMagic(MAGICID_TONGTIAN_DEATH,0,pChar->GetID());
					break;
				case 206: //炎魔2死亡效果
					g_pMagicCtrl->CreateMagic(MAGICID_YAN_MONSTER2_DEATH_EFFECT,0,pChar->GetID());
					break;
				case 215://地火兽死亡效果
				case 216:
				case 217:
					g_pMagicCtrl->CreateMagic(MAGICID_FLOOR_MONSTER_DEATH,0,pChar->GetID());
					break;
				case 221://....................  暗域之王死亡效果 ........................
					if(pChar->GetDir() < 4)
					{
						g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_DEATH1,0,pChar->GetID(),0,2*pChar->GetDir());
					}
					else
					{
						g_pMagicCtrl->CreateMagic(MAGICID_DARK_MONSTER_DEATH2,0,pChar->GetID(),0,2*pChar->GetDir());
					}
					break;
				case 236://....................  阿修罗神变身死亡效果 ........................
					g_pMagicCtrl->CreateMagic(MAGICID_AXIULUO_MONSTER_DEATH,0,pChar->GetID());	
					break;

				default:
					break;
				}
			}


			pChar->InitAction(&Next);
			break;
		}
	case ACTION_MAGIC:
	case ACTION_SWT:
	case ACTION_LSJ:
	case ACTION_JYL:
		{
			if(Next.uFlag &  CREATE_MAGIC)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(Next.iData1,Next.uData1,Next.uData2,Next.uData3,Next.cDir*2);
				if(ms)
				{
					ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
				}
				break;
			}

			WORD wMagicID = Next.iData; //魔法ID
			pChar->InitAction(&Next);


			//调整方向
			if(Next.iAimX != iX || Next.iAimY != iY )
			{
				int dir = GetDir8(Next.iAimX,Next.iAimY,iX,iY);
				pChar->SetDir(dir);
			}

			UINT uID = 0;
			UINT uMagicType = 0;

			if(Next.uTarget == SELF.GetID())
			{
				uID = SELF.GetID();
			}
			else
			{
				CSimpleCharacterNode * pTarget = g_pGameData->FindSimpleCharacter(Next.uTarget);
				if(pTarget)
				{
					uID = pTarget->GetID();
				}
				else
				{
					if(wMagicID == MAGICID_HEAL)
					{
						uID = pChar->GetID();//治愈术没有目标时放在自己身上
					}
					else
					{
						uID = MAKELONG(Next.iAimX,Next.iAimY);
						uMagicType |= EMP_MAGIC_NOTARGET;
					}
				}
			}

			if(pChar->GetFightOnLeopard())

			{
				wMagicID = GetFightOnLeopardMagicID(wMagicID);
				Next.iData = wMagicID;
			}


			uMagicType |= EMP_MAGIC_SERVER_ADVISE;

			//风火轮，护身和金刚的魔法在MSG_Monster_State中进行创建[20080716]
			//八方分影斩在MSG_ShadowKill8中创建
			//寒冰风暴MSG_Monster_ExtraState中创建
			WORD wDrawMagic = wMagicID;
			wMagicID = wMagicID % 1000;
			if(wMagicID != MAGICID_PROTECT_SKIN && wMagicID != MAGICID_STEEL_PROTECT && 
				wMagicID != MAGICID_ROTATE_FIRE && wMagicID != MAGICID_DAOZUN_JIANGLIN && 
				wMagicID != MAGICID_SHADOWKILL8 && wMagicID != MAGICID_COLDSTROM)
			{
				Magic_Show_s *ms = g_pMagicCtrl->CreateMagic(wDrawMagic,uMagicType,pChar->GetID(),uID);
				//如果这个魔法动作是在服务器已经返回成功之后才做（pNext->uData = 5），要去掉EMP_MAGIC_WAIT_SERVER标记
				if(Next.uData == 5 && ms)
				{
					ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
					if(Next.dwColor != 0)
						ms->dwColor = Next.dwColor;//颜色

					ms->ref->wTargetTileX = Next.iAimX;
					ms->ref->wTargetTileY = Next.iAimY;

					if(ms->ref)
						ms->ref->wData[1] = Next.iData2;//强化技能的魔法等级
				}
			}

			break;
		}
	case ACTION_ATTACK1:
		{

			pChar->InitAction(&Next);

			if(pChar->IsMonster() && pChar->GetRaceNo() == 115) //大刀，回到原来的地方
			{
				int cx,cy;
				pChar->GetXY(cx,cy);
				ActionNow.iAimX = cx;
				ActionNow.iAimY = cy;

				if(cx != Next.iAimX || cy != Next.iAimY)
				{
					cx = Next.iAimX;
					cy = Next.iAimY;
					g_pGameData->AddSimpleCharacter(cx,cy,pChar->GetID());
					pChar->SetXY(cx,cy);
					pChar->SetDeal(true);
				}
			}
			break;
		}
	case ACTION_APPEAR:
		pChar->SetOffset(0,0);
		pChar->InitAction(&Next);

		if(Next.iData == 20)
		{
			pChar->SetDir(Next.cDir);
			/*if (pChar->IsNpc())
			{
				SAction& saction = pChar->GetAction();
				saction.SetNpcRandStand(pChar->GetRaceNo());
			}
			else*/ pChar->InitAction(ACTION_STAND);
		}

		break;

	default:
		{
			pChar->InitAction(&Next);
			break;
		}
	}

	if (Next.uFlag & MOVE_ACTION)
	{
		ActionNow.iStartX = iX;
		ActionNow.iStartY = iY;
		ActionNow.iMoveX = (Next.iAimX - iX) * TILE_WIDTH;
		ActionNow.iMoveY = (Next.iAimY - iY) * TILE_HEIGHT;
	}
}

void CGameManager::SetMouseTile(int iX,int iY)
{
	m_iMouseTileX = iX;
	m_iMouseTileY = iY;
}

void CGameManager::GetMouseTile(int& x,int& y)
{
	x = m_iMouseTileX;
	y = m_iMouseTileY;
}

int CGameManager::GetPlayerDir(int iX,int iY)
{
	int iScrX = 0,iScrY = 0;
	SELF.GetScrXY(iScrX,iScrY);

	return GetDir8(iX,iY,iScrX+32,iScrY+16);
}

//是否允许主角跑步
bool CGameManager::PlayerCanRun()
{
	if(	SELF.GetPackageWeight() > SELF.GetPackageWeightMax() || SELF.GetHP() < 9)
		return false;

	if(SELF.IsBianShen())
		return false;

	return true;
}

bool CGameManager::IsCanGo(int iX,int iY,char cDir,int step)
{
	int dx = 0,dy=0;
	cDir = cDir % DIR_NUMS;
	GetDirStep(cDir,dx,dy);

	for(int i=1;i<=step;i++)
	{
		int x = iX+dx*i;
		int y = iY+dy*i;

		if(SELF.IsOppBlock(x,y))
			return false;
	}
	return true;
}

char CGameManager::GetGoDir2(int x,int y,char cDir,int step,BOOL bAttack,BOOL &bNeedRun)
{
	char goDir = DIR_NO;
	bNeedRun = false;

	char cRightDir  = (cDir+1) % DIR_NUMS;
	char cRightDir2 = (cDir+2) % DIR_NUMS;
	char cLeftDir   = (cDir+7) % DIR_NUMS;
	char cLeftDir2  = (cDir+6) % DIR_NUMS;

	if(IsCanGo(x,y,cDir))
	{
		goDir = cDir;
		if(step > 1 && IsCanGo(x,y,cDir,step))
			bNeedRun = true;
	}
	else if(IsCanGo(x,y,cLeftDir)) //Turn Left 1
	{
		goDir = cLeftDir;
		if(!bAttack && step>1 && IsCanGo(x,y,goDir,step))
			bNeedRun = true;
	}
	else if(IsCanGo(x,y,cRightDir)) //Turn Right 1
	{
		goDir = cRightDir;
		if(!bAttack && step>1 && IsCanGo(x,y,goDir,step))
			bNeedRun = true;
	}
	else if(IsCanGo(x,y,cLeftDir2))                //Turn Left 2
	{
		int dx=0,dy=0;
		GetDirStep(cLeftDir2,dx,dy);
		if(!IsCanGo(x+dx,y+dy,cLeftDir))
			goDir = DIR_NO;
		else
			goDir = cLeftDir2;
	}
	else if(IsCanGo(x,y,cRightDir2))               //Turn Right 2
	{
		int dx=0,dy=0;
		GetDirStep(cRightDir2,dx,dy);
		if(!IsCanGo(x+dx,y+dy,cRightDir))
			goDir = DIR_NO;
		else
			goDir = cRightDir2;
	}
	return goDir;
}

bool CGameManager::PlayerMove(SNextAction &Next,int iSelfX,int iSelfY,int iStep,BOOL bAttack,int *iIntegrity,int FailNoIntegrity)
{
	if(!g_AIMgr.IsCanMove()) //时间没有到，不需要重新计算路径的
		return true;

	SPoint	ptNext;
	bool	bFind		= false;		// 是否找路成功
	bool	bPathFollow	= false;		// 是否使用随路

	if(iStep > 1) //计算步骤
	{
		iStep = SELF.CalStepCount(Next.wAction);
	}

	m_PathFinder.SetOptimizeStep(iStep);

	UserPathFindingInfo_t ui;
	ui.SetValue(iSelfX,iSelfY,Next.iAimX,Next.iAimY);

	if(bAttack)
	{
		BOOL bBlock = SELF.BlockSwitch(Next.iAimX,Next.iAimY,false);
		bFind = m_PathFinder.GetPathClosePoint(ui,&ptNext,bAttack,0,iIntegrity);
		SELF.SetOppBlock(Next.iAimX,Next.iAimY,bBlock);
	}
	else 
	{
		bFind = m_PathFinder.GetPathClosePoint(ui,&ptNext,bAttack,0,iIntegrity);
	}

	if(bFind)
	{
		if(FailNoIntegrity && *iIntegrity)
		{
#ifdef _DEBUG
			output_debug("路径不完整(%d,%d)->(%d,%d)\n",iSelfX,iSelfY,Next.iAimX,Next.iAimY);
#endif
			return false;
		}
	}

	if(bFind)			// 移动过去
	{
		Next.cDir = GetDir8(ptNext.x,ptNext.y,iSelfX,iSelfY);
		SELF.SetDir(Next.cDir);

		if(iStep == 1 || !IsCanGo(iSelfX,iSelfY,Next.cDir,iStep)) //不能跑就走
		{
			int dx,dy;
			GetDirStep(Next.cDir,dx,dy);
			ptNext.x = iSelfX + dx;
			ptNext.y = iSelfY + dy;
		}

		if(abs(ptNext.x - iSelfX) > 1 || abs(ptNext.y - iSelfY) > 1)
		{
			Next.wAction = ACTION_RUN;
			g_pGameControl->SEND_Player_Run(ptNext.x,ptNext.y);
		}
		else
		{
			Next.wAction = ACTION_WALK;
			g_pGameControl->SEND_Player_Walk(ptNext.x,ptNext.y);
		}

		Next.iAimX = ptNext.x;
		Next.iAimY = ptNext.y;
		SELF.InitAction(&Next);

		SAction& ActionNow = SELF.GetAction();
		ActionNow.iStartX	= iSelfX;
		ActionNow.iStartY	= iSelfY;
		ActionNow.iMoveX	= (ptNext.x - iSelfX) * TILE_WIDTH;
		ActionNow.iMoveY	= (ptNext.y - iSelfY) * TILE_HEIGHT;

		return true;
	}
	else// 找不到路径,需要重新寻路得到新的路点
	{
		//下面寻路失败后转方向暂时注掉，因为被围了之后可能会导致每帧重复寻路而且方向一样，这样就每帧向服务顺发送协议
#ifdef _DEBUG
		output_debug("自动走路失败(%d,%d)->(%d,%d)\n",iSelfX,iSelfY,Next.iAimX,Next.iAimY);
#endif
		SELF.InitAction(ACTION_STAND);
		return false;
	}
}

// 角色移动
bool CGameManager::PlayerMoveImmediate(SNextAction& Next,int iSelfX,int iSelfY)
{
	if(!g_AIMgr.IsCanMove())
		return false;

	SPoint	ptNext;

	//计算步数
	BOOL bNeedRun = false;
	int iStep = SELF.CalStepCount(Next.wAction);

	if(!PlayerCanRun()) //怪物不能走
		iStep = 1;

	Next.cDir = GetGoDir2(iSelfX,iSelfY,Next.cDir,iStep,false,bNeedRun);
	if(Next.cDir == DIR_NO) //
		return false;

	if(bNeedRun)
		Next.wAction = ACTION_RUN;
	else
		Next.wAction = ACTION_WALK;

	if(!bNeedRun)
		iStep = 1;

	//计算目的地
	int dx,dy;
	GetDirStep(Next.cDir,dx,dy);
	ptNext.x = iSelfX + iStep*dx;
	ptNext.y = iSelfY + iStep*dy;

	Next.iAimX = ptNext.x;
	Next.iAimY = ptNext.y;

	SELF.InitAction(&Next);

	SAction& ActionNow = SELF.GetAction();
	ActionNow.iStartX	= iSelfX;
	ActionNow.iStartY	= iSelfY;
	ActionNow.iMoveX	= (ptNext.x - iSelfX) * TILE_WIDTH;
	ActionNow.iMoveY	= (ptNext.y - iSelfY) * TILE_HEIGHT;

	//发送协议
	if(bNeedRun)
		g_pGameControl->SEND_Player_Run(ptNext.x,ptNext.y);
	else
		g_pGameControl->SEND_Player_Walk(ptNext.x,ptNext.y);

	return true;
}

bool CGameManager::PlayerAttack(SNextAction& Next,int iSelfX,int iSelfY)
{
	if(!g_AIMgr.IsCanAttack())
		return false;

	WORD wMagicID = 0;
	bool bChop = g_AIMgr.HaveChopEquip();//挖矿

	DWORD dwCount = GetTickCount();

	if(Next.uTarget != 0)
	{
		//双烈火的时候 必须等准备好了才使用
		bool bNeedFire = true;
		if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsDoubleFire())
		{
			int iReadyDFire = g_AutoFightMgr.GetReadyDFike();
			if(iReadyDFire > 0 && (SELF.HasMagicState(MS_ATTACKFIRE) || SELF.HasMagicState(MS_THUNDERFIRE)))
			{
				iReadyDFire --;
				g_AutoFightMgr.SetReadyDFire(iReadyDFire);
				bNeedFire = true;
			}
			else
			{
				bNeedFire = false;
			}
		}

		if(bNeedFire)
		{
			if(SELF.HasMagicState(MS_ATTACKFIRE))
			{
				wMagicID = MAGICID_ATTACK_FIRE;
				SELF.RemoveMagicState(MS_ATTACKFIRE);
			}
			else if(SELF.HasMagicState(MS_THUNDERFIRE))
			{
				wMagicID = MAGICID_THUNDER_FIRE;
				SELF.RemoveMagicState(MS_THUNDERFIRE);
			}
		}

		if(wMagicID == 0 && !bChop && g_AICfgMgr.IsAutoSuperAttack() && SELF.HasMagicState(MS_SATTACKKILL)) //////////////////没有使用烈火,使用强化攻杀剑法
		{
			if((dwCount- g_AIMgr.GetReserveTime(plyLastSAttackKill)) >= 5000)
			{
				g_AIMgr.SetReserveTime(plyLastSAttackKill,dwCount);
				wMagicID = MAGICID_SUPER_ATTACK_KILL;
			}
		}

		if(wMagicID == 0) //没有使用其他技能
		{
			if(SELF.HasMagicState(MS_ATTACKKILL))
			{
				wMagicID = MAGICID_ATTACK_KILL;
				SELF.RemoveMagicState(MS_ATTACKKILL);
			}
			else if(SELF.HasMagicState(MS_REMAINSHADE))
			{
				wMagicID = MAGICID_REMAIN_SHADE;
				SELF.RemoveMagicState(MS_REMAINSHADE);
			}
			else if(SELF.HasMagicState(MS_BLOODSHADE))
			{	
				wMagicID = MAGICID_BLOOD_SHADE;
				SELF.RemoveMagicState(MS_BLOODSHADE);
			}
		}

		if(wMagicID == 0 && SELF.HasMagicState(MS_DESTROYSHIELD))
		{
			if(SELF.GetMP() > SELF.GetNeedMP(MAGICID_DESTROY_SHIELD))
				wMagicID = MAGICID_DESTROY_SHIELD;
		}
		if(wMagicID == 0 && SELF.HasMagicState(MS_DESTROYSHELL))
		{
			if(SELF.GetMP() > SELF.GetNeedMP(MAGICID_DESTROY_SHELL))
				wMagicID = MAGICID_DESTROY_SHELL;
		}

		if(wMagicID == 0 && SELF.HasMagicState(MS_ATTACKMOON))
		{
			if(SELF.GetMP() > SELF.GetNeedMP(MAGICID_ATTACK_MOON))
				wMagicID = MAGICID_ATTACK_MOON;
		}

		if(wMagicID == 0 && SELF.HasMagicState(MS_WHOOLEMOON))
		{
			if(SELF.GetMP() > SELF.GetNeedMP(MAGICID_WHOLE_MOON))
				wMagicID = MAGICID_WHOLE_MOON;
		}
	}

	if(wMagicID == 0 && SELF.HasMagicState(MS_ATTACKSTICK))			// 刺杀
	{
		int ax = 0;
		int ay = 0;
		GetDirStep(Next.cDir,ax,ay);
		if(Next.uFlag & FORCE_ACTION)				// 强行攻击
		{
			CSimpleCharacterNode * p = MapArray.FindSimpleCharacter(iSelfX + 2 * ax,iSelfY + 2 * ay);
			if(p && !p->IsDead())
			{
				wMagicID = MAGICID_ATTACK_STICK;
			}
		}
		else								
		{
			CSimpleCharacterNode *p1,*p2;
			p1 = MapArray.FindSimpleCharacter(iSelfX + ax,iSelfY + ay);
			p2 = MapArray.FindSimpleCharacter(iSelfX + 2 * ax,iSelfY + 2 * ay);
			if(p1 && p2 && (!p1->IsDead()) && (!p2->IsDead()))		// 判断
			{
				wMagicID = MAGICID_ATTACK_STICK;
			}
		}
	}

	if(wMagicID > 0 && !bChop && (Next.uTarget != 0 || wMagicID == MAGICID_ATTACK_STICK))
	{
		Next.uFlag |= MAGIC_ACTION;

		CMagicData * pMagic = SELF.FindMagic(wMagicID);
		if (pMagic)
		{
			Next.dwColor = pMagic->GetColor();// SELF.GetMagicColor(wMagicID);
			Next.iData = GetGreateMagicID(wMagicID,/*SELF.GetFlyType()*/1,pMagic->GetMagicLevel(),SELF.IsInDaoZunJiangLinState(),SELF.GetFightOnLeopard());
		}
	}

	//处理训马，挖矿等
	CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(Next.uTarget);
	if(bChop)//挖矿
	{
		g_pGameControl->SEND_Player_Force_Attack(Next.cDir);
		Next.iData = 5;
	}
	else if(pChar && pChar->IsHorse() && g_AIMgr.HaveTrainEquip())//训马
	{
		if(pChar->IsPet())
		{
			if(!g_pInput->IsShift() && !(g_AICfgMgr.IsRunAttack()))//追杀模式
			{
				SELF.InitAction(ACTION_STAND);
				return true;
			}
		}

		g_pGameControl->SEND_Player_Train_Horse(Next.iAimX,Next.iAimY); 
	}
	else
	{
		if(wMagicID > 0 && (Next.uTarget != 0 || wMagicID == MAGICID_ATTACK_STICK))
			g_pGameControl->SEND_Player_Attack_Sword(Next.cDir,wMagicID);
		else
			g_pGameControl->SEND_Player_Normal_Attack(Next.cDir,Next.uTarget);
	}

	SELF.InitAction(&Next);
	g_AIMgr.UpdateAttackTime(); //设置攻击时间

	return true;
}

bool  CGameManager::PlayerMagic(SNextAction& Next,int iSelfX,int iSelfY)
{
	//注：动作不变的直接return，否则只能break

	WORD wMagicID = Next.iData;
	WORD wDrawMagicID = Next.iData;
	int mx,my;
	GetMouseTile(mx,my); //鼠标所在格子

	switch(wMagicID)
	{
		// 以下部分都是发送的，不需要效果
	case MAGICID_FIGHTER_WILD:			// 战士兽化发送
	case MAGICID_TAOIST_WILD:			// 道士兽化发送
		{
			g_pGameControl->SEND_Player_Attack_Magic(0,SELF.GetDir(),0,wMagicID);
			return true;
		}
	case MAGICID_ENCHANTER_WILD:		// 魔法师兽化发送
	case MAGICID_ESCAPE_QUICK:         // 道士遁地
	case MAGICID_SHIFT_TRANSPORT:      // 战士移形换位发送
		{
			int iRealX = 0,iRealY = 0;
			SELF.GetRealXY(iRealX,iRealY);

			//if(iSelfX != mx || iSelfY != my)
			if(iRealX != mx || iRealY != my)
			{
				char dir = GetDir8(mx,my,iSelfX,iSelfY);
				SELF.SetDir(dir);
				g_pGameControl->SEND_Player_Attack_Magic(0,mx,my,wMagicID);
			}
			SELF.InitAction(ACTION_STAND);
			return true;
		}
	case MAGICID_WILD_COLLIDE:		// 野蛮冲撞发送
	case MAGICID_SUDDEN_KILL:       // 突斩
		{
			int iDir = SELF.GetDir();
			int iMx,iMy;
			int iX = mx,iY = my;
			SELF.GetXY(iMx,iMy);//定向突斩要根据自己和鼠标的位置,所以不用GetRealXY()

			if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsDirWild()) //定向突斩
			{
				///////////////而且如果有目标了，转方向,因为像骑战移了位置后如果点中目标使用定向突斩会老突不中
				UINT uID = g_pControl->GetMouseOnID();
				if(uID != 0 && uID != SELF.GetID())
				{
					CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(uID);
					if(pChar)
					{
						pChar->GetRealXY(iX,iY);
					}   
				}

				iDir = GetDir8(iX,iY,iMx,iMy);
			}

			if(iDir != SELF.GetDir())
			{
				g_pGameControl->SEND_Player_Turn(iDir);
				SELF.SetDir(iDir);
			}

			g_pGameControl->SEND_Player_Attack_Magic(0,iDir,0,wMagicID);
			SELF.InitAction(ACTION_STAND);
			return true;
		}
	default:								// 一般的魔法
		{
			DWORD uMagicType = 0;

			UINT uID = Next.uTarget;
			CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(uID);
			
// 			if (wMagicID == MAGICID_CALL_MONSTER ||
// 				wMagicID == MAGICID_CALL_JOSS ||
// 				wMagicID == MAGICID_ENCALL_MONSTER)
// 			{	
// 				if (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain())
// 				{
// 					mx = iSelfX + 1;
// 					my = iSelfY + 1;
// 				}
// 			}

			if(uID == SELF.GetID())
			{
				SELF.GetRealXY(Next.iAimX,Next.iAimY);
			}
			else if(pChar && (!pChar->IsDead() || wMagicID == MAGICID_CONTROL_CORPSE))
			{
				pChar->GetRealXY(Next.iAimX,Next.iAimY);
			}
			else
			{
				Next.iAimX = mx;
				Next.iAimY = my;

				if(wMagicID == MAGICID_HEAL)
				{
					uID = SELF.GetID();/////治愈术没有目标时放在自己身上
				}
				else
				{
					uID = MAKELONG(mx,my);
					uMagicType |= EMP_MAGIC_NOTARGET;
				}
			}

			//计算出方向
			Next.cDir = SELF.GetDir();
			switch (wMagicID)
			{
			case MAGICID_HELL_FIRE:
			case MAGICID_TRACE_THUNDER:
			case MAGICID_ICE_DRAG:
			case MAGICID_SWT:
			case MAGICID_LEOPARD_SWT:
				Next.cDir = GetLineDirect(iSelfX,iSelfY,Next.iAimX,Next.iAimY);
				if (Next.cDir < 0 || Next.cDir > 7)
				{
					Next.cDir = SELF.GetDir();
					if(Next.cDir < 0 || Next.cDir > 7)
						Next.cDir = 0;
				}
				break;
			default:
				{
					if(Next.iAimX != iSelfX || Next.iAimY != iSelfY)
						Next.cDir = GetDir8(Next.iAimX,Next.iAimY,iSelfX,iSelfY);;
				}
				break;
			}


			if(Next.iLooks != 0)
			{
				g_pGameControl->SEND_Player_Use_Magic_Reel(Next.uTarget,Next.iAimX,Next.iAimY,wMagicID,(DWORD)Next.iLooks);

				switch(wMagicID)
				{
				case MAGICID_DESTROY_SHELL:
				case MAGICID_DESTROY_SHIELD:
				case MAGICID_THUNDER_FIRE:
				case MAGICID_WHOLE_MOON:
				case MAGICID_ATTACK_FIRE:
				case MAGICID_ATTACK_MOON:
				case MAGICID_ATTACK_STICK:
				case MAGICID_ATTACK_KILL:
				case MAGICID_REMAIN_SHADE:
				case MAGICID_BLOOD_SHADE:
				case MAGICID_SUPER_ATTACK_KILL: //强化系列技能
					{
						Next.wAction = ACTION_ATTACK1;
						Next.uFlag = MAGIC_ACTION;
					}
					break;
				}

			}
			else
			{
				bool bNeedSend = true;//是否要发送使用魔法的协议
				CMagicData * pMagic = SELF.FindMagic(wMagicID);
				//if (g_dwServerVersion < 1070)
				//{
				//	g_AIMgr.AutoPlaceEquip(wMagicID); //自动上符和上毒
				//}
				//else
				{
					//现在符和毒直接在包裹里使用了
					if (!g_AIMgr.HasMagicEquip(wMagicID,false,true))
					{
						bNeedSend = false;
					}
				}

				if(wMagicID == MAGICID_LION_CALL) 		
				{
					if(g_AICfgMgr.IsSmartLionCall())//定向狮子吼
					{
						int iX = mx,iY = my;
						int attack_dir = GetDir8(iX,iY,iSelfX,iSelfY);
						SELF.SetDir(attack_dir);
						Next.cDir = attack_dir;
					}
					g_pGameControl->SEND_Player_Turn(Next.cDir);
				}

				if(wMagicID == MAGICID_SWT || wMagicID == MAGICID_LSJ || wMagicID == MAGICID_JYL)
				{

				}
				else
				{
					if (bNeedSend)
					{
						g_pGameControl->SEND_Player_Attack_Magic(Next.uTarget,Next.iAimX,Next.iAimY,wMagicID);	
					}
				}

				WORD wFightOnLeopardMagicID = GetFightOnLeopardMagicID(wMagicID);
				if(SELF.GetFightOnLeopard() && wFightOnLeopardMagicID != wDrawMagicID)
				{
					wDrawMagicID = wFightOnLeopardMagicID;
				}
				//风火轮，护身和金刚的魔法在MSG_Monster_State中进行创建[20080716]
				//八方分影斩在MSG_ShadowKill8中创建
				if (wMagicID != MAGICID_PROTECT_SKIN && wMagicID != MAGICID_STEEL_PROTECT && 
					wMagicID != MAGICID_ROTATE_FIRE && wMagicID != MAGICID_DAOZUN_JIANGLIN &&
					wMagicID != MAGICID_SHADOWKILL8)
				{
					if (wMagicID == wDrawMagicID)//wMagicID != wDrawMagicID的情况下,如使用真灵玉符,应该使用真灵玉符上的神魔属性,不能根据自己的神魔属性及技能等级来
					{
						if (pMagic)	wDrawMagicID = GetGreateMagicID(wMagicID,/*SELF.GetFlyType()*/1,pMagic->GetMagicLevel(),SELF.IsInDaoZunJiangLinState(),SELF.GetFightOnLeopard());
					}

					Magic_Show_s * ms = g_pMagicCtrl->CreateMagic(wDrawMagicID,uMagicType,SELF.GetID(),uID,-1,SELF.GetMagicColor(wMagicID));

					//如果这个魔法动作是在服务器已经返回成功之后才做（pNext->uData = 5），要去掉EMP_MAGIC_WAIT_SERVER标记
					if(Next.uData == 5 && ms)
					{
						ms->attr &= ~EMP_MAGIC_WAIT_SERVER;
						if(Next.dwColor != 0)
							ms->dwColor = Next.dwColor;//颜色

						if(ms->ref)
							ms->ref->wData[1] = Next.iData2;//强化技能的魔法等级
					}
				}

				if(wMagicID == MAGICID_FIGHTER_FIGHTER_KILL)
				{
					Next.wAction = ACTION_ATTACK1;
				}
				else if (wMagicID == MAGICID_SHADOWKILL8)
				{
					Next.wAction = ACTION_STAND;
				}
			}

			break;	
		}
	}
	SELF.InitAction(&Next);
	return true;
}

// 点击某角色的处理
bool CGameManager::OnClickChar(UINT uID)
{
	CSimpleCharacter *pChar = g_pGameData->FindSimpleCharacter(uID);
	if(!pChar)
	{
		GotoMousePos(FALSE);
		return true;
	}


	if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsRunAttack() && pChar && !pChar->GetBoothState()) //追杀
	{
		if(pChar->IsHuman() || pChar->IsMonster())
		{
			if(SELF.IsBianShen())
				uID = 0;

			ForceAttack(uID);
			return true;
		}
	}

	switch(pChar->GetRaceType())
	{
	case CHARACTER_NPC:
		{
			OnClickNpc(pChar);
			g_NPC.GetVectorPTData().clear();
			g_pGameControl->SEND_Exchange_Goin(uID); 
			break;
		}
	case CHARACTER_MONSTER:				// 表示为怪物
		{
			OnClickMonster(pChar);
			break;
		}
	case CHARACTER_HUMAN:				// 一般玩家
		{
			g_pGameControl->Send_Player_Prompt_Status(25,1);
			if (pChar->GetBoothState()) //点击摆摊者			
			{
				g_pGameControl->SEND_Exchange_Goin(uID);
			}

			break;
		}
	default:
		{
			GotoMousePos(FALSE);
			break;
		}
	}
	return true;
}

void CGameManager::OnClickNpc(CSimpleCharacter* pChar)
{
	if (!pChar) return;

// 	//微操及附身时不能点npc
// 	if (g_TrusteeshipData.IsTrusteeship() && (!g_TrusteeshipData.IsCaptain() || g_TrusteeshipData.GetCaptionSubstitutePos() >= 0 || g_TrusteeshipData.GetMicroControlPos() >= 0))
// 	{
// 		return;
// 	}
	
	const char* pName = pChar->GetName();
	if (strcmp(pName,"老兵") == 0 ||
		strcmp(pName,"铁匠") == 0 ||
		strcmp(pName,"药店掌柜") == 0 ||
		strcmp(pName,"仓库保管员") == 0)
	{
		for (int i=31;i<=43;i++)
		{
			if (!g_pGameData->HasStatus(i))
			{
				g_pGameControl->Send_Player_Prompt_Status(i,1);
				break;
			}
		}

	}
	
}

void CGameManager::OnClickMonster(CSimpleCharacter *pChar)
{
	if(!pChar)
		return;

	if(!pChar->IsDead())
	{
		DWORD dwID = pChar->GetID();
		if(pChar->IsPet() && !pChar->IsPetOfHeartMonster())
		{
			GotoMousePos(FALSE);
			return;
		}

		if(SELF.IsOnHorse() && !SELF.GetFightOnLeopard())
			return;
		SNextAction& Next = SELF.GetCNextAction();
		Next.Clear();
		Next.wAction	= ACTION_ATTACK1;
		Next.uTarget	= dwID;
		Next.cDir       = GetPlayerDir(m_iMouseX,m_iMouseY);

		SELF.SetReserveData(plyAttackLockID,dwID);
		SELF.SetReserveData(plyMagicLockID,0);

		g_pGameControl->Send_Player_Prompt_Status(26,1);			
	}
	else
	{
		GotoMousePos(FALSE);
	}
}

bool CGameManager::OnRClickChar(UINT uID)
{
	if (uID == 0)
		return false;

	CSimpleCharacterNode * pChar;
	int x,y;

	// 得到点击的对象的ID
	pChar = g_pGameData->FindSimpleCharacter(uID);
	if (!pChar)
		return false;

	if (pChar->IsMonster())
	{
		int iRaceNo = pChar->GetRaceNo();
		if(iRaceNo == 58)
		{
			g_pGameControl->SEND_Message_See_Mon(uID,3);
			return true;
		}
		else if(iRaceNo >= 251 && iRaceNo <= 253) //常胜玄兵
		{
			g_pGameControl->SEND_Message_See_Mon(uID,1);
			return true;
		}
	}
	else if(pChar->IsHuman())
	{
		pChar->GetRealXY(x,y);
		g_pGameControl->SEND_Look_Player_Info(uID,x,y,pChar->GetDir()); 
		return true;
	}
	return true;
}


bool CGameManager::OnLeftButtonDown(int iX,int iY)
{
	m_bMouseDClick	= false;
	UINT uID = g_pControl->GetMouseOnID();
	if (uID == SELF.GetID())
		uID = 0;

	CSimpleCharacter* pChar = NULL;
	if(uID != 0)
		pChar = g_pGameData->FindSimpleCharacter(uID);


//  	if (g_TrusteeshipData.IsTrusteeship())
// 	{
// 		if (!g_TrusteeshipData.IsCaptain()) 
// 			return true;
// 
// 		if (!g_pInput->IsCtrl() && g_pGameData->GetMouseType() == MOUSE_STANDARD)
// 		{
// 			for (int i = 0; i < MAX_TRUSTEESHIP_NUM; i++)
// 			{
// 				g_TrusteeshipData.GetTneupMember(i).bSelected = false;
// 			}
// 		}
// 
// 		if (g_TrusteeshipData.IsSelForceAttObj())
// 		{
// 			g_pGameData->SetMouseType(MOUSE_STANDARD);
// 			g_TrusteeshipData.SetSelForceAttObj(false);
// 
// 			UINT uID = g_pControl->GetMouseOnID();
// 			if (uID == SELF.GetID()) uID = 0;
// 			if (uID)
// 			{
// 				for (int i = 0; i < MAX_TRUSTEESHIP_NUM; i++)
// 				{
// 					if (g_TrusteeshipData.GetTneupMember(i).bSelected)
// 					{
// 						g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeAttack, uID);
// 					}
// 				}
// 			}
// 			m_bCannotMove = true;
// 			return true;
// 		}
// 
// 		if (g_TrusteeshipData.IsSelGarrsionXY())
// 		{
// 			g_pGameData->SetMouseType(MOUSE_STANDARD);
// 			g_TrusteeshipData.SetSelGarrsionXY(false);
// 
// 			int dwMouseX,dwMouseY;
// 			g_pGameMgr->GetMouseTile(dwMouseX,dwMouseY);
// 			WORD wX = dwMouseX, wY = dwMouseY;
// 			for (int i = 0; i < MAX_TRUSTEESHIP_NUM; i++)
// 			{
// 				if (g_TrusteeshipData.GetTneupMember(i).bSelected)
// 				{
// 					g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeGarrsion, MAKELONG(wX, wY));
// 				}
// 			}
// 			m_bCannotMove = true;
// 			return true;
// 		}
// 
// 		if (uID > 0)
// 		{
// 		 	TneupMember* pMember = g_TrusteeshipData.FindMemberByID(uID);
// 			if (pMember != NULL)
// 			{
// 				pMember->bSelected = !pMember->bSelected;
// 				return true;
// 			}	
// 		}
// 
// 		if (GetKeyState(VK_CONTROL)<0)
// 		{
// 			g_TrusteeshipData.SetSelecting(true);
// 			RECT& rc = g_TrusteeshipData.GetSelRect();
// 			rc.left = iX;
// 			rc.top = iY;
// 			rc.right = iX;
// 			rc.bottom = iY;
// 			m_bCannotMove = true;
// 			return true;
// 		}
// 	}
// 
// 

	

	//炼化葫芦
	if (g_pGameData->GetMouseType() == MOUSE_HULU)
	{
		g_pGameData->SetMouseType(MOUSE_STANDARD);
		if(pChar && !pChar->IsDead() && pChar->GetRaceType() == CHARACTER_MONSTER )
		{
			int iHuLuPos = g_pGameData->GetUseHuLuGoodPos();
			CGood& tmp = SELF.GetPackageGood(iHuLuPos);
			if (g_AIGoodMgr.IsLianHuLu(tmp))
			{
				g_pGameControl->SEND_Use_Object(iHuLuPos);
			}
		}
		m_bCannotMove = true;
		return true;
	}

	//左键点击人物时显示人物头像
	if(uID != 0)
	{
		if (pChar && pChar->IsNpc())
			m_bCannotMove = true;

		if(pChar && pChar->IsHuman())
		{
			g_pControl->PopupWindow(MSG_CTRL_PERSON_PANEL_WND,OPER_RECREATE,uID);
		}
		else
		{
			g_pControl->PopupWindow(MSG_CTRL_PERSON_PANEL_WND,OPER_CLOSE);
		}

		if (pChar && !pChar->IsNpc())
		{
			SELF.SetReserveData(plyClickCharID,uID);
		}
	}
	else
	{
		SELF.SetReserveData(plyClickCharID,0);
		g_pControl->PopupWindow(MSG_CTRL_PERSON_PANEL_WND,OPER_CLOSE);
	}

	if(SELF.GetBoothState())
	{
		if(uID != 0)
		{
			if(pChar)
			{
				switch(pChar->GetRaceType())
				{
				case CHARACTER_HUMAN:				// 一般玩家
					if (pChar->GetBoothState()) //点击摆摊者			
					{
						g_pGameControl->SEND_Exchange_Goin(uID);
					}
					break;
				}
			}
		}

		return true;
	}

	if(SELF.IsBianShen())
	{
		GotoMousePos(FALSE);
		return true;
	}

	if(SELF.IsDead()) //死亡以后不能动
		return true;

	SELF.SetReserveData(plyAttackLockID,0); //先清掉

	SELF.GetMapPathFinder().SetOnRoute(0);
	g_MapFinder.SetWalking(false);

	SNextAction& Next = SELF.GetCNextAction();
	int iSelfX,iSelfY,iDigX,iDigY;
	BYTE byDir = GetPlayerDir(iX,iY);
	SELF.GetXY(iSelfX,iSelfY);
	GetDirStep(byDir,iDigX,iDigY);
	iDigX += iSelfX;
	iDigY += iSelfY;

	if(g_pInput->IsShift())				// 强行攻击
	{
		m_FollowChar.SetChar();
		ForceAttack(uID);
	}
	else if(g_pGameMap->IsBlock(iDigX,iDigY) && g_AIMgr.HaveChopEquip())//挖矿			
	{
		Next.Clear();
		Next.wAction	= ACTION_ATTACK1;
		//Next.iData = 5;
		Next.cDir = byDir;
		Next.uTarget  = 0;
	}
	else if(g_pInput->IsAlt())
	{
		Next.Clear();
		Next.wAction	= ACTION_CUT;
		Next.uTarget	= uID;
		Next.cDir		= byDir;
	}
	else// 走路,拣物
	{
		if(m_iMouseTileX == iSelfX && m_iMouseTileY == iSelfY)
		{
			CSimpleGoodNode* pGood = MapArray.GetSimpleGoodHead(iSelfX,iSelfY);
			if(pGood)//脚下有物品，拣取脚下的物品
			{
				g_pGameControl->SEND_Object_Pickup(iSelfX,iSelfY);
				return true;
			}
		}

		if(uID == 0)
		{
			if(CGoodGrid::GetDropGoodFrom().DropGood.GetID() == 0)
				GotoMousePos(FALSE);
		}
		else
		{
			OnClickChar(uID);
		}
	}
	return true;
}

void CGameManager::ForceAttack(UINT uID)
{
	if(SELF.IsOnHorse() && !SELF.GetFightOnLeopard())
		return;

	SNextAction& Next = SELF.GetCNextAction();

	Next.Clear(); 
	Next.uFlag	|= FORCE_ACTION;
	Next.wAction	= ACTION_ATTACK1;
	Next.uTarget	= uID;
	Next.iData = 2;

	SELF.SetReserveData(plyAttackLockID,uID);

	if(uID != 0)
	{
		SELF.SetReserveData(plyMagicLockID,0);
	}
	else
	{
		Next.cDir = GetPlayerDir(m_iMouseX,m_iMouseY);
	}
}

void CGameManager::GotoMousePos(BOOL bNeedRun,BOOL bNeedCheckTurn)
{
	if (SELF.GetBoothState())
		return;
	if(SELF.IsWaitServer())
		return;

	int itx,ity,iSelfX,iSelfY;
	GetMouseTile(itx,ity);
	SELF.GetXY(iSelfX,iSelfY);

	if(itx == iSelfX && ity == iSelfY)
		return;

	if(bNeedCheckTurn && _closePoint(itx,ity,m_iPlayerX,m_iPlayerY,1))
	{
		int iDir = GetPlayerDir(m_iMouseX,m_iMouseY);
		DWORD dwTime = GetTickCount();
		if(iDir != SELF.GetDir() && dwTime - SELF.GetReserveData(plyLastTurnTime) > 500)
		{
			SELF.SetDir(iDir);
			g_pGameControl->SEND_Player_Turn(char(iDir));
		}
		return;
	}

	SNextAction& Next = SELF.GetCNextAction();

	Next.wAction = bNeedRun ? ACTION_RUN : ACTION_WALK;
	Next.iAimX   = itx;
	Next.iAimY   = ity;
	Next.cDir    = GetPlayerDir(m_iMouseX,m_iMouseY);
}

bool CGameManager::OnLeftButtonUp(int iX,int iY)
{
// 	if (g_TrusteeshipData.IsTrusteeship())
// 	{
// 		if (!g_TrusteeshipData.IsCaptain()) 
// 			return true;
// 
// 		if (g_TrusteeshipData.IsSelecting())
// 		{
// 			RECT& rc = g_TrusteeshipData.GetSelRect();
// 			rc.right = iX;
// 			rc.bottom = iY;
// 		}
// 	}
// 
// 	g_TrusteeshipData.SetSelecting(false);
// 	g_pGameData->SetMouseType(MOUSE_STANDARD);
// 	g_TrusteeshipData.SetSelGarrsionXY(false);
// 	g_TrusteeshipData.SetSelForceAttObj(false);

	m_dwAutoDigID = 0;
	SNextAction& Next = SELF.GetCNextAction();
	if(Next.IsMove())
	{
		Next.Clear();
	}

	m_bCannotMove = false;
	return true;
}

bool CGameManager::OnLeftButtonDClick(int iX,int iY)
{
// 	if (g_TrusteeshipData.IsTrusteeship())
// 	{
// 		if (!g_TrusteeshipData.IsCaptain()) return true;
// 	}

	m_dwAutoDigID = 0;

	DWORD dwMouseOnID = g_pControl->GetMouseOnID();
	if (!g_pInput->IsAlt() && !g_pInput->IsShift() && !g_pInput->IsCtrl() && dwMouseOnID)
	{
// 		TneupMember* pMember = g_TrusteeshipData.FindMemberByID(dwMouseOnID);
// 		if (pMember != NULL && pMember->dwID > 0 && pMember->wMaxHP > 0)
// 		{
// 			g_TrusteeshipData.MicroControlMember(pMember->byPos);
// 		}
// 		else
		{
			CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(dwMouseOnID);
			if(pChar && pChar->IsHuman())
			{
				g_pControl->Msg(MSG_CTRL_INSERT_TEXT,3,(CControl *)(pChar->GetName()));
			}
		}

		return true;
	}

	if(dwMouseOnID && g_pInput->IsAlt())
	{
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(dwMouseOnID);
		if(pChar)
		{
			int iSelfX,iSelfY;
			SELF.GetXY(iSelfX,iSelfY);
			if(_closePoint(iSelfX,iSelfY,pChar->GetX(),pChar->GetY(),1))
				m_dwAutoDigID = dwMouseOnID;
		}
	}


	return true;
}
bool CGameManager::OnRightButtonDown(int iX,int iY)
{
// 	if (g_TrusteeshipData.IsTrusteeship())
// 	{
// 		if (!g_TrusteeshipData.IsCaptain()) 
// 			return true;
// 
// 		g_TrusteeshipData.SetSelecting(false);
// 		g_pGameData->SetMouseType(MOUSE_STANDARD);
// 		g_TrusteeshipData.SetSelGarrsionXY(false);
// 		g_TrusteeshipData.SetSelForceAttObj(false);
// 	}

	
	if (g_pGameData->GetMouseType() == MOUSE_HULU || 
		g_pGameData->GetMouseType() == MOUSE_BUBING)
	{
		g_pGameData->SetMouseType(MOUSE_STANDARD);
	}

	m_bMouseDClick	= false;

	UINT uID = g_pControl->GetMouseOnID();

	if (g_pInput->IsAlt() && uID)
	{
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(uID);
		if(pChar && pChar->IsHuman())
		{
			string str = pChar->GetName();
			str += " ";

			g_pControl->Msg(MSG_CTRL_INSERT_TEXT,1,(CControl *)(str.c_str()));
		}
		return true;
	}

	if(g_pInput->IsCtrl() && uID != 0)
	{
		m_bCannotMove = true;
		OnRClickChar(uID);		
		return true;
	}

	if(SELF.GetBoothState() || SELF.IsDead())
		return true;

	SELF.GetMapPathFinder().SetOnRoute(0);

	if(SELF.GetHP() < 9 || SELF.GetPackageWeight() > SELF.GetPackageWeightMax() || SELF.IsBianShen())
		GotoMousePos(FALSE,TRUE);
	else
		GotoMousePos(TRUE,TRUE);

	return true;
}
bool CGameManager::OnRightButtonUp(int iX,int iY)
{
	m_bCannotMove = false;
	m_dwAutoDigID = 0;
	SNextAction& Next = SELF.GetCNextAction();
	if(Next.IsMove())
	{
		Next.Clear();
	}

	return true;
}

bool CGameManager::OnRightButtonDClick(int iX,int iY)
{
	return true;
}
bool CGameManager::OnMiddleButtonDown(int iX,int iY)
{
	return true;
}

bool CGameManager::OnMouseMove(int iX,int iY)
{
// 	if (g_TrusteeshipData.IsCaptain() && g_TrusteeshipData.IsSelecting())
// 	{
// 		RECT& rc = g_TrusteeshipData.GetSelRect();
// 		rc.right = iX;
// 		rc.bottom = iY;
// 		if (GetKeyState(VK_CONTROL)>=0)
// 		{
// 			g_TrusteeshipData.SetSelecting(false);
// 		}
// 		return true;
// 	}

	SNextAction& Next = SELF.GetCNextAction();

	if(m_bLeftDown || m_bRightDown || (g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsRunNotStop()))
	{
		if(Next.IsMove())
		{
			Next.cDir = GetPlayerDir(iX,iY);
		}
	}

	return true;
}

bool CGameManager::OnKeyDown(WORD wState,UCHAR cKey)
{
	switch(cKey)
	{
	case VK_F1:		
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
		{
			bool bHighKey = g_pInput->IsEscape();
			int iMagicID = 0;
			ConSkillData* pConSkillData = 0;

			if(!bHighKey)
			{
				//法决
				//if (SELF.GetYiHuoShortCutKey() == VK2KEY(cKey))
				//{
				//	//iMagicID = MAGICID_FAZHEN_CARRYON;
				//	//g_pGameControl->SEND_Player_Attack_Magic(0,0,0,iMagicID);
				//	//return true;
				//}

				if (iMagicID <= 0)
					iMagicID = SELF.MagicArray().FindByShortcut(VK2KEY(cKey));

				
				if (iMagicID <= 0)
				{
					pConSkillData = g_AIMgr.FindConSkillByKey(MAKELONG(cKey,0));
					if (pConSkillData)
						iMagicID = pConSkillData->GetCurrentMagicID();
				}
			}
			else if(bHighKey)
			{
				if(g_pInput->IsCtrl() || g_pInput->IsAlt() || g_pInput->IsShift())
					return false;

				////法决
				//if (SELF.GetYiHuoShortCutKeyEx() == MAKELONG(cKey,1))
				//{
				//	iMagicID = MAGICID_FAZHEN_CARRYON;
				//	//g_pGameControl->SEND_Player_Attack_Magic(0,0,0,iMagicID);
				//	//return true;
				//}

				if (iMagicID <= 0)
					iMagicID = SELF.MagicArray().FindByShortcutEx(MAKELONG(cKey,1));
				
				if (iMagicID <= 0)
				{
					pConSkillData = g_AIMgr.FindConSkillByKey(MAKELONG(cKey,1));
					if (pConSkillData)
						iMagicID = pConSkillData->GetCurrentMagicID();
				}
			}


			if(iMagicID > 0)
			{
				//不需吟唱等待
				if (/*iMagicID == MAGICID_FAZHEN_CARRYON || */!SELF.MagicArray().FindMagic(iMagicID)->HasAttr(MATTR_SINGING))
				{
					if(!AttackUseMagic(iMagicID,g_pControl->GetMouseOnID()))
					{
						if (pConSkillData)
						{
							g_AIMgr.SetLastPressConSkill(pConSkillData->iConSkillID);
							g_AIMgr.SetLastPressMagic(0);
						}
						else
						{
							g_AIMgr.SetLastPressConSkill(0);
							g_AIMgr.SetLastPressMagic(iMagicID);
						}
					}
					else
					{
						// 如果连击技能使用成功
						if (pConSkillData)
						{
							g_AIMgr.SetLastRepeatConSkill(pConSkillData->iConSkillID);
							g_AIMgr.SetLastRepeatMagic(0);
						}
						else
						{
							g_AIMgr.SetLastRepeatConSkill(0);
							g_AIMgr.SetLastRepeatMagic(iMagicID);
						}
					}

					if ((iMagicID == MAGICID_FIREBALL || iMagicID == MAGICID_PROTECT_SYMBOL) && !g_pGameData->HasPaoPaoStatus(EP_First_UseSkill_PaoPao))
					{
						g_pControl->PopupArrowTip(MSG_CTRL_GAMEWND,EP_First_UseSkill_PaoPao,0,0,XAL_TOPLEFT,false,XAL_TOPLEFT,1,SELF.GetID(),5000);
					}
				} 
				else if(!SELF.IsSinging() && g_AIMgr.CanUseMagic(iMagicID,true))//发送,保存,吟唱开始时间,技能id
				{
					//g_pGameData->SetSingingTimeStart(GetTickCount());
					//g_pGameData->SetSingingTechID(iMagicID);

					if(!AttackUseMagic(iMagicID,g_pControl->GetMouseOnID()))
					{
						g_AIMgr.SetLastPressConSkill(0);
						g_AIMgr.SetLastPressMagic(iMagicID);
					}
					//SELF.SetUsingMagicType(UMT_NOACTION);
				}

				return true;
			}

			break;
		}
	default:
		{
			if(!g_pInput->IsCtrl() && !g_pInput->IsAlt() && !g_pInput->IsShift() && cKey >= 'A' && cKey <= 'Z')
			{
				WORD wHighKey = g_pInput->IsEscape()?1:0;
				int iMagicID = SELF.MagicArray().FindByShortcutEx(MAKELONG(cKey,wHighKey));
				ConSkillData* pConSkillData = 0;
				if (iMagicID <= 0)
				{
					pConSkillData = g_AIMgr.FindConSkillByKey(MAKELONG(cKey,wHighKey));
					if (pConSkillData)
						iMagicID = pConSkillData->GetCurrentMagicID();
				}

				if(iMagicID > 0)
				{
					if(!AttackUseMagic(iMagicID,g_pControl->GetMouseOnID()))
					{
						if (pConSkillData)
						{
							g_AIMgr.SetLastPressConSkill(pConSkillData->iConSkillID);
							g_AIMgr.SetLastPressMagic(0);
						}
						else
						{
							g_AIMgr.SetLastPressConSkill(0);
							g_AIMgr.SetLastPressMagic(iMagicID);
						}
					}
					else
					{
						// 如果连击技能使用成功
						if (pConSkillData)
						{
							g_AIMgr.SetLastRepeatConSkill(pConSkillData->iConSkillID);
							g_AIMgr.SetLastRepeatMagic(0);
						}
						else
						{
							g_AIMgr.SetLastRepeatConSkill(0);
							g_AIMgr.SetLastRepeatMagic(iMagicID);
						}
					}
					return true;
				}
			}
		}		
	}
	return false;
}

bool CGameManager::OnKeyUp(WORD wState,UCHAR cKey)
{
	//switch(cKey)
	//{
	//case VK_F1:		
	//case VK_F2:
	//case VK_F3:
	//case VK_F4:
	//case VK_F5:
	//case VK_F6:
	//case VK_F7:
	//case VK_F8:
	//	{
	//		int iMagicID = SELF.MagicArray().FindByShortcut(VK2KEY(cKey));
	//		
	//		if(iMagicID > 0 && iMagicID == g_pGameData->GetSingingTechID())
	//		{
	//			//吟唱结束
	//			if (SELF.MagicArray().FindMagic(iMagicID)->HasAttr(MATTR_SINGING) && g_pGameData->GetSingingTimeStart() > 0)
	//			{
	//				g_pGameData->SetSingingTimeStart(0);
	//				g_pGameData->SetSingingTechID(0);
	//				int x,y;
	//				SELF.GetRealXY(x,y);
	//				g_pGameControl->SEND_Player_Attack_Magic(g_pControl->GetMouseOnID(),x,y,iMagicID);
	//				return true;
	//			} 	
	//		}
	//		break;
	//	}
	//default:
	//	break;
	//}
	return false;
}

bool CGameManager::AttackUseMagic(int iMagicID,DWORD dwTargetID,bool bAlert)
{
	//首先判断有没有学习该技能
	CMagicData* pMagic = SELF.FindMagic(iMagicID);
	if(pMagic == NULL/* && iMagicID != MAGICID_FAZHEN_CARRYON*/)
		return false;

	if (pMagic && pMagic->IsDisable())
	{
		if (bAlert)
		{
			if (iMagicID == MAGICID_SWT)
			{
				g_TalkMgr.PushSysTalk("只有身穿圣武天战甲(袍)手持怒狂降魔杵才能释放惊天动地技能");
			}
			else if (iMagicID == MAGICID_LSJ)
			{
				g_TalkMgr.PushSysTalk("只有身穿雷神绝魔袍(衣)手持幻龙风雷鞭才能释放幻影无极技能");
			}
			else if (iMagicID == MAGICID_JYL)
			{
				g_TalkMgr.PushSysTalk("只有身穿九幽灵道袍(衣)手持清心碧玉笛才能释放碧血涅磐技能");
			}
		}

		return false;
	}

	//摆摊和变身or死亡时不能用魔法
	if(SELF.GetBoothState() || SELF.IsBianShen() || SELF.IsDead())
		return false;

	if((strnicmp(g_pGameMap->GetMapName(),"wxxg",4) == 0))//五行玄关之中不能用魔法
	{
		if (bAlert)
		{
			g_TalkMgr.PushSysTalk("离火封魔阵内无法释放法术");
		}
		return false;
	}

	if(SELF.IsOnHorse() && !SELF.GetFightOnLeopard())
	{
		if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsDismount())
			g_pGameControl->SEND_Player_Ride_Horse();

		return false;
	}

	if(!g_AIMgr.CanUseMagic(iMagicID,bAlert)) //判断魔法是否可以使用了
		return false;

	//DWORD dwCount = m_dwTickCount;
	DWORD dwCount = GetTickCount();
	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	//清除动作
	SNextAction& Next = SELF.GetCNextAction();
	Next.Clear();

	switch(iMagicID)				// MagicID
	{
		// 被动技能
	case MAGICID_ATTACK_BASE:		// 基本剑术
	case MAGICID_ATTACK_SOUL:		// 精神力战法
	case MAGICID_ATTACK_KILL:		// 攻杀
	case MAGICID_SUPER_ATTACK_KILL: //强化攻杀剑法
	case MAGICID_REMAIN_SHADE:		// 残影刀法
	case MAGICID_BLOOD_SHADE:		// 血影刀法
	case MAGICID_WARRIOR_KNOW:		// 战魂真悟
	case MAGICID_TAOIST_KNOW:		// 道心清明
	case MAGICID_WIZ_KNOW:			// 法之魂
	case MAGICID_BEAST_SKILL:		// 兽灵术
	case MAGICID_SUCK_SOUL :        // 魔魂术
	case MAGICID_TRANSFER_HURT:		// 斗转星移
	case MAGICID_LIANJI:			// 连击技能
		{
			//被动技能
			return true;
		}
		break;
	case MAGICID_ATTACK_FIRE:		// 烈火
	case MAGICID_THUNDER_FIRE:		// 雷烈
	case MAGICID_ATTACK_STICK:		// 刺杀
	case MAGICID_ATTACK_MOON:		// 半月
	case MAGICID_WHOLE_MOON:		// 圆月
	case MAGICID_DESTROY_WEAPON:	// 击破
	case MAGICID_DESTROY_SHELL:     // 破甲 破击
	case MAGICID_DESTROY_SHIELD:    // 破盾
		{
			if(iMagicID == MAGICID_ATTACK_FIRE)
			{
				if(dwCount - g_AIMgr.GetReserveTime(plyLastFireTime) >= 10000)
				{
					g_pGameControl->SEND_Player_Attack_Magic(0,SELF.GetDir(),0,iMagicID);
				}
			}
			else if(iMagicID == MAGICID_THUNDER_FIRE)// 雷烈剑法必须在8秒后才可以再用
			{
				if(dwCount - g_AIMgr.GetReserveTime(plyLastFireTime) >= 8000)
				{
					g_pGameControl->SEND_Player_Attack_Magic(0,SELF.GetDir(),0,iMagicID);
				}
			}
			else if(iMagicID == MAGICID_DESTROY_WEAPON)// 击破必须在2秒后才可以再用
			{
				if((dwCount - g_AIMgr.GetReserveTime(plyLastDWTime)) >= 2000)
				{
					g_pGameControl->SEND_Player_Attack_Magic(0,SELF.GetDir(),0,iMagicID);
				}
			}
			else
			{
				g_pGameControl->SEND_Player_Attack_Magic(0,SELF.GetDir(),0,iMagicID);
			}

			return true;
		}
		break;
	case MAGICID_FIGHTER_WILD:
	case MAGICID_ENCHANTER_WILD:
	case MAGICID_TAOIST_WILD:
	case MAGICID_WILD_COLLIDE:
	case MAGICID_SUDDEN_KILL:
	case MAGICID_SHIFT_TRANSPORT:
	case MAGICID_ESCAPE_QUICK:
		{
			//填充附加数据
			Next.iData = iMagicID;
		}
		break;
	default:
		{
			if(iMagicID == MAGICID_FIREBALL 
				|| iMagicID == MAGICID_SUPER_FIREBALL
				|| iMagicID == MAGICID_ADV_FIREBALL 
				|| iMagicID == MAGICID_POISON_MAGIC 
				|| iMagicID == MAGICID_SUPER_POISON_MAGIC
				|| iMagicID == MAGICID_THUNDER 
				|| iMagicID == MAGICID_PROTECT_SYMBOL
				|| iMagicID == MAGICID_HELL_FIRE
				|| iMagicID == MAGICID_TRACE_THUNDER
				|| iMagicID == MAGICID_LURE_LIGHT
				|| iMagicID == MAGICID_BLOW_FIRE
				|| iMagicID == MAGICID_GOD_SAY
				|| iMagicID == MAGICID_SOUL_WALL
				|| iMagicID == MAGICID_ICE_ARROW
				|| iMagicID == MAGICID_ICE_DRAG
				|| iMagicID == MAGICID_CURSE
				|| iMagicID == MAGICID_SUPER_CURSE    //强化诅咒术
				|| iMagicID == MAGICID_DRAGON_LIGHT
				|| iMagicID == MAGICID_FIRE_JUJU
				|| iMagicID == MAGICID_ICE_STORM     // 冰咆哮
				|| iMagicID == MAGICID_MULTI_THUDER     // 五雷轰
				|| iMagicID == MAGICID_ICE_WHIRLWIND     // 冰旋风
				|| iMagicID == MAGICID_DESTROY_POISON     // 解毒
				|| iMagicID == MAGICID_GODLIGHT_SKILL     // 神光
				|| iMagicID == MAGICID_GRASP_DRAGON //擒龙手
				|| iMagicID == MAGICID_DULINGBO_ATTACK
				|| iMagicID == MAGICID_SHADOWKILL8//八方分影斩
				|| iMagicID == MAGICID_FIRE_SHIELD_ATTACK//强袭烈焰盾攻击
				//|| iMagicID == MAGICID_FAZHEN_CARRYON
				)
			{   
				//上述技能
				CSimpleCharacterNode * pChar = NULL;
				DWORD dwLockID = 0;

				if(dwTargetID != 0) //指定了目标
				{
					pChar = g_pGameData->FindSimpleCharacter(dwTargetID);

					//目标已经死亡时清掉
					if(NULL == pChar || pChar->IsDead() || pChar->IsNpc())
						dwTargetID = 0;

					dwLockID = dwTargetID;
				}
				else //没有指定目标的情况
				{
					dwLockID = SELF.GetReserveData(plyMagicLockID);

					if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsAbsoluteLock()) //绝对锁定
					{
						pChar = g_pGameData->FindSimpleCharacter(dwLockID);

						if(NULL == pChar || pChar->IsDead() || pChar->IsNpc())
						{
							pChar = g_pGameData->FindSimpleCharacter(g_pControl->GetMouseOnID());

							if(pChar && !pChar->IsDead() && !pChar->IsNpc())
							{
								dwLockID = g_pControl->GetMouseOnID();
							}
							else
							{
								dwLockID = 0;
							}
						}
						//其余情况不变
						dwTargetID = dwLockID;  //设定目标
					}
					else //不是绝对锁定
					{
						pChar = g_pGameData->FindSimpleCharacter(g_pControl->GetMouseOnID());
						if(pChar != NULL && !pChar->IsDead())
						{
							dwLockID = g_pControl->GetMouseOnID();
						}
						else if(g_pControl->GetMouseOnID() != SELF.GetID())// 鼠标没点中或者点中的已死了
						{
							pChar = g_pGameData->FindSimpleCharacter(dwLockID);
							if(pChar == NULL || pChar->IsDead() || pChar->IsNpc())
							{
								dwLockID = 0;
							}
						}
						dwTargetID = dwLockID; //设定目标

						//if(iMagicID != MAGICID_FAZHEN_CARRYON && !pMagic->HasAttr(MATTR_LOCK) && SELF.GetReserveData(plyMagicLockID) == 0 ) //恢复
						//	dwLockID = 0;
					}
				}

				SELF.SetReserveData(plyMagicLockID,dwLockID,iMagicID);
				//判断离目标的距离，如果太远就不攻击
				if(pChar)
				{
					int cx,cy;
					pChar->GetXY(cx,cy);
					if(abs(cx - iSelfX) <= 8 && abs(cy - iSelfY) <= 10)
						Next.uTarget = dwTargetID;
					else
						Next.uTarget = 0;
				}
				else
				{
					Next.uTarget = 0;
				}
			}
			else //其他技能
			{
				if(dwTargetID == 0) //没有指定目标的，使用鼠标上目标
					dwTargetID = g_pControl->GetMouseOnID();

				Next.uTarget = dwTargetID;
			}

			if(Next.uTarget)
			{
				if(Next.uTarget == SELF.GetID())
				{
					//目标为自身时施放的
					if(iMagicID == MAGICID_FIREBALL 
						|| iMagicID == MAGICID_ADV_FIREBALL	
						|| iMagicID == MAGICID_SUPER_FIREBALL 
						|| iMagicID == MAGICID_PROTECT_SYMBOL
						|| iMagicID == MAGICID_SHADOWKILL8
						|| iMagicID == MAGICID_FIRE_SHIELD_ATTACK
						)
						return true;
				}
				if(iMagicID == MAGICID_REPLACE_MAGIC) //替身法符不放
					return true;
			}

			if(iMagicID == MAGICID_MAGIC_PROTECT && SELF.HasStatus(CS_MAGICPROTECT))
				return true;

			if(iMagicID == MAGICID_PROTECT_SKIN  && SELF.HasStatus(CS_PROTECTSKIN))
				return true;

			if (iMagicID == MAGICID_ROTATE_FIRE && SELF.HasStatus(CS_ROTATEFIRE))
				return true;

			Next.iData = iMagicID;
			break;
		}
	}

	switch (iMagicID)
	{
	case MAGICID_AOFENGCUT_ATTACK:
	case MAGICID_ZHONGLEI_ATTACK:
	case MAGICID_SWT:
	case MAGICID_LSJ:
	case MAGICID_JYL:
		{
			CSimpleCharacterNode* pChar = NULL;
			if(dwTargetID)
				pChar = g_pGameData->FindSimpleCharacter(dwTargetID);
			int ix = 0,iy=0;
			if(pChar)
				pChar->GetXY(ix,iy);
			else
				g_pGameMgr->GetMouseTile(ix,iy);
			if(dwTargetID == 0 && iMagicID == MAGICID_AOFENGCUT_ATTACK)
				return true;

			g_pGameControl->SEND_Player_Attack_Magic(dwTargetID,ix,iy,iMagicID);
		}

		break;

	default:
		{
			Next.iLooks		= 0;
			Next.wAction	= ACTION_MAGIC;
			Next.uData1 = 0;
		}	
		break;
	}

	return true;
}

//处理自动跑路
bool CGameManager::DealAutoRun()
{
	if(!SELF.GetMapPathFinder().IsOnRoute() || SELF.IsWaitServer())
		return false;

	int iTask = SELF.GetMapPathFinder().TaskRunning(); //继续人物
	if(iTask > 0)
		return false;

	int	iX = -1,iY = -1;
	int iSelfX,iSelfY;

	SELF.GetXY(iSelfX,iSelfY);

	int iFindPathReason = g_OtherData.GetFindPathReason();
	int iLimit = (iFindPathReason == 2)?0:3;

	if(SELF.GetMapPathFinder().ReachCurrentRoutePoint(iSelfX,iSelfY,iLimit))
	{
		if(!SELF.GetMapPathFinder().AdvanceRoutePoint()) //寻找下一个Route点
		{
			if(SELF.GetMapPathFinder().ReachRouteEnd(iSelfX,iSelfY,iLimit))
			{
				SELF.GetMapPathFinder().SetOnRoute(0);

				if (iFindPathReason == 1)
				{
					int iX = 0,iY = 0;
					g_OtherData.GetFindPathClickNpcXY(iX,iY);
					CSimpleCharacterNode * pChar = MapArray.FindSimpleCharacter(iX,iY);
					if (pChar)
					{
						OnClickChar(pChar->GetID());
					}

				}
				else if(iFindPathReason == 2)
				{
					g_AutoPickMgr.AutoMoveFinish();
				}
				return false;
			}
			else
			{
				//重新找路径，如果找不到应该就结束查找了
				//mapPath.ReSubmitFindTask(iSelfX,iSelfY);
				SELF.GetMapPathFinder().ReFindPathImm(iSelfX,iSelfY);
			}
		}
	}

	SELF.GetMapPathFinder().GetCurrentRoutePoint(iX,iY);

	if(iX == -1 || iY == -1)
	{
		SELF.GetMapPathFinder().SetOnRoute(0);
		return false;
	}
	
	
	////////如果目的点上有阻挡，应该直接绕过
	if(abs(iX - iSelfX) >= 1 || abs(iY - iSelfY) >= 1)
	{
		int		step	= 1;
		BOOL	bDirect = false;		// 查看是否需要直接超方向跑

		SNextAction next;
		next.Clear();

		if(abs(iX - iSelfX) >= 2 || abs(iY - iSelfY) >= 2)
			bDirect = true;

		BOOL bPlayerCanRun = PlayerCanRun();
		if(bPlayerCanRun && bDirect)
			step = 2;
		else
			step = 1;

		next.wAction	= ACTION_RUN;
		next.iAimX		= iX;
		next.iAimY		= iY;
		if(!_closePoint(iSelfX,iSelfY,iX,iY,15))
		{
			SELF.GetMapPathFinder().ReSubmitFindTask(iSelfX,iSelfY);
			return false;
		}

		int iIntegrity = 0;
		BOOL bMove = PlayerMove(next,iSelfX,iSelfY,step,FALSE,&iIntegrity,1);

		if(!bMove || iIntegrity) //无法走到当前目的路点
		{
			if(_closePoint(iSelfX,iSelfY,iX,iY,12) && SELF.GetMapPathFinder().AdvanceRoutePoint())
			{
				//还有下一个路点,就选择下一个路点
			}
			else
			{
				SELF.GetMapPathFinder().ReSubmitFindTask(iSelfX,iSelfY);
				if(iFindPathReason == 2)
				{
					g_AutoPickMgr.AutoMoveFaild();
				}
				return false;
			}
		}
		return true;
	}
	return false;
}

bool CGameManager::AI_AutoWalk(int iX,int iY)
{
	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	if(SELF.GetMapPathFinder().ImmediatePathFindTask(iSelfX,iSelfY,iX,iY))
	{
		SELF.GetMapPathFinder().SetOnRoute(1);
		// 清除之前的动作
		SELF.GetCNextAction().Clear();
	}
	else
	{
		return false;
	}

	return true;
}

void CGameManager::AutoNextAction()
{
	if(SELF.IsWaitServer())
		return;

	SAction& ActionNow = SELF.GetAction();

	SNextAction& Next = SELF.GetCNextAction();
	UINT uID = SELF.GetReserveData(plyAttackLockID);
	CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(uID);


	if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsRunNotStop())//跑步不停
	{
		if(SELF.GetHP() < 9 || SELF.GetPackageWeight() > SELF.GetPackageWeightMax() || SELF.IsBianShen())
			GotoMousePos(FALSE,TRUE);
		else
			GotoMousePos(TRUE,TRUE);
	}
	else if(m_bLeftDown) //鼠标左键按下
	{
		if(g_pInput->IsShift() && ActionNow.wAction != ACTION_WALK && (!SELF.IsOnHorse() || SELF.GetFightOnLeopard()))//强制攻击
		{
			UINT uID = g_pControl->GetMouseOnID();
			if (uID == SELF.GetID())
			{
				uID = 0;
			}

			Next.uFlag	|= FORCE_ACTION;
			Next.wAction	= ACTION_ATTACK1;
			Next.uTarget	= uID;

			SELF.SetReserveData(plyAttackLockID,uID);

			if(uID != 0)
			{
				SELF.SetReserveData(plyMagicLockID,0);
			}
			else
			{
				Next.cDir	= GetPlayerDir(m_iMouseX,m_iMouseY);
			}

		}
		else if(g_pInput->IsAlt()) //挖尸体
		{
			Next.wAction = ACTION_CUT;
			Next.uTarget = ActionNow.uTarget;
			Next.cDir    = GetPlayerDir(m_iMouseX,m_iMouseY);
		}
		else if(g_pControl->GetMouseOnID() == 0  && !m_bCannotMove && CGoodGrid::GetDropGoodFrom().DropGood.GetID() == 0 && g_pGameData->GetDropMoneyFrom() == EDMYF_NONE)
		{
			GotoMousePos(FALSE);
		}
	}
	else if(m_dwAutoDigID != 0)
	{
		Next.wAction = ACTION_CUT;
		Next.uTarget = m_dwAutoDigID;
		Next.cDir    = GetPlayerDir(m_iMouseX,m_iMouseY);
	}
	//else if(ActionNow.wAction == ACTION_ATTACK1 && g_AIMgr.HaveChopEquip())//自动连续挖矿
	//{
	//    Next.wAction	= ACTION_ATTACK1;
	//    Next.cDir = ActionNow.cDir;
	//    Next.uTarget  = 0;
	//}
	else if(m_bRightDown)
	{
		if(!m_bCannotMove)
		{
			if(SELF.GetHP() < 9 || SELF.GetPackageWeight() > SELF.GetPackageWeightMax() || SELF.IsBianShen())
				GotoMousePos(FALSE,TRUE);
			else
				GotoMousePos(TRUE,TRUE);
		}
	}
	else
	{
		if(pChar && !pChar->IsDead()) //锁定有目标
		{
			if(pChar->IsMonster())
			{
				Next.wAction	= ACTION_ATTACK1;
				Next.uTarget	= uID;
				Next.cDir       = GetPlayerDir(m_iMouseX,m_iMouseY);
			}
			else if(pChar->IsHuman() && (g_pInput->IsShift() || (g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsRunAttack())))
			{
				ForceAttack(uID);
			}
		}
		else if(g_pInput->IsShift() && ActionNow.wAction == ACTION_ATTACK1)//强制攻击
		{
			UINT uID = g_pControl->GetMouseOnID();
			if (uID == SELF.GetID())
			{
				uID = 0;
			}

			Next.uFlag	|= FORCE_ACTION;
			Next.wAction	= ACTION_ATTACK1;
			Next.uTarget	= uID;

			SELF.SetReserveData(plyAttackLockID,uID);

			if(uID != 0)
			{
				SELF.SetReserveData(plyMagicLockID,0);
			}
			else
			{
				Next.cDir	= GetPlayerDir(m_iMouseX,m_iMouseY);
			}
		}
	}
}

void CGameManager::UpdateAlarm(bool bWaiting)
{
	//DWORD dwTemp;

	if(bWaiting)
	{
		//if(SELF.GetReserveData(10))			// 超过了10秒限制了
		//{
		//	dwTemp = 1;
		//	g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_SETALARMSTATE,1,(CControl *)dwTemp);
		//	dwTemp = 5;
		//	g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_SETALARMSTATE,2,(CControl *)dwTemp);
		//	return;
		//}
		//g_pGameControl->SEND_Test_Rungate_Answer();
		SELF.AddReserveData(plyWaitServerTimes);
	}
	else				// 正常的情况下察看服务器的相应的时间
	{
		////int iTemp = SELF.GetReserveData(11);
		////if(iTemp < 200)
		//dwTemp = 5;		// 快速
		////else
		////	dwTemp = 4;		// 良好

		//g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_SETALARMSTATE,1,(CControl *)dwTemp);
		//g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_SETALARMSTATE,2,(CControl *)dwTemp);

		g_OtherData.SetAlarmState(1,5);// 网络
		g_OtherData.SetAlarmState(2,5);// 服务器

		SELF.SetReserveData(plySendRungateNums,0);
		SELF.AddReserveData(plyNoWaitServerTimes);
	}
}

void CGameManager::SetCharBlock(CSimpleCharacter* pChar)
{
	if(pChar == NULL)
		return;

	int cx,cy;
	pChar->GetRealXY(cx,cy);
	SELF.SetOppBlock(cx,cy,TRUE);

	if(pChar->IsNpc())
	{
		switch(pChar->GetRaceNo())
		{
		case 100: //雕像，旁边的点也要阻挡掉
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 166:
		case 167:
		case 168:
		case 169:
		case 170:
		case 171:
		case 174:
			SELF.SetOppBlock(cx+1,cy,TRUE);
			break;
		case 44:
			SELF.SetOppBlock(cx,cy+1,TRUE);
			SELF.SetOppBlock(cx+1,cy,TRUE);
			SELF.SetOppBlock(cx+1,cy+1,TRUE);
			SELF.SetOppBlock(cx+1,cy+2,TRUE);
			SELF.SetOppBlock(cx+2,cy+2,TRUE);
			SELF.SetOppBlock(cx+2,cy+3,TRUE);
		case 258://炼火炉
			
			break;
		}
	}
	else if(pChar->IsMonster())
	{
		switch(pChar->GetRaceNo()) 
		{
		case 200:////////沙城的城门
			if(pChar->GetDir() != 7)
			{
				SELF.SetOppBlock(cx,cy-1,TRUE);
				SELF.SetOppBlock(cx-1,cy,TRUE);
				SELF.SetOppBlock(cx+1,cy-1,TRUE);
				SELF.SetOppBlock(cx+1,cy-2,TRUE);
			}
			break;
		}
	}
}

void CGameManager::OnRankUpLevel(int iOldLevel,int iNewLevel)
{
	if (iOldLevel >= 30) return;
	
	int iLevel = iNewLevel > 30?30:iNewLevel;

	for (int i = iOldLevel + 1; i <= iLevel;i++)
	{
		CTaskData::RankUpInfo* pInfo = g_TaskData.GetRankUpInfo(i);

		if (!pInfo) continue;
		
		vector<int>& vRankInfo = pInfo->vTask;

		for (int j = 0;j < vRankInfo.size();j++)
		{
			int iInfo = vRankInfo.at(j);
			g_pGameControl->Send_Player_Prompt_Status(iInfo,1);
		}
	}
}
