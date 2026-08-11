#include "PackageWnd.h"
#include "Global/Global.h"
#include "GameControl/GameControl.h"
#include "BaseClass/Control/UiManager.h"
#include "GameData/MsgBoxMgr.h"
#include "BaseClass/Control/ParserTip.h"
#include "GameData/GameData.h"
#include "GameData/WooolStoreData.h"
#include "GameData/ConfigData.h"
#include "GameClient/WidgetManager.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/GuildData.h"
#include "GameData/OtherData.h"
#include "GameAI/AIGoodMgr.h"
#include <Shellapi.h>



INIT_WND_POSX(CPackageWnd,POS_AUTO,POS_AUTO)

CPackageWnd::CPackageWnd(void)
{
//	m_sName = "PackageWnd";
// 	m_iPackageType = 1;//都用大包裹了
// 	m_pXmlWindow = g_pUiMgr->GetXmlWindow(m_sName,m_iPackageType);


	m_iCellWidth = 39;
	m_iCellHeight = 39;
	m_iCellDisW = 0;
	m_iCellDisH = 0;
	
	m_iWCells = 10;
	m_iHCells = 6;

// 	//如果自己是群英阵谱的队长并且不在微操状态
// 	if (g_TrusteeshipData.IsTrusteeship() && g_TrusteeshipData.IsCaptain() && g_TrusteeshipData.GetMicroControlPos() < 0)
// 	{
// 		m_iIndex = 706;		
// 	}
// 	else
	{
		m_iIndex = 707;
	}
	

	int m_iMouseOn =  -1;
	//快捷消费
	m_ptGoodPoint.x = 25;
	m_ptGoodPoint.y = 95;
	m_pLingXiBtn = NULL;
	m_pCreditBtn = NULL;

	m_iVersion = 2;
	AddTabPage(0,0,MAKELONG(707,PACKAGE_INTERFACE),471,64,180,181,182,183,"包裹一",NULL,true,4,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(707,PACKAGE_INTERFACE),471,64+50,180,181,182,183,"包裹二",NULL,true,4,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(707,PACKAGE_INTERFACE),471,64+100,180,181,182,183,"包裹三",NULL,true,4,0,0,FONT_YAHEI,FONTSIZE_SMALL);

	m_TabPage.iCurPage = 0;

}

CPackageWnd::~CPackageWnd(void)
{
	g_WooolStoreMgr.GetDropQuickItem().clear();

	//清空闪烁特效
	if (SELF.GetLevel() < 30)
	{
		for (int iPos = 0; iPos < MAX_PACKAGE_NUM; iPos++)
		{
			SELF.GetPackageGood(iPos).SetFlashTexID(0);
			SELF.GetPackageGood(iPos).SetArrowTipWnd(0);
		}
	}

}

void CPackageWnd::OnCreate()
{
	CCtrlWindowX::OnCreate();

	g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1001);//停止按钮闪烁
	g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_REMOVE_ARROWTIP_CONTROL,EP_First_GetTaoMuJian_PaoPao1);


	//冒泡,微操或附身时禁用
	//if (m_iIndex != 706)
	{
		BOOL bHaveArrowTip = FALSE;
		
		PopupArrowTip_Good(EP_First_GetTaoMuJian_PaoPao,"新手乌木剑");

		PopupArrowTip_BaoJian();

		PopupArrowTip_Good(EP_First_GetLianYuTiYuanKa_PaoPao,"炼狱体验卡");
		if (SELF.GetCareer() == 0)
		{
			PopupArrowTip_Good(EP_First_GetAttackSkillBook_PaoPao,"初级剑法");
		}
		else if (SELF.GetCareer() == 1)
		{
			PopupArrowTip_Good(EP_First_GetAttackSkillBook_PaoPao,"小火球");
		}
		else if (SELF.GetCareer() == 1)
		{
			PopupArrowTip_Good(EP_First_GetAttackSkillBook_PaoPao,"治疗术");
		}

		if (SELF.GetLevel() >= 30 && !g_pGameData->HasPaoPaoStatus(EP_First_Level_30))
		{
			AddArrowTip(EP_First_Level_30,117,60,XAL_TOPLEFT,false,XAL_TOPLEFT);
		}

	}
	//end 冒泡

	SetCloseButton(495,4,80);

	int iOffY = 0;
