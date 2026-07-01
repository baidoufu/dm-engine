#pragma once
#include "ECS.h"
#include <array>

/**
 *  TimerComponent.h — 统一定时器组件
 *
 *  实体归一:
 *    一个玩家 = 一个实体, 挂 1 个 TimerComponent (含玩家槽位 + 活体槽位)
 *    一个怪物 = 一个实体, 挂 1 个 TimerComponent (含怪物槽位 + 活体槽位)
 *    一个NPC  = 一个实体, 挂 1 个 TimerComponent (含活体槽位)
 *    组件连续存储, 缓存友好; ecs_view 联合查询可正常工作。
 *
 *  设计原则:
 *    - 组件为 POD 纯数据, 逻辑上移到 System/Manager
 *    - lastTickMs 为唯一持久状态(上次触发时间戳, 毫秒)
 *    - 创建时由 Manager 统一 fill 为当前帧时间, 避免首次立即触发
 */

// 定时器类型枚举 (值即槽位索引, 0~22 共 23 槽)
enum class TimerType : uint8_t
{
	// === 玩家定时器 (索引 0~16, 共 17 槽) ===
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
	// === 通用活体定时器 (索引 17~19, 共 3 槽, 玩家+怪物+NPC 共享) ===
	TMR_ACTION,
	TMR_HP_RECOVER,
	TMR_MP_RECOVER,
	// === 怪物专属定时器 (索引 20~22, 共 3 槽, 仅怪物) ===
	TMR_IDLE,                 // 怪物空闲计时(变身条件)
	TMR_BODY,                 // 尸体消失计时
	TMR_BETRAY,               // 宠物叛变计时
	TMR_COUNT                 // 总数标记 (=23)
};

// ==========================================
//  TimerComponent — 统一定时器组件
//  槽位数: TMR_COUNT (23)
//  所有实体类型(玩家/怪物/NPC)共享此组件类型
// ==========================================
struct TimerComponent
{
	std::array<int, static_cast<size_t>(TimerType::TMR_COUNT)> lastTickMs{};
	TimerComponent() = default;
};

// TimerType 值即槽位索引, 直接转换
inline int TimerTypeToIdx(TimerType t)
{
	int idx = static_cast<int>(t);
	return (idx >= 0 && idx < static_cast<int>(TimerType::TMR_COUNT)) ? idx : -1;
}

// ==========================================
//  AliveImmunityComponent — 技能/状态免疫数据
//  挂载于: 玩家+怪物 (共享)
// ==========================================
constexpr size_t ALIVE_IMMUNITY_SKILL_COUNT = 2;  // 施毒术(6) + 诅咒术(45)

struct AliveImmunityComponent
{
	// 技能免疫: [0]=施毒术(wMagicId=6), [1]=诅咒术(wMagicId=45)
	// lastTickMs + durationMs 配对, 通过 CheckImmunity/SetImmunity 访问
	std::array<int, ALIVE_IMMUNITY_SKILL_COUNT>   skillLastTickMs{};
	std::array<DWORD, ALIVE_IMMUNITY_SKILL_COUNT> skillDurationMs{};

	// 状态免疫: 麻痹(SI_PALSY=26)
	int   statusLastTickMs = 0;
	DWORD statusDurationMs = 0;

	AliveImmunityComponent() = default;
};

// 技能ID → 槽位索引: 6→0, 45→1, 其他→-1
inline int ImmunitySkillToIdx(int wMagicId)
{
	switch (wMagicId)
	{
	case 6:  return 0;  // 施毒术
	case 45: return 1;  // 诅咒术
	default: return -1;
	}
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
