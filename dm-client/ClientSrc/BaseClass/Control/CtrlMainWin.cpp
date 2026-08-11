#include "Control.h"
#include "Global/GlobalErr.h"
#include "Global/GlobalMsg.h"
#include "BaseClass/Misc/Input.h"
#include "GameData/GameData.h"
#include "CtrlMenuWnd.h"
#include "UiManager.h"
#include "CtrlWindowX.h"
#include "CtrlMsgBox.h"
#include "ParserTip.h"
#include "GameData/MsgBoxMgr.h"
#include "WndClass/LoginWnd/LevelCheck.h"
#include "GameData/TalkMgr.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

BOOL a_bEdit				= FALSE;
char a_strEditHkl[32]		= {0};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DTI_IMPLEMENT(CCtrlMainWin, CCtrlWindow)
CCtrlMainWin::CCtrlMainWin()
{
	m_iControlMode	= CTRL_MODE_MAINWIN;
	m_pFocusNow		= NULL;
	m_iMouseX		= 0;
	m_iMouseY		= 0;
	m_uMouseOnID	= 0;
	m_iMouseOnType  = 0;
	m_uMouseOnCtrl	= NULL;
	m_pParserTip    = NULL;
	m_pCtrlTip      = NULL;
	m_pMenuWnd      = NULL;
	m_pTipOwnerWnd	= NULL;
	m_pMenuWnd			= NULL;
	m_bHideCursor	= FALSE;
	m_pMainWnd		= this;
	m_dwWindowMode		= 0;
	m_WndMap.clear();

	m_MWnd_MicroControl["PackageWnd"] = NULL;
	m_MWnd_MicroControl["SkillWnd"] = NULL;
	m_MWnd_MicroControl["ShortCutWnd"] = NULL;
	m_MWnd_MicroControl["ActivityLogWnd"] = NULL;
	m_MWnd_MicroControl["MinMapWnd"] = NULL;
	m_MWnd_MicroControl["PersonsGuideWnd"] = NULL;
	m_MWnd_MicroControl["AssConfigWnd"] = NULL;
	m_MWnd_MicroControl["CommonConfirmWnd"] = NULL;
	m_MWnd_MicroControl["GreetingWnd"] = NULL;
	m_MWnd_MicroControl["HelpWnd"] = NULL;
	m_MWnd_MicroControl["PersonPanelWnd"] = NULL;
	m_MWnd_MicroControl["OtherPlayerEquip"] = NULL;
	m_MWnd_MicroControl["SysConfigWnd"] = NULL;
	m_MWnd_MicroControl["TeamWnd"] = NULL;
	m_MWnd_MicroControl["ActivityLogWnd"] = NULL;
	m_MWnd_MicroControl["TneupChartWnd"] = NULL;
	m_MWnd_MicroControl["TneupCtrlPanel"] = NULL;
	m_MWnd_MicroControl["TneupWnd"] = NULL;
	m_MWnd_MicroControl["TrusteeshipConfigWnd"] = NULL;
	m_MWnd_MicroControl["QuitTneupWnd"] = NULL;
	m_MWnd_MicroControl["RTSInstanceWnd"] = NULL;
	m_MWnd_MicroControl["InstanceZonePopupWnd"] = NULL;

	m_MWnd_CaptionSubstitute["WooolStoreWnd"] = NULL;
	m_MWnd_CaptionSubstitute["WoolStorBuyCount"] = NULL;
}

CCtrlMainWin::~CCtrlMainWin()
{
	SAFE_DELETE(m_pParserTip);
	SAFE_DELETE(m_pCtrlTip);
	m_pMenuWnd = NULL;
	m_vFrameMsg.clear();

	CControl *p, *p2; 
	p = m_pControls;
	while( p )
	{
		p2 = p;
		p = p->GetControlNext();
		SAFE_DELETE(p2);
	}
	m_pControls = NULL;
}

