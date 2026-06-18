#pragma once
#include <atomic>
#include <memory>
#include "appdb.h"

class CClientObj :
	public CClientObject
{
public:
	CClientObj(VOID);
	virtual ~CClientObj(VOID);
	VOID Clean();
	VOID Update();
	VOID OnConnection();
	VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size);
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	CAppDB& GetAppDB() { return m_appDB; }
	// 获取存活标志，用于异步回调安全验证
	std::shared_ptr<std::atomic<bool>> GetAliveFlag() { return m_bAlive; }
private:
	CAppDB m_appDB;
	std::shared_ptr<std::atomic<bool>> m_bAlive; // 异步回调存活验证标志
};