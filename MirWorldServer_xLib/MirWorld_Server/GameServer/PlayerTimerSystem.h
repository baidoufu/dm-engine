#pragma once

#include "ECS.h"
#include "TimerComponent.h"
#include "TimerSystem.h"
#include "ECSWorld.h"
#include <unordered_map>
#include <array>

class CHumanPlayer;

/**
 *  PlayerTimerSystem — 玩家定时器组件管理器
 *
 *  职责: 管理 CHumanPlayer 专属的 17 个 TimerComponent 实体。
 *  内部委托到 TimerSystem<17>，共享 ECSWorld 中的 ECSRegistry。
 */
class PlayerTimerSystem : public xSingletonClass<PlayerTimerSystem>
{
public:
	PlayerTimerSystem();
	~PlayerTimerSystem() = default;

	// 玩家登录时批量创建所有定时器实体
	VOID CreatePlayerTimers(CHumanPlayer* pPlayer);

	// 定时器操作
	BOOL  CheckTimer(UINT ownerId, TimerType type, DWORD intervalMs);
	BOOL  CheckTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs);
	VOID  ResetTimer(UINT ownerId, TimerType type);
	VOID  OffsetTimer(UINT ownerId, TimerType type, int offsetMs);
	int   GetTimerLastTick(UINT ownerId, TimerType type);

	// 玩家退出时清理
	VOID OnPlayerLogout(UINT ownerId);

private:
	static inline int PlayerTimerTypeToIdx(TimerType t) { return (int)(size_t)t; }

	static constexpr size_t PLAYER_TMR_COUNT = 17;
	TimerSystem<PLAYER_TMR_COUNT> m_timerSystem;
};
