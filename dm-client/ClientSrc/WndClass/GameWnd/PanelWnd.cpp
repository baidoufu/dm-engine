#include "PanelWnd.h"
#include "Global/Global.h"
#include "GameData/GameData.h"
#include "GameData/LoginData.h"
#include "GameControl/GameControl.h"
#include "BaseClass/Control/CtrlButton.h"
#include "Baseclass/Control/ParserTip.h"
#include "GameData/TalkMgr.h"
#include "GameData/OtherData.h"
#include "GameData/TalkName.h"
#include "GameData/WooolStoreData.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/ConfigData.h"
#include "GameData/MagicDefine.h"
#include "GameData/MagicCtrlMgr.h"
#include "Global/Interface/StreamInterface.h"
#include "GameClient/VoiceAdapter.h"
#include "BaseClass/Control/CtrlMenuWnd.h"
#include "GameData/TaskData.h"
 
#include "Global/Interface/AudioInterface.h"
#include "GameClient/WidgetManager.h"

#include "GameClient/SDOAInterface.h"
#include "GameAI/AIAutoMgr.h"
#include "GameAI/AIBossCfgMgr.h"
#include "GameAI/AIGoodMgr.h"


INIT_WND_POSX(CPanelWnd,POS_VARIABLE,POS_VARIABLE)



//窗口类
CPanelWnd::CPanelWnd(void)
{
	m_iControlMode = CTRL_MODE_PANELWND;

	m_bNoMove = true;
	m_bNoChangeLevel = true;
	m_bNeedSavePage = false;
	m_bNeedSavePos  = false;
	m_bDisableEscape = true;
	m_iLastExp = 0;
	m_bScale = g_bNeedScale;

	m_sName = "PanelWnd";


	m_uStyle &= ~CTRL_STYLE_TRANS;


	//if (g_EutUiType == EUT_FASHION)
	//{
	//	m_ptLeftPoint.x = 640;
	//	m_ptLeftPoint.y = 61;
	//}
	//else
	//{
	//	m_ptLeftPoint.x = 640;
	//	m_ptLeftPoint.y = 61;
	//}

	m_pXmlWindow = g_pUiMgr->GetXmlWindow(m_sName,(g_EutUiType == EUT_CLASSIC)?0:1);

	m_pHPMPViewer = NULL;


	m_iCellWidth = 35;m_iCellHeight = 35;
	m_iCellDisW = 6;m_iCellDisH = 0;
	m_iWCells = 6;m_iHCells = 1;

	m_iSuperFrame = 0;
	m_dwOtherMsgShowTime = 0;
	m_dwCustomMsgShowTime = 0;
	m_dwOtherMsg2ShowTime = 0;
	m_dwAnotherMsgShowTime = 0;
	m_dwOtherMsgStayTime = 0;
	m_dwOtherMsg2StayTime = 0;
	m_bMsg2FirstDraw = true;
	m_iDrawTime = 0;
}

CPanelWnd::~CPanelWnd(void)
{
}

