#include "selectcharwnd.h"
#include "GameControl/GameControl.h"
#include "GameData/GameGlobal.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/OtherData.h"
#include "GameData/LoginData.h"
#include "GameData/GameData.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/ConfigData.h"
#include "GameClient/SDSamplerMgr.h"
#include "Global/Interface/StreamInterface.h"
#include <Shellapi.h>

#define  MESSAGE_CREATED_NEW_CHAR  \
	"    祝贺您创建角色成功！\n" \
	"您可以恢复您最近所删除的角色中的一个角色，但请注意，如发\n"\
	"生以下的情况，您的角色将会被系统自动删除，并且无法恢复：\n"\
	"◎小于2级且7天内未上线     ◎小于3级且14天内未上线\n"\
	"◎小于8级且30天内未上线    ◎小于15级且120天内未上线\n"\
	"◎小于30级且365天内未上线。"



#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

DTI_IMPLEMENT(CSelectCharWnd,CCtrlWindow)

CSelectCharWnd::CSelectCharWnd()
{
	m_bScaleWidthAndHeight = false;

	m_bNoMove  = true;
	m_bNoChangeLevel=true;
	m_uStyle &= ~CTRL_STYLE_TRANS;
	m_bScale = g_bNeedScale;

	m_pStartButton	= NULL;
	m_pCreateCharButton	= NULL;
	m_pDeleteCharButton	= NULL;
	m_pRecoverCharButton  = NULL;
	m_pQuitButton	= NULL;
	m_pTip			= NULL;

	m_pCreateCharWnd = NULL;
	m_pRecoverCharWnd = NULL;

	m_pRegisterButton = NULL;

	m_bCreatePage = false;
	m_iWhichArea  = -1;
	m_iAnimCharNo = -1;
	m_iLastCharSize = 0;
	m_dwEnterTime = 0;
	m_dwLoadingTime = 0;
	m_dwSound = 0;
	g_pAudio->StopAll();

	InitLayout();

	m_sTipText[0] = "战士(男)";
	m_sTipText[1] = "法师(男)";
	m_sTipText[2] = "道士(男)";
	m_sTipText[3] = "战士(女)";
	m_sTipText[4] = "法师(女)";
	m_sTipText[5] = "道士(女)";

	if (g_Config.GetBkMusicOn())
	{
		g_pAudio->PlayMusic(EAT_BKMUSIC,9,true);	// 1009.ogg
	}

	PreLoadTex();
}

CSelectCharWnd::~CSelectCharWnd(void)
{	
	SAFE_DELETE(m_pTip);
	if(m_dwSound)
		g_pAudio->Stop(EAT_MAGIC,m_dwSound,0);

	g_OtherData.SetDelCharSelectIdx(-1);
	g_OtherData.SetUnDelCharSelectIdx(-1);


}

void CSelectCharWnd::PreLoadTex()
{
	return;

	LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,m_FirstIdx.front,EP_UI);
	if (!pTex)
	{
		return;
	}


	int iCharSize = (int)g_OtherData.GetCharList().size();

	for(int ii = 0;ii < iCharSize && ii < 2;ii++)
	{
		_CharInfo& info = g_OtherData.GetCharList().at(ii);

		int iJobGender = info.byJob+info.bySex*3;

		//g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,m_FirstIdx.front+iJobGender*100,m_FirstIdx.front+iJobGender*100 + m_FrameInfo[iJobGender].front);

		g_pTexMgr->PreLoadImm(PACKAGE_INTERFACE,m_FirstIdx.stand+iJobGender*100,m_FirstIdx.stand+iJobGender*100 + m_FrameInfo[iJobGender].stand,EP_UI);

		//int n = info.bySex?26400:26700;
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.front+iJobGender*100 + n,m_FirstIdx.front+iJobGender*100 + n + 60);
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.stand+iJobGender*100 + n,m_FirstIdx.stand+iJobGender*100 + n + 60);
		//
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.front+iJobGender*100 + 26000,m_FirstIdx.front+iJobGender*100 + 26000 + 60);
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.stand+iJobGender*100 + 26000,m_FirstIdx.stand+iJobGender*100 + 26000 + 60);

		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.front+iJobGender*100 + 23200,m_FirstIdx.front+iJobGender*100 + 23200 + 60);
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.stand+iJobGender*100 + 23200,m_FirstIdx.stand+iJobGender*100 + 23200 + 60);

		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.front+iJobGender*100 + 23000,m_FirstIdx.front+iJobGender*100 + 23000 + 60);
		//g_pTexMgr->PreLoadImm(PACKAGE_magic1,m_FirstIdx.stand+iJobGender*100 + 23000,m_FirstIdx.stand+iJobGender*100 + 23000 + 60);

	}
}

