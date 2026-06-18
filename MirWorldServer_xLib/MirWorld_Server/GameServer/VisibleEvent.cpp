#include "StdAfx.h"
#include ".\visibleevent.h"
#include ".\logicmap.h"
#include ".\eventmanager.h"
#include ".\eventprocessor.h"
#include ".\aliveobject.h"

CVisibleEvent::CVisibleEvent(VOID)
{
	Clean();
}

CVisibleEvent::~CVisibleEvent(VOID)
{
}

VOID CVisibleEvent::Clean()
{
	m_pMap = nullptr;
	m_pEventProcessor = nullptr;
	m_bClosed = FALSE;
	m_dwView = 0;
}

BOOL CVisibleEvent::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	std::array<DWORD, 2> dwArray = { m_dwParam1, m_dwParam2 };
	length = EncodeMsg(pszMsg, (DWORD)this->GetId(), 804, m_dwView & 0xffff, m_wX, m_wY, (LPVOID)dwArray.data(), sizeof(dwArray));
	return TRUE;
}

BOOL CVisibleEvent::GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	length = EncodeMsg(pszMsg, (DWORD)this->GetId(), 805, 0, m_wX, m_wY, nullptr);
	return TRUE;
}

BOOL CVisibleEvent::Create(CLogicMap* pMap, int x, int y, DWORD dwView, DWORD dwRunTick, DWORD dwLastTime, CEventProcessor* pProcessor, DWORD dwParam1, DWORD dwParam2)
{
	if (pMap == nullptr)return FALSE;
	m_pEventProcessor = pProcessor;
	setXY(x, y);
	m_dwView = dwView;
	m_CloseTimer.SetTimeOut(dwLastTime);
	m_CloseTimer.Savetime();

	m_RunTimer.SetTimeOut(dwRunTick);

	SetParam(dwParam1, dwParam2);

	if (!pMap->AddObject(this))return FALSE;
	if (pProcessor)
		pProcessor->OnCreate(this);
	return TRUE;
}

VOID CVisibleEvent::Close()
{
	if (m_bClosed)return;
	if (m_pMap) m_pMap->RemoveObject(this);
	CEventManager::GetInstance()->PreDelVisibleEvent(this);
	m_bClosed = TRUE;
}

BOOL CVisibleEvent::UpdateValid()
{
	if (m_pEventProcessor == nullptr)
		return TRUE;
	if (!m_RunTimer.IsTimeOut())return TRUE;
	if (this->m_CloseTimer.IsTimeOut())
	{
		m_pEventProcessor->OnClose(this);
		Close();
		return FALSE;
	}
	m_RunTimer.Savetime();
	m_pEventProcessor->OnUpdate(this);
	return TRUE;
}

VOID CVisibleEvent::OnEnter(CMapObject* pObject)
{
	if (this->m_pEventProcessor)
		m_pEventProcessor->OnEnter(this, pObject);
}

VOID CVisibleEvent::OnLeave(CMapObject* pObject)
{
	if (this->m_pEventProcessor)
		m_pEventProcessor->OnLeave(this, pObject);
}

VOID CVisibleEvent::OnEnterMap(CLogicMap* pMap)
{
	CMapObject::OnEnterMap(pMap);
	int mx = getX(), my = getY();
	char szBuffer[1024];
	int length = 1024;
	if (!GetViewmsg(szBuffer, length))return;
	for (int x = -12; x <= 12; x++)
	{
		for (int y = -12; y <= 12; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfoShared(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					if (pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pNode->getObject())->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pNode->getObject())->UpdateVisibleObject(this);
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
}

VOID CVisibleEvent::OnLeaveMap(CLogicMap* pMap)
{
	CMapObject::OnLeaveMap(pMap);
	int mx = getX(), my = getY();
	for (int x = -12; x <= 12; x++)
	{
		for (int y = -12; y <= 12; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfoShared(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					if (pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pNode->getObject())->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pNode->getObject())->RemoveVisibleObject(this);
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
}