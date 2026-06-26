import { useCallback } from 'react';
import { ChevronRight, ChevronDown } from 'lucide-react';
import { BTNode, BTResult, getNodeCategory, BTNodeCategory, NODE_TYPE_COLORS, RESULT_COLORS } from '../utils/btTypes';
import { useStore } from '../store/useStore';

interface TreeNodeProps {
  node: BTNode;
  isLast: boolean;
}

export function TreeNode({ node, isLast }: TreeNodeProps) {
  const {
    selectedNodeId,
    setSelectedNodeId,
    nodeResults,
    currentNodeId,
    toggleNodeCollapse,
    executedNodeIds,
  } = useStore();

  const category = getNodeCategory(node.type);
  const color = NODE_TYPE_COLORS[category];
  const result = nodeResults.get(node.id);
  const isActive = node.id === currentNodeId;
  const isSelected = node.id === selectedNodeId;
  const hasChildren = node.children.length > 0;
  const wasExecuted = executedNodeIds.includes(node.id);

  const handleClick = useCallback(() => {
    setSelectedNodeId(node.id);
  }, [node.id, setSelectedNodeId]);

  const handleToggle = useCallback((e: React.MouseEvent) => {
    e.stopPropagation();
    toggleNodeCollapse(node.id);
  }, [node.id, toggleNodeCollapse]);

  // 节点类型图标
  const getIcon = (): string => {
    switch (category) {
      case BTNodeCategory.COMPOSITE: return '◆';
      case BTNodeCategory.DECORATOR: return '◇';
      case BTNodeCategory.CONDITION: return '?';
      case BTNodeCategory.ACTION: return '▶';
    }
  };

  return (
    <div className="select-none">
      <div
        onClick={handleClick}
        className={`
          group flex items-center gap-1 py-1 px-2 rounded cursor-pointer transition-all duration-150
          ${isSelected ? 'bg-[#1a2a3a] ring-1 ring-[#4488ff]/30' : 'hover:bg-[#ffffff03]'}
          ${isActive ? 'animate-pulse' : ''}
          ${wasExecuted && !isActive ? 'opacity-90' : ''}
        `}
        style={{ paddingLeft: `${node.depth * 16 + 8}px` }}
      >
        {/* 展开/折叠按钮 */}
        {hasChildren ? (
          <button
            onClick={handleToggle}
            className="flex-shrink-0 w-4 h-4 flex items-center justify-center text-[#555] hover:text-[#888] transition-colors"
          >
            {node.collapsed ? <ChevronRight size={12} /> : <ChevronDown size={12} />}
          </button>
        ) : (
          <span className="w-4 flex-shrink-0" />
        )}

        {/* 节点类型图标 */}
        <span
          className="flex-shrink-0 text-[10px] w-4 text-center"
          style={{ color }}
        >
          {getIcon()}
        </span>

        {/* 节点名称 */}
        <span
          className="text-xs font-mono truncate transition-colors duration-200"
          style={{ color: isActive ? '#fff' : '#aaa' }}
        >
          {node.name}
        </span>

        {/* 节点类型标签 */}
        <span
          className="flex-shrink-0 text-[9px] px-1 py-0.5 rounded font-mono opacity-60"
          style={{ backgroundColor: color + '18', color }}
        >
          {node.type}
        </span>

        {/* 执行状态 */}
        {result && (
          <span
            className="flex-shrink-0 w-1.5 h-1.5 rounded-full ml-auto"
            style={{ backgroundColor: isActive ? RESULT_COLORS[BTResult.RUNNING] : RESULT_COLORS[result] }}
            title={result}
          />
        )}

        {/* 连接线 */}
        {!isLast && node.depth > 0 && (
          <div
            className="absolute left-0 w-px bg-[#2a2a4a]"
            style={{
              left: `${node.depth * 16 - 4}px`,
              top: '0',
              bottom: '0',
              height: '100%',
            }}
          />
        )}
      </div>

      {/* 子节点 */}
      {!node.collapsed && node.children.map((child, index) => (
        <TreeNode
          key={child.id}
          node={child}
          isLast={index === node.children.length - 1}
        />
      ))}

      {/* 连接线 */}
      {node.depth > 0 && (
        <div
          className="absolute left-0 w-px bg-[#16162e]"
          style={{
            left: `${node.depth * 16 - 4}px`,
            top: '0',
            height: '24px',
          }}
        />
      )}
    </div>
  );
}