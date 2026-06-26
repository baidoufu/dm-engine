import { useRef, useCallback } from 'react';
import { Upload, FolderOpen } from 'lucide-react';
import { useStore } from '../store/useStore';
import { SAMPLE_TREES } from '../utils/sampleData';

export function Toolbar() {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const { loadFile, loadXML, tree, isRunning, startAuto, stopAuto, stepOnce, resetExecution, speed, setSpeed } = useStore();

  const handleFileChange = useCallback(async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    try {
      await loadFile(file);
    } catch (err) {
      alert('文件加载失败: ' + (err as Error).message);
    }
    if (fileInputRef.current) fileInputRef.current.value = '';
  }, [loadFile]);

  const handleSampleSelect = useCallback((e: React.ChangeEvent<HTMLSelectElement>) => {
    const name = e.target.value;
    if (!name) return;
    const sample = SAMPLE_TREES.find((s) => s.name === name);
    if (sample) loadXML(sample.xml);
  }, [loadXML]);

  const handleDrop = useCallback(async (e: React.DragEvent) => {
    e.preventDefault();
    const file = e.dataTransfer.files[0];
    if (!file) return;
    try {
      await loadFile(file);
    } catch (err) {
      alert('文件加载失败: ' + (err as Error).message);
    }
  }, [loadFile]);

  const handleDragOver = useCallback((e: React.DragEvent) => {
    e.preventDefault();
  }, []);

  return (
    <div className="flex items-center gap-3 px-4 py-2.5 border-b border-[#2a2a4a] bg-[#0d0d1a]">
      <div className="flex items-center gap-2">
        <button
          onClick={() => fileInputRef.current?.click()}
          className="flex items-center gap-1.5 px-3 py-1.5 rounded text-xs font-mono text-[#888] hover:text-[#00ff88] border border-[#2a2a4a] hover:border-[#00ff88]/30 transition-all duration-200"
        >
          <Upload size={14} />
          加载XML
        </button>
        <input
          ref={fileInputRef}
          type="file"
          accept=".xml"
          onChange={handleFileChange}
          className="hidden"
        />
      </div>

      <div className="flex items-center gap-2">
        <select
          onChange={handleSampleSelect}
          defaultValue=""
          className="px-2.5 py-1.5 rounded text-xs font-mono bg-[#0d0d1a] text-[#888] border border-[#2a2a4a] hover:border-[#cc66ff]/30 focus:border-[#cc66ff]/50 focus:outline-none transition-all duration-200 cursor-pointer"
        >
          <option value="" disabled>选择示例...</option>
          {SAMPLE_TREES.map((s) => (
            <option key={s.name} value={s.name}>{s.label}</option>
          ))}
        </select>
      </div>

      <div className="w-px h-6 bg-[#2a2a4a]" />

      {tree && (
        <>
          <div className="flex items-center gap-1.5">
            <button
              onClick={stepOnce}
              disabled={isRunning}
              className="flex items-center gap-1 px-2.5 py-1.5 rounded text-xs font-mono text-[#00ccff] border border-[#00ccff]/20 hover:border-[#00ccff]/50 hover:bg-[#00ccff]/5 disabled:opacity-30 disabled:cursor-not-allowed transition-all duration-200"
              title="单步执行"
            >
              <span className="text-base leading-none">&#9654;|</span>
              单步
            </button>
            {isRunning ? (
              <button
                onClick={stopAuto}
                className="flex items-center gap-1 px-2.5 py-1.5 rounded text-xs font-mono text-[#ff4466] border border-[#ff4466]/20 hover:border-[#ff4466]/50 hover:bg-[#ff4466]/5 transition-all duration-200"
              >
                <span className="text-base leading-none">&#9632;</span>
                暂停
              </button>
            ) : (
              <button
                onClick={startAuto}
                className="flex items-center gap-1 px-2.5 py-1.5 rounded text-xs font-mono text-[#00ff88] border border-[#00ff88]/20 hover:border-[#00ff88]/50 hover:bg-[#00ff88]/5 transition-all duration-200"
              >
                <span className="text-base leading-none">&#9654;</span>
                播放
              </button>
            )}
            <button
              onClick={resetExecution}
              className="flex items-center gap-1 px-2.5 py-1.5 rounded text-xs font-mono text-[#888] border border-[#2a2a4a] hover:border-[#ffaa00]/40 hover:text-[#ffaa00] transition-all duration-200"
            >
              <span className="text-base leading-none">&#8634;</span>
              重置
            </button>
          </div>

          <div className="w-px h-6 bg-[#2a2a4a]" />

          <div className="flex items-center gap-2">
            <span className="text-[10px] font-mono text-[#555]">速度</span>
            <input
              type="range"
              min="100"
              max="2000"
              step="100"
              value={speed}
              onChange={(e) => setSpeed(Number(e.target.value))}
              className="w-20 h-1 accent-[#00ff88] cursor-pointer"
            />
            <span className="text-[10px] font-mono text-[#00ff88] w-10">{speed}ms</span>
          </div>
        </>
      )}

      <div className="flex-1" />

      {/* 拖拽提示 */}
      <div
        onDrop={handleDrop}
        onDragOver={handleDragOver}
        className="hidden xl:flex items-center gap-1.5 px-3 py-1.5 rounded text-[10px] font-mono text-[#444] border border-dashed border-[#2a2a4a]"
      >
        <FolderOpen size={12} />
        拖拽XML文件到此处
      </div>
    </div>
  );
}