#pragma once

/**
 *  FenghaoComponent — 玩家时长封号数据
 *  纯数据组件, 封号逻辑由 Human_FengHao.cpp 管理
 */
struct FenghaoComponent
{
	FenghaoInfo		Info;

	FenghaoComponent()
	{
		memset(&Info, 0, sizeof(Info));
	}
};
