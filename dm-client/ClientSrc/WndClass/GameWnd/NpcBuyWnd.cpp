#include "NpcBuyWnd.h"
#include "Global/Interface/AudioInterface.h"
#include "Baseclass/Control/ParserTip.h"
#include "GameControl/GameControl.h"
#include "GameData/WooolStoreData.h"
#include "GameData/NpcData.h"
#include "NpcWnd.h"
#include "GameData/GameData.h"

#define    PAGE_ROWS	10
#define    NPCBUY_TOP   36
#define    NPCBUY_ROW   18

INIT_WND_POSX(CNpcBuyWnd,POS_VARIABLE,POS_VARIABLE)

CNpcBuyWnd::CNpcBuyWnd()
{
	m_iDisLine = 0;
	m_iBuy = sm_dwWindowType;
	m_iCount = 0;
	m_strGoodName.clear();
	m_dwNpcID = 0;
	m_iClickRow = -1;

	m_iIndex = 104;
	m_iPages = 1;
	
	m_iAlignType = XAL_TOPLEFT;
	m_iOffX = 10;
	m_iOffY = 179;

	if(g_pControl->GetWindowPos(m_iOffX,m_iOffY,"NpcWnd"))
	{
		m_iOffY += CNpcWnd::m_sbHeaderNpc?170+202:202;
	}
	g_pControl->SetWindowPos(430,30,"PackageWnd");
	g_pControl->PopupWindow(MSG_CTRL_PACKAGEWND,OPER_RECREATE);
}

CNpcBuyWnd::~CNpcBuyWnd(void)
{

}

void CNpcBuyWnd::SetWndType(int type)
{
	m_iBuy = type;
	m_iDisLine = 0;
	m_iClickRow = -1;
}

void CNpcBuyWnd::OnCreate()
{
	CCtrlWindowX::OnCreate();

	SetCloseButton(348, 2, 80);

	//m_pUpButton = new CCtrlButton();
	//AddControl(m_pUpButton);
	//m_pUpButton->CreateEx(this,305,51,0,108,109);

	//m_pDownButton = new CCtrlButton();
	//AddControl(m_pDownButton);
	//m_pDownButton->CreateEx(this,305,171,0,110,111);

	m_pOkButton = new CCtrlButton(); 
	AddControl(m_pOkButton);
	m_pOkButton->CreateEx(this,95,220,90,91,92, 93);
	m_pOkButton->SetText("确定", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_MIDDLE, 0, FONT_YAHEI);

	m_pCancelButton = new CCtrlButton();
	AddControl(m_pCancelButton);
	m_pCancelButton->CreateEx(this,190,220,90,91,92, 93);
	m_pCancelButton->SetText("取消", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_MIDDLE, 0, FONT_YAHEI);

	m_pScroll = new CCtrlScroll();
	AddControl(m_pScroll);
	m_pScroll->CreateEx(this,345,33,16,180);
	m_pScroll->SetPos(0);

	SetMask(104);
}

bool CNpcBuyWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl )
{
	switch ( dwMsg )
	{
	case MSG_CTRL_BUTTON_CLICK:
		if(pControl == m_pOkButton)
		{
			int iPos = m_iClickRow + m_iDisLine;
			if(m_iClickRow < 0 ||  iPos >= m_iCount)
				return true;

			DWORD dwNpcID = g_NPC.GetID();

			if(m_iBuy == 1)
			{
				int iSize = g_NPC.GetVectorPTData().size();
				if(iPos < iSize)
				{
					//通过PT商城购买
					_SellData& Sell = g_NPC.GetVectorPTData().at(iPos);
					CQuickItem qitem;
					qitem.strName = Sell.name;
					qitem.strItemID = Sell.strId;
					g_WooolStoreMgr.BuyQuickItem(qitem,false);
				}
				else
				{

					_SellData& Sell = g_NPC.GetVectorSellData().at(iPos-iSize);

					if(Sell.bNext)
					{
						g_pGameControl->SEND_Exchange_Buy_Kind_Ok(dwNpcID,Sell.name.c_str(),Sell.byType);
						m_strGoodName = Sell.name;
						m_dwNpcID = dwNpcID;
					}
					else
					{
						g_pGameControl->SEND_Exchange_Buy_Ok(dwNpcID,Sell.id,Sell.name.c_str(),Sell.byType);
					}
				}
			}
			else if(m_iBuy == 3)
			{
				_SellData& Sell = g_NPC.GetVectorSellData().at(iPos);
				g_pGameControl->SEND_Exchange_Poison_Ok(dwNpcID,Sell.name.c_str());
				g_NPC.SetPoisonFlag(true);
			}
			else if(m_iBuy==4)
			{
				_SellData& Sell = g_NPC.GetVectorSellData().at(iPos);
				g_NPC.SetSecondBuy(true);
				g_pGameControl->SEND_Exchange_Buy_Ok(dwNpcID,Sell.id,Sell.name.c_str(),Sell.byType);
			}
			else if (m_iBuy ==2)
			{
				_StorageData& Storage = g_NPC.GetStorageData().at(iPos);

				g_pGameControl->SEND_Exchange_Getback_Ok(dwNpcID,Storage.id,Storage.name.c_str());
			}

			g_pAudio->Play(EAT_OTHER,7,g_pAudio->GetRand()++);
		}
		else if(pControl == m_pCancelButton)
		{
			OnClickCloseButton();
			return true;
		}
		break;

	case MSG_CTRL_NPCBUYWND:
		{
			if(dwData == 4)
			{
				SetWndType(4);
				return true;
			}
		}
		break;
	}

	return  CCtrlWindowX::Msg(dwMsg,dwData,pControl);

}


