#pragma once
typedef struct tagREGISTEREDSERVER
{
	std::array<char, 64> szName;
	ServerId Ident;
	SERVERADDR Addr;
	int iSendDBCount;
	DWORD dwConnections;
}REGISTEREDSERVER;

class CClientObj :
	public CClientObject
{
public:
	CClientObj(VOID);
	virtual ~CClientObj(VOID);
	VOID Clean();
	VOID Update();
	VOID OnConnection();
	VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size);
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	REGISTEREDSERVER* GetRegInfo() { return &m_RegInfo; }
private:
	REGISTEREDSERVER m_RegInfo;
};