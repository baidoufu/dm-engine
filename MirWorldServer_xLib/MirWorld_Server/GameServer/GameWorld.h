#pragma once
//#include "scriptpage.h"
#include "localdefine.h"
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <array>

class CPhysicsMapMgr;
class CLogicMapMgr;
class CHumanPlayerMgr;
class CHumanPlayer;
class CMapObject;
class CItemManager;
class CMagicManager;
class CAliveObject;
class CNpcManager;
class CScriptObjectMgr;
class CScriptObject;

struct tagOBJECTPROCESS;
typedef struct tagSTRING_BUFFER
{
	std::array<char, 1024> szString{};
}STRING_BUFFER;

typedef xListHost<CMapObject> MAPOBJECT_LIST;

typedef struct firstlogin_item
{
	firstlogin_item()
	{
		FILLSELF(0);
	}
	char szItem[32];
	int	nCount;
	BYTE btJob;
	BYTE btSex;
	BOOL boBind;
	firstlogin_item* pNext;
}FIRSTLOGIN_ITEM;

typedef struct firstlogin_info
{
	firstlogin_info()
	{
		FILLSELF(0);
	}
	int	level;
	int nGameTime;
	DWORD dwGold;
	DWORD dwYuanBao;
	FIRSTLOGIN_ITEM* pItems;
}FIRSTLOGIN_INFO;

typedef struct linenotice
{
	CServerTimer timer;
	DWORD dwDelay = 0;
	std::array<char, 256> szWords = {};
	linenotice() : dwDelay(0)
	{
		szWords.fill(0);
	}
}LINENOTICE;

class CMonsterEx;

// 工作线程类
class CGameWorkerThread : public xThread
{
public:
	CGameWorkerThread() : xThread() {}
	VOID SetOwner(CGameWorld* pOwner) { m_pOwner = pOwner; }
protected:
	VOID Execute(LPVOID lpParam) override;
private:
	CGameWorld* m_pOwner = nullptr;
};

class CGameWorld : public xSingletonClass<CGameWorld>
{
	xListHost<CMonsterEx> m_xUpdateMonsterList; // 有目标的怪
	xListHost<CMonsterEx> m_xUpdateAutoMonsterList; // 闲散无目标的怪
	SRWLOCK m_rwMonsterLock = SRWLOCK_INIT; // 用于保护m_xUpdateMonsterList和m_xUpdateAutoMonsterList的读写锁
private:
	enum MonsterUpdateType { MUT_AUTO, MUT_ACTIVE };
	VOID UpdateMonster(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType);
	VOID UpdateMonsterParallel(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType, int nBatchSize);
	VOID UpdatePlayers();
public:
	UINT GetUpdateMonsterListCount() 
	{ 
		SRLock lock(m_rwMonsterLock);
		UINT count = m_xUpdateMonsterList.getCount() + m_xUpdateAutoMonsterList.getCount();
		return count;
	}
	CGameWorld(VOID);
	virtual ~CGameWorld(VOID);

	VOID AddUpdateMonster(CMonsterEx* pMonster);

	VOID ShowSandCityNpc();
	VOID HideSandCityNpc();

	VOID ShowNpc(UINT nMapId);
	VOID HideNpc(UINT nMapId);

	BOOL LoadServerConfig();
	VOID LoadSafeArea();
	VOID LoadStartPoint();
	VOID LoadNotice();
	VOID LoadClientKeyConfig();
	BOOL Init();

	VOID Update();

	//工作线程池
	VOID InitThreadPool();
	VOID ShutdownThreadPool();
	VOID AsyncUpdateWorker();
	VOID ProcessAsyncUpdateResults();
	VOID SubmitAsyncTask(std::function<VOID()> task);

	BOOL AddMapObject(CMapObject* pObject);
	BOOL RemoveMapObject(CMapObject* pObject);

	VOID CleanAllMonsters();

	OBJECTPROCESS* AllocProcess(const char* pszString = nullptr)
	{
		OBJECTPROCESS* p = m_ObjProcesses.newObject();
		if (p == nullptr)return p;
		memset(p, 0, sizeof(*p));
		if (pszString != nullptr)
		{
			STRING_BUFFER* psb = m_StringBufferPool.newObject();
			if (psb == nullptr)
			{
				m_ObjProcesses.deleteObject(p);
				return nullptr;
			}
			o_strncpy(psb->szString.data(), pszString, 1023);
			p->pszParam = psb->szString.data();
		}
		return p;
	}

	VOID FreeProcess(OBJECTPROCESS* p)
	{
		if (p->pszParam != nullptr)
		{
			STRING_BUFFER* psb = (STRING_BUFFER*)p->pszParam;
			m_StringBufferPool.deleteObject(psb);
			p->pszParam = nullptr;
		}
		m_ObjProcesses.deleteObject(p);
	}

	const char* GetNotice() { return m_pNotice.get(); }

	BOOL GetStartPoint(int pro, int& mapid, int& x, int& y);

	HUMANDATADESC* GetHumanDataDesc(int pro, int level)
	{
		if (pro < 0 || pro >= PRO_MAX)return nullptr;
		if (level <= 0 || level > MAX_LEVEL)return nullptr;
		return &m_HumanData[pro][level - 1];
	}
	FIRSTLOGIN_INFO* GetFirstLoginInfo() { return &m_FirstLoginInfo; }

