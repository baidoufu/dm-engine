#include "StdAfx.h"
#include ".\server.h"
#include "gameworld.h"
#include "NpcManager.h"
#include "ScriptNpc.h"
#include "LogicMapMgr.h"
#include "ItemManager.h"
#include "DownItemMgr.h"
#include "magicmanager.h"
#include "monstermanagerex.h"
#include <vector>
#include <memory>

CServer::CServer(VOID)
{
	m_pGameWorld = nullptr;
	m_bWillClose = FALSE;
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
	int maxconnection = s.GetInteger(m_strServerName.c_str(), "MaxConnection", 100);
	create(maxconnection);
	m_EnterObjects.Create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);
	m_pGameWorld = CGameWorld::GetInstance();
	return m_pGameWorld->Init();
}

VOID CServer::CleanServer()
{
}

VOID CServer::Update()
{
	const DWORD dwUpdateKey = GetUpdateKey();
	// ===== 消费客户端数据包（在主线程上下文执行业务逻辑） =====
	// 数据包由IOCP工作线程自动填充到 m_xPacketQueue
	// 主线程只负责从队列中取出并执行业务逻辑
	ProcessClientPackets();
	// ===== 更新游戏世界（纯游戏逻辑） =====
	m_pGameWorld->SetUpdateKey(dwUpdateKey);
	m_pGameWorld->Update();
	// ===== 帧末：刷新所有连接的批量发送缓冲区 =====
	// WSASend 本身是线程安全的，主线程可以安全调用
	FlushAllBatchBuffers();
	// ===== SC/DB重连和状态维护（保留在主线程） =====
	if ((dwUpdateKey & 1) == 0)
		UpdateDBServer(); // 奇数帧：处理数据中心消息
	else
		UpdateSCServer(); // 偶数帧：处理服务中心消息
	// ===== 定期清理过期的进入游戏条目 =====
	static DWORD s_cleanupCounter = 0;
	if (++s_cleanupCounter >= 200)
	{
		s_cleanupCounter = 0;
		CleanupExpiredEnterInfos();
	}
}

VOID CServer::ProcessClientPackets()
{
	// 只处理活跃队列中有待处理包的连接
	// IOCP 工作线程收包时已通过 CAS 推入活跃队列，主线程仅消费
	auto& activeQueue = m_xIocpServer.GetActiveClientQueue();
	CClientObj* pObject = nullptr;
	while (pObject = (CClientObj*)activeQueue.pop())
	{
		// 防御ABA：对象被 DeleteClientObject 清理后 m_Id==0，跳过（槽位复用前）
		// 槽位复用后 m_Id!=0，配合 xClientObject::Clean() 中 m_bInActiveList 重置，
		// 可确保新客户端从干净的初始状态启动
		if (pObject->getId() == 0) continue;
		// 重置活跃标记（允许后续收包再次推入）
		pObject->m_bInActiveList.store(FALSE, std::memory_order_release);
		if (!pObject->IsConnected()) continue;
		while (pObject->GetPacketQueueCount() > 0)
		{
			pObject->Update(); // 取一个包 → OnDataPacket → 解析 → 业务处理
		}
		// 如果还有剩余包，重新推入活跃队列
		if (pObject->GetPacketQueueCount() > 0)
		{
			// exchange(TRUE) 原子地设置 TRUE 并返回旧值：
			// 旧值==FALSE → 首次获取，推入队列；旧值==TRUE → 已在队列中，跳过
			if (!pObject->m_bInActiveList.exchange(TRUE, std::memory_order_acq_rel))
				m_xIocpServer.PushActiveClientQueue(pObject);
		}
	}
}

VOID CServer::FlushAllBatchBuffers()
{
	// 遍历所有连接，刷新批量发送缓冲区 + 心跳检测
	// GameServer 的 ProcessClientPackets 只对活跃连接调 Update()
	// 空闲连接需要在这里补上心跳检测
	// 使用 ForEach 替代 First/Next：整个遍历期间持有共享读锁，防止 IOCP 工作线程
	// 通过 NewClientObject→DeleteClientObject 并发修改链表导致迭代器失效
	m_ObjectPool.ForEach([](CClientObj* pFlushObj)
	{
		if (pFlushObj->IsConnected())
		{
			if (pFlushObj->IsBatchMode())
				pFlushObj->FlushMsgQueue();
			// 空闲连接的心跳检测（活跃连接已在 ProcessClientPackets 中通过 Update() 检测）
			if (pFlushObj->GetPacketQueueCount() == 0)
				pFlushObj->UpdateStarPing();
		}
	});
}