void CSelectCharWnd::InitLayout()
{
	if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		m_CharArea[CA_MALE_FIGHTER].Init(258,217,350,399,-169,-153,285,474);
		m_CharArea[CA_MALE_ENCHANTER].Init(48,300,210,492,-139,-170,77,541);
		m_CharArea[CA_MALE_TAOIST].Init(663,130,770,257,-292,-164,129,427);
		m_CharArea[CA_FEMALE_FIGHTER].Init(138,85,219,255,-161,-188,642,330);
		m_CharArea[CA_FEMALE_ENCHANTER].Init(668,283,776,484,-320,-244,881,571);
		m_CharArea[CA_FEMALE_TAOIST].Init(507,229,600,417,-283,-205,19,413);

		m_CharArea[CA_MIDDLE_CHAR].Init(355,190,495,462,18,73,0,0);
		m_CharArea[CA_LEFT_CHAR].Init(215,190,355,462,-109,73,0,0);
		m_CharArea[CA_RIGHT_CHAR].Init(500,190,640,462,170,73,0,0);

		//m_FirstIdx.Init(2001,3001,8001,0,0);
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
		m_CharArea[CA_MALE_FIGHTER].Init(319,287,411,469,-108,-83,285,474);
		m_CharArea[CA_MALE_ENCHANTER].Init(58,360,220,552,-129,-110,77,541);
		m_CharArea[CA_MALE_TAOIST].Init(800,177,908,305,-91,-117,129,427);
		m_CharArea[CA_FEMALE_FIGHTER].Init(191,160,272,328,-108,-113,642,330);
		m_CharArea[CA_FEMALE_ENCHANTER].Init(868,403,976,614,-120,-114,881,571);
		m_CharArea[CA_FEMALE_TAOIST].Init(667,325,759,513,-123,-109,19,413);

		m_CharArea[CA_MIDDLE_CHAR].Init(451,332,591,604,99,215,0,0);
		m_CharArea[CA_LEFT_CHAR].Init(280,332,420,604,-44,215,0,0);
		m_CharArea[CA_RIGHT_CHAR].Init(605,332,745,604,265,215,0,0);

		//m_FirstIdx.Init(5001,6001,9001,0,0);
		m_FirstIdx.Init(19000,18000,20000,0,0);
		//18000区间存储被选中动画front
		//19000区间用于存储创建角色待选动画back
		//2000区间用于存储待选动画stand

		m_FrameInfo[CA_MALE_FIGHTER].Init(20,60,20,410,608);
		m_FrameInfo[CA_MALE_ENCHANTER].Init(20,60,20,421,601);
		m_FrameInfo[CA_MALE_TAOIST].Init(20,60,20,409,617);
		m_FrameInfo[CA_FEMALE_FIGHTER].Init(20,48,20,433,617);
		m_FrameInfo[CA_FEMALE_ENCHANTER].Init(20,60,20,412,613);
		m_FrameInfo[CA_FEMALE_TAOIST].Init(20,60,20,409,595);
	}

	else
	{
		m_CharArea[CA_MALE_FIGHTER].Init(425,287,517,469,-2,-83,285,474);
		m_CharArea[CA_MALE_ENCHANTER].Init(168,360,330,552,-19,-110,77,541);
		m_CharArea[CA_MALE_TAOIST].Init(910,177,1018,305,19,-117,129,427);
		m_CharArea[CA_FEMALE_FIGHTER].Init(300,160,382,328,2,-113,642,330);
		m_CharArea[CA_FEMALE_ENCHANTER].Init(978,403,1086,614,-10,-114,881,571);
		m_CharArea[CA_FEMALE_TAOIST].Init(777,325,870,513,-13,-109,19,413);

		m_CharArea[CA_MIDDLE_CHAR].Init(581,332,721,604,229,215,0,0);
		m_CharArea[CA_LEFT_CHAR].Init(400,332,640,604,76,215,0,0);
		m_CharArea[CA_RIGHT_CHAR].Init(725,332,865,604,385,215,0,0);

		//m_FirstIdx.Init(5001,6001,9001,0,0);
		m_FirstIdx.Init(19000,18000,20000,0,0);
		//18000区间存储被选中动画front
		//19000区间用于存储创建角色待选动画back
		//2000区间用于存储待选动画stand

		m_FrameInfo[CA_MALE_FIGHTER].Init(20,60,20,410,608);
		m_FrameInfo[CA_MALE_ENCHANTER].Init(20,60,20,421,601);
		m_FrameInfo[CA_MALE_TAOIST].Init(20,60,20,409,617);
		m_FrameInfo[CA_FEMALE_FIGHTER].Init(20,48,20,433,617);
		m_FrameInfo[CA_FEMALE_ENCHANTER].Init(20,60,20,412,613);
		m_FrameInfo[CA_FEMALE_TAOIST].Init(20,60,20,409,595);
	}
}

