#pragma once

#include "ECSSystem.h"
#include "StatusComponent.h"
#include "ECSView.h"
#include "AliveObject.h"

/**
 *  StatusSystem —— 状态系统
 */
class StatusSystem : public ECSSystem
{
public:
	const char* Name() const override { return "StatusSystem"; }
	int Priority() const override { return 10; }

	size_t Execute(int frameTime) override
	{
		auto& world = ECSWorld::GetInstance()->GetWorld();
		CGameWorld* pGameWorld = CGameWorld::GetInstance();
		if (!pGameWorld) return 0;

		size_t processed = 0;
		SRLock lock(world.m_mutex);
		// ==========================================
		//  状态过期消费 (StatusComponent)
		// ==========================================
		ecs_view<StatusComponent>(world).each([&](StatusComponent& sc) {
			uint32_t statusMask = sc.statusExpiredMask;
			uint32_t sysFlagMask = sc.systemFlagExpiredMask;
			if (!statusMask && !sysFlagMask) return;

			CAliveObject* pAlive = pGameWorld->GetAliveObjectById(sc.ownerId);
			if (!pAlive || pAlive->IsDeath()) return;
			// m_Status 过期位
			while (statusMask)
			{
				unsigned long i;
				_BitScanForward(&i, statusMask);
				statusMask &= ~(1u << i);
				sc.statusExpiredMask &= ~(1u << i);
				sc.activeSlotMask &= ~(1ull << i);			// 直接清除, 跳过 ClrStatus 的 ECS 回写
				pAlive->ClrStatusFromECS((int)i);			// 只清 OOP + 回调, 省去 GetStatusComp + 冗余写入
				processed++;
			}
			// m_SystemFlag 过期位
			while (sysFlagMask)
			{
				unsigned long i;
				_BitScanForward(&i, sysFlagMask);
				sysFlagMask &= ~(1u << i);
				sc.systemFlagExpiredMask &= ~(1u << i);
				sc.activeSlotMask &= ~(1ull << (i + 32));	// 直接清除, 跳过 ClearSystemFlagFromECS 的 ECS 回写
				pAlive->ClearSystemFlagFromECS((int)i);		// 只清 OOP + 回调, 省去冗余写入
				processed++;
			}
			});
		return processed;
	}
};
