#include "StdAfx.h"
#include <atomic>
#include "BotManager.h"
#include "BotPlayer.h"
#include "BotHumanBehavior.h"
#include "gameworld.h"
#include "humanplayermgr.h"

// ============================================================================
// 构造/析构
// ============================================================================
CBotManager::CBotManager()
{
}

CBotManager::~CBotManager()
{
	Cleanup();
}

// ============================================================================
// 从配置文件批量创建机器人（替代原LoadBotConfig）
// ============================================================================
BOOL CBotManager::CreateBotsFromConfig(const char* pszFile)
{
	CStringFile sf(pszFile);
	if (!sf.IsSucceed())
	{
		PRINT(ERROR_RED, "加载配置文件 [%s] 失败\n", pszFile);
		return FALSE;
	}
	int nCreated = 0;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (sf[i][0] == '#') continue;
		BOT_CREATE_DESC desc;
		if (!ParseBotSection(sf[i], desc))
		{
			LG2("机器人管理器: 解析 [%s] 失败，跳过\n", sf[i]);
			continue;
		}
		CBotPlayer* pBot = CreateBot(desc);
		if (pBot != nullptr)
			nCreated++;
		else
			LG2("机器人管理器: 创建机器人 [%s] 失败\n", desc.dbinfo.szName);
	}
	LG2("机器人管理器: 成功创建 %d 个机器人\n", nCreated);
	return nCreated > 0;
}

// ============================================================================
// 解析配置文件中的单个机器人段落
// ============================================================================
BOOL CBotManager::ParseBotSection(char* pszBotDesc, BOT_CREATE_DESC& desc)
{
	char* Params[15];
	int nParam = SearchParam(pszBotDesc, Params, 15, ",");
	if (nParam < 15)
	{
		PRINT(ERROR_RED, "机器人管理器: 字段 [%s] 配置错误\n", pszBotDesc);
		return FALSE;
	}
	if (Params[1] == '\0')
	{
		LG2("机器人管理器: 段落 [%s] 缺少Name字段\n", Params[1]);
		return FALSE;
	}
	o_strncpy(desc.dbinfo.szName, Params[1], 19);
	desc.dbinfo.btClass = (BYTE)StringToInteger(Params[2]);
	desc.dbinfo.btSex = (BYTE)StringToInteger(Params[3]);
	desc.dbinfo.wLevel = (WORD)StringToInteger(Params[4]);
	desc.dbinfo.mapid = (DWORD)StringToInteger(Params[5]);
	desc.dbinfo.x = (WORD)StringToInteger(Params[6]);
	desc.dbinfo.y = (WORD)StringToInteger(Params[7]);
	desc.dwThinkInterval = (DWORD)StringToInteger(Params[8]);
	desc.dwIdleChance = (DWORD)StringToInteger(Params[9]);
	desc.dwChatChance = (DWORD)StringToInteger(Params[10]);
	// 读取技能
	const char* pszSkills = Params[11];
	if (pszSkills[0] != '\0')
	{
		char szBuffer[512];
		o_strncpy(szBuffer, pszSkills, 511);
		char* Params2[27];
		int nParam = SearchParam(szBuffer, Params2, 27, "|");
		for (int i = 0; i < nParam; i++)
		{
			desc.vSkills.emplace_back(atoi(Params2[i]));
		}
	}
	// 读取行为配置文件
	const char* pszBehaviorFile = Params[12];
	o_strncpy(desc.szBehaviorFile, pszBehaviorFile, 255);
	// 读取装备
	const char* pszEquipments = Params[13];
	if (pszEquipments[0] != '\0')
	{
		char szBuffer[1024];
		o_strncpy(szBuffer, pszEquipments, 1023);
		char* Params2[_U_MAX];
		int nParam = SearchParam(szBuffer, Params2, _U_MAX, "|");
		for (int i = 0; i < nParam; i++)
		{
			desc.vEquipments.emplace_back(Params2[i]);
		}
	}
	// 读取背包
	const char* pszItemsBox = Params[14];
	if (pszItemsBox[0] != '\0')
	{
		int nBagMax = 0;
		char szBuffer[4096];
		o_strncpy(szBuffer, pszItemsBox, 4095);
		char* Params2[BIGBAG_SLOT];
		char* ParamsNum[2];
		int nParam = SearchParam(szBuffer, Params2, BIGBAG_SLOT, "|");
		for (int i = 0; i < nParam; i++)
		{
			int nParamNum = SearchParam(Params2[i], ParamsNum, 2, ".");
			if (nParamNum == 1)
			{
				desc.vItemsBox.emplace_back(ParamsNum[0]);
				nBagMax++;
			}
			else if (nParamNum == 2)
			{
				for (int j = 0; j < atoi(ParamsNum[1]); j++)
				{
					desc.vItemsBox.emplace_back(ParamsNum[0]);
					nBagMax++;
					if (nBagMax >= BIGBAG_SLOT) break;
				}
			}
			if (nBagMax >= BIGBAG_SLOT) break;
		}
	}
	desc.dbinfo.hp = 1;
	desc.dbinfo.mp = 1;
	desc.dbinfo.dwDBId = 900000 + (DWORD)std::hash<std::string>{}(desc.dbinfo.szName) % 99999;
	return TRUE;
}