bool CCtrlMainWin::Create(int iW,int iH)
{
	DisableIME();
	if(!CControl::Create(NULL, 0, 0,iW, iH, CTRL_STYLE_BACKMODE_NODRAW))
		return false;

	//Parser tips
	m_pParserTip = new CParserTip;
	m_pParserTip->Create(this);
	m_pParserTip->SetBackTexByID(0xB3000000);
	m_pParserTip->SetFrame(0,0);

	//装备对比的tips
	CParserTip * pSelfEquipTips = new CParserTip;
	pSelfEquipTips->SetBackTexByID(0xB3000000);
	pSelfEquipTips->SetFrame(0,0);
	m_pParserTip->AddControl(pSelfEquipTips);
	pSelfEquipTips->Create(m_pParserTip);
	m_pParserTip->SetSelfEquipTips(pSelfEquipTips);

	//Control tips
	m_pCtrlTip = new CCtrlTip;
	m_pCtrlTip->Create(this);
	m_pCtrlTip->SetBackTexByID(0x88000000);
	m_pCtrlTip->SetFrame(0,0);

	//Menu窗口
	m_pMenuWnd = new CCtrlMenuWnd;
	AddControl(m_pMenuWnd);
	m_pMenuWnd->Create(this);
	m_pMenuWnd->CloseMenu();


	return true;
}


//////////////////////////////////////////////////////////////////////
// 属性设置函数
//////////////////////////////////////////////////////////////////////

// 设置此控件为焦点控件
void CCtrlMainWin::SetFocusCtrl(CControl * pCtrl)
{
	//一个控件点击获得焦点setfocus的时候把其它的一个窗口焦点夺了killfocus,如果这个窗口失去焦点后关掉自己,此时焦点会给最上面一个窗口,然后killfocus返回true,焦点给了点中的控件,但最上层的那个窗口的m_bFocus还是true
	if(m_pFocusNow)
	{
		m_pFocusNow->SetHaveFocus(false);
	}

	m_pFocusNow = pCtrl;

	while( pCtrl && pCtrl->GetParent()!=this )
	{
		pCtrl = pCtrl->GetParent();
	}

	if( !pCtrl || pCtrl->IsNoChangeLevel())
		return;	// 两个主窗口的次序是不变的，总是在最后两位

	SwitchToTop(pCtrl);
}

bool CCtrlMainWin::SetTopWindowFocus()
{
	CControl* pNext = m_pControls;
	while(pNext)
	{
		int iMode = pNext->GetControlMode();

		if(iMode == CTRL_MODE_WINDOW || iMode == CTRL_MODE_MESSAGEBOX || iMode == CTRL_MODE_GAMEWND)
		{
			pNext->SetFocus();
			return true;
		}
		pNext = pNext->GetControlNext();
	}
	return false;
}

