#include "StdAfx.h"
#include "BotBehaviorTree.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

// ============================================================================
// 序列节点实现
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
// 选择节点实现
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
// 并行节点实现
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
// 随机节点实现
// ============================================================================
BTResult CBTRandom::Execute(CBotPlayer* pBot)
{
	int nRand = CBotHumanBehavior::RandomRange(m_children.size(), RANDOM_OI);
	if (m_children[nRand])
		m_children[nRand]->Execute(pBot);
	return BTR_SUCCESS;
}

// ============================================================================
// 概率节点实现 - 按概率决定是否执行子节点
// ============================================================================
BTResult CBTProbability::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;
	if (CBotHumanBehavior::RandomPercent(m_nChance))
		return m_pChild->Execute(pBot);
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
BTResult CBTDecoratorRepeat::Execute(CBotPlayer* pBot)
{
	if (!m_pChild)
		return BTR_FAILURE;

	int nCount = 0;
	while (m_nMaxRepeat == 0 || nCount < m_nMaxRepeat)
	{
		BTResult result = m_pChild->Execute(pBot);
		if (result == BTR_FAILURE)
			return BTR_SUCCESS;  // 重复直到失败，失败时返回SUCCESS
		nCount++;
	}
	return BTR_SUCCESS;
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
// 行为树管理器实现
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

	// 复合节点
	if (strcmp(pszType, "Sequence") == 0)       return new CBTSequence(); // 序列节点
	if (strcmp(pszType, "Selector") == 0)       return new CBTSelector(); // 选择节点
	if (strcmp(pszType, "Parallel") == 0)       return new CBTParallel(); // 并行节点
	if (strcmp(pszType, "Random") == 0)			return new CBTRandom();	  // 随机节点
	if (strcmp(pszType, "Probability") == 0)	return new CBTProbability(); // 概率节点

	// 装饰节点
	if (strcmp(pszType, "Inverter") == 0)       return new CBTDecoratorInverter(); // 反转子节点的结果
	if (strcmp(pszType, "DecoratorRepeat") == 0) return new CBTDecoratorRepeat(); // 重复执行子节点直到失败

	// 条件节点
	if (strcmp(pszType, "ConditionLowHP") == 0)  return new CBTConditionLowHP(); // HP低于阈值
	if (strcmp(pszType, "ConditionLowMP") == 0)  return new CBTConditionLowMP(); // MP低于阈值
	if (strcmp(pszType, "ConditionHasTarget") == 0) return new CBTConditionHasTarget(); // 是否有目标
	if (strcmp(pszType, "ConditionInSafeArea") == 0) return new CBTConditionInSafeArea(); // 是否在安全区
	if (strcmp(pszType, "ConditionBagFull") == 0) return new CBTConditionBagFull(); // 背包是否已满

	// 动作节点
	if (strcmp(pszType, "ActionChangeAttackMode") == 0) return new CBTActionChangeAttackMode(); // 切换攻击模式
	if (strcmp(pszType, "ActionUsePotion") == 0) return new CBTActionUsePotion(); // 使用药水
	if (strcmp(pszType, "ActionUseItem") == 0) return new CBTActionUseItem(); // 使用物品
	if (strcmp(pszType, "ActionAttack") == 0)   return new CBTActionAttack(); // 攻击目标
	if (strcmp(pszType, "ActionMoveToTarget") == 0) return new CBTActionMoveToTarget(); // 移向目标
	if (strcmp(pszType, "ActionPatrol") == 0)   return new CBTActionPatrol(); // 巡逻移动
	if (strcmp(pszType, "ActionPickupItem") == 0) return new CBTActionPickupItem(); // 拾取物品
	if (strcmp(pszType, "ActionFlee") == 0)     return new CBTActionFlee(); // 逃跑
	if (strcmp(pszType, "ActionRest") == 0)     return new CBTActionRest(); // 休息（站立恢复）
	if (strcmp(pszType, "ActionChat") == 0)     return new CBTActionChat(); // 聊天
	if (strcmp(pszType, "ActionUseSkill") == 0) return new CBTActionUseSkill(); // 使用技能

	return nullptr;
}
