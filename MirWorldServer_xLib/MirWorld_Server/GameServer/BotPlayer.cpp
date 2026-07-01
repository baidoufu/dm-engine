#include "StdAfx.h"
#include "BotPlayer.h"
#include "BotContext.h"
#include "BotManager.h"
#include "aliveobject.h"
#include "logicmap.h"
#include "gameworld.h"
#include "humanplayermgr.h"
#include "ItemManager.h"

// ============================================================================
// 构造/析构
// ============================================================================
CBotPlayer::CBotPlayer(VOID) : CHumanPlayer()
{
	m_pContext = nullptr;
	m_pBehaviorTree = nullptr;
	m_bRunning = FALSE;
	m_bPaused = FALSE;
	m_dwDeathTime = 0;
	m_dwThinkInterval = 500;    // 默认500ms思考一次
	m_dwIdleChance = 5;         // 5%概率发呆
	m_dwChatChance = 2;         // 2%概率聊天
	m_dwOnlineTime = 0;
	m_dwLastTickTime = 0;
	memset(&m_BotDesc, 0, sizeof(m_BotDesc));
}

CBotPlayer::~CBotPlayer(VOID)
{
	CleanupAI();
}

// ============================================================================
// 清理AI相关资源
// ============================================================================
VOID CBotPlayer::CleanupAI()
{
	if (m_pContext)
	{
		delete m_pContext;
		m_pContext = nullptr;
	}
	if (m_pBehaviorTree)
	{
		delete m_pBehaviorTree;
		m_pBehaviorTree = nullptr;
	}
}

// ============================================================================
// 初始化机器人
// ============================================================================
BOOL CBotPlayer::InitBot(BOT_CREATE_DESC& desc)
{
	// 初始化玩家基础数据
	CREATEHUMANDESC humanDesc;
	humanDesc.dbinfo = desc.dbinfo;
	humanDesc.pClientObj = nullptr;  // 机器人不需要真实客户端连接

	if (!CHumanPlayer::Init(humanDesc))
	{
		LG2("机器人: 初始化玩家基础数据失败 [%s]\n", desc.dbinfo.szName);
		return FALSE;
	}
	// 修正HP/MP为满值（Init中hp=1只是占位，避免走hp==0分支覆盖坐标）
	int iMaxHp = GetPropValue(PI_MAXHP);
	int iMaxMp = GetPropValue(PI_MAXMP);
	if (iMaxHp > 0)
		AddPropValue(PI_CURHP, iMaxHp - GetPropValue(PI_CURHP));
	if (iMaxMp > 0)
		AddPropValue(PI_CURMP, iMaxMp - GetPropValue(PI_CURMP));
	// 初始化技能列表
	m_fMagicLoaded = TRUE;
	for (size_t i = 0; i < desc.vSkills.size(); i++)
	{
		AddMagic(desc.vSkills.at(i), 3);
	}
	// 初始化穿戴装备
	ITEM item;
	CItemManager* pItemManager = CItemManager::GetInstance();
	for (size_t i = 0; i < desc.vEquipments.size(); i++)
	{
		if (pItemManager->CreateTempItem(desc.vEquipments.at(i).c_str(), item))
		{
			EquipItem(i, item, TRUE);
		}
	}
	// 初始化背包物品
	for (size_t i = 0; i < desc.vItemsBox.size(); i++)
	{
		if (pItemManager->CreateTempItem(desc.vItemsBox.at(i).c_str(), item))
		{
			m_ItemBox.AddItem(item);
		}
	}
	// 缓存创建描述
	m_BotDesc = desc;

	// 创建决策上下文（替代AIController）
	m_pContext = new CBotContext();
	if (!m_pContext)
	{
		LG2("机器人: 创建决策上下文失败 [%s]\n", desc.dbinfo.szName);
		CleanupAI();
		return FALSE;
	}
	m_pContext->Bind(this);

	// 加载行为树配置（唯一决策入口）
	m_pBehaviorTree = new CBotBehaviorTree();
	if (desc.szBehaviorFile[0] != '\0')
	{
		std::array<char, 1024> szFile{};
		sprintf_s(szFile.data(), szFile.size(), ".\\Data\\Bot\\%s", desc.szBehaviorFile);
		if (!m_pBehaviorTree->LoadFromFile(szFile.data()))
		{
			delete m_pBehaviorTree;
			m_pBehaviorTree = nullptr;
			LG2("机器人: 加载行为树失败 [%s]\n", desc.szBehaviorFile);
			// 行为树加载失败则无法决策，记录错误
		}
	}

	// 设置个性化参数（模拟真人，添加随机偏差）
	m_dwThinkInterval = CBotHumanBehavior::RandomizeThinkInterval(desc.dwThinkInterval);
	m_dwIdleChance = desc.dwIdleChance;
	m_dwChatChance = desc.dwChatChance;
	// 初始化定时器
	m_tmrThinkInterval.Savetime();
	m_tmrChat.Savetime();
	m_dwLastTickTime = CFrameTime::GetFrameTime();
	LG2("机器人: 初始化成功 [%s] 职业[%d] 等级[%d]\n",
		desc.dbinfo.szName, desc.dbinfo.btClass, desc.dbinfo.wLevel);
	return TRUE;
}

