#pragma once
#include "findfile.h"

class CScriptNpc;
class CMapObject;
class CNpcManager : public xSingletonClass<CNpcManager>
{
public:
	CNpcManager(VOID);
	virtual ~CNpcManager(VOID);
	BOOL Load(const char* pszFilename);

	CScriptNpc* AddNpc(const char* pszString);
	CScriptNpc* GetScriptNpcById(UINT id) { return m_ScriptNpcs.Get(id & 0xffffff); }
	NpcGoodsList* AllocGoodsList() { return m_GoodsLists.newObject(); }
	VOID FreeGoodsList(NpcGoodsList* p)
	{
		*p = NpcGoodsList{};
		m_GoodsLists.deleteObject(p);
	}
	NpcGoodsItemList* AllocGoodsItemList() { return m_GoodsItemLists.newObject(); }
	VOID FreeGoodsItemList(NpcGoodsItemList* p)
	{
		*p = NpcGoodsItemList{};
		m_GoodsItemLists.deleteObject(p);
	}
	INT	getCount() { return m_ScriptNpcs.GetCount(); }
	VOID Update();
	//添加动态NPC
	BOOL AddDynamicNpc(UINT nIdent, const char* pszName, UINT nView, UINT mapid, UINT x, UINT y, const char* pszScript);
	//溢出动态NPC
	BOOL RemoveDynamicNpc(UINT nIdent);
	//获取动态NPC
	CScriptNpc* GetDynamicNpc(UINT nIdent);
private:
	CScriptNpc* NewNpc();
	VOID DelNpc(CScriptNpc* pNpc);
	xListHost<CMapObject> m_xDynamicNpcList;
	xPtrQueue<CScriptNpc> m_xQNpcs;
	CIndexListEx<CScriptNpc> m_ScriptNpcs;
	xObjectPool<NpcGoodsList> m_GoodsLists;
	xObjectPool<NpcGoodsItemList> m_GoodsItemLists;
};