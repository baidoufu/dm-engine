#include "StdAfx.h"
#include ".\gameworld.h"
#include <unordered_map>
#include "physicsmapmgr.h"
#include "logicmapmgr.h"
#include "ScriptVariable.h"
#include "humanplayermgr.h"
#include "itemmanager.h"
#include "server.h"
#include "magicmanager.h"
#include "scriptobjectmgr.h"
#include "npcmanager.h"
#include "gmmanager.h"
#include "StringListManager.h"
#include "fmttextfile.h"
#include "titlemanager.h"
#include "FengHaoGrowManager.h"
#include "downitemmgr.h"
#include "BundleManager.h"
#include "eventmanager.h"
#include "BossTJ.h"
#include "TimeAchieve.h"
#include "GameStage.h"
#include "guildex.h"
#include "guildmanagerex.h"
#include "monsterex.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>

///////////// monsters ////////////////////
#include "monstermanagerex.h"
#include "monstergenmanager.h"
#include "monitemsmgr.h"
#include "sandcity.h"
#include "scriptnpc.h"
#include "timesystem.h"
#include "systemscript.h"
#include "topmanager.h"
#include ".\specialequipmentmanager.h"
#include "scriptobject.h"
#include "minelist.h"
#include "marketmanager.h"
#include "autoscriptmanager.h"
#include "mapscriptmanager.h"
#include "guildwarmanager.h"
#include "TaskManager.h"
#include "BotManager.h"

extern DWORD g_dwActionDelay[AT_MAX];

CGameWorld::CGameWorld(VOID) :
	m_dwUpdateKey(0),
	m_iStartPointCount(0),
	m_iNoticeLines(0),
	m_iNoticePtr(0),
	m_bWorkerRunning(false),
	m_bAsyncUpdateReady(false),
	m_fExpFactor(1.0f),
	m_DBUpdateTimer()
{
	m_pNameList.fill(nullptr);
	m_VarList.fill(0);
	m_dwChannelTime.fill(0);
	m_iBornPointCount.fill(0);
	
	for (auto& arr : m_HumanData) {
		arr.fill({});
	}
	for (auto& line : m_NoticeLines) {
		line = {};
	}
	
	memset(&m_FirstLoginInfo, 0, sizeof(m_FirstLoginInfo));

	m_pObjectList[OBJ_MONSTER] = std::make_unique<MAPOBJECT_LIST>();
	m_pObjectList[OBJ_PLAYER] = std::make_unique<MAPOBJECT_LIST>();
	m_pObjectList[OBJ_NPC] = std::make_unique<MAPOBJECT_LIST>();
	m_DBUpdateTimer.Savetime();
}

CGameWorld::~CGameWorld(VOID)
{
	ShutdownThreadPool();
	// 智能指针自动清理，无需手动delete
}

VOID CGameWorkerThread::Execute(LPVOID lpParam)
{
	m_pOwner->AsyncUpdateWorker();
}

VOID CGameWorld::ShowNpc(UINT nMapId)//NPC显示
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
	if (pMap == nullptr) return;
	std::vector<CScriptNpc*> npcsToShow;
	{
		xListHelper<CMapObject> objlist(m_pObjectList[OBJ_NPC].get());
		SRLock lock(m_rwObjectListLock);
		for (CScriptNpc* pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
		{
			if (pNpc->GetMapId() == nMapId && pNpc->GetMap() == nullptr)
				npcsToShow.push_back(pNpc);
		}
	}
	// 释放锁后再执行 AddObject
	for (CScriptNpc* pNpc : npcsToShow)
		pMap->AddObject(pNpc);
}

VOID CGameWorld::HideNpc(UINT nMapId)//NPC消失
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
	if (pMap == nullptr) return;
	std::vector<CScriptNpc*> npcsToShow;
	{
		xListHelper<CMapObject> objlist(m_pObjectList[OBJ_NPC].get());
		SRLock lock(m_rwObjectListLock);
		for (CScriptNpc* pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
		{
			if (pNpc->GetMapId() == nMapId && pNpc->GetMap() == pMap)
				npcsToShow.push_back(pNpc);
		}
	}
	// 释放锁后再执行 RemoveObject
	for (CScriptNpc* pNpc : npcsToShow)
		pMap->RemoveObject(pNpc);
}

VOID CGameWorld::ShowSandCityNpc()
{
	std::vector<CScriptNpc*> npcsToShow;
	{
		xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC].get());
		SRLock lock(m_rwObjectListLock);
		for (CScriptNpc* pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
		{
			if (pNpc->IsSandCityMerchant())
				npcsToShow.push_back(pNpc);
		}
	}
	// 释放锁后再执行 AddObject
	for (CScriptNpc* pNpc : npcsToShow)
	{
		CLogicMap* pMap = pNpc->GetMap();
		if (pMap) pMap->AddObject(pNpc);
	}
}

VOID CGameWorld::HideSandCityNpc()
{
	std::vector<CScriptNpc*> npcsToHide;
	{
		xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC].get());
		SRLock lock(m_rwObjectListLock);
		for (CScriptNpc* pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
		{
			if (pNpc->IsSandCityMerchant())
				npcsToHide.push_back(pNpc);
		}
	}
	// 释放锁后再执行 RemoveObject
	for (CScriptNpc* pNpc : npcsToHide)
	{
		CLogicMap* pMap = pNpc->GetMap();
		if (pMap) pMap->RemoveObject(pNpc);
	}
}

