#include "StdAfx.h"
#include "BotBehaviorTree.h"
#include "BotBehaviorComposite.h"
#include "BotBehaviorDecorator.h"
#include "BotBehaviorCondition.h"
#include "BotBehaviorAction.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

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
	switch (str_hash(pszType))  // 注意：是运行时哈希
	{
	// 装饰节点
	case "DecoratorRepeat"_hash: {
		int nMaxRepeat = 0;
		pElement->QueryIntAttribute("count", &nMaxRepeat);
		static_cast<CBTDecoratorRepeat*>(pNode)->SetMaxRepeat(nMaxRepeat);
		break;
	}
	case "Probability"_hash: {
		int nChance = 50;
		pElement->QueryIntAttribute("chance", &nChance);
		static_cast<CBTProbability*>(pNode)->SetChance(nChance);
		break;
	}
	case "DecoratorTimeout"_hash: {
		int nMs = 5000;
		pElement->QueryIntAttribute("ms", &nMs);
		static_cast<CBTDecoratorTimeout*>(pNode)->SetTimeout((DWORD)nMs);
		break;
	}
	case "DecoratorCooldown"_hash: {
		int nMs = 3000;
		pElement->QueryIntAttribute("ms", &nMs);
		static_cast<CBTDecoratorCooldown*>(pNode)->SetCooldown((DWORD)nMs);
		break;
	}
	case "DecoratorPeriodic"_hash: {
		int nMs = 30000;
		pElement->QueryIntAttribute("ms", &nMs);
		static_cast<CBTDecoratorPeriodic*>(pNode)->SetPeriod((DWORD)nMs);
		break;
	}
	// 条件节点
	case "ConditionLowHP"_hash: {
		int nPercent = 30;
		pElement->QueryIntAttribute("percent", &nPercent);
		static_cast<CBTConditionLowHP*>(pNode)->SetThreshold(nPercent);
		break;
	}
	case "ConditionLowMP"_hash: {
		int nPercent = 20;
		pElement->QueryIntAttribute("percent", &nPercent);
		static_cast<CBTConditionLowMP*>(pNode)->SetThreshold(nPercent);
		break;
	}
	case "ConditionHasItem"_hash: {
		const char* pszItemName = pElement->Attribute("itemName");
		static_cast<CBTConditionHasItem*>(pNode)->SetItemName(pszItemName);
		break;
	}
	case "ConditionSkillReady"_hash: {
		int nMagicId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		static_cast<CBTConditionSkillReady*>(pNode)->SetMagicId((WORD)nMagicId);
		break;
	}
	case "ConditionTargetDistance"_hash: {
		int nMin = 0, nMax = 16;
		pElement->QueryIntAttribute("minDist", &nMin);
		pElement->QueryIntAttribute("maxDist", &nMax);
		static_cast<CBTConditionTargetDistance*>(pNode)->SetRange(nMin, nMax);
		break;
	}
	case "ConditionTargetType"_hash: {
		int nType = 0;
		pElement->QueryIntAttribute("targetType", &nType);
		static_cast<CBTConditionTargetType*>(pNode)->SetTargetType(nType);
		break;
	}
	case "ConditionHasNearbyPlayer"_hash: {
		int nRange = 16;
		pElement->QueryIntAttribute("range", &nRange);
		static_cast<CBTConditionHasNearbyPlayer*>(pNode)->SetRange(nRange);
		break;
	}
	case "ConditionMonsterCount"_hash: {
		int nRange = 10, nMode = 0, nCount = 3;
		pElement->QueryIntAttribute("range", &nRange);
		pElement->QueryIntAttribute("mode", &nMode);
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTConditionMonsterCount*>(pNode)->SetParams(nRange, nMode, nCount);
		break;
	}
	case "ConditionHasDroppedItem"_hash: {
		int nRange = 5;
		pElement->QueryIntAttribute("range", &nRange);
		static_cast<CBTConditionHasDroppedItem*>(pNode)->SetRange(nRange);
		break;
	}
	case "ConditionHasPotion"_hash: {
		int nHpType = 1;
		pElement->QueryIntAttribute("hpType", &nHpType);
		static_cast<CBTConditionHasPotion*>(pNode)->SetPotionType(nHpType != 0);
		break;
	}
	case "ConditionHPRange"_hash: {
		int nMin = 0, nMax = 100;
		pElement->QueryIntAttribute("minPercent", &nMin);
		pElement->QueryIntAttribute("maxPercent", &nMax);
		static_cast<CBTConditionHPRange*>(pNode)->SetRange(nMin, nMax);
		break;
	}
	case "ConditionTimeOfDay"_hash: {
		int nPeriod = 0;
		pElement->QueryIntAttribute("period", &nPeriod);
		static_cast<CBTConditionTimeOfDay*>(pNode)->SetPeriod(nPeriod);
		break;
	}
	case "ConditionHasBuff"_hash: {
		int nBuffId = 0;
		pElement->QueryIntAttribute("buffId", &nBuffId);
		static_cast<CBTConditionHasBuff*>(pNode)->SetBuffId(nBuffId);
		break;
	}
	// 动作节点
	case "ActionUsePotion"_hash: {
		int nType = 1;
		pElement->QueryIntAttribute("hpType", &nType);
		static_cast<CBTActionUsePotion*>(pNode)->SetPotionType(nType != 0);
		break;
	}
	case "ActionUseItem"_hash: {
		const char* pszItemName = pElement->Attribute("itemName");
		static_cast<CBTActionUseItem*>(pNode)->SetUseItemName(pszItemName);
		break;
	}
	case "ActionChangeAttackMode"_hash: {
		int nType = 1;
		pElement->QueryIntAttribute("attackMode", &nType);
		static_cast<CBTActionChangeAttackMode*>(pNode)->SetAttackMode((e_humanattackmode)nType);
		break;
	}
	case "ActionRest"_hash: {
		int nDuration = 5000;
		pElement->QueryIntAttribute("duration", &nDuration);
		static_cast<CBTActionRest*>(pNode)->SetRestDuration((DWORD)nDuration);
		break;
	}
	case "ActionRevive"_hash: {
		int nHpPercent = 100;
		int nTeleportHome = 1;
		int nDelay = 0;
		pElement->QueryIntAttribute("hpPercent", &nHpPercent);
		pElement->QueryIntAttribute("teleportHome", &nTeleportHome);
		pElement->QueryIntAttribute("delay", &nDelay);
		if (nHpPercent < 1) nHpPercent = 1;
		if (nHpPercent > 100) nHpPercent = 100;
		if (nDelay < 0) nDelay = 0;
		static_cast<CBTActionRevive*>(pNode)->SetReviveParams(nHpPercent, nTeleportHome != 0);
		static_cast<CBTActionRevive*>(pNode)->SetReviveDelay((DWORD)nDelay);
		break;
	}
	case "ActionUseSkill"_hash: {
		int nMagicId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		static_cast<CBTActionUseSkill*>(pNode)->SetMagicId((WORD)nMagicId);
		break;
	}
	case "ActionSay"_hash: {
		const char* pszMsg = pElement->Attribute("message");
		static_cast<CBTActionSay*>(pNode)->SetMessage(pszMsg);
		break;
	}
	case "ActionRecall"_hash: {
		int nType = 0;
		pElement->QueryIntAttribute("type", &nType);
		static_cast<CBTActionRecall*>(pNode)->SetRecallType(nType);
		break;
	}
	case "ActionDelay"_hash: {
		int nMin = 200, nMax = 1000;
		pElement->QueryIntAttribute("minMs", &nMin);
		pElement->QueryIntAttribute("maxMs", &nMax);
		static_cast<CBTActionDelay*>(pNode)->SetDelayRange((DWORD)nMin, (DWORD)nMax);
		break;
	}
	case "ActionAttackDir"_hash: {
		int nDir = 0;
		pElement->QueryIntAttribute("dir", &nDir);
		static_cast<CBTActionAttackDir*>(pNode)->SetDirection(nDir);
		break;
	}
	case "ActionSpellCast"_hash: {
		int nMagicId = 0, nTargetX = 0, nTargetY = 0;
		DWORD dwTargetId = 0;
		pElement->QueryIntAttribute("magicId", &nMagicId);
		pElement->QueryIntAttribute("targetX", &nTargetX);
		pElement->QueryIntAttribute("targetY", &nTargetY);
		pElement->QueryIntAttribute("targetId", (int*)&dwTargetId);
		static_cast<CBTActionSpellCast*>(pNode)->SetSpellParams((WORD)nMagicId, nTargetX, nTargetY, dwTargetId);
		break;
	}
	case "ActionDropItem"_hash: {
		const char* pszName = pElement->Attribute("itemName");
		int nCount = 1;
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTActionDropItem*>(pNode)->SetDropParams(pszName, nCount);
		break;
	}
	case "ActionEquipItem"_hash: {
		const char* pszName = pElement->Attribute("itemName");
		static_cast<CBTActionEquipItem*>(pNode)->SetItemName(pszName);
		break;
	}
	case "ActionSummonPet"_hash: {
		const char* pszPetName = pElement->Attribute("petName");
		int nCount = 1;
		pElement->QueryIntAttribute("count", &nCount);
		static_cast<CBTActionSummonPet*>(pNode)->SetSummonParams(pszPetName, nCount);
		break;
	}
	case "ActionFollow"_hash: {
		const char* pszName = pElement->Attribute("targetName");
		int nRange = 3;
		pElement->QueryIntAttribute("followRange", &nRange);
		static_cast<CBTActionFollow*>(pNode)->SetFollowParams(pszName, nRange);
		break;
	}
	case "ActionGroup"_hash: {
		const char* pszName = pElement->Attribute("playerName");
		int nAction = 0;
		pElement->QueryIntAttribute("action", &nAction);
		static_cast<CBTActionGroup*>(pNode)->SetGroupParams(pszName, nAction);
		break;
	}
	case "ActionMine"_hash: {
		int nDuration = 10000;
		pElement->QueryIntAttribute("duration", &nDuration);
		static_cast<CBTActionMine*>(pNode)->SetDuration((DWORD)nDuration);
		break;
	}
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
	static const SmallFlatMap<std::string, std::function<CBTNode*()>, 64> s_FactoryMap = {
		/* 复合节点 7个 */
		{"Sequence",     []()->CBTNode*{ return new CBTSequence(); }},     // 序列节点
		{"Selector",     []()->CBTNode*{ return new CBTSelector(); }},     // 选择节点
		{"Parallel",     []()->CBTNode*{ return new CBTParallel(); }},     // 并行节点
		{"Random",       []()->CBTNode*{ return new CBTRandom(); }},       // 随机节点
		{"Probability",  []()->CBTNode*{ return new CBTProbability(); }},  // 概率节点
		{"MemSequence",  []()->CBTNode*{ return new CBTMemSequence(); }},  // 记忆序列
		{"MemSelector",  []()->CBTNode*{ return new CBTMemSelector(); }},  // 记忆选择

		/* 装饰节点 7个 */
		{"Inverter",          []()->CBTNode*{ return new CBTDecoratorInverter(); }},  // 反转子节点的结果
		{"DecoratorRepeat",   []()->CBTNode*{ return new CBTDecoratorRepeat(); }},    // 重复执行子节点直到失败
		{"DecoratorTimeout",  []()->CBTNode*{ return new CBTDecoratorTimeout(); }},   // 超时控制
		{"DecoratorCooldown", []()->CBTNode*{ return new CBTDecoratorCooldown(); }},  // 冷却控制
		{"DecoratorPeriodic", []()->CBTNode*{ return new CBTDecoratorPeriodic(); }},  // 周期性触发（替代 Succeeder hack）
		{"Succeeder",         []()->CBTNode*{ return new CBTDecoratorSucceeder(); }}, // 强制成功
		{"Failer",            []()->CBTNode*{ return new CBTDecoratorFailer(); }},    // 强制失败

		/* 条件节点 17个 */
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
		{"ConditionHasBuff",        []()->CBTNode*{ return new CBTConditionHasBuff(); }},        // 是否有指定Buff
		{"ConditionHasNearbyPlayer",[]()->CBTNode*{ return new CBTConditionHasNearbyPlayer(); }},// 附近是否有玩家
		{"ConditionMonsterCount",   []()->CBTNode*{ return new CBTConditionMonsterCount(); }},   // 周围怪物数量
		{"ConditionHasPotion",      []()->CBTNode*{ return new CBTConditionHasPotion(); }},      // 是否有药水
		{"ConditionHasDroppedItem", []()->CBTNode*{ return new CBTConditionHasDroppedItem(); }}, // 地上有掉落物
		{"ConditionHPRange",        []()->CBTNode*{ return new CBTConditionHPRange(); }},        // 血量百分比范围
		{"ConditionTimeOfDay",      []()->CBTNode*{ return new CBTConditionTimeOfDay(); }},      // 游戏内时间段

		/* 动作节点 23个 */
		{"ActionChangeAttackMode", []()->CBTNode*{ return new CBTActionChangeAttackMode(); }}, // 切换攻击模式
		{"ActionUsePotion",        []()->CBTNode*{ return new CBTActionUsePotion(); }},        // 使用药水
		{"ActionUseItem",          []()->CBTNode*{ return new CBTActionUseItem(); }},          // 使用物品
		{"ActionAttack",           []()->CBTNode*{ return new CBTActionAttack(); }},           // 攻击目标
		{"ActionMoveToTarget",     []()->CBTNode*{ return new CBTActionMoveToTarget(); }},     // 移向目标
		{"ActionPatrol",           []()->CBTNode*{ return new CBTActionPatrol(); }},           // 巡逻移动
		{"ActionPickupItem",       []()->CBTNode*{ return new CBTActionPickupItem(); }},       // 拾取物品
		{"ActionFlee",             []()->CBTNode*{ return new CBTActionFlee(); }},             // 逃跑
		{"ActionRest",             []()->CBTNode*{ return new CBTActionRest(); }},             // 休息（站立恢复）
		{"ActionRevive",           []()->CBTNode*{ return new CBTActionRevive(); }},           // 复活（死亡后恢复）
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
