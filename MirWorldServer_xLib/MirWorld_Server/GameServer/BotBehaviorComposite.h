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
