#include "wooolstorewnd.h"
#include "GameControl/GameControl.h"
#include "GameData/LoginData.h"
#include "GameData/GameData.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/MacroDefine.h"
#include "Baseclass/Control/ParserTip.h"
#include "BaseClass/Control/CtrlMenuWnd.h"
#include "GameClient/WidgetManager.h"
#include "GameClient/SDOAInterface.h"
#include "GameData/OtherData.h"
#include <Shellapi.h>

#define FONT_HALFLEN	6
#define FONT_HALFLEN1	8
#define DISPLAYLINES	12	// 左下控件可显示的行数
#define SCROLLINTERVAL 5000	// 滚动提示条的间隔

//////////////////////////////商城//////////////////////////////////////////////

#define STORESX  12         // 第一个控件起始的x坐标
#define STORESY  11         // 第一个控件起始的y坐标
#define STOREEX  30         // 第一个控件结束的x坐标
#define STOREEY  100        // 第一个控件结束的y坐标
#define STOREINTERVAL 90   // 控件y方向的间隔


//////////////////////////////大页签////////////////////////////////////////////
#define MAINSX 294						// sx表示第一个控件起始的x坐标，sy表示第一个控件起始的y坐标
#define MAINSY 14						
#define MAINXINTERVAL 50				// 主项控件x方向上的间隔
#define MAINEX 271						// ex表示第一个控件结束的x坐标，ey表示第一个控件结束的y坐标
#define MAINEY 33
#define TEXX 242
#define TEXY 0
//////////////////////////////小页签////////////////////////////////////////////
#define SUBY 41						// 小页签左边与MAINSX对其
#define SUBINTERVAL 6					// 小页签x方向间距
//////////////////////////////翻页标签////////////////////////////////////////////
#define PAGECTRLMIDDLE	445
#define PAGECTRLY		418
#define PAGEXINTERVAL	6
//////////////////////////////图标////////////////////////////////////////////
#define ICONSX 279
#define ICONSY 88
#define ICONEX 311
#define ICONEY 120
#define ICONXINTERVAL 251
#define ICONYINTERVAL 70
//////////////////////////////热区////////////////////////////////////////////
#define HOTSX 27
#define HOTSY 316
#define HOTEX (HOTSX + 33)
#define HOTEY (HOTSY + 34)
#define HOTYINTERVAL 45
//////////////////////////////元宝数量和提示////////////////////////////////////////////
#define TIPSX 482
#define TIPSY 424
#define HELPX 300
//////////////////////////////////////////////////////////////////////////

INIT_WND_POSX(CWooolStoreWnd,POS_VARIABLE,POS_VARIABLE)

CWooolStoreWnd::CWooolStoreWnd( )
{
	m_iWndType = sm_dwWindowType;

	DWORD dwCount = GetTickCount();

	memset(m_pButtonBuy, 0, sizeof(m_pButtonBuy));
	memset(m_pButtonSend, 0, sizeof(m_pButtonSend));
	m_pLeaveStore = NULL;
	m_pAddYuanBao = NULL;
	//m_pSNDAJiFen = NULL;
	//m_pClose = NULL;
	m_pCreditBtn = NULL;

	m_dwLastCheckTime = dwCount;
	m_PageCtrl.SetCtrlType(1);//设置页面控件的类型

	m_bDClicked = false;
	//if(g_dwServerSwitch & SS_CREDIT)
	//{
	//	m_iIndex = 10480;
	//}
	//else
	{
		m_iIndex = 10412;
	}	

 	m_iAlignType = XAL_TOP;
	m_iOffX = -10;
	m_iOffY = (g_pGfx->GetWidth() > 800)?40:0;

	m_iWidgetX = m_iWidgetY = 0;

	//CCtrlWindow *pWnd = g_pControl->FindWindowByName("UploadWnd");
	//if(pWnd && m_iWndType == 2)
	//{
	//	m_iAlignType = XAL_TOPLEFT;
	//	m_iOffX = pWnd->GetX();
	//	m_iOffY = pWnd->GetY();
	//}
	//g_pControl->PopupWindow(MSG_CTRL_UPLOAD_WND,OPER_CLOSE);

	//以下为新版页签相关
	m_iVersion = 2;

	//S_TabPage* pGameConfigPage = AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),0,34,85,86,87,88,"游戏商城",NULL,true,0,0,1);
	////////////////////商城页签不要,后面可能会加上,这里不换图,在isincontrol里处理一下,以后加上页签后去掉相关代码
	S_TabPage* pParentPage = AddTabPage(0,0,MAKELONG(10412,PACKAGE_INTERFACE),0,0,0,0,0,0,"",NULL,true,0,0,1);

	CWooolStoreMgr& rMgr = g_WooolStoreMgr;

	for (int i = 0; i < MAINGROUPCOUNT; i++)
	{
		MainGroup* pMainGroup = rMgr.GetMainGroupByIndex(i);
		if (pMainGroup)
		{
			AddTabPage(0,0,MAKELONG(10412,PACKAGE_INTERFACE),302 + i*50,14,115,116,117,118,pMainGroup->m_strMainGroupName.c_str(),pParentPage,false,0,0,1);
		}
		else
		{
			break;
		}
	}
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),244 + i*50,14,115,116,117,118,"首 页",pGameConfigPage,false,0,0,1);
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),294,14,115,116,117,118,"喜 庆",pGameConfigPage,false,0,0,1);
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),344,14,115,116,117,118,"百 变",pGameConfigPage,false,0,0,1);
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),394,14,115,116,117,118,"礼 包",pGameConfigPage,false,0,0,1);
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),444,14,115,116,117,118,"其 他",pGameConfigPage,false,0,0,1);
	//AddTabPage(0,0,MAKELONG(10400,PACKAGE_INTERFACE),494,14,115,116,117,118,"帮助",pGameConfigPage,false,0,0,1);

	AddTabPage(0,0,MAKELONG(0,PACKAGE_INTERFACE),0,0,0,0,0,0,"",pParentPage,false,0,0,1);

	//AddTabPage(0,0,MAKELONG(10401,PACKAGE_INTERFACE),0,119,85,86,87,88,"分红商城",NULL,true,0,0,1);

}

CWooolStoreWnd::~CWooolStoreWnd()
{
	//if(g_WooolStoreMgr.GetWooolStorePage() == WOOOLSTORE_PAIMAI)
	//{
	//	PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_C2C,HOSTAPP_CLOSE);
	//}
	g_WooolStoreMgr.SetWooolStorePage(0); 
	g_WidgetMgr.SetShow(EWT_FIRST,false);
	::SetFocus(g_hWnd);

}

