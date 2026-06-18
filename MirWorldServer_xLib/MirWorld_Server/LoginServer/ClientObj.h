#pragma once
#include <array>
#include <unordered_map>
#include <string>
#include <mutex>

// 全局账号登录失败计数器，用于防暴力破解
class CAccountFailCounter
{
public:
	static CAccountFailCounter& GetInstance()
	{
		static CAccountFailCounter instance;
		return instance;
	}
	// 增加指定账号的失败次数，返回当前累计次数
	UINT Increment(const char* pszAccount)
	{
		if (pszAccount == nullptr || pszAccount[0] == 0) return 0;
		std::lock_guard<std::mutex> lock(m_mutex);
		UINT& count = m_failCount[pszAccount];
		count++;
		return count;
	}
	// 检查指定账号是否被锁定（失败次数超过阈值）
	BOOL IsLocked(const char* pszAccount, UINT nThreshold = 10) const
	{
		if (pszAccount == nullptr || pszAccount[0] == 0) return FALSE;
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_failCount.find(pszAccount);
		return (it != m_failCount.end() && it->second >= nThreshold);
	}
	// 清除指定账号的失败计数（登录成功时调用）
	void Reset(const char* pszAccount)
	{
		if (pszAccount == nullptr || pszAccount[0] == 0) return;
		std::lock_guard<std::mutex> lock(m_mutex);
		m_failCount.erase(pszAccount);
	}
private:
	CAccountFailCounter() = default;
	mutable std::mutex m_mutex;
	std::unordered_map<std::string, UINT> m_failCount;
};

class CClientObj :
	public CClientObject
{
public:
	CClientObj(VOID);
	virtual ~CClientObj(VOID);
	VOID Clean();
	VOID Update();
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	VOID OnDBMsg(PMIRMSG pMsg, int datasize);
	VOID OnSCMsg(PMIRMSG pMsg, int datasize);
	VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize);
	VOID OnConnection();
private:
	VOID SendLoginSuccess(UINT nLoginId);
	VOID SendLoginFail(DWORD error);
	VOID SendSelectServerOk();
	FINDSERVER_RESULT m_SelectCharServer;
	std::array<CHAR, 20> m_szAccount;
	UINT m_nLid{ 0 };
	UINT m_nSid{ 0 };
	UINT m_nFailCount{ 0 };
	CServerTimer m_TimeOut;
};