void CPanelWnd::OnCreate()
{
	g_pControl->PopupWindow(MSG_CTRL_CHATPANEL_WND,OPER_RECREATE);

	g_OtherData.InitAlarmState();

	if (g_EutUiType == EUT_CLASSIC)
	{
		g_pControl->PopupWindow(MSG_CTRL_PLAYER_PANEL_WND,OPER_CLOSE);

		//血条
		m_pHPMPViewer = new CCommonViewer();
		AddControl(m_pHPMPViewer);
		m_pHPMPViewer->Create(this,8,16,/*(g_EutUiType == EUT_CLASSIC)?*/CCommonViewer::CV_PANEL_HPMP/*:CCommonViewer::CV_PANEL_HPMP_NEW*/);
		m_pHPMPViewer->ReSize(100,100);
	}
	else
	{
		g_pControl->PopupWindow(MSG_CTRL_PLAYER_PANEL_WND,OPER_CREATE);
	}

	m_pGoodGrid = new CGoodGrid(m_iWCells,m_iHCells,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
	AddControl(m_pGoodGrid);
	m_pGoodGrid->SetGridType(GOODGRID_TYPE_BELT);
	if (g_EutUiType == EUT_CLASSIC)
	{
		m_pGoodGrid->Create(this,131,68);	
	}
	else
	{
		m_pGoodGrid->Create(this,96,64);	
	}


	CCtrlWindowX::OnCreate();

	ResetControlPos();

	//跟Xml中定义的位置相关
	if(!m_pXmlWindow)
		return;
}

void CPanelWnd::OnDraw()
{

}

void CPanelWnd::Draw()
{
	CCtrlWindowX::Draw();
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}

	//if (g_EutUiType == EUT_CLASSIC)
	//{
	//	int x = m_ptLeftPoint.x + ((g_EutUiType == EUT_CLASSIC)?m_iOriginalWidth:0);//经典界面为右上角对齐
	//	int y = m_ptLeftPoint.y;
	//	//绘制快捷消费栏
	//	DWORD dwTime = GetTickCount();
	//	for(int i = 0;i < QUICKITEM_COUNT;i++)
	//	{
	//		CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(i);
	//		// 绘制物品图标
	//		if(dwTime > qitem.dwTime)
	//		{
	//			LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_items, qitem.iItemLooks,EP_UI);
	//			if(pTex)
	//				DrawTexture(x + i*m_iCellWidth,y,pTex);
	//		}
	//	}
	//}
	

	LPTexture pWeight = NULL,pWeight1 = NULL,pExp = NULL,pExp1 = NULL;
	if (g_EutUiType == EUT_CLASSIC)
	{
		pWeight = g_pTexMgr->GetTex(PACKAGE_INTERFACE,14205,EP_UI);
		pWeight1 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,14206,EP_UI);
		pExp = g_pTexMgr->GetTex(PACKAGE_INTERFACE,11241,EP_UI);
		pExp1 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,11242,EP_UI);
	}
	//else
	//{
	//	pWeight = g_pTexMgr->GetTex(PACKAGE_INTERFACE,22062,EP_UI);
	//	pExp = g_pTexMgr->GetTex(PACKAGE_INTERFACE,22061,EP_UI);
	//}

	if(SELF.GetReserveData(plyDrinkSuperRed))
	{
		if (g_EutUiType == EUT_CLASSIC)
		{
			LPTexture pTex = NULL;

			m_iSuperFrame++;
			if(m_iSuperFrame >= 6*10)
				m_iSuperFrame = 0;

			if(SELF.GetCareer() == 0 && SELF.GetLevel() < 27)//27级以下战士
			{
				pTex = g_pTexMgr->GetTexImm(PACKAGE_magic2,5211 + m_iSuperFrame/6,EP_UI);
			}
			else
			{
				pTex = g_pTexMgr->GetTexImm(PACKAGE_magic2,5221 + m_iSuperFrame/6,EP_UI);
			}

			if(pTex)
			{
				g_pGfx->SetRenderMode(RM_ADD2);
				DrawTexture(10,-45,pTex);
				g_pGfx->SetRenderMode();
			}
		} 
		//else
		//{
		//	LPTexture pTex = NULL;
		//	pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,22200,EP_UI);
		//	g_pGfx->SetRenderMode(RM_ADD2);
		//	DrawTexture(429,-55,pTex);
		//	g_pGfx->SetRenderMode();
		//}
		
	}

	if(SELF.GetReserveData(plyDrinkSuperBlue))
	{
		if (g_EutUiType == EUT_CLASSIC)
		{
			LPTexture pTex = NULL;
			m_iSuperFrame++;

			if(m_iSuperFrame >= 6*10)
				m_iSuperFrame = 0;

			if(SELF.GetCareer() == 0 && SELF.GetLevel() < 27)//27级以下战士
				pTex = NULL;
			else
			{
				pTex = g_pTexMgr->GetTexImm(PACKAGE_magic2,5241 + m_iSuperFrame/6,EP_UI);
			}

			if(pTex)
			{
				g_pGfx->SetRenderMode(RM_ADD2);
				DrawTexture(10,-45,pTex);
				g_pGfx->SetRenderMode();
			}
		} 
		//else
		//{
		//	LPTexture pTex = NULL;
		//	pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,22201,EP_UI);
		//	g_pGfx->SetRenderMode(RM_ADD2);
		//	DrawTexture(431,-55,pTex);
		//	g_pGfx->SetRenderMode();
		//}
		
	}

	RECT rc;
	rc.left = rc.top = 0;
	//if(pWeight && SELF.GetPackageWeightMax() !=0)
	//{
	//	float fPercent = (float) SELF.GetPackageWeight()/SELF.GetPackageWeightMax();
	//	if(fPercent > 1.0)
	//		fPercent = 1.0;

	//	char temp[256] = {0};
	//	sprintf(temp,"%2d%%",(int)((float) SELF.GetPackageWeight()/SELF.GetPackageWeightMax()*100));

	//	if (g_EutUiType == EUT_CLASSIC)
	//	{
	//		if(fPercent >= 0.90 && pWeight1)
	//		{
	//			rc.right = (int)(fPercent*pWeight1->GetWidth0());
	//			rc.bottom = pWeight1->GetHeight0();
	//			TextOut(5,170,temp,0xFFFF0000);
	//			DrawTexture(29,176,pWeight1,-1,&rc);
	//		}
	//		else if(pWeight)
	//		{
	//			rc.right = (int)(fPercent*pWeight->GetWidth0());
	//			rc.bottom = pWeight->GetHeight0();
	//			TextOut(5,170,temp,0xFFFFFFFF);
	//			DrawTexture(29,176,pWeight,-1,&rc);
	//		}
	//	}
	//	//else if(pWeight)
	//	//{
	//	//	rc.right = (int)(fPercent*pWeight->GetWidth0());
	//	//	rc.bottom = pWeight->GetHeight0();
	//	//	DrawTexture(603,100,pWeight,-1,&rc);
	//	//}
	//}

	if(pExp && SELF.GetLevelUpExp() !=0)
	{
		if( m_iLastExp == 0 || m_iLastExp >= SELF.GetExp() ) 
		{
			m_iLastExp = SELF.GetExp();
		}
		else
		{
			UINT64 iExpDiff = SELF.GetExp() - m_iLastExp;
			UINT64 iOnePercent = SELF.GetLevelUpExp() / 100;
			if (iExpDiff >= iOnePercent)
				m_iLastExp += iOnePercent;
			else
				m_iLastExp += iExpDiff;
		}
		DWORD dwColor = 0xFFFFFFFF;
		if((GetTickCount() - g_OtherData.GetExpUpTime()) < 4000 && (GetTickCount() % 500) > 250)
			dwColor = 0x60FFFFFF;

		double fPercent = 0.0f;  //显示图形
		int   iPerTxt = 0;   //显示文字,一下子就显示

		if(SELF.GetLevelUpExp() != 0)
		{
			fPercent = (double) m_iLastExp / SELF.GetLevelUpExp();
			iPerTxt =(int)( (double)SELF.GetExp() * 100 / (double)SELF.GetLevelUpExp());
		}

		if(fPercent > 1.0)
			fPercent = 1.0;

		if(iPerTxt > 100)
			iPerTxt = 100;

		char temp[256] = {0};
		sprintf(temp,"%2d%%",iPerTxt);

		if (g_EutUiType == EUT_CLASSIC)
		{
			if(fPercent >= 0.90 && pExp1!= NULL)
			{
				rc.right = (int)((pExp1->GetWidth0()*fPercent));
				rc.bottom = pExp1->GetHeight0();
				//TextOut(5,161,temp,0xFFFF0000);
				DrawTexture(111,109,pExp1,-1,&rc);
			}
			else
			{
				rc.right = (int)((pExp->GetWidth0()*fPercent));
				rc.bottom = pExp->GetHeight0();
				//TextOut(5,161,temp,0xFFFFFFFF);
				DrawTexture(111,109,pExp,dwColor,&rc);
			}
		}
		//else if(pExp)
		//{
		//	rc.right = (int)((pExp->GetWidth0()*fPercent));
		//	rc.bottom = pExp->GetHeight0();
		//	DrawTexture(28,100,pExp,dwColor,&rc);
		//}
	}

	//画网络状况及本机性能
	if (g_EutUiType == EUT_CLASSIC)
	{
		ShowAlarm();
	}

	//飘红
	ShowAnotherMsg();
	ShowOtherMsg();
	ShowOtherMsg2();
	ShowCustomPiaohong();

	//画香塔和香炉
	DrawXiangLuAndTower();

	ShowShenYouEff();//画神佑剩余时间提醒
	//幻彩套装
	if(g_OtherData.GetSetMsgStartTime() != 0)
	{
		if(GetTickCount() - g_OtherData.GetSetMsgStartTime() < 5000)
		{
			int x = (g_pGfx->GetWidth() - g_OtherData.GetSetMsg().size() * 6)/2;
			g_pFont->DrawText(x,m_iScreenY,g_OtherData.GetSetMsg().c_str(),0xFFFF0000,FONT_DEFAULT,FONTSIZE_DEFAULT,0,0x88000000);
		}
		else
			g_OtherData.SetSetMsgStartTime(0);
	}


	LPTexture pNumTex = NULL;
	if (g_EutUiType == EUT_CLASSIC)
	{
		pNumTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,13890,EP_UI);
		DrawTexture(137,91,pNumTex,-1);
	}
	else
	{
		pNumTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,13890,EP_UI);
		DrawTexture(104,86,pNumTex,-1);
	}

	

	//pNumTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,(g_pGfx->GetWidth() > 800)?17587:17588);
	//if (pNumTex)
	//{
	//	if (g_pGfx->GetWidth() == 1280)
	//	{
	//		POS sizeNum(1.0f, 1.0f);
	//		if (m_bScale)
	//		{
	//			sizeNum.fx = m_fScaleX;
	//			sizeNum.fy = m_fScaleY;
	//		}
	//		sizeNum.fx *= 951.0f / 694.0f;
	//		g_pGfx->DrawTextureFX(m_iScreenX + 170,m_iScreenY + 81,pNumTex,-1,NULL,&sizeNum);
	//	}
	//	else
	//	{
	//		g_pGfx->DrawTextureFX(m_iScreenX + 170,m_iScreenY + 81,pNumTex,-1,NULL,pScale);
	//	}
	//}

	//画珍宝值信息
	//ShowZhenBao();
	//绘制秒杀槽
	//ShowSecondKill();

	//等级
	char strTemp[12] = {0};
	sprintf(strTemp,"%d",SELF.GetLevel());
	m_iFontSize = FONTSIZE_MIDDLE;
	if (g_EutUiType == EUT_CLASSIC)
	{
		TextOut(156,38,strTemp,0xFFFFFF00,DTF_Center);
	}
	//else
	//{
	//	TextOut(510,87,strTemp,0xFFFFFF00,DTF_Center);
	//}
	m_iFontSize = FONTSIZE_DEFAULT;

	//死亡后三分钟自动断开连接
	if(SELF.IsDead())
	{
		if(GetTickCount() > g_dwDeathTime + 1000 * 60 * 3) // 3 分钟
		{
			g_dwDeathTime = GetTickCount() + 1000 * 60 * 60 * 24;
			g_pNet->Close(0);
			g_pGameControl->MSG_Connect_Error(NULL,0);
		}
	}
	else
	{
		g_dwDeathTime = 0;
	}

	//心跳
	if (GetTickCount() - g_OtherData.GetLastHeartBeat() > 1000 * 60)//1 分钟
	{
		g_OtherData.SetLastHeartBeat(GetTickCount());
		g_pGameControl->SEND_Heart_Beat();
	}
	
	//绘制血量蓝量数字
	if (g_EutUiType == EUT_CLASSIC && m_pHPMPViewer)
	{
		m_pHPMPViewer->DrawHPMP();
	}
}

