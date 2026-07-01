#pragma once

#include "ECSSystem.h"
#include "TimerComponent.h"
#include "ECSView.h"
#include "PlayerComponentManager.h"
#include "HumanPlayer.h"
#include "SystemScript.h"

/**
 *  TimerSystem —— 统一定时器消费 ECS System (单次 ecs_view<TimerComponent> 扫描)
 *
 *  替代 OOP Update() 中的全部定时器 CheckTimer 消费逻辑:
 *
 *  活体通用 (所有实体: 玩家+怪物+NPC):
 *    bit 18  TMR_HP_RECOVER  (默认3秒)  HP 自动恢复
 *    bit 19  TMR_MP_RECOVER  (默认3秒)  MP 自动恢复
 *
 *  玩家专属 (仅 CHumanPlayer):
 *    bit 0   TMR_DB_SAVE   — per-player 偏移量动态变化
 *    bit 1   TMR_STAMINA     (6分钟)    精力值自增
 *    bit 2   TMR_GAME_TIME   (1秒)      时长区倒计时 + 元气自增
 *    bit 4   TMR_JUST_PK     (配置)     灰名到期清除
 *    bit 5   TMR_PK_POINT    (配置)     PK值衰减
 *    bit 6   TMR_FENGHAO     (60秒)     封号过期检测
 *    bit 8   TMR_ADD_TO_GUILD (60秒)    行会请求超时
 *
 *  不处理:
 *    TMR_ACTION    — 间隔高度动态, 不适合批量
 *    TMR_CHAT_*    — per-channel 间隔不同
 *    TMR_HORSE     — 逻辑分散在多处, 暂保留 OOP 路径
 *    TMR_IDLE/BODY/BETRAY — 怪物专属, 间隔动态, 暂保留 OOP 路径
 *
 *  执行时机: BatchPrecomputeTimers 之后 (firedMask 已就绪)
 *  线程安全: 主线程串行执行 (UpdatePlayers 之前)
 */
class TimerSystem : public ECSSystem
{
public:
	const char* Name() const override { return "TimerSystem"; }
	int Priority() const override { return 10; }

