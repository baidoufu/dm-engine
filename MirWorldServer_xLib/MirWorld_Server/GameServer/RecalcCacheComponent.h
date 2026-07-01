#pragma once

/**
 *  RecalcCacheComponent — 被动技能重算缓存 (B类)
 *
 *  替代 CHumanPlayer 中的 m_nRecalcHit / m_nRecalcSpeed
 *  仅 RecalcHitSpeed() 使用, 纯临时缓存
 */
struct RecalcCacheComponent
{
	int		RecalcHit	= 0;	// 被动技能增加命中值缓存
	int		RecalcSpeed	= 0;	// 被动技能增加躲避值缓存

	RecalcCacheComponent() = default;
};
