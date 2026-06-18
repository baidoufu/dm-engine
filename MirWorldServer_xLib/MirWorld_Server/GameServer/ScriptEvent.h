#pragma once
#include "eventobject.h"
#include <array>

enum SE_FLAG
{
	SEF_ENTER = 1,
	SEF_LEAVE = 2,
};

class CScriptEvent :
	public CEventObject
{
public:
	CScriptEvent(VOID);
	virtual ~CScriptEvent(VOID);
	static CScriptEvent* Create(UINT nMapId, UINT x, UINT y, DWORD dwFlag, const char* pszPage);
	VOID Release();

	VOID OnEnter(CMapObject* pObject);
	VOID OnLeave(CMapObject* pObject);
	VOID OnEnterMap(CLogicMap* pMap);
protected:
	std::array<char, 256> m_szScriptPage{};
	DWORD m_dwFlag;
	xListHost<CScriptEvent>::xListNode m_xNode;
	static xListHost<CScriptEvent> m_xList;
	static xObjectPool<CScriptEvent> m_xScriptEventPool;
};