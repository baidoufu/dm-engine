import { Toolbar } from './components/Toolbar';
import { TreePanel } from './components/TreePanel';
import { PropertyPanel } from './components/PropertyPanel';
import { LogPanel } from './components/LogPanel';
import { useStore } from './store/useStore';
import { BTNode, getNodeCategory, BTNodeCategory, NODE_TYPE_COLORS } from './utils/btTypes';

function App() {
  const { tree, executedNodeIds } = useStore();

  const getStats = () => {
    if (!tree) return { total: 0, composite: 0, decorator: 0, condition: 0, action: 0 };
    const stats = { total: 0, composite: 0, decorator: 0, condition: 0, action: 0 };
    const walk = (node: BTNode) => {
      stats.total++;
      const cat = getNodeCategory(node.type);
      if (cat === BTNodeCategory.COMPOSITE) stats.composite++;
      else if (cat === BTNodeCategory.DECORATOR) stats.decorator++;
      else if (cat === BTNodeCategory.CONDITION) stats.condition++;
      else stats.action++;
      node.children.forEach(walk);
    };
    walk(tree);
    return stats;
  };

  const stats = getStats();

  return (
    <div className="h-screen flex flex-col bg-[#0a0a14] text-[#ccc] overflow-hidden">
      {/* 工具栏 */}
      <Toolbar />

      {/* 主内容区 */}
      <div className="flex-1 flex overflow-hidden">
        {/* 左侧 - 树形面板 */}
        <div className="w-[60%] flex flex-col border-r border-[#1a1a3a] bg-[#0d0d1a]">
          {/* 面板标题 */}
          <div className="flex items-center gap-2 px-4 py-2 border-b border-[#1a1a3a] bg-[#0a0a14]">
            <span className="text-[10px] font-mono text-[#555] tracking-wider">行为树结构</span>
            {tree && (
              <div className="flex items-center gap-3 ml-auto">
                <StatBadge label="总计" value={stats.total} color="#888" />
                <StatBadge label="复合" value={stats.composite} color={NODE_TYPE_COLORS[BTNodeCategory.COMPOSITE]} />
                <StatBadge label="装饰" value={stats.decorator} color={NODE_TYPE_COLORS[BTNodeCategory.DECORATOR]} />
                <StatBadge label="条件" value={stats.condition} color={NODE_TYPE_COLORS[BTNodeCategory.CONDITION]} />
                <StatBadge label="动作" value={stats.action} color={NODE_TYPE_COLORS[BTNodeCategory.ACTION]} />
                {executedNodeIds.length > 0 && (
                  <span className="text-[10px] font-mono text-[#00ff88]">
                    已执行 {executedNodeIds.length} 节点
                  </span>
                )}
              </div>
            )}
          </div>
          <TreePanel />
        </div>

        {/* 右侧 - 属性 + 日志 */}
        <div className="w-[40%] flex flex-col bg-[#0a0a14]">
          {/* 属性面板 */}
          <div className="flex-1 flex flex-col border-b border-[#1a1a3a] min-h-0">
            <div className="flex items-center px-4 py-2 border-b border-[#1a1a3a] bg-[#0d0d1a]">
              <span className="text-[10px] font-mono text-[#555] tracking-wider">节点属性</span>
            </div>
            <PropertyPanel />
          </div>

          {/* 执行日志 */}
          <div className="flex-1 flex flex-col min-h-0">
            <div className="flex items-center gap-2 px-4 py-2 border-b border-[#1a1a3a] bg-[#0d0d1a]">
              <span className="text-[10px] font-mono text-[#555] tracking-wider">执行日志</span>
              <span className="text-[10px] font-mono text-[#333]">执行顺序</span>
            </div>
            <LogPanel />
          </div>
        </div>
      </div>
    </div>
  );
}

function StatBadge({ label, value, color }: { label: string; value: number; color: string }) {
  return (
    <div className="flex items-center gap-1">
      <span className="text-[9px] font-mono text-[#555]">{label}</span>
      <span className="text-[9px] font-mono font-bold" style={{ color }}>{value}</span>
    </div>
  );
}

export default App;