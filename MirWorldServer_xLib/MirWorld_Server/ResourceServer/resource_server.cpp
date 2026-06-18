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
	m_dataRootPath = s.GetString(m_strServerName.c_str(), "DataRootPath", ".\\Res\\");
	m_fileListPath = s.GetString(m_strServerName.c_str(), "FileListPath", ".\\List\\");
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", 1024);
	create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	FileManager* pFM = FileManager::GetInstance();
	if (!pFM->Initialize(m_dataRootPath))
	{
		PRINT(ERROR_RED, "初始化文件管理器失败!\n");
		return FALSE;
	}

	bool anyListLoaded = false;
	std::string listDir = m_fileListPath;
	if (!listDir.empty() && listDir.back() != '/' && listDir.back() != '\\')
	{
		listDir += '\\';
	}
	WIN32_FIND_DATAA listFindData;
	HANDLE hListFind = FindFirstFileA((listDir + "*").c_str(), &listFindData);
	if (hListFind != INVALID_HANDLE_VALUE)
	{
		do {
			std::string fileName = listFindData.cFileName;
			if (fileName == "." || fileName == "..") continue;
			if (listFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			std::string fullPath = listDir + fileName;
			if (pFM->LoadFileList(fullPath))
			{
				PRINT(SUCCESS_GREEN, "[加载索引文件] %s\n", fullPath.c_str());
				anyListLoaded = true;
			}
		} while (FindNextFileA(hListFind, &listFindData));
		FindClose(hListFind);
	}

	if (!anyListLoaded)
	{
		PRINT(WARN_YELLOW, "未加载任何索引缓存文件!\n");
	}
	return TRUE;
}

BOOL CResourceServer::MakeIndex(CSettingFile& s)
{
	m_dataRootPath = s.GetString(m_strServerName.c_str(), "DataRootPath", ".\\Res\\");
	m_fileListPath = s.GetString(m_strServerName.c_str(), "FileListPath", ".\\List\\");
	FileManager* pFM = FileManager::GetInstance();
	if (!pFM->Initialize(m_dataRootPath))
	{
		PRINT(ERROR_RED, "初始化文件管理器失败!\n");
		return FALSE;
	}
	bool anyListLoaded = false;
	std::string listDir = m_fileListPath;
	if (!listDir.empty() && listDir.back() != '/' && listDir.back() != '\\')
	{
		listDir += '\\';
	}
	WIN32_FIND_DATAA listFindData;
	HANDLE hListFind = FindFirstFileA((listDir + "*").c_str(), &listFindData);
	if (hListFind != INVALID_HANDLE_VALUE)
	{
		do {
			std::string fileName = listFindData.cFileName;
			if (fileName == "." || fileName == "..") continue;
			if (listFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			std::string fullPath = listDir + fileName;
			if (pFM->MakeFileIndex(fullPath))
			{
				PRINT(SUCCESS_GREEN, "生成索引缓存文件: %s\n", fullPath.c_str());
				anyListLoaded = true;
			}
		} while (FindNextFileA(hListFind, &listFindData));
		FindClose(hListFind);
	}
	if (!anyListLoaded)
	{
		PRINT(WARN_YELLOW, "未生成任何文件列表!\n");
	}
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
