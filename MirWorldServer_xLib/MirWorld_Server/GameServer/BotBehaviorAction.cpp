#include "StdAfx.h"
#include "BotBehaviorAction.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

// ============================================================================
// 动作节点 - 使用药水
// ============================================================================
BTResult CBTActionUsePotion::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateUsePotion(m_bUseHP) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 使用物品
// ============================================================================
BTResult CBTActionUseItem::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateUseItem(m_szUseItemName.c_str()) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 切换攻击模式
// ============================================================================
BTResult CBTActionChangeAttackMode::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SimulateChangeAttackMode(m_attackMode);
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 攻击目标
// ============================================================================
BTResult CBTActionAttack::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateAttackTarget() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 移向目标
// ============================================================================
BTResult CBTActionMoveToTarget::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateMoveToTarget() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 巡逻移动
// ============================================================================
BTResult CBTActionPatrol::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SimulatePatrol();
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 拾取物品
// ============================================================================
BTResult CBTActionPickupItem::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SimulatePickupItem();
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 逃跑
// ============================================================================
BTResult CBTActionFlee::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateFlee() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 休息
// ============================================================================
BTResult CBTActionRest::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SimulateRest(m_dwRestDuration);
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 复活（死亡后复活，恢复HP/MP，可选回城）
// ============================================================================
BTResult CBTActionRevive::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	// 未死亡时不需复活，直接成功（幂等）
	if (!pBot->IsDeath()) return BTR_SUCCESS;
	// 复活延迟检查：死亡后未到延迟时间则返回RUNNING（配合MemSequence记忆位置，避免重复执行前置节点）
	if (m_dwDelay > 0)
	{
		DWORD dwElapsed = pBot->GetDeathElapsed();
		if (dwElapsed < m_dwDelay)
			return BTR_RUNNING;  // 延迟未到，继续等待
	}
	return pBot->SimulateRevive(m_nHpPercent, m_bTeleportHome) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 聊天
