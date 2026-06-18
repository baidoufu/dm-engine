#include "..\header\xinc.h"
#include "..\header\xiocpmanager.h"
#include "..\header\xiocpworkthread.h"

xIocpManager::xIocpManager(VOID)
{
	m_hIocp = nullptr;
	m_iIocpWorkThreadCount = 0;
}

xIocpManager::~xIocpManager(VOID)
{
	if (m_hIocp != nullptr)
		CloseHandle(m_hIocp);
}

BOOL xIocpManager::Start()
{
	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	// 获取物理核心数而非逻辑处理器数
	DWORD dwPhysicalCores = si.dwNumberOfProcessors;
	// Windows 7+ 支持获取真实物理核心数
	DWORD dwBufferSize = 0;
	GetLogicalProcessorInformation(nullptr, &dwBufferSize);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		auto pBuffer = std::make_unique<SYSTEM_LOGICAL_PROCESSOR_INFORMATION[]>(
			dwBufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
		if (GetLogicalProcessorInformation(pBuffer.get(), &dwBufferSize))
		{
			dwPhysicalCores = 0;
			DWORD count = dwBufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			for (DWORD i = 0; i < count; ++i)
			{
				if (pBuffer[i].Relationship == RelationProcessorCore)
					dwPhysicalCores++;
			}
		}
	}
	// IOCP工作线程数 = 物理核心数（上限16，避免过度竞争）
	m_iIocpWorkThreadCount = MIN(dwPhysicalCores, 16);
	if (m_iIocpWorkThreadCount < 2)
		m_iIocpWorkThreadCount = 2;  // 至少2个线程

	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, IOCP_KEY_CLIENT_SOCKET, m_iIocpWorkThreadCount);
	if (m_hIocp == nullptr)
	{
		setError(-1, "创建完成端口失败, Errorcode = %d\n", GetLastError());
		return FALSE;
	}

	m_pIocpWorkThread = std::make_unique<xIocpWorkThread[]>(m_iIocpWorkThreadCount);
	for (DWORD i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		m_pIocpWorkThread[i].setIocpHandle(m_hIocp);
		m_pIocpWorkThread[i].Start();
	}
	return TRUE;
}

VOID xIocpManager::Stop()
{
	if (m_hIocp == nullptr) return;
	if (!m_pIocpWorkThread)
	{
		CloseHandle(m_hIocp);
		m_hIocp = nullptr;
		m_iIocpWorkThreadCount = 0;
		return;
	}
	// 发送退出信号，检查返回值
	for (DWORD i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		if (!PostQueuedCompletionStatus(m_hIocp, 0, IOCP_KEY_SHUTDOWN, nullptr))
		{
			setError(GetLastError(), "PostQueuedCompletionStatus 退出信号发送失败, 线程索引 = %u, Error = %d", i, GetLastError());
		}
	}
	// 发送终止标志并等待线程退出（带超时保护）
	constexpr DWORD WAIT_TIMEOUT_MS = 5000; // 每个线程最长等待5秒
	for (DWORD i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		m_pIocpWorkThread[i].Terminate();
		DWORD waitResult = m_pIocpWorkThread[i].WaitFor(WAIT_TIMEOUT_MS);
		if (waitResult == WAIT_TIMEOUT)
		{
			// 线程未在超时内退出，尝试补发退出信号
			PostQueuedCompletionStatus(m_hIocp, 0, IOCP_KEY_SHUTDOWN, nullptr);
			waitResult = m_pIocpWorkThread[i].WaitFor(3000);
			if (waitResult == WAIT_TIMEOUT)
			{
				setError(-1, "IOCP工作线程 %u 在超时后仍未退出（可能死锁）", i);
			}
		}
	}

	m_pIocpWorkThread.reset();
	m_iIocpWorkThreadCount = 0;

	CloseHandle(m_hIocp);
	m_hIocp = nullptr;
}

BOOL xIocpManager::Bind(SOCKET socket, DWORD dwBindId)
{
	if (m_hIocp == nullptr)
	{
		setError(-2, "绑定socket失败, m_hIocp没有初始化!");
		return FALSE;
	}

	HANDLE hPort = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_hIocp, dwBindId, 0);
	if (hPort == nullptr)
	{
		setError(GetLastError(), "绑定socket失败,hPort端口没有初始化!");
		return FALSE;
	}
	return TRUE;
}
