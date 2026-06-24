#include "StdAfx.h"
#include "RateLimitSystem.h"
#include "HumanPlayer.h"

entity_t RateLimitSystem::GetOrCreateEntity(UINT objId)
{
	// 调用方必须已持有 ECSRegistry 锁
	auto& world = ECSWorld::GetInstance()->GetWorld();
	auto it = m_idToEntity.find(objId);
	if (it != m_idToEntity.end()) return it->second;

	entity_t e = world.create();
	world.emplace<RateLimitComponent>(e);
	m_idToEntity[objId] = e;
	return e;
}

RateLimitComponent* RateLimitSystem::GetRateLimit(CHumanPlayer* pPlayer)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	return world.get<RateLimitComponent>(e);
}

BOOL RateLimitSystem::TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	auto* rl = world.get<RateLimitComponent>(e);
	if (!rl) return TRUE;
	if (act == RateLimitComponent::ACT_SPECIAL_ATTACK) // 战士技能攻击速度根据攻速调整
		rl->SetInterval(RateLimitComponent::ACT_SPECIAL_ATTACK, rl->intervals[RateLimitComponent::ACT_SPECIAL_ATTACK] - 80 * pPlayer->GetPropValue(PI_ATTACKSPEED));
	return rl->TryExecute(act, CFrameTime::GetFrameTime());
}

VOID RateLimitSystem::GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	entity_t e = GetOrCreateEntity(pPlayer->GetId());
	auto* rl = world.get<RateLimitComponent>(e);
	if (rl) rl->SetInterval(act, ms);
}

VOID RateLimitSystem::OnPlayerLogout(UINT id)
{
	auto& world = ECSWorld::GetInstance()->GetWorld();
	std::lock_guard<ECSRegistry> lock(world);
	auto it = m_idToEntity.find(id);
	if (it != m_idToEntity.end())
	{
		world.destroy(it->second);
		m_idToEntity.erase(it);
	}
}
