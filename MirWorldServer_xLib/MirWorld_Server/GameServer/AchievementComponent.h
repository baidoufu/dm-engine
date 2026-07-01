#pragma once

#include "TimeAchieve.h"

/**
 *  AchievementComponent — 玩家成就数据
 *  纯数据组件, 成就逻辑由 Human_Achieve.cpp 管理
 */
struct AchievementComponent
{
	AchievementData		Data;

	AchievementComponent() = default;
};