void CSelectCharWnd::InitButtons()
{
	m_pStartButton->Destroy();
	m_pCreateCharButton->Destroy();
	m_pDeleteCharButton->Destroy();
	m_pRecoverCharButton->Destroy();
	m_pQuitButton->Destroy();

	m_pRegisterButton->Move(0,g_pGfx->GetHeight() - 130);

	if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		m_pStartButton->CreateEx(this,365,533,20735,20736,20737);
		m_pCreateCharButton->CreateEx(this,43,547,20727,20728,20729, 20730);
		m_pDeleteCharButton->CreateEx(this,171,547,20738,20739,20740, 20741);
		m_pRecoverCharButton->CreateEx(this,538,547,20731,20732,20733,20734);
		m_pQuitButton->CreateEx(this,665,547,20742,20743,20744, 20745);
	}
	else if(g_pGfx->GetWidth() == 1024)
	{
		m_pStartButton->CreateEx(this,471,705,20735,20736,20737);
		m_pCreateCharButton->CreateEx(this,133,717,20727,20728,20729, 20730);
		m_pDeleteCharButton->CreateEx(this,284,717,20738,20739,20740, 20741);
		m_pRecoverCharButton->CreateEx(this,636,717,20731,20732,20733,20734);
		m_pQuitButton->CreateEx(this,787,717,20742,20743,20744, 20745);
	}
	else if(g_pGfx->GetWidth() == 1280)
	{
		m_pStartButton->CreateEx(this,595,737,20735,20736,20737);
		m_pCreateCharButton->CreateEx(this,175,749,20727,20728,20729, 20730);
		m_pDeleteCharButton->CreateEx(this,366,749,20738,20739,20740, 20741);
		m_pRecoverCharButton->CreateEx(this,809,749,20731,20732,20733,20734);
		m_pQuitButton->CreateEx(this,1001,749,20742,20743,20744, 20745);
	}
}

void CSelectCharWnd::OnCreate()
{
	m_pStartButton = new CCtrlButton;
	AddControl(m_pStartButton);

	m_pCreateCharButton = new CCtrlButton;
	AddControl(m_pCreateCharButton);

	m_pDeleteCharButton = new CCtrlButton;
	AddControl(m_pDeleteCharButton);

	m_pRecoverCharButton = new CCtrlButton;
	AddControl(m_pRecoverCharButton);

	m_pQuitButton = new CCtrlButton;
	AddControl(m_pQuitButton);
	m_pQuitButton->SetSwitchTex(4870,4871,4872);

	m_pRegisterButton = new CCtrlButton;
	AddControl(m_pRegisterButton);
	m_pRegisterButton->CreateEx(this,0,g_pGfx->IsBig()?632:470,11550,11551,11552);
	m_pRegisterButton->SetText("点击修改身份信息");

	InitButtons(); //初始化创建按钮

	g_OtherData.GetCharList().clear(); //清除之前的数据
	g_OtherData.SetReceivedCharList(false); 
	Sleep(800); 

	//g_pGameControl->SEND_Selchar_CharServer();
	m_dwLoadingTime = GetTickCount();

	//如果已经注册之后或者服关闭了该功能就隐藏掉了 
	if((g_dwServerSwitch & SS_REGISTER_FCM) == 0)
	{
		m_pRegisterButton->SetShow(false);
		m_pRegisterButton->SetEnable(false);
		g_pControl->PopupWindow(MSG_CTRL_REGISTER_INFO_WND,OPER_CLOSE);
	}

	m_pTip = new CCtrlTip;
	m_pTip->Create(this);
	m_pTip->SetText("");
	m_pTip->SetBackTexByID(0x80222222);
	m_pTip->SetFrame(0,0);


	g_SdSamplerMgr.SendGameVersionSample();
	g_SdSamplerMgr.SendClientInfoSample();

}

void CSelectCharWnd::Draw()
{
	if(g_pGfx->IsNoDraw())
		return;

	//显示背景和特效
	if(g_pGfx->GetWidth()==800 || g_bNeedScale)
	{
		//DrawTexture(0,0,2000);
		//DrawTexture(120,0,2025);
		DrawTexture(0,0,19599);

		DrawTexture(0,0,20751);
		if (g_bHasDownLoadInitPackage)
		{
			g_pGfx->SetRenderMode(RM_ADD2);
			DrawTexture(0,0,20752);
			g_pGfx->SetRenderMode();
		}
	}
	else if(g_pGfx->GetWidth()==1024)
	{
		//DrawTexture(0,0,5000);
		//DrawTexture(151,0,5025);
		DrawTexture(0,0,19598);

		DrawTexture(0,0,20753);
		if (g_bHasDownLoadInitPackage)
		{
			g_pGfx->SetRenderMode(RM_ADD2);
			DrawTexture(0,0,20754);
			g_pGfx->SetRenderMode();
		}
	}
	else
	{
		DrawTexture(0, 0, 19597);

		DrawTexture(0,0,20753);
		if (g_bHasDownLoadInitPackage)
		{
			g_pGfx->SetRenderMode(RM_ADD2);
			DrawTexture(111,0,20754);
			g_pGfx->SetRenderMode();
		}
	}

	if(g_OtherData.IsReceivedCharList())
	{
		DrawCharacters(); //绘制创建人物和选中角色图片
	}

	if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
	{
		DrawTexture(0,465,20726);
	}
	else if(g_pGfx->GetWidth() == 1024)
	{
		DrawTexture(0,616 + 12,20725);
	}
	else
	{
		DrawTexture(0,648 + 12,20724);
	}
	DrawCharInfo(); //绘制角色信息

	CCtrlWindow::Draw();

	////// 显示组名
	//const char* szGroupName = g_Login.GetGroupName();

	//this->SetFont(FONT_LISHU,FONTSIZE_BIG);
	//TextOut(m_iWidth-STRLEN(szGroupName,FONTSIZE_BIG)-8,20,szGroupName,0xFFFFFFFF,DTF_BlackFrame);
	//this->SetFont();

	DrawProgressBar();
}