bool CCtrlMainWin::CloseTopWindow()
{
	CControl* pNext = m_pControls;
	while(pNext)
	{
		int iMode = pNext->GetControlMode();

		if(iMode == CTRL_MODE_WINDOW || iMode == CTRL_MODE_MESSAGEBOX)
		{
			CCtrlWindow *pWnd = dynamic_cast<CCtrlWindow *>(pNext);
			if(pWnd)
			{
				if (!pWnd->IsDisableEscape())
				{
					pWnd->OnClickCloseButton();
					return true;
				}
			}
		}
		pNext = pNext->GetControlNext();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// 消息处理函数
//////////////////////////////////////////////////////////////////////

bool CCtrlMainWin::OnMouseMove(int iX,int iY)
{
	g_pControl->GetTipWnd()->SetShow(false);//鼠标进入了MainWnd统统不显示tips,应该进不来这
	return false;
}

// 消息响应
bool CCtrlMainWin::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	CControl * p;

	if( !m_bEnable ) return false;

	switch(dwMsg) 
	{
	case MSG_INPUT_KEYDOWN:			// 这些消息只提供给焦点控件
	case MSG_INPUT_KEYUP:
	case MSG_INPUT_CHAR:
	case MSG_INPUT_WHEEL:
	case MSG_INPUT_CAPTURECHANGED:
		{
			TRY_BEGIN
				p = m_pFocusNow;
				while( p && p!=this )
				{
					if( p->IsEnable() && p->Msg(dwMsg,dwData,pControl) ) return true;
					p = p->GetParent();
				}
				return CCtrlWindow::Msg(dwMsg,dwData);
			TRY_END_DO_JUMP(
				m_pFocusNow = NULL;
			    SaveException(__FILE__,__FUNCTION__,__LINE__,pSec,strAddMsg.c_str());
			    return true;
			)

			break;
		}

	case MSG_INPUT_LEFTBT_UP:
		{
			p = m_pFocusNow;
			if( p && p!=this && p->IsClick() )  // 如果按住鼠标，但是拖动到控件外，鼠标弹起时还是给有焦点的控件发送消息
			{
				if( p->IsEnable() && p->Msg(dwMsg,dwData,pControl) ) return true;
				return CCtrlWindow::Msg(dwMsg,dwData);
			}
			else
			{
				if(AutoCloseWndProc(dwMsg, dwData, pControl))	return true;
				if( IsSonsMsg(dwMsg,dwData,pControl) ) return true;
				return CCtrlWindow::Msg(dwMsg,dwData,pControl);
			}
			break;
		}
	case MSG_INPUT_RIGHTBT_UP:
		{
			p = m_pFocusNow;
			if( p && p!=this && p->IsRBClick() )  // 如果按住鼠标，但是拖动到控件外，鼠标弹起时还是给有焦点的控件发送消息
			{
				if( p->IsEnable() && p->Msg(dwMsg,dwData,pControl) ) return true;
				return CCtrlWindow::Msg(dwMsg,dwData);
			}
			else
			{
				if(AutoCloseWndProc(dwMsg, dwData, pControl))	return true;
				if( IsSonsMsg(dwMsg,dwData,pControl) ) return true;
				return CCtrlWindow::Msg(dwMsg,dwData,pControl);
			}
			break;
		}
	case MSG_INPUT_MOVE:			// 这些消息发送给所有的控件，控件根据自身位置判断是否响应
		{
			p = m_pFocusNow;
			if( p && p!=this && (p->IsClick() || p->IsRBClick()) )  // 如果按住鼠标，但是拖动到控件外，消息还是给它,比如滚动条
			{
				if( p->IsEnable() && p->Msg(dwMsg,dwData,pControl) ) return true;
				return CCtrlWindow::Msg(dwMsg,dwData);
			}
			else if( (p = IsSonsMsg(dwMsg,dwData,pControl)) ) 
			{
				m_pTipOwnerWnd = p;
				return true;
			}

			return CCtrlWindow::Msg(dwMsg,dwData,pControl);
		}
	case MSG_INPUT_LEFTBT_DOWN:
	case MSG_INPUT_RIGHTBT_DOWN:
	case MSG_INPUT_LEFTBT_DCLICK:
	case MSG_INPUT_RIGHTBT_DCLICK:
	case MSG_INPUT_MIDDLEBT_DOWN:
	case MSG_INPUT_MIDDLEBT_UP:
		{
			if(AutoCloseWndProc(dwMsg, dwData, pControl))	return true;
			if( IsSonsMsg(dwMsg,dwData,pControl) ) return true;
			return CCtrlWindow::Msg(dwMsg,dwData,pControl);
		}
	case MSG_POPUP_MSG_BOX:
		{
			if(dwData == OPER_CLOSE)
			{
				if(pControl)
				{
					RemoveControl((CControl**)&pControl);
				}
			}
			else if(dwData == OPER_CREATE)
			{
				DWORD dwMsg = (DWORD)pControl;
				CCtrlMsgBox* pNew = NULL;

				if(dwMsg == MSG_CTRL_LEVEL_CHECK)
					pNew = new CLevelCheck();
				else
					pNew = new CCtrlMsgBox();

				AddControl(pNew);
				pNew->Create(this);
				pNew->SetFocus();
			}
			return true;
		}
	case MSG_CTRL_CLOSE_WINDOW:
		{
			if(pControl)
			{
				DELETECONTROL(&pControl);
			}
			return true;
		}
	case MSG_CTRL_POPUP_WINDOW: //弹出窗口
		{
			const char* szWindowName = (const char*)(pControl);
			PopupWindow(szWindowName,dwData);
			return true;
		}
	case MSG_CHANGE_GRAPHMODE:
		{
			PostMessage(g_hWnd,WM_DESTROY,0,0);
			return true;
		}
	}
	return CCtrlWindow::Msg(dwMsg,dwData,pControl);
}

void CCtrlMainWin::ADDCONTROL(CControl* pControl)
{
	if(pControl == NULL)
		return;

	AddControl(pControl);

	CCtrlWindowX * pXmlWnd = dynamic_cast<CCtrlWindowX *>(pControl);
	if(pXmlWnd && strlen(pXmlWnd->GetName()) > 0)
		m_WndMap[pXmlWnd->GetName()] = pXmlWnd;
}

void CCtrlMainWin::DELETECONTROL(CControl** pCtrl)
{
	if(pCtrl == NULL)
		return;
	
	//[20080716]
	bool bTopFocus = false; //窗口关闭时，顶端窗口获得焦点

	CCtrlWindowX * pXmlWnd = NULL;
	if(*pCtrl)
	{
		pXmlWnd = dynamic_cast<CCtrlWindowX *>((*pCtrl));
	}

	if(pXmlWnd && strlen(pXmlWnd->GetName()) > 0)
	{
		bTopFocus = true; //只有CCtrlWindowX的窗口才执行

		MWindow::iterator iter;
		if( (iter = m_WndMap.find(pXmlWnd->GetName())) != m_WndMap.end() )
		{
			m_WndMap.erase(iter);
		}
	}
	RemoveControl(pCtrl);

	if(bTopFocus) //顶端窗口获得焦点
	{
		SetTopWindowFocus();
	}
}

void CCtrlMainWin::ClearChildControl(void)
{
	while(!m_WndMap.empty())
	{
		MWindow::iterator it = m_WndMap.begin();
		CControl* pWnd = it->second;
		m_WndMap.erase(it);

		RemoveControl(&pWnd);
	}
	g_MsgBoxMgr.ClearAllExistMsgBox();
	ClearMsg();
}

void CCtrlMainWin::SetMouseOnID(UINT uID,CControl * pCtrl)
{
	m_uMouseOnID = uID;
	switch(uID)
	{
	case MOUSE_ON_CONTROL:
	case MOUSE_ON_INVALID:
	case MOUSE_ON_GOOD:
	case MOUSE_ON_SKILL:
	case MOUSE_ON_PETSKILL:
	case MOUSE_ON_CTRLACTION:
		m_uMouseOnCtrl = pCtrl;
		break;
	default:
		m_uMouseOnCtrl = NULL;
		break;
	}
}

void CCtrlMainWin::UpdateLayOut()
{
	if(!g_pControl)
		return;

	CControl *pCtrl = g_pControl->GetFocusCtrl();
	if(!pCtrl)
		return;

	if(pCtrl->IsNeedKeyInput())
	{
		if(a_bEdit == FALSE)
		{
			char str[32] = {0};
			GetKeyboardLayoutName(str);
			if(strlen(a_strEditHkl) > 0 && strcmp(str,a_strEditHkl) != 0)
				LoadKeyboardLayout(a_strEditHkl,KLF_ACTIVATE);
			a_bEdit = TRUE;
		}
	}
	else if(a_bEdit == TRUE)
	{
		char str[32] = {0};
		GetKeyboardLayoutName(str);
		if(strcmp(str,a_strEditHkl) != 0)
			strcpy(a_strEditHkl,str);

		if(strcmp(str,"00000804") != 0)
			LoadKeyboardLayout("00000804",KLF_ACTIVATE);

		a_bEdit = FALSE;
	}
}

void CCtrlMainWin::DisableIME()
{TRY_BEGIN
	LoadKeyboardLayout("00000804",KLF_ACTIVATE);
TRY_END_NOTHROW}

bool CCtrlMainWin::AutoCloseWndProc(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	return false;
}

void CCtrlMainWin::Draw()
{
	g_pInput->GetMousePos(m_iMouseX,m_iMouseY);

	//自动关闭有生命周期的窗口
	TRY_BEGIN
	DWORD dwTime = GetTickCount();
	CControl * p = m_pControls;
	while(p)
	{
		if(p->GetControlMode() == CTRL_MODE_WINDOW)
		{  
			CControl * pNext = p->GetControlNext();

			CCtrlWindow* wnd = static_cast<CCtrlWindow*>(p);
			DWORD dwLifeTime = wnd->GetLifeTime();
			if(dwLifeTime != 0 && dwTime > dwLifeTime)
			{
				wnd->OnClickCloseButton();
			}

			p = pNext;
			continue;
		}

		p = p->GetControlNext();
	}
	TRY_END_NOTHROW

	//处理异步的消息
	ExecuteMsg();

	CCtrlWindow::Draw();

}

//异步处理消息
void CCtrlMainWin::ExecuteMsg()
{
	while(!m_vFrameMsg.empty())
	{
		MSG_EX_STRUCT &msg = m_vFrameMsg.front();
		DWORD dwMsg = msg._msgStruct.dwMsg;
		DWORD dwData = msg._msgStruct.dwData;
		CControl* pControl = msg._msgStruct.pControl;
		CControl*	pReciever = msg._msgReciever;
		if(!pReciever)
			pReciever = g_pControl;

		m_vFrameMsg.pop_front();//不要放到pReciever->Msg之后,有可能在pReciever->Msg里面把front给erase了

		// 注意：本处pReciever所指向的CControl有可能已经被delete掉了！
		TRY_BEGIN
		pReciever->Msg(dwMsg, dwData, pControl);
		TRY_END_NOTHROW
	}
}

void CCtrlMainWin::PushMsg(DWORD dwMsg, DWORD dwData, CControl* pControl, CControl* pReciever)
{
	CControl::MSG_STRUCT msg;
	msg.Assign(dwMsg, dwData, pControl);
	MSG_EX_STRUCT	msgEx;
	msgEx._msgStruct = msg;
	msgEx._msgReciever = pReciever;
	m_vFrameMsg.push_back(msgEx);
}

void CCtrlMainWin::RemoveMsg(DWORD dwMsg, DWORD dwData, CControl* pControl, CControl* pReciever)
{
	for (MSG_DEQUE::iterator itr = m_vFrameMsg.begin(); itr != m_vFrameMsg.end();)
	{
		MSG_EX_STRUCT &msg = *itr;
		if (msg._msgStruct.dwMsg == dwMsg && msg._msgStruct.dwData == dwData && msg._msgStruct.pControl == pControl && msg._msgReciever == pReciever)
		{
			itr = m_vFrameMsg.erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

int CCtrlMainWin::FindMsg(DWORD dwMsg, DWORD dwData, CControl* pControl)
{
	int iCount = 0;
	for(MSG_DEQUE::const_iterator itor = m_vFrameMsg.begin();
		itor != m_vFrameMsg.end();
		++itor)
	{
		const MSG_EX_STRUCT& msgEx = *itor;
		if(msgEx._msgStruct.dwMsg == dwMsg && msgEx._msgStruct.dwData == dwData && msgEx._msgStruct.pControl == pControl)
			iCount++;
	}
	return iCount;
}

void CCtrlMainWin::ClearMsg()
{
	m_vFrameMsg.clear();
}

bool CCtrlMainWin::MsgToWnd(const char *szName,DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	CControl* pWindow = FindWindowByName(szName); //窗口已经打开
	if(pWindow)
	{
		return pWindow->Msg(dwMsg,dwData,pControl);
	}

	return false;
}

bool CCtrlMainWin::MsgToWnd(DWORD dwWndMsg,DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	MsgWndMap::iterator itr;
	itr = m_MMsgWnd.find(dwWndMsg);
	if(itr == m_MMsgWnd.end())
		return false;

	string strName = itr->second; //名字
	return MsgToWnd(strName.c_str(),dwMsg,dwData,pControl);
}


bool CCtrlMainWin::PopupWindow(const char* szName,DWORD dwData,DWORD dwWindowType)
{
	sm_dwWindowType = dwWindowType;//原来构造函数的参数
// 	if ((dwData == OPER_UPDATE || dwData == OPER_RECREATE || dwData == OPER_CREATE) && g_TrusteeshipData.IsTrusteeship() && g_TrusteeshipData.IsCaptain() && (g_TrusteeshipData.GetMicroControlPos() >= 0 || g_TrusteeshipData.GetCaptionSubstitutePos() >= 0))
// 	{
// 		if (m_MWnd_MicroControl.find(szName) == m_MWnd_MicroControl.end())
// 		{
// 			//if (g_TrusteeshipData.GetMicroControlPos() < 0 && g_TrusteeshipData.GetCaptionSubstitutePos() >= 0 && m_MWnd_CaptionSubstitute.find(szName) != m_MWnd_MicroControl.end())
// 			//附身情况下除了m_MWnd_MicroControl可以开外,还有m_MWnd_CaptionSubstitute可以开
// 			if (g_TrusteeshipData.GetMicroControlPos() >= 0 || m_MWnd_CaptionSubstitute.find(szName) == m_MWnd_CaptionSubstitute.end())
// 			{
// 				g_TalkMgr.PushSysTalk("操控其他队员时不能使用此功能！");
// 				return false;
// 			}
// 		}
// 	}

	CControl* pWindow = FindWindowByName(szName); //窗口已经打开
	if(pWindow)
	{
		//先把这个消息传给该窗口，有些地方要处理
		if(pWindow->Msg(dwData,dwWindowType))
			return true;

		if(dwData == OPER_CREATE) //已经打开
		{
			pWindow->SetFocus();
			return true;
		}
		else if(dwData == OPER_CLOSE || dwData == OPER_UPDATE) //关闭窗口,开关窗口
		{
			DELETECONTROL(&pWindow);
			return true;
		}
		else if(dwData== OPER_RECREATE) //已经打开了先关了再开
		{
			DELETECONTROL(&pWindow);
		}
		else //其他情况下，消息不处理
		{
			return false;
		}
	}
	else if(dwData != OPER_UPDATE && dwData != OPER_RECREATE && dwData != OPER_CREATE)
	{
		return false;
	}

	sm_dwWindowType = dwWindowType;//原来构造函数的参数
	//OPER_CREATE,OPER_UPDATE,OPER_RECREATE创建新窗口
	CCtrlWindowX * pNewWnd = NULL;
	AutoWndMap::iterator itr1 = m_MAutoWnd.find(szName);
	if(itr1 != m_MAutoWnd.end())
	{
		if(itr1->second)
		{
			pNewWnd = itr1->second();
		}
	}

	if(pNewWnd)
	{
		//有可能点了某个控件弹出窗口,设置焦点时要先把当前焦点的m_bClick,m_bRBClick设置为false,否则killfocus时会失败
		CControl *pFocusControl = g_pControl->GetFocusCtrl();
		if (pFocusControl)
		{
			pFocusControl->SetClick(false);
			pFocusControl->SetRBClick(false);
		}

		pNewWnd->SetName(szName);
		if(!pNewWnd->Create(this,pNewWnd->GetCurPage()))
		{
			SAFE_DELETE(pNewWnd);
			return false;
		}
		ADDCONTROL(pNewWnd);

		//有可能在创建窗口的时候会把焦点设置给此窗口的子控件,比如某个输入框,这种情况就不把焦点设置给此窗口
		pFocusControl = g_pControl->GetFocusCtrl();//有可能在pNewWnd->Create的时候导致GetFocusCtrl() 发生了改变
		bool bIsFoncusOnSubControl = false;
		while(pFocusControl)
		{
			if (pFocusControl->GetParent() == pNewWnd)
			{
				bIsFoncusOnSubControl = true;				
			}

			pFocusControl = pFocusControl->GetParent();
		}

		if (!bIsFoncusOnSubControl)
		{
			pNewWnd->SetFocus();
		}

		//托管状态下的窗口,让玩家看一下,3秒后自动关闭
// 		if(g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain() && pNewWnd->IsAutoCloseWhenTrusteeship())
// 			pNewWnd->SetLifeTime(GetTickCount() + 3000);


		return true;
	}

	//类不存在的话试着通过对应的XmlWindow来创建
	CXmlWindow* pXmlWnd = g_pUiMgr->GetXmlWindow(string(szName),dwWindowType);//xmlwindow通过iWindowType来表示xml的Pos
	if(pXmlWnd)
	{
		pNewWnd = new CCtrlWindowX();
		pNewWnd->SetName(szName);
		if(!pNewWnd->CreateXML(this,pXmlWnd))
		{
			SAFE_DELETE(pNewWnd);
			return false;
		}
		ADDCONTROL(pNewWnd);//必须放在CreateXML之后
		pNewWnd->SetFocus();
		return true;
	}
	return false;
}

bool CCtrlMainWin::PopupWindow(DWORD dwMsg,DWORD dwData,DWORD dwWindowType)
{
	MsgWndMap::iterator itr;
	itr = m_MMsgWnd.find(dwMsg);
	if(itr == m_MMsgWnd.end())
		return false;

	string strName = itr->second; //名字
	return PopupWindow(strName.c_str(),dwData,dwWindowType);
}

CCtrlWindow* CCtrlMainWin::FindWindowByName(const char* szName)
{
	MWindow::iterator iter = m_WndMap.find(string(szName));
	if(iter == m_WndMap.end())
		return NULL;

	return iter->second;
}

bool CCtrlMainWin::GetSPos(const char* szName,stSPos& Pos)
{
	string strName = szName;
	if(strName.empty())
		return false;

	SPosMap::iterator it = m_SPosMap.find(strName);
	if(it == m_SPosMap.end())
		return false;

	Pos = it->second;

	return true;
}

void CCtrlMainWin::SetSPos(const char* szName,const stSPos& Pos)
{
	string strName = szName;
	if(strName.empty())
		return;

	m_SPosMap[strName] = Pos;
}

bool CCtrlMainWin::DelSPos(const char* szName)
{
	SPosMap::iterator iter;
	if( (iter = m_SPosMap.find(szName)) != m_SPosMap.end() )
	{
		m_SPosMap.erase(iter);
		return true;
	}

	return false;
}

bool CCtrlMainWin::GetWindowPos(int &iX,int &iY,const char* szName)
{	
	//如果窗口开着返回现在的位置
	CCtrlWindow *pWnd = FindWindowByName(szName);
	if(pWnd)
	{
		iX = pWnd->GetX();
		iY = pWnd->GetY();
		return true;
	}

	stSPos Pos;
	if(GetSPos(szName,Pos))
	{
		iX = Pos.iLastX;
		iY = Pos.iLastY;
		return true;
	}
	return false;
}


void CCtrlMainWin::SetWindowPos(int iX,int iY,const char* szName)
{
	//如果窗口开着移动到对应位置
	CCtrlWindow *pWnd = FindWindowByName(szName);
	if(pWnd)
	{
		pWnd->Move(iX,iY);
		return;
	}

	stSPos Pos;
	GetSPos(szName,Pos);
	Pos.iLastX = iX;
	Pos.iLastY = iY;
	SetSPos(szName,Pos);
}

bool CCtrlMainWin::PopupArrowTip(DWORD dwWndMsg,int iTipID,int iX,int iY,int iAlignType,bool bShowNow,int iTipDir,int iTipType,DWORD dwData,DWORD dwLastTime,bool bNeedMsg)
{
	MsgWndMap::iterator itr;
	itr = m_MMsgWnd.find(dwWndMsg);
	if(itr == m_MMsgWnd.end())
		return false;

	string strName = itr->second; //名字
	return PopupArrowTip(strName.c_str(),iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
}

bool CCtrlMainWin::PopupArrowTip(const char* szWndName,int iTipID,int iX,int iY,int iAlignType,bool bShowNow,int iTipDir,int iTipType,DWORD dwData,DWORD dwLastTime,bool bNeedMsg)
{
	CCtrlWindow* pWindow = FindWindowByName(szWndName); //窗口已经打开
	if(pWindow)
	{
		return pWindow->AddArrowTip(iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
	}

	return false;
}

void CCtrlMainWin::ReSetAllWndPos()
{
	CCtrlWindow* pWnd = NULL;
	for(MWindow::iterator ite = m_WndMap.begin();ite != m_WndMap.end(); ite++ )
	{
		pWnd = ite->second;
		if(pWnd)
		{
			pWnd->ResetControlPos();
		}
	}

	m_SPosMap.clear();
}

bool CCtrlMainWin::ChangeUi(E_UITYPE eUi,bool bCheckWindowWidth)
{
	return false;
}
