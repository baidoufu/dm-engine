#include "..\header\xclientobject.h"
#include "..\header\xIocpUnit.h"
#include "..\header\xpacket.h"
#include "..\header\xIocpServer.h"

xClientObject::xClientObject(VOID)
	: m_dwSendBytes(0),
	  m_dwRecvBytes(0),
	  m_bQueryDisconnect(FALSE),
	  m_bBatchMode(FALSE)
{
	// 初始化批量缓冲区
	m_pBatchPacket = std::make_unique<xPacket>();
	m_pBatchPacket->create(DEF_PACKET_SIZE);
}

xClientObject::~xClientObject(VOID)
{
}

BOOL xClientObject::postSend(xPacket* pPacket)
{
	auto* pServer = getServer();
	if (pServer == nullptr)
	{
		setError(-7, "没有设置 Server 对象");
		return FALSE;
	}
	if (!IsConnected()) return FALSE;

	xIocpUnit* pIocpUnit = pServer->newIocpUnit();
	if (pIocpUnit == nullptr)
	{
		pServer->releasePacket(pPacket);
		return FALSE;
	}
	pIocpUnit->setEventListener(this);
	pIocpUnit->setData(pPacket);
	pIocpUnit->setType(IO_SEND);

	if (!postSend(pIocpUnit, pPacket))
	{
		pServer->releaseIocpUnit(pIocpUnit);
		pServer->releasePacket(pPacket);
		return FALSE;
	}
	return TRUE;
}

BOOL xClientObject::postSend(LPVOID lpData, int nSize)
{
	auto* pServer = getServer();
	if (pServer == nullptr)
	{
		setError(-7, "没有设置 Server 对象");
		return FALSE;
	}
	if (!IsConnected()) return FALSE;
	// 对于小于 DEF_PACKET_SIZE 的数据，使用 xPacket
	// 对于更大的数据，考虑直接分配临时缓冲区进行单次 WSASend
	if (nSize <= DEF_PACKET_SIZE)
	{
		xPacket* pPacket = pServer->newPacket();
		if (pPacket == nullptr) 
		{ 
			Disconnect(); 
			return FALSE;
		}
		pPacket->push(lpData, nSize);
		if (!postSend(pPacket))
		{
			pServer->releasePacket(pPacket);
			Disconnect();
			return FALSE;
		}
		return TRUE;
	}
	// 大数据：复制到 xPacket 中发送，防止调用者在异步完成前释放缓冲区导致 use-after-free
	xPacket* pPacket = pServer->newPacket();
	if (pPacket == nullptr)
	{
		Disconnect();
		return FALSE;
	}
	// 对象池 xPacket 默认 DEF_PACKET_SIZE 大小，大数据需要重新分配
	pPacket->create(nSize);
	pPacket->push(lpData, nSize);
	if (!postSend(pPacket))
	{
		pServer->releasePacket(pPacket);
		Disconnect();
		return FALSE;
	}
	return TRUE;
}

BOOL xClientObject::postSendBatch(LPVOID lpData, int nSize)
{
	if (!IsConnected()) return FALSE;
	if (!m_bBatchMode) return postSend(lpData, nSize);
	if (nSize <= 0) return TRUE;
	// 缓冲区剩余空间不足，先刷新再追加
	if (m_pBatchPacket->getfreesize() < nSize)
	{
		flushBatch();
		// 如果单条消息就超过缓冲区最大容量，直接发送
		if (nSize > m_pBatchPacket->getmaxsize())
			return postSend(lpData, nSize);
	}
	// 追加到批量缓冲区
	m_pBatchPacket->push(lpData, nSize);
	// 超过阈值，立即刷新
	if (m_pBatchPacket->getsize() >= DEF_PACKET_SIZE)
		return flushBatch();

	return TRUE;
}

BOOL xClientObject::flushBatch()
{
	if (m_pBatchPacket == nullptr || m_pBatchPacket->getsize() == 0)
		return TRUE;
	if (!IsConnected())
	{
		m_pBatchPacket->clear();
		return FALSE;
	}
	BOOL result = postSend(static_cast<LPVOID>(const_cast<char*>(m_pBatchPacket->getbuf())), m_pBatchPacket->getsize());
	m_pBatchPacket->clear();
	return result;
}

