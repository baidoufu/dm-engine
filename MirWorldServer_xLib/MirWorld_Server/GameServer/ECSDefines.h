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
 *  按组件类型的唯一 ID 生成器 (编译期确定, 通过静态局部变量实现)
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
