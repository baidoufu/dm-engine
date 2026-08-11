#include "magicdata.h"
#include "GameData/MagicDefine.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

#include <assert.h>

CMagicData::CMagicData(void)
{
	m_cShortCutKey	= 0;
	m_iMagicLevel	= 0;
	m_wSkillValue	= 0;
	m_wMagicID		= 0;
	m_MagicType		= 0;
	m_Eff			= 0;
	m_wSpell			= 0;
	m_wPower			= 0;

	for(int ii = 0;ii < 9;ii++)
	{
		m_NeedLevel[ii]  = 0;
		m_Train[ii]		= 0;
	}
	m_iJob			= 0;
	m_Def			= 0;
	m_DefPower		= 0;
	m_MaxPower		= 0;
	m_DefMaxPower	= 0;
	m_nDelay		= 0;
	m_color			= 0;

	m_dwAttr        = 0;
	m_color = 0xFFFFFFFF;
	m_bDisable = false;
	m_dwFlashTexID = 0;
}

CMagicData::~CMagicData(void)
{
}

DWORD CMagicData::GetNeedManaValue()
{
	if (m_iMagicLevel < 4)
	{
		return (DWORD)((((double)m_wSpell/4)*(double)(m_iMagicLevel + 1) + m_Def) + 0.5);
	}
	else
	{
		return (DWORD)(((double)m_wNewSpell / 4) * ((double)m_iMagicLevel + 1) + m_byNewDefSpell);
	}	
}

CMagicData::CMagicData(CMagicData & m)
{
	Assign(m);
}

void CMagicData::SetAttr(DWORD dwAttr)
{
	m_dwAttr = dwAttr;
}

void CMagicData::AddAttr(DWORD dwAttr)
{
	m_dwAttr |= dwAttr;
}

bool CMagicData::HasAttr(DWORD dwAttr)
{
	return (m_dwAttr & dwAttr) != 0;
}

void CMagicData::RemoveAttr(DWORD dwAttr)
{
	m_dwAttr &= ~dwAttr;
}

void CMagicData::UpdateAttr(DWORD dwAttr)
{
	if(HasAttr(dwAttr))
		RemoveAttr(dwAttr);
	else
		AddAttr(dwAttr);
}

void CMagicData::Assign(CMagicData& tmp)
{
	m_cShortCutKey	= tmp.m_cShortCutKey;
	m_iMagicLevel	= tmp.m_iMagicLevel;
	m_wSkillValue	= tmp.m_wSkillValue;
	m_wMagicID		= tmp.m_wMagicID;
	m_strName		= tmp.m_strName;
	m_MagicType		= tmp.m_MagicType;
	m_Eff			= tmp.m_Eff;
	m_wSpell			= tmp.m_wSpell;
	m_wPower			= tmp.m_wPower;

	for(int ii = 0;ii < 9;ii++)
	{
		m_NeedLevel[ii]  = tmp.m_NeedLevel[ii];
		m_Train[ii]		= tmp.m_Train[ii];
	}
	m_iJob			= tmp.m_iJob;
	m_Def			= tmp.m_Def;
	m_DefPower		= tmp.m_DefPower;
	m_MaxPower		= tmp.m_MaxPower;
	m_DefMaxPower	= tmp.m_DefMaxPower;
	m_nDelay		= tmp.m_nDelay;
	m_dwAttr        = tmp.m_dwAttr;
	m_dwShortCutKeyEx = tmp.m_dwShortCutKeyEx;
	m_color = tmp.m_color;
	m_bDisable = tmp.m_bDisable;
	m_dwFlashTexID = tmp.m_dwFlashTexID;
}