// ============================================================================
// 机器人主更新循环
// ============================================================================
VOID CBotPlayer::Update()
{
	// 先执行玩家基础更新
	CHumanPlayer::Update();
	// 更新在线时长统计
	DWORD dwNow = CFrameTime::GetFrameTime();
	if (m_dwLastTickTime > 0)
	{
		DWORD dwElapsed = dwNow - m_dwLastTickTime;
		if (dwElapsed < 60000)  // 防止时间跳变
			m_dwOnlineTime += dwElapsed;
	}
	m_dwLastTickTime = dwNow;
	// 暂停计时器检查
	if (m_bPaused && m_tmrPauseTimer.IsTimeOut())
		m_bPaused = FALSE;
	// 检查是否需要思考
	if (!m_bRunning || m_bPaused)
		return;

	// 帧级预计算：刷新帧时间缓存（轻量操作，每次Update都做）
	m_pContext->SetFrameTime(dwNow);

	// 自适应思考间隔：战斗中400ms / 巡逻1000ms / 安全区3000ms / 死亡跳过
	DWORD dwBaseInterval = ComputeThinkInterval();
	if (dwBaseInterval == 0) return;
	DWORD dwActualInterval = CBotHumanBehavior::RandomizeThinkInterval(dwBaseInterval);
	if (!m_tmrThinkInterval.IsTimeOut(dwActualInterval))
		return;
	m_tmrThinkInterval.Savetime();

	// 预计算目标
	m_pContext->PrecomputeTarget();
	// 随机发呆（模拟真人行为）
	/*if (CBotHumanBehavior::ShouldIdle(m_dwIdleChance))
		return;*/
	// 随机聊天
	/*if (CBotHumanBehavior::ShouldChat(m_dwChatChance))
		SimulateRandomChat();*/
	// 执行行为树（唯一决策入口，SetFrameTime/PrecomputeTarget 已在上面处理）
	if (m_pBehaviorTree && m_pBehaviorTree->IsLoaded())
	{
		m_pBehaviorTree->Execute(this);
	}
}

// ============================================================================
// 机器人不需要真实网络消息
// ============================================================================
BOOL CBotPlayer::CanRecvMsg()
{
	return FALSE;
}

// ============================================================================
// 自适应思考间隔：根据 Bot 当前状态动态调整
// 战斗中有目标 → 400ms（更快响应）/ 巡逻无目标 → 1000ms / 安全区 → 3000ms / 死亡 → 0
// ============================================================================
DWORD CBotPlayer::ComputeThinkInterval()
{
	if (!m_pContext)
		return 0;

	// 死亡时返回较小间隔，使行为树死亡复活序列可继续执行（非0）
	if (IsDeath())
		return 1000;

	BOOL bInSafeArea = InSafeArea();
	CAliveObject* pTarget = m_pContext->GetCachedTarget();

	if (pTarget && !bInSafeArea)
		return 400;   // 战斗状态：更快响应（默认 400）

	if (bInSafeArea)
		return 3000;  // 安全区内发呆：大幅降速（默认 3000）

	return 1000;      // 野外巡逻找怪：适度降速（默认 1000）
}

