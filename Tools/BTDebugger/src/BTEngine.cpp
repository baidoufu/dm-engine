#include "BTEngine.h"
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <sstream>

// 安全字符串转整数，失败返回默认值
static int SafeStoi(const std::wstring& s, int defVal = 0)
{
    try { return std::stoi(s); }
    catch (...) { return defVal; }
}

// ============================================================================
// XML 解析器实现
// ============================================================================

std::shared_ptr<BTNode> XMLParser::ParseFile(const std::wstring& filePath)
{
    std::wstring xml = ReadFileGBK(filePath);
    if (xml.empty()) return nullptr;
    return ParseWString(xml);
}

std::shared_ptr<BTNode> XMLParser::ParseString(const std::string& xmlContent)
{
    // 自适应编码：先尝试 UTF-8，失败则回退到系统默认代码页（简体中文 Windows 为 GBK）
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, xmlContent.c_str(), (int)xmlContent.size(), nullptr, 0);
    UINT codePage = CP_UTF8;
    if (wlen <= 0)
    {
        codePage = CP_ACP;
        wlen = MultiByteToWideChar(CP_ACP, 0, xmlContent.c_str(), (int)xmlContent.size(), nullptr, 0);
    }
    if (wlen <= 0) return nullptr;
    std::wstring xml(wlen, L'\0');
    MultiByteToWideChar(codePage, 0, xmlContent.c_str(), (int)xmlContent.size(), &xml[0], wlen);
    return ParseWString(xml);
}

std::shared_ptr<BTNode> XMLParser::ParseWString(const std::wstring& xml)
{
    // 跳过 XML 声明
    size_t pos = xml.find(L"<BehaviorTree");
    if (pos == std::wstring::npos) return nullptr;
    pos = xml.find(L'>', pos);
    if (pos == std::wstring::npos) return nullptr;
    pos++;

    // 跳过注释和空白，找到第一个真正的根节点
    while (pos < xml.length())
    {
        SkipWhitespace(xml, pos);
        if (pos >= xml.length()) break;
        if (xml.substr(pos, 4) == L"<!--")
        {
            pos = xml.find(L"-->", pos);
            if (pos == std::wstring::npos) break;
            pos += 3;
            continue;
        }
        if (xml[pos] == L'<')
        {
            XMLElement rootElem;
            pos = ParseElement(xml, pos, rootElem);
            if (!rootElem.tagName.empty())
                return BuildTree(rootElem, nullptr, 0);
        }
        break;
    }
    return nullptr;
}

std::wstring XMLParser::Trim(const std::wstring& s)
{
    size_t start = 0, end = s.length();
    while (start < end && (s[start] == L' ' || s[start] == L'\t' || s[start] == L'\r' || s[start] == L'\n'))
        start++;
    while (end > start && (s[end - 1] == L' ' || s[end - 1] == L'\t' || s[end - 1] == L'\r' || s[end - 1] == L'\n'))
        end--;
    return s.substr(start, end - start);
}

void XMLParser::SkipWhitespace(const std::wstring& xml, size_t& pos)
{
    while (pos < xml.length() && (xml[pos] == L' ' || xml[pos] == L'\t' || xml[pos] == L'\r' || xml[pos] == L'\n'))
        pos++;
}

std::wstring XMLParser::ReadName(const std::wstring& xml, size_t& pos)
{
    size_t start = pos;
    while (pos < xml.length() && (iswalnum(xml[pos]) || xml[pos] == L'_' || xml[pos] == L'-' || xml[pos] == L':'))
        pos++;
    return xml.substr(start, pos - start);
}

std::wstring XMLParser::ReadUntil(const std::wstring& xml, size_t& pos, wchar_t delim)
{
    size_t start = pos;
    while (pos < xml.length() && xml[pos] != delim)
        pos++;
    std::wstring result = xml.substr(start, pos - start);
    if (pos < xml.length()) pos++; // 跳过分隔符
    return result;
}

std::wstring XMLParser::DecodeEntities(const std::wstring& text)
{
    std::wstring result = text;
    size_t pos;
    while ((pos = result.find(L"&lt;")) != std::wstring::npos) result.replace(pos, 4, L"<");
    while ((pos = result.find(L"&gt;")) != std::wstring::npos) result.replace(pos, 4, L">");
    while ((pos = result.find(L"&amp;")) != std::wstring::npos) result.replace(pos, 5, L"&");
    while ((pos = result.find(L"&quot;")) != std::wstring::npos) result.replace(pos, 6, L"\"");
    while ((pos = result.find(L"&apos;")) != std::wstring::npos) result.replace(pos, 6, L"'");
    return result;
}