// 	if (m_iIndex == 706)//托管模式
// 	{
// 		iOffY = 151;
// 
// 		m_pTrusteeShipPackage1 = new CGoodGrid(3,2,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
// 		AddControl(m_pTrusteeShipPackage1);
// 		m_pTrusteeShipPackage1->SetGridType(GOODGRID_TYPE_TRUSTEESHIP_PACKAGE_1);
// 		m_pTrusteeShipPackage1->Create(this,21,78);
// 
// 		m_pSendToMermberPackage1 = new CCtrlRadio();
// 		AddControl(m_pSendToMermberPackage1);
// 		m_pSendToMermberPackage1->Create(this,35,159,125,126,127,128,110,20);
// 		m_pSendToMermberPackage1->SetText("给予物品",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
// 		m_pSendToMermberPackage1->SetTips("勾选该框后，放入该包裹内物品将会被赠送于\n该玩家（放入对方包裹内归其所有）。\n注：如果未勾选则放置入群英包裹的物品，视\n为仅供其使用。对方退队后物品会还回给你。");
// 		
// 		int iMemberPos = 0;
// 		if (iMemberPos >= g_TrusteeshipData.GetOriginalCaptainPos())
// 		{
// 			iMemberPos += 1;
// 		}
// 
// 		if (iMemberPos >= 0 && iMemberPos < MAX_TRUSTEESHIP_NUM)
// 		{
// 			m_pSendToMermberPackage1->SetChecked(g_TrusteeshipData.GetTneupMember(iMemberPos).bSendToPackage);
// 		}
// 		//
// 		m_pTrusteeShipPackage2 = new CGoodGrid(3,2,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
// 		AddControl(m_pTrusteeShipPackage2);
// 		m_pTrusteeShipPackage2->SetGridType(GOODGRID_TYPE_TRUSTEESHIP_PACKAGE_2);
// 		m_pTrusteeShipPackage2->Create(this,144,78);
// 		
// 		m_pSendToMermberPackage2 = new CCtrlRadio();
// 		AddControl(m_pSendToMermberPackage2);
// 		m_pSendToMermberPackage2->Create(this,35 + 123,159,125,126,127,128,110,20);
// 		m_pSendToMermberPackage2->SetText("给予物品",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
// 		m_pSendToMermberPackage2->SetTips("勾选该框后，放入该包裹内物品将会被赠送于\n该玩家（放入对方包裹内归其所有）。\n注：如果未勾选则放置入群英包裹的物品，视\n为仅供其使用。对方退队后物品会还回给你。");
// 		iMemberPos = 1;
// 		if (iMemberPos >= g_TrusteeshipData.GetOriginalCaptainPos())
// 		{
// 			iMemberPos += 1;
// 		}
// 
// 		if (iMemberPos >= 0 && iMemberPos < MAX_TRUSTEESHIP_NUM)
// 		{
// 			m_pSendToMermberPackage2->SetChecked(g_TrusteeshipData.GetTneupMember(iMemberPos).bSendToPackage);
// 		}		
// 		//
// 		m_pTrusteeShipPackage3 = new CGoodGrid(3,2,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
// 		AddControl(m_pTrusteeShipPackage3);
// 		m_pTrusteeShipPackage3->SetGridType(GOODGRID_TYPE_TRUSTEESHIP_PACKAGE_3);
// 		m_pTrusteeShipPackage3->Create(this,267,78);
// 	
// 		m_pSendToMermberPackage3 = new CCtrlRadio();
// 		AddControl(m_pSendToMermberPackage3);
// 		m_pSendToMermberPackage3->Create(this,35 + 123 * 2,159,125,126,127,128,110,20);
// 		m_pSendToMermberPackage3->SetText("给予物品",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
// 		m_pSendToMermberPackage3->SetTips("勾选该框后，放入该包裹内物品将会被赠送于\n该玩家（放入对方包裹内归其所有）。\n注：如果未勾选则放置入群英包裹的物品，视\n为仅供其使用。对方退队后物品会还回给你。");
// 		iMemberPos = 2;
// 		if (iMemberPos >= g_TrusteeshipData.GetOriginalCaptainPos())
// 		{
// 			iMemberPos += 1;
// 		}
// 
// 		if (iMemberPos >= 0 && iMemberPos < MAX_TRUSTEESHIP_NUM)
// 		{
// 			m_pSendToMermberPackage3->SetChecked(g_TrusteeshipData.GetTneupMember(iMemberPos).bSendToPackage);
// 		}		
// 		//
// 		m_pTrusteeShipPackage4 = new CGoodGrid(3,2,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
// 		AddControl(m_pTrusteeShipPackage4);
// 		m_pTrusteeShipPackage4->SetGridType(GOODGRID_TYPE_TRUSTEESHIP_PACKAGE_4);
// 		m_pTrusteeShipPackage4->Create(this,389,78);
// 
// 		m_pSendToMermberPackage4 = new CCtrlRadio();
// 		AddControl(m_pSendToMermberPackage4);
// 		m_pSendToMermberPackage4->Create(this,35 + 123 * 3,159,125,126,127,128,110,20);
// 		m_pSendToMermberPackage4->SetText("给予物品",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
// 		m_pSendToMermberPackage4->SetTips("勾选该框后，放入该包裹内物品将会被赠送于\n该玩家（放入对方包裹内归其所有）。\n注：如果未勾选则放置入群英包裹的物品，视\n为仅供其使用。对方退队后物品会还回给你。");
// 		iMemberPos = 3;
// 		if (iMemberPos >= g_TrusteeshipData.GetOriginalCaptainPos())
// 		{
// 			iMemberPos += 1;
// 		}
// 
// 		if (iMemberPos >= 0 && iMemberPos < MAX_TRUSTEESHIP_NUM)
// 		{
// 			m_pSendToMermberPackage4->SetChecked(g_TrusteeshipData.GetTneupMember(iMemberPos).bSendToPackage);
// 		}		
// 	}
// 	else
// 	{
// 		m_pTrusteeShipPackage1 = NULL;	
// 		m_pTrusteeShipPackage2 = NULL;	
// 		m_pTrusteeShipPackage3 = NULL;	
// 		m_pTrusteeShipPackage4 = NULL;	
// 		m_pSendToMermberPackage1 = NULL;
// 		m_pSendToMermberPackage2 = NULL;
// 		m_pSendToMermberPackage3 = NULL;
// 		m_pSendToMermberPackage4 = NULL;
// 	}

	g_pGameControl->SEND_Request_Server_Time();

	m_pGoodGrid = new CGoodGrid(m_iWCells,m_iHCells,m_iCellWidth,m_iCellHeight,m_iCellDisW,m_iCellDisH);
	AddControl(m_pGoodGrid);
	m_pGoodGrid->SetGridType(GOODGRID_TYPE_PACKAGE);
	m_pGoodGrid->SetStartPos(m_TabPage.iCurPage * 60);
	m_pGoodGrid->Create(this,86,iOffY + 70);

	//微操或附身时禁用	
