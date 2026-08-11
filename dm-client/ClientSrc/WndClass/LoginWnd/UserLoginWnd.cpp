#include "UserLoginWnd.h"
#include "GameData/GameGlobal.h"
#include "Global/Interface/StreamInterface.h"
#include "GameControl/GameControl.h"
#include "Global/StringUtil.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/GameData.h"
#include "GameData/LoginData.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/ConfigData.h"
#include "BaseClass/Control/CtrlScroll.h"
#include "GameClient/SDOAInterface.h"
#include "GameClient/OpenLoginClient.h"

#include <Shellapi.h>

INIT_WND_POS(CUserLoginWnd,POS_AUTO,POS_AUTO)

#define COLOR_BTN_NORMAL_ULW	0xFFB47D3D
#define COLOR_BTN_MOUSEON_ULW	0xFFFF6600
#define COLOR_BTN_PRESS_ULW		0xFF5AA0A0
#define COLOR_BTN_DISABLE_ULW	0xFF646464


CUserLoginWnd::CUserLoginWnd(void)
{
	m_bDisableEscape = true;
	m_isVaildShow = true;
	m_bInheritScaleFromParent = false;

	m_pEditUserID			= NULL;
	m_pEditPassword			= NULL;
	m_pSubmit				= NULL;
	m_pCancel				= NULL;
	m_pNewUser				= NULL;
	m_pChangePassword		= NULL;
	m_pHelp		= NULL;
	m_bPreDownLoadTex = false;
	g_pAudio->StopAll();
	if (g_Config.GetBkMusicOn())
	{
		g_pAudio->PlayMusic(EAT_BKMUSIC, 8, true); // 1002.ogg
	}
	m_iIndex = 1657;

	//if(g_pStreamMgr->GetGameInt("Area") >= 88 || g_iParamGameType != 0)
	//{
	//	m_iIndex = 16701;
	//	m_iPages = 1;
	//}
	//else
	//{
	//	m_iPages = 2;
	//	m_vIndex.push_back(16703);
	//	m_vIndex.push_back(16702);
	//}

	m_uStyle &= ~CTRL_STYLE_TRANS;

}

CUserLoginWnd::~CUserLoginWnd(void)
{
	SAFE_DELETE(g_pOpenLoginClient);

    g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);
	if (g_Login.IsSDOALogin() && g_pSDOAInterface && g_pSDOAInterface->IsShowUserLoginDialog())
	{
		g_pSDOAInterface->SetShowUserLoginDialog(FALSE);			
	}
}


