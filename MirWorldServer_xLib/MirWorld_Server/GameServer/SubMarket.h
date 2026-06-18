#pragma once
#include <memory>
class CHumanPlayer;

class CSubMarket
{
public:
	CSubMarket(UINT nMarketId, UINT nId, const char* pszName);
	virtual ~CSubMarket(VOID);
	UINT GetId()const{return m_Id;}
	const char* GetName(){return m_pszName.get() ? m_pszName.get() : "";}
	BOOL LoadSubMarket(const char* pszFilename);
	BOOL InitItemArray(UINT nSize);
	BOOL AddItem(UINT nImage, UINT nShowImage, const char* pszViewName, const char* pszItemName, UINT nPrice, UINT nCount, BOOL bBind, const char* pszTips);
	VOID QueryItems(CHumanPlayer* pPlayer, UINT nMarketId);
protected:
	char* ProcItemTips(const char* pszTips);
	UINT m_Id;
	pooled_string_ptr m_pszName;
	std::unique_ptr<MarketItem*[]> m_pItemArray;
	UINT m_nItemCount;
	UINT m_nMarketId;
};