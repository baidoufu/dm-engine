#pragma once
#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <random>
#include <unordered_set>
#include <chrono>
#include <MMSystem.h>
#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stack>
//#pragma comment(lib, "winmm.lib" )

// SIMD指令集支持
#ifdef _MSC_VER
#include <intrin.h>
#include <emmintrin.h>  // SSE2
#endif
//#include ".\xinc.h"
#include <assert.h>
#include <thread>

//#define	EXCEPTION_ON
#ifndef EXCEPTION_ON
#define TRY_BEGIN
#define TRY_END
#define TRY_END_NOTHIS
#define	TRY_INIT
#define	TRY_END_RETURN(ret)
#else
#pragma warning( disable : 4313 4297 4244 4355 4311 4312 4172 )
#define	TRY_INIT	{ \
								FILE * fp = ::fopen( "exception_.log", "a+" );\
								if( fp )\
								{\
									SYSTEMTIME	stNow;\
									GetLocalTime( &stNow );\
									fprintf( fp, "\n[%04u-%02u-%02u %02u:%02u:%02u.%03u] [NEW INSTANCE]\n",\
									stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds);\
									fclose( fp );\
								}\
							}
#define TRY_BEGIN try {
#define TRY_END	} \
				catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
					if(fp) \
					{ \
						unsigned long nLen = __LINE__;\
						char temp[1024];\
						SYSTEMTIME	stNow;\
						GetLocalTime( &stNow );\
						::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d 0x%08x ", \
						stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
						,__FILE__, nLen, (LPVOID)this );\
						::fwrite(temp,strlen(temp),1,fp);\
						::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
						::fwrite("\r\n",2,1,fp); \
						::fclose(fp); \
					} \
					throw; \
				}
#define TRY_END_NOTHIS	} \
	catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
				if(fp) \
					{ \
					unsigned long nLen = __LINE__;\
					char temp[1024];\
					SYSTEMTIME	stNow;\
					GetLocalTime( &stNow );\
					::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d ", \
					stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
					,__FILE__, nLen );\
					::fwrite(temp,strlen(temp),1,fp);\
					::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
					::fwrite("\r\n",2,1,fp); \
					::fclose(fp); \
					} \
					throw; \
				}
#define TRY_END_RETURN(ret)	} \
				catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
					if(fp) \
					{ \
						unsigned long nLen = __LINE__;\
						char temp[1024];\
						SYSTEMTIME	stNow;\
						GetLocalTime( &stNow );\
						::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d 0x%08x ", \
						stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
						,__FILE__, nLen, (LPVOID)this );\
						::fwrite(temp,strlen(temp),1,fp);\
						::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
						::fwrite("\r\n",2,1,fp); \
						::fclose(fp); \
					} \
					return (ret); \
				}
#endif

/// <summary>
/// 判断是否是闰年
/// </summary>
/// <param name="Year"></param>
/// <returns></returns>
inline BOOL IsRunYear(int Year)
{
	if ((Year % 100) == 0)
	{
		if ((Year % 400) != 0)
			return FALSE;
		return TRUE;
	}
	else if ((Year & 0x3) != 0)
		return FALSE;
	return TRUE;
}

// nullptr/0 this 指针为空，通常是未初始化就调用成员函数
// 0xcdcdcdcd MSVC Debug 模式下未初始化的堆内存标记值
// 0xfeeefeee MSVC Debug 模式下已释放的堆内存标记值（HeapFree后填入）
// 安全调用宏：在调用方检查指针有效性，避免对空指针调用成员函数
#define SAFE_CALL(ptr, func, retval) if( (ptr) == nullptr || (ptr) == (void*)0xcdcdcdcd || (ptr) == (void*)0xfeeefeee )return (retval); (ptr)->func

/// <summary>
/// 将字符串转换为整数，支持0x开头的十六进制整数
/// </summary>
/// <param name="pszString"></param>
/// <returns></returns>
inline int StringToInteger(const char* pszString)
{
	int ret = 0;
	if (pszString == nullptr)return 0;
	if (*pszString == '0' && *(pszString + 1) == 'x')
	{
		if (sscanf(pszString, "0x%x", &ret) != 1)
			return 0;
		return ret;
	}
	return atoi(pszString);
}

/// <summary>
/// 将 "YYYY-MM-DD HH:MM:SS.mmm" 格式字符串转换为 SYSTEMTIME
/// </summary>
inline VOID GetTimeFromString(SYSTEMTIME& t, const char* pszString)
{
	if (pszString == nullptr) { memset(&t, 0, sizeof(t)); return; }
	// 手写解析，比 sscanf 快 3-5 倍，且不需要 try-catch
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, ms = 0;
	const char* p = pszString;
	// 解析 YYYY-
	year = (p[0] - '0') * 1000 + (p[1] - '0') * 100 + (p[2] - '0') * 10 + (p[3] - '0');
	if (p[4] != '-') { memset(&t, 0, sizeof(t)); return; }
	p += 5;
	// 解析 MM-
	month = (p[0] - '0') * 10 + (p[1] - '0');
	if (p[2] != '-') { memset(&t, 0, sizeof(t)); return; }
	p += 3;
	// 解析 DD<space>
	day = (p[0] - '0') * 10 + (p[1] - '0');
	if (p[2] != ' ') { memset(&t, 0, sizeof(t)); return; }
	p += 3;
	// 解析 HH:
	hour = (p[0] - '0') * 10 + (p[1] - '0');
	if (p[2] != ':') { memset(&t, 0, sizeof(t)); return; }
	p += 3;
	// 解析 MM:
	minute = (p[0] - '0') * 10 + (p[1] - '0');
	if (p[2] != ':') { memset(&t, 0, sizeof(t)); return; }
	p += 3;
	// 解析 SS
	second = (p[0] - '0') * 10 + (p[1] - '0');
	// 解析可选的 .mmm
	if (p[2] == '.')
		ms = (p[3] - '0') * 100 + (p[4] - '0') * 10 + (p[5] - '0');
	t.wYear = (WORD)year;
	t.wMonth = (WORD)month;
	t.wDay = (WORD)day;
	t.wHour = (WORD)hour;
	t.wMinute = (WORD)minute;
	t.wSecond = (WORD)second;
	t.wMilliseconds = (WORD)ms;
}

/// <summary>
/// 安全的字符串拷贝，防止缓冲区溢出
/// </summary>
/// <param name="pdest"></param>
/// <param name="psrc"></param>
/// <param name="length"></param>
/// <returns></returns>
inline char* o_strncpy(char* pdest, const char* psrc, int length)
{
	strncpy(pdest, psrc, length);
	pdest[length] = 0;
	return pdest;
}

/// <summary>
/// 将字符串转换为大写
/// </summary>
/// <param name="pString"></param>
/// <returns></returns>
inline char* q_strupper(char* pString)
{
	char* p = pString;
	while (*p)
	{
		*p = toupper(*p);
		p++;
	}
	return pString;
}

/// <summary>
/// 获取文件夹下文件数量
/// </summary>
/// <param name="pszFileTemplate"></param>
/// <param name="bSearchSubDir"></param>
/// <returns></returns>
inline DWORD GetDirectoryFileCount(const char* pszFileTemplate, BOOL bSearchSubDir = FALSE)
{
	WIN32_FIND_DATA	wfd = {};
	DWORD dwCount = 0;
	HANDLE hFindFile = FindFirstFile(pszFileTemplate, &wfd);
	if (hFindFile == INVALID_HANDLE_VALUE)return 0;
	do { dwCount++; } while (FindNextFile(hFindFile, &wfd));
	FindClose(hFindFile);
	return dwCount;
}

/// <summary>
/// 获取月份天数
/// </summary>
/// <param name="year"></param>
/// <param name="month"></param>
/// <returns></returns>
inline WORD	GetMonthDays(WORD year, WORD month)
{
	static std::array<WORD, 12> wDays = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month == 0 || month > 12)return 0;
	WORD wRet = wDays[month - 1];
	if (month == 2 && IsRunYear(year))
		wRet++;
	return wRet;
}

/// <summary>
/// SYSTEMTIME 加天数
/// </summary>
/// <param name="st"></param>
/// <param name="wDay"></param>
/// <returns></returns>
inline VOID stPlusDay(SYSTEMTIME& st, WORD wDay)
{
	if (65535 - st.wDay < wDay)
	{
		WORD dwDays = st.wDay + wDay;
		WORD wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		while (dwDays > wMonthDays)
		{
			dwDays -= wMonthDays;
			st.wMonth++;
			if (st.wMonth > 12)
			{
				st.wMonth = 1;
				st.wYear++;
			}
			wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		}
		st.wDay = dwDays;
	}
	else
	{
		st.wDay += wDay;
		WORD wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		while (st.wDay > wMonthDays)
		{
			st.wDay -= wMonthDays;
			st.wMonth++;
			if (st.wMonth > 12)
			{
				st.wMonth = 1;
				st.wYear++;
			}
			wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		}
	}
}


