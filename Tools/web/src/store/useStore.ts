import { create } from 'zustand';
import { BTNode, BTResult, LogEntry, ExecutionContext } from '../utils/btTypes';
import { parseXMLToTree } from '../utils/xmlParser';
import { BTEngine } from '../utils/btEngine';

interface AppState {
  // 行为树
  tree: BTNode | null;
  setTree: (tree: BTNode | null) => void;
  loadXML: (xml: string) => BTNode | null;
  loadFile: (file: File) => Promise<BTNode | null>;

  // 选中节点
  selectedNodeId: string | null;
  setSelectedNodeId: (id: string | null) => void;

  // 执行状态
  engine: BTEngine;
  isRunning: boolean;
  speed: number;
  currentNodeId: string | null;
  nodeResults: Map<string, BTResult>;
  executedNodeIds: string[];
  logs: LogEntry[];
  stepIndex: number;

  // 执行控制
  setSpeed: (speed: number) => void;
  stepOnce: () => void;
  startAuto: () => void;
  stopAuto: () => void;
  resetExecution: () => void;
  setContext: (ctx: Partial<ExecutionContext>) => void;
  getContext: () => ExecutionContext;

  // 节点编辑
  updateNodeParam: (nodeId: string, key: string, value: string) => void;
  updateNodeName: (nodeId: string, name: string) => void;
  toggleNodeCollapse: (nodeId: string) => void;

  // 定时器ID
  timerId: number | null;
}

export const useStore = create<AppState>((set, get) => ({
  tree: null,
  setTree: (tree) => {
    const engine = new BTEngine();
    set({
      tree,
      currentNodeId: null,
      nodeResults: new Map(),
      executedNodeIds: [],
      logs: [],
      stepIndex: 0,
      engine,
      isRunning: false,
      selectedNodeId: null,
    });
  },

  loadXML: (xml) => {
    const tree = parseXMLToTree(xml);
    if (tree) {
      get().setTree(tree);
    }
    return tree;
  },

  loadFile: async (file) => {
    const { loadXML } = get();
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = (e) => {
        const content = e.target?.result as string;
        const tree = loadXML(content);
        if (tree) resolve(tree);
        else reject(new Error('解析失败'));
      };
      reader.onerror = () => reject(new Error('文件读取失败'));
      reader.readAsText(file, 'GBK');
    });
  },

  selectedNodeId: null,
  setSelectedNodeId: (id) => set({ selectedNodeId: id }),

  engine: new BTEngine(),
  isRunning: false,
  speed: 500,
  currentNodeId: null,
  nodeResults: new Map(),
  executedNodeIds: [],
  logs: [],
  stepIndex: 0,

  setSpeed: (speed) => set({ speed }),

  stepOnce: () => {
    const { tree, engine, logs, stepIndex } = get();
    if (!tree) return;

    // 重置引擎并执行完整树
    engine.reset();
    const steps = engine.executeFull(tree);
    const results = engine.getResults();
    const executed = engine.getExecutedNodes();

    // 生成日志
    const newLogs: LogEntry[] = [];
    for (const step of steps) {
      // 查找节点
      const node = findNodeById(tree, step.nodeId);
      newLogs.push({
        timestamp: Date.now(),
        nodeId: step.nodeId,
        nodeName: node?.name || '未知节点',
        result: step.result,
        depth: node?.depth || 0,
        type: node?.type || 'Action' as never,
      });
    }

    set({
      currentNodeId: steps[steps.length - 1]?.nodeId || null,
      nodeResults: results,
      executedNodeIds: executed,
      logs: [...logs, ...newLogs],
      stepIndex: stepIndex + 1,
    });
  },

  startAuto: () => {
    const { timerId } = get();
    if (timerId) return;

    get().stepOnce();
    const id = window.setInterval(() => {
      const { isRunning } = get();
      if (!isRunning) {
        window.clearInterval(id);
        set({ timerId: null });
        return;
      }
      get().stepOnce();
    }, get().speed);

    set({ isRunning: true, timerId: id });
  },

  stopAuto: () => {
    const { timerId } = get();
    if (timerId) {
      window.clearInterval(timerId);
      set({ isRunning: false, timerId: null });
    }
  },

  resetExecution: () => {
    const { timerId } = get();
    if (timerId) {
      window.clearInterval(timerId);
    }
    const engine = new BTEngine();
    set({
      engine,
      isRunning: false,
      currentNodeId: null,
      nodeResults: new Map(),
      executedNodeIds: [],
      logs: [],
      stepIndex: 0,
      timerId: null,
    });
  },

  setContext: (ctx) => {
    const { engine } = get();
    engine.setContext(ctx);
  },

  getContext: () => {
    return get().engine.getContext();
  },

  updateNodeParam: (nodeId, key, value) => {
    const { tree } = get();
    if (!tree) return;
    const newTree = deepCloneTree(tree);
    const node = findNodeById(newTree, nodeId);
    if (node) {
      node.params = { ...node.params, [key]: value };
      set({ tree: newTree });
    }
  },

  updateNodeName: (nodeId, name) => {
    const { tree } = get();
    if (!tree) return;
    const newTree = deepCloneTree(tree);
    const node = findNodeById(newTree, nodeId);
    if (node) {
      node.name = name;
      set({ tree: newTree });
    }
  },

  toggleNodeCollapse: (nodeId) => {
    const { tree } = get();
    if (!tree) return;
    const newTree = deepCloneTree(tree);
    const node = findNodeById(newTree, nodeId);
    if (node) {
      node.collapsed = !node.collapsed;
      set({ tree: newTree });
    }
  },

  timerId: null,
}));

// 在树中查找节点
export function findNodeById(tree: BTNode, id: string): BTNode | null {
  if (tree.id === id) return tree;
  for (const child of tree.children) {
    const found = findNodeById(child, id);
    if (found) return found;
  }
  return null;
}

// 深拷贝树节点
function deepCloneTree(node: BTNode): BTNode {
  return {
    ...node,
    params: { ...node.params },
    children: node.children.map(deepCloneTree),
  };
}