#pragma once
#include "clientobj.h"
#include "DBQueryThreadPool.h"
#include "DBConnectionPool.h"

constexpr UINT MAX_CLIENTOBJECT = 64;

class CServer : public CBaseServer, public xIndexObjectPool<CClientObj>, public xSingletonClass<CServer>
{
public:
	CServer(VOID);
	virtual ~CServer(VOID);
public:
	VOID OnInput(const char* pString);
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();
	VOID OnParseMsg(WORD wMsg);
	// 获取异步查询线程池
	CDBQueryThreadPool& GetQueryPool() { return m_queryPool; }
	// 获取连接池
	CMySQLConnectionPool& GetConnectionPool() { return m_connectionPool; }
	// 获取AppDB引用
	CAppDB& GetAppDB() { return m_appDB; }
private:
	CServerTimer m_xxShowTimer;
	std::array<DWORD, DM_END> m_dwMsgTimes;
	CAppDB m_appDB;
	std::array<char, 64> m_szServer;
	std::array<char, 64> m_szPort;
	std::array<char, 64> m_szDatabase;
	std::array<char, 64> m_szAccount;
	std::array<char, 64> m_szPassword;
	// 异步查询线程池
	CDBQueryThreadPool m_queryPool;
	// 数据库连接池
	CMySQLConnectionPool m_connectionPool;
	// 定时刷盘计时器
	DWORD m_dwFlushTimer;
};
