#pragma once
#include "vmap.h"
#include "flageventlistener.h"
#include <array>
#include <memory>

enum e_listnode_index
{
	LNI_AREA,
	LNI_MAP,
	LNI_WORLD,
	LNI_MAX,
};

class CLogicMap;
class CVArea;

class CMapObject : public CFlagEventListener
{
public:
	CMapObject(VOID);
	virtual ~CMapObject(VOID);
	xListHost<CMapObject>::xListNode* GetLinkNode(int Index)const
	{
		if (Index >= 0 && Index < LNI_MAX)
			return m_DLinkNodes[Index].get();
		return nullptr;
	}
	WORD getX()const { return m_wX; }
	WORD getY()const { return m_wY; }
	VOID setX(WORD x) { setXY(x, m_wY); }
	VOID setY(WORD y) { setXY(m_wX, y); }
	VOID setXY(WORD x, WORD y);

	virtual BOOL CanRecvMsg() { return FALSE; }
	virtual VOID OnAroundMsg(CMapObject* pSender, const char* pszCodedMsg, int size) {}
	virtual VOID OnEnterMap(CLogicMap* pMap) { m_pMap = pMap; }
	virtual VOID OnLeaveMap(CLogicMap* pMap) { m_pMap = nullptr; }
	virtual VOID Update() {	}
	virtual VOID Clean();
	virtual BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr) { return FALSE; }
	virtual BOOL GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr) { return FALSE; }
	//获取对象类型
	virtual e_object_type GetType() { return OBJ_MAX; }
	virtual e_class_type GetClassType() { return CLS_UNKNOWN; }

	UINT GetId() const { return m_Id; }
	VOID SetId(UINT Id) { m_Id = Id; }

	CLogicMap* GetMap() { return m_pMap; }
	virtual VOID OnSetPos(WORD oldx, WORD oldy, WORD newx, WORD newy) {}
	WORD GetInvisibleLevel()const { return m_wInvisibleLevel; }
	virtual WORD GetDeInvisibleLevel() { return 0; }
	DWORD AddRef() { return(++m_dwRefenceCount); }
	DWORD DecRef() { return (m_dwRefenceCount > 0) ? --m_dwRefenceCount : 0; }
	DWORD GetRef()const { return m_dwRefenceCount; }
	VOID SetDelTimer() { m_DeleteTimer.Savetime(); }
	BOOL IsDelTimerTimeOut(DWORD dwTimeOut) { return m_DeleteTimer.IsTimeOut(dwTimeOut); }
	UINT GetMapId()const { return m_Mapid; }
	VOID SetMapId(UINT mapid) { m_Mapid = mapid; }
	VOID SetUpdateKey(DWORD dwKey) { m_dwUpdateKey = dwKey; }
	DWORD GetUpdateKey()const { return m_dwUpdateKey; }

	virtual BOOL InSafeArea();
	BOOL InCityArea();
	BOOL InWarArea();
	virtual VOID OnEnterSafeArea() {}
	virtual VOID OnLeaveSafeArea() {}
	virtual VOID OnEnterCityArea() {}
	virtual VOID OnLeaveCityArea() {}
	virtual VOID OnEnterWarArea() {}
	virtual VOID OnLeaveWarArea() {}
	virtual const char* GetName() { return "MAPOBJECT"; }
	virtual VOID OnMoveTo(UINT ox, UINT oy, UINT nx, UINT ny) {}
protected:
	int EncodeMsg(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize = -1);
	int EncodeMsgRaw(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize = -1);

	CLogicMap* m_pMap;
	CServerTimer m_DeleteTimer;
	DWORD m_dwUpdateKey;
	std::atomic<DWORD> m_dwRefenceCount;
	WORD m_wInvisibleLevel;
	WORD m_wX, m_wY;
	UINT m_Mapid;
	UINT m_Id;
	std::array<std::unique_ptr<xListHost<CMapObject>::xListNode>, LNI_MAX> m_DLinkNodes;
};
