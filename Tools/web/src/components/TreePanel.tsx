import { useStore } from '../store/useStore';
import { TreeNode } from './TreeNode';

export function TreePanel() {
  const { tree } = useStore();

  if (!tree) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center text-[#444] font-mono">
        <div className="text-5xl mb-4 opacity-20">&#9672;</div>
        <div className="text-sm mb-2">暂无行为树数据</div>
        <div className="text-xs text-[#333]">
          加载 XML 文件或选择内置示例
        </div>
      </div>
    );
  }

  return (
    <div className="flex-1 overflow-auto custom-scrollbar">
      <div className="py-2">
        <div className="relative">
          <TreeNode
            key={tree.id}
            node={tree}
            isLast={true}
          />
        </div>
      </div>
    </div>
  );
}