#pragma once

#include "ECS.h"
#include "TimerComponent.h"
#include <unordered_map>
#include <array>
#include <mutex>

/**
 *  TimerSystem — 共享定时器工具类 (非单例)
 *
 *  职责: 管理一组 TimerComponent 实体的 CRUD + 到期检查。
 *  PlayerTimerSystem / AliveTimerSystem 各自持有一个 TimerSystem 实例。
 *
 *  模板参数 TimerCount: 该 TimerSystem 管理的定时器种类数量 (编译期确定, 零开销)
 */
template<size_t TimerCount>
class TimerSystem
{
public:
	// 索引映射: TimerType → 本地数组下标
	using IdxFunc = int(*)(TimerType);

	TimerSystem(ECSRegistry& world, IdxFunc toIdx)
		: m_world(world)
		, m_toIdx(toIdx)
	{
	}

	// ========== 实体管理 ==========

	entity_t GetOrCreateTimerEntity(UINT ownerId, TimerType type)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		return GetOrCreateTimerEntityUnsafe(ownerId, type);
	}

	entity_t GetTimerEntity(UINT ownerId, TimerType type) const
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		return GetTimerEntityUnsafe(ownerId, type);
	}

	// ========== 定时器操作 ==========

	// 检查定时器是否到期，到期自动前推 lastTickMs (避免累积漂移)
	BOOL CheckTimer(UINT ownerId, TimerType type, DWORD intervalMs)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		entity_t e = GetOrCreateTimerEntityUnsafe(ownerId, type);
		auto* tc = m_world.get<TimerComponent>(e);
		if (!tc) return FALSE;
		return tc->CheckAndAdvance(intervalMs, CFrameTime::GetFrameTime());
	}

	// 仅检查到期 (不自动重置)，用于需要查询剩余时间的场景
	BOOL CheckTimerNoReset(UINT ownerId, TimerType type, DWORD intervalMs, int& outLastTickMs)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		entity_t e = GetOrCreateTimerEntityUnsafe(ownerId, type);
		auto* tc = m_world.get<TimerComponent>(e);
		if (!tc) { outLastTickMs = 0; return FALSE; }

		outLastTickMs = tc->lastTickMs;
		int now = CFrameTime::GetFrameTime();
		return GetTimeToTime(tc->lastTickMs, now) >= intervalMs;
	}

	// 重置定时器
	VOID ResetTimer(UINT ownerId, TimerType type)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		entity_t e = GetOrCreateTimerEntityUnsafe(ownerId, type);
		auto* tc = m_world.get<TimerComponent>(e);
		if (tc)
			tc->lastTickMs = CFrameTime::GetFrameTime();
	}

	// 设置定时器已保存时间偏移 (用于错开保存等场景)
	VOID OffsetTimer(UINT ownerId, TimerType type, int offsetMs)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		entity_t e = GetTimerEntityUnsafe(ownerId, type);
		if (!m_world.valid(e)) return;
		auto* tc = m_world.get<TimerComponent>(e);
		if (tc)
			tc->lastTickMs -= offsetMs;
	}

	// 获取定时器上次 tick 时间
	int GetTimerLastTick(UINT ownerId, TimerType type)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		entity_t e = GetOrCreateTimerEntityUnsafe(ownerId, type);
		auto* tc = m_world.get<TimerComponent>(e);
		return tc ? tc->lastTickMs : 0;
	}

	// ========== 生命周期 ==========

	// 清理指定 owner 的所有定时器实体
	VOID Cleanup(UINT ownerId)
	{
		std::lock_guard<ECSRegistry> lock(m_world);
		auto it = m_entities.find(ownerId);
		if (it != m_entities.end())
		{
			for (entity_t e : it->second)
			{
				if (m_world.valid(e))
					m_world.destroy(e);
			}
			m_entities.erase(it);
		}
	}



private:
	// 无锁内部版本 — 调用方必须已持有 m_world 锁
	entity_t GetOrCreateTimerEntityUnsafe(UINT ownerId, TimerType type)
	{
		const int idx = m_toIdx(type);
		if (idx < 0 || (size_t)idx >= TimerCount) return INVALID_ENTITY;

		auto& arr = m_entities[ownerId];

		if (!m_world.valid(arr[idx]))
		{
			entity_t e = m_world.create();
			auto& tc = m_world.emplace<TimerComponent>(e);
			tc.typeId     = type;
			tc.intervalMs = 0;
			tc.lastTickMs = CFrameTime::GetFrameTime();
			tc.ownerId    = ownerId;
			arr[idx] = e;
		}
		return arr[idx];
	}

	entity_t GetTimerEntityUnsafe(UINT ownerId, TimerType type) const
	{
		auto it = m_entities.find(ownerId);
		if (it == m_entities.end()) return INVALID_ENTITY;

		const int idx = m_toIdx(type);
		if (idx < 0 || (size_t)idx >= TimerCount) return INVALID_ENTITY;
		return it->second[idx];
	}

	ECSRegistry&  m_world;
	IdxFunc       m_toIdx;
	std::unordered_map<UINT, std::array<entity_t, TimerCount>> m_entities;
};