	CAliveObject* GetAliveObjectById(UINT id);
	const char* GetName(e_nameindex index) { if (index < 0 || index >= ENI_MAX)return "<nullptr>"; return m_pNameList[index]; }
	DWORD GetVar(e_varindex index)const { if (index < 0 || index >= EVI_MAX)return 0; return m_VarList[index]; }
	BOOL AddGlobeProcess(e_process ident, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0,
		DWORD dwDelay = 0, int repeattimes = 0, const char* pszString = nullptr);
public:
	VOID PostSystemMessage(const char* pszWords, DWORD dwDelay = 0);
	VOID PostSystemMessage(DWORD dwColor, const char* pszWords, DWORD dwDelay = 0);
	VOID PostSystem10Message(const char* pszWords, DWORD dwDelay = 0);

	USERMAGIC* AllocUserMagic();
	VOID FreeUserMagic(USERMAGIC* p);

	BOOL GetBornPoint(int pro, int& mapid, int& x, int& y, char* pszName);
	BOOL GetStartPoint(const char* pszName, int& mapid, int& x, int& y);

	START_POINT* GetBornPoint(int pro)const;
	START_POINT* GetStartPoint(DWORD dwIndex)
	{
		if (m_pStartPoints == nullptr)return nullptr;
		if (dwIndex >= (DWORD)m_iStartPointCount)
			return nullptr;
		return &m_pStartPoints[dwIndex];
	}

	START_POINT* GetStartPoint(const char* pszName)
	{
		return (START_POINT*)m_StartPointHash.HGet(pszName);
	}
	FLOAT GetExpFactor()const { return m_fExpFactor; }
	VOID SetExpFactor(FLOAT fFactor) { m_fExpFactor = fFactor; }
	DWORD GetChannelWaitTime(e_chatchannel channel)const
	{
		if (channel < 0 || channel >= CCH_MAX)channel = (e_chatchannel)0;
		return m_dwChannelTime[channel] * 1000;
	}
	DWORD GetUpdateKey()const { return m_dwUpdateKey; }
	VOID SetUpdateKey(DWORD dwKey) { m_dwUpdateKey = dwKey; }
	BOOL CanSaveToDB() { if (m_DBUpdateTimer.IsTimeOut(GetVar(EVI_DBUPDATEDELAY)))return TRUE; return FALSE; }
	VOID UpdateDBUpdateTimer() { m_DBUpdateTimer.Savetime(); }
	ClientKeyState* GetClientKeyConfig() { return m_ClientKeyConfig.data(); }
private:
	FLOAT m_fExpFactor;
	BOOL GetValidPointFromStartPoint(START_POINT* pPoint, int& map, int& x, int& y, int depth = 0);
	std::array<char*, ENI_MAX> m_pNameList;
	std::array<DWORD, EVI_MAX> m_VarList;

	std::array<DWORD, CCH_MAX> m_dwChannelTime;
	DWORD m_dwUpdateKey;

	BOOL LoadHumanDataDesc(int pro, const char* pszFile);
	std::unique_ptr<char[]> m_pNotice;
	std::array<std::unique_ptr<MAPOBJECT_LIST>, OBJ_MAX> m_pObjectList;
	SRWLOCK m_rwObjectListLock = SRWLOCK_INIT; // 保护 m_pObjectList 的读写锁
	xMpscQueue<tagOBJECTPROCESS, 64> m_xGlobeProcessQueue;
	xObjectPool	<tagOBJECTPROCESS> m_ObjProcesses;
	xObjectPool <STRING_BUFFER> m_StringBufferPool;
	xObjectPool<USERMAGIC> m_UserMagicPool;
	std::array<std::unique_ptr<START_POINT*[]>, 3> m_BornPoints;
	std::array<int, 3> m_iBornPointCount;
	CSettingFile m_sfServer;
	std::array<std::array<HUMANDATADESC, MAX_LEVEL>, PRO_MAX> m_HumanData;
	FIRSTLOGIN_INFO m_FirstLoginInfo;
	CServerTimer m_LineNoticeTimer;
	std::unique_ptr<START_POINT[]> m_pStartPoints;
	int	m_iStartPointCount;
	CNameHash m_StartPointHash;
	std::array<LINENOTICE, 1024> m_NoticeLines;
	int	m_iNoticeLines;
	std::atomic<int> m_iNoticePtr;
	CServerTimer m_DBUpdateTimer;

	std::array<ClientKeyState, 100> m_ClientKeyConfig{}; // 自定义快捷键

	std::unique_ptr<CScriptObject> m_pSystemScriptObject; // 系统脚本对象

	// 工作线程池相关
	std::vector<std::unique_ptr<CGameWorkerThread>> m_WorkerThreads;
	std::queue<std::function<VOID()>> m_WorkerTaskQueue;
	std::mutex m_WorkerQueueMutex;
	std::condition_variable m_WorkerQueueCV;
	std::atomic<bool> m_bWorkerRunning;
	std::atomic<bool> m_bAsyncUpdateReady;

	// 异步通告结果缓冲
	struct AsyncNoticeResult {
		char szText[1024];
		int len;
		DWORD dwDelay;
	};
	std::queue<AsyncNoticeResult> m_AsyncNoticeQueue;
	std::mutex m_AsyncNoticeMutex;
};
