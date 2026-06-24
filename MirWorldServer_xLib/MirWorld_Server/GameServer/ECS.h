#pragma once

/**
 *  ECS —— 高性能实体组件系统
 *
 *  架构:
 *    ECSDefines.h      —— entity_t 基础类型、世代/索引编解码、type_id 生成器
 *    ECSComponentPool.h —— 稀疏集组件存储：稠密数组 + 稀疏映射，全 O(1) 操作
 *    ECSRegistry.h     —— 中心世界：实体生命周期、组件增删改查、池管理
 *    ECSView.h         —— 编译期视图：零开销遍历，多组件联合查询
 *
 *  性能特征:
 *    - 同类型组件存储于连续内存 (SoA)，迭代时缓存命中率极高
 *    - add / remove 均为 O(1)，采用 swap-with-last 策略
 *    - has / get 均为 O(1)，通过稀疏数组直接索引
 *    - View 使用编译期模板展开，无虚函数、无类型擦除开销
 *    - entity_t 内嵌世代号，实体销毁后旧句柄自动失效
 *
 *  快速开始:
 *    #include "ECS.h"
 *
 *    struct Position { float x, y; };
 *    struct Velocity { float dx, dy; };
 *
 *    ECSRegistry world;
 *
 *    entity_t e = world.create();
 *    world.emplace<Position>(e, 0.0f, 0.0f);
 *    world.emplace<Velocity>(e, 1.0f, 0.0f);
 *
 *    // 每帧更新
 *    ecs_view<Position, Velocity>(world).each([](Position& p, Velocity& v) {
 *        p.x += v.dx;
 *        p.y += v.dy;
 *    });
 *
 *  依赖:
 *    C++17，仅依赖 STL (<vector>, <tuple>, <memory>, <type_traits>)
 */

#include "ECSDefines.h"
#include "ECSComponentPool.h"
#include "ECSRegistry.h"
#include "ECSView.h"
