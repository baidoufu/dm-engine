#pragma once

#include "ECSSystem.h"
#include "PotionRecoverComponent.h"
#include "ECSView.h"
#include "AliveObject.h"
#include "GameWorld.h"

/**
 *  PotionRecoverSystem —— 吃药递增 ECS System (纯 SoA 遍历)
 *
 *  替代 CAliveObject::Update() case 1 中的吃药 HP/MP 递增逻辑。
 *
 *  数据流:
 *    OOP SetAddHp/SetAddMp → 写入 PotionRecoverComponent (ECS侧)
 *    PotionRecoverSystem → ecs_view<PotionRecoverComponent> 遍历
 *                      → 直接操作组件字段 (SoA, 缓存友好)
 *                      → AddPropValue / SendHpMpChanged (OOP侧动作)
 *
 *  只遍历 addHp>0 或 addMp>0 的实体 (零开销跳过空闲实体)
 *
 *  执行时机: TimerSystem 之后 (firedMask 已消费, HP/MP 自动恢复已处理)
 *  线程安全: 主线程串行执行 (UpdatePlayers 之前)
 */
class PotionRecoverSystem : public ECSSystem
{
public:
	const char* Name() const override { return "PotionRecover"; }
	int Priority() const override { return 20; }  // TimerSystem(10) 之后

	size_t Execute(int frameTime) override
	{
		auto& world = ECSWorld::GetInstance()->GetWorld();
		CGameWorld* pGameWorld = CGameWorld::GetInstance();
		if (!pGameWorld) return 0;

		size_t processed = 0;
		int now = CFrameTime::GetFrameTime();

		SRLock lock(world.m_mutex);

		ecs_view<PotionRecoverComponent>(world).each([&](PotionRecoverComponent& pc) {
			// 零开销跳过: 无待处理吃药效果
			if (pc.addHp == 0 && pc.addMp == 0) return;

			CAliveObject* pAlive = pGameWorld->GetAliveObjectById(pc.ownerId);
			if (!pAlive || pAlive->IsDeath()) return;

			BOOL bHpChanged = FALSE;
			BOOL bMpChanged = FALSE;
			int  nHp = 0;

			// 吃药增加HP (500ms 间隔)
			if (pc.addHp > 0 && GetTimeToTime(pc.lastHpTickMs, now) >= 500)
			{
				if (pAlive->GetPropValue(PI_CURHP) < pAlive->GetPropValue(PI_MAXHP))
				{
					bHpChanged = TRUE;
					nHp = (int)(pc.addHp > pc.addHpSpeed ? pc.addHpSpeed : pc.addHp);
					pAlive->AddPropValue(PI_CURHP, nHp);
					pc.addHp -= nHp;
				}
				else
					pc.addHp = 0;
				pc.lastHpTickMs = now;
				processed++;
			}

			// 吃药增加MP (500ms 间隔)
			if (pc.addMp > 0 && GetTimeToTime(pc.lastMpTickMs, now) >= 500)
			{
				if (pAlive->GetPropValue(PI_CURMP) < pAlive->GetPropValue(PI_MAXMP))
				{
					bMpChanged = TRUE;
					DWORD dwAddMp = pc.addMp > pc.addMpSpeed ? pc.addMpSpeed : pc.addMp;
					pAlive->AddPropValue(PI_CURMP, dwAddMp);
					pc.addMp -= dwAddMp;
				}
				else
					pc.addMp = 0;
				pc.lastMpTickMs = now;
				processed++;
			}

			if (bHpChanged) pAlive->SendHpMpChanged(-nHp);
			if (bMpChanged) pAlive->SendHpMpChanged();
		});

		return processed;
	}
};
