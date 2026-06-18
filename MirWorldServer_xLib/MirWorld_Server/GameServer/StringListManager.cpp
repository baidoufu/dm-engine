#include "StdAfx.h"
#include ".\stringlistmanager.h"

CStringListManager::CStringListManager(VOID) : m_StringListList(TRUE)
{
}

CStringListManager::~CStringListManager(VOID)
{
}

VOID CStringListManager::ClearStringList(const char* pszStringList)
{
	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(pszStringList);
	if (pList == nullptr)return;
	pList->Clear();
}

VOID CStringListManager::ClearAll()
{
	for (UINT i = 0; i < m_StringListList.GetCount(); i++)
	{
		xStringFileList* pList = (xStringFileList*)m_StringListList[i]->lpObject;
		if (pList != nullptr) pList->Clear();
	}
	m_StringListList.Clear();
}

BOOL CStringListManager::InStringList(const char* pszStringList, const char* pszString)
{
	std::array<char, 64> szStringList{};
	std::array<char, 64> szString{};
	o_strncpy(szStringList.data(), pszStringList, 63);
	q_strupper(szStringList.data());

	o_strncpy(szString.data(), pszString, 63);
	q_strupper(szString.data());

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(szStringList.data());
	if (pList == nullptr)return FALSE;
	return (pList->IndexOf(szString.data()) != -1);
}

VOID CStringListManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	LoadStringList(pszFilename, TRUE);
}

VOID CStringListManager::LoadStringList(const char* pszFilename, BOOL bFindFile)
{
	if (bFindFile == FALSE && !FileExist(pszFilename))return;
	std::array<char, 64> szStringListName{};
	BOOL bAdd = FALSE;
	_splitpath(pszFilename, nullptr, nullptr, szStringListName.data(), nullptr);

	q_strupper(szStringListName.data());

	xStringFileList* p = (xStringFileList*)this->m_StringListList.ObjectOf(szStringListName.data());
	if (p == nullptr)
	{
		p = m_StringListPool.newObject();
		p->SetIgnCase(TRUE);
		bAdd = TRUE;
	}
	p->LoadFromFile(pszFilename);
	if (bAdd)
	{
		m_StringListList.Add(szStringListName.data(), (LPVOID)p);
	}
}

BOOL CStringListManager::AddToStringList(const char* pszStringList, const char* pszString)
{
	std::array<char, 128> szStringList{};
	std::array<char, 128> szString{};
	BOOL bAdd = FALSE;
	o_strncpy(szStringList.data(), pszStringList, 120);
	q_strupper(szStringList.data());

	o_strncpy(szString.data(), pszString, 120);
	q_strupper(szString.data());

	xStringsExtracter<2> path(szStringList.data(), "\\");

	char* pszStringListName = path[path.getCount() - 1];

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(pszStringListName);
	if (pList == nullptr)
	{
		pList = m_StringListPool.newObject();
		if (pList == nullptr)return FALSE;

		//	´´½¨Ä¿Â¼
		if (path.getCount() > 1)
		{
			std::array<char, 256> szPath{};
			_snprintf(szPath.data(), 256, ".\\Data\\StringList\\%s", path[0]);
			if (!PathIsFolder(szPath.data()))
			{
				if (!CreateDirectory(szPath.data(), nullptr))return FALSE;
			}

			std::array<char, 256> szFileName{};
			_makepath(szFileName.data(), nullptr, szPath.data(), pszStringListName, ".txt");
			pList->SetFileName(szFileName.data());
		}

		pList->SetIgnCase(TRUE);
		bAdd = TRUE;
	}
	if (pList->IndexOf(szString.data()) == -1)
	{
		pList->Add(szString.data());
		SaveStringList(szStringList.data(), pList);
	}
	if (bAdd)
	{
		m_StringListList.Add(pszStringListName, (LPVOID)pList);
	}
	return TRUE;
}

BOOL CStringListManager::DelFromStringList(const char* pszStringList, const char* pszString)
{
	std::array<char, 64> szStringList{};
	std::array<char, 64> szString{};

	o_strncpy(szStringList.data(), pszStringList, 63);
	q_strupper(szStringList.data());

	o_strncpy(szString.data(), pszString, 63);
	q_strupper(szString.data());

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(szStringList.data());
	if (pList == nullptr)return FALSE;
	if (pList->Delete(szString.data()))
	{
		SaveStringList(szStringList.data(), pList);
		return TRUE;
	}
	return FALSE;
}

VOID CStringListManager::Load(const char* pszPath)
{
	CFindFile::StartFind(pszPath, "*.txt", TRUE);
}

VOID CStringListManager::SaveStringList(const char* pszName, xStringFileList* pList)
{
	if (pList->GetFileName() == nullptr)
	{
		std::array<char, 1024> szFile{};
		sprintf_s(szFile.data(), szFile.size(), ".\\Data\\StringList\\%s.txt", pszName);
		pList->SaveToFile(szFile.data());
	}
	else
		pList->SaveToFile(pList->GetFileName());
}