#pragma once

// 连击技能子技能定义(只读模板)
struct ConSubSkill
{
	WORD wMagicID;      // 子技能MagicID
	int  nBuffID;       // 关联Buff编号(-1=无)
};

// 连击技能模板(只读,全局共享)
struct ConSkillTemplate
{
	int  iConSkillID;                   // 连击编号(1-4)
	int  nCareer;                       // 职业(0战士/1法师/2道士)
	int  nMagicCount;                   // 子技能数量
	ConSubSkill wMagicIDs[3];           // 子技能列表(最多3个)

	ConSkillTemplate()
	{
		iConSkillID = 0;
		nCareer = 0;
		nMagicCount = 0;
		for (int i = 0; i < 3; i++)
		{
			wMagicIDs[i].wMagicID = 0;
			wMagicIDs[i].nBuffID = -1;
		}
	}
};

// 连击技能Buff模板(只读,全局共享)
struct ConSkillBuffTemplate
{
	int  nBuffID;           // Buff编号(0-4)
	int  nTotalTime;        // 总持续时间(毫秒)
	int  nIcon;             // 图标ID
};

// 玩家连击技能运行时状态
struct PlayerConSkillState
{
	int  m_iConSkillIndex[4];       // 每个连击的当前进度(0/1/2, 只记录该职业的连击)
	BOOL m_bActive[4];              // 每个连击是否激活(所有子技能>=大师级)
	BOOL m_bBuffActive[5];          // 每个Buff是否激活
	DWORD m_dwBuffStartTime[5];     // 每个Buff的开始时间
	WORD m_wBuffMagicID[5];         // 每个Buff关联的MagicID

	PlayerConSkillState()
	{
		memset(m_iConSkillIndex, 0, sizeof(m_iConSkillIndex));
		memset(m_bActive, 0, sizeof(m_bActive));
		memset(m_bBuffActive, 0, sizeof(m_bBuffActive));
		memset(m_dwBuffStartTime, 0, sizeof(m_dwBuffStartTime));
		memset(m_wBuffMagicID, 0, sizeof(m_wBuffMagicID));
	}

	void SetToFirst(BYTE btCareer)
	{
		for (int i = 0; i < 4; i++)
			m_iConSkillIndex[i] = 0;
	}

	int GetIndex(int iTemplateIdx)
	{
		if (iTemplateIdx >= 0 && iTemplateIdx < 4)
			return m_iConSkillIndex[iTemplateIdx];
		return 0;
	}

	void ChangeToNext(int iTemplateIdx, int nMagicCount)
	{
		if (iTemplateIdx >= 0 && iTemplateIdx < 4)
		{
			m_iConSkillIndex[iTemplateIdx]++;
			if (m_iConSkillIndex[iTemplateIdx] >= nMagicCount)
				m_iConSkillIndex[iTemplateIdx] = 0;
		}
	}
};

// ==================== 全局函数声明 ====================
class CHumanPlayer;

VOID InitConSkillSystem();
BOOL IsConSkillMagic(WORD wMagicID);
ConSkillTemplate* GetConSkillTemplate(int idx);
ConSkillBuffTemplate* GetConSkillBuffTemplate(int nBuffID);
VOID UpdateConSkillActive(CHumanPlayer* pPlayer);
VOID OnConSkillMagicSuccess(CHumanPlayer* pPlayer, WORD wMagicID);
VOID UpdateConSkillBuffs(CHumanPlayer* pPlayer);
VOID ClearAllConSkillBuffs(CHumanPlayer* pPlayer);