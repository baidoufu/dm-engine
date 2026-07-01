#include "StdAfx.h"
#include "logicmap.h"
#include "mapobject.h"
#include "physicsmapmgr.h"
#include "humanplayer.h"
#include "server.h"
#include "aliveobject.h"
#include "eventobject.h"
#include "changemapevent.h"
#include "physicsmap.h"
#include "visibleevent.h"
#include "itemmanager.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "gameworld.h"
#include "MonItemsMgr.h"
#include "DownItemMgr.h"

static constexpr std::array<POINT, SEARCH_COUNT> g_ptSearch =
{{
	{-1,-1},{0,-1},{1,-1},{1,0},		{1,1},{0,1},{-1,1},{-1,0},
	{-2,-2},{-1,-2},{0,-2},{1,-2},		{2,-2},{2,-1},{2,0},{2,1},
	{2,2},{1,2},{0,2},{-1,2},			{-2,2},{-2,1},{-2,0},{-2,-1},
	{-3,-3},{-2,-3},{-1,-3},{0,-3},		{1,-3},{2,-3},{3,-3},{3,-2},
	{3,-1},{3,0},{3,1},{3,2},			{3,3},{2,3},{1,3},{0,3},

	{-1,3},{-2,3},{-3,3},{-3,2},		{-3,1},{-3,0},{-3,-1},{-3,-2},
	{-4,-4},{-3,-4},{-2,-4},{-1,-4},	{0,-4},{1,-4},{2,-4},{3,-4},{4,-4},
	{4,-3},{4,-2},{4,-1},{4,0},         {4,1},{4,2},{4,3},{4,4},
	{3,4},{2,4},{1,4},{0,4},            {-1,4},{-2,4},{-3,4},{-4,4},
	{-4,3},{-4,2},{-4,1},{-4,0},        {-4,-1},{-4,-2},{-4,-3},
}};
xObjectPool<CMapCellInfo> CLogicMap::m_xCellInfoPool;
CLogicMap::CLogicMap(VOID)
{
	m_pPhysicsMap = nullptr;
	m_Id = 0;
	m_iMiniMap = 0;
	m_iMusicId = 0;
	m_nIndex = 0;
	m_iLinkCount = 0;
	m_pCellInfo = nullptr;
	m_pLockLayer = nullptr;
	m_iMaxBlockElements = 0;
	m_iWidth = 0;
	m_iHeight = 0;
	m_fExpFactor = 1.0f;
	m_iStartPointCount = 0;
	memset(m_pStartPoints, 0, sizeof(m_pStartPoints));
	memset(m_iObjectCount, 0, sizeof(m_iObjectCount));
	m_pMineItemList = nullptr;
	this->m_nMineRateMax = 0;
	m_Flag.Clean();
	this->m_xWeather.dwBGColor = 0xffffffff;
	//订阅事件
	subMonsterDropItemToken_ = SUBSCRIBE_EVENT(MonsterDeathEvent, [this](const MonsterDeathEvent& e) { this->handleMonsterDropItem(e); });
}

CLogicMap::~CLogicMap(VOID)
{
	//取消事件订阅
	UNSUBSCRIBE_EVENT(subMonsterDropItemToken_);
}

BOOL CLogicMap::AddObject(CMapObject* pObject)
{
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	if (pObject == nullptr || (pNode = pObject->GetLinkNode(LNI_MAP)) == nullptr)return FALSE;
	if (pNode->BelongTo(&m_xObjList))return FALSE;
	if (!m_xObjList.addNode(pNode))return FALSE;
	{// 锁只保护数据结构
		SWLock lock(m_MapMutex);
		if (!AddObjectToPos(pObject->getX(), pObject->getY(), pObject))
		{
			m_xObjList.removeNode(pNode);
			return FALSE;
		}
		AddObjectCount(pObject->GetType());
	}
	pObject->OnEnterMap(this);
	CheckEnterEvent(pObject, pObject->getX(), pObject->getY());
	if (pObject->InCityArea())
		pObject->OnEnterCityArea();
	if (pObject->InSafeArea())
		pObject->OnEnterSafeArea();
	if (pObject->InWarArea())
		pObject->OnEnterWarArea();
	return TRUE;
}

BOOL CLogicMap::RemoveObject(CMapObject* pObject)
{
	//	如果是空指针
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	if (pObject == nullptr || (pNode = pObject->GetLinkNode(LNI_MAP)) == nullptr)
		return FALSE;
	//	如果不属于地图的物件
	if (!pNode->BelongTo(&m_xObjList)) return FALSE;
	int nx = pObject->getX();
	int ny = pObject->getY();
	{// 锁只保护数据结构
		SWLock lock(m_MapMutex);
		if (!RemoveObjectFromPos(nx, ny, pObject))
			return FALSE;
		if (!m_xObjList.removeNode(pNode))
			return FALSE;
		DecObjectCount(pObject->GetType());
	}
	if (pObject->InCityArea())
		pObject->OnLeaveCityArea();
	if (pObject->InSafeArea())
		pObject->OnLeaveSafeArea();
	if (pObject->InWarArea())
		pObject->OnLeaveWarArea();
	pObject->OnLeaveMap(this);
	CheckLeaveEvent(pObject, nx, ny);
	return TRUE;
}