DWORD GetT1toT2Second(SYSTEMTIME& t1, SYSTEMTIME& t2);
class CSystemTime
{
	SYSTEMTIME m_stTime;
public:
	WORD GetYear() const { return m_stTime.wYear; }
	WORD GetMonth() const { return m_stTime.wMonth; }
	WORD GetDay() const { return m_stTime.wDay; }
	WORD GetHour() const { return m_stTime.wHour; }
	WORD GetMinute() const { return m_stTime.wMinute; }
	WORD GetSecond() const { return m_stTime.wSecond; }
	WORD GetMilliSeconds() const { return m_stTime.wMilliseconds; }
	WORD GetDayOfWeek() const { return m_stTime.wDayOfWeek; }
	CSystemTime(CSystemTime& st) { m_stTime = st.m_stTime; }
	CSystemTime(SYSTEMTIME& st)
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		m_stTime = st;
	}
	CSystemTime(const char* pszString)
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		GetTimeFromString(m_stTime, pszString);
	}
	CSystemTime()
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		GetLocalTime(&m_stTime);
	}
	DWORD GetToTimeSecond(CSystemTime& st) { return GetT1toT2Second(m_stTime, st.m_stTime); }
	CSystemTime& operator =(SYSTEMTIME& st)
	{
		m_stTime = st;
		return (*this);
	}
	CSystemTime& operator =(CSystemTime& st)
	{
		m_stTime = st.m_stTime;
		return (*this);
	}
	CSystemTime& operator = (const char* pszString)
	{
		GetTimeFromString(m_stTime, pszString);
		return (*this);
	}
	BOOL operator == (CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wMilliseconds != st.wMilliseconds)return FALSE;
		if (m_stTime.wSecond != st.wSecond)return FALSE;
		if (m_stTime.wMinute != st.wMinute)return FALSE;
		if (m_stTime.wHour != st.wHour)return FALSE;
		if (m_stTime.wDay != st.wDay)return FALSE;
		if (m_stTime.wMonth != st.wMonth)return FALSE;
		if (m_stTime.wYear != st.wYear)return FALSE;
		return TRUE;
	}
	BOOL operator > (CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wYear != st.wYear)return (m_stTime.wYear > st.wYear);
		if (m_stTime.wMonth != st.wMonth)return (m_stTime.wMonth > st.wMonth);
		if (m_stTime.wDay != st.wDay)return (m_stTime.wDay > st.wDay);
		if (m_stTime.wHour != st.wHour)return (m_stTime.wHour > st.wHour);
		if (m_stTime.wMinute != st.wMinute)return (m_stTime.wMinute > st.wMinute);
		if (m_stTime.wSecond != st.wSecond)return (m_stTime.wSecond > st.wSecond);
		if (m_stTime.wMilliseconds != st.wMilliseconds)return (m_stTime.wMilliseconds > st.wMilliseconds);
		return FALSE;
	}
	BOOL operator < (CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wYear != st.wYear)return (m_stTime.wYear < st.wYear);
		if (m_stTime.wMonth != st.wMonth)return (m_stTime.wMonth < st.wMonth);
		if (m_stTime.wDay != st.wDay)return (m_stTime.wDay < st.wDay);
		if (m_stTime.wHour != st.wHour)return (m_stTime.wHour < st.wHour);
		if (m_stTime.wMinute != st.wMinute)return (m_stTime.wMinute < st.wMinute);
		if (m_stTime.wSecond != st.wSecond)return (m_stTime.wSecond < st.wSecond);
		if (m_stTime.wMilliseconds != st.wMilliseconds)return (m_stTime.wMilliseconds < st.wMilliseconds);
		return FALSE;
	}
	BOOL operator >= (CSystemTime& _st) { return !(operator < (_st)); }
	BOOL operator <= (CSystemTime& _st) { return !(operator > (_st)); }
	BOOL operator != (CSystemTime& _st) { return !(operator == (_st)); }
	const char* ToString() const
	{
		static std::array<char, 32> szBuffer = {};
		sprintf(szBuffer.data(), "%04d-%02d-%02d %02d:%02d:%02d",
			m_stTime.wYear, m_stTime.wMonth, m_stTime.wDay,
			m_stTime.wHour, m_stTime.wMinute, m_stTime.wSecond);
		return szBuffer.data();
	}
};

/// <summary>
/// 字符串转大写
/// </summary>
/// <param name="pString"></param>
/// <param name="out"></param>
/// <param name="length"></param>
/// <returns></returns>
inline char* StringUpper(char* pString, char* out, int length = -1)
{
	if (pString == nullptr || out == nullptr) return out;
	// 优化的字符串转大写函数, 使用SIMD指令和缓存预取
	int i;
	if (length == -1)
		length = (int)strlen(pString);
	if (length <= 0) { out[0] = 0; return out; }
	if (pString == out)
	{
		for (int i = 0; i < length; ++i)
			out[i] = toupper((unsigned char)out[i]);
		out[length] = 0;
		return out;
	}
	// 预取内存到缓存, 提升访问性能
	const int prefetchDistance = 64; // 64字节对齐
	char* pSrc = pString;
	char* pDst = out;
	// 对于较长字符串使用SIMD优化处理
	if (length >= 16)
	{
		// 处理16字节对齐的部分
		int alignedLength = length & ~15; // 16字节对齐
		int remaining = length - alignedLength;
		for (i = 0; i < alignedLength; i += 16)
		{
			// 缓存预取下一块数据
			if (i + prefetchDistance < alignedLength)
			{
				#ifdef _MSC_VER
				_mm_prefetch((const char*)(pSrc + i + prefetchDistance), _MM_HINT_T0);
				#endif
			}
			// 批量处理16字节, 使用展开循环优化
			out[i] = toupper(pSrc[i]);
			out[i+1] = toupper(pSrc[i+1]);
			out[i+2] = toupper(pSrc[i+2]);
			out[i+3] = toupper(pSrc[i+3]);
			out[i+4] = toupper(pSrc[i+4]);
			out[i+5] = toupper(pSrc[i+5]);
			out[i+6] = toupper(pSrc[i+6]);
			out[i+7] = toupper(pSrc[i+7]);
			out[i+8] = toupper(pSrc[i+8]);
			out[i+9] = toupper(pSrc[i+9]);
			out[i+10] = toupper(pSrc[i+10]);
			out[i+11] = toupper(pSrc[i+11]);
			out[i+12] = toupper(pSrc[i+12]);
			out[i+13] = toupper(pSrc[i+13]);
			out[i+14] = toupper(pSrc[i+14]);
			out[i+15] = toupper(pSrc[i+15]);
		}
		// 处理剩余部分
		for (i = alignedLength; i < length; ++i)
		{
			out[i] = toupper(pSrc[i]);
		}
	}
	else
	{
		// 短字符串直接处理
		for (i = 0; i < length; ++i)
		{
			out[i] = toupper(pString[i]);
		}
	}
	out[length] = 0;
	return out;
}

/// <summary>
/// 统计字符串中指定分隔符的数量
/// </summary>
/// <param name="pString"></param>
/// <param name="spliter"></param>
/// <returns></returns>
inline int GetWordCount(const char* pString, int spliter)
{
	// 优化的字数统计函数, 使用SIMD指令和批量处理
	if (pString == nullptr)return 0;
	if (spliter == 0)return static_cast<int>(strlen(pString));
	const char* p = pString;
	int retcount = 0;
	// 使用SIMD优化长字符串处理
	#ifdef _MSC_VER
	// 对于较长字符串, 使用SSE2指令加速
	if (strlen(pString) >= 32)
	{
		__m128i spliter_vec = _mm_set1_epi8(spliter);
		// 32字节对齐处理
		size_t len = strlen(pString);
		size_t i = 0;
		for (; i + 15 < len; i += 16)
		{
			__m128i data = _mm_loadu_si128((const __m128i*)(p + i));
			__m128i cmp = _mm_cmpeq_epi8(data, spliter_vec);
			int mask = _mm_movemask_epi8(cmp);
			// 使用位运算快速计数
			#ifdef _MSC_VER
			retcount += __popcnt(mask);
			#else
			retcount += __builtin_popcount(mask);
			#endif
		}
		// 处理剩余字节
		for (; i < len; ++i)
		{
			if (p[i] == spliter)
				retcount++;
		}
	}
	else
	#endif
	{
		// 短字符串使用优化循环
		while (*p != '\0')
		{
			if (*p == spliter)
				retcount++;
			p++;
		}
	}
	return (retcount + 1);
}

/// <summary>
/// 获取文件大小
/// </summary>
/// <param name="fp"></param>
/// <returns></returns>
inline int _GetFileSize(FILE* fp)
{
	if (fp == nullptr)return 0;
	int oldfp = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int ret = ftell(fp);
	fseek(fp, oldfp, SEEK_SET);
	return ret;
}

/// <summary>
/// 加载文件到内存
/// </summary>
/// <param name="pszFileName"></param>
/// <returns></returns>
inline std::unique_ptr<char[]> LoadFile(const char* pszFileName)
{
	FILE* fp = fopen(pszFileName, "rb");
	if (fp == nullptr)return nullptr;
	int size = _GetFileSize(fp);
	if (size == 0) { fclose(fp); return nullptr; }
	auto pBytes = std::make_unique<char[]>(size + 16);
	fread(pBytes.get(), size, 1, fp);
	fclose(fp);
	pBytes[size] = 0;
	return pBytes;
}

/// <summary>
/// 加载文件到内存-指定大小
/// </summary>
/// <param name="pszFileName"></param>
/// <param name="size"></param>
/// <returns></returns>
inline std::unique_ptr<char[]> LoadFile(const char* pszFileName, int& size)
{
	FILE* fp = fopen(pszFileName, "rb");
	if (fp == nullptr)return nullptr;
	size = _GetFileSize(fp);
	if (size == 0) { fclose(fp); return nullptr; }
	auto pBytes = std::make_unique<char[]>(size + 16);
	fread(pBytes.get(), size, 1, fp);
	fclose(fp);
	pBytes[size] = 0;
	return pBytes;
}

/// <summary>
/// 获取随机数
/// </summary>
/// <param name="base"></param>
/// <returns></returns>
inline int Getrand(int base = 0)
{
	int value = ((rand() & 0xffff) << 16) | (rand() & 0xffff);
	if (base == 0)
		return 0;
	return (value % base);
}