bool CMagicData::FromBuffer(const char* buf,int iLen)
{
	if(iLen < 75)
		return false;

	char name[32]={0};
	if(buf[10] > 12 || buf[10] <= 0)
		return false;

	memcpy(name,buf + 11,buf[10]);

	SetShortCutKey(buf[0]); // 快捷键索引
	SetMagicLevel(buf[1]); // 技能等级
	SetSkillValue(*((WORD*)(buf + 4))); // 技能值
	SetNewSpell(*((WORD*)(buf + 6))); // 当前新技能经验值
	SetMagicID(*((WORD*)(buf + 8))); // 技能编号
	SetMagicName(name); // 技能名称
	SetMagicType(buf[23]); // 效果类型
	SetEffect(buf[24]); // 效果标志
	SetNewDefSpell(buf[25]); // 新防御技能
	SetSpell(Conv_WORD(buf + 26)); // 施法值
	SetPower(Conv_WORD(buf + 28)); // 威力值

	for(int ii = 0;ii < 4;ii++)
	{
		SetNeedLevel(ii,buf[30+ii]); // 升级需要的等级
		SetTrain(ii,Conv_DWORD(buf+36+ii*4)); // 修炼经验值
	}
	SetJob(buf[53]); // 职业类型
	SetMagicDelay(Conv_WORD(buf + 56)); // CD时间
	SetDefence(buf[60]); // 防御值
	SetDefencePower(buf[61]); // 防御威力
	SetMaxPower(buf[62]); // 最大威力
	SetDefenceMaxPower(buf[64]); // 防御最大威力

	SetNeedLevel(4,buf[66]);
	SetNeedLevel(5,buf[67]);
	SetTrain(4,Conv_DWORD(buf + 68));
	SetTrain(5,Conv_DWORD(buf + 72));

	if(iLen > 76)
	{
		SetNeedLevel(6,buf[76]);
		SetNeedLevel(7,buf[77]);
		SetNeedLevel(8,buf[78]);
		SetTrain(6,Conv_DWORD(buf + 79));
		SetTrain(7,Conv_DWORD(buf + 83));
		SetTrain(8,Conv_DWORD(buf + 87));
	}
	DefaultAttr();
	m_bDisable = false;
	m_dwFlashTexID = 0;

	return true;
}

void CMagicData::DefaultAttr()
{
	m_dwAttr = 0;
	if(m_wMagicID == MAGICID_FIREBALL
		|| m_wMagicID == MAGICID_SUPER_FIREBALL
		|| m_wMagicID == MAGICID_ADV_FIREBALL
		|| m_wMagicID == MAGICID_POISON_MAGIC
		|| m_wMagicID == MAGICID_SUPER_POISON_MAGIC
		|| m_wMagicID == MAGICID_THUNDER
		|| m_wMagicID == MAGICID_PROTECT_SYMBOL
		|| m_wMagicID == MAGICID_HELL_FIRE
		|| m_wMagicID == MAGICID_TRACE_THUNDER
		|| m_wMagicID == MAGICID_LURE_LIGHT
		|| m_wMagicID == MAGICID_BLOW_FIRE
		|| m_wMagicID == MAGICID_GOD_SAY
		|| m_wMagicID == MAGICID_SOUL_WALL
		|| m_wMagicID == MAGICID_ICE_ARROW
		|| m_wMagicID == MAGICID_ICE_DRAG
		|| m_wMagicID == MAGICID_CURSE
		|| m_wMagicID == MAGICID_SUPER_CURSE   //强化诅咒术
		|| m_wMagicID == MAGICID_DRAGON_LIGHT
		|| m_wMagicID == MAGICID_FIRE_JUJU
		|| m_wMagicID == MAGICID_ICE_STORM     // 冰咆哮
		|| m_wMagicID == MAGICID_MULTI_THUDER     // 五雷轰
		|| m_wMagicID == MAGICID_ICE_WHIRLWIND     // 冰旋风
		|| m_wMagicID == MAGICID_DESTROY_POISON     // 解毒
		|| m_wMagicID == MAGICID_GODLIGHT_SKILL
		|| m_wMagicID == MAGICID_GRASP_DRAGON
		|| m_wMagicID == MAGICID_FIRE_RAIN
		|| m_wMagicID == MAGICID_SHADOWKILL8//八方分影斩
		|| m_wMagicID == MAGICID_FIRE_SHIELD_ATTACK//强袭烈焰盾攻击
		|| m_wMagicID == MAGICID_JUDUZHAOZE //剧毒沼泽
		)     // 神光
		m_dwAttr |= MATTR_LOCK;

	if(m_wMagicID == MAGICID_FIREBALL
		|| m_wMagicID == MAGICID_SUPER_FIREBALL
		|| m_wMagicID == MAGICID_THUNDER
		|| m_wMagicID == MAGICID_FIRE_JUJU
		|| m_wMagicID == MAGICID_DRAGON_LIGHT
		|| m_wMagicID == MAGICID_PROTECT_SYMBOL
		|| m_wMagicID == MAGICID_MULTI_THUDER
		|| m_wMagicID == MAGICID_ICE_STORM
		|| m_wMagicID == MAGICID_FIRE_RAIN
		)
		m_dwAttr |= MATTR_REPEAT;

	//需要吟唱的魔法
	if (m_wMagicID == MAGICID_COLDSTROM)
	{
		m_dwAttr |= MATTR_SINGING;
	}
}

