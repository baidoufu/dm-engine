#include "StdAfx.h"
#include ".\scripttimermanager.h"

CScriptTimerManager::CScriptTimerManager(VOID)
{
	m_xTimer.fill({});
}

CScriptTimerManager::~CScriptTimerManager(VOID)
{
}

BOOL CScriptTimerManager::StartTimer(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return FALSE;
	this->m_xTimer[nIndex - 1].dwStartTime = CFrameTime::GetFrameTime();
	return TRUE;
}

BOOL CScriptTimerManager::StopTimer(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return FALSE;
	this->m_xTimer[nIndex - 1].dwStopTime = CFrameTime::GetFrameTime();
	return TRUE;
}

DWORD CScriptTimerManager::GetTimerTime(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return 0;
	DWORD dwTime = (DWORD)ROUND(GetTimeToTime(m_xTimer[nIndex - 1].dwStartTime, m_xTimer[nIndex - 1].dwStopTime) / 1000);
	return dwTime;
}