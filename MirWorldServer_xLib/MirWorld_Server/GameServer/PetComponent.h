#pragma once

/**
 *  PetComponent — 宠物专属数据（纯数据组件）
 *
 *  只有被驯服/召唤的怪物才会挂载此组件。
 *  普通怪物不挂载，减少内存占用。
 */
struct PetComponent
{
	BOOL  bSetOwner = FALSE;          // 是否已设置主人
	DWORD dwBetray = 0;               // 宠物叛变时间(ms)
	BOOL  bGotoOwner = FALSE;         // 是否已设置宠物拾取标志
	UINT  nCurFocusItemId = 0;        // 焦点物品ID指针

	PetComponent() = default;
};
