#include "StdAfx.h"
#include ".\gmmanager.h"
#include ".\humanplayer.h"
#include ".\ScriptObjectMgr.h"
#include ".\cmdproc.h"
#include "scriptshell.h"
#include "scriptview.h"
#include "scripttarget.h"
#include "systemscript.h"
#include "scriptobject.h"
#include "LogicMap.h"

CGmManager::CGmManager(VOID) : m_xCmdList(TRUE)
{
}

CGmManager::~CGmManager(VOID)
{
}

BOOL CGmManager::Load(const char* pszFile)
{
	CStringFile	sf(pszFile);
	char* Params[2];
	int	nParam = 0;
	gm_node* pnode = nullptr;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#') continue;
		nParam = SearchParam(sf[i], Params, 2, "/");
		if (nParam == 2)
		{
			pnode = m_xGmNodePool.newObject();
			if (pnode == nullptr) continue;
			if (m_GmHash.HAdd(Params[0], pnode))
			{
				o_strncpy(pnode->szAccount.data(), Params[0], 16);
				pnode->level = StringToInteger(Params[1]);
			}
		}
	}
	return TRUE;
}

VOID CGmManager::Save(const char* pszFile)
{
	return;
}

int	CGmManager::GetGmLevel(const char* pszAccount)
{
	gm_node* p = (gm_node*)m_GmHash.HGet(pszAccount);
	if (p) return p->level;
	return 0;
}

BOOL CGmManager::ExecGameCmd(const char* pszCommand, CHumanPlayer* pPlayer)
{
	int lenCmd = static_cast<int>(strlen(pszCommand));
	auto buffer = std::make_unique<char[]>(lenCmd + 1);
	auto callParams = std::make_unique<CallParamEx[]>(20);
	
	xCharSet csWht(" \t\"");
	xCharSet csSpl(" \t,");
	o_strncpy(buffer.get(), pszCommand, lenCmd);
	char* Params[20];
	replaceOutPair(buffer.get(), '(', ')', ' ');

	int nParam = ExtractStrings(buffer.get(), csWht, csSpl, Params, 20, FALSE);
	if (nParam > 0)
	{
		GameCommand rcmd;
		StringCacheNode* pRcmdPage = nullptr;
		CLogicMap* pMap = pPlayer->GetMap();
		DWORD dwInterFlag = 0;
		std::vector<std::string> szExtraParams;
		if (pMap && pMap->IsFlagSeted(MF_NOCMD, dwInterFlag, szExtraParams))
		{
			for (size_t i = 0; i < szExtraParams.size(); i ++)
			{
				if (Params[0] == szExtraParams[i])
				{
					pPlayer->SaySystem("@%s 命令在此地图禁止使用.", Params[0]);
					return FALSE;
				}
			}
		}

		GameCommand* pCmd = (GameCommand*)m_xCmdList.ObjectOf(Params[0]);
		
		if (pCmd == nullptr)
		{
			if (pPlayer->GetSystemFlagParam(SF_GAMEMASTER) == 10000)
			{
				memset(&rcmd, 0, sizeof(rcmd));
				pRcmdPage = new StringCacheNode();
				rcmd.fIsGmCmd = TRUE;
				rcmd.nLimitLevel = 10000;
				if (*Params[0] == '@')
				{
					rcmd.fIsCallPage = TRUE;
					rcmd.pPage = pRcmdPage;
					o_strncpy(pRcmdPage->szString, Params[0], 250);
				}
				else
				{
					rcmd.proc = CCommandManager::GetInstance()->GetCommandProc(Params[0]);
				}
				if ((rcmd.fIsCallPage && rcmd.pPage != nullptr) || rcmd.proc != nullptr)
				{
					pCmd = &rcmd;
				}
				else
				{
					delete pRcmdPage;
					pRcmdPage = nullptr;
				}
			}
			if (pCmd == nullptr)
			{
				pPlayer->SaySystem("<%s %s >", getstrings(SD_EXECCMDERROR_BADCOMMAND), Params[0]);
				return FALSE;
			}
		}

		if (pCmd->fIsGmCmd)
		{
			if (!pPlayer->IsGameMaster())
			{
				pPlayer->SaySystem("<%s %s >", getstrings(SD_EXECCMDERROR_BADCOMMAND), Params[0]);//输入错误的命令呢
				return FALSE;
			}
			else if (pPlayer->GetSystemFlagParam(SF_GAMEMASTER) < (INT)pCmd->nLimitLevel)
			{
				pPlayer->SaySystem(getstrings(SD_EXECCMDERROR_GMEXECCMDLEVELLOW));//您的权限不够, 无法执行该GM命令!
				return FALSE;
			}
		}
		else
		{
			if (!pPlayer->IsGameMaster())
			{
				if (pPlayer->GetPropValue(PI_LEVEL) < (INT)pCmd->nLimitLevel)
				{
					pPlayer->SaySystem(getstrings(SD_EXECCMDERROR_PLAYEREXECCMDLEVELLOW));//	"您等级不够, 无法使用该命令.该命令需要等级 %u", pCmd->nLimitLevel );
					return FALSE;
				}
			}
		}

		for (int i = 0; i < nParam - 1; i++)
		{
			if (*Params[i + 1] == '$')
			{
				CScriptTarget* pTarget = pPlayer->GetScriptTarget();
				callParams[i].pszParam = pTarget ? pTarget->GetVariableValue(Params[i + 1] + 1) : nullptr;
			}
			else
				callParams[i].pszParam = Params[i + 1];
			if (callParams[i].pszParam == nullptr)//如果指针为空, 结果为空
				callParams[i].pszParam = "";
			callParams[i].nParam = StringToInteger(callParams[i].pszParam);
		}
		if (pCmd->fIsCallPage && pCmd->pPage)
		{
			CSystemScript::GetInstance()->setCallParams(callParams.get());
			CSystemScript::GetInstance()->Execute(pPlayer->GetScriptTarget(), pCmd->pPage->szString, FALSE);
		}
		else
		{
			CScriptShell shell;
			CScriptPageView view(&shell);
			BOOL b1 = TRUE;
			DWORD result = pCmd->proc(&shell, pPlayer->GetScriptTarget(), &view, callParams.get(), nParam - 1, b1);
			char* p = (char*)view.getPacket().getbuf();
			p[view.getPacket().getsize()] = 0;
			if (pCmd->fIsGmCmd)
			{
				pPlayer->SaySystem("%s%u", getstrings(SD_CMDRESULTVALUEIS), result);//命令返回值
			}
		}
		// 释放临时GM命令的StringCacheNode内存
		if (pRcmdPage != nullptr)
			delete pRcmdPage;
		return TRUE;
	}
	return FALSE;
}

