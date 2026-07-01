#pragma once
#include <cstdint>

/**
 *  StaminaComponent —— 精力/活力组件
 *
 *  替代 CHumanPlayer 中的散落成员:
 *    m_wStamina     —— 当前精力值
 *    m_wMaxStamina  —— 最大精力值
 *    m_iHuoli       —— 活力值
 *
 *  逻辑闭环:
 *    精力恢复由 TimerComponent 的 TMR_STAMINA 定时器驱动,
 *    恢复逻辑(自增/封顶/同步客户端)通过 PlayerComponentManager 桥接访问本组件。
 *
 *  初始值:
 *    wStamina=0, wMaxStamina=2000, iHuoli=0
 *    (与原 CHumanPlayer::Clean 初始化一致)
 */
struct StaminaComponent
{
	uint16_t wStamina    = 0;    // 当前精力值
	uint16_t wMaxStamina = 2000; // 最大精力值
	int      iHuoli      = 0;    // 活力值
};
