#include "MainWnd.h"
#include "Global/GlobalMsg.h"
#include "Gamedata/GameGlobal.h"
#include "GameData/GameData.h"
#include "GameData/OtherData.h"
#include "GameData/LoginData.h"
#include "GameData/ConfigData.h"
#include "GameData/TaskData.h"
#include "GameAI/AIConfigMgr.h"
#include "Global/Interface/AudioInterface.h"
#include "GameClient/VoiceAdapter.h"
#include "GameAI/AIAutoMgr.h"
#include "GameAI/AIMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameAI/AutoKillMonsterMgr.h"
#include "GameAI/AIGoodMgr.h"
#include "GameData/TalkMgr.h"
#include  "BaseClass/Control/CtrlMsgBox.h"
#include "GameData/TradeData.h"
#include "GameData/FindGood.h"
#include "GameData/MsgBoxMgr.h"
#include "BaseClass/Control/ParserTip.h"
#include "BaseClass/Control/CtrlMenuWnd.h"
#include "WndClass/GameWnd/GameWnd.h"
#include "WndClass/LoginWnd/LoginWnd.h"
#include "WndClass/SelectCharWnd/SelectCharWnd.h"
#include "WndClass/SelectCharWnd/QuitWnd.h"
#include "GameControl/GameControl.h"
#include "WndClass/GameWnd/PanelWnd.h"
#include "WndClass/GameWnd/AvatarWnd.h"
#include "WndClass/GameWnd/PackageWnd.h"
#include "WndClass/GameWnd/NpcWnd.h"
#include "WndClass/GameWnd/NpcBuyWnd.h"
#include "WndClass/GameWnd/NpcQuickWnd.h"
#include "WndClass/GameWnd/NpcListWnd.h"
#include "WndClass/GameWnd/NpcSaleWnd.h"
#include "WndClass/GameWnd/NpcBookWnd.h"
#include "WndClass/GameWnd/NpcBoardWnd.h"
#include "WndClass/GameWnd/SkillWnd.h"
#include "GameData/MsgBoxMgr.h"
#include "BaseClass/Control/GoodGrid.h"
#include "WndClass/GameWnd/HandyMapWnd.h"
#include "WndClass/GameWnd/WooolStoreWnd.h"
#include "WndClass/GameWnd/ShortCutWnd.h"
#include "WndClass/GameWnd/WooolStoryWnd.h"
#include "WndClass/GameWnd/MinMapWnd.h"
#include "WndClass/GameWnd/PersonsGuideWnd.h"
#include "WndClass/GameWnd/LookLeaveWordsWnd.h"
#include "WndClass/GameWnd/QuickBuyWnd.h"
#include "WndClass/GameWnd/BoothNamePrice.h"
#include "WndClass/GameWnd/BoothSelectWnd.h"
#include "WndClass/GameWnd/BoothWnd.h"
#include "WndClass/GameWnd/TradeWnd.h"
#include "WndClass/GameWnd/AssConfigWnd.h"
#include "WndClass/GameWnd/AutoLeaveWordsWnd.h"
#include "WndClass/GameWnd/ComandWnd.h"
#include "WndClass/GameWnd/CommonCondenseWnd.h"
#include "WndClass/GameWnd/CommonConfirmWnd.h"
#include "WndClass/GameWnd/PetInfoWnd.h"
#include "WndClass/GameWnd/EquipLevelUpWnd.h"
#include "WndClass/GameWnd/GemCondenseWnd.h"
#include "WndClass/GameWnd/GreetingWnd.h"
#include "WndClass/GameWnd/GuildAddMember.h"
#include "WndClass/GameWnd/InputMiBaoWnd.h"
#include "WndClass/GameWnd/InputUuidWnd.h"
#include "WndClass/GameWnd/GuildAllyWnd.h"
#include "WndClass/GameWnd/GuildDelMember.h"
#include "WndClass/GameWnd/GuildEdit.h"
#include "WndClass/GameWnd/InputYelloFlagWnd.h"
#include "WndClass/GameWnd/LeaveMessageWnd.h"
#include "WndClass/GameWnd/UploadWnd.h"
#include "WndClass/GameWnd/NameWnd.h"
#include "WndClass/GameWnd/NoticeHelpWnd.h"
#include "WndClass/GameWnd/OtherBoothWnd.h"
#include "WndClass/GameWnd/OtherPlayerEquip.h"
#include "WndClass/GameWnd/PersonPanelWnd.h"
#include "WndClass/GameWnd/PetPackageWnd.h"
#include "WndClass/GameWnd/PetWnd.h"
#include "WndClass/GameWnd/ReadQuitWnd.h"
#include "WndClass/GameWnd/SmallGemCon.h"
#include "WndClass/GameWnd/SplitStoneWnd.h"
#include "WndClass/GameWnd/SysConfigWnd.h"
#include "WndClass/GameWnd/TaskWnd.h"
#include "WndClass/GameWnd/TeamWnd.h"
#include "WndClass/GameWnd/ElixirWnd.h"
#include "WndClass/GameWnd/UseObjectWnd.h"
#include "WndClass/GameWnd/UserInfoWnd.h"
#include "WndClass/GameWnd/RelationWnd.h"
#include "WndClass/GameWnd/RangeWnd.h"
#include "WndClass/SelectCharWnd/ReliveWithYuanBao.h"
#include "WndClass/SelectCharWnd/ReliveWnd.h"
#include "WndClass/LoginWnd/smltkeyboard.h"
#include "WndClass/LoginWnd/ChangePasswordWnd.h"
#include "WndClass/SelectCharWnd/RegisterInfoWnd.h"
#include "WndClass/GameWnd/CheckMessageBox.h"
#include "WndClass/GameWnd/ExternMessgeBoxWnd.h"
#include "WndClass/GameWnd/ExLeavWordsWnd.h"
#include "WndClass/GameWnd/VoiceWnd.h"
#include "WndClass/GameWnd/SetQuickItemWnd.h"
#include "WndClass/GameWnd/WoolStorBuyCount.h"
#include "WndClass/GameWnd/WoolStorSendWnd.h"
#include "WndClass/GameWnd/WoolStorSendOKWnd.h"
#include "WndClass/GameWnd/WoolStorCleanWnd.h"
#include "WndClass/GameWnd/WoolStorRecvOKWnd.h"
#include "WndClass/GameWnd/PrayTreeWnd.h"
#include "WndClass/GameWnd/RadioWnd.h"
#include "WndClass/LoginWnd/registerwnd.h"
#include "WndClass/GameWnd/HeartKnackWnd.h"
#include "WndClass/GameWnd/SealWnd.h"
#include "WndClass/GameWnd/SealBookWnd.h"
#include "WndClass/LoginWnd/ReservePasswordWnd.h"
#include "WndClass/GameWnd/LuxNpcWnd.h"
#include "WndClass/GameWnd/WuXingPrizeWnd.h"
#include "WndClass/GameWnd/CheckOpenWnd.h"
#include "WndClass/GameWnd/HuanHuaWnd.h"
#include "WndClass/GameWnd/BoxWnd.h"
#include "GameData/MagicCtrlMgr.h"
#include "WndClass/GameWnd/MazeWnd.h"
#include "WndClass/GameWnd/ReceiveMsgWnd.h"
//#include "WndClass/GameWnd/KfzMemberListWnd.h"
//#include "WndClass/GameWnd/KfzEditMemberWnd.h"
//#include "WndClass/GameWnd/KfzIEWnd.h"
//#include "WndClass/GameWnd/KfzResultWnd.h"
//#include "WndClass/GameWnd/KfzFlow.h"
//#include "WndClass/GameWnd/KfzMemberState.h"
//#include "WndClass/GameWnd/KfzArenaWnd.h"
#include "WndClass/GameWnd/GuildFlagSelWnd.h"
#include "WndClass/GameWnd/TreasureMapWnd.h"
#include "WndClass/GameWnd/NpcRecruit.h"
#include "WndClass/GameWnd/LicenseWnd.h"
#include "WndClass/GameWnd/ExtractAndUpgradeYuanshiWnd.h"
#include "WndClass/GameWnd/VipStoreWnd.h"
#include "WndClass/GameWnd/DelSkillWnd.h"
#include "WndClass/GameWnd/FuShiSeriesWnd.h"
#include "WndClass/GameWnd/FairyGuide.h"
#include "WndClass/GameWnd/MemoireWnd.h"
#include "WndClass/GameWnd/ProduceWnd.h"
#include "WndClass/GameWnd/LuckQyzWnd.h"
#include "WndClass/GameWnd/BlessZhongZhouWnd.h"
#include "WndClass/GameWnd/InstanceZonePopupWnd.h"
#include "WndClass/GameWnd/ActivityLogWnd.h"
#include "WndClass/GameWnd/BaoJianWnd.h"
#include "WndClass/GameWnd/YuanBaoWnd.h"
#include "WndClass/GameWnd/TaskTreeDesWnd.h"
#include "GameClient/SDOAInterface.h"
#include "WndClass/GameWnd/TigerRuneWnd.h"
#include "WndClass/GameWnd/MeritoriousWnd.h"
#include "WndClass/GameWnd/RTSInstanceWnd.h"
#include "WndClass/GameWnd/ShortCutKeyWnd.h"
#include "WndClass/LoginWnd/TodayActivityWnd.h"
#include "WndClass/LoginWnd/TodayActivityIEWnd.h"
#include "WndClass/LoginWnd/TodayActivityLocalWnd.h"
#include "WndClass/GameWnd/TaskWuSeDailyWnd.h"
#include "WndClass/GameWnd/LeaseBoothWnd.h"
#include "WndClass/GameWnd/LeaseOtherBoothWnd.h"
#include "WndClass/GameWnd/FirmWnd.h"
#include "WndClass/GameWnd/FirmDetailInfoWnd.h"
#include "GameClient/ReplayManager.h"
#include "WndClass/GameWnd/EquipSoulWnd.h"
#include "WndClass/GameWnd/DivinityWingWnd.h"
#include "WndClass/GameWnd/License2Wnd.h"
#include "WndClass/GameWnd/QuickItemFashionalWnd.h"
#include "WndClass/GameWnd/TalkViewWnd.h"
#include "WndClass/GameWnd/ChatPanelWnd.h"
#include "WndClass/LoginWnd/AuthenWnd.h"
#include "WndClass/GameWnd/BlessCompoundWnd.h"
#include "WndClass/GameWnd/WenPeiWnd.h"
#include "WndClass/GameWnd/EnterShanWnd.h"
#include "WndClass/GameWnd/AdventureWnd.h"
#include "WndClass/GameWnd/CommonSelectWnd.h"
#include "WndClass/GameWnd/ComposeEquipWnd.h"
#include "WndClass/GameWnd/BoothGuideWnd.h"
#include "WndClass/GameWnd/SanWeiFireWnd.h"
#include "WndClass/GameWnd/FaBaoShowWnd.h"
#include "WndClass/GameWnd/FaBaoLevelUpWnd.h"
#include "WndClass/GameWnd/FireArtificeWnd.h"
#include "WndClass/GameWnd/NoticeIEWnd.h"
#include "WndClass/GameWnd/OtherPlayerFaBaoEquip.h"
#include "WndClass/GameWnd/VipCardWnd.h"
#include "WndClass/GameWnd/LingWuSanWeiFire.h"
#include "WndClass/GameWnd/Defend12GongWnd.h"
#include "WndClass/GameWnd/Defend12CtrlWnd.h"
#include "WndClass/GameWnd/YiHuoZhanMsgWnd.h"
#include "WndClass/GameWnd/LoginCheckMaskWnd.h"
#include "WndClass/GameWnd/PlayerPanelWnd.h"
#include "WndClass/GameWnd/EquipChaifenWnd.h"
#include "BaseClass/Misc/AntiWaiGua.h"
#include "WndClass/GameWnd/HuFuFlushWnd.h"
#include "WndClass/GameWnd/CaiHongHelpWnd.h"
#include "WndClass/GameWnd/YuShouLevelUpWnd.h"
#include "WndClass/GameWnd/QiShouWnd.h"
#include "WndClass/GameWnd/BindPTWnd.h"
#include "WndClass/GameWnd/BindPTMsgWnd.h"


#pragma warning(disable : 4311)


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