void CWooolStoreWnd::OnCreate(void)
{
	CCtrlWindowX::OnCreate();

	iStoreIndex = StoreIndexGame;
	iMainIndex = 0;
	iSubIndex = 0;
	iItemIndex = 0;
	iOffset = 0;
	iTextPos = 0;

	//游戏商城,易宝商城,拍卖行的页签
	m_StoreGroupCtrl.vHotArea.clear();
	for(int i = 0; i < STOREGROUPCOUNT;++i)
	{
		//m_StoreGroupCtrl.PushElement(STORESX,STOREEX,STORESY + i*STOREINTERVAL,STOREEY + i*STOREINTERVAL,0,0,((i == 0)? -1 : (10457 + i)));
		m_StoreGroupCtrl.PushElement(0,0,0,0,0,0,0);
	}

	//点了分红这一块跳到分红页面,没有直接改变STOREGROUPCOUNT,怕以后还会加上,这里只是特殊处理一下分红,因为目前只有商城和这个页签有用
	PTStoreHotArea &gamestoreArea =m_StoreGroupCtrl.vHotArea[StoreIndexFH];
	gamestoreArea.left = 0;gamestoreArea.right = 25;gamestoreArea.top = 35;gamestoreArea.bottom = 116;
	PTStoreHotArea &fhArea =m_StoreGroupCtrl.vHotArea[StoreIndexFH];
	fhArea.left = 0;fhArea.right = 25;fhArea.top = 119;fhArea.bottom = 200;


	//初始化窗口数据指针
	CWooolStoreMgr& rMgr = g_WooolStoreMgr;

	pMain = rMgr.GetMainGroupByIndex(iMainIndex);
	pSub = rMgr.GetSubGroupByIndex(iMainIndex, iSubIndex);
	pItem = rMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);

	// 第一次请求数据，本机有则不向服务端发送请求
	g_WooolStoreMgr.QueryInfo(0);

	int i = 0;

	m_MainGroupCtrl.vHotArea.clear();
	for (i = 0; i < MAINGROUPCOUNT; ++i)
	{// 主类控件
		m_MainGroupCtrl.PushElement(MAINSX + i * MAINXINTERVAL, MAINEX + i * MAINXINTERVAL, MAINSY, MAINEY, 
			//TEXX, TEXY, ((i == 0) ? -1 : (10400 + i)));
		    TEXX, TEXY, 0);
	}

	// 子类目录动态创建

	// 物品控件
	m_ItemGroupCtrl.vHotArea.clear();
	for (i = 0; i < ICONCOUNT / 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{	
			//10个物品控件
			m_ItemGroupCtrl.PushElement(ICONSX + ICONXINTERVAL * j, ICONEX + ICONXINTERVAL * j,
				ICONSY + ICONYINTERVAL * i, ICONEY + ICONYINTERVAL * i);

			//购买按钮
			m_pButtonBuy[i * 2 + j] = new CCtrlButton();
			AddControl(m_pButtonBuy[i * 2 + j]);
			m_pButtonBuy[i * 2 + j]->CreateEx(this, ICONSX + ICONXINTERVAL * j + 148, ICONSY + ICONYINTERVAL * i - 3, 132, 133, 134, 135);
			m_pButtonBuy[i * 2 + j]->SetText("使用元宝购买", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

			//赠送按钮
			m_pButtonSend[i * 2 + j] = new CCtrlButton();
			AddControl(m_pButtonSend[i * 2 + j]);
			m_pButtonSend[i * 2 + j]->CreateEx(this, ICONSX + ICONXINTERVAL * j + 148, ICONSY + ICONYINTERVAL * i + 19, 132, 133, 134, 135);
			m_pButtonSend[i * 2 + j]->SetText("绑定元宝购买", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);
		}
	}

	m_HotGroupCtrl.vHotArea.clear();
	// 创建热卖区控件。并绑定指定数据到控件
	for (int i = 0; i < HOTCOUNT / 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			m_HotGroupCtrl.PushElement(HOTSX + HOTYINTERVAL * j, HOTEX + HOTYINTERVAL * j, HOTSY + i * HOTYINTERVAL, HOTEY + i * HOTYINTERVAL);
			(m_HotGroupCtrl.vHotArea.operator [](i * 5 + j)).SetAppend(g_WooolStoreMgr.GetHotItem(i * 5 + j));
		}
	}
	

	// 我要冲值按钮冲值
	m_pAddYuanBao = new CCtrlButton();
	AddControl(m_pAddYuanBao);
	m_pAddYuanBao->CreateEx(this, 608,450, 132, 133, 134, 135);
	m_pAddYuanBao->SetText("我要充值", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

	//m_pCreditBtn = new CCtrlButton();
	//AddControl(m_pCreditBtn);
	//m_pCreditBtn->CreateEx(this,395,418,17414,17415,17416);


	//// 盛大积分 按钮
	//m_pSNDAJiFen = new CCtrlButton();
	//AddControl(m_pSNDAJiFen);
	//m_pSNDAJiFen->CreateEx(this, 43, 418, 0, 10471, 10472);

	//活动领奖
	m_pActAward = new CCtrlButton();
	AddControl(m_pActAward);
	m_pActAward->CreateEx(this, 200,450, 132, 133, 134, 135);
	m_pActAward->SetText("活动奖励", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

	// 退出商城按钮
	m_pLeaveStore = new CCtrlButton();
	AddControl(m_pLeaveStore);
	m_pLeaveStore->CreateEx(this, 698,447, 90, 91, 92, 93);
	m_pLeaveStore->SetText("退出商城", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);


	//会员	
	m_pVipMemberBtn = new CCtrlButton();
	AddControl(m_pVipMemberBtn);
	m_pVipMemberBtn->CreateEx(this, 90,408, 165,166,167,168);
	m_pVipMemberBtn->SetText("会 员", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE,FONTSIZE_MIDDLE,0,FONT_YAHEI);


	// 关闭按钮
	//m_pClose = new CCtrlButton();
	//AddControl(m_pClose);
	//m_pClose->CreateEx(this, 782,2,10492,10493,10494) ;
	SetCloseButton(765,1,80);

	////说明框的滚动条
	//m_pArrowUp = new CCtrlButton();
	//AddControl(m_pArrowUp);
	//m_pArrowUp->CreateEx(this, 196, 240, 4626, 4627, 4628);

	//m_pArrowDown = new CCtrlButton();
	//AddControl(m_pArrowDown);
	//m_pArrowDown->CreateEx(this, 196, 383, 4623, 4624, 4625);

	//m_pScroll = new CCtrlScroll();
	//AddControl(m_pScroll);
	//m_pScroll->Create(this, 198, 260, 17, 122,4629);
	//m_pScroll->SetPos(0);
	//m_pScroll->SetEnable(false);

	// 详细说明框
	//m_pDisplayText = new CCtrlMultiEdit(12288,22,DISPLAYLINES,false);
	//AddControl(m_pDisplayText);
	//m_pDisplayText->Create(this,FONTSIZE_SMALL,46,210,206 - 46,426 - 210,true);
	//m_pDisplayText->SetText("   ",FONT_YAHEI,FONTSIZE_SMALL);


	//if(m_iWndType == 3)
	//{
	//	SwitchSubPage(WOOOLSTORE_PAIMAI);
	//}
	//else if(m_iWndType == 4)
	//{
	//	SwitchSubPage(WOOOLSTORE_JIFEN);
	//}
	//else if(m_iWndType == 5)
	//{
	//	SwitchSubPage(WOOOLSTORE_REAL);
	//}
	//else 
		if(m_iWndType == 6)//在uploadwnd处点不我要充值
	{
		SwitchSubPage(WOOOLSTORE_PAY);
	}
	else if(m_iWndType == 7)//从系统公告中点击商城道具名称直接跳到对面的页面
	{
		SetGameStoreGroup();//设置游戏商城的大类、子类、页
		SwitchSubPage(0);
	}
	else if(m_iWndType == 8)
	{
		SwitchSubPage(WOOOLSTORE_FENGHONG);
	}

	m_PageCtrl.vHotArea.clear();
	m_SubGroupCtrl.vHotArea.clear();
}

void CWooolStoreWnd::OnMove()
{
	CCtrlWindowX::OnMove();

	g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY);
}

