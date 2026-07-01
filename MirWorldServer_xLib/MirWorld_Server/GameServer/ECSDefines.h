#pragma once

#include <cstdint>
#include <type_traits>

/**
 *  ECS 实体类型
 *  低20位 = 索引, 高12位 = 世代 (防止悬空句柄)
 *  最多同时存活 1,048,576 个实体; 世代在复用 4096 次后回绕
 */
using entity_t   = uint32_t;
using entity_idx = uint32_t;
using entity_gen = uint16_t;

constexpr entity_idx ENTITY_INDEX_BITS      = 20;
constexpr entity_idx ENTITY_INDEX_MASK      = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr entity_idx ENTITY_GENERATION_BITS = 12;
constexpr entity_idx ENTITY_GENERATION_MASK = ((1u << ENTITY_GENERATION_BITS) - 1u) << ENTITY_INDEX_BITS;
constexpr entity_idx INVALID_ENTITY         = 0xFFFFFFFFu;
constexpr entity_idx MAX_ENTITIES           = ENTITY_INDEX_MASK + 1u;  // 1,048,576

inline entity_idx entity_to_index(entity_t e)  { return e & ENTITY_INDEX_MASK; }
inline entity_gen entity_to_gen(entity_t e)     { return static_cast<entity_gen>((e & ENTITY_GENERATION_MASK) >> ENTITY_INDEX_BITS); }
inline entity_t   make_entity(entity_idx idx, entity_gen gen) { return (static_cast<entity_t>(gen) << ENTITY_INDEX_BITS) | (idx & ENTITY_INDEX_MASK); }

/**
 *  NullComponent 占位符 —— 用于 View 类型列表中的占位填充
 */
struct NullComponent {};

/**
 *  按组件类型的唯一 ID 生成器 (运行期确定, 通过静态局部变量实现)
 *
 *  注意 (跨 DLL 风险):
 *    component_type_counter() 使用函数内 static 变量, 每个 DLL/EXE 模块有独立副本。
 *    若 ECS 跨模块边界使用 (如 GameServer.exe + 插件 DLL), 同一类型 T 在不同模块
 *    会得到不同的 type_id, 导致 ECSRegistry::pools_ 索引错乱 → 类型擦除池损坏。
 *
 *  当前约束:
 *    ECS 仅限单模块内使用 (GameServer.exe 内部), 不跨 DLL 边界。
 *    若未来需跨模块, 应改用显式注册的 type_id 方案 (如全局注册表 + 名字映射),
 *    或确保所有组件类型在单一公共头文件中定义并由主模块统一实例化 type_id。
 */
namespace ecs_detail {
	inline uint32_t& component_type_counter() {
		static uint32_t counter = 0;
		return counter;
	}

	template<typename T>
	inline uint32_t component_type_id() {
		static const uint32_t id = component_type_counter()++;
		return id;
	}
}

template<typename T>
inline uint32_t get_component_type_id() {
	return ecs_detail::component_type_id<std::decay_t<T>>();
}