//技能数组
CMagicDataArray::CMagicDataArray(int iSize)
{
	m_iSize = iSize;
	if(m_iSize > 0)
	{
		m_MagicArray = new CMagicData[m_iSize];
	}
}

CMagicDataArray::~CMagicDataArray()
{
	SAFE_DELETE_ARRAY(m_MagicArray);
}

void CMagicDataArray::Clear()
{
	for(int i = 0;i < m_iSize;i++)
	{
		m_MagicArray[i].SetMagicID(0);
	}
}

CMagicData& CMagicDataArray::Get(int i)
{
	//assert(i >= 0 && i < m_iSize);
	if(i < 0 || i >= m_iSize)
	{
		char buf[128];
		sprintf(buf,"MagicDataArray越界:%d",i); 
		throw exception(buf);
	}

	return m_MagicArray[i];
}

CMagicData* CMagicDataArray::Add(const char* buf,int iLen)
{
	for(int i = 0;i < m_iSize;i++)
	{
		if(m_MagicArray[i].GetMagicID() == 0)
		{
			m_MagicArray[i].FromBuffer(buf,iLen);
			return &(m_MagicArray[i]);
		}
	}
	return NULL;
}

int CMagicDataArray::FindMagicPos(WORD wMagicID)
{
	for(int ii = 0; ii < m_iSize;ii++)
	{
		if(m_MagicArray[ii].GetMagicID() == wMagicID)
			return ii;
	}
	return -1;
}

CMagicData* CMagicDataArray::FindMagic(WORD wMagicID)
{
	int pos = FindMagicPos(wMagicID);
	if(pos < 0)
		return NULL;

	return &m_MagicArray[pos];
}

CMagicData* CMagicDataArray::FindMagicByName(const char *szMagicName)
{
	for(int i = 0; i < m_iSize;i++)
	{
		if(strcmp(m_MagicArray[i].GetMagicName(),szMagicName) == 0)
			return &m_MagicArray[i];
	}

	return NULL;
}

bool CMagicDataArray::DeleteMagic(WORD wMagicID)
{
	if(wMagicID == 0)
		return false;

	for(int ii = 0; ii < m_iSize;ii++)
	{
		if(m_MagicArray[ii].GetMagicID() == wMagicID)
		{
			for(int j = ii+1; j < m_iSize;j++)
			{
				m_MagicArray[j-1] = m_MagicArray[j];
			}
			m_MagicArray[m_iSize-1].SetMagicID(0);
			return true;
		}
	}

	return false;
}

WORD CMagicDataArray::FindByShortcut(char cKey)
{
	if(cKey == 0)
		return 0;

	for(int ii = 0; ii < m_iSize;ii++)
	{
		WORD id = m_MagicArray[ii].GetMagicID();
		if(id == 0)
			continue;

		if(m_MagicArray[ii].GetShortCutKey() == cKey)
			return id;
	}
	return 0;
}

CMagicData* CMagicDataArray::FindMagicDataByShortcut(char cKey)
{
	if(cKey == 0)
		return NULL;

	for(int ii = 0; ii < m_iSize;ii++)
	{
		WORD id = m_MagicArray[ii].GetMagicID();
		if(id == 0)
			continue;

		if(m_MagicArray[ii].GetShortCutKey() == cKey)
			return &m_MagicArray[ii];
	}
	return NULL;
}

WORD CMagicDataArray::FindByShortcutEx(DWORD dwKey)
{
	if(dwKey == 0)
		return 0;

	for(int ii = 0; ii < m_iSize;ii++)
	{
		WORD id = m_MagicArray[ii].GetMagicID();
		if(id == 0)
			continue;

		if(m_MagicArray[ii].GetShortCutKeyEx() == dwKey)
			return id;
	}
	return 0;
}

CMagicData* CMagicDataArray::FindMagicDataByShortcutEx(DWORD dwKey)
{
	if(dwKey == 0)
		return NULL;

	for(int ii = 0; ii < m_iSize;ii++)
	{
		WORD id = m_MagicArray[ii].GetMagicID();
		if(id == 0)
			continue;

		if(m_MagicArray[ii].GetShortCutKeyEx() == dwKey)
			return &m_MagicArray[ii];
	}
	return NULL;
}

int CMagicDataArray::GetMagicNumber()
{
	int iCount = 0;
	for(int ii = 0; ii < m_iSize;ii++)
	{
		if(m_MagicArray[ii].GetMagicID() != 0)
			iCount++;
	}
	return iCount;
}



