#include "LoginWnd.h"
#include "UserLoginWnd.h"
#include "GameData/GameGlobal.h"
#include "SelectServerWnd.h"
#include "GameControl/GameControl.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameAI/AIAutoFightMgr.h"
#include "Global/Interface/StreamInterface.h"
#include "GameData/LoginData.h"
#include "GameData/GameData.h"
#include "Global/Interface/AudioInterface.h"
//#include "TodayActivityWnd.h"
#include "GameData/ConfigData.h"
#include "GameClient/SDOAInterface.h"
#include "GameClient/OpenLoginClient.h"

#define  MAX_COVER_BLOCK   256

DTI_IMPLEMENT(CLoginWnd,CCtrlWindow)

CLoginWnd::CLoginWnd()
{
	m_bDisableEscape = true;

	//连接资源服务器
	LPCTSTR strIP = (LPCTSTR)g_pStreamMgr->GetConfigStr("DownloadServerIp","0");
	int iPort = g_pStreamMgr->GetConfigInt("DownloadServerPort",0);
	if (iPort != 0)
	{
		if (g_pDownLoadNet && !g_pDownLoadNet->IsConnected())
		{
			g_pDownLoadNet->Close(0);
			g_pDownLoadNet->SetServer(0,strIP,iPort);
			//g_pDownLoadNet->Connect(0);//放到第一次请求资源的时候连接
		}

		if (g_pBgDownLoadNet && !g_pBgDownLoadNet->IsConnected())
		{
			g_pBgDownLoadNet->Close(0);
			g_pBgDownLoadNet->SetServer(0,strIP,iPort);
			//g_pBgDownLoadNet->Connect(0);//放到第一次请求资源的时候连接
		}

		if (g_pDownloadLimitSpeed && !g_pDownloadLimitSpeed->IsConnected())
		{
			g_pDownloadLimitSpeed->Close(0);
			g_pDownloadLimitSpeed->SetServer(0,strIP,iPort);
			g_pDownloadLimitSpeed->Connect(0);
			g_pDownloadLimitSpeed->SendDownLoadFile("plist.txt",true);
		}
	}



	m_bScaleWidthAndHeight = false;

	m_bNoFocus = true;
	m_bNoMove  = true;
	m_bNoChangeLevel=true;
	m_bScale = g_bNeedScale;

	m_pUserLoginWnd = NULL;
	m_pSelectServerWnd = NULL;
	//m_pTodayActWnd = NULL;
	m_dwAutoLoginGsFrame = 0;

	m_dwCoverTime = 0;
	m_dwSndaTime  = 0;
	//m_pCoverTex   = NULL;
// 	m_pSNDATex = NULL;
	m_dwSndaTexID = 0;
	m_uStyle &= ~CTRL_STYLE_TRANS;

	g_Login.SetHaveAutoLoginIn(false);
	m_bAutoLgoinStartCartoon = false;

	// 延迟创建选择服务器窗口
	m_dwSelectServerTexID = 0;
	m_bPendingCreateSelectServerWnd = false;

	if(g_Login.GetAutoLoginInType() > 0)
	{
		g_pTexMgr->PreLoad(PACKAGE_magic1,28500,28580,EP_MAGIC);
	}

	if (g_Config.GetBkMusicOn())
	{
		g_pAudio->PlayMusic(EAT_BKMUSIC,7,true); // 1001.ogg
	}

	m_strHomeUrl = g_pStreamMgr->GetWebsite("Website");
}

CLoginWnd::~CLoginWnd()
{
	//g_pTexMgr->ReleaseTex(m_pCoverTex);
// 	g_pTexMgr->ReleaseTex(m_pSNDATex);

}

