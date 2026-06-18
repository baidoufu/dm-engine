#pragma once
#include "xThread.h"

// 每次批量出队的最大完成包数量
constexpr DWORD IOCP_BATCH_DEQUEUE_COUNT = 32;

class xIocpWorkThread : public xThread
{
public:
	xIocpWorkThread(VOID);
	xIocpWorkThread(HANDLE hIocp);
	virtual ~xIocpWorkThread(VOID);
	VOID setIocpHandle(HANDLE hIocp) { m_hIocp = hIocp; }
protected:
	VOID Execute(LPVOID lpParam);
private:
	HANDLE m_hIocp;
	OVERLAPPED_ENTRY m_olEntries[IOCP_BATCH_DEQUEUE_COUNT];
};
