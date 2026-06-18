#include "../header/xiocpworkthread.h"
#include "../header/xIocpUnit.h"
#include "../header/xIocpManager.h"

xIocpWorkThread::xIocpWorkThread(VOID) : m_hIocp(nullptr), m_olEntries{}
{
}

xIocpWorkThread::xIocpWorkThread(HANDLE hIocp) : m_hIocp(hIocp), m_olEntries{}
{
}

xIocpWorkThread::~xIocpWorkThread(VOID)
{
}

VOID xIocpWorkThread::Execute(LPVOID lpParam)
{
	while (!xThread::IsTerminated())
	{
		ULONG ulNumEntriesRemoved = 0;
		BOOL bRet = GetQueuedCompletionStatusEx(
			m_hIocp,
			m_olEntries,
			IOCP_BATCH_DEQUEUE_COUNT,
			&ulNumEntriesRemoved,
			INFINITE,
			FALSE
		);
		if (!bRet)
		{
			DWORD dwError = GetLastError();
			if (dwError == WAIT_TIMEOUT) continue;
			// IOCP句柄可能已关闭，退出循环
			break;
		}
		for (ULONG i = 0; i < ulNumEntriesRemoved; ++i)
		{
			DWORD dwNumberOfBytes = m_olEntries[i].dwNumberOfBytesTransferred;
			ULONG_PTR ulCompletionKey = m_olEntries[i].lpCompletionKey;
			IOCP_UNIT* pIocpunit = reinterpret_cast<IOCP_UNIT*>(m_olEntries[i].lpOverlapped);
			if (pIocpunit == nullptr)
			{
				if (ulCompletionKey == IOCP_KEY_SHUTDOWN)
					return;  // 收到退出信号，直接退出线程
				continue;   // 空完成通知，继续
			}
			// 如果令牌无效，说明对应的 xIocpUnit 已被 releaseIocpUnit 归还对象池，
			// 其 pUnit 指针可能已失效，必须跳过此完成通知
			if (pIocpunit->dwValidToken != IOCP_UNIT_VALID_TOKEN)
			{
				// 过期的完成通知（CancelIoEx 未成功取消的残留 I/O）安全忽略，因为关联的资源已在 releaseIocpUnit 中清理
				continue;
			}
			// 检查 I/O 是否出错：通过 OVERLAPPED_EX 中保存的错误码判断
			OVERLAPPED_EX* pOlx = &pIocpunit->ol;
			if (pOlx->dwError != 0)
			{
				if (pOlx->dwError == ERROR_OPERATION_ABORTED)
				{
					// CancelIoEx 取消的 I/O 仍会通过 IOCP 投递完成通知，直接跳过
					continue;
				}
				// I/O操作失败，传递错误码给上层处理
				pIocpunit->pUnit->OnComplete(dwNumberOfBytes, ulCompletionKey, pOlx->dwError);
			}
			else
			{
				// 正常I/O完成，传递实际传输的字节数
				pIocpunit->pUnit->OnComplete(dwNumberOfBytes, ulCompletionKey, 0);
			}
		}
	}
}