BOOL CLogicMap::InitMapCells()
{
	assert(m_pPhysicsMap != nullptr);
	assert(m_iWidth != 0);
	assert(m_iHeight != 0);
	int count = m_iWidth * m_iHeight;
	m_pCellInfo = std::make_unique<CMapCellInfo*[]>(count);

	if (m_pCellInfo == nullptr)
	{
		MessageBox(nullptr, "无法分配足够的内存, 请确认虚拟内存大小并酌情减少地图的数量!",
			"地图管理器", 0);
		exit(0);
	}
	memset(m_pCellInfo.get(), 0, sizeof(CMapCellInfo*) * count);
	return TRUE;
}

BOOL CLogicMap::SendMsg(CMapObject* pSender, const char* pszCodedMsg, int size)
{
	if (pszCodedMsg == nullptr || size <= 0) return FALSE;
	SRLock lock(m_MapMutex);
	for (auto pNode = m_xObjList.getHead(); pNode != nullptr; pNode = pNode->getNext())
	{
		CMapObject* pObj = pNode->getObject();
		// 快速跳过非玩家对象，避免虚函数调用开销
		if (pObj == nullptr || pObj == pSender || pObj->GetType() != OBJ_PLAYER) continue;
		CHumanPlayer* pPlayer = static_cast<CHumanPlayer*>(pObj);
		if (pPlayer->CanRecvMsg())
			pPlayer->OnAroundMsg(pSender, pszCodedMsg, size);
	}
	return TRUE;
}

BOOL CLogicMap::IsBlocked(int x, int y)
{
	if (IsLocked(x, y))return TRUE;
	if (m_pPhysicsMap == nullptr)return TRUE;
	return m_pPhysicsMap->IsBlocked(x, y);
}

VOID CLogicMap::SetFlag(const char* pszFlag)
{
	if (pszFlag == nullptr || pszFlag[0] == '\0')
		return;

	char szTempFlag[1024];
	o_strncpy(szTempFlag, pszFlag, sizeof(szTempFlag) - 1);

	char* Params[20] = { nullptr };
	int nParam = 0;
	char* p = strchr(szTempFlag, '(');
	if (p != nullptr)
	{
		*p++ = '\0';
		size_t len = strlen(p);
		if (len > 0 && p[len - 1] == ')')
			p[len - 1] = '\0';
		nParam = SearchParam(p, Params, 20, ",");
	}

	e_map_flag flag = GetMapFlagFromString(szTempFlag);
	if (flag >= MF_MAX)
		return;

	// 先在临时变量中构建所有数据，然后在锁内原子性应用
	DWORD dwParam = 0;
	std::vector<std::string> newExtraParams;
	if (nParam >= 1 && IsIntegerNumber(Params[0]))
	{
		if (nParam >= 2 && IsIntegerNumber(Params[0]) && IsIntegerNumber(Params[1]))
		{
			if (nParam >= 3 && IsIntegerNumber(Params[0]) && IsIntegerNumber(Params[1]) && IsIntegerNumber(Params[2]))
			{
				DWORD dwParam1 = StringToInteger(Params[0]) & 0xffff;
				DWORD dwParam2 = StringToInteger(Params[1]) & 0xff;
				DWORD dwParam3 = StringToInteger(Params[2]) & 0xff;
				dwParam = dwParam1 | (dwParam2 << 16) | (dwParam3 << 24);
				if (nParam > 3)
					newExtraParams.assign(Params + 3, Params + nParam);
			}
			else
			{
				DWORD dwParam1 = StringToInteger(Params[0]) & 0xffff;
				DWORD dwParam2 = StringToInteger(Params[1]) & 0xffff;
				dwParam = dwParam1 | (dwParam2 << 16);
				if (nParam > 2)
					newExtraParams.assign(Params + 2, Params + nParam);
			}
		}
		else
		{
			dwParam = StringToInteger(Params[0]);
			if (nParam > 1)
				newExtraParams.assign(Params + 1, Params + nParam);
		}
	}
	else if (nParam >= 1)
		newExtraParams.assign(Params, Params + nParam);

	SWLock lock(m_MapMutex);
	if (m_Flag.IsSeted((int)flag))
	{
		m_Flag.ClrStatus((int)flag);
		m_flagExtraParams.erase((int)flag);
	}

	if (!newExtraParams.empty())
		m_flagExtraParams[(int)flag] = std::move(newExtraParams);

	m_Flag.SetStatus((int)flag, dwParam, 0xffffffff);
}

VOID CLogicMap::UnSetFlag(const char* pszFlag)
{
	char szTempFlag[1024];
	o_strncpy(szTempFlag, pszFlag, sizeof(szTempFlag) - 1);
	char* p = strchr(szTempFlag, '(');
	if (p != nullptr)
		*p++ = 0;
	e_map_flag flag = GetMapFlagFromString(szTempFlag);
	UnSetFlag(flag);
}

VOID CLogicMap::UnSetFlag(e_map_flag flag)
{
	SWLock lock(m_MapMutex);
	m_Flag.ClrStatus((int)flag);
	m_flagExtraParams.erase((int)flag);
}

BOOL CLogicMap::IsFlagSeted(e_map_flag findex, DWORD& dwParam, std::vector<std::string>& szExtraParams)
{
	SRLock lock(m_MapMutex);
	if (m_Flag.IsSeted((int)findex))
	{
		dwParam = m_Flag.GetParam((int)findex);
		auto it = m_flagExtraParams.find((int)findex);
		if (it != m_flagExtraParams.end())
			szExtraParams = it->second;
		return TRUE;
	}
	return FALSE;
}

BOOL CLogicMap::IsFlagSeted(e_map_flag findex, DWORD& dwParam)
{
	std::vector<std::string> szExtraParams;
	return IsFlagSeted(findex, dwParam, szExtraParams);
}

