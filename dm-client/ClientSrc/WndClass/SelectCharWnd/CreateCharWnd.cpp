#include "createcharwnd.h"
#include "GameData/GameData.h"
#include "GameControl/GameControl.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/GameGlobal.h"
#include "GameData/LoginData.h"
#include "GameData/OtherData.h"
#include "SelectCharWnd.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

INIT_WND_POS(CCreateCharWnd,POS_AUTO,POS_AUTO)

CCreateCharWnd::CCreateCharWnd()
{
	m_uStyle &= ~CTRL_STYLE_TRANS;
	CCreateCharWnd(0);
}

CCreateCharWnd::CCreateCharWnd(int nWhichChar)
{
	m_bInheritScaleFromParent = false;

	m_pNewCharID = NULL; 
	m_pFighter	 = NULL; 
	m_pEnchanter = NULL;  
	m_pTaoist	 = NULL;   
	m_pMale		 = NULL;
	m_pFemale	 = NULL;
	m_pSubmit    = NULL;

	m_pLHairStyle = NULL;
	m_pRHairStyle = NULL;
	m_pLHairColor = NULL;
	m_pRHairColor = NULL;
	m_nHairStyle  = 0;
	m_nHairColor  = 0;

	if(nWhichChar >=3)
	{
        m_iGender = 1;//女
		m_nHairStyle = NUM_HALF_HAIR+1;
	}
	else
	{
		m_iGender = 0;//男
		m_nHairStyle = 1;//默认豪放型
	}

	m_iJob = nWhichChar % 3;
	m_nHairColor = rand() % MAX_HAIR_COLOR;
	m_iIndex = 20720;
}

CCreateCharWnd::~CCreateCharWnd(void)
{
}

void CCreateCharWnd::OnClickCloseButton()
{
	g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);
}

void CCreateCharWnd::OnCreate()
{

	SetCloseButton(203, 19, 66);

	m_pNewCharID = new CCtrlEdit;
	AddControl(m_pNewCharID);
	m_pNewCharID->Create(this,FONTSIZE_DEFAULT,0xFFf7a05a,100,60,95,22);
	m_pNewCharID->SetFont(FONT_SONGTI);
	m_pNewCharID->SetMaxLength(14);

	//职业选择
	m_pFighter = new CCtrlRadio;
	AddControl(m_pFighter);
	m_pFighter->Create(this,95,118,20768,20770,0,20769);

	m_pEnchanter = new CCtrlRadio;
	AddControl(m_pEnchanter);
	m_pEnchanter->Create(this,95,146,20759,20761,0,20760);

	m_pTaoist = new CCtrlRadio;
	AddControl(m_pTaoist);
	m_pTaoist->Create(this,95,174,20756,20758,0,20757);

	m_pFighter->AddBuddy(m_pEnchanter);
	m_pFighter->AddBuddy(m_pTaoist);
	m_pFighter->SetRadio(m_iJob);

	m_pFighter->SetTextAlignType(XAL_CENTER);
	m_pEnchanter->SetTextAlignType(XAL_CENTER);
	m_pTaoist->SetTextAlignType(XAL_CENTER);

	//性别
	m_pMale = new CCtrlRadio;
	AddControl(m_pMale);
	m_pMale->Create(this,95,86,20762,20764,0,20763);

	m_pFemale = new CCtrlRadio;
	AddControl(m_pFemale);
	m_pFemale->Create(this,133 ,86,20765,20767,0,20766);

	m_pMale->AddBuddy(m_pFemale);
	m_pMale->SetRadio(m_iGender);

	m_pMale->SetTextAlignType(XAL_CENTER);
	m_pFemale->SetTextAlignType(XAL_CENTER);

	//头发
	m_pLHairStyle = new CCtrlButton;

	AddControl(m_pLHairStyle);
	m_pLHairStyle->CreateEx(this,76,425,20715,20716,20717);

	m_pRHairStyle = new CCtrlButton;
	AddControl(m_pRHairStyle);
	m_pRHairStyle->CreateEx(this,165,425,20712,20713,20714);

	m_pLHairColor = new CCtrlButton;
	AddControl(m_pLHairColor);
	m_pLHairColor->CreateEx(this,76,453,20715,20716,20717);

	m_pRHairColor = new CCtrlButton;
	AddControl(m_pRHairColor);
	m_pRHairColor->CreateEx(this,165,453,20712,20713,20714);

	m_pSubmit = new CCtrlButton;
	AddControl(m_pSubmit);
	m_pSubmit->CreateEx(this,82,486,59,60,61,0);
	m_pSubmit->SetText("确 定", 0xFFffb04a ,0xFFffb74f, 0xFFff924a, COLOR_BTN_DISABLE, FONTSIZE_MIDDLE,0, FONT_YAHEI);

	m_pNewCharID->SetFocus();
}

