#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

// 行为树节点类型
enum class BTNodeType
{
    SEQUENCE, SELECTOR, PARALLEL, RANDOM, PROBABILITY,
    MEM_SEQUENCE, MEM_SELECTOR,
    INVERTER, DECORATOR_REPEAT, DECORATOR_TIMEOUT, DECORATOR_COOLDOWN,
    SUCCEEDER, FAILER,
    CONDITION, ACTION
};

// 节点分类
enum class BTNodeCategory
{
    COMPOSITE, DECORATOR, CONDITION, ACTION
};

// 执行结果
enum class BTResult
{
    SUCCESS, FAILURE, RUNNING, IDLE
};

// 条件子类型
enum class ConditionType
{
    NONE, LOW_HP, LOW_MP, HAS_TARGET, IN_SAFE_AREA, BAG_FULL,
    HAS_ITEM, SKILL_READY, IS_DEAD, TARGET_DISTANCE, TARGET_TYPE,
    HAS_NEARBY_PLAYER, MONSTER_COUNT, HAS_POTION, HAS_DROPPED_ITEM,
    HP_RANGE, TIME_OF_DAY
};

// 动作子类型
enum class ActionType
{
    NONE, USE_POTION, USE_ITEM, CHANGE_ATTACK_MODE, ATTACK,
    MOVE_TO_TARGET, PATROL, PICKUP_ITEM, FLEE, REST, CHAT,
    USE_SKILL, SAY, RECALL, DELAY, ATTACK_DIR, SPELL_CAST,
    DROP_ITEM, EQUIP_ITEM, SUMMON_PET, FOLLOW, GROUP, MINE
};

// 行为树节点
struct BTNode
{
    std::wstring id;
    BTNodeType type = BTNodeType::ACTION;
    ConditionType conditionType = ConditionType::NONE;
    ActionType actionType = ActionType::NONE;
    std::wstring name;
    std::map<std::wstring, std::wstring> params;
    std::vector<std::shared_ptr<BTNode>> children;
    BTNode* parent = nullptr;
    int depth = 0;
    bool collapsed = false;

    // 执行状态
    BTResult lastResult = BTResult::IDLE;
    bool isActive = false;
    int execOrder = -1;
};

// 执行日志条目
struct LogEntry
{
    std::wstring nodeId;
    std::wstring nodeName;
    BTResult result;
    int depth;
    BTNodeType type;
};

// 执行上下文（模拟游戏状态）
struct ExecutionContext
{
    int hpPercent = 80;
    int mpPercent = 70;
    bool hasTarget = true;
    bool inSafeArea = false;
    bool bagFull = false;
    bool hasItem = true;
    bool skillReady = true;
    bool isDead = false;
    int targetDistance = 3;
    int monsterCount = 2;
};

// 节点类型颜色 (GBR格式)
inline COLORREF GetNodeColor(BTNodeType type)
{
    switch (type)
    {
    case BTNodeType::SEQUENCE:
    case BTNodeType::SELECTOR:
    case BTNodeType::PARALLEL:
    case BTNodeType::RANDOM:
    case BTNodeType::PROBABILITY:
    case BTNodeType::MEM_SEQUENCE:
    case BTNodeType::MEM_SELECTOR:
        return RGB(0x00, 0xAA, 0xFF);   // 复合节点 - 金色
    case BTNodeType::INVERTER:
    case BTNodeType::DECORATOR_REPEAT:
    case BTNodeType::DECORATOR_TIMEOUT:
    case BTNodeType::DECORATOR_COOLDOWN:
    case BTNodeType::SUCCEEDER:
    case BTNodeType::FAILER:
        return RGB(0xFF, 0x66, 0xCC);   // 装饰节点 - 紫色
    case BTNodeType::CONDITION:
        return RGB(0xFF, 0xCC, 0x00);   // 条件节点 - 青色
    case BTNodeType::ACTION:
        return RGB(0x44, 0x88, 0xFF);   // 动作节点 - 橙色
    }
    return RGB(0x88, 0x88, 0x88);
}