#define REMOVECONTROL(pControl)  RemoveControl((CControl**)(&pControl))
#define ADDAUTOWND(wndname,wndmsg) m_MMsgWnd[wndmsg] = #wndname;\
	m_MAutoWnd[#wndname] = &C##wndname::CreateObject;


DTI_IMPLEMENT(CMainWnd, CCtrlMainWin)
CMainWnd::CMainWnd(void)
{
	m_pGameWnd = NULL;
	m_pLoginWnd = NULL;
	m_pSelectCharWnd = NULL;
	m_pQuitWnd = NULL;
    m_pTalkViewWnd = NULL;

	//自定义窗口需要在这里注册
	ADDAUTOWND(PanelWnd,MSG_CTRL_PANEL_WND)
	ADDAUTOWND(PackageWnd,MSG_CTRL_PACKAGEWND)
	ADDAUTOWND(AvatarWnd,MSG_CTRL_CHARWND)
	ADDAUTOWND(SkillWnd,MSG_CTRL_SKILLWND)
	ADDAUTOWND(ShortCutWnd,MSG_CTRL_SHORTCUT_WND)
	ADDAUTOWND(NpcWnd,MSG_CTRL_NPCWND)
	ADDAUTOWND(NpcBuyWnd,MSG_CTRL_NPCBUYWND)
	ADDAUTOWND(NpcSaleWnd,MSG_CTRL_NPCSALEWND)
	ADDAUTOWND(NpcQuickWnd,MSG_CTRL_NPC_QUICK_WND)
	ADDAUTOWND(NpcListWnd,MSG_CTRL_NPCLISTWND)
	ADDAUTOWND(NpcBookWnd,MSG_CTRL_NPCBOOK_WND)
	ADDAUTOWND(NpcBoardWnd,MSG_CTRL_NPCBOARDWND)
	ADDAUTOWND(HandyMapWnd,MSG_CTRL_HANDYMAP_WND)
	ADDAUTOWND(WooolStoreWnd,MSG_CTRL_WOOOL_STORE_WND)
	ADDAUTOWND(WooolStoryWnd,MSG_CTRL_WOOOL_STORY_WND)
	ADDAUTOWND(MinMapWnd,MSG_CTRL_MINMAP_WND)
	ADDAUTOWND(PersonsGuideWnd,MSG_CTRL_PERSONS_GUIDE_WND)
	ADDAUTOWND(QuickBuyWnd,MSG_CTRL_QUICKBUY_WND)
	ADDAUTOWND(BoothNameWnd,MSG_CTRL_BOOTH_NAME)
	ADDAUTOWND(BoothPriceWnd,MSG_CTRL_BOOTH_ADD_GOOD)
	ADDAUTOWND(BoothSelectWnd,MSG_CTRL_SELECT_BOOTH_CONFIRM)
	ADDAUTOWND(BoothWnd,MSG_CTRL_BOOTH_WND)
	ADDAUTOWND(OtherBoothWnd,MSG_CTRL_OTHER_BOOTH)
	ADDAUTOWND(BoothBuyConfirmWnd,MSG_CTRL_BOOTH_BUY_CONFIRM)
	ADDAUTOWND(AssConfigWnd,MSG_CTRL_ASSCONFIG_WND)
	ADDAUTOWND(AutoLeaveWordsWnd,MSG_CTRL_AUTOLEAVEWORD_WND)
	ADDAUTOWND(ComandWnd,MSG_CTRL_COMMAND_WND)
	ADDAUTOWND(CommonCondenseWnd,MSG_CTRL_COMMONCONDENSE_WND)
	ADDAUTOWND(CommonConfirmWnd,MSG_CTRL_COMMON_CONFIRM_WND)
	ADDAUTOWND(PetInfoWnd,MSG_CTRL_PET_INFO_WND)
	ADDAUTOWND(EquipLevelUpWnd,MSG_CTRL_EQUIP_LEVEL_UP_WND)
	ADDAUTOWND(GemCondenseWnd,MSG_CTRL_GEMCONDENSE_WND)
	ADDAUTOWND(GreetingWnd,MSG_CTRL_GREETING_MSG_WND)
	ADDAUTOWND(GuildAddMember,MSG_GUILDADDMEMBER_WND)
	ADDAUTOWND(InputMiBaoWnd,MSG_CTRL_INPUT_MIBAO_LATER_WND)
	ADDAUTOWND(InputUuidWnd,MSG_CTRL_INPUT_MIBAO_WND)
	ADDAUTOWND(GuildAllyWnd,MSG_CTRL_GUILD_ALLYWND)
	ADDAUTOWND(GuildDelMember,MSG_GUILDDELMEMBER_WND)
	ADDAUTOWND(GuildEdit,MSG_GUILD_EDIT)
	ADDAUTOWND(InputYelloFlagWnd,MSG_CTRL_YELLOW_FLAG_WND)
	ADDAUTOWND(LeaveMessageWnd,MSG_CTRL_LEAV_MESSAGE_WND)
	ADDAUTOWND(RadioWnd,MSG_CTRL_RADIO_WND)
	//ADDAUTOWND(UploadWnd,MSG_CTRL_UPLOAD_WND)
	ADDAUTOWND(NameWnd,MSG_CTRL_NAMEWND)
	ADDAUTOWND(NoticeHelpWnd,MSG_CTRL_NOTICE_HELP_WND)
	ADDAUTOWND(PersonPanelWnd,MSG_CTRL_PERSON_PANEL_WND)
	ADDAUTOWND(PetPackageWnd,MSG_CTRL_PET_PACKAGE_WND)
	ADDAUTOWND(PetWnd,MSG_CTRL_PET_WND)
	ADDAUTOWND(OtherPlayerEquip,MSG_CTRL_OTHER_PLAYER_INFO)
	ADDAUTOWND(RangeWnd,MSG_CTRL_RANGE_WND)
	ADDAUTOWND(ReadQuitWnd,MSG_CTRL_READ_QUIT_WND)
	ADDAUTOWND(RelationWnd,MSG_CTRL_RELATION_WND)
	ADDAUTOWND(SmallGemCon,MSG_CTRL_SMALL_GEM_CON_WND)
	ADDAUTOWND(SplitStoneWnd,MSG_CTRL_SPLITSTONE_WND)
	ADDAUTOWND(SysConfigWnd,MSG_CTRL_SYSCONFIG_WND)
	ADDAUTOWND(TaskWnd,MSG_CTRL_TASK_WND)
	ADDAUTOWND(TeamWnd,MSG_CTRL_TEAMWND)
	ADDAUTOWND(TradeWnd,MSG_CTRL_TRADEWND)
	ADDAUTOWND(UseObjectWnd,MSG_CTRL_USE_OBJECT_WND)
	ADDAUTOWND(UserInfoWnd,MSG_CTRL_USERINFO_WND)
	ADDAUTOWND(ReliveWithYuanBaoWnd,MSG_CTRL_RELIVEWITHYUANBAO_WND)
	ADDAUTOWND(ReliveWnd,MSG_CTRL_RELIVE_WND)
	ADDAUTOWND(SmltKeyBoard,MSG_CTRL_SOFT_KEYBOARD_CREATE)
	ADDAUTOWND(ChangePasswordWnd,MSG_CTRL_OPEN_CHANGEPASSWORDWND)
	ADDAUTOWND(RegisterInfoWnd,MSG_CTRL_REGISTER_INFO_WND)
	ADDAUTOWND(CheckMessageBox,MSG_CTRL_CHECK_QUICK_WND)
	ADDAUTOWND(ExternMessgeBoxWnd,MSG_CTRL_EXTERN_MESSAGEBOX_WND)
	ADDAUTOWND(ExLeavWordsWnd,MSG_CTRL_EX_LEAVWORDS_WND)
#ifdef _CHAT
	ADDAUTOWND(VoiceWnd,MSG_CTRL_VOICE_WND)
	ADDAUTOWND(VoiceChatWnd,MSG_CTRL_VOICE_CHAT_WND)
	ADDAUTOWND(VoiceAddWnd,MSG_CTRL_VOICE_ADD_WND)
	ADDAUTOWND(VoiceCfgWnd,MSG_CTRL_VOICE_CONFIG_WND)
	ADDAUTOWND(VoiceSearchWnd,MSG_CTRL_VOICE_SEARCH_WND)
	ADDAUTOWND(VoicePasswordWnd,MSG_CTRL_VOICE_PASSWORD_WND)
#endif
	ADDAUTOWND(SetQuickItemWnd,MSG_CTRL_QUICKCONSUMESET_WND)
	ADDAUTOWND(WoolStorBuyCount,MSG_CTRL_WOOOLSTORE_BUYCOUNT_WND)
	ADDAUTOWND(WoolStorSendWnd,MSG_CTRL_WOOOLSTORE_SEND_WND)
	ADDAUTOWND(WoolStorSendOKWnd,MSG_CTRL_WOOOLSTORE_SEND_OK_WND)
	ADDAUTOWND(WoolStorCleanWnd,MSG_CTRL_WOOOLSTORE_RECV_INFO_WND)
	ADDAUTOWND(WoolStorRecvOKWnd,MSG_CTRL_WOOOLSTORE_RECV_OK_WND)
	ADDAUTOWND(PrayTreeWnd,MSG_CTRL_PRAYTREE_WND)
	ADDAUTOWND(RegisterWnd,MSG_CTRL_REGISTER_WND)
    ADDAUTOWND(HeartKnackWnd,MSG_CTRL_HEARTKNACK_WND)
	ADDAUTOWND(SealWnd,MSG_CTRL_SEAL_WND)
	ADDAUTOWND(SealBookWnd,MSG_CTRL_SEALBOOK_WND)
	ADDAUTOWND(ReservePasswordWnd,MSG_CTRL_RESERVEPASSWORD_WND)
    ADDAUTOWND(LuxNpcWnd,MSG_CTRL_LUXNPCWND)
    ADDAUTOWND(WuXingPrizeWnd,MSG_CTRL_WUXING_PRIZE_WND)
    ADDAUTOWND(CheckOpenWnd,MSG_CTRL_CHECK_OPEN_WND)
    ADDAUTOWND(HuanHuaWnd,MSG_CTRL_HUANHUA_WND)
	ADDAUTOWND(BoxWnd,MSG_CTRL_LINGXIBOX_WND)
	ADDAUTOWND(MazeWnd,MSG_CTRL_MAZE_WND)
	//ADDAUTOWND(IENpcWnd,MSG_CTRL_IENPC_WND)
	ADDAUTOWND(ReceiveMsgWnd,MSG_CTRL_RECEIVE_MSG_WND)
	//ADDAUTOWND(KfzMemberListWnd,MSG_CTRL_KFZ_MEMBER_LIST_WND)
	//ADDAUTOWND(KfzEditMemberWnd,MSG_CTRL_KFZ_EDIT_MEMBER_WND)
	//ADDAUTOWND(KfzIEWnd,MSG_CTRL_KFZ_IE_WND)
	//ADDAUTOWND(KfzResultWnd,MSG_CTRL_KFZ_RESULT)
	//ADDAUTOWND(KfzFlow,MSG_CTRL_KFZ_FLOW_WND)
	//ADDAUTOWND(KfzMemberState,MSG_CTRL_KFZ_MEMBER_STATE_WND)
	//ADDAUTOWND(KfzArenaWnd,MSG_CTRL_KFZ_ARENA_WND)	
	ADDAUTOWND(GuildFlagSelWnd,MSG_CTRL_GUILD_FLAG_SEL_WND)
	ADDAUTOWND(TreasureMapWnd,MSG_CTRL_TREASUREMAP_WND)
	ADDAUTOWND(NpcRecruit,MSG_CTRL_NPCRECRUIT)
	ADDAUTOWND(LicenseWnd,MSG_CTRL_LICENSEWND_WND)
	ADDAUTOWND(License2Wnd,MSG_CTRL_LICENSE2WND_WND)
	ADDAUTOWND(ExtractAndUpgradeYuanshiWnd,MSG_CTRL_EXTRACTUPGRADE_YUANSHI)
	ADDAUTOWND(DelSkillWnd,MSG_CTRL_DEL_SKILL_WND)
	ADDAUTOWND(VipStoreWnd,MSG_CTRL_VIPSTORE_WND)
	ADDAUTOWND(ElixirWnd,MSG_CTRL_ELIXIR_WND)
	ADDAUTOWND(FuShiSeriesWnd,MSG_FUSHI_WND)
	ADDAUTOWND(FairyGuide,MSG_CTRL_FAIRYGUIDE_WND)
	ADDAUTOWND(MemoireWnd,MSG_CTRL_MEMOIRE_WND)
	ADDAUTOWND(ProduceWnd,MSG_CTRL_PRODUCE_WND)
	ADDAUTOWND(LuckQyzWnd,MSG_CTRL_LUCKQYZ_WND)
	ADDAUTOWND(BlessZhongZhouWnd,MSG_CTRL_BLESSZHONGZHOU_WND)
	ADDAUTOWND(InstanceZonePopupWnd,MSG_CTRL_InstanceZonePopUp_Wnd)
	ADDAUTOWND(ActivityLogWnd,MSG_CTRL_ACTIVITY_LOG_WND)
	ADDAUTOWND(BaoJianWnd,MSG_CTRL_BAOJIAN_WND)
	ADDAUTOWND(TaskTreeDesWnd,MSG_CTRL_TASKTREEDES_WND)
	ADDAUTOWND(YuanBaoWnd,MSG_CTRL_YUANBAO_WND)
	ADDAUTOWND(TigerRuneWnd,MSG_CTRL_TIGERRUNE_WND)
	ADDAUTOWND(MeritoriousWnd,MSG_CTRL_MERITORIOUS_WND)
	ADDAUTOWND(RTSInstanceWnd,MSG_CTRL_RTSINSTANCE_WND)
// 	ADDAUTOWND(RtsGuideWnd,MSG_CTRL_RTSGUIDE_WND)
	ADDAUTOWND(ShortCutKeyWnd,MSG_CTRL_SHORTCUTKEY_WND)
	ADDAUTOWND(TodayActivityWnd,MSG_CTRL_TODAYACTIVITY_WND)
	ADDAUTOWND(TodayActivityIEWnd,MSG_CTRL_TODAYACTIVITY_IE_WND)
	ADDAUTOWND(TodayActivityLocalWnd,MSG_CTRL_TODAYACTIVITY_LOCAL_WND)
	ADDAUTOWND(TaskWuSeDailyWnd,MSG_CTRL_TASKWUSEDAILY_WND)


// 	ADDAUTOWND(TneupChartWnd, MSG_CTRL_TNEUPCHART_WND)
// 	ADDAUTOWND(TneupCtrlPanel, MSG_CTRL_TNEUPCTRLPANEL_WND)
// 	ADDAUTOWND(TneupWnd, MSG_CTRL_TNEUP_WND)
// 	ADDAUTOWND(TrusteeshipConfigWnd, MSG_CTRL_TRUSTEESHIPCONFIG_WND)
// 	ADDAUTOWND(QuitTneupWnd, MSG_CTRL_QUITTNEUP_WND)
// 	ADDAUTOWND(QunYingWithExpWnd,MSG_CTRL_QUNYING_EXP_WND)
// 	ADDAUTOWND(QuickQunYingWnd,MSG_CTRL_QUICKQUNYING_WND)
// 	ADDAUTOWND(QunYingReportWnd,MSG_CTRL_QUNYING_REPORT_WND)
// 	ADDAUTOWND(ZhenPuCuiLianWnd,MSG_CTRL_ZHENPUCUILIAN_WND)	
// 	ADDAUTOWND(OfflineTrusteeshipWnd,MSG_CTRL_OFFLINETRUSTEESHIP_WND)	

	ADDAUTOWND(LeaseBoothWnd,MSG_CTRL_LEASEBOOTH_WND)
	ADDAUTOWND(LeaseOtherBoothWnd,MSG_CTRL_LEASEOTHERBOOTH_WND)
	ADDAUTOWND(FirmWnd,MSG_CTRL_FIRM_WND)
	ADDAUTOWND(FirmDetailInfoWnd,MSG_CTRL_FIRMDETAIL_WND)
	ADDAUTOWND(EquipSoulWnd,MSG_CTRL_EQUIPSOUL_WND)
	ADDAUTOWND(DivinityWingWnd,MSG_CTRL_DIVINITYWING_WND)


	ADDAUTOWND(QuickItemFashionalWnd,MSG_CTRL_QUICKCONSUME_FASHIONAL_WND)
	ADDAUTOWND(ChatPanelWnd,MSG_CTRL_CHATPANEL_WND)

	ADDAUTOWND(AuthenWnd,MSG_CTRL_AUTHEN_WND)

	ADDAUTOWND(BlessCompoundWnd,MSG_CTRL_BLESSCOMPOUND_WND)
	ADDAUTOWND(WenPeiWnd,MSG_CTRL_WENPEI_WND)
	ADDAUTOWND(EnterShanWnd,MSG_CTRL_ENTERSHAN_WND)

	ADDAUTOWND(AdventureWnd,MSG_CTRL_ADVENTURE_WND)
	ADDAUTOWND(CommonSelectWnd,MSG_CTRL_COMMON_SELECT_WND)
	ADDAUTOWND(ComposeEquipWnd,MSG_CTRL_COMPOSEEQUIP_WND)
	ADDAUTOWND(BoothGuideWnd,MSG_CTRL_BOOTHGUILD_WND)
	
	ADDAUTOWND(SanWeiFireWnd,MSG_CTRL_SANWEIFIRE_WND)
	ADDAUTOWND(FaBaoShowWnd,MSG_CTRL_FABAOSHOW_WND)

	ADDAUTOWND(FaBaoLevelUpWnd,MSG_CTRL_FABAOLEVELUP_WND)
	ADDAUTOWND(FireArtificeWnd,MSG_CTRL_FIREARTIFICE_WND)
	ADDAUTOWND(NoticeIEWnd,MSG_CTRL_NOTICEIE_WND)

	ADDAUTOWND(OtherPlayerFaBaoEquip,MSG_CTRL_OTHERPLAYER_FABAOSHOW_WND)
	ADDAUTOWND(VipCardWnd,MSG_CTRL_VIPCARD_WND)
	ADDAUTOWND(LingWuSanWeiFire,MSG_CTRL_LINGWUSANWEIFIRE_WND)

	ADDAUTOWND(Defend12GongWnd,MSG_CTRL_DEFEND12GONG_WND)
	ADDAUTOWND(Defend12CtrlWnd, MSG_CTRL_DEFEND12CTRL_WND)
	ADDAUTOWND(YiHuoZhanMsgWnd,MSG_CTRL_YIHUOZHANMSG_WND)

	ADDAUTOWND(LoginCheckMaskWnd,MSG_CTRL_LOGIN_CHECK_MASK_WND)
	ADDAUTOWND(PlayerPanelWnd,MSG_CTRL_PLAYER_PANEL_WND)
	ADDAUTOWND(EquipChaifenWnd,MSG_CTRL_EQUIPCHAIFEN_WND)

	ADDAUTOWND(HuFuFlushWnd,MSG_CTRL_HUFUFLUSH_WND)

	ADDAUTOWND(CaiHongHelpWnd,MSG_CTRL_CAIHONGHELP_WND)
	ADDAUTOWND(YuShouLevelUpWnd,MSG_CTRL_YUSHOULEVELUP_WND)

	ADDAUTOWND(QiShouWnd,MSG_CTRL_QISHOU_WND)
	ADDAUTOWND(BindPTWnd,MSG_CTRL_BINDPTWND_WND)
	ADDAUTOWND(BindPTMsgWnd,MSG_CTRL_BINDPTMSGWND_WND)

}

CMainWnd::~CMainWnd(void)
{
	ClearChildControl();
}

void CMainWnd::OnCreate()
{
}

void CMainWnd::OnSetFocus()
{
	if(m_pLoginWnd)
	{
		g_pControl->Msg(MSG_CTRL_FOCUSLOGIN,0);
	}
}

bool CMainWnd::OnLeftButtonUp(int iX,int iY)
{
	return false;
}

bool CMainWnd::OnLeftButtonDown(int iX,int iY)
{
	return false;
}

int CMainWnd::GetWidth()
{
	return g_pGfx->GetWidth(); 
}

int CMainWnd::GetHeight()
{ 
	return g_pGfx->GetHeight();
}

void CMainWnd::ReSetAllWndPos()
{
	if(m_pGameWnd)
		m_pGameWnd->ResetControlPos();

	if(m_pLoginWnd)
		m_pLoginWnd->ResetControlPos();

	if(m_pSelectCharWnd)
		m_pSelectCharWnd->ResetControlPos();

	if(m_pQuitWnd)
		m_pQuitWnd->ResetControlPos();

	if(m_pTalkViewWnd)
		m_pTalkViewWnd->ResetControlPos();

	ReSetUI(g_EutUiType);//从全屏模式变成窗口模式,如果有缩放,要重新创建要求缩放的UI界面
	
	CCtrlMainWin::ReSetAllWndPos();
}

bool CMainWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	TRY_BEGIN;
	//自定义窗口消息
	//Dispatch Message
	if(dwMsg > MSG_INPUT_BEGIN && dwMsg < MSG_INPUT_END)
	{
		if(Msg_MainSystem(dwMsg,dwData,pControl))
			return true;
	}
	else if(dwMsg > MSG_CTRL_NORMAL_BEGIN && dwMsg < MSG_CTRL_NORMAL_END)
	{
		return CCtrlMainWin::Msg(dwMsg,dwData,pControl);
	}
	else if(dwMsg > MSG_CTRL_MAINWND_MANAGE_BEGIN && dwMsg < MSG_CTRL_MAINWND_MANAGE_END)
	{
		return Msg_ManageChild(dwMsg,dwData,pControl);
	}
	else if(dwMsg > MSG_CTRL_MESSAGEBOX_BEGIN && dwMsg < MSG_CTRL_MESSAGEBOX_END)
	{
		return Msg_MessageBox(dwMsg, dwData, pControl);
	}
	else if(dwMsg > MSG_CTRL_MAINWND_OTHER_BEGIN && dwMsg < MSG_CTRL_MAINWND_OTHER_END)
	{
		return Msg_MainOther(dwMsg,dwData,pControl);
	}
	else if(dwMsg > MSG_GAMEWNDS_MESSAGE_BEGIN && dwMsg < MSG_GAMEWNDS_MESSAGE_END)
	{
		return Msg_GameWnds(dwMsg,dwData,pControl);
	}
	else if(dwMsg > MSG_CTRL_TALKVIEWWND_BEGIN && dwMsg < MSG_CTRL_TALKVIEWWND_END)
	{
		if(m_pTalkViewWnd)
			return m_pTalkViewWnd->Msg(dwMsg,dwData,pControl);
		return true;
	}
	else if(dwMsg > MSG_CTRL_GAMEWND_BEGIN && dwMsg < MSG_CTRL_GAMEWND_END)
	{
		if(m_pGameWnd)
			return m_pGameWnd->Msg(dwMsg,dwData,pControl);
		return true;
	}
	else if(dwMsg > MSG_CTRL_LOGINWND_BEGIN && dwMsg < MSG_CTRL_LOGINWND_END)
	{
		if(m_pLoginWnd)
			return m_pLoginWnd->Msg(dwMsg,dwData,pControl);
	}
	else if(dwMsg > MSG_CTRL_SELECTCHAR_BEGIN && dwMsg < MSG_CTRL_SELECTCHAR_END)
	{
		if(m_pSelectCharWnd)
			return m_pSelectCharWnd->Msg(dwMsg,dwData,pControl);
	}
	else
	{
		OutputDebugString("CMainWnd::Msg() : invalid message\n");
		return true;
	}
	return CCtrlMainWin::Msg(dwMsg,dwData,pControl);
	TRY_END;
	return false;
}

