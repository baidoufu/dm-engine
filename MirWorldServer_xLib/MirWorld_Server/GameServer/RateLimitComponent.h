#pragma once
#include "ECS.h"

/**
 *  RateLimitComponent — 频率限制
 *
 *    1 个 RateLimitComponent = 统一管理所有操作频率限制
 *    新增限制只需加枚举值, 无需改动类定义
 */
struct RateLimitComponent
{
    enum Action : uint8_t
    {
        ACT_SPECIAL_ATTACK = 0,  // 战士技能攻击
        ACT_MINE,                 // 挖矿
        ACT_RELIVE,               // 复活
        ACT_USE_ITEM,             // 使用物品
        ACT_PICKUP_ITEM,          // 拾取物品
        ACT_DROP_ITEM,            // 丢弃物品
        ACT_EQUIP_CHANGE,         // 装备穿脱
        ACT_COUNT                 // 总数标记
    };

    // 默认间隔(毫秒)
    static constexpr int DEFAULT_INTERVALS[ACT_COUNT] = {
        800,   // ACT_SPECIAL_ATTACK
        800,   // ACT_MINE
        1000,  // ACT_RELIVE
        300,   // ACT_USE_ITEM
        300,   // ACT_PICKUP_ITEM
        300,   // ACT_DROP_ITEM
        300    // ACT_EQUIP_CHANGE
    };

    int intervals[ACT_COUNT];  // 各项间隔 (运行时配置, 支持动态修改)
    int lastTimes[ACT_COUNT];  // 各项上次执行时间戳 (毫秒)

    RateLimitComponent()
    {
        for (int i = 0; i < ACT_COUNT; ++i)
        {
            intervals[i] = DEFAULT_INTERVALS[i];
            lastTimes[i] = 0;
        }
    }

    // 检查并更新: 可执行返回 true 并记录时间, 否则返回 false
    bool TryExecute(Action act, int nowMs)
    {
        int idx = static_cast<int>(act);
        if (nowMs - lastTimes[idx] >= intervals[idx])
        {
            lastTimes[idx] = nowMs;
            return true;
        }
        return false;
    }

    // 仅查询不更新
    bool CanExecute(Action act, int nowMs) const
    {
        int idx = static_cast<int>(act);
        return (nowMs - lastTimes[idx]) >= intervals[idx];
    }

    // 动态调整间隔
    void SetInterval(Action act, int ms)
    {
        intervals[static_cast<int>(act)] = ms;
    }
};
