#pragma once

/**
 *  ZhenBaoComponent — 玩家珍宝/元气数据 (B类)
 *
 *  替代 CHumanPlayer 中的 m_dwZhenBaoExpMax / m_dwZhenBaoStar / m_wYuanQi / m_bYuanQi
 *  纯数据组件, 珍宝/元气逻辑由 HumanPlayer.cpp 管理
 */
struct ZhenBaoComponent
{
	DWORD	ZhenBaoExpMax	= 0;	// 珍宝最大经验
	DWORD	ZhenBaoStar		= 0;	// 珍宝星级
	WORD	YuanQi			= 0;	// 当前元气值
	BOOL	IsYuanQiFull	= FALSE;// 是否触发元气

	ZhenBaoComponent() = default;
};
