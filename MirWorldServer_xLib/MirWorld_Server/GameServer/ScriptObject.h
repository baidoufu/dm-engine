#pragma once
#include <array>
class CSe_Page;
class CSe_Goods;

typedef struct tagScriptVersion
{
	tagScriptVersion()
	{
		FILLSELF(0);
	}
	union
	{
		struct
		{
			BYTE v1;
			BYTE v2;
			BYTE v3;
			BYTE v4;
		};
		DWORD dwVersion;
	};
}ScriptVersion;

class CScriptObject :
	public xVariableProvider
{
public:
	CScriptObject(VOID);
	virtual ~CScriptObject(VOID);
	BOOL Load(const char* pszFilename);
	VOID Destroy();
	CSe_Page* GetPage(const char* pszPage);

	const char* getName() { return m_szName.data(); }
	ScriptVersion& getVersion() { return m_Version; }
	CSe_Goods* getGoodsList() { return m_pGoodsList; }
	BOOL IsItemTradeble(BYTE btStdMode);
	BOOL Reload();
	BOOL IsBigBox()const { return m_bBigBox; }
	CScriptObject* GetLeftPage();
private:
	std::array<char, 1024> m_szFileName{};
	VOID AddTradeItemType(BYTE btType);
	xStringList<32> m_xPageList;
	xVarList<32> m_xVarList;
	CSe_Page* m_pPageList;
	CSe_Goods* m_pGoodsList;
	ScriptVersion m_Version;
	std::array<char, 64> m_szName{};
	std::array<BYTE, 256> m_btTradeItemType{};
	int	 m_iTradeItemCount{ 0 };
	BOOL m_bBigBox{ FALSE };
	std::array<char, 256> m_szLeftPage{};
	CScriptObject* m_pLeftPage;
};