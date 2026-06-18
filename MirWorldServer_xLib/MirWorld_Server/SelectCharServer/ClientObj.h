#pragma once
typedef struct tagLOGINENTER
{
	tagLOGINENTER()
	{
		FILLSELF(0);
	}
	std::array<char, 12> szAccount;
	UINT nLid;
	UINT nSid;
	DWORD dwEnterTime;
	UINT nListId;
}LOGINENTER;

typedef struct tagCHARLISTNODE
{
	std::array<char, 32> szName;
	BYTE btLevel;
	BYTE btHair;
	BYTE btSex;
	BYTE btWhat;
}CHARLISTNODE;

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
protected:
	UINT m_nLoginId;
	UINT m_nSelectId;
	BOOL QueryCharList();
	VOID SendCharList(tQueryCharList_Result* pResult);
	VOID SendDelCharList(tQueryCharList_Result* pResult);
	LOGINENTER m_EnterInfo;
	selcharserverstate m_State;
	FINDSERVER_RESULT m_GameServerAddr;
	UINT m_nFailCount;
	CServerTimer m_TimeOut;
	BOOL m_bSelected;
	UINT m_nActiveCount{ 0 };
	std::array<CHAR, 32> m_szCharName{};
	BOOL m_bWaitForVerify{ FALSE };
	CServerTimer m_WaitForVerifyTimer;
};