// ============================================================================
BTResult CBTActionChat::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SimulateRandomChat();
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 使用技能
// ============================================================================
BTResult CBTActionUseSkill::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->SimulateUseSkill(m_wMagicId) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 说出指定文本
// ============================================================================
BTResult CBTActionSay::Execute(CBotPlayer* pBot)
{
	if (!pBot || m_szMessage.empty())
		return BTR_FAILURE;
	pBot->SimulateSay(m_szMessage.c_str());
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 使用回城卷轴/随机卷轴/地牢卷轴/沙城回城卷
// ============================================================================
BTResult CBTActionRecall::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	const char* pszItemName = nullptr;
	switch (m_nType)
	{
	case 0: pszItemName = "回城卷轴"; break;
	case 1: pszItemName = "随机卷轴"; break;
	case 2: pszItemName = "地牢卷轴"; break;
	case 3: pszItemName = "沙城回城卷"; break;
	default: return BTR_FAILURE;
	}
	return pBot->SimulateUseItem(pszItemName) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 延迟等待
// ============================================================================
BTResult CBTActionDelay::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	DWORD dwDelay = CBotHumanBehavior::RandomRange(m_dwMinMs, m_dwMaxMs);
	pBot->SimulateRest(dwDelay);
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 向指定方向攻击
// ============================================================================
BTResult CBTActionAttackDir::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	int nDir = (m_nDir >= 0 && m_nDir <= 7) ? m_nDir : CBotHumanBehavior::RandomDirection8();
	return pBot->SimulateAttack(nDir) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 精确施法
// ============================================================================
BTResult CBTActionSpellCast::Execute(CBotPlayer* pBot)
{
	if (!pBot || m_wMagicId == 0) return BTR_FAILURE;
	DWORD dwTargetId = m_dwTargetId;
	int nTargetX = m_nTargetX;
	int nTargetY = m_nTargetY;
	if (dwTargetId == 0)
	{
		CBotContext* pCtx = pBot->GetContext();
		if (pCtx)
		{
			CAliveObject* pTarget = pCtx->GetCachedTarget();
			if (pTarget)
			{
				dwTargetId = pTarget->GetId();
				nTargetX = pTarget->getX();
				nTargetY = pTarget->getY();
			}
		}
	}
	return pBot->SimulateSpellCast(nTargetX, nTargetY, dwTargetId, m_wMagicId) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 丢弃物品
// ============================================================================
BTResult CBTActionDropItem::Execute(CBotPlayer* pBot)
{
	if (!pBot || m_szItemName.empty()) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	DWORD dwItemIndex = pCtx->FindItemInBag(m_szItemName.c_str());
	if (dwItemIndex == 0) return BTR_FAILURE;
	int nDropped = 0;
	for (int i = 0; i < m_nCount; i++)
	{
		if (pBot->DropBagItem(dwItemIndex)) nDropped++;
		else break;
	}
	return (nDropped > 0) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 穿戴装备
// ============================================================================
BTResult CBTActionEquipItem::Execute(CBotPlayer* pBot)
{
	if (!pBot || m_szItemName.empty())
		return BTR_FAILURE;
	return pBot->SimulateEquipItem(m_szItemName.c_str()) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 召唤宠物
// ============================================================================
BTResult CBTActionSummonPet::Execute(CBotPlayer* pBot)
{
	if (!pBot || m_szPetName.empty())
		return BTR_FAILURE;
	if (pBot->GetPetCount() >= 5) return BTR_FAILURE;
	BOOL bSuccess = pBot->SummonMonster(m_szPetName.c_str(), TRUE, pBot->getX(), pBot->getY()) != nullptr;
	if (bSuccess) pBot->SimulateRest(500);
	return bSuccess ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 跟随目标
// ============================================================================
BTResult CBTActionFollow::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CAliveObject* pTarget = nullptr;
	if (!m_szTargetName.empty())
	{
		xListHost<VISIBLE_OBJECT>* pVisList = pBot->GetVisibleObjectList();
		if (pVisList)
		{
			xListHelper<VISIBLE_OBJECT> helper(pVisList);
			for (VISIBLE_OBJECT* pVO = helper.first(); pVO != nullptr; pVO = helper.next())
			{
				CMapObject* pMapObj = pVO->pObject;
				if (pMapObj && pMapObj->GetClassType() == CLS_ALIVEOBJECT && pMapObj->GetType() == OBJ_PLAYER)
				{
					if (strcmp(pMapObj->GetName(), m_szTargetName.c_str()) == 0)
					{
						pTarget = dynamic_cast<CAliveObject*>(pMapObj);
						break;
					}
				}
			}
		}
	}
	else
	{
		CBotContext* pCtx = pBot->GetContext();
		if (pCtx) pTarget = pCtx->GetCachedTarget();
	}
	if (!pTarget) return BTR_FAILURE;
	int nDist = CBotHumanBehavior::Distance8(pBot->getX(), pBot->getY(), pTarget->getX(), pTarget->getY());
	if (nDist <= m_nFollowRange) return BTR_SUCCESS;
	int nDir = CBotHumanBehavior::GetDirection8(pTarget->getX(), pTarget->getY(), pBot->getX(), pBot->getY());
	return pBot->SimulateWalk(nDir) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 组队操作 - 待完善
// ============================================================================
BTResult CBTActionGroup::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	if (m_szPlayerName.empty()) return BTR_FAILURE;
	switch (m_nAction)
	{
	case 0: case 1:
		return pBot->AddGroupMember(m_szPlayerName.c_str()) ? BTR_SUCCESS : BTR_FAILURE;
	case 2: // 请求入队
	{

	}
	break;
	case 3: // 退出队伍
		return BTR_SUCCESS;
	case 4: // 开启组队
	{
		pBot->SetGroupMode(TRUE);
		return BTR_SUCCESS;
	}
	break;
	case 5: // 关闭组队
	{
		pBot->SetGroupMode(FALSE);
		return BTR_SUCCESS;
	}
	break;
	}
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 挖矿
// ============================================================================
BTResult CBTActionMine::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->DoMine(pBot->getX(), pBot->getY(), (int)m_dwDuration / 1000);
	pBot->SimulateRest(m_dwDuration);
	return BTR_SUCCESS;
}

// ============================================================================
// 动作节点 - 放弃锁定目标：清除机器人当前锁定的目标
// ============================================================================
BTResult CBTActionUnlockTarget::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	pBot->SetTarget(nullptr);
	return BTR_SUCCESS;
}