void CLoginWnd::OnCreate()
{
	m_dwCoverTime = 0;
	m_dwSndaTime = GetTickCount();
	//m_pCoverTex   = g_pTexMgr->LoadTex(PACKAGE_others,g_pGfx->IsBig()?1:2);

	// 获取主界面第一张LOGO
	if (g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		m_dwSndaTexID    = MAKELONG(6,PACKAGE_others);
	}
	else if (g_pGfx->GetWidth() == 1024)
	{
		m_dwSndaTexID    = MAKELONG(5,PACKAGE_others);
	}
	else
	{
		m_dwSndaTexID    = MAKELONG(7,PACKAGE_others);
	}

	// 连接服务器
	g_pGameControl->SEND_Login_LoginInServer();
	//g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CREATE,this);
	// 弹选择组弹窗-从game.ini读取区组
	g_Login.GetGroupList().clear();
	int iGroupNum = g_pStreamMgr->GetGameInt("GroupNum", 0);
	for (int j = 0; j < iGroupNum; j++)
	{
		char key[64];
		sprintf(key, "Group%d", j);
		string strGroup = g_pStreamMgr->GetGameStr(key);
		sprintf(key, "GroupNick%d", j);
		string strNick = g_pStreamMgr->GetGameStr(key);
		g_Login.GetGroupList().push_back(_GroupInfo(strNick, strGroup));
	}

	// 预触发SelectServerWnd背景纹理下载（索引229，PACKAGE_INTERFACE包）
	// 等纹理下载完成后再创建窗口，避免窗口尺寸为0导致服务器列表不显示
	m_dwSelectServerTexID = MAKELONG(229, PACKAGE_INTERFACE);
	if (g_pStreamMgr->IsNeedDownloadFileFromServer())
	{
		g_pTexMgr->GetTexImm(m_dwSelectServerTexID, EP_UI);  // 触发下载
		m_bPendingCreateSelectServerWnd = true;
	}
	else
	{
		g_pControl->Msg(MSG_CTRL_SELECTSERVERWND, OPER_CREATE);
	}

}

