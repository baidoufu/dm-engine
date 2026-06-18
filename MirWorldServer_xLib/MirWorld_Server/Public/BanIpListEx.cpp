#include "stdafx.h"
#include "inc.h"
#include ".\baniplistex.h"

CIpListEx::CIpListEx(VOID)
{
}

CIpListEx::~CIpListEx(VOID)
{
}

BOOL CIpListEx::Load(const char* pszFilename)
{
	THREAD_PROTECT;	//	读取和检测要进行线程保护
	CStringFile sf(pszFilename);

	m_vNormalAddress.clear();
	m_vRangeAddress.clear();
	m_vMaskAddress.clear();

	DWORD dwNormalCount = 0, dwRangeCount = 0, dwMaskCount = 0;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); ++i)
	{
		char* pszAddress = TrimEx(sf[i]);
		if (*pszAddress == 0 ||
			*pszAddress == '#') continue;
		if (strchr(pszAddress, '-') != nullptr)
			++dwRangeCount;
		else if (strchr(pszAddress, ':') != nullptr)
			++dwMaskCount;
		else
			++dwNormalCount;
	}

	if (dwMaskCount > 0)
		m_vMaskAddress.reserve(dwMaskCount);
	if (dwRangeCount > 0)
		m_vRangeAddress.reserve(dwRangeCount);
	if (dwNormalCount > 0)
		m_vNormalAddress.reserve(dwNormalCount);

	for (UINT i = 0; i < (UINT)sf.GetLineCount(); ++i)
	{
		char* pszAddress = TrimEx(sf[i]);
		if (*pszAddress == 0 ||
			*pszAddress == '#') continue;
		char* p = strchr(pszAddress, '-');
		if (p != nullptr)
		{
			*p++ = 0;
			AddRangeAddr((DWORD)ntohl(inet_addr(TrimEx(pszAddress))), (DWORD)ntohl(inet_addr(TrimEx(p))));
		}
		else if ((p = strchr(pszAddress, ':')) != nullptr)
		{
			*p++ = 0;
			AddMaskAddr((DWORD)ntohl(inet_addr(TrimEx(pszAddress))), (DWORD)ntohl(inet_addr(TrimEx(p))));
		}
		else
			AddNormalAddr((DWORD)ntohl(inet_addr(TrimEx(pszAddress))));
	}
	return TRUE;
}

BOOL CIpListEx::AddressIn(const char* pszIpAddress)
{
	THREAD_PROTECT;	//	读取和检测要进行线程保护
	DWORD	dwAddr = (DWORD)ntohl(inet_addr(pszIpAddress));
	if (AddressInRangeAddr(dwAddr))return TRUE;
	if (AddressInNormalAddr(dwAddr))return TRUE;
	if (AddressInMaskAddr(dwAddr))return TRUE;
	return FALSE;
}

//	自动合并重复的范围
BOOL CIpListEx::AddRangeAddr(DWORD dwStart, DWORD dwEnd)
{
	DWORD	d1 = dwStart, d2 = dwEnd;
	dwStart = min(d1, d2);
	dwEnd = max(d1, d2);
	if (dwStart == dwEnd)return AddNormalAddr(dwStart);
	for (auto& range : m_vRangeAddress)
	{
		if (dwEnd < range.dwStart)
		{
			continue;
		}
		else if (dwStart > range.dwEnd)
		{
			continue;
		}

		if (dwEnd > range.dwEnd)
			range.dwEnd = dwEnd;

		if (dwStart < range.dwStart)
			range.dwStart = dwStart;

		return TRUE;
	}
	m_vRangeAddress.push_back({ dwStart, dwEnd });
	return TRUE;
}

BOOL CIpListEx::AddMaskAddr(DWORD dwAddr, DWORD dwMask)
{
	dwAddr &= dwMask;
	for (auto& mask : m_vMaskAddress)
	{
		if (mask.dwAddr == dwAddr)
		{
			if ((dwMask & mask.dwMask) == dwMask)
				return TRUE;
			if ((dwMask & mask.dwMask) == mask.dwMask)
			{
				mask.dwMask = dwMask;
				return TRUE;
			}
		}
	}
	m_vMaskAddress.push_back({ dwAddr, dwMask });
	return TRUE;
}

BOOL CIpListEx::AddNormalAddr(DWORD dwAddr)
{
	// 使用二分查找插入位置保持有序
	auto it = std::lower_bound(m_vNormalAddress.begin(), m_vNormalAddress.end(), dwAddr);
	if (it != m_vNormalAddress.end() && *it == dwAddr)
		return TRUE; // 已存在
	m_vNormalAddress.insert(it, dwAddr);
	return TRUE;
}

BOOL CIpListEx::AddressInNormalAddr(DWORD dwAddr)
{
	return std::binary_search(m_vNormalAddress.begin(), m_vNormalAddress.end(), dwAddr);
}

BOOL CIpListEx::AddressInMaskAddr(DWORD dwAddr)
{
	for (const auto& mask : m_vMaskAddress)
	{
		if (mask.AddrIn(dwAddr))return TRUE;
	}
	return FALSE;
}

BOOL CIpListEx::AddressInRangeAddr(DWORD dwAddr)
{
	for (const auto& range : m_vRangeAddress)
	{
		if (range.AddrIn(dwAddr))return TRUE;
	}
	return FALSE;
}
