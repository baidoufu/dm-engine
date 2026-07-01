#pragma once

// 保持向后兼容的类型别名: 任务ID → 任务索引映射
using TaskIdMap = SmallFlatMap<WORD, int, MAX_TASK>;

/**
 *  TaskComponent — 玩家任务数据
 *  纯数据组件, 任务逻辑由 Human_Task.cpp / TaskSystem 管理
 */
struct TaskComponent
{
	TASKINFO		TaskInfo;         // 任务信息
	TaskIdMap		TaskIdToIndexMap; // 任务ID→索引映射 (栈存储, 无堆分配)

	TaskComponent()
	{
		memset(&TaskInfo, 0, sizeof(TaskInfo));
	}
};
