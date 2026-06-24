#include "StdAfx.h"
#include "BotBehaviorTree.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

// ============================================================================
// 复合节点 - 序列节点实现
// ============================================================================
BTResult CBTSequence::Execute(CBotPlayer* pBot)
{
	for (auto& child : m_children)
	{
		BTResult result = child->Execute(pBot);
		if (result != BTR_SUCCESS)
			return result;
	}
	return BTR_SUCCESS;
}

// ============================================================================
// 复合节点 - 选择节点实现
// ============================================================================
BTResult CBTSelector::Execute(CBotPlayer* pBot)
{
	for (auto& child : m_children)
	{
		BTResult result = child->Execute(pBot);
		if (result == BTR_SUCCESS)
			return BTR_SUCCESS;
		// FAILURE则继续尝试下一个子节点
	}
	return BTR_FAILURE;
}

// ============================================================================
// 复合节点 - 并行节点实现
// ============================================================================
BTResult CBTParallel::Execute(CBotPlayer* pBot)
{
	int nSuccess = 0;
	int nFailure = 0;

	for (auto& child : m_children)
	{
		BTResult result = child->Execute(pBot);
		if (result == BTR_SUCCESS)
			nSuccess++;
		else if (result == BTR_FAILURE)
			nFailure++;
	}

	// 所有子节点都成功才返回SUCCESS
	if (nSuccess == (int)m_children.size())
		return BTR_SUCCESS;

	// 任一子节点失败则返回FAILURE
	if (nFailure > 0)
		return BTR_FAILURE;

	return BTR_RUNNING;
}

// ============================================================================
// 复合节点 - 随机节点实现
// ============================================================================
BTResult CBTRandom::Execute(CBotPlayer* pBot)
{
	int nRand = CBotHumanBehavior::RandomRange(m_children.size(), RANDOM_OI);
	if (m_children[nRand])
		m_children[nRand]->Execute(pBot);
	return BTR_SUCCESS;
}

// ============================================================================
// 复合节点 - 概率节点实现 - 按概率决定是否执行子节点
// ============================================================================
BTResult CBTProbability::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;
	if (CBotHumanBehavior::RandomPercent(m_nChance))
		return m_pChild->Execute(pBot);
	return BTR_FAILURE;
}

// ============================================================================
// 复合节点 - 记忆序列节点实现
// ============================================================================
BTResult CBTMemSequence::Execute(CBotPlayer* pBot)
{
	for (int i = m_nLastIndex; i < (int)m_children.size(); i++)
	{
		BTResult result = m_children[i]->Execute(pBot);
		if (result == BTR_FAILURE)
		{
			m_nLastIndex = i;
			return BTR_FAILURE;
		}
		if (result == BTR_RUNNING)
			return BTR_RUNNING;
	}
	m_nLastIndex = 0;
	return BTR_SUCCESS;
}

// ============================================================================
// 复合节点 - 记忆选择节点实现
// ============================================================================
BTResult CBTMemSelector::Execute(CBotPlayer* pBot)
{
	for (int i = m_nLastIndex; i < (int)m_children.size(); i++)
	{
		BTResult result = m_children[i]->Execute(pBot);
		if (result == BTR_SUCCESS)
		{
			m_nLastIndex = i;
			return BTR_SUCCESS;
		}
		if (result == BTR_RUNNING)
			return BTR_RUNNING;
	}
	m_nLastIndex = 0;
	return BTR_FAILURE;
}

// ============================================================================
// 装饰节点 - 反转器实现
// ============================================================================
BTResult CBTDecoratorInverter::Execute(CBotPlayer* pBot)
{
	if (!m_pChild)
		return BTR_FAILURE;

	BTResult result = m_pChild->Execute(pBot);
	switch (result)
	{
	case BTR_SUCCESS: return BTR_FAILURE;
	case BTR_FAILURE: return BTR_SUCCESS;
	default: return BTR_RUNNING;
	}
}