void CUserLoginWnd::OnCreate()
{
	//大退到登录界面都会退出游戏
	if (g_Login.GetLoginInExpType() > 0 && g_Login.IsInLoginInExp())
	{
		PostMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}

	SetMask(1657);

	if (g_Login.GetLoginInExpType() > 0)
	{
		m_isVaildShow = false;

		if (g_Login.GetLoginInExpType() == 1)
		{
			g_pGameControl->SEND_EXPLogin_Req();
		}
		else if (g_Login.GetLoginInExpType() == 2)
		{
			g_pGameControl->SEND_SGILogin_Req(g_Login.GetLoginInExp2AuthCode().c_str());
		}
	}
	else if (!g_pOpenLoginClient)
	{
		if (!g_strChannelName.empty())
		{
			g_pOpenLoginClient = new COpenLoginClient();
			if (!g_pOpenLoginClient->Open(g_hWnd, g_pGfx->GetD3DDev()))
			{
				SAFE_DELETE(g_pOpenLoginClient);
			}
			else
			{
				g_pOpenLoginClient->ResetWndPos(g_pGfx->GetWidth(), g_pGfx->GetHeight());
				m_isVaildShow = false;
			}
		}
	}

	if (m_isVaildShow)
	{
		if (g_pSDOAInterface && !g_pSDOAInterface->IsShowUserLoginDialog() && g_pSDOAInterface->GetSDOAApp())
		{
			if (g_Login.IsSDOALogin())
			{
				g_pSDOAInterface->GetSDOAApp()->ShowLoginDialog(CSDOAInterface::OnLogin,0,0);
				g_pSDOAInterface->SetShowUserLoginDialog(TRUE);
			}
		}
	}
	else
	{
		if (g_pSDOAInterface && g_pSDOAInterface->IsShowUserLoginDialog())
			g_pSDOAInterface->SetShowUserLoginDialog(FALSE);
	}


	m_pEditUserID = NULL;            // 用户id
	m_pEditPassword = NULL;	        // 用户密码
	m_pSubmit = NULL;				// 提交按钮	
	m_pCancel = NULL;				// 取消按钮	
	m_pNewUser = NULL;				// 新用户按钮
	m_pChangePassword = NULL;        // 修改密码按钮
	m_pHelp = NULL;		// 帮助按钮
	m_pBindDynamicPassword = NULL;   // 绑定密宝
	m_pTakeSafeCard = NULL;

	//CCtrlScroll *pScroll = new CCtrlScroll();
	//AddControl(pScroll);
	//pScroll->CreateEx(this,15,40,40,200);
	//pScroll->SetRange(10000, 20);

	//如果登录的是浩方竞技平台,大退到登录界面都会退出游戏
	if (g_Login.IsInHaoFangJingJi() && g_Login.GetAutoLoginInType() != 5)
	{
		PostMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	

	// 设置关闭按钮
	m_pCloseButton = new CCtrlButton;
	AddControl(m_pCloseButton);
	m_pCloseButton->CreateEx(this,330,39,66,67,68);

	//if(m_iPages >= 2)
	//{
	//	SetPageRect(PAGE_GAMEACCOUNT,38,45,158,66);
	//	SetPageRect(PAGE_PTACCOUNT,158,44,274,63);
	//}


	//公共的控件
	//m_pNewUser = new CCtrlButton;
	//AddControl(m_pNewUser);
	//m_pNewUser->CreateEx(this,76,285,1662,1663,1663,1663);
	//m_pNewUser->SetText("我要注册",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,12,DTF_BlackFrame,FONT_YAHEI);

	//m_pHelp = new CCtrlButton;
	//AddControl(m_pHelp);
	//m_pHelp->CreateEx(this,164,285,1662,1663,1663,1663);
	//m_pHelp->SetText("客服帮助",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,12,DTF_BlackFrame,FONT_YAHEI);


	//m_pChangePassword = new CCtrlButton;
	//AddControl(m_pChangePassword);
	//m_pChangePassword->CreateEx(this,164,313,1662,1663,1663,1663);
	//m_pChangePassword->SetText("密码服务",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,12,DTF_BlackFrame,FONT_YAHEI);

	//m_pBindDynamicPassword = new CCtrlButton;
	//AddControl(m_pBindDynamicPassword);
	//m_pBindDynamicPassword->CreateEx(this,75,313,1662,1663,1663,1663);
	//m_pBindDynamicPassword->SetText("盛大密宝",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,12,DTF_BlackFrame,FONT_YAHEI);

	//m_pTakeSafeCard = new CCtrlButton;
	//AddControl(m_pTakeSafeCard);
	//m_pTakeSafeCard->CreateEx(this,251,285,1662,1663,1663,1663);
	//m_pTakeSafeCard->SetText("领安全卡",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,12,DTF_BlackFrame,FONT_YAHEI);


	if (!g_Login.IsSDOALogin())//不是SDOA登录
	{
		m_pEditUserID = new CCtrlEdit;
		AddControl(m_pEditUserID);
		m_pEditUserID->Create(this,FONTSIZE_DEFAULT,0xFFFFFFFF,157,123,142,18);
		m_pEditUserID->SetMaxLength(50);
		m_pEditUserID->SetText(g_Login.GetAutoUser().c_str());

		m_pEditPassword = new CCtrlEdit;
		AddControl(m_pEditPassword);
		m_pEditPassword->Create(this,FONTSIZE_DEFAULT,0xFFFFFFFF,157,159,142,18);
		m_pEditPassword->SetPwd(true);
		m_pEditPassword->SetMaxLength(40);

		m_pSubmit = new CCtrlButton;				// 提交按钮	
		AddControl(m_pSubmit);
		m_pSubmit->SetText("登录",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,14,DTF_BlackFrame,FONT_YAHEI);
		m_pSubmit->CreateEx(this,111,227,1658,1659,1660);

		m_pCancel = new CCtrlButton; // 取消按钮	
		AddControl(m_pCancel);
		m_pCancel->SetText("取消",COLOR_BTN_NORMAL_ULW,COLOR_BTN_MOUSEON_ULW,COLOR_BTN_PRESS_ULW,COLOR_BTN_DISABLE_ULW,14,DTF_BlackFrame,FONT_YAHEI);
		m_pCancel->CreateEx(this,208,227,1658,1659,1660);

		m_pEditPassword->SetFocus();

		if(g_Login.GetAutoUser().empty())
			m_pEditUserID->SetFocus();

		SetBlankUserEdit();


		CCtrlWindow* pWnd = g_pControl->FindWindowByName("SmltKeyBoard");
		if(pWnd != NULL)
		{
			pWnd->SetEnable(false);
			pWnd->SetShow(false);
		}
	}

}

void CUserLoginWnd::OnClickCloseButton()
{
	PostMessage(g_hWnd,WM_DESTROY,0,0);
}

bool CUserLoginWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	if (dwMsg == MSG_CTRL_ENABLEOPENLOGIN)
	{
		if (dwData == 10 || dwData == 11)
		{
			m_isVaildShow = (dwData == 10);			

			if (g_Login.IsSDOALogin() && g_pSDOAInterface)
			{
				if (m_isVaildShow)
				{
					if (!g_pSDOAInterface->IsShowUserLoginDialog() && g_pSDOAInterface->GetSDOAApp())
					{
						g_pSDOAInterface->GetSDOAApp()->ShowLoginDialog(CSDOAInterface::OnLogin,0,0);
						g_pSDOAInterface->SetShowUserLoginDialog(TRUE);
					}
				}
				else
				{
					if (g_pSDOAInterface->IsShowUserLoginDialog())
					{
						g_pSDOAInterface->SetShowUserLoginDialog(FALSE);
					}
				}
			}

			if(!m_isVaildShow)
				g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);			

			if(m_pEditUserID)
			{
				if (m_isVaildShow)
				{
					m_pEditUserID->SetFocus();
				}
			}

			return true;
		}
	}

	if (!m_isVaildShow)
		return false;

	switch(dwMsg)
	{
	case MSG_CTRL_BUTTON_CLICK:
		{
			if(pControl == m_pSubmit)
			{
				if( g_Config.GetGM() )
				{
					g_MsgBoxMgr.PopEditBox("请输入Passcode：",MSG_CTRL_PASSPORT,0);					
				}
				else
				{
					this->SendLoginData();
				}
				return true;
			}
			else if(pControl == m_pCancel)
			{
				PostMessage(g_hWnd,WM_DESTROY,0,0);
				return true;
			}
			if (pControl == m_pChangePassword)
			{
				g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);
				//g_pControl->PopupWindow(MSG_CTRL_OPEN_CHANGEPASSWORDWND,OPER_CREATE,1-m_iCurPage);

				string strUrl = g_pStreamMgr->GetWebsite("AnQuan");
				if(strUrl.empty())
					strUrl = "http://anquan.sdo.com/MyAccount.aspx";

				ShellExecute(NULL,"open","iexplore.exe",strUrl.c_str(),NULL,SW_SHOW);
				return true;
			}
			if (pControl == m_pHelp)
			{
				g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);
				string strUrl;	
				strUrl = g_pStreamMgr->GetWebsite("Support","");
				if (strUrl.empty())
				{
					char szTemp[128] = {0};
					sprintf(szTemp,"http://kf.sdo.com/gamearea/Default.aspx?type=%d",WS_GAME_ID);
					strUrl = szTemp;
				}
				ShellExecute(NULL,"open","iexplore.exe",strUrl.c_str(),NULL,SW_SHOW);
				return true;
			}

			if(pControl == m_pTakeSafeCard)
			{
				string strUrl;	
				strUrl = "http://ecard.sdo.com/ecard/help/ecar_product.asp?cas=1&ticket=";
				ShellExecute(NULL,"open","iexplore.exe",strUrl.c_str(),NULL,SW_SHOW);
				return true;
			}

			if(pControl == m_pBindDynamicPassword)
			{
				g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);
				string strUrl;	
				strUrl = g_pStreamMgr->GetWebsite("Bind","");
				ShellExecute(NULL,"open","iexplore.exe",strUrl.c_str(),NULL,SW_SHOW);
				return true;
			}
			if (pControl == m_pNewUser)
			{
				if(g_pSDOAInterface)
				{
					g_pSDOAInterface->SetShowUserLoginDialog(FALSE);
				}
				g_pControl->Msg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);
				g_pControl->PopupWindow(MSG_CTRL_REGISTER_WND,OPER_CREATE);
				return true;
			}
			if(pControl != m_pEditPassword)
			{
				CCtrlWindow* pWnd = g_pControl->FindWindowByName("SmltKeyBoard");
				if(pWnd != NULL)
				{
					pWnd->SetEnable(false);
					pWnd->SetShow(false);
				}
			}

			g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
		
		}
		break;
	case MSG_CTRL_ENABLELOGINWND:
		{
			if(m_pEditUserID)
				m_pEditUserID->SetFocus();

			if(m_pSubmit)
				m_pSubmit->SetEnable(true);

			if(m_pNewUser)
				m_pNewUser->SetEnable(true);

			if(m_pChangePassword)
				m_pChangePassword->SetEnable(true);

			if(m_pHelp)
				m_pHelp->SetEnable(true);

			if(m_pBindDynamicPassword)
				m_pBindDynamicPassword->SetEnable(true);

			if (m_pTakeSafeCard)
				m_pTakeSafeCard->SetEnable(true);

			return true;
		}
		break;
	case MSG_CTRL_EDIT_ENTRY:
		{
			if(m_pEditUserID->IsFocus())
			{
				m_pEditPassword->SetFocus();
				return true;
			}
			else if(!m_pSubmit->IsEnable())
			{
				return true;
			}
			else
			{
				if( g_Config.GetGM() )
				{
					g_MsgBoxMgr.PopEditBox("请输入Passcode：",MSG_CTRL_PASSPORT,0);					
				}
				else
				{
					this->SendLoginData();					
				}
				return true;
			}
		}
		break;
	case MSG_CTRL_SOFT_KEYBOARD_INPUT:
		if(m_pEditPassword)
		{
			char temp[32] = {0};
			switch(dwData)
			{
			case 0://char
				if( strlen(m_pEditPassword->GetText()) < 10 )
				{
					strcpy(temp,m_pEditPassword->GetText());		
					temp[strlen(temp)] = *(char *)pControl;		
					m_pEditPassword->SetText(temp);
				}
				break;
			case 1://back
				strcpy(temp,m_pEditPassword->GetText());
				if(strlen(temp) > 0)
				{
					temp[strlen(temp) - 1] = 0;
					m_pEditPassword->SetText(temp);
				}
				break;
			case 2://enter
				Msg(MSG_CTRL_EDIT_ENTRY,0,NULL);
				break;
			}
		}
		return true;
	case MSG_CTRL_FOCUSLOGIN:
		if(m_pEditUserID)
			m_pEditUserID->SetFocus();
		return true;

		break;
	case MSG_CTRL_LOGIN_IN_NOUSE_OA:
		{
			RemoveBodyChildControl();
			OnCreate();
			return true;
		}
		break;
	}
	return CCtrlWindowS::Msg(dwMsg,dwData,pControl);
}

