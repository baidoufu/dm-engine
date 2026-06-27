#pragma once
#include "ECS.h"
#include <array>

/**
 *  TimerComponent.h — 定时器组件
 *
 *  实体归一:
 *    一个玩家 = 一个实体, 挂 1 个 PlayerTimerComponent
 *    一个生物 = 一个实体, 挂 1 个 AliveTimerComponent
 *    组件连续存储, 缓存友好; ecs_view 联合查询可正常工作。
 *
 *  设计原则:
 *    - 组件为 POD 纯数据, 逻辑上移到 System
 *    - lastTickMs 为唯一持久状态(上次触发时间戳, 毫秒)
 *    - 创建时由 System 统一 fill 为当前帧时间, 避免首次立即触发
 */

// 定时器类型枚举
enum class TimerType : uint8_t
{
    // === CHumanPlayer 定时器 (索引 0~16, 由 PlayerTimerComponent 管理) ===
    TMR_DB_SAVE = 0,          // 数据库保存检查
    TMR_STAMINA,              // 精力值检查
    TMR_GAME_TIME,            // 时长区计时
    TMR_PK,                   // PK值衰减
    TMR_JUST_PK,              // 刚刚PK状态
    TMR_PK_POINT,             // PK点计时
    TMR_FENGHAO,              // 封号计时
    TMR_HORSE,                // 骑马计时
    TMR_ADD_TO_GUILD,         // 加入行会计时
    TMR_CHAT_NORMAL,
    TMR_CHAT_CRY,
    TMR_CHAT_WISPER,
    TMR_CHAT_GROUP,
    TMR_CHAT_GUILD,
    TMR_CHAT_COUPLE,
    TMR_CHAT_GM,
    TMR_CHAT_FRIEND,
    // === CAliveObject 定时器 (由 AliveTimerComponent 管理) ===
    TMR_ACTION,
    TMR_HP_RECOVER,
    TMR_MP_RECOVER,
    TMR_COUNT                 // 总数标记 (20)
};

// CHumanPlayer 定时器槽位数
constexpr size_t PLAYER_TMR_COUNT = 17;

// CAliveObject 定时器槽位数
constexpr size_t ALIVE_TMR_COUNT = 3;

/**
 *  PlayerTimerComponent — 玩家定时器组件
 *
 *  lastTickMs[i] 对应 TimerType i (0= TMR_DB_SAVE, ..., 16= TMR_CHAT_FRIEND)。
 *  逻辑(到期检查/前推)由 PlayerTimerSystem 内联执行, 组件保持纯数据。
 */
struct PlayerTimerComponent
{
    std::array<int, PLAYER_TMR_COUNT> lastTickMs{};

    PlayerTimerComponent() = default;
};

/**
 *  AliveTimerComponent — 生物定时器组件
 *
 *  玩家实体也会挂载此组件(玩家继承自 CAliveObject, 需要 ACTION/HP/MP 定时器)。
 *  槽位映射: 0=TMR_ACTION, 1=TMR_HP_RECOVER, 2=TMR_MP_RECOVER。
 */
struct AliveTimerComponent
{
    std::array<int, ALIVE_TMR_COUNT> lastTickMs{};

    AliveTimerComponent() = default;
};

// CAliveObject 定时器类型 → 槽位索引转换 (供 AliveTimerSystem 使用)
inline int AliveTimerTypeToIdx(TimerType t)
{
    switch (t)
    {
    case TimerType::TMR_ACTION:     return 0;
    case TimerType::TMR_HP_RECOVER: return 1;
    case TimerType::TMR_MP_RECOVER: return 2;
    default: return -1;
    }
}

// CHumanPlayer 定时器类型 → 槽位索引转换 (即 TimerType 的整数值, 0~16)
inline int PlayerTimerTypeToIdx(TimerType t)
{
    int idx = static_cast<int>(t);
    return (idx >= 0 && idx < static_cast<int>(PLAYER_TMR_COUNT)) ? idx : -1;
}

// 聊天频道 → 定时器类型映射
inline TimerType ChatChannelToTimerType(int channel)
{
    switch (channel)
    {
    case 0: return TimerType::TMR_CHAT_NORMAL;
    case 1: return TimerType::TMR_CHAT_CRY;
    case 2: return TimerType::TMR_CHAT_WISPER;
    case 3: return TimerType::TMR_CHAT_GM;
    case 4: return TimerType::TMR_CHAT_GROUP;
    case 5: return TimerType::TMR_CHAT_GUILD;
    case 6: return TimerType::TMR_CHAT_COUPLE;
    case 7: return TimerType::TMR_CHAT_FRIEND;
    default: return TimerType::TMR_CHAT_NORMAL;
    }
}
