#pragma once
#include "aliveobject.h"
#include "scriptshell.h"
#include "NpcComponentManager.h"
class CScriptPage;
class CHumanPlayer;
struct tagGoods;

class CScriptNpc :
	public CAliveObject,
	public CScriptShell
{
public:
	CScriptNpc(VOID);
	virtual ~CScriptNpc(VOID);
	BOOL Init(UINT dbid, const char* pszName, int view, int x, int y, DWORD mapid, CScriptObject* pScriptObject);
	VOID QueryTalk(CHumanPlayer* pPlayer);
	VOID QuerySelectLink(CHumanPlayer* pPlayer, const char* pLink, BOOL bHumanQuery = TRUE);

	//VOID QueryGoodsItemList(CHumanPlayer* pPlayer, const char* pTemplate);
	bool containschar(const std::string& str);

	e_object_type GetType() { return OBJ_NPC; }
	e_class_type GetClassType() { return CLS_NPC; }
	DWORD GetFeather() {
		return MAKEFEATHER(0, GetView(), 0, 0x32);
	}
	DWORD GetHealth() {
		return 0x00640064;
	}
	const char* GetName() { return m_szName.data(); }
	const char* GetViewName() { return m_szLongName.data(); }
	BOOL IsNPC()const { auto* st = GetNpcState(); return st ? st->bIsNpc : FALSE; }
	BOOL CanMove() { return FALSE; }
	BOOL AddItem(ITEM& item);
	VOID Update();
	BOOL BeAttack(CAliveObject* pAttacker, int nDamage);

	DWORD GetItemSellPrice(ITEM& item);
	DWORD GetItemBuyPrice(ITEM& item);
	DWORD GetItemRepairPrice(CHumanPlayer* pPlayer, ITEM& item);
	BOOL RepairItem(CHumanPlayer* pPlayer, ITEM& item);
	BOOL SellItem(CHumanPlayer* pPlayer, ITEM& item);
	BOOL BuyItem(CHumanPlayer* pPlayer, const char* pszName, DWORD dwMakeIndex, DWORD& dwErrorCode);
	BOOL QueryItemList(CHumanPlayer* pPlayer, const char* pszItemName, int ptr);
	VOID SetTalk(BOOL isTalk){ auto* st = GetNpcState(); if (st) st->bTalk = isTalk; }
	BOOL IsTalk()const { auto* st = GetNpcState(); return st ? st->bTalk : FALSE; }
	VOID SetDistance(UINT nDistance) { auto* st = GetNpcState(); if (st) st->nDistance = nDistance; }
	UINT GetDistance()const { auto* st = GetNpcState(); return st ? st->nDistance : 8; }
	VOID SetBuyPercent(FLOAT fPercent)
	{
		auto* mc = GetNpcMerchant(); if (mc) mc->fBuyPercent = fPercent;
	}
	FLOAT GetBuyPercent()const { auto* mc = GetNpcMerchant(); return mc ? mc->fBuyPercent : 1.0f; }

	VOID SetSellPercent(FLOAT fPercent)
	{
		auto* mc = GetNpcMerchant(); if (mc) mc->fSellPercent = fPercent;
	}
	FLOAT GetSellPercent()const { auto* mc = GetNpcMerchant(); return mc ? mc->fSellPercent : 0.5f; }
	DWORD GetItemValue(ITEM& item, DWORD dwBasePrice);

	VOID SaveItems();
	VOID LoadItems();

	VOID SetLongName(const char* szLongName)
	{
		o_strncpy(m_szLongName.data(), szLongName, 127);
	}

	VOID OnEnterMap(CLogicMap* pMap);

	VOID SetView(int nView) { auto* st = GetNpcState(); if (st) st->nView = nView; }
	int GetView()const { auto* st = GetNpcState(); return st ? st->nView : 0; }
	const char* GetTitleName() { return GetName(); }
	UINT GetTitleId() { return GetId(); }
	UINT GetStoreId()const { auto* st = GetNpcState(); return st ? st->StoreId : 0; }
	VOID Clean();
	//CScriptPage * GetPage(){ return m_pPage;}
	BOOL IsSandCityMerchant()const { auto* st = GetNpcState(); return st ? st->fSandCityMerchant : FALSE; }

	VOID SendMerChantJsonMsg(CScriptTarget* pTarget, const char* pWords, UINT nType);
	VOID SendDayExpMain(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendDayExpHelp(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendActivityMain(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendCreateGuildHelp(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendCustomUIWnd(CHumanPlayer* pPlayer, const char* pWords);

	// ========== ECS 组件访问器 (统一入口, 对标 CMonsterEx) ==========
	inline NpcStateComponent* GetNpcState()
	{
		return NpcComponentManager::GetInstance()->GetNpcState(this);
	}
	inline const NpcStateComponent* GetNpcState() const
	{
		return NpcComponentManager::GetInstance()->GetNpcState(const_cast<CScriptNpc*>(this));
	}
	inline NpcMerchantComponent* GetNpcMerchant()
	{
		return NpcComponentManager::GetInstance()->GetNpcMerchant(this);
	}
	inline const NpcMerchantComponent* GetNpcMerchant() const
	{
		return NpcComponentManager::GetInstance()->GetNpcMerchant(const_cast<CScriptNpc*>(this));
	}
	inline VOID EnsureMerchantComponent()
	{
		NpcComponentManager::GetInstance()->EnsureMerchantComponent(this);
	}
protected:
	NpcGoodsList* FindGoodsList(ITEM& item);
	NpcGoodsList* FindGoodsList(const char* pszName);
	VOID SendClosePage(CHumanPlayer* pPlayer);
	VOID SendGoodsList(CHumanPlayer* pPlayer);

	VOID DeleteNpcGoodsList(NpcGoodsList* pList);
	VOID DeleteNpcGoodsItemList(NpcGoodsList* pList, NpcGoodsItemList* pItemList);
	BOOL InitGoods(tagGoods* pGoodsList);

	// === 保留字段: 非POD或身份字符串，不走ECS ===
	std::array<char, 32> m_szName;
	CServerTimer m_tmrUpdateItem;
};
