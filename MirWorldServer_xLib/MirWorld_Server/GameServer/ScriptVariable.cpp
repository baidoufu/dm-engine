#include "StdAfx.h"
#include "scriptvariable.h"
#include <array>

CVariableFile::CVariableFile()
{
	m_bModified = FALSE;
	m_pFilename.reset();
	m_szName[0] = 0;
	if (!PathIsFolder(".\\Data\\Variables"))
		CreateDirectory(".\\Data\\Variables", nullptr);
	if (!PathIsFolder(".\\Data\\CharVars"))
		CreateDirectory(".\\Data\\CharVars", nullptr);
}

CVariableFile::~CVariableFile()
{
	m_xVarList.ClearVars();
}

BOOL CVariableFile::Load(const char* pszFilename)
{
	CStringFile sf(pszFilename);
	char* pLine = nullptr;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == 0 || *pLine == '#')continue;
		xStringsExtracter<2> val(pLine, "=", " \t\"");
		if (val.getCount() == 0)continue;
		if (val.getCount() > 1)
			m_xVarList.AddVar(val[0], val[1]);
		else
			m_xVarList.AddVar(val[0], "");
	}
	m_pFilename.reset(copystring(pszFilename));
	_splitpath(m_pFilename.get(), nullptr, nullptr, m_szName.data(), nullptr);
	return TRUE;
}

BOOL CVariableFile::Reload()
{
	if (m_pFilename == nullptr)return FALSE;
	m_xVarList.ClearVars();
	CStringFile sf(m_pFilename.get());
	char* pLine = nullptr;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == 0 || *pLine == '#')continue;
		xStringsExtracter<2> val(pLine, "=", " \t\"");
		if (val.getCount() == 0)continue;
		if (val.getCount() > 1)
			m_xVarList.AddVar(val[0], val[1]);
		else
			m_xVarList.AddVar(val[0], "");
	}
	return TRUE;
}

BOOL CVariableFile::Save()
{
	if (m_pFilename == nullptr)
	{
		if (m_szName[0] == 0)return FALSE;
		char szFilename[1024];
		sprintf(szFilename, ".\\Data\\Variables\\%s.txt", m_szName.data());
		m_pFilename.reset(copystring(szFilename));
	}
	FILE* fp = fopen(m_pFilename.get(), "w");
	if (fp == nullptr)return FALSE;
	xStringList<32>* p = m_xVarList.GetList();
	for (UINT n = 0; n < p->GetCount(); n++)
	{
		xVarList<32>::Variable* var = (xVarList<32>::Variable*)(*p)[n]->lpObject;
		char* pVal = var->pszValue.get();
		if (pVal == nullptr)
			pVal = "";
		fprintf(fp, "%s = \"%s\"\n", (*p)[n]->pszString.get(), pVal);
	}
	fclose(fp);
	return TRUE;
}

const char* CVariableFile::GetVariableValue(const char* pszName)
{
	return m_xVarList.GetVarValue(pszName);
}

VOID CVariableFile::SetVariableValue(const char* pszName, const char* pszValue)
{
	m_xVarList.AddVar(pszName, (char*)pszValue);
}

VOID CVariableFile::DelVariable(const char* pszName)
{
	m_xVarList.DelVar(pszName);
}

VOID CVariableFile::Clear()
{
	m_xVarList.ClearVars();
}

CVariableRegisterAgent::CVariableRegisterAgent(const char* pszName, fnGetVariable fnGet)
{
	if (!CScriptVariableManager::GetInstance()->AddVariable(pszName, fnGet))
		PRINT(ERROR_RED, "×¢²á±äÁ¿º¯Êý %s Ê§°Ü!\n", pszName);
}

CScriptVariableManager::CScriptVariableManager() : m_xVarList(TRUE)
{
	m_nCachePtr = 0;
	memset(m_xStringCache.data(), 0, sizeof(m_xStringCache));
}

BOOL CScriptVariableManager::AddVariable(const char* pszName, fnGetVariable fnGet)
{
	if (m_xVarList.Add(pszName, (LPVOID)fnGet) == -1)return FALSE;
	return TRUE;
}

BOOL CScriptVariableManager::GetVariable(const char* pszName, CHumanPlayer* pPlayer, GetVariableStruct& var)
{
	BOOL boOK = FALSE;
	std::array<char, 256> szTemp{};
	strcpy_s(szTemp.data(), szTemp.size(), pszName);
	char* Params[5];
	auto callParams = std::make_unique<CallParamEx[]>(4);
	int nParam = SearchParam(szTemp.data(), Params, 5, "*");
	if (nParam > 0)
	{
		for (int i = 1; i < nParam; i++)
		{
			callParams[i-1].pszParam = Params[i];
			callParams[i-1].nParam = StringToInteger(Params[i]);
		}
		fnGetVariable fnGet = (fnGetVariable)m_xVarList.ObjectOf(Params[0]);
		if (fnGet)
		{
			fnGet(Params[0], callParams.get(), nParam - 1, var, pPlayer);
			boOK = TRUE;
		}
	}
	if (boOK) return TRUE;
	return FALSE;
}

VOID CScriptVariableManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	CVariableFile* pFile = NewVariableFile();
	if (pFile == nullptr)return;
	if (!pFile->Load(pszFilename))
	{
		DeleteVariableFile(pFile);
		return;
	}
	if (m_xVarGroupList.Add(pFile->GetName(), (LPVOID)pFile) == -1)
	{
		DeleteVariableFile(pFile);
		return;
	}
}

CVariableFile* CScriptVariableManager::NewVariableFile()
{
	return m_xVarGroupPool.newObject();
}

VOID CScriptVariableManager::DeleteVariableFile(CVariableFile* pFile)
{
	pFile->Clear();
	m_xVarGroupPool.deleteObject(pFile);
}

BOOL CScriptVariableManager::SetVariable(const char* pszName, const char* pszVarName, const char* pszValue)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)
	{
		pFile = NewVariableFile();
		if (pFile == nullptr)return FALSE;
		pFile->SetName(pszName);
		if (m_xVarGroupList.Add(pFile->GetName(), (LPVOID)pFile) == -1)
		{
			DeleteVariableFile(pFile);
			return FALSE;
		}
	}
	pFile->SetVariableValue(pszVarName, pszValue);
	pFile->Save();
	return TRUE;
}

const char* CScriptVariableManager::GetVariable(const char* pszName, const char* pszVarName)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)return nullptr;
	return pFile->GetVariableValue(pszVarName);
}

VOID CScriptVariableManager::DelVariable(const char* pszName, const char* pszVarName)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)return;
	pFile->DelVariable(pszVarName);
}