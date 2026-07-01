#include "StdAfx.h"
#include ".\server.h"
#include "loginserver.h"
#include <cstring>

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int t = s.GetInteger(m_strServerName.c_str(), "DisableRegister");
	if (t == 0)
	{
		PRINT(SUCCESS_GREEN, "允许注册新帐号!\n");
		m_bDisableRegister = FALSE;
	}
	else
	{
		PRINT(ERROR_RED, "禁止注册新帐号!\n");
		m_bDisableRegister = TRUE;
	}

	m_strWAddr = s.GetString(m_strServerName.c_str(), "WAddr", "127.0.0.1");
	m_nWPort = s.GetInteger(m_strServerName.c_str(), "WPort", 7501);
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	auto pBuffer = LoadFile(".\\Data\\Config\\UserTip.txt");
	if (pBuffer)
	{
		m_strServerTips = reinterpret_cast<const char*>(pBuffer.get());
	}
	pBuffer = LoadFile(".\\Data\\Config\\LoginTip.txt");
	if (pBuffer)
	{
		m_strLoginOkTips = reinterpret_cast<const char*>(pBuffer.get());
	}
	pBuffer = LoadFile(".\\Data\\Config\\RegisterTip.txt");
	if (pBuffer)
	{
		m_strRegisterTips = reinterpret_cast<const char*>(pBuffer.get());
	}
	return TRUE;
}

VOID CServer::CleanServer()
{
	m_strServerTips.clear();
	m_strLoginOkTips.clear();
	m_strRegisterTips.clear();
	m_strWAddr.clear();
	m_bDisableRegister = FALSE;
	m_nWPort = 0;
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}
	const DWORD dwUpdateKey = GetUpdateKey();
	if ((dwUpdateKey & 1) == 0)
		UpdateDBServer(); // 奇数帧：处理数据中心消息
	else
		UpdateSCServer(); // 偶数帧：处理服务中心消息
}
