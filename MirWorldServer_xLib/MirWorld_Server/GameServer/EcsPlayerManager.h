#pragma once

#include "ECS.h"
#include "ECSRateLimit.h"
#include "ECSShieldState.h"
#include "ECSSpecialEquip.h"
#include "ECSTimer.h"
#include "TimerSystem.h"
#include <unordered_map>
#include <vector>

class CHumanPlayer;

/**
 *  ECS 玩家组件管理器
 *
 *  职责: 统一管理玩家所有 ECS 组件注册表、实体映射、生命周期。
 */
class EcsPlayerManager : public xSingletonClass<EcsPlayerManager>
{
public:
	EcsPlayerManager();
	~EcsPlayerManager();

	// ========== 频率限制 ==========
	RateLimitComponent* GetRateLimit(CHumanPlayer* pPlayer);
	BOOL TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act);
	VOID GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms);

	// ========== 护盾状态 ==========
	ShieldStateComponent* GetShieldState(CHumanPlayer* pPlayer);
	VOID SetShieldCount(CHumanPlayer* pPlayer, int nCount);

	// ========== 特殊装备 ==========
	EcsSpecialEquip* GetSpecialEquip(CHumanPlayer* pPlayer);
	DWORD GetSpecialEquipFlag(CHumanPlayer* pPlayer, special_equipment_func func);

	// ========== 定时器 ==========
	// 玩家登录时批量创建所有定时器实体
	VOID CreatePlayerTimers(CHumanPlayer* pPlayer);
	// 检查定时器是否到期，到期自动重置
	BOOL CheckTimer(UINT ownerId, TimerType type, DWORD intervalMs);
	// 仅检查到期 (不自动重置)，用于需要查询剩余时间的场景
	BOOL CheckTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs);
	// 重置定时器
	VOID ResetTimer(UINT ownerId, TimerType type);
	// 获取定时器上次 tick 时间
	int GetTimerLastTick(UINT ownerId, TimerType type);
	// 设置定时器已保存时间偏移
	VOID OffsetTimer(UINT ownerId, TimerType type, int offsetMs);

	// ========== 生命周期 ==========
	// 玩家退出游戏
	VOID OnPlayerLogout(CHumanPlayer* pPlayer);

private:
	// 辅助: 获取或创建指定类型的实体
	template<typename T>
	entity_t GetOrCreateEntity(UINT objId, std::unordered_map<UINT, entity_t>& map)
	{
		auto it = map.find(objId);
		if (it != map.end()) return it->second;
		entity_t e = m_world.create();
		m_world.emplace<T>(e);
		map[objId] = e;
		return e;
	}

	VOID CleanupEntityMap(UINT id, std::unordered_map<UINT, entity_t>& map);

	// 玩家定时器类型 → 索引映射 (TimerType 0..16 直接对应数组下标)
	static inline int PlayerTimerTypeToIdx(TimerType t) { return (int)(size_t)t; }

	// === 统一 ECS 注册表 ===
	ECSRegistry m_world;

	// === 共享定时器系统 ===
	static constexpr size_t PLAYER_TMR_COUNT = 17;
	TimerSystem<PLAYER_TMR_COUNT> m_timerSystem;

	// === 实体映射 (按组件类型分离, O(1) 查找) ===
	std::unordered_map<UINT, entity_t>  m_idToRateEntity;
	std::unordered_map<UINT, entity_t>  m_idToShieldEntity;
	std::unordered_map<UINT, entity_t>  m_idToSpecialEquipEntity;
};
