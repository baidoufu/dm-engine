#include "StdAfx.h"
#include ".\scclientobj.h"

CSCClientObj::CSCClientObj(VOID)
{
	Clean();
}

CSCClientObj::~CSCClientObj(VOID)
{
}

VOID CSCClientObj::Update()
{
	CClientObject::Update();
}

VOID CSCClientObj::RegisterServer(SERVERADDR& Addr, ServerId& Id, const char* pszNickName)
{
	REGISTER_SERVER_INFO info;
	info.addr = Addr;
	info.Id = Id;
	strncpy(info.szName.data(), pszNickName, 64);
	SendMsg(0, SCM_REGISTERSERVER, 0, 0, 0, &info, sizeof(info));
}

VOID CSCClientObj::UpdateInfo(DWORD dwConnections)
{
	SendMsg(dwConnections, SCM_UPDATESERVERINFO, 0, 0, 0, nullptr, 0);
}

VOID CSCClientObj::FindServer(UINT id, servertype type, const char* pszServerName)
{
	SendMsg(id, SCM_FINDSERVER, (WORD)type, 0, 0, (LPVOID)pszServerName);
}

VOID CSCClientObj::SendMsgAcrossServer(UINT id, WORD wCmd, MAS_SENDTYPE sendtype, WORD wParam, const char* pszData, int datasize)
{
	SendMsg(id, SCM_MSGACROSSSERVER, wCmd, (WORD)sendtype, wParam, (LPVOID)pszData, datasize);
}

VOID CSCClientObj::GetGameServerAddr(UINT id, const char* pszServerName)
{
	SendMsg(id, SCM_GETGAMESERVERADDR, 0, 0, 0, (LPVOID)pszServerName);
}
