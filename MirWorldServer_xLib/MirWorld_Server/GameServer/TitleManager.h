#pragma once
#include <memory>
#include <array>
typedef std::array<char, 64> TITLE_NAME;
typedef struct tag_Title
{
	tag_Title()
	{
		FILLSELF(0);
	}
	DWORD dwLevel;
	DWORD dwExp;
	std::array<TITLE_NAME, 3> strTitle{};
}TITLE;
class CHumanPlayer;

// 玩家封号管理
class CTitleManager : public xSingletonClass<CTitleManager>
{
public:
	CTitleManager(VOID);
	virtual ~CTitleManager(VOID);
	// 加载玩家封号标题数据
	VOID LoadData(const char* pszData);
	// 获取封号标题
	BOOL GetTitle(CHumanPlayer* player, char* pszTitle);
	// 获取封号标题
	BOOL GetTitle(CHumanPlayer* player, char* pszTitle, int& index);
private:
	std::unique_ptr<TITLE[]> m_pTitles;
	int	m_iTitleCount;
};