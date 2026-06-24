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
// 复合节点 - 序列节点 - 依次执行所有子节点，全部成功才返回SUCCESS
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
// 复合节点 - 选择节点 - 依次执行子节点，任一成功即返回SUCCESS
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
// 复合节点 - 并行节点 - 同时执行所有子节点
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
// 复合节点 - 随机节点 - 随机执行子节点
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
// 复合节点 - 概率节点 - 按概率决定是否执行子节点
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
// 复合节点 - 记忆序列节点（带状态）- 失败后从上次失败节点重试
// ============================================================================
class CBTMemSequence : public CBTNode
{
public:
	CBTMemSequence() : m_nLastIndex(0) { m_eType = BTNT_SEQUENCE; }
	virtual ~CBTMemSequence() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}
	VOID Reset() { m_nLastIndex = 0; }

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
	int m_nLastIndex;  // 上次执行到的索引
};

// ============================================================================
// 复合节点 - 记忆选择节点（带状态）- 跳过已成功的子节点
// ============================================================================
class CBTMemSelector : public CBTNode
{
public:
	CBTMemSelector() : m_nLastIndex(0) { m_eType = BTNT_SELECTOR; }
	virtual ~CBTMemSelector() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_children.push_back(std::move(pNode));
	}
	VOID Reset() { m_nLastIndex = 0; }

private:
	std::vector<std::unique_ptr<CBTNode>> m_children;
	int m_nLastIndex;  // 上次执行到的索引
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
// 装饰节点 - 超时控制：子节点超过指定时间未完成则返回失败
// ============================================================================
class CBTDecoratorTimeout : public CBTNode
{
public:
	CBTDecoratorTimeout() : m_dwTimeoutMs(5000), m_dwStartTime(0) { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorTimeout() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}
	VOID SetTimeout(DWORD dwMs) { m_dwTimeoutMs = dwMs; }

private:
	std::unique_ptr<CBTNode> m_pChild;
	DWORD m_dwTimeoutMs;   // 超时时间(ms)，0 表示不启用超时
	DWORD m_dwStartTime;   // 本次执行的起始时间
};

// ============================================================================
// 装饰节点 - 冷却控制：冷却时间内重复执行返回失败
// ============================================================================
class CBTDecoratorCooldown : public CBTNode
{
public:
	CBTDecoratorCooldown() : m_dwCooldownMs(3000), m_dwLastExecTime(0) { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorCooldown() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}
	VOID SetCooldown(DWORD dwMs) { m_dwCooldownMs = dwMs; }

private:
	std::unique_ptr<CBTNode> m_pChild;
	DWORD m_dwCooldownMs;   // 冷却时间(ms)
	DWORD m_dwLastExecTime; // 上次执行时间
};

// ============================================================================
// 装饰节点 - 强制成功：无论子节点结果，固定返回成功
// ============================================================================
class CBTDecoratorSucceeder : public CBTNode
{
public:
	CBTDecoratorSucceeder() { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorSucceeder() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}

private:
	std::unique_ptr<CBTNode> m_pChild;
};