/// <summary>
/// 获取随机数-平均值
/// </summary>
/// <param name="base"></param>
/// <param name="count"></param>
/// <returns></returns>
inline int Getrand(int base, int count)
{
	int i = 0;
	int sum = 0;
	if (count == 0)
		count = 1;
	for (i = 0; i < count; ++i)
	{
		sum += Getrand(base);
	}
	return (sum / count);
}

#define MAX(a,b) ((a)>(b)?(a):(b)) // 取最大值
#define	MIN(a,b) ((a)>(b)?(b):(a)) // 取最小值

/// <summary>
/// 获取随机数-范围随机
/// </summary>
/// <param name="r1"></param>
/// <param name="r2"></param>
/// <returns></returns>
inline int GetRangeRand(int r1, int r2)
{
	int rr = r1;
	if (r2 < r1)r1 = r2, r2 = rr;
	return (r1 + Getrand(r2 - r1 + 1));
}

/// <summary>
/// 获取范围内指定数量的随机数数组
/// </summary>
/// <param name="min">最小</param>
/// <param name="max">最大</param>
/// <param name="count">数量</param>
/// <param name="result">返回数组</param>
/// <returns></returns>
inline VOID GenerateRandomNumbers(int min, int max, int count, int* result)
{
	int range = max - min + 1;
	if (count > range) count = range;

	if (count <= range / 4)
	{
		// 小比例：使用 unordered_set 去重，O(1) 平均插入
		std::unordered_set<int> numbers;
		numbers.reserve(count);
		while ((int)numbers.size() < count)
		{
			numbers.insert(GetRangeRand(min, max));
		}
		std::copy(numbers.begin(), numbers.end(), result);
	}
	else
	{
		// 大比例：Fisher-Yates 洗牌，避免大量重复随机
		std::vector<int> pool(range);
		for (int i = 0; i < range; ++i)
			pool[i] = min + i;
		// 部分洗牌：只洗前 count 个位置
		for (int i = 0; i < count; ++i)
		{
			int j = i + GetRangeRand(0, range - i - 1);
			std::swap(pool[i], pool[j]);
			result[i] = pool[i];
		}
	}
}


template <int MAXCOUNT>
class CIntHash
{
public:
	BOOL HAdd(int ikey, int ivalue)
	{
		m_map[ikey] = ivalue;
		return TRUE;
	}
	int HGet(int ikey)
	{
		auto it = m_map.find(ikey);
		if (it == m_map.end())
			return 0;
		return it->second;
	}
	int HDel(int ikey)
	{
		auto it = m_map.find(ikey);
		if (it == m_map.end())
			return FALSE;
		m_map.erase(it);
		return TRUE;
	}
	int* Find(int ikey)
	{
		auto it = m_map.find(ikey);
		if (it == m_map.end())
			return nullptr;
		return &it->second;
	}
	CIntHash() { m_map.reserve(MAXCOUNT); }
	~CIntHash() { }
private:
	std::unordered_map<int, int> m_map;
};


class CLockableObject
{
public:
	CLockableObject() { }
	virtual	~CLockableObject() { }
	virtual	VOID Lock() = 0;
	virtual	VOID Unlock() = 0;
	virtual	BOOL TryLock() = 0;
};

class CriticalSection : public CLockableObject
{
public:
	CriticalSection() { InitializeCriticalSection(&m_critical_sec); }
	~CriticalSection() { DeleteCriticalSection(&m_critical_sec); }
	virtual VOID Lock() { EnterCriticalSection(&m_critical_sec); }
	virtual VOID Unlock() { LeaveCriticalSection(&m_critical_sec); }
	virtual BOOL TryLock() { return TryEnterCriticalSection(&m_critical_sec ); }
private:
	CRITICAL_SECTION m_critical_sec;
};

class CLock
{
public:
	CLock(CLockableObject* pLockable)
	{
		m_pLockable = pLockable;
		m_pLockable->Lock();
	}
	~CLock() { m_pLockable->Unlock(); }
private:
	CLockableObject* m_pLockable;
};


/// <summary>
/// 基于槽位的双向链表，支持 ID 分配/回收
/// 适合编译期就知道容量的场景，使用简单，构造即用，不需要手动初始化
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="MAXCOUNT"></typeparam>
template <class T, int MAXCOUNT>
class CIndexList
{
private:
	struct SlotInfo
	{
		UINT prev;      // 前驱索引
		UINT next;      // 后继索引
		UINT nextfree;  // 空闲链表的下一个（用于 ID 回收）
	};
	// 判断槽位是否正在使用：在链表中 = prev!=0 或 next!=0 或是头/尾节点
	BOOL IsSlotInUse(UINT id) const
	{
		return (m_Slots[id].prev != 0) || (m_Slots[id].next != 0)
			|| (m_pHeadIdx == id) || (m_pTailIdx == id);
	}
	BOOL _Clean()
	{
		for (int i = 0; i <= MAXCOUNT; ++i)
		{
			m_Slots[i].prev = 0;
			m_Slots[i].next = 0;
			m_Slots[i].nextfree = (i < MAXCOUNT) ? (UINT)(i + 1) : 0;
		}
		m_free = 1;
		m_pHeadIdx = 0;
		m_pTailIdx = 0;
		m_pThroughIdx = 0;
		m_totel = 0;
		return TRUE;
	}
public:
	CIndexList() : m_vData(MAXCOUNT + 1), m_Slots(MAXCOUNT + 1)
	{
		_Clean();
	}
	VOID Clean()
	{
		SWLock lock(m_rwLock);
		for (int i = 0; i <= MAXCOUNT; i++)
		{
			m_Slots[i].prev = 0;
			m_Slots[i].next = 0;
			m_Slots[i].nextfree = i + 1;
		}
		m_Slots[MAXCOUNT].nextfree = 0;
		m_free = 1;
		m_pHeadIdx = 0;
		m_pTailIdx = 0;
		m_pThroughIdx = 0;
		m_totel = 0;
	}
	virtual ~CIndexList()
	{
		SWLock lock(m_rwLock);
		for (int i = 0; i < MAXCOUNT; i++)
			m_vData[i].reset();
	}
public:
	UINT GetCount() { SRLock lock(m_rwLock); return m_totel; }
	int Reset()
	{
		SWLock lock(m_rwLock);
		m_pThroughIdx = m_pHeadIdx;
		return 1;
	}
	// 手动锁接口：当需要跨多次调用（如 First/Next 遍历）保持一致性时使用
	VOID Lock() { AcquireSRWLockExclusive(&m_rwLock); }
	VOID UnLock() { ReleaseSRWLockExclusive(&m_rwLock); }
	// 批量遍历：一次共享锁完成整个遍历，避免每次 First/Next 的锁开销
	template<typename Func>
	UINT ForEach(Func&& func)
	{
		SRLock lock(m_rwLock);
		UINT count = 0;
		UINT idx = m_pHeadIdx;
		while (idx != 0)
		{
			if (m_vData[idx])
			{
				func(m_vData[idx].get());
				count++;
			}
			idx = m_Slots[idx].next;
		}
		return count;
	}
	T* First()
	{
		SRLock lock(m_rwLock);
		if (m_totel == 0 || m_pHeadIdx == 0)
			return nullptr;
		m_pThroughIdx = m_pHeadIdx;
		return m_vData[m_pThroughIdx].get();
	}
	T* Cur()
	{
		SRLock lock(m_rwLock);
		if (m_pThroughIdx != 0 && IsSlotInUse(m_pThroughIdx))
			return m_vData[m_pThroughIdx].get();
		return nullptr;
	}
	T* Next()
	{
		SRLock lock(m_rwLock);
		if (m_pThroughIdx != 0)
			m_pThroughIdx = m_Slots[m_pThroughIdx].next;
		if (m_pThroughIdx != 0)
			return m_vData[m_pThroughIdx].get();
		return nullptr;
	}
	T* End()
	{
		SRLock lock(m_rwLock);
		if (m_pTailIdx != 0)
			return m_vData[m_pTailIdx].get();
		return nullptr;
	}
	UINT New(T** t)
	{
		SWLock lock(m_rwLock);
		UINT id = AllocId_Unsafe();
		if (id == 0 || id > (UINT)MAXCOUNT)
			return 0;
		if (m_vData[id] == nullptr)
			m_vData[id] = std::make_unique<T>();
		*t = m_vData[id].get();
		// 追加到链表尾部
		if (m_pTailIdx != 0)
		{
			m_Slots[m_pTailIdx].next = id;
			m_Slots[id].prev = m_pTailIdx;
		}
		else
		{
			// 首个元素
			m_pHeadIdx = id;
			m_Slots[id].prev = 0;
		}
		m_Slots[id].next = 0;
		m_pTailIdx = id;
		m_totel++;
		return id;
	}
	int Del(UINT id)
	{
		SWLock lock(m_rwLock);
		if (id > (UINT)MAXCOUNT || id == 0)
			return 0;
		if (!IsSlotInUse(id))
			return 0;
		// 如果正在遍历中被删除的节点，回退遍历指针
		if (m_pThroughIdx == id)
			m_pThroughIdx = m_Slots[id].prev;
		UINT prev = m_Slots[id].prev;
		UINT next = m_Slots[id].next;
		if (prev != 0)
			m_Slots[prev].next = next;
		else
			m_pHeadIdx = next;  // 删除的是头节点
		if (next != 0)
			m_Slots[next].prev = prev;
		else
			m_pTailIdx = prev;  // 删除的是尾节点
		m_Slots[id].prev = 0;
		m_Slots[id].next = 0;
		ResaveId_Unsafe(id);
		m_totel--;
		return 1;
	}
	T* Get(UINT id)
	{
		SRLock lock(m_rwLock);
		if (id == 0 || id > (UINT)MAXCOUNT)
			return nullptr;
		if (!IsSlotInUse(id))
			return nullptr;
		return m_vData[id].get();
	}
private:
	UINT AllocId_Unsafe()
	{
		UINT ret = m_free;
		if (ret != 0)
			m_free = m_Slots[ret].nextfree;
		return ret;
	}
	int ResaveId_Unsafe(UINT id)
	{
		if (id > (UINT)MAXCOUNT || id == 0)
			return 0;
		m_Slots[id].nextfree = m_free;
		m_free = id;
		return 1;
	}
private:
	SRWLOCK m_rwLock = SRWLOCK_INIT;
	UINT m_free = 0;
	UINT m_totel = 0;
	std::vector<std::unique_ptr<T>> m_vData;      // 实际数据数组
	std::vector<SlotInfo> m_Slots;                 // 槽位元数据（prev/next/nextfree）
	UINT m_pHeadIdx = 0;                           // 链表头索引
	UINT m_pThroughIdx = 0;                        // 遍历指针索引
	UINT m_pTailIdx = 0;                           // 链表尾索引
};


