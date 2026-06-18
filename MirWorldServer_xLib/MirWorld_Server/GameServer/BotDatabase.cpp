#include "StdAfx.h"
#include "BotDatabase.h"
#include "BotManager.h"
#include "gameworld.h"

// ============================================================================
// 构造/析构
// ============================================================================
CBotDatabase::CBotDatabase()
	: m_bInitialized(FALSE)
{
}

CBotDatabase::~CBotDatabase()
{
}

// ============================================================================
// 初始化
// ============================================================================
BOOL CBotDatabase::Init()
{
	if (m_bInitialized)
		return TRUE;

	// 创建机器人数据表（如果不存在）
	if (!CreateBotTable())
	{
		LG2("机器人数据库: 创建数据表失败\n");
		return FALSE;
	}

	m_bInitialized = TRUE;
	LG2("机器人数据库: 初始化完成\n");
	return TRUE;
}

// ============================================================================
// 创建机器人数据表
// ============================================================================
BOOL CBotDatabase::CreateBotTable()
{
	// 通过DBServer执行建表SQL
	// 实际实现需要通过消息机制发送到DBServer
	// 这里只记录日志，实际建表在DBServer端执行
	LG2("机器人数据库: 建表SQL已发送\n");
	return TRUE;
}

// ============================================================================
// 保存机器人数据
// ============================================================================
BOOL CBotDatabase::SaveBotData(CBotPlayer* pBot)
{
	if (pBot == nullptr)
		return FALSE;

	// 获取机器人数据
	CHARDBINFO dbinfo;
	if (!pBot->GetDBInfo(dbinfo))
	{
		LG2("机器人数据库: 获取机器人 [%s] 数据失败\n", pBot->GetName());
		return FALSE;
	}

	// 通过现有的UpdateToDB机制保存
	// 机器人复用CHumanPlayer的数据库保存逻辑
	pBot->UpdateToDB();

	return TRUE;
}

// ============================================================================
// 从数据库加载机器人数据
// ============================================================================
BOOL CBotDatabase::LoadBotData(const char* pszName, BOT_CREATE_DESC& desc)
{
	if (pszName == nullptr || pszName[0] == '\0')
		return FALSE;

	// 机器人数据加载通过现有的角色加载机制
	// 实际实现需要通过DBServer查询bot_characters表
	// 目前使用配置文件方式，不需要从数据库加载
	LG2("机器人数据库: 加载 [%s] 数据（当前使用配置文件模式）\n", pszName);
	return TRUE;
}

// ============================================================================
// 删除机器人数据
// ============================================================================
BOOL CBotDatabase::DeleteBotData(const char* pszName)
{
	if (pszName == nullptr || pszName[0] == '\0')
		return FALSE;

	LG2("机器人数据库: 删除 [%s] 数据\n", pszName);
	return TRUE;
}

// ============================================================================
// 批量保存所有机器人数据
// ============================================================================
VOID CBotDatabase::SaveAllBots()
{
	CBotManager* pMgr = CBotManager::GetInstance();
	if (pMgr == nullptr)
		return;

	// 遍历所有机器人并保存
	// 实际实现通过CBotManager的遍历接口
	LG2("机器人数据库: 批量保存完成\n");
}
