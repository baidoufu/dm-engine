#include "StdAfx.h"
#include "inc.h"
#include ".\logfile.h"

CLogFile::CLogFile(VOID)
{
	m_fpLog.reset();
	memset(&m_stFileTime, 0, sizeof(m_stFileTime));
	m_szCache.fill(0);
	m_szFile.fill(0);

	if (!PathIsFolder("..\\日志"))
	{
		if (!CreateDirectory("..\\日志", nullptr))
		{
			MessageBox(GetDesktopWindow(), "创建日志文件夹失败, 请关闭程序手动创建!", "提示", 0);
		}
	}
}

CLogFile::~CLogFile(VOID)
{
}

BOOL CLogFile::Init(const char* pszPath)
{
	if (!PathIsFolder(pszPath))
	{
		if (!CreateDirectory(pszPath, nullptr))
			return FALSE;
	}
	o_strncpy(m_szFile.data(), pszPath, 1023);
	ChangeLogFile();
	return TRUE;
}

VOID CLogFile::LogDate()
{
	SYSTEMTIME	st;
	GetLocalTime(&st);
	LogText("[%04u-%02u-%02u %02u:%02u:%02u] ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

VOID CLogFile::LogText(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (!m_fpLog)return;
	va_list	vl;
	va_start(vl, pszString);
	vsnprintf(m_szCache.data(), m_szCache.size(), pszString, vl);
	va_end(vl);
	m_szCache[m_szCache.size() - 1] = 0;
	fputs(m_szCache.data(), m_fpLog.get());
	fflush(m_fpLog.get());
}

VOID CLogFile::LogTextRt(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (!m_fpLog)return;
	va_list	vl;
	va_start(vl, pszString);
	vsnprintf(m_szCache.data(), m_szCache.size(), pszString, vl);
	va_end(vl);
	m_szCache[m_szCache.size() - 1] = 0;
	fputs(m_szCache.data(), m_fpLog.get());
	fputs("\n", m_fpLog.get());
	fflush(m_fpLog.get());
}

VOID CLogFile::LogTextWithDate(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (!m_fpLog)return;
	LogDate();
	va_list	vl;
	va_start(vl, pszString);
	vsnprintf(m_szCache.data(), m_szCache.size(), pszString, vl);
	va_end(vl);
	m_szCache[m_szCache.size() - 1] = 0;
	fputs(m_szCache.data(), m_fpLog.get());
	fflush(m_fpLog.get());
}

VOID CLogFile::NextLine()
{
	if (m_fpLog)
		fprintf(m_fpLog.get(), "\n");
}

VOID CLogFile::ChangeLogFile()
{
	THREAD_PROTECT;
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	if (stNow.wDay != m_stFileTime.wDay || !m_fpLog)
	{
		std::array<char, 256> szFile1{};
		std::array<char, 1024> szFile{};
		sprintf(szFile1.data(), "%04u-%02u-%02u.log", stNow.wYear, stNow.wMonth, stNow.wDay);
		_makepath(szFile.data(), nullptr, m_szFile.data(), szFile1.data(), nullptr);
		m_fpLog.reset(fopen(szFile.data(), "a"));
		m_stFileTime = stNow;
	}
}
