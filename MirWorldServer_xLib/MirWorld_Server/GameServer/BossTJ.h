#pragma once
#include <array>

constexpr int MAXBOSSTJ = 50;  // 最大BOSSTJ数量

typedef struct tagBossItem
{
	tagBossItem()
	{
		FILLSELF(0);
	}
	std::array<char, 64> szShowName;
	std::array<char, 64> szName;
	tagBossItem* pNext;
} BOSSITEM;

typedef struct tagBoss_TJ
{
	tagBoss_TJ()
	{
		FILLSELF(0);
	}
	std::array<char, 64> szName;
	int nCate;
	int nEnable;
	std::array<char, 256> szTime;
	BYTE nTime;
	int nTimeType;
	int nLvRecomm;
	int nEnterLv;
	std::array<char, 64> szMap;
	int nFreshTime;
	int nPic;
	int nMoveBtn;
	std::array<char, 256> szOpenDesc;
	std::array<char, 512> szDesc;
	BOSSITEM* pItems;
} BOSS_TJ;

class CBossTJ : public xSingletonClass<CBossTJ>
{
public:
	CBossTJ(VOID);
	virtual ~CBossTJ(VOID);
	VOID Load(const char* pszPath);
	VOID SendBossList(CHumanPlayer* pPlayer) const;
	VOID SendBoss(CHumanPlayer* pPlayer, const char* pszName);
	VOID Update();
protected:
	VOID Clear();
private:
	CNameHash m_BossTJHash;
	std::array<BOSS_TJ*, MAXBOSSTJ> m_pBossTJList;
	int m_nBossTJCount;
	CServerTimer m_tmrUpdate; // 更新计时器
};