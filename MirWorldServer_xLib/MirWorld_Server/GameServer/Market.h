#pragma once
#include <array>
class CSubMarket;
class CHumanPlayer;

class CMarket
{
public:
	CMarket(UINT id);
	virtual ~CMarket(VOID);
	CSubMarket* AddSubMarket(UINT nId, const char* pszName);
	CSubMarket* GetSubMarket(UINT nId);
	UINT GetId()const { return m_Id; }

	VOID SendSubMarket(CHumanPlayer* pPlayer);
protected:
	std::array<CSubMarket*, 32> m_pSubMarketArray{};
	UINT m_nSubMarketCount;
	UINT m_Id;
};