inline COLORREF GetResultColor(BTResult result)
{
    switch (result)
    {
    case BTResult::SUCCESS: return RGB(0x88, 0xFF, 0x00);
    case BTResult::FAILURE: return RGB(0x66, 0x44, 0xFF);
    case BTResult::RUNNING: return RGB(0xFF, 0x88, 0x44);
    case BTResult::IDLE:    return RGB(0x66, 0x55, 0x55);
    }
    return RGB(0x88, 0x88, 0x88);
}

inline BTNodeCategory GetNodeCategory(BTNodeType type)
{
    switch (type)
    {
    case BTNodeType::SEQUENCE:
    case BTNodeType::SELECTOR:
    case BTNodeType::PARALLEL:
    case BTNodeType::RANDOM:
    case BTNodeType::PROBABILITY:
    case BTNodeType::MEM_SEQUENCE:
    case BTNodeType::MEM_SELECTOR:
        return BTNodeCategory::COMPOSITE;
    case BTNodeType::INVERTER:
    case BTNodeType::DECORATOR_REPEAT:
    case BTNodeType::DECORATOR_TIMEOUT:
    case BTNodeType::DECORATOR_COOLDOWN:
    case BTNodeType::SUCCEEDER:
    case BTNodeType::FAILER:
        return BTNodeCategory::DECORATOR;
    case BTNodeType::CONDITION:
        return BTNodeCategory::CONDITION;
    case BTNodeType::ACTION:
        return BTNodeCategory::ACTION;
    }
    return BTNodeCategory::ACTION;
}

inline const wchar_t* GetNodeTypeName(BTNodeType type)
{
    switch (type)
    {
    case BTNodeType::SEQUENCE: return L"序列(Sequence)";
    case BTNodeType::SELECTOR: return L"选择(Selector)";
    case BTNodeType::PARALLEL: return L"并行(Parallel)";
    case BTNodeType::RANDOM: return L"随机(Random)";
    case BTNodeType::PROBABILITY: return L"概率(Probability)";
    case BTNodeType::MEM_SEQUENCE: return L"记忆序列(MemSeq)";
    case BTNodeType::MEM_SELECTOR: return L"记忆选择(MemSel)";
    case BTNodeType::INVERTER: return L"反转(Inverter)";
    case BTNodeType::DECORATOR_REPEAT: return L"重复(Repeat)";
    case BTNodeType::DECORATOR_TIMEOUT: return L"超时(Timeout)";
    case BTNodeType::DECORATOR_COOLDOWN: return L"冷却(Cooldown)";
    case BTNodeType::SUCCEEDER: return L"强制成功";
    case BTNodeType::FAILER: return L"强制失败";
    case BTNodeType::CONDITION: return L"条件(Condition)";
    case BTNodeType::ACTION: return L"动作(Action)";
    }
    return L"未知";
}

inline const wchar_t* GetResultName(BTResult r)
{
    switch (r)
    {
    case BTResult::SUCCESS: return L"SUCCESS";
    case BTResult::FAILURE: return L"FAILURE";
    case BTResult::RUNNING: return L"RUNNING";
    case BTResult::IDLE: return L"IDLE";
    }
    return L"";
}

// 从标签名解析节点类型
inline BTNodeType ParseNodeType(const std::wstring& tagName)
{
    if (tagName == L"Sequence") return BTNodeType::SEQUENCE;
    if (tagName == L"Selector") return BTNodeType::SELECTOR;
    if (tagName == L"Parallel") return BTNodeType::PARALLEL;
    if (tagName == L"Random") return BTNodeType::RANDOM;
    if (tagName == L"Probability") return BTNodeType::PROBABILITY;
    if (tagName == L"MemSequence") return BTNodeType::MEM_SEQUENCE;
    if (tagName == L"MemSelector") return BTNodeType::MEM_SELECTOR;
    if (tagName == L"Inverter") return BTNodeType::INVERTER;
    if (tagName == L"DecoratorRepeat") return BTNodeType::DECORATOR_REPEAT;
    if (tagName == L"DecoratorTimeout") return BTNodeType::DECORATOR_TIMEOUT;
    if (tagName == L"DecoratorCooldown") return BTNodeType::DECORATOR_COOLDOWN;
    if (tagName == L"Succeeder") return BTNodeType::SUCCEEDER;
    if (tagName == L"Failer") return BTNodeType::FAILER;
    if (tagName.find(L"Condition") == 0) return BTNodeType::CONDITION;
    if (tagName.find(L"Action") == 0) return BTNodeType::ACTION;
    return BTNodeType::ACTION;
}

