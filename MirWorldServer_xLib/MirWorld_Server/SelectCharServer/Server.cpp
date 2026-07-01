#include "StdAfx.h"
#include ".\server.h"
#include "selectcharserver.h"

CServer::CServer(VOID)
{
}

CServer::~CServer(VOID)
{
}

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	m_EnterObject.Create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);
	return TRUE;
}

VOID CServer::CleanServer()
{
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}
	const DWORD dwUpdateKey = GetUpdateKey();
	if ((dwUpdateKey & 1) == 0)
		UpdateDBServer(); // 奇数帧：处理数据中心消息
	else
		UpdateSCServer(); // 偶数帧：处理服务中心消息
	// 定期清理过期的登录条目（每100帧执行一次）
	static DWORD s_cleanupCounter = 0;
	if (++s_cleanupCounter >= 100)
	{
		s_cleanupCounter = 0;
		CleanupExpiredEnterInfos();
	}
}

VOID CServer::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	switch (wCmd)
	{
	case MAS_RESTARTGAME:
	{
		ENTERGAMESERVER* pEnterInfo = (ENTERGAMESERVER*)pszData;
		SERVER_ERROR ret = SE_OK;
		ret = AddEnterAccount(pEnterInfo->nLoginId, pEnterInfo->szAccount.data(), pEnterInfo->nSelCharId);
		PRINT(SUCCESS_GREEN, "%s 重新开始游戏!\n", pEnterInfo->szName.data());
	}
	break;
	case MAS_ENTERSELCHARSERVER:
	{
		//	id/lid/account
		std::array<char*, 5> Params{};
		std::array<char, 200> szRetString{};
		int nParam = SearchParam((char*)pszData, Params.data(), 5, "/");
		SERVER_ERROR ret = SE_OK;
		UINT nClientId = static_cast<UINT>(strtoul(Params[0], nullptr, 10));
		if (nParam == 3)
		{
			UINT nLoginId = static_cast<UINT>(strtoul(Params[1], nullptr, 10));
			UINT nSelCharId = 0;
			LG2("收到登录请求: clientId=%u, loginId=%u, account=%s\n", nClientId, nLoginId, Params[2]);
			if ((ret = AddEnterAccount(nLoginId, Params[2], nSelCharId)) == SE_OK)
			{
				LG2("登录成功: loginId=%u, sid=%u, account=%s\n", nLoginId, nSelCharId, Params[2]);
				//	data = "lid/sid/account"
				sprintf_s(szRetString.data(), szRetString.size(), "%u/%u/%s",
					nLoginId, nSelCharId, Params[2]);
				m_SCClientObj.SendMsgAcrossServer(nClientId, MAS_ENTERSELCHARSERVER, MST_SINGLE, wIndex, szRetString.data(), static_cast<int>(strlen(szRetString.data())));
				break;
			}
		}
		else
			ret = SE_INVALIDPARAM;

		sprintf_s(szRetString.data(), szRetString.size(), "FAIL/%d", ret);
		m_SCClientObj.SendMsgAcrossServer(nClientId, MAS_ENTERSELCHARSERVER, MST_SINGLE, wIndex, szRetString.data(), static_cast<int>(strlen(szRetString.data())));
	}
	break;
	}
}

SERVER_ERROR CServer::AddEnterAccount(UINT nLoginId, const char* pszAccount, UINT& nSelCharId)
{
	LOGINENTER* pEnter = nullptr;
	UINT id = m_EnterObject.New(&pEnter);
	if (id == 0 || pEnter == nullptr)
		return	SE_SERVERFULL;
	if (!m_Inthash.HAdd(nLoginId, id))
	{
		m_EnterObject.Del(id);
		return SE_SERVERFULL;
	}
	pEnter->nListId = id;
	pEnter->nLid = nLoginId;
	pEnter->nSid = nSelCharId == 0 ? static_cast<UINT>(__rdtsc()) : nSelCharId;
	strncpy(pEnter->szAccount.data(), pszAccount, 10);
	pEnter->szAccount[10] = 0;
	pEnter->dwEnterTime = CFrameTime::GetFrameTime();

	nSelCharId = pEnter->nSid;
	return SE_OK;
}

BOOL CServer::GetLoginEnterInfo(UINT nLoginId, UINT nSelCharId, LOGINENTER& info)
{
	UINT id = (UINT)m_Inthash.HGet(nLoginId);
	if (id == 0)return FALSE;
	LOGINENTER* pInfo = m_EnterObject.Get(id);
	//memcpy( &info, pInfo, sizeof( LOGINENTER ));
	if (pInfo == nullptr || pInfo->nSid != nSelCharId || pInfo->nLid != nLoginId)
		return FALSE;
	info = *pInfo;
	m_EnterObject.Del(id);
	m_Inthash.HDel(nLoginId);
	return TRUE;
}

VOID CServer::CleanupExpiredEnterInfos()
{
	DWORD dwNow = CFrameTime::GetFrameTime();
	// 收集超时条目的ID，再逐一删除（不能在ForEach中删除）
	std::vector<UINT> expiredIds;
	m_EnterObject.ForEach([&](LOGINENTER* pInfo) {
		if (pInfo != nullptr && (dwNow - pInfo->dwEnterTime) > ENTER_INFO_TIMEOUT)
		{
			expiredIds.push_back(pInfo->nListId);
		}
	});
	for (UINT id : expiredIds)
	{
		LOGINENTER* pInfo = m_EnterObject.Get(id);
		if (pInfo != nullptr)
		{
			m_Inthash.HDel(pInfo->nLid);
			m_EnterObject.Del(id);
		}
	}
}