bool CCreateCharWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch(dwMsg)
	{
	case MSG_CTRL_BUTTON_CLICK:
		if(pControl == m_pSubmit)
		{
			OnSubmit();
			return true;
		}
		else if(pControl == m_pFighter || pControl == m_pEnchanter || pControl == m_pTaoist)
		{
			m_iJob = m_pFighter->GetRadio();
			return true;
		}		
		else if(pControl == m_pMale || pControl == m_pFemale)
		{
			m_iGender = m_pMale->GetRadio();
			m_nHairStyle = 1 + m_iGender * NUM_HALF_HAIR;
			return true;
		}
		else if(pControl == m_pLHairStyle)
		{
			m_nHairStyle--;
			if(m_iGender == 0)
			{
				if(m_nHairStyle < 0)
					m_nHairStyle = NUM_HALF_HAIR;
			}
			else
			{
				if(m_nHairStyle <= NUM_HALF_HAIR)
					m_nHairStyle = NUM_HAIR_STYLE-1;
			}
			return true;
		}
		else if(pControl == m_pRHairStyle)
		{
			m_nHairStyle++;
			if(m_iGender == 0)
			{
				if(m_nHairStyle >= NUM_HALF_HAIR + 1)
					m_nHairStyle = 0;
			}
			else
			{
				if(m_nHairStyle >= NUM_HAIR_STYLE)
					m_nHairStyle = NUM_HALF_HAIR + 1;
			}
			return true;

		}
		else if(pControl == m_pLHairColor)
		{
			m_nHairColor--;
			if(m_nHairColor < 0)
				m_nHairColor = MAX_HAIR_COLOR - 1;
			return true;
		}
		else if(pControl == m_pRHairColor)
		{
			m_nHairColor++;
			if(m_nHairColor >= MAX_HAIR_COLOR)
				m_nHairColor = 0;
			return true;
		}
		break;
	case MSG_CTRL_EDIT_ENTRY:
		{
			OnSubmit();
			return true;
		}
		break;
	case MSG_CTRL_EDIT_ESCAPE:
		{
			if(pControl!=NULL)
			{
				((CCtrlEdit*)pControl)->Clear();
			}
			return true;
		}
		break;
	case MSG_CTRL_RENAMECHARNAME:
		{
			m_pNewCharID->OnSelectAll();
			m_pNewCharID->SetFocus();
			return true;
		}
		break;
	case MSG_CTRL_CREATECHAR_UPDATE:
		{
			int nWhichChar = dwData-OPER_CUSTOM;
			if(nWhichChar >= 0 && nWhichChar < MAX_JOB_GENDER)
			{
				if(nWhichChar >=3)
				{
					m_iGender = 1;//女
					m_nHairStyle = NUM_HALF_HAIR+1;
				}
				else
				{
					m_iGender = 0;//男
					m_nHairStyle = 1;//默认豪放型
				}

				m_iJob = nWhichChar % 3;

				if (m_pMale)
					m_pMale->SetRadio(m_iGender);

				if (m_pFighter)
					m_pFighter->SetRadio(m_iJob);

				return true;
			}
		}
		break;
	default:
		break;
	}	
	return CCtrlWindowS::Msg(dwMsg,dwData,pControl);
}

bool CCreateCharWnd::OnSubmit()
{
	string strCharId = m_pNewCharID->GetText();

	if(strCharId.size() > 14 || strCharId.size() < 4 || StringUtil::hasInvalidChar(strCharId))
	{
		g_MsgBoxMgr.PopSimpleAlert("无法注册，请选择其他角色名重新输入！");
		return true;
	}
	
	char pInfo[256] = {0};	
	BYTE byHair = (m_nHairColor << 4) |(m_nHairStyle & 0x0F);

	//sprintf(pInfo,"%s/%s/%d/%d/%d" ,g_Login.GetLoginID(),strCharId.c_str(),byHair,m_iJob,m_iGender);
	//g_pGameControl->SEND_Create_Character(pInfo,strlen(pInfo));

	g_pGameControl->SEND_CreateRole_Req(strCharId.c_str(),BYTE(m_iJob),BYTE(m_iGender),byHair);

	GetParent()->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE,this);
	return true;
}

void CCreateCharWnd::OnDraw()
{
	DrawTexture(32,230,20702+m_iJob);
	DrawTexture(108,366,268+m_nHairStyle);	
	//DrawTexture(108,366,20711);

	if(m_nHairStyle > 0)//光头不显示发型
	{
		DWORD dwColor = g_OtherData.GetHairColor(m_nHairColor);
		DrawTexture(108,366, 278+m_nHairStyle-1,dwColor);
	}

	TextOut(136,428,GetHairStyle(m_nHairStyle),0xFFB48C5A, DTF_Center);
	TextOut(136,457,GetHairColor(m_nHairColor),0xFFB48C5A, DTF_Center);
	g_pFont->DrawText(84+m_iScreenX, 23+m_iScreenY, "创建角色", 0xFFFAC896, FONT_YAHEI, FONTSIZE_BIG);
	g_pFont->DrawText(46+m_iScreenX, 63+m_iScreenY, "角色名", 0xFFB46428, FONT_YAHEI);
	g_pFont->DrawText(56+m_iScreenX, 93+m_iScreenY, "性别", 0xFFB46428, FONT_YAHEI);
	g_pFont->DrawText(56+m_iScreenX, 123+m_iScreenY, "职业", 0xFFB46428, FONT_YAHEI);
	g_pFont->DrawText(134+m_iScreenX, 123+m_iScreenY, "战士", 0xFFB48C5A, FONT_YAHEI);
	g_pFont->DrawText(134+m_iScreenX, 151+m_iScreenY, "法师", 0xFFB48C5A, FONT_YAHEI);
	g_pFont->DrawText(134+m_iScreenX, 179+m_iScreenY, "道士", 0xFFB48C5A, FONT_YAHEI);
	g_pFont->DrawText(36+m_iScreenX, 427+m_iScreenY, "发型", 0xFFB46428, FONT_YAHEI);
	g_pFont->DrawText(36+m_iScreenX, 457+m_iScreenY, "颜色", 0xFFB46428, FONT_YAHEI);
}