BOOL CLogicMap::IsFlagSeted(e_map_flag findex)
{
	DWORD dwParam = 0;
	std::vector<std::string> szExtraParams;
	return IsFlagSeted(findex, dwParam, szExtraParams);
}

BOOL CLogicMap::LoadMap(const char* pszFilename)
{
	m_Flag.Clean();
	m_DataFile.Open(pszFilename);
	char* pszPhysicsMapName = (char*)m_DataFile.GetString("define", "blockmap", nullptr);
	char szPhysicsMapName[256];
	o_strncpy(szPhysicsMapName, pszPhysicsMapName, 255);

	m_strName = m_DataFile.GetString("define", "name", "NoName");
	m_iMusicId = m_DataFile.GetInteger("define", "musicid");
	m_iMiniMap = m_DataFile.GetInteger("define", "minimap");
	m_nIndex = m_DataFile.GetInteger("define", "mapid");
	m_iLinkCount = m_DataFile.GetInteger("define", "linkcount");
	m_fExpFactor = m_DataFile.GetInteger("define", "expfactor", 100) / 100.0f;
	char* pszFlag = (char*)m_DataFile.GetString("define", "flag", "");
	xStringsExpander<32> flags(pszFlag, '|');
	for (int i = 0; i < flags.getCount(); i++)
	{
		SetFlag(flags[i]);
	}
	char* pszValue = nullptr;
	q_strupper(szPhysicsMapName);
	if (pszPhysicsMapName == nullptr)
	{
		m_DataFile.Close();
		return FALSE;
	}
	CPhysicsMap* pPhysicsMap = CPhysicsMapMgr::GetInstance()->GetPhysicsMapByName(szPhysicsMapName);
	if (pPhysicsMap == nullptr)
	{
		m_DataFile.Close();
		return FALSE;
	}
	SetPhysicsMap(pPhysicsMap);
	if (!InitMapCells())return FALSE;
	return TRUE;
}

VOID CLogicMap::SetPhysicsMap(CPhysicsMap* pPhysicsMap)
{
	m_pPhysicsMap = pPhysicsMap;
	m_iWidth = m_pPhysicsMap->GetWidth();
	m_iHeight = m_pPhysicsMap->GetHeight();
	m_iMaxBlockElements = (m_iWidth * m_iHeight + 31) / 32;
	m_pLockLayer = m_pPhysicsMap->GetBlockLayer();
}

CMapObject* CLogicMap::FindObject(int x, int y, e_object_type type)
{
	SRLock lock(m_MapMutex);
	CMapCellInfo* pCellInfo = GetMapCellInfo(x, y);
	if (pCellInfo == nullptr)return nullptr;
	xListHost<CMapObject>::xListNode* pNode = pCellInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj && pObj->GetType() == type)
			return pObj;
		pNode = pNode->getNext();
	}
	return nullptr;
}

CMapObject* CLogicMap::FindEventObject(int x, int y, int View)
{
	SRLock lock(m_MapMutex);
	CMapCellInfo* pCellInfo = GetMapCellInfo(x, y);
	if (pCellInfo == nullptr) return nullptr;

	if (pCellInfo->m_pVisibleEventCache)
	{
		CMapObject* pCached = pCellInfo->m_pVisibleEventCache;
		if (pCached->GetType() != OBJ_VISIBLEEVENT)
		{
			// 缓存指向了非事件对象（不应发生），清除脏缓存
			pCellInfo->m_pVisibleEventCache = nullptr;
		}
		else
		{
			CVisibleEvent* pEvent = static_cast<CVisibleEvent*>(pCached);
			if (pEvent->GetView() == View)
				return pCached;
		}
	}
	// 遍历链表查找
	xListHost<CMapObject>::xListNode* pNode = pCellInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj && pObj->GetType() == OBJ_VISIBLEEVENT)
		{
			CVisibleEvent* pEvent = static_cast<CVisibleEvent*>(pObj);
			if (pEvent->GetView() == View)
			{
				pCellInfo->m_pVisibleEventCache = pObj;
				return pObj;
			}
		}
		pNode = pNode->getNext();
	}
	return nullptr;
}

CAliveObject* CLogicMap::FindTarget(CAliveObject* pAttacker, UINT x, UINT y, BOOL IsProperTarget, BOOL IsDeath)
{
	if (pAttacker == nullptr) return nullptr;
	SRLock lock(m_MapMutex);
	CMapCellInfo* pCellInfo = GetMapCellInfo(x, y);
	if (pCellInfo == nullptr)return nullptr;
	xListHost<CMapObject>::xListNode* pNode = pCellInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj && pObj->GetClassType() == CLS_ALIVEOBJECT)
		{
			CAliveObject* pTarget = static_cast<CAliveObject*>(pObj);
			if (pTarget)
			{
				if (IsDeath)
				{
					if (!pTarget->IsDeath())
					{
						if (!IsProperTarget || pAttacker->IsProperTarget(pTarget))
							return pTarget;
					}
				}
				else
				{
					if (!IsProperTarget || pAttacker->IsProperTarget(pTarget))
						return pTarget;
				}
			}
		}
		pNode = pNode->getNext();
	}
	return nullptr;
}