BOOL xClientObject::postRecv()
{
	if (!IsConnected())
		return FALSE;
	auto* pServer = getServer();
	if (pServer == nullptr) return FALSE;
	xPacket* pPacket = pServer->newPacket();
	if (pPacket == nullptr) return FALSE;
	xIocpUnit* pIocpUnit = pServer->newIocpUnit();
	if (pIocpUnit == nullptr)
	{
		pServer->releasePacket(pPacket);
		return FALSE;
	}
	pIocpUnit->setEventListener(this);
	pIocpUnit->setData(pPacket);
	pIocpUnit->setType(IO_READ);

	if (!postRecv(pIocpUnit, pPacket))
	{
		pServer->releaseIocpUnit(pIocpUnit);
		pServer->releasePacket(pPacket);
		Disconnect();
		return FALSE;
	}
	return TRUE;
}

VOID xClientObject::OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam)
{
	auto* pServer = getServer();
	if (pServer == nullptr)
	{
		Disconnect();
		return;
	}
	switch (iEvent)
	{
	case xIocpUnit::IUE_SEND:
	{
		m_stats.bytes_sent += iParam;
		m_stats.packets_sent++;
		m_stats.last_activity = getCurrentTime();

		auto* pIocpUnit = static_cast<xIocpUnit*>(lpParam);
		auto* pPacket = static_cast<xPacket*>(pIocpUnit->getData());
		addSendBytes(iParam);
		pServer->addSendBytes(iParam);

		if (pPacket != nullptr)
		{
			int sendsize = pPacket->getsize();
			DWORD dwNewOffset = pIocpUnit->getSendOffset() + iParam;

			if (dwNewOffset < (DWORD)sendsize)
			{
				// 部分发送：更新偏移量，重新投递 WSASend 发送剩余数据
				pIocpUnit->setSendOffset(dwNewOffset);
				if (!postSend(pIocpUnit, pPacket))
				{
					pServer->releaseIocpUnit(pIocpUnit);
					pServer->releasePacket(pPacket);
					Disconnect();
				}
				return;
			}
			// 全部发送完成，释放资源
			pServer->releaseIocpUnit(pIocpUnit);
			pServer->releasePacket(pPacket);
		}
		else
		{
			pServer->releaseIocpUnit(pIocpUnit);
		}
	}
	break;
	case xIocpUnit::IUE_READ:
	{
		m_stats.bytes_received += iParam;
		m_stats.packets_received++;
		m_stats.last_activity = getCurrentTime();

		auto* pIocpUnit = static_cast<xIocpUnit*>(lpParam);
		auto* pPacket = static_cast<xPacket*>(pIocpUnit->getData());
		BOOL bDisconnect = FALSE;
		pPacket->setsize(iParam);
		if (iParam == 0)
		{
			bDisconnect = TRUE; //	断开连接
		}
		else
		{
			addRecvBytes(iParam);
			pServer->addRecvBytes(iParam);
			//	失败断开连接
			if (m_xPacketQueue.push(pPacket))
			{
				// 首次收包时推入活跃队列（CAS 避免重复推入）
				BOOL expected = FALSE;
				if (m_bInActiveList.compare_exchange_strong(expected, TRUE))
				{
					pServer->PushActiveClientQueue(this);
				}
				pPacket = pServer->newPacket();
				if (pPacket == nullptr)
				{
					// 对象池耗尽，释放 pIocpUnit 并断开连接
					if (pIocpUnit) pServer->releaseIocpUnit(pIocpUnit);
					Disconnect();
					break;
				}
				pIocpUnit->setData(pPacket);
				if (!postRecv(pIocpUnit, pPacket))
				{
					if (pIocpUnit) pServer->releaseIocpUnit(pIocpUnit);
					bDisconnect = TRUE;
				}
			}
			else
			{
				// 收包队列满，记录日志后断开连接
				setError(-1, "收包队列满，断开客户端连接");
				bDisconnect = TRUE;
			}
		}
		if (bDisconnect)
		{
			pServer->releaseIocpUnit(pIocpUnit);
			pServer->releasePacket(pPacket);
			Disconnect();
		}
	}
	break;
	case xIocpUnit::IUE_ERROR:
	{
		// I/O操作失败，释放资源并断开连接
		auto* pIocpUnit = static_cast<xIocpUnit*>(lpParam);
		auto* pPacket = static_cast<xPacket*>(pIocpUnit->getData());
		pServer->releaseIocpUnit(pIocpUnit);
		if (pPacket) pServer->releasePacket(pPacket);
		Disconnect();
	}
	break;
	}
}

