#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "BotBehaviorTree.h"

// 前置声明
class CBotPlayer;

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
	CBTConditionHasItem() { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetItemName(const char* pszName) { m_szItemName = (pszName ? pszName : ""); }

private:
	std::string m_szItemName;  // 物品名称
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
// 条件节点 - 检查身上是否有指定Buff/状态
// 代替 ConditionHPRange(0,100)+Inverter hack，真正检查游戏内状态位
// buffId 枚举：
//   0 = 魔法盾(SI_BUBBLEDEFENCEUP)   1 = 护身真气(SI_HUSHENZHENQI)
//   2 = 金刚护体(SF_STRONGSHIELD)     3 = 幽灵盾(SI_DEFENCEUP)
//   4 = 神圣战甲术(SI_MAGDEFENCEUP)   5 = 隐身术(SI_CLOAK)
// ============================================================================
class CBTConditionHasBuff : public CBTNode
{
public:
	CBTConditionHasBuff() : m_nBuffId(0) { m_eType = BTNT_CONDITION; }
	virtual ~CBTConditionHasBuff() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetBuffId(int nId) { m_nBuffId = nId; }

private:
	int m_nBuffId;  // buff类型ID，见上方注释
};