bool CSelectCharWnd::OnMouseMove(int iX,int iY)
{
	m_iWhichArea = InWhichArea(iX,iY);
	m_pTip->Move(iX + 10,iY + 10);
	return true;
}

bool CSelectCharWnd::OnLeftButtonUp(int iX, int iY)
{
	if(m_bCreatePage) //弹出
	{
		//弹出创建的角色窗口
		int iWhichArea = InWhichArea(iX,iY);
		if(iWhichArea >= 0 && iWhichArea < MAX_JOB_GENDER)
		{
			m_bClick = false;
			g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CUSTOM+iWhichArea);
			return true;
		}
	}
	else
	{
		int iWhichArea = InWhichArea(iX,iY);
		if(iWhichArea == CA_LEFT_CHAR || iWhichArea == CA_MIDDLE_CHAR)
		{
			m_bClick = false;

			g_OtherData.SetCharNo(0);
			VCharInfo& VChar = g_OtherData.GetCharList();
			if(VChar.size() <= 0)
				return true;

			if (g_bHasDownLoadInitPackage)
			{
				m_iAnimCharNo = 0;
				if(m_dwSound)
					g_pAudio->Stop(EAT_MAGIC,m_dwSound,0);

				m_dwSound = 9 + 2 * VChar[0].byJob + VChar[0].bySex;	
				g_pAudio->Play(EAT_MAGIC,m_dwSound,0);
			}
			return true;
		}
		else if(iWhichArea == CA_RIGHT_CHAR)
		{
			m_bClick = false;

			g_OtherData.SetCharNo(1);
			VCharInfo& VChar = g_OtherData.GetCharList();
			if(VChar.size() <= 1)
				return true;

			if (g_bHasDownLoadInitPackage)
			{
				m_iAnimCharNo = 1;
				if(m_dwSound)
					g_pAudio->Stop(EAT_MAGIC,m_dwSound,0);

				m_dwSound = 9 + 2*VChar[1].byJob + VChar[1].bySex;	
				g_pAudio->Play(EAT_MAGIC,m_dwSound,0);
			}
			return true;
		}
	}
	return CCtrlWindow::OnLeftButtonUp(iX,iY);
}

bool CSelectCharWnd::OnLeftButtonDClick(int iX,int iY)
{
	int iWhichArea = InWhichArea(iX,iY);
	if(iWhichArea >= CA_MIDDLE_CHAR && iWhichArea <= CA_RIGHT_CHAR)
	{
		Msg(MSG_CTRL_BUTTON_CLICK,0,m_pStartButton);
		return true;
	}
	return CCtrlWindow::OnLeftButtonDClick(iX,iY);
}

