#include "StdAfx.h"
#include ".\fireburnevent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include ".\logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"
//CFireBurnEvent g_FireBurnEvent;
xObjectPool<CFireBurnEvent>	CFireBurnEvent::m_xEventPool;
CFireBurnEvent::CFireBurnEvent(VOID)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	m_pEvents.fill(nullptr);
}

CFireBurnEvent::~CFireBurnEvent(VOID)
{
}

VOID CFireBurnEvent::OnUpdate(CVisibleEvent* pEvent)
{
	CLogicMap* pMap = pEvent->GetMap();
	if (pMap == nullptr)return;
	if (m_pOwner != nullptr)
	{
		if (m_pOwner->GetInstanceKey() != m_dwOwnerInstanceKey)
		{
			m_pOwner = nullptr;
			Destroy();
			return;
		}
	}
	CLogicMap* pEventMap = pEvent->GetMap();
	if (pEventMap == nullptr) { Destroy(); return; }
	CMapCellInfo* pInfo = pEventMap->GetMapCellInfoShared(pEvent->getX(), pEvent->getY());
	if (pInfo)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		CMapObject* pObject = nullptr;
		while (pNode)
		{
			pObject = pNode->getObject();
			if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && pObject != m_pOwner && !((CAliveObject*)pObject)->IsDeath() && m_pOwner->IsProperTarget((CAliveObject*)pObject))
			{
				UINT nDamage = Getrand(m_nDamage);
				((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, nDamage, m_pOwner == nullptr ? 0 : m_pOwner->GetId(), DT_DIRECT);
			}
			pNode = pNode->getNext();
		}
	}
}

VOID CFireBurnEvent::OnClose(CVisibleEvent* pEvent)
{
	int	count = 0;
	for (auto& event : m_pEvents)
	{
		if (event != nullptr)
		{
			if (event == pEvent)
				event = nullptr;
			else
				count++;
		}
	}
	if (count == 0)
		Destroy();
}

CFireBurnEvent* CFireBurnEvent::Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr) return nullptr;;
	CFireBurnEvent* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;
	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();

	pEvent->m_pEvents.fill(nullptr);
	static const std::array<POINT, 5> ptFire =
	{{
		{ 0, 0 },//中
		{ -1, 0 }, { 1, 0 },//左、右
		{ 0, -1 }, { 0, 1 },//上、下
	}};
	int successcount = 0;
	auto* pEventManager = CEventManager::GetInstance();
	for (int i = 0; i < 5; i++)
	{
		int targetX = x + ptFire[i].x;
		int targetY = y + ptFire[i].y;
		// 检查边界
		if (targetX < 0 || targetX >= pMap->GetWidth() || targetY < 0 || targetY >= pMap->GetHeight()) continue;
		pEvent->m_pEvents[i] = pEventManager->NewVisibleEvent(pOwner->GetMap(), targetX, targetY, VE_FIREWALL, dwTick, dwLastTime, pEvent);
		if (pEvent->m_pEvents[i] != nullptr)
			successcount++;
	}
	if (successcount == 0)
	{
		RemoveObject(pEvent);
		pEvent = nullptr;
	}
	else
		pEventManager->AddEventProcessor(pEvent);
	return pEvent;
}

VOID CFireBurnEvent::Destroy()
{
	for (auto& event : m_pEvents)
	{
		if (event)
		{
			event->Close();
			event = nullptr;
		}
	}
	m_nDamage = 0;
	m_pOwner = nullptr;
	CEventManager::GetInstance()->RemoveEventProcessor(this);
}

VOID CFireBurnEvent::Update()
{
	for (auto& event : m_pEvents)
	{
		if (event) event->UpdateValid();
	}
}
