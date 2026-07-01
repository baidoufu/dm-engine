#include "StdAfx.h"
#include "BotBehaviorCondition.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

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
	if (!pBot || m_szItemName.empty()) return BTR_FAILURE;
	CBotContext* pCtx = pBot->GetContext();
	if (!pCtx) return BTR_FAILURE;
	return (pCtx->FindItemInBag(m_szItemName.c_str()) > 0) ? BTR_SUCCESS : BTR_FAILURE;
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
// 条件节点 - 检查是否有指定Buff
// ============================================================================
BTResult CBTConditionHasBuff::Execute(CBotPlayer* pBot)
{
	if (!pBot) return BTR_FAILURE;
	BOOL bHas = FALSE;
	switch (m_nBuffId)
	{
	case 0: bHas = pBot->IsStatusSet(SI_BUBBLEDEFENCEUP);  break; // 魔法盾
	case 1: bHas = pBot->IsStatusSet(SI_HUSHENZHENQI);     break; // 护身真气
	case 2: bHas = pBot->IsSystemFlagSeted(SF_STRONGSHIELD); break;// 金刚护体
	case 3: bHas = pBot->IsStatusSet(SI_DEFENCEUP);        break; // 幽灵盾
	case 4: bHas = pBot->IsStatusSet(SI_MAGDEFENCEUP);     break; // 神圣战甲术
	case 5: bHas = pBot->IsStatusSet(SI_CLOAK);            break; // 隐身术
	default: return BTR_FAILURE;
	}
	return bHas ? BTR_SUCCESS : BTR_FAILURE;
}
