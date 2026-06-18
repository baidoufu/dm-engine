#pragma once
#include <string>
#include <array>
#include <vector>
#include "GuildManagerEx.h"
class CHumanPlayer;

typedef struct tagListNode
{
	tagListNode()
	{
		FILLSELF(0);
	}
	BOOL bTitle;
	UINT nParam;
	LPVOID lpData;
	std::array<char, 64> szName{};
}GuildListNode;

class CGuildMemberEx;
class CGuildGroupEx;

typedef xListHost<CGuildMemberEx>::xListNode GUILDMEMBERNODE;
typedef xListHost<CGuildGroupEx>::xListNode GUILDGROUPNODE;
#define MAX_GUILD_NOTICE_LENGTH	2048

class CGuildMemberEx
{
public:
	CGuildMemberEx();
	~CGuildMemberEx();
	GUILDMEMBERNODE* getNode() { return m_pNode; }
	VOID setNode(GUILDMEMBERNODE* pNode) { m_pNode = pNode; }

	VOID Init(const char* pszName, UINT nExp, UINT nPower, WORD wLevel, BYTE btJob, CHumanPlayer* pRefPlayer = nullptr);
	BOOL IsRefValid();
	VOID SetRefPlayer(CHumanPlayer* pPlayer);

	const char* GetName() { return m_szCharName.data(); }
	UINT GetExp()const { return m_nExp; }
	VOID AddExp(UINT nExp) { m_nExp += nExp; }
	VOID SetExp(UINT nExp) { m_nExp = nExp; }
	VOID DecExp(UINT nExp) { if (m_nExp >= nExp) m_nExp -= nExp; else m_nExp = 0; }
	UINT GetPower()const { return m_nPower; }
	VOID AddPower(UINT nPower) { m_nPower += nPower; }
	VOID SetPower(UINT nPower) { m_nPower = nPower; }
	VOID DecPower(UINT nPower) { if (m_nPower >= nPower) m_nPower -= nPower; else m_nPower = 0; }
	UINT GetLevel() const { return m_wLevel; }
	VOID SetLevel(WORD wLevel) { m_wLevel = wLevel; }
	UINT GetPro() const { return m_btJob; }
	VOID SetPro(BYTE btJob) { m_btJob = btJob; }
	CHumanPlayer* GetRefPlayer() { return m_pRefPlayer; }

	CGuildGroupEx* GetGroup() { return m_pGroup; }
	VOID SetGroup(CGuildGroupEx* pGroup) { m_pGroup = pGroup; }
	//是否允许行会聊天
	BOOL IsNoSay() const { return m_boNoSay; }
	//设置是否允许行会俩天
	VOID SetNoSay() { m_boNoSay = !m_boNoSay; };
protected:
	GUILDMEMBERNODE* m_pNode;
	std::array<char, 20> m_szCharName;
	WORD m_wLevel; //玩家缓存等级
	BYTE m_btJob; //玩家缓存职业
	UINT m_nExp;
	UINT m_nPower;
	CHumanPlayer* m_pRefPlayer;
	DWORD m_dwInstanceKey;
	CGuildGroupEx* m_pGroup;
	BOOL m_boNoSay; // 是否被聊天禁言
};

class CGuildGroupEx
{
public:
	CGuildGroupEx();
	~CGuildGroupEx();

	BOOL AddMember(CGuildMemberEx* pMember, BOOL bConfirm = FALSE);
	VOID RemoveMember(CGuildMemberEx* pMember);
	VOID ConfirmMembers();

	const char* GetName() { return m_szName.data(); }
	VOID SetName(const char* pszName) { o_strncpy(m_szName.data(), pszName, 60); }

	UINT GetLevel()const { return m_nLevel; }
	VOID SetLevel(UINT nLevel) { m_nLevel = nLevel; }

	UINT GetCount() { return m_xMemberList.getCount(); }
	VOID ClearAllRef();

	VOID SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData = nullptr, int iDataSize = -1);
	VOID OnMsg(const char* pszMsg, int iMsgLength);

	VOID SaveToFile(FILE* fp);

	VOID SendWords(const char* pszWords);
	VOID UpdateNames();

	// 时光区-组员消息数据
	VOID AppendDurationMembers(xPacket& packet);
	VOID AppendDurationMembers2(xPacket& packet);

	VOID ReviewAroundNameColor();

	CGuildMemberEx* GetFirstMember();
protected:
	UINT m_nLevel; // 行会分组等级，大于10的分组可以进行显示编辑
	static xObjectPool<GUILDMEMBERNODE> m_xGuildMemberNodePool;
	xListHost<CGuildMemberEx> m_xMemberList;
	std::array<char, 64> m_szName;
};

