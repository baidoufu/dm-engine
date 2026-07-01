#pragma once
#include <vector>
#include <unordered_map>
#include <array>

struct Skill {
	int level;
	int value1;
	int value2;
	int value3;
	int value4;
	int value5;
	int value6;
	int value7;
	int value8;
};

struct Magic {
	int id;
	std::vector<Skill> skills;
};
class CMagicManager : public xSingletonClass<CMagicManager>
{
public:
	CMagicManager(VOID);
	virtual ~CMagicManager(VOID);
	//清空技能数据
	VOID ClearMagicData();
	//重新加载技能数据后更新所有在线玩家的技能指针
	VOID ReloadAllPlayerSkills()const;
	//添加解析技能字符串
	BOOL AddMagicClassString(char* pszMagicClassDesc);
	//创建技能
	BOOL CreateMagic(const char* pszName, MAGIC& magic);
	BOOL CreateMagic(UINT id, MAGIC& magic);
	//加载技能扩展数据
	VOID LoadMagicExt(const char* pszMagicExtFile);
	//加载技能BaseMagic.csv数据
	VOID LoadMaigc(const char* pszMagicFile);
	//加载技能MagicSkill.xml属性值
	VOID LoadMaigcskill(const char* pszMagicFile);
	//获取错误文字
	const char* GetErrorMsg() { return m_strErrorMsg.c_str(); }
	//通过Id获取技能类
	MAGICCLASS* GetClassById(int id)const { return m_pMagicArray[id]; }
	//通过名字获取技能类
	MAGICCLASS* GetClassByName(const char* pszMagic);
	//通过技能Id获取技能
	Magic& GetMagic(WORD wMagicId);
	//通过技能类获取技能
	VOID GetMagicFromClass(MAGICCLASS* pClass, MAGIC& magic);
	//从数据库删除玩家的技能
	BOOL DeleteMagicFromDB(DWORD dwOwner, WORD wMagicId);
private:
	// 添加技能模板
	BOOL AddMagicClass(MAGICCLASS* pMagicclass);
private:
	std::string m_strErrorMsg;
	xListHost<MAGICCLASS> m_xMagicClassList;
	CNameHash m_MagicClassHash;
	std::array<MAGICCLASS*, 128> m_pMagicArray{};
	SmallFlatMap<WORD, Magic, 128> magics; // 栈存储替代 unordered_map
};