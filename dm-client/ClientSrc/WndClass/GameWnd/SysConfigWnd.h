#pragma once


#include "BaseClass/Control/CtrlWindowX.h"
#include "BaseClass/Control/CtrlRadio.h"
#include "BaseClass/Control/CtrlMultiEdit.h"
#include "BaseClass/Control/CtrlMenuButton.h"

class CSysConfigWnd :public CCtrlWindowX
{
	DECLARE_WND_POSX(CSysConfigWnd)

public:
	enum
	{
		PAGE_PICTURE = 1,
		PAGE_SOUND,
		PAGE_GAME,
		PAGE_CLISERV,
		PAGE_PERSONALITY
	};
public:
	CSysConfigWnd(void);
	~CSysConfigWnd(void);
public:
	virtual void Draw(void);
	virtual bool Msg(DWORD dwMsg,DWORD dwData,CControl * pControl = NULL);
	virtual void OnCreate();
	virtual bool OnLeftButtonDown(int iX, int iY);
	virtual bool OnLeftButtonUp(int iX, int iY);
	virtual bool OnMouseMove(int iX,int iY);

protected:	
//	int    m_iCurPage;
//	void   SwitchToPage(int iPage);

	bool DealPictureMsg(DWORD dwData,CControl* pControl);
	bool DealSoundMsg(DWORD dwData,CControl* pControl);
	bool DealGameFuncMsg(DWORD dwData,CControl* pControl);
	bool DealClientServiceMsg(DWORD dwData,CControl* pControl);
	bool DealPersionalizeMsg(DWORD dwData,CControl* pControl);

	void OnClickPerform(); //最佳性能
	void OnClickEffect();  //最佳效果
	void UpdateDataGraphic(bool bSaveAndValidate);
	void UpdateDataSound(bool bSaveAndValidate);
	void UpdateDataGameFunc(bool bSaveAndValidate);

	void UpdateDataPerson(bool bSaveAndValidate);
	bool OnclickOK();

private:

	//图像设置部分
	int  m_iWindowWidth;
	bool m_bAllWndMode;
	bool m_bHalfSpeed;
	//bool m_b3DMode;
	int  m_iTransparence;
	bool m_bIsTransparence;
	//bool m_bCanTrans;
	//bool m_bVSync;
	int  m_iMouse;
	int  m_iRGB32;
	bool m_bDefault;
	E_UITYPE m_EutUiType;
	//LPTexture m_pArrow;
	//LPTexture m_p3D;

	CCtrlRadio*          m_pRD1280;
	CCtrlRadio*          m_pRD1600;
	CCtrlRadio*          m_pRD1920;
	CCtrlRadio*          m_pRD2560;
	CCtrlRadio*          m_pRD1024;
	CCtrlRadio*          m_pRD800;
	CCtrlRadio*          m_pRDWinMode;
	CCtrlRadio*          m_pRDScreenMode;
	CCtrlRadio*          m_pRDSpeed;
	CCtrlRadio*          m_pRDHalfSpeed;
	//CCtrlRadio*          m_pRDHighQ;
	//CCtrlRadio*          m_pRDLowQ;
	CCtrlRadio*          m_pRD32Bit;
	CCtrlRadio*          m_pRD16Bit;
	//CCtrlRadio*          m_pRDVSynOpen;
	//CCtrlRadio*          m_pRDVSynClose;
	CCtrlRadio*          m_pRDCursorOpen;
	CCtrlRadio*          m_pRDCursorClose;

	//声音设置
	CCtrlRadio*          m_pRDSoundOn;
	CCtrlRadio*          m_pRDSoundOff;
	CCtrlRadio*          m_pRDNearVoiceOn;
	CCtrlRadio*          m_pRDNearVoiceOff;
	CCtrlRadio*          m_pRDTeamVoiceOn;
	CCtrlRadio*          m_pRDTeamVoiceOff;
	CCtrlRadio*          m_pRDMicroPhoneOn;
	CCtrlRadio*          m_pRDMicroPhoneOff;

	CCtrlRadio*			 m_pBkMusicOn;
	CCtrlRadio*			 m_pBkMusicOff;


	CCtrlRadio*          m_pUIHelpOn;
	CCtrlRadio*          m_pUIHelpOff;

	CCtrlRadio*          m_pClassicUI;
	CCtrlRadio*          m_pFashionUi;


	//声音（音效）
	int					 m_iSndVol;
	bool				 m_bSndOn;
	bool                 m_bIsVol;

	int					 m_iBkSndVol;
	bool				 m_bBkSndOn;
	bool				 m_bIsBkVol;
	/////////////////////////////////////////
	//圈圈的语言控制
	bool                 m_bIsIMVol;
	bool                 m_bNearVoice; // 附近语音
	bool                 m_bTeamVoice; // 组队语音
	bool                 m_bMicroPhone;
	int                  m_iVoiceVolume;

	//游戏功能

	bool				 m_bFloodOn;
	int			     	 m_iLightOn;	//0：关 1：开  2：灰色
	int					 m_iWeatherOn;
	int					 m_iTexPrepare; //0：关 1：智能  2：完全
	bool				 m_bColorHP ;
	int					 m_iSmooth;		//关闭 标准 最优	(Smooth: 0/1/2)
	bool				 m_bAutoPath;	//智能寻路
	bool                 m_bShowUIHelp;


	CCtrlRadio*         m_pRDFloodOn;
	CCtrlRadio*         m_pRDFloodOff;
	CCtrlRadio*         m_pRDColorHPOn;
	CCtrlRadio*         m_pRDColorHPOff;
	//CCtrlRadio*         m_pRDLightOn;
	//CCtrlRadio*         m_pRDLightOff;
	CCtrlRadio*         m_pRDWeatherOn;
	CCtrlRadio*         m_pRDWeatherOff;
	CCtrlRadio*         m_pRDTexPreOff;
	CCtrlRadio*         m_pRDTexPreHalf;
	CCtrlRadio*         m_pRDTexPreOn;
	CCtrlRadio*         m_pRDSmoothOff;
	CCtrlRadio*         m_pRDSmoothHalf;
	CCtrlRadio*         m_pRDSmoothOn;

	//客服功能
	CCtrlButton * m_pWebsite;
	CCtrlButton * m_pSupport;
	CCtrlButton * m_pAct;
	CCtrlButton * m_pHelp;

	//个性化设置
	bool                 m_bHusbandAndWife;
	bool                 m_bShaMember;
	bool                 m_bOpenWing;
	bool                 m_bOpenShine;
	bool                 m_bTempMask;
	std::string          m_strTitleStr;
	std::string          m_strTempMaskStr;

	CCtrlMultiEdit*      m_pMultiEdit;
	CCtrlRadio*          m_pRDHusband;
	CCtrlRadio*          m_pRDTeacher;
	CCtrlRadio*          m_pRDShaMember;
	CCtrlRadio*          m_pRDOpenWing;
	CCtrlRadio*          m_pRDOpenShine;
	CCtrlRadio*          m_pRDTempMask;
	CCtrlMenuButton*     m_pMBTempMask;         // 临时封号

	//公共部分
	CCtrlButton*  m_pOK;
	CCtrlButton*  m_pBestCompatBtn;
	CCtrlButton*  m_pBestEffectBtn;
	bool				 m_bDown;

};