BOOL CLogicMap::MoveObject(CMapObject* pObject, int x, int y)
{
	if (pObject->getX() == x && pObject->getY() == y)
		return TRUE;
	if (IsBlocked(x, y)) return FALSE;
	int oldx = pObject->getX();
	int oldy = pObject->getY();
	BOOL bInCity = pObject->InCityArea();
	BOOL bInSafeArea = pObject->InSafeArea();
	BOOL bInWarArea = pObject->InWarArea();
	{// 锁保护 RemoveObjectFromPos + AddObjectToPos 的原子性
		SWLock lock(m_MapMutex);
		CMapCellInfo* pCellInfo = GetMapCellInfo_Safe(oldx, oldy);
		CMapCellInfo* pToCellInfo = GetMapCellInfo_Safe(x, y);
		if (pCellInfo == nullptr || pToCellInfo == nullptr)return FALSE;
		if (!RemoveObjectFromPos(oldx, oldy, pObject)) //从旧格子移除对象
			return FALSE;
		if (!AddObjectToPos(x, y, pObject)) //将对象添加到新格子
		{
			// 回滚：将对象重新添加回旧位置
			AddObjectToPos(oldx, oldy, pObject);
			return FALSE;
		}
	}
	//	检查离开事件
	CheckLeaveEvent(pObject, oldx, oldy);
	//	设置坐标～
	pObject->setXY(x, y);
	//	检查进入事件
	pObject->OnMoveTo(oldx, oldy, x, y);
	CheckEnterEvent(pObject, x, y);

	if (bInCity != pObject->InCityArea())
	{
		if (bInCity)
			pObject->OnLeaveCityArea();
		else
			pObject->OnEnterCityArea();
	}
	if (bInSafeArea != pObject->InSafeArea())
	{
		if (bInSafeArea)
			pObject->OnLeaveSafeArea();
		else
			pObject->OnEnterSafeArea();
	}
	if (bInWarArea != pObject->InWarArea())
	{
		if (bInWarArea)
			pObject->OnLeaveWarArea();
		else
			pObject->OnEnterWarArea();
	}
	return TRUE;
}

BOOL CLogicMap::AddObjectToPos(int x, int y, CMapObject* pObject)
{
	CMapCellInfo* pInfo = GetMapCellInfo_Safe(x, y);
	if (pInfo == nullptr)return FALSE;
	xListHost<CMapObject>::xListNode* pNode = pObject->GetLinkNode(LNI_AREA);
	if (pNode == nullptr || !pInfo->m_xObjectList.addNode(pNode))
	{
		if (pInfo->m_xObjectList.getCount() <= 0)
			RemoveMapCellInfo_Safe(x, y);
		return FALSE;
	}
	if (pObject->GetType() == OBJ_VISIBLEEVENT)
		pInfo->m_pVisibleEventCache = pObject;
	return TRUE;
}

// 注意：此方法不加锁，调用者必须持有m_MapMutex写锁
VOID CLogicMap::RemoveMapCellInfo_Safe(int x, int y)
{
	if (x < 0 || y < 0 || y >= m_iHeight || x >= m_iWidth)return;
	int index = x + y * m_iWidth;
	CMapCellInfo* pCellInfo = this->m_pCellInfo[index];
	if (pCellInfo == nullptr)return;
	if (pCellInfo->wEventFlag != 0 || pCellInfo->wFlag != 0)return;
	m_xCellInfoPool.deleteObject(pCellInfo);
	this->m_pCellInfo[index] = nullptr;
}

BOOL CLogicMap::RemoveObjectFromPos(int x, int y, CMapObject* pObject)
{
	CMapCellInfo* pInfo = GetMapCellInfo(x, y);
	if (pInfo == nullptr)return FALSE;
	xListHost<CMapObject>::xListNode* pNode = pObject->GetLinkNode(LNI_AREA);
	if (pNode == nullptr)return FALSE;
	if (!pInfo->m_xObjectList.removeNode(pNode))
		return FALSE;
	if (pInfo->m_pVisibleEventCache == pObject)
		pInfo->m_pVisibleEventCache = nullptr;
	if (pInfo->m_xObjectList.getCount() <= 0)
		RemoveMapCellInfo_Safe(x, y);
	return TRUE;
}

int CLogicMap::GetDupCount(int x, int y)
{
	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = GetMapCellInfo(x, y);
	if (pInfo == nullptr)return 0;
	int iDupCount = 0;
	xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr)
		{
			pNode = pNode->getNext();
			continue;
		}
		e_object_type type = pObj->GetType();
		if (type == OBJ_MONSTER || type == OBJ_NPC || type == OBJ_PLAYER)
		{
			CAliveObject* pAObject = static_cast<CAliveObject*>(pObj);
			if (!pAObject->IsDeath())
				iDupCount++;
		}
		pNode = pNode->getNext();
	}
	return iDupCount;
}

int CLogicMap::GetDupCount(int x, int y, e_object_type type)
{
	// 障碍处没有堆积~
	if (this->m_pPhysicsMap && this->m_pPhysicsMap->IsBlocked(x, y))return -1;
	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = GetMapCellInfo(x, y);
	if (pInfo == nullptr)return 0;
	int iDupCount = 0;
	xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr)
		{
			pNode = pNode->getNext();
			continue;
		}
		e_object_type rtype = pObj->GetType();
		if (rtype == type) iDupCount++;
		pNode = pNode->getNext();
	}
	return iDupCount;
}