bool CPanelWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch(dwMsg)
	{
	case MSG_CTRL_FLASH_BTN:
		{
			//DWORD dwFlashTexID = 0;
			//string szBtnName = "";
			//if (dwData % 1000 == 1)//包裹按钮闪烁
			//{
			//	szBtnName = "PackageWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("PackageWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 2)
			//{
			//	szBtnName = "SkillWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("SkillWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 3)
			//{
			//	szBtnName = "TaskWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("TaskWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 4)
			//{
			//	szBtnName = "TroopWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("TeamWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 5)
			//{
			//	szBtnName = "CharWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("AvatarWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 6)
			//{
			//	szBtnName = "SocialityWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("RelationWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}
			//else if (dwData % 1000 == 7)
			//{
			//	szBtnName = "FireWndButton";
			//	if (dwData > 1000)//停止闪烁
			//	{
			//		dwFlashTexID = 0;
			//	}
			//	else
			//	{
			//		if (!g_pControl->FindWindowByName("SanWeiFireWnd"))
			//		{
			//			dwFlashTexID = 1;
			//		}
			//	}
			//}


			//CCtrlButton * pBtn = this->FindButtonByName(szBtnName.c_str());
			//if(pBtn)
			//{
			//	pBtn->SetFlashTexID(dwFlashTexID);
			//}

			return true;
		}
		break;
	case MSG_CTRL_LEFT_FLOW_INFO://初始化左飘数据
		{
			m_iDrawTime = 0;
			m_dwOtherMsgShowTime = GetTickCount();
			m_dwOtherMsgStayTime = 0;
		}
		break;
	case MSG_CTRL_CUSTOM_PIAOHONG://自定义飘红
		{
			m_dwCustomMsgShowTime = 0;
		}
		break;
	case MSG_CTRL_BUTTON_CLICK:
		{
			if(pControl == NULL)
				break;
			
			g_pAudio->Play(EAT_OTHER, 2, g_pAudio->GetRand()++);

			if(stricmp(pControl->GetName(),"OutButton") == 0)
			{
				if(g_pControl->FindWindowByName("ReadQuitWnd"))
					return true;
				g_MsgBoxMgr.PopSimpleComfirm("您想退出游戏回到选择角色界面吗?",MSG_CTRL_QUIT_WND,OPER_CUSTOM+1);
				return true;
			}
			else if (stricmp(pControl->GetName(),"RangeWndButton") == 0)
			{
				if(g_pControl->FindWindowByName("RangeWnd") == NULL)
					g_pGameControl->Send_Range_Sign_Up(SELF.GetID(),0);
				else
					g_pControl->Msg(MSG_CTRL_RANGE_WND,OPER_CLOSE);

				return true;
			}
			else if (stricmp(pControl->GetName(),"HelloButton") == 0)
			{
				if(SELF.IsOnLepoard() || !SELF.IsOnHorse())	//只有在豹子上或者步行状态才能作揖
				{
					SELF.GetCNextAction().wAction = ACTION_SALUTE;
				}
				return true;
			}
			else if (stricmp(pControl->GetName(),"QuitButton") == 0)
			{
				if(g_pControl->FindWindowByName("ReadQuitWnd"))
					return true;
				g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CREATE);
				//g_pControl->PopupWindow(MSG_CTRL_OFFLINETRUSTEESHIP_WND,OPER_UPDATE);				

				return true;
			}
			else if (stricmp(pControl->GetName(),"TradeWndButton") == 0)
			{
				g_pGameControl->SEND_Trade_Goin();
				return true;
			}
			else if (stricmp(pControl->GetName(),"FireWndButton") == 0)
			{
				if (g_OtherData.GetSanWeiZhenHuoInfo().byFireLevel > 0)
				{
					g_pControl->PopupWindow(MSG_CTRL_SANWEIFIRE_WND,OPER_UPDATE);
				}
				else
				{
					g_MsgBoxMgr.PopSimpleAlert("当人物修炼到28级并且完成相应任务之后才能开启三昧真火。\n如你已达到28级，请检查任务面板并完成相应主线任务。");
				}
				
				return true;
			}
			else if (stricmp(pControl->GetName(),"PMWndButton") == 0)
			{
				g_MsgBoxMgr.PopSimpleAlert("拍卖行系统稍后推出！");
				return true;


				if(g_PetBoothData.IsPaimaiClosed()) //判断是否关闭了拍卖行
					return true;

				g_WidgetMgr.LoginPaiMai();
				g_pControl->PopupWindow(MSG_CTRL_WOOOL_STORE_WND,OPER_CREATE,3);
				return true;
			}
			//else if (stricmp(pControl->GetName(),"RainbowWndButton") == 0)
			//{
			//	g_pGameControl->Send_Player_Prompt_Status(30,1);

			//	if(g_pSDOAInterface && g_pSDOAInterface->OpenWidget("sdoHelper") == SDOA_OK)
			//	{
			//		int i = 3; 
			//	}
			//	else
			//	{
			//		//调用新的接口彩虹帮助
			//		StartRainBowSpirt();
			//	}

			//	return true;
			//}
			else if(stricmp(pControl->GetName(),"BoothWndButton") == 0)
			{
// 				if (g_TrusteeshipData.IsTrusteeship() && g_TrusteeshipData.IsCaptain() && (g_TrusteeshipData.GetMicroControlPos() >= 0 || g_TrusteeshipData.GetCaptionSubstitutePos() >= 0))
// 				{
// 					g_TalkMgr.PushSysTalk("操控其他队员时不能使用此功能！");
// 					return true;
// 				}

				//g_pControl->PopupWindow(MSG_CTRL_SELECT_BOOTH_CONFIRM,OPER_CREATE);

				if (g_Login.GetLoginInExpType() == 1)
				{
					g_pControl->PopupWindow(MSG_CTRL_BINDPTWND_WND,OPER_RECREATE);
				}
				else
				{
					g_pControl->PopupWindow(MSG_CTRL_BOOTH_WND,OPER_UPDATE);

					if (g_pControl->FindWindowByName("BoothWnd"))
					{
						g_pControl->SetWindowPos((g_pGfx->GetWidth() > 800)?500:360,50,"PackageWnd");
						g_pControl->PopupWindow(MSG_CTRL_PACKAGEWND,OPER_RECREATE);
					}
				}

				return true;
			}
//			else if(stricmp(pControl->GetName(),"VoiceWndButton") == 0)
//			{
//#ifdef _CHAT
//				OnVoiceWndBtnClicked(false);
//#endif
//				return true;
//			}
			//else if(stricmp(pControl->GetName(),"SocialityWndButton") == 0)
			//{
			//	if(g_pControl->FindWindowByName("RelationWnd"))
			//	{
			//		g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_CLOSE);
			//	}
			//	else
			//	{
			//		g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@main");
			//	}

			//	return true;
			//}
			else if(stricmp(pControl->GetName(),"ChangeChannel") == 0)
			{
				OnChangeChannel(pControl);
				return true;
			}
			else if(stricmp(pControl->GetName(),"ConfigWndButton") == 0)
			{
				//微操模式下按F12弹托管设置窗口
				if (g_pControl->PopupWindow(MSG_CTRL_ASSCONFIG_WND,OPER_CLOSE) || g_pControl->PopupWindow(MSG_CTRL_SYSCONFIG_WND,OPER_CLOSE))
					return true;

				g_pControl->PopupWindow(MSG_CTRL_ASSCONFIG_WND,OPER_UPDATE);
				return true;
			}
			else if(stricmp(pControl->GetName(),"TroopWndButton") == 0)
			{
				this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_Team_Step1,NULL);
				return true;
			}
			//else if(stricmp(pControl->GetName(),"TaskWndButton") == 0)
			//{
			//	this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_FirstGetNewTask_PaoPao,NULL);
			//	return true;
			//}
			else if(stricmp(pControl->GetName(),"PackageWndButton") == 0)
			{
				this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_GetTaoMuJian_PaoPao,NULL);
				return true;
			}
			else if(stricmp(pControl->GetName(),"SkillWndButton") == 0)
			{
				this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_StudySkill_PaoPao,NULL);
				return true;
			}


			//for(int ii = 0; ii < (int)m_vTalkFilter.size();ii++)
			//{
			//	if(stricmp(pControl->GetName(),m_vTalkFilter[ii].str.c_str()) == 0)
			//	{
			//		CCtrlRadio* pRadio = dynamic_cast<CCtrlRadio*>(pControl);
			//		if(pRadio)
			//		{
			//			string strTips = pRadio->GetTips();
			//			if(pRadio->IsChecked())
			//				strTips.replace(8,4,"停止");
			//			else
			//				strTips.replace(8,4,"开始");


			//			pRadio->SetTips(strTips.c_str());

			//			g_TalkMgr.ModifyFilter(m_vTalkFilter[ii].filter,pRadio->IsChecked());
			//			//完成第一次发言任务
			//			if (ii == 2 && g_pGameData->HasPaoPaoStatus(EP_First_FaYan_Step3) && !g_pGameData->HasPaoPaoStatus(EP_First_FaYan))
			//			{
			//				this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_FaYan_Step3,NULL);
			//				g_pGameControl->Send_Player_Prompt_Status(EP_First_FaYan);//完成任务
			//			}

			//		}
			//		return true;
			//	}
			//}
            
			break;
		}
	//case MSG_CTRL_VOICE_CHAT_BEGIN:
	//	{
	//		OnVoiceWndBtnClicked(true,dwData);
	//		return true;
	//	}
	//	break;
	//case MSG_CTRL_POP_MENU:
	//	{
	//		if(dwData >= POP_MENU_ITEM_CHANNEL_COMMON && dwData <= POP_MENU_ITEM_CHANNEL_VOICE)
	//		{
	//			CCtrlButton * pBtn = this->FindButtonByName("ChangeChannel");
	//			if(pBtn)
	//			{
	//				eSendTalkChannel eSendTalkChannelType = (eSendTalkChannel)(dwData - POP_MENU_ITEM_CHANNEL_COMMON);
	//				g_pGameData->SetSendTalkChannelType(eSendTalkChannelType);
	//				pBtn->ReloadTex(9300 + eSendTalkChannelType*4,9301 + eSendTalkChannelType*4,9302 + eSendTalkChannelType*4);
	//			}

	//			if (dwData == POP_MENU_ITEM_CHANNEL_SHOUT && g_pGameData->HasPaoPaoStatus(EP_First_FaYan) && !g_pGameData->HasPaoPaoStatus(EP_First_FaYan_Step2))
	//			{
	//				this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_FaYan_Step1,NULL);
	//				if (g_EutUiType == EUT_CLASSIC)
	//				{
	//					g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_FaYan_Step2,212,171,XAL_TOPLEFT,false,XAL_BOTTOMRIGHT);
	//				} 
	//				else
	//				{
	//					g_pControl->PopupArrowTip(MSG_CTRL_TALKVIEW_WND,EP_First_FaYan_Step2,30,149,XAL_TOPLEFT,false,XAL_BOTTOMRIGHT);
	//				}
	//				
	//			}
	//		}

	//		return true;
	//	}
	//	break;

	default:
		break;

	}
	return CCtrlWindowX::Msg(dwMsg,dwData,pControl);
}

