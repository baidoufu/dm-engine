#pragma once
#include "BotPlayer.h"
#include <vector>
#include <unordered_map>
#include <string>

// ============================================================================
// 机器人管理器 - 负责机器人的创建、删除、配置加载和批量管理
// 单例模式，由CGameWorld持有并管理生命周期
// ============================================================================
class CBotManager : public xSingletonClass<CBotManager>
{
public:
	CBotManager();
	~CBotManager();

	// 从配置文件批量创建机器人（替代原LoadBotConfig）
	BOOL CreateBotsFromConfig(const char* pszFile);

	// 创建单个机器人
	CBotPlayer* CreateBot(BOT_CREATE_DESC& desc);

	// 删除机器人
	BOOL DeleteBot(CBotPlayer* pBot);
	BOOL DeleteBotByName(const char* pszName);

	// 查找
	CBotPlayer* FindBotByName(const char* pszName);
	CBotPlayer* FindBotById(UINT id);

	// 批量控制
	VOID StartAllBots();
	VOID StopAllBots();
	VOID PauseAllBots(DWORD dwDuration);

	// 统计
	int GetBotCount() const { return (int)m_vecBots.size(); }
	int GetRunningBotCount() const;
	DWORD GetTotalOnlineTime() const;

	// 清理所有机器人
	VOID Cleanup();

	// ========================================================================
	// GM命令 - 运行时查询所有Bot状态
	// ========================================================================

	// 列出所有机器人摘要信息（名称/状态/HP%/地图/坐标/目标）
	VOID DumpAllBots(class CHumanPlayer* pCaller);
	// 单个机器人详细信息（BT名/节点命中/在线时长/think间隔/快照）
	VOID DumpBotInfo(class CHumanPlayer* pCaller, const char* pszName);

private:
	// 解析配置文件中的单个机器人段落
	BOOL ParseBotSection(char* pszBotDesc, BOT_CREATE_DESC& desc);

	// 将机器人添加到管理列表
	BOOL AddBotToList(CBotPlayer* pBot);

	// 从管理列表中移除机器人
	VOID RemoveBotFromList(CBotPlayer* pBot);

	// 机器人列表（有序向量，支持分批索引遍历）
	std::vector<CBotPlayer*> m_vecBots;

	// 机器人索引（按名称）
	std::unordered_map<std::string, CBotPlayer*> m_mapBotsByName;

	// 机器人索引（按ID）
	std::unordered_map<UINT, CBotPlayer*> m_mapBotsById;

	// 配置缓存
	std::unordered_map<std::string, BOT_CREATE_DESC> m_ConfigCache;
};
