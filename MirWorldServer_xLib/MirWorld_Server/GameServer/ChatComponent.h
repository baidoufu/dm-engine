#pragma once

#include "localdefine.h"
#include <array>

/**
 *  ChatComponent — 玩家聊天状态数据
 *  纯数据组件, 聊天逻辑由 HumanPlayer.cpp 管理
 */
struct ChatComponent
{
	std::array<BOOL, CCH_MAX>	DisabledChannels{};	// 频道禁用表
	e_chatchannel				CurrentChannel = CCH_NORMAL;	// 当前聊天频道
	std::array<char, 32>		CurWisperTarget{};	// 当前私聊目标
	BYTE						ChatColor = 1;		// 聊天颜色 [B类迁移]

	ChatComponent() = default;
};