VOID CServer::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	switch (wCmd)
	{
	case MAS_ENTERGAMESERVER:
	{
		ENTERGAMESERVER* pEnterInfo = (ENTERGAMESERVER*)pszData;
		UINT id = pEnterInfo->nClientId;
		if (m_EnterObjects.GetFreeCount() == 0)
			pEnterInfo->result = SE_SERVERFULL; // 游戏服务器人数已满
		else
			pEnterInfo->result = AddEnterAccount(pEnterInfo->nLoginId, pEnterInfo->nSelCharId, pEnterInfo->szAccount.data(), pEnterInfo->szName.data(), wIndex);
		m_SCClientObj.SendMsgAcrossServer(id, MAS_ENTERGAMESERVER, MST_SINGLE, wIndex, pszData, datasize);
	}
	break;
	}
}

VOID CServer::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_FINDSERVER:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			SERVERADDR* pAddr = (SERVERADDR*)pMsg->data;
			PRINT(SUCCESS_GREEN, "找到了服务器地址 %s:%d\n", pAddr->addr, pAddr->nPort);
		}
		else
			PRINT(ERROR_RED, "没有找到服务器 %s 的地址!\n", pMsg->data);
	}
	break;
	default:
		CBaseServer::OnSCMsg(pMsg, datasize);
		break;
	}
}

SERVER_ERROR CServer::AddEnterAccount(UINT nLoginId, UINT nSelCharId, const char* pszAccount, const char* pszName, WORD wIndex)
{
	ENTERGAMESERVER* pEnterInfo = nullptr;
	UINT id = m_EnterObjects.New(&pEnterInfo);
	if (id == 0 || pEnterInfo == nullptr)return SE_SERVERFULL;
	if (!m_Inthash.HAdd(nLoginId, id))
	{
		m_EnterObjects.Del(id);
		return SE_SERVERFULL;
	}
	pEnterInfo->nClientId = id;
	pEnterInfo->nLoginId = nLoginId;
	pEnterInfo->nSelCharId = nSelCharId;
	pEnterInfo->dwEnterTime = CFrameTime::GetFrameTime();
	pEnterInfo->nListId = id;
	strncpy(pEnterInfo->szAccount.data(), pszAccount, 10);
	o_strncpy(pEnterInfo->szName.data(), pszName, 20);
	pEnterInfo->szAccount[10] = 0;
	pEnterInfo->dwSelectCharServerId = wIndex;
	return SE_OK;
}

BOOL CServer::GetEnterInfo(UINT nLoginId, UINT nSelCharId, const char* pszAccount, ENTERGAMESERVER& enterinfo)
{
	UINT id = (UINT)m_Inthash.HGet(nLoginId);
	if (id == 0)return FALSE;
	ENTERGAMESERVER* pEnterInfo = m_EnterObjects.Get(id);
	if (pEnterInfo == nullptr)return FALSE;
	if (pEnterInfo->nLoginId != nLoginId || pEnterInfo->nSelCharId != nSelCharId)return FALSE;
	enterinfo = *pEnterInfo;
	m_EnterObjects.Del(id);
	m_Inthash.HDel(nLoginId);
	return TRUE;
}

VOID CServer::CleanupExpiredEnterInfos()
{
	DWORD dwNow = CFrameTime::GetFrameTime();
	std::vector<UINT> expiredIds;
	m_EnterObjects.ForEach([&](ENTERGAMESERVER* pInfo) {
		if (pInfo != nullptr && (dwNow - pInfo->dwEnterTime) > ENTER_GAME_TIMEOUT)
		{
			expiredIds.push_back(pInfo->nListId);
		}
	});
	for (UINT id : expiredIds)
	{
		ENTERGAMESERVER* pInfo = m_EnterObjects.Get(id);
		if (pInfo != nullptr)
		{
			m_Inthash.HDel(pInfo->nLoginId);
			m_EnterObjects.Del(id);
		}
	}
}

