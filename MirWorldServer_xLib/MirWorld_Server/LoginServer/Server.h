#pragma once
#include "clientobj.h"
#include <memory>
#include <string>
#include <atomic>

constexpr UINT MAX_CLIENTOBJECT = 2048;

class CServer :
	public CBaseServer,
	public xIndexObjectPool<CClientObj>,
	public xSingletonClass<CServer>
{
public:
	CServer(VOID) = default;
	virtual ~CServer(VOID) = default;
public:
	VOID OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();
	BOOL RegisterDisabled() { return m_bDisableRegister; }
	const char* GetServerTips() const { return m_strServerTips.c_str(); }
	const char* GetLoginOkTips() const { return m_strLoginOkTips.c_str(); }
	const char* GetRegisterTips() const { return m_strRegisterTips.c_str(); }
	const char* GetWAddr() const { return m_strWAddr.c_str(); }
	const int GetWPort() const { return m_nWPort; }
	// 生成全局唯一递增LoginId，避免随机数冲突
	UINT NextLoginId() { return ++m_nNextLoginId; }
protected:
	std::string m_strServerTips;
	std::string m_strLoginOkTips;
	std::string m_strRegisterTips;
	BOOL m_bDisableRegister{ FALSE };
	std::string m_strWAddr;
	int m_nWPort{ 0 };
	std::atomic<UINT> m_nNextLoginId{ 1 }; // 原子递增LoginId
};
