#pragma once
#include "clientobj.h"
#include <array>

constexpr UINT MAX_CLIENTOBJECT = 8;
constexpr int MAX_SERVER_COUNT = 64;

typedef struct tagSERVERARRAY
{
	std::array<UINT, MAX_SERVER_COUNT> Ids{ 0 };
	int count{ 0 };
	int pickptr{ 0 };

	VOID DelId(UINT id)
	{
		for (int i = 0; i < count; i++)
		{
			if (Ids[i] == id)
			{
				for (int j = i; j < count - 1; j++)
				{
					Ids[j] = Ids[j + 1];
				}
				count--;
				// 调整pickptr：如果被删除元素在pickptr之前，pickptr前移
				if (pickptr > i && pickptr > 0)
					pickptr--;
				// 如果pickptr超出范围，回绕到0
				if (pickptr >= count)
					pickptr = 0;
				break;
			}
		}
	}
	BOOL AddId(UINT id)
	{
		if (count >= MAX_SERVER_COUNT) return FALSE;
		Ids[count++] = id;
		return TRUE;
	}
}SERVERARRAY;

class CServer :
	public CBaseServer,
	public xIndexObjectPool<CClientObj>,
	public xSingletonClass<CServer>
{
public:
	CServer(VOID);
	virtual ~CServer(VOID);
	VOID OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();

	SERVER_ERROR RegisterServer(CClientObj* pObj, REGISTER_SERVER_INFO* pInfo, REGISTER_SERVER_RESULT* pResult);
	SERVER_ERROR UnRegisterServer(CClientObj* pObj);
	SERVER_ERROR FindServer(servertype type, const char* pszName, FINDSERVER_RESULT* pResult);
public:
	VOID SendDBServer(SERVERADDR* pAddr, int count);
	int PrepareServer(int type, int count, SERVERADDR* pAddrArray);
protected:
	SERVERARRAY	m_ServerArrays[ST_MAX];
};