#define	LOADVAR( var, defvalue ) m_VarList[EVI_##var] = (DWORD)m_sfServer.GetInteger( "Var", #var, (defvalue) );
#define	LOADNAME( var, defvalue ) m_pNameList[ENI_##var] = (char*)m_sfServer.GetString( "Name", #var, (defvalue) );
BOOL CGameWorld::LoadServerConfig()
{
	m_sfServer.Close();
	if (!m_sfServer.Open(".\\Data\\Server.txt"))
	{
		PRINT(ERROR_RED, "无法打开服务器配置文件 .\\Data\\Server.txt");
		return FALSE;
	}

	g_dwActionDelay[AT_WALK] = (DWORD)m_sfServer.GetInteger("Speed", "Walk", 600);
	g_dwActionDelay[AT_RUN] = (DWORD)m_sfServer.GetInteger("Speed", "Run", 600);
	g_dwActionDelay[AT_ATTACK] = (DWORD)m_sfServer.GetInteger("Speed", "Attack", 850);
	g_dwActionDelay[AT_BEATTACK] = (DWORD)m_sfServer.GetInteger("Speed", "Beattack", 300);

	m_pNameList[ENI_GOLD] = (char*)m_sfServer.GetString("Name", "GoldName", "金币");
	m_pNameList[ENI_MALE] = (char*)m_sfServer.GetString("Name", "MaleName", "男");
	m_pNameList[ENI_FEMALE] = (char*)m_sfServer.GetString("Name", "FemaleName", "女");
	m_pNameList[ENI_GUILDNOTICE] = (char*)m_sfServer.GetString("Name", "GuildNotice", "公告");
	m_pNameList[ENI_KILLGUILDS] = (char*)m_sfServer.GetString("Name", "KillGuilds", "敌对行会");
	m_pNameList[ENI_ALLYGUILDS] = (char*)m_sfServer.GetString("Name", "AllyGuilds", "联盟行会");
	m_pNameList[ENI_MEMBERS] = (char*)m_sfServer.GetString("Name", "Members", "行会成员");
	m_pNameList[ENI_GUILDDECLARATION] = (char*)m_sfServer.GetString("Name", "GuildDeclaration", "行会宣言");
	m_pNameList[ENI_GUILDAPPLYLIST] = (char*)m_sfServer.GetString("Name", "GuildApplyList", "申请列表");
	m_pNameList[ENI_WARR] = (char*)m_sfServer.GetString("Name", "Warr", "战士");
	m_pNameList[ENI_MAGICAN] = (char*)m_sfServer.GetString("Name", "Magican", "法师");
	m_pNameList[ENI_TAOSHI] = (char*)m_sfServer.GetString("Name", "Taoshi", "道士");
	m_pNameList[ENI_TOPOFWORLD] = (char*)m_sfServer.GetString("Name", "TopOfWorld", "天下第一");
	m_pNameList[ENI_UPGRADEMINESTONE] = (char*)m_sfServer.GetString("Name", "UpgradeMineStone", "黑铁矿石");
	LOADNAME(PHYSICSMAPPATH, ".\\Data\\Maps\\Physics");
	LOADNAME(PHYSICSCACHEPATH, ".\\Data\\Maps\\Cache");

	m_fExpFactor = m_sfServer.GetInteger("Setting", "ExpFactor", 100) / 100.0f;//爆率

	m_VarList[EVI_MAXGOLD] = (DWORD)m_sfServer.GetInteger("Var", "MaxGold", 5000000);
	m_VarList[EVI_MAXBIGGOLD] = (DWORD)m_sfServer.GetInteger("Var", "MaxBigGold", 10000000);
	m_VarList[EVI_MAXYUANBAO] = (DWORD)m_sfServer.GetInteger("Var", "MaxYuanBao", 2000);
	m_VarList[EVI_ITEMUPDATETIME] = (DWORD)m_sfServer.GetInteger("Var", "ItemUpdateTime", 300) * 1000;
	m_VarList[EVI_MAXGROUPMEMBER] = (DWORD)m_sfServer.GetInteger("Var", "MaxGroupMember", 10);
	m_VarList[EVI_REDPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "RedPkPoint", 12);
	m_VarList[EVI_YELLOWPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "YellowPkPoint", 6);
	m_VarList[EVI_ONCEPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "OncePkPoint", 3);
	m_VarList[EVI_ONEPKPOINTTIME] = (DWORD)m_sfServer.GetInteger("Var", "OnePkPointTime", 120);
	m_VarList[EVI_BODYTIME] = (DWORD)m_sfServer.GetInteger("Var", "BodyTime", 60);
	m_VarList[EVI_STOREAGESIZE] = (DWORD)m_sfServer.GetInteger("Var", "StoreageSize", 39);
	m_VarList[EVI_SANDCITYTAKETIME] = (DWORD)m_sfServer.GetInteger("Var", "SandCityTakeTime", 300);
	m_VarList[EVI_WARENEMYCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarEnemyColor", 243);
	m_VarList[EVI_WARALLYCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarAllyColor", 4);
	m_VarList[EVI_WARNORMALCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarNormalColor", 219);
	m_VarList[EVI_WARTIMELONG] = (DWORD)m_sfServer.GetInteger("Var", "WarTimeLong", 240);
	m_VarList[EVI_WARSTARTTIME] = ((DWORD)m_sfServer.GetInteger("Var", "WarStartTime", 20)) % 24;
	m_VarList[EVI_ENABLESAFEAREANOTICE] = (DWORD)m_sfServer.GetInteger("Var", "EnableSafeAreaNotice", 0);
	m_VarList[EVI_ADDFRIENDLEVEL] = (DWORD)m_sfServer.GetInteger("Var", "AddFriendLevel", 7);
	m_VarList[EVI_GUILDLEVELNUM] = (DWORD)m_sfServer.GetInteger("Var", "GuildLevelNum", 9);
	LOADVAR(PRIVATESHOPLEVEL, 20);
	LOADVAR(INITDRESSCOLOR, -1);
	LOADVAR(REPAIRDAMAGEDURA, 1000);
	LOADVAR(GRAYNAMETIME, 60);
	LOADVAR(DROPTARGETDISTANCE, 12);
	LOADVAR(WEAPONDAMAGERATE, 15);
	LOADVAR(DRESSDAMAGERATE, 15);
	LOADVAR(DEFENCEDAMAGERATE, 15);
	LOADVAR(JEWELRYDAMAGERATE, 15);
	LOADVAR(RANDOMUPGRADEITEMRATE, 16);
	LOADVAR(CHARINFOBACKUPTIME, 30);
	LOADVAR(DBUPDATEDELAY, 2000);
	LOADVAR(MAXUPGRADETIMES, 10);
	LOADVAR(MONGENFACTOR, 100);
	LOADVAR(PKCURSERATE, 50);
	LOADVAR(GUILDWARTIME, 3600 * 3);
	LOADVAR(STARTGUILDMEMBERCOUNT, 64);
	LOADVAR(DROPTARGETTIME, 3);

	m_dwChannelTime[CCH_NORMAL] = (DWORD)m_sfServer.GetInteger("ChatWait", "Normal", 1);
	m_dwChannelTime[CCH_CRY] = (DWORD)m_sfServer.GetInteger("ChatWait", "Cry", 10);
	m_dwChannelTime[CCH_WISPER] = (DWORD)m_sfServer.GetInteger("ChatWait", "Whisper", 2);
	m_dwChannelTime[CCH_GROUP] = (DWORD)m_sfServer.GetInteger("ChatWait", "Group", 2);
	m_dwChannelTime[CCH_GUILD] = (DWORD)m_sfServer.GetInteger("ChatWait", "Guild", 3);
	m_dwChannelTime[CCH_COUPLE] = (DWORD)m_sfServer.GetInteger("ChatWait", "Couple", 1);
	m_dwChannelTime[CCH_GM] = (DWORD)m_sfServer.GetInteger("ChatWait", "Gm", 0);
	m_dwChannelTime[CCH_FRIEND] = (DWORD)m_sfServer.GetInteger("ChatWait", "Friend", 2);

	auto* pStringListManager = CStringListManager::GetInstance();
	pStringListManager->ClearAll();
	pStringListManager->Load(".\\Data\\StringList");

	//	读取角色信息文件
	char* pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Warrior", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_WARRIOR, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Warrior");
	pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Magician", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_MAGICIAN, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Magician");

	pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Taoshi", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_TAOSHI, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Taoshi");
	char* pszTemp = "";
	m_FirstLoginInfo.nGameTime = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartGameTime", -1);
	m_FirstLoginInfo.dwGold = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartGold", 0);
	m_FirstLoginInfo.dwYuanBao = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartYuanBao", 0);
	m_FirstLoginInfo.level = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartLevel", 1);
	pszTemp = (char*)m_sfServer.GetString("FirstLogin", "StartItem", nullptr);
	while (m_FirstLoginInfo.pItems != nullptr)
	{
		FIRSTLOGIN_ITEM* p = m_FirstLoginInfo.pItems;
		m_FirstLoginInfo.pItems = p->pNext;
		delete p;
	}
	if (pszTemp != nullptr)
	{
		char* Params[BIGBAG_SLOT], * Params2[5];
		int nParam = SearchParam(pszTemp, Params, BIGBAG_SLOT, "/");
		int nParam2 = 0;
		for (int i = 0; i < nParam; i++)
		{
			nParam2 = SearchParam(Params[i], Params2, 5, "*");
			FIRSTLOGIN_ITEM* pItem = new FIRSTLOGIN_ITEM;
			if (nParam2 == 1)
			{
				pItem->nCount = 1;
				pItem->btJob = 99;//不限职业
				pItem->btSex = 99;//不限性别
				pItem->boBind = 1;//绑定
			}
			else if (nParam2 <= 2)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = 99;
				pItem->btSex = 99;
				pItem->boBind = 1;
			}
			else if(nParam2 <= 3)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = 99;
				pItem->boBind = 1;
			}
			else if (nParam2 <= 4)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = StringToInteger(Params2[3]);
				pItem->boBind = 1;
			}
			else
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = StringToInteger(Params2[3]);
				pItem->boBind = StringToInteger(Params2[4]);
			}
			o_strncpy(pItem->szItem, Params2[0], 31);
			pItem->pNext = m_FirstLoginInfo.pItems;
			m_FirstLoginInfo.pItems = pItem;
		}
	}
	return TRUE;
}

