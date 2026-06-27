#pragma once
#include "BTNode.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <random>
#include <sstream>

// ============================================================================
// XML 解析器 - 解析行为树 XML 配置
// ============================================================================
class XMLParser
{
public:
    static std::shared_ptr<BTNode> ParseFile(const std::wstring& filePath);
    static std::shared_ptr<BTNode> ParseString(const std::string& xmlContent);
    static std::string SerializeTree(std::shared_ptr<BTNode> root, const std::wstring& treeName);

private:
    static std::shared_ptr<BTNode> ParseWString(const std::wstring& xml);
    struct XMLElement
    {
        std::wstring tagName;
        std::map<std::wstring, std::wstring> attributes;
        std::wstring text;
        std::vector<XMLElement> children;
    };

    static XMLElement ParseXML(const std::wstring& xml);
    static std::shared_ptr<BTNode> BuildTree(const XMLElement& elem, BTNode* parent, int depth);
    static std::wstring ReadFileGBK(const std::wstring& path);
    static std::wstring Trim(const std::wstring& s);
    static size_t ParseElement(const std::wstring& xml, size_t pos, XMLElement& outElem);
    static void SkipWhitespace(const std::wstring& xml, size_t& pos);
    static std::wstring ReadUntil(const std::wstring& xml, size_t& pos, wchar_t delim);
    static std::wstring ReadName(const std::wstring& xml, size_t& pos);
    static std::wstring DecodeEntities(const std::wstring& text);
};

// ============================================================================
// 行为树执行引擎
// ============================================================================
class BTEngine
{
public:
    BTEngine();

    void Reset();
    void ExecuteFull(std::shared_ptr<BTNode> root);
    void ResetNodeStates(std::shared_ptr<BTNode> node);

    std::vector<LogEntry>& GetLogs() { return m_logs; }
    int GetStepCount() const { return m_stepCount; }
    ExecutionContext& GetContext() { return m_ctx; }

private:
    BTResult ExecuteNode(std::shared_ptr<BTNode> node);
    BTResult ExecuteSequence(std::shared_ptr<BTNode> node);
    BTResult ExecuteSelector(std::shared_ptr<BTNode> node);
    BTResult ExecuteParallel(std::shared_ptr<BTNode> node);
    BTResult ExecuteRandom(std::shared_ptr<BTNode> node);
    BTResult ExecuteProbability(std::shared_ptr<BTNode> node);
    BTResult ExecuteMemSequence(std::shared_ptr<BTNode> node);
    BTResult ExecuteMemSelector(std::shared_ptr<BTNode> node);
    BTResult ExecuteInverter(std::shared_ptr<BTNode> node);
    BTResult ExecuteDecoratorRepeat(std::shared_ptr<BTNode> node);
    BTResult ExecuteDecoratorTimeout(std::shared_ptr<BTNode> node);
    BTResult ExecuteDecoratorCooldown(std::shared_ptr<BTNode> node);
    BTResult ExecuteSucceeder(std::shared_ptr<BTNode> node);
    BTResult ExecuteFailer(std::shared_ptr<BTNode> node);
    BTResult ExecuteCondition(std::shared_ptr<BTNode> node);
    BTResult ExecuteAction(std::shared_ptr<BTNode> node);

    void AddLog(const std::wstring& nodeId, const std::wstring& nodeName, BTResult result, int depth, BTNodeType type);

    std::mt19937 m_rng;
    ExecutionContext m_ctx;
    std::vector<LogEntry> m_logs;
    std::map<std::wstring, size_t> m_memPositions;
    int m_stepCount = 0;
    int m_execOrder = 0;
};

// ============================================================================
// 搜索节点
// ============================================================================
inline std::shared_ptr<BTNode> FindNodeById(std::shared_ptr<BTNode> node, const std::wstring& id)
{
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto& child : node->children)
    {
        auto found = FindNodeById(child, id);
        if (found) return found;
    }
    return nullptr;
}

inline void CollectAllNodes(std::shared_ptr<BTNode> node, std::vector<std::shared_ptr<BTNode>>& out)
{
    if (!node) return;
    out.push_back(node);
    for (auto& child : node->children)
        CollectAllNodes(child, out);
}

inline int CountNodes(std::shared_ptr<BTNode> node)
{
    if (!node) return 0;
    int count = 1;
    for (auto& child : node->children)
        count += CountNodes(child);
    return count;
}