void CMainWnd::DrawMouse()
{
	//画鼠标上的物品
	int iMouseX,iMouseY;
	g_pInput->GetMousePos(iMouseX,iMouseY);
	CGoodGrid::DrawSelectedGood(iMouseX,iMouseY);

	if(CGoodGrid::GetDropGoodFrom().DropGood.GetID() != 0)
	{
		g_pGameData->SetSkillShortCut(0,0,-1,-1);
	}
	else
	{
		int mid = g_pGameData->GetSkillShortCut().iMagicID;
		LPTexture pTex = NULL;

		SkillShortCut& lSkillShortCut = g_pGameData->GetSkillShortCut();
		if (mid > 0)
		{
			pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,GetTexIDByMagicID(mid),EP_UI);
			g_pGfx->DrawTextureNL(iMouseX - 16,iMouseY - 16,pTex);
		}
		else if (lSkillShortCut.pConSkill)
		{
			mid = lSkillShortCut.pConSkill->GetCurrentMagicID();

			pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,GetTexIDByMagicID(mid),EP_UI);
			g_pGfx->DrawTextureNL(iMouseX - 16,iMouseY - 16,pTex);
		}
	}

	LPTexture pTex = NULL;
	if(g_pGameData->GetDropMoneyFrom() !=  EDMYF_NONE)
	{
		pTex = g_pTexMgr->GetTexImm(PACKAGE_items,229,EP_UI);
	}
	else if(g_pGameData->GetDropYuanBaoFrom()  !=  EDMYF_NONE)
	{
		pTex = g_pTexMgr->GetTexImm(PACKAGE_items,488,EP_UI);
	}

	if(pTex)
	{
		g_pGfx->DrawTextureNL(iMouseX - pTex->GetWidth0() / 2,iMouseY - pTex->GetHeight0() / 2,pTex);
	}
	//快捷消费物品
	CQuickItem qitem = g_WooolStoreMgr.GetDropQuickItem(); //鼠标上面的物品
	if(!qitem.strItemID.empty())
	{
		pTex = g_pTexMgr->GetTexImm(PACKAGE_items,qitem.iItemLooks,EP_UI);
		if(pTex != NULL)
		{
			g_pGfx->DrawTextureNL(iMouseX - 16,iMouseY - 16,pTex,0xCCFFFFFF);
		}

	}
	//绘制鼠标
	if(g_pGameData->IsSelfCursor())
	{
		int x,y;
		g_pInput->GetMousePos(x,y);
		if( x<0 || y<0 )
		{
			if( !g_pGameData->IsHardCursor() )
			{
				g_pGameData->SetHardCursor(true);
				::ShowCursor(true);
			}
		}
		else
		{
			if( g_pGameData->IsHardCursor() )
			{
				g_pGameData->SetHardCursor(false);
				::ShowCursor(false);
			}

			pTex = g_pTexMgr->GetTexImm(g_pGameData->GetMouseTexID(),EP_UI);
			if(pTex)
			{
				if (g_pGameData->GetMouseTexRenderMode() != RM_ALPHA)
					g_pGfx->SetRenderMode((RenderMode)g_pGameData->GetMouseTexRenderMode());				
				
				g_pGfx->DrawTextureNL(iMouseX,iMouseY,pTex);

				if (g_pGameData->GetMouseTexRenderMode() != RM_ALPHA)
					g_pGfx->SetRenderMode();
			}
		}


#ifdef ENABLE_REPLAY
		if(g_ReplayManager.IsInReplay())
		{
			char szTemp[128] = {0};
			sprintf(szTemp,"FrameCountNow:%d,Speed:%f",g_pGfx->GetFrameCount(),g_pGfx->GetLastSetFPS() / 50.0f);
			g_pFont->DrawText(10,10,szTemp,-1,FONT_SONGTI,FONTSIZE_MIDDLE);
		}
#endif

	}


	//if (g_Config.IsShowTestFontText())
	//{
	//	///////////////////////
	//	DWORD dwFlag = 0;//DTF_BlackFrame;
	//  g_pFont->DrawText(80,55,"12:abcxyzABCXYZfpjl何国辉,在球棒 鬼地方一地方官",-1,FONT_SONGTI,16);
	//	g_pFont->DrawText(80,55,"12:abcABC何国辉,在球棒 鬼地方一地方官",-1,FONT_YAHEI,12,dwFlag);
	//	g_pFont->DrawText(80,70,"18:abcABC何国辉,在球棒 鬼地方一地方官",-1,FONT_YAHEI,18,dwFlag);

	//	//////////////////////
	//	g_pFont->DrawText(80,95,"12:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,12,dwFlag);
	//	g_pFont->DrawText(80,115,"14:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,14,dwFlag);
	//	g_pFont->DrawText(80,135,"16:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,16,dwFlag);
	//	g_pFont->DrawText(80,155,"18:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,18,dwFlag);
	//	g_pFont->DrawText(80,175,"30:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",-1,FONT_YAHEI,30,dwFlag);

	//	///////////////////////
	//	g_pFont->DrawText(80,200 + 55,"12:中州abcABC何国辉,在球棒 鬼地方一地方官",0xFFFFFF00,FONT_YAHEI,12,dwFlag);
	//	g_pFont->DrawText(80,200 + 70,"18:中州abcABC何国辉,在球棒 鬼地方一地方官",0xFFFFFF00,FONT_YAHEI,18,dwFlag);

	//	//////////////////////
	//	g_pFont->DrawText(80,200 + 95,"12:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",0xFFFFFF00,FONT_YAHEI,12,dwFlag);
	//	g_pFont->DrawText(80,200 + 115,"14:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",0xFFFFFF00,FONT_YAHEI,14,dwFlag);
	//	g_pFont->DrawText(80,200 + 135,"16:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",0xFFFFFF00,FONT_YAHEI,16,dwFlag);
	//	g_pFont->DrawText(80,200 + 155,"18:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",0xFFFFFF00,FONT_YAHEI,18,dwFlag);
	//	g_pFont->DrawText(80,200 + 175,"30:中州新王月abcABC爷热源的犹太人主徼优胜者枯地枯琦一在珍仍和有和遥舶",0xFFFFFF00,FONT_YAHEI,30,dwFlag);
	//	g_pFont->DrawText(80,200 + 210,"30:123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",0xFFFFFF00,FONT_YAHEI,30,dwFlag);
	//}
}

