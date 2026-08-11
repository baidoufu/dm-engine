#include "stdafx.h"
#include "resource_server.h"

CResourceServer::CResourceServer(VOID)
{
}

CResourceServer::~CResourceServer(VOID)
{
}

BOOL CResourceServer::InitServer(CSettingFile& s)
{
	m_wpfDataPath = s.GetString(m_strServerName.c_str(), "WpfDataPath", "");
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", 1024);
	create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	if (m_wpfDataPath.empty())
	{
		PRINT(ERROR_RED, "未配置WpfDataPath,请在Config.ini中设置!\n");
		return FALSE;
	}

	if (!DirectoryExists(m_wpfDataPath))
	{
		PRINT(ERROR_RED, "WpfDataPath目录不存在: %s\n", m_wpfDataPath.c_str());
		return FALSE;
	}

	FileManager* pFM = FileManager::GetInstance();
	int wpfCount = pFM->LoadWpfFiles(m_wpfDataPath);
	if (wpfCount <= 0)
	{
		PRINT(ERROR_RED, "%s 中未找到有效wpf文件!\n", m_wpfDataPath.c_str());
		return FALSE;
	}

	PRINT(SUCCESS_GREEN, "已从 %s 加载 %d 个wpf归档文件(数据+路径均来自wpf)\n", m_wpfDataPath.c_str(), wpfCount);
	return TRUE;
}

VOID CResourceServer::Update()
{
	CResourceClient* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}

	// 帧末：刷新所有连接的批量发送缓冲区
	CResourceClient* pFlushObj = m_ObjectPool.First();
	while (pFlushObj)
	{
		if (pFlushObj->IsConnected() && pFlushObj->IsBatchMode())
			pFlushObj->FlushMsgQueue();
		pFlushObj = m_ObjectPool.Next();
	}

	UpdateSCServer();
}