void CWooolStoreWnd::OnDraw()
{
	CCharacter *pOldSelf = g_pSelf;
	g_pSelf = &ORIGINALSELF;

	if (g_strChannelName.empty())
		g_pFont->DrawText(m_iScreenX + 52,m_iScreenY + 454,"盛大积分",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

	g_pFont->DrawText(m_iScreenX + 295,m_iScreenY + 454,"元宝余额",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
	g_pFont->DrawText(m_iScreenX + 440,m_iScreenY + 454,"绑定元宝余额",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

	int iGuildTexID = 10407;
	if (SELF.GetSndaMark() >= 80000)
	{
		iGuildTexID = 10409;
	}
	else if (SELF.GetSndaMark() >= 20000)
	{
		iGuildTexID = 10408;
	}

	g_pGfx->DrawTextureNL(m_iScreenX + 14,m_iScreenY + 30,g_pTexMgr->GetTex(PACKAGE_INTERFACE,iGuildTexID,EP_UI));


	if (m_SubGroupCtrl.vHotArea.size() == 0 && pMain != 0 && pMain->m_vSubGroup.size() > 0)	// 大类下拥有小类数据，但没有控件
	{// 有大类下的子类数据，但没有子类控件， 创建子类控件，只在需要时创建
		int sx = MAINSX;//(m_MainGroupCtrl.vHotArea.operator [](iMainIndex)).left;
		for (int i = 0; i < pMain->m_vSubGroup.size(); ++i)
		{
			SubGroup& sg = pMain->m_vSubGroup.operator [](i);
			std::string& strTemp = sg.m_strSubGroupName;
			m_SubGroupCtrl.PushElement(sx, FONT_HALFLEN1 * strTemp.size() + sx, SUBY, SUBY + 12, -1, -1, -1, PACKAGE_INTERFACE, strTemp.c_str());
			sx += FONT_HALFLEN1 * strTemp.size() + SUBINTERVAL;
		}

		pSub = g_WooolStoreMgr.GetSubGroupByIndex(iMainIndex, iSubIndex);	// 更新子目录的指针
	}

	if (pSub != 0 && pSub->m_vSubItem.size() > 0)	// 拥有子类和子类下的物品
	{
		if (m_PageCtrl.vHotArea.size() == 0)
		{// 如果没有页面控件（即未进行分页）则创建（分页用，即窗体下沿的页码），只在需要时创建
			int iPageCount = pSub->m_vSubItem.size() / ICONCOUNT + ((pSub->m_vSubItem.size() % ICONCOUNT == 0) ? 0 : 1);
			int sx;
			char szTemp[64];
			memset(szTemp,0,sizeof(szTemp));
			for(int k=1;k<=iPageCount;++k)
			{	
				if(k%NEXTPAGEUNION==1 )
				{	
					if(k==1 && iPageCount<=NEXTPAGEUNION)
						sx = (PAGECTRLMIDDLE - (iPageCount * FONT_HALFLEN*5 + (iPageCount-1)* PAGEXINTERVAL) / 2);
					else if(iPageCount-k>(NEXTPAGEUNION-1)) //既有上一页 又有下一页
					{	
						if(k == 1)
							sx = (PAGECTRLMIDDLE - ( (NEXTPAGEUNION+1) * FONT_HALFLEN*5 + (NEXTPAGEUNION+1)* PAGEXINTERVAL) / 2);
						else
							sx = (PAGECTRLMIDDLE - ( (NEXTPAGEUNION+2) * FONT_HALFLEN*5 + (NEXTPAGEUNION+1)* PAGEXINTERVAL) / 2);
					}
					else//只有上一页
						sx = (PAGECTRLMIDDLE - ( (NEXTPAGEUNION+1) * FONT_HALFLEN*5 + (NEXTPAGEUNION)* PAGEXINTERVAL) / 2);
					if( k > NEXTPAGEUNION)
					{
						sprintf(szTemp, "%s", "上一页");
						m_PageCtrl.PushElement(sx, sx+FONT_HALFLEN * strlen(szTemp), PAGECTRLY, PAGECTRLY + 20, -1, -1, -1, PACKAGE_INTERFACE, szTemp);
						sx += FONT_HALFLEN * strlen(szTemp) + PAGEXINTERVAL;
					}
				}

				sprintf(szTemp, "第%d页", k);
				m_PageCtrl.PushElement(sx, sx+FONT_HALFLEN * strlen(szTemp), PAGECTRLY, PAGECTRLY + 20, -1, -1, -1, PACKAGE_INTERFACE, szTemp);
				sx += FONT_HALFLEN * strlen(szTemp) + PAGEXINTERVAL;

				if( k%NEXTPAGEUNION== 0 && (iPageCount)>k )
				{
					sprintf(szTemp, "%s", "下一页");
					m_PageCtrl.PushElement(sx, sx+FONT_HALFLEN * strlen(szTemp), PAGECTRLY, PAGECTRLY + 20, -1, -1, -1, PACKAGE_INTERFACE, szTemp);
					sx += FONT_HALFLEN * strlen(szTemp) + PAGEXINTERVAL;
				}
			}

			if (iOffset == -1)
			{// 击中热区后iOffset保存选中物品的ID，并会进入该处，由保存过来的物品ID号，在子类下找到对应物品

				int i;
				iOffset = m_PageCtrl.iActiveIndex;//这里仅仅作初始化
				std::string strExtract;

				if (iBuyItemID.size() > 4)
					strExtract.assign(iBuyItemID.c_str() + 4, iBuyItemID.size() - 4);
				//int mainindex,subindex;
				//SubGroup* pSub = g_WooolStoreMgr.GetSubGroupByID(iOffset, mainindex, subindex);
				//if (pSub != NULL)
				{// 搜索

					for (i = 0; i < pSub->m_vSubItem.size(); ++ i)
					{

						if (pSub->m_vSubItem.operator [](i).m_dwItemID == strExtract) 
						{
							break;
						}
					}
				}

				if (i == pSub->m_vSubItem.size() || i == 0) 
				{// 在对应的子类里没有找到对应
					m_PageCtrl.iActiveIndex = 0;
					m_ItemGroupCtrl.iActiveIndex = 0;
				}
				else
				{// 找到对应的物品，更新对应索引
					m_ItemGroupCtrl.iActiveIndex = i % ICONCOUNT;
					m_PageCtrl.iActiveIndex = i / ICONCOUNT;
				}
				iItemIndex = m_ItemGroupCtrl.iActiveIndex;
				iOffset = m_PageCtrl.iActiveIndex;
			}
			g_WooolStoreMgr.QueryInfo(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);// 请求选定物品的详细说明

			// 更新选定物品的指针，因为在没得到数据以前，pItem为NULL，现在有数据，则指向对应数据
			pItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);
		}

		for (int i = 0; i < ICONCOUNT; ++i)
		{// 绑定item到item控件上,没有对应物品的则取消原来的绑定
			if ((i + iOffset * ICONCOUNT) < pSub->m_vSubItem.size())
			{
				(m_ItemGroupCtrl.vHotArea.operator [](i)).SetAppend(&(pSub->m_vSubItem.operator [](i + iOffset * ICONCOUNT)));
			}
			else
				(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();
		}
		//pItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);
	}

	//绘制各个控件组,什么商城
	if(iStoreIndex < STOREGROUPCOUNT)
	{
 		m_StoreGroupCtrl.Draw(m_iScreenX,m_iScreenY,HADT_TEX);
	}
	else
	{
		LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,10458,EP_UI);
		if(pTex)
			g_pGfx->DrawTextureNL(m_iScreenX,m_iScreenY,pTex);

		for(int i = 0;i < m_StoreGroupCtrl.vHotArea.size();++i)
			(m_StoreGroupCtrl.vHotArea.operator[](i)).SetClickEnable(true);
	}

	//if(iStoreIndex != StoreIndexRS && iStoreIndex != StoreIndexJF)
	//{
	//	LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,10488);
	//	if(pTex)
	//	{
	//		g_pGfx->DrawTextureNL(m_iScreenX+12,m_iScreenY+106,pTex);
	//	}
	//}

	//游戏商城内界面
	if(iStoreIndex == StoreIndexGame)
	{
		if(iMainIndex < MAINGROUPCOUNT)
		{
			m_MainGroupCtrl.Draw(m_iScreenX, m_iScreenY, HADT_TEX);
		}
		else
		{
			//LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,10406);
			//if(pTex)
			//	g_pGfx->DrawTextureNL(m_iScreenX + TEXX,m_iScreenY + TEXY,pTex);

			for(int i = 0;i < m_MainGroupCtrl.vHotArea.size();++i)
				(m_MainGroupCtrl.vHotArea.operator[](i)).SetClickEnable(true);
		}
		m_SubGroupCtrl.Draw(m_iScreenX, m_iScreenY, HADT_TEXT);
		m_ItemGroupCtrl.Draw(m_iScreenX, m_iScreenY, HADT_ICON);
		m_HotGroupCtrl.Draw(m_iScreenX, m_iScreenY, HADT_HOT);
		m_PageCtrl.Draw(m_iScreenX, m_iScreenY, HADT_PAGE);

		// 绘制预览和提示
		//DrawPreviewAndTips();

		if (iMainIndex == HELPINDEX)
		{// drawhelp
			g_pGfx->DrawFillRect(m_iScreenX + 270,m_iScreenY + 60,770 - 270,409 - 60,0xFF000000);

			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 108, "游戏商城帮助", -1,FONT_YAHEI);
			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 128, "1、如果元宝余额不足，请点击“我要充值”按钮充值。", -1,FONT_YAHEI);
			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 148, "2、单击选择好的商品，可以在商城界面左侧看到道具图像和介绍。", -1,FONT_YAHEI);
			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 168, "3、单击道具下方的“购买”按钮即可购买该道具。", -1,FONT_YAHEI);
			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 188, "4、如果元宝余额或包裹空位不足，将无法购买道具。", -1,FONT_YAHEI);
			g_pFont->DrawText(m_iScreenX + HELPX, m_iScreenY + 208, "5、购买好的道具会直接发送到角色的包裹里。", -1,FONT_YAHEI);			
			
			DrawSysInfo(); //绘制盛大积分，元宝等信息
			g_pSelf = pOldSelf;
			return;
		}
	}

	DrawSysInfo(); //绘制盛大积分，元宝等信息

	//绘制登陆实物商城的等待框
	int iPage = g_WooolStoreMgr.GetWooolStorePage();
	if((iStoreIndex != StoreIndexGame || iMainIndex == PAYINDEX)&& iPage > 0 && !g_WidgetMgr.IsShow(EWT_FIRST))
	{
		//if(iPage != WOOOLSTORE_PAY)
		//{
		//	DrawTexture(248,160, 10419); 
		//}

		string str;
		switch(iPage)
		{
		//case WOOOLSTORE_REAL:
		//	str = "正在加载最新商品...";
		//	break;
		//case WOOOLSTORE_PAIMAI:
		//	str = "正在加载最新拍卖道具...";
		//	break;
		case WOOOLSTORE_PAY:
			str = "正在连接中，请稍候...";
			//g_pGfx->DrawFillRect(m_iScreenX + 234,m_iScreenY + 58,657 - 234,409 - 58,0xFF000000);
			g_pGfx->DrawFillRect(m_iScreenX + 270,m_iScreenY + 60,770 - 270,409 - 60,0xFF000000);
			break;
		case WOOOLSTORE_FENGHONG:
			str = "正在登陆分红商城";
			break;
		//case WOOOLSTORE_JIFEN:
		//	str = "正在登陆积分商城";
		//	break;
		}
		g_pFont->DrawText(m_iScreenX + 385, m_iScreenY + 230,str.c_str(),COLOR_YELLOW,FONT_YAHEI,FONTSIZE_MIDDLE);
	}

	g_WidgetMgr.RenderWidget(EWT_FIRST);

	//if(m_pClose)
	//	m_pClose->Draw();

	//CheckRecv();//检查接受礼品

	//if(pTip->IsShow())
	//{
	//	pTip->Draw();
	//}


	g_pSelf = pOldSelf;
}


void CWooolStoreWnd::DrawPreviewAndTips()
{
	// 绘制左上角的预览图片和左下角的说明文字
	if (pItem != NULL)
	{
		LPTexture pTex = g_pTexMgr->GetTexImm( PACKAGE_bgitems, pItem->m_dwPreview,EP_UI);
		if (pTex != NULL)
		{
			g_pGfx->DrawTextureNL(m_iScreenX + 51, m_iScreenY + 28, pTex);
		}

		/*
		if (pItem->m_strDesc.size() > 0 && m_pDisplayText != NULL && strlen(m_pDisplayText->GetText()) < 4)
		{
			m_pDisplayText->SetText(pItem->m_strDesc.c_str(),FONT_YAHEI,FONTSIZE_SMALL);
			//if (m_pDisplayText->GetLines() > DISPLAYLINES && m_pScroll != NULL) 
			//{
			//	m_pScroll->SetEnable(true);
			//	m_pScroll->SetRange(m_pDisplayText->GetLines() - DISPLAYLINES);
			//	m_pScroll->SetPos(0);
			//}
		}
		*/
	}
}