void CMainWnd::Draw(void)
{

    CCtrlMainWin::Draw();

	//绘制技能吟唱条
	if (g_pGameData->GetSingingTechID() > 0 && g_pGameData->GetSingingTimeStart() > 0)
	{
		DWORD curTime = GetTickCount();
		
		//最大吟唱时间
		WORD wMagicID = g_pGameData->GetSingingTechID();
		CMagicData* pMagic = SELF.MagicArray().FindMagic(wMagicID);
		DWORD magicStartTime = g_pGameData->GetSingingTimeStart();

		//技能图标
		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,GetTexIDByMagicID(wMagicID),EP_UI);

		DWORD maxTechTime = 0;
		if (pMagic != NULL)
		{
			switch (wMagicID)
			{
			case MAGICID_COLDSTROM:
				{
					maxTechTime = 2000 + pMagic->GetMagicLevel()*1000;
				}
				break;
			}
		}

		if (maxTechTime != 0)
		{
			int iWidth = g_pGfx->GetWidth();
			int iHeight = g_pGfx->GetHeight();
			
			LPTexture tech = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,4,EP_UI);		//技能框
			LPTexture tiao = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,5,EP_UI);		//吟唱框
			LPTexture jindu = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,6,EP_UI);		//吟唱进度
			LPTexture point = g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,7,EP_UI);		//吟唱亮点

			if (jindu)
			{
				int ox = iWidth/2 - jindu->GetWidth()/2;
				int oy = iHeight/4*3;
				g_pGfx->DrawTextureNL(ox - 40,oy - 10,tech);
				g_pGfx->DrawTextureNL(ox - 37,oy - 8,pTex);

				g_pGfx->DrawTextureNL(ox,oy,tiao);
				RECT jinduRect;
				jinduRect.left = 0;
				jinduRect.right = (int)(min((float)(curTime - magicStartTime) / maxTechTime,1.0f) * jindu->GetWidth());
				jinduRect.top = 0;
				jinduRect.bottom = jindu->GetHeight();
				g_pGfx->DrawPartTexture(ox,oy,jindu,0,0,jinduRect.right,-1);
				
				if(point)
					g_pGfx->DrawTextureNL(ox + jinduRect.right - point->GetWidth()/2,oy - 2,point);
			}
			
			
			

			//如果已达最大吟唱时间,释放魔法
			if (curTime - magicStartTime >= maxTechTime || (curTime - magicStartTime > pMagic->GetMagicDelay() && !g_pInput->IsKeyDown(VK_F1 + pMagic->GetShortCutKey() -0x31)))
			{
				g_pGameData->SetSingingTimeStart(0);
				g_pGameData->SetSingingTechID(0);
				int x,y;
				g_pGameMgr->GetMouseTile(x,y);
				g_pGameControl->SEND_Player_Attack_Magic(g_pControl->GetMouseOnID(),x,y,wMagicID);

				SELF.RemoveUsingMagicType(UMT_NOACTION);//以防服务器没有取消掉吟唱状态导致无法移动
			}
		}
		
	}
	

	if(g_Config.GetShowUIHelp())
	{
		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,15504,EP_UI);
		g_pGfx->DrawTextureNL(0,0,pTex,0x80ffffff);
	}
	
	//绘制控件的TIPS,tips要在所有控件的最上层绘制
	if(m_uMouseOnID == MOUSE_ON_CONTROL && m_uMouseOnCtrl)	// 已经设置了tips的内容
	{
		const char* s = (char *)m_uMouseOnCtrl->IsShowTips(g_pGfx->GetFrameCount());
		if(s)
		{
			int iMouseX,iMouseY;
			g_pInput->GetMousePos(iMouseX,iMouseY);
			m_pCtrlTip->SetText(s);
			m_pCtrlTip->Move(m_iMouseX,m_iMouseY-m_pCtrlTip->GetHeight());
			m_pCtrlTip->Draw();
		}
	}
	
	if(m_pParserTip->IsShow())//鼠标指向其它窗口上的物品要求显示tips的
	{
		m_pParserTip->Draw();
	}

	DrawMouse();
	g_pAudio->TimerHandler();

	//绘制物品提示信息
	CFindGood::VFindData& VStr = g_FindGood.GetDisplayString();
	for(int ii = 0;ii < VStr.size();ii++)
	{
		TextOut(10,30+ii*16,VStr.at(ii).str.c_str(),0xFFFFFFFF,DTF_BlackFrame);
	}
	if(VStr.size() != 0)
	{
		//1秒以后删除该信息
		if(GetTickCount() - VStr.front().dwCount > 2000)
		{
			VStr.erase(VStr.begin());
		}
	}

#ifdef _CHAT
	//登陆搜索服务器延时处理
	if(g_VoiceAdapter.GetWaitLoginSearchStartTime() && GetTickCount() - g_VoiceAdapter.GetWaitLoginSearchStartTime() > 2100)
	{
		g_VoiceAdapter.SearchSeverLogin();
		g_VoiceAdapter.SetWaitLoginSearchStartTime(0);
	}

	if(g_VoiceAdapter.GetWaitLoginVoiceSeverStartTime() && GetTickCount() - g_VoiceAdapter.GetWaitLoginVoiceSeverStartTime() > 2100)
	{
		g_VoiceAdapter.ReLoginKeepOn();
		g_VoiceAdapter.SetWaitLoginVoiceSeverStartTime(0);
	}
#endif
	if(g_OtherData.GetReadUsbEkeyDelayStartTime() > 0 && GetTickCount() - g_OtherData.GetReadUsbEkeyDelayStartTime() > 100)
	{
		Sleep(300);
		g_OtherData.SetReadUsbEkeyDelayStartTime(0);
		//g_pGameControl->SEND_UsbMb(1,2);
	}

	if(g_OtherData.GetCreditAlertState() == 1 && g_pSDOAInterface && g_pSDOAInterface->GetSDOAApp())// && g_pSDOAInterface->GetLoginStateA(LOGINMETHOD_SSO) == LOGINSTATE_OK)
	{
		if (GetTickCount() - g_OtherData.GetEnterGameTime() > 3000)
		{
			//有欠款或未开通手机验证弹出提醒页面
			if (SELF.GetCreditArrearage() != 0 || SELF.GetCreditState() == 1)
			{
				if (SDOA_FALSE == (g_pSDOAInterface->OpenWidgetEx("sdoCredit","gatetype=3",0)))
				{
					output_debug("OpenWidgetExA(\"sdoCredit\",\"gatetype=3\",0) Fail\r\n");
				}
			}

			g_OtherData.SetCreditAlertState(100);	
		}
	}


	if (g_bHasUiInDownloading)
	{
		LPTexture pTex = g_pTexMgr->GetTex(PACKAGE_INTERFACE,23210,EP_UI);
		
		if (pTex)
		{
			g_pGfx->SetRenderMode(RM_ADD2);
			g_pGfx->DrawTextureNL(g_pGfx->GetWidth() / 2 - 150,g_pGfx->GetHeight() - 180,pTex);
			g_pGfx->SetRenderMode();
		}

		//g_pFont->DrawText(g_pGfx->GetWidth() / 2 - 36,g_pGfx->GetHeight() - 180,"资源下载中......",0xFFFFFF00,FONT_YAHEI,FONTSIZE_MIDDLE,DTF_BlackFrame);
	}

	////多开限制
	//if (g_AntiWaiGua.IsWaiGua())
	//{
	//	g_pGameControl->SEND_HasWaiGua(1);
	//	g_AntiWaiGua.ClearState();
	//}

}

bool CMainWnd::Msg_ManageChild(DWORD dwMsg, DWORD dwData, CControl * pControl)
{
	switch(dwMsg)
	{
	case MSG_CTRL_GAMEWND:
        {
			if(dwData == OPER_CLOSE)
			{
				REMOVECONTROL(m_pGameWnd);
				REMOVECONTROL(m_pTalkViewWnd);
				REMOVECONTROL(m_pQuitWnd);
			}
		    else if( m_dwWindowMode != dwMsg )
		    {
			    g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CLOSE); //关闭角色窗口
				g_pControl->Msg(MSG_CTRL_LOGINWND,OPER_CLOSE);//关于登录窗口,跨服战时不会创建selectcharwnd,所以在进入游戏的时候在这里关掉登录窗口

			    m_dwWindowMode = dwMsg;
			    m_pGameWnd = new CGameWnd();
			    AddControl(m_pGameWnd);
			    m_pGameWnd->Create(this,0,0,m_iWidth,m_iHeight);

				PopupWindow(MSG_CTRL_PANEL_WND,OPER_CREATE);//panel wnd

                m_pTalkViewWnd = new CTalkViewWnd();
                AddControl(m_pTalkViewWnd);
                m_pTalkViewWnd->Create(this);
		    }
		    return true;
        }

	case MSG_CTRL_LOGINWND:
        {
		    if(dwData == OPER_CLOSE)
		    {
			    REMOVECONTROL(m_pLoginWnd);
		    }
		    else if( m_dwWindowMode != dwMsg )
		    {
				g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CLOSE); //关闭角色窗口,从角色窗口可以返回登录界面

			    m_dwWindowMode = dwMsg;
			    m_pLoginWnd = new CLoginWnd();
			    AddControl(m_pLoginWnd);
			    m_pLoginWnd->Create(this,0,0,m_iWidth,m_iHeight);
			    m_pLoginWnd->SetFocus();
		    }
		    return true;
        }

	case MSG_CTRL_SELECTCHARWND:
        {
            if(dwData == OPER_CLOSE)
		    {
			    REMOVECONTROL(m_pSelectCharWnd);
		    }
		    else if(m_dwWindowMode != dwMsg)
		    {
			    g_pControl->Msg(MSG_CTRL_LOGINWND,OPER_CLOSE); //关闭登陆窗口

				REMOVECONTROL(m_pSelectCharWnd);

			    m_dwWindowMode = dwMsg;
			    m_pSelectCharWnd = new CSelectCharWnd();
			    AddControl(m_pSelectCharWnd);
			    m_pSelectCharWnd->Create(this,0,0,m_iWidth,m_iHeight);
			    m_pSelectCharWnd->SetFocus();
		    }
		    return true;
        }

	case MSG_CTRL_QUIT_WND:
        {
		    if(dwData == OPER_CLOSE)
		    {
			    REMOVECONTROL(m_pQuitWnd);
				SetTopWindowFocus();
		    }
		    else if(dwData == OPER_CUSTOM || (dwData == OPER_CUSTOM + 2)) //退到登陆界面,OPER_CUSTOM + 2:不发大退协议
		    {
				ClearChildControl();

			    g_pGameControl->SEND_Quit_Game(dwData == OPER_CUSTOM);

				g_pControl->Msg(MSG_CTRL_GAMEWND,OPER_CLOSE);
				g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CLOSE);
				g_pControl->Msg(MSG_CTRL_LOGINWND,OPER_CREATE);
			    REMOVECONTROL(m_pQuitWnd);
		    }
			else if(dwData == OPER_CUSTOM+1) //退到角色界面
			{
				if((g_dwServerSwitch & SS_QUIT_DELAY) && SELF.GetID() != 0 && !SELF.IsDead())//读秒退出
				{
					g_pControl->PopupWindow(MSG_CTRL_READ_QUIT_WND,OPER_CREATE,0);
				}
				else
				{
					g_pControl->Msg(MSG_CTRL_GAMEWND,OPER_CLOSE);
					ClearChildControl();

					g_pGameControl->SEND_Out_Game();	
				}
			}
			else if (dwData == OPER_CUSTOM + 3)
			{
				if (m_pQuitWnd)
				{
					m_pQuitWnd->SetMode(true);
					m_pQuitWnd->SetFocus();
				}
			}
		    else
		    {
			    if(m_pQuitWnd)
			    {
				    m_pQuitWnd->SetFocus();
				    return true;
			    }
			    m_pQuitWnd = new CQuitWnd();
			    AddControl(m_pQuitWnd);
			    m_pQuitWnd->Create(this);
			    m_pQuitWnd->SetFocus();
		    }
		    return true;
        }
	case MSG_CTRL_DISPLAY_MODE:
		{
			int iW = g_pGfx->GetWidth();
			int iH = g_pGfx->GetHeight();

			ReSize(iW,iH);
			if(m_pLoginWnd)
				m_pLoginWnd->ReSize(iW,iH);

			if(m_pSelectCharWnd)
			{
				m_pSelectCharWnd->ReSize(iW,iH);
				m_pSelectCharWnd->Msg(MSG_CTRL_DISPLAY_MODE,0);
			}

			if(m_pGameWnd)
			{
				m_pGameWnd->ReSize(iW,iH);

				CCtrlWindow* pWnd = FindWindowByName("HandyMapWnd");
				if(pWnd)
				{
					pWnd->Move(g_pControl->GetWidth() - 140,0);
				}
			}

			g_TalkMgr.Refill();

			ReSetAllWndPos();

			return true;
		}
	default:
		break;
	}
	return false;
}

