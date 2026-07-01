#pragma once

#include "ECSWorld.h"
#include <vector>
#include <memory>
#include <algorithm>

/**
 *  ECSSystem —— 基类
 *
 *  设计目标:
 *    将 OOP Update() 中的逐帧逻辑抽取为独立的 ECS System,
 *    使用 ecs_view 批量遍历替代 per-entity 虚函数调用,
 *    享受 SoA 连续内存 + 缓存友好遍历。
 *
 *  桥接原则:
 *    - 系统只操作组件 (ECS 侧), 不写回 OOP
 *    - 与 OOP 的交互通过组件内 ownerId 字段回调
 *    - 按优先级排序执行, 数值越小越先执行
 *
 *  使用示例:
 *    class HpRecoverSystem : public ECSSystem {
 *        size_t Execute(int frameTime) override { ... }
 *        const char* Name() const override { return "HpRecover"; }
 *    };
 *    SystemRunner::GetInstance()->Register(std::make_unique<HpRecoverSystem>());
 */
class ECSSystem
{
public:
	virtual ~ECSSystem() = default;

	/**
	 *  每帧执行
	 *  @param frameTime  CFrameTime::GetFrameTime()
	 *  @return  本帧处理的实体数量 (用于性能分析)
	 */
	virtual size_t Execute(int frameTime) = 0;

	/**
	 *  系统名称 (用于日志/性能分析)
	 */
	virtual const char* Name() const = 0;

	/**
	 *  执行优先级 (数值越小越先执行)
	 *  默认 0 = 普通优先级
	 */
	virtual int Priority() const { return 0; }
};

/**
 *  SystemRunner —— 系统注册与执行调度
 *
 *  职责:
 *    - 注册系统 (按 Priority 排序)
 *    - 每帧按顺序执行所有系统
 *    - 提供类型安全获取指定系统实例
 *
 *  注册时机:
 *    在所有 ECS 组件初始化 (InitPoolCache) 完成之后,
 *    首次 BatchPrecompute 之前注册首批系统。
 */
class SystemRunner : public xSingletonClass<SystemRunner>
{
public:
	SystemRunner()  = default;
	~SystemRunner() = default;

	/**
	 *  注册系统 (按 Priority 排序插入)
	 *  线程安全: 仅在服务器初始化阶段调用 (首帧循环前)
	 */
	void Register(std::unique_ptr<ECSSystem> sys)
	{
		if (!sys) return;
		auto it = std::lower_bound(m_systems.begin(), m_systems.end(), sys,
			[](const std::unique_ptr<ECSSystem>& a, const std::unique_ptr<ECSSystem>& b) {
				return a->Priority() < b->Priority();
			});
		m_systems.insert(it, std::move(sys));
	}

	/**
	 *  按顺序执行所有系统
	 *  线程安全: 仅在主线程调用 (UpdatePlayers 执行前)
	 */
	void ExecuteAll(int frameTime)
	{
		for (auto& sys : m_systems)
		{
			size_t count = sys->Execute(frameTime);
			// TODO: 可收集系统执行统计 (实体数/耗时)
			(void)count;
		}
	}

	/**
	 *  获取指定类型系统 (用于调试/热更新)
	 */
	template<typename T>
	T* Get()
	{
		for (auto& sys : m_systems)
		{
			T* p = dynamic_cast<T*>(sys.get());
			if (p) return p;
		}
		return nullptr;
	}

	/**
	 *  已注册系统数量
	 */
	size_t Count() const { return m_systems.size(); }

private:
	// 按 Priority 排序的系统列表
	std::vector<std::unique_ptr<ECSSystem>> m_systems;
};
