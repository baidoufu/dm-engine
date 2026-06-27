#include "..\header\xinc.h"
#include "..\header\xsocket.h"

xSocket::xSocket() : m_state(SS_UNINITED), m_Socket(INVALID_SOCKET), m_nPort(0)
{
	m_szAddress.fill(0);
	memset(&m_stats, 0, sizeof(m_stats));
	m_stats.connection_time = getCurrentTime();
}

xSocket::xSocket(SOCKET Socket) : m_state(SS_UNINITED), m_Socket(INVALID_SOCKET), m_nPort(0)
{
	m_szAddress.fill(0);
	setSocket(Socket);
	memset(&m_stats, 0, sizeof(m_stats));
	m_stats.connection_time = getCurrentTime();
}

xSocket::~xSocket(VOID)
{
	close();
}

BOOL xSocket::makeSocket(int af, int type, int protocol)
{
	m_Socket = socket(af, type, protocol);
	if (m_Socket == INVALID_SOCKET)
	{
		setError(WSAGetLastError(), "socket( %d, %d, %d ) 调用失败!", af, type, protocol);
		setState(SS_UNINITED);
		return FALSE;
	}
	setState(SS_UNUSED);
	return TRUE;
}

BOOL xSocket::setSocket(SOCKET Socket)
{
	m_Socket = Socket;
	if (Socket != INVALID_SOCKET)
		setState(SS_UNUSED);
	else
	{
		m_szAddress.fill(0);
		m_nPort = 0;
		setState(SS_UNINITED);
	}
	return TRUE;
}

VOID xSocket::close()
{
	if (m_Socket != INVALID_SOCKET)
	{
		HANDLE hFile = reinterpret_cast<HANDLE>(static_cast<INT_PTR>(m_Socket));
		if (!CancelIoEx(hFile, nullptr))
		{
			DWORD dwError = GetLastError();
			if (dwError != ERROR_NOT_FOUND)
			{
				setError(static_cast<int>(dwError), 
					"CancelIoEx 失败 (socket=%d, error=%d), 残留的 IOCP 完成通知可能访问已释放对象",
					static_cast<int>(m_Socket), static_cast<int>(dwError));
			}
		}
		closesocket(m_Socket);
	}
	m_Socket = INVALID_SOCKET;
	setState(SS_DISCONNECTED);
}

BOOL xSocket::sendEx(LPVOID lpData, int nDatasize, DWORD& dwBytesSent, DWORD dwFlag, LPOVERLAPPED lpOverlapped)
{
	WSABUF wsabuf{};
	wsabuf.buf = static_cast<char*>(lpData);
	wsabuf.len = nDatasize;

	dwBytesSent = 0;
	
	if (WSASend(m_Socket, &wsabuf, 1, &dwBytesSent, dwFlag, lpOverlapped, nullptr) == SOCKET_ERROR)
	{
		int code = WSAGetLastError();
		if (code != WSA_IO_PENDING)
		{
			setError(code, "WSASend() 失败!");
			return FALSE;
		}
		// 异步操作, 稍后会通过完成端口通知
	} else {
		// 同步完成
		m_stats.bytes_sent += dwBytesSent;
		m_stats.packets_sent++;
		m_stats.last_activity = getCurrentTime();
	}
	return TRUE;
}

BOOL xSocket::recvEx(LPVOID lpDataBuf, int nBufsize, DWORD& dwBytesReceived, DWORD& dwFlag, LPOVERLAPPED lpOverlapped)
{
	WSABUF wsabuf{};
	wsabuf.buf = static_cast<char*>(lpDataBuf);
	wsabuf.len = nBufsize;

	if (WSARecv(m_Socket, &wsabuf, 1, &dwBytesReceived, &dwFlag, lpOverlapped, nullptr) == SOCKET_ERROR)
	{
		int code = WSAGetLastError();
		if (code != WSA_IO_PENDING)
		{
			setError(code, "WSARecv() 失败!");
			return FALSE;
		}
	} else {
		// 同步完成, 更新统计
		m_stats.bytes_received += dwBytesReceived;
		m_stats.packets_received++;
		m_stats.last_activity = getCurrentTime();
	}
	return TRUE;
}

BOOL xSocket::acceptEx(xSocket& sAccept, LPVOID lpDataBuf, DWORD dwRecvBufferLength, DWORD& dwBytesReceived, LPOVERLAPPED lpOverlapped)
{
	if (sAccept.getState() != SS_UNUSED)
	{
		sAccept.close();
		sAccept.makeSocket();
	}

	if (AcceptEx(m_Socket, sAccept.getSocketFd(), lpDataBuf, dwRecvBufferLength, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytesReceived, lpOverlapped))
	{
		sAccept.setState(SS_ACCEPTED);
		return TRUE;
	}

	if (WSAGetLastError() == ERROR_IO_PENDING)
		return TRUE;

	setError(WSAGetLastError(), "AcceptEx() 失败!");

	return FALSE;
}

BOOL xSocket::connect(const char* cp, UINT nPort)
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result = nullptr;
	if (getaddrinfo(cp, nullptr, &hints, &result) != 0 || result == nullptr)
	{
		setError(WSAGetLastError(), "connect() 中 getaddrinfo( %s ) 失败!", cp);
		return FALSE;
	}

	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket();
	}

	struct sockaddr_in ServAddr = {};
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
	ServAddr.sin_port = htons(nPort);
	freeaddrinfo(result);

	// 设置为非阻塞模式
	setNoBlocking();

	int erri = ::connect(m_Socket, reinterpret_cast<struct sockaddr*>(&ServAddr), sizeof(ServAddr));
	if (erri == SOCKET_ERROR)
	{
		int code = WSAGetLastError();
		if (code != WSAEWOULDBLOCK)  // 非阻塞连接正常返回此错误
		{
			setError(code, "connect() 失败!");
			return FALSE;
		}
		// WSAEWOULDBLOCK 表示连接正在进行中，不在此处阻塞等待
		// 调用方应使用 pollConnectResult() 检查连接结果
		setEndPoint(cp, nPort);
		setState(SS_CONNECTING);
		return TRUE;
	}
	setEndPoint(cp, nPort);
	setState(SS_CONNECTED);
	return TRUE;
}

