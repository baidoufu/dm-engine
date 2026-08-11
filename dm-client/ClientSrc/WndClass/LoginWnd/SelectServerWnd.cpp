
#include "SelectServerWnd.h"
#include "GameData/GameData.h"
#include "GameData/LoginData.h"
#include "GameControl/GameControl.h"
#include "Global/Interface/AudioInterface.h"
//#include "WndClass/GameWnd/SysConfigWnd.h"
#include "Global/Interface/StreamInterface.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

INIT_WND_POS(CSelectServerWnd, POS_AUTO, POS_AUTO)

CSelectServerWnd::CSelectServerWnd(void)
{
	m_bDisableEscape = true;

	m_iIndex = 229;
	m_iOffX = 20;
	m_iOffY = 60;
	m_bHavePreLoadSelCharTex = false;
	m_bInheritScaleFromParent = false;

	for(int i = 0;i < MAX_GROUP_NUM;i++)
		m_pServerButton[i] = NULL;

	m_uStyle &= ~CTRL_STYLE_TRANS;
}


CSelectServerWnd::~CSelectServerWnd(void)
{

}

void CSelectServerWnd::Draw()
{
	CCtrlWindowS::Draw();
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}
	g_pFont->DrawText(113+m_iScreenX, 44+m_iScreenY, "选择服务器", 0xFFFAC896, FONT_YAHEI, FONTSIZE_BIG);

	//if (!m_bHavePreLoadSelCharTex && g_pStreamMgr->IsNeedDownloadFileFromServer())
	//{
	//	LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,19000);
	//	if (!pTex)
	//	{
	//		//创建角色的角色动画
	//		for (int i = 0; i < 6; i++)
	//		{
 //				//g_pTexMgr->PreLoad(PACKAGE_INTERFACE,19000 + i * 100,19000 + i * 100 + 19 + 1);
 //				//g_pTexMgr->PreLoad(PACKAGE_magic1,23000 + 19000 + i * 100,23000 + 19000 + i * 100 + 19 + 1);

	//			g_pTexMgr->PreDownLoadTex(PACKAGE_INTERFACE,19000 + i * 100,19000 + i * 100 + 19 + 1);
	//			g_pTexMgr->PreDownLoadTex(PACKAGE_magic1,23000 + 19000 + i * 100,23000 + 19000 + i * 100 + 19 + 1);
	//		}

	//		//角色动画
	//		int back = 19000,front = 18000,stand = 20000;
	//		int FrameInfo_Front[6] = {60,60,60,48,60,60};
	//		int FrameInfo_Stand[6] = {20,20,20,20,20,20};

	//		for(int iJob = 0;iJob < 3;iJob++)
	//		{
	//			for(int iSex = 0;iSex < 2;iSex++)
	//			{
	//				int iJobGender = iJob+iSex*3;

	//				g_pTexMgr->PreLoad(PACKAGE_INTERFACE,front+iJobGender*100,front+iJobGender*100 + FrameInfo_Front[iJobGender] + 1);

	//				g_pTexMgr->PreLoad(PACKAGE_INTERFACE,stand+iJobGender*100,stand+iJobGender*100 + FrameInfo_Stand[iJobGender] + 1);

	//				int n = iSex?26400:26700;
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,front+iJobGender*100 + n,front+iJobGender*100 + n + 60 + 1);
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,stand+iJobGender*100 + n,stand+iJobGender*100 + n + 60 + 1);

	//				g_pTexMgr->PreLoad(PACKAGE_magic1,front+iJobGender*100 + 26000,front+iJobGender*100 + 26000 + 60 + 1);
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,stand+iJobGender*100 + 26000,stand+iJobGender*100 + 26000 + 60 + 1);
	//			
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,front+iJobGender*100 + 23200,front+iJobGender*100 + 23200 + 60 + 1);
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,stand+iJobGender*100 + 23200,stand+iJobGender*100 + 23200 + 60 + 1);
	//			
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,front+iJobGender*100 + 23000,front+iJobGender*100 + 23000 + 60 + 1);
	//				g_pTexMgr->PreLoad(PACKAGE_magic1,stand+iJobGender*100 + 23000,stand+iJobGender*100 + 23000 + 60 + 1);
	//			}
	//		}
	//	}

	//	m_bHavePreLoadSelCharTex = true;
	//}

}

void CSelectServerWnd::OnClickCloseButton()
{
	PostMessage(g_hWnd,WM_DESTROY,0,0);
}

void CSelectServerWnd::OnCreate()
{
	SetCloseButton(234,42, 66);

	// 设置组信息
	int iGroupNum = g_Login.GetGroupList().size();
	iGroupNum = min(iGroupNum,MAX_GROUP_NUM);
 
	for(int ii=0;ii< iGroupNum;ii++)
	{
		_GroupInfo& grp = g_Login.GetGroupList().at(ii);
		if(grp.strName.empty() || grp.strName_Show.empty())
			continue;

		//创建按钮
		m_pServerButton[ii] = new CCtrlButton();
		AddControl(m_pServerButton[ii]);
		m_pServerButton[ii]->CreateEx(this,58,96+ii*27,230,231,231);
		m_pServerButton[ii]->SetText(grp.strName_Show.c_str(), 0xFFF8C993, 0xFFFFFFFF);
	}
}

bool CSelectServerWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch ( dwMsg )
	{
	case MSG_CTRL_BUTTON_CLICK:
		{
			for(int i=0;i< MAX_GROUP_NUM;i++)
			{
				if( m_pServerButton[i] == pControl )
				{
					g_Login.SetGroupNo(i);
                    //g_pGameControl->SEND_Login_ServerName();					
					g_pGameControl->SEND_SelGroup_Req(g_Login.GetGroupName());					
					g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);

					return true;
				}
			}
			break;
		}
	default:
		break;
	}

	return CCtrlWindowS::Msg(dwMsg,dwData,pControl);
}