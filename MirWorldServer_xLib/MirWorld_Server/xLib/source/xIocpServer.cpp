#include "..\header\xiocpserver.h"
#include "..\header\xiocpunit.h"
#include "..\header\xpacket.h"

xIocpServer::xIocpServer(VOID)
{
	m_dwSendBytes = 0;
	m_dwRecvBytes = 0;
	
	// 初始化WSA
	WSADATA	wsad;
	memset(&wsad, 0, sizeof(wsad));
	int wsast = WSAStartup(0x202, &wsad);
	if (wsast != 0)
	{
		switch (wsast)
		{
		case WSASYSNOTREADY:
			setError(wsast, "WSAStartup失败: 网络子系统不可用");
			break;
		case WSAVERNOTSUPPORTED:
			setError(wsast, "WSAStartup失败: 不支持的Winsock版本 %d.%d", LOBYTE(0x202), HIBYTE(0x202));
			break;
		case WSAEINPROGRESS:
			setError(wsast, "WSAStartup失败: Winsock初始化正在进行中");
			break;
		case WSAEPROCLIM:
			setError(wsast, "WSAStartup失败: 达到任务限制");
			break;
		case WSAEFAULT:
			setError(wsast, "WSAStartup失败: 无效的lpWSAData指针");
			break;
		default:
			setError(wsast, "WSAStartup失败: 未知错误代码 %d", wsast);
			break;
		}
		m_bWSAInitialized = FALSE;
		return;
	}
	
	// 验证Winsock版本
	if (wsad.wVersion != 0x202)
	{
		setError(-1, "Winsock版本不匹配: 期望 %d.%d, 实际 %d.%d", 
			LOBYTE(0x202), HIBYTE(0x202), LOBYTE(wsad.wVersion), HIBYTE(wsad.wVersion));
		WSACleanup(); // 清理已初始化的资源
		m_bWSAInitialized = FALSE;
		return;
	}
	
	// WSA初始化成功
	m_bWSAInitialized = TRUE;
}

xIocpServer::~xIocpServer(VOID)
{
	// 安全停止服务器
	stop();
	
	// 只有当WSA初始化成功时才调用WSACleanup
	if (isWSAReady())
	{
		WSACleanup();
		m_bWSAInitialized = FALSE;
	}
}

BOOL xIocpServer::postListen(const char* cp, UINT nPort, int nPostAccept, UINT id)
{
	// 检查WSA初始化状态
	if (!isWSAReady())
	{
		setError(-1, "无法监听端口 %d: WSA未正确初始化", nPort);
		return FALSE;
	}
	// 分配listen对象
	xListenObject* pListenObject = m_xListenObjectPool.newObject();
	if (pListenObject == nullptr)
	{
		setError(-4, "无法分配新的监听对象!");
		return FALSE;
	}
	pListenObject->setServer(this);
	pListenObject->setId(id);
	// 开始监听（使用配置的IP地址，而非默认的0.0.0.0）
	if (!pListenObject->listen(cp, nPort))
	{
		setError(*pListenObject);
		return FALSE;
	}
	// 绑定到完成端口
	if (!m_xIocpManager.Bind(pListenObject->getSocketFd(), IOCP_KEY_LISTEN_SOCKET))
	{
		setError(m_xIocpManager);
		return FALSE;
	}
	// 发送accept请求
	pListenObject->postAccept(nPostAccept);
	return TRUE;
}

BOOL xIocpServer::start()
{
	// 检查WSA是否已正确初始化
	if (!isWSAReady())
	{
		setError(-1, "无法启动服务器: WSA未正确初始化, 请检查网络子系统");
		return FALSE;
	}
	
	//	iocp 管理器开始
	if (!m_xIocpManager.Start())
	{
		setError(m_xIocpManager);
		return FALSE;
	}
	return TRUE;
}

BOOL xIocpServer::stop()
{
	m_xIocpManager.Stop();
	return TRUE;
}

VOID xIocpServer::update()
{
	// 批量处理新建连接
	int acceptQueueSize = m_xAcceptConnectionQueue.getcount();
	int maxAcceptBatch = MIN(acceptQueueSize, 128);  // 最多128个/帧
	for (int i = 0; i < maxAcceptBatch; ++i)
	{
		xTempClient* pTempSocket = m_xAcceptConnectionQueue.pop();
		if (!pTempSocket) break;
		sendEvent(ISE_ONCONNECTION, pTempSocket->getId(), static_cast<LPVOID>(pTempSocket));
		pTempSocket->preDelete(5000);  // 设置5秒超时保护，高负载时过短易误杀新连接
		m_vPreDeleteWait.push_back(pTempSocket);  // 加入预删除等待列表
	}
	// 批量检查预删除项是否超时
	size_t writeIdx = 0;
	for (size_t i = 0; i < m_vPreDeleteWait.size(); ++i)
	{
		xTempClient* pTempSocket = m_vPreDeleteWait[i];
		if (pTempSocket->deleteTimeOut())
		{
			// 超时且未被接管，清理
			pTempSocket->close();
			m_xTempClientPool.deleteObject(pTempSocket);
		}
		else
		{
			// 尚未超时，保留在等待列表中
			m_vPreDeleteWait[writeIdx++] = pTempSocket;
		}
	}
	m_vPreDeleteWait.resize(writeIdx);
	// 批量处理断开连接
	int disconnectQueueSize = m_xDisconnectQueue.getcount();
	int maxDisconnectBatch = MIN(disconnectQueueSize, 64);
	for (int i = 0; i < maxDisconnectBatch; ++i)
	{
		xSocket* pSocket = m_xDisconnectQueue.pop();
		if (!pSocket) break;
		sendEvent(ISE_ONDISCONNECTION, 0, pSocket);
	}
}

