#include "StdAfx.h"
#include ".\clientobj.h"
#include "server.h"
#include "selectcharserver.h"

CClientObj::CClientObj(VOID)
{
	Clean();
}

CClientObj::~CClientObj(VOID)
{
}

VOID CClientObj::Clean()
{
	CClientObject::Clean();
	m_State = SCS_NOTVERIFIED;
	m_TimeOut.Savetime();
	m_nFailCount = 0;
	m_bSelected = FALSE;
	m_szCharName.fill(0);
	m_EnterInfo = LOGINENTER();
	m_nLoginId = 0;
	m_nSelectId = 0;
	m_nActiveCount = 0;
}

VOID CClientObj::OnConnection()
{
	m_TimeOut.Savetime();
}

VOID CClientObj::Update()
{
	if (m_State == SCS_WAITINGFORVERIFIED)//账号验证状态
	{
		if (CServer::GetInstance()->GetLoginEnterInfo(m_nLoginId, m_nSelectId, m_EnterInfo))
		{
			m_TimeOut.Savetime();
			m_State = SCS_VERIFIED;
			PRINT(SUCCESS_GREEN, "帐号 %s 验证成功...\n", m_EnterInfo.szAccount.data());
			if (!QueryCharList())
			{
				PRINT(ERROR_RED, "帐号 %s 查询不了...\n", m_EnterInfo.szAccount.data());
				SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
				Disconnect();
				return;
			}
		}
		else
		{
			if (m_TimeOut.IsTimeOut(10 * 1000))
			{
				PRINT(ERROR_RED, "账号 %s 验证超时...\n", m_EnterInfo.szAccount.data());
				SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
				Disconnect();
				return;
			}
		}
	}
	else
	{
		if (m_TimeOut.IsTimeOut(5 * 60 * 1000))
		{
			PRINT(ERROR_RED, "账号 %s 验证超时\n", m_EnterInfo.szAccount.data());
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect();
			return;
		}
		if (m_nFailCount >= 9)
		{
			LG2("帐号 %s 验证失败次数过多\n", m_EnterInfo.szAccount.data());
			PRINT(ERROR_RED, "帐号 %s 验证失败次数过多...\n", m_EnterInfo.szAccount.data());
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect();
			return;
		}
	}
	CClientObject::Update();
}

VOID CClientObj::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	switch (wCmd)
	{
	case MAS_ENTERGAMESERVER:
	{
		ENTERGAMESERVER* pEnterInfo = (ENTERGAMESERVER*)pszData;
		if (pEnterInfo->nSelCharId == m_EnterInfo.nSid && pEnterInfo->nLoginId == m_EnterInfo.nLid)
		{
			if (m_State == SCS_VERIFIED && pEnterInfo->result == SE_OK)
			{
				char szData[200];
				sprintf_s(szData, sizeof(szData), "%s/%u", m_GameServerAddr.addr.addr.data(),
					m_GameServerAddr.addr.nPort);
				SendMsg(0, SM_SELECTCHAROK, 0, 0, 0, (LPVOID)szData);
				PRINT(SUCCESS_GREEN, "%s\n", szData);
			}
			else if (pEnterInfo->result == SE_SERVERFULL)
			{
				m_bSelected = FALSE; // 重置选择状态，允许重试
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"服务器人满, 请稍后连接!");
			}
			else
			{
				m_bSelected = FALSE; // 重置选择状态，允许重试
				LG2("MAS_ENTERGAMESERVER查询失败\n");
				PRINT(ERROR_RED, "选人查询失败...\n");
				SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			}
		}
	}
	break;
	}
}

VOID CClientObj::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	switch (pMsg->wCmd)
	{
	case SCM_GETGAMESERVERADDR:
	{
		if (pMsg->wParam[0] == SE_OK && m_State == SCS_VERIFIED)
		{
			m_GameServerAddr = *((FINDSERVER_RESULT*)pMsg->data);
			CSCClientObj* pObj = pServer->GetSCConnection();
			if (pObj != nullptr)
			{
				ENTERGAMESERVER	enterinfo;
				enterinfo.nLoginId = m_EnterInfo.nLid;
				enterinfo.nSelCharId = m_EnterInfo.nSid;
				enterinfo.nClientId = getId();
				strncpy(enterinfo.szAccount.data(), m_EnterInfo.szAccount.data(), 10);
				enterinfo.szAccount[10] = 0;
				o_strncpy(enterinfo.szName.data(), m_szCharName.data(), 20);
				pObj->SendMsgAcrossServer(0, MAS_ENTERGAMESERVER, MST_SINGLE, m_GameServerAddr.Id.bIndex, (char*)&enterinfo, sizeof(enterinfo));
				break;
			}
		}
		LG2("SCM_GETGAMESERVERADDR查询失败\n");
		PRINT(ERROR_RED, "SCM_GETGAMESERVERADDR查询失败...\n");
		SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
	}
	break;
	}
}

