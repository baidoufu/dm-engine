#include "StdAfx.h"
#include ".\scriptview.h"
#include ".\scriptshell.h"
#include ".\humanplayer.h"

CScriptView::CScriptView(CScriptShell* pShell)
{
	m_szBuffer = std::make_unique<char[]>(65536);
	m_xScriptPacket.create(m_szBuffer.get(), 65536);
	m_szBuffer[0] = 0;
	ChangeShell(pShell);
	m_dwParam = 0;
	m_nPageSize = 0;
}

CScriptView::~CScriptView(VOID)
{
	m_xScriptPacket.destroy();
}

static std::array<char, 65536> s_szTempBuffer{};
BOOL CScriptView::AppendWordsEx(const char* pszWords, ...)
{
	va_list	vl;
	va_start(vl, pszWords);
	vsnprintf(s_szTempBuffer.data(), s_szTempBuffer.size(), pszWords, vl);
	va_end(vl);
	s_szTempBuffer[s_szTempBuffer.size() - 1] = 0;
	return AppendWords(s_szTempBuffer.data());
}

CScriptPageView::CScriptPageView(CScriptShell* pShell) : CScriptView(pShell)
{
	if (m_pShell)
	{
		m_xScriptPacket.push((LPVOID)pShell->GetTitleName(), (int)strlen(pShell->GetTitleName()));
		m_xScriptPacket.push((LPVOID)"/", 1);
	}
}

BOOL CScriptPageView::AppendWords(const char* pszWords)
{
	UINT nLength = static_cast<UINT>(strlen(pszWords));
	m_nPageSize += nLength;
	return m_xScriptPacket.push((LPVOID)pszWords, (int)nLength);
}

VOID CScriptPageView::SendPageToTarget(CScriptTarget* pTarget, DWORD dwParam)
{
	m_dwParam = dwParam;
	if (m_nPageSize > 0)
	{
		if (pTarget)pTarget->SendPage(this->m_pShell, this);
	}
}

VOID CScriptPageView::SendClosePageToTarget(CScriptTarget* pTarget)
{
	if (pTarget)pTarget->SendClosePage(m_pShell);
}

VOID CScriptPageView::ChangeShell(CScriptShell* pShell)
{
	m_xScriptPacket.clear();
	m_xScriptPacket.push((LPVOID)pShell->GetTitleName(), (int)strlen(pShell->GetTitleName()));
	m_xScriptPacket.push((LPVOID)"/", 1);
	CScriptView::ChangeShell(pShell);
}