// ============================================================================
// 死亡处理
// ============================================================================
VOID CBotPlayer::OnDeath(DWORD dwKiller)
{
	CAliveObject* pKiller = CGameWorld::GetInstance()->GetAliveObjectById(dwKiller);
	// 先执行玩家基础死亡处理
	CHumanPlayer::OnDeath(dwKiller);
	// 记录死亡时间戳（供CBTActionRevive判断复活延迟）
	m_dwDeathTime = CFrameTime::GetFrameTime();
	// 注意：不停止机器人(m_bRunning保持TRUE)，以便行为树死亡复活序列可继续执行
	// 死亡状态由 IsDeath() 跟踪，复活由行为树 CBTActionRevive 节点处理
	LG2("机器人: 死亡 [%s] 击杀者[%s]\n", GetName(), pKiller ? pKiller->GetName() : "未知");
}

// ============================================================================
// 受伤时锁定攻击者
// ============================================================================
VOID CBotPlayer::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
	CHumanPlayer::OnDamage(pAttacker, nDamage, type);
	if (pAttacker && !IsDeath())
	{
		if (IsProperTarget(pAttacker) && (GetTarget() == nullptr || Getrand(3) == 0))
			SetTarget(pAttacker);
	}
}

// 获取死亡后已流逝的毫秒数
DWORD CBotPlayer::GetDeathElapsed() const
{
	if (m_dwDeathTime == 0)
		return 0xFFFFFFFF;  // 未记录死亡时间，返回极大值表示无需等待
	return CFrameTime::GetFrameTime() - m_dwDeathTime;
}

// ============================================================================
// 行为控制
// ============================================================================
VOID CBotPlayer::StartBot()
{
	m_bRunning = TRUE;
	m_bPaused = FALSE;
	m_tmrThinkInterval.Savetime();
	m_dwLastTickTime = CFrameTime::GetFrameTime();
	LG2("机器人: 启动 [%s]\n", GetName());
}

VOID CBotPlayer::StopBot()
{
	m_bRunning = FALSE;
	m_bPaused = FALSE;
	LG2("机器人: 停止 [%s]\n", GetName());
}

VOID CBotPlayer::PauseBot(DWORD dwDuration)
{
	m_bPaused = TRUE;
	m_tmrPauseTimer.Savetime();
	m_tmrPauseTimer.SetTimeOut(dwDuration);
}

VOID CBotPlayer::ResumeBot()
{
	m_bPaused = FALSE;
	m_tmrThinkInterval.Savetime();
}

// ============================================================================
// 模拟转向
// ============================================================================
VOID CBotPlayer::SimulateTurn(int dir)
{
	Turn(dir);
}

// ============================================================================
// 模拟停止移动
// ============================================================================
VOID CBotPlayer::SimulateStop()
{
	Stop();
}

// ============================================================================
// 模拟施法
// ============================================================================
BOOL CBotPlayer::SimulateSpellCast(int targetX, int targetY, DWORD dwTargetId, WORD wMagicId)
{
	return SpellCast(targetX, targetY, dwTargetId, wMagicId);
}

// ============================================================================
// 模拟行走 - 主方向被阻挡时自动尝试相邻方向绕行
// ============================================================================
BOOL CBotPlayer::SimulateWalk(int dir)
{
	// 尝试主方向
	if (Walk(dir)) return TRUE;
	// 主方向被阻挡，扇形展开尝试相邻方向
	static const int offsets[] = { 1, -1, 2, -2, 3, -3 };
	for (int i = 0; i < 6; i++)
	{
		int newDir = (dir + offsets[i] + 8) % 8;
		if (Walk(newDir))
			return TRUE;
	}
	return FALSE;
}

// ============================================================================
// 模拟跑步 - 使用智能BFS绕障寻路
// ============================================================================
BOOL CBotPlayer::SimulateRun(int dir)
{
	// 快速路径：主方向可行则直接跑
	if (Run(dir)) return TRUE;
	// 跑步被阻挡，扇形展开尝试相邻方向跑步
	static const int offsets[] = { 1, -1, 2, -2, 3, -3 };
	for (int i = 0; i < 6; i++)
	{
		int newDir = (dir + offsets[i] + 8) % 8;
		if (Run(newDir))
			return TRUE;
	}
	// 相邻方向全部失败 → 尝试通过CBotContext获取目标并启动BFS智能寻路
	if (m_pContext)
	{
		CAliveObject* pTarget = m_pContext->GetCachedTarget();
		if (pTarget)
		{
			int bfsDir = FindPathBFS(pTarget->getX(), pTarget->getY());
			if (bfsDir >= 0)
			{
				if (Run(bfsDir))
					return TRUE;
				if (Walk(bfsDir))
					return TRUE;
			}
		}
	}
	// BFS也找不到路，降级为行走
	return SimulateWalk(dir);
}

