#include "StdAfx.h"
#include "BotBehaviorComposite.h"
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
