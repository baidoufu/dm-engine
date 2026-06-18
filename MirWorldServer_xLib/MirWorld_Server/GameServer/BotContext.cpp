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
		m_dwTargetUpdateTime = CFrameTime::GetFrameTime();
	}

	return pBestTarget;
}

// ============================================================================
// 获取缓存目标
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
	return m_pBot->GetPropValue(PI_CURHP) <= 0;
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
	WORD wBestSkill = 0;
	int nBestWeight = 0;

	// 遍历已学技能，选择权重最高的攻击技能
	// 简化实现：根据职业和距离选择
	if (btPro == PRO_WARRIOR)
	{
		// 战士：近距离使用技能攻击
		if (nTargetDistance <= 2)
		{
			// 优先烈火、半月等近战技能
			USERMAGIC* pMagic = m_pBot->GetMagic(25); // 烈火
			if (pMagic && IsSkillReady(25)) return 25;
			pMagic = m_pBot->GetMagic(12); // 半月
			if (pMagic && IsSkillReady(12)) return 12;
		}
	}
	else if (btPro == PRO_MAGICIAN)
	{
		// 法师：中远距离使用魔法攻击
		if (nTargetDistance <= 8)
		{
			USERMAGIC* pMagic = m_pBot->GetMagic(8); // 雷电
			if (pMagic && IsSkillReady(8)) return 8;
			pMagic = m_pBot->GetMagic(26); // 爆裂火焰
			if (pMagic && IsSkillReady(26)) return 26;
		}
	}
	else if (btPro == PRO_TAOSHI)
	{
		// 道士：中距离使用施毒和灵魂火符
		if (nTargetDistance <= 7)
		{
			USERMAGIC* pMagic = m_pBot->GetMagic(13); // 施毒
			if (pMagic && IsSkillReady(13)) return 13;
			pMagic = m_pBot->GetMagic(27); // 灵魂火符
			if (pMagic && IsSkillReady(27)) return 27;
		}
	}

	return wBestSkill;
}

BOOL CBotContext::IsSkillReady(WORD wSkillId)
{
	if (!m_pBot) return FALSE;
	USERMAGIC* pMagic = m_pBot->GetMagic(wSkillId);
	if (pMagic == nullptr) return FALSE;
	// 检查技能冷却（简化实现）
	return TRUE;
}

// ============================================================================
// 距离/方向计算
// ============================================================================
int CBotContext::DistanceTo(CAliveObject* pTarget)
{
	if (!m_pBot || !pTarget) return 999;
	return CBotHumanBehavior::Distance8(
		m_pBot->getX(), m_pBot->getY(),
		pTarget->getX(), pTarget->getY());
}

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

BOOL CBotContext::IsWalkable(int x, int y)
{
	if (!m_pBot) return FALSE;
	CLogicMap* pMap = m_pBot->GetMap();
	if (!pMap) return FALSE;
	return !pMap->IsBlocked(x, y);
}