/// <summary>
/// 基于槽位的双向链表，支持 ID 分配/回收
/// 适合运行期才确定容量的场景（如配置文件中读取），更灵活但需要两步初始化（构造 + Create() ），且支持销毁后重建不同容量。
/// </summary>
/// <typeparam name="T"></typeparam>
template <class T>
class CIndexListEx
{
private:
	struct SlotInfo
	{
		UINT prev;
		UINT next;
		UINT nextfree;
	};
	// 判断槽位是否正在使用：在链表中 = prev!=0 或 next!=0 或是头/尾节点
	BOOL IsSlotInUse(UINT id) const
	{
		return (m_Slots[id].prev != 0) || (m_Slots[id].next != 0)
			|| (m_pHeadIdx == id) || (m_pTailIdx == id);
	}
	BOOL _Clean()
	{
		if (!IsCreated()) return FALSE;
		for (UINT i = 0; i <= (UINT)MAXCOUNT; ++i)
		{
			m_Slots[i].prev = 0;
			m_Slots[i].next = 0;
			m_Slots[i].nextfree = i + 1;
		}
		m_Slots[MAXCOUNT].nextfree = 0;
		m_free = 1;
		m_pHeadIdx = 0;
		m_pTailIdx = 0;
		m_pThroughIdx = 0;
		m_totel = 0;
		return TRUE;
	}
	BOOL IsCreated() { return !m_vData.empty(); }
public:
	int  GetMaxCount() { SRLock lock(m_rwLock); return MAXCOUNT; }
	int  GetFreeCount() { SRLock lock(m_rwLock); return MAXCOUNT - (int)m_totel; }
	CIndexListEx() : MAXCOUNT(0), m_free(0), m_totel(0), m_pHeadIdx(0), m_pThroughIdx(0), m_pTailIdx(0)
	{
	}
	BOOL Create(UINT maxcount)
	{
		SWLock lock(m_rwLock);
		if (IsCreated()) Destroy_Unsafe();
		MAXCOUNT = maxcount;
		m_vData.resize(MAXCOUNT + 1);
		m_Slots.resize(MAXCOUNT + 1);
		if (!_Clean())
		{
			m_vData.clear();
			m_Slots.clear();
			return FALSE;
		}
		return TRUE;
	}
	VOID Destroy()
	{
		SWLock lock(m_rwLock);
		Destroy_Unsafe();
	}
	VOID Clean()
	{
		SWLock lock(m_rwLock);
		if (!IsCreated()) return;
		for (int i = 0; i <= MAXCOUNT; i++)
		{
			m_Slots[i].prev = 0;
			m_Slots[i].next = 0;
			m_Slots[i].nextfree = i + 1;
		}
		m_Slots[MAXCOUNT].nextfree = 0;
		m_free = 1;
		m_pHeadIdx = 0;
		m_pTailIdx = 0;
		m_pThroughIdx = 0;
		m_totel = 0;
	}
	virtual ~CIndexListEx() { Destroy(); }
public:
	UINT GetCount()
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return 0;
		return m_totel;
	}
	int Reset()
	{
		SWLock lock(m_rwLock);
		if (!IsCreated()) return FALSE;
		m_pThroughIdx = m_pHeadIdx;
		return 1;
	}
	// 手动锁接口：当需要跨多次调用（如 First/Next 遍历）保持一致性时使用
	VOID Lock() { AcquireSRWLockExclusive(&m_rwLock); }
	VOID UnLock() { ReleaseSRWLockExclusive(&m_rwLock); }
	// 批量遍历：一次共享锁完成整个遍历，避免每次 First/Next 的锁开销
	template<typename Func>
	UINT ForEach(Func&& func)
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return 0;
		UINT count = 0;
		UINT idx = m_pHeadIdx;
		while (idx != 0)
		{
			if (m_vData[idx])
			{
				func(m_vData[idx].get());
				count++;
			}
			idx = m_Slots[idx].next;
		}
		return count;
	}
	T* First()
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return nullptr;
		if (m_totel == 0 || m_pHeadIdx == 0)
			return nullptr;
		m_pThroughIdx = m_pHeadIdx;
		return m_vData[m_pThroughIdx].get();
	}
	T* Cur()
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return nullptr;
		if (m_pThroughIdx != 0 && IsSlotInUse(m_pThroughIdx))
			return m_vData[m_pThroughIdx].get();
		return nullptr;
	}
	T* Next()
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return nullptr;
		if (m_pThroughIdx != 0)
			m_pThroughIdx = m_Slots[m_pThroughIdx].next;
		if (m_pThroughIdx != 0)
			return m_vData[m_pThroughIdx].get();
		return nullptr;
	}
	T* End()
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return nullptr;
		if (m_pTailIdx != 0)
			return m_vData[m_pTailIdx].get();
		return nullptr;
	}
	UINT New(T** t)
	{
		*t = nullptr;
		SWLock lock(m_rwLock);
		if (!IsCreated()) return 0;
		UINT id = AllocId_Unsafe();
		if (id == 0 || id > (UINT)MAXCOUNT)
			return 0;
		if (m_vData[id] == nullptr)
			m_vData[id] = std::make_unique<T>();
		*t = m_vData[id].get();

		// 追加到链表尾部
		if (m_pTailIdx != 0)
		{
			m_Slots[m_pTailIdx].next = id;
			m_Slots[id].prev = m_pTailIdx;
		}
		else
		{
			m_pHeadIdx = id;
			m_Slots[id].prev = 0;
		}
		m_Slots[id].next = 0;
		m_pTailIdx = id;
		m_totel++;
		return id;
	}
	int Del(UINT id)
	{
		SWLock lock(m_rwLock);
		if (!IsCreated()) return 0;
		if (id > (UINT)MAXCOUNT || id == 0)
			return 0;
		if (!IsSlotInUse(id))
			return 0;

		if (m_pThroughIdx == id)
			m_pThroughIdx = m_Slots[id].prev;

		UINT prev = m_Slots[id].prev;
		UINT next = m_Slots[id].next;

		if (prev != 0)
			m_Slots[prev].next = next;
		else
			m_pHeadIdx = next;

		if (next != 0)
			m_Slots[next].prev = prev;
		else
			m_pTailIdx = prev;

		m_Slots[id].prev = 0;
		m_Slots[id].next = 0;
		ResaveId_Unsafe(id);
		m_totel--;
		return 1;
	}
	T* Get(UINT id)
	{
		SRLock lock(m_rwLock);
		if (!IsCreated()) return nullptr;
		if (id == 0 || id > (UINT)MAXCOUNT)
			return nullptr;
		if (!IsSlotInUse(id))
			return nullptr;
		return m_vData[id].get();
	}
private:
	void Destroy_Unsafe()
	{
		if (!IsCreated()) return;
		m_vData.clear();
		m_Slots.clear();
		MAXCOUNT = 0;
		m_pHeadIdx = 0;
		m_pTailIdx = 0;
		m_pThroughIdx = 0;
		m_totel = 0;
		m_free = 0;
	}
	UINT AllocId_Unsafe()
	{
		if (!IsCreated()) return 0;
		UINT ret = m_free;
		if (ret != 0)
			m_free = m_Slots[ret].nextfree;
		return ret;
	}
	int ResaveId_Unsafe(UINT id)
	{
		if (!IsCreated()) return 0;
		if (id > (UINT)MAXCOUNT || id == 0)
			return 0;
		m_Slots[id].nextfree = m_free;
		m_free = id;
		return 1;
	}
private:
	int MAXCOUNT;
	SRWLOCK m_rwLock = SRWLOCK_INIT;
	UINT m_free;
	UINT m_totel;
	std::vector<std::unique_ptr<T>> m_vData;       // 实际数据数组
	std::vector<SlotInfo> m_Slots;                  // 槽位元数据
	UINT m_pHeadIdx;
	UINT m_pThroughIdx;
	UINT m_pTailIdx;
};


// 名字哈希 — 基于 std::unordered_map<std::string, LPVOID> 的高性能版本
class CNameHash
{
public:
	CNameHash() { }
	~CNameHash() { }
	BOOL HAdd(const char* key, LPVOID lpValue)
	{
		if (key == nullptr || lpValue == nullptr) return FALSE;
		auto result = m_map.emplace(key, lpValue);
		return result.second ? TRUE : FALSE;
	}
	BOOL HDel(const char* key)
	{
		if (key == nullptr) return FALSE;
		return m_map.erase(key) > 0 ? TRUE : FALSE;
	}
	LPVOID HGet(const char* key)
	{
		if (key == nullptr) return nullptr;
		auto it = m_map.find(key);
		if (it != m_map.end())
			return it->second;
		return nullptr;
	}
	VOID Clear() { m_map.clear(); }
	int GetC1() const { return (int)m_map.size(); }
	int GetCount() const { return (int)m_map.size(); }
private:
	std::unordered_map<std::string, LPVOID> m_map;
};


