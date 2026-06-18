#include "StdAfx.h"
#include ".\scriptfunction.h"
#include ".\cmdproc.h"
#include ".\scriptview.h"
#include ".\scripttarget.h"
#include ".\scriptshell.h"
#include ".\Humanplayer.h"
#include ".\ScriptElement.h"
#include ".\ScriptObject.h"
#include "sandcity.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "StringListManager.h"
#include "guildEx.h"
#include "SCDoor.h"
#include "LogicMapMgr.h"
#include "logicmap.h"
#include "monstergenmanager.h"
#include "GroupObject.h"
#include "topmanager.h"
#include "gameworld.h"
#include "magicmanager.h"
#include "humanplayermgr.h"
#include "npcmanager.h"
#include "scriptnpc.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "scriptobjectmgr.h"
#include "gmmanager.h"
#include "downitemmgr.h"
#include "guildmanagerex.h"
#include "marketmanager.h"
#include "guildwarmanager.h"
#include "autoscriptmanager.h"
#include "monitemsmgr.h"
#include "TriggerEvent.h"
#include "FengHaoGrowManager.h"

extern DWORD g_dwActionDelay[AT_MAX];
//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：创建掉落物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DROPITEM) {
	if (nParam >= 1)
	{
		int count = 1;
		if (nParam >= 2)
			count = Params[1].nParam;
		if (count == 0)count = 1;
		for (int i = 0; i < count; i++)
		{
			ITEM item;
			if (CItemManager::GetInstance()->CreateTempItem(Params[0].pszParam, item, FALSE))
			{
				pPlayer->DropItem(item);
			}
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：踢指定玩家下线
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(KICK) {
	if (nParam >= 1)
	{
		if (Params[0].pszParam[0] == '*' && Params[0].pszParam[1] == 0)
		{
			CServer::GetInstance()->KickAll();
			return 1;
		}
		CHumanPlayer* pPlayer1 = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (pPlayer1)
		{
			CClientObj* pObj = pPlayer1->GetClientObject();
			if (pObj)
			{
				if (nParam >= 2)
					pPlayer1->SaySystem("你被GM踢下线了, 原因是 %s, 10秒钟后断开连接!",
						Params[1].pszParam);
				else
					pPlayer1->SaySystem("你被GM踢下线了, 10秒钟后断开连接!");
				pObj->Disconnect(10000);
			}
		}
	}
	else
	{
		CClientObj* pObj = pPlayer->GetClientObject();
		if (pObj)
		{
			pPlayer->SaySystem("你被禁止进入游戏, 3秒钟后断开连接!");
			pObj->Disconnect(3000);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：开启天地合一
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ALLOWSPECIALPOWER) {
	if (!pPlayer->IsSystemFlagSeted(SF_ALLOWSPECIALPOWER))
	{
		pPlayer->SetSystemFlag(SF_ALLOWSPECIALPOWER, TRUE);
		pPlayer->SaySystem("开启天地合一!");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置玩家是否是大背包
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETBIGBAG) {
	if (nParam != 1)return FALSE;
	if (Params[0].nParam == 0)
		pPlayer->SetBagCountLimit(SMALLBAG_SLOT);
	else
		pPlayer->SetBagCountLimit(BIGBAG_SLOT);
	pPlayer->SendMsg(pPlayer->GetId(), 0x9594, 0, pPlayer->GetBagCountLimit(), 0);//38292发送背包大小
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送聊天消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDSINGLEMSG) {
	if (nParam != 2)return FALSE;
	thread_local char szMsg[65536]; // 使用thread_local复用缓冲区，避免每帧64KB堆分配
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p != nullptr)
	{
		int size = GetMsgFromString(Params[0].pszParam, szMsg);
		if (size > 0)
			p->OnAroundMsg(pPlayer, szMsg, size);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送聊天消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDMSG) {
	if (nParam != 1)return FALSE;
	thread_local char szMsg[65536]; // 使用thread_local复用缓冲区，避免每帧64KB堆分配
	int size = GetMsgFromString(Params[0].pszParam, szMsg);
	if (size > 0)
		pPlayer->OnAroundMsg(pPlayer, szMsg, size);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加玩家技能
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDMAGIC) {
	if (nParam > 2 || nParam == 0) return FALSE;
	if (nParam == 1)
	{
		if (Params[0].pszParam[0] != 0 && Params[0].nParam == 0)
			return pPlayer->AddMagic(Params[0].pszParam);
		else
			return pPlayer->AddMagic(Params[0].nParam);
	}
	else if (nParam == 2)
	{
		if (Params[0].pszParam[0] != 0 && Params[0].nParam == 0)
			return pPlayer->AddMagic(Params[0].pszParam, Params[1].nParam);
		else
			return pPlayer->AddMagic(Params[0].nParam, Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除玩家技能
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REMOVEMAGIC) {
	if (nParam != 1)return FALSE;
	if (Params[0].pszParam[0] != 0 && Params[0].nParam == 0)
		return pPlayer->RemoveMagic(Params[0].pszParam);
	else
		return pPlayer->RemoveMagic(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否有某技能
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASMAGIC) {
	if (nParam != 1)return 0;
	if (Params[0].pszParam[0] != 0 && Params[0].nParam == 0)
		return (pPlayer->GetMagicByName(Params[0].pszParam) != nullptr);
	else
		return (pPlayer->GetMagic(Params[0].nParam) != nullptr);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家的技能等级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKMAGICLEVEL) {
	if (nParam < 3)return FALSE;
	USERMAGIC* p = nullptr;
	if (Params[0].pszParam[0] != 0 && Params[0].nParam == 0)
		p = pPlayer->GetMagicByName(Params[0].pszParam);
	else
		p = pPlayer->GetMagic(Params[0].nParam);
	if (p == nullptr)return FALSE;
	return CompareValue(p->magic.btLevel, Params[1].pszParam, Params[2].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加NPC
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDNPC) {
	if (nParam != 1)return 0;
	CNpcManager::GetInstance()->AddNpc(Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：加载指定脚本PAGE
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDSCRIPTPAGE) {
	if (nParam != 1)return 0;
	CScriptObjectMgr::GetInstance()->OnFoundFile(Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：玩家自杀
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(NOWDEATH) {
	pPlayer->ToDeath();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：召唤怪
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CALLMON) {
	if (nParam >= 1)
	{
		BOOL bSetOwner = FALSE;
		BOOL b_Callmon = TRUE;  //注意这个问题, 
		if (*Params[0].pszParam == '!')
		{
			Params[0].pszParam++;
			bSetOwner = TRUE;
		}
		UINT nCount = 1;
		if (nParam > 1)
			nCount = Params[1].nParam > 0 ? Params[1].nParam : 1;
#ifdef	_DEBUG
		UINT rnCount = pPlayer->GetMap() ? pPlayer->GetMap()->CountAllMonsters() : 0;
#endif
		ITEM* pItem = nullptr;
		int temp = 0;
		pItem = pPlayer->GetUsingItem();
		if (pItem != nullptr && strcmp(pItem->baseitem.szName, "豹魔石") == 0)
		{
			time_t t;
			time(&t);
			DWORD dwT2 = (DWORD)t;
			if (!pItem->IsBind()) // 绑定灵兽
			{
				pItem->SetBind(TRUE);
				// 豹魔石绑定时间
				*reinterpret_cast<DWORD*>(&pItem->btItemExt[277]) = dwT2;
				// 喂养时间
				pItem->SetPetTime();
				pPlayer->SendUpdateItem(*pItem);
			}

			pItem->SetExName("丛林豹");
			pPlayer->SendPetName(pItem);
			
			// 判断喂养时间
			temp = (dwT2 - pItem->GetPetTime()) / 86400;
			if (temp > 4)
			{
				b_Callmon = FALSE; // 大于4饿死
				pPlayer->SaySystemAttrib(CC_REDPET, "你的宠物已经饿死,你可以带豹神水请驯兽师帮你复活.");
			}
		}
		for (UINT i = 0; i < nCount; i++)
		{
			if (b_Callmon)
				pPlayer->SummonPet(Params[0].pszParam, bSetOwner);
		}
		if (pPlayer->IsHasPet() && pItem != nullptr)
		{
			if (temp <= 2)
				pPlayer->SaySystemAttrib(CC_REDPET, "你的从林豹精神饱满!");
			else if (temp <= 3)
				pPlayer->SaySystemAttrib(CC_REDPET, "你的从林豹处于微饿状态!");
			else if (temp <= 4)
				pPlayer->SaySystemAttrib(CC_REDPET, "你的从林豹处于饥饿状态!");
		}
#ifdef	_DEBUG
		PRINT(SUCCESS_GREEN, "召唤前怪数量:%u, 召唤后怪数量:%u\n", rnCount, pPlayer->GetMap() ? pPlayer->GetMap()->CountAllMonsters() : 0 );
#endif
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：游戏管理
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GAMEMASTER){
	DWORD dwGmLevel = 0;
	if (pPlayer->IsGameMaster())
	{
		dwGmLevel = pPlayer->GetSystemFlagParam(SF_GAMEMASTER);
	}
	if (dwGmLevel == 0)
	{
		dwGmLevel = (DWORD)CGmManager::GetInstance()->GetGmLevel(pPlayer->GetAccount());
		if (dwGmLevel > 0)
		{
			pPlayer->SetSystemFlag(SF_GAMEMASTER, TRUE, dwGmLevel);
			pPlayer->SaySystem("进入GM模式!");
		}
		else
		{
			pPlayer->SaySystem("对不起, 你不是本服务器的GM!");
			return 0;
		}
	}
	else
	{
		pPlayer->SetSystemFlag(SF_GAMEMASTER, FALSE);
		pPlayer->SaySystem("退出GM模式!");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整玩家等级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETLEVEL) {
	if (nParam == 1)
	{
		pPlayer->SetExp(0);
		pPlayer->LevelUp(Params[0].nParam);
	}
	else {
		CHumanPlayer* pFindPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		pFindPlayer->SetExp(0);
		pFindPlayer->LevelUp(Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：滚动文字模式
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SCROLLTEXTMODE) {
	if (!pPlayer->IsSystemFlagSeted(SF_SCROLLTEXTMODE))
	{
		pPlayer->SaySystem("进入滚动文字模式, 现在您说的话将会以滚动文字的形式发往所有的客户端!");
		pPlayer->SetSystemFlag(SF_SCROLLTEXTMODE, TRUE);
	}
	else
	{
		pPlayer->SaySystem("退出滚动文字模式, 恢复正常聊天功能!");
		pPlayer->SetSystemFlag(SF_SCROLLTEXTMODE, FALSE);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载配置
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADCONFIG) {
	if (nParam >= 1)
	{
		if (_stricmp(Params[0].pszParam, "serverconfig") == 0)
		{
			CGameWorld::GetInstance()->LoadServerConfig();
			pPlayer->SaySystem("重新加载服务器配置完成!");
		}
		else if (_stricmp(Params[0].pszParam, "item") == 0)
		{
			CItemManager::GetInstance()->ClearItemData();
			CItemManager::GetInstance()->Load(".\\Data\\Config\\BaseItem.csv");
			pPlayer->SaySystem("重新加载物品数据完成!");
		}
		else if (_stricmp(Params[0].pszParam, "monster") == 0)
		{
			CMonsterManagerEx::GetInstance()->ClearMonsterData();
			CMonsterManagerEx::GetInstance()->LoadMonsters(".\\Data\\Monsters");
			pPlayer->SaySystem("重新加载怪物数据完成!");
		}
		else if (_stricmp(Params[0].pszParam, "skill") == 0)
		{
			CMagicManager::GetInstance()->ClearMagicData();
			CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
			CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv");
			CMagicManager::GetInstance()->LoadMaigcskill(".\\Data\\Config\\MagicSkill.xml");
			// 重新加载技能数据后, 需要更新所有在线玩家的技能指针
			CMagicManager::GetInstance()->ReloadAllPlayerSkills();
			pPlayer->SaySystem("重新加载技能数据完成!");
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置玩家状态
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETSTATUS) {
	if (nParam == 3)
		pPlayer->SetStatus(Params[0].nParam, Params[1].nParam, Params[2].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：清除玩家状态
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLRSTATUS) {
	if (nParam == 1)
		pPlayer->ClrStatus(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：宠物休息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REST) {
	if (pPlayer->GetPetCount() == 0)
		return FALSE;
	pPlayer->SetPetsActive();
	if (pPlayer->IsPetsActive())
		pPlayer->SaySystem("宠物行动!");
	else
		pPlayer->SaySystem("宠物休息!");
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：回家
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HOME) {
	pPlayer->Home();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：不死状态
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(NODEAD) {
	if (pPlayer->IsNoDead())
		pPlayer->SaySystem("取消不死状态!");
	else
		pPlayer->SaySystem("进入不死状态!");
	pPlayer->SetNoDead(!pPlayer->IsNoDead());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：无敌状态
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(NODAMAGE) {
	if (pPlayer->IsNoDamage())
		pPlayer->SaySystem("取消无敌状态!");
	else
		pPlayer->SaySystem("进入无敌状态!");
	pPlayer->SetNoDamage(!pPlayer->IsNoDamage());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加魔法值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDMP) {
	if (nParam == 1)
		pPlayer->AddMp(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：一击必杀
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SUPERHIT) {
	if (pPlayer->IsSuperHit())
	{
		pPlayer->SaySystem("退出一击必杀模式!");
		pPlayer->SetSuperHit(FALSE);
	}
	else
	{
		pPlayer->SaySystem("进入一击必杀模式!");
		pPlayer->SetSuperHit(TRUE);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：地图信息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MAPINFO) {
	CLogicMap* pMap = pPlayer->GetMap();
	if (pMap)
	{
		pPlayer->SaySystem("---===[%s.nmp 地图信息]===---", pMap->GetName());
		pPlayer->SaySystem("配置序号: %d", pMap->GetIndex());
		pPlayer->SaySystem("地面物品: %d 个", pMap->GetObjectCount(OBJ_DOWNITEM));
		pPlayer->SaySystem("怪物个数: %d 个", pMap->GetObjectCount(OBJ_MONSTER));
		pPlayer->SaySystem("ＮＰＣ数: %d 个", pMap->GetObjectCount(OBJ_NPC));
		pPlayer->SaySystem("玩家个数: %d 个", pMap->GetObjectCount(OBJ_PLAYER));
		pPlayer->SaySystem("地面事件: %d 个", pMap->GetObjectCount(OBJ_EVENT));
		pPlayer->SaySystem("地面特效: %d 个", pMap->GetObjectCount(OBJ_VISIBLEEVENT));
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：服务器信息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SERVERINFO) {
	int u, f, t;
	DWORD a, b, c, d;
	pPlayer->SaySystem("---===[服务器信息]===---");
	pPlayer->SaySystem("地面物品: %d 个", CDownItemMgr::GetInstance()->getCount());
	pPlayer->SaySystem("怪物数量: %d 个", CMonsterManagerEx::GetInstance()->getCount());
	pPlayer->SaySystem("ＮＰＣ数: %d 个", CNpcManager::GetInstance()->getCount());
	pPlayer->SaySystem("玩家数量: %d 个", CHumanPlayerMgr::GetInstance()->getCount());
	pPlayer->SaySystem("地图数量: %d 个", CLogicMapMgr::GetInstance()->getCount());
	CLogicMap::GetCellInfoInfo(u, f, t);
	pPlayer->SaySystem("地图格子: used %d freed %d total %d", u, f, t);
	CAliveObject::GetVisibleObjectInfo(u, f, t);
	pPlayer->SaySystem("可视连接: used %d freed %d total %d", u, f, t);
	CItemManager::GetInstance()->GetMiscCount(a, b, c, d);
	pPlayer->SaySystem("物品数量: Temp %u Create %u Delete %u Ident %u", a, b, c, d);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改攻击模式
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ATTACKMODE) {
	if (nParam == 1)
		pPlayer->ChangeAttackMode(Params[0].nParam);
	else
		pPlayer->ChangeAttackMode(-1);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：频道信息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CCINFO) {
	pPlayer->ChangeChatChannel(pPlayer->GetChatChannel());
	pPlayer->SaySystemAttrib(CC_GREEN, "频道控制命令 @开通频道 @关闭频道");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：开启频道
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ENABLECHANNEL) {
	if (nParam >= 1)
	{
		e_chatchannel channel = GetChannelFromString(Params[0].pszParam);
		pPlayer->EnableChannel(channel);
	}
	else
		pPlayer->EnableChannel();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：禁止频道
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DISABLECHANNEL) {
	if (nParam >= 1)
	{
		e_chatchannel channel = GetChannelFromString(Params[0].pszParam);
		pPlayer->DisableChannel(channel);
	}
	else
		pPlayer->DisableChannel();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：禁止全行会聊天
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DISABLEGUILDALLSAY) {
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(pPlayer))
	{
		pGuild->SetAllNoSay();
		if (!pGuild->IsAllNoSay())
			pPlayer->SaySystemAttrib(CC_GREENS, "[允许全行会聊天]");
		else
			pPlayer->SaySystemAttrib(CC_GREENS, "[禁止全行会聊天]");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：禁止行会聊天
//		注释：@禁止行会聊天 玩家名字
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DISABLEGUILDSAY) {
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(pPlayer))
	{
		const char* pszCharName = Params[0].pszParam;
		pGuild->SetNoSay(pszCharName);
		if (!pGuild->IsNoSay(pszCharName))
			pPlayer->SaySystemAttrib(CC_GREENS, "[允许 %s 行会聊天]", pszCharName);
		else
			pPlayer->SaySystemAttrib(CC_GREENS, "[禁止 %s 行会聊天]", pszCharName);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：创建行会
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CREATEGUILD) {
	if (nParam == 1)
	{
		if (CGuildManagerEx::GetInstance()->BuildGuild(pPlayer, Params[0].pszParam))
			pPlayer->UpdateViewName();
		else
			pPlayer->SaySystem("创建门派失败～");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置动作延时
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETACTIONDELAY) {
	if (nParam == 2)
	{
		actiontype type = GetActionType(Params[0].pszParam);
		if (type != AT_MAX)
		{
			g_dwActionDelay[type] = Params[1].nParam;
			pPlayer->SaySystem("%s 设置为 %u", Params[0].pszParam, g_dwActionDelay[type]);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示动作时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWACTIONDELAY) {
	if (nParam == 1)
	{
		actiontype type = GetActionType(Params[0].pszParam);
		if (type != AT_MAX)
			pPlayer->SaySystem("%s 设置为 %u", Params[0].pszParam, g_dwActionDelay[type]);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：退出行会
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(QUITGUILD) {
	if (pPlayer->GetGuild())
	{
		if(pPlayer->GetGuild()->RemoveMember(pPlayer->GetName()))
			pPlayer->UpdateViewName();
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载公告
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADNOTICE) {
	CGameWorld::GetInstance()->LoadNotice();
	pPlayer->SaySystem("重新加载公告完成!");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：系统消息模式
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(NOTICEMODE) {
	if (!pPlayer->IsSystemFlagSeted(SF_NOTICEMODE))
	{
		pPlayer->SetSystemFlag(SF_NOTICEMODE, TRUE);
		pPlayer->SaySystem("进入系统消息模式, 您输入的每一句话, 将以系统消息的形式发往所有客户端!");
	}
	else
	{
		pPlayer->SetSystemFlag(SF_NOTICEMODE, FALSE);
		pPlayer->SaySystem("退出系统消息模式!");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：开始攻城战
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STARTSABUKWAR) {
	CSandCity* pSandCity = CSandCity::GetInstance();
	if (!pSandCity->IsWarStarted())
	{
		if (!pSandCity->StartWar())
			pPlayer->SaySystem("开始攻城战失败: %s", pSandCity->getErrorMsg());
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：停止攻城战
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STOPSABUKWAR) {
	CSandCity* pSandCity = CSandCity::GetInstance();
	if (pSandCity->IsWarStarted())
		pSandCity->EndWar();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：打开沙城大门
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(OPENSABUKDOOR) {
	CSandCity::GetInstance()->OpenGate();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：关闭沙城大门
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLOSESABUKDOOR) {
	CSandCity::GetInstance()->CloseGate();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修理沙城大门
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REPAIRSABUKDOOR) {
	CSandCity::GetInstance()->RepairGate();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADSCRIPT) {
	if (nParam == 1)
	{
		CScriptObject* pObject = CScriptObjectMgr::GetInstance()->GetScriptObject(Params[0].pszParam);
		if (pObject)
		{
			if (pObject->Reload())
				pPlayer->SaySystem("重新加载 %s.txt 脚本成功!", Params[0].pszParam);
			else
				pPlayer->SaySystem("重新加载 %s.txt 脚本失败!", Params[0].pszParam);
		}
		else
			pPlayer->SaySystem("系统中不存在名字为 %s 的脚本对象, 无法重新加载!", Params[0].pszParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修理沙城皇宫墙
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REPAIRSABUKWALL) {
	if (nParam == 1)
	{
		int index = Params[0].nParam;
		CSandCity::GetInstance()->RepairWall(index);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置沙城拥有者
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETSABUKOWNER) {
	if (nParam < 1) return FALSE;
	CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
	if (pGuild)
	{
		CSandCity::GetInstance()->ChangeOwner(pGuild);
		CSCDoor* scDoor = CSandCity::GetInstance()->GetMainGate();
		if (scDoor) scDoor->Open();
		return TRUE;
	}
	else
		pPlayer->SaySystem("您指定的行会不存在!");
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示NPC
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWNPC) {
	UINT nMapId = 0;
	if (nParam == 1)
		nMapId = Params[0].nParam;
	else
		nMapId = pPlayer->GetMapId();
	CGameWorld::GetInstance()->ShowNpc(nMapId);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：隐藏NPC
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HIDENPC) {
	UINT nMapId = 0;
	if (nParam == 1)
		nMapId = Params[0].nParam;
	else
		nMapId = pPlayer->GetMapId();
	CGameWorld::GetInstance()->HideNpc(nMapId);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加攻城请求
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDATTACKREQUEST) {
	if (nParam >= 1)
	{
		CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
		if (pGuild == nullptr)
			pPlayer->SaySystem("找不到 %s 行会!", Params[0].pszParam);
		else
		{
			if (!CSandCity::GetInstance()->AddAttackRequest(pGuild, nParam >= 2 ? TRUE : FALSE))
				pPlayer->SaySystem("添加攻城请求失败, %s", CSandCity::GetInstance()->getErrorMsg());
			else
				pPlayer->SaySystem("添加攻城请求成功!");
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：完全隐身
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SUPERHIDE) {
	if (pPlayer->IsSystemFlagSeted(SF_HIDED))
	{
		pPlayer->SetSystemFlag(SF_HIDED, FALSE);
		pPlayer->SaySystem("离开完全隐身状态!");
		pPlayer->Show();
	}
	else
	{
		pPlayer->Hide();
		pPlayer->SetSystemFlag(SF_HIDED, TRUE);
		pPlayer->SaySystem("进入完全隐身状态!");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：传送
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TELEPORT) {
	if (nParam == 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
			p->FlyTo(pPlayer->GetMap(), pPlayer->getX(), pPlayer->getY());
		else
			pPlayer->SaySystem("%s 不在线!", Params[0].pszParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示玩家信息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWINFO) {
	if (nParam == 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			pPlayer->SaySystem("%s(%s)的详细信息：", Params[0],
				p->GetClientObject() == nullptr ? "0.0.0.0" : p->GetClientObject()->getAddress());
			pPlayer->SaySystem("账号: %s 位置: [%s]( %u, %u ) 行会: %s(%s)",
				p->GetAccount(), p->GetMap() == nullptr ? "无" : p->GetMap()->GetName(), p->getX(), p->getY(),
				p->GetGuild() == nullptr ? "无" : p->GetGuild()->GetName(), p->GetGuildTitle());
			char	szLoginLong[200];
			o_strncpy(szLoginLong, p->GetScriptVarValue("LOGINLONG"), 40);
			pPlayer->SaySystem("登录时间: %s 在线时长: %s",
				p->GetScriptVarValue("LOGINTIME"), szLoginLong);
			pPlayer->SaySystem("HP: %u/%u MP: %u/%u 金钱: %u 元宝: %u",
				p->GetPropValue(PI_CURHP), p->GetPropValue(PI_MAXHP),
				p->GetPropValue(PI_CURMP), p->GetPropValue(PI_MAXMP),
				p->GetMoney(MT_GOLD), p->GetMoney(MT_YUANBAO));
			pPlayer->SaySystem("攻击: %u-%u 魔法: %u-%u 精神: %u-%u 防御: %u-%u 魔防: %u-%u",
				p->GetPropValue(PI_MINDC), p->GetPropValue(PI_MAXDC), p->GetPropValue(PI_MINMC),
				p->GetPropValue(PI_MAXMC), p->GetPropValue(PI_MINSC), p->GetPropValue(PI_MAXSC),
				p->GetPropValue(PI_MINAC), p->GetPropValue(PI_MAXAC), p->GetPropValue(PI_MINMAC), p->GetPropValue(PI_MAXMAC));
			xPacketPool::ScopedPacket packet(65536);
			p->GetViewDetail(*packet);
			pPlayer->SendMsg(p->GetId(), 0x2ef, 0, MAKEWORD(0, _U_MAX), MAKEWORD(0, 1), (LPVOID)packet->getbuf(), packet->getsize());
		}
		else
			pPlayer->SaySystem("%s 不在线!", Params[0].pszParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置玩家的师父
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETMASTER) {
	if (nParam == 0) return pPlayer->LeaveTeacher();
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p) return p->AddStudent(pPlayer);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置玩家老师
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETTEACHER) {
	if (nParam == 0) return FALSE;
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p)
		return p->AddStudent(pPlayer);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：离开老师
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(LEAVETEACHER) {
	return pPlayer->LeaveTeacher();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置夫妻
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETWIFE) {
	if (nParam == 0)
		return pPlayer->UnMarry();
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p) return pPlayer->Marry(p);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：结婚
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MARRY) {
	if (nParam == 0) return FALSE;
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p) return pPlayer->Marry(p);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：解除婚姻
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(UNMARRY) {
	return pPlayer->UnMarry();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加好友
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDFRIEND) {
	if (nParam == 1) 
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p) pPlayer->AddFriend(p);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除好友
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELETEFRIEND) {
	if (nParam == 1)
		pPlayer->DeleteFriend(Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：同意加好友
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ALLOWFRIEND) {
	pPlayer->ToggleFriendMode();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置技能等级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETMAGICLEVEL) {
	if (nParam == 2)
	{
		if (pPlayer->SetMagicLevel(Params[0].pszParam, Params[1].nParam))
			pPlayer->SaySystem("设置技能等级成功!");
		else
			pPlayer->SaySystem("设置技能等级失败!");
	}
	else
		if (nParam == 3)
		{
			CHumanPlayer* player = pPlayer;
			player = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
			if (!player->SetMagicLevel(Params[1].pszParam, Params[2].nParam))
				pPlayer->SaySystem("对 %s 设置技能等级失败!", Params[0].pszParam);
			else
				pPlayer->SaySystem("对 %s 设置技能等级成功!", Params[0].pszParam);
		}
		else
			pPlayer->SaySystem("设置技能等级失败!");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：禁言
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(BAN) {
	if (nParam >= 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			DWORD dwTime = 0xffffffff;
			if (nParam > 1)
				dwTime = Params[1].nParam;
			if (p->IsSystemFlagSeted(SF_BANED))
			{
				p->SetSystemFlag(SF_BANED, FALSE);
				pPlayer->SaySystem("%s 发言禁止被解除!", Params[0].pszParam);
			}
			else
			{
				p->SetSystemFlag(SF_BANED, TRUE, 0, dwTime);
				pPlayer->SaySystem("%s 被禁止发言!", Params[0].pszParam);
			}
		}
		else
			pPlayer->SaySystem("%s 不在线!", Params[0].pszParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载天下第一列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADTOPLIST) {
	CTopManager::GetInstance()->Load(".\\Data\\Figure\\TopList.txt");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改头发
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEHAIR) {
	if (nParam == 1)
		pPlayer->ChangeHair((BYTE)Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：特殊装备移动
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DMOVE) {
	if (pPlayer->GetMap() == nullptr || pPlayer->GetMap()->IsFlagSeted(MF_NODMOVE))
	{
		pPlayer->SaySystem("该地图不支持特殊装备移动");
		return 0;
	}
	if (pPlayer->IsSpecialEquipmentFunctionOn(SEF_TELEPORT))
	{
		if (nParam == 2)
			pPlayer->FlyTo(pPlayer->GetMap(), Params[0].nParam, Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：寻宝
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SEARCHING) {
	if (pPlayer->IsSpecialEquipmentFunctionOn(SEF_SEARCH))
	{
		if (nParam == 1)
		{
			CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
			if (p)
				pPlayer->SaySystem("%s 在 %s 的 %d %d 位置!",
					p->GetName(), p->GetMap() == nullptr ? "" : p->GetMap()->GetTitle(), p->getX(), p->getY());
			else
				pPlayer->SaySystem("%s 不在线!", Params[0].pszParam);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：关闭天地合一
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REFUSESPECIALPOWER) {
	if (pPlayer->IsSystemFlagSeted(SF_ALLOWSPECIALPOWER))
	{
		pPlayer->SetSystemFlag(SF_ALLOWSPECIALPOWER, FALSE);
		pPlayer->SaySystem("关闭天地合一!");
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：组队移动
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(USESPECIALPOWER) {
	if (pPlayer->IsSpecialEquipmentFunctionOn(SEF_TELEPORTGROUP))
	{
		if (pPlayer->GetMap() == nullptr || pPlayer->GetMap()->IsFlagSeted(MF_NOGROUPMOVE))
		{
			pPlayer->SaySystem("这个地图无法使用组队移动");
			return 0;
		}
		if (CGroupObject* pGrp = pPlayer->GetGroupObject())
		{
			xAutoPtrArray<CHumanPlayer>* parray = &pGrp->GetMemberArray();
			CLogicMap* pMap = pPlayer->GetMap();
			if (pMap)
			{
				for (UINT i = 0; i < (*parray).GetCount(); i++)
				{
					if ((*parray)[i] && (*parray)[i] != pPlayer && (*parray)[i]->IsSystemFlagSeted(SF_ALLOWSPECIALPOWER))
						(*parray)[i]->FlyTo(pPlayer->GetMap(), pPlayer->getX(), pPlayer->getY());
				}
			}
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载GM命令列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADCMDLIST){
	CGmManager::GetInstance()->LoadCommandDef(".\\Data\\GameMaster\\CmdList.txt");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载商城列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADMARKET) {
	CMarketManager::GetInstance()->LoadScrollText(".\\Data\\Market\\ScrollText.txt");
	CMarketManager::GetInstance()->LoadMarkets(".\\Data\\Market\\MainDir.txt");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：马休息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HORSEREST) {
	CMonsterEx* pHorse = pPlayer->GetHorse();
	if (pHorse)
	{
		pPlayer->ToggleHorseRest();
		if (pPlayer->IsHorseRest())
			pPlayer->SaySystemAttrib(CC_GREEN, "%s 开始休息", pHorse->GetName());
		else
			pPlayer->SaySystemAttrib(CC_GREEN, "%s 开始行动", pHorse->GetName());
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载怪物掉落物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADMONITEM){
	if (nParam == 0)return 0;
	CMonItemsMgr::GetInstance()->OnFoundFile(Params[0].pszParam, 1);
	if (CMonItemsMgr::GetInstance()->GetMonItems(Params[0].pszParam) == nullptr)return 0;
	return 1;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置经验倍数
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETEXPFACTOR){
	if (nParam == 0)return 0;
	FLOAT	f = (FLOAT)Params[0].nParam / 100.0f;
	pPlayer->SetExpFactor(f);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否死亡
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISDEATH){
	if (pPlayer->IsDeath())return 1;
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载自动机器人脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADAUTOSCRIPT)
	CAutoScriptManager::GetInstance()->Load(".\\Data\\AutoScript.txt");
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载基础技能数据
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADMAGIC){
	CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载技能扩展配置
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADMAGICEXT)
	CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv");
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：地图标示列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(LISTMAPFLAG){
	CLogicMap* pMap = nullptr;
	if (nParam == 1)
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
	else
		pMap = pPlayer->GetMap();
	if (pMap == nullptr)return 0;
	DWORD dwParam = 0;
	pPlayer->SaySystem("地图 %s (id = %u)标志：", pMap->GetTitle(), pMap->GetIndex());
	for (int i = 0; i < MF_MAX; i++)
	{
		if (pMap->IsFlagSeted((e_map_flag)i, dwParam))
		{
			pPlayer->SaySystem("%s(%u,%u)", g_map_flag[i], LOWORD(dwParam), HIWORD(dwParam));
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查地图标示
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKMAPFLAG){
	if (nParam == 0)return 0;
	CLogicMap* pMap = nullptr;
	char* pszFlag = nullptr;
	if (nParam == 1)
	{
		pMap = pPlayer->GetMap();
		pszFlag = Params[0].pszParam;
	}
	else
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		pszFlag = Params[1].pszParam;
	}
	if (pMap == nullptr)return 0;
	e_map_flag f = GetMapFlagFromString(pszFlag);
	return pMap->IsFlagSeted(f);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除地图标示
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REMOVEMAPFLAG){
	if (nParam == 0)return 0;
	char* pszFlag = nullptr;
	CLogicMap* pMap = nullptr;
	if (nParam == 1)
	{
		pMap = pPlayer->GetMap();
		pszFlag = Params[0].pszParam;
	}
	else
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		pszFlag = Params[1].pszParam;
	}
	if (pMap == nullptr)return 0;
	DWORD dwParam = 0;
	e_map_flag f = GetMapFlagFromString(pszFlag);
	if (f != MF_MAX)
	{
		if (!pMap->IsFlagSeted(f))
			pPlayer->SaySystem("地图 %s 的 %s 标志未打开", pMap->GetTitle(), Params[0].pszParam);
		else
		{
			pMap->UnSetFlag(f);
			return 1;
		}
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加地图表示
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDMAPFLAG){
	if (nParam == 1)
	{
		if (pPlayer->GetMap() != nullptr)
		{
			pPlayer->GetMap()->SetFlag(Params[0].pszParam);
			return 1;
		}
	}
	else if (nParam == 2)
	{
		CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		if (pMap)
		{
			pMap->SetFlag(Params[1].pszParam);
			return 1;
		}
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置出生点
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETRELIVEPOINT){
	if (nParam == 0)return 0;
	START_POINT* pStartPoint = CGameWorld::GetInstance()->GetStartPoint(Params[0].pszParam);
	if (pStartPoint)
	{
		pPlayer->SetStartPointIndex(pStartPoint->index);
		return 1;
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：清除地图所有怪物
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLEARMAPMONSTER){
	CLogicMap* pMap = nullptr;
	char* pszMonster = nullptr;
	if (nParam == 0)
		pMap = pPlayer->GetMap();
	else if (nParam == 1)
	{
		if (Params[0].pszParam[0] != 0 &&
			Params[0].nParam == 0)
		{
			pMap = pPlayer->GetMap();
			pszMonster = Params[0].pszParam;
		}
		else
			pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
	}
	else if (nParam > 1)
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		pszMonster = Params[1].pszParam;
	}
	if (pMap == nullptr)return 0;
	pMap->ClearAllMonsters(pszMonster);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：行会联盟
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(BUILDALLY){
	if (pPlayer->GetMap() == nullptr)return 0;
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(pPlayer))
	{
		int x, y;
		pPlayer->GetFrontPosition(x, y);
		CHumanPlayer* p = (CHumanPlayer*)pPlayer->GetMap()->FindObject(x, y, OBJ_PLAYER);
		if (p)
		{
			p->GetFrontPosition(x, y);
			CGuildEx* pGuild2 = p->GetGuild();
			if (x == pPlayer->getX() && y == pPlayer->getY() && pGuild2 && pGuild2->IsMaster(p))
			{
				pGuild->BuildAlly(pGuild2);
				pGuild2->BuildAlly(pGuild);
				return 1;
			}
		}
		pPlayer->SaySystem("你必须和对方掌门人面对面站立");
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：取消联盟
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(BREAKALLY){
	if (nParam == 0)return 0;
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && pGuild->IsMaster(pPlayer))
	{
		if (pGuild->BreakAlly(Params[0].pszParam))
		{
			CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
			if (pGuild)
				pGuild->BreakAlly(pGuild->GetName());
			return 1;
		}
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：请求行会战
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REQUESTGUILDWAR){
	if (nParam == 0)return 0;
	if (pPlayer->GetGuild() && pPlayer->GetGuild()->IsMaster(pPlayer))
	{
		CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
		if (pGuild == nullptr)
		{
			pPlayer->SaySystem( "该行会不存在" );
			return 0;
		}
		if (!CGuildWarManager::GetInstance()->RequestWar(pPlayer->GetGuild(), pGuild))
		{
			pPlayer->SaySystem(CGuildWarManager::GetInstance()->getErrorMsg());
			return 0;
		}
		return 1;
	}
	else
		pPlayer->SaySystem( "只有行会会长才能申请" );
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：允许联盟
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ALLOWALLY){
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && !pGuild->IsMaster(pPlayer))return 0;
	pPlayer->SetSystemFlag(SF_ALLOWALLY, !pPlayer->IsSystemFlagSeted(SF_ALLOWALLY));
	if (pPlayer->IsSystemFlagSeted(SF_ALLOWALLY))
		pPlayer->SaySystemAttrib(CC_GREEN, "[允许联盟]");
	else
		pPlayer->SaySystemAttrib(CC_GREEN, "[禁止联盟]");
	return 1;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否同意联盟
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISALLOWALLY)
	return pPlayer->IsSystemFlagSeted(SF_ALLOWALLY);
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否退出行会
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISGUILDEXIST){
	if (nParam < 1)return 0;
	return (CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam) != nullptr);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加行会最大成员数量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDGUILDMAXMEMBERCOUNT){
	CGuildEx* pGuild = nullptr;
	if (nParam == 0)return 0;
	UINT nAdd = 0;
	if (nParam == 1)
	{
		pGuild = pPlayer->GetGuild();
		nAdd = Params[0].nParam;
	}
	else
	{
		pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
		nAdd = Params[1].nParam;
	}
	if (pGuild == nullptr)return 0;
	pGuild->AddMaxMemberCount(nAdd);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置行会最大成员数量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETGUILDMAXMEMBERCOUNT){
	CGuildEx* pGuild = nullptr;
	if (nParam == 0)return 0;
	UINT nAdd = 0;
	if (nParam == 1)
	{
		pGuild = pPlayer->GetGuild();
		nAdd = Params[0].nParam;
	}
	else
	{
		pGuild = CGuildManagerEx::GetInstance()->FindGuild(Params[0].pszParam);
		nAdd = Params[1].nParam;
	}
	if (pGuild == nullptr)return 0;
	pGuild->SetMaxMemberCount(nAdd);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：沙城大门开关
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SABUKDOOR) {
	if (nParam == 0)return FALSE;
	CSandCity* sandCity = CSandCity::GetInstance();
	if (sandCity == nullptr)return FALSE;
	if (Params[0].nParam == 0)
		sandCity->GetMainGate()->Close();
	else
		sandCity->GetMainGate()->Open();
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：存沙城金币
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(PUTSABUKGOLD){
	if (nParam == 0)return 0;
	if (Params[0].nParam > pPlayer->GetMoney(MT_GOLD))return 0;
	if (!CSandCity::GetInstance()->AddTotalGold(Params[0].nParam))return 0;
	pPlayer->CostMoney(MT_GOLD, Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：取沙城金币
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GETSABUKGOLD){
	if (nParam == 0)return 0;
	if (Params[0].nParam > CSandCity::GetInstance()->GetTotalGold())return 0;
	if (!pPlayer->AddMoney(MT_GOLD, Params[0].nParam))return 0;
	CSandCity::GetInstance()->DecTotalGold(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置沙城出售税率
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETSABUKTEXRATE){
	if (nParam == 0)return 0;
	CSandCity::GetInstance()->SetTexRate(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置沙城购买税率
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETSABUKREBATE){
	if (nParam == 0)return 0;
	CSandCity::GetInstance()->SetRebate(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置宠物背包大小
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETPETBAG){
	UINT nCount = 0;
	if (nParam > 0)
		nCount = Params[0].nParam;
	return pPlayer->SetPetBagSize(nCount);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查时间时候是下一天
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISNEXTDAY) {
	DWORD nDay = IsNextDay(Params[0].nParam, pPlayer->GetTimeStamp());
	pPlayer->setVParam(0, nDay);
	if (nParam == 1 && nDay > 0)
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送时长区公告
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDGAMETIMENOTICE) {
	pPlayer->Sendfirstdlg(CGameWorld::GetInstance()->GetNotice());
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送打开时长区时间弹窗
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDOPENGAMETIMEINFO) {
	pPlayer->SendOpenGameTimeInfo();
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改玩家时长区游戏时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEGAMETIME) {
	if (nParam < 2) return FALSE;
	switch (Params[0].pszParam[0])
	{
	case '+':
		pPlayer->AddGameTime(Params[1].nParam);
		break;
	case '-':
		pPlayer->DecGameTime(Params[1].nParam);
		break;
	case '=':
		pPlayer->SetGameTime(Params[1].nParam);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示玩家特效
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWHUMEFFECT) {
	if (nParam == 1)
	{
		pPlayer->SendMsg(pPlayer->GetId(), 0x1fe, 1, 1, Params[0].nParam);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：创建事件
//		注释：A 地图名、B X坐标、C Y坐标、D 类型代码 、E 持续时间(秒) F 触发page
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CREATEEVENT) {
	if (nParam == 6)
	{
		TriggerEvent::Create(pPlayer, Params[0].pszParam, Params[1].nParam, Params[2].nParam, Params[3].nParam, Params[5].pszParam, Params[4].nParam * 1000, 500);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送倒计时
//		注释：时间(秒)四字节 + 协议号(0x0339) + 类型(2字节) + 显示精度(1字节) + 显示信息(可选)
// 倒计时时间, 单位：秒
// 倒计时类型：
//		1 - 20000	客户端自己用
//		20001 - 30000	图形倒计时
//		30001 - 40000	普通倒计时, 要显示内容
//		40001 - 50000	图形(小图)加普通文字显示
//		0	简单倒计时（无文字）
//		1	跨服争霸倒计时（特殊文字）
// 精度分为：
//		0 分钟, 1 秒.
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SECNONDTIMEOUT) {
	if (nParam < 2) return FALSE;
	if (nParam == 2)
		CGameWorld::GetInstance()->AddGlobeProcess(EP_SECNONDTIMEOUT, Params[0].nParam, Params[1].nParam, 0, 0, 50, 1);
    else if (nParam == 3)
		CGameWorld::GetInstance()->AddGlobeProcess(EP_SECNONDTIMEOUT, Params[0].nParam, Params[1].nParam, Params[2].nParam, 0, 50, 1);
    else if (nParam == 4)
		CGameWorld::GetInstance()->AddGlobeProcess(EP_SECNONDTIMEOUT, Params[0].nParam, Params[1].nParam, Params[2].nParam, 0, 50, 1, Params[3].pszParam);
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送珍宝经验
//		注释：参数1：珍宝经验值、参数2：珍宝最大经验值、参数3：珍宝星星（最大为5）
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDTREASUREINFO) {
	if (nParam == 1)
	{
		pPlayer->SendZhenBao(Params[0].nParam);
		return TRUE;
	}
	if (nParam == 3)
	{
		pPlayer->SendZhenBao(Params[0].nParam, Params[1].nParam, Params[2].nParam);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置时长封号
//		注释：参数1：封号序号、参数2：是否激活
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETFENGHAOGROW) {
	if (nParam >= 2)
	{
		UINT nId = Params[0].nParam;
		UINT nBoolean = Params[1].nParam;
		FenghaoInfo* pFenghaoInfo = pPlayer->GetFenghaoInfo();
		pFenghaoInfo->mFengHaoRow[nId].boActivation = nBoolean == 1;

		CFengHaoGrowManager* pMgr = CFengHaoGrowManager::GetInstance();
		FengHaoGrowItem* pConfig = pMgr->GetItem(nId);
		if (!pConfig)return FALSE;
		if (nBoolean == 0)
		{
			switch (pConfig->btType)
			{
			case 0:
				if (pFenghaoInfo->btType1 == nId) pFenghaoInfo->btType1 = 0;
			break;
			case 1:
				if (pFenghaoInfo->btType2 == nId) pFenghaoInfo->btType2 = 0;
			break;
			case 2:
				if (pFenghaoInfo->btType3 == nId) pFenghaoInfo->btType3 = 0;
			break;
			}
			pPlayer->SendMsg(pPlayer->GetId(), 0x9b0, 1, 0, 0);//卸下
			pPlayer->RecalcFengHaoProp(nId, FALSE);
		}
		if (nBoolean && pConfig->btLastDay > 0)
		{
			DWORD dwNow = (DWORD)time(nullptr); // 返回秒
			pFenghaoInfo->mFengHaoRow[nId].dwLastDate = ONE_DAY_SECONDS * pConfig->btLastDay + dwNow;
		}
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整成就组进度值
//		注释： 参数1 ：成就组ID 参数2 ：操作符 （+ 、- 、=） 参数3 ：值
//		此命令执行后，将调整指定成就组内所有的成就ID进度值，满足成就总进度，引擎将自动完成成就！
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEACHIEVEGROUPEXP) {
	if (nParam == 3)
	{
		switch (Params[1].pszParam[0])
		{
		case '+':
			return pPlayer->ChangeAchieveGroupExp(Params[0].nParam, 0, Params[2].nParam);
		case '-':
			return pPlayer->ChangeAchieveGroupExp(Params[0].nParam, 1, Params[2].nParam);
		case '=':
			return pPlayer->ChangeAchieveGroupExp(Params[0].nParam, 2, Params[2].nParam);
		default:
			return FALSE;
		}
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整指定成就ID进度值
//		注释： 参数1 ：成就ID 参数2 ：操作符 （+ 、- 、=） 参数3 ：值
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEACHIEVEEXP) {
	if (nParam == 3)
	{
		switch (Params[1].pszParam[0])
		{
		case '+':
			return pPlayer->ChangeAchieveExp(Params[0].nParam, 0, Params[2].nParam);
		case '-':
			return pPlayer->ChangeAchieveExp(Params[0].nParam, 1, Params[2].nParam);
		case '=':
			return pPlayer->ChangeAchieveExp(Params[0].nParam, 2, Params[2].nParam);
		default:
			return FALSE;
		}
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整指定成就ID状态
//		注释： 参数1 ：成就ID 参数2 ：状态值 （ 0：未完成，1：已完成/可领取，2：已领取）
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETACHIEVESTATE) {
	if (nParam == 2)
		return pPlayer->SetAchieveState(Params[0].nParam, Params[1].nParam);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整指定成就ID完成时间
//		注释： 参数1 ：成就ID 参数2 ：完成时间 （需要使用此时间变量 <$DATETIMETOWOLTIME(2018-11-11-18:00:00)> ）
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETACHIEVETIME) {
	if (nParam == 2)
		return pPlayer->SetAchieveTime(Params[0].nParam, Params[1].nParam);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送更新指定成就相关信息
//		注释： 参数1 ：成就ID
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDGOTACHIEVE) {
	if (nParam == 1)
		return pPlayer->SendGotAchieve(Params[0].nParam);
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调整玩家成就点
//		注释： 参数1 ：操作符 （ + 、- ） 参数2 ：值
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEACHIEVEPOINT) {
	if (nParam == 2)
	{
		switch (Params[0].pszParam[0])
		{
		case '+':
			return pPlayer->ChangeAchievePoint(0, Params[1].nParam);
		case '-':
			return pPlayer->ChangeAchievePoint(1, Params[1].nParam);
		case '=':
			return pPlayer->ChangeAchievePoint(2, Params[1].nParam);
		default:
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：宝箱开启
//		参数1：宝箱类型（1赤金灵匣, 2白银灵匣, 3神秘灵匣, 4青木灵匣）
//		参数2：物品列表 (格式: 物品名/发光标识/数量|物品名/发光标识/数量|...)
//		注释：变量格式说明;looks值：经验951、元宝951、声望282、金币
//		- 物品列表用 | 分隔
//		- 每个物品格式: 物品名/Looks/发光标识
//		- 发光标识: 2=发光(贵重物品), 1=普通物品
//		- 数量: 物品数量, 可为空则默认为1
//		示例: EXCHANGEBOX 1 变量1|变量2|变量3/951/1|变量6|变量7/1|变量9/1|变量10|变量11
//		11个物品中, 最后3个是表示下一轮中奖物品.
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(EXCHANGEBOX) {
	if (nParam == 2)
	{
		xPacketPool::ScopedPacket packet(65536);
		xStringsExtracter<11> items(Params[1].pszParam, "|");
		int nCount = items.getCount();
		if (nCount <= 0) return FALSE;
		int nWinNum = Getrand(nCount); // 随机一个中奖
		char szItem[16]{}; // 中奖物品名
		int nLooks = 0; // 中奖物品Looks
		for (int i = 0; i < nCount; i++)
		{
			TreasureBoxItem pBoxItem{};
			pBoxItem.btGoodType = 1; // 默认为普通物品
			xStringsExtracter<3> itemFields(items[i], "/", "", FALSE);
			if (itemFields.getCount() > 1 && itemFields[0] != nullptr && itemFields[1] != nullptr)
			{
				strncpy_s(pBoxItem.szName, itemFields[0], 14); // 物品名
				pBoxItem.Looks = StringToInteger(itemFields[1]); // Looks
				if (i == nWinNum) // 中奖物品
				{
					strncpy_s(szItem, itemFields[0], 14);
					nLooks = pBoxItem.Looks;
				}
			}
			else
				continue;
			if (itemFields.getCount() > 2 && itemFields[2] != nullptr)
				pBoxItem.btGoodType = StringToInteger(itemFields[2]); // 发光标识
			packet->push((LPVOID)&pBoxItem, sizeof(pBoxItem));
		}
		// w1 MAKEWORD[阶段(银界面3,4 / 金界面5,6),轮次(1-4)]、w2 物品数量、w3 宝箱类型、lpdata 物品数据
		pPlayer->SendMsg(pPlayer->GetId(), 0x273, MAKEWORD(5, 1), nCount, Params[0].nParam, (LPVOID)packet->getbuf(), packet->getsize());
		pPlayer->AddProcess(EP_EXCHANGEBOX, nLooks, 1, 0, 0, 10000, 1, szItem);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION