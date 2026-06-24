#pragma once
#include "humanplayer.h"
#include "BotContext.h"
#include "BotBehaviorTree.h"
#include "BotHumanBehavior.h"

// ============================================================================
// 机器人创建描述结构
// ============================================================================
typedef struct tagBOT_CREATE_DESC
{
	CHARDBINFO dbinfo = CHARDBINFO();		// 角色数据库信息
	DWORD dwThinkInterval = 0;				// 思考间隔(ms)
	DWORD dwIdleChance = 0;					// 发呆概率(%)
	DWORD dwChatChance = 0;					// 聊天概率(%)
	CHAR szBehaviorFile[256]{};				// 行为树文件路径
	std::vector<int> vSkills;				// 技能列表
	std::vector<std::string> vEquipments;	// 穿戴装备
	std::vector<std::string> vItemsBox;		// 背包物品列表
}BOT_CREATE_DESC;

// ============================================================================
// 机器人玩家类 - 继承自CHumanPlayer
// 行为树为唯一决策入口，CBotContext提供决策查询接口
// ============================================================================
class CBotPlayer : public CHumanPlayer
{
public:
	CBotPlayer(VOID);
	virtual ~CBotPlayer(VOID);

	// 初始化机器人
	BOOL InitBot(BOT_CREATE_DESC& desc);

	// 重写关键虚函数
	virtual VOID Update();                  // 机器人主更新循环
	virtual BOOL CanRecvMsg();              // 机器人不需要真实网络消息
	virtual VOID OnDeath(DWORD dwKiller);   // 死亡处理

	// 决策上下文接口
	CBotContext* GetContext() { return m_pContext; }

	// 行为控制
	VOID StartBot();                // 启动机器人
	VOID StopBot();                 // 停止机器人
	VOID PauseBot(DWORD dwDuration);// 暂停指定时间
	VOID ResumeBot();               // 恢复运行

	// 状态查询
	BOOL IsBotRunning() const { return m_bRunning; }
	BOOL IsBotPaused() const { return m_bPaused; }
	BOOL IsBot() const override { return TRUE; }  // 标识为机器人

	// 行为模拟接口（模拟客户端消息，不走网络层）
	BOOL SimulateWalk(int dir);     // 模拟行走（返回TRUE=移动成功）
	BOOL SimulateRun(int dir);      // 模拟跑步（返回TRUE=移动成功，失败自动降级行走）
	BOOL SimulateAttack(int dir);   // 模拟攻击
	VOID SimulatePickupItem();      // 模拟拾取
	VOID SimulateUseItem(DWORD dwMakeIndex);  // 模拟使用物品
	VOID SimulateSay(const char* pszMsg);     // 模拟聊天
	VOID SimulateRandomChat();      // 模拟随机聊天
	VOID SimulateChangeAttackMode(e_humanattackmode attackMode);// 模拟切换攻击模式
	VOID SimulateTurn(int dir);     // 模拟转向
	VOID SimulateStop();            // 模拟停止移动
	BOOL SimulateSpellCast(int targetX, int targetY, DWORD dwTargetId, WORD wMagicId); // 模拟施法

	// 高级行为接口（由行为树调用）
	BOOL SimulateAttackTarget();    // 攻击当前目标
	BOOL SimulateMoveToTarget();    // 移向当前目标
	VOID SimulatePatrol();          // 巡逻移动
	BOOL SimulateFlee();            // 逃跑
	VOID SimulateRest(DWORD dwDuration);  // 休息
	BOOL SimulateUsePotion(BOOL bHP);     // 使用药水
	BOOL SimulateUseItem(const char* pszItemName);// 使用物品
	BOOL SimulateEquipItem(const char* pszItemName);// 穿戴装备
	BOOL SimulateUseSkill(WORD wMagicId); // 使用技能

	// 获取创建描述
	const BOT_CREATE_DESC& GetBotDesc() const { return m_BotDesc; }

	// 视野对象访问（CBotContext需要访问）
	xListHost<VISIBLE_OBJECT>* GetVisibleObjectList() { return &m_xVisibleObjectList; }
	xListHost<VISIBLE_OBJECT>* GetVisibleItemsList() { return &m_xVisibleItemsList; }

	// 清理AI相关资源
	VOID CleanupAI();

	// 智能寻路：使用BFS在局部范围内搜索绕过障碍物的路径
	int FindPathBFS(int targetX, int targetY);

	// 自适应思考间隔：战斗中400ms / 巡逻1000ms / 安全区3000ms / 死亡跳过
	DWORD ComputeThinkInterval();

private:
	CBotContext* m_pContext;             // 决策上下文（替代AIController）
	CBotBehaviorTree* m_pBehaviorTree;  // 行为树（唯一决策入口）
	CServerTimer m_tmrThinkInterval;    // 思考间隔定时器
	CServerTimer m_tmrActionDelay;      // 动作延迟定时器
	CServerTimer m_tmrPauseTimer;       // 暂停计时器
	CServerTimer m_tmrChat;             // 聊天频率限制定时器

	BOOL m_bRunning;        // 是否运行中
	BOOL m_bPaused;         // 是否暂停
	
	// 人类行为模拟参数
	DWORD m_dwThinkInterval;// 思考间隔(毫秒)
	DWORD m_dwIdleChance;   // 发呆概率
	DWORD m_dwChatChance;   // 聊天概率

	// 统计数据
	DWORD m_dwOnlineTime;   // 在线时长
	DWORD m_dwLastTickTime; // 上次tick时间

	// 创建描述缓存
	BOT_CREATE_DESC m_BotDesc;
};
