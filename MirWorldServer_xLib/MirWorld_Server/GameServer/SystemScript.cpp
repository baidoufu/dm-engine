#include "StdAfx.h"
#include ".\systemscript.h"

CSystemScript::CSystemScript(VOID)
{
	m_pScriptObject = nullptr;
}

CSystemScript::~CSystemScript(VOID)
{
}

VOID CSystemScript::Init(CScriptObject* pScriptObject)
{
	m_pScriptObject = pScriptObject;
}