void CNpcBuyWnd::Draw(void)
{
	CCtrlWindowX::Draw();
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}

	char temp[100];

	if(m_iBuy ==1||m_iBuy ==3 || m_iBuy == 4)
	{
		g_pFont->DrawText(m_iScreenX + 63,m_iScreenY+14,"物品列表",0xFFB46428, FONT_YAHEI, FONTSIZE_MIDDLE);
		g_pFont->DrawText(m_iScreenX + 224,m_iScreenY+14,"价格",0xFFB46428, FONT_YAHEI, FONTSIZE_MIDDLE);
		g_pFont->DrawText(m_iScreenX + 302,m_iScreenY+14,"持久",0xFFB46428, FONT_YAHEI, FONTSIZE_MIDDLE);
		m_iCount = g_NPC.GetVectorSellData().size();

		if(m_iBuy == 1)
			m_iCount+= g_NPC.GetVectorPTData().size();
	}
	else if(m_iBuy ==2) //仓库
	{
		g_pFont->DrawText(m_iScreenX + 63,m_iScreenY+17,"物品列表",0xFFB46428, FONT_YAHEI);
		g_pFont->DrawText(m_iScreenX + 224,m_iScreenY+17,"持久",0xFFB46428, FONT_YAHEI);
		g_pFont->DrawText(m_iScreenX + 302,m_iScreenY+17,"状态",0xFFB46428, FONT_YAHEI); //新增状态

		m_iCount = g_NPC.GetStorageData().size();
	}
	else //异常数据放弃
	{
		return;
	}

	if(m_pScroll)
	{
		if(m_iBuy == 4)
		{
			m_pScroll->SetShow(false);
			m_pScroll->SetEnable(false);
		}
		else
		{
			m_pScroll->SetShow(true);
			m_pScroll->SetEnable(true);
		}
	}

	if(m_iClickRow + m_iDisLine >= m_iCount)
	{
		m_iClickRow = m_iCount - m_iDisLine - 1;
	}

	if(m_pScroll && m_pScroll->IsEnable())
	{
		m_iDisLine = m_pScroll->GetPos();
		m_pScroll->SetRange(m_iCount - 1);
	}

	for(int i = 0; i < PAGE_ROWS && i+m_iDisLine < m_iCount; i++ )
	{
		DWORD dwColor;
		int iPrintY = m_iScreenY + NPCBUY_TOP + NPCBUY_ROW*i + 2;

		if(m_iClickRow == i)
		{
			dwColor = COLOR_BTN_PRESS;
			g_pFont->DrawText(m_iScreenX + 46,iPrintY,"o",dwColor, FONT_YAHEI);
		}
		else
		{ 
			dwColor = COLOR_TEXT_NORMAL;
		}

		if(m_iBuy == 1)
		{
			int iSize = g_NPC.GetVectorPTData().size();
			if(i+m_iDisLine < iSize)
			{
				_SellData& Sell = g_NPC.GetVectorPTData().at(i+m_iDisLine);

				if(dwColor!=COLOR_BTN_PRESS)
					dwColor = 0xFFFFFF00;

				g_pFont->DrawText(m_iScreenX + 58,iPrintY,Sell.name.c_str(),dwColor, FONT_YAHEI);

				sprintf(temp,"%d元宝",Sell.price);
				g_pFont->DrawText(m_iScreenX +213,iPrintY,temp,dwColor);

				sprintf(temp,"%d/%d",Sell.nDura, Sell.nDura); //持久值
				g_pFont->DrawText(m_iScreenX +301,iPrintY,temp,dwColor, FONT_YAHEI);
			}
			else
			{
				_SellData& Sell = g_NPC.GetVectorSellData().at(i + m_iDisLine - iSize);

				g_pFont->DrawText(m_iScreenX + 58,iPrintY,Sell.name.c_str(),dwColor, FONT_YAHEI);

				sprintf(temp,"%d金币",Sell.price);
				g_pFont->DrawText(m_iScreenX +213,iPrintY,temp,dwColor, FONT_YAHEI);

				sprintf(temp,"%d/%d",Sell.nDura, Sell.nDura); //持久值
				g_pFont->DrawText(m_iScreenX +301,iPrintY,temp,dwColor, FONT_YAHEI);
			}
		}
		else if(m_iBuy == 3)
		{
			_SellData& Sell = g_NPC.GetVectorSellData().at(i + m_iDisLine);

			g_pFont->DrawText(m_iScreenX + 58,iPrintY,Sell.name.c_str(),dwColor);
			//if(Sell.bNext)
			//	g_pFont->DrawText(m_iScreenX + 200,iPrintY,"o",dwColor, FONT_YAHEI);

			sprintf(temp,"%d金币",Sell.price);

			g_pFont->DrawText(m_iScreenX +213,iPrintY,temp,dwColor, FONT_YAHEI);
		}
		else if(m_iBuy == 2)
		{
			//显示新增状态
			_StorageData& Storage = g_NPC.GetStorageData()[i + m_iDisLine];

			if(Storage.flag == 1)
			{
				g_pFont->DrawText(m_iScreenX +301,iPrintY,"锁",dwColor, FONT_YAHEI);
			}
			else if(Storage.flag == 2)
			{
				if(dwColor!=COLOR_BTN_PRESS)
					dwColor = 0xFFFFFF00;
				g_pFont->DrawText(m_iScreenX +301,iPrintY,"封",dwColor, FONT_YAHEI);
			}
			else if(Storage.flag == 3)
			{
				if(dwColor!=COLOR_BTN_PRESS)
					dwColor = 0xFFFFFF00;
				g_pFont->DrawText(m_iScreenX +301,iPrintY,"封/锁",dwColor, FONT_YAHEI);
			}
			else
			{
				g_pFont->DrawText(m_iScreenX +301,iPrintY,"无",dwColor, FONT_YAHEI);
			}

			g_pFont->DrawText(m_iScreenX + 58,iPrintY,Storage.name.c_str(),dwColor, FONT_YAHEI);
			sprintf(temp,"%d/%d",Storage.durSmall,Storage.durBig);
			g_pFont->DrawText(m_iScreenX +213,iPrintY,temp,dwColor, FONT_YAHEI);
		}
		else if(m_iBuy ==4)
		{
			_SellData& Sell = g_NPC.GetVectorSellData().at(i + m_iDisLine);

			g_pFont->DrawText(m_iScreenX + 58,iPrintY,Sell.name.c_str(),dwColor, FONT_YAHEI);
			sprintf(temp,"%d金币",Sell.price);
			g_pFont->DrawText(m_iScreenX +213,iPrintY,temp,dwColor, FONT_YAHEI);
			sprintf(temp,"%d/%d",Sell.bNext, Sell.bNext); //持久值
			g_pFont->DrawText(m_iScreenX +301,iPrintY,temp,dwColor, FONT_YAHEI);
		}
	}

}