BOOL xSocket::pollConnectResult(UINT timeoutMs)
{
	if (getState() != SS_CONNECTING)
		return FALSE;

	fd_set writeSet{};
	fd_set errSet{};
	FD_ZERO(&writeSet);
	FD_ZERO(&errSet);
	FD_SET(m_Socket, &writeSet);
	FD_SET(m_Socket, &errSet);
	timeval tv = { static_cast<long>(timeoutMs / 1000), static_cast<long>((timeoutMs % 1000) * 1000) };

	int ret = select(0, nullptr, &writeSet, &errSet, &tv);
	if (ret <= 0)
	{
		// 超时或 select 失败
		setError(WSAGetLastError(), "pollConnectResult() 超时!");
		return FALSE;
	}

	// 检查是否有错误（连接被拒绝等）
	if (FD_ISSET(m_Socket, &errSet))
	{
		int optval = 0;
		int optlen = sizeof(optval);
		getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen);
		setError(optval, "pollConnectResult() 连接失败!");
		return FALSE;
	}

	// 连接成功，检查 SO_ERROR 确认（某些情况下 writeSet 也可能触发但连接失败）
	int optval = 0;
	int optlen = sizeof(optval);
	getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen);
	if (optval != 0)
	{
		setError(optval, "pollConnectResult() 连接失败!");
		return FALSE;
	}

	setState(SS_CONNECTED);
	return TRUE;
}

BOOL xSocket::listen(UINT nPort)
{
	struct	sockaddr_in	serveraddr;
	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	setReuseAddr(TRUE);

	memset(static_cast<VOID*>(&serveraddr), 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(nPort);

	if (::bind(m_Socket, reinterpret_cast<struct sockaddr*>(&serveraddr), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 bind() 失败!");
		return FALSE;
	}
	if (::listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 listen() 失败!");
		return FALSE;
	}
	setEndPoint("0.0.0.0", nPort);
	setState(SS_LISTENING);
	return TRUE;
}

BOOL xSocket::listen(const char* cp, UINT nPort)
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result = nullptr;
	if (getaddrinfo(cp, nullptr, &hints, &result) != 0 || result == nullptr)
	{
		setError(WSAGetLastError(), "listen( %s, %u ) 中 getaddrinfo( %s ) 失败!", cp, nPort, cp);
		return FALSE;
	}

	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket();
	}

	setReuseAddr(TRUE);

	struct sockaddr_in serveraddr = {};
	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
	serveraddr.sin_port = htons(nPort);
	freeaddrinfo(result);

	if (::bind(m_Socket, reinterpret_cast<struct sockaddr*>(&serveraddr), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 bind() 失败!");
		return FALSE;
	}
	if (::listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 listen() 失败!");
		return FALSE;
	}
	setEndPoint(cp, nPort);
	setState(SS_LISTENING);
	return TRUE;
}

BOOL xSocket::setNoBlocking()
{
	unsigned long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "setNoBlocking() 中 ioctlsocket() 失败!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSocket::setBlocking()
{
	unsigned long mode = 0;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "setBlocking() 中 ioctlsocket() 失败!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSocket::setReuseAddr(BOOL reuse)
{
	BOOL opt = reuse;
	return setSocketOption(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

BOOL xSocket::setKeepAlive(BOOL keepalive, UINT keepalive_time, UINT keepalive_interval)
{
	if (!setSocketOption(SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive))) {
		return FALSE;
	}
	
	if (keepalive) {
		// Windows 特定的 keepalive 设置
		tcp_keepalive keepalive_vals;
		keepalive_vals.onoff = 1;
		keepalive_vals.keepalivetime = keepalive_time * 1000;  // 转换为毫秒
		keepalive_vals.keepaliveinterval = keepalive_interval * 1000;  // 转换为毫秒
		
		DWORD bytes_returned;
		if (WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &keepalive_vals, sizeof(keepalive_vals),
					 nullptr, 0, &bytes_returned, nullptr, nullptr) == SOCKET_ERROR) {
			setError(WSAGetLastError(), "setKeepAlive() 中 WSAIoctl() 失败!");
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL xSocket::setTcpNoDelay(BOOL nodelay)
{
	BOOL opt = nodelay;
	return setSocketOption(IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

BOOL xSocket::setSendBuffer(UINT size)
{
	return setSocketOption(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

BOOL xSocket::setRecvBuffer(UINT size)
{
	return setSocketOption(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

BOOL xSocket::setSocketOption(int level, int optname, const VOID* optval, int optlen)
{
	if (setsockopt(m_Socket, level, optname, static_cast<const char*>(optval), optlen) == SOCKET_ERROR) {
		setError(WSAGetLastError(), "setSocketOption() 失败!level=%d, optname=%d", level, optname);
		return FALSE;
	}
	return TRUE;
}

DWORD xSocket::getCurrentTime() const
{
	return GetSteadyTimeMS();
}

DWORD xSocket::getConnectionAge() const
{
	DWORD current_time = getCurrentTime();
	return current_time - m_stats.connection_time;
}

BOOL xSocket::isConnectionTimeout(DWORD timeout_ms) const
{
	return getConnectionAge() >= timeout_ms;
}

DWORD xSocket::getIdleTime() const
{
	DWORD current_time = getCurrentTime();
	return current_time - m_stats.last_activity;
}
