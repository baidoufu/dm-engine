#pragma once
#include "scriptshell.h"

class CSystemScript : public CScriptShell, public xSingletonClass<CSystemScript>
{
public:
	CSystemScript(VOID);
	virtual ~CSystemScript(VOID);
	VOID Init(CScriptObject* pScriptObject);
};