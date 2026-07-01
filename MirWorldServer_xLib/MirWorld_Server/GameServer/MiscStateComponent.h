#pragma once

#include <array>

/**
 *  MiscStateComponent — 玩家杂项状态数据
 *  纯数据组件
 */
struct MiscStateComponent
{
	CSystemTime					LoginTime;
	DWORD						MineCounter			= 0;
	int							MaterialBagPos		= -1;
	FLOAT						ExpFactor			= 1.0f;
	UINT						CutMonsterId		= 0;
	UINT						CutTimes			= 0;
	std::array<DWORD, 4>		Params{};				// 通用参数(GetParam/SetParam)
	std::array<char, 256>		TempScriptVarValue{};	// 临时脚本变量缓存

	MiscStateComponent() = default;
};