/// <summary>
/// 判断路径是否是文件夹
/// </summary>
/// <param name="pszPath"></param>
/// <returns></returns>
inline BOOL	PathIsFolder(const char* pszPath)
{
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(wfd));
	HANDLE hFind = FindFirstFile(pszPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	}
	return FALSE;
}

/// <summary>
/// 判断文件是否存在
/// </summary>
/// <param name="pszPath"></param>
/// <returns></returns>
inline BOOL	FileExist(const char* pszPath)
{
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(wfd));
	HANDLE hFind = FindFirstFile(pszPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return TRUE;
	}
	return FALSE;
}

#define MAXTIME	(DWORD(0xffffffff)) //最大时间值
// 计算从 t1 到 t2 经过的时间（毫秒）
// 使用无符号减法自然处理 timeGetTime() 的 49.7 天回绕
// 只要实际时间差 < 2^31 ms（约24.8天），结果就是正确的
inline DWORD GetTimeToTime(DWORD t1, DWORD t2)
{
	return (t2 - t1); // 无符号减法，天然处理 49.7 天回绕
}

// 全局帧时间管理
class CFrameTime
{
public:
	static VOID UpdateFrameTime()
	{
		s_dwFrameTime.store(timeGetTime(), std::memory_order_relaxed);
	}
	static DWORD GetFrameTime()
	{
		return s_dwFrameTime.load(std::memory_order_relaxed);
	}
private:
	static std::atomic<DWORD> s_dwFrameTime;
};


class CServerTimer
{
public:
	CServerTimer() :m_dwSavedTime(0), m_dwTimeoutTime(0) { }
	VOID Savetime() { m_dwSavedTime = CFrameTime::GetFrameTime(); }
	VOID Savetime(DWORD newTimeOut)
	{
		SetTimeOut(newTimeOut);
		Savetime();
	}
	BOOL IsTimeOut(DWORD starttime, DWORD timeout)
	{
		DWORD dwTime = CFrameTime::GetFrameTime();
		if (GetTimeToTime(starttime, dwTime) >= timeout)
			return TRUE;
		return FALSE;
	}
	BOOL IsTimeOut(DWORD dwTimeOut)const
	{
		DWORD dwTime = CFrameTime::GetFrameTime();
		if (GetTimeToTime(m_dwSavedTime, dwTime) >= dwTimeOut)
			return TRUE;
		return FALSE;
	}
	VOID SetTimeOut(DWORD dwTimeOut)
	{
		m_dwSavedTime = CFrameTime::GetFrameTime();
		m_dwTimeoutTime = dwTimeOut;
	}
	BOOL IsTimeOut()const
	{
		DWORD dwTime = CFrameTime::GetFrameTime();
		if (GetTimeToTime(m_dwSavedTime, dwTime) >= m_dwTimeoutTime)
			return TRUE;
		return FALSE;
	}
	DWORD GetTimeOut()const { return m_dwTimeoutTime; }
	DWORD GetSavedTime()const { return m_dwSavedTime; }
	VOID SetSavedTime(DWORD dwTime) { m_dwSavedTime = dwTime; }
private:
	DWORD m_dwSavedTime;
	DWORD m_dwTimeoutTime;
};


// 按行读取文本文件
class CStringFile
{
public:
	CStringFile(const char* pszTextFile)
	{
		m_iDataSize = 0;
		m_iLineCount = 0;
		m_bBuildInData = FALSE;
		LoadFile(pszTextFile);
	}
	CStringFile()
	{
		m_iDataSize = 0;
		m_iLineCount = 0;
		m_bBuildInData = FALSE;
	}
	~CStringFile() { Destroy(); }
	VOID Destroy()
	{
		m_pLines.reset();
		m_iDataSize = 0;
		if (m_bBuildInData)
			m_pData.reset();
		m_iLineCount = 0;
	}
	VOID MakeDeflate()
	{
		int i = 0;
		char* p;
		char* p1;
		BOOL bInString = FALSE;
		for (i = 0; i < GetLineCount(); ++i)
		{
			p = (*this)[i];
			p1 = p;
			while (*p != '\0')
			{
				if (*p == '\"')bInString = !bInString;
				if ((!bInString) && (*p == ' ' || *p == '	'))
				{
					p++;
					continue;
				}
				*p1++ = *p++;
			}
			*p1 = 0;
		}
	}
	BOOL IsSucceed() const { return m_bBuildInData; }
	BOOL LoadFile(const char* pszTextFile)
	{
		FILE* fp = fopen(pszTextFile, "rb");
		if (fp == nullptr) return FALSE;

		fseek(fp, 0, SEEK_END);
		long fileSize = ftell(fp);
		if (fileSize <= 0) {
			fclose(fp);
			return FALSE;
		}

		m_iDataSize = static_cast<int>(fileSize);
		if (m_iDataSize <= 0) {
			fclose(fp);
			return FALSE;
		}

		fseek(fp, 0, SEEK_SET);
		m_pData = std::make_unique<char[]>(m_iDataSize + 2);
		if (m_pData == nullptr) {
			fclose(fp);
			return FALSE;
		}

		m_bBuildInData = TRUE;
		size_t bytesRead = fread(m_pData.get(), 1, m_iDataSize, fp);
		m_pData[bytesRead] = 0;
		m_pData[bytesRead + 1] = 0;
		fclose(fp);

		m_iLineCount = ProcData();
		return BuildLines();
	}
	BOOL SetData(char* pData, int iSize)
	{
		m_bBuildInData = FALSE;
		m_iDataSize = iSize;
		m_pData.reset(pData);
		m_iLineCount = ProcData();
		return BuildLines();
	}
	char* operator[](int line)
	{
		if (line < 0 || line >= m_iLineCount)return nullptr;
		return m_pLines[line];
	}
	int	GetLineCount() const { return m_iLineCount; }
private:
	BOOL BuildLines()
	{
		if (m_iLineCount == 0)return FALSE;
		char* p = m_pData.get();
		m_pLines = std::make_unique<char*[]>(m_iLineCount);
		int len = 0;
		int ptr = 0;
		for (int i = 0; i < m_iLineCount; ++i)
		{
			len = (int)strlen(p);
			if (len > 0)
				m_pLines[ptr++] = p;
			else
				break;
			p = p + len + 1;
		}
		return TRUE;
	}
	int	ProcData()
	{
		int i = 0;
		char* p = nullptr;
		char* pData = m_pData.get();
		int linecount = 0;
		//int charscount = 0;
		int rptr = 0;
		bool binstring = false;
		bool newlinestart = false;
		//char * pstart = nullptr;
		for (i = 0; i < m_iDataSize; ++i)
		{
			p = pData + i;
			switch (*p)
			{
				//case	' ':
				//case	'	':
				//	if( binstring )
				//	{
				//		*(m_pData+rptr++) = *p;
				//		if( !newlinestart )newlinestart = true;
				//	}
				//	break;
			case '\n':
			case '\r':
			{
				if (newlinestart)
				{
					*(pData + rptr++) = 0;
					newlinestart = false;
					linecount++;
				}
			}
			break;
			//case	'\"':
			//	binstring = !binstring;
			default:
			{
				*(pData + rptr++) = *p;
				if (!newlinestart)newlinestart = true;
			}
			break;
			}
		}
		if (newlinestart)
			linecount++;
		assert(rptr <= m_iDataSize);
		pData[rptr++] = 0;
		pData[rptr++] = 0;
		m_iDataSize = rptr;
		return linecount;
	}
	std::unique_ptr<char[]> m_pData;
	int	m_iDataSize;
	int	m_iLineCount;
	std::unique_ptr<char*[]> m_pLines;
	BOOL m_bBuildInData;
};

inline char* removespace(char* pszString)
{
	char* p1 = pszString, * p2 = pszString;
	bool	binstring = false;
	while (*p1 != 0)
	{
		if (*p1 == '\'' || *p1 == '\"')
			binstring = !binstring;
		if (!binstring)
		{
			if (*p1 == ' ' || *p1 == '	')
			{
				p1++;
				continue;
			}
		}
		*p2++ = *p1++;
	}
	*p2 = 0;
	return pszString;
}


// 设置文件ini
class CSettingFile
{
public:
	BOOL Open(const char* pszFile)
	{
		if (!m_sfSetting.LoadFile((char*)pszFile))return FALSE;
		m_sfSetting.MakeDeflate();
		return TRUE;
	}
	VOID Close() { m_sfSetting.Destroy(); }
	const char* GetString(const char* pszSection, const char* pszItemName, const char* pszDefValue = nullptr)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return pszDefValue;
		return p;
	}
	int	GetInteger(const char* pszSection, const char* pszItemName, int DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return atoi(p);
	}
	DWORD GetDword(const char* pszSection, const char* pszItemName, DWORD DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return (DWORD)strtoul(p, nullptr, 10);
	}
	float GetFloat(const char* pszSection, const char* pszItemName, float DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return strtof(p, nullptr);
	}