void CLoginWnd::Draw()
{
	if(g_pGfx->IsNoDraw())
		return;

	//g_pGfx->ClearColor(0);
  //	g_pFont->DrawText(m_iWidth / 2 - 180,55,"何国辉,13343546,在球棒 鬼地方一地方官",-1,FONT_YAHEI,12);
  //	g_pFont->DrawText(m_iWidth / 2 - 180,70,"何国辉,13343546,在球棒 鬼地方一地方官",-1,FONT_YAHEI,18);
 
	//g_pFont->DrawText(m_iWidth / 2 - 180,95,"爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,12);
	//g_pFont->DrawText(m_iWidth / 2 - 180,115,"爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,14);
	//g_pFont->DrawText(m_iWidth / 2 - 180,135,"爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,16);
	//g_pFont->DrawText(m_iWidth / 2 - 180,155,"爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,18);
	//g_pFont->DrawText(m_iWidth / 2 - 180,175,"爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,30);

	//return;

	
	if (g_Login.GetLoginInExpType() > 0)
	{
		
	}
	else if(g_Login.GetAutoLoginInType() > 0)
	{
		if(m_pUserLoginWnd)
		{
			string& strAutoUser = g_Login.GetAutoUser();
			string& strAutoPassword = g_Login.GetAutoPassword();

			if(strAutoUser.size() > 0 && strAutoPassword.size() > 0)
			{
				m_pUserLoginWnd->SendLoginData(strAutoUser.c_str(),strAutoPassword.c_str());
			}
		}

		g_pGfx->ClearColor(0xFF000000);
		if(g_Login.IsHaveAutoLoginIn())
		{
			if(m_dwAutoLoginGsFrame == 0)
			{
				g_pAudio->Play(EAT_OTHER,913,g_pAudio->GetRand()++);
				g_pAudio->PlayEx(EAT_OTHER,914,g_pAudio->GetRand()++,100,100,100,100);//300-m_dwAutoLoginGsFrame);
			}
			m_dwAutoLoginGsFrame ++;
			m_bAutoLgoinStartCartoon = true;
		}

		//显示开门动画
		DWORD dwDrawFrame = m_dwAutoLoginGsFrame / 15;
		if(dwDrawFrame > 18)
		{
			g_Login.SetHaveAutoLoginIn(false);
			g_pControl->Msg(MSG_CTRL_GAMEWND,OPER_CREATE);
			g_pControl->PopupWindow(MSG_CTRL_GREETING_MSG_WND,OPER_CREATE);

			g_pAudio->Stop(EAT_OTHER,913,0);
			g_pAudio->Stop(EAT_OTHER,914,0);

			//把原来角色的快捷键等设置复制到跨服战服务器
			if(g_Login.GetAutoLoginInType() == 1 || g_Login.GetAutoLoginInType() == 3)
			{
				string strSrcId = StringUtil::format("%d_%s_%s",
					g_Login.GetAreaNo_Bak(),g_Login.GetGroupName_Bak(),g_Login.GetRoleName_Bak());

				string strDestId = StringUtil::format("%d_%s_%s",
					g_Login.GetAreaNo(),g_Login.GetGroupName(),SELF.GetName());

				string path = StringUtil::format("%s\\config\\",GetGameDataDir());
				string srcpath = path + strSrcId + "/";
				string destpath = path + strDestId + "/";

				if(mkdir(destpath.c_str()) == 0)
				{
					VString  Vfiles;
					Vfiles.push_back("asstool.ini");
					Vfiles.push_back("asstool.txt");
					Vfiles.push_back("bestitem.ini");
					Vfiles.push_back("boss.ini");
					Vfiles.push_back("Eat.ini");
					Vfiles.push_back("merconfig.ini");

					for(int i = 0; i < Vfiles.size(); i ++)
					{
						CopyFile((srcpath + Vfiles[i]).c_str(),(destpath + Vfiles[i]).c_str(),TRUE);
					}
				}
			}

			//这个时候LoginWnd已经被析构掉了
			return;
		}

		int iBegin = g_pGfx->GetWidth() > 800?31420:31400;
		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_magic1,iBegin + dwDrawFrame,EP_MAGIC);
		if(pTex)
		{
			if (g_bNeedScale)
			{
				g_pGfx->DrawTextureFX(0,0,pTex,-1,0,&g_ScaleRate);
			}
			else
			{
				g_pGfx->DrawTextureNL(0,0,pTex);
			}
		}
		else if(dwDrawFrame == 0)
		{
			g_pTexMgr->PreLoad(PACKAGE_magic1,iBegin,iBegin + 18,EP_MAGIC);
		}

		if(!m_bAutoLgoinStartCartoon)
		{
			string str = "正在进入跨服争霸区，请稍候...";
			if(g_Login.GetAutoLoginInType() == 1)
			{
				str = "正在进入跨服争霸区，请稍候...";
			}
			else if(g_Login.GetAutoLoginInType() == 2)
			{
				str = "正在返回原服务器，请稍候...";
			}
			else if(g_Login.GetAutoLoginInType() == 3)
			{
				str = "正在进入天绝魔域，请稍候...";
			}
			else if(g_Login.GetAutoLoginInType() == 4)
			{
				str = "正在返回中洲大陆，请稍候...";
			}
			else
			{
				str = "正在登录，请稍候...";
			}


			int w = str.length()*6;
			int h = 12;
			int x = (g_pGfx->GetWidth() -w)/2;
			int y = (g_pGfx->GetHeight() - h)/2;
			g_pFont->DrawText(x,y,str.c_str(),0xFFFFFFFF,FONT_DEFAULT,FONTSIZE_DEFAULT,0,0xFF000000);
		}

		return;
	}


	DWORD dwCount = GetTickCount();

	//正常的绘制在以上两种情况下就不绘制了
	if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		DrawTexture(0,0,22125);
		//if (g_bHasDownLoadInitPackage)
		//{
		//	g_pGfx->SetRenderMode(RM_ADD1);
		//	DrawTexture(170,204,22086);
		//	DrawTexture(508,208,22087);
		//	g_pGfx->SetRenderMode();
		//	DrawTexture(220,220,22091);
		//	DrawTexture(457,240,22092);
		//	DrawTexture(-128,220,22089);
		//}
	}
	else if(g_pGfx->GetWidth() == 1024)
	{
		DrawTexture(0,0,22126);
		//if (g_bHasDownLoadInitPackage)
		//{
		//	g_pGfx->SetRenderMode(RM_ADD1);
		//	DrawTexture(240,289,22086);
		//	DrawTexture(670,295,22087);
		//	g_pGfx->SetRenderMode();
		//	DrawTexture(220,380,22091);
		//	DrawTexture(657,389,22092);
		//	DrawTexture(-128,360,22089);
		//}
	}
	else if (g_pGfx->GetWidth() == 1280)
	{
		DrawTexture(0,0,22127);
		//if (g_bHasDownLoadInitPackage)
		//{
		//	g_pGfx->SetRenderMode(RM_ADD1);
		//	DrawTexture(330,300,22086);
		//	DrawTexture(849,307,22087);
		//	g_pGfx->SetRenderMode();
		//	DrawTexture(313,401,22091);
		//	DrawTexture(836,410,22092);
		//	DrawTexture(0,395,22089);
		//}
	}
	else if (g_pGfx->GetWidth() == 1600)
	{
		DrawTexture(0, 0, 22165);
	}
	else if (g_pGfx->GetWidth() == 1920)
	{
		DrawTexture(0, 0, 22166);
	}
	else //if (g_pGfx->GetWidth() == 2560)
	{
		DrawTexture(0, 0, 22167);
	}

	// 画版本内容
	char strVersion[256] = {0};
	sprintf(strVersion,"传奇世界 v%s",g_strVersion);
	//TextOut(m_iWidth / 2,15,strVersion,0xFFFFFFFF,DTF_Center);//里面会再缩放一次,没有必要,因为外面用不是绝对位置
	g_pFont->DrawText(m_iScreenX + m_iWidth / 2,m_iScreenY + 15,strVersion,0xFFFFFFFF,m_iFont,m_iFontSize, DTF_Center);

	// 画警告
	if(m_pUserLoginWnd)
	{
		if(g_pGfx->GetWidth() > 800)
		{
			DrawTexture(2,640,819);
			//TextOut(93,730,m_strHomeUrl.c_str(),0xFFFFFF00,DTF_UnderLine); //网站链接
		}
		else
		{
			DrawTexture(-21,494,819);
			//TextOut(71,579,m_strHomeUrl.c_str(),0xFFFFFF00,DTF_UnderLine); //网站链接
		}
	}

	// 延迟创建选择服务器窗口：等资源服背景纹理下载就绪后再创建
	if (m_bPendingCreateSelectServerWnd)
	{
		LPTexture pTex = g_pTexMgr->GetTexImm(m_dwSelectServerTexID, EP_DONT_DOWNLOAD);
		if (pTex)
		{
			g_pControl->Msg(MSG_CTRL_SELECTSERVERWND, OPER_CREATE);
			m_bPendingCreateSelectServerWnd = false;
		}
	}

	CCtrlWindow::Draw();

	//if(m_dwCoverTime > 0)
	//{
	//	if(m_dwCoverTime == 1)
	//		m_dwCoverTime = dwCount;

	//	if(m_pCoverTex)
	//	{
	//		int dwPassTime = dwCount - m_dwCoverTime;
	//		int   iStartPos = dwPassTime / 10; //每20ms移动一个像素
	//		iStartPos -= 128;  //先停64像素的时间
	//		if(iStartPos < 0) iStartPos = 0;
	//		
	//		//计算结束的位置
	//		int   iEndPos = 0;
	//		for(int ii = 0;ii < m_pCoverTex->GetFrames();ii++)
	//		{
	//			m_pCoverTex->SetCurFrame(ii);
	//			iEndPos += m_pCoverTex->GetHeight();
	//		}
	//		iEndPos -= m_iHeight;
	//		if(iEndPos < 0) iEndPos = 0;

	//		if(iStartPos < iEndPos + 128) //结束
	//		{
	//			if(iStartPos > iEndPos) //停留在最后一页一会
	//				iStartPos = iEndPos;

	//			int  iFrame = iStartPos / MAX_COVER_BLOCK;
	//			int  y =  -(iStartPos % MAX_COVER_BLOCK);

	//			while(y < m_iHeight && iFrame < m_pCoverTex->GetFrames())
	//			{
	//				m_pCoverTex->EnableSysAnim(FALSE);
	//				m_pCoverTex->SetCurFrame(iFrame);
	//				g_pGfx->DrawTextureNL(0,y,m_pCoverTex);

	//				y += MAX_COVER_BLOCK;
	//				iFrame++;
	//			}
	//		}
	//		else
	//		{
	//			OnInput();
	//			return;
	//		}
	//	}
	//	else
	//	{
	//		OnInput(); //点击了一下按钮
	//		return;
	//	}
	//	return;
	//}

	return; // 直接返回，不去显示盛大LOGO

	if(m_dwSndaTime > 0)
	{
		if(dwCount < m_dwSndaTime + 2000) //停留2秒
		{
			if(m_dwSndaTexID)
			{
				//DWORD dwColor = 0xFF * (m_dwSndaTime+4000 - dwCount) / 2000;
				//dwColor = (dwColor << 24) | 0x00FFFFFF;
				
				//画主界面第一张LOGO
				if (g_bNeedScale)
				{
					g_pGfx->DrawTextureFX(0,0,g_pTexMgr->GetTexImm(m_dwSndaTexID,EP_UI),-1,0,&g_ScaleRate);
				}
				else
				{
					g_pGfx->DrawTextureNL(0,0,g_pTexMgr->GetTexImm(m_dwSndaTexID,EP_UI));
				}
			}
			else
			{
				OnInput(); //点击了一下窗口
				return;
			}
		}
		else
		{
			OnInput();
			return;
		}
		return;
	}
}