	size_t Execute(int frameTime) override
	{
		auto& world = ECSWorld::GetInstance()->GetWorld();
		CGameWorld* pGameWorld = CGameWorld::GetInstance();
		if (!pGameWorld) return 0;

		size_t processed = 0;

		// 一次性读取全局配置
		DWORD justPkInterval = pGameWorld->GetVar(EVI_GRAYNAMETIME) * 1000;
		DWORD pkPointInterval = pGameWorld->GetVar(EVI_ONEPKPOINTTIME) * 1000;

		SRLock lock(world.m_mutex);

		ecs_view<TimerComponent>(world).each([&](TimerComponent& tc) {
			uint32_t mask = tc.firedMask;
			if (!mask) return;

			CAliveObject* pAlive = pGameWorld->GetAliveObjectById(tc.ownerId);
			if (!pAlive || pAlive->IsDeath()) return;

			// ==========================================
			//  活体通用: HP/MP 恢复 (bit 18, 19)
			//  作用于全部实体: 玩家+怪物+NPC
			// ==========================================

			// TMR_HP_RECOVER = 18
			if (mask & (1u << 18))
			{
				tc.firedMask &= ~(1u << 18);
				if (pAlive->CanRecover())
				{
					int nHp = pAlive->GetAutoRecoverHp();
					if (nHp != 0)
					{
						if (pAlive->GetAutoRecoverHptime() > 0 &&
							(pAlive->GetPropValue(PI_CURHP) < pAlive->GetPropValue(PI_MAXHP) || nHp < 0))
						{
							if (nHp > 0)
								pAlive->AddPropValue(PI_CURHP, nHp);
							else
								pAlive->DecPropValue(PI_CURHP, -nHp);
							pAlive->SendHpMpChanged(-nHp);
						}
					}
				}
				processed++;
			}

			// TMR_MP_RECOVER = 19
			if (mask & (1u << 19))
			{
				tc.firedMask &= ~(1u << 19);
				if (pAlive->CanRecover())
				{
					int nMp = pAlive->GetAutoRecoverMp();
					if (nMp != 0)
					{
						if (pAlive->GetAutoRecoverMptime() > 0 &&
							(pAlive->GetPropValue(PI_CURMP) < pAlive->GetPropValue(PI_MAXMP) || nMp < 0))
						{
							if (nMp > 0)
								pAlive->AddPropValue(PI_CURMP, nMp);
							else
								pAlive->DecPropValue(PI_CURMP, -nMp);
							pAlive->SendHpMpChanged();
						}
					}
				}
				processed++;
			}

			// ==========================================
			//  玩家专属定时器 (bit 1,2,4,5,6,8)
			//  仅作用于 CHumanPlayer
			// ==========================================
			CHumanPlayer* pPlayer = dynamic_cast<CHumanPlayer*>(pAlive);
			if (!pPlayer) return;  // 非玩家实体, 跳过玩家专属定时器

			// TMR_DB_SAVE (0): DB 存档, 动态间隔 (配置基础 + per-player 错峰偏移)
			if (mask & (1u << 0))
			{
				tc.firedMask &= ~(1u << 0);
				if (pGameWorld->CanSaveToDB())
				{
					pGameWorld->UpdateDBUpdateTimer();
					DWORD dwSaveOffset = (pPlayer->GetId() % 30) * 10 * 1000;
					tc.lastTickMs[0] -= (int)dwSaveOffset;
					pPlayer->UpdateToDB();
				}
				processed++;
			}

			// TMR_STAMINA (1): 精力值自增, 6分钟
			if (mask & (1u << 1))
			{
				tc.firedMask &= ~(1u << 1);
				if (pPlayer->GetPropValue(PI_LEVEL) >= 7)
				{
					auto* sc = PlayerComponentManager::GetInstance()->GetStamina(pPlayer);
					if (sc)
					{
						CLogicMap* pMap = pPlayer->GetMap();
						int expFactor = (pMap != nullptr) ? (int)ceilf(pMap->GetExpFactor()) : 1;
						if (expFactor > 1)
							sc->wStamina += expFactor;
						else
							sc->wStamina++;
						if (sc->wStamina <= sc->wMaxStamina)
						{
							pPlayer->SendJingLiZhi(sc->wStamina);
							pPlayer->UpdateProp();
						}
						else
							sc->wStamina = sc->wMaxStamina;
					}
				}
				processed++;
			}

			// TMR_GAME_TIME (2): 时长区倒计时 + 元气自增, 1秒
			if (mask & (1u << 2))
			{
				tc.firedMask &= ~(1u << 2);
				int nGameTime = pPlayer->GetGameTIme();
				if (nGameTime > -1)
				{
					if (nGameTime > 0)
					{
						pPlayer->DecGameTime(1);
						if (pPlayer->GetGameTIme() == 0)
							CSystemScript::GetInstance()->Execute(pPlayer->GetScriptTarget(), "游戏时长.TimeOver", FALSE);
					}
				}
				auto* zb = pPlayer->GetZhenBaoComp();
				if (zb && !zb->IsYuanQiFull && zb->YuanQi < 2000)
				{
					zb->YuanQi++;
					pPlayer->SendMsg(pPlayer->GetId(), 0x9611, zb->YuanQi, 2000, 0);
					if (zb->YuanQi >= 2000)
						zb->IsYuanQiFull = TRUE;
				}
				processed++;
			}

			// TMR_JUST_PK (4): 灰名到期清除
			if (mask & (1u << 4))
			{
				tc.firedMask &= ~(1u << 4);
				auto* pk = pPlayer->GetPkComp();
				if (pk && pk->JustPk)
				{
					pk->JustPk = FALSE;
					pPlayer->SendChangeName();
				}
				processed++;
			}

			// TMR_PK_POINT (5): PK值衰减
			if (mask & (1u << 5))
			{
				tc.firedMask &= ~(1u << 5);
				auto* pk = pPlayer->GetPkComp();
				if (pk && pk->PkValue > 0)
				{
					BYTE btColor = pPlayer->GetNameColor(pPlayer);
					pk->PkValue--;
					if (btColor != pPlayer->GetNameColor(pPlayer))
						pPlayer->SendChangeName();
				}
				processed++;
			}

			// TMR_FENGHAO (6): 封号过期检测, 60秒
			if (mask & (1u << 6))
			{
				tc.firedMask &= ~(1u << 6);
				pPlayer->CheckFengHaoTimeOut();
				processed++;
			}

			// TMR_ADD_TO_GUILD (8): 行会请求超时, 60秒
			if (mask & (1u << 8))
			{
				tc.firedMask &= ~(1u << 8);
				if (pPlayer->GetAddToGuildRequester() != nullptr)
					pPlayer->ReplyAddToGuildRequest(FALSE);
				processed++;
			}
			});

		return processed;
	}
};
