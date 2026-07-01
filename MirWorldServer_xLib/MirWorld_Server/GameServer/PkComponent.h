#pragma once

/**
 *  PkComponent — 玩家PK状态数据
 *  纯数据组件, PK逻辑由 HumanPlayer.cpp 管理
 */
struct PkComponent
{
	DWORD		PkValue = 0;	// PK值
	BOOL		JustPk = FALSE;	// 刚刚PK标志

	PkComponent() = default;
};
