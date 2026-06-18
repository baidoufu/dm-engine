#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

// 前置声明
class CBotPlayer;

// ============================================================================
// 行为树节点类型枚举
// ============================================================================
enum BTNodeType
{
	BTNT_SEQUENCE,      // 序列节点 - 依次执行子节点，全部成功才返回TRUE
	BTNT_SELECTOR,      // 选择节点 - 依次执行子节点，任一成功即返回TRUE
	BTNT_PARALLEL,      // 并行节点 - 同时执行所有子节点
	BTNT_RANDOM,		// 随机节点 - 随机执行子节点
	BTNT_PROBABILITY,   // 概率节点 - 按概率决定是否执行子节点

	BTNT_DECORATOR,     // 装饰节点 - 修改子节点的返回结果

	BTNT_CONDITION,     // 条件节点 - 判断条件是否满足
	BTNT_ACTION,        // 动作节点 - 执行具体行为
};

// ============================================================================
// 行为树节点执行结果
// ============================================================================
enum BTResult
{
	BTR_SUCCESS,    // 执行成功
	BTR_FAILURE,    // 执行失败
	BTR_RUNNING,    // 正在执行中
};

// ============================================================================
// 行为树节点基类
// ============================================================================
class CBTNode
{
public:
	CBTNode() : m_eType(BTNT_ACTION), m_szName("") {}
	virtual ~CBTNode() {}

	// 执行节点，返回执行结果
	virtual BTResult Execute(CBotPlayer* pBot) = 0;

	// 添加子节点
	virtual VOID AddChild(std::unique_ptr<CBTNode> pNode) {}

	// 获取节点类型
	BTNodeType GetType() const { return m_eType; }

	// 设置/获取节点名称（用于调试和日志）
	VOID SetName(const char* pszName) { m_szName = pszName ? pszName : ""; }
	const char* GetName() const { return m_szName.c_str(); }

protected:
	BTNodeType m_eType;         // 节点类型
	std::string m_szName;       // 节点名称
};

// ============================================================================
// 序列节点 - 依次执行所有子节点，全部成功才返回SUCCESS
// ============================================================================
class CBTSequence : public CBTNode
{
public:
	CBTSequence() { m_eType = BTNT_SEQUENCE; }
	virtual ~CBTSequence() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
};

// ============================================================================
// 选择节点 - 依次执行子节点，任一成功即返回SUCCESS
// ============================================================================
class CBTSelector : public CBTNode
{
public:
	CBTSelector() { m_eType = BTNT_SELECTOR; }
	virtual ~CBTSelector() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
};

// ============================================================================
// 并行节点 - 同时执行所有子节点
// ============================================================================
class CBTParallel : public CBTNode
{
public:
	CBTParallel() { m_eType = BTNT_PARALLEL; }
	virtual ~CBTParallel() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
};

// ============================================================================
// 随机节点 - 随机执行子节点
// ============================================================================
class CBTRandom : public CBTNode
{
public:
	CBTRandom() { m_eType = BTNT_RANDOM; }
	virtual ~CBTRandom() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
};

// ============================================================================
// 概率节点 - 按概率决定是否执行子节点
// ============================================================================
class CBTProbability : public CBTNode
{
public:
	CBTProbability() : m_nChance(50) { m_eType = BTNT_PROBABILITY; }
	virtual ~CBTProbability() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}
	VOID SetChance(int nChance) { m_nChance = nChance; }

private:
	std::unique_ptr<CBTNode> m_pChild;
	int m_nChance;  // 执行概率(0-100)
};

// ============================================================================
// 装饰节点 - 反转子节点的结果
// ============================================================================
class CBTDecoratorInverter : public CBTNode
{
public:
	CBTDecoratorInverter() { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorInverter() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}

private:
	std::unique_ptr<CBTNode> m_pChild;
};

// ============================================================================
// 装饰节点 - 重复执行子节点直到失败
// ============================================================================
class CBTDecoratorRepeat : public CBTNode
{
public:
	CBTDecoratorRepeat() : m_nMaxRepeat(0) { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorRepeat() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}
	VOID SetMaxRepeat(int nCount) { m_nMaxRepeat = nCount; }

private:
	std::unique_ptr<CBTNode> m_pChild;
	int m_nMaxRepeat;  // 最大重复次数，0表示无限
};

// ============================================================================
// 条件节点 - HP低于阈值
// ============================================================================
class CBTConditionLowHP : public CBTNode
{
public:
	CBTConditionLowHP() : m_nPercent(30) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionLowHP() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetThreshold(int nPercent) { m_nPercent = nPercent; }

private:
	int m_nPercent;  // HP百分比阈值
};

// ============================================================================
// 条件节点 - MP低于阈值
// ============================================================================
class CBTConditionLowMP : public CBTNode
{
public:
	CBTConditionLowMP() : m_nPercent(20) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionLowMP() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetThreshold(int nPercent) { m_nPercent = nPercent; }

private:
	int m_nPercent;  // MP百分比阈值
};

