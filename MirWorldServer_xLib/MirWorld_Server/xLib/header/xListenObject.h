#pragma once
#include "xsupport.h"
#include "xsocket.h"
#include "xserverobject.h"
#include <array>

class xIocpUnit;
class xListenObject;

class xAcceptSocket :
	public xSocket
{
public:
	xAcceptSocket()
	{
		m_pListenObject = nullptr;
		m_szBuffer.fill(0);
	}
	xListenObject* getListenObject() { return m_pListenObject; }
	VOID setListenObject(xListenObject* pObject) { m_pListenObject = pObject; }

	char* getBuffer() { return m_szBuffer.data(); }
	VOID reset() {
		makeSocket();
	}

private:
	std::array<char, sizeof(SOCKADDR_IN) * 2 + 32> m_szBuffer;
	xListenObject* m_pListenObject;
};
class xListenObject :
	public xSocket,
	public xEventListener,
	public xServerObject
{
public:
	xListenObject(VOID);
	virtual ~xListenObject(VOID);
	VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam);
	VOID postAccept(int nAccept);
	UINT getId() { return m_Id; }
	VOID setId(UINT id) { m_Id = id; }
private:
	UINT m_Id;
	BOOL postAccept(xIocpUnit* pIocpUnit, xAcceptSocket* pAcceptSocket);
	xObjectPool<xAcceptSocket>	m_xAccpetSocketPool;
	xAcceptSocket* newAcceptSocket()
	{
		xAcceptSocket* p = m_xAccpetSocketPool.newObject();
		if (p)p->setListenObject(this);
		return p;
	}
	VOID releaseAcceptSocket(xAcceptSocket* pAcceptSocket) { m_xAccpetSocketPool.deleteObject(pAcceptSocket); }
};
