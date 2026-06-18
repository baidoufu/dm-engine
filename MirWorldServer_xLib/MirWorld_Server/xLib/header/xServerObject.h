#pragma once

class xIocpServer;
class xServerObject
{
public:
	xServerObject(VOID);
	virtual ~xServerObject(VOID);
	xIocpServer* getServer() { return m_pxIocpServer; }
	VOID	setServer(xIocpServer* pServer) { m_pxIocpServer = pServer; }
private:
	xIocpServer* m_pxIocpServer;
};