int CLogicMap::GetDropItemPoint(int x, int y, POINT* ptArray, int ArraySize)
{
	int drops[SEARCH_COUNT];
	memset(drops, 0xcd, sizeof(drops));
	int	droppointcount = 0;
	const POINT* pPt = nullptr;
	int	count = 0;
	int	index = 0;
	for (int i = 0; i < ArraySize; i++)
	{
		for (int j = 0; j < SEARCH_COUNT; j++)
		{
			if (drops[j] == 0xcdcdcdcd)
			{
				if (IsBlocked(x + g_ptSearch[j].x, y + g_ptSearch[j].y))
					drops[j] = -1;
				else
					drops[j] = GetDupCount(x + g_ptSearch[j].x, y + g_ptSearch[j].y, OBJ_DOWNITEM);
			}
			if (drops[j] == -1) continue;
			if (drops[j] < 10)
			{
				if (pPt == nullptr || drops[j] < count)
				{
					count = drops[j];
					pPt = &g_ptSearch[j];
					index = j;
					if (count == 0) break;
				}
			}
		}
		if (pPt == nullptr) break;
		drops[index]++;
		ptArray[droppointcount].x = x + pPt->x;
		ptArray[droppointcount].y = y + pPt->y;
		droppointcount++;
		if (droppointcount >= ArraySize)return droppointcount;
		pPt = nullptr;
		count = 0;
	}
	return droppointcount;
}

int CLogicMap::GetValidPoint(int x, int y, POINT* ptArray, int ArraySize)
{
	int count = 0;
	for (const auto& pt : g_ptSearch)
	{
		ptArray[count].x = MAX(0, x + pt.x);
		ptArray[count].y = MAX(0, y + pt.y);
		if (IsBlocked(ptArray[count].x, ptArray[count].y)) continue;
		count++;
		if (count >= ArraySize) return ArraySize;
	}
	return count;
}

VOID CLogicMap::CheckEnterCity(CHumanPlayer* pPlayer)
{
	if (pPlayer == nullptr)return;
	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = GetMapCellInfo(pPlayer->getX(), pPlayer->getY());
	if (pInfo == nullptr)return;

	START_POINT* pStartPoint = m_pStartPoints[pInfo->wEventFlag & 0xff];
	if (pStartPoint != nullptr)
	{
		if (pStartPoint->index != pPlayer->GetStartPointIndex())
			pPlayer->SetStartPointIndex(pStartPoint->index);
	}
}

VOID CLogicMap::CheckEnterEvent(CMapObject* pObject, int x, int y)
{
	if (pObject == nullptr) return;
	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = GetMapCellInfo(x, y);
	if (pInfo == nullptr || !(pInfo->wEventFlag & EVENTFLAG_ENTEREVENT))
		return;
	xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr || pObj->GetClassType() != CLS_EVENT)
		{
			pNode = pNode->getNext();
			continue;
		}
		CEventObject* pEObject = static_cast<CEventObject*>(pObj);
		if (!pEObject->IsDisabled())
			pEObject->OnEnter(pObject);
		pNode = pNode->getNext();
	}
}

VOID CLogicMap::CheckLeaveEvent(CMapObject* pObject, int x, int y)
{
	if (pObject == nullptr) return;
	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = GetMapCellInfo(x, y);
	if (pInfo == nullptr || !(pInfo->wEventFlag & EVENTFLAG_LEAVEEVENT))
		return;
	xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr || pObj->GetClassType() != CLS_EVENT)
		{
			pNode = pNode->getNext();
			continue;
		}
		CEventObject* pEObject = static_cast<CEventObject*>(pObj);
		if (!pEObject->IsDisabled())
			pEObject->OnLeave(pObject);
		pNode = pNode->getNext();
	}
}

VOID CLogicMap::InitLinks()
{
	int x, y, tmid, tx, ty;
	char szVname[200] = {0};
	char* pszValue = nullptr;
	for (int i = 0; i < m_iLinkCount; i++)
	{
		sprintf(szVname, "linkpoint%d", i + 1);
		pszValue = (char*)m_DataFile.GetString("linkpoint", szVname);
		if (pszValue != nullptr)
		{
			int result = sscanf(pszValue, "(%d,%d)=>[%d](%d,%d)", &x, &y, &tmid, &tx, &ty);
			if (result == 5) // 确保所有5个参数都成功解析
				CChangeMapEvent::Create(m_nIndex, x, y, tmid, tx, ty);
		}
	}
}

VOID CLogicMap::SetSafeArea(int x, int y, int range)
{
	int startx = MAX(0, x - range);
	int starty = MAX(0, y - range);
	int endx = MIN((int)m_iWidth, x + range);
	int endy = MIN((int)m_iHeight, y + range);
	if (startx > endx || starty > endy || range < 0)
		return;
	SWLock lock(m_MapMutex);
	CMapCellInfo* pInfo = nullptr;
	for (int tx = startx; tx < endx; tx++)
	{
		for (int ty = starty; ty < endy; ty++)
		{
			pInfo = GetMapCellInfo_Safe(tx, ty);
			if (pInfo != nullptr)
				pInfo->wFlag |= MAPCELLFLAG_NOPK;
		}
	}
}

VOID CLogicMap::SetMapEventFlagRect(int x, int y, int xrange, int yrange, DWORD dwFlag)
{
	int startx = MAX(0, x - xrange);
	int starty = MAX(0, y - yrange);
	int endx = MIN((int)m_iWidth, x + xrange);
	int endy = MIN((int)m_iHeight, y + yrange);
	if (startx > endx || starty > endy || xrange < 0 || yrange < 0)
		return;
	SWLock lock(m_MapMutex);
	CMapCellInfo* pInfo = nullptr;
	for (int tx = startx; tx < endx; tx++)
	{
		for (int ty = starty; ty < endy; ty++)
		{
			pInfo = GetMapCellInfo_Safe(tx, ty);
			if (pInfo != nullptr)
				pInfo->wEventFlag |= (dwFlag & 0xffff);
		}
	}
}