void CPanelWnd::OnChangeChannel(CControl * pControl)
{
	CCtrlMenuWnd* pMenuWnd = g_pControl->GetMenuWnd();
	pMenuWnd->Clear();

	S_POP_MENU_DATA & PopMenuData = pMenuWnd->GetData();
	PopMenuData.wBackTex = 9324;
	PopMenuData.fItemHeight = 15.3f;
	PopMenuData.iLeftOffX = 27;
	PopMenuData.iRightOffX = 4;
	PopMenuData.iTopOffY = PopMenuData.iBottomOffY = 3;
	PopMenuData.fItemHeight = 17.0f;
	PopMenuData.iFontType = FONT_YAHEI;
	PopMenuData.dwBackColor = 0xFFFFFFFF;
	PopMenuData.dwItemTextColor = 0xffb46428;
	PopMenuData.dwHoverTextColor = 0xffffe650;
	PopMenuData.dwSelectColor = 0xff1e6464;
	PopMenuData.iSelBackColorLeftOffX = 25;


	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_COMMON,"普通聊天频道");
	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_TEAM,"组队频道");
	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_SHOUT,"喊话频道");
	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_PHYLE,"宗族频道");
	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_GUILD,"行会频道");
	pMenuWnd->AddMenuItem(POP_MENU_ITEM_CHANNEL_VOICE,"语聊频道");

	//PopMenuData.iW = PopMenuData.iFontSize*12/2 + PopMenuData.iLeftOffX + PopMenuData.iRightOffX;
	//PopMenuData.iH = (int)( PopMenuData.fItemHeight*pMenuWnd->GetMenuItemVector().size()) + PopMenuData.iTopOffY + PopMenuData.iBottomOffY;
	PopMenuData.iW = 105;
	PopMenuData.iH = 104;

	PopMenuData.iX = pControl->GetScreenX()+1;
	PopMenuData.iY = pControl->GetScreenY() - PopMenuData.iH - 1;

	m_bClick = false;
	m_bRBClick = false;//否则MenuWnd获得不到焦点
	pMenuWnd->SetCaller(this);
	pMenuWnd->ShowMenu();
	g_pControl->GetTipWnd()->SetShow(false);
}

bool CPanelWnd::IsInControl(int iX,int iY)
{
	if(CCtrlWindowX::IsInControl(iX,iY))
		return true;

	//if(g_OtherData.GetCenserStartTime() > 0)
	//{
	//	if (g_EutUiType == EUT_CLASSIC && iX > 58 && iY > -200 && iX < 58 + 8 && iY < -200 + 190)
	//	{
	//		return true;
	//	}
	//}

	return false;
}

bool CPanelWnd::OnLeftButtonDClick(int iX,int iY)
{
	//int iOldX = iX,iOldY = iY;
	//if(m_bScale)
	//{
	//	iOldX = (int)(iOldX / g_ScaleRate.fx + 0.5);
	//	iOldY = (int)(iOldY / g_ScaleRate.fy + 0.5);
	//}

	return CCtrlWindowX::OnLeftButtonDClick(iX,iY);
}