void CUserLoginWnd::Draw()
{
	//OA界面还没有打开出来
	if (g_Login.IsSDOALogin())
	{
		if (g_pSDOAInterface)
		{
			if (g_pSDOAInterface->IsShowUserLoginDialog())
			{
				int ix,iy;
				g_pSDOAInterface->GetWindowPos(ix,iy);
				if (ix != m_iScreenX + 65 || iy != m_iScreenY + 58)
				{
					g_pSDOAInterface->SetWindowPos(m_iScreenX + 65,m_iScreenY + 58);
				}
			}
			else
			{
				//g_pSDOAInterface->SetShowUserLoginDialog(TRUE);			
				return;
			}
		}
	}

	if (m_isVaildShow)
	{
		CCtrlWindowS::Draw();
	}

	
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}


	//预先加载创建角色和选角色界面的图素
	if (!g_bHasDownLoadInitPackage && !m_bPreDownLoadTex)
	{
		m_bPreDownLoadTex = true;

		if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
		{
			g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,19599,EP_UI);
		}
		else if(g_pGfx->GetWidth() == 1024)
		{
			g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,19598,EP_UI);
		}
		else
		{
			g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,19597,EP_UI);
		}

		struct FrameCount_t
		{
			void Init(int b,int f,int s,int x,int y)
			{
				back = b;
				front = f;
				stand = s;
				shadow.x = x;
				shadow.y = y;
			}
			int  back;
			int  front;
			int  stand;

			POINT shadow;
		};

		enum E_CHAR_AREA
		{
			CA_MALE_FIGHTER = 0,
			CA_MALE_ENCHANTER,
			CA_MALE_TAOIST,
			CA_FEMALE_FIGHTER,
			CA_FEMALE_ENCHANTER,
			CA_FEMALE_TAOIST,
			CA_MIDDLE_CHAR,
			CA_LEFT_CHAR,
			CA_RIGHT_CHAR,
			MAX_CHAR_AREA
		};

		FrameCount_t  m_FrameInfo[6]; //帧数信息
		FrameCount_t  m_FirstIdx;

		if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
		{
			m_FirstIdx.Init(19000,18000,20000,0,0);

			m_FrameInfo[CA_MALE_FIGHTER].Init(20,60,20,410,608);
			m_FrameInfo[CA_MALE_ENCHANTER].Init(20,60,20,421,601);
			m_FrameInfo[CA_MALE_TAOIST].Init(20,60,20,409,617);
			m_FrameInfo[CA_FEMALE_FIGHTER].Init(20,48,20,433,617);
			m_FrameInfo[CA_FEMALE_ENCHANTER].Init(20,60,20,412,613);
			m_FrameInfo[CA_FEMALE_TAOIST].Init(20,60,20,409,595);
		}
		else if(g_pGfx->GetWidth() == 1024)
		{
			m_FirstIdx.Init(19000,18000,20000,0,0);

			m_FrameInfo[CA_MALE_FIGHTER].Init(20,60,20,410,608);
			m_FrameInfo[CA_MALE_ENCHANTER].Init(20,60,20,421,601);
			m_FrameInfo[CA_MALE_TAOIST].Init(20,60,20,409,617);
			m_FrameInfo[CA_FEMALE_FIGHTER].Init(20,48,20,433,617);
			m_FrameInfo[CA_FEMALE_ENCHANTER].Init(20,60,20,412,613);
			m_FrameInfo[CA_FEMALE_TAOIST].Init(20,60,20,409,595);
		}
		else
		{
			m_FirstIdx.Init(19000,18000,20000,0,0);

			m_FrameInfo[CA_MALE_FIGHTER].Init(20,60,20,410,608);
			m_FrameInfo[CA_MALE_ENCHANTER].Init(20,60,20,421,601);
			m_FrameInfo[CA_MALE_TAOIST].Init(20,60,20,409,617);
			m_FrameInfo[CA_FEMALE_FIGHTER].Init(20,48,20,433,617);
			m_FrameInfo[CA_FEMALE_ENCHANTER].Init(20,60,20,412,613);
			m_FrameInfo[CA_FEMALE_TAOIST].Init(20,60,20,409,595);
		}


		for(int iJob = 0;iJob <= 2;iJob++)
		{
			for (int iSex = 0; iSex < 2; iSex++)
			{
				int iJobGender = iJob+iSex*3;

				//g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,m_FirstIdx.front+iJobGender*100,m_FirstIdx.front+iJobGender*100 + m_FrameInfo[iJobGender].front);

				g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,m_FirstIdx.stand+iJobGender*100,m_FirstIdx.stand+iJobGender*100 + m_FrameInfo[iJobGender].stand,EP_UI);
			}
		}


		for(int ii = 0;ii < 6;ii++)
		{
			g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,m_FirstIdx.back+ii*100,m_FirstIdx.back+ii*100 + m_FrameInfo[ii].back,EP_UI);
		}


	}


	if (!m_isVaildShow)
		return;	


	//自动登录
	string& strAutoUser = g_Login.GetAutoUser();
	string& strAutoPassword = g_Login.GetAutoPassword();

	if(strAutoUser.size() > 0 && strAutoPassword.size() > 0)
	{
		SendLoginData(strAutoUser.c_str(),strAutoPassword.c_str());
	}

	if(m_pEditPassword && m_pEditPassword->IsFocus() )
	{
		CCtrlWindow* pWnd = g_pControl->FindWindowByName("SmltKeyBoard");
		if(pWnd == NULL)
		{
			g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CREATE);
			m_pEditPassword->SetFocus();
		}

		CCtrlWindow* pNewWnd = g_pControl->FindWindowByName("SmltKeyBoard");
		if(pNewWnd != NULL && (!pNewWnd->IsShow() || !pWnd))
		{
			pNewWnd->SetEnable(true);
			pNewWnd->SetShow(true);
			int nX,nY;
			CaculateKeyBoardWndPos(nX,nY);
			pNewWnd->Move(nX,nY);
			pNewWnd->SetOffX(nX);
			pNewWnd->SetOffY(nY);
			pNewWnd->SetOriginalOffX(nX);
			pNewWnd->SetOriginalOffY(nY);
		}
	}

	if(m_pEditUserID && m_pEditUserID->IsFocus())	
	{
		CCtrlWindow* pWnd = g_pControl->FindWindowByName("SmltKeyBoard");
		if(pWnd != NULL)
		{
			pWnd->SetEnable(false);
			pWnd->SetShow(false);
		}		    
	}

	if (!g_Login.IsSDOALogin())
	{
		g_pFont->DrawText(m_iScreenX + 81,m_iScreenY + 126,"盛大通行证",0xffff6600,FONT_YAHEI);
		g_pFont->DrawText(m_iScreenX + 109,m_iScreenY + 163,"密码",0xffff6600,FONT_YAHEI);
		g_pFont->DrawText(m_iScreenX + 99,m_iScreenY + 198,"请进入游戏两分钟后,再输入游戏密宝",0xffff6600,FONT_YAHEI);
	}	

	g_pFont->DrawText(164+m_iScreenX, 67+m_iScreenY, "游戏登录", 0xFFFAC896, FONT_YAHEI, FONTSIZE_BIG);
	g_pFont->DrawText(m_iScreenX + 99,m_iScreenY + 349,"使用盛大通行证可直接登录传奇世界",0xff7e4928,FONT_YAHEI);
}