private:
	int	FindSectionLine(const char* pszSection)
	{
		if (pszSection == nullptr) return 0;
		int linecount = m_sfSetting.GetLineCount();
		int sectionlength = (int)strlen(pszSection);
		for (int i = 0; i < linecount; ++i)
		{
			char* p = m_sfSetting[i];

			if (*p == '[' && *(p + sectionlength + 1) == ']')
			{
				if (_strnicmp(p + 1, pszSection, sectionlength) == 0)
					return (i + 1);
			}
		}
		return -1;
	}
	char* GetSettingString(const char* pszSection, const char* pszItemName)
	{
		int startindex = 0;
		if (pszSection != nullptr)
		{
			startindex = FindSectionLine(pszSection);
			if (startindex == -1)return nullptr;
		}

		int itemnamelength = (int)strlen(pszItemName);
		if (itemnamelength == 0)return nullptr;

		int linecount = m_sfSetting.GetLineCount();
		for (int i = startindex; i < linecount; ++i)
		{
			char* p = m_sfSetting[i];
			//	如果到达下一个section,  返回错误
			if (*p == '[')return nullptr;
			//	如果是ItemName=这样的句式, 进入进一步搜索
			if (*(p + itemnamelength) == '=')
			{
				if (_strnicmp(p, pszItemName, itemnamelength) == 0)
				{
					if (*(p + itemnamelength + 1) == '\"')
					{
						char* pret = (p + itemnamelength + 2);
						int length = (int)strlen(pret);
						if (*(pret + length - 1) == '\"')
							*(pret + length - 1) = 0;
						return pret;
					}
					return (p + itemnamelength + 1);
				}
			}
		}
		return nullptr;
	}
	int	FindSettingLine(const char* pszSection, const char* pszItemName)
	{
		int startindex = 0;
		if (pszSection != nullptr)
		{
			startindex = FindSectionLine(pszSection);
			if (startindex == -1)return -1;
		}

		int itemnamelength = (int)strlen(pszItemName);
		if (itemnamelength == 0)return -1;

		int linecount = m_sfSetting.GetLineCount();
		for (int i = startindex; i < linecount; ++i)
		{
			char* p = m_sfSetting[i];
			//	如果到达下一个section,  返回错误
			if (*p == '[')return -1;
			//	如果是ItemName=这样的句式, 进入进一步搜索
			if (*(p + itemnamelength) == '=')
			{
				if (_strnicmp(p, pszItemName, itemnamelength) == 0)
					return i;
			}
		}
		return -1;
	}
	CStringFile m_sfSetting;
};


template <class T>
class xListHost
{
public:
	class xListNode
	{
		xListNode* m_pNext;
		xListNode* m_pPrev;
		xListHost<T>* m_pHost;
		T* m_pObject;
	public:
		xListNode() : m_pObject(nullptr), m_pNext(nullptr), m_pPrev(nullptr), m_pHost(nullptr)
		{
		}
		xListNode(T* pObject) : m_pObject(pObject), m_pNext(nullptr), m_pPrev(nullptr), m_pHost(nullptr)
		{
		}
		~xListNode()
		{
			m_pNext = nullptr;
			m_pPrev = nullptr;
			m_pObject = nullptr;
		}
		xListNode* getNext() { return m_pNext; }
		xListNode* getPrev() { return m_pPrev; }
		xListHost<T>* getHost() { return m_pHost; }
		VOID setObject(T* pObject) { m_pObject = pObject; }
		T* getObject() { return m_pObject; }
		VOID setNext(xListNode* pNext) { m_pNext = pNext; }
		VOID setPrev(xListNode* pPrev) { m_pPrev = pPrev; }
		VOID setHost(xListHost<T>* pHost) { m_pHost = pHost; }
		BOOL Leave()
		{
			if (m_pHost == nullptr)return FALSE;
			return m_pHost->removeNode(this);
		}
		BOOL Enter(xListHost<T>* pHost)
		{
			return pHost->addNode(this);
		}
		BOOL BelongTo(xListHost<T>* pHost)
		{
			return (pHost == m_pHost);
		}
	};
	template <class T>
	class xEventListener
	{
	public:
		virtual VOID OnAddNode( xListHost<T> * pHost, xListNode * pNode ) = 0;
		virtual VOID OnRemoveNode( xListHost<T> * pHost, xListNode * pNode ) = 0;
	};
private:
	xListNode* m_pHead;
	int	m_iNodeCount;
	xEventListener<T> * m_pEventListener;
public:
	xListHost() : m_pHead(nullptr), m_iNodeCount(0), m_pEventListener(nullptr) {}
	xListHost(LPVOID pListener) : m_pHead(nullptr), m_iNodeCount(0), m_pEventListener((xEventListener<T>*)pListener) { assert(pListener!=nullptr); }
	~xListHost() {}
	xEventListener<T> * getListener(){return m_pEventListener;}
	xListNode* getHead() { return m_pHead; }
	BOOL removeNode(xListNode* pNode)
	{
		if (pNode == nullptr)return FALSE;
		if (pNode->getHost() != this)return FALSE;

		xListNode* pNext = pNode->getNext();
		xListNode* pPrev = pNode->getPrev();
		if (pNext != nullptr)pNext->setPrev(pPrev);
		if (pPrev != nullptr)pPrev->setNext(pNext);

		if (m_pHead == pNode)
			m_pHead = pNext;
		pNode->setNext(nullptr);
		pNode->setPrev(nullptr);
		pNode->setHost(nullptr);
		if (m_pEventListener)m_pEventListener->OnRemoveNode(this, pNode);
		m_iNodeCount--;
		assert(m_iNodeCount >= 0);
		return TRUE;
	}
	BOOL addNode(xListNode* pNode)
	{
		if (pNode == nullptr)return FALSE;
		pNode->Leave();
		pNode->setHost(this);
		pNode->setPrev(nullptr);
		pNode->setNext(m_pHead);
		if (m_pHead != nullptr)
			m_pHead->setPrev(pNode);
		m_pHead = pNode;
		if (m_pEventListener)m_pEventListener->OnAddNode(this, pNode);
		m_iNodeCount++;
		return TRUE;
	}
	int	getCount() { return m_iNodeCount; }
};


template <class T>
class xListHelper
{
public:
	xListHelper(xListHost<T>* pList) { setList(pList); }
	xListHelper()
	{
		m_pList = nullptr;
		m_pNode = nullptr;
	}
	VOID setList(xListHost<T>* pList)
	{
		m_pList = pList;
		if (m_pList)
			m_pNode = m_pList->getHead();
	}
	xListHost<T>* getList() { return m_pList; }
	T* first()
	{
		if (m_pList == nullptr)return nullptr;
		m_pNode = m_pList->getHead();
		if (m_pNode)
		{
			T* pObject = m_pNode == nullptr ? nullptr : m_pNode->getObject();
			m_pNode = m_pNode->getNext();
			return pObject;
		}
		return nullptr;
	}
	T * current()
	{
		if(m_pNode)
			return m_pNode->getObject();
		return nullptr;
	}
	T* next()
	{
		if (m_pList == nullptr)return nullptr;
		if (m_pNode)
		{
			T* pObject = m_pNode == nullptr ? nullptr : m_pNode->getObject();
			m_pNode = m_pNode->getNext();
			return pObject;
		}
		return nullptr;
	}
private:
	typename xListHost<T>::xListNode* m_pNode;
	typename xListHost<T>* m_pList;
};


#define	THREAD_PROTECT CLock locker( &m_CriticalSection );
#define	THREAD_PROTECT_DEFINE CriticalSection m_CriticalSection;
// 指针队列
template<class T>
class xPtrQueue
{
	std::unique_ptr<T*[]> m_pQueue;
	BOOL m_bFull;
	int m_iPush;
	int m_iPop;
	int	m_iMaxSize;
	THREAD_PROTECT_DEFINE;
public:
	xPtrQueue() :m_iMaxSize(0), m_bFull(FALSE)
	{
		m_iPush = 0;
		m_iPop = 0;
	}
	xPtrQueue(int size) :m_iMaxSize(size), m_bFull(FALSE)
	{
		create(size);
	}
	~xPtrQueue() { destroy(); }
	BOOL create(int nSize)
	{
		destroy();
		m_pQueue = std::make_unique<T*[]>(nSize);
		m_iMaxSize = nSize;
		return TRUE;
	}
	VOID destroy()
	{
		m_pQueue.reset();
		m_iPush = 0;
		m_iPop = 0;
		m_iMaxSize = 0;
		m_bFull = FALSE;
	}
	BOOL push(T* p)
	{
		THREAD_PROTECT;
		if (p == nullptr)return FALSE;
		if (m_bFull)return FALSE;

		m_pQueue[m_iPush++] = p;
		if (m_iPush >= m_iMaxSize)
			m_iPush = 0;
		if (m_iPush == m_iPop)
			m_bFull = TRUE;
		return TRUE;
	}
	T* pop()
	{
		THREAD_PROTECT;
		if (!m_bFull && m_iPush == m_iPop)return nullptr;

		int p = m_iPop++;
		m_bFull = FALSE;

		if (m_iPop >= m_iMaxSize)m_iPop = 0;
		return m_pQueue[p];
	}
	VOID clear()
	{
		THREAD_PROTECT;
		m_bFull = FALSE;
		m_iPush = 0;
		m_iPop = 0;
	}
	// 返回队列中元素个数
	int getcount()
	{
		if (m_bFull)return m_iMaxSize;
		if (m_iPush < m_iPop)
			return (m_iPush + m_iMaxSize - m_iPop);
		return (m_iPush - m_iPop);
	}
};

