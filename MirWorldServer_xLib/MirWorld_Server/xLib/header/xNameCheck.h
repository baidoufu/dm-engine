#pragma once
#include <array>
inline BOOL xCheckCharname(const char* pszName)
{
	const char* p = pszName;
	while (*p)
	{
		if (*p < 0)
		{
			if (*(p + 1) != 0)
			{
				p++;
			}
			else
				return FALSE;
		}
		else
		{
			if (*p <= 32)
				return FALSE;
			if (*p == '\'' || *p == '\"')
				return FALSE;
		}
		p++;
	}
	return TRUE;
}

inline BOOL xCheckAccount(const char* pszName)
{
	int length = (int)strlen(pszName);
	if (length < 3 || length > 10)
		return FALSE;
	return xCheckCharname(pszName);
}

inline BOOL xCheckBirthday(const char* pszBirthday)
{
	std::array<char, 40> szText = {};
	o_strncpy(szText.data(), pszBirthday, 10);
	xStringsExpander<20> birthday(szText.data(), '/');
	if (birthday.getCount() != 3)return FALSE;
	int t = atoi(birthday[0]);
	if (t < 1900 || t > 9999)return FALSE;
	t = StringToInteger(birthday[1]);
	if (t < 1 || t > 12)return FALSE;
	t = StringToInteger(birthday[2]);
	if (t < 1 || t > 31)return FALSE;
	return TRUE;
}

inline BOOL xCheckEmail(const char* pszEmail)
{
	const char* p = strchr(pszEmail, '@');
	if (p == nullptr)return FALSE;
	p = pszEmail;
	while (*p)
	{
		if (*p == '-' || *p == '.' || *p == '@' || *p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))
		{
		}
		else
			return FALSE;
		p++;
	}
	return TRUE;
}

inline BOOL	xCheckTelephone(const char* pszNumber)
{
	const char* p = pszNumber;
	while (*p)
	{
		if ((*p >= '0' && *p <= '9') || *p == '-')
		{
		}
		else
			return FALSE;
		p++;
	}
	return TRUE;
}

inline BOOL xCheckMobilePhone(const char* pszMobile)
{
	const char* p = pszMobile;
	while (*p)
	{
		if ((*p >= '0' && *p <= '9'))
		{
		}
		else
			return FALSE;
		p++;
	}
	return TRUE;
}

inline BOOL xCheckIdCard(const char* pszIdCard)
{
	const char* p = pszIdCard;
	while (*p)
	{
		if ((*p >= '0' && *p <= '9') || *p == 'x' || *p == 'X')
		{
		}
		else
			return FALSE;
		p++;
	}
	return TRUE;
}

inline BOOL xCheckPassword(const char* pszPassword)
{
	const char* p = pszPassword;
	int length = 0;
	while (*p)
	{
		if (*p == '\'' ||
			*p == '\"')
			return FALSE;
		if (*p < 0)
			return FALSE;
		length++;
		p++;
	}
	if (length < 3 || length > 10)return FALSE;
	return TRUE;
}

inline BOOL xCheckNormalString(const char* pszPassword)
{
	const char* p = pszPassword;
	while (*p)
	{
		if (*p == '\'' ||
			*p == '\"')
			return FALSE;
		//if( *p < 0 )
		//	return FALSE;
		p++;
	}
	return TRUE;
}
inline char* xGetValidName(char* pszName, int length)
{
	char* p = pszName;
	int i = 0;
	BOOL bInHz = FALSE;
	while (*p)
	{
		i++;
		if (bInHz)
			bInHz = FALSE;
		else
		{
			if (*p < 0)
				bInHz = TRUE;
			else if (*p == '\'' || *p == '\"')
				*p = '_';
		}
		if (i >= length)
			break;
		p++;
	}

	if (bInHz)
	{
		pszName[i - 1] = 0;
	}
	else
		pszName[i] = 0;
	return pszName;
}