bool CSelectCharWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch ( dwMsg )
	{
	case MSG_DELETE_CHAR_OK:
		{
			g_OtherData.SetDelCharSelectIdx(g_OtherData.GetCharNo());
			g_pGameControl->SEND_DelRole_Req(g_Login.GetLoginID(),g_OtherData.GetCharName());

			if(g_OtherData.GetCharNo() == 1)
			{
				g_OtherData.SetCharNo(0);
			}
			return true;
		}
		break;
	case MSG_CTRL_ENTER_GAME_BEGIN:
		{
			//m_bEntering = true;
			m_dwEnterTime = GetTickCount();
			break;
		}
	case MSG_CTRL_ENTER_GAME_STOP:
		{
			m_dwEnterTime = 0;
			break;
		}
	case MSG_CTRL_BUTTON_CLICK:
		{
			if( pControl == m_pStartButton)
			{
				if(IsProgressBar())
					return true;

				if(g_OtherData.GetCharList().size() <= 0)
					return true;

				g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);

				//g_pGameControl->SEND_Selchar_SelectChar();
				g_pGameControl->SEND_SelRole_Req(g_Login.GetLoginID(),g_OtherData.GetCharName());
				m_dwEnterTime = GetTickCount();
				return true;
			}
			else if(pControl == m_pCreateCharButton)
			{
				if(IsProgressBar())
					return true;

				g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
				if(m_bCreatePage)
				{
					g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CREATE);
				}
				else
				{
					m_bCreatePage = true;
					m_pQuitButton->SetState(true);
					//退出纹理变返回纹理
					m_pQuitButton->ReloadTex(20775,20776,20777,20778);
				}
				return true;
			}
			else if(pControl == m_pDeleteCharButton)
			{
				if(IsProgressBar())
					return true;

				//正在删除还没有回应
				if (g_OtherData.GetDelCharSelectIdx() >= 0)
				{
					return true;
				}

				g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
				string str = StringUtil::format("你真的要删除%s这个角色吗？\n"
					"一段时间内，你将不能使用相同的角色名。\n你真的想要删除角色吗？",g_OtherData.GetCharName());

				g_MsgBoxMgr.PopSimpleComfirm(str.c_str(),MSG_DELETE_CHAR_OK,0);

				return true;
			}
			else if( pControl == m_pRecoverCharButton)
			{
				if(IsProgressBar())
					return true;

				g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
				g_pControl->Msg(MSG_CTRL_RECOVERCHARWND,OPER_CREATE);
				return true;
			}
			else if( pControl == m_pQuitButton)
			{
				g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
				if(m_bCreatePage && g_OtherData.GetCharList().size() != 0)
				{
					m_bCreatePage = false;
					m_pQuitButton->SetState(false);
					if (g_bHasDownLoadInitPackage)
					{
						m_iAnimCharNo = g_OtherData.GetCharNo();
					}

					//返回纹理变退出纹理
					m_pQuitButton->ReloadTex(20742,20743,20744,20745);
				}
				else
				{
					g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CREATE);
				}
				return true;
			}
			else if(pControl == m_pRegisterButton)
			{
				//g_pControl->PopupWindow(MSG_CTRL_REGISTER_INFO_WND,OPER_CREATE);
				//g_pControl->Msg(MSG_CTRL_REGISTER_INFO_WND,MSG_CTRL_REGISTER_INFO_WND_FOCUS);				
				string strUrl = g_pStreamMgr->GetWebsite("FanChenMi");
				if(strUrl.empty())
					strUrl = "http://fcm.sdo.com/PTInfo/FillAdultInfo.aspx";

				ShellExecute(NULL,"open","iexplore.exe",strUrl.c_str(),NULL,SW_SHOW);

				return true;
			}
		}
		break;
	case MSG_CTRL_RECOVERCHARWND:
		if(dwData == OPER_CLOSE)
		{
			RemoveControl((CControl **)(&m_pRecoverCharWnd) );
			return true;
		}
		else if(dwData == OPER_CREATE)
		{
			if(m_pRecoverCharWnd)
				return true;

			m_pRecoverCharWnd = new CRecoverCharWnd();
			AddControl(m_pRecoverCharWnd);
			m_pRecoverCharWnd->Create(this);
			return true;
		}
		break;
	case MSG_CTRL_CREATECHARWND:
		if(dwData == OPER_CLOSE)
		{
			RemoveControl((CControl**)(&m_pCreateCharWnd));
			return true;
		}
		else if(dwData >= OPER_CUSTOM || dwData == OPER_CREATE)
		{
			if(IsProgressBar())
				return true;

			int iWhichArea = dwData - OPER_CUSTOM;
			if(iWhichArea < 0)
				iWhichArea = rand() % MAX_JOB_GENDER;

			if(m_pCreateCharWnd)
			{
				m_pCreateCharWnd->Msg(MSG_CTRL_CREATECHAR_UPDATE,dwData); //更新
				return true;
			}

			m_pCreateCharWnd = new CCreateCharWnd(iWhichArea);
			AddControl(m_pCreateCharWnd);

			if (g_bNeedScale)
			{
				if(iWhichArea == 0 || iWhichArea == 1 || iWhichArea == 3)
					m_pCreateCharWnd->Create(this,g_pGfx->GetWidth() - 320,30);
				else
					m_pCreateCharWnd->Create(this,80,30);
			}
			else
			{
				if(g_pGfx->GetWidth() == 1280)
				{
					if(iWhichArea == 0 || iWhichArea == 1 || iWhichArea == 3)
						m_pCreateCharWnd->Create(this,900,70);
					else
						m_pCreateCharWnd->Create(this,138,70);
				}
				else if(g_pGfx->GetWidth() == 800)
				{
					if(iWhichArea == 0 || iWhichArea == 1 || iWhichArea == 3)
						m_pCreateCharWnd->Create(this,480,30);
					else
						m_pCreateCharWnd->Create(this,80,30);
				}
				else
				{
					if(iWhichArea == 0 || iWhichArea == 1 || iWhichArea == 3)
						m_pCreateCharWnd->Create(this,648,70);
					else
						m_pCreateCharWnd->Create(this,138,70);
				}
			}

			return true;
		}
		break;
	case MSG_CTRL_REGISTERINFO_BTN_HIDE:
		{
			m_pRegisterButton->SetShow(false);
			m_pRegisterButton->SetEnable(false);
			return true;
		}
	case MSG_CTRL_DISPLAY_MODE:
		{
			InitLayout();
			InitButtons();
			return true;
		}
		break;
	default:
		break;
	}
	return CCtrlWindow::Msg(dwMsg,dwData,pControl);
}