VOID CLogicMap::AddStartPoint(START_POINT* pStartPoint)
{
	if (pStartPoint == nullptr || m_iStartPointCount >= 256)
		return;
	BYTE nIndex = (BYTE)m_iStartPointCount;
	m_pStartPoints[nIndex] = pStartPoint;
	m_iStartPointCount++;

	int x = pStartPoint->x;
	int y = pStartPoint->y;
	int range = pStartPoint->range;

	int startx = MAX(0, x - range);
	int starty = MAX(0, y - range);
	int endx = MIN((int)m_iWidth, x + range);
	int endy = MIN((int)m_iHeight, y + range);
	if (startx > endx || starty > endy || range < 0)
		return;
	SWLock lock(m_MapMutex);
	CMapCellInfo* pInfo = nullptr;
	for (int tx = startx; tx < endx; tx++)
	{
		for (int ty = starty; ty < endy; ty++)
		{
			pInfo = GetMapCellInfo_Safe(tx, ty);
			if (pInfo != nullptr)
			{
				// 只在未设置城市事件标志时才设置
				if ((pInfo->wEventFlag & EVENTFLAG_CITYEVENT) == 0)
				{
					pInfo->wEventFlag |= EVENTFLAG_CITYEVENT;
					pInfo->wEventFlag = (pInfo->wEventFlag & 0xff00) | nIndex;
				}
			}
		}
	}
}

VOID CLogicMap::AddMineItem(const char* pszName, WORD wDMin, WORD wDMax, WORD wRate)
{
	if (this->m_pMineItemList == nullptr)
		this->m_pMineItemList = new MineItemList;
	else
	{
		MineItemList* p = new MineItemList;
		p->pNext = m_pMineItemList;
		this->m_pMineItemList = p;
	}
	this->m_pMineItemList->wDuraMax = wDMax;
	this->m_pMineItemList->wDuraMin = wDMin;
	this->m_pMineItemList->wRate = wRate;
	this->m_nMineRateMax += wRate;
	o_strncpy(this->m_pMineItemList->szName, pszName, 30);
}

BOOL CLogicMap::GotMineItem(CHumanPlayer* pPlayer)
{
	if (pPlayer == nullptr || m_pMineItemList == nullptr || m_nMineRateMax == 0)
		return FALSE;
	CItemManager* pItemMgr = CItemManager::GetInstance();
	UINT nRate = Getrand(m_nMineRateMax);
	UINT nAllRate = 0;
	MineItemList* pList = m_pMineItemList;
	ITEM item;
	while (pList)
	{
		nAllRate += pList->wRate;
		if (nRate < nAllRate && pItemMgr->CreateTempItem(pList->szName, item))
		{
			pList->w0++;
			if (item.baseitem.btStdMode == ISM_MINE)
				item.wCurDura = GetRangeRand(pList->wDuraMin, pList->wDuraMax) * 1000;
			// 背包满时掉落到地面，避免物品消失
			if (!pPlayer->AddBagItem(item))
			{
				if (!CDownItemMgr::GetInstance()->DropItem(this, item, pPlayer->getX(), pPlayer->getY(), TRUE, pPlayer))
				{
					CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
					return FALSE;
				}
			}
			return TRUE;
		}
		pList = pList->pNext;
	}
	return FALSE;
}

VOID CLogicMap::DecObjectCount(DWORD dwType)
{
	if (dwType < OBJ_MAX)
	{
		this->m_iObjectCount[dwType]--;
		if (this->m_iObjectCount[dwType] < 0)
			this->m_iObjectCount[dwType] = 0;
	}
}

VOID CLogicMap::AddObjectCount(DWORD dwType)
{
	if (dwType < OBJ_MAX)
		this->m_iObjectCount[dwType]++;
}

VOID CLogicMap::ClearAllMonsters(const char* pszClassName)
{
	MonsterClass* pDesc = nullptr;
	CGameWorld* pGameWorld = CGameWorld::GetInstance();
	CMonsterManagerEx* pMonsterMgr = CMonsterManagerEx::GetInstance();
	if (pszClassName != nullptr)
		pDesc = pMonsterMgr->GetClassByName(pszClassName);

	// 在锁内收集怪物指针，避免遍历期间 m_xObjList 被并发修改
	std::vector<CMonsterEx*> monstersToRemove;
	{
		SRLock lock(m_MapMutex);
		xListHost<CMapObject>::xListNode* pNode = m_xObjList.getHead();
#ifdef _DEBUG
		UINT nCount = 0;
		UINT nMCount = 0;
		UINT nOCount = 0;
#endif
		if (pszClassName == nullptr) // 删除所有怪物
		{
			while (pNode)
			{
				CMapObject* pObj = pNode->getObject();
				if (pObj && pObj->GetType() == OBJ_MONSTER)
				{
					monstersToRemove.push_back(static_cast<CMonsterEx*>(pObj));
#ifdef _DEBUG
					nCount++;
					nMCount++;
#endif
				}
#ifdef _DEBUG
				nOCount++;
#endif
				pNode = pNode->getNext();
			}
		}
		else // 只删除指定类型的怪物
		{
			while (pNode)
			{
				CMapObject* pObj = pNode->getObject();
				if (pObj && pObj->GetType() == OBJ_MONSTER)
				{
					CMonsterEx* pMonster = static_cast<CMonsterEx*>(pObj);
					if (pDesc == pMonster->GetDesc())
					{
						monstersToRemove.push_back(pMonster);
#ifdef _DEBUG
						nCount++;
#endif
					}
#ifdef _DEBUG
					nMCount++;
#endif
				}
#ifdef _DEBUG
				nOCount++;
#endif
				pNode = pNode->getNext();
			}
		}
#ifdef	_DEBUG
		PRINT(ERROR_RED, "trace %u objects found %u monsters remove %u monsters\n", nOCount, nMCount, nCount);
#endif
	}// SRLock 释放 — RemoveMapObject→RemoveObject 内部会获取 SWLock

	for (auto* pMonster : monstersToRemove)
	{
		pGameWorld->RemoveMapObject(pMonster);
		pMonsterMgr->DeleteMonsterImm(pMonster);
	}
}