class CGuildEx;

typedef struct tagGuildRef
{
	tagGuildRef()
	{
		szName[0] = 0;
		pGuild = nullptr;
	}
	char szName[64];
	CGuildEx* pGuild;
}GuildRef;

class CGuildEx :
	public xError
{
public:
	CGuildEx(VOID);
	virtual ~CGuildEx(VOID);

	CGuildMemberEx* GetMember(const char* pszName);
	CGuildMemberEx* GetMember(CHumanPlayer* pMember);
	BOOL AddMember(const char* pszName);
	BOOL AddMember(CHumanPlayer* pMember);
	BOOL RemoveMember(const char* pszName);

	VOID MemberLogon(CHumanPlayer* pMember);
	VOID MemberLogoff(CHumanPlayer* pMember);

	BOOL Create(CHumanPlayer* pCreator, const char* pszName);
	VOID Init();
	//验证名字是否合法
	BOOL IsProperName(const char* pszName);

	VOID Save();
	BOOL Load(const char* pszFile);
	//添加申请玩家
	BOOL AddApplyPlayer(CHumanPlayer* pOperator);
	//删除申请玩家
	BOOL DelApplyPlayer(const char* pszCharName);
	//发送申请玩家到数据包
	VOID SendApplyList(CHumanPlayer* pOperator);
	VOID SendWords(const char* pszWords);

	VOID RefreshMemberName();
	VOID RefreshMemberTitle();
	const char* GetName() { return m_szName.data(); }
	VOID SetName(const char* pszName) { o_strncpy(m_szName.data(), pszName, 60); }

	BOOL IsNoSay(const char* pszCharName);
	VOID SetNoSay(const char* pszCharName);

	BOOL IsMember(CHumanPlayer* pPlayer);
	BOOL IsMaster(CHumanPlayer* pPlayer);	
	BOOL IsAllyGuild(CGuildEx* pGuild) const;
	BOOL IsAllyGuildMember(CHumanPlayer* pPlayer) const;
	BOOL IsKillGuild(CGuildEx* pGuild) const;
	BOOL IsKillGuildMember(CHumanPlayer* pPlayer) const;
	BOOL IsFirstMaster(CHumanPlayer* pPlayer);
	CHumanPlayer* GetMaster();

	VOID SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData = nullptr, int iDataSize = -1);
	VOID SetAttackSabuk(BOOL fAttack) { m_fAttackSabuk = fAttack; }
	BOOL IsAttackSabuk()const { return m_fAttackSabuk; }

	const char* GetNotice() { return m_szNotice.data(); }
	VOID SetNotice(const char* pszNotice) { o_strncpy(m_szNotice.data(), pszNotice, MAX_GUILD_NOTICE_LENGTH - 1); }

	BOOL SendFirstPage(CHumanPlayer* pPlayer);//xPacket & packet );
	//时光区-成员列表
	BOOL SendDurationMemberList(CHumanPlayer* pPlayer = nullptr);
	BOOL SendExp(CHumanPlayer* pPlayer);
	BOOL AddGroupToList(CGuildGroupEx* pGroup);
	//是否行会人数已满
	BOOL IsFull()
	{
		if (this->m_xMemberNameList.GetCount() >= m_nMaxMemberCount)
			return TRUE;
		return FALSE;
	}

	UINT GetExp()const { return m_nExp; }
	VOID AddExp(UINT nExp) 
	{
		if (m_nLevel >= 10) return;
		CGuildManagerEx* GuildManager = CGuildManagerEx::GetInstance();
		m_nExp += nExp;
		if (m_nExp >= GuildManager->GetUpExp(m_nLevel)) // 行会升级判断
		{
			m_nLevel++;
			m_nExp = 0;
		}
	}
	UINT GetLevel()const { return m_nLevel; }

	UINT GetGold()const { return m_nGold; }

	BOOL BuildAlly(CGuildEx* pGuild);
	BOOL BreakAlly(const char* pszName);

	BOOL AddKillGuild(CGuildEx* pGuild);
	BOOL RemoveKillGuild(CGuildEx* pGuild);

	UINT GetKillGuildCount()const { return m_nKillGuildCount; }

	VOID ReviewAroundNameColor();
	VOID Clear();

	char* GetVariableValue(const char* pszVariable);
	VOID SetVariableValue(const char* pszVariable, const char* pszValue);
	VOID ClrVariable(const char* pszVariable);
	BOOL AddVariable(const char* pszVariable, const char* pszValue);
	UINT GetMemberCountFor46Level();
	UINT GetMemberCount() { return m_xMemberNameList.GetCount(); }
	UINT GetMaxMemberCount()const { return this->m_nMaxMemberCount; }
	VOID AddMaxMemberCount(UINT nAdd) { this->m_nMaxMemberCount += nAdd; if (m_nMaxMemberCount > 1000)m_nMaxMemberCount = 1000; }
	VOID SetMaxMemberCount(UINT nCount) { if (nCount > 1000)nCount = 1000; this->m_nMaxMemberCount = nCount; }
	VOID SetAllNoSay() { m_boAllNoSay = !m_boAllNoSay; }
	BOOL IsAllNoSay() const { return m_boAllNoSay; }
	const char* GetFirstOwnerName();
	VOID SetRecruitState(BOOL boRecruitState) { m_boRecruitState = boRecruitState; }
	BOOL GetRecruitState()const { return m_boRecruitState; }
	VOID SetDeclarationList(std::string sNotict)
	{
		m_DeclarationList.clear();
		std::string sNotictTemp = sNotict;
		std::string SC;
		while (!sNotictTemp.empty())
		{
			// 按回车符分隔 (#D = #13 = \r)
			size_t pos = sNotictTemp.find('\r');
			if (pos != std::string::npos)
			{
				SC = sNotictTemp.substr(0, pos);
				sNotictTemp = sNotictTemp.substr(pos + 1);
			}
			else
			{
				SC = sNotictTemp;
				sNotictTemp.clear();
			}
			if (!SC.empty())
				m_DeclarationList.push_back(SC);
		}
		Save();
	}
	std::string GetDeclarationList(BOOL boBreak = TRUE)
	{
		std::string szTemp;
		for (const auto& item : m_DeclarationList)
		{
			szTemp += item;
			if (boBreak) szTemp += "\r";
		}
		szTemp += "";
		return std::move(szTemp);
	}