BOOL CGmManager::LoadCommandDef(const char* pszFile)
{
	ClearCmdList();
	CStringFile sf(pszFile);
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		xStringsExtracter<10> cmd(sf[i], ")=", "( \t");
		if (cmd.getCount() != 3)continue;
		MapCommand(StringToInteger(cmd[0]), cmd[1], cmd[2]);
	}
	return TRUE;
}

StringCacheNode* CGmManager::AllocStringCache()
{
	StringCacheNode* pString = m_xStringCachePool.newObject();
	if (pString) // 初始化/重置
		memset(pString, 0, sizeof(StringCacheNode));
	return pString;
}

VOID CGmManager::FreeStringCache(StringCacheNode* pStringCahce)
{
	if (pStringCahce) m_xStringCachePool.deleteObject(pStringCahce);
}

VOID CGmManager::ClearCmdList()
{
	GameCommand* p = nullptr;
	for (UINT i = 0; i < m_xCmdList.GetCount(); i++)
	{
		p = (GameCommand*)m_xCmdList[i]->lpObject;
		if (p->pPage)
			FreeStringCache(p->pPage);
		delete p;
		m_xCmdList[i]->lpObject = nullptr;
	}
	m_xCmdList.Clear();
}

BOOL CGmManager::MapCommand(int iLevel, const char* pszCommand, const char* pszBuildInCommand)
{
	GameCommand gcmd;
	if (iLevel <= 0)
	{
		gcmd.fIsGmCmd = FALSE;
		iLevel = abs(iLevel);
	}
	else
		gcmd.fIsGmCmd = TRUE;
	gcmd.nLimitLevel = iLevel;

	if (*pszBuildInCommand == '@')
	{
		gcmd.fIsCallPage = TRUE;
		if (CSystemScript::GetInstance()->getScriptObject() == 0 || CSystemScript::GetInstance()->getScriptObject()->GetPage(pszBuildInCommand) == nullptr)
		{
			PRINT(ERROR_RED, "%s %s %s\n", pszCommand, getstrings(SD_CMDMAPERROR_SCRIPTNOTFOUND), pszBuildInCommand);
			return FALSE;
		}
		gcmd.pPage = AllocStringCache();
		o_strncpy(gcmd.pPage->szString, pszBuildInCommand, 250);
	}
	else
	{
		gcmd.proc = CCommandManager::GetInstance()->GetCommandProc(pszBuildInCommand);
		if (gcmd.proc == nullptr)
		{
			PRINT(ERROR_RED, "%s %s %s\n", pszCommand, getstrings(SD_CMDMAPERROR_CMDNOTFOUND), pszBuildInCommand);
			return FALSE;
		}
	}

	GameCommand* pCmd = new GameCommand;
	*pCmd = gcmd;
	if (m_xCmdList.Add(pszCommand, (LPVOID)pCmd) == -1)
	{
		PRINT(ERROR_RED, "%s %s\n", pszCommand, getstrings(SD_CMDMAPERROR_CMDALREADYREGISTERED));//命令已经被注册
		delete pCmd;
		return FALSE;
	}
	else
		DPRINT(FUNC_PURPLE, "映射 %s 命令到 %s\n", pszCommand, pszBuildInCommand);
	return TRUE;
}
