#pragma once
#include "vmap.h"
#include "physicsmap.h"
#include "MapObject.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <array>

class CArea;
class CHumanPlayer;
class CAliveObject;
typedef struct tagMineItemList
{
	tagMineItemList()
	{
		FILLSELF(0);
	}
	char szName[32];
	WORD wDuraMax;
	WORD wDuraMin;
	WORD wRate;
	WORD w0;
	tagMineItemList* pNext;
}MineItemList;

class CLogicMap
{
public:
	CLogicMap(VOID);
	virtual ~CLogicMap(VOID);
	BOOL LoadMap(const char* pszFilename);
	BOOL IsBlocked(int x, int y);

	BOOL SendMsg(CMapObject* pSender, const char* pszCodedMsg, int size);

	BOOL MoveObject(CMapObject* pObject, int x, int y);

	BOOL AddObject(CMapObject* pObject);
	BOOL RemoveObject(CMapObject* pObject);

	CMapObject* FindObject(int x, int y, e_object_type type);
	CMapObject* FindEventObject(int x, int y, int View);

	// 内部方法：调用者需确保已持有 m_MapMutex 锁
	CMapCellInfo* GetMapCellInfo(UINT x, UINT y)
	{
		if (m_pCellInfo == nullptr || !VerifyPos(x, y))return nullptr;
		return (m_pCellInfo[x + y * m_iWidth]);
	}
	CMapCellInfo** GetCellInfoBase() const { return m_pCellInfo.get(); }
	// 内部方法：调用者需确保已持有 m_MapMutex 写锁
	CMapCellInfo* GetMapCellInfo_Safe(UINT x, UINT y)
	{
		if (m_pCellInfo == nullptr || !VerifyPos(x, y))return nullptr;
		CMapCellInfo** ppCellInfo = m_pCellInfo.get() + x + y * m_iWidth;
		if (*ppCellInfo == nullptr)
		{
			*ppCellInfo = m_xCellInfoPool.newObject();
			(*ppCellInfo)->wEventFlag = 0;
			(*ppCellInfo)->wFlag = 0;
			(*ppCellInfo)->m_pVisibleEventCache = nullptr;
		}
		return (*ppCellInfo);
	}
	// 线程安全的只读查询：自动加 shared_lock
	CMapCellInfo* GetMapCellInfoShared(UINT x, UINT y)
	{
		std::shared_lock lock(m_MapMutex);
		return GetMapCellInfo(x, y);
	}
	// 线程安全的写查询：自动加 unique_lock（可能分配新 CellInfo）
	CMapCellInfo* GetMapCellInfoExclusive(UINT x, UINT y)
	{
		std::unique_lock lock(m_MapMutex);
		return GetMapCellInfo_Safe(x, y);
	}
	// 线程安全的区域标记查询
	BOOL IsCellFlagSet(UINT x, UINT y, WORD wFlag)
	{
		std::shared_lock lock(m_MapMutex);
		CMapCellInfo* pInfo = GetMapCellInfo(x, y);
		return pInfo && (pInfo->wFlag & wFlag);
	}
	BOOL IsCellEventFlagSet(UINT x, UINT y, WORD wFlag)
	{
		std::shared_lock lock(m_MapMutex);
		CMapCellInfo* pInfo = GetMapCellInfo(x, y);
		return pInfo && (pInfo->wEventFlag & wFlag);
	}
	VOID RemoveMapCellInfo_Safe(int x, int y);
	int GetDupCount(int x, int y);
	int GetDupCount(int x, int y, e_object_type type);
	int GetDropItemPoint(int x, int y, POINT* ptArray, int ArraySize);
	int GetValidPoint(int x, int y, POINT* ptArray, int ArraySize);
	VOID CheckEnterEvent(CMapObject* pObject, int x, int y);
	VOID CheckLeaveEvent(CMapObject* pObject, int x, int y);
	VOID SetSafeArea(int x, int y, int range);
	VOID SetMapEventFlagRect(int x, int y, int xrange, int yrange, DWORD dwFlag);
	VOID AddStartPoint(START_POINT* pStartPoint);
	int	GetObjectCount(e_object_type type)const
	{
		int itype = (int)type;
		if (itype < 0 || itype >= OBJ_MAX)return 0;
		return m_iObjectCount[type];
	}
	BOOL IsPhysicsBlocked(int x, int y)
	{
		if (m_pPhysicsMap == nullptr)return TRUE;
		return m_pPhysicsMap->IsBlocked(x, y);
	}
	//在指定位置上是否有某类物体
	BOOL IsObjAtPosition(int x, int y, e_object_type type)
	{
		std::shared_lock lock(m_MapMutex);
		CMapCellInfo* pInfo = GetMapCellInfo(x, y);
		if (pInfo == nullptr) return FALSE;
		if (pInfo)
		{
			xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
			while (pNode)
			{
				if (pNode->getObject() && pNode->getObject()->GetType() == type)
					return TRUE;
				pNode = pNode->getNext();
			}
		}
		return FALSE;
	}
	VOID CheckEnterCity(CHumanPlayer* pPlayer);
	VOID DecObjectCount(DWORD dwType);
	VOID AddObjectCount(DWORD dwType);
	BOOL LockPos(int x, int y)
	{
		//	当可穿人的选项打开时, 这个层就是空的
		if (m_pLockLayer == nullptr)return TRUE;
		//	已经锁住了, 那么就无法再次锁住
		if (IsLocked(x, y))return FALSE;
		//	计算Lock的精确位置
		int f = y * m_iWidth + x;
		int blockIdx = f >> 5;
		int bitIdx = f & 31;
		//	blockIdx 超出最大的Layer范围, 那么锁定失败
		if (blockIdx >= m_iMaxBlockElements)return FALSE;
		m_pLockLayer[blockIdx] |= (1 << bitIdx);
		return TRUE;
	}
	BOOL UnLockPos(int x, int y)
	{
		if (m_pLockLayer == nullptr)return TRUE;
		//	这里没有被锁住, 那么就无法解锁
		if (!IsLocked(x, y))return FALSE;
		int f = y * m_iWidth + x;
		int blockIdx = f >> 5;
		int bitIdx = f & 31;
		if (blockIdx >= m_iMaxBlockElements)return FALSE;
		m_pLockLayer[blockIdx] ^= (m_pLockLayer[blockIdx] & (1 << bitIdx));
		return TRUE;
	}
	VOID ClearAllMonsters(const char* pszClassName = nullptr);
	UINT CountAllMonsters(const char* pszClassName = nullptr);
public:
	CAliveObject* FindTarget(CAliveObject* pAttacker, UINT x, UINT y, BOOL IsProperTarget = TRUE, BOOL IsDeath = TRUE);
	VOID AddMineItem(const char* pszName, WORD wDMin, WORD wDMax, WORD wRate);
	BOOL GotMineItem(CHumanPlayer* pPlayer);

