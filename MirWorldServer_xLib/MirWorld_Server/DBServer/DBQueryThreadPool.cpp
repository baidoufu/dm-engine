#include "StdAfx.h"
#include "DBQueryThreadPool.h"
#include "AppDB.h"
#include "server.h"
#include "dbserver.h"

CDBQueryThreadPool::CDBQueryThreadPool()
{
}

CDBQueryThreadPool::~CDBQueryThreadPool()
{
	Stop();
}

BOOL CDBQueryThreadPool::Start(int nThreadCount,
	const char* pszServer, const char* pszPort,
	const char* pszDBName, const char* pszId, const char* pszPassword)
{
	if (m_bRunning.load())
		return FALSE;

	// 保存连接参数（工作线程使用）
	strncpy_s(m_szServer.data(), m_szServer.size(), pszServer, _TRUNCATE);
	strncpy_s(m_szPort.data(), m_szPort.size(), pszPort, _TRUNCATE);
	strncpy_s(m_szDBName.data(), m_szDBName.size(), pszDBName, _TRUNCATE);
	strncpy_s(m_szId.data(), m_szId.size(), pszId, _TRUNCATE);
	strncpy_s(m_szPassword.data(), m_szPassword.size(), pszPassword, _TRUNCATE);

	// 确保线程数合理
	if (nThreadCount < 1) nThreadCount = 1;
	if (nThreadCount > 16) nThreadCount = 16;

	m_bRunning.store(true);

	// 创建查询线程
	for (int i = 0; i < nThreadCount; i++)
	{
		m_threads.emplace_back(&CDBQueryThreadPool::WorkerThreadProc, this, i);
	}

	PRINT(SUCCESS_GREEN, "DB异步查询线程池启动: %d 个工作线程\n", nThreadCount);
	return TRUE;
}

VOID CDBQueryThreadPool::Stop()
{
	if (!m_bRunning.load())
		return;

	m_bRunning.store(false);

	// 唤醒所有等待的工作线程
	m_taskCV.notify_all();

	// 等待所有工作线程退出
	for (auto& thread : m_threads)
	{
		if (thread.joinable())
			thread.join();
	}
	m_threads.clear();

	// 清空队列
	{
		std::lock_guard<std::mutex> lock(m_taskMutex);
		std::queue<DBAsyncWork> empty;
		m_taskQueue.swap(empty);
	}
	{
		std::lock_guard<std::mutex> lock(m_resultMutex);
		std::queue<std::function<void()>> empty;
		m_resultQueue.swap(empty);
	}

	m_nPendingTasks.store(0);
	m_nCompletedResults.store(0);
}

// ============================================================================
// 主线程调用：投递异步工作
// ============================================================================
BOOL CDBQueryThreadPool::SubmitWork(DBAsyncWork&& work)
{
	if (!m_bRunning.load())
		return FALSE;

	{
		std::lock_guard<std::mutex> lock(m_taskMutex);
		m_taskQueue.push(std::move(work));
	}
	m_nPendingTasks.fetch_add(1, std::memory_order_relaxed);

	// 唤醒一个工作线程
	m_taskCV.notify_one();
	return TRUE;
}

// ============================================================================
// 主线程调用：每帧消费完成结果
// ============================================================================
VOID CDBQueryThreadPool::ProcessCompletedResults()
{
	// 批量取出所有结果，避免长时间持锁
	std::queue<std::function<void()>> localResults;
	{
		std::lock_guard<std::mutex> lock(m_resultMutex);
		m_resultQueue.swap(localResults);
	}

	// 在主线程上下文中执行回调（可安全调用 SendMsg 等）
	while (!localResults.empty())
	{
		auto& func = localResults.front();
		if (func)
		{
			func();
		}
		m_nCompletedResults.fetch_sub(1, std::memory_order_relaxed);
		localResults.pop();
	}
}

// ============================================================================
// 获取待处理任务数
// ============================================================================
int CDBQueryThreadPool::GetPendingTaskCount() const
{
	return m_nPendingTasks.load(std::memory_order_relaxed);
}

// ============================================================================
// 获取已完成结果数
// ============================================================================
int CDBQueryThreadPool::GetCompletedResultCount() const
{
	return m_nCompletedResults.load(std::memory_order_relaxed);
}

