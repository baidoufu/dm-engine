#include "StdAfx.h"
#include "BotContext.h"
#include "BotPlayer.h"
#include "BotHumanBehavior.h"
#include "aliveobject.h"
#include "logicmap.h"
#include "gameworld.h"
#include "HumanPlayer.h"
#include "ItemBox.h"

// ============================================================================
// 目标查询 - 搜索视野范围内最近的怪物
// ============================================================================
CAliveObject* CBotContext::FindNearestMonster(int nRange)
{
	if (!m_pBot) return nullptr;

	CAliveObject* pBestTarget = nullptr;
	int nBestDist = nRange + 1;

	xListHost<VISIBLE_OBJECT>* pList = m_pBot->GetVisibleObjectList();
	if (pList && pList->getCount() > 0)
	{
		xListHost<VISIBLE_OBJECT>::xListNode* pNode = pList->getHead();
		while (pNode)
		{
			VISIBLE_OBJECT* pVO = pNode->getObject();
			if (pVO && pVO->pObject && pVO->pObject->GetType() == OBJ_MONSTER)
			{
				CAliveObject* pMonster = static_cast<CAliveObject*>(pVO->pObject);
				if (IsTargetValid(pMonster))
				{
					int dist = CBotHumanBehavior::Distance8(
						m_pBot->getX(), m_pBot->getY(),
						pMonster->getX(), pMonster->getY());
					if (dist < nBestDist)
					{
						nBestDist = dist;
						pBestTarget = pMonster;
					}
				}
			}
			pNode = pNode->getNext();
		}
	}

	// 更新缓存目标
	if (pBestTarget)
	{
		m_refCachedTarget.SetObject(pBestTarget);
		m_dwTargetUpdateTime = m_dwFrameTime;
	}

	return pBestTarget;
}

// ============================================================================
// 预计算目标：在 BT 执行前调用，将重型 FindNearestMonster 提前到冷路径
// 同一帧内 BT 节点再调用 GetCachedTarget 时直接命中缓存，O(1) 返回
// ============================================================================
VOID CBotContext::PrecomputeTarget()
{
	// 仅当缓存失效时才触发搜索（同一帧内第二次调用无开销）
	if (!m_refCachedTarget.IsValid())
	{
		m_refCachedTarget.SetObject(nullptr);
		FindNearestMonster();
		return;
	}
	CAliveObject* pTarget = m_refCachedTarget.getObject();
	if (!pTarget || pTarget->GetType() != OBJ_MONSTER || !IsTargetValid(pTarget))
	{
		FindNearestMonster();
	}
}

// ============================================================================
// 获取缓存目标（保留 lazy 逻辑作为兜底：目标在 BT 执行期间异常失效时仍能工作）
// ============================================================================
CAliveObject* CBotContext::GetCachedTarget()
{
	// 先验证引用是否仍然有效
	if (!m_refCachedTarget.IsValid())
	{
		m_refCachedTarget.SetObject(nullptr);
		return FindNearestMonster();
	}
	CAliveObject* pTarget = m_refCachedTarget.getObject();
	if (pTarget && pTarget->GetType() == OBJ_MONSTER)
	{
		if (IsTargetValid(pTarget))
			return pTarget;
	}
	// 缓存失效，重新搜索
	return FindNearestMonster();
}

// ============================================================================
// 验证目标是否有效
// ============================================================================
BOOL CBotContext::IsTargetValid(CAliveObject* pTarget)
{
	if (pTarget == nullptr) return FALSE;
	if (pTarget->IsDeath()) return FALSE;
	// 检查距离
	int dist = CBotHumanBehavior::Distance8(
		m_pBot->getX(), m_pBot->getY(),
		pTarget->getX(), pTarget->getY());
	return dist <= BOT_VIEW_RANGE;
}

// ============================================================================
// 自身状态查询
// ============================================================================
int CBotContext::GetHpPercent()
{
	if (!m_pBot) return 0;
	int hp = m_pBot->GetPropValue(PI_CURHP);
	int maxHp = m_pBot->GetPropValue(PI_MAXHP);
	if (maxHp <= 0) return 0;
	return hp * 100 / maxHp;
}

int CBotContext::GetMpPercent()
{
	if (!m_pBot) return 0;
	int mp = m_pBot->GetPropValue(PI_CURMP);
	int maxMp = m_pBot->GetPropValue(PI_MAXMP);
	if (maxMp <= 0) return 0;
	return mp * 100 / maxMp;
}

