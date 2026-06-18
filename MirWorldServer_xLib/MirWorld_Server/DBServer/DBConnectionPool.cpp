#include "StdAfx.h"
#include "DBConnectionPool.h"
#include "server.h"
#include "dbserver.h"

CMySQLConnectionPool::~CMySQLConnectionPool()
{
	Stop();
}

BOOL CMySQLConnectionPool::Start(const char* pszServer, const char* pszPort,
	const char* pszDBName, const char* pszId, const char* pszPassword,
	int nMinPoolSize, int nMaxPoolSize)
{
	if (m_bRunning.load()) return FALSE;
	m_szServer = pszServer ? pszServer : "";
	m_szPort = pszPort ? pszPort : "3306";
	m_szDBName = pszDBName ? pszDBName : "";
	m_szId = pszId ? pszId : "";
	m_szPassword = pszPassword ? pszPassword : "";
	m_nMinPoolSize = (nMinPoolSize < 1) ? 1 : nMinPoolSize;
	m_nMaxPoolSize = (nMaxPoolSize < m_nMinPoolSize) ? m_nMinPoolSize : nMaxPoolSize;
	m_bRunning.store(TRUE);
	m_lastKeepAliveTime = std::chrono::steady_clock::now();

	// 预创建最小数量的连接
	int nCreated = 0;
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		for (int i = 0; i < m_nMinPoolSize; i++)
		{
			CMySQLDBConnection* pConn = CreateNewConnectionLocked();
			if (pConn)
			{
				m_freeConnections.emplace(pConn, std::chrono::steady_clock::now());
				nCreated++;
			}
			else
				PRINT(ERROR_RED, "数据库连接池: 预创建连接 %d/%d 失败\n", i + 1, m_nMinPoolSize);
		}
	}
	// 至少需要创建1个连接才算启动成功，否则MySQL不可用
	if (nCreated == 0)
	{
		m_bRunning.store(FALSE);
		PRINT(ERROR_RED, "MySQL连接池启动失败: 无法创建任何连接, 请检查MySQL服务是否运行\n");
		return FALSE;
	}
	PRINT(SUCCESS_GREEN, "MySQL连接池启动: 预创建 %d/%d 连接, 最大 %d\n",
		nCreated, m_nMinPoolSize, m_nMaxPoolSize);
	return TRUE;
}

VOID CMySQLConnectionPool::Stop()
{
	if (!m_bRunning.load()) return;
	m_bRunning.store(FALSE);
	std::lock_guard<std::mutex> lock(m_poolMutex);
	// 关闭所有连接
	for (auto& pConn : m_allConnections)
	{
		if (pConn) pConn->UnInit();
	}
	m_allConnections.clear();
	// 清空空闲队列
	while (!m_freeConnections.empty())
		m_freeConnections.pop();
}

CMySQLDBConnection* CMySQLConnectionPool::Acquire()
{
	if (!m_bRunning.load()) return nullptr;
	// 在锁内快速取出一个空闲连接
	CMySQLDBConnection* pConn = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		while (!m_freeConnections.empty())
		{
			pConn = m_freeConnections.front().pConn;
			m_freeConnections.pop();
			break; // 取出一个，锁外验证
		}
	}
	// 锁外验证连接有效性（网络IO）
	if (pConn != nullptr)
	{
		if (ValidateConnection(pConn))
			return pConn;
		else
		{
			PRINT(ERROR_RED, "数据库连接池: 空闲连接无效且重连失败，移除\n");
			std::lock_guard<std::mutex> lock(m_poolMutex);
			RemoveConnectionLocked(pConn);
		}
	}
	// 没有空闲连接，尝试创建新的
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		if ((int)m_allConnections.size() < m_nMaxPoolSize)
		{
			CMySQLDBConnection* pNewConn = CreateNewConnectionLocked();
			if (pNewConn) return pNewConn;
		}
	}
	PRINT(ERROR_RED, "数据库连接池: 已达上限 %d，无法分配新连接\n", m_nMaxPoolSize);
	return nullptr;
}

VOID CMySQLConnectionPool::Release(CMySQLDBConnection* pConnection)
{
	if (pConnection == nullptr) return;
	// 清理 DataUnit，保留 MySQL 句柄
	pConnection->ResetForPoolReuse();

	if (!m_bRunning.load())
	{
		// 池已停止，直接关闭
		std::lock_guard<std::mutex> lock(m_poolMutex);
		RemoveConnectionLocked(pConnection);
		return;
	}

	// 锁外验证连接有效性
	bool bValid = pConnection->IsConnectionValid();
	if (!bValid)
	{
		// 尝试重连（网络IO，不持锁）
		bValid = (pConnection->Reconnect() == SE_OK);
	}

	std::lock_guard<std::mutex> lock(m_poolMutex);
	if (bValid)
	{
		m_freeConnections.emplace(pConnection, std::chrono::steady_clock::now());
	}
	else
	{
		PRINT(ERROR_RED, "数据库连接池: 归还连接重连失败，移除\n");
		RemoveConnectionLocked(pConnection);
	}
}

