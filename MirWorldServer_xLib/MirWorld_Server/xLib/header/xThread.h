#pragma once
#include "xinc.h"
#include <stdexcept>
#include <atomic>

class xThread
{
public:
	xThread(VOID);
	virtual ~xThread(VOID);
	// 启动线程
	BOOL Start(LPVOID lpParam = nullptr, BOOL bSuspended = FALSE);
	// 继续线程
	BOOL Resume(VOID);
	// 挂起线程
	BOOL Suspend(VOID);
	// 终止线程
	VOID Terminate(VOID);
	// 判断线程是否已终止
	BOOL IsTerminated(VOID);
	// 线程是否已启动
	BOOL IsStarted(VOID) const;
	// 等待线程退出并关闭句柄，dwMilliseconds 为超时时间（默认 INFINITE）
	DWORD WaitFor(DWORD dwMilliseconds = INFINITE);
	// 获取线程句柄
	HANDLE getHandle()const { return m_hThread; }
	// 获取线程号
	DWORD getThreadID()const { return m_dwThreadID; }
protected:
	///线程的主运行体
	virtual VOID Execute(LPVOID lpParam) = 0;
	// 线程终止事件
	virtual VOID OnTerminated(BOOL bExcepted = FALSE) {};
	// Execute异常事件
	virtual VOID OnExecuteException(const std::exception& e) {}
	///标志是否已终止
	std::atomic<BOOL> m_bTerminated;
private:
	LPVOID m_lpParam;
	///线程号
	DWORD m_dwThreadID;
	///线程句柄
	HANDLE m_hThread;
	///标志是否已挂起
	BOOL m_bIsSuspended;
	///线程例程
	static unsigned WINAPI ThreadProc(LPVOID pParam);
};
