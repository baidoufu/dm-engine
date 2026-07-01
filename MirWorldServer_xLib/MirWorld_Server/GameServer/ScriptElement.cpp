#include "StdAfx.h"
#include "cmdproc.h"
#include ".\scriptelement.h"
#include ".\scriptview.h"
#include ".\scripttarget.h"
#include ".\scriptshell.h"
#include ".\scriptobjectmgr.h"
#include ".\scriptobject.h"
#include ".\humanplayer.h"
#include <memory>
#include <array>

BOOL g_bDebugScript = FALSE;
xObjectPool<xPacket> CScriptElement::m_xParamStackPool;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：读取GOODS列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_Goods::Parse(CScriptFile& file)
{
	char* pLine = nullptr;
	while (pLine = file.NextLine())
	{
		if (*pLine == '#') continue; // 如果列表中前加了#表示跳过
		if (*pLine == '[')
		{
			file.PrevLine();
			return TRUE;
		}
		xStringsExtracter<3> goods(pLine, " \t", " \t");
		if (goods[0] == nullptr || goods[0][0] == 0)
			continue;
		auto pGoods = std::make_unique<Goods>();
		o_strncpy(pGoods->szName.data(), goods[0], 30);
		if (goods[1])
			pGoods->wCount = (WORD)StringToInteger(goods[1]);
		else
			pGoods->wCount = 0;
		if (goods[2])
			pGoods->wRefreshTime = (WORD)StringToInteger(goods[2]);
		else
			pGoods->wRefreshTime = 5;
		AddGoods(std::move(pGoods));
	}
	return TRUE;
}

bool CSe_Goods::hasFourchar(const char* szName)//没用废弃, 拦不住
{
	size_t len = strlen(szName);
	if (len <= 3)
		return false;
	return true;
}

VOID CSe_Goods::AddGoods(std::unique_ptr<Goods> pGoods)
{
	Goods* pRaw = pGoods.release();
	if (this->m_pGoodsList)
	{
		Goods* p = m_pGoodsList;
		while (p->pNext)
			p = p->pNext;
		p->pNext = pRaw;
	}
	else
		m_pGoodsList = pRaw;
	pRaw->pNext = nullptr;
}