bool CSelectCharWnd::OnKeyDown(WORD wState, UCHAR cKey)
{
	switch (cKey)
	{
	case VK_RETURN:
		{
			Msg(MSG_CTRL_BUTTON_CLICK,0,m_pStartButton);
			return true;
		}
		break;
	default:
		break;
	}
	return CCtrlWindow::OnKeyDown(wState,cKey);
}

int CSelectCharWnd::InWhichArea(int iX,int iY)
{
	int iScaleX = iX,iScaleY = iY;
	if(m_bScale)
	{
		iScaleX = (int)(iScaleX / g_ScaleRate.fx + 0.5);
		iScaleY = (int)(iScaleY / g_ScaleRate.fy + 0.5);
	}

	if(m_bCreatePage)
	{
		for(int ii = 0;ii < MAX_JOB_GENDER;ii++)
		{
			if(m_CharArea[ii].rc.PtInRect(iScaleX,iScaleY))
				return ii;
		}
	}
	else
	{
		if(g_OtherData.GetCharList().size() > 1)
		{
			for(int ii = CA_LEFT_CHAR;ii <= CA_RIGHT_CHAR;ii++)
			{
				if(m_CharArea[ii].rc.PtInRect(iScaleX,iScaleY))
					return ii;
			}
		}
		else
		{
			if(m_CharArea[CA_MIDDLE_CHAR].rc.PtInRect(iScaleX,iScaleY))
				return CA_MIDDLE_CHAR;
		}
	}
	return -1;
}

void CSelectCharWnd::DrawCharInfo()
{
	int iSize = g_OtherData.GetCharList().size();
	m_iFont = FONT_YAHEI;

	for(int ii = 0; ii < iSize;ii++)
	{
		_CharInfo& info = g_OtherData.GetCharList().at(ii);

		DWORD dwColor = 0xff808080;
		if(g_OtherData.GetCharNo() == ii)
			dwColor = 0xFFFFFFFF;

		string strLevel = StringUtil::format("%d",info.wLevel);

		if(g_pGfx->GetWidth() == 800 || g_bNeedScale)
		{
			int iStartX = 83;
			if(ii > 0)
				iStartX = 570;

			TextOut(iStartX, 529,info.szRoleName,dwColor, DTF_Center);
			if(ii == 0)
				TextOut(iStartX+115,529,GetJobName(info.byJob),dwColor, DTF_Center);
			else
				TextOut(iStartX+110,529,GetJobName(info.byJob),dwColor, DTF_Center);

			if(ii == 0)
				TextOut(iStartX+207,529,strLevel.c_str(),dwColor, DTF_Center);
			else
				TextOut(iStartX+199,529,strLevel.c_str(),dwColor, DTF_Center);

		}
		else if(g_pGfx->GetWidth() == 1024)
		{
			int iStartX = 153;
			if(ii > 0)
				iStartX = 683;

			TextOut(iStartX+10,698,info.szRoleName,dwColor, DTF_Center);
			TextOut(iStartX+132 ,698,GetJobName(info.byJob),dwColor, DTF_Center);
			TextOut(iStartX+220,698,strLevel.c_str(),dwColor, DTF_Center);
		}
		else
		{
			int iStartX = 182;
			if(ii > 0)
				iStartX = 839;

			TextOut(iStartX+10,730,info.szRoleName,dwColor, DTF_Center);
			TextOut(iStartX+170 ,730,GetJobName(info.byJob),dwColor, DTF_Center);
			TextOut(iStartX+305,730,strLevel.c_str(),dwColor, DTF_Center);
		}
	}

	m_iFont = FONT_DEFAULT;

}