bool CMainWnd::Msg_MainSystem(DWORD dwMsg, DWORD dwData, CControl * pControl)
{
	TRY_BEGIN;
	char c = 0;
	WORD wKey;
	WORD w1 = HIWORD( dwData );
	WORD w2 = LOWORD( dwData );

	if(!g_pInput)
		return false;

	switch (dwMsg)
	{
	case MSG_INPUT_KEYDOWN:			// 这些消息只提供给焦点控件
		{
			wKey = LOWORD(dwData);
			if(!g_pInput)
			{
				return false;
			}

			//拖管模式下，不是对队不能做任何操作
// 			if (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain())
// 			{
// 				//可以聊天
// 				if(m_pPanelWnd)
// 				{
// 					m_pPanelWnd->Msg(dwMsg,dwData,pControl);
// 				}
// 				return true;
// 			}

			//检查界面的快捷键
			if(DoAccelKey(wKey))
			{
				return true;
			}
			
			if(wKey == VK_ESCAPE)//ESC键先给聊天栏输入框先处理,如果聊天栏有光标,有假焦点(fakefocus),按Esc清楚聊天栏输入框内容,再按聊天栏输入框失去假焦点,再按关闭最上层窗口
			{
				if(m_pTalkViewWnd && m_pTalkViewWnd->Msg(dwMsg,dwData,pControl))
				{
					return true;
				}
			}

			if(m_pFocusNow && m_pFocusNow != this && m_pFocusNow != m_pGameWnd && m_pFocusNow->IsEnable() && m_pFocusNow->Msg(dwMsg,dwData,pControl)) 
			{
				return true;
			}

			if(m_pTalkViewWnd && m_pTalkViewWnd->Msg(dwMsg,dwData,pControl))
			{
				return true;
			}


			switch(wKey)
			{
			case VK_F1:
#ifdef _DEBUG
				{
					//g_pControl->PopupWindow(MSG_CTRL_SKILLWND,OPER_RECREATE);
					//g_pControl->PopupWindow(MSG_CTRL_BINDPTWND_WND,OPER_RECREATE);
					//g_pControl->MsgToWnd("EquipSoulWnd",MSG_CTRL_EQUIPSOUL_WND,101);
					//g_pGameControl->SEND_APLogin_Req("2133code", "sign=cc3e6caca39fc3a73fe162742ab177d4&userid=6704153&time=1331882002;21");					
				}
				break;
#endif
			case VK_F2:
#ifdef _DEBUG
				{
					//g_pControl->PopupWindow(MSG_CTRL_CHARWND,OPER_RECREATE);
					//string url = g_pStreamMgr->GetWebsite("NewUser");
					//string str = "                        错  误\\";
					//str += "您输入的盛大通行证或密码有误，请重新输入或<注册新的盛大通行证/@@OpenUrl " + url + ">。";	
					//g_MsgBoxMgr.PopTagAlert(str.c_str());
				}
				break;
#endif
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
				{
					if(PopupWindow("AssConfigWnd",dwMsg,dwData))//如果魔法设置界面处理了该键按下事件返回
						return true;

					if(m_pGameWnd)
						return m_pGameWnd->Msg(dwMsg,dwData,pControl);
				}
				break;
			case VK_UP:
			case VK_DOWN:
				if(m_pFocusNow && m_pFocusNow->IsNeedKeyInput())
				{
					m_pFocusNow->OnKeyDown(w1,(UCHAR)w2);
					return true;
				}
			case VK_RETURN:
				if(g_pInput->IsAlt())
					return true;
				break;
			case VK_ESCAPE:
			case VK_PRIOR:
			case VK_NEXT:
				break;

			default:
				if(m_pFocusNow && m_pFocusNow == m_pGameWnd && m_pTalkViewWnd)//如果pPanelEdit->IsFakeFocus(),不能把按键消息传到gamewnd否则在输入文字的时候就可能会触发魔扩展快捷键
				{
					CGlyphEdit*  pPanelEdit = m_pTalkViewWnd->GetEditCtrl();
					if(pPanelEdit && pPanelEdit->IsFakeFocus())
					{
						return true;
					}
				}

				break;

			}
			break;
		}
	case MSG_INPUT_KEYUP:
		{
			switch(LOWORD(dwData))
			{
			case VK_SNAPSHOT:
			case VK_PAUSE:
				{
					_mkdir("../SnapShot");
					char sJpgFile[256] = {0};
					string strFileName = g_Timer.GetDateTime("%Y%m%d_%H%M%S");
					sprintf(sJpgFile,"..\\SnapShot\\%s.jpg",strFileName.c_str());
					g_pGfx->SaveSnapShot(sJpgFile);

					
					
					char strPath[MAX_PATH]={0};
					GetCurrentDirectory(MAX_PATH,strPath);
					string strTemp = strPath;

					int iPos = strTemp.find_last_of('\\');
					//如果最后是"\\",去掉
					if (iPos == strTemp.length() - 1)
					{
						strTemp = strTemp.substr(0,iPos);
						iPos = strTemp.find_last_of('\\');
					}

					if (iPos != string::npos)
					{
						if (iPos < strTemp.size() - 1)
						{
							strTemp = strTemp.substr(0,iPos);
						}
					}
					
					
					sprintf(sJpgFile,"截图成功！保存在文件夹%s\\SnapShot\\%s.jpg",strTemp.c_str(),strFileName.c_str());
					g_TalkMgr.PushSysTalk(sJpgFile);

					return true;
				}
				break;
			case VK_RETURN:
				if(g_pInput->IsAlt()) 
					return true;
				break;
			}

			//拖管模式下，不是对队或微操状态下不能做任何操作
// 			if (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain())
// 			{
// 				return true;
// 			}

			//p = m_pFocusNow;
			//while( p && p!=this )
			//{
			//	if( p->IsEnable() && p->Msg(dwMsg,dwData,pControl))
			//		return true;

			//	p = p->GetParent();
			//}
			break;
		}
	case MSG_INPUT_CHAR:
		{
			wKey = LOWORD(dwData);

	 		if(m_pTalkViewWnd)
			{			
				if(m_pTalkViewWnd->Msg(dwMsg,dwData,pControl))
				{
					return true;
				}

				if( (wKey==VK_ESCAPE ) && m_pFocusNow && m_pFocusNow->GetControlMode()==CTRL_MODE_EDIT )
				{
					m_pFocusNow->GetParent()->SetFocus();
					return true;
				}

				//拖管模式下，不是对队不能做任何操作
// 				if (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain())
// 				{
// 					return true;
// 				}

				if(m_pFocusNow && !m_pFocusNow->IsNeedKeyInput())
				{
					switch(wKey)
					{
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
						{
							if(!g_pInput->IsShift())
							{
								int liPos = MAX_PACKAGE_NUM + wKey - '1';
								CGood& tmp = SELF.GetPackageGood(liPos);
								if (g_AIGoodMgr.IsLianHuLu(tmp))
								{
									//炼化葫芦

									if (g_pGameData->GetMouseType() == MOUSE_HULU)
										g_pGameData->SetMouseType(MOUSE_STANDARD);
									else if(!SELF.IsDead() && !SELF.GetBoothState() && !SELF.IsBianShen())
									{
										g_pGameData->SetMouseType(MOUSE_HULU);
										g_pGameData->SetUseHuLuGoodPos(liPos);
									}
								}
								else
								{
									g_pGameControl->SEND_Use_Object(liPos);
								}
								return true;
							}
							break;
						}
// 					case '7':
// 					case '8':
// 					case '9':
// 					case '0':
// 					case '-':
// 					case '=':
// 					case VK_OEM_MINUS://'-'
// 					case VK_OEM_PLUS://'='
// 						{
// 							int iIndex = 0;
// 							if(wKey == '7' || wKey == '8' || wKey == '9')
// 								iIndex = wKey - '7';
// 							else if(wKey == '0')
// 								iIndex = 3;
// 							else if(wKey == VK_OEM_MINUS || wKey == '-')
// 								iIndex = 4;
// 							else if(wKey == VK_OEM_PLUS || wKey == '=')
// 								iIndex = 5;
// 
// 							if(g_Config.GetCheckQuick())
// 							{
// 								if(iIndex >= 0 && iIndex < QUICKITEM_COUNT && !(g_WooolStoreMgr.GetQuickItem(iIndex).strName.empty()) && g_WooolStoreMgr.GetQuickItem(iIndex).iItemLooks != 0)
// 								{
// 									g_pControl->PopupWindow( MSG_CTRL_CHECK_QUICK_WND,OPER_RECREATE,iIndex);
// 								}
// 							}
// 							else
// 								g_WooolStoreMgr.UseQuickItem(iIndex);
// 
// 							g_WooolStoreMgr.GetQuickItem(iIndex).dwTime = GetTickCount() + 200;
// 							return true;
// 						}
// 						break;
						case '~':
						case '`':
							{
								int iSelfX,iSelfY;
								static int dwLastPickTime = 0;
								SELF.GetXY(iSelfX,iSelfY);
								if(dwLastPickTime == 0 || (int)(GetTickCount() - dwLastPickTime) > 500)
								{
									if(m_pGameWnd)
										m_pGameWnd->GetDropedObject(iSelfX,iSelfY);
									dwLastPickTime = GetTickCount();
								}

								CSimpleGood* pGood = NULL;
								pGood = g_pGameData->FindSimpleGood(g_pControl->GetMouseOnID());
								if(pGood)
								{
									int iTileX = 0,iTileY = 0;
									pGood->GetXY(iTileX,iTileY);
									if(iTileX != iSelfX || iTileY != iSelfY)
									{
										if(g_AIAutoMgr.IsEnalbeWaiGua())
										{
											g_AutoPickMgr.SetPetGood(pGood->GetID(),iTileX,iTileY);
										}
										g_pGameControl->SEND_Pet_Object_Pickup(iTileX,iTileY);
									}
								}

								if(g_pControl->GetMouseOnID() == 0)
									SELF.SetReserveData(plyMagicLockID,0);
								else
								{
									CSimpleCharacter* pChar = g_pGameData->FindSimpleCharacter(g_pControl->GetMouseOnID());
									if(pChar)
									{
										SELF.SetReserveData(plyMagicLockID,pChar->GetID());
									}
									else
									{
										SELF.SetReserveData(plyMagicLockID,0);
									}
								}
								return true;
							}
							break;

					default:
						break;

					}
				}

			}
			break;
		}
	case MSG_INPUT_RIGHTBT_DOWN:
	case MSG_INPUT_RIGHTBT_UP:
	case MSG_INPUT_LEFTBT_DOWN:
	case MSG_INPUT_LEFTBT_UP:
		{
			if(AutoCloseWndProc(dwMsg, dwData, pControl))
				return true;
			break;
		}
	case MSG_INPUT_LEFTBT_DCLICK:
		{
			if(AutoCloseWndProc(dwMsg, dwData, pControl))
				return true;

			if(m_pFocusNow && m_pFocusNow->IsNeedKeyInput())
			{
				//m_pFocusNow->SetText("");
			}
			break;
		}
	case MSG_INPUT_MOVE:
		{
			m_iMouseX = LOWORD( dwData );
			m_iMouseY = HIWORD( dwData );

			break;
		}
	case MSG_INPUT_MIDDLEBT_DOWN:
	case MSG_INPUT_MIDDLEBT_UP:
		{
			break;
		}
	}
	return false;

	TRY_END;
	return false;
}

bool CMainWnd::IsPanelFakeFocus()
{
	if(m_pTalkViewWnd)
	{
		CGlyphEdit*  pPanelEdit = m_pTalkViewWnd->GetEditCtrl();
		if(pPanelEdit && pPanelEdit->IsFakeFocus())
		{
			return true;
		}
	}

	return false;
}

bool CMainWnd::Msg_MessageBox(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch(dwMsg)
	{
	case MSG_CTRL_ENTER_GAME:
		{
			g_pGameControl->SEND_Game_EnterGame(); //进入游戏
			break;
		}
	case MSG_RECONNECT_SERVER:
		{
			g_Login.SetAutoLoginInType(0);

			if (g_Login.GetLoginInExpType() > 0)
			{
				if(g_pControl->GetGameState() == MSG_CTRL_GAMEWND)
				{
					g_pGameControl->SEND_Objects_Position();
				}
				PostMessage(g_hWnd,WM_DESTROY,0,0);
			}
			else
			{
				if(m_pLoginWnd)
				{
					if(g_pControl->GetGameState() == MSG_CTRL_GAMEWND)
					{
						g_pGameControl->SEND_Objects_Position();
					}

					g_pGameControl->SEND_Login_LoginInServer();
					//PostMessage(g_hWnd,WM_DESTROY,0,0);
				}
				else
				{
					Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM + 2);
				}
			}

			break;
		}
	case MSG_CTRL_BOOTH_ADD_CONFIRM:
		{
			if(SELF.GetBoothState())
			{
				g_BoothData.SetAddingState(true);
				g_pGameControl->SEND_Booth_Request_Stall();
			}
			else
			{
				g_BoothData.AddGood();
			}
			break;
		}
	case MSG_CTRL_SELL_GOOD_VIPWND:
		{
			GoodPrice_t& readyPrice = g_BoothData.GetReadyPrice();
			g_pGameControl->SEND_Player_Vip_Option(3,readyPrice.GetID(),readyPrice.GetMoney(),readyPrice.GetYuanBao());
			break;
		}
	case MSG_CTRL_BOOTH_ADD_CANCEL:
		{
			g_BoothData.ClearReadyGood(); 
			break;
		}
	case MSG_CTRL_LEASEBOOTH_ADD_CONFIRM:
		{
			g_OtherBoothData.SetAddingState(true);
			g_pGameControl->Send_LeaseBooth_Start(g_OtherBoothData.GetOtherPlayerID(),true);
			break;
		}
	case MSG_CTRL_BOOTH_FIRSTTIME_CONFIRM:
		{
			g_OtherBoothData.AddGood();
			break;
		}
	case MSG_CTRL_LEASEBOOTH_LEASE_CONFIRM:
		{
			g_pGameControl->Send_LeaseBooth_Request(dwData);
			break;
		}
	case MSG_CTRL_USE_QIANKUNSUO:// 包裹使用乾坤锁
	case MSG_CTRL_USE_QIANKUNSUO_CHAREQ:// 装备栏使用乾坤锁
		g_pGameControl->SEND_Use_Object(dwData);
		break;
	case MSG_CTRL_PASSPORT:
		{
			if(m_pLoginWnd)
			{
				strcpy(g_cBuf,g_MsgBoxMgr.GetInstance()->GetInputString());
				m_pLoginWnd->Msg(MSG_CTRL_SEND_PASSWORD,0);
			}
		}
		break;
	case MSG_CTRL_DROPMONEY://丢钱
		{
			if (!g_MsgBoxMgr.GetInstance())
				break;
			string str;
			str = g_MsgBoxMgr.GetInstance()->GetInputString();
			if(strlen(str.c_str()) <= 10)
			{
				int iDropMoney = atol(str.c_str());
				if(iDropMoney > 0)
				{
					if(iDropMoney > SELF.GetGold())
					{
						g_TalkMgr.PushSysTalk("你没有这么多钱!");
						g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
						return true;
					}
					else
						g_pGameControl->SEND_Drop_Gold(iDropMoney);		//丢钱
					g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
				}
			}
			break;
		}
	case MSG_CTRL_TRADEGOLDBOX:		//交易的时候放钱
		{
			if (!g_MsgBoxMgr.GetInstance())
				break;
			string str;
			str = g_MsgBoxMgr.GetInstance()->GetInputString();
			if(strlen(str.c_str()) <= 10)
			{
				int iDropMoney = atol(str.c_str());
				if(iDropMoney > 0)
				{
					if(iDropMoney > SELF.GetGold())
					{
						g_TalkMgr.PushSysTalk("你没有这么多钱!");
						g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
						return true;
					}
					else if(iDropMoney> g_TradeData.GetSelfGolds())
						g_pGameControl->SEND_Trade_Money_Tackin(iDropMoney);		//丢钱
					else
					{
						//m_pMessagBox->ShowText("您放的金币只能比原来多，不能减少，请重新输入：");	
						//m_pMessagBox->SetEditText("");
						g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
						return true;
					}
					g_pGameData->SetDropMoneyFrom(EDMYF_NONE);
				}
			}
			break;
		}
	case MSG_CTRL_TRADEYUANBAOBOX:		//交易的时候放元宝
		{
			if (!g_MsgBoxMgr.GetInstance())
				break;
			string str;
			str = g_MsgBoxMgr.GetInstance()->GetInputString();
			if(strlen(str.c_str()) <= 10)
			{
				int iDropYuanBao = atol(str.c_str());
				if(iDropYuanBao > 0)
				{
					if(iDropYuanBao > SELF.GetYuanBao())
					{
						g_TalkMgr.PushSysTalk("你没有这么多元宝!");
						g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
						return true;
					}
					else if(iDropYuanBao> g_TradeData.GetSelfYuanBao())
						g_pGameControl->SEND_Trade_Money_Tackin(iDropYuanBao,true);		//丢元宝
					else
					{
						//m_pMessagBox->ShowText("您放的元宝只能比原来多，不能减少，请重新输入：");	
						//m_pMessagBox->SetEditText("");
						g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
						return true;
					}
					g_pGameData->SetDropYuanBaoFrom(EDMYF_NONE);
				}
			}
			break;
		}
	case MSG_CTRL_CHANGSHENGXUANBIN_WND://确认使用常胜玄兵等道具
		g_pGameControl->SEND_Message_ChangshengXuanbin(dwData,(WORD)pControl,1);
		break;
	case MSG_CTRL_ABANDON_TASK://放弃任务
		g_pGameControl->SEND_Player_Abandon_Task(g_TaskData.GetAbandonTaskID());
		break;
	case MSG_CTRL_RELEASE_EXP://要从宠物身上释放多少经验
		{
			DWORD dwExp = atol(g_MsgBoxMgr.GetInstance()->GetInputString());
			if(dwExp > 0)
			{								
				map<DWORD,PET_SENDOUT>& maplist = g_PetData.GetSendOutPetList();
				map<DWORD,PET_SENDOUT>::iterator it;
				it = maplist.find(dwData);
				if(it != maplist.end())
				{
					PET_SENDOUT &SendOutPet = it->second;

					if(dwExp > SendOutPet.dwSorbExp)
						g_TalkMgr.PushSysTalk("你的灵兽身上没有这么多经验!");
					else 
						g_pGameControl->SEND_Pet_Adopt_Request(5,dwExp,SendOutPet.dwPetItemID);//释放经验
				}
			}
		}
		break;
	case MSG_CTRL_START_USE_YSTPY:	// 开启御兽开袍／衣 穿上并骑上豹子
		g_AIMgr.PlayerStartUseCloth(dwData);
		break;
	case MSG_CTRL_LEAVE_JIPINGE:	//玩家离开极品阁
		if(dwData == 0)
		{
			g_pGameControl->SEND_ZHENBAO_INFO(0);
		}
		else if (dwData == 1)
		{
			g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);
		}		
		else if (dwData == 100)
		{
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@main");
		}
		else if (dwData == 101)
		{
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@OpenZBGrid");
		}
		else if (dwData == 102)
		{
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@shengongfang");
		}
		else if (dwData == 103)
		{
			g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@RecTask");
		}
		else if (dwData == 104)
		{
			g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM + 104);
		}
		else if (dwData == 105)
		{
			g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM + 105);
		}	
		break;
	case MSG_CTRL_ENTER_JIPINGE:	//玩家进入极品阁
		if(dwData >= 10000)
		{
			g_pGameControl->SEND_ZHENBAO_INFO(2,(BYTE)(dwData - 10000));
			g_NPC.GetZhenBaoState().bInViewMode = true;
			break;
		}		

		if(SELF.PackageGood().FindGoodByName("灵皓石") != -1)
		{
			g_pGameControl->SEND_ZHENBAO_INFO(2,(BYTE)dwData);
		}
		else
		{
			g_WooolStoreMgr.SetQuickBuyItem("灵皓石");
			CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

			if(QuickBuyData.pItem)
			{
				char cTemp[128]={0};
				sprintf(cTemp,"你包裹里没有灵皓石，是否需要购买？单价%d元宝",QuickBuyData.pItem->iPrice);
				QuickBuyData.iUseAfterBuyType = 7;
				QuickBuyData.strMsg = cTemp;
				QuickBuyData.iType = 1;
				QuickBuyData.iObjPos = dwData;
				if(g_MsgBoxMgr.GetInstance())
				{
					(g_MsgBoxMgr.GetInstance())->SetMode(false);
				}
				g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);						
			}
			else
			{
				g_TalkMgr.PushSysTalk("你包裹里没有灵皓石，请到商城购买。");
			}
		}
		break;
	case MSG_CTRL_LEAVE_GUILD:
		g_pGameControl->SEND_Message_Send("@退出门派",strlen("@退出门派"));
		break;
	case MSG_CTRL_DISMISS_PHYLE:
		g_pGameControl->SEND_Phyle_Operate(NULL,3);
		break;
