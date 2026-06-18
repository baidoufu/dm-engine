#pragma once
#include "BotPlayer.h"

// ============================================================================
// 机器人数据库接口 - 负责机器人数据的持久化存储和读取
// 复用现有的DBServer异步查询机制，通过消息队列与DBServer通信
// ============================================================================
class CBotDatabase
{
public:
	CBotDatabase();
	~CBotDatabase();

	// 初始化数据库连接
	BOOL Init();

	// 保存机器人数据到数据库
	BOOL SaveBotData(CBotPlayer* pBot);

	// 从数据库加载机器人数据
	BOOL LoadBotData(const char* pszName, BOT_CREATE_DESC& desc);

	// 删除机器人数据
	BOOL DeleteBotData(const char* pszName);

	// 批量保存所有机器人数据
	VOID SaveAllBots();

	// 创建机器人数据表（首次运行时调用）
	BOOL CreateBotTable();

private:
	BOOL m_bInitialized;  // 是否已初始化
};
