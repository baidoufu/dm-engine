#pragma once

#include "ECS.h"
#include <array>

/**
 *  StatusComponent —— 状态/系统标志 ECS 组件
 *
 *  每帧由 BatchPrecomputeStatusExpire 预计算过期位,
 *  供 StatusExpireSystem / OOP Update 消费。
 *
 *  数据布局:
 *    statusExpiredMask[31:0]      — m_Status 中本帧到期的位
 *    systemFlagExpiredMask[31:0]  — m_SystemFlag 中本帧到期的位
 *    lastTickMs[64]               — 各槽位最后更新时间戳
 *    durationMs[64]               — 各槽位持续时间 (0xFFFFFFFF=永久)
 *      [0-31]  = m_Status 槽位
 *      [32-63] = m_SystemFlag 槽位
 */

struct StatusComponent
{
	uint32_t statusExpiredMask      = 0;  // m_Status 过期位 (BatchPrecompute写入, InterlockedAnd消费)
	uint32_t systemFlagExpiredMask  = 0;  // m_SystemFlag 过期位
	uint64_t activeSlotMask         = 0;  // 已激活槽位掩码 (BatchPrecompute通过此掩码跳过空闲槽)
	int      lastTickMs[64]         {};   // 上次设置时间戳 (毫秒)
	DWORD    durationMs[64]         {};   // 持续时间 (0xFFFFFFFF = 永久)
	UINT     ownerId                = 0;  // 反向引用: ECS 实体 → OOP 对象

	StatusComponent() = default;
};

// 状态槽位索引 → 组件内数组索引
inline int StatusSlotToCompIdx(int index) { return index; }             // [0-31]
inline int SystemFlagSlotToCompIdx(int index) { return index + 32; }    // [32-63]
