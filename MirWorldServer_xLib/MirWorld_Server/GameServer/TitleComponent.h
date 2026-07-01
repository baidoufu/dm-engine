#pragma once

#include <array>

/**
 *  TitleComponent — 玩家称号数据
 *  纯数据组件, 称号逻辑由 HumanPlayer.cpp 管理
 */
struct TitleComponent
{
	std::array<char, 200>	CurrentTitle{}; // 当前称号
	int						CurrentTitleIndex = 0; // 称号ID

	TitleComponent() = default;
};
