#pragma once

#include "control.h"
#include <vector>
#include "XmlControl.h"

class CCtrlRadio :public CControl
{
	DTI_DECLARE(CCtrlRadio, CControl)
private:
	//void ReleaseRadioTex();
	void LoadRadioTex(int &iW,int &iH);

	bool		m_bChecked;
	/*LPTexture	m_pNormal;
	LPTexture   m_pChecked;
	LPTexture   m_pDisabled;
	LPTexture   m_pMouseOn;*/

	//LPTexture	m_vTexs[4];		// 按钮控件的帧数
	WORD        m_vTexID[4];
	int m_iBackTexWidth,m_iBackTexHeight;//背景图的大小
	bool m_bUseBackTexWidth,m_bUseBackTexHeight;//是否使用背景图大小作为控件的大小

	DWORD       m_dwColor;
	DWORD       m_dwCheckedColor;
	DWORD       m_dwMouseOnColor;
	DWORD       m_dwDisableColor;
	int         m_iRadioType;//0:普通的checkbox,1:group radio box
	int         m_iTextAlignType;       //在父窗口中的排列方式,XALINEMENT之一

	vector<CCtrlRadio*> m_VBuddy;
	bool m_bGroup;
public:
	CCtrlRadio(void);
	~CCtrlRadio(void);
	virtual bool Create(CControl* pParent, int iX, int iY,WORD wNormal,WORD wChecked,WORD wDisabled = 0,WORD wMouseOn = 0,int iW = 0,int iH = 0);
	virtual bool CreateEx(CControl* pParent, int iX, int iY,int iW = 0,int iH = 0,WORD wNormal = 125,WORD wChecked = 126,WORD wDisabled = 127,WORD wMouseOn = 128);
	virtual bool CreateXML(CControl* pParent,CXmlRadio* radio);
	virtual bool OnClick(int iButton);
	virtual void Draw(void);
	virtual void ResetWidthAndHeight(int iW,int iH);

	bool IsChecked(void) { return m_bChecked; }
	void EnableChecked() { m_bChecked = true; }
	void DisableChecked() { m_bChecked = false; }
	void SetChecked(bool bChecked){ m_bChecked = bChecked; }
	void SetGroup(bool bGroup){m_bGroup = bGroup;}
	int  GetRadio();
	void SetRadio(int i);
	void AddBuddy(CCtrlRadio* buddy);
	void SetRadioType(int iType){m_iRadioType = iType;}
	int  GetRadioType(){return m_iRadioType;}
	int  GetTextAlignType() const { return m_iTextAlignType; }
	void SetTextAlignType(int val) { m_iTextAlignType = val; }

	void ReloadTex(WORD wTex,WORD wMTex,WORD wCTex,WORD wDTex = 0);
	void SetText(const char * sText,DWORD dwColor = 0xFFFF0000,DWORD dwMouseOnColor = 0xFFFF0000,DWORD dwClickColor = 0xFFFF0000,DWORD dwDisableColor = 0xFFFF0000,int iFontSize = FONTSIZE_DEFAULT,DWORD dwFlag = 0,int iFont = FONT_DEFAULT,int iOffX = 0,int iOffY = 0,int iGap = 0);
};