size_t XMLParser::ParseElement(const std::wstring& xml, size_t pos, XMLElement& outElem)
{
    if (pos >= xml.length() || xml[pos] != L'<')
        return pos;

    pos++; // 跳过 '<'

    // 检查是否是结束标签
    if (pos < xml.length() && xml[pos] == L'/')
    {
        return pos; // 返回结束标签位置
    }

    // 跳过注释 <!-- ... --> 和声明 <!DOCTYPE ...> / <![CDATA[ ... ]]>
    if (pos < xml.length() && xml[pos] == L'!')
    {
        if (pos + 3 <= xml.length() && xml[pos + 1] == L'-' && xml[pos + 2] == L'-')
        {
            // XML 注释 <!-- ... -->
            pos = xml.find(L"-->", pos + 3);
            if (pos != std::wstring::npos)
                return pos + 3; // 跳过 -->
        }
        else
        {
            // 其他声明 <!DOCTYPE ...> 或 <![CDATA[ ... ]]>
            pos = xml.find(L'>', pos);
            if (pos != std::wstring::npos)
                return pos + 1;
        }
        return pos; // 未找到闭合，返回当前位置
    }

    // 读取标签名
    outElem.tagName = ReadName(xml, pos);

    // 读取属性
    while (pos < xml.length())
    {
        SkipWhitespace(xml, pos);
        if (pos >= xml.length()) break;

        if (xml[pos] == L'>')
        {
            pos++; // 跳过 '>'
            // 读取文本内容
            size_t textStart = pos;
            while (pos < xml.length() && xml[pos] != L'<')
                pos++;
            outElem.text = Trim(DecodeEntities(xml.substr(textStart, pos - textStart)));

            // 解析子元素
            while (pos < xml.length())
            {
                if (pos + 1 < xml.length() && xml[pos] == L'<' && xml[pos + 1] == L'/')
                {
                    // 结束标签
                    pos += 2;
                    SkipWhitespace(xml, pos);
                    std::wstring endTag = ReadName(xml, pos);
                    SkipWhitespace(xml, pos);
                    if (pos < xml.length() && xml[pos] == L'>') pos++;
                    break;
                }
                else if (xml[pos] == L'<')
                {
                    XMLElement child;
                    size_t newPos = ParseElement(xml, pos, child);
                    if (newPos == pos) break; // 防止死循环
                    if (!child.tagName.empty())
                        outElem.children.push_back(std::move(child));
                    pos = newPos;
                }
                else
                {
                    pos++;
                }
            }
            return pos;
        }
        else if (xml[pos] == L'/' && pos + 1 < xml.length() && xml[pos + 1] == L'>')
        {
            pos += 2; // 跳过 '/>'
            return pos;
        }
        else
        {
            // 属性名
            std::wstring attrName = ReadName(xml, pos);
            if (attrName.empty())
            {
                // 死循环防护：遇到非法字符时跳过它继续
                pos++;
                continue;
            }
            SkipWhitespace(xml, pos);
            if (pos < xml.length() && xml[pos] == L'=')
            {
                pos++; // 跳过 '='
                SkipWhitespace(xml, pos);
                if (pos < xml.length() && (xml[pos] == L'"' || xml[pos] == L'\''))
                {
                    wchar_t quote = xml[pos];
                    pos++;
                    std::wstring attrValue = ReadUntil(xml, pos, quote);
                    outElem.attributes[attrName] = DecodeEntities(attrValue);
                }
            }
        }
    }
    return pos;
}

std::shared_ptr<BTNode> XMLParser::BuildTree(const XMLElement& elem, BTNode* parent, int depth)
{
    auto node = std::make_shared<BTNode>();
    node->id = std::to_wstring(depth) + L"_" + elem.tagName + L"_" + std::to_wstring((size_t)node.get());
    node->type = ParseNodeType(elem.tagName);
    node->conditionType = ParseConditionType(elem.tagName);
    node->actionType = ParseActionType(elem.tagName);
    node->parent = parent;
    node->depth = depth;

    // 读取 name 属性
    auto nameIt = elem.attributes.find(L"name");
    if (nameIt != elem.attributes.end())
        node->name = nameIt->second;
    else
        node->name = elem.tagName;

    // 读取其他属性作为参数（排除 name）
    for (auto& attr : elem.attributes)
    {
        if (attr.first != L"name")
            node->params[attr.first] = attr.second;
    }

    // 递归构建子节点
    for (auto& child : elem.children)
    {
        auto childNode = BuildTree(child, node.get(), depth + 1);
        if (childNode)
            node->children.push_back(childNode);
    }

    return node;
}