VOID CServer::OnInput(const char* pString)
{
	std::array<char, 256> szLine{};
	o_strncpy(szLine.data(), pString, 250);
	xStringsExtracter<16> cmd(szLine.data(), " \t,", " \t");
	if (static_cast<int>(cmd.getCount()) > 0)
	{
		if (_stricmp(cmd[0], "reloadserverconfig") == 0)
		{
			CGameWorld::GetInstance()->LoadServerConfig();
			PRINT(SUCCESS_GREEN, "服务器配置文件已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloaditem") == 0)
		{
			CItemManager::GetInstance()->ClearItemData();
			CItemManager::GetInstance()->Load(".\\Data\\Config\\BaseItem.csv");
			PRINT(SUCCESS_GREEN, "物品配置文件BaseItem.csv已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloadmonster") == 0)
		{
			CMonsterManagerEx::GetInstance()->ClearMonsterData();
			CMonsterManagerEx::GetInstance()->LoadMonsters(".\\Data\\Monsters");
			PRINT(SUCCESS_GREEN, "怪物配置Monsters下的所有文件已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloadskill") == 0) {
			CMagicManager::GetInstance()->ClearMagicData();
			CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
			CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv");
			CMagicManager::GetInstance()->LoadMaigcskill(".\\Data\\Config\\MagicSkill.xml");
			// 重新加载技能数据后, 需要更新所有在线玩家的技能指针
			CMagicManager::GetInstance()->ReloadAllPlayerSkills();
			PRINT(SUCCESS_GREEN, "技能配置文件BaseMagic.csv、MagicExt.csv、MagicSkill.xml已重新加载!\n");
		}
	}
	CBaseServer::OnInput(pString);
}

VOID CServer::KickAll()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Disconnect(2000);
		pObject = m_ObjectPool.Next();
	}
}

#include "humanplayermgr.h"
#include "humanplayer.h"
#include "guildmanagerex.h"
#include "sandcity.h"
#include "simpledbconnection.h"

VOID CServer::OnTerminated(BOOL bExcepted)
{
	// 直接使用当前线程（主线程/终端）保存数据, 不创建新线程
	PRINT(STRING_GREEN, "开始存储行会信息...\n");
	CGuildManagerEx::GetInstance()->SaveAll();

	PRINT(STRING_GREEN, "开始存储沙城信息...\n");
	CSandCity::GetInstance()->Save();

	PRINT(STRING_GREEN, "开始存储角色信息...\n");
	CIndexListEx<CHumanPlayer>* pList = CHumanPlayerMgr::GetInstance()->GetPlayerList();
	if (pList == nullptr || pList->GetCount() == 0)
	{
		PRINT(STRING_GREEN, "没有玩家在线, 无需存储.\n");
		return;
	}

	// 获取DBServer地址
	SERVERADDR* pAddr = GetDBAddr(0);
	if (pAddr == nullptr)
	{
		PRINT(ERROR_RED, "无法取得DB服务器的地址, 无法完成数据备份.\n");
		return;
	}

	// 创建临时DB连接（同步方式）
	CSimpleDBConnection dbConn;
	if (!dbConn.Connect(pAddr->addr.data(), pAddr->nPort))
	{
		PRINT(ERROR_RED, "无法连接到数据库服务 %s:%u, 数据备份失败.\n", pAddr->addr.data(), pAddr->nPort);
		return;
	}

	PRINT(SUCCESS_GREEN, "连接到数据库服务, 开始数据备份\n");

	CHumanPlayer* pPlayer = pList->First();
	int nSavedCount = 0;

	// 先批量发送所有玩家数据，再统一等待确认，避免逐个等待导致超时
	std::vector<CHumanPlayer*> players;
	while (pPlayer)
	{
		players.push_back(pPlayer);
		pPlayer = pList->Next();
	}

	// 批量发送阶段
	for (auto* p : players)
	{
		p->SaveVars();
		xPacketPool::ScopedPacket msgpacket(1024 * 64);
		p->GetDBInfoPacket(*msgpacket);
		int size = msgpacket->getsize();
		if (size > 0)
		{
			dbConn.Send((LPVOID)msgpacket->getbuf(), size);
		}
		nSavedCount++;
	}

	// 统一等待阶段：循环Update处理所有响应，最多等待10秒
	PRINT(STRING_GREEN, "等待数据库确认所有保存操作...\n");
	DWORD dwWaitStart = GetSteadyTimeMS(); // 使用系统时间而非帧时间（主线程已停止更新帧时间）
	while (GetSteadyTimeMS() - dwWaitStart < 10000)
	{
		dbConn.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	for (auto* p : players)
	{
		PRINT(STRING_GREEN, "%s 角色信息已经保存到数据库\n", p->GetName());
	}
	// 最终刷新：确保所有发送缓冲区数据已发出
	DWORD dwFinalStart = GetSteadyTimeMS();
	while (GetSteadyTimeMS() - dwFinalStart < 2000)
	{
		dbConn.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	PRINT(SUCCESS_GREEN, "数据已备份完毕, 共保存 %d 个角色.\n", nSavedCount);
}