VOID CClientObj::OnDBMsg(PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	switch (pMsg->wCmd)
	{
	case DM_GETCHARPOSITIONFORSELCHAR:
	{
		tQueryMapPosition_Result* pResult = (tQueryMapPosition_Result*)pMsg->data;
		if (pResult->dwKey != m_dwClientKey)
			break;
		CSCClientObj* pObj = pServer->GetSCConnection();
		if (m_State == SCS_VERIFIED && pObj != nullptr)
		{
			pObj->GetGameServerAddr(getId(), pServer->GetNickName());
		}
	}
	break;
	case DM_RESTORECHARACTER:
	{
		if (m_dwClientKey != *((DWORD*)pMsg->data))
			break;
		if (pMsg->wParam[0] == SE_OK)
		{
			SendMsg(1, SM_UNDELCHAROK, 0, 0, 0);
			QueryCharList();
		}
		else
		{
			LG2("DM_RESTORECHARACTER查询失败\n");
			PRINT(ERROR_RED, "DM_RESTORECHARACTER查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect(1000);
		}
	}
	break;
	case DM_DELETEDCHARLIST:
	{
		if (m_dwClientKey != *((DWORD*)pMsg->data))
			break;
		if (pMsg->wParam[0] == SE_OK)
		{
			SendDelCharList((tQueryCharList_Result*)pMsg->data);
		}
		else
		{
			LG2("DM_DELETEDCHARLIST查询失败\n");
			PRINT(ERROR_RED, "DM_DELETEDCHARLIST查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect(1000);
		}
	}
	break;
	case DM_DELETECHARACTER:
	{
		if (m_dwClientKey != *((DWORD*)pMsg->data))
			break;
		if (pMsg->wParam[0] == SE_OK)
		{
			SendMsg(1, SM_DELCHAROK, 0, 0, 0);
			QueryCharList();
		}
		else
		{
			LG2("DM_DELETECHARACTER查询失败\n");
			PRINT(ERROR_RED, "DM_DELETECHARACTER查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect(1000);
		}
	}
	break;
	case DM_QUERYCHARLIST:
	{
		if (m_dwClientKey != *((DWORD*)pMsg->data))
			break;
		if (pMsg->wParam[0] == SE_OK)
		{
			SendCharList((tQueryCharList_Result*)pMsg->data);
		}
		else
		{
			LG2("DM_QUERYCHARLIST查询失败\n");
			PRINT(ERROR_RED, "DM_QUERYCHARLIST查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
			Disconnect(1000);
		}
	}
	break;
	case DM_CREATECHARACTER:
	{
		if (m_dwClientKey != *((DWORD*)pMsg->data))
			break;
		if (pMsg->wParam[0] == SE_OK)
		{
			SendMsg(1, SM_CREATECHAROK, 0, 0, 0);
			QueryCharList();
		}
		else
		{
			LG2("DM_CREATECHARACTER查询失败\n");
			PRINT(ERROR_RED, "DM_CREATECHARACTER查询失败...\n");
			SendMsg(0, SM_CREATECHARFAIL, 0, 0, 0);
		}
	}
	break;
	default:
		break;
	}
}

VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	BOOL bSaveTime = TRUE;
	if (m_State == SCS_VERIFIED)
	{
		switch (pMsg->wCmd)
		{
		case CM_QUERYSELECTCHAR:
		{
			if (m_bSelected) break;
			CDBClientObj* pObj = pServer->GetDBConnection(0);
			if (pObj != nullptr)
			{
				char* Params[5];
				int nParam = SearchParam(pMsg->data, Params, 5, "/");
				if (nParam == 2)
				{
					m_bSelected = TRUE;
					o_strncpy(this->m_szCharName.data(), Params[1], 20);
					m_TimeOut.Savetime();
					pObj->SendQueryMapPosition(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), pServer->GetNickName(), Params[1]);
					break;
				}
			}
			// DB连接不可用或参数错误，不设置m_bSelected，允许重试
			bSaveTime = FALSE;
			PRINT(ERROR_RED, "CM_QUERYSELECTCHAR查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
		}
		break;
		case CM_QUERYUNDELCHAR:
		{
			CDBClientObj* pObj = pServer->GetDBConnection(0);
			if (pObj != nullptr)
			{
				pObj->SendQueryRestoreChar(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), pServer->GetNickName(), pMsg->data);
				break;
			}
			bSaveTime = FALSE;
			PRINT(ERROR_RED, "CM_QUERYUNDELCHAR查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
		}
		break;
		case CM_QUERYDELCHARLIST:
		{
			CDBClientObj* pObj = pServer->GetDBConnection(0);
			if (pObj != nullptr)
			{
				pObj->SendQueryDeletedCharList(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), pServer->GetNickName());
				break;
			}
			bSaveTime = FALSE;
			PRINT(ERROR_RED, "CM_QUERYDELCHARLIST查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
		}
		break;
		case CM_QUERYDELCHAR:
		{
			CDBClientObj* pObj = pServer->GetDBConnection(0);
			if (pObj != nullptr)
			{
				if (m_nActiveCount > 0)
					m_nActiveCount--;
				pObj->SendQueryDelChar(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), pServer->GetNickName(), pMsg->data);
				break;
			}
			bSaveTime = FALSE;
			PRINT(ERROR_RED, "CM_QUERYDELCHAR查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
		}
		break;
		case CM_QUERYCREATECHAR:
		{
			if (m_nActiveCount >= 2)
			{
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"已经激活两个角色, 不能再创建角色!");
				break;
			}
			char* Params[10];
			int nParam = SearchParam(pMsg->data, Params, 10, "/");
			//	比如验证之后,才能创建角色
			if (nParam == 5)
			{
				CDBClientObj* pObj = (CDBClientObj*)pServer->GetDBConnection(0);
				if (pObj)
				{
					m_nActiveCount++;
					pObj->SendQueryCreateChar(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), pServer->GetNickName(), Params[1],
						static_cast<BYTE>(atoi(Params[3])), static_cast<BYTE>(atoi(Params[4])), static_cast<BYTE>(atoi(Params[2])));
					break;
				}
			}
			bSaveTime = FALSE;
			PRINT(ERROR_RED, "CM_QUERYCREATECHAR查询失败...\n");
			SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0);
		}
		break;
		case CS_SELCHAR_CHARSERVER:
		{
		}
		break;
		default:
		{
			m_nFailCount++;
			bSaveTime = FALSE;
#ifdef _DEBUG
			pServer->OnUnknownMsg(pMsg, datasize);
#endif
		}
		break;
		}
	}
	else
	{
		if (pMsg->wCmd == CM_QUERYCHARLIST)
		{
			char* Params[5];
			int nParam = SearchParam(pMsg->data, Params, 5, "/");
			if (nParam == 2)
			{
				if (*Params[0] == '*')
					Params[0]++;
				m_nLoginId = static_cast<UINT>(strtoul(Params[0], nullptr, 10));
				m_nSelectId = static_cast<UINT>(strtoul(Params[1], nullptr, 10));
				m_State = SCS_WAITINGFORVERIFIED;
				this->m_TimeOut.Savetime();
			}
			else 
			{
				SendMsg(0, SM_QUERYCHR_FAIL, 0, 0, 0, nullptr);
			}
		}
		else
		{
			//PRINT(ERROR_RED, "pMsg->wCmd == CM_QUERYCHARLIST为假...\n");
			//SendMsg( 0,SM_QUERYCHR_FAIL, 0, 0, 0, nullptr );
			//m_nFailCount ++;
			//bSaveTime = FALSE;    23.8.27  匹配1.9客户端修改这里, 因为客户端发了460
		}
	}
	if (m_bSelected) return;
	if (bSaveTime)m_TimeOut.Savetime();
}

