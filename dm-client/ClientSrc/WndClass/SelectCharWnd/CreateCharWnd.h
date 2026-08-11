#pragma once


#include "Global/Global.h"
#include "BaseClass/Control/CtrlWindowS.h"
#include "BaseClass/Control/CtrlEdit.h"
#include "BaseClass/Control/CtrlButton.h"
#include "BaseClass/Control/CtrlRadio.h"

class CCreateCharWnd : public CCtrlWindowS
{
	DECLARE_WND_POS(CCreateCharWnd)
public:
	CCreateCharWnd();
	CCreateCharWnd(int nWhichChar);
	~CCreateCharWnd(void);

	virtual void OnDraw();
	virtual void OnCreate();
	virtual void OnClickCloseButton();
	virtual bool Msg(DWORD dwMsg,DWORD dwData,CControl * pControl = NULL);

protected:
	CCtrlEdit  * m_pNewCharID;	//新角色ID
    
	//职业
	CCtrlRadio * m_pFighter;    //武士
	CCtrlRadio * m_pEnchanter;  //魔法师
	CCtrlRadio * m_pTaoist;     //道士 
	int 		 m_iJob;

	//性别
	CCtrlRadio * m_pMale;
	CCtrlRadio * m_pFemale;
	int		     m_iGender;

	CCtrlButton * m_pSubmit;

	CCtrlButton * m_pLHairStyle;
	CCtrlButton * m_pRHairStyle;
	CCtrlButton * m_pLHairColor;
	CCtrlButton * m_pRHairColor;

	int m_nHairStyle;
	int m_nHairColor; 
protected:
	bool OnSubmit();
};
