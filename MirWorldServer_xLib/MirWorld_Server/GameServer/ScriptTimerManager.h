#pragma once
#include <array>
typedef struct tagScriptTimerDef
{
	DWORD dwStartTime;
	DWORD dwStopTime;
}ScriptTimerDef;
class CScriptTimerManager :
	public xSingletonClass<CScriptTimerManager>
{
public:
	CScriptTimerManager(VOID);
	virtual ~CScriptTimerManager(VOID);
	BOOL StartTimer(UINT nIndex);
	BOOL StopTimer(UINT nIndex);
	DWORD GetTimerTime(UINT nIndex);
protected:
	std::array<ScriptTimerDef, 256> m_xTimer;
};