void CWooolStoreWnd::DrawSysInfo()
{
	char szTemp[128] = {0};

	// 绘制元宝数量
	DWORD lYuanBao = SELF.GetYuanBao();

	sprintf(szTemp, "%d", lYuanBao);
	g_pFont->DrawText(m_iScreenX + 360,m_iScreenY + 454,szTemp,COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

	if (g_strChannelName.empty())
	{
		// 绘制盛大积分
		sprintf(szTemp, "%d", SELF.GetSndaMark());
		g_pFont->DrawText(m_iScreenX + 120,m_iScreenY + 454,szTemp,COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
	}


	DWORD lBindYuanBao = SELF.GetBindYuanBao();

	sprintf(szTemp, "%d", lBindYuanBao);
	g_pFont->DrawText(m_iScreenX + 530,m_iScreenY + 454,szTemp,COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

	//// 绘制滚动的提示信息
	//if(g_dwServerSwitch & SS_CREDIT)
	//{
	//	sprintf(szTemp, "%d/%d", SELF.GetCredit(),SELF.GetCreditMax()); 
	//	g_pFont->DrawText(m_iScreenX + 451, m_iScreenY + 423, szTemp, -1);		
	//}
	//else// 绘制滚动的提示信息
	//{
	//	static DWORD isTime = 0;
	//	DWORD dwCount = GetTickCount();
	//	if(isTime == 0)
	//		isTime = dwCount;

	//	int iLine;
	//	const char* pTextShow = NULL;

	//	if (SCROLLINTERVAL != 0 && g_WooolStoreMgr.GetScrollTextLines() != 0)
	//	{
	//		iLine = ((dwCount - isTime) / SCROLLINTERVAL) % g_WooolStoreMgr.GetScrollTextLines();
	//		pTextShow = g_WooolStoreMgr.GetScrollTextContent(iLine);
	//		if (pTextShow != NULL)
	//			g_pFont->DrawText(m_iScreenX + TIPSX, m_iScreenY + TIPSY, pTextShow, -1);
	//	}
	//}
}

bool CWooolStoreWnd::OnLeftButtonDown(int iX, int iY)
{
	m_bDClicked = false;

	if(iX > 43 && iY > 23 && iX < 772 && iY < 408)
		return true;

	return CCtrlWindowX::OnLeftButtonDown(iX, iY);
}

bool CWooolStoreWnd::OnRightButtonDown(int iX,int iY)
{
	if(iX > 43 && iY > 23 && iX < 772 && iY < 408)
		return true;

	return CCtrlWindowX::OnRightButtonDown(iX,iY);
}

bool CWooolStoreWnd::OnRightButtonUp(int iX,int iY)
{
	if (m_ItemGroupCtrl.ClickArea(iX, iY) > -1)
	{
		iItemIndex = m_ItemGroupCtrl.iActiveIndex;

		if (g_WooolStoreMgr.QueryInfo(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT) == 1)	// 请求需要的数据
		{// 向服务器请求的是大类消息
			if (iSubIndex != 0)
			{// 向服务器请求的是大类的信息，但是需要的小类不是默认给的第一个小类，则请求自己需要的小类信息
				char szTemp[256] = {0};
				sprintf(szTemp, "%d", ((iMainIndex + 1) * 1000 + (iSubIndex + 1) * 10));
				g_pGameControl->SEND_PT_Store_Info(3, szTemp, strlen(szTemp));
			}
		}

		pItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);

		if(pItem)
		{
			//m_pDisplayText->SetText("   ",FONT_YAHEI,FONTSIZE_SMALL);

			CCtrlMenuWnd* pMenuWnd = g_pControl->GetMenuWnd();
			pMenuWnd->Clear();

			S_POP_MENU_DATA & PopMenuData = pMenuWnd->GetData();
			PopMenuData.fItemHeight = 18.2f;
			PopMenuData.iLeftOffX = PopMenuData.iRightOffX = 3;
			PopMenuData.iTopOffY = 4;
			PopMenuData.iBottomOffY = 0;

			pMenuWnd->AddMenuItem(POP_MENU_ITEM_STORE_BUY,"使用元宝购买");
			pMenuWnd->AddMenuItem(POP_MENU_ITEM_STORE_PRESENT,"绑定元宝购买");

			if(pItem->m_iTotalNum > 1)
				pMenuWnd->AddMenuItem(POP_MENU_ITEM_STORE_SET_QUICK_ITEM,"设置商城快捷键","",false);
			else
				pMenuWnd->AddMenuItem(POP_MENU_ITEM_STORE_SET_QUICK_ITEM,"设置商城快捷键");


			m_bClick = false;
			m_bRBClick = false;//否则MenuWnd获得不到焦点
			pMenuWnd->SetCaller(this);
			pMenuWnd->ShowMenu();
			g_pControl->GetTipWnd()->SetShow(false);
		}

		return true;
	}


	if(iX > 259 && iY > 82 && iX < 647 && iY < 384)
		return true;

	return CCtrlWindowX::OnRightButtonUp(iX,iY);
}
bool CWooolStoreWnd::OnMouseMove(int iX,int iY)
{
	CParserTip *pTip = g_pControl->GetTipWnd();

	pTip->Clear();
	if(g_strChannelName.empty() && iX > 52 && iX < 195 && iY > 450 && iY<468)
	{
		char strTemp[256]="  盛大积分是通过向盛大游戏等应用充值后所获\n"
			"  拥有盛大积分，即可参加丰富礼品兑换、积分\n"
			"  活动、淘金游戏等多彩内容！";

		pTip->AddText(strTemp,0xFFFFFFFF,-1);

		int x = m_iScreenX + iX + 10;
		int y = m_iScreenY + iY + 10;
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);
		return true;
	}
// 	else if(g_dwServerSwitch & SS_CREDIT && iX > 453 && iX < 520 && iY > 423 && iY < 437)
// 	{
// 		char strTemp[256]={0};
// 
// 		if (SELF.GetCreditState() == 0)//未开通信用
// 		{
// 			sprintf(strTemp,"信用额度%d信用点，等值于%.2f元宝",SELF.GetCreditMax(),(float)(SELF.GetCreditMax()) / 100);
// 			pTip->AddText(strTemp,0xFFFFFFFF,-1);
// 			pTip->AddText("您尚未开通信用服务，进入圈圈信用服务可以申请开通",0xFFFFFFFF,-1);
// 		}
// 		else if (SELF.GetCredit() > 0)//有可用信用额度
// 		{
// 			sprintf(strTemp,"可透支额为%d信用点，等值于%.2f元宝",SELF.GetCredit(),(float)(SELF.GetCredit())/100);
// 			pTip->AddText(strTemp,0xFFFFFFFF,-1);
// 		}
// 		else// if (SELF.GetCredit() == 0)
// 		{
// 			if (SELF.GetCreditArrearage() > 0)//有欠款
// 			{
// 				pTip->AddText("可透支额为0信用点，请及时充值还款",0xFFFFFFFF,-1);
// 			}
// 			else
// 			{
// 				pTip->AddText("手机未认证，可用透支额已被冻结，请进入圈圈信用服务进行认证",0xFFFFFFFF,-1);
// 			}
// 		}
// 
// 
// 		int x = m_iScreenX + iX + 10;
// 		int y = m_iScreenY + iY + 10;
// 		pTip->AdjustXY(x,y);
// 		pTip->Move(x,y);
// 		pTip->SetShow(true);
// 		return true;
// 	}
	else if (m_ItemGroupCtrl.ClickArea(iX, iY) > -1)
	{
		iItemIndex = m_ItemGroupCtrl.iActiveIndex;
		pItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);

		if(pItem)
		{
			OnLeftButtonUp(iX,iY);//如果没有物品数据,则请求,否则tips里显示的不对

			char szTemp[128];
			pTip->SetBackTexByID(0xFF000000);
			pTip->AddText(pItem->m_strName.c_str(),TIPS_GOODNAME,-1,0,-1,FONT_DEFAULT,FONTSIZE_MIDDLE);

			int liW = 150 + 40;
			pTip->SetMaxWidth(150 + 40);
			sprintf(szTemp, "%d 元宝", pItem->m_iPrice);
			pTip->AddText(szTemp,TIPS_GOODNAME,-1,0,-1,FONT_DEFAULT,FONTSIZE_MIDDLE);			
			
			pTip->AddPic(1,0x888C7C40,0,-1,0,1,true,XAL_TOPLEFT,-1,10,true,true,0,6,0);

			pTip->AddPic(MAKELONG(pItem->m_dwPreview, PACKAGE_bgitems),-1,20,-1,150,150);

			pTip->AddPic(1,0x888C7C40,0,-1,0,1,true,XAL_TOPLEFT,-1,3,true,true,0,6,0);

			pTip->AddText(pItem->m_strDesc.c_str(), -1, liW,0,-1,FONT_DEFAULT,FONTSIZE_DEFAULT,0,true,XAL_TOPLEFT,1,10);

			int x = m_iScreenX + iX + 10;
			int y = m_iScreenY + iY + 10;
			pTip->AdjustXY(x,y);
			pTip->Move(x,y);
			pTip->SetShow(true);
		}

		return true;
	}
	else if (m_HotGroupCtrl.ClickArea(iX, iY) > -1)
	{// 热区物品被点中
		int iHotClick = m_HotGroupCtrl.iActiveIndex;
		
		CWooolStoreItem* lpItem = g_WooolStoreMgr.GetHotItem(iHotClick);
		if (lpItem && lpItem->m_dwItemID.size() > 0)
		{
			char szTemp[128];
			pTip->SetBackTexByID(0xFF000000);
			pTip->AddText(lpItem->m_strName.c_str(),TIPS_GOODNAME,-1,0,-1,FONT_DEFAULT,FONTSIZE_MIDDLE);

			int liW = 150 + 40;
			pTip->SetMaxWidth(150 + 40);
			sprintf(szTemp, "%d 元宝", lpItem->m_iPrice);
			pTip->AddText(szTemp,TIPS_GOODNAME,-1,0,-1,FONT_DEFAULT,FONTSIZE_MIDDLE);			

			int x = m_iScreenX + iX + 10;
			int y = m_iScreenY + iY + 10;
			pTip->AdjustXY(x,y);
			pTip->Move(x,y);
			pTip->SetShow(true);
		}
		return true;
	}
	else
	{
		pTip->SetShow(false);
	}

	return CCtrlWindowX::OnMouseMove(iX,iY);
}
bool CWooolStoreWnd::OnLeftButtonUp(int iX, int iY)
{
	if(m_bDClicked)
	{
		m_bDClicked = false;
		return CCtrlWindowX::OnLeftButtonUp(iX,iY);
	}

	CWooolStoreMgr& rMgr = g_WooolStoreMgr;

	bool bInPayPage =false;//是否在冲值页面
	if(	m_StoreGroupCtrl.iActiveIndex == StoreIndexGame && m_MainGroupCtrl.iActiveIndex == PAYINDEX)//在冲值页面
		bInPayPage = true;

	bool bHotArea = false;
	int  iCurPage = m_StoreGroupCtrl.iActiveIndex;
	int  iSubGroup = m_SubGroupCtrl.iActiveIndex;

//	if (m_StoreGroupCtrl.ClickArea(iX,iY) > -1)
//	{
//		if(m_StoreGroupCtrl.iActiveIndex == StoreIndexPM  || m_StoreGroupCtrl.iActiveIndex == StoreIndexCS)
//		{
//			if(g_PetBoothData.IsPaimaiClosed()) //判断是否关闭了拍卖行
//			{
//				m_StoreGroupCtrl.iActiveIndex = iCurPage;
//				return true;
//			}
//		}
//		else if(m_StoreGroupCtrl.iActiveIndex == StoreIndexJF )//没有积分商城的页签直接返回
//		{
//			m_StoreGroupCtrl.iActiveIndex = iCurPage;
//			return true;
//		}
//
//		if(m_StoreGroupCtrl.iActiveIndex == iStoreIndex) //没有改变
//			return true;
//
///*		if(m_StoreGroupCtrl.iActiveIndex == StoreIndexRS)
//		{
//			SwitchSubPage(WOOOLSTORE_REAL);
//		}
//		else if(m_StoreGroupCtrl.iActiveIndex == StoreIndexPM)
//		{
//			SwitchSubPage(WOOOLSTORE_PAIMAI);
//		}
//		else */if(m_StoreGroupCtrl.iActiveIndex == StoreIndexFH)
//		{
//			SwitchSubPage(WOOOLSTORE_FENGHONG);
//		}
//		//else if(m_StoreGroupCtrl.iActiveIndex == StoreIndexCS)
//		//{
//		//	g_pControl->Msg(MSG_CTRL_UPLOAD_WND,OPER_CREATE);
//		//	return true;
//		//}
//		else
//		{
//			SwitchSubPage(0);
//		}
//	}
	//已经在易宝商城/拍卖行/积分商城
	//else 
	if (iStoreIndex == StoreIndexRS || iStoreIndex == StoreIndexPM || iStoreIndex == StoreIndexJF || iStoreIndex == StoreIndexFH) 
	{
		return true;
	}
	//else if (m_MainGroupCtrl.ClickArea(iX, iY) > -1)
	//{// 大项改变，子项也变
	//	if(m_MainGroupCtrl.iActiveIndex == iMainIndex)
	//		return true;

	//	if(bInPayPage)
	//		g_WidgetMgr.SetShow(EWT_FIRST,false);

	//	m_SubGroupCtrl.vHotArea.clear();
	//	m_PageCtrl.vHotArea.clear();

	//	m_SubGroupCtrl.iActiveIndex = 0;
	//	m_ItemGroupCtrl.iActiveIndex = 0;
	//	m_PageCtrl.iActiveIndex = 0;

	//	for (int i = 0; i < ICONCOUNT; ++i)
	//		(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();
	//}
	else if (m_SubGroupCtrl.ClickArea(iX, iY) > -1)
	{// 子项改变
		//点击游戏商城的拍卖区
		if(m_SubGroupCtrl.iActiveIndex < pMain->m_vSubGroup.size() && (int)pMain->m_vSubGroup.at(m_SubGroupCtrl.iActiveIndex).m_dwSubGroupID == -1)
		{
			if(g_PetBoothData.IsPaimaiClosed())
			{
				m_SubGroupCtrl.iActiveIndex = iSubGroup;
				return true;
			}
			//SwitchSubPage(WOOOLSTORE_PAIMAI);
		}
		else
		{
			m_PageCtrl.vHotArea.clear();

			m_ItemGroupCtrl.iActiveIndex = 0;
			m_PageCtrl.iActiveIndex = 0;

			for (int i = 0; i < ICONCOUNT; ++i)
				(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();
		}
	}
	else if (m_ItemGroupCtrl.ClickArea(iX, iY) > -1)
	{
		// 选中的物品改变
		if(iMainIndex == HELPINDEX || iMainIndex == PAYINDEX)
			return true;
	}
	else if (m_HotGroupCtrl.ClickArea(iX, iY) > -1)
	{// 热区物品被点中
		int iHotClick = m_HotGroupCtrl.iActiveIndex;

		m_SubGroupCtrl.vHotArea.clear();
		m_PageCtrl.vHotArea.clear();
		for (int i = 0; i < ICONCOUNT; ++i)
			(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();

		CWooolStoreItem* pHotClickedItem = rMgr.GetHotItem(iHotClick);

		if (pHotClickedItem != NULL && pHotClickedItem->m_dwItemID.size() > 0)
		{
			if(bInPayPage)//在冲值页面
				g_WidgetMgr.SetShow(EWT_FIRST,false);

			int iM,iS;//,iI;
			rMgr.GetSubGroupByID(pHotClickedItem->m_dwItemID, iM, iS);

			bHotArea = true;

			// 设置当前的页面索引
			m_MainGroupCtrl.iActiveIndex = iM;
			m_SubGroupCtrl.iActiveIndex = iS;
			m_ItemGroupCtrl.iActiveIndex = 0;//iI % ICONCOUNT;
			m_PageCtrl.iActiveIndex = 0;
			iBuyItemID = (pHotClickedItem->m_dwItemID.c_str());// 利用iBuyItemID保存物品ID，一会用ID推导物品后，这两个值会被更新为正常作用

			if (iM >= 0 && iM < m_TabPage.vSubTabPage[0].vSubTabPage.size())
			{
				S_TabPage *pPage = &(m_TabPage.vSubTabPage[0].vSubTabPage[m_TabPage.vSubTabPage[0].iCurPage]);
				pPage->pTabBtn->SetBackState(BTEX_NONE);
				m_TabPage.vSubTabPage[0].iCurPage = iM;
				pPage = &(m_TabPage.vSubTabPage[0].vSubTabPage[m_TabPage.vSubTabPage[0].iCurPage]);
				pPage->pTabBtn->SetBackState(BTEX_PUSHED);
			}
		}
	}
	else if (m_PageCtrl.ClickArea(iX, iY) > -1)
	{// 只有有物品数据才会有页控件

		m_ItemGroupCtrl.iActiveIndex = 0;
	}
	else if (iX != -1 || iY != -1)///////(-1,-1)是因为点中按钮时强行置的焦点
	{
		return true;	// 点到其它区域，直接跳出
	}

	iStoreIndex = m_StoreGroupCtrl.iActiveIndex;
	iMainIndex = m_MainGroupCtrl.iActiveIndex;
	iSubIndex = m_SubGroupCtrl.iActiveIndex;
	iItemIndex = m_ItemGroupCtrl.iActiveIndex;

	if (bHotArea)
		iOffset = -1;
	else
		iOffset = m_PageCtrl.iActiveIndex;

	if (g_WooolStoreMgr.QueryInfo(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT) == 1)	// 请求需要的数据
	{// 向服务器请求的是大类消息
		if (iSubIndex != 0)
		{// 向服务器请求的是大类的信息，但是需要的小类不是默认给的第一个小类，则请求自己需要的小类信息
			char szTemp[256] = {0};
			sprintf(szTemp, "%d", ((iMainIndex + 1) * 1000 + (iSubIndex + 1) * 10));
			g_pGameControl->SEND_PT_Store_Info(3, szTemp, strlen(szTemp));
		}
	}

	if(iStoreIndex == StoreIndexGame)
	{
		pMain = rMgr.GetMainGroupByIndex(iMainIndex);
		pSub = rMgr.GetSubGroupByIndex(iMainIndex, iSubIndex);
		pItem = rMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);
	}
	else
	{
		pMain = NULL;
		pSub = NULL;
		pItem = NULL;
	}

	////////////////直接清空文字说明框的内容，不作是否改变的相关判断，方便
	//m_pDisplayText->SetText("   ",FONT_YAHEI,FONTSIZE_SMALL);
	//if (m_pScroll != NULL)
	//{
	//	m_pScroll->SetPos(0);
	//	m_pScroll->SetEnable(false);
	//}
	iTextPos = 0;

	for (int i = 0; i < ICONCOUNT; ++i)
	{
		if (m_pButtonBuy[i] != NULL && m_pButtonSend[i] != NULL)
		{
			if (iStoreIndex != StoreIndexGame ||
				iMainIndex == HELPINDEX ||
				iMainIndex == PAYINDEX)	// 帮助版面，按钮控件不显示
			{
				m_pButtonBuy[i]->SetShow(false);
				m_pButtonBuy[i]->SetEnable(false);
				m_pButtonSend[i]->SetShow(false);
				m_pButtonSend[i]->SetEnable(false);
			}
			else
			{
				m_pButtonBuy[i]->SetShow(true);
				m_pButtonBuy[i]->SetEnable(true);
				m_pButtonSend[i]->SetShow(true);
				m_pButtonSend[i]->SetEnable(true);
			}
		}
	}

	return true;

	//return CCtrlWindowX::OnLeftButtonUp(iX, iY);
}

bool CWooolStoreWnd::OnLeftButtonDClick(int iX, int iY)
{
	m_bDClicked = true;

	int iHotMark = 0;
	bool bHotArea = false;
	int  iCurPage = m_StoreGroupCtrl.iActiveIndex;

	if (m_ItemGroupCtrl.ClickArea(iX, iY) > -1)
	{
		// 选中的物品改变
		if(iMainIndex == HELPINDEX || iMainIndex == PAYINDEX)
			return true;

		if(iStoreIndex == StoreIndexGame)
		{
			pItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);
			if(pItem) iHotMark = pItem->m_dwItemLooks;
		}

		if(pItem && pItem->m_iTotalNum == 1)
		{
			CQuickItem& qitem = g_WooolStoreMgr.GetTmpQuickItem();
			qitem.strItemID = pItem->m_dwItemID;
			qitem.iItemLooks = pItem->m_dwItemLooks;
			qitem.strName = pItem->m_strName;

			g_pControl->Msg(MSG_CTRL_QUICKCONSUMESET_WND,OPER_CREATE);
			return true;
		}
	}
	else if (m_HotGroupCtrl.ClickArea(iX, iY) > -1)
	{// 热区物品被点中
		int iHotClick = m_HotGroupCtrl.iActiveIndex;
		m_SubGroupCtrl.vHotArea.clear();
		m_PageCtrl.vHotArea.clear();
		for (int i = 0; i < ICONCOUNT; ++i)
			(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();

		CWooolStoreItem* pHotClickedItem = g_WooolStoreMgr.GetHotItem(iHotClick);
		if(pHotClickedItem) iHotMark = pHotClickedItem->m_dwItemLooks;
	}

	return true;
}

bool CWooolStoreWnd::Msg(DWORD dwMsg, DWORD dwData, CControl* pControl)
{
	CCharacter *pOldSelf = g_pSelf;
	g_pSelf = &ORIGINALSELF;

	CWooolStoreItem* pFocusItem = NULL;	// 点中的对应的item的指针，默认为空的
	int iClickWhichOne = 0;
	switch (dwMsg)
	{
	case MSG_CTRL_WOOOL_STORE_WND:
		{
			if (dwData == 1)
			{
				int X,Y;
				g_pControl->GetMouseXY(X,Y);
				X -= m_iScreenX;
				Y -= m_iScreenY;
				OnMouseMove(X,Y);
			}
			return true;
		}
		break;
	case MSG_CTRL_TABPAGE_TABCHANGE:
		{
			if (pControl)
			{
				S_TabPage * pParentTabpage = (S_TabPage * )pControl;

				int iX = 0,iY = 0;
				g_pControl->GetMouseXY(iX,iY);

				if (pParentTabpage->iCurPage == dwData)
				{
					g_pSelf = pOldSelf;
					return true;
				}

				if (pParentTabpage == &m_TabPage && dwData == 0)
				{
					m_TabPage.vSubTabPage[0].iCurPage = 0;
				}

				SwitchToPage(dwData,pParentTabpage);

				if (pParentTabpage == &m_TabPage)
				{
					if (dwData == 1)
					{
						m_StoreGroupCtrl.iActiveIndex = StoreIndexFH;
					}
					else
					{
						m_StoreGroupCtrl.iActiveIndex = StoreIndexGame;
					}

					if(m_StoreGroupCtrl.iActiveIndex == iStoreIndex) //没有改变
					{
						g_pSelf = pOldSelf;
						return true;
					}

					if(m_StoreGroupCtrl.iActiveIndex == StoreIndexFH)
					{
						SwitchSubPage(WOOOLSTORE_FENGHONG);
					}
					else
					{
						SwitchSubPage(0);
					}
				}
				else if (pParentTabpage == &(m_TabPage.vSubTabPage[0]))
				{
					m_MainGroupCtrl.iActiveIndex = (int)dwData;

					// 大项改变，子项也变
					if(m_MainGroupCtrl.iActiveIndex == iMainIndex)
					{
						g_pSelf = pOldSelf;
						return true;
					}

					bool bInPayPage =false;//是否在冲值页面
					if(	m_StoreGroupCtrl.iActiveIndex == StoreIndexGame && m_MainGroupCtrl.iActiveIndex == PAYINDEX)//在冲值页面
						bInPayPage = true;
					if(bInPayPage)
						g_WidgetMgr.SetShow(EWT_FIRST,false);

					m_SubGroupCtrl.vHotArea.clear();
					m_PageCtrl.vHotArea.clear();

					m_SubGroupCtrl.iActiveIndex = 0;
					m_ItemGroupCtrl.iActiveIndex = 0;
					m_PageCtrl.iActiveIndex = 0;

					for (int i = 0; i < ICONCOUNT; ++i)
						(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();

				}

				OnLeftButtonUp(-1,-1);

				g_pSelf = pOldSelf;
				return true;
			}
		}
		break;
		

	case MSG_CTRL_BUTTON_CLICK:
		if (pControl == m_pLeaveStore) 
		{// 离开商店
			CloseWindow();
		}
		else if (pControl == m_pCreditBtn)//信用
		{
			ShellExecute(NULL,"open","iexplore.exe","http://home.woool.sdo.com/project/200906_xinyong/",NULL,SW_SHOW);
		}
		else if (pControl == m_pVipMemberBtn)
		{
			g_pControl->PopupWindow(MSG_CTRL_VIPCARD_WND,OPER_UPDATE);
		}
		else if (pControl == m_pAddYuanBao)
		{
			// 增加点券
			//SwitchSubPage(WOOOLSTORE_PAY);

			if (!g_strChannelName.empty())
			{
				char szUrl[512] = {0};
				sprintf(szUrl, "%s?channelname=%s&userid=%s&userfrom=%s&area=%d"
					, g_pStreamMgr->GetWebsite("ChannelPayURL","http://act.ws.sdo.com/pay/pay.asp")
					, g_strChannelName.c_str(), g_Login.GetThirdUserID().c_str(), g_Login.GetThirdChannel().c_str(), g_Login.GetInnerAreaNo());
				ShellExecute(NULL,"open",szUrl,NULL,NULL,SW_SHOW);
				//OpenWebUrl(szUrl,true);				
			}
			else if(g_pSDOAInterface)
			{
				if(SDOA_FALSE == g_pSDOAInterface->OpenWidget("sdoNewPay"))
				{
					//m_TabPage.iCurPage = 1;
					//m_TabPage.vSubTabPage[StoreIndexGame].iCurPage = 6;
					//SwitchToPage(StoreIndexGame,&m_TabPage);
					//SwitchSubPage(WOOOLSTORE_PAY);

					char szDefalutUrl[128] = {0};
					sprintf(szDefalutUrl,"%s%u","https://pay.sdo.com/Deposit/HomePage.aspx?sel_game=",WS_GAME_ID);

					string url = g_pStreamMgr->GetWebsite("PayURL",szDefalutUrl);
					ShellExecute(NULL,"open","iexplore.exe",url.c_str(),NULL,SW_SHOW);
					//OpenWebUrl(url.c_str(),false);
				}
			}
			else
			{			
				//m_TabPage.iCurPage = 1;
				//m_TabPage.vSubTabPage[StoreIndexGame].iCurPage = 6;
				//SwitchToPage(StoreIndexGame,&m_TabPage);
				//SwitchSubPage(WOOOLSTORE_PAY);

				char szDefalutUrl[128] = {0};
				sprintf(szDefalutUrl,"%s%u","https://pay.sdo.com/Deposit/HomePage.aspx?sel_game=",WS_GAME_ID);

				string url = g_pStreamMgr->GetWebsite("PayURL",szDefalutUrl);
				ShellExecute(NULL,"open","iexplore.exe",url.c_str(),NULL,SW_SHOW);
				//OpenWebUrl(url.c_str(),false);
			}

		}
		else if (pControl == m_pActAward)
		{
			//发送领奖协议
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-7),"@main");
			CloseWindow();
		}
		//else if(pControl == m_pSNDAJiFen)
		//{
		//	SwitchSubPage(WOOOLSTORE_JIFEN);
		//}
		else 
		{	
			iClickWhichOne = FindFocusButton(pControl, &pFocusItem);

			// 点中购买或者赠送按钮
			if (pFocusItem != NULL)
			{
				if (iClickWhichOne == 1) 
				{// 点中购买
					OnBuy(*pFocusItem, 2);
				}
				else if (iClickWhichOne == -1)
				{// 点中赠送
					//OnSend(*pFocusItem);
					OnBuy(*pFocusItem, 1);
				}
			}
			else	// 其余按钮交由父类的Msg处理
				break;
		}

		g_pSelf = pOldSelf;
		return true;// 处理了对应消息，返回
	case MSG_CTRL_POP_MENU://弹出菜单
		{
			if(dwData == POP_MENU_ITEM_STORE_BUY)
				OnPopMenuBuy();
			else if(dwData == POP_MENU_ITEM_STORE_PRESENT)
				OnPopMenuPresent();
			else if(dwData == POP_MENU_ITEM_STORE_SET_QUICK_ITEM)
				OnPopMenuSetQuicItem();

			g_pSelf = pOldSelf;
			return true;
		}
		break;
	case MSG_CTRL_WOOOLSTORE_BUYCONFIRM:
		BuyItem(dwData,pControl != NULL);
		break;
	//case MSG_CTRL_REQUIRE_GUID:
	//	{
	//		if (g_WooolStoreMgr.GetWooolStorePage() == WOOOLSTORE_REAL)
	//			g_WidgetMgr.LoginRealStore();

	//	    g_pSelf = pOldSelf;
	//		return true;
	//	}
	//	break;
	default:
		break;
	}

	bool bRtn = CCtrlWindowX::Msg(dwMsg, dwData, pControl);

	g_pSelf = pOldSelf;

	return bRtn;
}