public: // 行会组管理
	//创建行会组
	CGuildGroupEx* NewGroup(const char* pszName, UINT nLevel = 0) const;
	//发送行会组封号数据
	VOID SendGroupFengHaoData(CHumanPlayer* pPlayer = nullptr);
	//通灵塔信息
	VOID SendGuildTowerInfo(CHumanPlayer* pPlayer);
	//删除行会组
	VOID DeleteGroup(CGuildGroupEx* pGroup);
	//从列表中删除行会组
	VOID DeleteGroupFromList(CGuildGroupEx* pGroup);
	//切换分组
	BOOL ChangeGroup(const char* pszName, UINT nLevel, UINT nOldLevel);
	//通过等级获取行会组
	CGuildGroupEx* GetGroupByLevel(UINT nLevel)
	{
		for (UINT i = 0; i < m_nGroupCount; i++)
		{
			if (m_xGroups[i] && m_xGroups[i]->GetLevel() == nLevel)
				return m_xGroups[i];
		}
		return nullptr;
	}
	//通过名字获取行会组
	CGuildGroupEx* GetGroupByName(const char* pszName)
	{
		for (UINT i = 0; i < m_nGroupCount; i++)
		{
			if (m_xGroups[i] && strcmp(m_xGroups[i]->GetName(), pszName) == 0)
				return m_xGroups[i];
		}
		return nullptr;
	}
protected:
	BOOL AddMemberInternal(const char* pszName, WORD wLevel, BYTE btJob, CHumanPlayer* pRefPlayer = nullptr);
	xStringList<50> m_xMemberNameList;
	std::array<CGuildGroupEx*, 100> m_xGroups{};
	UINT m_nGroupCount;
	xStringList<50> m_xApplyPlayers;
	static xObjectPool<CGuildMemberEx> m_xMemberPool;
	static xObjectPool<CGuildGroupEx> m_xGroupPool;
	std::array<char, 64> m_szName;
	std::array<char, MAX_GUILD_NOTICE_LENGTH> m_szNotice;
	std::array<char, 1024> m_szFilename;
	UINT m_nLevel; // 行会等级等=通灵塔等级
	UINT m_nExp; // 行会经验
	UINT m_nGold; // 行会金币数量
	UINT m_nMaxMemberCount;
	BOOL m_boAllNoSay; // 是否全行会聊天禁言
	std::array<GuildRef, 10> m_sAllyGuilds{};
	std::array<GuildRef, 10> m_sKillGuilds{};
	UINT m_nKillGuildCount;
	UINT m_nAllyGuildCount;
	BOOL m_fAttackSabuk;
	BOOL m_boRecruitState; // 招募状态-时长区
	std::vector<std::string> m_DeclarationList; // 行会宣言-时长区
	xVarList<32> m_xVarList;
};