bool CNpcBuyWnd::OnLeftButtonDown(int iX, int iY)
{
	m_bClick = true;
	return CCtrlWindowX::OnLeftButtonDown(iX,iY);
}

bool CNpcBuyWnd::OnLeftButtonUp(int iX, int iY)
{
	m_bClick = false;

	if((iX > 15 && iX < 345 && iY > 36 && iY < 214))
	{
		m_iClickRow = (iY - NPCBUY_TOP) / NPCBUY_ROW;

		if(m_iClickRow >= 0 && m_iClickRow < PAGE_ROWS &&
			m_iClickRow + m_iDisLine < m_iCount)
		{
			g_pAudio->Play(EAT_OTHER,9,g_pAudio->GetRand()++);
			OnMouseMove(iX,iY);//tips
		}
		else
		{
			m_iClickRow = -1;
		}
	}
	return CCtrlWindowX::OnLeftButtonUp(iX,iY);
}

bool CNpcBuyWnd::OnMouseMove(int iX, int iY)
{
	CCtrlWindowX::OnMouseMove(iX,iY);
	CParserTip* pTip = g_pControl->GetTipWnd();
	if(!pTip)
		return true;

	pTip->SetShow(false);
	pTip->Clear();

	if(m_iClickRow >= 0 && (m_iBuy == 2 ||m_iBuy == 4))
	{
		int iSize = g_NPC.GetGoodList().size();
		int iPos = m_iClickRow+m_iDisLine;

		if(iPos>=0 && iPos < iSize)
		{
			pTip->Parse(g_NPC.GetGoodList().at(m_iClickRow+m_iDisLine));
			pTip->Move(m_iScreenX + 255,m_iScreenY + m_iClickRow*NPCBUY_ROW + NPCBUY_TOP);
			pTip->SetShow(true);
		}
	}

	return true;
}

bool CNpcBuyWnd::OnWheel(int iWheel)
{
	m_pScroll->OnWheel(iWheel);
	m_iDisLine = m_pScroll->GetPos();
	return true;
}

void CNpcBuyWnd::OnClickCloseButton()
{
	g_pAudio->Play(EAT_OTHER,1,g_pAudio->GetRand()++);
	CloseWindow();
}