// 返回击中的按钮, 返回1表示点中购买按钮，-1表示赠送按钮，0表示没有按钮被点中， ppFocusItem表示对应的item，没点中时为NULL
int CWooolStoreWnd::FindFocusButton(IN CControl* pControl, OUT CWooolStoreItem** ppFocusItem)
{
	*ppFocusItem = NULL;
	int iReturn = 0;

	if (!pControl)
	{
		return iReturn;
	}

	for (int i = 0; i < ICONCOUNT; ++i)
	{
		if (pControl == m_pButtonSend[i] || pControl == m_pButtonBuy[i]) 
		{
			*ppFocusItem = g_WooolStoreMgr.GetItemByIndex(iMainIndex, iSubIndex, i + iOffset * ICONCOUNT);
			if (*ppFocusItem != NULL)
			{
				if (pControl == m_pButtonSend[i])
					iReturn = -1;
				else if (pControl == m_pButtonBuy[i])
					iReturn = 1;

				if (m_ItemGroupCtrl.iActiveIndex != i)
				{
					iReturn = 0;	// 先置焦点,有这一句，置焦点但不购买，没有这一句置焦点，并且购买
					m_ItemGroupCtrl.iActiveIndex = i;
					OnLeftButtonUp(-1, -1);
				}
			}

			return (iReturn);
		}
	}

	return 0;

}

