#pragma once

#include "SimpleCharacter.h"
#include "SimpleGood.h"
#include "Global\DebugTry.h"
#include "GameDefine.h"

#define IDMAXNUMBER 1000

#define VIEW_WIDTH	12
#define VIEW_HEIGHT 12

#define hashXYfunc(x,y)	{ x %= MAPARR_WIDTH; y%= MAPARR_WIDTH; }

typedef std::map<DWORD ,CSimpleCharacterNode *> MiniChar;
typedef std::map<DWORD ,CSimpleGoodNode *>		MiniGood;

class CMapArray
{
private:
	struct _Data
	{
		CSimpleGoodNode			m_good;
		CSimpleCharacterNode	m_character;
	};
public:
	CMapArray(void);
	~CMapArray(void);

	void Clear();
	CSimpleGoodNode * AddGood(int x,int y,DWORD nID);
	CSimpleCharacterNode * AddCharacter(int x,int y,DWORD nID);
	CSimpleGoodNode * GetSimpleGoodHead(int x,int y);
	CSimpleCharacterNode * GetSimpleCharacterHead(int x,int y);
	CSimpleGoodNode * FindSimpleGood(DWORD nID);
	CSimpleCharacterNode * FindSimpleCharacter(DWORD nID);
    CSimpleCharacterNode * FindSimpleCharacter(const char* name);
	CSimpleCharacterNode * FindSimpleCharacter(int x,int y);

	inline MiniChar& GetMiniChar()		{ return m_MiniChar; } // Add by Yuan
	inline MiniGood& GetMiniGood()		{ return m_MiniGood; } // Add by Yuan
	void DeleteGood(DWORD nID);			// 删除物品编号为nID的物品
	void DeleteCharacter(DWORD nID);		// 删除角色编号为nID的物品
	void DeleteAllXYGoods(int x,int y);				// 删除地图上所有坐标为x,y的物品
	void DeleteAllXYCharacters(int x,int y);			// 删除地图上所有坐标为x,y的角色
	void ResetAllCharacters();

protected: 
	inline long hashIDfunc(DWORD nID) { return nID % IDMAXNUMBER; }
	CSimpleGoodNode * AddSimpleGood(DWORD nID);
	CSimpleCharacterNode * AddSimpleCharacter(DWORD nID);

private:
	CSimpleCharacterNode	m_arrID[IDMAXNUMBER];		// 角色hashtable
	CSimpleGoodNode			m_arrIDgood[IDMAXNUMBER];   // 物品hashtable

	_Data m_arrMap[MAPARR_WIDTH][MAPARR_WIDTH];  /// 地图数组

	MiniChar m_MiniChar; // Add by Yuan，地图中角色的链表
	MiniGood m_MiniGood; // 地图中物品的链表
};
