import {
  BTNode,
  BTNodeType,
  BTResult,
  ExecutionContext,
  getNodeCategory,
  BTNodeCategory,
} from './btTypes';

// 简单的伪随机数生成器（可重现）
class SeededRandom {
  private seed: number;
  constructor(seed: number) { this.seed = seed; }
  next(): number {
    this.seed = (this.seed * 16807 + 0) % 2147483647;
    return this.seed / 2147483647;
  }
  nextInt(max: number): number {
    return Math.floor(this.next() * max);
  }
  percentChance(percent: number): boolean {
    return this.next() * 100 < percent;
  }
}

// 行为树执行引擎
export class BTEngine {
  private rng: SeededRandom;
  private nodeResults: Map<string, BTResult> = new Map();
  private executedNodes: string[] = [];
  private memPositions: Map<string, number> = new Map();
  private ctx: ExecutionContext;

  constructor(seed: number = 42) {
    this.rng = new SeededRandom(seed);
    this.ctx = this.createDefaultContext();
  }

  reset(seed?: number): void {
    if (seed !== undefined) this.rng = new SeededRandom(seed);
    this.nodeResults.clear();
    this.executedNodes = [];
    this.memPositions.clear();
    this.ctx = this.createDefaultContext();
  }

  getResults(): Map<string, BTResult> {
    return new Map(this.nodeResults);
  }

  getExecutedNodes(): string[] {
    return [...this.executedNodes];
  }

  getContext(): ExecutionContext {
    return { ...this.ctx, blackboard: new Map(this.ctx.blackboard) };
  }

  setContext(partial: Partial<ExecutionContext>): void {
    Object.assign(this.ctx, partial);
  }

  // 执行整棵树，返回所有执行过的节点和结果
  executeFull(tree: BTNode): { nodeId: string; result: BTResult }[] {
    this.nodeResults.clear();
    this.executedNodes = [];
    const steps: { nodeId: string; result: BTResult }[] = [];
    this.executeNode(tree, steps);
    return steps;
  }

  // 单步执行：返回下一个要执行的节点
  executeStep(node: BTNode): { nodeId: string; result: BTResult; done: boolean } {
    const steps = this.executeNodeAtomic(node);
    if (steps.length === 0) {
      return { nodeId: node.id, result: BTResult.FAILURE, done: true };
    }
    const last = steps[steps.length - 1];
    return { nodeId: last.nodeId, result: last.result, done: true };
  }

  private executeNodeAtomic(node: BTNode): { nodeId: string; result: BTResult }[] {
    this.nodeResults.clear();
    this.executedNodes = [];
    const steps: { nodeId: string; result: BTResult }[] = [];
    this.executeNode(node, steps);
    return steps;
  }

  private executeNode(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    this.executedNodes.push(node.id);

    let result: BTResult;
    const category = getNodeCategory(node.type);

    switch (node.type) {
      case BTNodeType.SEQUENCE:
        result = this.executeSequence(node, steps);
        break;
      case BTNodeType.SELECTOR:
        result = this.executeSelector(node, steps);
        break;
      case BTNodeType.PARALLEL:
        result = this.executeParallel(node, steps);
        break;
      case BTNodeType.RANDOM:
        result = this.executeRandom(node, steps);
        break;
      case BTNodeType.PROBABILITY:
        result = this.executeProbability(node, steps);
        break;
      case BTNodeType.MEM_SEQUENCE:
        result = this.executeMemSequence(node, steps);
        break;
      case BTNodeType.MEM_SELECTOR:
        result = this.executeMemSelector(node, steps);
        break;
      case BTNodeType.INVERTER:
        result = this.executeInverter(node, steps);
        break;
      case BTNodeType.DECORATOR_REPEAT:
        result = this.executeDecoratorRepeat(node, steps);
        break;
      case BTNodeType.DECORATOR_TIMEOUT:
        result = this.executeDecoratorTimeout(node, steps);
        break;
      case BTNodeType.DECORATOR_COOLDOWN:
        result = this.executeDecoratorCooldown(node, steps);
        break;
      case BTNodeType.SUCCEEDER:
        result = this.executeSucceeder(node, steps);
        break;
      case BTNodeType.FAILER:
        result = this.executeFailer(node, steps);
        break;
      case BTNodeType.CONDITION:
        result = this.executeCondition(node);
        break;
      case BTNodeType.ACTION:
        result = this.executeAction(node);
        break;
      default:
        result = BTResult.FAILURE;
    }

    this.nodeResults.set(node.id, result);
    steps.push({ nodeId: node.id, result });
    return result;
  }

  // === 复合节点 ===

  private executeSequence(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    for (const child of node.children) {
      const r = this.executeNode(child, steps);
      if (r !== BTResult.SUCCESS) return r;
    }
    return BTResult.SUCCESS;
  }

  private executeSelector(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    for (const child of node.children) {
      const r = this.executeNode(child, steps);
      if (r === BTResult.SUCCESS) return BTResult.SUCCESS;
    }
    return BTResult.FAILURE;
  }

  private executeParallel(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    let successCount = 0;
    let failureCount = 0;
    for (const child of node.children) {
      const r = this.executeNode(child, steps);
      if (r === BTResult.SUCCESS) successCount++;
      else if (r === BTResult.FAILURE) failureCount++;
    }
    if (successCount === node.children.length) return BTResult.SUCCESS;
    if (failureCount > 0) return BTResult.FAILURE;
    return BTResult.RUNNING;
  }