bool CPanelWnd::OnLeftButtonUp(int iX,int iY)
{
	//int iOldX = iX,iOldY = iY;
	//if(m_bScale)
	//{
	//	iOldX = (int)(iOldX / g_ScaleRate.fx + 0.5);
	//	iOldY = (int)(iOldY / g_ScaleRate.fy + 0.5);
	//}

	return CCtrlWindowX::OnLeftButtonUp(iX,iY);
}
bool CPanelWnd::OnMouseMove(int iX, int iY)
{
	CParserTip *pTip = g_pControl->GetTipWnd();
	int iOldX = iX,iOldY = iY;
	if(m_bScale)
	{
		iOldX = (int)(iOldX / g_ScaleRate.fx + 0.5);
		iOldY = (int)(iOldY / g_ScaleRate.fy + 0.5);
	}

	if(IsInExp(iOldX,iOldY))
	{
		pTip->Clear();
		pTip->AddText("经验值:");
		char temp[256] = {0};
		sprintf(temp,"%I64d/%I64d",SELF.GetExp(),SELF.GetLevelUpExp());
		pTip->AddText(temp,-1,-1);

		int x = m_iScreenX + iX - 10;
		int y = m_iScreenY + iY - 35;
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);

		return true;
	}
	//else if(IsInWeight(iOldX,iOldY))
	//{
	//	pTip->Clear();
	//	char temp[256] = {0};
	//	sprintf(temp,"负重:%d/%d",SELF.GetPackageWeight(),SELF.GetPackageWeightMax());
	//	pTip->AddText(temp);

	//	int x = m_iScreenX + iX - 10;
	//	int y = m_iScreenY + iY - 20;
	//	pTip->AdjustXY(x,y);
	//	pTip->Move(x,y);
	//	pTip->SetShow(true);
	//	return true;
	//}
	else if(int iTemp = InAlarmTip(iOldX,iOldY))
	{
		char sTemp[128];
		stAlarm &alarm = g_OtherData.GetAlarmState(iTemp - 1);
		strcpy(sTemp,alarm.sAlarmName);
		strcat(sTemp,":");
		strcat(sTemp,alarm.sAlarmTip + alarm.iState * 64);
		pTip->Clear();
		pTip->AddText(sTemp);  
		pTip->SetShow(true);
		int x = m_iScreenX + iX - 10;
		int y = m_iScreenY + iY - 20;
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		return true;
	}
	//else if(IsInZhenBaoVal(iOldX,iOldY))
	//{
	//	pTip->Clear();
	//	char temp[1024] = {0};

	//	sprintf(temp,"珍宝值:%.2f%%",m_fZhenBaoValuePercent * 100);
	//	pTip->AddText(temp,0xFFFFFFFF);
	//	pTip->AddText("可通过打怪累积",0xFFFFFFFF);

	//	int x = m_iScreenX + iX + 10;
	//	int y = m_iScreenY + iY + 10;
	//	pTip->AdjustXY(x,y);
	//	pTip->Move(x,y);
	//	pTip->SetShow(true);
	//	return true;
	//}
	//else if(IsInZhenBaoBall(iOldX,iOldY))
	//{
	//	pTip->Clear();

	//	pTip->AddText("20级以后消灭任意怪物都可以获得珍宝值。",0xFF00FF00);
	//	pTip->AddText("每当珍宝值累积满一条时都会点亮一颗珍宝珠，并获得一次怪物大爆的机会以及一定的荣耀点。",0xFF5AA0A0);
	//	pTip->AddText("四颗珍宝珠全部点亮以后，会获得10分钟的双倍爆率时间。",0xFFFF0000);

	//	int x = m_iScreenX + iX + 10;
	//	int y = m_iScreenY + iY + 10;
	//	pTip->AdjustXY(x,y);
	//	pTip->Move(x,y);
	//	pTip->SetShow(true);
	//	return true;
	//}	
	//else if (IsInStoreGrid(iOldX,iOldY,index))
	//{
	//	//快速消费提示信息
	//	CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(index);
	//	pTip->Clear();
	//	if(qitem.strName.size() >0)
	//	{
	//		pTip->AddText(qitem.strName.c_str(),0xFFFFFF00);
	//		pTip->AddText("双击购买使用");
	//		int x = m_iScreenX + iX + 10;
	//		int y = m_iScreenY + iY + 10;
	//		pTip->AdjustXY(x,y);
	//		pTip->Move(x,y);
	//		pTip->SetShow(true);
	//	}
	//	else
	//	{
	//		pTip->SetShow(false);
	//	}
	//	return true;
	//}
	//else if(g_EutUiType == EUT_CLASSIC && g_OtherData.GetCenserDurTime() > 0 && iX > 58 && iY > -200 && iX < 58 + 8 && iY < -200 + 190)
	//{
	//	pTip->Clear();

	//	int iLeft = (g_OtherData.GetCenserDurTime() - (GetTickCount() - g_OtherData.GetCenserStartTime())) / 1000 / 60 + 1;

	//	char str[256] = "";
	//	sprintf(str,"剩余时间： %d分钟",iLeft);
	//	pTip->AddText(str);		

	//	int x = m_iScreenX + iX + 10;
	//	int y = m_iScreenY + iY + 10;		
	//	pTip->AdjustXY(x,y);
	//	pTip->Move(x,y);
	//	pTip->SetShow(true);

	//	return true;
	//}

	return false;
}

bool CPanelWnd::IsInExp(int ix,int iy)
{
	if (g_EutUiType == EUT_CLASSIC)
	{
		if(ix >= 111 && iy >= 108 && ix <= 111 + 562 && iy <= 109 + 7)
			return true;
	}
	//else
	//{
	//	if(ix >= 25 && iy >= 98 && ix <= 420 && iy <= 104)
	//		return true;
	//}

	return false;
}

//bool CPanelWnd::IsInWeight(int ix,int iy)
//{
//	if (g_EutUiType == EUT_CLASSIC)
//	{
//		if(ix >= 4 && iy >= 172 && ix <= 4 + 122 && iy <= 172 + 8)
//			return true;
//	}
//	//else
//	//{
//	//	if(ix >= 603 && iy >= 98 && ix <= 997 && iy <= 104)
//	//		return true;
//	//}
//
//	return false;
//}

//bool CPanelWnd::IsInZhenBaoVal(int ix, int iy)
//{
//	if (g_EutUiType == EUT_CLASSIC)
//	{
//		if (iy >= 83 && iy <= 90 && ix >= 173 && ix < m_iOriginalWidth - 234)
//		{
//			return true;
//		}	
//	}
//	else
//	{
//		int x = abs(ix - 508);
//		int y = abs(iy - 52);
//		int iDis = x*x + y*y;
//
//		if(iDis >= 32*32 && iDis <= 40 * 40)
//		{
//			return true;
//		}		
//	}
//
//	return false;
//}

//bool CPanelWnd::IsInZhenBaoBall(int ix, int iy)
//{
//	if (g_EutUiType == EUT_CLASSIC)
//	{
//		if (iy >= 77 && iy <= 90 && ix >= m_iOriginalWidth - 228 && ix < m_iOriginalWidth - 240 + 80)
//		{
//			return true;
//		}	
//	}
//	else
//	{
//		for(int i = 0; i < 4; i++)
//		{
//			if(ix >= m_ZhenBaoPos[i].x && iy >= m_ZhenBaoPos[i].y && ix < m_ZhenBaoPos[i].x + 20 && iy < m_ZhenBaoPos[i].y + 20)
//				return true;
//		}
//	}
//
//	return false;
//}

