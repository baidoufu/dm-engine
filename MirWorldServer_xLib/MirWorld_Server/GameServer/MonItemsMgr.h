#pragma once
#include "findfile.h"
#include <array>

typedef struct tagDownItem
{
	tagDownItem()
	{
		FILLSELF(0);
	}
	BOOL updatecycle()
	{
		nCur++;
		if (nCur >= nCycleMax)
		{
			if (nMax < 5)
				nCycleMax = nMax;
			else
				nCycleMax = GetRangeRand(nMin, nMax);
			nCur = 0;
			return TRUE;
		}
		return FALSE;
	}
	std::array<char, 32> szName{};
	int	nCount;
	int nCountMax;
	int	nMax;
	int	nMin;
	int	nCur;
	int	nCycleMax;
	bool bRandomUpgradeItem; // 是否按自定义随机物品属性.
	bool bGold;
	std::array<BYTE, 2> btFlag{};
	tagDownItem* pNext;
}DOWNITEM;

typedef struct tagMonItems
{
	tagMonItems()
	{
		FILLSELF(0);
	}
	DOWNITEM* pItems;
	std::array<char, 32> szMonName{};
	std::array<char, 256> szFilename{};
}MONITEMS;

class CMonItemsMgr : public CFindFile, public xSingletonClass<CMonItemsMgr>
{
public:
	CMonItemsMgr(VOID);
	virtual ~CMonItemsMgr(VOID);
	BOOL LoadMonItems(const char* pszPath);
	MONITEMS* GetMonItems(const char* pMonsterName)
	{
		return (MONITEMS*)m_MonItemsHash.HGet(pMonsterName);
	}
	VOID OnFoundFile(const char* pszFilename, UINT nParam1 = 0);
	BOOL CreateDownItem(DOWNITEM* pDownItem, ITEM& item);
private:
	CNameHash m_MonItemsHash;
};