VOID CGameWorld::LoadSafeArea()
{
	CFmtTextFile ftfSafeArea("w4", ".\\Data\\Config\\SafeArea.csv", TRUE);
	WORD wSafeArea[4] = { 0 };
	for (int i = 0; i < ftfSafeArea.GetCount(); i++)
	{
		if (ftfSafeArea.GetStruct(i, (LPVOID)wSafeArea))
		{
			CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(wSafeArea[0]);
			if (pMap != nullptr)
			{
				//	设置安全区
				PRINT(COOL_BLUE, "设置 %s 地图的 (%u,%u) 为中心半径 %u 为安全区\n", pMap->GetTitle(), wSafeArea[1], wSafeArea[2], wSafeArea[3]);
				pMap->SetSafeArea(wSafeArea[1], wSafeArea[2], wSafeArea[3]);
			}
		}
	}
}

VOID CGameWorld::LoadStartPoint()
{
	m_iStartPointCount = 0;
	m_pStartPoints.reset();
	CFmtTextFile ftfStartPoint("s32w4b3", ".\\Data\\Config\\StartPoint.csv", TRUE);
	if (ftfStartPoint.GetCount() > 0)
	{
		m_pStartPoints = std::make_unique<START_POINT[]>(ftfStartPoint.GetCount());
		for (int i = 0; i < ftfStartPoint.GetCount(); i++)
		{
			if (ftfStartPoint.GetStruct(i, &m_pStartPoints[m_iStartPointCount]))
			{
				PRINT(STRING_GREEN, "读取出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[m_iStartPointCount].szName,
					m_pStartPoints[m_iStartPointCount].x,
					m_pStartPoints[m_iStartPointCount].y,
					m_pStartPoints[m_iStartPointCount].mapid,
					m_pStartPoints[m_iStartPointCount].range);
				CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(m_pStartPoints[m_iStartPointCount].mapid);
				if (pMap == nullptr)
				{
					PRINT(ERROR_RED, "没有该地图号所指的地图, 该出生点作废!\n");
					continue;
				}
				m_pStartPoints[m_iStartPointCount].index = m_iStartPointCount;
				if (!m_StartPointHash.HAdd(m_pStartPoints[m_iStartPointCount].szName, &m_pStartPoints[m_iStartPointCount]))
				{
					PRINT(ERROR_RED, "由于名字重复, 该出生点作废!\n");
					continue;
				}
				pMap->AddStartPoint(&m_pStartPoints[m_iStartPointCount]);
				if (m_pStartPoints[m_iStartPointCount].bFighterBorn)
					m_iBornPointCount[0]++;
				if (m_pStartPoints[m_iStartPointCount].bMagicianBorn)
					m_iBornPointCount[1]++;
				if (m_pStartPoints[m_iStartPointCount].bTaoshiBorn)
					m_iBornPointCount[2]++;

				m_iStartPointCount++;
			}
		}

		m_BornPoints[0].reset();
		m_BornPoints[1].reset();
		m_BornPoints[2].reset();

		m_BornPoints[0] = std::make_unique<START_POINT*[]>(m_iBornPointCount[0]);
		m_BornPoints[1] = std::make_unique<START_POINT*[]>(m_iBornPointCount[1]);
		m_BornPoints[2] = std::make_unique<START_POINT*[]>(m_iBornPointCount[2]);
		m_iBornPointCount[0] = 0;
		m_iBornPointCount[1] = 0;
		m_iBornPointCount[2] = 0;

		for (int i = 0; i < m_iStartPointCount; i++)
		{
			if (m_pStartPoints[i].bFighterBorn)
			{
				m_BornPoints[0][m_iBornPointCount[0]++] = m_pStartPoints.get() + i;
				PRINT(STRING_GREEN, "战士出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
			if (m_pStartPoints[i].bMagicianBorn)
			{
				m_BornPoints[1][m_iBornPointCount[1]++] = m_pStartPoints.get() + i;
				PRINT(STRING_GREEN, "法师出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
			if (m_pStartPoints[i].bTaoshiBorn)
			{
				m_BornPoints[2][m_iBornPointCount[2]++] = m_pStartPoints.get() + i;
				PRINT(STRING_GREEN, "道士出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
		}

	}
}

VOID CGameWorld::LoadNotice()
{
	m_pNotice = LoadFile(".\\Data\\Config\\Notice.txt");
	if (!m_pNotice)
	{
		m_pNotice = nullptr;
	}
	CStringFile sfLN(".\\Data\\Config\\LineNotice.txt");
	m_iNoticeLines = 0;
	m_iNoticePtr = 0;
	for (int i = 0; i < (sfLN.GetLineCount() > 1024 ? 1024 : sfLN.GetLineCount()); i++)
	{
		if (*sfLN[i] == '#')continue;
		xStringsExpander<10> ss(sfLN[i], '|');
		if (ss.getCount() == 1)
		{
			o_strncpy(m_NoticeLines[i].szWords.data(), ss[0], 255);
			m_NoticeLines[m_iNoticeLines].dwDelay = 300000;
			m_NoticeLines[m_iNoticeLines].timer.Savetime();
			m_iNoticeLines++;
		}
		else if (ss.getCount() >= 2)
		{
			o_strncpy(m_NoticeLines[m_iNoticeLines].szWords.data(), ss[1], 255);
			m_NoticeLines[m_iNoticeLines].dwDelay = 1000 * StringToInteger(ss[0]);
			m_NoticeLines[m_iNoticeLines].timer.Savetime();
			m_iNoticeLines++;
		}
	}
	m_LineNoticeTimer.Savetime();
}

VOID CGameWorld::LoadClientKeyConfig()
{
	m_ClientKeyConfig.fill({});
	FILE* fp = fopen(".\\Data\\Config\\ClientKeyConfig.json", "rb");
	if (fp == nullptr)
	{
		PRINT(ERROR_RED, "无法打开 ClientKeyConfig.json 文件\n");
		return;
	}
	std::array<char, 6144> readBuffer{};
	rapidjson::FileReadStream is(fp, readBuffer.data(), readBuffer.size());
	rapidjson::Document doc;
	doc.ParseStream(is);
	fclose(fp);
	if (doc.HasParseError())
	{
		PRINT(ERROR_RED, "ClientKeyConfig.json 解析错误: %d\n", doc.GetParseError());
		return;
	}
	if (!doc.IsArray())
	{
		PRINT(ERROR_RED, "ClientKeyConfig.json 格式错误: 根节点应为数组\n");
		return;
	}
	int nCount = doc.Size();
	if (nCount > 100) nCount = 100;
	for (int i = 0; i < nCount; i++)
	{
		const rapidjson::Value& item = doc[i];
		if (!item.IsObject()) continue;
		m_ClientKeyConfig[i].Key1 = item.HasMember("Key1") && item["Key1"].IsString() ? (BYTE)std::stoi(item["Key1"].GetString(), nullptr, 16) : 0;
		m_ClientKeyConfig[i].Key2 = item.HasMember("Key2") && item["Key2"].IsString() ? (BYTE)std::stoi(item["Key2"].GetString(), nullptr, 16) : 0;
		m_ClientKeyConfig[i].unknow = item.HasMember("unknow") && item["unknow"].IsString() ? (WORD)std::stoi(item["unknow"].GetString(), nullptr, 16) : 0;
	}
}

BOOL CGameWorld::Init()
{
	// 脚本系统首先读取
	CScriptObjectMgr::GetInstance()->Load(".\\Data\\Script");
	CScriptVariableManager::GetInstance()->StartFind(".\\Data\\Variables", "*.txt", TRUE);
	CScriptObject* pScriptObject = CScriptObjectMgr::GetInstance()->GetScriptObject("system");
	if (pScriptObject)
		CSystemScript::GetInstance()->Init(pScriptObject);
	else
	{
		m_pSystemScriptObject = std::make_unique<CScriptObject>();
		CSystemScript::GetInstance()->Init(m_pSystemScriptObject.get());
	}
	if (!LoadServerConfig())return FALSE;
	CPhysicsMapMgr::GetInstance()->Init(GetName(ENI_PHYSICSMAPPATH), GetName(ENI_PHYSICSCACHEPATH));
	CLogicMapMgr::GetInstance()->Load(".\\Data\\Maps\\Logic");
	// 地图密切相关的东西
	LoadSafeArea();
	LoadStartPoint();
	LoadNotice();
	LoadClientKeyConfig();

	CItemManager::GetInstance()->Load(".\\Data\\Config\\BaseItem.csv");
	CItemManager::GetInstance()->LoadLimit(".\\Data\\Config\\ItemLimit.txt");
	CItemManager::GetInstance()->LoadScriptLink(".\\Data\\ItemScript.txt");
	CItemManager::GetInstance()->LoadPetINI(".\\Data\\Config\\PetLevel.ini");
	CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
	CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv");
	CMagicManager::GetInstance()->LoadMaigcskill(".\\Data\\Config\\MagicSkill.xml");
	CTitleManager::GetInstance()->LoadData(".\\Data\\Config\\Titles.csv");
	CFengHaoGrowManager::GetInstance()->LoadData(".\\Data\\Config\\FengHaoGrow.csv");

	CBundleManager::GetInstance()->LoadBundle(".\\Data\\Config\\BundleItem.csv");

	CNpcManager::GetInstance()->Load(".\\Data\\NpcGens.csv");
	CGmManager::GetInstance()->Load(".\\Data\\GameMaster\\GmList.txt");
	CGmManager::GetInstance()->LoadCommandDef(".\\Data\\GameMaster\\CmdList.txt");

	CGuildManagerEx::GetInstance()->LoadGuildUpExp(".\\Data\\GuildBase\\GuildInfo.ini", GetVar(EVI_GUILDLEVELNUM));
	CGuildManagerEx::GetInstance()->StartFind(".\\Data\\GuildBase\\Guilds", "*.ini", FALSE);

	CMonItemsMgr::GetInstance()->LoadMonItems(".\\Data\\MonItems");
	CMonsterManagerEx::GetInstance()->LoadMonsters(".\\Data\\Monsters");
	CMonsterManagerEx::GetInstance()->LoadMonsterScript(".\\Data\\MonsterScript.txt");
	CMonsterGenManager::GetInstance()->LoadMonGen(".\\Data\\MonGens");
	CMonsterGenManager::GetInstance()->InitAllGen();

	CSandCity::GetInstance()->Init();

	CTopManager::GetInstance()->LoadFigure(".\\Data\\Figure\\TopNpc.txt");
	CTopManager::GetInstance()->Load(".\\Data\\Figure\\TopList.txt");

	CSpecialEquipmentManager::GetInstance()->LoadSpecialEquipmentFunction(".\\Data\\Config\\SpecialItem.txt");
	LoadMineList(".\\Data\\Config\\MineList.txt");

	CMarketManager::GetInstance()->LoadScrollText(".\\Data\\Market\\ScrollText.txt");
	CMarketManager::GetInstance()->LoadMarkets(".\\Data\\Market\\MainDir.txt");
	CAutoScriptManager::GetInstance()->Load(".\\Data\\AutoScript.txt");
	CMapScriptManager::GetInstance()->Load(".\\Data\\MapScript.txt");
	CTaskManager::GetInstance()->Load(".\\Data\\Task");
	CBossTJ::GetInstance()->Load(".\\Data\\Config\\BossTJ.xml");
	CTimeAchieve::GetInstance()->Load(".\\Data\\Config\\TimeAchieve.xml");
	CGameStage::GetInstance()->Load(".\\Data\\Config\\GameStage.xml");
	CBotManager::GetInstance()->CreateBotsFromConfig(".\\Data\\Bot\\BotConfig.csv");

	InitThreadPool(); // 初始化工作线程池
	return TRUE;
}

VOID CGameWorld::InitThreadPool()
{
	if (m_bWorkerRunning) return;
	m_bWorkerRunning = true;
	m_bAsyncUpdateReady = false;
	// 获取系统CPU亲和性掩码
	DWORD_PTR systemMask = 0;
	DWORD_PTR processMask = 0;
	GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask);
	// 计算 systemMask 中置位的位数（可用核心数）
	int activeCoreCount = 0;
	DWORD_PTR tempMask = systemMask;
	while (tempMask)
	{
		activeCoreCount += tempMask & 1;
		tempMask >>= 1;
	}
	// 动态线程数 - 根据CPU核心数决定，上限32
	int cpuCores = std::thread::hardware_concurrency();
	int threadCount = MIN(MAX(cpuCores, 4), 32); // 4-32个线程
	PRINT(ORANGE, "检测到 %d 个CPU核心，初始化 %d 个工作线程\n", cpuCores, threadCount);

	for (int i = 0; i < threadCount; ++i)
	{
		// 计算此线程绑定的核心
		int coreNum = 0;
		int assignedCore = -1;
		for (int j = 0; j < 64; j++)
		{
			if (systemMask & (DWORD_PTR(1) << j))
			{
				if (coreNum == (i % activeCoreCount))
					{
						assignedCore = j;
						break;
					}
				coreNum++;
			}
		}

		auto pThread = std::make_unique<CGameWorkerThread>();
		pThread->SetOwner(this);
		if (pThread->Start())
		{
			// 设置线程核心亲和性
			if (assignedCore >= 0)
			{
				DWORD_PTR affinityMask = DWORD_PTR(1) << assignedCore;
				SetThreadAffinityMask(pThread->getHandle(), affinityMask);
			}
			m_WorkerThreads.push_back(std::move(pThread));
		}
		else
		{
			PRINT(ERROR_RED, "工作线程 %d 启动失败\n", i);
		}
	}
	PRINT(ORANGE, "工作线程池已初始化, 线程数: %d\n", (int)m_WorkerThreads.size());
}

VOID CGameWorld::ShutdownThreadPool()
{
	if (!m_bWorkerRunning) return;
	m_bWorkerRunning = false;
	// 唤醒所有工作线程
	m_WorkerQueueCV.notify_all();
	// 等待所有工作线程结束
	for (auto& pThread : m_WorkerThreads)
	{
		pThread->Terminate();
	}
	m_WorkerQueueCV.notify_all();
	for (auto& pThread : m_WorkerThreads)
	{
		pThread->WaitFor();
	}
	m_WorkerThreads.clear();
	// 清空任务队列
	{
		std::lock_guard<std::mutex> lock(m_WorkerQueueMutex);
		while (!m_WorkerTaskQueue.empty())
		{
			m_WorkerTaskQueue.pop();
		}
	}
	// 清空通告结果队列
	{
		std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
		while (!m_AsyncNoticeQueue.empty())
		{
			m_AsyncNoticeQueue.pop();
		}
	}
}

VOID CGameWorld::PostSystemMessage(const char* pszWords, DWORD dwDelay)
{
	std::array<char, 1024> sztext{};
	int len = EncodeMsg(sztext.data(), 0, SM_SYSCHAT, 0x38ff, 0, 0, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext.data());
}

VOID CGameWorld::PostSystemMessage(DWORD dwColor, const char* pszWords, DWORD dwDelay)
{
	std::array<char, 1024> sztext{};
	int len = EncodeMsg(sztext.data(), 0, SM_SYSCHAT, dwColor & 0xffff, dwColor >> 16, 0, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext.data());
}

VOID CGameWorld::PostSystem10Message(const char* pszWords, DWORD dwDelay)
{
	std::array<char, 1024> sztext{};
	int len = EncodeMsg(sztext.data(), 0, SM_SYSCHAT, 0, 0, 10, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext.data());
}

VOID CGameWorld::Update()
{
	//分帧更新
	DWORD dwkey = (m_dwUpdateKey % 10);
	switch (dwkey)
	{
	case 0: case 4:
	{
		if (m_xUpdateAutoMonsterList.getCount() > 1000)
			UpdateMonsterParallel(m_xUpdateAutoMonsterList, MUT_AUTO, 1000);
		else
			UpdateMonster(m_xUpdateAutoMonsterList, MUT_AUTO);
	}
	break;
	case 1: case 5:
	{
		if (m_xUpdateMonsterList.getCount() > 600)
			UpdateMonsterParallel(m_xUpdateMonsterList, MUT_ACTIVE, 600);
		else
			UpdateMonster(m_xUpdateMonsterList, MUT_ACTIVE);
	}
	break;
	case 2: case 7:
	{
		CSpinBarrier barrier(1);
		SubmitAsyncTask([&barrier]() {
			CTimeSystem::GetInstance()->Update(); // 时间定时器
			barrier.Signal();
			});
		SubmitAsyncTask([&barrier]() {
			CBossTJ::GetInstance()->Update(); // Boss图鉴刷新时间更新
			barrier.Signal();
		});
		SubmitAsyncTask([&barrier]() {
			CEventManager::GetInstance()->UpdateEvents();
			barrier.Signal();
		});
		SubmitAsyncTask([&barrier]() {
			CDownItemMgr::GetInstance()->UpdateDownItem();//更新掉落物品
			barrier.Signal();
		});
		SubmitAsyncTask([&barrier]() {
			CMonsterManagerEx::GetInstance()->UpdateFreeObjects(); // 释放
			barrier.Signal();
		});
		barrier.Arrive();
		CEventManager::GetInstance()->UpdateDeleteObject();
		CDownItemMgr::GetInstance()->UpdateDeletedObject();
		CMonsterManagerEx::GetInstance()->UpdateDeleteMonster(); // 删除
	}
	break;
	case 3: case 8:
	{
		CSpinBarrier barrier(1);
		SubmitAsyncTask([&barrier]() {
			CSandCity* pSandCity = CSandCity::GetInstance();
			if (pSandCity && pSandCity->IsWarStarted())
				CSandCity::GetInstance()->UpdateWar();
			barrier.Signal();
			});
		SubmitAsyncTask([&barrier]() {
			CGuildWarManager::GetInstance()->Update();
			barrier.Signal();
			});
		SubmitAsyncTask([&barrier]() {
			CNpcManager::GetInstance()->Update(); // NPC线程
			barrier.Signal();
			});
		SubmitAsyncTask([&barrier]() {
			CMonsterGenManager::GetInstance()->UpdateGen(); // 刷怪
			barrier.Signal();
		});
		barrier.Arrive();
	}
	break;
	case 6: case 9:
	{
		// 处理异步更新结果（如果有
		if (m_bAsyncUpdateReady) ProcessAsyncUpdateResults();
		SubmitAsyncTask([this]() {
			if (m_iNoticeLines > 0 && m_LineNoticeTimer.IsTimeOut(m_NoticeLines[m_iNoticePtr].dwDelay))
			{
				std::array<char, 1024> sztext{};
				int len = 0;
				DWORD dwDelay = m_NoticeLines[m_iNoticePtr].dwDelay;

				if (m_NoticeLines[m_iNoticePtr].szWords[0] != 0)
				{
					len = EncodeMsg(sztext.data(), 0, 0X64, 0x38ff, 0, 0, (LPVOID)m_NoticeLines[m_iNoticePtr].szWords.data());
					// 将结果放入队列供主线程处理
					std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
					AsyncNoticeResult result{};
					memcpy(result.szText, sztext.data(), sztext.size());
					result.len = len;
					result.dwDelay = 50;
					m_AsyncNoticeQueue.push(result);
					m_bAsyncUpdateReady = true;
				}
				m_iNoticePtr = (m_iNoticePtr + 1) % m_iNoticeLines;
				m_LineNoticeTimer.Savetime();
			}
		});
	}
	break;
	}
	UpdatePlayers(); // 更新所有玩家和机器人
}

VOID CGameWorld::AsyncUpdateWorker()
{
	while (m_bWorkerRunning)
	{
		std::function<VOID()> task;
		{
			std::unique_lock<std::mutex> lock(m_WorkerQueueMutex);
			m_WorkerQueueCV.wait(lock, [this] { return !m_WorkerTaskQueue.empty() || !m_bWorkerRunning; });
			if (!m_bWorkerRunning) break;
			if (!m_WorkerTaskQueue.empty())
			{
				task = std::move(m_WorkerTaskQueue.front());
				m_WorkerTaskQueue.pop();
			}
		}
		if (task)task();
	}
}

VOID CGameWorld::ProcessAsyncUpdateResults()
{
	std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
	int processCount = 0; // 限制每帧最多处理 10 个
	while (!m_AsyncNoticeQueue.empty() && processCount < 10) {
		AsyncNoticeResult result = m_AsyncNoticeQueue.front();
		m_AsyncNoticeQueue.pop();
		// 在主线程中发送消息（避免跨线程直接调用）
		AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, result.dwDelay, 1, result.szText);
		processCount++;
	}
	m_bAsyncUpdateReady = !m_AsyncNoticeQueue.empty(); // 如果还有未处理的, 标记为需要继续处理
}

VOID CGameWorld::SubmitAsyncTask(std::function<VOID()> task)
{
	if (!m_bWorkerRunning) return;
	{
		std::lock_guard<std::mutex> lock(m_WorkerQueueMutex);
		m_WorkerTaskQueue.push(std::move(task));
	}
	m_WorkerQueueCV.notify_one();
}

BOOL CGameWorld::AddGlobeProcess(e_process ident, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4, DWORD dwDelay, int repeattimes, const char* pszString)
{
	OBJECTPROCESS* p = AllocProcess(pszString);
	if (p == nullptr)return FALSE;
	if (!m_xGlobeProcessQueue.push(p))
	{
		FreeProcess(p);
		return FALSE;
	}
	p->dwDelayTime = dwDelay;
	p->dwDeliverTime = CFrameTime::GetFrameTime();
	p->dwParam[0] = dwParam1;
	p->dwParam[1] = dwParam2;
	p->dwParam[2] = dwParam3;
	p->dwParam[3] = dwParam4;
	p->repeattimes = repeattimes;
	p->ident = ident;
	return TRUE;
}

BOOL CGameWorld::AddMapObject(CMapObject* pObject)
{
	MAPOBJECT_LIST* pList = m_pObjectList[pObject->GetType()].get();
	{
		SWLock lock(m_rwObjectListLock);
		if (pList && !pList->addNode(pObject->GetLinkNode(LNI_WORLD))) return FALSE;
	}
	UINT id = pObject->GetMapId();
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(id);
	if (pMap == nullptr || !pMap->AddObject(pObject))
	{
		if (pList)
		{
			SWLock lock(m_rwObjectListLock);
			pList->removeNode(pObject->GetLinkNode(LNI_WORLD));
		}
		return FALSE;
	}
	return TRUE;
}

BOOL CGameWorld::RemoveMapObject(CMapObject* pObject)
{
	CLogicMap* pMap = pObject->GetMap();
	if (pMap != nullptr)
		pMap->RemoveObject(pObject);
	if (pObject->GetType() < OBJ_MAX)
	{
		MAPOBJECT_LIST* pList = m_pObjectList[pObject->GetType()].get();
		if (pList)
		{
			SWLock lock(m_rwObjectListLock);
			pList->removeNode(pObject->GetLinkNode(LNI_WORLD));
		}
	}
	if (pObject->GetType() == OBJ_MONSTER)
	{
		std::array<char, 1024> szBuffer{};
		int length = 0;
		if (((CMonsterEx*)pObject)->GetOutViewmsg(szBuffer.data(), length)) ((CMonsterEx*)pObject)->SendAroundMsg(szBuffer.data(), length);
	}
	return TRUE;
}

BOOL CGameWorld::GetStartPoint(int pro, int& mapid, int& x, int& y)
{
	if (pro < 0 || pro >= 3) return FALSE;
	return TRUE;
}

BOOL CGameWorld::LoadHumanDataDesc(int pro, const char* pszFile)
{
	if (pro >= PRO_MAX) return FALSE;
	CStringFile	sf(pszFile);
	char* Params[100];
	int	nParam = 0;
	int	nLevel = 0;
	if (sf.GetLineCount() == 0) return FALSE;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		nParam = SearchParam(sf[i], Params, 100, ",");
		if (nParam < 26)continue;
		nLevel = atoi(Params[0]);
		if (nLevel > MAX_LEVEL || nLevel < 1)continue;
		HUMANDATADESC* p = &m_HumanData[pro][nLevel - 1];
		p->minac = (BYTE)atoi(Params[1]) & 0xff;
		p->maxac = (BYTE)atoi(Params[2]) & 0xff;
		p->minmac = (BYTE)atoi(Params[3]) & 0xff;
		p->maxmac = (BYTE)atoi(Params[4]) & 0xff;
		p->mindc = (BYTE)atoi(Params[5]) & 0xff;
		p->maxdc = (BYTE)atoi(Params[6]) & 0xff;
		p->minmc = (BYTE)atoi(Params[7]) & 0xff;
		p->maxmc = (BYTE)atoi(Params[8]) & 0xff;
		p->minsc = (BYTE)atoi(Params[9]) & 0xff;
		p->maxsc = (BYTE)atoi(Params[10]) & 0xff;
		p->wHp = (WORD)atoi(Params[11]) & 0xffff;
		p->wMp = (WORD)atoi(Params[12]) & 0xffff;
		p->bagweight = (WORD)atoi(Params[13]) & 0xffff;
		p->bodyweight = (BYTE)atoi(Params[14]) & 0xff;
		p->handweight = (BYTE)atoi(Params[15]) & 0xff;
		p->hitrate = (BYTE)atoi(Params[16]) & 0xff;
		p->escape = (BYTE)atoi(Params[17]) & 0xff;
		p->magicrecover = (BYTE)atoi(Params[18]) & 0xff;
		p->hprecover = (BYTE)atoi(Params[19]) & 0xff;
		p->magescape = (BYTE)atoi(Params[20]) & 0xff;
		p->magicnicety = (BYTE)atoi(Params[21]) & 0xff;
		p->poisonescape = (BYTE)atoi(Params[22]) & 0xff;
		p->poisonnicety = (BYTE)atoi(Params[23]) & 0xff;
		p->huoli = (WORD)atoi(Params[24]) & 0xffff;
		p->dwLevelupExp = (DWORD)strtoul(Params[25], nullptr, 10);
	}
	return TRUE;
}

CAliveObject* CGameWorld::GetAliveObjectById(UINT id)
{
	BYTE btType = id >> 24;
	switch (btType)
	{
	case OBJ_MONSTER:
		return (CAliveObject*)CMonsterManagerEx::GetInstance()->GetMonsterById(id);
	case OBJ_NPC:
		return (CAliveObject*)CNpcManager::GetInstance()->GetScriptNpcById(id);
	case OBJ_PLAYER:
		return (CAliveObject*)CHumanPlayerMgr::GetInstance()->FindbyId(id);
	}
	return nullptr;
}

USERMAGIC* CGameWorld::AllocUserMagic()
{
	USERMAGIC* p = m_UserMagicPool.newObject();
	if (p)
		memset(p, 0, sizeof(*p));
	return p;
}

VOID CGameWorld::FreeUserMagic(USERMAGIC* p)
{
	m_UserMagicPool.deleteObject(p);
}

BOOL CGameWorld::GetBornPoint(int pro, int& mapid, int& x, int& y, char* pszName)
{
	if (pro < 0 || pro >= 3)return FALSE;

	int selectindex = Getrand(m_iBornPointCount[pro]);
	START_POINT* pPoint = m_BornPoints[pro][selectindex];
	o_strncpy(pszName, pPoint->szName, 15);
	return GetValidPointFromStartPoint(pPoint, mapid, x, y);
}

START_POINT* CGameWorld::GetBornPoint(int pro)const
{
	if (pro < 0 || pro >= 3)return nullptr;
	if (m_iBornPointCount[pro] <= 0)return nullptr;
	int selectindex = Getrand(m_iBornPointCount[pro]);
	return m_BornPoints[pro][selectindex];
}

BOOL CGameWorld::GetStartPoint(const char* pszName, int& mapid, int& x, int& y)
{
	START_POINT* pPoint = GetStartPoint(pszName);
	if (!pPoint)pPoint = m_pStartPoints.get();
	if (pPoint == nullptr)return FALSE;
	return GetValidPointFromStartPoint(pPoint, mapid, x, y);
}

VOID CGameWorld::CleanAllMonsters()
{
	// 先收集所有怪物指针，再逐个删除，避免遍历与删除的锁冲突
	std::vector<CMonsterEx*> monsters;
	{
		xListHelper<CMapObject> helper;
		helper.setList(m_pObjectList[OBJ_MONSTER].get());
		SRLock lock(m_rwObjectListLock);
		int totalCount = m_pObjectList[OBJ_MONSTER]->getCount();
		monsters.reserve(totalCount);
		for (CMonsterEx* pMonster = (CMonsterEx*)helper.first(); pMonster != nullptr; pMonster = (CMonsterEx*)helper.next())
		{
			monsters.push_back(pMonster);
		}
	}
	for (CMonsterEx* pMonster : monsters)
	{
		RemoveMapObject((CMapObject*)pMonster);
		CMonsterManagerEx::GetInstance()->DeleteMonster(pMonster);
	}
}

BOOL CGameWorld::GetValidPointFromStartPoint(START_POINT* pPoint, int& map, int& x, int& y, int depth)
{
	if (depth > 10) return FALSE; // 防止无限递归
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(pPoint->mapid);
	if (pMap == nullptr)return FALSE;
	map = pPoint->mapid;
	int rx = Getrand(pPoint->range * 2) - pPoint->range;
	int ry = Getrand(pPoint->range * 2) - pPoint->range;
	rx += pPoint->x;
	ry += pPoint->y;
	if (pMap->IsBlocked(rx, ry))
	{
		POINT	pt;
		if (pMap->GetValidPoint(rx, ry, &pt, 1) > 0)
		{
			x = pt.x;
			y = pt.y;
		}
		else
			return GetValidPointFromStartPoint(pPoint, map, x, y, depth + 1);
	}
	else
	{
		x = rx;
		y = ry;
	}
	return TRUE;
}

VOID CGameWorld::AddUpdateMonster(CMonsterEx* pMonster)
{
	if (pMonster == nullptr || pMonster->IsDeath()) return;
	xListHost<CMonsterEx>::xListNode* pNode = pMonster->getUpdateNode();
	if (pNode == nullptr) return;

	SWLock lock(m_rwMonsterLock);
	if (pMonster->GetRef() == 0)
	{
		if (pNode->BelongTo(&m_xUpdateAutoMonsterList))
			m_xUpdateAutoMonsterList.removeNode(pNode);
	}
	else if (!pNode->BelongTo(&m_xUpdateAutoMonsterList))
		m_xUpdateAutoMonsterList.addNode(pNode);
}

VOID CGameWorld::UpdateMonster(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType)
{
	// 使用thread_local vector避免每帧内存分配
	struct ThreadLocalBuffers {
		std::vector<CMonsterEx*> updateMonsters;
		std::vector<xListHost<CMonsterEx>::xListNode*> nodesToRemove;
		std::vector<xListHost<CMonsterEx>::xListNode*> switchMonsters;
	};
	thread_local ThreadLocalBuffers tls;
	tls.updateMonsters.clear();
	tls.nodesToRemove.clear();
	tls.switchMonsters.clear();
	// 获取目标列表引用
	xListHost<CMonsterEx>& targetList = (updateType == MUT_AUTO) ? m_xUpdateMonsterList : m_xUpdateAutoMonsterList;
	// 收集有效怪物 & 移除死节点
	{
		int totalCount = monsterList.getCount();
		if (totalCount <= 0) return;
		if ((int)tls.updateMonsters.capacity() < totalCount)
			tls.updateMonsters.reserve(totalCount);
		if ((int)tls.nodesToRemove.capacity() < totalCount)
			tls.nodesToRemove.reserve(totalCount);
		xListHost<CMonsterEx>::xListNode* pNode = monsterList.getHead();
		while (pNode)
		{
			CMonsterEx* p = pNode->getObject();
			if (p)
			{
				if (p->IsDeath() || p->GetRef() == 0)
					tls.nodesToRemove.push_back(pNode);
				else
					tls.updateMonsters.push_back(p);
			}
			else
				tls.nodesToRemove.push_back(pNode);
			pNode = pNode->getNext();
		}
		// 删除无效节点
		for (auto node : tls.nodesToRemove)
		{
			monsterList.removeNode(node);
			if (node->BelongTo(&targetList))
				targetList.removeNode(node);
		}
	}
	if (tls.updateMonsters.empty()) return;
	DWORD dwUpdateKey = this->m_dwUpdateKey;
	BOOL checkHasTarget = (updateType == MUT_AUTO);
	// 注意：此处怪物指针的安全性依赖于以下时序保证：
	// 怪物对象的实际删除在主线程的 CMonsterManagerEx::UpdateDeleteMonster() 中执行，
	// 该操作在 UpdateMonster 之后的帧阶段进行，因此 Update 期间指针不会悬垂。
	for (size_t idx = 0; idx < tls.updateMonsters.size(); idx++)
	{
		CMonsterEx* p = tls.updateMonsters[idx];
		if (p->IsDeath() || p->GetRef() == 0) continue; // 复检：可能已被脚本置为死亡
		p->SetUpdateKey(dwUpdateKey);
		p->Update();
		// 怪物可能在 Update() 中自删除，跳过后续成员访问
		if (p->IsDeath() || p->GetRef() == 0) continue;

		BOOL shouldSwitch = checkHasTarget ? (p->GetTarget() != nullptr) : (p->GetTarget() == nullptr);
		if (shouldSwitch)
		{
			xListHost<CMonsterEx>::xListNode* pNode = p->getUpdateNode();
			if (pNode && !pNode->BelongTo(&targetList))
				tls.switchMonsters.push_back(pNode);
		}
	}
	// 量切换列表
	if (!tls.switchMonsters.empty())
	{
		for (const auto& pNode : tls.switchMonsters)
		{
			CMonsterEx* p = pNode->getObject();
			if (!p || p->IsDeath() || p->GetRef() == 0) continue; // 切换前复检
			if (!pNode->BelongTo(&targetList))
				targetList.addNode(pNode);
		}
	}
}

VOID CGameWorld::UpdateMonsterParallel(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType, int nBatchSize)
{
	// 主线程收集所有有效怪物 & 移除死节点（持锁）
	std::vector<CMonsterEx*> validMonsters;
	{
		SWLock lock(m_rwMonsterLock);
		int totalCount = monsterList.getCount();
		if (totalCount <= 0) return;
		validMonsters.reserve(totalCount);

		xListHost<CMonsterEx>& targetList = (updateType == MUT_AUTO) ? m_xUpdateMonsterList : m_xUpdateAutoMonsterList;
		xListHost<CMonsterEx>::xListNode* pNode = monsterList.getHead();
		std::vector<xListHost<CMonsterEx>::xListNode*> deadNodes;
		deadNodes.reserve(totalCount);

		while (pNode)
		{
			CMonsterEx* p = pNode->getObject();
			if (p && !p->IsDeath() && p->GetRef() != 0)
				validMonsters.push_back(p);
			else
				deadNodes.push_back(pNode);
			pNode = pNode->getNext();
		}
		for (auto node : deadNodes)
		{
			monsterList.removeNode(node);
			if (node->BelongTo(&targetList))
				targetList.removeNode(node);
		}
	}
	if (validMonsters.empty()) return;

	// 按批次并行怪物 AI 更新（无锁，每批次独立收集切换节点）
	int totalMonsters = (int)validMonsters.size();
	int numBatches = (totalMonsters + nBatchSize - 1) / nBatchSize;
	DWORD dwUpdateKey = m_dwUpdateKey;
	BOOL checkHasTarget = (updateType == MUT_AUTO);

	// 每个批次有独立的切换节点向量，避免跨线程竞争
	std::vector<std::vector<xListHost<CMonsterEx>::xListNode*>> batchSwitchNodes(numBatches);

	// 预计算目标列表指针，避免工作线程直接读取CGameWorld成员变量（消除benign data race）
	xListHost<CMonsterEx>* pTargetList = (updateType == MUT_AUTO) ? &m_xUpdateMonsterList : &m_xUpdateAutoMonsterList;

	CSpinBarrier barrier(numBatches);
	for (int batch = 0; batch < numBatches; batch++)
	{
		int start = batch * nBatchSize;
		int end = MIN((batch + 1) * nBatchSize, totalMonsters);
		SubmitAsyncTask([&barrier, &validMonsters, &batchSwitchNodes, pTargetList, start, end, dwUpdateKey, checkHasTarget, batch]() {
			auto& switchNodes = batchSwitchNodes[batch];
			xListHost<CMonsterEx>& targetList = *pTargetList;
			for (int i = start; i < end; i++)
			{
				CMonsterEx* p = validMonsters[i];
				if (p->IsDeath() || p->GetRef() == 0) continue; // 复检：锁释放后可能已死亡
				p->SetUpdateKey(dwUpdateKey);
				p->Update();
				// 怪物可能在 Update() 中自删除（m_pDesc==nullptr → DeleteMonster），
				// 此时应跳过后续成员访问，防止使用已标记删除的对象
				if (p->IsDeath() || p->GetRef() == 0) continue;

				BOOL shouldSwitch = checkHasTarget ? (p->GetTarget() != nullptr) : (p->GetTarget() == nullptr);
				if (shouldSwitch)
				{
					xListHost<CMonsterEx>::xListNode* pNode = p->getUpdateNode();
					if (pNode && !pNode->BelongTo(&targetList))
						switchNodes.push_back(pNode);
				}
			}
			barrier.Signal();
		});
	}
	// 3秒超时：若工作线程异常未能Signal，避免主线程永久卡死
	if (!barrier.Arrive(3000))
	{
		assert(!"UpdateMonsterParallel: worker tasks did not complete within 3s timeout");
	}

	// 主线程合并各批次切换节点，批量列表迁移（持锁）
	{
		SWLock lock(m_rwMonsterLock);
		xListHost<CMonsterEx>& targetList = (updateType == MUT_AUTO) ? m_xUpdateMonsterList : m_xUpdateAutoMonsterList;
		for (int batch = 0; batch < numBatches; batch++)
		{
			for (auto pNode : batchSwitchNodes[batch])
			{
				CMonsterEx* p = pNode->getObject();
				if (!p || p->IsDeath() || p->GetRef() == 0) continue; // 切换前复检
				if (!pNode->BelongTo(&targetList))
					targetList.addNode(pNode);
			}
		}
	}
}

VOID CGameWorld::UpdatePlayers()
{
	OBJECTPROCESS* pProcesses[UPDATE_GLOBE_PROCESS_COUNT] = { nullptr };
	int	globeprocesscount = 0;
	int i = 0;
	while (globeprocesscount < UPDATE_GLOBE_PROCESS_COUNT && (pProcesses[globeprocesscount] = m_xGlobeProcessQueue.pop()))
		globeprocesscount++;
	MAPOBJECT_LIST* pList = m_pObjectList[OBJ_PLAYER].get();
	// 使用静态缓冲区避免每次重新分配
	struct ThreadLocalPlayerBuffers {
		std::vector<CHumanPlayer*> players;
	};
	thread_local ThreadLocalPlayerBuffers ptls;
	ptls.players.clear();
	{
		SRLock lock(m_rwObjectListLock);
		for (auto pNode = pList->getHead(); pNode != nullptr; pNode = pNode->getNext())
		{
			CHumanPlayer* p = (CHumanPlayer*)pNode->getObject();
			if (p) ptls.players.push_back(p);
		}
	}
	if (ptls.players.empty())
	{
		for (i = 0; i < globeprocesscount; i++)
		{
			OBJECTPROCESS* pProceess = pProcesses[i];
			if (pProceess && pProceess->ident < EP_MAX)
				FreeProcess(pProceess);
		}
		return;
	}
	DWORD dwUpdateKey = this->m_dwUpdateKey;
	// 提前判断是否需要处理全局消息
	BOOL needGlobeProcess = (globeprocesscount > 0);
	// 定义处理单个玩家的逻辑
	// 线程安全说明：按地图分组确保同一地图的玩家在同一线程处理，
	// 避免跨地图共享状态冲突。ProcessPlayer内不应访问其他地图对象。
	auto ProcessPlayer = [needGlobeProcess, pProcesses, globeprocesscount, dwUpdateKey](CHumanPlayer* p) {
		if (needGlobeProcess && p->NeedGlobeProcess(UPDATE_GLOBE_PROCESS_COUNT))
		{
			for (int j = 0; j < globeprocesscount; j++)
			{
				OBJECTPROCESS* pProceess = pProcesses[j];
				if (pProceess && pProceess->ident < EP_MAX)
					p->AddProcess(pProceess);
			}
		}
		p->SetUpdateKey(dwUpdateKey);
		p->Update();
	};
	// 按地图分组进行并行处理
	struct MapGroup {
		std::vector<CHumanPlayer*> players;
	};
	// 使用thread_local复用，避免每帧堆分配
	struct MapGroupBuffers
	{
		std::vector<MapGroup> groups;
		std::unordered_map<UINT, size_t> mapIndex;
		VOID clear()
		{
			for (auto& g : groups) g.players.clear();
			groups.clear();
			mapIndex.clear();
		}
	};
	thread_local MapGroupBuffers mgBuf;
	mgBuf.clear();
	auto& groups = mgBuf.groups;
	auto& mapIndex = mgBuf.mapIndex;
	{
		for (auto* p : ptls.players)
		{
			UINT mapId = p->GetMapId();
			auto it = mapIndex.find(mapId);
			if (it == mapIndex.end())
			{
				size_t idx = groups.size();
				mapIndex[mapId] = idx;
				MapGroup g;
				g.players.push_back(p);
				groups.push_back(std::move(g));
			}
			else
			{
				groups[it->second].players.push_back(p);
			}
		}
	}
	if (groups.size() > 1 && m_WorkerThreads.size() >= 4)
	{
		// 单个地图玩家数低于此阈值时，异步分发的开销大于收益，直接由主线程处理
		static const size_t MIN_PLAYERS_PER_ASYNC_GROUP = 5;
		// 使用原子计数 + 自旋等待
		// 帧内同步等待时间极短（通常 < 1ms），自旋避免内核态切换开销
		CSpinBarrier barrier((int)groups.size());
		// 主线程采用 work-stealing 策略：处理一个较大的分组，避免空等
		int mainThreadGroupIdx = -1;
		for (size_t i = 0; i < groups.size(); i++)
		{
			auto& group = groups[i];
			if (group.players.size() < MIN_PLAYERS_PER_ASYNC_GROUP)
			{
				// 小分组：直接由主线程处理，省去异步队列的锁竞争和上下文切换开销
				for (auto* p : group.players)
				{
					ProcessPlayer(p);
				}
				barrier.Signal();
			}
			else if (mainThreadGroupIdx < 0)
			{
				// 主线程偷取第一个大分组自己处理（work-stealing）
				mainThreadGroupIdx = (int)i;
			}
			else
			{
				// 其余大分组提交到工作线程并行处理
				SubmitAsyncTask([&barrier, &group, &ProcessPlayer]() {
					for (auto* p : group.players)
					{
						ProcessPlayer(p);
					}
					barrier.Signal();
				});
			}
		}
		// 主线程处理偷取的大分组
		if (mainThreadGroupIdx >= 0)
		{
			for (auto* p : groups[mainThreadGroupIdx].players)
			{
				ProcessPlayer(p);
			}
			barrier.Signal();
		}
		// 自旋等待所有工作线程完成（无需内核态切换）
		barrier.Arrive();
	}
	else
	{
		// 单地图或 worker 数量不足时，直接同步处理，避免不必要的分发开销
		for (auto& group : groups)
		{
			for (auto* p : group.players)
			{
				ProcessPlayer(p);
			}
		}
	}
	for (i = 0; i < globeprocesscount; i++)
	{
		OBJECTPROCESS* pProceess = pProcesses[i];
		if (pProceess && pProceess->ident < EP_MAX)
			FreeProcess(pProceess);
	}
}