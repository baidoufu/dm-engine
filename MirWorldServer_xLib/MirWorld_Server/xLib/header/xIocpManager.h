#pragma once

constexpr DWORD IOCP_KEY_LISTEN_SOCKET = 0xFFFFFFFF;
constexpr DWORD IOCP_KEY_CLIENT_SOCKET = 0;
constexpr ULONG_PTR IOCP_KEY_SHUTDOWN = 0xDEADBEEF;

class xIocpWorkThread;
class xIocpManager :public xError
{
public:
	xIocpManager(VOID);
	virtual ~xIocpManager(VOID);
	BOOL Start();
	VOID Stop();
	BOOL Bind(SOCKET socket, DWORD dwBindId);
protected:
	HANDLE m_hIocp;
	std::unique_ptr<xIocpWorkThread[]> m_pIocpWorkThread;
	DWORD m_iIocpWorkThreadCount;
};