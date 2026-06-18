#pragma once
#include <unordered_map>
#include <functional>

enum e_sendmsg_rage
{
	SMR_AREA,
	SMR_AROUND,
	SMR_MAP,
	SMR_SERVER,
	SMR_WORLD,
	SMR_GROUP,
	SMR_GUILD,
};

enum e_gm_command;
class CHumanPlayer;

class CClientObj :
	public CClientObject
{
public:
	CClientObj(VOID);
	virtual ~CClientObj(VOID);
	VOID Clean();
	VOID Update();
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	VOID OnDBMsg(PMIRMSG pMsg, int datasize);
	VOID OnVerifyString(const char* pszString);
	VOID OnDisconnect();
	VOID OnConnection();
	VOID OnDBItem(DBITEM* pItemArray, int nCount, BYTE btFlag);
public:
	VOID SendActionResult(BOOL bSuccess);
	VOID SendAddItem(ITEM& item);
	VOID SendEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex);
	VOID SendUnEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex);
	int	GetGmLevel()const { return m_iGmLevel; }
	const char* GetAccount() { return m_EnterInfo.szAccount.data(); }
	VOID OnCreateItem(ITEM& item, int pos, BYTE btFlag);
	int GetPacketQueueCount() { return m_xPacketQueue.getcount(); }
private:
	VOID SendClientNewMail(WORD Parm1, WORD Parm2, WORD Parm3);
	VOID SendBagItems(DBITEM* pItems, int count);
	VOID SendEquipments( /*DBITEM * pItems, int count*/);
	VOID ProcClientMsg(PMIRMSG pMsg, int datasize);
	VOID OnCommand(const char* pszCommand);
