#include "StdAfx.h"
#include ".\ioconsole.h"
#include "stdio.h"

CIOConsole::CIOConsole(VOID)
{
	m_pInputListener = nullptr;
}

CIOConsole::~CIOConsole(VOID)
{
}

VOID CIOConsole::OutPut(DWORD dwColor, const char* pszString, ...)
{
	if (this == nullptr)return;
	std::array<char, 2048> szBuff{};
	va_list	vl;
	va_start(vl, pszString);
	vsnprintf(szBuff.data(), szBuff.size(), pszString, vl);
	va_end(vl);
	szBuff[szBuff.size() - 1] = 0;
	OutPutStatic(GetColor(dwColor), szBuff.data());
}

DWORD CIOConsole::GetColor(DWORD index)
{
	switch (index)
	{
	case TEXT_WHITE:
		return 0xF8F8F2; // Èá°×É«
	case COOL_BLUE:
		return 0x88C0D0; // ¾²Ú×À¶
	case STRING_GREEN:
		return 0x98C379; // »îÁ¦ÂÌ
	case WARN_YELLOW:
		return 0xE5C07B; // Ã÷ÑÞ»Æ
	case KEYWORD_PINK:
		return 0xFF6188; // Éºº÷³È
	case FUNC_PURPLE:
		return 0xBD93F9; // µ­ÑÅ×Ï
	case CYAN:
		return 0x05D9E8; // Ó«¹âÇà
	case ORANGE:
		return 0xD19A66; // µÍ±¥ºÍ³È
	case SUCCESS_GREEN:
		return 0xFF00; // ÏÊÂÌÉ«
	case ERROR_RED:
		return 0xFF; // ´¿ºìÉ«
	default:
	return index;
	}
}