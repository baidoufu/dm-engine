#pragma once
#include "inc.h"

class xClientObject;
class CMsgProcessor
{
public:
	virtual VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size) {}
	virtual VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize) {}
};