// ============================================================================
// 装饰节点 - 重复器实现
// ============================================================================
// 硬限制：防止 m_nMaxRepeat=0（无限）时子节点永不返回FAILURE导致死循环
// 正常使用下不应达到此限制，达到说明行为树配置有误
static const int MAX_REPEAT_HARD_LIMIT = 100;
BTResult CBTDecoratorRepeat::Execute(CBotPlayer* pBot)
{
	if (!m_pChild)
		return BTR_FAILURE;

	int nCount = 0;
	// 当 m_nMaxRepeat == 0 时使用硬限制替代，防止死循环阻塞整个线程
	int nEffectiveMax = (m_nMaxRepeat > 0) ? m_nMaxRepeat : MAX_REPEAT_HARD_LIMIT;
	while (nCount < nEffectiveMax)
	{
		BTResult result = m_pChild->Execute(pBot);
		if (result == BTR_FAILURE)
			return BTR_SUCCESS;  // 重复直到失败，失败时返回SUCCESS
		nCount++;
	}
	// 达到硬限制：子节点始终返回SUCCESS/RUNNING，行为树配置可能有误
	if (m_nMaxRepeat == 0)
	{
		LG2("行为树警告: DecoratorRepeat [%s] 无限重复达到硬限制 %d 次, "
			"子节点未返回FAILURE，强制退出避免死循环\n",
			m_szName.c_str(), MAX_REPEAT_HARD_LIMIT);
	}
	return BTR_SUCCESS;
}

// ============================================================================
// 装饰节点 - 超时控制：累计计时，子节点超过指定时间未完成则返回失败
// ============================================================================
BTResult CBTDecoratorTimeout::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;

	// 不启用超时：直接透传子节点结果
	if (m_dwTimeoutMs == 0)
		return m_pChild->Execute(pBot);

	// 首次执行：记录起始时间（使用上下文缓存的帧时间，避免重复系统调用）
	if (m_dwStartTime == 0)
		m_dwStartTime = pBot->GetContext()->GetFrameTime();

	// 检查是否超时（累计计时，跨多次 tick）
	DWORD dwElapsed = pBot->GetContext()->GetFrameTime() - m_dwStartTime;
	if (dwElapsed >= m_dwTimeoutMs)
	{
		m_dwStartTime = 0;
		return BTR_FAILURE;
	}

	// 未超时：执行子节点
	BTResult result = m_pChild->Execute(pBot);

	// 子节点结束（成功或失败）：重置计时器，下次重新开始计时
	if (result == BTR_SUCCESS || result == BTR_FAILURE)
		m_dwStartTime = 0;

	return result;
}

// ============================================================================
// 装饰节点 - 冷却控制
// ============================================================================
BTResult CBTDecoratorCooldown::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;
	DWORD dwNow = pBot->GetContext()->GetFrameTime();
	if (m_dwLastExecTime > 0 && (dwNow - m_dwLastExecTime) < m_dwCooldownMs)
		return BTR_FAILURE;
	m_dwLastExecTime = dwNow;
	return m_pChild->Execute(pBot);
}

// ============================================================================
// 装饰节点 - 强制成功
// ============================================================================
BTResult CBTDecoratorSucceeder::Execute(CBotPlayer* pBot)
{
	if (m_pChild) m_pChild->Execute(pBot);
	return BTR_SUCCESS;
}

// ============================================================================
// 装饰节点 - 强制失败
// ============================================================================
BTResult CBTDecoratorFailer::Execute(CBotPlayer* pBot)
{
	if (m_pChild) m_pChild->Execute(pBot);
	return BTR_FAILURE;
}

// ============================================================================
// 条件节点 - HP低于阈值
// ============================================================================
BTResult CBTConditionLowHP::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;

	int hp = pBot->GetPropValue(PI_CURHP);
	int maxHp = pBot->GetPropValue(PI_MAXHP);
	if (maxHp <= 0) return BTR_FAILURE;

	int percent = hp * 100 / maxHp;
	return (percent < m_nPercent) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - MP低于阈值
// ============================================================================
BTResult CBTConditionLowMP::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;

	int mp = pBot->GetPropValue(PI_CURMP);
	int maxMp = pBot->GetPropValue(PI_MAXMP);
	if (maxMp <= 0) return BTR_FAILURE;

	int percent = mp * 100 / maxMp;
	return (percent < m_nPercent) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 是否有目标