VOID CMySQLConnectionPool::Maintain()
{
	if (!m_bRunning.load()) return;

	auto now = std::chrono::steady_clock::now();
	auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastKeepAliveTime).count();
	if (elapsedSec < POOL_KEEPALIVE_INTERVAL_SEC)
		return;

	m_lastKeepAliveTime = now;

	// 收集需要保活和回收的连接
	std::vector<CMySQLDBConnection*> toKeepAlive;
	std::vector<CMySQLDBConnection*> toRemove;

	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		std::queue<FreeConnEntry> newFreeQueue;

		while (!m_freeConnections.empty())
		{
			auto& entry = m_freeConnections.front();
			auto idleSec = std::chrono::duration_cast<std::chrono::seconds>(now - entry.idleSince).count();

			if (idleSec >= POOL_MAX_IDLE_TIME_SEC && (int)m_allConnections.size() > m_nMinPoolSize)
			{
				// 空闲超时且总数超过最小值，回收
				toRemove.push_back(entry.pConn);
			}
			else
			{
				// 需要保活
				toKeepAlive.push_back(entry.pConn);
				newFreeQueue.emplace(entry.pConn, entry.idleSince); // 保留原始空闲时间
			}
			m_freeConnections.pop();
		}
		m_freeConnections.swap(newFreeQueue);
	}

	// 锁外执行网络IO：保活 ping
	for (auto* pConn : toKeepAlive)
	{
		if (!pConn->IsConnectionValid())
		{
			if (pConn->Reconnect() != SE_OK)
			{
				PRINT(ERROR_RED, "数据库连接池: 保活ping失败且重连失败，移除\n");
				std::lock_guard<std::mutex> lock(m_poolMutex);
				RemoveConnectionLocked(pConn);
			}
		}
	}

	// 锁外关闭超时连接
	for (auto* pConn : toRemove)
	{
		PRINT(STRING_GREEN, "数据库连接池: 回收空闲超时连接\n");
		pConn->UnInit();
		std::lock_guard<std::mutex> lock(m_poolMutex);
		RemoveConnectionLocked(pConn);
	}

	// 自动补充：确保总连接数不低于 MinPoolSize
	// 当连接因保活失败或空闲超时被移除后，池中连接数可能低于最小值
	// 在此补充创建，保证后续 Acquire() 有连接可用
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		int nTotal = (int)m_allConnections.size();
		int nFree = (int)m_freeConnections.size();
		int nNeed = m_nMinPoolSize - nTotal;
		for (int i = 0; i < nNeed; i++)
		{
			CMySQLDBConnection* pConn = CreateNewConnectionLocked();
			if (pConn)
				m_freeConnections.emplace(pConn, std::chrono::steady_clock::now());
			else
			{
				PRINT(ERROR_RED, "数据库连接池: 补充连接失败(%d/%d), MySQL可能不可用\n", i, nNeed);
				break;
			}
		}
		if (nNeed > 0)
		{
			PRINT(SUCCESS_GREEN, "数据库连接池: 补充连接 %d/%d, 当前总数=%d 空闲=%d\n",
				(nNeed - (m_nMinPoolSize - (int)m_allConnections.size())), nNeed, (int)m_allConnections.size(), (int)m_freeConnections.size());
		}
	}
}

int CMySQLConnectionPool::GetTotalCount() const
{
	std::lock_guard<std::mutex> lock(m_poolMutex);
	return (int)m_allConnections.size();
}

int CMySQLConnectionPool::GetFreeCount() const
{
	std::lock_guard<std::mutex> lock(m_poolMutex);
	return (int)m_freeConnections.size();
}

CMySQLDBConnection* CMySQLConnectionPool::CreateNewConnectionLocked()
{
	auto pConn = std::make_unique<CMySQLDBConnection>();
	if (!pConn) return nullptr;
	// 初始化 MySQL 句柄
	if (pConn->Init(nullptr) != SE_OK) return nullptr;
	// 连接数据库
	if (pConn->Connect(m_szServer.c_str(), m_szPort.c_str(),
		m_szDBName.c_str(), m_szId.c_str(), m_szPassword.c_str()) != SE_OK)
	{
		pConn->UnInit();
		return nullptr;
	}
	CMySQLDBConnection* pRaw = pConn.get();
	m_allConnections.push_back(std::move(pConn));
	return pRaw;
}

BOOL CMySQLConnectionPool::ValidateConnection(CMySQLDBConnection* pConnection)
{
	if (pConnection == nullptr) return FALSE;
	if (pConnection->IsConnectionValid()) return TRUE;
	// 尝试重连
	return (pConnection->Reconnect() == SE_OK);
}

VOID CMySQLConnectionPool::RemoveConnectionLocked(CMySQLDBConnection* pConnection)
{
	for (auto it = m_allConnections.begin(); it != m_allConnections.end(); ++it)
	{
		if (it->get() == pConnection)
		{
			pConnection->UnInit();
			m_allConnections.erase(it);
			break;
		}
	}
}