// 	if (g_TrusteeshipData.GetMicroControlPos() < 0 && g_TrusteeshipData.GetCaptionSubstitutePos() < 0)
// 	{
		//////////////////////////////////////////////////////////////////////////
		m_pEnlargeMoneybag = new CCtrlButton();
		AddControl(m_pEnlargeMoneybag);
		m_pEnlargeMoneybag->CreateEx(this,480,iOffY + 220,16039,16040,16041);
		m_pEnlargeMoneybag->SetTips("扩展钱袋");

		m_pLingXiBtn = new CCtrlButton();
		AddControl(m_pLingXiBtn);
		m_pLingXiBtn->CreateEx(this,480,iOffY + 252,16408,16409,16410);
		m_pLingXiBtn->SetTips("至尊宝");		
		
		m_pZhengLiButton = new CCtrlButton();
		AddControl(m_pZhengLiButton);
		m_pZhengLiButton->CreateEx(this,480,iOffY + 284,187,188,189);
		m_pZhengLiButton->SetTips("整理包裹");

		m_pMoreButton = new CCtrlButton();		
		AddControl(m_pMoreButton); 
		m_pMoreButton->CreateEx(this,19,iOffY + 341,95,96,97);
		m_pMoreButton->SetText("更多",COLOR_BTN_NORMAL,COLOR_BTN_MOUSEON,COLOR_BTN_PRESS,COLOR_BTN_DISABLE,12);

		m_pVIPTradeWndBtn = new CCtrlButton();
		AddControl(m_pVIPTradeWndBtn);
		m_pVIPTradeWndBtn->CreateEx(this,328,iOffY + 38,90,91,92);
		m_pVIPTradeWndBtn->SetText("会员特权",COLOR_BTN_NORMAL,COLOR_BTN_MOUSEON,COLOR_BTN_PRESS,COLOR_BTN_DISABLE,12);
	
		m_pVIPButton = new CCtrlButton();
		AddControl(m_pVIPButton);
		m_pVIPButton->CreateEx(this,423,iOffY + 38,90,91,92);
		m_pVIPButton->SetText("龙虎阁",COLOR_BTN_NORMAL,COLOR_BTN_MOUSEON,COLOR_BTN_PRESS,COLOR_BTN_DISABLE,12);

		m_pWenPeiButton = new CCtrlButton();
		AddControl(m_pWenPeiButton);
		m_pWenPeiButton->CreateEx(this,80,iOffY + 38,90,91,92);
		m_pWenPeiButton->SetText("圣殿纹佩",COLOR_BTN_NORMAL,COLOR_BTN_MOUSEON,COLOR_BTN_PRESS,COLOR_BTN_DISABLE,12);

		//金币,元宝按钮
		m_pGoldButton = new CCtrlButton();
		AddControl(m_pGoldButton);
		m_pGoldButton->CreateEx(this,82,iOffY + 305,715,716,717);
		m_pGoldButton->SetTips("金币");

		m_pYuanBaoButton = new CCtrlButton();
		AddControl(m_pYuanBaoButton);
		m_pYuanBaoButton->CreateEx(this,203,iOffY + 305,718,719,720);
		m_pYuanBaoButton->SetTips("元宝");
// 	}
// 	else
// 	{
// 		m_pEnlargeMoneybag = NULL;
// 		m_pMoreButton = NULL;
// 		m_pVIPButton = NULL;
// 		m_pGoldButton = NULL;
// 		m_pYuanBaoButton = NULL;
// 	}
}

