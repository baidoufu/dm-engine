#include "StdAfx.h"
#include "ConSkill.h"
#include "HumanPlayer.h"
#include "gameworld.h"

// 4套预定义连招模板(只读,全局共享)
static ConSkillTemplate g_ConSkillTemplates[4];
// 5个Buff模板(只读,全局共享)
static ConSkillBuffTemplate g_ConSkillBuffTemplates[5];

// 连击技能涉及的所有子技能MagicID
static WORD g_ConSkillMagicIDs[] = {
	1,   // MAGICID_FIREBALL (小火球)
	5,   // MAGICID_ADV_FIREBALL (火炎刀)
	23,  // MAGICID_BLOW_FIRE (爆裂火焰)
	42,  // MAGICID_PROTECT_SKIN (护身真气)
	27,  // MAGICID_WILD_COLLIDE (野蛮冲撞)
	13,  // MAGICID_PROTECT_SYMBOL (灵魂道符)
	6,   // MAGICID_POISON_MAGIC (施毒术)
	45,  // MAGICID_CURSE (诅咒术)
};

static const int g_nConSkillMagicIDCount = sizeof(g_ConSkillMagicIDs) / sizeof(g_ConSkillMagicIDs[0]);

VOID InitConSkillSystem()
{
	// 初始化5个Buff模板
	memset(g_ConSkillBuffTemplates, 0, sizeof(g_ConSkillBuffTemplates));

	g_ConSkillBuffTemplates[0].nBuffID = 0;
	g_ConSkillBuffTemplates[0].nTotalTime = 6000;
	g_ConSkillBuffTemplates[0].nIcon = 308;

	g_ConSkillBuffTemplates[1].nBuffID = 1;
	g_ConSkillBuffTemplates[1].nTotalTime = 6000;
	g_ConSkillBuffTemplates[1].nIcon = 344;

	g_ConSkillBuffTemplates[2].nBuffID = 2;
	g_ConSkillBuffTemplates[2].nTotalTime = 6000;
	g_ConSkillBuffTemplates[2].nIcon = 352;

	g_ConSkillBuffTemplates[3].nBuffID = 3;
	g_ConSkillBuffTemplates[3].nTotalTime = 6000;
	g_ConSkillBuffTemplates[3].nIcon = 310;

	g_ConSkillBuffTemplates[4].nBuffID = 4;
	g_ConSkillBuffTemplates[4].nTotalTime = 6000;
	g_ConSkillBuffTemplates[4].nIcon = 388;

	// 连击1: 法师(1) - 小火球→火炎刀→爆裂火焰
	{
		ConSkillTemplate& sk = g_ConSkillTemplates[0];
		sk.iConSkillID = 1;
		sk.nCareer = JOB_MAG;
		sk.nMagicCount = 3;
		sk.wMagicIDs[0].wMagicID = 1;    // 小火球
		sk.wMagicIDs[0].nBuffID = -1;
		sk.wMagicIDs[1].wMagicID = 5;    // 火炎刀
		sk.wMagicIDs[1].nBuffID = 0;     // Buff0: 火炎刀伤害提升
		sk.wMagicIDs[2].wMagicID = 23;   // 爆裂火焰
		sk.wMagicIDs[2].nBuffID = 1;     // Buff1: 爆裂火焰100%暴击
	}

	// 连击2: 战士(0) - 护身真气→野蛮冲撞
	{
		ConSkillTemplate& sk = g_ConSkillTemplates[1];
		sk.iConSkillID = 2;
		sk.nCareer = JOB_WAR;
		sk.nMagicCount = 2;
		sk.wMagicIDs[0].wMagicID = 42;   // 护身真气
		sk.wMagicIDs[0].nBuffID = -1;
		sk.wMagicIDs[1].wMagicID = 27;   // 野蛮冲撞
		sk.wMagicIDs[1].nBuffID = 2;     // Buff2: 野蛮冲撞强化(回血)
	}

	// 连击3: 道士(2) - 灵魂道符→施毒术
	{
		ConSkillTemplate& sk = g_ConSkillTemplates[2];
		sk.iConSkillID = 3;
		sk.nCareer = JOB_TAO;
		sk.nMagicCount = 2;
		sk.wMagicIDs[0].wMagicID = 13;   // 灵魂道符
		sk.wMagicIDs[0].nBuffID = -1;
		sk.wMagicIDs[1].wMagicID = 6;    // 施毒术
		sk.wMagicIDs[1].nBuffID = 3;     // Buff3: 毒爆(施毒术)
	}

	// 连击4: 道士(2) - 灵魂道符→诅咒术
	{
		ConSkillTemplate& sk = g_ConSkillTemplates[3];
		sk.iConSkillID = 4;
		sk.nCareer = JOB_TAO;
		sk.nMagicCount = 2;
		sk.wMagicIDs[0].wMagicID = 13;   // 灵魂道符
		sk.wMagicIDs[0].nBuffID = -1;
		sk.wMagicIDs[1].wMagicID = 45;   // 诅咒术
		sk.wMagicIDs[1].nBuffID = 4;     // Buff4: 毒爆(诅咒术)
	}
}

BOOL IsConSkillMagic(WORD wMagicID)
{
	for (int i = 0; i < g_nConSkillMagicIDCount; i++)
	{
		if (g_ConSkillMagicIDs[i] == wMagicID)
			return TRUE;
	}
	return FALSE;
}

