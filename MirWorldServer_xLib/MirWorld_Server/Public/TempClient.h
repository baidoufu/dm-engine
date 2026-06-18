#pragma once
#include "clientobject.h"
class CTempClient :
	public CClientObject
{
public:
	CTempClient(VOID);
	virtual ~CTempClient(VOID);
	BOOL IsTimeOut(DWORD dwTimeOut) { return m_LiveTimer.IsTimeOut(dwTimeOut); }
	VOID SaveTime() { m_LiveTimer.Savetime(); }
private:
	CServerTimer m_LiveTimer;
};