BOOL CClientObj::QueryCharList()
{
	CDBClientObj* pObj = (CDBClientObj*)CServer::GetInstance()->GetDBConnection(0);
	if (pObj)
	{
		pObj->SendQueryCharlist(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), CServer::GetInstance()->GetNickName());
		return TRUE;
	}
	return FALSE;
}

VOID CClientObj::SendCharList(tQueryCharList_Result* pResult)
{
	CHAR szData[2048] = "";
	char* p = szData;
	//data = "*名字/职业/头发/级别/性别/"
	//两个人的情况 * 在上次登陆（或者新创建的）的角色名字前面
	//data = "名字/职业/头发/级别/性别/名字/职业/头发/级别/性别/"
	int nCount = pResult->count;
	for (int i = 0; i < nCount; i++)
	{
		if (i >= MAX_CHARLISTCOUNT)break;
		if (pResult->charlist[i].date.bflag)
			*p++ = '*';
		sprintf_s(p, sizeof(szData) - (p - szData), "%s/%u/%u/%u/%u/", pResult->charlist[i].szName, pResult->charlist[i].btClass,
			pResult->charlist[i].btHair, pResult->charlist[i].wLevel, pResult->charlist[i].btSex);
		p += strlen(p);
	}
	SendMsg(nCount, SM_CHARLIST, 0, 0, 1, (LPVOID)szData);
	m_nActiveCount = nCount;
}

VOID CClientObj::SendDelCharList(tQueryCharList_Result* pResult)
{
	CHAR szData[2048] = "";
	char* p = szData;
	//data = "名字/职业/头发/级别/性别/yyyy-mm-dd hh:mm/"
	int nCount = pResult->count;
	for (int i = 0; i < nCount; i++)
	{
		if (i >= MAX_DELCHARLISTCOUNT)break;
		sprintf_s(p, sizeof(szData) - (p - szData), "%s/%u/%u/%u/%u/%04u-%02u-%02u %02u:%02u/", pResult->charlist[i].szName, pResult->charlist[i].btClass,
			pResult->charlist[i].btHair, pResult->charlist[i].wLevel, pResult->charlist[i].btSex,
			pResult->charlist[i].date.year, pResult->charlist[i].date.month, pResult->charlist[i].date.day,
			pResult->charlist[i].date.hour, pResult->charlist[i].date.minute);
		p += strlen(p);
	}
	SendMsg(nCount, SM_DELCHARLIST, 0, 0, 1, (LPVOID)szData);
}