// ============================================================================
// 查询线程入口
// ============================================================================
VOID CDBQueryThreadPool::WorkerThreadProc(int nThreadIndex)
{
	// 每个工作线程创建独立的 CAppDB + MySQL 连接
	// 连接失败时重试，避免线程退出导致已提交任务的 queryFunc/resultFunc 永远不执行
	CAppDB appDB;
	{
		int nRetryCount = 0;
		while (m_bRunning.load())
		{
			if (appDB.OpenDataBase(m_szServer.data(), m_szPort.data(),
				m_szDBName.data(), m_szId.data(), m_szPassword.data()) == SE_OK)
				break;

			nRetryCount++;
			PRINT(ERROR_RED, "DB查询线程[%d] 连接数据库失败(第%d次), 5秒后重试...\n", nThreadIndex, nRetryCount);

			// 等待5秒，期间检查线程池是否停止
			{
				std::unique_lock<std::mutex> lock(m_taskMutex);
				m_taskCV.wait_for(lock, std::chrono::seconds(5), [this]() {
					return !m_bRunning.load();
				});
			}
		}

		if (!m_bRunning.load())
		{
			PRINT(ERROR_RED, "DB查询线程[%d] 线程池已停止, 退出\n", nThreadIndex);
			xPacketPool::DrainThreadLocal(); // 修复: 释放线程TLS封包池, 避免泄漏
			return;
		}
	}

	PRINT(SUCCESS_GREEN, "DB查询线程[%d] 启动成功\n", nThreadIndex);

	// 心跳保活计时器
	auto lastKeepAlive = std::chrono::steady_clock::now();
	constexpr int keepAliveIntervalSec = 120; // 每120秒保活一次

	// 工作循环
	while (m_bRunning.load())
	{
		DBAsyncWork work;

		// 等待任务（带超时，避免无限阻塞导致无法保活）
		{
			std::unique_lock<std::mutex> lock(m_taskMutex);
			m_taskCV.wait_for(lock, std::chrono::seconds(keepAliveIntervalSec), [this]() {
				return !m_taskQueue.empty() || !m_bRunning.load();
			});

			if (!m_bRunning.load() && m_taskQueue.empty())
				break;

			if (m_taskQueue.empty())
			{
				// 超时无任务，执行心跳保活
				auto now = std::chrono::steady_clock::now();
				auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - lastKeepAlive).count();
				if (elapsedSec >= keepAliveIntervalSec)
				{
					lastKeepAlive = now;
					// 保活：执行轻量查询验证连接
					CVirtualDBConnection* pConn = appDB.GetConnection();
					if (pConn && !pConn->IsConnectionValid())
					{
						PRINT(ERROR_RED, "DB查询线程[%d] 心跳检测连接断开，尝试重连...\n", nThreadIndex);
						if (pConn->Reconnect() == SE_OK)
							PRINT(SUCCESS_GREEN, "DB查询线程[%d] 重连成功\n", nThreadIndex);
						else
							PRINT(ERROR_RED, "DB查询线程[%d] 重连失败!\n", nThreadIndex);
					}
				}
				continue;
			}

			work = std::move(m_taskQueue.front());
			m_taskQueue.pop();
		}

		// 在查询线程中执行数据库操作
		// 执行前验证连接有效性，断开则尝试重连
		BOOL bQueryExecuted = FALSE;
		if (work.queryFunc)
		{
			CVirtualDBConnection* pConn = appDB.GetConnection();
			if (pConn && !pConn->IsConnectionValid())
			{
				PRINT(ERROR_RED, "DB查询线程[%d] 执行前检测连接断开，尝试重连...\n", nThreadIndex);
				if (pConn->Reconnect() != SE_OK)
				{
					PRINT(ERROR_RED, "DB查询线程[%d] 重连失败，跳过本次任务\n", nThreadIndex);
					// 连接不可用，跳过 queryFunc 和 resultFunc，避免resultFunc访问未初始化数据
				}
				else
				{
					PRINT(SUCCESS_GREEN, "DB查询线程[%d] 重连成功，继续执行\n", nThreadIndex);
					work.queryFunc(appDB);
					bQueryExecuted = TRUE;
				}
			}
			else if (pConn)
			{
				work.queryFunc(appDB);
				bQueryExecuted = TRUE;
			}
			else
			{
				PRINT(ERROR_RED, "DB查询线程[%d] 无可用连接，跳过本次任务\n", nThreadIndex);
			}
		}

		m_nPendingTasks.fetch_sub(1, std::memory_order_relaxed);

		// 只有queryFunc成功执行后才投递resultFunc到主线程
		if (bQueryExecuted && work.resultFunc)
		{
			{
				std::lock_guard<std::mutex> lock(m_resultMutex);
				m_resultQueue.push(std::move(work.resultFunc));
			}
			m_nCompletedResults.fetch_add(1, std::memory_order_relaxed);
		}
	}

	// 修复: 线程退出前释放TLS封包池, 避免泄漏
	xPacketPool::DrainThreadLocal();
}