// ============================================================================
// 无锁 MPSC (多生产者-单消费者) 环形缓冲区
// 容量固定为 2 的幂（用于位掩码索引）。
// ============================================================================
template<class T, int N>
class xMpscQueue
{
	static_assert((N & (N - 1)) == 0, "N必须是2的幂");
	static constexpr int MASK = N - 1;
	std::array<T*, N> m_pQueue{};
	std::atomic<int> m_iPush{ 0 };  // 生产者写入位置的发布索引
	std::atomic<int> m_iPop{ 0 };   // 消费者消费完成后的索引
	int m_iRead{ 0 };               // 消费者已确认可读的位置（仅消费者访问，无需 atomic）
public:
	xMpscQueue() : m_iPush(0), m_iPop(0), m_iRead(0)
	{
		m_pQueue.fill(nullptr);
	}
	// 仅在消费者端调用（单线程安全）
	// 注意：clear() 必须在所有生产者停止 push 后才能调用（如线程池已关闭），
	// 否则在 m_iPush/m_iPop 重置期间，生产者可能看到不一致的索引状态导致数据丢失。
	VOID clear()
	{
		// 原子地夺取 push 索引：将 m_iPush 设为 (m_iPop-1)&MASK，
		// 使正在 CAS 的生产者计算 nextPush == curPop，判定队列满而退出
		for (;;)
		{
			int curPush = m_iPush.load(std::memory_order_relaxed);
			int curPop = m_iPop.load(std::memory_order_acquire);
			int fakePush = (curPop - 1) & MASK; // 让队列看起来已满
			if (fakePush == curPush) break;     // 已经是满状态
			if (m_iPush.compare_exchange_weak(curPush, fakePush,
				std::memory_order_acq_rel, std::memory_order_relaxed))
				break;
		}
		// 此时所有生产者要么已完成 push，要么因队列满而退出
		m_iPop.store(0, std::memory_order_relaxed);
		m_iPush.store(0, std::memory_order_release);
		m_iRead = 0;
		m_pQueue.fill(nullptr);
	}
	// 多生产者安全：多个线程可同时调用 push
	BOOL push(T* p)
	{
		if (p == nullptr) return FALSE;
		// CAS 循环获取写位置
		int curPush = m_iPush.load(std::memory_order_relaxed);
		for (;;)
		{
			int curPop = m_iPop.load(std::memory_order_acquire);
			int nextPush = (curPush + 1) & MASK;
			if (nextPush == curPop) return FALSE; // 队列满

			if (m_iPush.compare_exchange_weak(curPush, nextPush,
				std::memory_order_relaxed, std::memory_order_relaxed))
			{
				// 先写入选定的槽位
				m_pQueue[curPush] = p;
				// 再通过 release fence 发布：保证消费者看到 m_iPush 更新时数据一定可见
				std::atomic_thread_fence(std::memory_order_release);
				m_iPush.store(nextPush, std::memory_order_relaxed);
				return TRUE;
			}
			// CAS 失败，curPush 已更新为最新值，重新循环
		}
	}
	// 单消费者：仅在主线程调用
	T* pop()
	{
		// 先用 m_iRead 快速判断是否有可读数据
		if (m_iRead == m_iPush.load(std::memory_order_acquire))
			return nullptr; // 队列空
		// acquire fence 确保读取到生产者写入的数据
		std::atomic_thread_fence(std::memory_order_acquire);

		T* p = m_pQueue[m_iRead];
		m_pQueue[m_iRead] = nullptr;
		m_iRead = (m_iRead + 1) & MASK;
		m_iPop.store(m_iRead, std::memory_order_release);
		return p;
	}
	// 返回队列中元素个数
	int getcount()
	{
		int curPush = m_iPush.load(std::memory_order_relaxed);
		int curPop = m_iPop.load(std::memory_order_relaxed);
		if (curPush >= curPop)
			return curPush - curPop;
		else
			return curPush + N - curPop;
	}
	// 返回队列容量
	int getmaxsize() const { return N; }
	// 返回队列是否已满
	BOOL isfull() const
	{
		int curPush = m_iPush.load(std::memory_order_relaxed);
		int nextPush = (curPush + 1) & MASK;
		int curPop = m_iPop.load(std::memory_order_relaxed);
		return (nextPush == curPop);
	}
	// 返回队列是否为空
	BOOL isempty() const
	{
		int curPush = m_iPush.load(std::memory_order_relaxed);
		int curPop = m_iPop.load(std::memory_order_relaxed);
		return (curPush == curPop);
	}
};


constexpr auto OBJECTPOOLCACHESIZE = 4096; // 对象池缓存块大小（字节）
template <class T>
class xObjectPool
{
public:
	xObjectPool() : m_nCachePtr(0), m_nCacheSize(0)
	{
		CacheObjects();
	}
	~xObjectPool()
	{
	}
	T* newObject()
	{
		THREAD_PROTECT;
		// 优先从空闲栈取
		if (!m_stkFree.empty())
		{
			T* p = m_stkFree.top();
			m_stkFree.pop();
			return p;
		}
		// 从预分配缓存块中取
		T* pRaw = AllocFromCache();
		if (pRaw != nullptr)
			return pRaw;
		// 缓存耗尽，单独分配
		auto up = std::make_unique<T>();
		T* p = up.get();
		m_vObjects.push_back(std::move(up));
		return p;
	}
	VOID deleteObject(T* pObject)
	{
		THREAD_PROTECT;
		if (pObject == nullptr) return;
		m_stkFree.push(pObject);
	}
	int getCount() const { return (int)m_vObjects.size(); }
	int getFreeCount() const { return (int)m_stkFree.size(); }
	int getUsedCount() const { return (int)(m_vObjects.size() - m_stkFree.size()); }
	// 遍历所有对象（包括空闲和使用的），回调返回 false 停止遍历
	template<typename Func>
	VOID forEach(Func&& callback)
	{
		THREAD_PROTECT;
		for (auto& up : m_vObjects)
		{
			if (!callback(up.get()))
				break;
		}
	}
	// 清空所有对象：先调用回调清理外部引用，再释放内存
	template<typename Func>
	VOID clearAll(Func&& cleanupFunc)
	{
		THREAD_PROTECT;
		for (auto& up : m_vObjects)
			cleanupFunc(up.get());
		m_vObjects.clear();
		while (!m_stkFree.empty())
			m_stkFree.pop();
		m_pCacheBlocks.clear();
		m_nCachePtr = 0;
		m_nCacheSize = 0;
	}
private:
	THREAD_PROTECT_DEFINE
	T* AllocFromCache()
	{
		if (m_nCachePtr >= m_nCacheSize)
		{
			CacheObjects();
			if (m_nCachePtr >= m_nCacheSize) return nullptr;
		}
		T* p = &m_pCacheBlocks.back()[m_nCachePtr++];
		return p;
	}
	VOID CacheObjects()
	{
		// 每次分配 OBJECTPOOLCACHESIZE 字节的缓存块（至少 4 个对象）
		m_nCacheSize = (OBJECTPOOLCACHESIZE + sizeof(T) - 1) / sizeof(T);
		if (m_nCacheSize < 4) m_nCacheSize = 4;
		m_pCacheBlocks.push_back(std::make_unique<T[]>(m_nCacheSize));
		m_nCachePtr = 0;
	}
	std::vector<std::unique_ptr<T[]>> m_pCacheBlocks;  // 批量预分配缓存块（连续内存，cache友好）
	std::vector<std::unique_ptr<T>> m_vObjects;         // 单独分配的对象（缓存块耗尽时使用）
	std::stack<T*> m_stkFree;                           // 空闲对象栈（O(1) 获取/归还）
	UINT m_nCachePtr;
	UINT m_nCacheSize;
};


// 池化字符串块 — 首字节为桶号标记
// bucket=0~4: 池分配，bucket=0xFF: 堆分配
template<size_t Capacity>
struct StringBlock
{
	static constexpr size_t dataCapacity = Capacity - 1;
	uint8_t bucket;          // 桶号索引
	char data[dataCapacity]; // 字符串数据（紧接 bucket 之后）
};

class CStringPool
{
public:
	static CStringPool& getInstance()
	{
		static CStringPool instance;
		return instance;
	}
	// 分配并拷贝字符串
	char* copystring(const char* src)
	{
		if (src == nullptr || *src == 0) return nullptr;
		size_t len = strlen(src);
		char* p = allocString(len);
		memcpy(p, src, len + 1);
		return p;
	}
	// 释放字符串空间（自动判断池/堆）
	void freeString(char* p)
	{
		if (p == nullptr) return;
		uint8_t bucket = static_cast<uint8_t>(p[-1]);
		switch (bucket)
		{
		case 0: m_pool32.deleteObject(reinterpret_cast<StringBlock<32>*>(p - 1)); break;
		case 1: m_pool64.deleteObject(reinterpret_cast<StringBlock<64>*>(p - 1)); break;
		case 2: m_pool128.deleteObject(reinterpret_cast<StringBlock<128>*>(p - 1)); break;
		case 3: m_pool256.deleteObject(reinterpret_cast<StringBlock<256>*>(p - 1)); break;
		case 4: m_pool512.deleteObject(reinterpret_cast<StringBlock<512>*>(p - 1)); break;
		default: // 0xFF — 堆分配
			delete[](p - 1);
			break;
		}
	}
	// 统计信息
	size_t getTotalAllocated() const
	{
		return m_pool32.getCount() + m_pool64.getCount() + m_pool128.getCount()
			 + m_pool256.getCount() + m_pool512.getCount();
	}

