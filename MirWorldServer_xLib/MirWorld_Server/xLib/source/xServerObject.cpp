#include "..\header\xiocpserver.h"
#include "..\header\xserverobject.h"

xServerObject::xServerObject(VOID)
{
	m_pxIocpServer = nullptr;
}

xServerObject::~xServerObject(VOID)
{
	m_pxIocpServer = nullptr;
}