// ============================================================================
// 创建单个机器人
// ============================================================================
CBotPlayer* CBotManager::CreateBot(BOT_CREATE_DESC& desc)
{
	if (desc.dbinfo.szName[0] == '\0')
	{
		LG2("机器人管理器: 机器人名称为空\n");
		return nullptr;
	}
	if (FindBotByName(desc.dbinfo.szName) != nullptr)
	{
		LG2("机器人管理器: 机器人 [%s] 已存在\n", desc.dbinfo.szName);
		return nullptr;
	}
	CBotPlayer* pBot = new CBotPlayer();
	if (pBot == nullptr)
	{
		LG2("机器人管理器: 分配内存失败\n");
		return nullptr;
	}
	static std::atomic<DWORD> s_dwBotIdCounter(10000);
	DWORD dwBotId = (s_dwBotIdCounter.fetch_add(1) + 1) | (OBJ_PLAYER << 24);
	pBot->SetId(dwBotId);
	if (!pBot->InitBot(desc))
	{
		LG2("机器人管理器: 初始化机器人 [%s] 失败\n", desc.dbinfo.szName);
		delete pBot;
		return nullptr;
	}
	if (!CHumanPlayerMgr::GetInstance()->RegisterBotPlayer(pBot))
	{
		LG2("机器人管理器: 注册机器人 [%s] 到玩家管理器失败\n", desc.dbinfo.szName);
		pBot->Clean();
		delete pBot;
		return nullptr;
	}
	if (!CGameWorld::GetInstance()->AddMapObject(pBot))
	{
		LG2("机器人管理器: 机器人 [%s] 进入地图 [%d] 失败\n",
			desc.dbinfo.szName, desc.dbinfo.mapid);
		CHumanPlayerMgr::GetInstance()->UnregisterBotPlayer(pBot);
		pBot->Clean();
		delete pBot;
		return nullptr;
	}
	AddBotToList(pBot);
	pBot->StartBot();
	LG2("机器人管理器: 创建机器人 [%s] 成功\n", desc.dbinfo.szName);
	return pBot;
}

// ============================================================================
// 删除机器人
// ============================================================================
BOOL CBotManager::DeleteBot(CBotPlayer* pBot)
{
	if (pBot == nullptr) return FALSE;
	LG2("机器人管理器: 删除机器人 [%s]\n", pBot->GetName());
	pBot->StopBot();
	RemoveBotFromList(pBot);
	CHumanPlayerMgr::GetInstance()->UnregisterBotPlayer(pBot);
	CGameWorld::GetInstance()->RemoveMapObject(pBot);
	pBot->Clean();
	delete pBot;
	return TRUE;
}

