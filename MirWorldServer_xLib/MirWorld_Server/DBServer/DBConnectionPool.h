#pragma once
#include "MySQLDatabase.h"
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include <memory>
#include <atomic>

// ============================================================================
// CMySQLConnectionPool — MySQL 连接池
//
// 核心设计：
//   - 预创建 minPoolSize 个连接，按需扩展到 maxPoolSize
//   - Acquire() 获取空闲连接，Release() 归还连接
//   - 归还时自动清理 DataUnit，保留 MySQL 句柄供复用
//   - 获取连接时自动验证有效性（ping），失效则重连
//   - 定期保活：Maintain() 在主线程每帧调用，对空闲连接执行 ping
//   - 空闲超时回收：超过 maxIdleTime 的空闲连接被关闭
//   - 线程安全：所有操作通过 mutex 保护
// ============================================================================

// 空闲连接保活检查间隔（秒）
constexpr DWORD POOL_KEEPALIVE_INTERVAL_SEC = 60;
// 空闲连接最大存活时间（秒），超过此时间的空闲连接将被回收
constexpr DWORD POOL_MAX_IDLE_TIME_SEC = 1800;

class CMySQLConnectionPool
{
public:
	CMySQLConnectionPool() = default;
	~CMySQLConnectionPool();
	// 启动连接池：预创建连接
	BOOL Start(const char* pszServer, const char* pszPort,
		const char* pszDBName, const char* pszId, const char* pszPassword,
		int nMinPoolSize = 4, int nMaxPoolSize = 32);
	// 停止连接池：关闭所有连接
	VOID Stop();
	// 从池中获取一个可用连接（线程安全）
	// 如果池中没有空闲连接且未达到上限，则创建新连接
	// 如果达到上限，返回 nullptr
	CMySQLDBConnection* Acquire();
	// 将连接归还到池中（线程安全）
	// 会自动清理 DataUnit，验证连接有效性
	VOID Release(CMySQLDBConnection* pConnection);
	// 连接池维护：保活 + 空闲回收（主线程定期调用）
	VOID Maintain();
	// 获取连接池统计信息
	int GetTotalCount() const;
	int GetFreeCount() const;
	BOOL IsRunning() const { return m_bRunning.load(); }
private:
	// 创建一个新的连接（调用者需持锁）
	CMySQLDBConnection* CreateNewConnectionLocked();
	// 验证连接有效性，无效则尝试重连（不持锁，网络IO）
	BOOL ValidateConnection(CMySQLDBConnection* pConnection);
	// 从 m_allConnections 中移除并关闭连接（调用者需持锁）
	VOID RemoveConnectionLocked(CMySQLDBConnection* pConnection);
private:
	// 空闲连接条目：记录连接指针和进入空闲队列的时间
	struct FreeConnEntry
	{
		CMySQLDBConnection* pConn;
		std::chrono::steady_clock::time_point idleSince;
		FreeConnEntry(CMySQLDBConnection* p, std::chrono::steady_clock::time_point t)
			: pConn(p), idleSince(t) {}
	};

	std::vector<std::unique_ptr<CMySQLDBConnection>> m_allConnections;
	std::queue<FreeConnEntry> m_freeConnections;
	mutable std::mutex m_poolMutex;

	std::string	m_szServer;
	std::string	m_szPort;
	std::string	m_szDBName;
	std::string	m_szId;
	std::string	m_szPassword;

	int m_nMinPoolSize = 4;
	int m_nMaxPoolSize = 32;
	std::atomic<BOOL> m_bRunning{FALSE};

	// 保活计时
	std::chrono::steady_clock::time_point m_lastKeepAliveTime;
};
