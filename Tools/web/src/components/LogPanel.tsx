import { useEffect, useRef } from 'react';
import { Terminal } from 'lucide-react';
import { useStore } from '../store/useStore';
import { BTResult, RESULT_COLORS } from '../utils/btTypes';

export function LogPanel() {
  const { logs, tree } = useStore();
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [logs.length]);

  if (!tree) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center text-[#444] font-mono">
        <Terminal size={28} className="mb-3 opacity-20" />
        <div className="text-xs">加载行为树后开始调试</div>
      </div>
    );
  }

  if (logs.length === 0) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center text-[#444] font-mono">
        <Terminal size={28} className="mb-3 opacity-20" />
        <div className="text-xs">点击"单步"或"播放"开始执行</div>
      </div>
    );
  }

  const resultLabel = (r: BTResult): string => {
    switch (r) {
      case BTResult.SUCCESS: return 'SUCCESS';
      case BTResult.FAILURE: return 'FAILURE';
      case BTResult.RUNNING: return 'RUNNING';
      case BTResult.IDLE: return 'IDLE';
    }
  };

  return (
    <div ref={scrollRef} className="flex-1 overflow-auto custom-scrollbar p-2 font-mono text-[11px] leading-relaxed">
      {logs.map((log, i) => {
        const color = RESULT_COLORS[log.result];
        const prefix = '─'.repeat(log.depth);
        return (
          <div
            key={`${log.nodeId}_${i}`}
            className="flex items-start gap-2 py-0.5 hover:bg-[#ffffff03] rounded px-1 transition-colors"
          >
            <span className="text-[#444] flex-shrink-0 w-12 text-right">
              {String(i + 1).padStart(3, '0')}
            </span>
            <span className="text-[#2a2a4a] flex-shrink-0">{prefix}</span>
            <span
              className="px-1 py-0 rounded text-[9px] font-bold flex-shrink-0"
              style={{ backgroundColor: color + '18', color }}
            >
              {resultLabel(log.result)}
            </span>
            <span className="text-[#999] truncate">{log.nodeName}</span>
            <span className="text-[#444] flex-shrink-0 ml-auto">{log.type}</span>
          </div>
        );
      })}
    </div>
  );
}