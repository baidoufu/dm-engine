#pragma once
#include "clientobj.h"

constexpr UINT MAX_CLIENTOBJECT = 2048;
constexpr DWORD ENTER_INFO_TIMEOUT = 60 * 1000; // 登录条目超时时间60秒

class CServer :
	public CBaseServer,
	public xIndexObjectPool<CClientObj>,
	public xSingletonClass<CServer>
{
public:
	CServer(VOID);
	virtual ~CServer(VOID);
public:
	VOID OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();
	VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize);
	BOOL GetLoginEnterInfo(UINT nLoginId, UINT nSelCharId, LOGINENTER& info);
protected:
	SERVER_ERROR AddEnterAccount(UINT nLoginId, const char* pszAccount, UINT& nSelCharId);
	VOID CleanupExpiredEnterInfos();
	CIndexListEx<LOGINENTER/*, MAX_CLIENTOBJECT*/>	m_EnterObject;
	CIntHash<1024> m_Inthash;
};