VOID CSe_Goods::Destroy()
{
	Goods* p = m_pGoodsList;
	Goods* pNext = nullptr;
	while (p)
	{
		pNext = p->pNext;
		delete p;
		p = pNext;
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
CSe_Page::CSe_Page(VOID)
{
	this->m_pElements.reset();
	this->m_AccessRight = PAR_PUBLIC;
	this->m_szName[0] = 0;
	m_pObject = nullptr;
}

VOID CSe_Page::Destroy()
{
	if (m_pElements)
	{
		m_pElements->delToTail();
		m_pElements.reset();
	}
}

BOOL CSe_Page::Parse(CScriptFile& file)
{
	CScriptElement::Parse(file);
	char* pLine = file.CurrentLine();
	if (pLine == nullptr)return FALSE;
	char* tp = strchr(pLine, ':');
	if (tp)
	{
		*tp++ = 0;
		pLine = TrimEx(pLine);
		tp = TrimEx(tp);
		if (_stricmp(tp, "public") == 0)
			this->m_AccessRight = PAR_PUBLIC;
		else if (_stricmp(tp, "private") == 0)
			this->m_AccessRight = PAR_PRIVATE;
		else if (_stricmp(tp, "protected") == 0)
			this->m_AccessRight = PAR_PROTECTED;
		else
			this->m_AccessRight = PAR_PUBLIC;
	}
	if (*pLine == '[')pLine++;
	tp = strchr(pLine, ']');
	if (tp)
	{
		*tp = 0;
		pLine = TrimEx(pLine);
	}
	if (*pLine == '@')
		o_strncpy(m_szName, pLine, 60);
	else
	{
		m_szName[0] = '@';
		o_strncpy(m_szName + 1, pLine, 60);
	}
	e_parse_state parsestate = EPS_SAY;
	while (pLine = file.NextLine())
	{
		if (*pLine == 0 || (pLine[0] == '/' && pLine[1] == '/'))continue;
		if (*pLine == '#')
		{
			CScriptElement* pElement = nullptr;
			if (_stricmp(pLine, "#if") == 0 || _stricmp(pLine, "#ifnot") == 0 || _stricmp(pLine, "#ifone") == 0)
			{
				pElement = new CSe_IfStatement;
				if (!pElement->Parse(file))
				{
					PRINT(ERROR_RED, "%s 块解析失败在 %s 的 %u 行!\n", pLine, file.GetFileName(), file.GetCurrentLineNumber());
					delete pElement;
				}
				else
					this->AddScriptElement(pElement);
			}
			else if (_stricmp(pLine, "#switch") == 0)
			{
				pElement = new CSe_SwitchStatement;
				if (!pElement->Parse(file))
				{
					PRINT(ERROR_RED, "switch 块解析失败在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
					delete pElement;
				}
				else
					this->AddScriptElement(pElement);
			}
			else if (_strnicmp(pLine, "#json", 5) == 0)
			{
				parsestate = EPS_JSON;
				pElement = new CSe_JsonStatement;
				if (!pElement->Parse(file))
				{
					PRINT(ERROR_RED, "json 块解析失败在 %s 的 %u 行!", file.GetFileName(), file.GetCurrentLineNumber());
					delete pElement;
				}
				else
					this->AddScriptElement(pElement);
			}
			else if (_strnicmp(pLine, "#flash", 6) == 0)
			{
				parsestate = EPS_FLASH;
				pElement = new CSe_FlashStatement;
				if (!pElement->Parse(file))
				{
					PRINT(ERROR_RED, "flash 块解析失败在 %s 的 %u 行!", file.GetFileName(), file.GetCurrentLineNumber());
					delete pElement;
				}
				else
					this->AddScriptElement(pElement);
			}
			else if (_stricmp(pLine, "#act") == 0)
				parsestate = EPS_ACT;
			else if (_stricmp(pLine, "#say") == 0)
				parsestate = EPS_SAY;
			else
			{
				PRINT(ERROR_RED, "非法指令 %s 在 %s 的 %u 行!\n", file.CurrentLine(), file.GetFileName(), file.GetCurrentLineNumber());
				return FALSE;
			}
		}
		else if (*pLine == '[')
		{
			file.PrevLine();
			break;
		}
		else
		{
			CScriptElement* pElement = nullptr;
			if (parsestate == EPS_ACT)
				pElement = new CSe_NormalAct;
			else
				pElement = new CSe_NormalSay;
			if (!pElement->Parse(file))
			{

				PRINT(ERROR_RED, "语句 %s 解析失败在 %s 的 %u 行!\n", file.CurrentLine(), file.GetFileName(), file.GetCurrentLineNumber());
				delete pElement;
			}
			else
				this->AddScriptElement(pElement);
		}
	}
	return TRUE;
}

VOID CSe_Page::AddScriptElement(CScriptElement* pElement)
{
	if (m_pElements)
		m_pElements->addTail(pElement);
	else
		m_pElements.reset(pElement);
}

BOOL CSe_Page::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (g_bDebugScript)
	{
		PRINT(ERROR_RED, "(");
		PRINT(SUCCESS_GREEN, "%u", this->m_nLineNumber);
		PRINT(ERROR_RED, ")[");
		PRINT(SUCCESS_GREEN, "%s", ((CScriptTargetForPlayer*)pTarget)->GetOwner() ? ((CScriptTargetForPlayer*)pTarget)->GetOwner()->GetName() : "null");
		PRINT(ERROR_RED, "]");
		PRINT(ERROR_RED, ":ExecPage:");
		PRINT(SUCCESS_GREEN, "%s.%s \n", this->m_pObject->getName(), this->m_szName);
	}
	CScriptTargetForPlayer* pPTarget = (CScriptTargetForPlayer*)pTarget;
	if (pPTarget && pPTarget->CheckExceedDistance(TRUE))
		return FALSE;

	CScriptElement* p = m_pElements.get();
	while (p)
	{
		if (!p->Execute(pShell, pTarget, pView))
			return FALSE;
		p = p->getNext();
	}
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_NormalAct::Parse(CScriptFile& file)
{
	CScriptElement::Parse(file);
	xCharSet csWht(" \t\"");
	xCharSet csSpl(" \t,");
	char* pLine = file.CurrentLine();
	if (pLine == nullptr || *pLine == 0)return FALSE;
	char* Params[20];
	replaceOutPair(pLine, '(', ')', ' ');

	int nParam = ExtractStrings(pLine, csWht, csSpl, Params, 20, FALSE);
	if (nParam <= 0)return FALSE;

	m_fnProc = CCommandManager::GetInstance()->GetCommandProc(Params[0]);
	if (m_fnProc == nullptr)return FALSE;

	m_nParamCount = nParam - 1;
	if (m_nParamCount > 0)
	{
		if (m_nParamCount > MAX_CALL_PARAMS + 1)m_nParamCount = MAX_CALL_PARAMS + 1;
		m_pParams = std::make_unique<ScriptParamEx[]>(m_nParamCount);
		for (UINT i = 0;i < m_nParamCount;i++)
		{
			char* p = Params[i + 1];
			if (*p == '$')
			{
				p = CScriptObjectMgr::GetInstance()->GetVariableValue(p + 1);
				if (p)
				{
					m_pParams[i].wType = SP_STRING;
					m_pParams[i].nParam = (UINT)StringToInteger(p);
				}
				else
				{
					m_pParams[i].wType = SP_VARIABLE;
					p = Params[i + 1] + 1;
				}
				m_pParams[i].pszParam = copystring(p);
			}
			else
			{
				//xCharSet whtset( " \t\"" );
				//p = TrimEx( p, whtset );
				m_pParams[i].wType = SP_STRING;
				m_pParams[i].pszParam = copystring(p);
				m_pParams[i].nParam = (UINT)StringToInteger(p);
			}
		}
	}
	m_pViewString.reset(copystring(Params[0]));
	return TRUE;
}

BOOL CSe_NormalAct::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (m_fnProc == nullptr)return FALSE;
	CParamStackHelper paramstack;
	std::array<CallParamEx, MAX_CALL_PARAMS + 1> Params{};
	for (UINT i = 0;i < m_nParamCount;i++) // 构建最终参数表
	{
		if (m_pParams[i].wType == SP_VARIABLE)
		{
			char* p = pTarget->GetVariableValue(m_pParams[i].pszParam);
			if (p)
			{
				Params[i].pszParam = (char*)paramstack.getfreebuf();
				paramstack.push((LPVOID)p, (int)strlen(p) + 1);
				Params[i].nParam = (UINT)StringToInteger(Params[i].pszParam);
			}
			else
			{
				Params[i].pszParam = "";
				Params[i].nParam = 0;
			}
		}
		else
		{
			Params[i].pszParam = m_pParams[i].pszParam;
			Params[i].nParam = m_pParams[i].nParam;
		}
	}
	BOOL bContinue = TRUE;
	if (g_bDebugScript)
	{
		PRINT(ERROR_RED, "(");
		PRINT(SUCCESS_GREEN, "%u", this->m_nLineNumber);
		PRINT(ERROR_RED, ")[");
		PRINT(SUCCESS_GREEN, "%s", ((CScriptTargetForPlayer*)pTarget)->GetOwner() ? ((CScriptTargetForPlayer*)pTarget)->GetOwner()->GetName() : "null");
		PRINT(ERROR_RED, "]");
		PRINT(ERROR_RED, ":ExecCmd:");
		PRINT(SUCCESS_GREEN, "%s ", m_pViewString.get());
		for (UINT i = 0;i < m_nParamCount;i++)
		{
			PRINT(SUCCESS_GREEN, "%s ", Params[i].pszParam);
		}
		PRINT(ERROR_RED, "\n");
	}
	m_dwResult = m_fnProc(pShell, pTarget, pView, Params.data(), m_nParamCount, bContinue);
	if(g_bDebugScript)
		PRINT( ERROR_RED, "返回值: ( %u ) %s\n", m_dwResult, bContinue?"继续":"中断" );
	return bContinue;
}

VOID CSe_NormalAct::Destroy()
{
	m_pParams.reset();
	m_fnProc = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
static thread_local int s_reentryGuard = 0;
static thread_local std::array<char, 65536> s_szTempBuffer{};
BOOL CSe_NormalSay::Parse(CScriptFile& file)
{
	CScriptElement::Parse(file);
	char* pLine = file.CurrentLineRaw();
	if (pLine == nullptr || *pLine == 0)return FALSE;
	if (s_reentryGuard > 0)
	{
		std::string localBuf(s_szTempBuffer.size(), '\0');
		int size = ProcFmtText(pLine, &localBuf[0], (int)localBuf.size() - 1, (xVariableProvider*)CScriptObjectMgr::GetInstance());
		if (size > 0)
			this->m_pSayWords.reset(copystring(localBuf.c_str()));
	}
	else
	{
		++s_reentryGuard;
		int size = ProcFmtText(pLine, s_szTempBuffer.data(), (int)s_szTempBuffer.size() - 1, (xVariableProvider*)CScriptObjectMgr::GetInstance());
		if (size > 0)
			this->m_pSayWords.reset(copystring(s_szTempBuffer.data()));
		--s_reentryGuard;
	}
	return TRUE;
}

BOOL CSe_NormalSay::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (m_pSayWords)
	{
		if (s_reentryGuard > 0)
		{
			std::string localBuf(s_szTempBuffer.size(), '\0');
			ProcFmtText(m_pSayWords.get(), &localBuf[0], (int)localBuf.size() - 1, (xVariableProvider*)pTarget);
			if (g_bDebugScript)
			{
				PRINT(ERROR_RED, "(");
				PRINT(SUCCESS_GREEN, "%u", this->m_nLineNumber);
				PRINT(ERROR_RED, ")[");
				PRINT(SUCCESS_GREEN, "%s", ((CScriptTargetForPlayer*)pTarget)->GetOwner() ? ((CScriptTargetForPlayer*)pTarget)->GetOwner()->GetName() : "null");
				PRINT(ERROR_RED, "]");
				PRINT(ERROR_RED, ":AddWords:");
				PRINT(SUCCESS_GREEN, "%s \n", localBuf.c_str());
			}
			pView->AppendWords(localBuf.c_str());
		}
		else
		{
			++s_reentryGuard;
			int size = ProcFmtText(m_pSayWords.get(), s_szTempBuffer.data(), (int)s_szTempBuffer.size() - 1, (xVariableProvider*)pTarget);
			if (g_bDebugScript)
			{
				PRINT(ERROR_RED, "(");
				PRINT(SUCCESS_GREEN, "%u", this->m_nLineNumber);
				PRINT(ERROR_RED, ")[");
				PRINT(SUCCESS_GREEN, "%s", ((CScriptTargetForPlayer*)pTarget)->GetOwner() ? ((CScriptTargetForPlayer*)pTarget)->GetOwner()->GetName() : "null");
				PRINT(ERROR_RED, "]");
				PRINT(ERROR_RED, ":AddWords:");
				PRINT(SUCCESS_GREEN, "%s \n", s_szTempBuffer.data());
			}
			pView->AppendWords(s_szTempBuffer.data());
			--s_reentryGuard;
		}
	}
	return TRUE;
}

VOID CSe_NormalSay::Destroy()
{
	m_pSayWords.reset();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_IfStatement::Parse(CScriptFile& file)
{
	char* pLine = nullptr;
	e_parse_state parsestate = EPS_ACT;
	if_parse_state statementstate = IPS_IF;
	pLine = file.CurrentLine();
	if (_stricmp(pLine, "#ifnot") == 0)
		this->m_bNot = TRUE;
	else
		this->m_bNot = FALSE;
	if (_stricmp(pLine, "#ifone") == 0)
		this->m_bOne = TRUE;
	else
		this->m_bOne = FALSE;
	while (pLine = file.NextLine())
	{
		if (*pLine == 0)continue;
		if (*pLine == '#')
		{
			if (_stricmp(pLine, "#act") == 0)
			{
				if (statementstate == IPS_FALSE) //	如果正在解析FALSE部分
				{
					file.PrevLine();
					break;
				}
				parsestate = EPS_ACT;
				if (statementstate != IPS_TRUE)statementstate = IPS_TRUE;
			}
			else if (_stricmp(pLine, "#say") == 0)
			{
				if (statementstate == IPS_FALSE)
				{
					file.PrevLine();
					break;
				}
				parsestate = EPS_SAY;
				if (statementstate != IPS_TRUE)statementstate = IPS_TRUE;
			}
			else if (_stricmp(pLine, "#elseact") == 0)
			{
				parsestate = EPS_ACT;
				if (statementstate != IPS_FALSE)statementstate = IPS_FALSE;
			}
			else if (_stricmp(pLine, "#elsesay") == 0)
			{
				parsestate = EPS_SAY;
				if (statementstate != IPS_FALSE)statementstate = IPS_FALSE;
			}
			else if (_stricmp(pLine, "#end") == 0)
				return TRUE;
			else if (_stricmp(pLine, "#elseif") == 0)
			{
				this->m_pElseIfStatement.reset(new CSe_IfStatement);
				return this->m_pElseIfStatement->Parse(file);
			}
			else
			{
				file.PrevLine();
				break;
			}
		}
		else if (*pLine == '[')
		{
			file.PrevLine();
			break;
		}
		else
		{
			CScriptElement* pElement = nullptr;
			if (statementstate == IPS_IF)
				parsestate = EPS_ACT;
			if (parsestate == EPS_ACT)
				pElement = new CSe_NormalAct;
			else
				pElement = new CSe_NormalSay;
			if (!pElement->Parse(file))
			{
				PRINT(ERROR_RED, "语句 %s 解析失败在 %s 的 %u 行!\n", file.CurrentLine(), file.GetFileName(), file.GetCurrentLineNumber());
				delete pElement;
			}
			else
			{
				std::unique_ptr<CScriptElement>* ppList = nullptr;
				if (statementstate == IPS_IF)
					ppList = (std::unique_ptr<CScriptElement>*)&m_pCondition;
				else if (statementstate == IPS_TRUE)
					ppList = &m_pTrueElements;
				else
					ppList = &m_pFalseElements;
				if (ppList)
				{
					if (*ppList)
						(*ppList)->addTail(pElement);
					else
						ppList->reset(pElement);
				}
			}
		}
	}
	return TRUE;
}

BOOL CSe_IfStatement::CheckConditionList(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	CSe_NormalAct* p = m_pCondition.get();
	while (p)
	{
		p->Execute(pShell, pTarget, pView);
		if (this->m_bOne)
		{
			if (p->getResult() == 1) return TRUE;
		}
		else
		{
			if (p->getResult() == 0) return FALSE;
		}
		p = (CSe_NormalAct*)p->getNext();
	}
	return !this->m_bOne;
}

BOOL CSe_IfStatement::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	CScriptElement* pElement = nullptr;
	CScriptElement* p = nullptr;

	BOOL bCheckResult = CheckConditionList(pShell, pTarget, pView);

	if (this->m_bNot)
		bCheckResult = !bCheckResult;
	if (bCheckResult)
		p = m_pTrueElements.get();
	else
		p = m_pFalseElements.get();

	while (p)
	{
		if (!p->Execute(pShell, pTarget, pView))
			return FALSE;
		p = p->getNext();
	}

	if (!bCheckResult && this->m_pElseIfStatement)
		return this->m_pElseIfStatement->Execute(pShell, pTarget, pView);
	return TRUE;
}

VOID CSe_IfStatement::Destroy()
{
	if (m_pCondition)
	{
		m_pCondition->delToTail();
		m_pCondition.reset();
	}
	if (m_pTrueElements)
	{
		m_pTrueElements->delToTail();
		m_pTrueElements.reset();
	}
	if (m_pFalseElements)
	{
		m_pFalseElements->delToTail();
		m_pFalseElements.reset();
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_CaseBlock::Parse(CScriptFile& file)
{
	char* pLine = file.CurrentLine();
	if (pLine == nullptr)return FALSE;

	if (_strnicmp(pLine, "#case:", 6) == 0)
		m_nCode = StringToInteger(pLine + 6);
	else
		m_nCode = 0;
	e_parse_state state = EPS_ACT;
	while (pLine = file.NextLine())
	{
		if (*pLine == 0)continue;
		if (*pLine == '#')
		{
			if (_stricmp(pLine, "#act") == 0)
				state = EPS_ACT;
			else if (_stricmp(pLine, "#say") == 0)
				state = EPS_SAY;
			else
			{
				file.PrevLine();
				break;
			}
		}
		else if (*pLine == '[')
		{
			file.PrevLine();
			break;
		}
		else
		{
			CScriptElement* pElement = nullptr;
			if (state == EPS_ACT)
				pElement = new CSe_NormalAct;
			else
				pElement = new CSe_NormalSay;
			if (!pElement->Parse(file))
			{
				PRINT(ERROR_RED, "语句 %s 解析失败在 %s 的 %u 行!\n", file.CurrentLine(), file.GetFileName(), file.GetCurrentLineNumber());
				delete pElement;
			}
			else
			{
				if (m_pElements)
					m_pElements->addTail(pElement);
				else
					m_pElements.reset(pElement);
			}
		}
	}
	return TRUE;
}

BOOL CSe_CaseBlock::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	CScriptElement* p = this->m_pElements.get();
	while (p)
	{
		if (!p->Execute(pShell, pTarget, pView))
			return FALSE;
		p = p->getNext();
	}
	return TRUE;
}

VOID CSe_CaseBlock::Destroy()
{
	this->m_nCode = 0;
	if (m_pElements)
	{
		this->m_pElements->delToTail();
		this->m_pElements.reset();
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_SwitchStatement::Parse(CScriptFile& file)
{
	UINT nStartIndex = file.GetCurrentLineNumber();
	char* pLine = nullptr;
	UINT nCaseBlocks = 0;
	UINT nDefaultBlocks = 0;
	//	计算CASE的数量
	while (pLine = file.NextLine())
	{
		if (pLine[0] == 0)continue;
		if (pLine[0] == '[')break;
		if (pLine[0] != '#')continue;

		if (_strnicmp(pLine, "#case:", 6) == 0)
			nCaseBlocks++;
		else if (_stricmp(pLine, "#default") == 0)
			continue;
		else if (_stricmp(pLine, "#switch") == 0)
			continue;
		else if (_stricmp(pLine, "#say") == 0)
			continue;
		else if (_stricmp(pLine, "#act") == 0)
			continue;
		else
			break;
	}

	switch_parse_state parsestate = SPS_SWITCH;
	file.SetLineIndex(nStartIndex);
	if (nCaseBlocks > 0)
	{
		m_pCaseBlocks.reserve(nCaseBlocks);
	}
	while (pLine = file.NextLine())
	{
		if (pLine[0] == 0)continue;
		if (pLine[0] == '#')
		{
			parsestate = SPS_CASESAY;
			if (_strnicmp(pLine, "#case:", 6) == 0)
			{
				auto pCaseBlock = std::make_unique<CSe_CaseBlock>();
				if (!pCaseBlock->Parse(file))
				{
					PRINT(ERROR_RED, "CASE块解析失败在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
				}
				else
					m_pCaseBlocks.emplace_back(std::move(pCaseBlock));
			}
			else if (_stricmp(pLine, "#default") == 0)
			{
				if (m_pDefaultBlock)
					PRINT(ERROR_RED, "多余的DEFAULT在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
				else
				{
					m_pDefaultBlock = std::make_unique<CSe_CaseBlock>();
					if (!m_pDefaultBlock->Parse(file))
					{
						PRINT(ERROR_RED, "DEFAULT解析失败在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
						m_pDefaultBlock.reset();
					}
				}
			}
			else if (_stricmp(pLine, "#end") == 0)
				return TRUE;
			else if (_stricmp(pLine, "#switch") == 0)//	多余的switch
			{
				file.PrevLine();
				break;
			}
			else
			{
				file.PrevLine();
				break;
			}
		}
		else if (*pLine == '[')
		{
			file.PrevLine();
			break;
		}
		else
		{
			if (parsestate == SPS_SWITCH)
			{
				if (m_pBranchSource)
				{
					PRINT(ERROR_RED, "警告: Switch过多的条件项目在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
					continue;
				}
				else
				{
					m_pBranchSource = std::make_unique<CSe_NormalAct>();
					if (!m_pBranchSource->Parse(file))
					{
						PRINT(ERROR_RED, "错误: Switch 条件解析失败在 %s 的 %u 行!\n", file.GetFileName(), file.GetCurrentLineNumber());
						m_pBranchSource.reset();
					}
				}
			}
		}
	}
	return TRUE;
}

BOOL CSe_SwitchStatement::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (m_pBranchSource)
	{
		if (!m_pBranchSource->Execute(pShell, pTarget, pView))
			return FALSE;
		DWORD nCode = m_pBranchSource->getResult();

		for (const auto& pCaseBlock : m_pCaseBlocks)
		{
			if (pCaseBlock && pCaseBlock->getCode() == nCode)
				return pCaseBlock->Execute(pShell, pTarget, pView);
		}
	}
	if (m_pDefaultBlock)
		return m_pDefaultBlock->Execute(pShell, pTarget, pView);
	return TRUE;
}

VOID CSe_SwitchStatement::Destroy()
{
	m_pCaseBlocks.clear();
	m_pBranchSource.reset();
	m_pDefaultBlock.reset();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：JSON控制语句解析
//		注释：支持 #json 后面跟数字参数
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_JsonStatement::Parse(CScriptFile& file)
{
	char* pLine = file.CurrentLine();
	if (pLine == nullptr)return FALSE;
	if (_strnicmp(pLine, "#json", 5) != 0)
	{
		PRINT(ERROR_RED, "无效的JSON指令 %s 在 %s 的 %u 行!", pLine, file.GetFileName(), file.GetCurrentLineNumber());
		return FALSE;
	}
	char* pParam = pLine + 5;
	if (*pParam != 0)
		m_nType = (UINT)StringToInteger(pParam);
	else
		m_nType = 0;

	std::string combinedContent;
	while (pLine = file.NextLine())
	{
		if (_stricmp(pLine, "#end") == 0)break;
		combinedContent += pLine;
	}
	m_pWords.reset(copystring(combinedContent.c_str()));
	return TRUE;
}

BOOL CSe_JsonStatement::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (m_pWords)
	{
		if (s_reentryGuard > 0)
		{
			std::string localBuf(s_szTempBuffer.size(), '\0');
			ProcFmtText(m_pWords.get(), &localBuf[0], (int)localBuf.size() - 1, (xVariableProvider*)pTarget);
			CScriptNpc* pNPC = (CScriptNpc*)pShell;
			if (pNPC)
				pNPC->SendMerChantJsonMsg(pTarget, localBuf.c_str(), m_nType);
		}
		else
		{
			++s_reentryGuard;
			ProcFmtText(m_pWords.get(), s_szTempBuffer.data(), (int)s_szTempBuffer.size() - 1, (xVariableProvider*)pTarget);
			CScriptNpc* pNPC = (CScriptNpc*)pShell;
			if (pNPC)
				pNPC->SendMerChantJsonMsg(pTarget, s_szTempBuffer.data(), m_nType);
			--s_reentryGuard;
		}
	}
	return TRUE;
}

VOID CSe_JsonStatement::Destroy()
{
	m_nType = 0;
	m_pWords.reset();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：FLASH控制语句解析
//		注释：支持 #flash 后面跟数字参数
//----------------------------------------------------------------------------------------------------------------------------------------------------------
BOOL CSe_FlashStatement::Parse(CScriptFile& file)
{
	char* pLine = file.CurrentLine();
	if (pLine == nullptr)return FALSE;
	if (_strnicmp(pLine, "#flash", 6) != 0)
	{
		PRINT(ERROR_RED, "无效的FLASH指令 %s 在 %s 的 %u 行!", pLine, file.GetFileName(), file.GetCurrentLineNumber());
		return FALSE;
	}
	char* pParam = pLine + 6;
	if (*pParam != 0)
		m_nType = (UINT)StringToInteger(pParam);
	else
		m_nType = 0;

	std::string combinedContent;
	while (pLine = file.NextLine())
	{
		if (_stricmp(pLine, "#end") == 0)break;
		combinedContent += pLine;
	}
	m_pWords.reset(copystring(combinedContent.c_str()));
	return TRUE;
}

BOOL CSe_FlashStatement::Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView)
{
	if (m_pWords)
	{
		if (s_reentryGuard > 0)
		{
			std::string localBuf(s_szTempBuffer.size(), '\0');
			ProcFmtText(m_pWords.get(), &localBuf[0], (int)localBuf.size() - 1, (xVariableProvider*)pTarget);
			pView->AppendWords(localBuf.c_str());
			pView->SetParam(m_nType);
		}
		else
		{
			++s_reentryGuard;
			ProcFmtText(m_pWords.get(), s_szTempBuffer.data(), (int)s_szTempBuffer.size() - 1, (xVariableProvider*)pTarget);
			pView->AppendWords(s_szTempBuffer.data());
			pView->SetParam(m_nType);
			--s_reentryGuard;
		}
	}
	return TRUE;
}

VOID CSe_FlashStatement::Destroy()
{
	m_nType = 0;
	m_pWords.reset();
}