  private executeRandom(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.FAILURE;
    const idx = this.rng.nextInt(node.children.length);
    return this.executeNode(node.children[idx], steps);
  }

  private executeProbability(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    const chance = parseInt(node.params['chance'] || '50');
    if (this.rng.percentChance(chance)) {
      let result = BTResult.SUCCESS;
      for (const child of node.children) {
        result = this.executeNode(child, steps);
      }
      return result;
    }
    return BTResult.FAILURE;
  }

  private executeMemSequence(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    const startIdx = this.memPositions.get(node.id) || 0;
    for (let i = startIdx; i < node.children.length; i++) {
      const r = this.executeNode(node.children[i], steps);
      if (r !== BTResult.SUCCESS) {
        this.memPositions.set(node.id, i);
        return r;
      }
    }
    this.memPositions.delete(node.id);
    return BTResult.SUCCESS;
  }

  private executeMemSelector(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    const startIdx = this.memPositions.get(node.id) || 0;
    for (let i = startIdx; i < node.children.length; i++) {
      const r = this.executeNode(node.children[i], steps);
      if (r === BTResult.SUCCESS) {
        this.memPositions.delete(node.id);
        return BTResult.SUCCESS;
      }
    }
    this.memPositions.delete(node.id);
    return BTResult.FAILURE;
  }

  // === 装饰节点 ===

  private executeInverter(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.FAILURE;
    const r = this.executeNode(node.children[0], steps);
    if (r === BTResult.SUCCESS) return BTResult.FAILURE;
    if (r === BTResult.FAILURE) return BTResult.SUCCESS;
    return BTResult.RUNNING;
  }

  private executeDecoratorRepeat(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    const count = parseInt(node.params['count'] || '1');
    if (node.children.length === 0) return BTResult.FAILURE;
    let lastResult = BTResult.SUCCESS;
    const maxIterations = count === 0 ? 3 : count; // count=0 表示无限，这里限制3次
    for (let i = 0; i < maxIterations; i++) {
      lastResult = this.executeNode(node.children[0], steps);
      if (lastResult !== BTResult.SUCCESS) return lastResult;
    }
    return lastResult;
  }

  private executeDecoratorTimeout(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.FAILURE;
    // 模拟超时检查
    const timeout = parseInt(node.params['timeout'] || '5000');
    return this.executeNode(node.children[0], steps);
  }

  private executeDecoratorCooldown(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.FAILURE;
    // 模拟冷却检查
    return this.executeNode(node.children[0], steps);
  }

  private executeSucceeder(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.SUCCESS;
    this.executeNode(node.children[0], steps);
    return BTResult.SUCCESS;
  }

  private executeFailer(node: BTNode, steps: { nodeId: string; result: BTResult }[]): BTResult {
    if (node.children.length === 0) return BTResult.FAILURE;
    this.executeNode(node.children[0], steps);
    return BTResult.FAILURE;
  }

  // === 条件节点 ===

  private executeCondition(node: BTNode): BTResult {
    const condType = node.conditionType;
    const params = node.params;

    switch (condType) {
      case 'ConditionLowHP': {
        const percent = parseInt(params['percent'] || '50');
        return this.ctx.hpPercent < percent ? BTResult.SUCCESS : BTResult.FAILURE;
      }
      case 'ConditionLowMP': {
        const percent = parseInt(params['percent'] || '50');
        return this.ctx.mpPercent < percent ? BTResult.SUCCESS : BTResult.FAILURE;
      }
      case 'ConditionHasTarget':
        return this.ctx.hasTarget ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionInSafeArea':
        return this.ctx.inSafeArea ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionBagFull':
        return this.ctx.bagFull ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionHasItem':
        return this.ctx.hasItem ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionSkillReady':
        return this.ctx.skillReady ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionIsDead':
        return this.ctx.isDead ? BTResult.SUCCESS : BTResult.FAILURE;
      case 'ConditionTargetDistance': {
        const min = parseInt(params['min'] || '0');
        const max = parseInt(params['max'] || '10');
        return (this.ctx.targetDistance >= min && this.ctx.targetDistance <= max)
          ? BTResult.SUCCESS : BTResult.FAILURE;
      }
      case 'ConditionMonsterCount': {
        const count = parseInt(params['count'] || '0');
        return this.ctx.monsterCount >= count ? BTResult.SUCCESS : BTResult.FAILURE;
      }
      case 'ConditionHasPotion':
      case 'ConditionHasDroppedItem':
      case 'ConditionTargetType':
      case 'ConditionHasNearbyPlayer':
      case 'ConditionHPRange':
      case 'ConditionTimeOfDay':
        // 通用条件模拟
        return this.rng.percentChance(50) ? BTResult.SUCCESS : BTResult.FAILURE;
      default:
        return BTResult.FAILURE;
    }
  }

  // === 动作节点 ===

  private executeAction(node: BTNode): BTResult {
    // 动作节点模拟执行，通常返回 SUCCESS
    return this.rng.percentChance(85) ? BTResult.SUCCESS : BTResult.FAILURE;
  }

  // === 默认上下文 ===

  private createDefaultContext(): ExecutionContext {
    return {
      hpPercent: 80,
      mpPercent: 70,
      hasTarget: true,
      inSafeArea: false,
      bagFull: false,
      hasItem: true,
      skillReady: true,
      isDead: false,
      targetDistance: 3,
      monsterCount: 2,
      blackboard: new Map(),
    };
  }
}