UINT CLogicMap::CountAllMonsters(const char* pszClassName)
{
	MonsterClass* pDesc = nullptr;
	if (pszClassName != nullptr)
		pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(pszClassName);
	SRLock lock(m_MapMutex);
	xListHost<CMapObject>::xListNode* pNode = this->m_xObjList.getHead();
	UINT nCount = 0;
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr || pObj->GetType() != OBJ_MONSTER)
		{
			pNode = pNode->getNext();
			continue;
		}
		CMonsterEx* pTarget = static_cast<CMonsterEx*>(pObj);
		if (!pTarget->IsDeath())
		{
			if (pDesc && pDesc != pTarget->GetDesc())
				continue;
			nCount++;
		}
		pNode = pNode->getNext();
	}
	return nCount;
}

VOID CLogicMap::SendAroundMsg(int x, int y, int range, const char* szMsg, int size, CMapObject* pSender, BOOL bIncludeSelf)
{
	if (range < 0 || size <= 0 || szMsg == nullptr) return;
	int startx = MAX(0, x - range);
	int starty = MAX(0, y - range);
	int endx = MIN(m_iWidth - 1, x + range);
	int endy = MIN(m_iHeight - 1, y + range);
	if (startx > endx || starty > endy) return;

	SRLock lock(m_MapMutex);
	CMapCellInfo* pInfo = nullptr;
	for (int _x = startx; _x <= endx; _x++)
	{
		for (int _y = starty; _y <= endy; _y++)
		{
			pInfo = this->GetMapCellInfo(_x, _y);
			if (pInfo == nullptr || pInfo->m_xObjectList.getCount() == 0) continue;
			auto* pNode = pInfo->m_xObjectList.getHead();
			while (pNode)
			{
				CMapObject* pTarget = pNode->getObject();
				pNode = pNode->getNext();
				if (pTarget == nullptr) continue;
				// 快速跳过非玩家对象，避免虚函数调用开销
				if (pTarget->GetType() != OBJ_PLAYER) continue;
				if (!bIncludeSelf && pSender == pTarget) continue;
				// 直接调用玩家消息处理，跳过 CanRecvMsg 虚函数
				CHumanPlayer* pPlayer = static_cast<CHumanPlayer*>(pTarget);
				if (pPlayer->CanRecvMsg())
					pPlayer->OnAroundMsg(pSender, szMsg, size);
			}
		}
	}
}

BOOL CLogicMap::DamageAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, int nDamage, damage_type damagetype, DWORD dwFlag, DWORD dwDelay, BOOL bPushed)
{
	BOOL bRet = FALSE;
	// 边界预计算
	int nStartX = MAX(0, (int)x - (int)nRange);
	int nStartY = MAX(0, (int)y - (int)nRange);
	int nEndX = MIN((int)m_iWidth - 1, (int)x + (int)nRange);
	int nEndY = MIN((int)m_iHeight - 1, (int)y + (int)nRange);
	if (nStartX > nEndX || nStartY > nEndY) return FALSE;

	// 推迟击退操作到锁释放后执行，避免 SRLock(S)→SWLock(X) SRWLOCK 升级死锁
	struct PendingPush { CAliveObject* target; int dir; };
	PendingPush pendingPushes[12]{};
	int pushCount = 0;

	{// SRLock 作用域 — 只保护 CellInfo 遍历和 BeAttack，击退推迟到锁外
		xListHost<CMapObject>::xListNode* pNode = nullptr;
		CMapCellInfo* pInfo = nullptr;
		SRLock lock(m_MapMutex);
		for (int _x = nStartX; _x <= nEndX; _x++)
		{
			if (_x < 0 || _x >= this->m_iWidth)continue;
			for (int _y = nStartY; _y <= nEndY; _y++)
			{
				if (_y < 0 || _y >= this->m_iHeight)continue;
				pInfo = this->GetMapCellInfo(_x, _y);
				if (pInfo == nullptr || pInfo->m_xObjectList.getCount() == 0)continue;
				pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObj = pNode->getObject();
					if (pObj == nullptr || pObj->GetClassType() != CLS_ALIVEOBJECT)
					{
						pNode = pNode->getNext();
						continue;
					}
					CAliveObject* pTarget = static_cast<CAliveObject*>(pObj);
					if (pTarget && !pTarget->IsDeath() && pAttacker->IsProperTarget(pTarget))
					{
						if (dwDelay == 0)
							pTarget->BeAttack(pAttacker, nDamage, damagetype, dwFlag);
						else
							pTarget->AddProcess(EP_BEATTACKED, nDamage, pAttacker->GetId(), (DWORD)damagetype, dwFlag, dwDelay, 0, nullptr);
						if (bPushed && pushCount < 12 && pTarget->CanBePushed(pAttacker))
						{
							int rx = pTarget->getX();
							int ry = pTarget->getY();
							if (rx != x || ry != y)
							{
								pendingPushes[pushCount].target = pTarget;
								pendingPushes[pushCount].dir = GetDirectionTo(rx, ry, x, y);
								pushCount++;
							}
						}
						bRet = TRUE;
					}
					pNode = pNode->getNext();
				}
			}
		}
	}// SRLock 在此释放

	// 锁释放后执行击退 — DoPushed → MoveObject 内部会获取 SWLock
	for (int i = 0; i < pushCount; i++)
	{
		pendingPushes[i].target->DoPushed(pendingPushes[i].dir);
	}

	return bRet;
}

