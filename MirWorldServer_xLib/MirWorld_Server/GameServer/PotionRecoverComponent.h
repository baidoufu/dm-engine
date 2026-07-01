#pragma once
#include "ECS.h"

/**
 *  PotionRecoverComponent —— 吃药恢复 ECS 组件
 *
 *  替代 CAliveObject 上分散的 6 个 OOP 字段:
 *    m_dwAddHp / m_dwAddHpSpeed / m_AddHpTimer
 *    m_dwAddMp / m_dwAddMpSpeed / m_AddMpTimer
 *
 *  生命周期:
 *    - 实体创建时自动挂载 (CreateAliveComponents)
 *    - addHp 和 addMp 均为 0 时 PotionRecoverSystem 跳过 (零开销)
 *    - OOP 侧 SetAddHp/SetAddMp 直接写入此组件
 */
struct PotionRecoverComponent
{
	DWORD addHp        = 0;   // 待递增 HP 总量
	DWORD addHpSpeed   = 0;   // 每次递增 HP 量
	int   lastHpTickMs = 0;   // 上次 HP 递增时间戳 (替代 CServerTimer)
	DWORD addMp        = 0;   // 待递增 MP 总量
	DWORD addMpSpeed   = 0;   // 每次递增 MP 量
	int   lastMpTickMs = 0;   // 上次 MP 递增时间戳 (替代 CServerTimer)
	UINT  ownerId      = 0;   // 反向引用 OOP 对象

	PotionRecoverComponent() = default;
};