ConSkillTemplate* GetConSkillTemplate(int idx)
{
	if (idx < 0 || idx >= 4)
		return nullptr;
	return &g_ConSkillTemplates[idx];
}

ConSkillBuffTemplate* GetConSkillBuffTemplate(int nBuffID)
{
	if (nBuffID < 0 || nBuffID >= 5)
		return nullptr;
	return &g_ConSkillBuffTemplates[nBuffID];
}

// 根据MagicID和职业查找连击模板索引
static int FindConSkillTemplateIndex(BYTE btCareer, WORD wMagicID, PlayerConSkillState& state)
{
	for (int i = 0; i < 4; i++)
	{
		ConSkillTemplate& tmpl = g_ConSkillTemplates[i];
		if (tmpl.nCareer != btCareer)
			continue;
		if (!state.m_bActive[i])
			continue;

		int idx = state.GetIndex(i);
		if (idx >= 0 && idx < tmpl.nMagicCount)
		{
			if (tmpl.wMagicIDs[idx].wMagicID == wMagicID)
				return i;
		}
	}
	return -1;
}

// 检查并更新玩家连击技能激活状态(所有子技能>=大师级)
VOID UpdateConSkillActive(CHumanPlayer* pPlayer)
{
	PlayerConSkillState& state = pPlayer->GetConSkillState();
	BYTE btCareer = pPlayer->GetPro();

	for (int i = 0; i < 4; i++)
	{
		ConSkillTemplate& tmpl = g_ConSkillTemplates[i];
		if (tmpl.nCareer != btCareer)
			continue;

		state.m_bActive[i] = TRUE;
		for (int j = 0; j < tmpl.nMagicCount; j++)
		{
			USERMAGIC* pMagic = pPlayer->GetMagic(tmpl.wMagicIDs[j].wMagicID);
			// 连招中所有子技能等级大于等于 4（大师级）。
			// 客户端代码位置：AIMgr.cpp 1464 if (pMagic == 0 || pMagic->GetMagicLevel() < 4)
			if (pMagic == nullptr || pMagic->magic.btLevel < 4)  
			{
				state.m_bActive[i] = FALSE;
				break;
			}
		}
	}
}

// 玩家使用连击子技能成功后，推进进度并激活Buff
VOID OnConSkillMagicSuccess(CHumanPlayer* pPlayer, WORD wMagicID)
{
	BYTE btCareer = pPlayer->GetPro();
	PlayerConSkillState& state = pPlayer->GetConSkillState();

	int iTemplateIdx = FindConSkillTemplateIndex(btCareer, wMagicID, state);
	if (iTemplateIdx < 0)
		return;

	ConSkillTemplate& tmpl = g_ConSkillTemplates[iTemplateIdx];

	// 推进到下一个子技能
	state.ChangeToNext(iTemplateIdx, tmpl.nMagicCount);

	// 获取下一个子技能的Buff
	int idx = state.GetIndex(iTemplateIdx);
	if (idx >= 0 && idx < tmpl.nMagicCount)
	{
		int nBuffID = tmpl.wMagicIDs[idx].nBuffID;
		if (nBuffID >= 0)
		{
			state.m_bBuffActive[nBuffID] = TRUE;
			state.m_dwBuffStartTime[nBuffID] = CFrameTime::GetFrameTime();
			state.m_wBuffMagicID[nBuffID] = wMagicID;
		}
	}

	// 发送 SC_CONSKILLBUFF 给客户端 (iTime=6秒)
	pPlayer->SendMsg(wMagicID, SM_CONSKILLBUFF, 6, 0, 0);
}

// 更新所有Buff超时检测(在玩家Update中调用)
VOID UpdateConSkillBuffs(CHumanPlayer* pPlayer)
{
	DWORD dwNow = CFrameTime::GetFrameTime();
	PlayerConSkillState& state = pPlayer->GetConSkillState();

	for (int i = 0; i < 5; i++)
	{
		if (!state.m_bBuffActive[i])
			continue;

		ConSkillBuffTemplate* pBuffTmpl = GetConSkillBuffTemplate(i);
		if (pBuffTmpl == nullptr)
			continue;

		if (dwNow - state.m_dwBuffStartTime[i] >= (DWORD)pBuffTmpl->nTotalTime)
		{
			// Buff超时，发送取消通知 (iTime=0)
			pPlayer->SendMsg(state.m_wBuffMagicID[i], SM_CONSKILLBUFF, 0, 0, 0);

			// 重置所有连击进度到第一招
			state.SetToFirst(pPlayer->GetPro());

			// 清除所有Buff
			for (int j = 0; j < 5; j++)
			{
				state.m_bBuffActive[j] = FALSE;
				state.m_dwBuffStartTime[j] = 0;
			}
			break;
		}
	}
}

// 清除玩家所有连击Buff(死亡/下线/切换地图时调用)
VOID ClearAllConSkillBuffs(CHumanPlayer* pPlayer)
{
	PlayerConSkillState& state = pPlayer->GetConSkillState();

	for (int i = 0; i < 5; i++)
	{
		if (state.m_bBuffActive[i])
		{
			// 发送取消通知
			pPlayer->SendMsg(state.m_wBuffMagicID[i], SM_CONSKILLBUFF, 0, 0, 0);
		}
		state.m_bBuffActive[i] = FALSE;
		state.m_dwBuffStartTime[i] = 0;
	}

	// 所有连击进度归零
	state.SetToFirst(pPlayer->GetPro());
}