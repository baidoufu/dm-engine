#pragma once
#include "ECS.h"
#include <functional>

/**
 *  TimerComponent — 统一管理-公共组件
 *
 *  设计原则:
 *    - 每个 TimerComponent 代表一个独立的定时器实例
 *    - 支持通过 PlayerTimerSystem::CheckTimer / AliveTimerSystem::CheckTimer 内联查询
 *    - 也支持通过 CHumanPlayer::CheckTimer / CAliveObject::CheckTimer 便捷访问
 *    - 支持循环定时器(intervalMs > 0)
 *    - CheckAndAdvance() 方法内置到期判断 + 自动前推逻辑，Manager 只需一行委托
 *
 *  用法:
 *    // 内联检查 (在 CHumanPlayer::Update() 中)
 *    if (CheckTimer(TimerType::TMR_DB_SAVE, dwInterval))
 *    {
 *        UpdateToDB();
 *    }
 */

// 定时器类型枚举 — 对应原有的各个 CServerTimer 成员
enum class TimerType : uint8_t
{
    // === CHumanPlayer 定时器 ===
    TMR_DB_SAVE = 0,          // 原 m_DBTimer — 数据库保存检查
    TMR_STAMINA,               // 原 m_StaminaTimer — 精力值检查
    TMR_GAME_TIME,             // 原 m_tmrGameTime — 时长区计时
    TMR_PK,                    // 原 m_tmrPkTimer — PK值衰减
    TMR_JUST_PK,              // 原 m_tmrJustPk — 刚刚PK状态
    TMR_PK_POINT,              // 原 m_tmrPkPointTimer — PK点计时
    TMR_FENGHAO,              // 原 m_tmrFenghaoTime — 封号计时
    TMR_HORSE,                // 原 m_HorseTimer — 骑马计时
    TMR_ADD_TO_GUILD,          // 原 m_AddToGuildTimer — 加入行会计时
    TMR_CHAT_NORMAL,          // 原 m_ChatChannelTimer[CCH_NORMAL]
    TMR_CHAT_CRY,             // 原 m_ChatChannelTimer[CCH_CRY]
    TMR_CHAT_WISPER,          // 原 m_ChatChannelTimer[CCH_WISPER]
    TMR_CHAT_GROUP,           // 原 m_ChatChannelTimer[CCH_GROUP]
    TMR_CHAT_GUILD,           // 原 m_ChatChannelTimer[CCH_GUILD]
    TMR_CHAT_COUPLE,          // 原 m_ChatChannelTimer[CCH_COUPLE]
    TMR_CHAT_GM,              // 原 m_ChatChannelTimer[CCH_GM]
    TMR_CHAT_FRIEND,          // 原 m_ChatChannelTimer[CCH_FRIEND]
    // === CAliveObject 定时器 ===
    TMR_ACTION,
    TMR_HP_RECOVER,
    TMR_MP_RECOVER,
    TMR_COUNT                 // 总数标记 (20)
};

// 定时器回调函数类型: 参数为玩家ID和定时器参数
using TimerCallback = std::function<void(UINT ownerId, int param1)>;

struct TimerComponent
{
    TimerType  typeId     = TimerType::TMR_DB_SAVE;  // 定时器类型
    int        intervalMs = 0;    // 间隔(毫秒), 0=一次性
    int        lastTickMs = 0;    // 上次触发时间戳(毫秒)
    int        param1     = 0;    // 附加参数
    UINT       ownerId    = 0;    // 所属玩家ID (用于回调派发)

    TimerComponent() = default;

    // 便利方法: 检查是否到期 (纯查询, 不修改状态)
    inline BOOL IsTimeOut(DWORD intervalMs, int now) const
    {
        return GetTimeToTime(lastTickMs, now) >= intervalMs;
    }

    // 检查到期并自动前推 (自服务: 组件自己处理逻辑)
    // 返回 TRUE 表示到期, 同时自动前推 lastTickMs 避免累积漂移
    inline BOOL CheckAndAdvance(DWORD intervalMs, int now)
    {
        if (GetTimeToTime(lastTickMs, now) >= intervalMs)
        {
            lastTickMs += intervalMs;
            return TRUE;
        }
        return FALSE;
    }
};

// 聊天频道 → 定时器类型映射 (用于替代 m_ChatChannelTimer[channel])
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