BOOL CLogicMap::CureBagStatusAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, std::vector<BYTE>& btArray, UINT nArraySize, std::vector<CAliveObject*>* retTargets)
{
	BOOL bRet = FALSE;
	// 边界预计算
	int nStartX = MAX(0, (int)x - (int)nRange);
	int nStartY = MAX(0, (int)y - (int)nRange);
	int nEndX = MIN((int)m_iWidth - 1, (int)x + (int)nRange);
	int nEndY = MIN((int)m_iHeight - 1, (int)y + (int)nRange);
	if (nStartX > nEndX || nStartY > nEndY) return FALSE;
	if (nArraySize == 0) return FALSE;
	if (retTargets != nullptr) retTargets->clear(); // 清空返回的目标列表
	SRLock lock(m_MapMutex);
	for (int _x = nStartX; _x <= nEndX; _x++)
	{
		if (_x < 0 || _x >= this->m_iWidth)continue;
		for (int _y = nStartY; _y <= nEndY; _y++)
		{
			if (_y < 0 || _y >= this->m_iHeight)continue;
			CMapCellInfo* pInfo = this->GetMapCellInfo(_x, _y);
			if (pInfo == nullptr || pInfo->m_xObjectList.getCount() == 0)continue;
			xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
			if (retTargets == nullptr)
			{
				while (pNode)
				{
					CMapObject* pObj = pNode->getObject();
					if (pObj == nullptr || pObj->GetClassType() != CLS_ALIVEOBJECT)
					{
						pNode = pNode->getNext();
						continue;
					}
					CAliveObject* pTarget = static_cast<CAliveObject*>(pObj);
					if (!pTarget->IsDeath() && pAttacker->IsProperFriend(pTarget))
					{
						for (UINT n = 0; n < nArraySize; n++)
						{
							if (pTarget->IsStatusSet(btArray[n]))
							{
								pTarget->ClrStatus(btArray[n]);
								bRet = TRUE;
							}
						}
					}
					pNode = pNode->getNext();
				}
			}
			else
			{
				while (pNode)
				{
					CMapObject* pObj = pNode->getObject();
					if (pObj == nullptr || pObj->GetClassType() != CLS_ALIVEOBJECT)
					{
						pNode = pNode->getNext();
						continue;
					}
					CAliveObject* pTarget = static_cast<CAliveObject*>(pObj);
					if (!pTarget->IsDeath() && pAttacker->IsProperFriend(pTarget))
					{
						BOOL bTarget = FALSE;
						for (UINT n = 0; n < nArraySize; n++)
						{
							if (pTarget->IsStatusSet(btArray[n]))
							{
								pTarget->ClrStatus(btArray[n]);
								bTarget = TRUE;
							}
						}
						if (bTarget)
						{
							bRet = TRUE;
							retTargets->push_back(pTarget);
							if (retTargets->size() >= nArraySize) return bRet;
						}
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
	return bRet;
}

BOOL CLogicMap::AddAllProcess(DWORD dwTypeFlag, e_process ident, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4,
	DWORD dwDelay, int repeattimes, const char* pszString)
{
	SRLock lock(m_MapMutex);
	xListHost<CMapObject>::xListNode* pNode = m_xObjList.getHead();
	while (pNode)
	{
		CMapObject* pObj = pNode->getObject();
		if (pObj == nullptr || pObj->GetType() != dwTypeFlag)
		{
			pNode = pNode->getNext();
			continue;
		}
		if (pObj->GetClassType() == CLS_ALIVEOBJECT)
		{
			CAliveObject* pAObj = static_cast<CAliveObject*>(pObj);
			pAObj->AddProcess(ident, dwParam1, dwParam2, dwParam3, dwParam4, dwDelay, repeattimes, pszString);
		}
		pNode = pNode->getNext();
	}
	return TRUE;
}

VOID CLogicMap::handleMonsterDropItem(const MonsterDeathEvent& e)
{
	if (e.m_boDropItem && e.monster->GetMap() == this)
	{
		POINT pts[SEARCH_COUNT];
		MonsterClass* pDesc = e.monster->GetDesc();
		int dropcount = GetDropItemPoint(e.monster->getX(), e.monster->getY(), pts, SEARCH_COUNT);
		int droptotal = 0;
		ITEM item;
		if (dropcount > 0)
		{
			if (pDesc != nullptr && pDesc->pDownItems != nullptr)
			{
				DOWNITEM* pDownItem = pDesc->pDownItems->pItems;
				while (pDownItem)
				{
					int x = pts[droptotal].x;
					int y = pts[droptotal].y;
					if (pDownItem->updatecycle())
					{
						if (CMonItemsMgr::GetInstance()->CreateDownItem(pDownItem, item))
						{
							CDownItemMgr::GetInstance()->DropItem(this, item, x, y, TRUE, e.killer);
							droptotal++;
						}
						if (droptotal >= dropcount) droptotal = 0;
					}
					pDownItem = pDownItem->pNext;
				}
			}
		}
	}
}