////显示珍宝值
//void CPanelWnd::ShowZhenBao()
//{
//	DWORD dwZhenBaoVal = SELF.GetZhenBaoValue();
//	if(dwZhenBaoVal > 0)
//	{
//		int iLevel = 0;
//
//		float fPower = 0.0f;
//		if(SELF.GetFullZhenBaoTime() == 0)//没有满显示现在的值
//		{
//			for (int i = 0; i < MAX_ZHENBAO_LEVEL; i++)
//			{
//				if (dwZhenBaoVal >= SELF.GetZhenBaoLevelMaxExp(i))
//				{
//					iLevel ++;
//				}
//				else
//				{
//					break;
//				}
//			}	
//
//			DWORD dwPreLevelVal = 0;
//			if(iLevel > 0)
//			{
//				if (iLevel > MAX_ZHENBAO_LEVEL - 1)
//				{
//					dwPreLevelVal = SELF.GetZhenBaoLevelMaxExp(iLevel - 2);
//				}
//				else
//				{
//					dwPreLevelVal = SELF.GetZhenBaoLevelMaxExp(iLevel - 1);
//				}
//			}
//			DWORD dwZhenBaoValMax = 1;//当前档最大珍宝值
//			if (iLevel > MAX_ZHENBAO_LEVEL - 1)
//			{
//				dwZhenBaoValMax = SELF.GetZhenBaoLevelMaxExp(iLevel - 1) - dwPreLevelVal;
//			}
//			else
//			{
//				dwZhenBaoValMax = SELF.GetZhenBaoLevelMaxExp(iLevel) - dwPreLevelVal;
//			}
//			if(dwZhenBaoValMax <= 0)
//				fPower = 0.0f;
//			else
//				fPower = (float)(dwZhenBaoVal - dwPreLevelVal) / dwZhenBaoValMax;
//		}
//		else//已经满了显示满的状态还剩多少时间结束
//		{
//			static DWORD sdwLastTime[MAX_ZHENBAO_LEVEL] = {60000,120000,180000,240000};//满了以后每档持续时间
//			DWORD dwLeftTime = 0;
//			if(SELF.GetFullZhenBaoTime() > GetTickCount())
//				dwLeftTime = SELF.GetFullZhenBaoTime() - GetTickCount();
//
//			iLevel = -1;
//			for (int i = 0; i < MAX_ZHENBAO_LEVEL - 1; i++)
//			{
//				if (dwLeftTime > sdwLastTime[i])
//				{
//					iLevel = i;
//					dwLeftTime -= sdwLastTime[i];
//				}
//				else
//				{
//					break;
//				}
//			}
//
//			iLevel += 1;
//			fPower = (float)(dwLeftTime) / sdwLastTime[iLevel];
//
//			if (dwLeftTime == 0)
//			{
//				SELF.SetFullZhenBaoTime(0);
//			}
//		}
//
//		if(fPower > 1.0f)
//			fPower = 1.0f;
//
//		m_fZhenBaoValuePercent = fPower;
//		int iPowerWidth = 395;
//		if (g_pGfx->GetWidth() == 800 || g_bNeedScale)
//		{
//			iPowerWidth = 393;
//		}
//		else if (g_pGfx->GetWidth() == 1024)
//		{
//			iPowerWidth = 619;
//		}
//		else if (g_pGfx->GetWidth() == 1280)
//		{
//			iPowerWidth = 876;
//		}
//
//		RECT rc;
//		rc.left = rc.top = 0;
//		rc.bottom = -1;
//		rc.right = iPowerWidth;
//
//		RECT rc2;
//		rc2.left = rc2.top = 0;
//		rc2.bottom = -1;
//		rc2.right = (int)(iPowerWidth * fPower);
//
//		if (g_EutUiType == EUT_CLASSIC)
//		{
//			for (int i = 0;i < MAX_ZHENBAO_LEVEL; i++)
//			{
//				if(i < iLevel)
//					DrawTexture(173,84,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14724 + i,EP_UI),-1,&rc);
//
//				if (i < iLevel || SELF.GetFullZhenBaoTime() != 0)
//					DrawTexture(m_iOriginalWidth - 230 + i * 18,75,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14728,EP_UI));
//			}	
//
//			DrawTexture(173,84,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14724 + ((iLevel < MAX_ZHENBAO_LEVEL)?iLevel:(MAX_ZHENBAO_LEVEL - 1)) ,EP_UI),-1,&rc2);
//		}
//		else
//		{
//			for (int i = 0;i < MAX_ZHENBAO_LEVEL; i++)
//			{
//				if(i < iLevel)
//					DrawTexture(383,- 75,g_pTexMgr->GetTex(PACKAGE_INTERFACE,22065 + i,EP_UI));
//
//				if (i < iLevel || SELF.GetFullZhenBaoTime() != 0)
//					DrawTexture(m_ZhenBaoPos[i].x,m_ZhenBaoPos[i].y,g_pTexMgr->GetTex(PACKAGE_INTERFACE,22057,EP_UI));
//			}
//
//			g_pGfx->DrawArcTexture(m_iScreenX + 511,m_iScreenY + 53,g_pTexMgr->GetTex(PACKAGE_INTERFACE,22065 + ((iLevel < MAX_ZHENBAO_LEVEL)?iLevel:(MAX_ZHENBAO_LEVEL - 1)) ,EP_UI),0,fPower*360.0f);			
//		}
//
//		//满了有特效
//		if(SELF.GetFullZhenBaoTime() != 0)
//		{
//			g_pGfx->SetRenderMode(RM_ADD2);
//			if (g_EutUiType == EUT_CLASSIC)
//			{
//				if (g_pGfx->GetWidth() == 800 || g_bNeedScale)
//				{
//					DrawTexture(165,81,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14730,EP_UI));
//				}
//				else if (g_pGfx->GetWidth() == 1024)
//				{
//					DrawTexture(105,58,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14731,EP_UI));
//				}
//				else if (g_pGfx->GetWidth() == 1280)
//				{
//					DrawTexture(79,58,g_pTexMgr->GetTex(PACKAGE_INTERFACE,14732,EP_UI));
//				}
//
//			}
//			else
//			{
//				DrawTexture(391,-30,g_pTexMgr->GetTex(PACKAGE_INTERFACE,22058,EP_UI));
//			}
//			g_pGfx->SetRenderMode();
//		}
//	}
//	else
//	{
//		m_fZhenBaoValuePercent = 0.0f;
//	}
//}


int CPanelWnd::InAlarmTip(int iX, int iY)
{
	if (g_EutUiType == EUT_CLASSIC)
	{
	    //iX -= m_iOriginalWidth;//右上角对齐
	}
	else 
	{
		return 0;
	}

	int i = 0;
	for(; i < 3; i++)
	{
		stAlarm &alarm = g_OtherData.GetAlarmState(i);
		if(iX >= alarm.x && iY >= alarm.y && iX < alarm.x + 6 && iY < alarm.y + 6)
			break;
	}
	if(i == 3)
		return 0;
	else
		return i + 1;
}

void CPanelWnd::ShowAlarm(void)
{
	g_OtherData.UpdateLocalState();

	if (g_EutUiType == EUT_CLASSIC)
	{
		LPTexture pTex2 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,711,EP_UI);
		if(pTex2)
		{
			for(int i = 0; i < 3; i++)
			{
				stAlarm &alarm = g_OtherData.GetAlarmState(i);
				DWORD dwColor = g_OtherData.GetAlarmColor(alarm.iState);
				DrawTexture(alarm.x,alarm.y,pTex2,dwColor);
			}
		}
	}
	//else
	//{
	//	for(int i = 0; i < 3; i++)
	//	{
	//		DrawTexture(m_Alarm[i].x,m_Alarm[i].y,g_pTexMgr->GetTex(PACKAGE_INTERFACE,711,EP_UI),m_dwAlarmColor[m_Alarm[i].dwState]);
	//	}
	//}
}

