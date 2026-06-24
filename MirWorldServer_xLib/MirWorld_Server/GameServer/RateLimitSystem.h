#pragma once

#include "ECS.h"
#include "RateLimitComponent.h"
#include "ECSWorld.h"
#include <unordered_map>

class CHumanPlayer;

/**
 *  RateLimitSystem — 频率限制组件管理器
 *
 *  职责: 管理 RateLimitComponent 的实体生命周期和访问。
 *  共享 ECSWorld 中的 ECSRegistry。
 */
class RateLimitSystem : public xSingletonClass<RateLimitSystem>
{
public:
	RateLimitSystem()  = default;
	~RateLimitSystem() = default;

	RateLimitComponent* GetRateLimit(CHumanPlayer* pPlayer);

	// 尝试执行操作 (含战士攻速调整)
	BOOL TryRateLimit(CHumanPlayer* pPlayer, RateLimitComponent::Action act);

	// GM 动态调整间隔
	VOID GmSetRateLimitInterval(CHumanPlayer* pPlayer, RateLimitComponent::Action act, int ms);

	// 玩家退出时清理
	VOID OnPlayerLogout(UINT id);

private:
	entity_t GetOrCreateEntity(UINT objId);

	std::unordered_map<UINT, entity_t> m_idToEntity;
};