BOOL CBotContext::IsDead()
{
	if (!m_pBot) return TRUE;
	return m_pBot->IsDeath();
}

BOOL CBotContext::InSafeArea()
{
	if (!m_pBot) return FALSE;
	return m_pBot->InSafeArea();
}

// ============================================================================
// 物品查询
// ============================================================================
BOOL CBotContext::HasPotionInBag()
{
	return FindPotionInBag(TRUE) != 0 || FindPotionInBag(FALSE) != 0;
}

DWORD CBotContext::FindPotionInBag(BOOL bHP)
{
	if (!m_pBot) return 0;

	CItemBox& bag = m_pBot->GetBag();
	// 遍历背包查找药水
	// HP药水名称特征：金创药、强效金创药等
	// MP药水名称特征：魔法药、强效魔法药等
	const char* pszKeyword = bHP ? "金创药" : "魔法药";

	ITEM* pItem = bag.FindItem(pszKeyword, FALSE);
	if (pItem)
		return pItem->dwMakeIndex;

	// 尝试查找强效版本
	char szName[64];
	sprintf_s(szName, "强效%s", pszKeyword);
	pItem = bag.FindItem(szName, FALSE);
	if (pItem)
		return pItem->dwMakeIndex;

	return 0;
}

DWORD CBotContext::FindItemInBag(const char* pszName)
{
	if (!m_pBot || pszName == nullptr) return 0;

	CItemBox& bag = m_pBot->GetBag();
	ITEM* pItem = bag.FindItem(pszName, FALSE);
	if (pItem)
		return pItem->dwMakeIndex;
	return 0;
}

// ============================================================================
// 技能查询
// ============================================================================
WORD CBotContext::FindBestAttackSkill(int nTargetDistance)
{
	if (!m_pBot) return 0;

	BYTE btPro = m_pBot->GetPro();

	// 遍历已学技能，选择权重最高的攻击技能
	if (btPro == PRO_WARRIOR)
	{
		if (nTargetDistance <= 2)
		{
			if (IsSkillReady(26)) return 26; // 烈火
			if (IsSkillReady(25)) return 25; // 半月
		}
	}
	else if (btPro == PRO_MAGICIAN)
	{
		if (nTargetDistance <= 8)
		{
			if (IsSkillReady(11)) return 11; // 雷电
			if (IsSkillReady(23)) return 23; // 爆裂火焰
		}
	}
	else if (btPro == PRO_TAOSHI)
	{
		if (nTargetDistance <= 7)
		{
			if (IsSkillReady(6))  return 6;  // 施毒
			if (IsSkillReady(67)) return 67; // 幽冥火咒
			if (IsSkillReady(13)) return 13; // 灵魂火符
		}
	}

	return 0;
}

BOOL CBotContext::IsSkillReady(WORD wSkillId)
{
	if (!m_pBot) return FALSE;
	USERMAGIC* pMagic = m_pBot->GetMagic(wSkillId);
	if (pMagic == nullptr) return FALSE;
	// 检查技能冷却时间
	if (!pMagic->useTimer.IsTimeOut(pMagic->pClass->wDelay)) return FALSE;
	return TRUE;
}

// ============================================================================
// 距离计算
// ============================================================================
int CBotContext::DistanceTo(CAliveObject* pTarget)
{
	if (!m_pBot || !pTarget) return 999;
	return CBotHumanBehavior::Distance8(
		m_pBot->getX(), m_pBot->getY(),
		pTarget->getX(), pTarget->getY());
}

// ============================================================================
// 方向计算
// ============================================================================
int CBotContext::DirectionTo(CAliveObject* pTarget)
{
	if (!m_pBot || !pTarget) return 0;
	return CBotHumanBehavior::GetDirection8(
		pTarget->getX(), pTarget->getY(),
		m_pBot->getX(), m_pBot->getY());
}

// ============================================================================
// 地图查询
// ============================================================================
UINT CBotContext::GetCurrentMapId()
{
	if (!m_pBot) return 0;
	return m_pBot->GetMapId();
}

// ============================================================================
// 坐标是否遮挡
// ============================================================================
BOOL CBotContext::IsWalkable(int x, int y)
{
	if (!m_pBot) return FALSE;
	CLogicMap* pMap = m_pBot->GetMap();
	if (!pMap) return FALSE;
	return !pMap->IsBlocked(x, y);
}
