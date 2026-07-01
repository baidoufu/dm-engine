#pragma once

#include <vector>
#include <unordered_map>
#include <array>

struct AchievementData //玩家成就数据
{
	BYTE btLevel = 0; // 成就等级
	DWORD dwExp = 0; // 当前成就进度值
	std::vector<BYTE>  btStatus;      // 状态: 0未完成, 1已完成/可领取, 2已领取
	std::vector<DWORD> btRecentCount; // 近期获得成就完成次数
	std::vector<DWORD> dwCompleteTime;// 完成时间戳
};

// 单个属性值 (value_1, value_2)
typedef struct tagTimeAchievePropValue
{
	tagTimeAchievePropValue()
	{
		FILLSELF(0);
	}
	BYTE nValue1;
	BYTE nValue2;
} TIMEACHIEVE_PROPVALUE;

// 单个显示属性 (name -> show_name)
typedef struct tagTimeAchieveShowProp
{
	tagTimeAchieveShowProp()
	{
		FILLSELF(0);
	}
	std::array<char, 8> szName{}; //属性类型
	std::array<char, 8> szShowName{}; //属性文字显示
} TIMEACHIEVE_SHOWPROP;

// 单个职业属性 (name -> value)
typedef struct tagTimeAchieveJobProp
{
	tagTimeAchieveJobProp()
	{
		FILLSELF(0);
	}
	std::array<char, 8> szName{}; //属性类型
	TIMEACHIEVE_PROPVALUE xValue; //属性值
} TIMEACHIEVE_JOBPROP;

// 成就等级配置
typedef struct tagTimeAchieveLevel
{
	tagTimeAchieveLevel()
	{
		nMaxExp = 0;
	}
	DWORD nMaxExp; //成就总进度
	// job -> (prop_name -> prop_value)
	SmallFlatMap<int, SmallFlatMap<std::string, TIMEACHIEVE_PROPVALUE, 8>, 8> mapJobProps; //成就职业属性 (栈存储替代 unordered_map)
} TIMEACHIEVE_LEVEL;

// 成就配置 (Achieve)
typedef struct tagTimeAchieveItem
{
	tagTimeAchieveItem()
	{
		FILLSELF(0);
	}
	WORD nId; //成就ID
	std::array<char, 32> szName{}; //成就名字
	DWORD nMaxExp; //成就总进度
	BYTE nGroup; //成就组ID
	WORD nPoint; //成就点
} TIMEACHIEVE_ITEM;

// 成就系统
class CTimeAchieve : public xSingletonClass<CTimeAchieve>
{
public:
	CTimeAchieve(VOID);
	virtual ~CTimeAchieve(VOID);

	VOID Load(const char* pszPath);

	// 获取某个职业的显示属性列表
	const std::vector<TIMEACHIEVE_SHOWPROP>* GetShowProps(int nJob) const;
	// 获取成就等级数量
	int GetLevelCount() const { return (int)m_vecLevels.size(); }
	// 获取某个等级的配置
	const TIMEACHIEVE_LEVEL* GetLevel(int nLevel) const;
	// 获取某个等级、某个职业的所有属性加成
	const SmallFlatMap<std::string, TIMEACHIEVE_PROPVALUE, 8>* GetJobProps(int nLevel, int nJob) const;
	// 获取某个等级、某个职业、某个属性的加成值
	BOOL GetPropValue(int nLevel, int nJob, const char* pszPropName, TIMEACHIEVE_PROPVALUE& outValue) const;
	// 获取某个等级升级所需的最大经验 (max_exp)
	DWORD GetLevelMaxExp(int nLevel) const;
	// 获取成就项数量
	int GetAchieveCount() const { return (int)m_vecAchieves.size(); }
	// 获取某个成就项
	const TIMEACHIEVE_ITEM* GetAchieve(int nIndex) const;
	// 根据ID查找成就项
	const TIMEACHIEVE_ITEM* FindAchieveById(int nId) const;
	// 根据ID查找成就序号
	const int FindIndexById(int nId) const;
	// 根据ID查找成就组 (返回索引列表)
	const std::vector<int>* FindAchieveGroupById(int nId) const;
	// 发送成就数据给客户端
	VOID SendAchieveData(CHumanPlayer* pPlayer);
private:
	// job -> ShowProp list
	SmallFlatMap<int, std::vector<TIMEACHIEVE_SHOWPROP>, 8> m_mapShowProps; // 栈存储替代 unordered_map
	// level index -> Level config
	std::vector<TIMEACHIEVE_LEVEL> m_vecLevels;
	// Achieve list
	std::vector<TIMEACHIEVE_ITEM> m_vecAchieves;
	// Achieve ID -> index (for fast lookup, 栈存储替代 unordered_map)
	SmallFlatMap<int, int, 512> m_mapAchieveIdToIndex;
	// Group ID -> Achieve index list (for fast lookup)
	SmallFlatMap<int, std::vector<int>, 128> m_mapGroupToIndex; // 栈存储替代 unordered_map
};

//插入属性块数据
static VOID PushPropBlock(xPacket& packet, BYTE btJob, const std::vector<TIMEACHIEVE_SHOWPROP>& vec, const TIMEACHIEVE_LEVEL& level)
{
	DWORD nCount = (DWORD)vec.size();
	packet.push(&nCount, 4);
	for (const auto& showProp : vec)
	{
		packet.push(showProp.szShowName.data());
		packet.push(1);
	}

	packet.push(&nCount, 4);
	auto itJob = level.mapJobProps.find(btJob);
	const auto& mapProps = itJob->second;
	for (const auto& showProp : vec)
	{
		auto it = mapProps.find(showProp.szName.data());
		if (it != mapProps.end())
		{
			WORD nVal1 = (WORD)it->second.nValue1;
			WORD nVal2 = (WORD)it->second.nValue2;
			packet.push(&nVal1, 2);
			packet.push(&nVal2, 2);
		}
		else
			packet.push(4);
	}
}