void CWooolStoreWnd::OnBuy(CWooolStoreItem& FocusItem, DWORD dwBind)
{

	char szTemp[256] = {0};

	if(FocusItem.m_iRealGood == 1)//该物品为实物,需要填写邮寄信息购买
	{
		if (SELF.GetYuanBao() < FocusItem.m_iPrice)
		{
			g_MsgBoxMgr.PopTagAlert(STRING_NOT_ENOUGH_YUANBAO);
		}
		else
		{
			iBuyItemID = FocusItem.m_dwItemID;

			char strPrice[128]="";
			char strItem[128]="";
			sprintf(strPrice,"%d", FocusItem.m_iPrice);
			sprintf(strItem,"%d", FocusItem.m_dwItemLooks);
			g_WooolStoreMgr.GetBuyData()->clear();
			g_WooolStoreMgr.GetBuyData()->m_strItemID = FocusItem.m_dwItemID;
			g_WooolStoreMgr.GetBuyData()->m_strItemPrice = strPrice;
			g_WooolStoreMgr.GetBuyData()->m_strItemLooks = strItem;
			g_WooolStoreMgr.GetBuyData()->m_strItemCount = "1";//默认为一件物品
			g_WooolStoreMgr.GetBuyData()->m_strItemReal = "1";//设置为实物
			g_WooolStoreMgr.GetBuyData()->m_strOriginUser = SELF.GetName();

			g_pControl->Msg(MSG_CTRL_WOOOLSTORE_BUYCOUNT_WND,OPER_CREATE);  //弹出购买数量确定窗口

		}
		return;
	}
	//--------------------------------------------
	if (FocusItem.m_iTotalNum > GetLeftGrid())
	{
		g_MsgBoxMgr.PopSimpleAlert(STRING_PACKAGE_FULL);		
	}
	//else if (SELF.GetYuanBao() < FocusItem.m_iPrice)
	//{
	//	g_MsgBoxMgr.PopTagAlert(STRING_NOT_ENOUGH_YUANBAO);
	//}
	else
	{
		//确定购买物品的ID
		iBuyItemID = FocusItem.m_dwItemID;

		char strPrice[128]="";
		char strItem[128]="";

		sprintf(strPrice,"%d", FocusItem.m_iPrice);
		sprintf(strItem,"%d", FocusItem.m_dwItemLooks);
		g_WooolStoreMgr.GetBuyData()->clear();
		g_WooolStoreMgr.GetBuyData()->m_strItemID = FocusItem.m_dwItemID;
		g_WooolStoreMgr.GetBuyData()->m_strItemPrice = strPrice;
		g_WooolStoreMgr.GetBuyData()->m_strItemLooks = strItem;
		g_WooolStoreMgr.GetBuyData()->m_strItemCount = "1";//默认为一件物品
		g_WooolStoreMgr.GetBuyData()->m_strItemReal = "0";//设置为虚拟道具
		g_WooolStoreMgr.GetBuyData()->m_strOriginUser = SELF.GetName();


		g_pControl->Msg(MSG_CTRL_WOOOLSTORE_BUYCOUNT_WND,OPER_CREATE, (CControl*)dwBind);  //弹出购买数量确定窗口
	}
}