// 	case MSG_CTRL_TRUSTEESHIP_RESPONSE:
// 		{			
// 			WORD tmp = LOWORD(dwData);
// 			if (HIWORD(dwData) == 1)
// 			{//队长				
// 				g_pGameControl->Send_TrusteeshipRespond((HIBYTE(tmp)==1), LOBYTE(tmp), NULL);
// 			}
// 			else
// 			{
// 				char* pName = (char*)pControl;
// 				g_pGameControl->Send_TrusteeshipRespond((HIBYTE(tmp)==1), LOBYTE(tmp), pName);
// 				if(pName) delete []pName;
// 			}
// 		}
// 		break;
	//case MSG_CTRL_TRUSTEESHIP_CONFIRM:
	//	g_pGameControl->Send_TrusteeshipSwitch((BYTE)dwData,true);
	//	break;
	case MSG_CTRL_LEAVE_PHYLE:
		g_pGameControl->SEND_Phyle_Operate(NULL,2);
		break;
	case MSG_CTRL_OPEN_BOX:
		if (SELF.GetBindYuanBao() < 10)
		{
			g_MsgBoxMgr.PopSimpleAlert("您的绑定元宝不足,无法租赁");
		}
		else
		{
			g_pGameControl->SEND_BOX_INFO(4,0,0);
		}
		break;
	//case MSG_CTRL_ENLARGE_BOX:
	//	g_pGameControl->SEND_BOX_INFO(2,0);
	//	break;	
	//case MSG_CTRL_FILL_BOX:
	//	{
	//		string str = g_MsgBoxMgr.GetInstance()->GetInputString();
	//		dwData = atol(str.c_str());
	//		if(dwData != 0)
	//		{
	//			g_pGameControl->SEND_BOX_INFO(4,dwData);
	//		}
	//		break;
	//	}
	case MSG_CTRL_UPLEVEL_ARM:  //更换武器附加技能类型
		{
			g_pControl->PopupWindow(MSG_CTRL_CHARWND,OPER_CREATE);
			CGood *pDest = NULL;
			WORD wType = HIWORD(dwData);
			if (wType == 1)//武器
			{
				g_pControl->MsgToWnd(MSG_CTRL_CHARWND,MSG_CTRL_EQUIPMENT_EFFECT,1);
				pDest = &(SELF.GetEquipGood(ITEM_POS_WEAPON));
			}
			else if (wType == 2)//盾牌
			{
				g_pControl->MsgToWnd(MSG_CTRL_CHARWND,MSG_CTRL_EQUIPMENT_EFFECT,3);
				pDest = &(SELF.GetEquipGood(ITEM_POS_SHIELD));
			}

			CGood& Good2Use = SELF.GetPackageGood((int)LOWORD(dwData));
			CGoodGrid::GetDropGoodFrom().DropGood.SetPos(0);
			if (pDest && Good2Use.GetID() != 0)
			{
				g_pGameControl->SEND_AddObjToObj(Good2Use,*pDest);
				g_pAudio->Play(EAT_OTHER,200,g_pAudio->GetRand()++);
			}		
		}
		break;
	case MSG_CTRL_USE_MAGIC_BOOK://学习技能书
	case MSG_CTRL_USE_RED_PACKAGE://打开红包确认
	case MSG_CTRL_USE_AUTHENTICATE://确认使用物品鉴定
		{
			if(dwData != -1 && dwData < MAX_PACKAGE_ELEMENT)
				g_pGameControl->SEND_Use_Object(dwData);
		}
		break;
	case MSG_CTRL_CLEARALL_LEAVEWORD:
		{
			g_pControl->MsgToWnd(MSG_CTRL_RECEIVE_MSG_WND,MSG_CTRL_RECEIVE_MSG_WND,1);
		}
		break;
	case MSG_CTRL_CREATE_TEAM_VOICE_ROOM:
		{
#ifdef _CHAT
			if(g_VoiceAdapter.IsInRoom())
			{
				g_VoiceAdapter.SetCreateTeamRoomAfterLeave(true);
				g_VoiceAdapter.LeaveRoom();
			}
			else
				g_VoiceAdapter.CreateTeamVoiceRoom(); //创建组队房间
#endif
		}
		break;
	case MSG_CTRL_EXIT_MAZE_OK:
		{
			g_pGameControl->SEND_ExitMazeMap();
		}
		break;
	case MSG_CTRL_INCORPORATE_MEDAL_OK:
	case MSG_CTRL_INCORPORATE_MEDAL_CANCEL:
		{
			g_pControl->MsgToWnd("PackageWnd",dwMsg,dwData,pControl);
		}
		break;
	case MSG_CTRL_SPIRT_UPGRATE_WEAPON:
		g_pControl->MsgToWnd("PackageWnd",dwMsg,dwData,NULL);
		break;
	case MSG_CTRL_ADDLINGLI:
		g_pControl->MsgToWnd("PackageWnd",dwMsg,dwData,NULL);
		break;
	case MSG_CTRL_BUY_YUANBAO:
	case MSG_CTRL_SELL_YUANBAO:
		g_pControl->MsgToWnd("YuanBaoWnd",dwMsg,dwData,NULL);
		break;
	case MSG_CTRL_OPENDOOR:
		g_pGameControl->SEND_INSTANCEZONE_POPUP(1,(byte)dwData);
		break;
	case MSG_CTRL_USE_TREASURE_MAP:
		g_pGameControl->SEND_Use_Object(dwData);
		g_pControl->PopupWindow(MSG_CTRL_TREASUREMAP_WND,OPER_CLOSE);
		break;
	case MSG_CTRL_QUICKNPC_SELL_OK:
		{
			CGood &temp = g_NPC.GetGood();
			if(temp.GetID() != 0)
			{
				g_pGameControl->SEND_Exchange_Sale_Ok(g_NPC.GetID(),temp.GetID(),temp.GetName());
			}
		}
		break;
	case MSG_CTRL_QUICKNPC_SELL_CANCEL:
		{
			g_NPC.BackToPackage();
			g_NPC.SetSaleGold(0);
		}
		break;
	case MSG_CTRL_QUICKNPC_REPAIR_OK:
		{
			CGood &temp = g_NPC.GetGood();
			if(temp.GetID() != 0)
			{
				g_NPC.SetExchangeFromQuickNpcWnd(true);
				g_pGameControl->SEND_Exchange_Repair_Ok(g_NPC.GetID(),temp.GetID(),temp.GetName(),0,(BYTE)dwData);
			}
		}
		break;
	case MSG_CTRL_QUICKNPC_REPAIR_CANCEL:
		{
			g_NPC.GetGood().SetID(0);
		}
		break;
	case MSG_CTRL_INPUT_NEW_PASWORD:
		{
			string str = g_MsgBoxMgr.GetInstance()->GetInputString();;
			if(str.length() <= 16)
			{
				if(g_pRoomManager->GetMyRoom())
					g_pRoomManager->GetMyRoom()->ChangePwd(str.c_str());
			}
			else
				g_TalkMgr.PushSysTalk("密码太长，请重新输入!");
		}
		break;
    case MSG_CTRL_REPAIR_DAMAGED_EQUIPMENT:
        {
            int iDura = 0;
            //int iPos = -1;

            for(int i = 0;i < MAX_PACKAGE_ELEMENT;i++)
            {
                CGood& good = SELF.GetPackageGood(i);
                if(good.GetID() != 0 && good.GetStdMode() == 37 && good.GetShape() == 221)
                {
                    iDura += good.GetDura();
                }
			}

            if(iDura >= g_NPC.GetSaleGold())
            {
                //发送
                g_pGameControl->SEND_Exchange_Repair_Ok(0,g_NPC.GetDamageGood().GetID(),g_NPC.GetDamageGood().GetName(),3);							
                g_NPC.GetDamageGood().SetID(0);                    
                g_NPC.SetSaleGold(0);
            }
            else
            {

                //弹出快捷消费
                g_WooolStoreMgr.SetQuickBuyItem("五色晶石");
                CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

                if(QuickBuyData.pItem)
                {
                    char cTemp[128]={0};
                    sprintf(cTemp,"你包裹里五色晶石持久不够，是否需要购买？单价%d元宝",QuickBuyData.pItem->iPrice);
                    QuickBuyData.iUseAfterBuyType = 5;
                    QuickBuyData.strMsg = cTemp;
                    QuickBuyData.iType = 1;
					if(g_MsgBoxMgr.GetInstance())
					{
						(g_MsgBoxMgr.GetInstance())->SetMode(false);
					}
					g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);						
                }
                else
                {
                    g_TalkMgr.PushSysTalk("你包裹中没有五色晶石或者其持久不够，请到商城购买后再进行修复。",0xFDFF);
                }
            }
            
        }
        break;
	case MSG_CTRL_GET_USBMB_CODE:
		{
			//g_pGameControl->SEND_UsbMb((BYTE)dwData,1);
		}
		break;
	case MSG_CTRL_RELOGIN_GAME:
		{
			SELF.clear();
			g_OtherData.SetAutoEnter(true);
			//清除
			g_pGameControl->SEND_Out_Game(false);
			Sleep(1000);

			//if(g_Login.GetLoginGateType() != 1)
			//	g_pNet->Reset();

			//g_pGameControl->SEND_Selchar_CharServer();
		}
		break;
 	case MSG_CTRL_LOGIN_KEZ_SERVER://登录跨服战服务器
// 	case MSG_CTRL_LOGIN_XL_SERVER://登录仙灵大陆
 		{
 			string strTemp = g_Login.GetKfzLoginString();//  ptAccount/AreaNo/AreaName/LoginGateIp/LoginGatePort/GroupName/GroupNickName
 
 			//先备份一下game.ini文件,跨服战打完后可以直接返回原来的游戏
 			string strSrcPath = GetGameDataDir();
 			strSrcPath += "\\game.ini";
 
 			DataStreamInterface* stream = g_pStreamMgr->OpenDataFile(strSrcPath.c_str(),false,true,true,EP_MOST_HIGH);
 			if(stream)
 			{
 				string buf;
 				while(!stream->eof())
 				{
 					buf += stream->getLine();
 					buf += "\r\n";
 				}        
 				g_Login.SetGameIniContent(buf);
 				SAFE_DELETE(stream);
 			}
 
 
 			int iPos1 = strTemp.find("/",0);
 			g_Login.GetAutoUser() = strTemp.substr(0,iPos1);
 			//string strPwd = g_Login.GetPassWord();
 			//char strPwdTemp[512]={0};
 			//for(int i = 0;i < strPwd.length();i++)
 			//{
 			//	strPwdTemp[i] = strPwd.c_str()[i] ^ g_byPwdEncode;
 			//}
 			//strPwd = g_pNet->NewDecode(strPwdTemp,strlen(strPwdTemp));
 			//g_Login.GetAutoPassword() = strPwd;
 			g_Login.SetLoginID(g_Login.GetAutoUser().c_str());
 
 			int iPos2 = strTemp.find("/",iPos1 + 1);
 			string strAreaNo = strTemp.substr(iPos1+1,iPos2-iPos1-1);
 			g_pStreamMgr->SetGameStr("Area",strAreaNo.c_str());
 
 			iPos1 = strTemp.find("/",iPos2 + 1);
 			string strAreaName = strTemp.substr(iPos2+1,iPos1-iPos2-1);
 			g_pStreamMgr->SetGameStr("AreaName",strAreaName.c_str());
 
 			iPos2 = strTemp.find("/",iPos1 + 1);
 			string strLoginGateIp = strTemp.substr(iPos1+1,iPos2-iPos1-1);
 			g_pStreamMgr->SetGameStr("ServerIP",strLoginGateIp.c_str());
 
 			iPos1 = strTemp.find("/",iPos2 + 1);
 			string strLoginGatePort = strTemp.substr(iPos2+1,iPos1-iPos2-1);
 			g_pStreamMgr->SetGameStr("ServerPort",strLoginGatePort.c_str());
 
 			g_pStreamMgr->SetGameStr("GroupNum","1");
 
 			iPos2 = strTemp.find("/",iPos1 + 1);
 			string strGroupName = strTemp.substr(iPos1+1,iPos2-iPos1-1);
 			g_pStreamMgr->SetGameStr("Group0",strGroupName.c_str());
 
 			string strGroupNickName = strTemp.substr(iPos2 + 1,strTemp.length() - iPos2 - 1);
 			g_pStreamMgr->SetGameStr("GroupNick0",strGroupNickName.c_str());
 
 			//if(dwMsg == MSG_CTRL_LOGIN_KEZ_SERVER)
 			//{
 			//	g_Login.SetAutoLoginInType(1);
 			//}
 			//else if(dwMsg == MSG_CTRL_LOGIN_XL_SERVER)
 			//{
 			//	g_Login.SetAutoLoginInType(3);
 			//}
 
 			g_Login.SetGroupNo_Bak(g_Login.GetGroupNo());
 			g_Login.SetAreaNo_Bak(g_Login.GetAreaNo());
 			g_Login.SetRoleName_Bak(SELF.GetName());
 			g_Login.SetGroupName_Bak(g_Login.GetGroupName());
 			g_Login.SetGroupNo(0);
 			g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM);
 		}
 		break; 
 	case MSG_CTRL_BACK_FROM_KFZ:
