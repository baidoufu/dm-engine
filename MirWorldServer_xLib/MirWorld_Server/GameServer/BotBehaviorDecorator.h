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
// 装饰节点 - 周期性触发器：每N毫秒执行一次子节点，返回子节点真实结果
// 代替 DecoratorTimeout+Succeeder hack，语义更清晰
// 周期内返回 FAILURE（不阻塞 Selector 优先级链继续往下）
// 触发时执行子节点并返回子节点的结果
// ============================================================================
class CBTDecoratorPeriodic : public CBTNode
{
public:
	CBTDecoratorPeriodic() : m_dwPeriodMs(30000), m_dwLastExecTime(0) { m_eType = BTNT_DECORATOR; }
	virtual ~CBTDecoratorPeriodic() {}

	BTResult Execute(CBotPlayer* pBot) override;
	VOID AddChild(std::unique_ptr<CBTNode> pNode) override
	{
		m_pChild = std::move(pNode);
	}
	VOID SetPeriod(DWORD dwMs) { m_dwPeriodMs = dwMs; }

private:
	std::unique_ptr<CBTNode> m_pChild;
	DWORD m_dwPeriodMs;      // 触发周期(ms)
	DWORD m_dwLastExecTime;  // 上次触发时间，0=首次无条件触发
};
