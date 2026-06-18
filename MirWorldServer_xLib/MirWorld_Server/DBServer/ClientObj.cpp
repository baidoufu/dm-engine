#include "StdAfx.h"
#include ".\clientobj.h"
#include "server.h"
#include "dbserver.h"
#include "DBQueryThreadPool.h"

static VOID _tout(char* pret, char* p, int maxlen)
{
	if (p == nullptr || pret == nullptr || maxlen <= 0)
	{
		if (pret) pret[0] = 0;
		return;
	}
	int len = (unsigned char)*p; // 使用unsigned char避免符号扩展导致负值
	if (len > maxlen)
		len = maxlen;
	p++;
	strncpy(pret, p, len);
	pret[len] = 0;
}

CClientObj::CClientObj(VOID)
{
	Clean();
}

CClientObj::~CClientObj(VOID)
{
}

VOID CClientObj::Clean()
{
	// 使旧的存活标志失效，防止已提交的异步回调访问回收后的对象
	if (m_bAlive)
		m_bAlive->store(false);
	// 创建新的存活标志，标记当前对象生命周期开始
	m_bAlive = std::make_shared<std::atomic<bool>>(true);
	// 安全重置AppDB状态：归还连接池连接、清空缓冲区
	// 替代原来的手动析构+placement new，避免非POD类型的不安全重建
	m_appDB.Reset();
	CClientObject::Clean();
}

VOID CClientObj::Update()
{
	CClientObject::Update();
}

VOID CClientObj::OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size)
{
}