// 	case MSG_CTRL_BACK_FROM_XL:
 		{
 			const string & gameIniContent = g_Login.GetGameIniContent();
 			if(gameIniContent.length() > 0)
 			{
 				string strDestPath = GetGameDataDir();
 				strDestPath += "\\game.ini";
 
 				FILE * fp = fopen(strDestPath.c_str(),"wb");
 				if(!fp)
 				{
 					g_MsgBoxMgr.PopSimpleAlert("返回失败，请关闭客户端后重新登录。");
 					return true;
 				}
 
 				fwrite(gameIniContent.c_str(),gameIniContent.length(),1,fp);
 				fclose(fp);
 				g_Login.SetGameIniContent("");
 
 				//if(dwMsg == MSG_CTRL_BACK_FROM_KFZ)
 				//{
 				//	g_Login.SetAutoLoginInType(2);
 				//}
 				//else if(dwMsg == MSG_CTRL_BACK_FROM_XL)
 				//{
 				//	g_Login.SetAutoLoginInType(4);
 				//}
 
 				g_Login.GetAutoUser() = g_Login.GetLoginID();
 				//string strPwd = g_Login.GetPassWord();
 				//char strTemp[512]={0};
 				//for(int i = 0;i < strPwd.length();i++)
 				//{
 				//	strTemp[i] = strPwd.c_str()[i] ^ g_byPwdEncode;
 				//}
 				//strPwd = g_pNet->NewDecode(strTemp,strlen(strTemp));
 				//g_Login.GetAutoPassword() = strPwd;
 
 				g_Login.SetGroupNo(g_Login.GetGroupNo_Bak());
 				g_OtherData.GetTimeOut().clear();
 
 				g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM);
 			}
 			else
 			{
 				g_MsgBoxMgr.PopSimpleAlert("返回失败,请关闭客户端后重新登录.");
 			}
 		}
 		break;
// 	case MSG_CTRL_ENLARGE_PACKAGE:
// 		{
// 			g_pGameControl->SEND_EnlargePackage(1);
// 		}
// 		break;
	case MSG_CTRL_ENLARGE_MONEYBAG:
		{
			g_pGameControl->SEND_EnlargePackage(2);
		}
		break;
	case MSG_CTRL_BOOTH:
		{
			g_pGameControl->SEND_EnlargePackage(3);
		}
		break;
	case MSG_CTRL_CONFIRM_KFZ_MEMBER_STATE:
		{
			g_pGameControl->SEND_Kfz_MemberState(2);
		}
		break;
	case MSG_CTRL_MORE_STORAGE_GRID:
		{
			g_pGameControl->SEND_EnlargePackage(4);
		}
		break;
	case MSG_CTRL_GUILD_MOBILIZE:
		{
			g_pControl->MsgToWnd("RelationWnd",MSG_CTRL_GUILD_MOBILIZE_CONFIRM);
		}
		break;
	case MSG_CTRL_TAN_BAO:
		{
			g_pGameControl->Send_Pet_Explore(dwData,0);
		}
		break;
	case MSG_CTRL_ZHAO_HUAN:
		{
			g_pGameControl->Send_Pet_Explore(dwData,1);
		}
		break;
	case MSG_CTRL_JOIN_CONFIRM:
		g_pGameControl->SEND_Guild_Stele_Register();
		break;
	case MSG_CTRL_ACCESSFGHVALUE_ANNWAY:
		g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@assessChallenge");
		break;
	case MSG_CTRL_QUIT_TO_LOGINGWND:
		g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM);
		break;
	case MSG_CTRL_CLOSE_CLIENT:
		PostMessage(g_hWnd,WM_DESTROY,0,0);
		break;
	case MSG_CTRL_START_HUANHUA:
		g_pControl->Msg(MSG_CTRL_HUANHUA_WND,OPER_CUSTOM + 1);
		break;
	case MSG_CTRL_DEL_SKILL:
		g_pControl->Msg(MSG_CTRL_DEL_SKILL_WND,OPER_CUSTOM);
		break;
	case MSG_CTRL_DISASSEMBLE_WEAPON:
		g_pControl->Msg(MSG_CTRL_EXTRACTUPGRADE_YUANSHI,OPER_CUSTOM + 2);
		break;
	case MSG_UPGRADE_YUANSHI:
		g_pControl->Msg(MSG_CTRL_GEMCONDENSE_WND,OPER_CUSTOM + 3);
		break;
	/*case MSG_RELIVE_DEMON:
		g_pGameControl->Send_Relive_Demon();
		break;*/
	case MSG_CTRL_LUCKQYZ_CLOSECANCEL:
		break;
	case MSG_CTRL_LUCKQYZ_CLOSEOK:
		g_pControl->PopupWindow(MSG_CTRL_LUCKQYZ_WND,OPER_CLOSE);
		break;
	case MSG_CTRL_LUCJQYZ_CONTINUE:
		g_pControl->PopupWindow(MSG_CTRL_LUCKQYZ_WND,OPER_CREATE);
		break;
	case MSG_CTRL_JIEJINGTI:								//结晶体
		g_pGameControl->SEND_Use_Object(dwData);
		break;
	case MSG_CTRL_REJECTOBJECT:
		{
			CGood *pGood = SELF.PackageGood().FindGood(dwData);
			if (pGood)
			{
				g_pGameControl->SEND_Package_Object_Reject(*pGood);
			}
		}
		
		break;
	case MSG_CTRL_BREAKDOWN_EQUIP:
		g_pGameControl->Send_EquipSoul(3, dwData);
		break;
	case MSG_CTRL_CHAIFEN_NUM:
		{
			string str = g_MsgBoxMgr.GetInstance()->GetInputString();
			if(strlen(str.c_str()) <= 10)
			{
				int iSplitNum = atol(str.c_str());
				g_pGameControl->SEND_Stone_Split_Request(dwData,iSplitNum,0);
			}
		}
		break;
	case MSG_CTRL_WENPEI_CHAIFEN:
		g_pGameControl->SEND_WenPei_Decompound(dwData);
		break;
	case MSG_CTRL_USESUBWENPEI:
		{
			g_pGameControl->SEND_Use_Object(dwData);//使用纹佩配件
			g_pControl->PopupWindow(MSG_CTRL_WENPEI_WND,OPER_CREATE);
			g_pControl->Msg(MSG_CTRL_WENPEI_WND,10,pControl);
		}
		break;
	case MSG_CTRL_RENZHU_FABAO:
		{
			g_pGameControl->SEND_Request_FaBaoRenZhu(dwData);//法宝认主
		}
		break;
	case MSG_CTRL_AUTOKILL:
		g_AutoKillMonsterMgr.SetEnabled(!g_pGameData->IsAutoKillMonster());
		g_pControl->Msg(MSG_CTRL_AUTOKILLBTN,0);
		break;
	case MSG_CTRL_CHAIFEN_FABAO:
		g_pGameControl->Send_FabaoChaifen(dwData);
		break;
	case MSG_CTRL_BUY_VIPTRADE:
		g_pGameControl->SEND_Player_Vip_Option(0,dwData);
		break;
	case MSG_CTRL_FIREARTIFICE:
		g_pControl->MsgToWnd("FireArtificeWnd",MSG_CTRL_FIREARTIFICE_WND,1002);
		break;
	case MSG_CTRL_LOCK_EQUIK:
		{
			MsgBoxInfo &info = g_MsgBoxMgr.GetInstance()->GetInfo();
			CGood *pSrc = SELF.PackageGood().FindGood(info.m_stcOkMsg.dwData);
			CGood *pDest = SELF.PackageGood().FindGood(info.m_stcCancelMsg.dwData);

			if (!pSrc || !pDest)
			{
				return true;
			}

			GoodAddEffect &goodEffect = pDest->GetGoodAddEffect();
			goodEffect.dwStartID = MAKELONG(1070,PACKAGE_stateitem);
			goodEffect.iFrams = 15;
			goodEffect.RM = RM_ADD2;
			goodEffect.iOffX = -63;
			goodEffect.iOffY = -93;
			goodEffect.iSoundID = 7;

			g_pGameControl->SEND_AddObjToObj(*pSrc,*pDest);

			g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
		}
		break;
	case MSG_CTRL_YUSHOULEVELUP:
		{
			if (dwData == 0)
			{
				g_pGameControl->Send_HorseManShip_Watch_Req(1,SELF.GetName(),0);
			}
			else if (dwData == 1)
			{
				g_pGameControl->Send_HorseManShip_Watch_Req(3,g_NPC.GetSelFriendName().c_str(),0);
			}
			else if (dwData == 2)
			{
				g_pGameControl->Send_HorseManShip_Watch_Req(2,g_NPC.GetSelFriendName().c_str(),0);
			}
		}
		break;
	default:
		break;
	}
	return true;
}


bool CMainWnd::Msg_MainOther(DWORD dwMsg, DWORD dwData, CControl * pControl)
{
	return true;
}

bool CMainWnd::Msg_GameWnds(DWORD dwMsg, DWORD dwData, CControl * pControl)
{
	MsgWndMap::iterator itr = m_MMsgWnd.find(dwMsg);
	if(itr != m_MMsgWnd.end())//////////////////////////////////////////注册的消息直接给窗口处理，不再传递
	{
		CCtrlWindow* pWnd = FindWindowByName(itr->second.c_str());
		if(pWnd)
		{
			if(pWnd->Msg(dwMsg,dwData,pControl))
				return true;
		}

		if(dwData == OPER_CLOSE || dwData == OPER_CREATE || dwData == OPER_UPDATE || dwData == OPER_RECREATE)
			return PopupWindow(dwMsg,dwData,DWORD(pControl));
	}

	return CCtrlMainWin::Msg(dwMsg,dwData,pControl);
}

