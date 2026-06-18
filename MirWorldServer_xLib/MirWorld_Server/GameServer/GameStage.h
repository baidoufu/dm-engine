#pragma once
#include <array>
#include <memory>

constexpr int MAXGAMESTAGE = 50;  // 最大游戏阶段数量

typedef struct tagStageItem
{
	tagStageItem()
	{
		FILLSELF(0);
	}
	int nLooks;
	std::array<char, 128> szDesc;
	tagStageItem* pNext;
} STAGEITEM;

typedef struct tagStageContent
{
	tagStageContent()
	{
		FILLSELF(0);
	}
	std::array<char, 64> szName;
	int nLooks;
	STAGEITEM* pItems;
	tagStageContent* pNext;
} STAGECONTENT;

typedef struct tagStageInfo
{
	tagStageInfo()
	{
		FILLSELF(0);
	}
	int nId;
	std::array<char, 64> szName;
	int nDay;
	int nMaxLevel;
	STAGECONTENT* pContents;
} STAGEINFO;

typedef struct tagStageVar
{
	tagStageVar()
	{
		FILLSELF(0);
	}
	std::array<char, 128> szCurStage;
	std::array<char, 128> szCurDay;
	std::array<char, 128> szSelfLv;
} STAGEVAR;

class CGameStage : public xSingletonClass<CGameStage>
{
public:
	CGameStage(VOID);
	virtual ~CGameStage(VOID);
	VOID Load(const char* pszPath);
	VOID SendPlayerMapJumpHome(CHumanPlayer* pPlayer);
	VOID SendPlayerMapJumpPage(CHumanPlayer* pPlayer, const char* pszName);
protected:
	VOID Clear();
private:
	CNameHash m_StageHash;
	std::array<STAGEINFO*, MAXGAMESTAGE> m_pStageList;
	int m_nStageCount;
	std::unique_ptr<STAGEVAR> m_pStageVar;
};