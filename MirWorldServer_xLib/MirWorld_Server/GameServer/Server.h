#pragma once
#include "gsclientobj.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <memory>

constexpr int UPDATE_GLOBE_PROCESS_COUNT = 32;
constexpr DWORD ENTER_GAME_TIMEOUT = 60 * 1000; // 进入游戏条目超时时间60秒

class CGameWorld;

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
	VOID OnSCMsg(PMIRMSG pMsg, int datasize);
	VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize);
	SERVER_ERROR AddEnterAccount(UINT nLoginId, UINT nSelCharId, const char* pszAccount, const char* pszName, WORD wIndex);
	BOOL GetEnterInfo(UINT nLoginId, UINT nSelCharId, const char* pszAccount, ENTERGAMESERVER& enterinfo);
	VOID CleanupExpiredEnterInfos();
	VOID KickAll();

	VOID OnTerminated(BOOL bExcepted = FALSE);
private:
	// 主线程处理：消费活跃连接队列的数据包
	VOID ProcessClientPackets();
	// 主线程处理：刷新所有连接的批量发送缓冲区
	VOID FlushAllBatchBuffers();
protected:
	BOOL m_bWillClose;
	CIntHash<1024> m_Inthash;
	CIndexListEx<ENTERGAMESERVER> m_EnterObjects;
	CGameWorld* m_pGameWorld;
};

// 测试函数内运行时间（线程安全版本）
//使用案例：xTestTimer time("UpdateMonster");
class xTestTimer
{
public:
	xTestTimer(const char* name = "Timer", int batchIndex = -1) : m_Name(name)
	{
		m_ThreadId = std::this_thread::get_id();
		m_STartTimepoint = std::chrono::high_resolution_clock::now();
	}
	~xTestTimer()
	{
		Stop();
	}
	VOID Stop() const
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_STartTimepoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration * 0.001;
		std::thread::id endThreadId = std::this_thread::get_id();
		std::ostringstream oss;
		oss << endThreadId;
		const char* sameThread = (m_ThreadId == endThreadId) ? "相同" : "不同";
		PRINT(SUCCESS_GREEN, "[%s][线程:%s][%s] %.3f 毫秒\n", m_Name, oss.str().c_str(), sameThread, ms);
	}
private:
	const char* m_Name;
	mutable std::thread::id m_ThreadId;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_STartTimepoint;
};