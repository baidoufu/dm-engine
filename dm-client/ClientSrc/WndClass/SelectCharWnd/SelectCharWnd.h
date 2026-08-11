#pragma once

#include "Global/Global.h"
#include "BaseClass/Control/Control.h"
#include "BaseClass/Control/CtrlButton.h"
#include "BaseClass/Control/CtrlTip.h"
#include "CreateCharWnd.h"
#include "RecoverCharWnd.h"
#include "RegisterInfoWnd.h"
#include "GameData/GameGlobal.h"

#define MAX_JOB_GENDER         6
#define SPEED_CHAR   60

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


struct CharArea_t
{
	CharArea_t()
	{
		bSelected = false;
		pt.x = pt.y = 0;
		shadow.x = shadow.y = 0;
		dwStartTime = 0;
	}
	void Init(int l,int t,int r,int b,int x,int y,int sx,int sy)
	{
		rc.left = l;
		rc.right = r;
		rc.top = t;
		rc.bottom = b;
		pt.x = x;
		pt.y = y;
		shadow.x = sx;
		shadow.y = sy;
		dwStartTime = 0;
	}
	bool IsFrameEnd(int count,DWORD dwTime)
	{
		if(count < 1 || dwStartTime == 0)
			return false;

		if((dwTime - dwStartTime) >= SPEED_CHAR * count)
			return true;
		return false;
	}
	int  GetFrame(int count,DWORD dwTime)
	{
		if(count < 1)
			return 0;

		if (!g_bHasDownLoadInitPackage)
			return 0;

		DWORD tPass = 0;
		if(dwStartTime != 0)
			tPass = dwTime - dwStartTime;
		else
			dwStartTime = dwTime;

		int iFrame = tPass / SPEED_CHAR;

		if(tPass >= SPEED_CHAR * count) //停留在最后一帧
		{
			iFrame = count - 1;
			dwStartTime = dwTime;
		}
		return iFrame;
	}
	SRect  rc;
	POINT  pt;
	POINT  shadow;
	DWORD  dwStartTime;
	bool   bSelected;
};

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

class CSelectCharWnd : 	public CCtrlWindow
{
	DTI_DECLARE(CSelectCharWnd,CCtrlWindow)
public:
	CSelectCharWnd(void);
	~CSelectCharWnd(void);

	virtual void OnCreate();
	virtual void Draw();
	virtual bool OnLeftButtonUp(int iX,int iY);
	virtual bool OnMouseMove(int iX,int iY);
	virtual bool OnKeyDown(WORD wState, UCHAR cKey);
	virtual bool Msg(DWORD dwMsg,DWORD dwData,CControl * pControl = NULL);
	virtual bool OnLeftButtonDClick(int iX,int iY);
	virtual void ResetControlPos();

protected:
	CCtrlButton * m_pRegisterButton;

	CCtrlButton * m_pStartButton;
	CCtrlButton * m_pCreateCharButton;
	CCtrlButton * m_pDeleteCharButton;
	CCtrlButton * m_pRecoverCharButton;
	CCtrlButton * m_pQuitButton;

	CCreateCharWnd     * m_pCreateCharWnd;
	CRecoverCharWnd    * m_pRecoverCharWnd;

	CCtrlTip*     m_pTip;
	char*         m_sTipText[MAX_JOB_GENDER];

	CharArea_t    m_CharArea[MAX_CHAR_AREA]; //角色信息区域
	FrameCount_t  m_FrameInfo[MAX_JOB_GENDER]; //帧数信息
	FrameCount_t  m_FirstIdx;
	bool          m_bCreatePage;  //创建角色模式
	int           m_iWhichArea;   //在创建区选中的区域
	int           m_iAnimCharNo;  //正在放动画的角色编号
	int           m_iLastCharSize;  //上一次角色总数
	DWORD         m_dwSound;

	DWORD         m_dwEnterTime;   //进入游戏的时间
	DWORD         m_dwLoadingTime; //当前页面的时间

	int  InWhichArea(int iX,int iY);//决定鼠标在那个区
	void DrawCharInfo();
	void DrawCharacters();
	void DrawProgressBar();
	bool IsProgressBar();
	void InitLayout();
	void InitButtons();
	void SetControlState();

	void PreLoadTex();//预载图片
};
