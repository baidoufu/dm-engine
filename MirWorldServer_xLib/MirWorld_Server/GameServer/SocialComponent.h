#pragma once

#include <array>

// 与 HumanPlayer.h 中 typedef 保持一致
typedef std::array<char, 20> S_CHARNAME;

/**
 *  SocialComponent — 玩家社交关系数据
 *  纯数据组件, 社交逻辑由 human_community.cpp 管理
 */
struct SocialComponent
{
	S_CHARNAME						Wife{};		// 妻子
	S_CHARNAME						Master{};	// 师傅
	std::array<S_CHARNAME, 3>		Students{};	// 3个徒弟
	std::array<S_CHARNAME, 32>		Friends{};	// 32个好友

	SocialComponent() = default;
};
