#pragma once

/**
 *  ShieldStateComponent — 替代玩家相关散落成员变量
 *    1 个 ShieldStateComponent = 统一管理
 */
struct ShieldStateComponent
{
    int hushenBuffDamage  = 0;   // — 护身/金刚受到的累计伤害
    int magShieldResCount = 0;   // — 魔法盾剩余抵抗次数
    int magShieldNoDamage = 0;   // — 魔法盾免伤百分比
    int jingangNoDamage   = 0;   // — 金刚护体免伤百分比
    int hushenLevel       = 0;   // — 护身/金刚等级索引
    int poisonToggle      = 0;   // — 施毒/诅咒类型切换 (0/1)
};