bool CLoginWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch(dwMsg)
	{
	case MSG_CTRL_ENABLELOGINWND:
		{
			m_dwSndaTime = m_dwCoverTime = 0;
			break;
		}
	case MSG_INPUT_KEYUP:
	case MSG_INPUT_LEFTBT_UP:
	case MSG_INPUT_RIGHTBT_UP:
	case MSG_INPUT_MIDDLEBT_UP:
		{
			if(OnInput())
				return true;
			break;
		}
	case MSG_INPUT_KEYDOWN:
	case MSG_INPUT_LEFTBT_DOWN:
	case MSG_INPUT_MIDDLEBT_DOWN:
	case MSG_INPUT_RIGHTBT_DOWN:
		{
			if(m_dwCoverTime != 0 || m_dwSndaTime != 0)
				return true;
		}
		break;
	case MSG_CTRL_USERLOGINWND:
		if(dwData == OPER_CLOSE)
		{
			RemoveControl((CControl**)&m_pUserLoginWnd);
			return true;
		}
		else if(dwData == OPER_CREATE)
		{
			if(m_pUserLoginWnd)
			{
				m_pUserLoginWnd->SetFocus();
				return true;
			}

			m_pUserLoginWnd = new CUserLoginWnd;
			AddControl(m_pUserLoginWnd);

			int iCurPage = 0;
			// 切换到默认的登陆界面
			{
				string str = g_pStreamMgr->GetConfigStr("LPoptang","yes");
				if(stricmp(str.c_str(),"no") == 0)
					iCurPage = 1;
				else
					iCurPage = 0;
			}

			m_pUserLoginWnd->Create(this, POS_AUTO, POS_AUTO, iCurPage);
			return true;
		}
		break;
	case MSG_CTRL_SELECTSERVERWND:

		if(dwData == OPER_CLOSE)
		{
			RemoveControl((CControl**)&m_pSelectServerWnd);
			return true;
		}
		else if(dwData == OPER_CREATE)
		{
			if(m_pSelectServerWnd)
			{
				RemoveControl((CControl**)&m_pSelectServerWnd);
			}

			m_pSelectServerWnd = new CSelectServerWnd;
			AddControl(m_pSelectServerWnd);
			m_pSelectServerWnd->Create(this, POS_AUTO, POS_AUTO);

			m_pSelectServerWnd->SetFocus();
			return true;
		}
		return true;
	//case MSG_CTRL_ACTIVITYLOG_WND:
	//	{
	//		if(dwData == OPER_CLOSE)
	//		{
	//			RemoveControl((CControl**)&m_pTodayActWnd);
	//			return true;
	//		}
	//		else if(dwData == OPER_CREATE)
	//		{
	//			if(m_pTodayActWnd)
	//				return true;

	//			m_pTodayActWnd = new CTodayActivityWnd;
	//			AddControl(m_pTodayActWnd);
	//			if(g_pGfx->GetWidth() > 800)
	//				m_pTodayActWnd->Create(this,400,260);
	//			else
	//				m_pTodayActWnd->Create(this,400,255);
	//			return true;
	//		}
	//		return true;
	//	}
	case MSG_CTRL_SEND_PASSWORD:
		{
			m_pUserLoginWnd->SendUserPwd(g_cBuf);
			return true;
		}
	}
	return CCtrlWindow::Msg(dwMsg,dwData,pControl);
}