	const char* GetName() { assert(m_pPhysicsMap != nullptr); return m_pPhysicsMap->GetName(); }
	const char* GetTitle() { return m_strName.c_str(); }
	int	 GetWidth()const { return m_iWidth; }
	int	 GetHeight()const { return m_iHeight; }
	int GetMusicId()const { return m_iMusicId; }
	CPhysicsMap* GetPhysicsMap() { return m_pPhysicsMap; }
	VOID SetPhysicsMap(CPhysicsMap* pPhysicsMap);

	UINT GetId()const { return m_Id; }
	VOID SetId(UINT id) { m_Id = id; }
	int	GetMiniMap()const { return m_iMiniMap; }
	int GetIndex()const { return m_nIndex; }
	VOID InitLinks();
	FLOAT GetExpFactor()const { return m_fExpFactor; }
	VOID SetExpFactor(FLOAT fFactor) { m_fExpFactor = fFactor; }
	VOID GetObjList(xListHelper<CMapObject>& objlist)
	{
		objlist.setList(&m_xObjList);
	}
	VOID SetFlag(const char* pszFlag);
	VOID UnSetFlag(const char* pszFlag);
	VOID UnSetFlag(e_map_flag flag);
	// 判断地图是否配置某标识, 并返回 dwParam 一个DWORD数 和 szExtraParams 一个字符数组
	BOOL IsFlagSeted(e_map_flag findex, DWORD& dwParam, std::vector<std::string>& szExtraParams);
	BOOL IsFlagSeted(e_map_flag findex, DWORD& dwParam);
	BOOL IsFlagSeted(e_map_flag findex);
	BOOL IsLocked(int x, int y)
	{
		//	make sure that before call this , the pos must be right
		//	如果没有这层, 表示所有地方都没有被锁住
		if (m_pLockLayer == nullptr)return FALSE;
		int f = y * m_iWidth + x;
		int blockIdx = f >> 5;
		int bitIdx = f & 31;
		return (blockIdx >= m_iMaxBlockElements || (m_pLockLayer[blockIdx] & (1 << bitIdx)) != 0);
	}
	static VOID	GetCellInfoInfo(int& used, int& free, int& total)
	{
		used = m_xCellInfoPool.getUsedCount();
		free = m_xCellInfoPool.getFreeCount();
		total = m_xCellInfoPool.getCount();
	}
	// 获取地图随机坐标的函数（带最大尝试次数限制，防止死循环）
	std::pair<int, int> GetRandomCoordinate()
	{
		const int MAX_RETRY = 1000;
		for (int retry = 0; retry < MAX_RETRY; ++retry)
		{
			int x = GetRangeRand(0, m_iWidth - 1);
			int y = GetRangeRand(0, m_iHeight - 1);
			if (!IsPhysicsBlocked(x, y))//判断是否有阻挡
			{
				return std::make_pair(x, y);
			}
		}
		// 随机方式失败，降级为遍历查找可通行点
		for (int x = 0; x < m_iWidth; ++x)
		{
			for (int y = 0; y < m_iHeight; ++y)
			{
				if (!IsPhysicsBlocked(x, y))
				{
					return std::make_pair(x, y);
				}
			}
		}
		// 地图无任何可通行点，使用出生点兜底
		if (m_iStartPointCount > 0 && m_pStartPoints[0] != nullptr)
		{
			return std::make_pair(m_pStartPoints[0]->x, m_pStartPoints[0]->y);
		}
		return std::make_pair(0, 0);
	}
	VOID SendAroundMsg(int x, int y, int range, const char* szMsg, int size, CMapObject* pSender = nullptr, BOOL bIncludeSelf = TRUE);
	Weather& GetWeather() { return m_xWeather; }
	BOOL DamageAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, int nDamage, damage_type damagetype, DWORD dwFlag, DWORD dwDelay = 0, BOOL bPushed = TRUE);
	BOOL CureBagStatusAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, std::vector<BYTE>& btArray, UINT nArraySize, std::vector<CAliveObject*>* retTargets = nullptr);
	BOOL AddAllProcess(DWORD dwTypeFlag, e_process ident, DWORD dwParam1 = 0,
		DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0,
		DWORD dwDelay = 0, int repeattimes = 0, const char* pszString = nullptr);
