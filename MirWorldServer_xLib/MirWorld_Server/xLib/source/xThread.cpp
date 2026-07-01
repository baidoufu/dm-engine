#include "../header/xthread.h"
#include "../header/xPacket.h"
#include <array>

xThread::xThread(VOID) : m_dwThreadID(0), m_hThread(INVALID_HANDLE_VALUE),
m_bTerminated(FALSE), m_bIsSuspended(FALSE), m_lpParam(nullptr)
{
}

xThread::~xThread(VOID)
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		if (m_bIsSuspended)
			Resume();
		Terminate();
		WaitFor();
	}
	else if (m_bTerminated.load())
	{
		// 线程已自行退出但 WaitFor 尚未被调用，句柄仍需关闭
		// 此分支正常情况不会进入，因为 WaitFor 会将 m_hThread 置为 INVALID_HANDLE_VALUE
	}
}

DWORD xThread::WaitFor(DWORD dwMilliseconds)
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		HANDLE hThread = m_hThread;
		std::array<HANDLE, 1> handles = { hThread };
		uint64_t ullStartTime = GetSteadyTimeMS64();
		for (;;)
		{
			uint64_t ullElapsed = GetSteadyTimeMS64() - ullStartTime;
			DWORD dwRemaining = (dwMilliseconds == INFINITE) ? INFINITE :
				(ullElapsed >= dwMilliseconds) ? 0 : static_cast<DWORD>(dwMilliseconds - ullElapsed);
			DWORD result = MsgWaitForMultipleObjects(1, handles.data(), FALSE, dwRemaining, QS_ALLINPUT);
			if (result == WAIT_OBJECT_0)
			{
				CloseHandle(hThread);
				m_hThread = INVALID_HANDLE_VALUE;
				return WAIT_OBJECT_0; // 线程已退出
			}
			else if (result == WAIT_OBJECT_0 + 1)
			{
				MSG msg; // 有消息, 处理所有消息后继续等待
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						PostQuitMessage(static_cast<int>(msg.wParam)); // 将 WM_QUIT 重新投递, 保持外部应用行为不变
					}
					else
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				// 检查是否超时
				if (dwMilliseconds != INFINITE && (GetSteadyTimeMS64() - ullStartTime) >= dwMilliseconds)
				{
					return WAIT_TIMEOUT;
				}
				// 继续循环等待线程信号或更多消息
			}
			else if (result == WAIT_TIMEOUT)
			{
				return WAIT_TIMEOUT; // 超时
			}
			else
			{
				CloseHandle(hThread);
				m_hThread = INVALID_HANDLE_VALUE;
				return WAIT_FAILED; // WAIT_FAILED 或错误
			}
		}
	}
	return WAIT_OBJECT_0; // 线程句柄无效，视为已退出
}

BOOL xThread::Start(LPVOID lpParam, BOOL bSuspended)
{
	UINT dwThreadID;
	int nFlag = 0;
	m_lpParam = lpParam;
	if (bSuspended)
	{
		nFlag = CREATE_SUSPENDED;
		m_bIsSuspended = TRUE;
	}
	else
		m_bIsSuspended = FALSE;

	HANDLE hThread = CreateThread(nullptr,
		0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadProc),
		static_cast<LPVOID>(this),
		nFlag,
		reinterpret_cast<LPDWORD>(&dwThreadID));
	if (hThread == nullptr) return FALSE;
	if (hThread == INVALID_HANDLE_VALUE)
		return FALSE;
	// 这样IOCP工作线程能优先处理网络IO事件
	SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	m_dwThreadID = dwThreadID;
	m_hThread = hThread;
	return TRUE;
}

BOOL xThread::Resume(VOID)
{
	if (ResumeThread(m_hThread) != static_cast<DWORD>(-1))
	{
		m_bIsSuspended = FALSE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL xThread::Suspend(VOID)
{
	if (SuspendThread(m_hThread) != static_cast<DWORD>(-1))
	{
		m_bIsSuspended = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL xThread::IsTerminated(VOID)
{
	return m_bTerminated;
}

BOOL xThread::IsStarted(VOID) const
{
	return m_hThread != INVALID_HANDLE_VALUE;
}

VOID xThread::Terminate(VOID)
{
	m_bTerminated.store(TRUE);
}

unsigned WINAPI xThread::ThreadProc(LPVOID pParam)
{
	xThread* pThread = static_cast<xThread*>(pParam);
	pThread->m_bTerminated.store(FALSE);
	srand(static_cast<UINT>(reinterpret_cast<uintptr_t>(pThread->getHandle()) + GetSteadyTimeMS()));
	BOOL bException = FALSE;
	try
	{
		pThread->Execute(pThread->m_lpParam);
	}
	catch (const std::exception& e)
	{
		bException = TRUE;
		pThread->OnExecuteException(e);
	}
	catch (...)
	{
		bException = TRUE;
	}
	pThread->OnTerminated(bException);
	pThread->m_dwThreadID = 0;
	pThread->m_bTerminated = TRUE;
	xPacketPool::DrainThreadLocal();
	return 0;
}
