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
	CBTActionUseItem() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionUseItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetUseItemName(const char* szUseItemName) { m_szUseItemName = (szUseItemName ? szUseItemName : ""); }

private:
	std::string m_szUseItemName;  // 物品名称
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
// 动作节点 - 复活（死亡后复活，恢复HP/MP，可选回城）
// ============================================================================
class CBTActionRevive : public CBTNode
{
public:
	CBTActionRevive() : m_nHpPercent(100), m_bTeleportHome(TRUE), m_dwDelay(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionRevive() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetReviveParams(int nHpPercent, BOOL bTeleportHome)
	{ m_nHpPercent = nHpPercent; m_bTeleportHome = bTeleportHome; }
	VOID SetReviveDelay(DWORD dwMs) { m_dwDelay = dwMs; }

private:
	int m_nHpPercent;        // 复活后HP恢复百分比(1-100)
	BOOL m_bTeleportHome;    // TRUE=回城复活(刷新视野), FALSE=原地复活
	DWORD m_dwDelay;         // 死亡后复活延迟(ms)，0=立即复活
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
	CBTActionSay() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionSay() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetMessage(const char* pszMsg) { m_szMessage = (pszMsg ? pszMsg : ""); }

private:
	std::string m_szMessage;  // 聊天文本（复制自XML，避免悬空指针）
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
	CBTActionDropItem() : m_nCount(1) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionDropItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetDropParams(const char* pszName, int nCount) { m_szItemName = (pszName ? pszName : ""); m_nCount = nCount; }

private:
	std::string m_szItemName;  // 物品名称
	int m_nCount;              // 丢弃数量
};

// ============================================================================
// 动作节点 - 穿戴装备
// ============================================================================
class CBTActionEquipItem : public CBTNode
{
public:
	CBTActionEquipItem() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionEquipItem() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetItemName(const char* pszName) { m_szItemName = (pszName ? pszName : ""); }

private:
	std::string m_szItemName;  // 装备名称
};

// ============================================================================
// 动作节点 - 召唤宠物
// ============================================================================
class CBTActionSummonPet : public CBTNode
{
public:
	CBTActionSummonPet() : m_nCount(1) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionSummonPet() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetSummonParams(const char* pszPetName, int nCount) { m_szPetName = (pszPetName ? pszPetName : ""); m_nCount = nCount; }

private:
	std::string m_szPetName;  // 宠物名称（如"骷髅"）
	int m_nCount;              // 召唤数量
};

// ============================================================================
// 动作节点 - 跟随目标
// ============================================================================
class CBTActionFollow : public CBTNode
{
public:
	CBTActionFollow() : m_nFollowRange(3) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionFollow() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetFollowParams(const char* pszName, int nRange) { m_szTargetName = (pszName ? pszName : ""); m_nFollowRange = nRange; }

private:
	std::string m_szTargetName;  // 跟随目标名称（空=当前目标）
	int m_nFollowRange;          // 保持距离
};

// ============================================================================
// 动作节点 - 组队操作
// ============================================================================
class CBTActionGroup : public CBTNode
{
public:
	CBTActionGroup() : m_nAction(0) { m_eType = BTNT_ACTION; }
	virtual ~CBTActionGroup() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID SetGroupParams(const char* pszName, int nAction) { m_szPlayerName = (pszName ? pszName : ""); m_nAction = nAction; }

private:
	std::string m_szPlayerName;  // 目标玩家名称
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
// 动作节点 - 放弃锁定目标（清除当前锁定目标）
// ============================================================================
class CBTActionUnlockTarget : public CBTNode
{
public:
	CBTActionUnlockTarget() { m_eType = BTNT_ACTION; }
	virtual ~CBTActionUnlockTarget() {}

	BTResult Execute(CBotPlayer* pBot) override;
};