bool CMainWnd::DoAccelKey(WORD wKey)
{
	if(m_pSelectCharWnd && g_pInput->IsAlt() && wKey == 'X')
	{
		g_MsgBoxMgr.ClearAllExistMsgBox();
		g_MsgBoxMgr.PopSimpleComfirm("是否重新登录?",MSG_CTRL_RELOGIN_GAME,0);						
		return true;
	}

	if(!m_pGameWnd || g_OtherData.GetEnterGameTime() == 0)/////////////////////还没有进入游戏不处理快捷键
	{
		return false;
	}

	if(g_AIAutoMgr.DoAccelKey(wKey)) //处理辅助工具相关的Key
		return true;

	if(g_pInput->IsAlt())
	{
		switch(wKey)
		{
		case 'X':
			{
				if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsQuickOutGame())
				{
					g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM + 1);
				}
				else
				{
					if(!g_pControl->FindWindowByName("ReadQuitWnd"))
					{
						g_MsgBoxMgr.PopSimpleComfirm("您想退出游戏回到选择角色界面吗?",MSG_CTRL_QUIT_WND,OPER_CUSTOM+1);						
					}
				}
			}

			return true;
		case 'Q':
			//g_AutoKillMonsterMgr.SetEnabled(!g_pGameData->IsAutoKillMonster());

			if(g_pControl->FindWindowByName("ReadQuitWnd"))
				return true;

			return true;
		case 'Z':
			{
				g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(7,7));
				return true;
			}

			g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CREATE);
			return true;
		}
	}
	else if(g_pInput->IsCtrl())
	{
		switch(wKey)
		{
		case 'Q':
			if(m_pGameWnd)
			{
				if(SELF.IsLearnMagic(MAGICID_PET_CALL))
					g_pGameMgr->AttackUseMagic(MAGICID_PET_CALL);
			}
			return true;
		case 'A':
			g_pGameControl->SEND_Message_Rest("@Rest",5);
			return true;
		case 'H':
			g_pGameControl->SEND_Guild_Ally("@AttackMode");
			return true;
		case 'R':
			{
				if(!SELF.IsOnHorse())
				{
					PET_SENDOUT_MAP& petMap = g_PetData.GetSendOutPetList();
					PET_SENDOUT_MAP::iterator itr = petMap.begin();
					for(;itr != petMap.end();itr++)
					{
						if((itr->second).szReserved[0] == 1)
						{
							break;
						}
					}
					if(itr != petMap.end())
					{
						g_AIMgr.PlayerRidePet(itr);	
					}	
					else	//判断有没有马
					{
						if(g_PetData.GetMyHorse().bStatus == 1)
						{
							g_pGameControl->SEND_Player_Ride_Horse();
						}
					}
				}
				else
				{
					g_pGameControl->SEND_Player_Ride_Horse();
				}
				
			}
			return true;
		case 'P':
			g_pControl->PopupWindow(MSG_CTRL_PET_INFO_WND,OPER_UPDATE);
			return true;
		case 'D':
			g_pGameControl->SEND_Message_Rest("@HorseRest",10);
			return true;
		case 'W':
			g_pControl->PopupWindow(MSG_CTRL_PET_PACKAGE_WND,OPER_UPDATE);
			return true;
		case 'Z':

			//更换新协议 如果玩家没有骑在豹子上,计算出豹子的方向
			//g_pGameControl->SEND_Message_Rest("@bellow",7);
			//每隔3秒才能吼叫一次
			{
				static DWORD dwRequestTime = 0;
				if(GetTickCount() < dwRequestTime + 5000)
					return true;
				dwRequestTime = GetTickCount();

				int iDir = -1;

				if(m_pGameWnd)
				{
					int x,y;
					g_pGameMgr->GetMouseTile(x,y);
					g_pGameControl->SEND_Leopard_Action(x,y);
					int mx,my;
					g_pInput->GetMousePos(mx,my);
					iDir = g_pGameMgr->GetPlayerDir(mx,my);

				}

				if(SELF.IsOnLepoard())
				{
					/*if( iDir != -1 && iDir != (int)(SELF.GetDir()) )
						SELF.SetDir((int)(iDir));*/

					SELF.PushSNextAction(ACTION_LSALUTE);
				}
				return true;
			}
		case 'S':
			{
				if((SELF.IsOnLepoard() || !SELF.IsOnHorse()) && !SELF.IsBianShen())	//只有在豹子上或者步行状态才能作揖
				{
					SELF.GetCNextAction().wAction = ACTION_SALUTE;
				}
				return true;
			}		
		case VK_TAB:
			if(strnicmp(g_pGameMap->GetMapName(),"maze",4) != 0)
				g_pControl->PopupWindow(MSG_CTRL_MINMAP_WND,OPER_UPDATE);
			else g_pControl->PopupWindow(MSG_CTRL_MAZE_WND,OPER_UPDATE);
			return true;
		case 'J':
			g_pControl->PopupWindow(MSG_CTRL_WOOOL_STORY_WND,OPER_CREATE);
			return true;
		case VK_HOME:
			g_pControl->PopupWindow(MSG_CTRL_ASSCONFIG_WND,OPER_CREATE);
			return true;
		case 'T':
			g_pControl->PopupWindow(MSG_CTRL_TASK_WND,OPER_UPDATE,0);
			return true;
		case 'I':
			if(g_pControl->FindWindowByName("RelationWnd"))
			{
				g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_CLOSE);
			}
			else
			{
				g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@main");
			}

			return true;
		case 'L':
			{
				if (g_OtherData.GetSanWeiZhenHuoInfo().byFireLevel > 0)
				{
					g_pControl->PopupWindow(MSG_CTRL_SANWEIFIRE_WND,OPER_UPDATE);
				}
				else
				{
					g_MsgBoxMgr.PopSimpleAlert("当人物修炼到42级并且完成相应任务之后才能开启三昧真火。");
				}
			}
			return true;
		case 'N'://察看活动日志
			g_pControl->PopupWindow(MSG_CTRL_ACTIVITY_LOG_WND,OPER_UPDATE);
			//g_pGameControl->SEND_Exchange_Goin(-5);
			return true;
		case 'K'://察看附近的NPC和玩家(CTRL+K)
			g_pControl->PopupWindow(MSG_CTRL_PERSONS_GUIDE_WND,OPER_UPDATE);
			return true;
		case 'O':
			//if(g_pSDOAInterface)
				g_pControl->PopupWindow(MSG_CTRL_RECEIVE_MSG_WND,OPER_UPDATE);
			//else
			//	g_pControl->PopupWindow(MSG_CTRL_RADIO_WND,OPER_UPDATE);

			return true;		
		case VK_RIGHT:
			if (m_pTalkViewWnd)
			{
				m_pTalkViewWnd->ChangeTalkAutoWheel();
			}
			return true;
		//case 'E':
		//	g_pGameControl->SEND_Player_Beast_ChangeState();
		//	return true;
		}
		return false;
	}
	else if(g_pInput->IsShift())
	{
// 		if (g_TrusteeshipData.IsCaptain() && m_pFocusNow && !m_pFocusNow->IsNeedKeyInput() && !IsPanelFakeFocus())
// 		{
// 			switch (wKey)
// 			{
// 			case 'A'://强攻模式
// 				{
// 					g_TrusteeshipData.SetSelForceAttObj(true);
// 					g_pGameData->SetMouseType(MOUSE_SELFORCEATTOBJ);
// 					return true;
// 				}
// 			case 'S'://停止模式
// 				{
// 					for (int i=0; i<5; i++)
// 					{
// 						if(g_TrusteeshipData.GetTneupMember(i).bSelected)
// 							g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeStop);
// 					}
// 					return true;
// 					break;
// 				}
// 			case 'D'://攻击模式
// 				{
// 					for (int i=0; i<5; i++)
// 					{
// 						if(g_TrusteeshipData.GetTneupMember(i).bSelected)
// 							g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeFight);
// 					}
// 					return true;
// 				}
// 			case 'Z'://驻扎模式
// 				{
// 					g_TrusteeshipData.SetSelGarrsionXY(true);
// 					g_pGameData->SetMouseType(MOUSE_SELGARRSIONXY);
// 					return true;
// 				}
// 			case 'X'://守护模式
// 				{
// 					for (int i=0; i<5; i++)
// 					{
// 						if(g_TrusteeshipData.GetTneupMember(i).bSelected)
// 							g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeGuard);
// 					}
// 					return true;
// 				}
// 			case 'C'://跟随模式
// 				{
// 					for (int i=0; i<5; i++)
// 					{
// 						if(g_TrusteeshipData.GetTneupMember(i).bSelected)
// 							g_pGameControl->Send_TrusteeshipMode(g_TrusteeshipData.GetTneupMember(i).szName, eTrusteeshipModeFollow);
// 					}
// 					return true;
// 				}
// 		default:
// 			break;
// 			}
// 		}
	}
	else
	{

		if(wKey >= VK_F9 && wKey <= VK_F12)
		{
			g_pAudio->Play(EAT_OTHER,2,g_pAudio->GetRand()++);
		}

		switch(wKey)
		{
// 		case 'A':
// 		case 'S':
// 		case 'D':
// 		case 'F':
// 		case 'G':
// 			{
// 				if (g_TrusteeshipData.IsTrusteeship() && g_TrusteeshipData.IsCaptain() && m_pFocusNow && !m_pFocusNow->IsNeedKeyInput() && !IsPanelFakeFocus())
// 				{
// 					int sel = 0;
// 					if (wKey == 'S')
// 						sel = 1;
// 					else if (wKey == 'D')
// 						sel = 2;
// 					else if (wKey == 'F')
// 						sel = 3;
// 					else if (wKey == 'G')
// 						sel = 4;
// 
// 					return g_TrusteeshipData.MicroControlMember(sel);
// 				}
// 			}
// 			break;
		case VK_F9:
			g_pControl->PopupWindow(MSG_CTRL_PACKAGEWND,OPER_UPDATE);
			return true;
		case VK_F10:
			//微操或附身模式下不能按F10
// 			if (g_TrusteeshipData.GetMicroControlPos() < 0 && g_TrusteeshipData.GetCaptionSubstitutePos() < 0)
			{
				g_pControl->PopupWindow(MSG_CTRL_CHARWND,OPER_UPDATE);
			}
			return true;
		case VK_F11:
			g_pControl->PopupWindow(MSG_CTRL_SKILLWND,OPER_UPDATE);
			return true;
		case VK_F12:
			{
				//微操模式下按F12弹托管设置窗口
// 				if (g_TrusteeshipData.GetMicroControlPos() >= 0)
// 				{
// 					g_pControl->PopupWindow(MSG_CTRL_TRUSTEESHIPCONFIG_WND, OPER_UPDATE,g_TrusteeshipData.GetMicroControlPos());
// 				}
// 				else if (g_TrusteeshipData.GetCaptionSubstitutePos() >= 0)
// 				{
// 					g_pControl->PopupWindow(MSG_CTRL_TRUSTEESHIPCONFIG_WND, OPER_UPDATE,g_TrusteeshipData.GetCaptionSubstitutePos());
// 				}
// 				else
				{
					if (g_pControl->PopupWindow(MSG_CTRL_ASSCONFIG_WND,OPER_CLOSE) || g_pControl->PopupWindow(MSG_CTRL_SYSCONFIG_WND,OPER_CLOSE))
						return true;

					g_pControl->PopupWindow(MSG_CTRL_ASSCONFIG_WND,OPER_UPDATE);
				}
			}
			return true;
		}


	}

	return false;
}

bool CMainWnd::PopupArrowTip(DWORD dwWndMsg,int iTipID,int iX,int iY,int iAlignType,bool bShowNow,int iTipDir,int iTipType,DWORD dwData,DWORD dwLastTime,bool bNeedMsg)
{
	if (!g_AICfgMgr.IsShowNewHandPrompt())
	{
		return false;
	}
	
	switch (dwWndMsg)
	{
	case MSG_CTRL_TALKVIEW_WND:
		if (m_pTalkViewWnd)
		{
			return m_pTalkViewWnd->AddArrowTip(iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
		}
		break;
	case MSG_CTRL_GAMEWND:
		if (m_pGameWnd)
		{
			return m_pGameWnd->AddArrowTip(iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
		}
		break;
	default:
		return CCtrlMainWin::PopupArrowTip(dwWndMsg,iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
		break;
	}

	return false;
}

bool CMainWnd::PopupArrowTip(const char* szWndName,int iTipID,int iX,int iY,int iAlignType,bool bShowNow,int iTipDir,int iTipType,DWORD dwData,DWORD dwLastTime,bool bNeedMsg)
{
	if (!g_AICfgMgr.IsShowNewHandPrompt())
	{
		return false;
	}
	
	if (strcmp(szWndName,"TalkViewWnd") == 0)
	{
		if (m_pTalkViewWnd)
		{
			return m_pTalkViewWnd->AddArrowTip(iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
		}
	}
	else if (strcmp(szWndName,"GameWnd") == 0)
	{
		if (m_pGameWnd)
		{
			return m_pGameWnd->AddArrowTip(iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
		}
	}

	return CCtrlMainWin::PopupArrowTip(szWndName,iTipID,iX,iY,iAlignType,bShowNow,iTipDir,iTipType,dwData,dwLastTime,bNeedMsg);
}

bool CMainWnd::MsgToWnd(DWORD dwWndMsg,DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch (dwWndMsg)
	{
	case MSG_CTRL_LOGINWND:
		if (m_pLoginWnd)
		{
			return m_pLoginWnd->Msg(dwMsg,dwData,pControl);
		}
		break;
	case MSG_CTRL_SELECTCHARWND:
		if (m_pSelectCharWnd)
		{
			return m_pSelectCharWnd->Msg(dwMsg,dwData,pControl);
		}
		break;
	case MSG_CTRL_QUIT_WND:
		if (m_pQuitWnd)
		{
			return m_pQuitWnd->Msg(dwMsg,dwData,pControl);
		}
		break;
	case MSG_CTRL_TALKVIEW_WND:
		if (m_pTalkViewWnd)
		{
			return m_pTalkViewWnd->Msg(dwMsg,dwData,pControl);
		}
		break;
	case MSG_CTRL_GAMEWND:
		if (m_pGameWnd)
		{
			return m_pGameWnd->Msg(dwMsg,dwData,pControl);
		}
		break;
	default:
		return CCtrlMainWin::MsgToWnd(dwWndMsg,dwMsg,dwData,pControl);
		break;
	}

	return false;
}

bool CMainWnd::MsgToWnd(const char *szName,DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	if (strcmp(szName,"LoginWnd") == 0)
	{
		if (m_pLoginWnd)
		{
			return m_pLoginWnd->Msg(dwMsg,dwData,pControl);
		}
	}
	else if (strcmp(szName,"SelectCharWnd") == 0)
	{
		if (m_pSelectCharWnd)
		{
			return m_pSelectCharWnd->Msg(dwMsg,dwData,pControl);
		}
	}
	else if (strcmp(szName,"QuitWnd") == 0)
	{
		if (m_pQuitWnd)
		{
			return m_pQuitWnd->Msg(dwMsg,dwData,pControl);
		}
	}
	else if (strcmp(szName,"TalkViewWnd") == 0)
	{
		if (m_pTalkViewWnd)
		{
			return m_pTalkViewWnd->Msg(dwMsg,dwData,pControl);
		}
	}
	else if (strcmp(szName,"GameWnd") == 0)
	{
		if (m_pGameWnd)
		{
			return m_pGameWnd->Msg(dwMsg,dwData,pControl);
		}
	}

	return CCtrlMainWin::MsgToWnd(szName,dwMsg,dwData,pControl);
}


bool CMainWnd::ChangeUi(E_UITYPE eUi,bool bCheckWindowWidth)
{
	//if (bCheckWindowWidth && g_pGfx->GetWidth() <= 800 && eNewType == EUT_FASHION)
	//{
	//	g_MsgBoxMgr.PopSimpleAlert("时尚版界面不支持800*600分辨率。");
	//	return false;
	//}


	E_UITYPE eNewType = eUi;
	if (eUi == EUT_NONE)
	{
		if(g_EutUiType == EUT_CLASSIC)
			eNewType = EUT_FASHION;
		else
			eNewType = EUT_CLASSIC;
	}

	if (g_EutUiType == eNewType)
	{
		return true;
	}

	ReSetUI(eNewType);

	ReSetAllWndPos();

	return true;
}

void CMainWnd::ReSetUI(E_UITYPE eNewType)
{
	g_EutUiType = eNewType;

	//写配置文件
	string strFileName = GetGameDataDir();
	strFileName += "\\config\\LoginIdCfg.ini";
	char strTemp[32]={0};
	sprintf(strTemp,"%d",int(g_EutUiType));
	WritePrivateProfileString("ChooseUi",g_Login.GetLoginID(),strTemp,strFileName.c_str());


	if (FindWindowByName("PanelWnd"))
	{
		PopupWindow(MSG_CTRL_PANEL_WND,OPER_RECREATE);//panel wnd
	}
	if (FindWindowByName("HandyMapWnd"))
	{
		PopupWindow("HandyMapWnd",OPER_RECREATE);
	}

	if(m_pTalkViewWnd)
	{
		//m_pTalkViewWnd->ReCreate();
		REMOVECONTROL(m_pTalkViewWnd);
		m_pTalkViewWnd = new CTalkViewWnd();
		AddControl(m_pTalkViewWnd,10000);
		m_pTalkViewWnd->Create(this);
	}

	g_TalkMgr.Refill();

	//香重新点
	if (g_OtherData.GetCenserStartTime() > 0)
	{
		g_pMagicCtrl->FinishMagicByID(MAGICID_SENCER_EFFECT);
		g_OtherData.SetCenserStart(true);
	}
}

