///////////////////////////////////////////////////////////////////////
//文件说明：
//    用户信息收集窗口
///////////////////////////////////////////////////////////////////////

#pragma once

#include "BaseClass/Control/CtrlWindowX.h"


class CReadQuitWnd : public CCtrlWindowX
{
	DECLARE_WND_POSX(CReadQuitWnd)

public:
	CReadQuitWnd();
	~CReadQuitWnd(void);
public:
	virtual void Draw(void);
	virtual void OnCreate();
	virtual bool Msg(DWORD dwMsg, DWORD dwData, CControl* pControl);

protected:
	DWORD   m_dwStartTime; 
};
