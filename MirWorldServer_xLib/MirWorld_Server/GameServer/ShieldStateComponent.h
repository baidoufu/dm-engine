#pragma once

/**
 *  ShieldStateComponent — 替代 CHumanPlayer 中 6 个护盾/免伤相关散落成员变量
 *
 *  改造前 (6个散落成员):
 *    int  HushenBuffdamage;   // 护身/金刚受到的累计伤害
 *    int  ResMag_Count;       // 魔法盾抵抗次数
 *    int  NoDamage;           // 魔法盾免伤百分比
 *    int  JingganNoDamage;    // 金刚护体免伤百分比
 *    int  hushenleve;         // 护身等级索引
 *    BOOL boPoison;           // 施毒/诅咒类型切换
 *
 *  改造后:
 *    1 个 ShieldStateComponent = 统一管理所有护盾/免伤状态
 */
struct ShieldStateComponent
{
    int hushenBuffDamage  = 0;   // 原 HushenBuffdamage  — 护身/金刚受到的累计伤害
    int magShieldResCount = 0;   // 原 ResMag_Count      — 魔法盾剩余抵抗次数
    int magShieldNoDamage = 0;   // 原 NoDamage           — 魔法盾免伤百分比
    int jingangNoDamage   = 0;   // 原 JingganNoDamage    — 金刚护体免伤百分比
    int hushenLevel       = 0;   // 原 hushenleve         — 护身/金刚等级索引
    int poisonToggle      = 0;   // 原 boPoison           — 施毒/诅咒类型切换 (0/1)
};
