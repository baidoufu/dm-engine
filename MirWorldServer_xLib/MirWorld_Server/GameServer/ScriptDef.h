#pragma once
#include <array>
typedef struct tagGoods
{
	tagGoods()
	{
		FILLSELF(0);
	}
	std::array<char, 32> szName{};
	WORD wCount;
	WORD wRefreshTime;
	tagGoods* pNext;
}Goods;

enum script_param
{
	SP_DATAPOINTER,		//数据指针
	SP_INTEGER,			//整数类型
	SP_STRING,			//字符串类型
	SP_VARIABLE,		//变量
};

typedef struct tagScriptParamEx
{
	tagScriptParamEx()
	{
		FILLSELF(0);
	}
	WORD wType;
	WORD wSize;
	char* pszParam;
	UINT nParam;
}ScriptParamEx;

typedef struct tagCallParamEx
{
	tagCallParamEx()
	{
		FILLSELF(0);
		pszParam = "";
	}

	tagCallParamEx& operator =(const tagCallParamEx& p1)
	{
		nParam = p1.nParam;
		this->pszParam = p1.pszParam;
		if (p1.pszParam == p1.szbuffer.data())
		{
			o_strncpy(szbuffer.data(), p1.szbuffer.data(), 19);
			this->pszParam = szbuffer.data();
		}
		else if (this->pszParam == nullptr)
			this->pszParam = "";
		return *this;
	}

	tagCallParamEx& operator =(const char* szParam)
	{
		pszParam = (char*)szParam;
		if (szParam)
		{
			nParam = StringToInteger(szParam);
		}
		else
		{
			nParam = 0;
			szParam = "";
		}
		return *this;
	}

	tagCallParamEx& operator =(UINT val)
	{
		nParam = val;
		pszParam = szbuffer.data();
		sprintf(szbuffer.data(), "%u", val);
		return *this;
	}

	char* pszParam;
	UINT nParam;
	std::array<char, 20> szbuffer{};
}CallParamEx;

class CScriptShell;
class CScriptTarget;
class CScriptView;
//class CSe_NormalAct;

typedef DWORD(*fnCommandProc)(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView, CallParamEx* Params, UINT nParam, BOOL& bContinue);

typedef struct tagGetVariableStruct
{
	tagGetVariableStruct()
	{
		FILLSELF(0);
	}
	VOID SetValue(int nVal)
	{
		nType = 0;
		nValue = (UINT)nVal;
	}
	VOID SetValue(DWORD nVal)
	{
		nType = 0;
		nValue = (UINT)nVal;
	}
	VOID SetValue(WORD nVal)
	{
		nType = 0;
		nValue = (UINT)nVal;
	}
	VOID SetValue(BYTE nVal)
	{
		nType = 0;
		nValue = (UINT)nVal;
	}
	VOID SetValue(UINT nVal)
	{
		nType = 0;
		nValue = nVal;
	}
	VOID SetValue(char* pszVal)
	{
		nType = 1;
		pszValue = pszVal;
	}
	VOID SetValue(const char* pszVal)
	{
		nType = 1;
		pszValue = (char*)pszVal;
	}
	union
	{
		UINT nValue;
		char* pszValue;
	};
	UINT nType;
}GetVariableStruct;

class CHumanPlayer;
typedef BOOL(*fnGetVariable)(const char* pVarName, CallParamEx* Params, UINT nParam, GetVariableStruct& var, CHumanPlayer* pPlayer);

enum USEITEM_RESULT
{
	UR_NORESULT,
	UR_UPDATED,
	UR_DELETED,
};
