import { BTNode, BTNodeType, ConditionType, ActionType } from './btTypes';

let nodeIdCounter = 0;

function generateId(): string {
  return `node_${++nodeIdCounter}_${Math.random().toString(36).slice(2, 8)}`;
}

// 条件节点类型映射表
const CONDITION_TYPE_SET = new Set<string>(Object.values(ConditionType));
// 动作节点类型映射表
const ACTION_TYPE_SET = new Set<string>(Object.values(ActionType));

// 条件节点子类型 -> BTNodeType 映射
function getConditionType(tagName: string): ConditionType | undefined {
  const normalized = tagName.trim();
  if (CONDITION_TYPE_SET.has(normalized)) {
    return normalized as ConditionType;
  }
  return undefined;
}

// 动作节点子类型 -> BTNodeType 映射
function getActionType(tagName: string): ActionType | undefined {
  const normalized = tagName.trim();
  if (ACTION_TYPE_SET.has(normalized)) {
    return normalized as ActionType;
  }
  return undefined;
}

// 标签名 -> BTNodeType 映射
function getNodeType(tagName: string): BTNodeType {
  const normalized = tagName.trim();
  switch (normalized) {
    case 'Sequence': return BTNodeType.SEQUENCE;
    case 'Selector': return BTNodeType.SELECTOR;
    case 'Parallel': return BTNodeType.PARALLEL;
    case 'Random': return BTNodeType.RANDOM;
    case 'Probability': return BTNodeType.PROBABILITY;
    case 'MemSequence': return BTNodeType.MEM_SEQUENCE;
    case 'MemSelector': return BTNodeType.MEM_SELECTOR;
    case 'Inverter': return BTNodeType.INVERTER;
    case 'DecoratorRepeat': return BTNodeType.DECORATOR_REPEAT;
    case 'DecoratorTimeout': return BTNodeType.DECORATOR_TIMEOUT;
    case 'DecoratorCooldown': return BTNodeType.DECORATOR_COOLDOWN;
    case 'Succeeder': return BTNodeType.SUCCEEDER;
    case 'Failer': return BTNodeType.FAILER;
    default:
      if (CONDITION_TYPE_SET.has(normalized)) return BTNodeType.CONDITION;
      if (ACTION_TYPE_SET.has(normalized)) return BTNodeType.ACTION;
      return BTNodeType.ACTION;
  }
}

// 解析 XML 字符串为行为树节点
export function parseXMLToTree(xmlStr: string): BTNode | null {
  nodeIdCounter = 0;
  try {
    const parser = new DOMParser();
    const doc = parser.parseFromString(xmlStr, 'text/xml');

    const errorNode = doc.querySelector('parsererror');
    if (errorNode) {
      console.error('XML 解析错误:', errorNode.textContent);
      return null;
    }

    const rootElement = doc.documentElement;
    if (!rootElement || rootElement.tagName !== 'BehaviorTree') {
      console.error('根节点必须是 BehaviorTree');
      return null;
    }

    const treeName = rootElement.getAttribute('name') || '未命名行为树';

    // 寻找第一个非注释子节点作为实际根节点
    let actualRoot: Element | null = null;
    for (let i = 0; i < rootElement.children.length; i++) {
      const child = rootElement.children[i];
      if (child.nodeType === 1) {
        actualRoot = child;
        break;
      }
    }

    if (!actualRoot) {
      console.error('行为树中没有有效的子节点');
      return null;
    }

    return parseElement(actualRoot, null, 0, treeName);
  } catch (e) {
    console.error('解析 XML 失败:', e);
    return null;
  }
}

// 递归解析 XML 元素
function parseElement(
  element: Element,
  parentId: string | null,
  depth: number,
  treeName: string
): BTNode {
  const id = generateId();
  const tagName = element.tagName;
  const type = getNodeType(tagName);
  const conditionType = getConditionType(tagName);
  const actionType = getActionType(tagName);

  // 提取属性作为参数
  const params: Record<string, string> = {};
  for (let i = 0; i < element.attributes.length; i++) {
    const attr = element.attributes[i];
    if (attr.name !== 'name') {
      params[attr.name] = attr.value;
    }
  }

  const name = element.getAttribute('name') || getDefaultName(tagName, type, treeName, depth);

  // 解析子节点（跳过注释和其他非元素节点）
  const children: BTNode[] = [];
  for (let i = 0; i < element.children.length; i++) {
    const child = element.children[i];
    if (child.nodeType === 1) {
      children.push(parseElement(child, id, depth + 1, treeName));
    }
  }

  return {
    id,
    type,
    conditionType,
    actionType,
    name,
    params,
    children,
    parentId,
    depth,
    collapsed: depth > 3,
  };
}

// 获取节点默认名称
function getDefaultName(
  tagName: string,
  type: BTNodeType,
  treeName: string,
  depth: number
): string {
  if (depth === 0) return treeName;
  const baseName = tagName.replace('Condition', '').replace('Action', '');
  return baseName.replace(/([A-Z])/g, ' $1').trim();
}

// 从文件加载并解析
export function loadXMLFile(file: File): Promise<BTNode | null> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      const content = e.target?.result as string;
      const tree = parseXMLToTree(content);
      resolve(tree);
    };
    reader.onerror = () => reject(new Error('文件读取失败'));
    reader.readAsText(file, 'GBK');
  });
}