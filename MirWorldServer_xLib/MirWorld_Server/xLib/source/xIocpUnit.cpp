#include "..\header\xiocpunit.h"

xIocpUnit::xIocpUnit(VOID) : m_dwSendOffset(0)
{
	m_type = IO_NOTSET;
	m_UnitStruct.pUnit = this;
	m_UnitStruct.dwValidToken = 0;
	m_lpData = nullptr;
}

xIocpUnit::~xIocpUnit(VOID)
{
	m_UnitStruct.dwValidToken = 0;
	m_UnitStruct.pUnit = nullptr;
}

VOID xIocpUnit::OnComplete(DWORD dwNumberOfBytes, ULONG_PTR dwCompletionKey, DWORD dwError)
{
	if (dwError != 0)
	{
		// I/O²Ù×÷Ê§°Ü£¬·¢ËÍ´íÎóÊÂ¼þ£¬iParam=´íÎóÂë
		sendEvent(IUE_ERROR, static_cast<int>(dwError), static_cast<LPVOID>(this));
		return;
	}
	int eventId = IUE_ID;
	switch (m_type)
	{
	case IO_READ:   eventId = IUE_READ;   break;
	case IO_SEND:   eventId = IUE_SEND;   break;
	case IO_ACCEPT: eventId = IUE_ACCEPT; break;
	default:        eventId = IUE_ID;     break;
	}
	sendEvent(eventId, static_cast<int>(dwNumberOfBytes), static_cast<LPVOID>(this));
}