// ============================================================================
BTResult CBTConditionHasTarget::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return pCtx->GetCachedTarget() != nullptr ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 是否在安全区
// ============================================================================
BTResult CBTConditionInSafeArea::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return pBot->InSafeArea() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 背包是否已满
// ============================================================================
BTResult CBTConditionBagFull::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	return (pBot->GetBagFree() <= 0) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 是否有指定物品
// ============================================================================
BTResult CBTConditionHasItem::Execute(CBotPlayer* pBot)
{
	if (!pBot || !m_szItemName) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return (pCtx->FindItemInBag(m_szItemName) > 0) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 技能是否就绪
// ============================================================================
BTResult CBTConditionSkillReady::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return pCtx->IsSkillReady(m_wMagicId) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 是否死亡
// ============================================================================
BTResult CBTConditionIsDead::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return pCtx->IsDead() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 目标距离范围
// ============================================================================
BTResult CBTConditionTargetDistance::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	CAliveObject* pTarget = pCtx->GetCachedTarget();
	if (!pTarget) return BTR_FAILURE;
	int nDist = pCtx->DistanceTo(pTarget);
	return (nDist >= m_nMinDist && nDist <= m_nMaxDist) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 目标类型
// 0、OBJ_DOWNITEM,		//掉落物品
// 1、OBJ_NPC,			//NPC
// 2、OBJ_VISIBLEEVENT,	//可见事件
// 3、OBJ_EVENT,		//事件
// 4、OBJ_PLAYER,		//玩家
// 5、OBJ_MONSTER,		//怪物
// 6、OBJ_PET,			//宠物
// 7、OBJ_GUARD,		//卫士、弓箭手
// 8、OBJ_TREE,			//树木
// ============================================================================
BTResult CBTConditionTargetType::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	CAliveObject* pTarget = pCtx->GetCachedTarget();
	if (!pTarget) return BTR_FAILURE;
	int nObjType = (int)pTarget->GetType();
	return (nObjType == m_nTargetType) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 附近是否有玩家
// ============================================================================
BTResult CBTConditionHasNearbyPlayer::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	xListHost<VISIBLE_OBJECT>* pVisList = pBot->GetVisibleObjectList();
	if (!pVisList) return BTR_FAILURE;
	xListHelper<VISIBLE_OBJECT> helper(pVisList);
	for (VISIBLE_OBJECT* pVO = helper.first(); pVO != nullptr; pVO = helper.next())
	{
		CMapObject* pObj = pVO->pObject;
		if (pObj && pObj->GetClassType() == CLS_ALIVEOBJECT && pObj->GetType() == OBJ_PLAYER)
		{
			CAliveObject* pAlive = dynamic_cast<CAliveObject*>(pObj);
			if (pAlive && pAlive != pBot && !pAlive->IsDeath())
			{
				int nDist = CBotHumanBehavior::Distance8(pBot->getX(), pBot->getY(), pAlive->getX(), pAlive->getY());
				if (nDist <= m_nRange)
					return BTR_SUCCESS;
			}
		}
	}
	return BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 周围怪物数量
// ============================================================================
BTResult CBTConditionMonsterCount::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	xListHost<VISIBLE_OBJECT>* pVisList = pBot->GetVisibleObjectList();
	if (!pVisList) return BTR_FAILURE;
	int nCount = 0;
	xListHelper<VISIBLE_OBJECT> helper(pVisList);
	for (VISIBLE_OBJECT* pVO = helper.first(); pVO != nullptr; pVO = helper.next())
	{
		CMapObject* pObj = pVO->pObject;
		if (pObj && pObj->GetClassType() == CLS_ALIVEOBJECT &&
			(pObj->GetType() == OBJ_MONSTER))
		{
			CAliveObject* pAlive = dynamic_cast<CAliveObject*>(pObj);
			if (pAlive && !pAlive->IsDeath())
			{
				int nDist = CBotHumanBehavior::Distance8(pBot->getX(), pBot->getY(), pAlive->getX(), pAlive->getY());
				if (nDist <= m_nRange)
					nCount++;
			}
		}
	}
	if (m_nMode == 0) return (nCount >= m_nCount) ? BTR_SUCCESS : BTR_FAILURE;
	if (m_nMode == 1) return (nCount <= m_nCount) ? BTR_SUCCESS : BTR_FAILURE;
	if (m_nMode == 2) return (nCount == m_nCount) ? BTR_SUCCESS : BTR_FAILURE;
	return BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 背包中是否有药水
// ============================================================================
BTResult CBTConditionHasPotion::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return pCtx->HasPotionInBag() ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 地上是否有可拾取物品
// ============================================================================
BTResult CBTConditionHasDroppedItem::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	xListHost<VISIBLE_OBJECT>* pItemsList = pBot->GetVisibleItemsList();
	if (!pItemsList) return BTR_FAILURE;
	xListHelper<VISIBLE_OBJECT> helper(pItemsList);
	for (VISIBLE_OBJECT* pVO = helper.first(); pVO != nullptr; pVO = helper.next())
	{
		CMapObject* pObj = pVO->pObject;
		if (pObj && (pObj->GetClassType() == CLS_DOWNITEM))
		{
			int nDist = CBotHumanBehavior::Distance8(pBot->getX(), pBot->getY(), pObj->getX(), pObj->getY());
			if (nDist <= m_nRange)
				return BTR_SUCCESS;
		}
	}
	return BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 血量百分比范围
// ============================================================================
BTResult CBTConditionHPRange::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	int hp = pBot->GetPropValue(PI_CURHP);
	int maxHp = pBot->GetPropValue(PI_MAXHP);
	if (maxHp <= 0) return BTR_FAILURE;
	int percent = hp * 100 / maxHp;
	return (percent >= m_nMinPercent && percent <= m_nMaxPercent) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 条件节点 - 游戏内时间段
// ============================================================================
BTResult CBTConditionTimeOfDay::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	SYSTEMTIME st;
	GetLocalTime(&st);
	int nHour = st.wHour;
	int nPeriod = 0;
	if (nHour >= 6 && nHour < 18) nPeriod = 0;
	else if (nHour >= 18 && nHour < 21) nPeriod = 2;
	else if (nHour >= 21 || nHour < 4) nPeriod = 1;
	else nPeriod = 3;
	return (nPeriod == m_nPeriod) ? BTR_SUCCESS : BTR_FAILURE;
}

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
	return pBot->SimulateUseItem(m_szUseItemName) ? BTR_SUCCESS : BTR_FAILURE;
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
	if (!pBot || !m_szMessage || m_szMessage[0] == '\0')
		return BTR_FAILURE;
	pBot->SimulateSay(m_szMessage);
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
	if (!pBot || !m_szItemName) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	DWORD dwItemIndex = pCtx->FindItemInBag(m_szItemName);
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
	if (!pBot || !m_szItemName || m_szItemName[0] == '\0')
		return BTR_FAILURE;
	return pBot->SimulateEquipItem(m_szItemName) ? BTR_SUCCESS : BTR_FAILURE;
}