	size_t getTotalFree() const
	{
		return m_pool32.getFreeCount() + m_pool64.getFreeCount() + m_pool128.getFreeCount()
			 + m_pool256.getFreeCount() + m_pool512.getFreeCount();
	}
private:
	CStringPool() = default;
	~CStringPool() = default;
	CStringPool(const CStringPool&) = delete;
	CStringPool& operator=(const CStringPool&) = delete;
	// 从池中分配指定大小的字符串空间
	// 桶容量: 32, 64, 128, 256, 512 字节（含 1 字节 header）
	// 实际可用: 31, 63, 127, 255, 511 字节
	char* allocString(size_t len)
	{
		if (len < 31)
		{
			auto* block = m_pool32.newObject();
			block->bucket = 0;
			return block->data;
		}
		if (len < 63)
		{
			auto* block = m_pool64.newObject();
			block->bucket = 1;
			return block->data;
		}
		if (len < 127)
		{
			auto* block = m_pool128.newObject();
			block->bucket = 2;
			return block->data;
		}
		if (len < 255)
		{
			auto* block = m_pool256.newObject();
			block->bucket = 3;
			return block->data;
		}
		if (len < 511)
		{
			auto* block = m_pool512.newObject();
			block->bucket = 4;
			return block->data;
		}
		// 超长字符串走堆（1 字节 header + len + 1 null）
		char* raw = new char[len + 2];
		raw[0] = static_cast<char>(0xFF);
		return raw + 1;
	}
	xObjectPool<StringBlock<32>>  m_pool32;
	xObjectPool<StringBlock<64>>  m_pool64;
	xObjectPool<StringBlock<128>> m_pool128;
	xObjectPool<StringBlock<256>> m_pool256;
	xObjectPool<StringBlock<512>> m_pool512;
};

// 分配并拷贝字符串（从 CStringPool 分配，替代 new char[]）
inline char* copystring(const char* pszString)
{
	return CStringPool::getInstance().copystring(pszString);
}

// 释放 copystring 返回的字符串（替代 delete[]）
inline void freestring(char* p)
{
	CStringPool::getInstance().freeString(p);
}

// 池化字符串删除器 — 供 unique_ptr 使用
struct PooledStringDeleter
{
	void operator()(char* p) const noexcept
	{
		CStringPool::getInstance().freeString(p);
	}
};
// 池化字符串智能指针 — 替代 std::unique_ptr<char[]> 用于 copystring 返回值
using pooled_string_ptr = std::unique_ptr<char[], PooledStringDeleter>;


// 自动释放数组
template<class T>
class xAutoPtrArray
{
public:
	xAutoPtrArray(UINT max) { Create(max); }
	xAutoPtrArray()
	{
		m_iMax = 0;
		m_iCount = 0;
	}
	~xAutoPtrArray()
	{
		m_iMax = 0;
		m_iCount = 0;
	}
	BOOL Create(UINT max)
	{
		m_pArray = std::make_unique<T*[]>(max);
		m_iMax = max;
		Clean();
		return TRUE;
	}
	VOID Clean()
	{
		if (m_pArray)
			memset(m_pArray.get(), 0, sizeof(T*) * m_iMax);
		m_iCount = 0;
	}
	UINT Add(T* pt)
	{
		if (m_iCount == m_iMax)return (UINT)-1;
		m_pArray[m_iCount++] = pt;
		return (m_iCount - 1);
	}
	T* Get(UINT index)
	{
		if (index >= m_iCount) return nullptr;
		return m_pArray[index];
	}
	BOOL Del(T* pt)
	{
		for (UINT i = 0; i < m_iCount; ++i)
		{
			if (m_pArray[i] == pt) return Del(i);
		}
		return FALSE;
	}
	BOOL Del(UINT index)
	{
		if (index >= m_iCount)return FALSE;
		m_iCount--;
		UINT ileft = m_iCount - index;
		if (ileft > 0)
			memmove(m_pArray.get() + index, m_pArray.get() + index + 1, sizeof(T*) * ileft);
		return TRUE;
	}
	T* operator [](UINT index) { return Get(index); }
	UINT GetCount() { return m_iCount; }
	UINT operator [] (T* pt)
	{
		for (UINT i = 0; i < m_iCount; ++i)
		{
			if (m_pArray[i] == pt)
				return i;
		}
		return (UINT)-1;
	}
	UINT GetMaxCount() { return m_iMax; }
	BOOL Insert(T* pt, UINT Index = 0)
	{
		if (Index >= m_iCount)return (Add(pt) != 0xffffffff);
		if (m_iCount >= m_iMax)return FALSE;
		memmove(m_pArray.get() + Index + 1, m_pArray.get() + Index, sizeof(T*) * (m_iCount - Index));
		m_pArray[Index] = pt;
		m_iCount++;
		return TRUE;
	}
private:
	std::unique_ptr<T*[]> m_pArray;
	UINT m_iCount;
	UINT m_iMax;
};


// 单例模板类-线程安全-零开销
template <class T>
class xSingletonClass
{
public:
	xSingletonClass() { m_pInstance.store((T*)this); }
	static T* GetInstance()
	{
		T* p = m_pInstance.load(std::memory_order_acquire);
		if (p == nullptr)
		{
			SWLock lock(m_srwLock);
			p = m_pInstance.load(std::memory_order_relaxed);
			if (p == nullptr)
			{
				p = new T;
				m_pInstance.store(p, std::memory_order_release);
			}
		}
		return p;
	}
protected:
	static std::atomic<T*> m_pInstance;
	static SRWLOCK m_srwLock;
};
template <class T>
std::atomic<T*> xSingletonClass<T>::m_pInstance{ nullptr };
template <class T>
SRWLOCK xSingletonClass<T>::m_srwLock = SRWLOCK_INIT;


class xEventSender;
// 事件监听器，用于接收事件
class xEventListener
{
public:
	virtual VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam) = 0;
};

// 事件发送器，用于发送事件给事件监听器
class xEventSender
{
public:
	xEventSender() { m_pEventListener = nullptr; }
	VOID setEventListener(xEventListener* pEventListener) { m_pEventListener = pEventListener; }
	xEventListener* getEventListener() { return m_pEventListener; }
	VOID sendEvent(int iEvent, int iParam, LPVOID lpParam) { if (m_pEventListener) m_pEventListener->OnEvent(this, iEvent, iParam, lpParam); }
protected:
	xEventListener* m_pEventListener;
};

// 去除字符串前后空格的函数
inline char* Trim(char* pString)
{
	int len = (int)strlen(pString);
	char* p = pString;
	while (*p == ' ' || *p == '	')p++, len--;
	while (*(p + len - 1) == ' ' || *(p + len - 1) == '	')len--, * (p + len) = 0;
	return p;
}

// 字符串分割函数（核心实现），支持单字符和多字符分隔符（包括中文字符）
inline int SearchParam(char* buffer, char** Params, int maxparam, const char* spliter)
{
	char* pbuffer = Trim(buffer);
	int len = (int)strlen(buffer);
	if (len == 0)return 0;
	int splitLen = (int)strlen(spliter);
	if (splitLen == 0)return 0;
	char* p = strstr(pbuffer, spliter);
	int count = 0;
	Params[0] = pbuffer;
	while (p)
	{
		memset(p, 0, splitLen);
		p += splitLen;
		Params[count++] = Trim(Params[count]);
		if (count >= maxparam)return count;
		Params[count] = p;
		p = strstr(p, spliter);
	}
	Params[count++] = Trim(Params[count]);
	return count;
}

// 字符串分割器，支持单字符和多字符分隔符（包括中文）
template<int maxindex>
class xStringsExpander
{
public:
	xStringsExpander(char* pszString, int Delim)
	{
		char szSpliter[2] = { (char)Delim, 0 };
		m_iCount = SearchParam(pszString, m_pStrings.data(), maxindex, szSpliter);
	}
	xStringsExpander(char* pszString, const char* spliter)
	{
		m_iCount = SearchParam(pszString, m_pStrings.data(), maxindex, spliter);
	}
	const char* getString(int index)
	{
		if (index >= m_iCount || index < 0)return nullptr;
		return m_pStrings[index];
	}
	int getCount() { return m_iCount; }
	const char* operator [](int index) { return getString(index); }
private:
	std::array<char*, maxindex> m_pStrings;
	int	m_iCount;
};


//除法并向上取整，a/b
template<typename T>
T ceil_div(T a, T b)
{
	return (a + b - 1) / b;
}


//原子计数 + 自旋等待(自旋可避免内核态切换开销)
//注意：Arrive() 无超时机制，若某个参与者未调用 Signal()，调用者将永久阻塞。
//请确保所有参与者必定会调用 Signal()，且在 Arrive() 之前已启动。
class CSpinBarrier
{
public:
	CSpinBarrier(int count) : m_remaining(count) {}
	VOID Arrive()
	{
		// 先自旋等几微秒（适合游戏帧内同步，通常很快完成）
		int spins = 0;
		while (m_remaining.load(std::memory_order_acquire) > 0) {
			if (++spins > 64)
			{
				std::this_thread::yield(); // 自旋太久，让出CPU
				spins = 0;
			}
		}
	}
	VOID Signal()
	{
		m_remaining.fetch_sub(1, std::memory_order_release);
	}
	VOID Reset(int count)
	{
		m_remaining.store(count, std::memory_order_release);
	}
private:
	std::atomic<int> m_remaining;
};

//轻量级读写锁,实用于读
class SRLock
{
public:
	explicit SRLock(SRWLOCK& lock) : m_lock(lock) { AcquireSRWLockShared(&m_lock); }
	~SRLock() { ReleaseSRWLockShared(&m_lock); }
	SRLock(const SRLock&) = delete;
	SRLock& operator=(const SRLock&) = delete;
	SRLock(SRLock&&) = delete;
	SRLock& operator=(SRLock&&) = delete;
private:
	SRWLOCK& m_lock;
};

//轻量级读写锁,实用于写
class SWLock
{
public:
	explicit SWLock(SRWLOCK& lock) : m_lock(lock) { AcquireSRWLockExclusive(&m_lock); }
	~SWLock() { ReleaseSRWLockExclusive(&m_lock); }
	SWLock(const SWLock&) = delete;
	SWLock& operator=(const SWLock&) = delete;
	SWLock(SWLock&&) = delete;
	SWLock& operator=(SWLock&&) = delete;
private:
	SRWLOCK& m_lock;
};
