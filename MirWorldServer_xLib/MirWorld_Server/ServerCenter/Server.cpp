#include "StdAfx.h"
#include ".\server.h"
#include "servercenter.h"

CServer::CServer(VOID)
{
}

CServer::~CServer(VOID)
{
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);
	return TRUE;
}

VOID CServer::Update()
{
	CClientObj* pObj = m_ObjectPool.First();
	while (pObj)
	{
		pObj->Update();
		pObj = m_ObjectPool.Next();
	}
	// 帧末：刷新所有连接的批量发送缓冲区
	CClientObj* pFlushObj = m_ObjectPool.First();
	while (pFlushObj)
	{
		if (pFlushObj->IsConnected() && pFlushObj->IsBatchMode())
			pFlushObj->FlushMsgQueue();
		pFlushObj = m_ObjectPool.Next();
	}
}

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	UnRegisterServer((CClientObj*)pObject);
	deleteObject((CClientObj*)pObject);
}

VOID CServer::CleanServer()
{
	//	all connection~~
}

static BOOL CheckType(int type)
{
	if (type <= ST_UNKNOWN || type >= ST_MAX)
		return FALSE;
	return TRUE;
}

SERVER_ERROR CServer::RegisterServer(CClientObj* pObj, REGISTER_SERVER_INFO* pInfo, REGISTER_SERVER_RESULT* pResult)
{
	int type = pInfo->Id.bType;
	if (!CheckType(type))
		return SE_INVALIDPARAM;
	PRINT(SUCCESS_GREEN, "%s|%s:%u 注册到服务器中心.\n",
		GetServerTypeNameByType((servertype)pInfo->Id.bType),
		pInfo->addr.addr.data(), pInfo->addr.nPort);
	UINT id = pObj->getId();
	if (!m_ServerArrays[type - 1].AddId(id))
		return SE_SERVERFULL;
	REGISTEREDSERVER* pRegInfo = pObj->GetRegInfo();

	pRegInfo->Ident = pInfo->Id;
	pRegInfo->Addr = pInfo->addr;
	memcpy(pRegInfo->szName.data(), pInfo->szName.data(), 64);
	pRegInfo->dwConnections = 0;
	pRegInfo->Ident.bIndex = static_cast<BYTE>(id);

	memset(pResult, 0, sizeof(*pResult));

	pResult->Id = pRegInfo->Ident;

	if (type != ST_DATABASESERVER)
	{
		if (type == ST_GAMESERVER)
			pResult->nDbCount = PrepareServer(ST_DATABASESERVER, 2, pResult->DbAddr.data());
		else
			pResult->nDbCount = PrepareServer(ST_DATABASESERVER, 1, pResult->DbAddr.data());
		pRegInfo->iSendDBCount = pResult->nDbCount;
	}
	else
	{
		SERVERADDR	addr[2];
		int icount = PrepareServer(ST_DATABASESERVER, 2, addr);
		if (icount == 2)
		{
			SendDBServer(addr, icount);
		}
	}
	return SE_OK;
}

SERVER_ERROR CServer::UnRegisterServer(CClientObj* pObj)
{
	REGISTEREDSERVER* pInfo = pObj->GetRegInfo();
	if (pInfo->Ident.dwId == 0)
		return SE_NOTINITLIZED;
	int type = pInfo->Ident.bType;
	if (type <= ST_UNKNOWN || type >= ST_MAX)
		return SE_INVALIDPARAM;
	PRINT(ERROR_RED, "%s|%s:%u 从服务器中心注销.\n",
		GetServerTypeNameByType((servertype)pInfo->Ident.bType),
		pInfo->Addr.addr.data(), pInfo->Addr.nPort);
	m_ServerArrays[type - 1].DelId(pObj->getId());
	return SE_OK;
}

SERVER_ERROR CServer::FindServer(servertype type, const char* pszName, FINDSERVER_RESULT* pResult)
{
	if (type <= ST_UNKNOWN || type >= ST_MAX)return SE_INVALIDPARAM;
	int i = 0;
	SERVERARRAY* pArray = &m_ServerArrays[type - 1];
	REGISTEREDSERVER* pInfo = nullptr;
	for (i = 0; i < pArray->count; i++)
	{
		CClientObj* pObj = (CClientObj*)getObject(pArray->Ids[i]);
		if (pObj != nullptr)
		{
			pInfo = pObj->GetRegInfo();
			if (pInfo && strcmp(pszName, pInfo->szName.data()) == 0)
			{
				pResult->addr = pInfo->Addr;
				pResult->Id = pInfo->Ident;
				return SE_OK;
			}
		}
		else
		{
			//	this is what?
		}
	}
	return SE_SERVERNOTFOUND;
}

int CServer::PrepareServer(int type, int count, SERVERADDR* pAddrArray)
{
	SERVERARRAY* pArray = &m_ServerArrays[type - 1];
	if (pArray->count == 0)
		return 0;
	int i = 0;
	for (; i < count; i++)
	{
		if (pArray->pickptr < 0 || pArray->pickptr >= pArray->count)pArray->pickptr = 0;
		UINT id = pArray->Ids[pArray->pickptr];
		CClientObj* pObj = (CClientObj*)GetClientObject(id);
		if (pObj)
		{
			pAddrArray[i] = pObj->GetRegInfo()->Addr;
		}
		else
		{
			// 跳过无效的服务器连接，不留下未初始化的地址
			i--; // 回退计数器，不占用结果槽位
		}
		pArray->pickptr++;
	}
	return i;
}

VOID CServer::SendDBServer(SERVERADDR* pAddr, int count)
{
	REGISTER_SERVER_RESULT result;
	if (count <= 0)return;
	result.DbAddr[0] = *pAddr;
	result.DbAddr[1] = *(pAddr + 1);
	CClientObj* pObj = m_ObjectPool.First();
	while (pObj)
	{
		REGISTEREDSERVER* pRegInfo = pObj->GetRegInfo();
		if (pRegInfo != nullptr && 
			pRegInfo->Ident.bType != ST_UNKNOWN && pRegInfo->Ident.bType != ST_DATABASESERVER &&
			pRegInfo->iSendDBCount == 0)
		{
			result.Id = pRegInfo->Ident;

			if (pRegInfo->Ident.bType == ST_GAMESERVER)
				result.nDbCount = count;
			else
				result.nDbCount = 1;
			pRegInfo->iSendDBCount = result.nDbCount;
			pObj->SendMsg(0, SCM_REGISTERSERVER, SE_OK, 0, 0, &result, sizeof(result));
		}
		pObj = m_ObjectPool.Next();
	}
}

