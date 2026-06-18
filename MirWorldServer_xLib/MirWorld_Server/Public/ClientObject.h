#pragma once
#include "inc.h"
#include "msgprocessor.h"

constexpr auto PINGTIME = 10000; //应用层心跳间隔10秒
class CClientObject :
	public xClientObject, public CMsgProcessor
{
public:
	CClientObject(VOID);
	virtual ~CClientObject(VOID);
public:
	virtual VOID Clean(VOID);
	VOID OnClientPingRet();
	BOOL SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData = nullptr, int datasize = -1);
	BOOL SendMsg(PMIRMSG pMsg, int datasize);
	BOOL PostMsg(const char* pszCodedMsg, int size);
	VOID OnSCMsg_T(PMIRMSG pMsg, int datasize);
	int	ParseMessage(char* pszMsg, int iSize);
	virtual VOID OnDBMsg(PMIRMSG pMsg, int datasize) {}
	virtual VOID OnSCMsg(PMIRMSG pMsg, int datasize) {}
	virtual VOID OnConnection() {}
	virtual VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize) {}
	virtual VOID Update();
	VOID OnDataPacket(xPacket* pPacket);
	VOID UpdateStarPing()
	{
		TRY_BEGIN
			if (m_Pingtimer.IsTimeOut(PINGTIME))
			{
				m_Pingtimer.Savetime();
				if (m_bPingNoRet)
				{
					Disconnect();
					return;
				}
				PostMsg("*", 1);
			}
		TRY_END
	}
	DWORD GetKey()
	{
		return m_dwClientKey;
	}
	CMsgProcessor* GetMsgProcessor() { return this->m_pMsgProcessor; }
	VOID SetMsgProcessor(CMsgProcessor* pMsgProcessor) { m_pMsgProcessor = pMsgProcessor; }

	// 批量发送模式控制
	VOID EnableBatchMode(BOOL bEnable) { m_bBatchMode = bEnable; }
	BOOL IsBatchMode()const { return m_bBatchMode; }
	BOOL FlushMsgQueue() { return flushBatch(); }
protected:
	DWORD m_dwClientKey;
	DWORD m_dwPing;
private:
	CMsgProcessor* m_pMsgProcessor;
	CServerTimer m_Pingtimer;
	BOOL m_bPingNoRet;
	xPacket	m_xParsePacket;
	xPacket	m_xSendPacket;
	xPacket	m_xDecodePacket;
};