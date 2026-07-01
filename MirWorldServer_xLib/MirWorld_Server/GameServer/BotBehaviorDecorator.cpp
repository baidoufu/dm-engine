#include "StdAfx.h"
#include "BotBehaviorDecorator.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotHumanBehavior.h"
#include "tinyxml.h"
#include "aliveobject.h"

// ============================================================================
// 装饰节点 - 反转器实现
// ============================================================================
BTResult CBTDecoratorInverter::Execute(CBotPlayer* pBot)
{
	if (!m_pChild)
		return BTR_FAILURE;

	BTResult result = m_pChild->Execute(pBot);
	switch (result)
	{
	case BTR_SUCCESS: return BTR_FAILURE;
	case BTR_FAILURE: return BTR_SUCCESS;
	default: return BTR_RUNNING;
	}
}

// ============================================================================
// 装饰节点 - 重复器实现
// ============================================================================
// 硬限制：防止 m_nMaxRepeat=0（无限）时子节点永不返回FAILURE导致死循环
// 正常使用下不应达到此限制，达到说明行为树配置有误
static const int MAX_REPEAT_HARD_LIMIT = 100;
BTResult CBTDecoratorRepeat::Execute(CBotPlayer* pBot)
{
	if (!m_pChild)
		return BTR_FAILURE;

	int nCount = 0;
	// 当 m_nMaxRepeat == 0 时使用硬限制替代，防止死循环阻塞整个线程
	int nEffectiveMax = (m_nMaxRepeat > 0) ? m_nMaxRepeat : MAX_REPEAT_HARD_LIMIT;
	while (nCount < nEffectiveMax)
	{
		BTResult result = m_pChild->Execute(pBot);
		if (result == BTR_FAILURE)
			return BTR_SUCCESS;  // 重复直到失败，失败时返回SUCCESS
		nCount++;
	}
	// 达到硬限制：子节点始终返回SUCCESS/RUNNING，行为树配置可能有误
	if (m_nMaxRepeat == 0)
	{
		LG2("行为树警告: DecoratorRepeat [%s] 无限重复达到硬限制 %d 次, "
			"子节点未返回FAILURE，强制退出避免死循环\n",
			m_szName.c_str(), MAX_REPEAT_HARD_LIMIT);
	}
	return BTR_SUCCESS;
}

// ============================================================================
// 装饰节点 - 超时控制：累计计时，子节点超过指定时间未完成则返回失败
// ============================================================================
BTResult CBTDecoratorTimeout::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;

	// 不启用超时：直接透传子节点结果
	if (m_dwTimeoutMs == 0)
		return m_pChild->Execute(pBot);

	// 首次执行：记录起始时间（使用上下文缓存的帧时间，避免重复系统调用）
	if (m_dwStartTime == 0)
		m_dwStartTime = pBot->GetContext()->GetFrameTime();

	// 检查是否超时（累计计时，跨多次 tick）
	DWORD dwElapsed = pBot->GetContext()->GetFrameTime() - m_dwStartTime;
	if (dwElapsed >= m_dwTimeoutMs)
	{
		m_dwStartTime = 0;
		return BTR_FAILURE;
	}

	// 未超时：执行子节点
	BTResult result = m_pChild->Execute(pBot);

	// 子节点结束（成功或失败）：重置计时器，下次重新开始计时
	if (result == BTR_SUCCESS || result == BTR_FAILURE)
		m_dwStartTime = 0;

	return result;
}

// ============================================================================
// 装饰节点 - 冷却控制
// ============================================================================
BTResult CBTDecoratorCooldown::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;
	DWORD dwNow = pBot->GetContext()->GetFrameTime();
	if (m_dwLastExecTime > 0 && (dwNow - m_dwLastExecTime) < m_dwCooldownMs)
		return BTR_FAILURE;
	m_dwLastExecTime = dwNow;
	return m_pChild->Execute(pBot);
}

// ============================================================================
// 装饰节点 - 强制成功
// ============================================================================
BTResult CBTDecoratorSucceeder::Execute(CBotPlayer* pBot)
{
	if (m_pChild) m_pChild->Execute(pBot);
	return BTR_SUCCESS;
}

// ============================================================================
// 装饰节点 - 强制失败
// ============================================================================
BTResult CBTDecoratorFailer::Execute(CBotPlayer* pBot)
{
	if (m_pChild) m_pChild->Execute(pBot);
	return BTR_FAILURE;
}

// ============================================================================
// 装饰节点 - 周期性触发器
// 每 m_dwPeriodMs 执行一次子节点并返回子节点真实结果
// 周期内返回 FAILURE，不阻塞 Selector 优先级链
// ============================================================================
BTResult CBTDecoratorPeriodic::Execute(CBotPlayer* pBot)
{
	if (!m_pChild) return BTR_FAILURE;
	if (m_dwPeriodMs == 0) return BTR_FAILURE;

	DWORD dwNow = pBot->GetContext()->GetFrameTime();
	DWORD dwElapsed = (m_dwLastExecTime > 0) ? (dwNow - m_dwLastExecTime) : m_dwPeriodMs;

	// 周期未到：返回 FAILURE，让上层 Selector 继续检查其他分支
	if (dwElapsed < m_dwPeriodMs)
		return BTR_FAILURE;

	// 周期已到：执行子节点并记录时间
	m_dwLastExecTime = dwNow;
	return m_pChild->Execute(pBot);
}
