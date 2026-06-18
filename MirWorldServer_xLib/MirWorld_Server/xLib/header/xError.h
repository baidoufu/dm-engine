#pragma once
#include <array>

class xError
{
public:
	xError()
	{
		m_dwErrorCode = 0;
		m_szErrorMsg[0] = 0;
	}
	~xError()
	{
		m_szErrorMsg[0] = 0;
		m_dwErrorCode = 0;
	}
	VOID setError(DWORD dwErrorCode, const char* pszError, ...)
	{
		SWLock lock(m_errorLock);
		m_dwErrorCode = dwErrorCode;
		va_list vl;
		va_start(vl, pszError);
		vsnprintf(m_szErrorMsg.data(), m_szErrorMsg.size(), pszError, vl);
		va_end(vl);
		m_szErrorMsg[m_szErrorMsg.size() - 1] = 0;
	}
	VOID setError(xError& error)
	{
		setError(error.getErrorCode(), error.getErrorMsg());
	}
	DWORD getErrorCode() const { return m_dwErrorCode; }
	const char* getErrorMsg() { return m_szErrorMsg.data(); }
private:
	SRWLOCK m_errorLock = SRWLOCK_INIT;
	DWORD m_dwErrorCode;
	std::array<char, 1024> m_szErrorMsg;
};