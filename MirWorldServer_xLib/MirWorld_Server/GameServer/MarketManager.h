#pragma once
#include <array>
class CMarket;
class CHumanPlayer;

class CMarketManager :
	public xSingletonClass<CMarketManager>
{
public:
	CMarketManager(VOID);
	virtual ~CMarketManager(VOID);

	const char* GetMarketScrollText() { return m_pszScrollText.get(); }

	MarketItem* getItem(UINT nId);
	VOID LoadScrollText(const char* pszFilename);

	BOOL LoadMarkets(const char* pszFilename);

	CMarket* AddMarket(UINT nId);
	CMarket* GetMarket(UINT nMarketId);
	MarketItem* newItem();
	VOID deleteItem(MarketItem* pItem);

	VOID OpenMarket(CHumanPlayer* pPlayer);
	VOID QueryMarket(CHumanPlayer* pPlayer, UINT nMarketId);
	VOID QueryItemTips(CHumanPlayer* pPlayer, UINT nItemId);

	BOOL QueryBuyItem(CHumanPlayer* pPlayer, UINT nItemId);

	VOID OnClientMsg(CHumanPlayer* pPlayer, WORD wCmd, WORD wParam1, WORD wParam2, char* pszData, int iDatasize);
protected:
	CIndexListEx<MarketItem> m_xItemList;
	std::unique_ptr<char[]> m_pszScrollText;

	std::array<CMarket*, 32> m_pMarketArray{};
	UINT m_nMarketCount;
};