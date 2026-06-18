#pragma once
#include "xsupport.h"
#include "xlistenobject.h"
#include "xiocpmanager.h"
#include "xsocket.h"
#include "xindexobject.h"
#include "xclientobject.h"
class xIocpUnit;

#define DEF_PACKET_SIZE	 8192

class xTempClient :
	public xClientObject
{
public:
	xTempClient()
	{
		m_bPreDeleted = FALSE;
	}

	VOID Clean()
	{
		xClientObject::Clean();
		m_bPreDeleted = FALSE;
	}

	VOID preDelete(DWORD dwTime = 0)
	{
		if (dwTime == 0)
			dwTime = 10000;
		m_deleteTimer.SetTimeOut(dwTime);
		m_deleteTimer.Savetime();
		m_bPreDeleted = TRUE;
	}

	BOOL deleteTimeOut(DWORD dwTime = 0)
	{
		if (dwTime > 0)
			return m_deleteTimer.IsTimeOut(dwTime);
		return m_deleteTimer.IsTimeOut();
	}

	BOOL isPreDeleted() { return m_bPreDeleted; }

private:
	BOOL m_bPreDeleted;
	CServerTimer m_deleteTimer;
};

class xIocpServer : public xError, public xEventSender
{
	// 禁止复制和移动
	xIocpServer(const xIocpServer&) = delete;
	xIocpServer& operator=(const xIocpServer&) = delete;
	xIocpServer(xIocpServer&&) = delete;
	xIocpServer& operator=(xIocpServer&&) = delete;
public:
	enum eventid
	{
		ISE_ID = 2000,
		ISE_ONCONNECTION,
		ISE_ONDISCONNECTION,
	};

	xIocpServer(VOID);
	virtual ~xIocpServer(VOID);

	BOOL postListen(const char* cp, UINT nPort, int nPostAccept = 64, UINT id = 0);
	BOOL postConnection(const char* cp, UINT nPort, xSocket& socket);
	BOOL start();
	BOOL stop();
	VOID update();

	xPacket* newPacket();
	VOID releasePacket(xPacket* pPacket);

	xIocpUnit* newIocpUnit();
	VOID releaseIocpUnit(xIocpUnit* pIocpUnit);

	BOOL onConnection(xSocket* pSocket, UINT id); // 返回TRUE=接受连接, FALSE=拒绝连接(需关闭socket)
	VOID onDisconnect(xSocket* pSocket);

	VOID addRecvBytes(DWORD dwBytes)
	{
		m_dwRecvBytes.fetch_add(dwBytes, std::memory_order_relaxed);
	}
	DWORD getRecvKBytes() const { return m_dwRecvBytes.load(std::memory_order_relaxed) / 1024; }
	VOID addSendBytes(DWORD dwBytes)
	{
		m_dwSendBytes.fetch_add(dwBytes, std::memory_order_relaxed);
	}
	DWORD getSendKBytes() const { return m_dwSendBytes.load(std::memory_order_relaxed) / 1024; }
	VOID getPacketCount(int& free, int& used, int& all)
	{
		free = m_xPacketPool.getFreeCount();
		used = m_xPacketPool.getUsedCount();
		all = m_xPacketPool.getCount();
	}
	VOID getUnitCount(int& free, int& used, int& all)
	{
		free = m_xIocpUnitPool.getFreeCount();
		used = m_xIocpUnitPool.getUsedCount();
		all = m_xIocpUnitPool.getCount();
	}
	BOOL hasPendingEvents()
	{
		return m_xAcceptConnectionQueue.getcount() > 0 || m_xDisconnectQueue.getcount() > 0;
	}
	BOOL isWSAReady() const { return m_bWSAInitialized; }
	// 活跃连接队列：IOCP 工作线程收包时推入，主线程消费
	VOID PushActiveClientQueue(xClientObject* pClient)
	{
		m_xActiveClientQueue.push(pClient);
	}
	xMpscQueue<xClientObject, 1024>& GetActiveClientQueue() { return m_xActiveClientQueue; }
protected:
private:
	std::atomic<DWORD> m_dwRecvBytes;
	std::atomic<DWORD> m_dwSendBytes;
	std::atomic<BOOL> m_bWSAInitialized; // WSA初始化状态
	xMpscQueue<xSocket, 1024> m_xDisconnectQueue;
	xMpscQueue<xTempClient, 1024> m_xAcceptConnectionQueue; // 新建连接
	std::vector<xTempClient*> m_vPreDeleteWait;  // 预删除等待列表
	xObjectPool<xListenObject> m_xListenObjectPool;
	xObjectPool<xPacket> m_xPacketPool;
	xObjectPool<xIocpUnit> m_xIocpUnitPool;
	xObjectPool<xTempClient> m_xTempClientPool;
	xIocpManager m_xIocpManager;
	xMpscQueue<xClientObject, 1024> m_xActiveClientQueue; // 活跃连接队列
	// 连接限流器
	std::atomic<DWORD> m_dwConnectionsPerSecond{ 0 };
	DWORD m_dwMaxConnectionsPerSecond = 100;  // 每秒最大新连接数
	CServerTimer m_ConnectionRateTimer;
};