// ============================================================================
// 消息处理入口
// 异步核心原则：
//   1. pMsg 的内存在异步执行时可能已被 IOCP 层回收，必须**值捕获**所有请求参数
//   2. SendMsg 只能在主线程的 resultFunc 中调用（网络层非线程安全）
//   3. 需要保序的复合操作（如 CreateItem → FindItemId）在单个 queryFunc 内完成
//   4. fire-and-forget 操作不需要 resultFunc
// ============================================================================
VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	CAppDB& appDB = GetAppDB();
	CDBQueryThreadPool& queryPool = pServer->GetQueryPool();

	pServer->OnParseMsg(pMsg->wCmd);
	// 如果线程池未启动，降级为同步模式
	const BOOL bAsync = queryPool.IsRunning();
	// 保存当前客户端Key，用于异步回调时验证对象是否仍然有效
	const DWORD dwSavedClientKey = m_dwClientKey;
	// 捕获存活标志的shared_ptr副本，避免通过this访问可能被回收的对象
	auto& aliveFlag = m_bAlive;
	// 内联辅助：验证客户端对象是否仍属于当前连接
	// 通过shared_ptr引用的atomic标志验证，不依赖this指针
	auto IsClientValid = [aliveFlag, dwSavedClientKey, this]() -> BOOL {
		return (aliveFlag && aliveFlag->load() && m_dwClientKey == dwSavedClientKey && IsConnected());
	};
	switch (pMsg->wCmd)
	{
	// ====================================================================
	// 查询类操作 — 需要返回结果（request-response）
	// ====================================================================
	case DM_QUERYTASKINFO:
	{
		if (bAsync)
		{
			// 拷贝请求参数
			DWORD dwOwner = *(DWORD*)pMsg->data;
			DWORD dwFlag = pMsg->dwFlag;
			WORD wKey1 = pMsg->wParam[0], wKey2 = pMsg->wParam[1];

			auto pResult = std::make_shared<TASKINFO>();
			memcpy(pResult.get(), pMsg->data, sizeof(TASKINFO));

			DBAsyncWork work;
			work.queryFunc = [dwOwner, pResult](CAppDB& db) {
				pResult->dwAchievement = 0;  // 重置用于接收查询结果
				db.QueryTaskInfo(dwOwner, pResult.get());
			};
			work.resultFunc = [this, pResult, dwFlag, wKey1, wKey2, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_QUERYTASKINFO, wKey1, wKey2, 0, (LPVOID)pResult.get(), sizeof(TASKINFO));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			pMsg->wParam[2] = appDB.QueryTaskInfo(*(DWORD*)pMsg->data, (TASKINFO*)pMsg->data);
			SendMsg(pMsg, sizeof(TASKINFO));
		}
	}
	break;
	case DM_QUERYFENGHAO:
	{
		if (bAsync)
		{
			DWORD dwOwner = *(DWORD*)pMsg->data;
			DWORD dwFlag = pMsg->dwFlag;
			WORD wKey1 = pMsg->wParam[0], wKey2 = pMsg->wParam[1];

			auto pResult = std::make_shared<FenghaoInfo>();
			memcpy(pResult.get(), pMsg->data, sizeof(FenghaoInfo));

			DBAsyncWork work;
			work.queryFunc = [dwOwner, pResult](CAppDB& db) {
				db.QueryFengHaoInfo(dwOwner, pResult.get());
			};
			work.resultFunc = [this, pResult, dwFlag, wKey1, wKey2, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_QUERYFENGHAO, wKey1, wKey2, 0, (LPVOID)pResult.get(), sizeof(FenghaoInfo));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			pMsg->wParam[2] = appDB.QueryFengHaoInfo(*(DWORD*)pMsg->data, (FenghaoInfo*)pMsg->data);
			SendMsg(pMsg, sizeof(FenghaoInfo));
		}
	}
	break;
	case DM_EXECSQL:
	{
		// DM_EXECSQL 比较复杂，涉及动态列定义，保持同步执行以保证正确性
		// 未来可以拆分为更细粒度的异步操作
		xPacketPool::ScopedPacket packet(65536);
		DWORD* pdwParam = (DWORD*)pMsg->data;
		char* pszString = pMsg->data + sizeof(DWORD) * 2;
		EXECSQLRECORD* pRecord = (EXECSQLRECORD*)(pMsg->data + sizeof(DWORD) * 2 + pdwParam[1]);
		appDB.ExecSqlCommand(pdwParam[0], pszString, pRecord, *packet);
		SendMsg(pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], (LPVOID)packet->getbuf(), static_cast<int>(packet->getsize()));
	}
	break;
	case DM_QUERYUPGRADEITEM:
	{
		if (bAsync)
		{
			DWORD dwOwner = *(DWORD*)pMsg->data;
			DWORD dwFlag = pMsg->dwFlag;
			WORD wKey1 = pMsg->wParam[0], wKey2 = pMsg->wParam[1];

			auto pItems = std::make_shared<DBITEM>();
			memset(pItems.get(), 0, sizeof(DBITEM));
			auto pCount = std::make_shared<int>(1);
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, pItems, pCount, pRet](CAppDB& db) {
				*pRet = db.QueryItems(dwOwner, IDF_UPGRADE, pItems.get(), *pCount);
			};
			work.resultFunc = [this, pItems, pCount, pRet, dwFlag, wKey1, wKey2, IsClientValid]() {
				if (!IsClientValid()) return;
				if (*pRet == SE_OK)
				{
					SendMsg(dwFlag, DM_QUERYUPGRADEITEM, wKey1, wKey2, *pCount, (LPVOID)pItems.get(), sizeof(DBITEM));
				}
				else
				{
					SendMsg(dwFlag, DM_QUERYUPGRADEITEM, wKey1, wKey2, 0, nullptr, 0);
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			int count = 1;
			if (appDB.QueryItems(*(DWORD*)pMsg->data, IDF_UPGRADE, (DBITEM*)pMsg->data, count) == SE_OK)
			{
				pMsg->wParam[2] = count;
				SendMsg(pMsg, sizeof(DBITEM));
			}
			else
			{
				pMsg->wParam[2] = 0;
				SendMsg(pMsg, 0);
			}
		}
	}
	break;
	case DM_QUERYMAGIC:
	{
		if (bAsync)
		{
			DWORD dwOwner = *(DWORD*)pMsg->data;
			DWORD dwFlag = pMsg->dwFlag;
			WORD wKey1 = pMsg->wParam[0], wKey2 = pMsg->wParam[1];

			auto pMagicArray = std::make_shared<std::array<MAGICDB, 64>>();
			memset(pMagicArray->data(), 0, sizeof(MAGICDB) * 64);
			auto pCount = std::make_shared<int>(64);
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, pMagicArray, pCount, pRet](CAppDB& db) {
				*pRet = db.QueryMagic(dwOwner, pMagicArray->data(), *pCount);
			};
			work.resultFunc = [this, pMagicArray, pCount, pRet, dwFlag, wKey1, wKey2, IsClientValid]() {
				if (!IsClientValid()) return;
				if (*pRet != SE_OK)
				{
					SendMsg(dwFlag, DM_QUERYMAGIC, wKey1, wKey2, 0x8000 | (*pRet & 0xffff), nullptr, 0);
				}
				else
				{
					SendMsg(dwFlag, DM_QUERYMAGIC, wKey1, wKey2, *pCount, (LPVOID)pMagicArray->data(), *pCount * sizeof(MAGICDB));
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			int	count = 64;
			SERVER_ERROR ret = appDB.QueryMagic(*(DWORD*)pMsg->data, (MAGICDB*)pMsg->data, count);
			if (ret != SE_OK)
			{
				pMsg->wParam[2] = 0x8000 | (ret & 0xffff);
				count = 0;
			}
			else
			{
				pMsg->wParam[2] = count;
			}
			SendMsg(pMsg, count * sizeof(MAGICDB));
		}
	}
	break;
	case DM_QUERYITEMS:
	{
		if (bAsync)
		{
			DWORD* pdwArray = (DWORD*)pMsg->data;
			DWORD dwKey = pdwArray[0];         // data[0] = key（GameServer 端用于校验）
			DWORD dwOwner = pdwArray[1];        // data[1] = owner
			BYTE btFlag = static_cast<BYTE>(pMsg->wParam[1]);
			int maxCount = (int)pMsg->wParam[2];
			DWORD dwFlag = pMsg->dwFlag;

			auto pItems = std::make_shared<std::vector<DBITEM>>(maxCount);
			memset(pItems->data(), 0, sizeof(DBITEM) * maxCount);
			auto pCount = std::make_shared<int>(maxCount);
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, btFlag, pItems, pCount, pRet](CAppDB& db) {
				*pRet = db.QueryItems(dwOwner, btFlag, pItems->data(), *pCount);
			};
			work.resultFunc = [this, pItems, pCount, pRet, dwFlag, btFlag, dwKey, IsClientValid]() {
				if (!IsClientValid()) return;
				if (*pRet == SE_OK)
				{
					// 同步版格式：DWORD(key) + DBITEM * count
					auto pBuf = std::make_shared<std::vector<char>>(sizeof(DWORD) + sizeof(DBITEM) * *pCount);
					*(DWORD*)pBuf->data() = dwKey;
					memcpy(pBuf->data() + sizeof(DWORD), pItems->data(), sizeof(DBITEM) * *pCount);
					SendMsg(dwFlag, DM_QUERYITEMS, *pRet, btFlag, *pCount, (LPVOID)pBuf->data(), static_cast<int>(pBuf->size()));
				}
				else
				{
					// 同步版失败：data 中仍包含 key
					SendMsg(dwFlag, DM_QUERYITEMS, *pRet, btFlag, 0, (LPVOID)&dwKey, sizeof(DWORD));
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			DWORD* pdwArray = (DWORD*)pMsg->data;
			int count = (int)pMsg->wParam[2];
			DBITEM* pItems = (DBITEM*)(pMsg->data + sizeof(DWORD));
			pMsg->wParam[0] = appDB.QueryItems(pdwArray[1], static_cast<BYTE>(pMsg->wParam[1]), pItems, count);
			if (pMsg->wParam[0] == SE_OK)
			{
				pMsg->wParam[2] = count;
				SendMsg(pMsg, sizeof(DWORD) + sizeof(DBITEM) * count);
			}
			else
				SendMsg(pMsg, sizeof(DWORD));
		}
	}
	break;
	case DM_CREATEITEM:
	{
		if (bAsync)
		{
			CREATEITEM info;
			memcpy(&info, pMsg->data, sizeof(CREATEITEM));
			DWORD dwFlag = pMsg->dwFlag;
			int nDatasize = datasize;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);
			auto pItemResult = std::make_shared<CREATEITEM>(info);

			DBAsyncWork work;
			work.queryFunc = [pItemResult, pRet](CAppDB& db) {
				*pRet = db.CreateItem(pItemResult->dwOwner, pItemResult->btFlag, pItemResult->wPos, &pItemResult->item);
			};
			work.resultFunc = [this, pItemResult, pRet, dwFlag, nDatasize, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_CREATEITEM, (WORD)*pRet, 0, 0, (LPVOID)pItemResult.get(), nDatasize);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			CREATEITEM* pInfo = (CREATEITEM*)pMsg->data;
			pMsg->wParam[0] = appDB.CreateItem(pInfo->dwOwner, pInfo->btFlag, pInfo->wPos, &pInfo->item);
			SendMsg(pMsg, datasize);
		}
	}
	break;
	case DM_GETCHARDBINFO:
	{
		if (bAsync)
		{
			tQueryPersonInfo info;
			memcpy(&info, pMsg->data, sizeof(tQueryPersonInfo));
			DWORD dwFlag = pMsg->dwFlag;

			auto pResult = std::make_shared<CHARDBINFO>();
			memset(pResult.get(), 0, sizeof(CHARDBINFO));
			pResult->dwClientKey = info.dwKey;
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pResult, pRet](CAppDB& db) {
				*pRet = db.GetCharDBInfo(info.szAccount.data(), info.szServerName.data(), info.szName.data(), pResult.get());
			};
			work.resultFunc = [this, pResult, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_GETCHARDBINFO, (WORD)*pRet, 0, 0, (LPVOID)pResult.get(), sizeof(CHARDBINFO));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryPersonInfo* pInfo = (tQueryPersonInfo*)pMsg->data;
			pMsg->wParam[0] = appDB.GetCharDBInfo(pInfo->szAccount.data(), pInfo->szServerName.data(), pInfo->szName.data(), (CHARDBINFO*)pMsg->data);
			SendMsg(pMsg, sizeof(CHARDBINFO));
		}
	}
	break;
	case DM_GETCHARPOSITIONFORSELCHAR:
	{
		if (bAsync)
		{
			tQueryPersonInfo info;
			memcpy(&info, pMsg->data, sizeof(tQueryPersonInfo));
			DWORD dwFlag = pMsg->dwFlag;

			auto pResult = std::make_shared<tQueryMapPosition_Result>();
			memset(pResult.get(), 0, sizeof(tQueryMapPosition_Result));
			pResult->dwKey = info.dwKey;
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pResult, pRet](CAppDB& db) {
				*pRet = db.GetMapPosition(info.szAccount.data(), info.szServerName.data(), info.szName.data(), pResult.get());
			};
			work.resultFunc = [this, pResult, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_GETCHARPOSITIONFORSELCHAR, (WORD)*pRet, 0, 0, (LPVOID)pResult.get(), sizeof(tQueryMapPosition_Result));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;
			pMsg->wParam[0] = appDB.GetMapPosition(pinfo->szAccount.data(), pinfo->szServerName.data(), pinfo->szName.data(), (tQueryMapPosition_Result*)pMsg->data);
			SendMsg(pMsg, sizeof(tQueryMapPosition_Result));
		}
	}
	break;
	case DM_RESTORECHARACTER:
	{
		if (bAsync)
		{
			tQueryPersonInfo info;
			memcpy(&info, pMsg->data, sizeof(tQueryPersonInfo));
			DWORD dwFlag = pMsg->dwFlag;
			DWORD dwKey = info.dwKey;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pRet](CAppDB& db) {
				*pRet = db.RestoreCharacter(info.szAccount.data(), info.szServerName.data(), info.szName.data());
			};
			work.resultFunc = [this, pRet, dwFlag, dwKey, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_RESTORECHARACTER, (WORD)*pRet, 0, 0, (LPVOID)&dwKey, sizeof(DWORD));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;
			pMsg->wParam[0] = appDB.RestoreCharacter(pinfo->szAccount.data(), pinfo->szServerName.data(), pinfo->szName.data());
			SendMsg(pMsg, 4);
		}
	}
	break;
	case DM_DELETEDCHARLIST:
	{
		if (bAsync)
		{
			tQueryCharList info;
			memcpy(&info, pMsg->data, sizeof(tQueryCharList));
			DWORD dwFlag = pMsg->dwFlag;

			auto pResult = std::make_shared<tQueryCharList_Result>();
			memset(pResult.get(), 0, sizeof(tQueryCharList_Result));
			pResult->dwKey = info.dwKey;
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pResult, pRet](CAppDB& db) {
				*pRet = db.GetDelCharList(info.szAccount.data(), info.szServerName.data(), pResult.get());
			};
			work.resultFunc = [this, pResult, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_DELETEDCHARLIST, (WORD)*pRet, 0, 0, (LPVOID)pResult.get(), pResult->GetSize());
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryCharList* plist = (tQueryCharList*)pMsg->data;
			tQueryCharList_Result* pResult = (tQueryCharList_Result*)pMsg->data;
			pMsg->wParam[0] = appDB.GetDelCharList(plist->szAccount.data(), plist->szServerName.data(), pResult);
			SendMsg(pMsg, pResult->GetSize());
		}
	}
	break;
	case DM_DELETECHARACTER:
	{
		if (bAsync)
		{
			tQueryPersonInfo info;
			memcpy(&info, pMsg->data, sizeof(tQueryPersonInfo));
			DWORD dwFlag = pMsg->dwFlag;
			DWORD dwKey = info.dwKey;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pRet](CAppDB& db) {
				*pRet = db.DelCharacter(info.szAccount.data(), info.szServerName.data(), info.szName.data());
			};
			work.resultFunc = [this, pRet, dwFlag, dwKey, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_DELETECHARACTER, (WORD)*pRet, 0, 0, (LPVOID)&dwKey, sizeof(DWORD));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;
			pMsg->wParam[0] = appDB.DelCharacter(pinfo->szAccount.data(), pinfo->szServerName.data(), pinfo->szName.data());
			SendMsg(pMsg, 4);
		}
	}
	break;
	case DM_CREATECHARACTER:
	{
		if (bAsync)
		{
			CREATECHARDESC desc;
			memcpy(&desc, pMsg->data, sizeof(CREATECHARDESC));
			DWORD dwFlag = pMsg->dwFlag;
			DWORD dwKey = desc.dwKey;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [desc, pRet](CAppDB& db) mutable {
				*pRet = db.CreateCharacter(&desc);
			};
			work.resultFunc = [this, pRet, dwFlag, dwKey, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_CREATECHARACTER, (WORD)*pRet, 0, 0, (LPVOID)&dwKey, sizeof(DWORD));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			pMsg->wParam[0] = appDB.CreateCharacter((CREATECHARDESC*)pMsg->data);
			SendMsg(pMsg, 4);
		}
	}
	break;
	case DM_QUERYCHARLIST:
	{
		if (bAsync)
		{
			tQueryCharList info;
			memcpy(&info, pMsg->data, sizeof(tQueryCharList));
			DWORD dwFlag = pMsg->dwFlag;

			auto pResult = std::make_shared<tQueryCharList_Result>();
			memset(pResult.get(), 0, sizeof(tQueryCharList_Result));
			pResult->dwKey = info.dwKey;
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [info, pResult, pRet](CAppDB& db) {
				*pRet = db.GetCharList(info.szAccount.data(), info.szServerName.data(), pResult.get());
			};
			work.resultFunc = [this, pResult, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_QUERYCHARLIST, (WORD)*pRet, 0, 0, (LPVOID)pResult.get(), pResult->GetSize());
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			tQueryCharList* plist = (tQueryCharList*)pMsg->data;
			tQueryCharList_Result* pResult = (tQueryCharList_Result*)pMsg->data;
			pMsg->wParam[0] = appDB.GetCharList(plist->szAccount.data(), plist->szServerName.data(), pResult);
			SendMsg(pMsg, pResult->GetSize());
		}
	}
	break;
	case DM_CHANGEPASSWORD:
	{
		if (bAsync)
		{
			char* Params[4];
			int nParam = SearchParam(pMsg->data, Params, 4, "\t");
			DWORD dwFlag = pMsg->dwFlag;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_LOGIN_ACCOUNTNOTEXIST);
			std::string szP1, szP2, szP3;

			if (nParam == 3)
			{
				szP1 = Params[0];
				szP2 = Params[1];
				szP3 = Params[2];

				DBAsyncWork work;
				work.queryFunc = [szP1, szP2, szP3, pRet](CAppDB& db) {
					*pRet = db.ChangePassword(szP1.c_str(), szP2.c_str(), szP3.c_str());
				};
				work.resultFunc = [this, pRet, dwFlag, IsClientValid]() {
					if (!IsClientValid()) return;
					SendMsg(dwFlag, DM_CHANGEPASSWORD, (WORD)*pRet, 0, 0, nullptr, 0);
				};
				queryPool.SubmitWork(std::move(work));
			}
			else
			{
				DBAsyncWork work;
				work.queryFunc = [pRet](CAppDB&) { /* no-op */ };
				work.resultFunc = [this, pRet, dwFlag, IsClientValid]() {
					if (!IsClientValid()) return;
					SendMsg(dwFlag, DM_CHANGEPASSWORD, (WORD)*pRet, 0, 0, nullptr, 0);
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			char* Params[4];
			int nParam = SearchParam(pMsg->data, Params, 4, "\t");
			if (nParam != 3)
			{
				pMsg->wParam[0] = SE_LOGIN_ACCOUNTNOTEXIST;
			}
			else
			{
				pMsg->wParam[0] = static_cast<WORD>(appDB.ChangePassword(Params[0], Params[1], Params[2]));
			}
			SendMsg(pMsg, 0);
		}
	}
	break;
	case DM_CHECKACCOUNT:
	{
		if (bAsync)
		{
			char* p = strchr(pMsg->data, '/');
			DWORD dwFlag = pMsg->dwFlag;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);
			std::string szAccount, szPassword;

			if (p != nullptr)
			{
				szAccount.assign(pMsg->data, p - pMsg->data);
				szPassword = p + 1;

				DBAsyncWork work;
				work.queryFunc = [szAccount, szPassword, pRet](CAppDB& db) {
					*pRet = db.CheckAccount(szAccount.c_str(), szPassword.c_str());
				};
				work.resultFunc = [this, pRet, dwFlag, szAccount, IsClientValid]() {
					if (!IsClientValid()) return;
					SendMsg(dwFlag, DM_CHECKACCOUNT, (WORD)*pRet, 0, 0, (LPVOID)szAccount.c_str(), static_cast<int>(szAccount.size()));
				};
				queryPool.SubmitWork(std::move(work));
			}
			else
			{
				DBAsyncWork work;
				work.queryFunc = [pRet](CAppDB&) { /* no-op */ };
				work.resultFunc = [this, pRet, dwFlag, IsClientValid]() {
					if (!IsClientValid()) return;
					SendMsg(dwFlag, DM_CHECKACCOUNT, (WORD)*pRet, 0, 0, nullptr, 0);
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			char* p = strchr(pMsg->data, '/');
			pMsg->wParam[0] = SE_FAIL;
			if (p != nullptr)
			{
				*p = 0;
				pMsg->wParam[0] = static_cast<WORD>(appDB.CheckAccount(pMsg->data, p + 1));
			}
			SendMsg(pMsg, static_cast<int>(strlen(pMsg->data)));
		}
	}
	break;
	case DM_CHECKACCOUNTEXIST:
	{
		if (bAsync)
		{
			std::string szAccount(pMsg->data);
			DWORD dwFlag = pMsg->dwFlag;

			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [szAccount, pRet](CAppDB& db) {
				*pRet = db.CheckAccountExist(szAccount.c_str());
			};
			work.resultFunc = [this, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_CHECKACCOUNTEXIST, (WORD)*pRet, 0, 0, nullptr, 0);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			pMsg->wParam[0] = static_cast<WORD>(appDB.CheckAccountExist(pMsg->data));
			SendMsg(pMsg, 0);
		}
	}
	break;
	case DM_QUERYCOMMUNITY:
	{
		if (bAsync)
		{
			DWORD dwOwner = *(DWORD*)pMsg->data;
			DWORD dwFlag = pMsg->dwFlag;
			WORD w1 = pMsg->wParam[0], w2 = pMsg->wParam[1], w3 = pMsg->wParam[2];

			auto pText = std::make_shared<std::array<char, 4096>>();
			pText->fill(0);
			auto pSize = std::make_shared<int>(4096);
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, pText, pSize, pRet](CAppDB& db) {
				*pRet = db.QueryCommunity(dwOwner, pText->data(), *pSize);
			};
			work.resultFunc = [this, pText, pSize, pRet, dwFlag, w1, w2, w3, IsClientValid]() {
				if (!IsClientValid()) return;
				if (*pRet == SE_OK)
				{
					SendMsg(dwFlag, DM_QUERYCOMMUNITY, w1, w2, w3, (LPVOID)pText->data(), *pSize);
				}
				else
				{
					SendMsg(dwFlag, DM_QUERYCOMMUNITY, w1, w2, w3, (LPVOID)"", 1);
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			char szText[4096];
			int iSize = 4096;
			DWORD dwOwner = *(DWORD*)pMsg->data;
			if (appDB.QueryCommunity(dwOwner, szText, iSize) == SE_OK)
			{
				SendMsg(pMsg->dwFlag, DM_QUERYCOMMUNITY, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2],
					(LPVOID)szText, iSize);
			}
			else
			{
				SendMsg(pMsg->dwFlag, DM_QUERYCOMMUNITY, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2],
					(LPVOID)"", 1);
			}
		}
	}
	break;
	// ====================================================================
	// 写入类操作 — fire-and-forget（不需要发送响应）
	// ====================================================================
	case DM_UPDATETASKINFO:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			TASKINFO info;
			memcpy(&info, pMsg->data, sizeof(TASKINFO));

			DBAsyncWork work;
			work.queryFunc = [dwOwner, info](CAppDB& db) {
				db.UpdateTaskInfo(dwOwner, const_cast<TASKINFO*>(&info));
			};
			// 无 resultFunc — fire-and-forget
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpdateTaskInfo(pMsg->dwFlag, (TASKINFO*)pMsg->data);
		}
	}
	break;
	case DM_UPDATEFENGHAO:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			FenghaoInfo info;
			memcpy(&info, pMsg->data, sizeof(FenghaoInfo));

			DBAsyncWork work;
			work.queryFunc = [dwOwner, info](CAppDB& db) {
				db.UpdateFengHaoInfo(dwOwner, const_cast<FenghaoInfo*>(&info));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpdateFengHaoInfo(pMsg->dwFlag, (FenghaoInfo*)pMsg->data);
		}
	}
	break;
	case DM_DELETEMAGIC:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			WORD wId = pMsg->wParam[0];

			DBAsyncWork work;
			work.queryFunc = [dwOwner, wId](CAppDB& db) {
				db.DeleteMagic(dwOwner, wId);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.DeleteMagic(pMsg->dwFlag, pMsg->wParam[0]);
		}
	}
	break;
	case DM_ADDCREDIT:
	{
		if (bAsync)
		{
			UINT n = *(UINT*)pMsg->data;
			std::string szName(pMsg->data + sizeof(UINT));

			DBAsyncWork work;
			work.queryFunc = [szName, n](CAppDB& db) {
				db.AddCredit(szName.c_str(), n);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			UINT n = *(UINT*)pMsg->data;
			char* pszName = pMsg->data + sizeof(UINT);
			appDB.AddCredit(pszName, n);
		}
	}
	break;
	case DM_UPDATECHARGUILDNAME:
	{
		if (bAsync)
		{
			std::string szName(pMsg->data);
			std::string szGuildName(pMsg->data + 20);

			DBAsyncWork work;
			work.queryFunc = [szName, szGuildName](CAppDB& db) {
				db.UpdateCharacterGuildName(szName.c_str(), szGuildName.c_str());
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			char* pszName = pMsg->data;
			char* pspszGuildName = pMsg->data + 20;
			appDB.UpdateCharacterGuildName(pszName, pspszGuildName);
		}
	}
	break;
	case DM_RESTOREGUILDNAME:
	{
		if (bAsync)
		{
			xStringsExtracter<2> ss(pMsg->data);
			if (ss.getCount() == 2)
			{
				std::string s1(ss[0]), s2(ss[1]);
				DBAsyncWork work;
				work.queryFunc = [s1, s2](CAppDB& db) {
					db.RestoreGuild(s1.c_str(), s2.c_str());
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			xStringsExtracter<2> ss(pMsg->data);
			if (ss.getCount() == 2)
			{
				appDB.RestoreGuild(ss[0], ss[1]);
			}
		}
	}
	break;
	case DM_UPGRADEITEM:
	{
		if (bAsync)
		{
			DWORD dwMakeIndex = pMsg->dwFlag;
			DWORD dwUpgrade = *(DWORD*)&pMsg->wParam[0];

			DBAsyncWork work;
			work.queryFunc = [dwMakeIndex, dwUpgrade](CAppDB& db) {
				db.UpgradeItem(dwMakeIndex, dwUpgrade);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpgradeItem(pMsg->dwFlag, *(DWORD*)&pMsg->wParam[0]);
		}
	}
	break;
	case DM_CACHECHARDATA:
	{
		// 空操作，无需异步
	}
	break;
	case DM_UPDATEITEMPOSEX:
	{
		if (bAsync)
		{
			BYTE btFlag = static_cast<BYTE>(pMsg->wParam[1]);
			WORD wCount = pMsg->wParam[0];
			auto pPositions = std::make_shared<std::vector<BAGITEMPOS>>(wCount);
			memcpy(pPositions->data(), pMsg->data, sizeof(BAGITEMPOS) * wCount);

			DBAsyncWork work;
			work.queryFunc = [btFlag, wCount, pPositions](CAppDB& db) {
				db.UpdateItemPosEx(btFlag, wCount, pPositions->data());
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpdateItemPosEx(static_cast<BYTE>(pMsg->wParam[1]), pMsg->wParam[0], (BAGITEMPOS*)pMsg->data);
		}
	}
	break;
	case DM_UPDATEMAGIC:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			int count = pMsg->wParam[2];
			auto pMagics = std::make_shared<std::vector<MAGICDB>>(count);
			memcpy(pMagics->data(), pMsg->data, sizeof(MAGICDB) * count);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, count, pMagics](CAppDB& db) {
				db.UpdateMagic(dwOwner, pMagics->data(), count);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpdateMagic(pMsg->dwFlag, (MAGICDB*)pMsg->data, pMsg->wParam[2]);
		}
	}
	break;
	case DM_UPDATEITEMS:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			BYTE btFlag = static_cast<BYTE>(pMsg->wParam[1]);
			int nCount = pMsg->wParam[0];
			auto pItems = std::make_shared<std::vector<DBITEM>>(nCount);
			memcpy(pItems->data(), pMsg->data, sizeof(DBITEM) * nCount);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, btFlag, nCount, pItems](CAppDB& db) {
				for (int i = 0; i < nCount; i++)
				{
					if (pItems->at(i).item.dwMakeIndex & 0x80000000 && pItems->at(i).btFlag == 0)
					{
						db.CreateItemEx(dwOwner, btFlag, pItems->at(i).pos, &pItems->at(i).item);
					}
					else
					{
						db.UpdateItem(dwOwner, btFlag, pItems->at(i).pos, &pItems->at(i).item);
					}
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			DBITEM* pItem = (DBITEM*)pMsg->data;
			for (int i = 0; i < pMsg->wParam[0]; i++)
			{
				if (pItem[i].item.dwMakeIndex & 0x80000000 && pItem[i].btFlag == 0)
				{
					appDB.CreateItemEx(pMsg->dwFlag, static_cast<BYTE>(pMsg->wParam[1]), pItem[i].pos, &pItem[i].item);
				}
				else
				{
					appDB.UpdateItem(pMsg->dwFlag, static_cast<BYTE>(pMsg->wParam[1]), pItem[i].pos, &pItem[i].item);
				}
			}
		}
	}
	break;
	case DM_DELETEITEM:
	{
		if (bAsync)
		{
			DWORD dwItemIndex = pMsg->dwFlag;

			DBAsyncWork work;
			work.queryFunc = [dwItemIndex](CAppDB& db) {
				db.DeleteItem(dwItemIndex);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.DeleteItem(pMsg->dwFlag);
		}
	}
	break;
	case DM_UPDATEITEMOWNER:
	{
		if (bAsync)
		{
			DWORD dwItemIndex = pMsg->dwFlag;
			DWORD dwId = *(DWORD*)pMsg->data;
			BYTE btFlag = pMsg->wParam[0] & 0xff;
			WORD wPos = pMsg->wParam[1];

			DBAsyncWork work;
			work.queryFunc = [dwItemIndex, dwId, btFlag, wPos](CAppDB& db) {
				db.UpdateItemOwner(dwItemIndex, dwId, btFlag, wPos);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			DWORD dwId = *(DWORD*)pMsg->data;
			appDB.UpdateItemOwner(pMsg->dwFlag, dwId, pMsg->wParam[0] & 0xff, pMsg->wParam[1]);
		}
	}
	break;
	case DM_UPDATEITEMPOS:
	{
		if (bAsync)
		{
			DWORD dwItemIndex = pMsg->dwFlag;
			BYTE btFlag = pMsg->wParam[0] & 0xff;
			WORD wPos = pMsg->wParam[1];

			DBAsyncWork work;
			work.queryFunc = [dwItemIndex, btFlag, wPos](CAppDB& db) {
				db.UpdateItemPos(dwItemIndex, btFlag, wPos);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.UpdateItemPos(pMsg->dwFlag, pMsg->wParam[0] & 0xff, pMsg->wParam[1]);
		}
	}
	break;
	case DM_UPDATEITEM:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			BYTE btFlag = pMsg->wParam[0] & 0xff;
			WORD wPos = pMsg->wParam[1];
			ITEM item;
			memcpy(&item, pMsg->data, sizeof(ITEM));

			DBAsyncWork work;
			work.queryFunc = [dwOwner, btFlag, wPos, item](CAppDB& db) {
				db.UpdateItem(dwOwner, btFlag, wPos, const_cast<ITEM*>(&item));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			ITEM* pItem = (ITEM*)pMsg->data;
			appDB.UpdateItem(pMsg->dwFlag, pMsg->wParam[0] & 0xff, pMsg->wParam[1], pItem);
		}
	}
	break;

	case DM_PUTCHARDBINFO:
	{
		if (bAsync)
		{
			CHARDBINFO info;
			memcpy(&info, pMsg->data, sizeof(CHARDBINFO));

			DBAsyncWork work;
			work.queryFunc = [info](CAppDB& db) {
				db.PutCharDBInfo(const_cast<CHARDBINFO*>(&info));
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			appDB.PutCharDBInfo((CHARDBINFO*)pMsg->data);
		}
	}
	break;
	case DM_UPDATECOMMUNITY:
	{
		if (bAsync)
		{
			DWORD dwOwner = pMsg->dwFlag;
			std::string szData(pMsg->data);

			DBAsyncWork work;
			work.queryFunc = [dwOwner, szData](CAppDB& db) {
				if (db.UpdateCommunity(dwOwner, szData.c_str()) != SE_OK)
				{
					PRINT(ERROR_RED, "更新社交信息出错～!\n");
				}
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			if (appDB.UpdateCommunity(pMsg->dwFlag, pMsg->data) != SE_OK)
				PRINT(ERROR_RED, "更新社交信息出错～!\n");
		}
	}
	break;
	case DM_BREAKMARRIAGE:
	{
		if (bAsync)
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				std::string s1(param[0]), s2(param[1]);
				DBAsyncWork work;
				work.queryFunc = [s1, s2](CAppDB& db) {
					if (db.DeleteMarriage(s1.c_str(), s2.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", s1.c_str(), s2.c_str());
					if (db.DeleteMarriage(s2.c_str(), s1.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", s2.c_str(), s1.c_str());
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				if (appDB.DeleteMarriage(param[0], param[1]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", param[0], param[1]);
				if (appDB.DeleteMarriage(param[1], param[0]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", param[1], param[0]);
			}
		}
	}
	break;
	case DM_BREAKMASTER:
	{
		if (bAsync)
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				std::string s1(param[0]), s2(param[1]);
				DBAsyncWork work;
				work.queryFunc = [s1, s2](CAppDB& db) {
					if (db.DeleteStudent(s1.c_str(), s2.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s(师父) 和 %s(徒弟) 的师徒关系出错～!\n", s1.c_str(), s2.c_str());
					if (db.DeleteTeacher(s2.c_str(), s1.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s(徒弟) 和 %s(师父) 的好友关系出错～!\n", s2.c_str(), s1.c_str());
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				if (appDB.DeleteStudent(param[0], param[1]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s(师父) 和 %s(徒弟) 的师徒关系出错～!\n", param[0], param[1]);
				if (appDB.DeleteTeacher(param[1], param[0]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s(徒弟) 和 %s(师父) 的好友关系出错～!\n", param[1], param[0]);
			}
		}
	}
	break;
	case DM_BREAKFRIEND:
	{
		if (bAsync)
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				std::string s1(param[0]), s2(param[1]);
				DBAsyncWork work;
				work.queryFunc = [s1, s2](CAppDB& db) {
					if (db.BreakFriend(s1.c_str(), s2.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", s1.c_str(), s2.c_str());
					if (db.BreakFriend(s2.c_str(), s1.c_str()) != SE_OK)
						PRINT(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", s2.c_str(), s1.c_str());
				};
				queryPool.SubmitWork(std::move(work));
			}
		}
		else
		{
			xStringsExpander<20> param(pMsg->data, '/');
			if (param.getCount() == 2)
			{
				if (appDB.BreakFriend(param[0], param[1]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", param[0], param[1]);
				if (appDB.BreakFriend(param[1], param[0]) != SE_OK)
					PRINT(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", param[1], param[0]);
			}
		}
	}
	break;
	// ====================================================================
	// 特殊操作
	// ====================================================================
	case DM_CHECKFREE:
	{
		// 无数据库操作，直接响应
		SendMsg(pMsg->dwFlag, DM_CHECKFREE, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]);
	}
	break;
	case DM_CREATEACCOUNT:
	{
		if (datasize < 226)
		{
			pMsg->wParam[0] = SE_REG_INVALIDACCOUNT;
			SendMsg(pMsg, 0);
			break;
		}

		// _tout 从 pMsg->data 中提取长度前缀字符串到局部 buffer
		CHAR szAccount[256];  CHAR szPassword[256];  CHAR szEMail[256];
		CHAR szQ1[256];       CHAR szA1[256];        CHAR szQ2[256];
		CHAR szA2[256];       CHAR szMobilePhoneNumber[256];
		CHAR szPhoneNumber[256];  CHAR szName[256];
		CHAR szBirthday[256]; CHAR szIdCard[256];
		_tout(szAccount, pMsg->data + RAI_ACCOUNT, 10);
		_tout(szPassword, pMsg->data + RAI_PASSWORD, 10);
		_tout(szName, pMsg->data + RAI_NAME, 20);
		_tout(szIdCard, pMsg->data + RAI_IDCARD, 20);
		_tout(szPhoneNumber, pMsg->data + RAI_PHONENUMBER, 12);
		_tout(szQ1, pMsg->data + RAI_Q1, 20);
		_tout(szA1, pMsg->data + RAI_A1, 20);
		_tout(szEMail, pMsg->data + RAI_MAIL, 40);
		_tout(szQ2, pMsg->data + RAI_Q2, 20);
		_tout(szA2, pMsg->data + RAI_A2, 20);
		_tout(szBirthday, pMsg->data + RAI_BIRTHDAY, 10);
		_tout(szMobilePhoneNumber, pMsg->data + RAI_MOBILEPHONENUMBER, 11);

		if (bAsync)
		{
			DWORD dwFlag = pMsg->dwFlag;
			auto pRet = std::make_shared<SERVER_ERROR>(SE_FAIL);

			// 注意：栈数组不能被 [=] 按值捕获（数组退化为指针，lambda 执行时栈已销毁）
			// 必须用 std::string 值捕获
			std::string sAccount(szAccount), sPassword(szPassword), sName(szName);
			std::string sBirthday(szBirthday), sQ1(szQ1), sA1(szA1), sQ2(szQ2), sA2(szA2);
			std::string sEMail(szEMail), sPhoneNumber(szPhoneNumber), sMobilePhoneNumber(szMobilePhoneNumber);
			std::string sIdCard(szIdCard);

			DBAsyncWork work;
			work.queryFunc = [sAccount, sPassword, sName, sBirthday,
				sQ1, sA1, sQ2, sA2, sEMail, sPhoneNumber, sMobilePhoneNumber, sIdCard, pRet](CAppDB& db) {
				*pRet = db.CreateAccount(sAccount.c_str(), sPassword.c_str(), sName.c_str(), sBirthday.c_str(),
					sQ1.c_str(), sA1.c_str(), sQ2.c_str(), sA2.c_str(), sEMail.c_str(),
					sPhoneNumber.c_str(), sMobilePhoneNumber.c_str(), sIdCard.c_str());
			};
			work.resultFunc = [this, pRet, dwFlag, IsClientValid]() {
				if (!IsClientValid()) return;
				SendMsg(dwFlag, DM_CREATEACCOUNT, (WORD)*pRet, 0, 0, nullptr, 0);
			};
			queryPool.SubmitWork(std::move(work));
		}
		else
		{
			pMsg->wParam[0] = static_cast<WORD>(appDB.CreateAccount(szAccount, szPassword, szName, szBirthday,
				szQ1, szA1, szQ2, szA2, szEMail,
				szPhoneNumber, szMobilePhoneNumber, szIdCard));
			SendMsg(pMsg, 0);
		}
	}
	break;
	}
}

VOID CClientObj::OnConnection()
{
	EnableBatchMode(TRUE);
}