bool CUserLoginWnd::SendUserPwd(const char * str)           //support function
{
	char user[64]={0},pwd[64]={0};

	if(m_pEditUserID && m_pEditPassword)
	{
		strcpy(user,m_pEditUserID->GetText());
		strcpy(pwd ,m_pEditPassword->GetText());
		if( strlen(user)<3 || strlen(pwd)<3 ) 
			return false;
	}

	if(m_pSubmit)
		m_pSubmit->SetEnable(false);

	if(m_pNewUser)
		m_pNewUser->SetEnable(false);

	if(m_pChangePassword)
		m_pChangePassword->SetEnable(false);

	if(m_pHelp)
		m_pHelp->SetEnable(false);

	if(m_pBindDynamicPassword)
		m_pBindDynamicPassword->SetEnable(false);

	if (m_pTakeSafeCard)
		m_pTakeSafeCard->SetEnable(false);

	// 内测专用,GM登录需要Passcode
	char temp[256] = {0};
	if(strlen(str) == 0)
	{
		g_pGameControl->SEND_PTLogin_Req(user, pwd);
	}
	else
	{
		sprintf(temp,"%s/%s",pwd,str);
		g_pGameControl->SEND_PTLogin_Req(user, temp);
	}
	return true;
}

bool CUserLoginWnd::SendLoginData(const char* user,const char* password)           //support function
{
	if(!m_pEditUserID || !m_pEditPassword)
		return false;

	string strUser = m_pEditUserID->GetText();
	string strPwd  = m_pEditPassword->GetText();

	//如果设置了参数则使用参数里面的东东
	if(user)
		strUser.assign(user);
	if(password)
		strPwd.assign(password);


	//去掉空格
	StringUtil::trim(strUser);
	StringUtil::trim(strPwd);

	if(strUser.size() == 0)
	{
		m_pEditUserID->SetFocus();
		return false;
	}

	if(strPwd.size() == 0)
	{
		m_pEditPassword->SetFocus();
		return false;
	}

	if(m_pSubmit)
		m_pSubmit->SetEnable(false);

	if(m_pNewUser)
		m_pNewUser->SetEnable(false);

	if(m_pChangePassword)
		m_pChangePassword->SetEnable(false);

	if(m_pHelp)
		m_pHelp->SetEnable(false);

	if(m_pBindDynamicPassword)
		m_pBindDynamicPassword->SetEnable(false);

	if (m_pTakeSafeCard)
		m_pTakeSafeCard->SetEnable(false);

	g_pGameControl->SEND_PTLogin_Req(strUser.c_str(), strPwd.c_str());
	return true;
}

