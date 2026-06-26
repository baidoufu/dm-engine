// 行为树节点类型枚举
export enum BTNodeType {
  SEQUENCE = 'Sequence',
  SELECTOR = 'Selector',
  PARALLEL = 'Parallel',
  RANDOM = 'Random',
  PROBABILITY = 'Probability',
  MEM_SEQUENCE = 'MemSequence',
  MEM_SELECTOR = 'MemSelector',
  INVERTER = 'Inverter',
  DECORATOR_REPEAT = 'DecoratorRepeat',
  DECORATOR_TIMEOUT = 'DecoratorTimeout',
  DECORATOR_COOLDOWN = 'DecoratorCooldown',
  SUCCEEDER = 'Succeeder',
  FAILER = 'Failer',
  CONDITION = 'Condition',
  ACTION = 'Action',
}

// 行为树节点类型分类
export enum BTNodeCategory {
  COMPOSITE = 'composite',
  DECORATOR = 'decorator',
  CONDITION = 'condition',
  ACTION = 'action',
}

// 条件节点子类型
export enum ConditionType {
  LOW_HP = 'ConditionLowHP',
  LOW_MP = 'ConditionLowMP',
  HAS_TARGET = 'ConditionHasTarget',
  IN_SAFE_AREA = 'ConditionInSafeArea',
  BAG_FULL = 'ConditionBagFull',
  HAS_ITEM = 'ConditionHasItem',
  SKILL_READY = 'ConditionSkillReady',
  IS_DEAD = 'ConditionIsDead',
  TARGET_DISTANCE = 'ConditionTargetDistance',
  TARGET_TYPE = 'ConditionTargetType',
  HAS_NEARBY_PLAYER = 'ConditionHasNearbyPlayer',
  MONSTER_COUNT = 'ConditionMonsterCount',
  HAS_POTION = 'ConditionHasPotion',
  HAS_DROPPED_ITEM = 'ConditionHasDroppedItem',
  HP_RANGE = 'ConditionHPRange',
  TIME_OF_DAY = 'ConditionTimeOfDay',
}

// 动作节点子类型
export enum ActionType {
  USE_POTION = 'ActionUsePotion',
  USE_ITEM = 'ActionUseItem',
  CHANGE_ATTACK_MODE = 'ActionChangeAttackMode',
  ATTACK = 'ActionAttack',
  MOVE_TO_TARGET = 'ActionMoveToTarget',
  PATROL = 'ActionPatrol',
  PICKUP_ITEM = 'ActionPickupItem',
  FLEE = 'ActionFlee',
  REST = 'ActionRest',
  CHAT = 'ActionChat',
  USE_SKILL = 'ActionUseSkill',
  SAY = 'ActionSay',
  RECALL = 'ActionRecall',
  DELAY = 'ActionDelay',
  ATTACK_DIR = 'ActionAttackDir',
  SPELL_CAST = 'ActionSpellCast',
  DROP_ITEM = 'ActionDropItem',
  EQUIP_ITEM = 'ActionEquipItem',
  SUMMON_PET = 'ActionSummonPet',
  FOLLOW = 'ActionFollow',
  GROUP = 'ActionGroup',
  MINE = 'ActionMine',
}

// 节点执行结果
export enum BTResult {
  SUCCESS = 'SUCCESS',
  FAILURE = 'FAILURE',
  RUNNING = 'RUNNING',
  IDLE = 'IDLE',
}

// 行为树节点数据模型
export interface BTNode {
  id: string;
  type: BTNodeType;
  conditionType?: ConditionType;
  actionType?: ActionType;
  name: string;
  params: Record<string, string>;
  children: BTNode[];
  parentId: string | null;
  depth: number;
  collapsed: boolean;
}

// 判断节点类型分类
export function getNodeCategory(type: BTNodeType): BTNodeCategory {
  switch (type) {
    case BTNodeType.SEQUENCE:
    case BTNodeType.SELECTOR:
    case BTNodeType.PARALLEL:
    case BTNodeType.RANDOM:
    case BTNodeType.PROBABILITY:
    case BTNodeType.MEM_SEQUENCE:
    case BTNodeType.MEM_SELECTOR:
      return BTNodeCategory.COMPOSITE;
    case BTNodeType.INVERTER:
    case BTNodeType.DECORATOR_REPEAT:
    case BTNodeType.DECORATOR_TIMEOUT:
    case BTNodeType.DECORATOR_COOLDOWN:
    case BTNodeType.SUCCEEDER:
    case BTNodeType.FAILER:
      return BTNodeCategory.DECORATOR;
    case BTNodeType.CONDITION:
      return BTNodeCategory.CONDITION;
    case BTNodeType.ACTION:
      return BTNodeCategory.ACTION;
  }
}

