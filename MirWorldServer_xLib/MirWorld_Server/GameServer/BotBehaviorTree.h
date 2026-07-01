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
	std::unique_ptr<CBTNode> ParseNode(class TiXmlElement* pElement);

	// 根据类型名称创建节点
	std::unique_ptr<CBTNode> CreateNodeByTypeName(const char* pszType);

	std::unique_ptr<CBTNode> m_rootNode;  // 根节点
	std::string m_szName;                 // 行为树名称
};