void CUserLoginWnd::SetBlankUserEdit()
{
	if(m_pEditUserID)
	{
		if(m_iCurPage == PAGE_PTACCOUNT)
			m_pEditUserID->SetPrompt("在此输入盛大通行证");
		else
			m_pEditUserID->SetPrompt("在此输入游戏帐号");
	}
}

void CUserLoginWnd::CaculateKeyBoardWndPos(int& nX, int& nY)
{
	int nTop,nBottom,nLeft,nRight;
	nTop = GetY();
	nBottom = nTop + GetHeight();
	nLeft = GetX();
	nRight = nLeft + GetWidth();


	//保证不挡住IGA,并且软键盘全在屏幕内
	if(nLeft < (g_pGfx->GetWidth() - GetWidth()) / 2)
		nX = nRight + rand()%50;
	else
		nX = nLeft - 340 - 10 - rand()%50;

	if(nX < 0)
		nX = 10 + rand()%50;
	else if(nX + 340 > g_pGfx->GetWidth())
		nX = g_pGfx->GetWidth() - 340 - 10 - rand()%50;

	nY = 300 + rand()%(g_pGfx->GetHeight() - 300 - 170);

}

bool CUserLoginWnd::OnKeyDown(WORD wState,UCHAR cKey)
{
	switch(cKey)
	{
	case VK_RETURN:
		{
			if(m_pEditUserID)
				m_pEditUserID->SetFocus();
			return true;
		}
		break;
	} 

	return CCtrlWindowS::OnKeyDown(wState,cKey);
}

bool CUserLoginWnd::OnMouseMove(int iX,int iY)
{
	return CCtrlWindowS::OnMouseMove(iX,iY);
}

bool CUserLoginWnd::Move(int iX,int iY)
{
	if(iX < POS_NOMOVE)
		m_iX = iX;

	if(iY < POS_NOMOVE)
		m_iY = iY;

	//oa登录不让窗口移动外面,因为oa窗口不可以移出屏幕
	if (g_Login.IsSDOALogin())
	{
		if (m_iX <0)
		{
			m_iX = 0;
		}
		else if (m_iX + m_iWidth > g_pControl->GetWidth())
		{
			m_iX = g_pControl->GetWidth() - m_iWidth;
		}

		if (m_iY <0)
		{
			m_iY = 0;
		}
		else if (m_iY + m_iHeight > g_pControl->GetHeight())
		{
			m_iY = g_pControl->GetHeight() - m_iHeight;
		}
	}	

	OnMove();

	return true;
}