std::wstring XMLParser::ReadFileGBK(const std::wstring& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return L"";

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    file.read(&buffer[0], size);
    file.close();

    // 编码自动检测：BOM 头
    UINT codePage = 936; // 默认 GBK
    const char* rawData = buffer.c_str();
    size_t offset = 0;

    if (size >= 3 && (unsigned char)rawData[0] == 0xEF && (unsigned char)rawData[1] == 0xBB && (unsigned char)rawData[2] == 0xBF)
    {
        codePage = CP_UTF8;   // UTF-8 BOM
        offset = 3;
    }
    else if (size >= 2 && (unsigned char)rawData[0] == 0xFF && (unsigned char)rawData[1] == 0xFE)
    {
        codePage = 1200;      // UTF-16 LE
        offset = 2;
    }
    else if (size >= 2 && (unsigned char)rawData[0] == 0xFE && (unsigned char)rawData[1] == 0xFF)
    {
        codePage = 1201;      // UTF-16 BE
        offset = 2;
    }
    else if (size > 0)
    {
        // 无 BOM：试探 UTF-8 有效性
        int testLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, rawData, (int)size, nullptr, 0);
        if (testLen > 0)
            codePage = CP_UTF8;
    }

    int wlen = MultiByteToWideChar(codePage, 0, rawData + offset, (int)(size - offset), nullptr, 0);
    std::wstring result(wlen, L'\0');
    MultiByteToWideChar(codePage, 0, rawData + offset, (int)(size - offset), &result[0], wlen);
    return result;
}

// ============================================================================
// 行为树引擎实现
// ============================================================================

BTEngine::BTEngine()
    : m_rng(std::random_device{}())
{
}

void BTEngine::Reset()
{
    m_logs.clear();
    m_memPositions.clear();
    m_stepCount = 0;
    m_execOrder = 0;
    m_ctx = ExecutionContext();
}

void BTEngine::ResetNodeStates(std::shared_ptr<BTNode> node)
{
    if (!node) return;
    node->lastResult = BTResult::IDLE;
    node->isActive = false;
    node->execOrder = -1;
    for (auto& child : node->children)
        ResetNodeStates(child);
}

BTResult BTEngine::ExecuteNode(std::shared_ptr<BTNode> node)
{
    if (!node) return BTResult::FAILURE;

    node->isActive = true;
    node->execOrder = m_execOrder++;

    BTResult result;
    switch (node->type)
    {
    case BTNodeType::SEQUENCE:       result = ExecuteSequence(node); break;
    case BTNodeType::SELECTOR:       result = ExecuteSelector(node); break;
    case BTNodeType::PARALLEL:       result = ExecuteParallel(node); break;
    case BTNodeType::RANDOM:         result = ExecuteRandom(node); break;
    case BTNodeType::PROBABILITY:    result = ExecuteProbability(node); break;
    case BTNodeType::MEM_SEQUENCE:   result = ExecuteMemSequence(node); break;
    case BTNodeType::MEM_SELECTOR:   result = ExecuteMemSelector(node); break;
    case BTNodeType::INVERTER:       result = ExecuteInverter(node); break;
    case BTNodeType::DECORATOR_REPEAT: result = ExecuteDecoratorRepeat(node); break;
    case BTNodeType::DECORATOR_TIMEOUT: result = ExecuteDecoratorTimeout(node); break;
    case BTNodeType::DECORATOR_COOLDOWN: result = ExecuteDecoratorCooldown(node); break;
    case BTNodeType::SUCCEEDER:      result = ExecuteSucceeder(node); break;
    case BTNodeType::FAILER:         result = ExecuteFailer(node); break;
    case BTNodeType::CONDITION:      result = ExecuteCondition(node); break;
    case BTNodeType::ACTION:         result = ExecuteAction(node); break;
    default: result = BTResult::FAILURE;
    }

    node->lastResult = result;
    node->isActive = false;
    AddLog(node->id, node->name, result, node->depth, node->type);
    return result;
}

void BTEngine::ExecuteFull(std::shared_ptr<BTNode> root)
{
    Reset();
    ResetNodeStates(root);
    ExecuteNode(root);
}

void BTEngine::AddLog(const std::wstring& nodeId, const std::wstring& nodeName, BTResult result, int depth, BTNodeType type)
{
    LogEntry entry;
    entry.nodeId = nodeId;
    entry.nodeName = nodeName;
    entry.result = result;
    entry.depth = depth;
    entry.type = type;
    m_logs.push_back(entry);
    m_stepCount++;
}

