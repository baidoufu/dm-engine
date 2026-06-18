#pragma once
#include "virtualdatabase.h"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <memory>

// 前向声明
class CClientObj;
class CAppDB;

// ============================================================================
// DB 异步工作单元
//
// 每个工作单元包含两个回调：
//   queryFunc  — 在查询线程中执行，接收线程私有的 CAppDB&
//   resultFunc — 在主线程中执行（用于 SendMsg 等操作）
//
// 使用模式：
//   1. 主线程 OnCodedMsg 中：
//      a. 从 pMsg 中拷贝所有请求数据（值捕获到 lambda）
//      b. 创建共享的结果持有者（shared_ptr）
//      c. queryFunc 捕获请求数据 + 结果持有者
//      d. resultFunc 捕获结果持有者 + 响应参数
//   2. 查询线程执行 queryFunc，结果写入持有者
//   3. 主线程 ProcessCompletedResults 执行 resultFunc，通过持有者读取结果
// ============================================================================
struct DBAsyncWork
{
	// 在查询线程中执行（持有独立的 CAppDB + MySQL 连接）
	std::function<void(CAppDB&)>	queryFunc;

	// 在主线程中执行（用于发送响应消息）
	// 如果为空，表示 fire-and-forget 操作（无需发送响应）
	std::function<void()>	resultFunc;
};

// ============================================================================
// CDBQueryThreadPool — DBServer 异步查询线程池
//
// 架构：
//   主线程:    收包 → 拷贝请求数据 → 构造 DBAsyncWork → 投递到任务队列 → 继续收下一个包
//   查询线程:  取任务 → 执行 queryFunc(线程私有CAppDB) → 将 resultFunc 投递到结果队列
//   主线程:    每帧消费结果队列 → 执行 resultFunc（SendMsg 等）
//
// 关键设计：
//   - 每个查询线程持有独立的 CAppDB（含 MySQL 连接），避免共享连接的锁竞争
//   - 需要保序的操作（如 CreateItem → FindItemId）在单个 queryFunc 内完成
//   - pMsg 的数据必须在投递前拷贝到 lambda 中，因为 pMsg 在异步执行时已失效
//   - SendMsg 只能在主线程的 resultFunc 中调用
// ============================================================================
class CDBQueryThreadPool
{
public:
	CDBQueryThreadPool();
	~CDBQueryThreadPool();

	// 启动线程池
	// nThreadCount: 查询线程数量，建议 = CPU核心数
	// pszServer/pszPort/pszDBName/pszId/pszPassword: MySQL 连接参数
	BOOL Start(int nThreadCount,
		const char* pszServer, const char* pszPort,
		const char* pszDBName, const char* pszId, const char* pszPassword);

	// 停止线程池
	VOID Stop();

	// 主线程调用：投递异步工作（非阻塞）
	BOOL SubmitWork(DBAsyncWork&& work);

	// 主线程调用：每帧消费完成结果，在主线程上下文中执行回调
	VOID ProcessCompletedResults();

	// 获取待处理任务数（用于监控）
	int GetPendingTaskCount() const;

	// 获取已完成结果数（用于监控）
	int GetCompletedResultCount() const;

	// 线程池是否正在运行
	BOOL IsRunning() const { return m_bRunning.load(); }

private:
	// 查询线程入口
	VOID WorkerThreadProc(int nThreadIndex);

private:
	// 线程池
	std::vector<std::thread>	m_threads;

	// 任务队列（主线程写，查询线程读）
	std::mutex					m_taskMutex;
	std::condition_variable		m_taskCV;
	std::queue<DBAsyncWork>		m_taskQueue;
	std::atomic<bool>			m_bRunning{false};

	// 结果队列（查询线程写，主线程读）
	std::mutex					m_resultMutex;
	std::queue<std::function<void()>>	m_resultQueue;

	// MySQL 连接参数（拷贝，用于工作线程创建 CAppDB）
	std::array<char, 256>		m_szServer{};
	std::array<char, 32>		m_szPort{};
	std::array<char, 256>		m_szDBName{};
	std::array<char, 256>		m_szId{};
	std::array<char, 256>		m_szPassword{};

	// 统计
	std::atomic<int>			m_nPendingTasks{0};
	std::atomic<int>			m_nCompletedResults{0};
};
