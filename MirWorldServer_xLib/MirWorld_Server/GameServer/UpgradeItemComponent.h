#pragma once

/**
 *  UpgradeItemComponent — 武器升级中的物品 (B类)
 *
 *  替代 CHumanPlayer 中的 m_UpgradeItem
 *  仅 UpgradeWeapon/TakeUpgradeWeapon/UpdateItemsToDB 使用
 */
struct UpgradeItemComponent
{
	ITEM		Item;	// 升级中的武器

	UpgradeItemComponent()
	{
		memset(&Item, 0, sizeof(Item));
	}
};
