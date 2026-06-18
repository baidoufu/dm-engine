#pragma once
#include <array>
struct gm_node
{
	std::array<char, 32> szAccount{};
	int	level = 0;
};

class CSe_Page;

typedef struct tagGameCommand
{
	tagGameCommand()
	{
		FILLSELF(0);
	}
	//	CSe_Page * pPage;
	StringCacheNode* pPage;
	fnCommandProc proc;
	BOOL fIsCallPage;
	BOOL fIsGmCmd;
	UINT nLimitLevel;
}GameCommand;

class CGmManager : public xSingletonClass<CGmManager>
{
public:
	CGmManager(VOID);
	virtual ~CGmManager(VOID);
	BOOL Load(const char* pszFile);
	VOID Save(const char* pszFile);
	int	GetGmLevel(const char* pszAccount);
	BOOL LoadCommandDef(const char* pszFile);
	BOOL ExecGameCmd(const char* pszCommand, CHumanPlayer* pPlayer);
	BOOL MapCommand(int iLevel, const char* pszCommand, const char* pszBuildInCommand);
private:
	VOID ClearCmdList();
	// ·ÖÅä×Ö·û´®»º´æ
	StringCacheNode* AllocStringCache();
	// ÊÍ·Å×Ö·û´®»º´æ
	VOID FreeStringCache(StringCacheNode* pStringCahce);
private:
	xStringList<512> m_xCmdList;
	xObjectPool<gm_node> m_xGmNodePool;
	xObjectPool<StringCacheNode> m_xStringCachePool;
	CNameHash m_GmHash;
};