#pragma once

#include "localdefine.h"
#include <array>

/**
 *  MarketComponent — 玩家摆摊数据
 *  纯数据组件, 摆摊逻辑由 HumanPlayer.cpp 管理
 */
struct MarketComponent
{
	int										ItemCount = 0;
	std::array<PrivateShopItemCache, 10>	ShopCache{};
	std::array<char, 64>					ShopName{};
	WORD									ShopStyle = 0;
	WORD									ShopFlags = 0;
	DWORD									ShopSign = 0;

	MarketComponent() = default;
};