// ============================================================================
// 智能绕障寻路 (BFS)
// ============================================================================
int CBotPlayer::FindPathBFS(int targetX, int targetY)
{
	int startX = getX();
	int startY = getY();
	if (startX == targetX && startY == targetY)
		return -1;

	CLogicMap* pMap = GetMap();
	if (!pMap)
		return -1;

	constexpr int SEARCH_RADIUS = 16;
	constexpr int GRID_SIZE = SEARCH_RADIUS * 2 + 1;
	constexpr int MAX_NODES = 2048;

	struct BfsNode
	{
		short x, y;
		short parent;
		short firstDir;
	};

	thread_local BfsNode s_queue[MAX_NODES];
	thread_local bool s_visited[GRID_SIZE * GRID_SIZE];
	memset(s_visited, 0, sizeof(s_visited));

	int head = 0, tail = 0;

	auto toIdx = [&](int x, int y) -> int {
		int dx = x - startX + SEARCH_RADIUS;
		int dy = y - startY + SEARCH_RADIUS;
		return dy * GRID_SIZE + dx;
	};

	s_queue[tail].x = (short)startX;
	s_queue[tail].y = (short)startY;
	s_queue[tail].parent = -1;
	s_queue[tail].firstDir = -1;
	tail++;
	s_visited[toIdx(startX, startY)] = true;

	int bestDistByDir[8];
	for (int d = 0; d < 8; d++)
		bestDistByDir[d] = 999999;

	while (head < tail && tail < MAX_NODES)
	{
		const BfsNode& cur = s_queue[head];
		int curDist = CBotHumanBehavior::Distance8(cur.x, cur.y, targetX, targetY);
		if (cur.parent != -1)
		{
			int fd = cur.firstDir;
			if (fd >= 0 && fd < 8 && curDist < bestDistByDir[fd])
				bestDistByDir[fd] = curDist;
		}
		if (curDist <= 1)
			break;

		for (int d = 0; d < 8; d++)
		{
			if (tail >= MAX_NODES) break; // 越界守卫: 队列已满, 停止扩展避免s_queue越界
			int nx = cur.x + G_DIROFS[d].x;
			int ny = cur.y + G_DIROFS[d].y;

			if (abs(nx - startX) > SEARCH_RADIUS ||
				abs(ny - startY) > SEARCH_RADIUS)
				continue;

			int nIdx = toIdx(nx, ny);
			if (s_visited[nIdx]) continue;
			if (pMap->IsBlocked(nx, ny)) continue;

			s_visited[nIdx] = true;
			short firstDir = (cur.parent == -1) ? (short)d : cur.firstDir;

			s_queue[tail].x = (short)nx;
			s_queue[tail].y = (short)ny;
			s_queue[tail].parent = (short)head;
			s_queue[tail].firstDir = firstDir;
			tail++;
		}
		head++;
	}

	struct DirScore { int dir; int dist; };
	DirScore ranked[8];
	for (int d = 0; d < 8; d++)
	{
		ranked[d].dir = d;
		ranked[d].dist = bestDistByDir[d];
	}
	for (int i = 1; i < 8; i++)
	{
		DirScore key = ranked[i];
		int j = i - 1;
		while (j >= 0 && ranked[j].dist > key.dist)
		{
			ranked[j + 1] = ranked[j];
			j--;
		}
		ranked[j + 1] = key;
	}

	if (ranked[0].dist < 999999)
		return ranked[0].dir;

	return -1;
}

// ============================================================================
// 模拟攻击
// ============================================================================
BOOL CBotPlayer::SimulateAttack(int dir)
{
	e_humanattackmode mode = GetAttackMode();
	return Attack(dir, 0, mode, DT_PHYSICS);
}

