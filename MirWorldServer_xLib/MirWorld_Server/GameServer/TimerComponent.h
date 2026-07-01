#pragma once
#include "ECS.h"
#include <array>

/**
 *  TimerComponent.h — 统一定时器组件
 *
 *  实体归一:
 *    一个实体 = 一个 TimerComponent (23槽, 覆盖全部定时器)
 *
 *  firedMask 位索引 = TimerType 枚举值 (绝对索引, 无碰撞):
 *    bit 0~16: 玩家定时器 (TMR_DB_SAVE ~ TMR_CHAT_FRIEND)
 *    bit 17~19: 活体定时器 (TMR_ACTION / TMR_HP_RECOVER / TMR_MP_RECOVER)
 *    bit 20~22: 怪物定时器 (TMR_IDLE / TMR_BODY / TMR_BETRAY)
 */

enum class TimerType : uint8_t
{
	TMR_DB_SAVE = 0,
	TMR_STAMINA,
	TMR_GAME_TIME,
	TMR_PK,
	TMR_JUST_PK,
	TMR_PK_POINT,
	TMR_FENGHAO,
	TMR_HORSE,
	TMR_ADD_TO_GUILD,
	TMR_CHAT_NORMAL,
	TMR_CHAT_CRY,
	TMR_CHAT_WISPER,
	TMR_CHAT_GROUP,
	TMR_CHAT_GUILD,
	TMR_CHAT_COUPLE,
	TMR_CHAT_GM,
	TMR_CHAT_FRIEND,
	TMR_ACTION,
	TMR_HP_RECOVER,
	TMR_MP_RECOVER,
	TMR_IDLE,
	TMR_BODY,
	TMR_BETRAY
};

constexpr size_t TOTAL_TMR_COUNT = 23;

struct TimerComponent
{
	std::array<int, TOTAL_TMR_COUNT> lastTickMs{};
	uint32_t firedMask = 0;
	UINT    ownerId = 0;

	TimerComponent() = default;
};

inline int TimerTypeToIdx(TimerType t)
{
	int idx = static_cast<int>(t);
	return (idx >= 0 && idx < static_cast<int>(TOTAL_TMR_COUNT)) ? idx : -1;
}

// ==========================================
//  AliveImmunityComponent — 技能/状态免疫
// ==========================================
constexpr size_t ALIVE_IMMUNITY_SKILL_COUNT = 2;

struct AliveImmunityComponent
{
	std::array<int, ALIVE_IMMUNITY_SKILL_COUNT>   skillLastTickMs{};
	std::array<DWORD, ALIVE_IMMUNITY_SKILL_COUNT> skillDurationMs{};
	int   statusLastTickMs = 0;
	DWORD statusDurationMs = 0;
	AliveImmunityComponent() = default;
};

inline int ImmunitySkillToIdx(int wMagicId)
{
	switch (wMagicId)
	{
	case 6:  return 0;
	case 45: return 1;
	default: return -1;
	}
}

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
