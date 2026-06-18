#pragma once
//#include "mapobject.h"

constexpr int MAP_AREA_WIDTH = 1;
constexpr int MAP_AREA_HEIGHT = 1;

class CMapObject;
class CLogicMap;
class CPhysicsMap;

typedef struct t_VisibleObject
{
	t_VisibleObject() :Node(this)
	{
		pObject = nullptr;
		//btVisibleFlag = 0;
	}
	CMapObject* pObject;
	//BYTE	btVisibleFlag;
	//BYTE	btType;
	//DWORD	dwId;
	xListHost<t_VisibleObject>::xListNode Node;
}VISIBLE_OBJECT;

typedef xListHost<CMapObject> CELLOBJECTLIST;
typedef xListHost<CHumanPlayer> CELLHUMANLIST;

class CMapCellInfo
{
public:
	WORD wFlag;
	WORD wEventFlag;
	xListHost<CMapObject> m_xObjectList;
	//xListHost<CHumanPlayer> m_xHumanList;
	CMapObject* m_pVisibleEventCache; // VISIBLEEVENT»º´æ
	CMapCellInfo()
	{
		wFlag = 0;
		wEventFlag = 0;
		m_pVisibleEventCache = nullptr;
	}
};
