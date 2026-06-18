#include "StdAfx.h"
#include ".\fmttextfile.h"
#include <array>

CFmtTextFile::CFmtTextFile(const char* pszStruct, const char* pszFile, BOOL bCSV)
{
	int	sptr = 0;
	std::array<char, 200> szNumber{};
	int	nptr = 0;
	int	eptr = 0;
	m_iStructElements = 0;
	m_iMaxParams = 0;

	fmt_struct_element* pTail = nullptr;
	m_pStructDesc = nullptr;

	while (pszStruct[sptr])
	{
		char dtype = tolower(pszStruct[sptr++]);

		nptr = 0;
		while (pszStruct[sptr] >= '0' && pszStruct[sptr] <= '9')
		{
			szNumber[nptr++] = pszStruct[sptr++];
		}
		szNumber[nptr] = 0;
		fmt_type type;

		switch (dtype)
		{
		case 's':
			type = FT_STRING;
			break;
		case 'b':
			type = FT_BYTE;
			break;
		case 'w':
			type = FT_WORD;
			break;
		case 'd':
			type = FT_DWORD;
			break;
		default:
			continue;
		}
		if (pTail == nullptr)
		{
			m_pStructDesc = std::make_unique<fmt_struct_element>();
			pTail = m_pStructDesc.get();
		}
		else
		{
			pTail->pNext = std::make_unique<fmt_struct_element>();
			pTail = pTail->pNext.get();
		}
		pTail->wType = static_cast<WORD>(type);
		pTail->wLength = StringToInteger(szNumber.data());
		if (pTail->wLength == 0) pTail->wLength = 1;
		if (type != FT_STRING)
			m_iMaxParams += pTail->wLength;
		else
			m_iMaxParams += 1;
		m_iStructElements++;
	}
	Params.resize(m_iMaxParams);
	m_bLoaded = Load(pszFile, bCSV);
}

BOOL CFmtTextFile::Load(const char* pszFile, BOOL bCSV)
{
	if (pszFile == nullptr) return FALSE;
	m_sfFile.Destroy();
	if (!m_sfFile.LoadFile(pszFile)) return FALSE;
	m_bCSV = bCSV;
	m_bLoaded = TRUE;
	return TRUE;
}

static std::array<char, 4096> g_szFmtTempLine{};
BOOL CFmtTextFile::GetStruct(int line, LPVOID lpStruct)
{
	if (!m_bLoaded) return FALSE;
	char* p = static_cast<char*>(lpStruct);
	int ptr = 0;
	if (line >= m_sfFile.GetLineCount()) return FALSE;
	if (*m_sfFile[line] == '#') return FALSE;
	o_strncpy(g_szFmtTempLine.data(), m_sfFile[line], 4095);
	nParam = SearchParam(g_szFmtTempLine.data(), Params.data(), m_iMaxParams, (m_bCSV ? "," : "/"));

	int bytewidth = 0;
	if (nParam != m_iMaxParams) return FALSE;

	int pptr = 0;
	fmt_struct_element* pStructElement = m_pStructDesc.get();
	while (pStructElement)
	{
		auto pCurElement = pStructElement;
		pStructElement = pStructElement->pNext.get();
		switch (pCurElement->wType)
		{
		case FT_STRING:
			strncpy(p + ptr, Params[pptr++], pCurElement->wLength);
			ptr += pCurElement->wLength;
			continue;
		case FT_BYTE:
			bytewidth = 1;
			break;
		case FT_WORD:
			bytewidth = 2;
			break;
		case FT_DWORD:
			bytewidth = 4;
			break;
		default:
			continue;
		}
		for (int j = 0; j < pCurElement->wLength; j++)
		{
			if (pptr >= m_iMaxParams) return FALSE;
			int t = StringToInteger(Params[pptr++]);
			memcpy(p + ptr, &t, bytewidth);
			ptr += bytewidth;
		}
	}
	return TRUE;
}