#pragma once
#include <array>
#include <string>
#include <thread>
#include "virtualserver.h"
#include "clientobject.h"
#include "ioconsole.h"
#include "scclientobj.h"
#include "dbclientobj.h"
#include "baniplistex.h"
#include "inc.h"
#include "msgprocessor.h"
#include "IoToMainQueue.h"

class CTempClient;
class CBaseServer :
	public CVirtualServer, public xEventListener, public CInputListener,
	public xThread, public CMsgProcessor
{
public:
	CBaseServer(VOID);
	virtual ~CBaseServer(VOID);
public:
	virtual VOID GetServerState(SERVERSTATE& state);
	BOOL Start(CSettingFile&);
	BOOL Stop();
	VOID UpdateTotalInfo();
public:
	VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size);
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	DWORD GetUpdateKey()const { return m_nLoopCount; }
	virtual CClientObject* NewClientObject() { return nullptr; }
	virtual VOID DeleteClientObject(CClientObject* pObject) {}
	virtual CClientObject* GetClientObject(UINT id) { return nullptr; }
	virtual BOOL InitServer(CSettingFile&) { return TRUE; }
	virtual VOID CleanServer() {}
	virtual VOID Update();
	virtual VOID OnDBMsg(PMIRMSG pMsg, int datasize) {}
	virtual VOID OnSCMsg(PMIRMSG pMsg, int datasize);
	virtual VOID OnSCServerReady() {}
	virtual VOID OnDBServerReady(int index) {}
	virtual VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize) {}
	virtual VOID WillClose() {}
public:
	// IOCP事件回调（在IOCP工作线程中执行）
	VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam);
public:
	//VOID ExectueProc( LPVOID lpParam );
	CIOConsole* GetIoConsole() { return m_pIoConsole; }
	VOID SetIoConsole(CIOConsole* pIoConsole) { m_pIoConsole = pIoConsole; }
	VOID SetServerName(const char* pServerName) { m_strServerName = pServerName ? pServerName : ""; }
	const char* GetServerName() { return m_strServerName.c_str(); }
	VOID SetNickName(const char* pNickName) { strncpy(m_szNickName.data(), pNickName, 64);m_szNickName[63] = 0; }
	const char* GetNickName() { return m_szNickName.data(); }
	CDBClientObj* GetDBConnection(int index) { if (index < 0 || index >= m_iDBServerCount) return nullptr;return &m_DBClientObjs[index]; }
	CSCClientObj* GetSCConnection() { return &m_SCClientObj; }
	int	GetDBConnectionCount()const { return m_iDBServerCount; }
	// 打印消息日志, type 0未知消息 / 1接收消息 / 2发送消息
	VOID OnUnknownMsg(PMIRMSG pMsg, int datasize, int type = 0);
	ServerId& GetServerId() { return m_Ident; }
	SERVERADDR* GetDBAddr(int index) { if (index < 0 || index >= m_iDBServerCount) return nullptr; return &m_DBAddrs[index]; }
	BOOL GetAddrByName(const char* pszHostName, char* pszIpAddress);
	virtual VOID OnInput(const char* pszString);
	// 启动IO线程
	VOID StartIoThread();
	// 停止IO线程
	VOID StopIoThread();
	// 主线程调用：消费IO线程投递的事件
	VOID ProcessIoMessages();
protected:
	VOID Execute(LPVOID lpParam);
	//	处理连接相关的事件, 连上和断开
	VOID ProcConnectionEvent();
	//	更新sc的消息
	VOID UpdateSCServer();
	//	更新db的消息
	VOID UpdateDBServer();
	//	连接sc的服务器
	BOOL ConnectSCServer();
	//	连接db的服务器
	BOOL ConnectDBServer();
	//VOID OnDisconnect(CSocketUnit*pUnit);
	std::string m_strServerName;
	std::array<char, 64> m_szNickName;
	CIOConsole* m_pIoConsole = nullptr;
	CSCClientObj m_SCClientObj;
	std::array<CDBClientObj, 2> m_DBClientObjs;
	servertype m_Type;
	ServerId m_Ident;
	SERVERADDR m_ServerAddr;
	SERVERADDR m_SCAddr;
	int m_iDBServerCount;
	std::array<SERVERADDR, 2> m_DBAddrs;
	//	for total
	SYSTEMTIME m_stStartTime;
	BOOL m_bStateInfoUpdated;
	FLOAT m_fLoopTime;
	DWORD m_dwLoopTimes;
	DWORD m_dwLoopStartTime;
	DWORD m_dwConnectedTimes;
	DWORD m_dwDisconnectTimes;
	std::array<CHAR, 1024> m_szServerState;
	DWORD m_dwPreConnections;
	xObjectPool<CTempClient > m_xTempClients;
	xPtrQueue<CTempClient> m_xQTempClient;
	std::array<char, 256> m_szBanIpFile;
	std::array<char, 256> m_szTrustIpFile;
	CIpListEx m_BanList;
	CIpListEx m_TrustList;
	xIocpServer	m_xIocpServer;

	std::thread m_IoThread;           // IO独立线程
	std::atomic<BOOL> m_bIoRunning{FALSE}; // IO线程运行标志
	IoToMainQueue m_IoToMainQueue;    // IO线程→主线程 MPSC无锁消息队列
	IoToMainMsgPool m_IoToMainMsgPool; // 消息对象池（配合指针语义队列）
private:
	DWORD m_nLoopCount; // 整体分帧更新索引
	std::array<CServerTimer, 2> m_dbReconnectTimers; // 每个DB服务器独立的重连间隔定时器
	CServerTimer m_scReconnectTimer; // 重连间隔
};