void CPanelWnd::DrawXiangLuAndTower()
{
	int iCenserOffY = g_pGfx->GetHeight() - 10;

	//绘制香炉
	DWORD dwCenserStartTime = g_OtherData.GetCenserStartTime();
	if(dwCenserStartTime > 0)//有香
	{
		LPTexture pCenserTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,74,EP_UI);
		LPTexture pIncenserTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,75,EP_UI);//因为后面要用到这张图的高度,所以用GetTexImm
		DWORD dwCurTime = GetTickCount();
		DWORD dwDurTime = g_OtherData.GetCenserDurTime();

		//if (g_EutUiType == EUT_CLASSIC)
		//{
		//	g_pGfx->DrawTextureNL(iCenserOffX + 28,iCenserOffY-18,pCenserTex);//炉灶
		//}

		if(dwDurTime - (dwCurTime - dwCenserStartTime) > 1000)//香芯
		{
			int px1,py1;
			DWORD dwColor = 0xFFFFFFFF;
			float fScale = (dwCurTime - dwCenserStartTime)/float(dwDurTime);
			if(fScale < 1.0f)
			{
				if(pIncenserTex)
				{
					px1 = 5 - pIncenserTex->GetOffX();
					py1 = iCenserOffY - 183 - pIncenserTex->GetOffY();
					g_pGfx->DrawPartTexture(px1,py1,pIncenserTex,0,(int)(fScale*pIncenserTex->GetHeight()));					
				}
			}
		}
		else
		{
			g_OtherData.SetCenserStartTime(0);
			g_OtherData.SetCenserDurTime(0);
			g_OtherData.SetCenserStart(false);
		}

		if(g_OtherData.IsCenserStart())//开始烧香效果
		{
			if(!g_pMagicCtrl->FindMagicByID(MAGICID_SENCER_EFFECT))
			{
				Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_SENCER_EFFECT,EMP_MAGIC_NOTARGET|EMP_MAGIC_NOOWNER,0,0,-1,0xFFFFFFFF,9 - 50 - TILE_WIDTH/2,0);
				if(ms)
				{
					if(pIncenserTex)
						ms->iData3 =  pIncenserTex->GetHeight0();//总共可以下降的高度
					else
						ms->iData3 = 0;

					ms->iData1 = 9 - 50;//x位置
					ms->iData2 = iCenserOffY - 390;//y位置
					ms->ref->wOriginTileX = ms->ref->wOriginTileY = ms->ref->wTargetTileX = ms->ref->wTargetTileY = 0;
				}
				g_OtherData.SetCenserStart(false);
			}
		}
	}
}


void CPanelWnd::ShowAnotherMsg()//右侧到左侧飘红
{
	if (g_OtherData.GetAnotherMsg().empty())
		return;

	int iBackColor,iX = g_pGfx->GetWidth();
	_OtherMsg TempStruct;
	TempStruct = g_OtherData.GetAnotherMsg().front();

	BYTE cFontColor = LOBYTE(TempStruct.wColor);
	if(cFontColor == 0) 
		cFontColor = 0xF9;

	BYTE cBackColor = HIBYTE(TempStruct.wColor);
	if(cBackColor > 0)
		iBackColor = (g_pGameData->GetMirColor(cBackColor) & 0x00FFFFFF) | 0x88000000;
	else
	{
		iBackColor = g_pStreamMgr->GetConfigInt("TipColor");
		if(iBackColor == 0)
			iBackColor = 0x88000000;
	}

	if(m_dwAnotherMsgShowTime == 0)
	{
		m_dwAnotherMsgShowTime = GetTickCount();
	}
	else
	{
		DWORD dwCurTime = GetTickCount();
		iX = g_pGfx->GetWidth() - ((dwCurTime - m_dwAnotherMsgShowTime) * 3 / 150);
		if(iX + (int)(TempStruct.strMsg.size() * 6) < 0)
		{
			g_OtherData.GetAnotherMsg().erase(g_OtherData.GetAnotherMsg().begin());
			m_dwAnotherMsgShowTime = 0;
		}
	}
	g_pGfx->DrawFillRect(iX,20,TempStruct.strMsg.size() * FONTSIZE_DEFAULT / 2,FONTSIZE_DEFAULT,iBackColor);
	g_pFont->DrawText(iX,20,TempStruct.strMsg.c_str(),g_pGameData->GetMirColor(cFontColor));

}
void CPanelWnd::ShowOtherMsg()//右侧到左侧飘红
{
	if ( g_OtherData.GetOtherMsg().empty())
		return;

	_OtherMsg TempStruct;
	TempStruct = g_OtherData.GetOtherMsg().front();

	VString vecStr;
	CutByUnicode(TempStruct.strMsg.c_str(),vecStr,31);
	int iBackColor;
	int iX = -120;
	int iY = (g_pGfx->GetHeight() - vecStr.size() * FONTSIZE_DEFAULT)/2;

	BYTE cFontColor = LOBYTE(TempStruct.wColor);
	if(cFontColor == 0)
		cFontColor = 0xF9;

	BYTE cBackColor = HIBYTE(TempStruct.wColor);
	if(cBackColor > 0 )
		iBackColor = (g_pGameData->GetMirColor(cBackColor) & 0x00FFFFFF) | 0x88000000;
	else
	{
		iBackColor = g_pStreamMgr->GetConfigInt("TipColor");
		if(iBackColor == 0)
			iBackColor = 0x88000000;
	}

	//static int iDrawTime = 0;
	int step = 3;

	if(m_dwOtherMsgShowTime == 0)
	{
		m_dwOtherMsgShowTime = GetTickCount();
	}
	else
	{
		int midx = g_pGfx->GetWidth() / 2;
		if((abs(iX + m_iDrawTime * step - (midx - 32 * FONTSIZE_DEFAULT/4)) < step) && m_dwOtherMsgStayTime == 0)
		{
			m_dwOtherMsgStayTime = GetTickCount();
		}
		else
		{
			if (m_dwOtherMsgStayTime == 0 || GetTickCount() - m_dwOtherMsgStayTime > 10000)
			{
				m_iDrawTime++;
			}

			if(iX + m_iDrawTime*step> g_pGfx->GetWidth() )
			{
				g_OtherData.GetOtherMsg().erase(g_OtherData.GetOtherMsg().begin());
				m_dwAnotherMsgShowTime = 0;
				m_iDrawTime = 0;
				m_dwOtherMsgStayTime = 0;
			}
		}
	}

	g_pGfx->DrawFillRect(iX + m_iDrawTime*step,iY,32 * FONTSIZE_DEFAULT / 2,vecStr.size() * FONTSIZE_DEFAULT,iBackColor);
	for (int i = 0;i< vecStr.size();i++)
	{	
		g_pFont->DrawText(iX + m_iDrawTime*step,iY + i*FONTSIZE_DEFAULT,vecStr[i].c_str(),g_pGameData->GetMirColor(cFontColor));	
	}
}
void CPanelWnd::ShowOtherMsg2()//飘红,右飘
{
	if (g_OtherData.GetOtherMsg2().empty())
		return;

	//BYTE cFontColor =  251;
	int step = -2;
	_OtherMsg TempStruct;
	TempStruct = g_OtherData.GetOtherMsg2().front();

	VString vecStr;
	CutByUnicode(TempStruct.strMsg.c_str(),vecStr,31);

	BYTE cBackColor = HIBYTE(TempStruct.wColor);
	int iX = g_pGfx->GetWidth();
	int iY = m_iScreenY - 50;

	static int iDrawTime = 0;

	if(m_dwOtherMsg2ShowTime == 0)
	{  
		m_dwOtherMsg2ShowTime = GetTickCount();
	}
	else
	{
		LPTexture pPublicNews1 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,10451,EP_UI);
		if(pPublicNews1)
		{
			g_pGfx->DrawTextureNL(iX - 12 + iDrawTime * step,iY - 28,pPublicNews1);
		}

		for (int i = 0;i< vecStr.size();i++)
		{	
			g_pFont->DrawText(iX + iDrawTime * step + 12,iY + i*FONTSIZE_DEFAULT,vecStr[i].c_str(),/*g_pGameData->GetMirColor(cFontColor)*/0xffffff00,FONT_DEFAULT,FONTSIZE_DEFAULT,DTF_BlackFrame);	
		}

		if(abs(iDrawTime * step) >= 224 && m_dwOtherMsg2StayTime == 0)
		{
			m_dwOtherMsg2StayTime = GetTickCount();
			m_bMsg2FirstDraw = false;
		}
		if(m_bMsg2FirstDraw)
		{
			LPTexture pPublicNews3 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,10453,EP_UI);
			if(pPublicNews3)
			{
				g_pGfx->DrawTextureNL(g_pGfx->GetWidth() - 10,iY - 28,pPublicNews3);
			}
			iDrawTime++;
		}
		else
		{
			if(GetTickCount() - m_dwOtherMsg2StayTime > 3000 && m_dwOtherMsg2StayTime != 0)
			{
				LPTexture pPublicNews3 = g_pTexMgr->GetTex(PACKAGE_INTERFACE,10453,EP_UI);
				if(pPublicNews3)
				{
					g_pGfx->DrawTextureNL(g_pGfx->GetWidth() - 10,iY - 28,pPublicNews3);
				}

				iDrawTime--;

				if(iDrawTime <= 0)
				{
					g_OtherData.GetOtherMsg2().erase(g_OtherData.GetOtherMsg2().begin());
					iDrawTime = 0;
					m_bMsg2FirstDraw = true;
					m_dwOtherMsg2ShowTime = 0;
					m_dwOtherMsg2StayTime = 0;
				}
			}			
		}
	}
}