void CSelectCharWnd::DrawCharacters()
{
	int iCharSize = (int)g_OtherData.GetCharList().size();

	//还没有角色的情况下，直接跳到创建角色页面
	if(!m_bCreatePage && iCharSize == 0)
	{
		m_bCreatePage = true;
		//其他要做的事情...
		m_pQuitButton->SetState(false);
	}

	//有角色创建出来，显示处于选中状态的刚刚创建的角色
	else if(m_iLastCharSize != iCharSize && iCharSize > 0 && iCharSize > m_iLastCharSize) 
	{
		if (g_bHasDownLoadInitPackage)
		{
			m_iAnimCharNo = g_OtherData.GetCharNo();
		}

		PreLoadTex();

		//新建了角色
		if(m_bCreatePage)
			g_MsgBoxMgr.PopSimpleAlert(MESSAGE_CREATED_NEW_CHAR);

		m_bCreatePage = false;
	}
	m_iLastCharSize = iCharSize;

	DWORD dwTickCount = GetTickCount();

	//显示角色和人物
	if(m_bCreatePage)  //创建人物的模式下
	{
		for(int ii = 0;ii < MAX_JOB_GENDER;ii++)
		{
			//int iShadow = (g_pGfx->IsBig()?5026:2026) + ii;

			//DrawTexture(m_CharArea[ii].shadow.x,m_CharArea[ii].shadow.y,iShadow);
			int iFrame = m_FirstIdx.back+ii*100;
			iFrame += m_CharArea[ii].GetFrame(m_FrameInfo[ii].back,dwTickCount);

			//DWORD dwColor = 0xFFDDDDDD;
			DWORD dwColor = 0xFFFFFFFF;
			if(m_iWhichArea == ii)
				dwColor = 0xFFFFFFFF;

			LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,iFrame,EP_UI);

			if (pTex)
			{
				DrawTexture(m_CharArea[ii].pt.x,m_CharArea[ii].pt.y,pTex,dwColor);
			}
			//else
			//{
			//	int test = 3;
			//}

			if (g_bHasDownLoadInitPackage)
			{
				pTex = g_pTexMgr->GetTexImm(PACKAGE_magic1,iFrame+23000,EP_MAGIC);
				if(pTex)
				{
					if(IsFadeIn())
						dwColor &= m_dwBackColor;

					g_pGfx->SetRenderMode(RM_ADD2);
					//g_pGfx->DrawTextureNL(m_CharArea[ii].pt.x,m_CharArea[ii].pt.y,pTex,dwColor);
					DrawTexture(m_CharArea[ii].pt.x,m_CharArea[ii].pt.y,pTex,dwColor);
					g_pGfx->SetRenderMode();
				}
				//else
				//{
				//	int test = 3;
				//}
			}


			if(m_iWhichArea == ii)
			{
				m_pTip->SetText(m_sTipText[ii]);
				m_pTip->Draw();
			}
		}
	}
	else
	{
		int ia = CA_MIDDLE_CHAR;

		for(int ii = 0;ii < iCharSize && ii < 2;ii++)
		{
			_CharInfo& info = g_OtherData.GetCharList().at(ii);

			if(iCharSize > 1) //两个以上角色
				ia += 1;

			int iJobGender = info.byJob+info.bySex*3;

			int iFrame = 0;

			if(ii == m_iAnimCharNo)
			{
				if(m_CharArea[ia].IsFrameEnd(m_FrameInfo[iJobGender].front,dwTickCount))
					m_iAnimCharNo = -1;

				iFrame = m_FirstIdx.front+iJobGender*100
					+ m_CharArea[ia].GetFrame(m_FrameInfo[iJobGender].front,dwTickCount);
			}
			else
			{
				iFrame = m_FirstIdx.stand+iJobGender*100
					+ m_CharArea[ia].GetFrame(m_FrameInfo[iJobGender].stand,dwTickCount);
			}

			//绘制
			int iShadow = (g_pGfx->IsBig()?5033:2033) + iJobGender;
			int ioff = g_pGfx->IsBig()?150:120;
			//if(info.iSex == 1) //女人
			//	ioff = -ioff;

			//绘制阴影
			//DrawTexture(m_CharArea[ia].pt.x+m_FrameInfo[iJobGender].shadow.x,
			//	m_CharArea[ia].pt.y+m_FrameInfo[iJobGender].shadow.y,iShadow);

			//DWORD dwColor = 0xFF808080;
			DWORD dwColor = 0xFFAAAAAA;
			if(g_OtherData.GetCharNo() == ii)
				dwColor = 0xFFFFFFFF;

			//绘制角色
			LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,iFrame,EP_UI);
			if (pTex)
			{
				DrawTexture(m_CharArea[ia].pt.x+ioff,m_CharArea[ia].pt.y,pTex,dwColor);
			}
			//else
			//{
			//	int test = 3;
			//}

			if (g_bHasDownLoadInitPackage)
			{
				//绘制特效
				if(ii == m_iAnimCharNo)//单人展示
				{
					if(m_CharArea[ia].dwStartTime == dwTickCount)
					{
						VCharInfo& VChar = g_OtherData.GetCharList();
						if(m_dwSound)
							g_pAudio->Stop(EAT_MAGIC,m_dwSound,0);

						m_dwSound = 9 + 2 * VChar[ii].byJob + VChar[ii].bySex;	
						g_pAudio->Play(EAT_MAGIC,m_dwSound,0);
					}

					if((info.byJob == JOB_FASHI && info.bySex == 1) || (info.byJob == JOB_ZHANSHI && info.bySex == 0))
					{
						int n = info.bySex?26400:26700;
						pTex = g_pTexMgr->GetTexImm(PACKAGE_magic1,iFrame+n,EP_MAGIC);
						if(pTex)
						{
							if(IsFadeIn())
								dwColor &= m_dwBackColor;

							//g_pGfx->SetRenderMode(RM_ADD1);
							//g_pGfx->DrawTextureNL(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
							DrawTexture(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
							//g_pGfx->SetRenderMode();
						}
						//else
						//{
						//	int test = 3;
						//}
					}

					pTex = g_pTexMgr->GetTexImm(PACKAGE_magic1,iFrame+26000,EP_MAGIC);
					if(pTex)
					{
						if(IsFadeIn())
							dwColor &= m_dwBackColor;

						g_pGfx->SetRenderMode(RM_ADD1);
						//g_pGfx->DrawTextureNL(m_CharArea[ia].pt.x+ioff,m_CharArea[ia].pt.y,pTex,dwColor);
						DrawTexture(m_CharArea[ia].pt.x+ioff,m_CharArea[ia].pt.y,pTex,dwColor);
						g_pGfx->SetRenderMode();
					}
					//else
					//{
					//	int test = 3;
					//}
				}
				else//单人待机
				{
					if(info.byJob == JOB_FASHI && info.bySex == 1)//有normal和add特效之分,先绘制normal
					{
						LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_magic1,iFrame+23200,EP_MAGIC);
						if(pTex)
						{
							if(IsFadeIn())
								dwColor &= m_dwBackColor;

							//g_pGfx->DrawTextureNL(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
							DrawTexture(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
						}
						//else
						//{
						//	int test = 3;
						//}
					}

					LPTexture pTex = g_pTexMgr->GetTexImm(PACKAGE_magic1,iFrame+23000,EP_MAGIC);
					if(pTex)
					{
						if(IsFadeIn())
							dwColor &= m_dwBackColor;

						g_pGfx->SetRenderMode(RM_ADD1);
						//g_pGfx->DrawTextureNL(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
						DrawTexture(m_CharArea[ia].pt.x+ioff, m_CharArea[ia].pt.y,pTex,dwColor);
						g_pGfx->SetRenderMode();
					}
					//else
					//{
					//	int test = 3;
					//}
				}
			}

		}
	}
}