private:
	mutable std::shared_mutex m_MapMutex; // 读写保护锁
	FLOAT m_fExpFactor;
	BOOL AddObjectToPos(int x, int y, CMapObject* pObject);
	BOOL RemoveObjectFromPos(int x, int y, CMapObject* pObject);
	BOOL InitMapCells();
	BOOL VerifyPos(int x, int y)const
	{
		if (x < 0 || x >= m_iWidth)return FALSE;
		if (y < 0 || y >= m_iHeight)return FALSE;
		return TRUE;
	}
private:
	xStatus	m_Flag;
	std::unordered_map<int, std::vector<std::string>> m_flagExtraParams;
	int	m_nIndex;
	DWORD* m_pLockLayer;
	int m_iMaxBlockElements;
	CSettingFile m_DataFile;
	CPhysicsMap* m_pPhysicsMap;
	std::string m_strName;
	int	m_iWidth;
	int m_iHeight;
	UINT m_Id;
	int	m_iMiniMap;
	int	m_iLinkCount;
	int m_iMusicId;
	xListHost<CMapObject> m_xObjList;
	// MapCell信息
	std::unique_ptr<CMapCellInfo*[]> m_pCellInfo;
	START_POINT* m_pStartPoints[256];
	int	m_iStartPointCount;
	int	m_iObjectCount[OBJ_MAX];
	static xObjectPool<CMapCellInfo> m_xCellInfoPool;
	MineItemList* m_pMineItemList;
	UINT m_nMineRateMax;
	Weather m_xWeather;
private:// 事件系统使用
	//怪物死亡掉落物品
	VOID handleMonsterDropItem(const MonsterDeathEvent& e);
	SubscriptionToken subMonsterDropItemToken_;
};
