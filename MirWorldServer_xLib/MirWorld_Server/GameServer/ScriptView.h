#pragma once
#include <memory>
class CScriptShell;
class CScriptTarget;

class CScriptView
{
public:
	CScriptView(CScriptShell* pShell);
	virtual ~CScriptView(VOID);

	virtual BOOL AppendWords(const char* pszWords) { return TRUE; }

	BOOL AppendWordsEx(const char* pszWords, ...);

	virtual VOID SendPageToTarget(CScriptTarget* pTarget, DWORD dwParam = 0) {}
	virtual VOID SendClosePageToTarget(CScriptTarget* pTarget) {}

	virtual VOID ChangeShell(CScriptShell* pShell) { m_pShell = pShell; }

	VOID Clear() { m_xScriptPacket.clear(); }
	xPacket& getPacket() { return m_xScriptPacket; }

	DWORD GetParam()const { return m_dwParam; }
	VOID SetParam(DWORD dwParam) { m_dwParam = dwParam; }
	UINT GetSize()const { return m_nPageSize; }
protected:
	CScriptShell* m_pShell;
	xPacket m_xScriptPacket;
	std::unique_ptr<char[]> m_szBuffer;
	DWORD m_dwParam;
	UINT m_nPageSize;
};

class CScriptPageView : public CScriptView
{
public:
	CScriptPageView(CScriptShell* pShell);
	BOOL AppendWords(const char* pszWords);
	VOID SendPageToTarget(CScriptTarget* pTarget, DWORD dwParam = 0);
	VOID SendClosePageToTarget(CScriptTarget* pTarget);
	VOID ChangeShell(CScriptShell* pShell);
};