public:
	VOID HandleRideHorse(PMIRMSG pMsg, int datasize);
	VOID HandleAddFriend(PMIRMSG pMsg, int datasize);
	VOID HandleDeleteFriend(PMIRMSG pMsg, int datasize);
	VOID HandleConfirmFriend(PMIRMSG pMsg, int datasize);
	VOID HandleSetItemPosition(PMIRMSG pMsg, int datasize);
	VOID HandleViewEquipment(PMIRMSG pMsg, int datasize);
	VOID HandleLeaveServer(PMIRMSG pMsg, int datasize);
	VOID HandleRecordHomeStone(PMIRMSG pMsg, int datasize);
	VOID HandleMasterApprentice(PMIRMSG pMsg, int datasize);
	VOID HandleTradeRequest(PMIRMSG pMsg, int datasize);
	VOID HandlePetFeed(PMIRMSG pMsg, int datasize);
	VOID HandleGuildMemberList(PMIRMSG pMsg, int datasize);
	VOID HandleDropItem(PMIRMSG pMsg, int datasize);
	VOID HandlePickupItem(PMIRMSG pMsg, int datasize);
	VOID HandleTakeOnItem(PMIRMSG pMsg, int datasize);
	VOID HandleTakeOffItem(PMIRMSG pMsg, int datasize);
	VOID HandleUseItem(PMIRMSG pMsg, int datasize);
	VOID HandleDigCorpse(PMIRMSG pMsg, int datasize);
	VOID HandleSkillShortcut(PMIRMSG pMsg, int datasize);
	VOID HandleSoftClose(PMIRMSG pMsg, int datasize);
	VOID HandleClickNpcOrShop(PMIRMSG pMsg, int datasize);
	VOID HandleNpcClick(PMIRMSG pMsg, int datasize);
	VOID HandleGetItemPrice(PMIRMSG pMsg, int datasize);
	VOID HandleNpcSellItem(PMIRMSG pMsg, int datasize);
	VOID HandleBuyShopItem(PMIRMSG pMsg, int datasize);
	VOID HandleNpcSellList(PMIRMSG pMsg, int datasize);
	VOID HandleDropGold(PMIRMSG pMsg, int datasize);
	VOID HandleConfirmFirstDialog(PMIRMSG pMsg, int datasize);
	VOID HandleChangeGroupMode(PMIRMSG pMsg, int datasize);
	VOID HandleAddGroupMember(PMIRMSG pMsg, int datasize);
	VOID HandleDeleteGroupMember(PMIRMSG pMsg, int datasize);
	VOID HandleRepairItem(PMIRMSG pMsg, int datasize);
	VOID HandlePutRepairItem(PMIRMSG pMsg, int datasize);
	VOID HandleQueryTrade(PMIRMSG pMsg, int datasize);
	VOID HandlePutTradeItem(PMIRMSG pMsg, int datasize);
	VOID HandleCancelTrade(PMIRMSG pMsg, int datasize);
	VOID HandlePutTradeGold(PMIRMSG pMsg, int datasize);
	VOID HandleQueryTradeEnd(PMIRMSG pMsg, int datasize);
	VOID HandlePutToBank(PMIRMSG pMsg, int datasize);
	VOID HandleTakeFromBank(PMIRMSG pMsg, int datasize);
	VOID HandleWalkToMapLink(PMIRMSG pMsg, int datasize);
	VOID HandleViewGuildInfo(PMIRMSG pMsg, int datasize);
	VOID HandleJoinGuild(PMIRMSG pMsg, int datasize);
	VOID HandleRemoveGuildMember(PMIRMSG pMsg, int datasize);
	VOID HandleEditGuildNotice(PMIRMSG pMsg, int datasize);
	VOID HandleViewGuildExp(PMIRMSG pMsg, int datasize);
	VOID HandleRequestGroupPos(PMIRMSG pMsg, int datasize);
	VOID HandlePetBackExp(PMIRMSG pMsg, int datasize);
	VOID HandleTimeAchieve(PMIRMSG pMsg, int datasize);
	VOID HandleShortcutKey(PMIRMSG pMsg, int datasize);
	VOID HandleNewMail(PMIRMSG pMsg, int datasize);
	VOID HandleSocialInfo(PMIRMSG pMsg, int datasize);
	VOID HandleFengHao(PMIRMSG pMsg, int datasize);
	VOID HandleFuncCollection(PMIRMSG pMsg, int datasize);
	VOID HandleAvatarFrame(PMIRMSG pMsg, int datasize);
	VOID HandleGuildFengHao(PMIRMSG pMsg, int datasize);
	VOID HandleReplyGuild(PMIRMSG pMsg, int datasize);
	VOID HandleTrainHorse(PMIRMSG pMsg, int datasize);
	VOID HandleTurn(PMIRMSG pMsg, int datasize);
	VOID HandleWalk(PMIRMSG pMsg, int datasize);
	VOID HandleGetMeal(PMIRMSG pMsg, int datasize);
	VOID HandleRun(PMIRMSG pMsg, int datasize);
	VOID HandleAttack(PMIRMSG pMsg, int datasize);
	VOID HandleMine(PMIRMSG pMsg, int datasize);
	VOID HandleSpellSkill(PMIRMSG pMsg, int datasize);
	VOID HandleSpecialHit(PMIRMSG pMsg, int datasize);
	VOID HandleStop(PMIRMSG pMsg, int datasize);
	VOID HandleBowAction(PMIRMSG pMsg, int datasize);
	VOID HandleSay(PMIRMSG pMsg, int datasize);
	VOID HandleClientTime(PMIRMSG pMsg, int datasize);
	VOID HandlePetPickup(PMIRMSG pMsg, int datasize);
	VOID HandlePutToPetBag(PMIRMSG pMsg, int datasize);
	VOID HandleGetFromPetBag(PMIRMSG pMsg, int datasize);
	VOID HandleOpenMarket(PMIRMSG pMsg, int datasize);
	VOID HandlePrivateShop(PMIRMSG pMsg, int datasize);
	VOID HandleDeleteTask(PMIRMSG pMsg, int datasize);
	VOID HandleRequestPrivateShop(PMIRMSG pMsg, int datasize);
	VOID HandleCreateGuild(PMIRMSG pMsg, int datasize);
	VOID HandlePersonSetting(PMIRMSG pMsg, int datasize);
	VOID HandlePutItem(PMIRMSG pMsg, int datasize);
	VOID HandleShowPetInfo(PMIRMSG pMsg, int datasize);
	VOID HandlePetWindow(PMIRMSG pMsg, int datasize);
	VOID HandleHeroRank(PMIRMSG pMsg, int datasize);
	VOID HandleQueryHeroRank(PMIRMSG pMsg, int datasize);
private:
	ENTERGAMESERVER	m_EnterInfo;
	gameserverusermode m_State;
	CHumanPlayer* m_pPlayer;
	int	m_iGmLevel;
	BOOL m_bScrollTextMode;
	BOOL m_bNoticeMode;
	BOOL m_bCompetlyQuit;
	// 消息频率限制(防恶意刷包)
	static constexpr DWORD MSG_RATE_WINDOW = 1000; // 统计窗口1秒
	static constexpr DWORD MSG_RATE_MAX = 50; // 每秒最大消息数
	static constexpr DWORD MSG_RATE_WARN = 30; // 警告阈值
	CServerTimer m_tmrMsgRate; // 消息频率统计窗口定时器
	DWORD m_dwMsgCount; // 当前窗口内消息计数
	DWORD m_dwMsgWarnCount; // 连续超警告次数
};
