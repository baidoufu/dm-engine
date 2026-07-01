#pragma once
#include <array>

// 前向声明 (避免引入 localdefine.h 的完整依赖)
typedef struct tagNpcGoodsList NpcGoodsList;

/**
 *  NpcStateComponent — NPC 运行时交互状态（纯数据组件）
 *
 *  从 CScriptNpc 中抽取的简单值类型字段，连续存储，缓存友好。
 *  替代 CScriptNpc 中的散落状态变量。
 *  所有字段均为值类型，无指针/引用，生命周期与 NPC 对象一致。
 */
struct NpcStateComponent
{
	// === 外观与身份 ===
	INT  nView = 0;                  // NPC 外观编号
	UINT StoreId = 0;                // 商店唯一标识 (dbid | 0x70000000)

	// === 交互状态 ===
	BOOL  bTalk = FALSE;             // 是否可对话
	BOOL  bIsNpc = FALSE;            // 是否为 NPC
	UINT  nDistance = 8;             // 与玩家交互距离
	BOOL  fSandCityMerchant = FALSE; // 是否沙城商人

	NpcStateComponent() = default;
};

/**
 *  NpcMerchantComponent — NPC 商人数据（纯数据组件）
 *
 *  只有具备商人功能的 NPC 才挂载此组件（如买卖物品、修理）。
 *  普通对话 NPC 不挂载，减少内存占用。
 *
 *  注意: pSellGoodsList 指向 CNpcManager 对象池分配的链表，
 *  生命周期由 CScriptNpc::Clean() / NpcComponentManager::DestroyNpcComponents 管理。
 */
struct NpcMerchantComponent
{
	// === 商人货物链表 (指针由 CNpcManager 对象池管理) ===
	NpcGoodsList* pSellGoodsList = nullptr;

	// === 货物刷新状态 ===
	std::array<BYTE, 100> btSellList{};
	INT   iSellListCount = 0;
	BOOL  fChanged = FALSE;
	DWORD dwTimeOut = 0;

	// === 价格比例 ===
	FLOAT fSellPercent = 0.5f;       // 出售百分比
	FLOAT fBuyPercent = 1.0f;        // 购买百分比

	NpcMerchantComponent() = default;
};