// ============================================================================
// 模拟拾取
// ============================================================================
VOID CBotPlayer::SimulatePickupItem()
{
	if (!CanPickupItem()) return;
	if (PickupItem())
	{
		SendWeightChanged();
	}
}

// ============================================================================
// 模拟使用物品
// ============================================================================
VOID CBotPlayer::SimulateUseItem(DWORD dwMakeIndex)
{
	if (!CanUseItem()) return;
	UseItem(dwMakeIndex, 0);
}

// ============================================================================
// 模拟聊天
// ============================================================================
VOID CBotPlayer::SimulateSay(const char* pszMsg)
{
	if (pszMsg == nullptr || pszMsg[0] == '\0')
		return;
	if (!m_tmrChat.IsTimeOut(3000))
		return;
	m_tmrChat.Savetime();
	ChannelSay(CCH_NORMAL, nullptr, pszMsg);
}

// ============================================================================
// 模拟随机聊天
// ============================================================================
VOID CBotPlayer::SimulateRandomChat()
{
	const char* pszMsg = CBotHumanBehavior::GenerateChatMessage();
	SimulateSay(pszMsg);
}

// ============================================================================
// 模拟切换攻击模式
// ============================================================================
VOID CBotPlayer::SimulateChangeAttackMode(e_humanattackmode attackMode)
{
	ChangeAttackMode(attackMode);
}

// ============================================================================
// 普通攻击当前目标（通过CBotContext获取目标）
// ============================================================================
BOOL CBotPlayer::SimulateAttackTarget()
{
	if (!m_pContext) return FALSE;

	CAliveObject* pTarget = m_pContext->GetCachedTarget();
	if (pTarget == nullptr) return FALSE;

	int dist = CBotHumanBehavior::Distance8(
		getX(), getY(),
		pTarget->getX(), pTarget->getY());

	// 模拟人类攻击犹豫
	if (CBotHumanBehavior::ShouldHesitate(10))
		return FALSE;

	int dir = CBotHumanBehavior::GetDirection8(
		pTarget->getX(), pTarget->getY(),
		getX(), getY());

	BOOL IsMove = FALSE;
	if (dist <= 1)  // 近距离
		SimulateAttack(dir);
	else
		IsMove = TRUE;
	if (IsMove)
	{
		if (dist > 5)
			SimulateRun(dir);
		else
			SimulateWalk(dir);
	}
	return TRUE;
}

// ============================================================================
// 移向当前目标
// ============================================================================
BOOL CBotPlayer::SimulateMoveToTarget()
{
	if (!m_pContext)
		return FALSE;

	CAliveObject* pTarget = m_pContext->GetCachedTarget();
	if (pTarget == nullptr)
		return FALSE;

	int dir = CBotHumanBehavior::GetDirection8(
		pTarget->getX(), pTarget->getY(),
		getX(), getY());

	int dist = CBotHumanBehavior::Distance8(
		getX(), getY(), pTarget->getX(), pTarget->getY());

	if (dist > 5)
		SimulateRun(dir);
	else
		SimulateWalk(dir);

	return TRUE;
}

// ============================================================================
// 巡逻移动
// ============================================================================
VOID CBotPlayer::SimulatePatrol()
{
	int dir = CBotHumanBehavior::RandomDirection8();
	if (CBotHumanBehavior::RandomPercent(60))
		SimulateWalk(dir);
	else
		SimulateRun(dir);
}

// ============================================================================
// 逃跑
// ============================================================================
BOOL CBotPlayer::SimulateFlee()
{
	int dir;
	if (m_pContext)
	{
		CAliveObject* pTarget = m_pContext->GetCachedTarget();
		if (pTarget)
		{
			dir = CBotHumanBehavior::GetDirection8(
				getX(), getY(),
				pTarget->getX(), pTarget->getY());
		}
		else
		{
			dir = CBotHumanBehavior::RandomDirection8();
		}
	}
	else
	{
		dir = CBotHumanBehavior::RandomDirection8();
	}
	SimulateRun(dir);
	return TRUE;
}

// ============================================================================
// 休息
// ============================================================================
VOID CBotPlayer::SimulateRest(DWORD dwDuration)
{
	PauseBot(dwDuration);
}

