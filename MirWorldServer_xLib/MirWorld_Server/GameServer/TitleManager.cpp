#include "StdAfx.h"
#include ".\titlemanager.h"
#include ".\fmttextfile.h"
#include ".\humanplayer.h"

CTitleManager::CTitleManager(VOID)
{
	m_iTitleCount = 0;
}

CTitleManager::~CTitleManager(VOID)
{
}

static int	sort_title(const TITLE* p1, const TITLE* p2)
{
	if (p1->dwLevel > p2->dwLevel)return 1;
	if (p1->dwLevel == p2->dwLevel)
	{
		if (p1->dwExp > p2->dwExp)return 1;
		if (p1->dwExp < p2->dwExp)return -1;
		return 0;
	}
	return -1;
}

VOID CTitleManager::LoadData(const char* pszData)
{
	CFmtTextFile ftf("d2s64s64s64", pszData, TRUE);
	if (ftf.GetCount() == 0)return;
	m_pTitles = std::make_unique<TITLE[]>(ftf.GetCount());
	for (int i = 0; i < ftf.GetCount(); i++)
	{
		if (ftf.GetStruct(i, &m_pTitles[m_iTitleCount]))
			m_iTitleCount++;
	}
	qsort((VOID*)m_pTitles.get(), m_iTitleCount, sizeof(TITLE), (int(*)(const VOID*, const VOID*)) sort_title);
}

BOOL CTitleManager::GetTitle(CHumanPlayer* player, char* pszTitle)
{
	if (!m_pTitles || m_iTitleCount == 0 || player == nullptr) return FALSE;
	const DWORD dwLevel = player->GetPropValue(PI_LEVEL);
	const DWORD dwExp = player->GetPropValue(PI_EXP);
	const BYTE proIndex = player->GetPro() % 3;

	int left = 0;
	int right = m_iTitleCount - 1;
	int matchIndex = -1;
	while (left <= right)
	{
		int mid = left + (right - left) / 2;
		const TITLE* pTitle = &m_pTitles[mid];

		if (dwLevel > pTitle->dwLevel)
		{
			left = mid + 1;
			matchIndex = mid;
		}
		else if (dwLevel < pTitle->dwLevel)
		{
			right = mid - 1;
		}
		else
		{
			if (dwExp > pTitle->dwExp)
			{
				left = mid + 1;
				matchIndex = mid;
			}
			else
			{
				if (mid <= 0) return FALSE;
				const TITLE* pTargetTitle = &m_pTitles[mid - 1];
				strcpy(pszTitle, pTargetTitle->strTitle[proIndex].data());
				return TRUE;
			}
		}
	}
	if (matchIndex >= 0)
	{
		const TITLE* pTargetTitle = &m_pTitles[matchIndex];
		strcpy(pszTitle, pTargetTitle->strTitle[proIndex].data());
		return TRUE;
	}
	return FALSE;
}

BOOL CTitleManager::GetTitle(CHumanPlayer* player, char* pszTitle, int& index)
{
	if (!m_pTitles || m_iTitleCount == 0 || player == nullptr) return FALSE;
	const DWORD dwLevel = player->GetPropValue(PI_LEVEL);
	const DWORD dwExp = player->GetPropValue(PI_EXP);
	const BYTE proIndex = player->GetPro() % 3;

	int left = 0;
	int right = m_iTitleCount - 1;
	int matchIndex = -1;
	while (left <= right)
	{
		int mid = left + (right - left) / 2;
		const TITLE* pTitle = &m_pTitles[mid];

		if (dwLevel > pTitle->dwLevel)
		{
			left = mid + 1;
			matchIndex = mid;
		}
		else if (dwLevel < pTitle->dwLevel)
		{
			right = mid - 1;
		}
		else
		{
			if (dwExp > pTitle->dwExp)
			{
				left = mid + 1;
				matchIndex = mid;
			}
			else
			{
				if (mid <= 0) return FALSE;

				const TITLE* pTargetTitle = &m_pTitles[mid - 1];
				strcpy(pszTitle, pTargetTitle->strTitle[proIndex].data());
				index = mid - 1;
				return TRUE;
			}
		}
	}
	if (matchIndex >= 0)
	{
		const TITLE* pTargetTitle = &m_pTitles[matchIndex];
		strcpy(pszTitle, pTargetTitle->strTitle[proIndex].data());
		index = matchIndex;
		return TRUE;
	}
	return FALSE;
}