void CPanelWnd::ShowCustomPiaohong()//自定义飘红
{
	VCustomPiaohongMsg& vecCustomMessage = g_OtherData.GetCustomPiaohong();

	if (vecCustomMessage.empty())
		return;

	CustomPiaohongMsg CustomMessage = vecCustomMessage.front();
	BYTE byFontSpacing = CustomMessage.byFontSize/2;	//字间距,字体大小的一半

	
	
	BYTE cFontColor =  CustomMessage.byFontColor;
	if(cFontColor == 0)
		cFontColor = 0xF9;
	
	VString vecStr;
	CutByUnicode(CustomMessage.Message.c_str(),vecStr,128,true,'\\');
	
	BYTE byFontSize = CustomMessage.byFontSize;
	int iX = g_pGfx->GetWidth()/2 +28;
	int iY = g_pGfx->GetHeight() / 4 - vecStr.size() / 2 * byFontSize - (vecStr.size() - 1) / 2 * byFontSpacing;

	BYTE cBackColor = CustomMessage.byBkColor;	//描边颜色

	if(m_dwCustomMsgShowTime == 0)
	{
		m_dwCustomMsgShowTime = GetTickCount();
	}
	else
	{
		if (GetTickCount() - m_dwCustomMsgShowTime > 10000)	//持续10秒
		{
			g_OtherData.GetCustomPiaohong().erase(g_OtherData.GetCustomPiaohong().begin());
			m_dwCustomMsgShowTime = 0;
			return;
		}

	}

	BYTE flag = 0;
	flag |= CustomMessage.bNeedbk?DTF_BlackFrame:~DTF_BlackFrame;
	if (CustomMessage.byFontColor == CustomMessage.byBkColor)
	{
		flag = 0;
	}

	for (int i = 0;i< vecStr.size();i++)
	{	
		g_pFont->DrawText(iX ,iY + i * byFontSize,vecStr[i].c_str(),g_pGameData->GetMirColor(cFontColor),CustomMessage.byFontType,byFontSize,flag|DTF_Center,0,g_pGameData->GetMirColor(cBackColor));	
	}

}

void CPanelWnd::ShowShenYouEff()
{
	DWORD iLastTime = SELF.GetReserveData(plyShenYouLastTime);
	if(iLastTime == 0)
		return;

	DWORD iStartTime = SELF.GetReserveData(plyShenYouStartTime);
	DWORD dwTickCount = GetTickCount();

	if(dwTickCount - iStartTime > iLastTime || iLastTime- (dwTickCount - iStartTime) > 300000)
		return;
	LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_magic1,11303,EP_UI);
	LPTexture pTex1 = NULL;
	if (iLastTime- (dwTickCount - iStartTime) <= 300000 && iLastTime- (dwTickCount - iStartTime) >= 180000)
		pTex1 = g_pTexMgr->GetTex(PACKAGE_magic1,11302,EP_UI);
	else if (iLastTime- (dwTickCount - iStartTime) < 180000 && iLastTime- (dwTickCount - iStartTime) >= 60000)
		pTex1 = g_pTexMgr->GetTex(PACKAGE_magic1,11301,EP_UI);
	else
		pTex1 = g_pTexMgr->GetTex(PACKAGE_magic1,11300,EP_UI);

	g_pGfx->DrawTextureNL(g_pGfx->GetWidth()/2 - 127,30,pTex);
	g_pGfx->DrawTextureNL(g_pGfx->GetWidth()/2 - 20,25,pTex1);

	return;
}


//void CPanelWnd::ShowSecondKill()
//{
//	static int iIndex = 0;
//	bool b = g_OtherData.GetSecondKillStatus();	
//	if(!b) return;
//
//	//绘画一次
//	int hz = max(g_Config.GetSmooth(),1);
//	if(iIndex > 26 * 6 * hz)
//	{
//		g_OtherData.SetSecondKillStatus(false);//不再显示
//		iIndex = 0;
//		return;
//	}
//
//	LPTexture pTex = NULL;
//	WORD wTexID = 14870;
//	int iY = m_iScreenY - 38;
//	if(g_pGfx->GetWidth() != 800 && !g_bNeedScale)
//	{
//		wTexID = 14850;
//		iY = m_iScreenY;
//	}
//
//	pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,wTexID + (iIndex / 3 / hz) % 13,EP_UI);
//	if(pTex)
//	{
//		g_pGfx->SetRenderMode(RM_ADD2);
//		g_pGfx->DrawTextureNL(m_iScreenX,iY,pTex);
//		g_pGfx->SetRenderMode();
//	}
//	iIndex++;
//}


//void CPanelWnd::OnVoiceWndBtnClicked(bool bFromIGW,DWORD dwData)
//{
//#ifdef _CHAT
//
//	if (bFromIGW && dwData == HOSTAPP_CLOSE)
//	{
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_CHAT_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_SEARCH_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_PASSWORD_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_ADD_WND,OPER_CLOSE);
//		return;
//	}
//
//	if (g_VoiceAdapter.IsLoginOutAfterLeaveRoom())
//	{
//		if(bFromIGW)
//			PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_VOICE_CHAT,HOSTAPP_CLOSE);
//
//		return; 
//	}
//
//	if(GetTickCount() - g_OtherData.GetEnterGameTime() < 3000)
//	{
//		g_TalkMgr.PushSysTalk("语聊系统正在初始化，请稍后再试！");
//		if(bFromIGW)
//			PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_VOICE_CHAT,HOSTAPP_CLOSE);
//
//		return;
//	}
//
//	if(!bFromIGW)
//		PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_VOICE_CHAT,HOSTAPP_OPEN);
//
//	if(g_VoiceAdapter.IsInRoom())
//	{
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_CHAT_WND,OPER_CREATE);
//	}
//	else
//	{
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_CHAT_WND,OPER_CLOSE);
//		g_pControl->PopupWindow(MSG_CTRL_VOICE_WND,OPER_CREATE);
//	}
//
//#endif
//}
//

void CPanelWnd::ResetControlPos()
{
	CCtrlWindowX::ResetControlPos();
	m_bScale = g_bNeedScale;
	g_AIAutoMgr.SetDrawInfoXY(40,g_pGfx->GetHeight() - 360);
}
