#include "StdAfx.h"
#include "mapobject.h"
#include "vmap.h"
#include "logicmap.h"

CMapObject::CMapObject(VOID)
{
	m_pMap = nullptr;
	m_Mapid = 0;
	m_Id = 0;
	m_wX = 0, m_wY = 0;
	m_wInvisibleLevel = 0;
	m_dwRefenceCount = 0;
	m_dwUpdateKey = 0;
	for (auto& node : m_DLinkNodes)
	{
		node = std::make_unique<xListHost<CMapObject>::xListNode>(this);
	}
}

CMapObject::~CMapObject(VOID)
{
}

VOID CMapObject::Clean()
{
	m_wX = 0, m_wY = 0;
	for (auto& node : m_DLinkNodes)
	{
		if (node)
			node->Leave();
	}
	m_pMap = nullptr;
	m_Mapid = 0;
	m_wInvisibleLevel = 0;
	m_dwRefenceCount = 0;
	m_dwUpdateKey = 0;
	m_Id = 0;
}

VOID CMapObject::setXY(WORD x, WORD y)
{
	if (x != m_wX || y != m_wY)
		OnSetPos(m_wX, m_wY, x, y);
	m_wX = x, m_wY = y;
}

int CMapObject::EncodeMsg(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	MIRMSGHEADER header(dwFlag, wCmd, w1, w2, w3);
	int	codedsize = 1;
	buffer[0] = '#';
	codedsize += _CodeGameCode((BYTE*)&header, sizeof(header), (BYTE*)buffer + codedsize);
	if (lpdata != nullptr)
	{
		if (datasize < 0)
			datasize = (int)strlen((char*)lpdata);
		codedsize += _CodeGameCode((BYTE*)lpdata, datasize, (BYTE*)buffer + codedsize);
	}
	buffer[codedsize++] = '!';
	buffer[codedsize] = 0;
	return codedsize;
}

int CMapObject::EncodeMsgRaw(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	MIRMSGHEADER header(dwFlag, wCmd, w1, w2, w3);
	int	codedsize = 1;
	buffer[0] = '#';
	// Header 部分使用编码加密
	codedsize += _CodeGameCode((BYTE*)&header, sizeof(header), (BYTE*)buffer + codedsize);
	if (lpdata != nullptr)
	{
		if (datasize < 0)
			datasize = (int)strlen((char*)lpdata);
		// Data 部分直接拷贝，不使用编码
		memcpy(buffer + codedsize, lpdata, datasize);
		codedsize += datasize;
	}

	buffer[codedsize++] = '!';
	buffer[codedsize] = 0;
	return codedsize;
}

BOOL CMapObject::InSafeArea()
{
	if (m_pMap == nullptr)return TRUE;
	return m_pMap->IsCellFlagSet(getX(), getY(), MAPCELLFLAG_NOPK);
}

BOOL CMapObject::InCityArea()
{
	if (m_pMap == nullptr)return TRUE;
	return m_pMap->IsCellEventFlagSet(getX(), getY(), EVENTFLAG_CITYEVENT);
}

BOOL CMapObject::InWarArea()
{
	if (m_pMap == nullptr)return TRUE;
	return m_pMap->IsCellEventFlagSet(getX(), getY(), EVENTFLAG_WAREVENT);
}