// ============================================================================
// 装饰节点 - 强制失败：无论子节点结果，固定返回失败
// ============================================================================
class CBTDecoratorFailer : public CBTNode
{
public:
	CBTDecoratorFailer() { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorFailer() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}

private:
	std::unique_ptr<CBTNode> m_pChild;
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
// 条件节点 - 是否有指定物品
// ============================================================================
class CBTConditionHasItem : public CBTNode
{
public:
	CBTConditionHasItem() : m_szItemName(nullptr) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetItemName(const char* pszName) { m_szItemName = pszName; }

private:
	const char* m_szItemName;  // 物品名称
};

// ============================================================================
// 条件节点 - 技能是否就绪（非冷却中）
// ============================================================================
class CBTConditionSkillReady : public CBTNode
{
public:
	CBTConditionSkillReady() : m_wMagicId(0) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionSkillReady() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetMagicId(WORD wId) { m_wMagicId = wId; }

private:
	WORD m_wMagicId;  // 技能ID
};

// ============================================================================
// 条件节点 - 是否死亡
// ============================================================================
class CBTConditionIsDead : public CBTNode
{
public:
	CBTConditionIsDead() { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionIsDead() {}

	BTResult Execute(CBotPlayer* pBot) override;
};

// ============================================================================
// 条件节点 - 目标距离范围
// ============================================================================
class CBTConditionTargetDistance : public CBTNode
{
public:
	CBTConditionTargetDistance() : m_nMinDist(0), m_nMaxDist(16) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionTargetDistance() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRange(int nMin, int nMax) { m_nMinDist = nMin; m_nMaxDist = nMax; }

private:
	int m_nMinDist;  // 最小距离
	int m_nMaxDist;  // 最大距离
};

// ============================================================================
// 条件节点 - 目标类型
// ============================================================================
class CBTConditionTargetType : public CBTNode
{
public:
	CBTConditionTargetType() : m_nTargetType(0) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionTargetType() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetTargetType(int nType) { m_nTargetType = nType; }

private:
	int m_nTargetType;  // 0=怪物, 1=玩家, 2=宠物
};

// ============================================================================
// 条件节点 - 附近是否有玩家
// ============================================================================
class CBTConditionHasNearbyPlayer : public CBTNode
{
public:
	CBTConditionHasNearbyPlayer() : m_nRange(16) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasNearbyPlayer() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRange(int nRange) { m_nRange = nRange; }

private:
	int m_nRange;  // 检测范围
};

// ============================================================================
// 条件节点 - 周围怪物数量
// ============================================================================
class CBTConditionMonsterCount : public CBTNode
{
public:
	CBTConditionMonsterCount() : m_nRange(10), m_nMode(0), m_nCount(3) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionMonsterCount() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetParams(int nRange, int nMode, int nCount) { m_nRange = nRange; m_nMode = nMode; m_nCount = nCount; }

private:
	int m_nRange;  // 检测范围
	int m_nMode;   // 比较模式: 0=>=, 1=<=, 2==
	int m_nCount;  // 比较数量
};

// ============================================================================
// 条件节点 - 背包中是否有药水
// ============================================================================
class CBTConditionHasPotion : public CBTNode
{
public:
	CBTConditionHasPotion() : m_bHpType(TRUE) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasPotion() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetPotionType(BOOL bHP) { m_bHpType = bHP; }

private:
	BOOL m_bHpType;  // TRUE=HP药水, FALSE=MP药水
};

// ============================================================================
// 条件节点 - 地上是否有可拾取物品
// ============================================================================
class CBTConditionHasDroppedItem : public CBTNode
{
public:
	CBTConditionHasDroppedItem() : m_nRange(5) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasDroppedItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRange(int nRange) { m_nRange = nRange; }

private:
	int m_nRange;  // 检测范围
};

// ============================================================================
// 条件节点 - 血量百分比范围
// ============================================================================
class CBTConditionHPRange : public CBTNode
{
public:
	CBTConditionHPRange() : m_nMinPercent(0), m_nMaxPercent(100) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHPRange() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRange(int nMin, int nMax) { m_nMinPercent = nMin; m_nMaxPercent = nMax; }

private:
	int m_nMinPercent;  // 最小HP百分比
	int m_nMaxPercent;  // 最大HP百分比
};

// ============================================================================
// 条件节点 - 游戏内时间段
// ============================================================================
class CBTConditionTimeOfDay : public CBTNode
{
public:
	CBTConditionTimeOfDay() : m_nPeriod(0) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionTimeOfDay() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetPeriod(int nPeriod) { m_nPeriod = nPeriod; }

private:
	int m_nPeriod;  // 0=白天, 1=夜晚, 2=黄昏, 3=黎明
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
// 动作节点 - 说出指定文本
// ============================================================================
class CBTActionSay : public CBTNode
{
public:
	CBTActionSay() : m_szMessage(nullptr) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionSay() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetMessage(const char* pszMsg) { m_szMessage = pszMsg; }

private:
	const char* m_szMessage;  // 聊天文本
};

// ============================================================================
// 动作节点 - 使用回城卷轴/随机卷轴/地牢卷轴/沙城回城卷
// ============================================================================
class CBTActionRecall : public CBTNode
{
public:
	CBTActionRecall() : m_nType(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionRecall() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetRecallType(int nType) { m_nType = nType; }

private:
	int m_nType;  // 0=回城卷轴, 1=随机卷轴, 2=地牢卷轴, 3=沙城回城卷
};

// ============================================================================
// 动作节点 - 延迟等待（带正态分布的人类反应偏差）
// ============================================================================
class CBTActionDelay : public CBTNode
{
public:
	CBTActionDelay() : m_dwMinMs(200), m_dwMaxMs(1000) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionDelay() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetDelayRange(DWORD dwMin, DWORD dwMax) { m_dwMinMs = dwMin; m_dwMaxMs = dwMax; }

private:
	DWORD m_dwMinMs;  // 最小延迟(ms)
	DWORD m_dwMaxMs;  // 最大延迟(ms)
};

// ============================================================================
// 动作节点 - 向指定方向攻击（精确控制）
// ============================================================================
class CBTActionAttackDir : public CBTNode
{
public:
	CBTActionAttackDir() : m_nDir(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionAttackDir() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetDirection(int nDir) { m_nDir = nDir; }

private:
	int m_nDir;  // 攻击方向 0-7
};

// ============================================================================
// 动作节点 - 精确施法
// ============================================================================
class CBTActionSpellCast : public CBTNode
{
public:
	CBTActionSpellCast() : m_wMagicId(0), m_nTargetX(0), m_nTargetY(0), m_dwTargetId(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionSpellCast() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetSpellParams(WORD wMagicId, int nTargetX, int nTargetY, DWORD dwTargetId) 
		{ m_wMagicId = wMagicId; m_nTargetX = nTargetX; m_nTargetY = nTargetY; m_dwTargetId = dwTargetId; }

private:
	WORD m_wMagicId;    // 技能ID
	int m_nTargetX;     // 目标X
	int m_nTargetY;     // 目标Y
	DWORD m_dwTargetId; // 目标ID（0表示指向坐标）
};

// ============================================================================
// 动作节点 - 丢弃物品
// ============================================================================
class CBTActionDropItem : public CBTNode
{
public:
	CBTActionDropItem() : m_szItemName(nullptr), m_nCount(1) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionDropItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetDropParams(const char* pszName, int nCount) { m_szItemName = pszName; m_nCount = nCount; }

private:
	const char* m_szItemName;  // 物品名称
	int m_nCount;              // 丢弃数量
};

// ============================================================================
// 动作节点 - 穿戴装备
// ============================================================================
class CBTActionEquipItem : public CBTNode
{
public:
	CBTActionEquipItem() : m_szItemName(nullptr) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionEquipItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetItemName(const char* pszName) { m_szItemName = pszName; }

private:
	const char* m_szItemName;  // 装备名称
};

// ============================================================================
// 动作节点 - 召唤宠物
// ============================================================================
class CBTActionSummonPet : public CBTNode
{
public:
	CBTActionSummonPet() : m_szPetName(nullptr), m_nCount(1) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionSummonPet() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetSummonParams(const char* pszPetName, int nCount) { m_szPetName = pszPetName; m_nCount = nCount; }

private:
	const char* m_szPetName;  // 宠物名称（如"骷髅"）
	int m_nCount;              // 召唤数量
};

// ============================================================================
// 动作节点 - 跟随目标
// ============================================================================
class CBTActionFollow : public CBTNode
{
public:
	CBTActionFollow() : m_szTargetName(nullptr), m_nFollowRange(3) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionFollow() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetFollowParams(const char* pszName, int nRange) { m_szTargetName = pszName; m_nFollowRange = nRange; }

private:
	const char* m_szTargetName;  // 跟随目标名称（空=当前目标）
	int m_nFollowRange;          // 保持距离
};

// ============================================================================
// 动作节点 - 组队操作
// ============================================================================
class CBTActionGroup : public CBTNode
{
public:
	CBTActionGroup() : m_szPlayerName(nullptr), m_nAction(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionGroup() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetGroupParams(const char* pszName, int nAction) { m_szPlayerName = pszName; m_nAction = nAction; }

private:
	const char* m_szPlayerName;  // 目标玩家名称
	int m_nAction;               // 0=创建队伍, 1=邀请组队, 2=请求入队, 3=退出队伍, 4=开启组队, 5=关闭组队
};

// ============================================================================
// 动作节点 - 挖矿
// ============================================================================
class CBTActionMine : public CBTNode
{
public:
	CBTActionMine() : m_dwDuration(10000) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionMine() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetDuration(DWORD dwMs) { m_dwDuration = dwMs; }

private:
	DWORD m_dwDuration;  // 挖矿时长(ms)
};

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
	std::string m_szName;                 // 行为树名称
};