bool CLoginWnd::OnInput()
{
	/*if(m_dwCoverTime != 0)
	{
		//接着SNDA的LOGO
		m_dwCoverTime = 0;
		m_dwSndaTime = GetTickCount();
		return true;
	}
	else */
	if(m_dwSndaTime != 0)
	{
		m_dwSndaTime = 0;
		return true;
	}
	return false;
}


void CLoginWnd::SetControlState()
{
	m_iOriginalWidth = g_pGfx->GetWidth();
	m_iOriginalHeight = g_pGfx->GetHeight();

	// 获取主界面第一张LOGO
	if (g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		m_dwSndaTexID    = MAKELONG(6,PACKAGE_others);
	}
	else if (g_pGfx->GetWidth() == 1024)
	{
		m_dwSndaTexID    = MAKELONG(5,PACKAGE_others);
	}
	else
	{
		m_dwSndaTexID    = MAKELONG(7,PACKAGE_others);
	}

	if(m_pUserLoginWnd)
	{
		if (g_pOpenLoginClient)
		{
			g_pOpenLoginClient->ResetWndPos(m_iOriginalWidth, m_iOriginalHeight);
		}
	}
}
void CLoginWnd::ResetControlPos()
{
	SetControlState();

	m_bScale = g_bNeedScale;
	CCtrlWindow::ResetControlPos();
}