void CWooolStoreWnd::OnSend(CWooolStoreItem& FocusItem)
{
	//物品ID 价格 买主  赠送对象 留言
	if(FocusItem.m_iRealGood == 1)//该物品为实物,需要填写邮寄信息购买
	{
		char temp[256];
		sprintf(temp,"如果您想将%s赠送给好友，请直接点击购买，\n并填写您好友的邮寄信息。",FocusItem.m_strName.c_str());
		g_MsgBoxMgr.PopSimpleAlert(temp);
		return;
	}

	if (SELF.GetYuanBao() < FocusItem.m_iPrice)
	{
		g_MsgBoxMgr.PopTagAlert(STRING_NOT_ENOUGH_YUANBAO);
	}
	else
	{
		g_pControl->Msg(MSG_CTRL_WOOOLSTORE_SEND_WND,OPER_CREATE);  //弹出留言框
		//g_pControl->Msg(MSG_CTRL_WOOOLSTORE_SEND_FOUCUS,0,NULL);//设置赠送的确定框的焦点

		char strPrice[128]="";
		char strItem[128]="";
		sprintf(strPrice,"%d", FocusItem.m_iPrice);
		sprintf(strItem,"%d", FocusItem.m_dwItemLooks);
		g_WooolStoreMgr.GetSendData()->m_strItemID = FocusItem.m_dwItemID;
		g_WooolStoreMgr.GetSendData()->m_strItemPrice = strPrice;
		g_WooolStoreMgr.GetSendData()->m_strItemLooks = strItem;
		g_WooolStoreMgr.GetSendData()->m_strOriginUser = SELF.GetName();		
	}

}

int CWooolStoreWnd::GetLeftGrid()
{
	int iReturn  = 0;
	for(int i = 0; i < MAX_PACKAGE_ELEMENT - 6; ++i)
	{
		if(SELF.GetPackageGood(i).GetID() == 0)
			++iReturn;
	}
	return iReturn;
}

void CWooolStoreWnd::CheckRecv()
{
	// 判断包裹容量 如果包裹放不下 提示清理包裹的窗口
	// 如果放得下 弹出接收赠品的窗口
	if(GetTickCount()-m_dwLastCheckTime>5000)
	{
		m_dwLastCheckTime = GetTickCount();

		for(int i=0;i<10;i++)
		{
			CWoolStoreSendData* pRecvData = g_WooolStoreMgr.GetRecvData(i);

			if(pRecvData->m_strItemLooks.length()>1 &&  pRecvData->m_strItemName.length()>1)
			{
				if(GetLeftGrid() < atoi (pRecvData->m_strItemGrid.c_str() ) )
				{
					g_pControl->Msg(MSG_CTRL_WOOOLSTORE_RECV_INFO_WND,OPER_CREATE);//弹出清包裹通知窗口
				}
				else
				{
					g_pControl->Msg(MSG_CTRL_WOOOLSTORE_RECV_OK_WND,OPER_CREATE);//弹出接收赠品窗口
				}
				return;
			}
		}
	}
}