inline ConditionType ParseConditionType(const std::wstring& tagName)
{
    if (tagName == L"ConditionLowHP") return ConditionType::LOW_HP;
    if (tagName == L"ConditionLowMP") return ConditionType::LOW_MP;
    if (tagName == L"ConditionHasTarget") return ConditionType::HAS_TARGET;
    if (tagName == L"ConditionInSafeArea") return ConditionType::IN_SAFE_AREA;
    if (tagName == L"ConditionBagFull") return ConditionType::BAG_FULL;
    if (tagName == L"ConditionHasItem") return ConditionType::HAS_ITEM;
    if (tagName == L"ConditionSkillReady") return ConditionType::SKILL_READY;
    if (tagName == L"ConditionIsDead") return ConditionType::IS_DEAD;
    if (tagName == L"ConditionTargetDistance") return ConditionType::TARGET_DISTANCE;
    if (tagName == L"ConditionTargetType") return ConditionType::TARGET_TYPE;
    if (tagName == L"ConditionHasNearbyPlayer") return ConditionType::HAS_NEARBY_PLAYER;
    if (tagName == L"ConditionMonsterCount") return ConditionType::MONSTER_COUNT;
    if (tagName == L"ConditionHasPotion") return ConditionType::HAS_POTION;
    if (tagName == L"ConditionHasDroppedItem") return ConditionType::HAS_DROPPED_ITEM;
    if (tagName == L"ConditionHPRange") return ConditionType::HP_RANGE;
    if (tagName == L"ConditionTimeOfDay") return ConditionType::TIME_OF_DAY;
    return ConditionType::NONE;
}

inline ActionType ParseActionType(const std::wstring& tagName)
{
    if (tagName == L"ActionUsePotion") return ActionType::USE_POTION;
    if (tagName == L"ActionUseItem") return ActionType::USE_ITEM;
    if (tagName == L"ActionChangeAttackMode") return ActionType::CHANGE_ATTACK_MODE;
    if (tagName == L"ActionAttack") return ActionType::ATTACK;
    if (tagName == L"ActionMoveToTarget") return ActionType::MOVE_TO_TARGET;
    if (tagName == L"ActionPatrol") return ActionType::PATROL;
    if (tagName == L"ActionPickupItem") return ActionType::PICKUP_ITEM;
    if (tagName == L"ActionFlee") return ActionType::FLEE;
    if (tagName == L"ActionRest") return ActionType::REST;
    if (tagName == L"ActionChat") return ActionType::CHAT;
    if (tagName == L"ActionUseSkill") return ActionType::USE_SKILL;
    if (tagName == L"ActionSay") return ActionType::SAY;
    if (tagName == L"ActionRecall") return ActionType::RECALL;
    if (tagName == L"ActionDelay") return ActionType::DELAY;
    if (tagName == L"ActionAttackDir") return ActionType::ATTACK_DIR;
    if (tagName == L"ActionSpellCast") return ActionType::SPELL_CAST;
    if (tagName == L"ActionDropItem") return ActionType::DROP_ITEM;
    if (tagName == L"ActionEquipItem") return ActionType::EQUIP_ITEM;
    if (tagName == L"ActionSummonPet") return ActionType::SUMMON_PET;
    if (tagName == L"ActionFollow") return ActionType::FOLLOW;
    if (tagName == L"ActionGroup") return ActionType::GROUP;
    if (tagName == L"ActionMine") return ActionType::MINE;
    return ActionType::NONE;
}