// ============================================================================
// 动作节点 - 召唤宠物
// ============================================================================
BTResult CBTActionSummonPet::Execute(CBotPlayer* pBot)
{
	if (!pBot || !m_szPetName || m_szPetName[0] == '\0')
		return BTR_FAILURE;
	if (pBot->GetPetCount() >= 5) return BTR_FAILURE;
	BOOL bSuccess = pBot->SummonMonster(m_szPetName, TRUE, pBot->getX(), pBot->getY()) != nullptr;
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
	if (m_szTargetName && m_szTargetName[0] != '\0')
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
					if (strcmp(pMapObj->GetName(), m_szTargetName) == 0)
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
	if (!m_szPlayerName || m_szPlayerName[0] == '\0') return BTR_FAILURE;
	switch (m_nAction)
	{
	case 0: case 1: // 0创建队伍 // 1邀请组队
		return pBot->AddGroupMember(m_szPlayerName) ? BTR_SUCCESS : BTR_FAILURE;
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
// 行为树管理器实现
// ============================================================================
// ============================================================================
CBotBehaviorTree::CBotBehaviorTree() : m_rootNode(nullptr)
{
}

CBotBehaviorTree::~CBotBehaviorTree()
{
}

BOOL CBotBehaviorTree::LoadFromFile(const char* pszFile)
{
	if (pszFile == nullptr || pszFile[0] == '\0')
	{
		LG2("行为树: 文件路径为空\n");
		return FALSE;
	}

	TiXmlDocument doc;
	if (!doc.LoadFile(pszFile))
	{
		LG2("行为树: 加载文件失败 [%s]\n", pszFile);
		return FALSE;
	}

	TiXmlElement* pRoot = doc.RootElement();
	if (pRoot == nullptr)
	{
		LG2("行为树: XML根节点为空 [%s]\n", pszFile);
		return FALSE;
	}

	// 读取行为树名称
	const char* pszName = pRoot->Attribute("name");
	if (pszName)
		m_szName = pszName;

	// 解析根节点下的第一个行为节点
	TiXmlElement* pFirstNode = pRoot->FirstChildElement();
	if (pFirstNode == nullptr)
	{
		LG2("行为树: 没有子节点 [%s]\n", pszFile);
		return FALSE;
	}

	CBTNode* pNode = ParseNode(pFirstNode);
	if (pNode == nullptr)
	{
		LG2("行为树: 解析根行为节点失败 [%s]\n", pszFile);
		return FALSE;
	}

	m_rootNode.reset(pNode);
	LG2("行为树: 加载成功 [%s] 名称[%s]\n", pszFile, m_szName.c_str());
	return TRUE;
}

BTResult CBotBehaviorTree::Execute(CBotPlayer* pBot)
{
	if (!m_rootNode || !pBot)
		return BTR_FAILURE;

	return m_rootNode->Execute(pBot);
}

CBTNode* CBotBehaviorTree::ParseNode(TiXmlElement* pElement)
{
	if (pElement == nullptr)
		return nullptr;

	const char* pszType = pElement->Value();
	if (pszType == nullptr)
		return nullptr;

	CBTNode* pNode = CreateNodeByTypeName(pszType);
	if (pNode == nullptr)
	{
		LG2("行为树: 未知节点类型 [%s]\n", pszType);
		return nullptr;
	}

	// 设置节点名称
	const char* pszName = pElement->Attribute("name");
	if (pszName)
		pNode->SetName(pszName);

	// 解析节点特有属性
	if (strcmp(pszType, "ConditionLowHP") == 0)
	{
		int nPercent = 30;
		pElement->QueryIntAttribute("percent", &nPercent);
		static_cast<CBTConditionLowHP*>(pNode)->SetThreshold(nPercent);
	}
	else if (strcmp(pszType, "ConditionLowMP") == 0)
	{
		int nPercent = 20;
		pElement->QueryIntAttribute("percent", &nPercent);
		static_cast<CBTConditionLowMP*>(pNode)->SetThreshold(nPercent);
	}
	else if (strcmp(pszType, "ActionUsePotion") == 0)
	{
		int nType = 1;
		pElement->QueryIntAttribute("hpType", &nType);
		static_cast<CBTActionUsePotion*>(pNode)->SetPotionType(nType != 0);
	}
	else if (strcmp(pszType, "ActionUseItem") == 0)
	{
		const char* pszItemName = pElement->Attribute("itemName");
		static_cast<CBTActionUseItem*>(pNode)->SetUseItemName(pszItemName);
	}
	else if (strcmp(pszType, "ActionChangeAttackMode") == 0)
	{
		int nType = 1;
		pElement->QueryIntAttribute("attackMode", &nType);
		static_cast<CBTActionChangeAttackMode*>(pNode)->SetAttackMode((e_humanattackmode)nType);
	}
	else if (strcmp(pszType, "ActionRest") == 0)
	{
		int nDuration = 5000;
		pElement->QueryIntAttribute("duration", &nDuration);
		static_cast<CBTActionRest*>(pNode)->SetRestDuration((DWORD)nDuration);
	}
	else if (strcmp(pszType, "ActionUseSkill") == 0)
	{
		int nMagicId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		static_cast<CBTActionUseSkill*>(pNode)->SetMagicId((WORD)nMagicId);
	}
	else if (strcmp(pszType, "DecoratorRepeat") == 0)
	{
		int nMaxRepeat = 0;
		pElement->QueryIntAttribute("count", &nMaxRepeat);
		static_cast<CBTDecoratorRepeat*>(pNode)->SetMaxRepeat(nMaxRepeat);
	}
	else if (strcmp(pszType, "Probability") == 0)
	{
		int nChance = 50;
		pElement->QueryIntAttribute("chance", &nChance);
		static_cast<CBTProbability*>(pNode)->SetChance(nChance);
	}
	// === 新增节点属性解析 ===
	// 条件节点属性
	else if (strcmp(pszType, "ConditionHasItem") == 0)
	{
		const char* pszItemName = pElement->Attribute("itemName");
		static_cast<CBTConditionHasItem*>(pNode)->SetItemName(pszItemName);
	}
	else if (strcmp(pszType, "ConditionSkillReady") == 0)
	{
		int nMagicId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		static_cast<CBTConditionSkillReady*>(pNode)->SetMagicId((WORD)nMagicId);
	}
	else if (strcmp(pszType, "ConditionTargetDistance") == 0)
	{
		int nMin = 0, nMax = 16;
		pElement->QueryIntAttribute("minDist", &nMin);
		pElement->QueryIntAttribute("maxDist", &nMax);
		static_cast<CBTConditionTargetDistance*>(pNode)->SetRange(nMin, nMax);
	}
	else if (strcmp(pszType, "ConditionTargetType") == 0)
	{
		int nType = 0;
		pElement->QueryIntAttribute("targetType", &nType);
		static_cast<CBTConditionTargetType*>(pNode)->SetTargetType(nType);
	}
	else if (strcmp(pszType, "ConditionHasNearbyPlayer") == 0)
	{
		int nRange = 16;
		pElement->QueryIntAttribute("range", &nRange);
		static_cast<CBTConditionHasNearbyPlayer*>(pNode)->SetRange(nRange);
	}
	else if (strcmp(pszType, "ConditionMonsterCount") == 0)
	{
		int nRange = 10, nMode = 0, nCount = 3;
		pElement->QueryIntAttribute("range", &nRange);
		pElement->QueryIntAttribute("mode", &nMode);
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTConditionMonsterCount*>(pNode)->SetParams(nRange, nMode, nCount);
	}
	else if (strcmp(pszType, "ConditionHasDroppedItem") == 0)
	{
		int nRange = 5;
		pElement->QueryIntAttribute("range", &nRange);
		static_cast<CBTConditionHasDroppedItem*>(pNode)->SetRange(nRange);
	}
	else if (strcmp(pszType, "ConditionHasPotion") == 0)
	{
		int nHpType = 1;
		pElement->QueryIntAttribute("hpType", &nHpType);
		static_cast<CBTConditionHasPotion*>(pNode)->SetPotionType(nHpType != 0);
	}
	else if (strcmp(pszType, "ConditionHPRange") == 0)
	{
		int nMin = 0, nMax = 100;
		pElement->QueryIntAttribute("minPercent", &nMin);
		pElement->QueryIntAttribute("maxPercent", &nMax);
		static_cast<CBTConditionHPRange*>(pNode)->SetRange(nMin, nMax);
	}
	else if (strcmp(pszType, "ConditionTimeOfDay") == 0)
	{
		int nPeriod = 0;
		pElement->QueryIntAttribute("period", &nPeriod);
		static_cast<CBTConditionTimeOfDay*>(pNode)->SetPeriod(nPeriod);
	}
	// 动作节点属性
	else if (strcmp(pszType, "ActionSay") == 0)
	{
		const char* pszMsg = pElement->Attribute("message");
		static_cast<CBTActionSay*>(pNode)->SetMessage(pszMsg);
	}
	else if (strcmp(pszType, "ActionRecall") == 0)
	{
		int nType = 0;
		pElement->QueryIntAttribute("type", &nType);
		static_cast<CBTActionRecall*>(pNode)->SetRecallType(nType);
	}
	else if (strcmp(pszType, "ActionDelay") == 0)
	{
		int nMin = 200, nMax = 1000;
		pElement->QueryIntAttribute("minMs", &nMin);
		pElement->QueryIntAttribute("maxMs", &nMax);
		static_cast<CBTActionDelay*>(pNode)->SetDelayRange((DWORD)nMin, (DWORD)nMax);
	}
	else if (strcmp(pszType, "ActionAttackDir") == 0)
	{
		int nDir = 0;
		pElement->QueryIntAttribute("dir", &nDir);
		static_cast<CBTActionAttackDir*>(pNode)->SetDirection(nDir);
	}
	else if (strcmp(pszType, "ActionSpellCast") == 0)
	{
		int nMagicId = 0, nTargetX = 0, nTargetY = 0;
		DWORD dwTargetId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		pElement->QueryIntAttribute("targetX", &nTargetX);
		pElement->QueryIntAttribute("targetY", &nTargetY);
		pElement->QueryIntAttribute("targetId", (int*)&dwTargetId);
		static_cast<CBTActionSpellCast*>(pNode)->SetSpellParams((WORD)nMagicId, nTargetX, nTargetY, dwTargetId);
	}
	else if (strcmp(pszType, "ActionDropItem") == 0)
	{
		const char* pszName = pElement->Attribute("itemName");
		int nCount = 1;
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTActionDropItem*>(pNode)->SetDropParams(pszName, nCount);
	}
	else if (strcmp(pszType, "ActionEquipItem") == 0)
	{
		const char* pszName = pElement->Attribute("itemName");
		static_cast<CBTActionEquipItem*>(pNode)->SetItemName(pszName);
	}
	else if (strcmp(pszType, "ActionSummonPet") == 0)
	{
		const char* pszPetName = pElement->Attribute("petName");
		int nCount = 1;
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTActionSummonPet*>(pNode)->SetSummonParams(pszPetName, nCount);
	}
	else if (strcmp(pszType, "ActionFollow") == 0)
	{
		const char* pszName = pElement->Attribute("targetName");
		int nRange = 3;
		pElement->QueryIntAttribute("followRange", &nRange);
		static_cast<CBTActionFollow*>(pNode)->SetFollowParams(pszName, nRange);
	}
	else if (strcmp(pszType, "ActionGroup") == 0)
	{
		const char* pszName = pElement->Attribute("playerName");
		int nAction = 0;
		pElement->QueryIntAttribute("action", &nAction);
		static_cast<CBTActionGroup*>(pNode)->SetGroupParams(pszName, nAction);
	}
	else if (strcmp(pszType, "ActionMine") == 0)
	{
		int nDuration = 10000;
		pElement->QueryIntAttribute("duration", &nDuration);
		static_cast<CBTActionMine*>(pNode)->SetDuration((DWORD)nDuration);
	}
	// 复合/装饰节点属性
	else if (strcmp(pszType, "DecoratorTimeout") == 0)
	{
		int nMs = 5000;
		pElement->QueryIntAttribute("ms", &nMs);
		static_cast<CBTDecoratorTimeout*>(pNode)->SetTimeout((DWORD)nMs);
	}
	else if (strcmp(pszType, "DecoratorCooldown") == 0)
	{
		int nMs = 3000;
		pElement->QueryIntAttribute("ms", &nMs);
		static_cast<CBTDecoratorCooldown*>(pNode)->SetCooldown((DWORD)nMs);
	}

	// 递归解析子节点
	for (TiXmlElement* pChild = pElement->FirstChildElement(); pChild != nullptr; pChild = pChild->NextSiblingElement())
	{
		CBTNode* pChildNode = ParseNode(pChild);
		if (pChildNode)
			pNode->AddChild(std::unique_ptr<CBTNode>(pChildNode));
	}

	return pNode;
}

CBTNode* CBotBehaviorTree::CreateNodeByTypeName(const char* pszType)
{
	if (pszType == nullptr)
		return nullptr;

	// 静态工厂表：key=XML标签名, value=节点构造函数; O(1) 查找替代 44 次 strcmp 线性扫描
	static const std::unordered_map<std::string, std::function<CBTNode*()>> s_FactoryMap = {
		/* 复合节点 */
		{"Sequence",     []()->CBTNode*{ return new CBTSequence(); }},     // 序列节点
		{"Selector",     []()->CBTNode*{ return new CBTSelector(); }},     // 选择节点
		{"Parallel",     []()->CBTNode*{ return new CBTParallel(); }},     // 并行节点
		{"Random",       []()->CBTNode*{ return new CBTRandom(); }},       // 随机节点
		{"Probability",  []()->CBTNode*{ return new CBTProbability(); }},  // 概率节点
		{"MemSequence",  []()->CBTNode*{ return new CBTMemSequence(); }},  // 记忆序列
		{"MemSelector",  []()->CBTNode*{ return new CBTMemSelector(); }},  // 记忆选择

		/* 装饰节点 */
		{"Inverter",          []()->CBTNode*{ return new CBTDecoratorInverter(); }},  // 反转子节点的结果
		{"DecoratorRepeat",   []()->CBTNode*{ return new CBTDecoratorRepeat(); }},    // 重复执行子节点直到失败
		{"DecoratorTimeout",  []()->CBTNode*{ return new CBTDecoratorTimeout(); }},   // 超时控制
		{"DecoratorCooldown", []()->CBTNode*{ return new CBTDecoratorCooldown(); }},  // 冷却控制
		{"Succeeder",         []()->CBTNode*{ return new CBTDecoratorSucceeder(); }}, // 强制成功
		{"Failer",            []()->CBTNode*{ return new CBTDecoratorFailer(); }},    // 强制失败

		/* 条件节点 */
		{"ConditionLowHP",          []()->CBTNode*{ return new CBTConditionLowHP(); }},          // HP低于阈值
		{"ConditionLowMP",          []()->CBTNode*{ return new CBTConditionLowMP(); }},          // MP低于阈值
		{"ConditionHasTarget",      []()->CBTNode*{ return new CBTConditionHasTarget(); }},      // 是否有目标
		{"ConditionInSafeArea",     []()->CBTNode*{ return new CBTConditionInSafeArea(); }},     // 是否在安全区
		{"ConditionBagFull",        []()->CBTNode*{ return new CBTConditionBagFull(); }},        // 背包是否已满
		{"ConditionHasItem",        []()->CBTNode*{ return new CBTConditionHasItem(); }},        // 是否有指定物品
		{"ConditionSkillReady",     []()->CBTNode*{ return new CBTConditionSkillReady(); }},     // 技能是否就绪
		{"ConditionIsDead",         []()->CBTNode*{ return new CBTConditionIsDead(); }},         // 是否死亡
		{"ConditionTargetDistance", []()->CBTNode*{ return new CBTConditionTargetDistance(); }}, // 目标距离范围
		{"ConditionTargetType",     []()->CBTNode*{ return new CBTConditionTargetType(); }},     // 目标类型
		{"ConditionHasNearbyPlayer",[]()->CBTNode*{ return new CBTConditionHasNearbyPlayer(); }},// 附近是否有玩家
		{"ConditionMonsterCount",   []()->CBTNode*{ return new CBTConditionMonsterCount(); }},   // 周围怪物数量
		{"ConditionHasPotion",      []()->CBTNode*{ return new CBTConditionHasPotion(); }},      // 是否有药水
		{"ConditionHasDroppedItem", []()->CBTNode*{ return new CBTConditionHasDroppedItem(); }}, // 地上有掉落物
		{"ConditionHPRange",        []()->CBTNode*{ return new CBTConditionHPRange(); }},        // 血量百分比范围
		{"ConditionTimeOfDay",      []()->CBTNode*{ return new CBTConditionTimeOfDay(); }},      // 游戏内时间段

		/* 动作节点 */
		{"ActionChangeAttackMode", []()->CBTNode*{ return new CBTActionChangeAttackMode(); }}, // 切换攻击模式
		{"ActionUsePotion",        []()->CBTNode*{ return new CBTActionUsePotion(); }},        // 使用药水
		{"ActionUseItem",          []()->CBTNode*{ return new CBTActionUseItem(); }},          // 使用物品
		{"ActionAttack",           []()->CBTNode*{ return new CBTActionAttack(); }},           // 攻击目标
		{"ActionMoveToTarget",     []()->CBTNode*{ return new CBTActionMoveToTarget(); }},     // 移向目标
		{"ActionPatrol",           []()->CBTNode*{ return new CBTActionPatrol(); }},           // 巡逻移动
		{"ActionPickupItem",       []()->CBTNode*{ return new CBTActionPickupItem(); }},       // 拾取物品
		{"ActionFlee",             []()->CBTNode*{ return new CBTActionFlee(); }},             // 逃跑
		{"ActionRest",             []()->CBTNode*{ return new CBTActionRest(); }},             // 休息（站立恢复）
		{"ActionChat",             []()->CBTNode*{ return new CBTActionChat(); }},             // 聊天
		{"ActionUseSkill",         []()->CBTNode*{ return new CBTActionUseSkill(); }},         // 使用技能
		{"ActionSay",              []()->CBTNode*{ return new CBTActionSay(); }},              // 说出指定文本
		{"ActionRecall",           []()->CBTNode*{ return new CBTActionRecall(); }},           // 使用回城/传送卷轴
		{"ActionDelay",            []()->CBTNode*{ return new CBTActionDelay(); }},            // 延迟等待
		{"ActionAttackDir",        []()->CBTNode*{ return new CBTActionAttackDir(); }},        // 向指定方向攻击
		{"ActionSpellCast",        []()->CBTNode*{ return new CBTActionSpellCast(); }},        // 精确施法
		{"ActionDropItem",         []()->CBTNode*{ return new CBTActionDropItem(); }},         // 丢弃物品
		{"ActionEquipItem",        []()->CBTNode*{ return new CBTActionEquipItem(); }},        // 穿戴装备
		{"ActionSummonPet",        []()->CBTNode*{ return new CBTActionSummonPet(); }},        // 召唤宠物
		{"ActionFollow",           []()->CBTNode*{ return new CBTActionFollow(); }},           // 跟随目标
		{"ActionGroup",            []()->CBTNode*{ return new CBTActionGroup(); }},            // 组队操作
		{"ActionMine",             []()->CBTNode*{ return new CBTActionMine(); }},             // 挖矿
	};

	auto it = s_FactoryMap.find(pszType);
	return (it != s_FactoryMap.end()) ? it->second() : nullptr;
}