//切换到IE页面
void CWooolStoreWnd::SwitchSubPage(int iOpenType)
{
	g_WooolStoreMgr.SetWooolStorePage(iOpenType);

	//if(iOpenType == WOOOLSTORE_PAIMAI)
	//{
	//	PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_C2C,HOSTAPP_OPEN);
	//}
	//else
	//{
	//	PostMessage(g_hWnd,MSG_IGW_WIDGET,APPID_C2C,HOSTAPP_CLOSE);
	//}

	//处理页面上控件的显示和隐藏
	if(iOpenType > 0)
	{
		//m_pArrowUp->SetShow(false);
		//m_pArrowUp->SetEnable(false);
		//m_pArrowDown->SetShow(false);
		//m_pArrowDown->SetEnable(false);
		//m_pScroll->SetShow(false);					// 滚动条控件
		//m_pScroll->SetEnable(false);

		for (int i = 0; i < HOTCOUNT; ++i)
		{
			if(iOpenType != WOOOLSTORE_PAY)
			{
				(m_HotGroupCtrl.vHotArea.operator [](i)).ClearAppend();
			}
			else
			{
				// 创建热卖区控件。并绑定指定数据到控件
				(m_HotGroupCtrl.vHotArea.operator [](i)).SetAppend(g_WooolStoreMgr.GetHotItem(i));
			}
		}

		//清除掉面板上的控件
		m_SubGroupCtrl.vHotArea.clear();
		m_PageCtrl.vHotArea.clear();

		m_SubGroupCtrl.iActiveIndex = 0;
		m_ItemGroupCtrl.iActiveIndex = 0;
		m_PageCtrl.iActiveIndex = 0;

		//清除掉物品列表
		for (int i = 0; i < ICONCOUNT; ++i)
			(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();

		iStoreIndex = m_StoreGroupCtrl.iActiveIndex;
		iMainIndex = m_MainGroupCtrl.iActiveIndex;
		iSubIndex = m_SubGroupCtrl.iActiveIndex;
		iItemIndex = m_ItemGroupCtrl.iActiveIndex;

		iOffset = m_PageCtrl.iActiveIndex;

		pMain = NULL;
		pSub = NULL;
		pItem = NULL;

		// 直接清空文字说明框的内容，不作是否改变的相关判断，方便
		//m_pDisplayText->SetText("   ",FONT_YAHEI,FONTSIZE_SMALL);
		//m_pDisplayText->SetShow(false);
		//m_pDisplayText->SetEnable(false);
		iTextPos = 0;

		for (int i = 0; i < ICONCOUNT; ++i)
		{
			if (m_pButtonBuy[i] != NULL && m_pButtonSend[i] != NULL)
			{
				m_pButtonBuy[i]->SetShow(false);
				m_pButtonBuy[i]->SetEnable(false);
				m_pButtonSend[i]->SetShow(false);
				m_pButtonSend[i]->SetEnable(false);
			}
		}

		switch(iOpenType)
		{
		//case WOOOLSTORE_REAL:
		//	{
		//		m_iWidgetX = 36,m_iWidgetY = 15;
		//		g_WidgetMgr.LoginRealStore();
		//		g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY,750,400);
		//		break;
		//	}
		//case WOOOLSTORE_PAIMAI:
		//	{
		//		m_StoreGroupCtrl.iActiveIndex = iStoreIndex = StoreIndexPM;//强行设置

		//		m_iWidgetX = 36,m_iWidgetY = 15;
		//		g_WidgetMgr.LoginPaiMai();
		//		g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY,750,400);
		//		break;
		//	}
		//case WOOOLSTORE_JIFEN:
		//	{
		//		m_StoreGroupCtrl.iActiveIndex = iStoreIndex = StoreIndexJF;//强行设置

		//		m_iWidgetX = 36,m_iWidgetY = 15;
		//		g_WidgetMgr.LoginJiFengStore();
		//		g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY,750,400);
		//		break;
		//	}
		case WOOOLSTORE_PAY:
			{
				m_StoreGroupCtrl.iActiveIndex = iStoreIndex = StoreIndexGame;
				m_MainGroupCtrl.iActiveIndex = iMainIndex = PAYINDEX;

				m_iWidgetX = 272,m_iWidgetY = 63;
				g_WidgetMgr.LoginPayServer();
				g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY,495,343);
				break;
			}
		case WOOOLSTORE_FENGHONG:
			{
				m_StoreGroupCtrl.iActiveIndex = iStoreIndex = StoreIndexFH;//强行设置

				m_iWidgetX = 42,m_iWidgetY = 22;
				g_WidgetMgr.LoginFengHongStore();
				g_WidgetMgr.MoveWindow(EWT_FIRST,m_iScreenX+m_iWidgetX,m_iScreenY+m_iWidgetY,750,400);
				break;
			}
		}
	}
	else
	{
		g_WidgetMgr.SetShow(EWT_FIRST,false);
		::SetFocus(g_hWnd);

		m_SubGroupCtrl.vHotArea.clear();
		m_PageCtrl.vHotArea.clear();

		m_MainGroupCtrl.iActiveIndex = 0;
		m_SubGroupCtrl.iActiveIndex = 0;
		m_ItemGroupCtrl.iActiveIndex = 0;
		m_PageCtrl.iActiveIndex = 0;
		//m_pDisplayText->SetShow(true);
		//m_pDisplayText->SetEnable(true);

		//物品栏重新生成
		for (int i = 0; i < ICONCOUNT; ++i)
			(m_ItemGroupCtrl.vHotArea.operator [](i)).ClearAppend();

		for (int i = 0; i < HOTCOUNT; ++i)
		{
			// 创建热卖区控件。并绑定指定数据到控件
			(m_HotGroupCtrl.vHotArea.operator [](i)).SetAppend(g_WooolStoreMgr.GetHotItem(i));
		}

		//m_pArrowUp->SetShow(true);
		//m_pArrowUp->SetEnable(true);
		//m_pArrowDown->SetShow(true);
		//m_pArrowDown->SetEnable(true);
		//m_pScroll->SetShow(true);					// 滚动条控件
		//m_pScroll->SetEnable(true);
	}
}

void CWooolStoreWnd::BuyItem(int i,bool bBind)
{
	//if(bBind)
	//{
	//	char szTemp[128];
	//	sprintf(szTemp, "%d", i);	
	//	iBuyItemID.append(1,'&');
	//	iBuyItemID +=szTemp;

	//	g_pGameControl->SEND_PT_Store_Info(10, iBuyItemID.c_str(), iBuyItemID.size());
	//}
	//else
	{
		//g_pGameControl->SEND_PT_Store_Info(5, iBuyItemID.c_str(), iBuyItemID.size());

		CQuickItem qitem;
		qitem.strItemID = iBuyItemID;
		if (bBind)
		{
			qitem.iBuyType = 1;
		}

		g_WooolStoreMgr.BuyQuickItem(qitem,false,i,false,false);
	}

	//更新购买物品的信息 比如限卖物品的剩余数量
	if(pItem->m_iLimitCount > 0)
		g_pGameControl->SEND_PT_Store_Info(4, pItem->m_dwItemID.c_str(), pItem->m_dwItemID.size());
}

//设置游戏商城的大类、子类、页
void CWooolStoreWnd::SetGameStoreGroup()
{
	COpenStorePara  &OpenStorePara = g_WooolStoreMgr.GetOpenStorePara();
	if(OpenStorePara.iMainGroup < 0 || OpenStorePara.iSubGroup < 0 )
		return;

	CWooolStoreMgr& rMgr = g_WooolStoreMgr;
	// 设置当前的页面索引
	m_MainGroupCtrl.iActiveIndex = OpenStorePara.iMainGroup;
	m_SubGroupCtrl.iActiveIndex = OpenStorePara.iSubGroup;
	m_ItemGroupCtrl.iActiveIndex = OpenStorePara.iIndex%ICONCOUNT;
	m_PageCtrl.iActiveIndex = OpenStorePara.iIndex/ICONCOUNT;

	iStoreIndex = m_StoreGroupCtrl.iActiveIndex;
	iMainIndex = m_MainGroupCtrl.iActiveIndex;
	iSubIndex = m_SubGroupCtrl.iActiveIndex;
	iItemIndex = m_ItemGroupCtrl.iActiveIndex;

	iOffset = m_PageCtrl.iActiveIndex;

	pMain = rMgr.GetMainGroupByIndex(iMainIndex);
	pSub = rMgr.GetSubGroupByIndex(iMainIndex, iSubIndex);
	pItem = rMgr.GetItemByIndex(iMainIndex, iSubIndex, iItemIndex + iOffset * ICONCOUNT);

	OpenStorePara.id = 0;
}

void CWooolStoreWnd::OnPopMenuBuy()
{
	// 点中购买或者赠送按钮
	if (pItem)
	{
		OnBuy(*pItem, 2);
	}

}
void CWooolStoreWnd::OnPopMenuPresent()
{
	// 点中购买或者赠送按钮
	if (pItem)
	{
		//OnSend(*pItem);
		OnBuy(*pItem, 1);
	}
}
void CWooolStoreWnd::OnPopMenuSetQuicItem()
{
	if(pItem && pItem->m_iTotalNum == 1)
	{
		CQuickItem& qitem = g_WooolStoreMgr.GetTmpQuickItem();
		qitem.strItemID = pItem->m_dwItemID;
		qitem.iItemLooks = pItem->m_dwItemLooks;
		qitem.strName = pItem->m_strName;

		g_pControl->Msg(MSG_CTRL_QUICKCONSUMESET_WND,OPER_CREATE);
	}
}

void  CWooolStoreWnd::OnSetFocus()
{
	g_WidgetMgr.SetFocus(EWT_FIRST,true);
}

void CWooolStoreWnd::OnKillFocus()
{
	g_WidgetMgr.SetFocus(EWT_FIRST,false);
}

bool CWooolStoreWnd::IsInControl(int iX,int iY)
{
	//if (iX < 22)
	//{
	//	return false;
	//}

	return CCtrlWindowX::IsInControl(iX,iY);
}
