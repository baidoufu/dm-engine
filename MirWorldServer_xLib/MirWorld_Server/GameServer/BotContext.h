#pragma once
#include "MapObject.h"

// 前置声明
class CBotPlayer;
class CAliveObject;
class CLogicMap;

// ============================================================================
// 机器人视野范围常量
// ============================================================================
const int BOT_VIEW_RANGE = 16;

// ============================================================================
// 决策上下文 - 为行为树节点提供查询接口
// 替代CBotAIController的查询功能，解耦决策与执行
// ============================================================================
class CBotContext
{
public:
	CBotContext() : m_pBot(nullptr)
	{
		m_dwTargetUpdateInterval = 0;
		m_dwTargetUpdateTime = 0;
		m_dwFrameTime = 0;
	}
	~CBotContext() {}

	// 绑定机器人
	VOID Bind(CBotPlayer* pBot) { m_pBot = pBot; }

	// 帧时间缓存
	VOID SetFrameTime(DWORD dwTime) { m_dwFrameTime = dwTime; }
	DWORD GetFrameTime() const { return m_dwFrameTime; }

	// 目标查询
	CAliveObject* FindNearestMonster(int nRange = BOT_VIEW_RANGE);
	CAliveObject* GetCachedTarget();
	BOOL IsTargetValid(CAliveObject* pTarget);

	// 预计算：在 BT 执行前刷新目标缓存，避免重型搜索进入 BT 热路径
	VOID PrecomputeTarget();

	// 自身状态查询
	int GetHpPercent();
	int GetMpPercent();
	BOOL IsDead();
	BOOL InSafeArea();

	// 物品查询
	BOOL HasPotionInBag();
	DWORD FindPotionInBag(BOOL bHP);
	DWORD FindItemInBag(const char* pszName);

	// 技能是否准备好
	BOOL IsSkillReady(WORD wSkillId);

	// 距离/方向计算
	int DistanceTo(CAliveObject* pTarget);
	int DirectionTo(CAliveObject* pTarget);

	// 地图查询
	UINT GetCurrentMapId();
	BOOL IsWalkable(int x, int y);

	// 获取绑定机器人
	CBotPlayer* GetBot() { return m_pBot; }

private:
	CBotPlayer* m_pBot;				// 绑定的机器人
	CRefObject m_refCachedTarget;	// 缓存的目标引用
	DWORD m_dwTargetUpdateTime;		// 目标更新时间
	DWORD m_dwTargetUpdateInterval;	// 目标更新间隔(ms)
	DWORD m_dwFrameTime;			// 当前帧时间缓存
};
