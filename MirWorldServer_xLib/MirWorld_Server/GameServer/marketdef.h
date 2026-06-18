#pragma once
#include <array>
class CMarket;
class CSubMarket;
typedef struct tagMarketItem
{
	tagMarketItem()
	{
		FILLSELF(0);
	}
	UINT nSellCount;
	UINT nId;
	UINT nImage;
	UINT nShowImage;
	std::array<char, 64> szName{};
	std::array<char, 64> szItemName{};
	WORD wCount;
	WORD wPrice;
	BOOL bBind;
	char* pszTips;
	ITEM itemTemplate;
	UINT nMarketId;
	UINT nSubMarketId;
}MarketItem;