BOOL xClientObject::postSend(xIocpUnit* pIocpUnit, xPacket* pPacket)
{
	DWORD dwOffset = pIocpUnit->getSendOffset();
	return sendEx(const_cast<char*>(pPacket->getbuf()) + dwOffset, pPacket->getsize() - dwOffset,
		pIocpUnit->getOverlappedEx()->dwNumberOfBytes,
		pIocpUnit->getOverlappedEx()->dwFlag,
		pIocpUnit->getOverlapped());
}

BOOL xClientObject::postRecv(xIocpUnit* pIocpUnit, xPacket* pPacket)
{
	return recvEx(const_cast<char*>(pPacket->getbuf()), pPacket->getmaxsize(),
		pIocpUnit->getOverlappedEx()->dwNumberOfBytes,
		pIocpUnit->getOverlappedEx()->dwFlag,
		pIocpUnit->getOverlapped());
}

VOID xClientObject::Update()
{
	if (!IsConnected()) return;
	auto* pServer = getServer();
	// 动态调整每帧包处理上限：队列积压时提高处理量，防止消息延迟
	const int MAX_PROCESS_PER_FRAME = 32;
	int nCount = m_xPacketQueue.getcount();
	int nProcessLimit = MAX_PROCESS_PER_FRAME * ceil_div(nCount, MAX_PROCESS_PER_FRAME);
	xPacket* pPacket = nullptr;
	int nProcessed = 0;
	while (nProcessed < nCount && nProcessed < nProcessLimit)
	{
		pPacket = m_xPacketQueue.pop();
		if (pPacket == nullptr) break;
		OnDataPacket(pPacket);
		if (pServer) pServer->releasePacket(pPacket);
		nProcessed++;
	}

	if (m_bQueryDisconnect && this->m_xDisconnectTimer.IsTimeOut())
		Disconnect();
}

BOOL xClientObject::IsConnected()
{
	if (getState() == SS_ACCEPTED || getState() == SS_CONNECTED || getState() == SS_CONNECTING)
		return TRUE;
	return FALSE;
}

VOID xClientObject::Disconnect(DWORD dwTimeOut)
{
	if (dwTimeOut == 0)
	{
		// 使用原子CAS确保只有一个线程执行断开逻辑，避免竞态条件导致重复断开
		socket_state expected = SS_ACCEPTED;
		if (!m_state.compare_exchange_strong(expected, SS_DISCONNECTED))
		{
			expected = SS_CONNECTING;
			if (!m_state.compare_exchange_strong(expected, SS_DISCONNECTED))
			{
				expected = SS_CONNECTED;
				if (!m_state.compare_exchange_strong(expected, SS_DISCONNECTED))
					return; // 已被其他线程断开，直接返回
			}
		}
		close();
		auto* pServer = getServer();
		if (pServer) pServer->onDisconnect(this);
	}
	else if (m_bQueryDisconnect == FALSE)
	{
		m_bQueryDisconnect = TRUE;
		this->m_xDisconnectTimer.Savetime();
		this->m_xDisconnectTimer.SetTimeOut(dwTimeOut);
	}
}

VOID xClientObject::Clean()
{
	m_dwRecvBytes = 0;
	m_dwSendBytes = 0;
	m_bQueryDisconnect = FALSE;
	m_stats = SocketStats{};
	xPacket* pPacket = nullptr;
	auto* pServer = getServer();
	while (pPacket = m_xPacketQueue.pop())
	{
		if (pServer) pServer->releasePacket(pPacket);
	}
	if (m_pBatchPacket)
		m_pBatchPacket->clear();
	clear();
	xIndexObject::Clean();
}