// ============================================================================
// 复活（死亡后复活，恢复HP/MP，可选回城）
//   1.清除死亡状态 2.恢复HP/MP 3.重置机器人运行标志 4.刷新客户端视野
// ============================================================================
BOOL CBotPlayer::SimulateRevive(int nHpPercent, BOOL bTeleportHome)
{
	// 仅在死亡状态下复活（幂等保护）
	if (!IsDeath())
		return TRUE;

	if (nHpPercent < 1)  nHpPercent = 1;
	if (nHpPercent > 100) nHpPercent = 100;

	// 清除死亡状态
	SetDeath(FALSE);

	// 恢复HP/MP（按百分比恢复HP，MP恢复满）
	int iMaxHp = GetPropValue(PI_MAXHP);
	int iMaxMp = GetPropValue(PI_MAXMP);
	int iTargetHp = iMaxHp * nHpPercent / 100;
	int iCurHp = GetPropValue(PI_CURHP);
	int iCurMp = GetPropValue(PI_CURMP);
	if (iTargetHp > iCurHp)
		AddPropValue(PI_CURHP, iTargetHp - iCurHp);
	if (iMaxMp > iCurMp)
		AddPropValue(PI_CURMP, iMaxMp - iCurMp);

	// 重置机器人运行状态（OnDeath曾停止机器人，复活后需恢复）
	m_bRunning = TRUE;
	m_bPaused = FALSE;

	// 刷新客户端显示：HP/MP
	SendHpMpChanged();

	// 刷新客户端视野（让周围玩家看到复活后的对象）
	if (bTeleportHome)
	{
		// 回城复活：FlyTo会自然刷新新旧地图所有客户端视野
		Home();
	}
	else
	{
		// 原地复活：参照CSCDoor::Repair，先移除死亡视野再重新显示活的对象
		std::array<char, 1024> szMsg = {};
		int length = 0;
		if (GetOutViewmsg(szMsg.data(), length))
			SendAroundMsg(szMsg.data(), length);
		if (GetViewmsg(szMsg.data(), length))
			SendAroundMsg(szMsg.data(), length);
	}

	LG2("机器人: 复活 [%s] HP=%d%% 回城=%d\n", GetName(), nHpPercent, bTeleportHome);
	return TRUE;
}

// ============================================================================
// 使用药水（通过CBotContext查找）
// ============================================================================
BOOL CBotPlayer::SimulateUsePotion(BOOL bHP)
{
	if (!m_pContext)
		return FALSE;

	DWORD dwPotionIndex = m_pContext->FindPotionInBag(bHP);
	if (dwPotionIndex == 0)
		return FALSE;

	SimulateUseItem(dwPotionIndex);
	return TRUE;
}

// ============================================================================
// 使用物品（通过CBotContext查找）
// ============================================================================
BOOL CBotPlayer::SimulateUseItem(const char* pszItemName)
{
	if (!m_pContext)
		return FALSE;

	DWORD dwItemIndex = m_pContext->FindItemInBag(pszItemName);
	if (dwItemIndex == 0)
		return FALSE;

	SimulateUseItem(dwItemIndex);
	return TRUE;
}

// ============================================================================
// 穿戴装备（通过CBotContext查找）
// ============================================================================
BOOL CBotPlayer::SimulateEquipItem(const char* pszItemName)
{
	if (!m_pContext)
		return FALSE;

	DWORD dwItemIndex = m_pContext->FindItemInBag(pszItemName);
	if (dwItemIndex == 0)
		return FALSE;

	return EquipItem(dwItemIndex);
}

// ============================================================================
// 使用技能
// ============================================================================
BOOL CBotPlayer::SimulateUseSkill(WORD wMagicId)
{
	if (!m_pContext)
		return FALSE;

	// 如果指定了技能ID，直接使用
	if (wMagicId != 0)
	{
		USERMAGIC* pMagic = GetMagic(wMagicId);
		if (pMagic == nullptr)
			return FALSE;

		CAliveObject* pTarget = m_pContext->GetCachedTarget();
		if (pTarget)
		{
			SpellCast(pTarget->getX(), pTarget->getY(),
				pTarget->GetId(), wMagicId);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
