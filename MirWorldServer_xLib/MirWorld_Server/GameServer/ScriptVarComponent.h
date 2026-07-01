#pragma once

#include <array>

typedef std::array<char, 128> S_PARAM;
typedef DWORD V_PARAM;

/**
 *  ScriptVarComponent — 玩家脚本变量数据
 *  纯数据组件, 脚本逻辑由外部 ScriptFunction 管理
 */
struct ScriptVarComponent
{
	std::array<S_PARAM, 10>	StringParams{};	// 个人字符串变量
	std::array<V_PARAM, 10>	ValueParams{};	// 个人数值变量

	ScriptVarComponent() = default;
};