bool CSelectCharWnd::IsProgressBar()
{
	if(m_dwEnterTime != 0)
		return true;

	if(!g_OtherData.IsReceivedCharList() && m_dwLoadingTime != 0)
		return true;

	return false;
}

void CSelectCharWnd::DrawProgressBar()
{
	LPTexture pBarTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,1681,EP_UI);

	DWORD dwCount = GetTickCount();
	if(m_dwEnterTime != 0) //在发送消息后将此变量赋值
	{
		DWORD dwTime = dwCount - m_dwEnterTime;

		LPTexture pEnterTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,1680,EP_UI);

		if(pBarTex && dwTime > 1)
		{
			int iBar = (((dwTime / 100)  % 20) * pBarTex->GetWidth() / 19);

			int x = g_pGfx->GetWidth() - 240, y = g_pGfx->GetHeight() - 220;

			if(pEnterTex)
				g_pGfx->DrawTextureNL(x,y,pEnterTex);

			g_pGfx->DrawPartTexture(x+40,y+64,pBarTex,0,0,iBar,-1);

			g_pFont->DrawText(x+40,y+35,"正在进入游戏请稍候:",0xffff6600,FONT_YAHEI,14);
		}

		if(dwTime > 20*1000)
		{
			m_dwEnterTime = 0;
			g_pGameControl->GCL_AddErr();
		}
	}


	if(!g_OtherData.IsReceivedCharList() && m_dwLoadingTime != 0) //在没有收到消息的时候
	{
		DWORD dwTime = dwCount - m_dwLoadingTime;

		LPTexture pLoadingTex = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,1680,EP_UI);

		if(pBarTex && dwTime > 1)
		{
			int iBar = (((dwTime / 100)  % 20) * pBarTex->GetWidth() / 19);
			int x = (g_pGfx->GetWidth() - 240) / 2, y = g_pGfx->GetHeight() > 800?330:248;

			if(pLoadingTex)
				g_pGfx->DrawTextureNL(x,y,pLoadingTex);

			g_pGfx->DrawPartTexture(x+40,y+64,pBarTex,0,0,iBar,-1);
			g_pFont->DrawText(x+40,y+35,"正在读取角色信息:",0xffff6600,FONT_YAHEI,14);
		}

		if(dwTime > 20*1000)
		{
			m_dwLoadingTime = 0;
			g_pGameControl->GCL_AddErr();
		}
	}
}

void CSelectCharWnd::SetControlState()
{
	m_iOriginalWidth = g_pGfx->GetWidth();
	m_iOriginalHeight = g_pGfx->GetHeight();

	InitLayout();
	InitButtons();
}

void CSelectCharWnd::ResetControlPos()
{
	SetControlState();
	m_bScale = g_bNeedScale;
	CCtrlWindow::ResetControlPos();
}

