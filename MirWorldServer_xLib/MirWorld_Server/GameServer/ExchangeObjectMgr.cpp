#include "StdAfx.h"
#include ".\exchangeobjectmgr.h"
#include ".\exchangeobj.h"

xObjectPool<CExchangeObj> CExchangeObjectMgr::m_xExchangeObjPool;
CExchangeObjectMgr::CExchangeObjectMgr(VOID)
{
}

CExchangeObjectMgr::~CExchangeObjectMgr(VOID)
{
}

CExchangeObj* CExchangeObjectMgr::BeginExchange(CHumanPlayer* p1, CHumanPlayer* p2)
{
	CExchangeObj* pObj = newObject();
	if (pObj == nullptr)return nullptr;
	if (pObj->Begin(p1, p2))return pObj;
	deleteObject(pObj);
	return nullptr;
}

VOID CExchangeObjectMgr::EndExchange(CExchangeObj* pObj)
{
	deleteObject(pObj);
}