void CPackageWnd::OnDraw()
{
	int iOffY = 0;
	char str[64] = {0};

// 	if (m_iIndex == 706)
// 	{
// 		iOffY = 151;
// 
// 		int iX = 82;
// 		for (int i = 0,iNo = 0; i < MAX_TRUSTEESHIP_NUM; i ++)
// 		{
// 			if (i == g_TrusteeshipData.GetOriginalCaptainPos())
// 			{
// 				continue;
// 			}
// 			
// 			if (i == g_TrusteeshipData.GetCaptainPos() || g_TrusteeshipData.GetTneupMember(i).dwID == 0)
// 			{
// 				LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,17538);		
// 
// 				for(int j = 0;j < 6;j++)
// 				{
// 					g_pGfx->DrawTextureNL(m_iScreenX + 25 +(j % 3) * 39 + 123 * iNo,m_iScreenY + 80 + 39 * (j / 3),pTex);
// 				}
// 			}
// 			else
// 			{
// 				g_pFont->DrawText(m_iScreenX + iX,m_iScreenY + 41 ,g_TrusteeshipData.GetTneupMember(i).szName,-1,FONT_YAHEI,FONTSIZE_DEFAULT,DTF_Center);
// 				g_pFont->DrawText(m_iScreenX + iX,m_iScreenY + 54 ,"包裹",-1,FONT_YAHEI,FONTSIZE_DEFAULT,DTF_Center);
// 			}
// 
// 			iNo ++;
// 			iX += 121;
// 		}
// 
// 		if(g_TrusteeshipData.GetCaptionSubstitutePos() >= 0)
// 		{
// 			//队长元宝
// 			DWORD lOriginalMoney = ORIGINALSELF.GetGold();
// 			if(lOriginalMoney >= 0)
// 			{
// 				g_pFont->DrawText(m_iScreenX + 35,m_iScreenY + 187,"队长金币：",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
// 				ToCommaStr(str,lOriginalMoney);
// 				g_pFont->DrawText(m_iScreenX + 103,m_iScreenY + 187,str,COLOR_TEXT_NORMAL);
// 			}
// 
// 			//显示元宝
// 			DWORD lOriginalYuanBao = ORIGINALSELF.GetYuanBao();
// 			if(lOriginalYuanBao >= 0)
// 			{
// 				g_pFont->DrawText(m_iScreenX + 158,m_iScreenY + 187,"队长元宝：",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
// 				ToCommaStr(str,lOriginalYuanBao);
// 				g_pFont->DrawText(m_iScreenX + 224,m_iScreenY + 187,str,COLOR_TEXT_NORMAL);
// 			}
// 		}
// 
// 	}
// 
	g_pFont->DrawText(m_iScreenX + 246,m_iScreenY + 10,"包 裹",COLOR_TEXT_MAIN_TITLE,FONT_YAHEI,FONTSIZE_BIG);
	g_pFont->DrawText(m_iScreenX + 33,m_iScreenY + iOffY + 69,"商城",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_MIDDLE);
	if (g_strChannelName.empty())	
		g_pFont->DrawText(m_iScreenX + 330,m_iScreenY + iOffY + 314,"盛大积分",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
	g_pFont->DrawText(m_iScreenX + 340,m_iScreenY + iOffY + 344,"荣耀点",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
	//显示钱
	bool bEnableCredit = (g_dwServerSwitch & SS_CREDIT)?true:false;//是否启用信用系统

	DWORD lMoney = SELF.GetGold();
	if(lMoney >= 0)
	{
		ToCommaStr(str,lMoney);

		if(bEnableCredit)
			g_pFont->DrawText(m_iScreenX + 116,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
		else
			g_pFont->DrawText(m_iScreenX + 116,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
	}

	//显示元宝
	DWORD lYuanBao = SELF.GetYuanBao();
	if(lYuanBao >= 0)
	{
		ToCommaStr(str,lYuanBao);

		if(bEnableCredit)
			g_pFont->DrawText(m_iScreenX + 237,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
		else
			g_pFont->DrawText(m_iScreenX + 237,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
	}
	//显示绑定金币
	DWORD BindGold = SELF.GetBindGold();
	if(BindGold >= 0)
	{
		ToCommaStr(str,BindGold);

		if(bEnableCredit)
			g_pFont->DrawText(m_iScreenX + 116,m_iScreenY + iOffY + 344,str,COLOR_TEXT_NORMAL);
		else
			g_pFont->DrawText(m_iScreenX + 116,m_iScreenY + iOffY + 344,str,COLOR_TEXT_NORMAL);
	}
	//显示绑定元宝
	DWORD BindYuanBao = SELF.GetBindYuanBao();
	if(BindYuanBao >= 0)
	{
		ToCommaStr(str,BindYuanBao);

		if(bEnableCredit)
			g_pFont->DrawText(m_iScreenX + 237,m_iScreenY + iOffY + 344,str,COLOR_TEXT_NORMAL);
		else
			g_pFont->DrawText(m_iScreenX + 237,m_iScreenY + iOffY + 344,str,COLOR_TEXT_NORMAL);
	}
	if (g_strChannelName.empty())
	{	
		//盛大积分
		DWORD lSndaMark = SELF.GetSndaMark();
		if(lSndaMark >= 0)
		{
			ToCommaStr(str,lSndaMark);

			if(bEnableCredit)
				g_pFont->DrawText(m_iScreenX + 397,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
			else
				g_pFont->DrawText(m_iScreenX + 397,m_iScreenY + iOffY + 315,str,COLOR_TEXT_NORMAL);
		}
	}

	//荣耀值
	DWORD honorValue = SELF.GetHonorValue();
	if(honorValue >= 0)
	{
		ToCommaStr(str,honorValue);
		
		g_pFont->DrawText(m_iScreenX + 397,m_iScreenY + iOffY + 344,str,COLOR_TEXT_NORMAL);
	}


	////行会银两
	//DWORD lGuildTael = g_GuildData.GetGuildTael();
	//if(lGuildTael >= 0)
	//{
	//	ToCommaStr(str,lGuildTael);

	//	if(bEnableCredit)
	//		g_pFont->DrawText(m_iScreenX + 291,m_iScreenY + 307,str,0xFFFFFFFF);
	//	else
	//		g_pFont->DrawText(m_iScreenX + 383,m_iScreenY + 307,str,0xFFFFFFFF);
	//}
	////信用点
	//if(bEnableCredit)
	//{
	//	DWORD lCredit = SELF.GetCredit();
	//	sprintf(str,"%d/%d",SELF.GetCredit(),SELF.GetCreditMax());
	//	g_pFont->DrawText(m_iScreenX + 413,m_iScreenY + 307,str,0xFFFFFFFF);
	//}

	//绘制商城物品
	for(int i=0;i<QUICKITEM_COUNT;i++)
	{
		CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(i);

		int iLeft = m_iScreenX + m_ptGoodPoint.x + 3;
		int iTop = m_iScreenY + iOffY + m_ptGoodPoint.y + m_iCellHeight*i - 2;

		if(m_iMouseOn == i)
		{
			g_pGfx->DrawFillRect(iLeft,iTop,m_iCellWidth - 1,m_iCellHeight - 1,0x44FF0000);
		}

		// 绘制物品图标
		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_items, qitem.iItemLooks,EP_UI);
		if (pTex)
			g_pGfx->DrawTextureNL(m_iScreenX+ m_ptGoodPoint.x + 6, m_iScreenY + iOffY + m_ptGoodPoint.y - 2 + m_iCellHeight*i+2,pTex);
	}

	// 包裹翻页
	if(MAX_PACKAGE_ELEMENT  < 66 + m_TabPage.iCurPage * 60)
	{
		int iLocked = min(66 + m_TabPage.iCurPage * 60 - MAX_PACKAGE_ELEMENT,60);

		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,17538,EP_UI);		

		for(int i = 0;i < iLocked;i++)
		{
			g_pGfx->DrawTextureNL(m_iScreenX + 441 - (i % 10) * 39,m_iScreenY + iOffY + 264 - 39 * (i / 10),pTex);
		}
	}

}

bool CPackageWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{

	switch(dwMsg)
	{
	case MSG_CTRL_BUTTON_CLICK:
		if(!pControl)
			return false;

		if(stricmp(pControl->GetName(),"PMButton") == 0)
		{
			//if(g_PetBoothData.IsPaimaiClosed()) //判断是否关闭了拍卖行
			//	return true;
			//g_WidgetMgr.LoginPaiMai();
			//g_pControl->PopupWindow(MSG_CTRL_WOOOL_STORE_WND,OPER_CREATE,3);
			return true;
		}
		//else if(stricmp(pControl->GetName(),"SaleButton") == 0)
		//{
		//	if(g_PetBoothData.IsPaimaiClosed()) //判断是否关闭了拍卖行
		//		return true;

		//	g_pControl->Msg(MSG_CTRL_UPLOAD_WND,OPER_CREATE);
		//	return true;
		//}
		else if(pControl == m_pEnlargeMoneybag)
		{
			//g_MsgBoxMgr.PopOkCancelBox("你现在可以将钱袋扩充到存放1千万金币，但要消耗5个元宝和10万金币。你要扩钱袋吗？",MSG_CTRL_ENLARGE_MONEYBAG,0);
			g_pGameControl->SEND_Enlarge_Package_Info_Req();
			return true;
		}
		else if(pControl == m_pMoreButton)
		{
			g_pControl->PopupWindow(MSG_CTRL_WOOOL_STORE_WND,OPER_CREATE,2);
			return true;
		}
		//else if(stricmp(pControl->GetName(),"JFButton") == 0)
		//{
		//	//g_pWeb->LoginJiFen();
		//	//打开商城积分
		//	g_pControl->PopupWindow(MSG_CTRL_WOOOL_STORE_WND,OPER_CREATE,4);
		//	return true;
		//}
		else if(stricmp(pControl->GetName(),"GuildMoney") == 0)
		{
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@main");
			return true;
		}
		else if(pControl == m_pLingXiBtn)
		{
			//服务器还没有创建宝盒数据,先让它创建
			if (g_pGameData->GetBoxInfo().bOpen == false)
			{
				g_pGameControl->SEND_BOX_INFO(1,0,0);
			}

			g_pControl->PopupWindow(MSG_CTRL_LINGXIBOX_WND);
		}
		else if(pControl == m_pCreditBtn)
		{
			ShellExecute(NULL,"open","iexplore.exe","http://home.woool.sdo.com/project/200906_xinyong/",NULL,SW_SHOW);
		}
		else if(pControl == m_pVIPButton)
		{
			g_pControl->PopupWindow(MSG_CTRL_VIPSTORE_WND,OPER_UPDATE);
		}
		else if(pControl == m_pWenPeiButton)
		{
			g_pControl->PopupWindow(MSG_CTRL_WENPEI_WND,OPER_CREATE);
			this->Msg(MSG_REMOVE_ARROWTIP_CONTROL,EP_First_Level_30,NULL);
		}
		else if(pControl == m_pGoldButton)
		{
			stGoodFrom GoodFrom = CGoodGrid::GetDropGoodFrom();
			if(GoodFrom.DropGood.GetID() != 0) 
				return true;
			if(g_pGameData->GetDropMoneyFrom() == EDMYF_PACKAG)  
			{
				g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
				g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
			}
			else if(g_pGameData->GetDropMoneyFrom() == EDMYF_TRADEWND)  //交易窗口放回包裹
			{
				g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
			}
			else
			{
				g_pGameData->SetDropMoneyFrom(EDMYF_PACKAG);
				g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
			}
			return true;
		}
		else if(pControl == m_pYuanBaoButton)
		{
			stGoodFrom GoodFrom = CGoodGrid::GetDropGoodFrom();
			if(GoodFrom.DropGood.GetID() != 0) 
				return true;

			if(g_pGameData->GetDropYuanBaoFrom() == EDMYF_PACKAG)  
			{
				g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
				g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
			}
			else if(g_pGameData->GetDropYuanBaoFrom() == EDMYF_TRADEWND)  //交易窗口放回包裹
			{
				//不允许把钱拿回来
				g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
			}
			else
			{
				g_pGameData->SetDropYuanBaoFrom(EDMYF_PACKAG);
				g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
			}
			return true;
		}
		else if(pControl == m_pVIPTradeWndBtn)
		{
			g_pControl->PopupWindow(MSG_CTRL_VIPCARD_WND,OPER_UPDATE);
			return true;
		}
		else if(pControl == m_pZhengLiButton)//整理包裹
		{
			SELF.PackageGood().SortGoods(0,MAX_PACKAGE_ELEMENT - 6);
			return true;
		}
// 		else if (pControl == m_pSendToMermberPackage1 || pControl == m_pSendToMermberPackage2 || pControl == m_pSendToMermberPackage3 || pControl == m_pSendToMermberPackage4)
// 		{
// 			int iMemberPos = 0;
// 			if (pControl == m_pSendToMermberPackage2)
// 				iMemberPos = 1;
// 			else if (pControl == m_pSendToMermberPackage3)
// 				iMemberPos = 2;
// 			else if (pControl == m_pSendToMermberPackage4)
// 				iMemberPos = 3;
// 
// 			if (iMemberPos >= g_TrusteeshipData.GetOriginalCaptainPos())
// 			{
// 				iMemberPos += 1;
// 			}
// 
// 			if (iMemberPos >= 0 && iMemberPos < MAX_TRUSTEESHIP_NUM)
// 			{
// 				g_TrusteeshipData.GetTneupMember(iMemberPos).bSendToPackage = ((CCtrlRadio*)pControl)->IsChecked();
// 			}
// 		}

		break;
	case MSG_CTRL_INCORPORATE_MEDAL_OK:
		InCorporateMedal(0,1,dwData,(int)(__int64)(pControl));
		break;
	case MSG_CTRL_INCORPORATE_MEDAL_CANCEL:
		InCorporateMedal(0,0,dwData,(int)(__int64)(pControl));
		break;
	case MSG_CTRL_SPIRT_UPGRATE_WEAPON:
		InCorporateMedal(1,1,0,(int)dwData);
		break;
	case MSG_CTRL_ADDLINGLI:
		g_pGameControl->SEND_Use_Object(dwData);
		break;
	case MSG_CTRL_POPUP_ARROWTIP://冒泡
		{
			if (dwData == EP_First_GetQunYingBaoJian_PaoPao)
			{
				PopupArrowTip_BaoJian();
			}
			else if(dwData >= EP_First_GetTaoMuJian && dwData <= EP_First_Team && pControl)
			{
				PopupArrowTip_Good(ePaoPaoPromptStatus(dwData),(const char *)pControl);
			}
			return true;
		}
		break;

	default:
		break;
	}


	return CCtrlWindowX::Msg(dwMsg,dwData,pControl);
}

BOOL CPackageWnd::PopupArrowTip_BaoJian()
{
	if(!g_pGameData->HasPaoPaoStatus(EP_First_GetQunYingBaoJian_PaoPao))
	{
		int iBaoJianPos = -1,iKeyPos = -1;
		for (int iPos = 0; iPos < MAX_PACKAGE_NUM; iPos++)
		{
			CGood &good = SELF.GetPackageGood(iPos);
			if (good.GetID() != 0)
			{
				if (strcmp(SELF.GetPackageGood(iPos).GetName(),"群英宝鉴") == 0)
				{
					iBaoJianPos = iPos;						
				}
				else if (strcmp(SELF.GetPackageGood(iPos).GetName(),"群英令") == 0)
				{
					iKeyPos = iPos;						
				}
			}
		}

		if (iBaoJianPos >= 0 && iKeyPos >= 0)
		{
			CGood &tmp = SELF.GetPackageGood(iBaoJianPos);
			AddArrowTip(EP_First_GetQunYingBaoJian_PaoPao,93 + (iBaoJianPos % m_iWCells) * m_iCellWidth + m_iCellWidth / 2,72 + iBaoJianPos / m_iWCells * m_iCellHeight + m_iCellHeight / 2,XAL_TOPLEFT,false,XAL_TOPLEFT);
			tmp.SetArrowTipWnd(MSG_CTRL_PACKAGEWND);
			tmp.SetArrowTipMsgID(EP_First_GetQunYingBaoJian_PaoPao);
			return true;
		}
	}

	return false;
}

//物品冒泡
BOOL CPackageWnd::PopupArrowTip_Good(ePaoPaoPromptStatus eState,const char *szName)
{
	if (!g_pGameData->HasPaoPaoStatus(eState))
	{
		int iPos = -1,iNotFlashTexGoodPos = -1;//不闪烁的物品的位置
		for(int i = 0;i < MAX_PACKAGE_NUM;i++)
		{
			CGood &good = SELF.GetPackageGood(i);
			if(good.GetID() == 0)
				continue;

			if(good.GetID() != 0 && strcmp(SELF.GetPackageGood(i).GetName(),szName) == 0)
			{
				if (good.GetFlashTexID() != 0)
				{
					iPos = i;
					break;
				}
				else if(iNotFlashTexGoodPos == -1)
				{
					iNotFlashTexGoodPos = i;
				}
			}
		}

		if (iPos == -1)
		{
			iPos = iNotFlashTexGoodPos;
		}

		if (iPos >= 0)
		{
			CGood &tmp = SELF.GetPackageGood(iPos);

			AddArrowTip(eState,93 + (iPos % m_iWCells) * m_iCellWidth + m_iCellWidth / 2,72 + iPos / m_iWCells * m_iCellHeight + m_iCellHeight / 2,XAL_TOPLEFT,false,XAL_TOPLEFT);
			tmp.SetArrowTipWnd(MSG_CTRL_PACKAGEWND);
			tmp.SetArrowTipMsgID(eState);
			return true;
		}
	}

	return false;
}

bool CPackageWnd::OnMouseMove(int iX,int iY)
{
	int iIndex = GetIndex(iX,iY);
	CParserTip *pTip = g_pControl->GetTipWnd();

	int x = m_iScreenX + iX + 10;
	int y = m_iScreenY + iY + 10;

	//是否在绑定金币和绑定元宝处
	if (!(iX < 82 || iX >= 107 || iY < 337 || iY >= 361))
	{
		
		pTip->Clear();			
		pTip->AddText("绑定金币");
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);
		return true;
	}

	if (!(iX < 204 || iX >= 228 || iY < 337 || iY >= 361))
	{

		pTip->Clear();			
		pTip->AddText("绑定元宝");
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);
		return true;
	}

	int iOffY = 0;
	// 	if (m_iIndex == 706)
	// 	{
	// 		iOffY = 151;
    //  }
	if (!(iX < 397 || iX >= 397 + 81 || iY < 342  + iOffY|| iY >= 344 + 14 + iOffY))
	{
		pTip->Clear();			
		pTip->AddText("可在中州皇宫的荣耀特使处换取月光宝盒、荣耀勋章等物品。",0xFFFFFF00);
		//pTip->AddText("获得途径",0xFF0000FF);
		//pTip->AddText("1.在商城使用元宝购买物品的同时可获得荣耀点",0xFF0000FF);
		//pTip->AddText("2.完成各类每日任务、活动时可以获得荣耀点，如：讨伐任务、护送任务、勇士任务、军机令、天佑中州等。",0xFF0000FF);
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);
		return true;
	}
	
		


	if(iIndex >= 0 && iIndex < QUICKITEM_COUNT)
	{
		//有提示
		
		CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(iIndex);

		if(qitem.strName.size() >0)
		{
			m_iMouseOn = iIndex;
			pTip->Clear();
			pTip->AddText(qitem.strName.c_str(),0xFFFFFF00);				
			pTip->AddText("双击购买");
			pTip->AdjustXY(x,y);
			pTip->Move(x,y);
			pTip->SetShow(true);
			return true;
		}
		else
		{
			pTip->SetShow(false);
		}
	}	
	else
	{
		if(g_dwServerSwitch & SS_CREDIT)
		{
			bool bInCreditArea = false;

			if(iX >= 411 && iY >= 304 && iX <= 490 && iY <= 318)
				bInCreditArea = true;

			if(bInCreditArea)
			{
				pTip->Clear();
				char strTemp[256]={0};

				if (SELF.GetCreditState() == 0)//未开通信用
				{
					sprintf(strTemp,"信用额度%d信用点，等值于%.2f元宝",SELF.GetCreditMax(),(float)(SELF.GetCreditMax()) / 100);
					pTip->AddText(strTemp,0xFFFFFFFF,-1);
					pTip->AddText("您尚未开通信用服务，进入圈圈信用服务可以申请开通",0xFFFFFFFF,-1);
				}
				else if (SELF.GetCredit() > 0)//有可用信用额度
				{
					sprintf(strTemp,"可透支额为%d信用点，等值于%.2f元宝",SELF.GetCredit(),(float)(SELF.GetCredit())/100);
					pTip->AddText(strTemp,0xFFFFFFFF,-1);
				}
				else// if (SELF.GetCredit() == 0)
				{
					if (SELF.GetCreditArrearage() > 0)//有欠款
					{
						pTip->AddText("可透支额为0信用点，请及时充值还款",0xFFFFFFFF,-1);
					}
					else
					{
						pTip->AddText("手机未认证，可用透支额已被冻结，请进入圈圈信用服务进行认证",0xFFFFFFFF,-1);
					}
				}

				pTip->AdjustXY(x,y);
				pTip->Move(x,y);
				pTip->SetShow(true);
				return true;
			}
		}

		m_iMouseOn = -1;
		pTip->SetShow(false);
	}

	return CCtrlWindowX::OnMouseMove(iX,iY);
}

bool CPackageWnd::OnLeftButtonUp(int iX, int iY)
{
	//微操或附身包裹不能点右边快捷消费栏
	if (m_iIndex == 706)
	{
		return CCtrlWindowX::OnLeftButtonUp(iX, iY);
	}

	//if (IsInMoneyGrid(iX,iY))
	//{
	//	stGoodFrom GoodFrom = CGoodGrid::GetDropGoodFrom();
	//	if(GoodFrom.DropGood.GetID() != 0) 
	//		return true;
	//	if(g_pGameData->GetDropMoneyFrom() == EDMYF_PACKAG)  
	//	{
	//		g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
	//		g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
	//	}
	//	else if(g_pGameData->GetDropMoneyFrom() == EDMYF_TRADEWND)  //交易窗口放回包裹
	//	{
	//		g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
	//	}
	//	else
	//	{
	//		g_pGameData->SetDropMoneyFrom(EDMYF_PACKAG);
	//		g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
	//	}
	//	return true;
	//}
	//if(IsInYuanBaoGrid(iX,iY))
	//{	
	//	stGoodFrom GoodFrom = CGoodGrid::GetDropGoodFrom();
	//	if(GoodFrom.DropGood.GetID() != 0) 
	//		return true;

	//	if(g_pGameData->GetDropYuanBaoFrom() == EDMYF_PACKAG)  
	//	{
	//		g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
	//		g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
	//	}
	//	else if(g_pGameData->GetDropYuanBaoFrom() == EDMYF_TRADEWND)  //交易窗口放回包裹
	//	{
	//		//不允许把钱拿回来
	//		g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
	//	}
	//	else
	//	{
	//		g_pGameData->SetDropYuanBaoFrom(EDMYF_PACKAG);
	//		g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
	//	}
	//	return true;
	//}
	int iIndex = GetIndex(iX,iY);

	if(iIndex >= 0)
	{
		stGoodFrom GoodFrom = CGoodGrid::GetDropGoodFrom();

		if(GoodFrom.DropGood.GetID() != 0 && (GoodFrom.eFromWnd == Package_Wnd || GoodFrom.eFromWnd == Panel_Wnd))
		{
			//增加一个消息框
			g_MsgBoxMgr.PopSimpleComfirm("如果需要设置快捷栏中的物品，请进入商城双击你需要的物品。\n",0,0);
			return true;
		}
		
		////////点击了一个物品
		CQuickItem& tmp = g_WooolStoreMgr.GetQuickItem(iIndex);
		if(GoodFrom.DropGood.GetID() == 0)
		{
			if(tmp.strItemID == g_WooolStoreMgr.GetDropQuickItem().strItemID)
			{
				g_WooolStoreMgr.GetDropQuickItem().clear();
			}
			else if(tmp.strItemID.size() > 0)
			{
				g_WooolStoreMgr.GetDropQuickItem() = tmp;
			}
			else
			{
				g_WooolStoreMgr.GetDropQuickItem().clear();
			}
			return true;
		}
		return true;
	}
	return CCtrlWindowX::OnLeftButtonUp(iX, iY);
}

bool CPackageWnd::OnLeftButtonDClick(int iX,int iY)
{

	//快捷消费
	int iIndex = GetIndex(iX,iY);
	if(iIndex >= 0)
	{
		m_bClick = false;
		CGoodGrid::ReleaseDrop();

		if(g_Config.GetCheckQuick())
		{
			if(iIndex >= 0 && iIndex < QUICKITEM_COUNT && !(g_WooolStoreMgr.GetQuickItem(iIndex).strName.empty()) && g_WooolStoreMgr.GetQuickItem(iIndex).iItemLooks != 0)
			{
				g_pControl->PopupWindow(MSG_CTRL_CHECK_QUICK_WND,OPER_CREATE,iIndex);
			}
		}
		else//这个窗口双击只购买 不使用物品
		{
			g_WooolStoreMgr.UseQuickItem(iIndex , false);
		}
		return true;
	}

	return CCtrlWindowX::OnLeftButtonDClick(iX, iY);
}

bool CPackageWnd::IsInMoneyGrid(int ix,int iy)
{
	int iOffY = 0;
	if (m_iIndex == 706)
	{
		iOffY = 151;
	}
	if( ix > 83 && ix < 108 && iy > 308 + iOffY && iy < 332 + iOffY)
		return true;

	return false;
}

bool CPackageWnd::IsInYuanBaoGrid(int ix,int iy)
{
	int iOffY = 0;
	if (m_iIndex == 706)
	{
		iOffY = 151;
	}
	if( ix > 204 && ix < 229 && iy + iOffY> 308 && iy < 332 + iOffY)
		return true;

	return false;
}

int	 CPackageWnd::GetIndex(int iX,int iY)
{
	int iOffY = 0;
	if (m_iIndex == 706)
	{
		iOffY = 151;
	}
	int w = iX - m_ptGoodPoint.x;
	int h = iY - m_ptGoodPoint.y - iOffY;

	int iIndex = -1;

	if(w > 0 && w < m_iCellWidth && h >= 0 && h < m_iCellHeight * QUICKITEM_COUNT)
	{
		iIndex = h / m_iCellHeight;
	}
	return iIndex;
}

void CPackageWnd::InCorporateMedal(int iType,DWORD dwData,DWORD drFromGoodID,int iClickPos)
{
	if(iClickPos < 0 )
		return;

	if(iType == 0)//升级勋章
	{
		if (iClickPos >= MAX_PACKAGE_ELEMENT) return;

		CGood &clickedGood = SELF.GetPackageGood(iClickPos);
		stGoodFrom &goodFrom = CGoodGrid::GetDropGoodFrom();
		goodFrom.DropGood.SetID(drFromGoodID);
		if(dwData == 1)
		{
			GoodAddEffect &goodEffect = clickedGood.GetGoodAddEffect();
			goodEffect.dwStartID = MAKELONG(1070,PACKAGE_stateitem);
			goodEffect.iFrams = 15;
			goodEffect.RM = RM_ADD2;
			goodEffect.iOffX = -63;
			goodEffect.iOffY = -93;
			goodEffect.iSoundID = 7;

			if (g_pGameControl->SEND_AddObjToObj(clickedGood,goodFrom.DropGood,false))
			{
				CGoodGrid::ClearGoodFrom();
			}

		}
		else if(dwData == 0)
		{
			CGood *pGood = &(SELF.GetPackageGood(goodFrom.iWndPos));
			if(pGood->GetID() != 0)
				pGood = SELF.PackageGood().FindGood(0);
			if(pGood)
			{
				pGood->Assign(goodFrom.DropGood);
				CGoodGrid::ClearGoodFrom();
			}
		}
	}
	else if(iType == 1)//圣殿宝石升武器
	{
		CGood* pDest = NULL;
		if (LOWORD(iClickPos) >= MAX_PACKAGE_ELEMENT) return;

		if (HIWORD(iClickPos) == 1)
		{
			pDest = &(SELF.GetEquipGood(ITEM_POS_WEAPON));
		}
		else if (HIWORD(iClickPos) == 2)
		{
			pDest = &(SELF.GetEquipGood(ITEM_POS_SHIELD));
		}

		if(pDest == NULL)
		{
			//g_MsgBoxMgr.PopSimpleAlert("你没有装备武器或盾牌,无法使用圣灵宝石增强附加技能的威力！");
			g_MsgBoxMgr.PopSimpleAlert("你没有装备武器,无法使用圣殿宝石增强附加技能的威力！");
		}
		else
		{
			g_pGameControl->SEND_AddObjToObj(SELF.GetPackageGood(LOWORD(iClickPos)),*pDest);
			g_pAudio->Play(EAT_OTHER,200,g_pAudio->GetRand()++);
		}
	}
}
