#pragma once

#include "global\global.h"

#define KEY_BASE        0x30

#define VK2KEY(a)		(a-VK_F1+1+KEY_BASE)		//键盘扫描码到快捷键：F1 到 1
#define KEY2IDX(a)		(a-1-KEY_BASE)				//KEY到索引：1 到 0
#define IDX2KEY(a)		(a+1+KEY_BASE)				//索引到KEY：0 到 1 
#define KEY2DIGITAL(a)	(a-KEY_BASE)				//显示数字： 1 到 1

enum eMagic_Attr
{
	MATTR_LOCK = 0x00000001,
	MATTR_REPEAT = 0x00000002,
	MATTR_SINGING = 0x00000004,		//吟唱
};

class CMagicData
{
public:
	CMagicData(void);
	CMagicData(CMagicData & m);
	~CMagicData(void);

	void SetShortCutKeyEx(DWORD key);
	DWORD GetShortCutKeyEx();
	void SetShortCutKey(char key);
	char GetShortCutKey();
	void SetMagicLevel(char level);
	char GetMagicLevel();
	void SetSkillValue(WORD value);
	WORD GetSkillValue();
	void SetMagicID(WORD id);
	WORD GetMagicID();
	void SetMagicName(const char * strName);
	const char * GetMagicName();
	void SetMagicType(char type);
	char GetMagicType();
	void SetEffect(char eff);
	char GetEffect();
	void SetSpell(WORD spell);
	WORD GetSpell();
	void SetPower(WORD power);
	WORD GetPower();
	void SetNeedLevel(int i,char level);
	char GetNeedLevel(int i);
	void SetTrain(int i, DWORD train);
	DWORD GetTrain(int i);
	void SetJob(char job);
	char GetJob();
	void SetDefence(char def);
	char GetDefence();
	void SetDefencePower(char defPower);
	char GetDefencePower();
	void SetMaxPower(char maxPower);
	char GetMaxPower();
	void SetDefenceMaxPower(char power);
	char GetDefenceMaxPower();
	WORD GetMagicDelay();
	void SetMagicDelay(WORD m);
	void SetColor(DWORD color);
	DWORD GetColor();

	void SetAttr(DWORD dwAttr);
	bool HasAttr(DWORD dwAttr);
	void AddAttr(DWORD dwAttr);
	void RemoveAttr(DWORD dwAttr);
	void UpdateAttr(DWORD dwAttr);//有dwattr则去除，没有则加上
	void DefaultAttr();

	DWORD GetNeedManaValue();

	void SetDisable(bool b);
	bool IsDisable();
	void SetNewSpell(WORD wNewSpell);
	WORD GetNewSpell();
	void SetNewDefSpell(BYTE byNewDefSpell);
	BYTE GetNewDefSpell();
	//函数
	bool FromBuffer(const char* buf,int iLen);

	void Assign(CMagicData& tmp);

	DWORD     GetFlashTexID();
	void      SetFlashTexID(DWORD dwID);

private:
	char   m_cShortCutKey;		// 魔法快捷键
	DWORD  m_dwShortCutKeyEx;   //扩展快捷键

	int    m_iMagicLevel;		// 魔法等级
	WORD   m_wSkillValue;	    // 技能值
	WORD   m_wMagicID;		    // id.
	string m_strName;		    // 魔法名称

	char   m_MagicType;			// 0:普通技能,1:生产技能
	char   m_Eff;
	WORD   m_wSpell;	            //
	WORD   m_wPower;               // 
	char   m_NeedLevel[9];	    // 升级所需要的等级
	DWORD  m_Train[9];           // 升级所需要的技能值
	char   m_iJob;				    // 技能所属职业
	char   m_Def;	
    char   m_DefPower;
    char   m_MaxPower;
	char   m_DefMaxPower;
	WORD   m_nDelay;
	DWORD  m_color;

	DWORD  m_dwAttr;	          //魔法属性选项
	bool   m_bDisable;
	WORD   m_wNewSpell;
	BYTE   m_byNewDefSpell;
	DWORD  m_dwFlashTexID;////是否要闪烁,新手帮助有时候要闪烁特定的按钮,为0表示不要闪烁,< 0x0000FFFF表示闪烁的类型,>0x0000FFFF表示闪烁纹理ID,客户端自己的逻辑控制用

};
//魔法数组
class CMagicDataArray
{
public:
	CMagicDataArray(int iSize);
	~CMagicDataArray();

	int  Size(){ return m_iSize; }
	void Clear();
	CMagicData& Get(int i);
	CMagicData* Add(const char* buf,int iLen);
	int  FindMagicPos(WORD wMagicID);
	CMagicData* FindMagic(WORD wMagicID);
	CMagicData* FindMagicByName(const char *szMagicName);
	bool DeleteMagic(WORD wMagicID);
	WORD FindByShortcut(char cKey);
	CMagicData* FindMagicDataByShortcut(char cKey);
	WORD FindByShortcutEx(DWORD dwKey);
	CMagicData* FindMagicDataByShortcutEx(DWORD dwKey);
	int  GetMagicNumber();
private:
	CMagicData* m_MagicArray;
	int   m_iSize;
};

#include "MagicData.inl"