BOOL CBotManager::DeleteBotByName(const char* pszName)
{
	CBotPlayer* pBot = FindBotByName(pszName);
	if (pBot == nullptr) return FALSE;
	return DeleteBot(pBot);
}

// ============================================================================
// 查找机器人
// ============================================================================
CBotPlayer* CBotManager::FindBotByName(const char* pszName)
{
	if (pszName == nullptr)
		return nullptr;
	auto it = m_mapBotsByName.find(pszName);
	if (it != m_mapBotsByName.end())
		return it->second;
	return nullptr;
}

CBotPlayer* CBotManager::FindBotById(UINT id)
{
	auto it = m_mapBotsById.find(id);
	if (it != m_mapBotsById.end())
		return it->second;
	return nullptr;
}

// ============================================================================
// 批量控制
// ============================================================================
VOID CBotManager::StartAllBots()
{
	for (auto* pBot : m_vecBots)
	{
		if (pBot)
			pBot->StartBot();
	}
	LG2("机器人管理器: 启动所有机器人，共 %d 个\n", GetBotCount());
}

VOID CBotManager::StopAllBots()
{
	for (auto* pBot : m_vecBots)
	{
		if (pBot)
			pBot->StopBot();
	}
	LG2("机器人管理器: 停止所有机器人，共 %d 个\n", GetBotCount());
}

VOID CBotManager::PauseAllBots(DWORD dwDuration)
{
	for (auto* pBot : m_vecBots)
	{
		if (pBot)
			pBot->PauseBot(dwDuration);
	}
}

// ============================================================================
// 统计
// ============================================================================
int CBotManager::GetRunningBotCount() const
{
	int count = 0;
	for (const auto* pBot : m_vecBots)
	{
		if (pBot && pBot->IsBotRunning())
			count++;
	}
	return count;
}

DWORD CBotManager::GetTotalOnlineTime() const
{
	DWORD dwTotal = 0;
	for (const auto* pBot : m_vecBots)
	{
		if (pBot)
			dwTotal += pBot->GetBotDesc().dwThinkInterval;
	}
	return dwTotal;
}

// ============================================================================
// 清理所有机器人
// ============================================================================
VOID CBotManager::Cleanup()
{
	for (auto* pBot : m_vecBots)
	{
		if (pBot)
		{
			pBot->StopBot();
			CHumanPlayerMgr::GetInstance()->UnregisterBotPlayer(pBot);
			CGameWorld::GetInstance()->RemoveMapObject(pBot);
			pBot->Clean();
			delete pBot;
		}
	}
	m_vecBots.clear();
	m_mapBotsByName.clear();
	m_mapBotsById.clear();
	m_ConfigCache.clear();
	LG2("机器人管理器: 清理完成\n");
}

// ============================================================================
// 添加到管理列表
// ============================================================================
BOOL CBotManager::AddBotToList(CBotPlayer* pBot)
{
	if (pBot == nullptr) return FALSE;
	const char* pszName = pBot->GetName();
	if (pszName == nullptr || pszName[0] == '\0')
		return FALSE;
	m_vecBots.push_back(pBot);
	m_mapBotsByName[pszName] = pBot;
	m_mapBotsById[pBot->GetId()] = pBot;
	return TRUE;
}

// ============================================================================
// 从管理列表中移除
// ============================================================================
VOID CBotManager::RemoveBotFromList(CBotPlayer* pBot)
{
	if (pBot == nullptr) return;
	const char* pszName = pBot->GetName();
	if (pszName && pszName[0] != '\0')
		m_mapBotsByName.erase(pszName);
	m_mapBotsById.erase(pBot->GetId());
	// 从向量中移除（swap-and-pop，O(1)复杂度）
	auto it = std::find(m_vecBots.begin(), m_vecBots.end(), pBot);
	if (it != m_vecBots.end())
	{
		*it = m_vecBots.back();
		m_vecBots.pop_back();
	}
}
