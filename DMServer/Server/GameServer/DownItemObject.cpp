#include "StdAfx.h"
#include ".\downitemobject.h"
#include "ItemManager.h"
#include "downitemmgr.h"
#include "gameworld.h"
#include "logicmap.h"
#include "aliveobject.h"
#include "systemscript.h"
#include "autoscriptmanager.h"
#include "humanplayer.h"

CDownItemObject::CDownItemObject(VOID)
{
	Clean();
}

CDownItemObject::~CDownItemObject(VOID)
{
}

VOID CDownItemObject::Clean()
{
	m_Id = 0;
	this->m_pClass = nullptr;
	memset(&m_Item, 0, sizeof(ITEM));
	SetActionObject(nullptr);
	m_nScriptTimes = 0;
	CMapObject::Clean();
}

BOOL CDownItemObject::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	length = EncodeMsg(pszMsg, m_Item.dwMakeIndex, SM_DOWNITEMAPPEAR,
		m_wX, m_wY, m_Item.baseitem.wImageIndex,
		m_Item.baseitem.szName, m_Item.baseitem.btNameLength);    //23.9.6暂时改成14, 后续更新
	return TRUE;
}

BOOL CDownItemObject::GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	length = EncodeMsg(pszMsg, m_Item.dwMakeIndex, SM_DOWNITEMDISAPPEAR, m_wX, m_wY, 0, nullptr);
	return TRUE;
}

VOID CDownItemObject::SetActionObject(CHumanPlayer* pObject)
{
	this->m_pActionObject = pObject;
	if (pObject)
		this->m_dwActionObjectInstanceKey = pObject->GetInstanceKey();
	else
		this->m_dwActionObjectInstanceKey = 0;
}

BOOL CDownItemObject::UpdateValid()
{
	if (m_pClass && m_pClass->wDropPageId != 0 && m_tTimer.IsTimeOut(m_pClass->dwDropPageDelay * m_nScriptTimes * 1000))
	{
		if (this->m_pActionObject && this->m_pActionObject->GetInstanceKey() != this->m_dwActionObjectInstanceKey)
			this->m_pActionObject = nullptr;

		CHumanPlayer* pExecuter = (m_pActionObject != nullptr ? m_pActionObject : CAutoScriptManager::GetInstance()->GetScriptTarget());
		if (pExecuter)
		{
			const char* pszDropPage = CItemManager::GetInstance()->GetStringFromPool(m_pClass->wDropPageId);
			if (pszDropPage[0] != 0)
			{
				CSystemScript::GetInstance()->Execute(pExecuter->GetScriptTarget(), pszDropPage, FALSE);
				pExecuter->SetParam(0, getX());
				pExecuter->SetParam(1, getY());
				pExecuter->SetParam(2, GetMapId());
				pExecuter->SetParam(3, m_nScriptTimes);
				pExecuter->SetUsingItem(&m_Item);
			}
			pExecuter->SetUsingItem(nullptr);
			pExecuter->ClearParam();
			if (m_Item.dwParam[3] == UR_DELETED)
			{
				CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, this);
				return FALSE;
			}
		}
		m_nScriptTimes++;
		if (m_nScriptTimes >= m_pClass->dwDropPageExecuteTimes)
			m_pClass = nullptr;
		else
			return TRUE;
	}
	if (m_tTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_ITEMUPDATETIME)))
	{
		if (this->m_OwnerId == 0)
		{
			//	删除自己
			//CItemManager::GetInstance()->DeleteItem(m_Item.dwMakeIndex);1
			CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, this);
			return FALSE;
		}
		else
		{
			this->m_OwnerId = 0;
			m_tTimer.Savetime();
		}
	}
	return TRUE;
}

VOID CDownItemObject::OnEnterMap(CLogicMap* pMap)
{
	CMapObject::OnEnterMap(pMap);
	int mx = getX(), my = getY();
	// 搜索范围与玩家视野搜索范围 m_bViewSearchRange 保持一致
	for (int x = -m_bViewSearchRange; x <= m_bViewSearchRange; x++)
	{
		for (int y = -m_bViewSearchRange; y <= m_bViewSearchRange; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfoShared(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHelper<CMapObject> helper(&pInfo->m_xObjectList);
				for (CMapObject* pObj = helper.first(); pObj != nullptr; pObj = helper.next())
				{
					if (pObj->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pObj)->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pObj)->UpdateVisibleObject(this);
					}
				}
			}
		}
	}
	m_pClass = CItemManager::GetInstance()->GetItemClass(this->m_Item);
}

VOID CDownItemObject::OnLeaveMap(CLogicMap* pMap)
{
	CMapObject::OnLeaveMap(pMap);
	int mx = getX(), my = getY();
	// 搜索范围与玩家视野搜索范围 m_bViewSearchRange 保持一致
	for (int x = -m_bViewSearchRange; x <= m_bViewSearchRange; x++)
	{
		for (int y = -m_bViewSearchRange; y <= m_bViewSearchRange; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfoShared(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHelper<CMapObject> helper(&pInfo->m_xObjectList);
				for (CMapObject* pObj = helper.first(); pObj != nullptr; pObj = helper.next())
				{
					if (pObj->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pObj)->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pObj)->RemoveVisibleObject(this);
					}
				}
			}
		}
	}
}

VOID CDownItemObject::OnDroped()
{
	m_tTimer.Savetime();
	UpdateValid();
}