// 执行上下文（模拟游戏状态）
export interface ExecutionContext {
  hpPercent: number;
  mpPercent: number;
  hasTarget: boolean;
  inSafeArea: boolean;
  bagFull: boolean;
  hasItem: boolean;
  skillReady: boolean;
  isDead: boolean;
  targetDistance: number;
  monsterCount: number;
  blackboard: Map<string, unknown>;
}

// 执行状态
export interface ExecutionState {
  tree: BTNode | null;
  currentNodeId: string | null;
  nodeResults: Map<string, BTResult>;
  executedNodeIds: string[];
  isRunning: boolean;
  speed: number;
  logs: LogEntry[];
  executionStack: string[];
  stepIndex: number;
}

// 单条日志
export interface LogEntry {
  timestamp: number;
  nodeId: string;
  nodeName: string;
  result: BTResult;
  depth: number;
  type: BTNodeType;
}

// 节点类型颜色映射
export const NODE_TYPE_COLORS: Record<BTNodeCategory, string> = {
  [BTNodeCategory.COMPOSITE]: '#ffaa00',
  [BTNodeCategory.DECORATOR]: '#cc66ff',
  [BTNodeCategory.CONDITION]: '#00ccff',
  [BTNodeCategory.ACTION]: '#ff8844',
};

// 结果颜色映射
export const RESULT_COLORS: Record<BTResult, string> = {
  [BTResult.SUCCESS]: '#00ff88',
  [BTResult.FAILURE]: '#ff4466',
  [BTResult.RUNNING]: '#4488ff',
  [BTResult.IDLE]: '#555566',
};

// 节点类型中文名
export const NODE_TYPE_LABELS: Record<string, string> = {
  'Sequence': '序列节点',
  'Selector': '选择节点',
  'Parallel': '并行节点',
  'Random': '随机节点',
  'Probability': '概率节点',
  'MemSequence': '记忆序列',
  'MemSelector': '记忆选择',
  'Inverter': '反转节点',
  'DecoratorRepeat': '重复装饰',
  'DecoratorTimeout': '超时装饰',
  'DecoratorCooldown': '冷却装饰',
  'Succeeder': '强制成功',
  'Failer': '强制失败',
  'ConditionLowHP': 'HP低于阈值',
  'ConditionLowMP': 'MP低于阈值',
  'ConditionHasTarget': '是否有目标',
  'ConditionInSafeArea': '是否在安全区',
  'ConditionBagFull': '背包是否已满',
  'ConditionHasItem': '是否有物品',
  'ConditionSkillReady': '技能是否就绪',
  'ConditionIsDead': '是否死亡',
  'ConditionTargetDistance': '目标距离判断',
  'ConditionTargetType': '目标类型判断',
  'ConditionHasNearbyPlayer': '附近是否有玩家',
  'ConditionMonsterCount': '怪物数量',
  'ConditionHasPotion': '是否有药水',
  'ConditionHasDroppedItem': '地上有掉落物',
  'ConditionHPRange': 'HP百分比范围',
  'ConditionTimeOfDay': '游戏时间段',
  'ActionUsePotion': '使用药水',
  'ActionUseItem': '使用物品',
  'ActionChangeAttackMode': '切换攻击模式',
  'ActionAttack': '攻击目标',
  'ActionMoveToTarget': '移向目标',
  'ActionPatrol': '巡逻移动',
  'ActionPickupItem': '拾取物品',
  'ActionFlee': '逃跑',
  'ActionRest': '休息',
  'ActionChat': '聊天',
  'ActionUseSkill': '使用技能',
  'ActionSay': '说出文本',
  'ActionRecall': '使用回城卷',
  'ActionDelay': '延迟等待',
  'ActionAttackDir': '方向攻击',
  'ActionSpellCast': '精确施法',
  'ActionDropItem': '丢弃物品',
  'ActionEquipItem': '穿戴装备',
  'ActionSummonPet': '召唤宠物',
  'ActionFollow': '跟随目标',
  'ActionGroup': '组队操作',
  'ActionMine': '挖矿',
};