BTResult BTEngine::ExecuteSequence(std::shared_ptr<BTNode> node)
{
    for (auto& child : node->children)
    {
        BTResult r = ExecuteNode(child);
        if (r != BTResult::SUCCESS) return r;
    }
    return BTResult::SUCCESS;
}

BTResult BTEngine::ExecuteSelector(std::shared_ptr<BTNode> node)
{
    for (auto& child : node->children)
    {
        BTResult r = ExecuteNode(child);
        if (r == BTResult::SUCCESS) return BTResult::SUCCESS;
    }
    return BTResult::FAILURE;
}

BTResult BTEngine::ExecuteParallel(std::shared_ptr<BTNode> node)
{
    int success = 0, failure = 0;
    for (auto& child : node->children)
    {
        BTResult r = ExecuteNode(child);
        if (r == BTResult::SUCCESS) success++;
        else if (r == BTResult::FAILURE) failure++;
    }
    if (success == (int)node->children.size()) return BTResult::SUCCESS;
    if (failure > 0) return BTResult::FAILURE;
    return BTResult::RUNNING;
}

BTResult BTEngine::ExecuteRandom(std::shared_ptr<BTNode> node)
{
    if (node->children.empty()) return BTResult::FAILURE;
    int idx = m_rng() % node->children.size();
    return ExecuteNode(node->children[idx]);
}

BTResult BTEngine::ExecuteProbability(std::shared_ptr<BTNode> node)
{
    auto it = node->params.find(L"chance");
    int chance = it != node->params.end() ? SafeStoi(it->second, 50) : 50;
    if ((int)(m_rng() % 100) < chance)
    {
        BTResult r = BTResult::SUCCESS;
        for (auto& child : node->children)
            r = ExecuteNode(child);
        return r;
    }
    return BTResult::FAILURE;
}

BTResult BTEngine::ExecuteMemSequence(std::shared_ptr<BTNode> node)
{
    size_t startIdx = 0;
    auto it = m_memPositions.find(node->id);
    if (it != m_memPositions.end()) startIdx = it->second;

    for (size_t i = startIdx; i < node->children.size(); i++)
    {
        BTResult r = ExecuteNode(node->children[i]);
        if (r != BTResult::SUCCESS)
        {
            m_memPositions[node->id] = i;
            return r;
        }
    }
    m_memPositions.erase(node->id);
    return BTResult::SUCCESS;
}

BTResult BTEngine::ExecuteMemSelector(std::shared_ptr<BTNode> node)
{
    size_t startIdx = 0;
    auto it = m_memPositions.find(node->id);
    if (it != m_memPositions.end()) startIdx = it->second;

    for (size_t i = startIdx; i < node->children.size(); i++)
    {
        BTResult r = ExecuteNode(node->children[i]);
        if (r == BTResult::SUCCESS)
        {
            m_memPositions.erase(node->id);
            return BTResult::SUCCESS;
        }
    }
    m_memPositions.erase(node->id);
    return BTResult::FAILURE;
}

BTResult BTEngine::ExecuteInverter(std::shared_ptr<BTNode> node)
{
    if (node->children.empty()) return BTResult::FAILURE;
    BTResult r = ExecuteNode(node->children[0]);
    if (r == BTResult::SUCCESS) return BTResult::FAILURE;
    if (r == BTResult::FAILURE) return BTResult::SUCCESS;
    return r;
}

BTResult BTEngine::ExecuteDecoratorRepeat(std::shared_ptr<BTNode> node)
{
    if (node->children.empty()) return BTResult::FAILURE;
    auto it = node->params.find(L"count");
    int count = it != node->params.end() ? SafeStoi(it->second, 1) : 1;
    int maxIter = count == 0 ? 3 : count;
    BTResult last = BTResult::SUCCESS;
    for (int i = 0; i < maxIter; i++)
    {
        last = ExecuteNode(node->children[0]);
        if (last != BTResult::SUCCESS) return last;
    }
    return last;
}

BTResult BTEngine::ExecuteDecoratorTimeout(std::shared_ptr<BTNode> node)
{
    if (node->children.empty()) return BTResult::FAILURE;
    return ExecuteNode(node->children[0]);
}

BTResult BTEngine::ExecuteDecoratorCooldown(std::shared_ptr<BTNode> node)
{
    if (node->children.empty()) return BTResult::FAILURE;
    return ExecuteNode(node->children[0]);
}