// ============================================================================
// 条件节点 - 是否有目标
// ============================================================================
class CBTConditionHasTarget : public CBTNode
{
public:
	CBTConditionHasTarget() { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasTarget() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 条件节点 - 是否在安全区
// ============================================================================
class CBTConditionInSafeArea : public CBTNode
{
public:
	CBTConditionInSafeArea() { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionInSafeArea() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 条件节点 - 背包是否已满
// ============================================================================
class CBTConditionBagFull : public CBTNode
{
public:
	CBTConditionBagFull() { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionBagFull() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 使用药水
// ============================================================================
class CBTActionUsePotion : public CBTNode
{
public:
	CBTActionUsePotion() : m_bUseHP(TRUE) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionUsePotion() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetPotionType(BOOL bHP) { m_bUseHP = bHP; }

private:
	BOOL m_bUseHP;  // TRUE=使用HP药水, FALSE=使用MP药水
};

// ============================================================================
// 动作节点 - 使用物品
// ============================================================================
class CBTActionUseItem : public CBTNode
{
public:
	CBTActionUseItem() : m_szUseItemName(nullptr) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionUseItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetUseItemName(const char* szUseItemName) { m_szUseItemName = szUseItemName; }

private:
	const char* m_szUseItemName;  // TRUE=使用HP药水, FALSE=使用MP药水
};

// ============================================================================
// 动作节点 - 切换攻击模式
// ============================================================================
class CBTActionChangeAttackMode : public CBTNode
{
public:
	CBTActionChangeAttackMode()
	{
		m_eType = BTNT_ACTION;
		m_attackMode = HAM_PEACE;
	}
	virtual ~CBTActionChangeAttackMode() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetAttackMode(e_humanattackmode attackMode) { m_attackMode = attackMode; }

private:
	e_humanattackmode m_attackMode;  // 攻击类型
};

// ============================================================================
// 动作节点 - 攻击目标
// ============================================================================
class CBTActionAttack : public CBTNode
{
public:
	CBTActionAttack() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionAttack() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 移向目标
// ============================================================================
class CBTActionMoveToTarget : public CBTNode
{
public:
	CBTActionMoveToTarget() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionMoveToTarget() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 巡逻移动
// ============================================================================
class CBTActionPatrol : public CBTNode
{
public:
	CBTActionPatrol() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionPatrol() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 拾取物品
// ============================================================================
class CBTActionPickupItem : public CBTNode
{
public:
	CBTActionPickupItem() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionPickupItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 逃跑
// ============================================================================
class CBTActionFlee : public CBTNode
{
public:
	CBTActionFlee() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionFlee() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 休息（站立恢复）
// ============================================================================
class CBTActionRest : public CBTNode
{
public:
	CBTActionRest() : m_dwRestDuration(5000) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionRest() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRestDuration(DWORD dwMs) { m_dwRestDuration = dwMs; }

private:
	DWORD m_dwRestDuration;  // 休息持续时间(ms)
};

// ============================================================================
// 动作节点 - 聊天
// ============================================================================
class CBTActionChat : public CBTNode
{
public:
	CBTActionChat() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionChat() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 动作节点 - 使用技能
// ============================================================================
class CBTActionUseSkill : public CBTNode
{
public:
	CBTActionUseSkill() : m_wMagicId(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionUseSkill() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetMagicId(WORD wId) { m_wMagicId = wId; }

private:
	WORD m_wMagicId;  // 技能ID，0表示自动选择
};

// ============================================================================
// 条件节点工厂函数类型
// ============================================================================
typedef std::function<BTResult(CBotPlayer*)> BTConditionFunc;
typedef std::function<BTResult(CBotPlayer*)> BTActionFunc;

// ============================================================================
// 通用条件节点 - 使用函数对象
// ============================================================================
class CBTConditionGeneric : public CBTNode
{
public:
	CBTConditionGeneric(BTConditionFunc func) : m_func(func) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionGeneric() {}

	BTResult Execute(CBotPlayer* pBot) override
	{
		if (m_func)
			return m_func(pBot) ? BTR_SUCCESS : BTR_FAILURE;
		return BTR_FAILURE;
	}

private:
	BTConditionFunc m_func;
};

// ============================================================================
// 通用动作节点 - 使用函数对象
// ============================================================================
class CBTActionGeneric : public CBTNode
{
public:
	CBTActionGeneric(BTActionFunc func) : m_func(func) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionGeneric() {}

	BTResult Execute(CBotPlayer* pBot) override
	{
		if (m_func)
			return m_func(pBot);
		return BTR_FAILURE;
	}

private:
	BTActionFunc m_func;
};

// ============================================================================
// 行为树管理器 - 负责行为树的加载、执行和管理
// ============================================================================
class CBotBehaviorTree
{
public:
	CBotBehaviorTree();
	~CBotBehaviorTree();

	// 从XML文件加载行为树配置
	BOOL LoadFromFile(const char* pszFile);

	// 执行行为树
	BTResult Execute(CBotPlayer* pBot);

	// 是否已加载
	BOOL IsLoaded() const { return m_rootNode != nullptr; }

	// 获取行为树名称
	const char* GetName() const { return m_szName.c_str(); }
	VOID SetName(const char* pszName) { m_szName = pszName ? pszName : ""; }

private:
	// 解析XML节点，递归构建行为树
	CBTNode* ParseNode(class TiXmlElement* pElement);

	// 根据类型名称创建节点
	CBTNode* CreateNodeByTypeName(const char* pszType);

	std::unique_ptr<CBTNode> m_rootNode;  // 根节点
	std::string m_szName;                  // 行为树名称
};
