#include "StdAfx.h"
#include ".\server.h"
#include "time.h"
#include "dbserver.h"

CServer::CServer(VOID)
{
	m_szServer.fill(0);
	m_szPort.fill(0);
	m_szDatabase.fill(0);
	m_szAccount.fill(0);
	m_szPassword.fill(0);
	m_dwMsgTimes.fill(0);
	m_dwFlushTimer = 0;
}

CServer::~CServer(VOID)
{
}

CClientObject* CServer::NewClientObject()
{
	CClientObj* pObj = newObject();
	// 使用连接池获取数据库连接
	if (pObj->GetAppDB().OpenDataBase(m_connectionPool) != SE_OK)
	{
		PRINT(ERROR_RED, "无法从连接池获取数据库连接!\n");
		deleteObject(pObj);
		return nullptr;
	}
	return pObj;
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	char* pszServer = (char*)s.GetString(m_strServerName.c_str(), "DbServer", "localhost");
	char* pszPort = (char*)s.GetString(m_strServerName.c_str(), "DbPort", "3306");
	char* pszDatabase = (char*)s.GetString(m_strServerName.c_str(), "DbName", "mirworlddb");
	char* szAccount = (char*)s.GetString(m_strServerName.c_str(), "DbAccount", "root");
	char* szPassword = (char*)s.GetString(m_strServerName.c_str(), "DbPassword", "root");

	strncpy(m_szServer.data(), pszServer, 63);
	strncpy(m_szPort.data(), pszPort, 63);
	strncpy(m_szDatabase.data(), pszDatabase, 63);
	strncpy(m_szAccount.data(), szAccount, 63);
	strncpy(m_szPassword.data(), szPassword, 63);

	if (m_appDB.OpenDataBase(pszServer, pszPort, pszDatabase, szAccount, szPassword) != SE_OK)
	{
		PRINT(ERROR_RED, "无法连接数据库, 请检查服务器是否开启!\n");
		return FALSE;
	}
	else
		PRINT(SUCCESS_GREEN, "连接数据库成功!\n");
	m_appDB.DoInit();
	// 启动连接池
	int nMinPool = s.GetInteger(m_strServerName.c_str(), "MinPoolSize", 4);
	int nMaxPool = s.GetInteger(m_strServerName.c_str(), "MaxPoolSize", 32);
	if (m_connectionPool.Start(pszServer, pszPort, pszDatabase, szAccount, szPassword, nMinPool, nMaxPool))
		PRINT(SUCCESS_GREEN, "数据库连接池启动成功! (最小%d, 最大%d)\n", nMinPool, nMaxPool);
	else
		PRINT(ERROR_RED, "数据库连接池启动失败!\n");
	// 启用延迟写入
	m_appDB.EnableDeferredWrite(true);
	// 启动异步查询线程池
	int nQueryThreads = s.GetInteger(m_strServerName.c_str(), "QueryThreads", 4);
	if (!m_queryPool.Start(nQueryThreads, pszServer, pszPort, pszDatabase, szAccount, szPassword))
	{
		PRINT(ERROR_RED, "查询线程池启动失败!\n");
		// 非致命错误，降级为同步模式继续运行
	}
	m_xxShowTimer.Savetime();
	return TRUE;
}

VOID CServer::CleanServer()
{
	// 1. 先刷盘服务器级别的延迟写入
	m_appDB.FlushDeferredWrites();
	// 2. 停止查询线程池（等待已提交任务完成，不再接受新任务）
	m_queryPool.Stop();
	// 3. 刷盘所有客户端连接的延迟写入（仅处理已连接的客户端）
	CClientObj* pObj = m_ObjectPool.First();
	while (pObj)
	{
		// 仅对已连接且有待写入数据的客户端刷盘，避免访问已回收对象
		if (pObj->IsConnected() && pObj->GetAppDB().GetDeferredWriteCount() > 0)
		{
			SERVER_ERROR ret = pObj->GetAppDB().FlushDeferredWrites();
			if (ret != SE_OK)
				PRINT(ERROR_RED, "客户端延迟写入刷盘失败, 错误=%d\n", ret);
		}
		pObj = m_ObjectPool.Next();
	}
	// 4. 最后关闭连接池
	m_connectionPool.Stop();
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}
	// 消费异步查询结果（在主线程上下文中执行回调，安全调用 SendMsg）
	m_queryPool.ProcessCompletedResults();
	// 定时刷盘（每5秒刷一次延迟写入缓冲）
	m_dwFlushTimer++;
	if (m_dwFlushTimer >= 300) // 假设60fps，300帧≈5秒
	{
		m_dwFlushTimer = 0;
		if (m_appDB.GetDeferredWriteCount() > 0)
			m_appDB.FlushDeferredWrites();
	}
	// 连接池维护：保活 + 空闲回收
	m_connectionPool.Maintain();
	// 帧末：刷新所有连接的批量发送缓冲区
	CClientObj* pFlushObj = m_ObjectPool.First();
	while (pFlushObj)
	{
		if (pFlushObj->IsConnected() && pFlushObj->IsBatchMode())
			pFlushObj->FlushMsgQueue();
		pFlushObj = m_ObjectPool.Next();
	}
	UpdateSCServer();
}

VOID CServer::OnParseMsg(WORD wMsg)
{
	if (wMsg >= DM_START && wMsg < DM_END)
		this->m_dwMsgTimes[wMsg]++;
}

VOID CServer::OnInput(const char* pString)
{
	// 先处理DBServer自有命令
	std::array<char, 256> szLine{};
	o_strncpy(szLine.data(), pString, szLine.size());
	xStringsExtracter<16> cmd(szLine.data(), " \t,", " \t");
	if (static_cast<int>(cmd.getCount()) > 0)
	{
		if (_stricmp(cmd[0], "poolinfo") == 0)
		{
			// 显示连接池状态
			int nTotal = m_connectionPool.GetTotalCount();
			int nFree = m_connectionPool.GetFreeCount();
			int nInUse = nTotal - nFree;
			PRINT(SUCCESS_GREEN, "===== 数据库连接池状态 =====\n");
			PRINT(SUCCESS_GREEN, "总连接数: %d  空闲: %d  使用中: %d\n", nTotal, nFree, nInUse);
			PRINT(SUCCESS_GREEN, "查询线程池: 待处理=%d 已完成=%d 运行=%s\n",
				m_queryPool.GetPendingTaskCount(),
				m_queryPool.GetCompletedResultCount(),
				m_queryPool.IsRunning() ? "是" : "否");
			// 统计当前在线客户端数
			int nClients = 0;
			CClientObj* pObj = m_ObjectPool.First();
			while (pObj)
			{
				if (pObj->IsConnected()) nClients++;
				pObj = m_ObjectPool.Next();
			}
			PRINT(SUCCESS_GREEN, "在线客户端: %d\n", nClients);
			PRINT(SUCCESS_GREEN, "============================\n");
			return;
		}
	}
	// 非自有命令交给基类处理
	CBaseServer::OnInput(pString);
}