xPacket* xIocpServer::newPacket()
{
	xPacket* pPacket = m_xPacketPool.newObject();
	if (pPacket == nullptr) return nullptr;
	if (pPacket->notcreated()) pPacket->create(DEF_PACKET_SIZE); // 从4KB增加到8KB
	return pPacket;
}

VOID xIocpServer::releasePacket(xPacket* pPacket)
{
	pPacket->clear();
	m_xPacketPool.deleteObject(pPacket);
}

xIocpUnit* xIocpServer::newIocpUnit()
{
	xIocpUnit* pIocpUnit = m_xIocpUnitPool.newObject();
	if (pIocpUnit == nullptr) return nullptr;
	pIocpUnit->setValidToken(IOCP_UNIT_VALID_TOKEN);
	return pIocpUnit;
}

VOID xIocpServer::releaseIocpUnit(xIocpUnit* pIocpUnit)
{
	pIocpUnit->setValidToken(0);
	pIocpUnit->setType(IO_NOTSET);
	pIocpUnit->setData(nullptr);
	pIocpUnit->setEventListener(nullptr);
	m_xIocpUnitPool.deleteObject(pIocpUnit);
}

BOOL xIocpServer::onConnection(xSocket* pSocket, UINT id)
{
	// 连接速率检查
	if (m_ConnectionRateTimer.IsTimeOut(1000))
	{
		m_ConnectionRateTimer.Savetime();
		m_dwConnectionsPerSecond = 0;
	}
	if (m_dwConnectionsPerSecond.fetch_add(1) >= m_dwMaxConnectionsPerSecond)
	{
		// 超过每秒连接限制，记录日志并返回FALSE，由调用方关闭socket
		setError(-1, "连接速率超限(%u/s)，拒绝新连接", m_dwMaxConnectionsPerSecond);
		return FALSE;
	}
	xTempClient* pTempSocket = m_xTempClientPool.newObject();
	pTempSocket->Clean();
	pTempSocket->steelSocket(*pSocket);
	m_xIocpManager.Bind(pTempSocket->getSocketFd(), 0);
	pTempSocket->setId(id);
	
	// 性能优化设置
	if (!pTempSocket->setTcpNoDelay(TRUE))// TCP_NODELAY 对游戏服务器很重要，记录警告
	{
		setError(-1, "警告: 客户端 %u 设置 TCP_NODELAY 失败", id);
	}
	if (!pTempSocket->setKeepAlive(TRUE, 10, 3))
	{
		setError(-1, "警告: 客户端 %u 设置 KeepAlive 失败", id);
	}
	pTempSocket->setSendBuffer(65536);    // 64KB发送缓冲区
	pTempSocket->setRecvBuffer(131072);   // 128KB接收缓冲区
	
	//pTempSocket->postRecv();
	pTempSocket->setServer(this);
	
	if (!m_xAcceptConnectionQueue.push(pTempSocket))
	{
		//队列满
		pTempSocket->close();
		m_xTempClientPool.deleteObject(pTempSocket);
		return FALSE;
	}
	return TRUE;
}

VOID xIocpServer::onDisconnect(xSocket* pSocket)
{
	m_xDisconnectQueue.push(pSocket);
}

BOOL xIocpServer::postConnection(const char* cp, UINT nPort, xSocket& socket)
{
	// 检查WSA是否已正确初始化
	if (!isWSAReady())
	{
		setError(-1, "无法连接服务器: WSA未正确初始化, 请检查网络子系统");
		return FALSE;
	}
	if (socket.connect(cp, nPort))
	{
		if (socket.getState() == xSocket::SS_CONNECTING)
		{
			// 使用5秒超时等待连接完成，确保目标服务器有足够时间完成初始化
			// 启动阶段目标服务器可能尚未就绪，100ms超时会导致连接频繁失败
			if (!socket.pollConnectResult(5000))
			{
				socket.close();
				return FALSE;
			}
		}
		// 性能优化设置（与onConnection保持一致）
		socket.setTcpNoDelay(TRUE);
		socket.setKeepAlive(TRUE, 10, 3);
		socket.setSendBuffer(65536);    // 64KB发送缓冲区
		socket.setRecvBuffer(131072);   // 128KB接收缓冲区
		
		if (m_xIocpManager.Bind(socket.getSocketFd(), 0))
			return TRUE;
		socket.close();
	}
	return FALSE;
}
