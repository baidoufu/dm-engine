#include "..\header\xlistenobject.h"
#include "..\header\xiocpunit.h"
#include "..\header\xclientobject.h"
#include "..\header\xInc.h"
#include "..\header\xiocpserver.h"

xListenObject::xListenObject(VOID)
{
	m_Id = 0;
}

xListenObject::~xListenObject(VOID)
{
}

VOID xListenObject::OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam)
{
	sockaddr* p1 = nullptr, * p2 = nullptr;
	int	len1, len2;
	if (iEvent == xIocpUnit::IUE_ACCEPT)
	{
		auto* pIocpUnit = static_cast<xIocpUnit*>(lpParam);
		auto* pAcceptSocket = static_cast<xAcceptSocket*>(pIocpUnit->getData());
		// 更新 Accept Socket 的上下文，使其继承监听 Socket 的属性
		SOCKET listenFd = getSocketFd();
		setsockopt(pAcceptSocket->getSocketFd(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, 
			reinterpret_cast<const char*>(&listenFd), sizeof(listenFd));
		GetAcceptExSockaddrs(static_cast<LPVOID>(pAcceptSocket->getBuffer()), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &p1, &len1, &p2, &len2);
		if (p2 != nullptr)
		{
			char addrBuf[INET_ADDRSTRLEN];
			const char* p = inet_ntop(AF_INET, &reinterpret_cast<SOCKADDR_IN*>(p2)->sin_addr, addrBuf, sizeof(addrBuf));
			if (p != nullptr)
				pAcceptSocket->setEndPoint(p, ntohs(reinterpret_cast<SOCKADDR_IN*>(p2)->sin_port));
		}
		pAcceptSocket->setState(SS_ACCEPTED);
		if (!getServer()->onConnection(pAcceptSocket, getId()))
		{
			// 连接被拒绝（速率限制或队列满），关闭已接受的socket防止泄漏
			pAcceptSocket->close();
		}
		pAcceptSocket->reset();
		postAccept(static_cast<xIocpUnit*>(pSender), pAcceptSocket);
	}
	else if (iEvent == xIocpUnit::IUE_ERROR)
	{
		// Accept I/O失败，释放资源
		auto* pIocpUnit = static_cast<xIocpUnit*>(lpParam);
		auto* pAcceptSocket = static_cast<xAcceptSocket*>(pIocpUnit->getData());
		getServer()->releaseIocpUnit(pIocpUnit);
		if (pAcceptSocket) releaseAcceptSocket(pAcceptSocket);
	}
}

VOID xListenObject::postAccept(int nAccept)
{
	xIocpUnit* pIocpUnit = nullptr;
	xAcceptSocket* pAcceptSocket = nullptr;
	
	// 批量预分配对象，减少分配开销
	int actualAccept = MAX(nAccept, 16);  // 确保至少16个Accept
	for (int i = 0; i < actualAccept; ++i)
	{
		if (pIocpUnit == nullptr)pIocpUnit = getServer()->newIocpUnit();
		if (pIocpUnit == nullptr)continue;
		if (pAcceptSocket == nullptr)pAcceptSocket = newAcceptSocket();
		if (pAcceptSocket == nullptr)
		{
			getServer()->releaseIocpUnit(pIocpUnit);
			pIocpUnit = nullptr;  // 确保下次重新分配
			continue;
		}
		
		pIocpUnit->setEventListener(this);
		pIocpUnit->setType(IO_ACCEPT);
		pIocpUnit->setData(pAcceptSocket);
		if (!postAccept(pIocpUnit, pAcceptSocket))
		{
			getServer()->releaseIocpUnit(pIocpUnit);
			releaseAcceptSocket(pAcceptSocket);
		}
		// 无论成功失败，重置指针以确保下次循环重新分配
		pIocpUnit = nullptr;
		pAcceptSocket = nullptr;
	}
	// 清理未使用的对象
	if (pIocpUnit)getServer()->releaseIocpUnit(pIocpUnit);
	if (pAcceptSocket)releaseAcceptSocket(pAcceptSocket);
}

BOOL xListenObject::postAccept(xIocpUnit* pIocpUnit, xAcceptSocket* pAcceptSocket)
{
	return acceptEx(*pAcceptSocket, static_cast<LPVOID>(pAcceptSocket->getBuffer()), 0, pIocpUnit->getOverlappedEx()->dwNumberOfBytes, pIocpUnit->getOverlapped());
}
