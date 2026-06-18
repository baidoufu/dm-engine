#pragma once
#include "xsocket.h"
#include "xserverobject.h"
#include "xsupport.h"
#include "xindexobject.h"
#include <memory>
class xPacket;
class xIocpUnit;

class xClientObject :
	public xSocket,
	public xServerObject,
	public xEventListener,
	public xIndexObject
{
public:
	xClientObject(VOID);
	virtual ~xClientObject(VOID);
	BOOL postSend(xPacket* pPacket);
	BOOL postSend(LPVOID lpData, int nSize);
	BOOL postRecv();
	VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam);

	virtual VOID Update();
	virtual VOID OnDataPacket(xPacket* pPacket) {}
	virtual VOID OnDisconnect() {}
	virtual VOID Clean();

	BOOL IsConnected();
	VOID Disconnect(DWORD dwTimeOut = 0);

	VOID addSendBytes(DWORD dwBytes)
	{
		m_dwSendBytes.fetch_add(dwBytes, std::memory_order_relaxed);
	}
	DWORD getSendBytes() { return m_dwSendBytes.load(std::memory_order_relaxed); }

	VOID addRecvBytes(DWORD dwBytes)
	{
		m_dwRecvBytes.fetch_add(dwBytes, std::memory_order_relaxed);
	}
	DWORD getRecvBytes() { return m_dwRecvBytes.load(std::memory_order_relaxed); }

	// 批量发送接口：将消息追加到批量缓冲区
	BOOL postSendBatch(LPVOID lpData, int nSize);
	// 刷新批量缓冲区（将累积的消息一次性发出）
	BOOL flushBatch();
	// 活跃连接标记：首次收包时 CAS false→true 并推入活跃队列，避免重复推入
	std::atomic<BOOL> m_bInActiveList{ FALSE };
private:
	std::atomic<DWORD> m_dwSendBytes;
	std::atomic<DWORD> m_dwRecvBytes;
	BOOL postSend(xIocpUnit* pIocpUnit, xPacket* pPacket);
	BOOL postRecv(xIocpUnit* pIocpUnit, xPacket* pPacket);
protected:
	xMpscQueue<xPacket, 128> m_xPacketQueue;
	CServerTimer m_xDisconnectTimer;
	BOOL m_bQueryDisconnect;
	// 批量发送相关
	std::unique_ptr<xPacket> m_pBatchPacket; // 批量发送缓冲区指针
	BOOL m_bBatchMode;       // 批量模式是否启用
};