BTResult BTEngine::ExecuteSucceeder(std::shared_ptr<BTNode> node)
{
    if (!node->children.empty())
        ExecuteNode(node->children[0]);
    return BTResult::SUCCESS;
}

BTResult BTEngine::ExecuteFailer(std::shared_ptr<BTNode> node)
{
    if (!node->children.empty())
        ExecuteNode(node->children[0]);
    return BTResult::FAILURE;
}

BTResult BTEngine::ExecuteCondition(std::shared_ptr<BTNode> node)
{
    switch (node->conditionType)
    {
    case ConditionType::LOW_HP:
    {
        auto it = node->params.find(L"percent");
        int pct = it != node->params.end() ? SafeStoi(it->second, 50) : 50;
        return m_ctx.hpPercent < pct ? BTResult::SUCCESS : BTResult::FAILURE;
    }
    case ConditionType::LOW_MP:
    {
        auto it = node->params.find(L"percent");
        int pct = it != node->params.end() ? SafeStoi(it->second, 50) : 50;
        return m_ctx.mpPercent < pct ? BTResult::SUCCESS : BTResult::FAILURE;
    }
    case ConditionType::HAS_TARGET:
        return m_ctx.hasTarget ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::IN_SAFE_AREA:
        return m_ctx.inSafeArea ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::BAG_FULL:
        return m_ctx.bagFull ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::HAS_ITEM:
        return m_ctx.hasItem ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::SKILL_READY:
        return m_ctx.skillReady ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::IS_DEAD:
        return m_ctx.isDead ? BTResult::SUCCESS : BTResult::FAILURE;
    case ConditionType::TARGET_DISTANCE:
    {
        auto itMin = node->params.find(L"min");
        auto itMax = node->params.find(L"max");
        int min = itMin != node->params.end() ? SafeStoi(itMin->second, 0) : 0;
        int max = itMax != node->params.end() ? SafeStoi(itMax->second, 10) : 10;
        return (m_ctx.targetDistance >= min && m_ctx.targetDistance <= max) ? BTResult::SUCCESS : BTResult::FAILURE;
    }
    case ConditionType::MONSTER_COUNT:
    {
        auto it = node->params.find(L"count");
        int cnt = it != node->params.end() ? SafeStoi(it->second, 0) : 0;
        return m_ctx.monsterCount >= cnt ? BTResult::SUCCESS : BTResult::FAILURE;
    }
    default:
        return (m_rng() % 2 == 0) ? BTResult::SUCCESS : BTResult::FAILURE;
    }
}

BTResult BTEngine::ExecuteAction(std::shared_ptr<BTNode> node)
{
    return (m_rng() % 100 < 85) ? BTResult::SUCCESS : BTResult::FAILURE;
}

// ============================================================================
// XML 序列化 —— 将行为树写回 XML 字符串
// ============================================================================
static void SerializeNode(std::wostringstream& out, std::shared_ptr<BTNode> node, int indent)
{
    std::wstring pad(indent * 4, L' ');
    std::wstring tag = GetTagName(node->type, node->conditionType, node->actionType);

    out << pad << L"<" << tag;
    if (!node->name.empty() && node->name != tag)
        out << L" name=\"" << node->name << L"\"";
    for (auto& p : node->params)
    {
        if (p.first == L"name") continue;
        out << L" " << p.first << L"=\"" << p.second << L"\"";
    }

    if (node->children.empty())
    {
        out << L" />\r\n";
    }
    else
    {
        out << L">\r\n";
        for (auto& child : node->children)
            SerializeNode(out, child, indent + 1);
        out << pad << L"</" << tag << L">\r\n";
    }
}

std::string XMLParser::SerializeTree(std::shared_ptr<BTNode> root, const std::wstring& treeName)
{
    if (!root) return "";

    std::wostringstream wss;
    wss << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
    wss << L"<BehaviorTree name=\"" << treeName << L"\">\r\n";
    SerializeNode(wss, root, 1);
    wss << L"</BehaviorTree>\r\n";

    std::wstring wx = wss.str();
    // 输出 UTF-8（带 BOM 兼容大多数编辑器）
    std::string result;
    result.push_back('\xEF'); result.push_back('\xBB'); result.push_back('\xBF'); // BOM
    int len = WideCharToMultiByte(CP_UTF8, 0, wx.c_str(), (int)wx.size(), nullptr, 0, nullptr, nullptr);
    if (len > 0)
    {
        result.resize(3 + len);
        WideCharToMultiByte(CP_UTF8, 0, wx.c_str(), (int)wx.size(), &result[3], len, nullptr, nullptr);
    }
    return result;
}