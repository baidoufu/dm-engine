#pragma once

#include "ECS.h"

/**
 *  ECSWorld — 共享 ECS 注册表单例
 *
 *  职责: 持有唯一的 ECSRegistry，供所有对象组件 Manager 共享访问。
 *  通过 GetWorld() 获取引用，实现在同一世界中的组件协同。
 */
class ECSWorld : public xSingletonClass<ECSWorld>
{
public:
	ECSWorld()  = default;
	~ECSWorld() = default;
	// 获取ECSRegistry对象
	ECSRegistry& GetWorld() { return m_world; }
private:
	ECSRegistry m_world;
};
