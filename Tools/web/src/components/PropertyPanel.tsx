import { useState } from 'react';
import { Edit3, Check, X } from 'lucide-react';
import { useStore, findNodeById } from '../store/useStore';
import { getNodeCategory, NODE_TYPE_COLORS, NODE_TYPE_LABELS } from '../utils/btTypes';

export function PropertyPanel() {
  const { tree, selectedNodeId, updateNodeParam, updateNodeName } = useStore();
  const [editingKey, setEditingKey] = useState<string | null>(null);
  const [editValue, setEditValue] = useState('');

  if (!tree || !selectedNodeId) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center text-[#444] font-mono">
        <div className="text-3xl mb-3 opacity-20">&#9881;</div>
        <div className="text-xs">点击左侧节点查看属性</div>
      </div>
    );
  }

  const node = findNodeById(tree, selectedNodeId);
  if (!node) {
    return (
      <div className="flex-1 flex items-center justify-center text-[#444] font-mono text-xs">
        节点未找到
      </div>
    );
  }

  const category = getNodeCategory(node.type);
  const color = NODE_TYPE_COLORS[category];
  const label = NODE_TYPE_LABELS[node.conditionType || node.actionType || node.type] || node.type;

  const startEdit = (key: string, value: string) => {
    setEditingKey(key);
    setEditValue(value);
  };

  const saveEdit = () => {
    if (editingKey) {
      if (editingKey === '__name__') {
        updateNodeName(selectedNodeId, editValue);
      } else {
        updateNodeParam(selectedNodeId, editingKey, editValue);
      }
      setEditingKey(null);
    }
  };

  const cancelEdit = () => {
    setEditingKey(null);
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') saveEdit();
    if (e.key === 'Escape') cancelEdit();
  };

  return (
    <div className="flex-1 overflow-auto custom-scrollbar p-3">
      {/* 节点标题 */}
      <div className="mb-3 pb-3 border-b border-[#1a1a3a]">
        <div className="flex items-center gap-2 mb-2">
          <span
            className="text-[10px] px-1.5 py-0.5 rounded font-mono"
            style={{ backgroundColor: color + '20', color }}
          >
            {node.type}
          </span>
          <span className="text-[10px] font-mono text-[#666]">{label}</span>
        </div>

        {/* 名称编辑 */}
        {editingKey === '__name__' ? (
          <div className="flex items-center gap-1">
            <input
              value={editValue}
              onChange={(e) => setEditValue(e.target.value)}
              onKeyDown={handleKeyDown}
              className="flex-1 px-2 py-1 rounded text-xs font-mono bg-[#0d0d1a] text-[#eee] border border-[#00ff88]/30 focus:border-[#00ff88]/60 focus:outline-none"
              autoFocus
            />
            <button onClick={saveEdit} className="p-1 text-[#00ff88] hover:bg-[#00ff88]/10 rounded"><Check size={12} /></button>
            <button onClick={cancelEdit} className="p-1 text-[#ff4466] hover:bg-[#ff4466]/10 rounded"><X size={12} /></button>
          </div>
        ) : (
          <div
            className="flex items-center gap-2 group cursor-pointer"
            onClick={() => startEdit('__name__', node.name)}
          >
            <span className="text-sm font-mono text-[#ddd]">{node.name}</span>
            <Edit3 size={10} className="opacity-0 group-hover:opacity-40 text-[#888] transition-opacity" />
          </div>
        )}
      </div>

      {/* 基本信息 */}
      <div className="mb-3">
        <div className="text-[10px] font-mono text-[#555] mb-2 uppercase tracking-wider">基本信息</div>
        <div className="space-y-1.5">
          <InfoRow label="节点ID" value={node.id.slice(0, 16) + '...'} />
          <InfoRow label="深度" value={String(node.depth)} />
          <InfoRow label="子节点数" value={String(node.children.length)} />
          <InfoRow label="父节点" value={node.parentId ? '有' : '无（根节点）'} />
        </div>
      </div>

      {/* 参数列表 */}
      {Object.keys(node.params).length > 0 && (
        <div>
          <div className="text-[10px] font-mono text-[#555] mb-2 uppercase tracking-wider">节点参数</div>
          <div className="space-y-1">
            {Object.entries(node.params).map(([key, value]) => (
              <div key={key} className="flex items-center gap-2 py-1 px-2 rounded bg-[#0a0a18] group">
                <span className="text-[10px] font-mono text-[#666] w-20 flex-shrink-0">{key}</span>
                {editingKey === key ? (
                  <div className="flex items-center gap-1 flex-1">
                    <input
                      value={editValue}
                      onChange={(e) => setEditValue(e.target.value)}
                      onKeyDown={handleKeyDown}
                      className="flex-1 px-2 py-0.5 rounded text-xs font-mono bg-[#0d0d1a] text-[#eee] border border-[#00ff88]/30 focus:border-[#00ff88]/60 focus:outline-none"
                      autoFocus
                    />
                    <button onClick={saveEdit} className="p-1 text-[#00ff88] hover:bg-[#00ff88]/10 rounded"><Check size={12} /></button>
                    <button onClick={cancelEdit} className="p-1 text-[#ff4466] hover:bg-[#ff4466]/10 rounded"><X size={12} /></button>
                  </div>
                ) : (
                  <>
                    <span className="text-xs font-mono text-[#ffaa00] flex-1">{value}</span>
                    <button
                      onClick={() => startEdit(key, value)}
                      className="opacity-0 group-hover:opacity-40 text-[#888] hover:opacity-100 transition-opacity"
                    >
                      <Edit3 size={10} />
                    </button>
                  </>
                )}
              </div>
            ))}
          </div>
        </div>
      )}

      {/* 子节点预览 */}
      {node.children.length > 0 && (
        <div className="mt-3">
          <div className="text-[10px] font-mono text-[#555] mb-2 uppercase tracking-wider">
            子节点 ({node.children.length})
          </div>
          <div className="space-y-0.5">
            {node.children.map((child) => {
              const childCat = getNodeCategory(child.type);
              const childColor = NODE_TYPE_COLORS[childCat];
              return (
                <div
                  key={child.id}
                  className="flex items-center gap-1.5 py-0.5 px-2 rounded text-[10px] font-mono bg-[#0a0a18]"
                >
                  <span style={{ color: childColor }}>
                    {childCat === 'composite' ? '◆' : childCat === 'decorator' ? '◇' : childCat === 'condition' ? '?' : '▶'}
                  </span>
                  <span className="text-[#999] truncate">{child.name}</span>
                  <span className="text-[#555] ml-auto flex-shrink-0">{child.type}</span>
                </div>
              );
            })}
          </div>
        </div>
      )}
    </div>
  );
}

function InfoRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex items-center gap-2 py-0.5 px-2 rounded bg-[#0a0a18]">
      <span className="text-[10px] font-mono text-[#555] w-16 flex-shrink-0">{label}</span>
      <span className="text-[10px] font-mono text-[#999]">{value}</span>
    </div>
  );
}