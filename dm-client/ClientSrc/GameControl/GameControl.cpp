#include "GameControl.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameDefine.h"
#include "GameData/GameData.h"
#include "GameMap/GameMap.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/RangeStruct.h"
#include "WndClass/GameWnd/GameManager.h"
#include "GameData/OtherData.h"
#include "Global/Interface/AudioInterface.h"
#include "GameMap/MinMap.h"
#include "BaseClass/Misc/cliggproto.h"
#include "BaseClass/Misc/cligsproto.h"
#include "GameData/ConfigData.h"
#include "GameClient/ReplayManager.h"


CGameControl::CGameControl(void)
{
	m_msgs.clear();
	memset(m_cMsgBuf,0,MAX_MESSAGE_BUFFER);
	memset(m_cBuf,0,MAX_SEND_MESSAGE_SIZE);

    //////////////////////////////////////////////////////////////////////////

	m_callback[LG_CLI_SS_INFO]=                    &CGameControl::MSG_DlServerIP_Ntf;
	m_callback[LG_CLI_GroupList_NTF]=              &CGameControl::MSG_GroupList_Ntf;
    //m_callback[LG_CLI_SelGroup_ACK]=               &CGameControl::MSG_SelGroup_Ack;
	//m_callback[SM_SELECTSERVER_OK]=               &CGameControl::MSG_SelGroup_Ack;
    //m_callback[GG_CLI_Login_ACK]=                  &CGameControl::MSG_Login_Ack;
	m_callback[SM_PASSWD_FAIL]=                  &CGameControl::MSG_Login_Error_Ack;		//登录失败,账号密码错误
	m_callback[SC_LOGIN_USERPWDOK]=                  &CGameControl::MSG_Login_Succ;		//登录成功 SM_PASSOK_SELECTSERVER 529
	m_callback[SC_LOGIN_CHARSERVER]=                  &CGameControl::MSG_SelGroup_Ack;		//登录成功 SM_SELECTSERVER_OK 530
	m_callback[SM_QUERYCHR]=                  &CGameControl::MSG_Login_Ack;				//返回角色列表
	//m_callback[SM_QUERYCHR_FAIL]=                  &CGameControl::MSG_Login_Ack;		//查询失败处理
	
	//m_callback[GG_CLI_CreateRole_ACK]=             &CGameControl::MSG_CreateRole_Ack;
	m_callback[SC_CHARACTER_INFO] =                 &CGameControl::MSG_CreateRole_Ack;		//创建角色成功

    //m_callback[GG_CLI_SelectRole_ACK]=             &CGameControl::MSG_SelRole_Ack;
	m_callback[SC_SELCHAR_RUNSERVER] =              &CGameControl::MSG_SelRole_Ack;		//选择角色成功,进入游戏

	m_callback[GG_CLI_DelRole_ACK]=                &CGameControl::MSG_DelRole_Ack;      //lzhez 删除角色成功处理
    m_callback[GG_CLI_ReselRole_ACK]=              &CGameControl::MSG_ReselRole_Ack;
	m_callback[GG_CLI_UnDelRole_ACK]=              &CGameControl::MSG_UnDelRole_Ack;    //lzhez 删除角色失败处理
	m_callback[GG_CLI_KickOff_NTF]=                &CGameControl::MSG_KickOff_NTF;
	m_callback[GG_CLI_GroupInfo_NTF]=              &CGameControl::MSG_GroupInfo_NTF;

	m_callback[CLI_GG_MERGE_PT_ACK]=               &CGameControl::MSG_MergePT_Ack;

	m_callback[GG_CLI_NeedAuthen_REQ]=             &CGameControl::MSG_NeedAuthen_Req;

	m_callback[GG_CLI_Challenge_REQ]=              &CGameControl::MSG_Challenge_Req;
    m_callback[GG_CLI_Verify_REQ]=                 &CGameControl::MSG_Verify_Req;
	m_callback[GG_CLI_DynCode_NTF]=                &CGameControl::MSG_DynCode_NTF;
	m_callback[GG_CLI_InGameVerify_REQ]=           &CGameControl::MSG_InGameVerify_Req;
	m_callback[GG_CLI_LoginVerify_REQ]=            &CGameControl::Msg_Login_Check_Mask;

	m_callback[SC_CONFIG_INFO] =				   &CGameControl::MSG_Config_Req;		//客户端配置信息
    //////////////////////////////////////////////////////////////////////////


	m_callback[SC_ENLARGE_PACKAGE_INFO_ACK] =       &CGameControl::MSG_Enlarge_Package_Info_Ack;
	m_callback[SC_ACTIVERATE_PRIZE] =               &CGameControl::MSG_Activerate_Prize;
	m_callback[SC_PLAYER_VIPINFO] =                 &CGameControl::MSG_Player_VipInfo;
	m_callback[SC_VIPSTORE_INFO] =                  &CGameControl::MSG_VipStoreInfo;
	m_callback[SC_HASWAIGUA] =                      &CGameControl::MSG_HasWaiGua;
	m_callback[SC_FULL_ZHENBAO_LEFT_TIME] =         &CGameControl::MSG_Full_ZhenBao_Left_Time;
	m_callback[SC_ZHENBAO_VALUE] =                  &CGameControl::MSG_ZhenBao_Value;
	m_callback[SC_HONOR_VALUE] =                    &CGameControl::MSG_Honor_Value;
// 	m_callback[SC_TRUSTEPSHIP_ONLY_FRIEND] =        &CGameControl::MSG_Trustepship_Only_Friend;
// 	m_callback[SC_TRUSTEESHIP_MEMBER_IS_OFFLINE] =  &CGameControl::MSG_Trusteeship_Member_Is_Offline;
 	m_callback[SC_MONSTER_CHANGE_BALL] =            &CGameControl::MSG_Monster_Change_Ball;
	m_callback[SC_MSG_TXZG] =                       &CGameControl::Msg_Txzg;
// 	m_callback[SC_MICROCONTROL_SELECT] =            &CGameControl::MSG_MicroControl_Select;
// 	m_callback[SC_MICROCONTROL_RECEIVEPROTOCOL] =   &CGameControl::MSG_MicroControl_ReceiveProtocol;
// 	m_callback[SC_TRUSTEESHIP_GOOD_EXCHANGE] =      &CGameControl::MSG_Trusteeship_GoodExchange;
// 	m_callback[SC_TRUSTEESHIP_DATA_EXCHANGE] =      &CGameControl::MSG_Trusteeship_DataExchange;
	m_callback[SC_GIVEONLINEPRIZE] =                &CGameControl::Msg_GiveOnlinePrize;
	m_callback[SC_BLESSZHONGZHOU_CHANGE_DIALOG] =   &CGameControl::MSG_Open_BlessZhongZhouChangeDlg;
	m_callback[SC_LUCKQYZ_PRIZE] =                  &CGameControl::MSG_LuckQYZ_Prize;
	m_callback[SC_LUCKYDIALOG] =	                &CGameControl::MSG_Open_LuckQYZWnd;
	m_callback[SC_VIP_STORE_INFO] =	                &CGameControl::MSG_Vip_Store_Info;
	m_callback[SC_VIP_STORE_PERSONAL_INFO] =	    &CGameControl::MSG_Vip_Store_Personal_Info;
	m_callback[SC_QUICK_TG_MONSTER_LIST] =	        &CGameControl::MSG_Quick_TG_Monster_List;
	m_callback[SC_HEIYANMO_REFRACTION] =	        &CGameControl::MSG_HeiYanMo_Refraction;
	m_callback[SC_MSGBOX_COMMAND] =	                &CGameControl::MSG_MsgBox_Command;
	m_callback[SC_DYNOBJ_APPEAR] =	                &CGameControl::MSG_DynObj_Appear;
	m_callback[SC_DYNOBJ_DISAPPEAR] =	            &CGameControl::MSG_DynObj_DisAppear;
	m_callback[SC_CREDIT_TOKEN] =	                &CGameControl::MSG_Credit_Token;
	m_callback[SC_CREDIT_REQ] =	                    &CGameControl::MSG_Credit_Req;
	m_callback[SC_CREDIT_INFO] =	                &CGameControl::MSG_Credit_Info;
	m_callback[SC_KFZ_ARENA_LIST] =	                &CGameControl::MSG_Kfz_ArenaList;
	m_callback[SC_KFZ_MEMBER_STATE] =	            &CGameControl::MSG_Kfz_MemberState;
	m_callback[SC_KFZ_RESULT] =			            &CGameControl::MSG_Kfz_Result;
	m_callback[SC_SECNOND_TIME_OUT] =			    &CGameControl::MSG_Second_TimeOut;
	m_callback[SC_KFZ_VISTOR_JOIN] =			    &CGameControl::MSG_Kfz_Visitor_Join;
	m_callback[SC_KFZ_ACTIVATE_RESULT] =			&CGameControl::MSG_Kfz_Activate_Result;
	m_callback[SC_KFZ_JOIN_MEMBER] =			    &CGameControl::MSG_Kfz_Join_Member;
    m_callback[SC_TUTENG_STATE] =				    &CGameControl::Msg_TuTeng_State;
	m_callback[SC_FLAG_WUXING] =				    &CGameControl::Msg_WuXing_Flag;
    m_callback[SC_SEAL_TAKE_PUT] =				    &CGameControl::Msg_Take_Put_Seal;
    m_callback[SC_UPDATE_MEDAL_EXP] =			    &CGameControl::Msg_Update_Medal_Exp;
    m_callback[SC_UPDATE_MEDAL_ATTRIBUTE] =		    &CGameControl::Msg_Update_Medal_Attribute;
    m_callback[SC_PLAYER_SKILLPOWER] =              &CGameControl::MSG_Player_SkillPower;
	m_callback[SC_SPLIT_STONE] =                    &CGameControl::MSG_Split_Stone;
	m_callback[SC_MASTER_PRENTICE_MSG] =            &CGameControl::MSG_Master_Prentice;
	m_callback[SC_PLAYER_TRADE] =                   &CGameControl::MSG_Player_Trade;
	m_callback[SC_NPC_QUICK_EXCHANGE] =             &CGameControl::MSG_Npc_Quick_Exchange;
	m_callback[SC_PET_ADOPT_END] =                  &CGameControl::MSG_PET_ADOPT_END;
	m_callback[SC_MYADOPT_PET_INFO] =               &CGameControl::MSG_MyAdoptPet_Info;
	m_callback[SC_PET_ADOPT_INFO] =                 &CGameControl::MSG_Pet_Adopt_Info;
	m_callback[SC_SENDOUT_PET_INFO] =               &CGameControl::MSG_SendOutPet_Info; 
	m_callback[CS_SPECIALITEMACT] =                 &CGameControl::MSG_Deal_Special_Obj; 
	m_callback[SC_SMALL_GEM_COUNT] =                &CGameControl::MSG_SmallGem_Count;
	m_callback[SC_SNDAMARK_ANSWER] =                &CGameControl::MSG_SndaMark_Answer;
    m_callback[SC_PET_GOODINFO] =                   &CGameControl::MSG_PetGoodInfo;
	m_callback[SC_PRENTICE_FLAG] =                  &CGameControl::MSG_Prentice_Flag;
	m_callback[SC_CELLPHONE_USER_INPUT_REQUEST] =   &CGameControl::MSG_CellPhoneUser_InputRequest;
	m_callback[SC_PLAYER_CONNECTED] =               &CGameControl::MSG_Player_Connected;
	m_callback[SC_RUNGATE_ENCRYPT] =                &CGameControl::MSG_Rungate_Encrypt;
	//m_callback[SC_LEVEL_CHECK] =                    &CGameControl::MSG_Level_Check;
	m_callback[SC_LOGIN_TUIGUANG] =                 &CGameControl::MSG_Login_TuiGuang;
	m_callback[SC_LOGIN_VALIDATE] =                 &CGameControl::MSG_Login_Validate;
	m_callback[SC_YUANBAO_INFO] =                   &CGameControl::MSG_Yuanbao_Info;
	m_callback[SC_BIND_YUANBAO_INFO] =              &CGameControl::MSG_Bind_Yuanbao_Info;
	m_callback[SC_DYNAMIC_PASSWORD_CHANGE] =        &CGameControl::MSG_Dynamic_Password_Change;
	m_callback[SC_OBJECT_DETAIL] =                  &CGameControl::MSG_Object_Detail;
	m_callback[SC_OTHER_MESSAGES] =                 &CGameControl::MSG_Other_Messages;
	m_callback[SC_ISVALID_ACTION] =                 &CGameControl::MSG_Isvalid_Action;
	m_callback[SC_PLAYER_AUTO_ARM] =                &CGameControl::MSG_Player_Auto_Arm;
	m_callback[SC_PACKAGE_UPDATE_OBJECT] =          &CGameControl::MSG_Package_Update_Object;
	m_callback[SC_SESSION_ID] =                     &CGameControl::MSG_Session_ID;
	m_callback[SC_KICK_OUT] =                       &CGameControl::MSG_Kick_Out;
	m_callback[SC_GUILD_WAR_MESSAGE] =              &CGameControl::MSG_Guild_War_Message;
	//m_callback[SC_RESTORE_CHAR_OK] =                &CGameControl::MSG_Restore_Char_Ok;
	//m_callback[SC_RESTORE_CHAR_ERROR] =             &CGameControl::MSG_Restore_Char_Error;
	//m_callback[SC_DELETED_CHAR_INFO] =			    &CGameControl::MSG_Deleted_Char_Info;
	m_callback[CONNECT_ERROR] =                     &CGameControl::MSG_Connect_Error;
	m_callback[SC_PASSWORD_CHANGE_OK] =             &CGameControl::MSG_Password_Change_Ok;
	m_callback[SC_PASSWORD_CHANGE_FAIL] =           &CGameControl::MSG_Password_Change_Fail;
	m_callback[SC_USER_FULL] =                      &CGameControl::MSG_User_Full;
	m_callback[SC_USERNAME_INFO] =                  &CGameControl::MSG_Username_Info;
	m_callback[SC_GS_JUMP_BEGIN] =                  &CGameControl::MSG_GS_Jump_Begin;
	m_callback[SC_GAME_GREETING] =                  &CGameControl::MSG_Game_Greeting;
	m_callback[SC_WEAPON_BREAK] =                   &CGameControl::MSG_Weapon_Break;
	m_callback[SC_SPECIAL_EFFECT] =                 &CGameControl::MSG_Special_Effect;
	m_callback[SC_PLAYER_MAGIC_COLOR] =             &CGameControl::MSG_Player_Magic_Color;
	m_callback[SC_GUILD_INVITED] =                  &CGameControl::MSG_Guild_Invited;	
	m_callback[SC_OBJECT_APPEARANCE] =              &CGameControl::MSG_Object_Appearance;
	m_callback[SC_OBJECT_DISAPPEARANCE] =           &CGameControl::MSG_Object_Disappearance;
	m_callback[SC_PLAYER_POSITION] =                &CGameControl::MSG_Player_Position;
	m_callback[SC_PACKAGE_ALL_INFO] =               &CGameControl::MSG_Package_All_Info;
	m_callback[SC_PLAYER_INFO2] =                   &CGameControl::MSG_Player_Info2;
	m_callback[SC_MAGIC_ABILITY] =                  &CGameControl::MSG_Magic_Ability;
	m_callback[SC_PLAYER_ATTRIBUTE] =               &CGameControl::MSG_Player_Attribute;
	m_callback[SC_PLAYER_ARM_INFO] =                &CGameControl::MSG_Player_Arm_Info;
	m_callback[SC_PACKAGE_ADD_OBJECT] =             &CGameControl::MSG_Package_Add_Object;
	m_callback[SC_PACKAGE_REJECT_CONFIRM] =         &CGameControl::MSG_Package_Reject_Confirm;
	m_callback[SC_PLAYER_INFO1] =                   &CGameControl::MSG_Player_Info1;
	m_callback[SC_PLAYER_MONEY] =                   &CGameControl::MSG_Player_Money;
	m_callback[SC_PLAYER_LIFE] =                    &CGameControl::MSG_Player_Life;
	m_callback[SC_PLAYER_EXPERIENCE_UP] =           &CGameControl::MSG_Player_Experience_Up;
	m_callback[SC_PLAYER_RANK_UP] =                 &CGameControl::MSG_Player_Rank_Up;
	m_callback[SC_PLAYER_SUCCESS]		=           &CGameControl::MSG_Player_Success;
	m_callback[SC_PLAYER_FAIL]			=           &CGameControl::MSG_Player_Fail;
	m_callback[SC_OBJECT_USE_SUCCESS] =             &CGameControl::MSG_Object_Use_Success;
	m_callback[SC_OBJECT_USE_FAILED] =              &CGameControl::MSG_Object_Use_Failed;
	m_callback[SC_MESSAGE_TALK] =                   &CGameControl::MSG_Message_Talk;
	m_callback[SC_MESSAGE_GUILD] =                  &CGameControl::MSG_Message_Guild;
	m_callback[SC_MESSAGE_SYSTEM] =                 &CGameControl::MSG_Message_System;
	m_callback[SC_MAP_NAME] =                       &CGameControl::MSG_Map_Name;
	m_callback[SC_MAP_JUMP] =                       &CGameControl::MSG_Map_Jump;
	m_callback[SC_MAP_JUMP_SUCCESS] =               &CGameControl::MSG_Map_Jump_Success;
	m_callback[SC_MAP_DESCRIPTION]	=				&CGameControl::MSG_Map_Desc;
	m_callback[SC_MAP_CHANGE] =                     &CGameControl::MSG_Map_Change;
	m_callback[SC_GUILD_ALIGN_FAILED] =             &CGameControl::MSG_Guild_Align_Failed;
	m_callback[SC_OBJECT_TAKE] =                    &CGameControl::MSG_Object_Take;
	m_callback[SC_MAP_NO_SMALL_MAP] =               &CGameControl::MSG_Map_No_Small_Map;
	m_callback[SC_MAP_OUTDOOR_NUMBER] =             &CGameControl::MSG_Map_Small_Map;
	m_callback[SC_GUILD_MEMBER_TITLE_EDIT_FAILED] = &CGameControl::MSG_Guild_Member_Title_Edit_Failed;
	m_callback[SC_MONSTER_REFRESH] =                &CGameControl::MSG_Monster_Refresh;
	m_callback[SC_PLAYER_SWORD_SWITCH] =            &CGameControl::MSG_Player_Sword_Switch;
	m_callback[SC_PLAYER_USE_SWORD] =               &CGameControl::MSG_Player_Use_Sword;
	m_callback[SC_PLAYER_DIG_BREAK] =               &CGameControl::MSG_Player_Dig_Break;
	m_callback[SC_PLAYER_PET_STATE] =               &CGameControl::MSG_Player_Pet_State;
	m_callback[SC_NEED_DYN_CRYPT] =                 &CGameControl::MSG_NEED_DYN_CRYPT;
	m_callback[SC_CHECK_USER] =                     &CGameControl::MSG_Check_User;

	m_callback[SC_GUILD_MEMBER_GET_TITLE] =         &CGameControl::MSG_Guild_Member_Get_Title;
	m_callback[SC_OTHER_PLAYER_INFO] =              &CGameControl::MSG_Other_Player_Info;
	m_callback[SC_MONSTER_NAME] =                   &CGameControl::MSG_Monster_Name;
	m_callback[SC_MONSTER_APPEARANCE] =             &CGameControl::MSG_Monster_Appearance;
	m_callback[SC_MONSTER_WALK] =                   &CGameControl::MSG_Monster_Walk;
	m_callback[SC_MONSTER_RUN] =                    &CGameControl::MSG_Monster_Run;
	m_callback[SC_PLAYER_ARM_REFRESH] =             &CGameControl::MSG_Player_Arm_Refresh;
	m_callback[SC_PLAYER_ARM_FAIL] =                &CGameControl::MSG_Player_Arm_Fail;
	m_callback[SC_PLAYER_UNARM_REFRESH] =           &CGameControl::MSG_Player_Unarm_Refresh;
	m_callback[SC_PLAYER_UNARM_FAIL] =              &CGameControl::MSG_Player_Unarm_Fail;
	m_callback[SC_PLAYER_TASK_INFO] =               &CGameControl::MSG_Player_Task_Info;
	m_callback[SC_MESSAGE_PRIVATE] =                &CGameControl::MSG_Message_Private;
	m_callback[SC_MONSTER_CORPUS] =                 &CGameControl::MSG_Monster_Corpus;
	m_callback[SC_MONSTER_BONE] =                   &CGameControl::MSG_Monster_Bone;
	m_callback[SC_MONSTER_DISAPPEARANCE] =          &CGameControl::MSG_Monster_Disappearance;
	m_callback[SC_PACKAGE_REJECT_FAIL] =            &CGameControl::MSG_Package_Reject_Fail;
	m_callback[SC_JUMP_REFRESH] =                   &CGameControl::MSG_Jump_Refresh;
	m_callback[SC_PLAYER_REFRESH] =                 &CGameControl::MSG_Player_Refresh;
	m_callback[SC_MONSTER_HURTED] =                 &CGameControl::MSG_Monster_Hurted;
	m_callback[SC_MONSTER_HURTED2] =                &CGameControl::MSG_Monster_Hurted2;
	m_callback[SC_OBJECT_DURA] =                    &CGameControl::MSG_Object_Dura;
	m_callback[SC_PLAY_ATTACKED_SOUND] =            &CGameControl::MSG_Play_Attacked_Sound;
	m_callback[SC_MONSTER_STATE] =                  &CGameControl::MSG_Monster_State;
	m_callback[SC_MONSTER_EXTRASTATE] =             &CGameControl::MSG_Monster_ExtraState;
	m_callback[SC_MONSTER_NAMECOLOR] =              &CGameControl::MSG_Monster_NameColor;
	m_callback[SC_MONSTER_DEAD] =                   &CGameControl::MSG_Monster_Dead;
	m_callback[SC_MONSTER_ATTACK1] =                &CGameControl::MSG_Monster_Attack1;
	m_callback[SC_MONSTER_ATTACK2] =                &CGameControl::MSG_Monster_Attack2;
	m_callback[SC_MONSTER_ATTACK3] =                &CGameControl::MSG_Monster_Attack3;
	m_callback[SC_MONSTER_ATTACK_KILL] =            &CGameControl::MSG_Monster_Attack_Kill;
	m_callback[SC_MONSTER_ATTACK_STICK] =           &CGameControl::MSG_Monster_Attack_Stick;
	m_callback[SC_MONSTER_ATTACK_MOON] =            &CGameControl::MSG_Monster_Attack_Moon;
	m_callback[SC_MONSTER_ATTACK_FIRE] =            &CGameControl::MSG_Monster_Attack_Fire;
	m_callback[SC_MONSTER_ATTACK_SWORD] =           &CGameControl::MSG_Monster_Attack_Sword;
	m_callback[SC_MONSTER_CUT] =                    &CGameControl::MSG_Monster_Cut;
	m_callback[SC_MONSTER_BACK] =                   &CGameControl::MSG_Monster_Back;
	m_callback[SC_MONSTER_BACK1] =                  &CGameControl::MSG_Monster_Back1;
	m_callback[SC_MONSTER_BACK2] =					&CGameControl::MSG_Monster_Back2;
	m_callback[SC_MONSTER_ANIMATE_APPEAR] =         &CGameControl::MSG_Monster_Animate_Appear;
	m_callback[SC_MONSTER_ANIMATE_DISAPPEAR] =      &CGameControl::MSG_Monster_Animate_Disappear;
	m_callback[SC_CORPSE_RELIVE] =                  &CGameControl::MSG_Corpse_Relive;
	m_callback[SC_MONSTER_MAGIC_FLY] =              &CGameControl::MSG_Monster_Magic_Fly;
	m_callback[SC_MONSTER_PROTECTOR_COLOR] =        &CGameControl::MSG_Monster_Protector_Color;
	m_callback[SC_MONSTER_SOULWALL_STATE] =         &CGameControl::MSG_Monster_SoulWall_State;
	m_callback[SC_GROUP_ENABLE] =                   &CGameControl::MSG_Group_Enable;
	m_callback[SC_VOICECHAT_ENABLE] =               &CGameControl::MSG_VoiceChat_Enable;
	m_callback[SC_VOICE_SERVER_ADDR]   =            &CGameControl::MSG_Voice_Server_Addr;
	m_callback[SC_VOICE_INFO]       =               &CGameControl::MSG_Voice_Info;
	m_callback[SC_GROUP_MEMBER_INFO] =              &CGameControl::MSG_Group_Member_Info;
	m_callback[SC_GROUP_MEMBER_DELETE_OK] =         &CGameControl::MSG_Group_Member_Delete_Ok;
	m_callback[SC_GROUP_MEMBER_DELETE_FAILED] =     &CGameControl::MSG_Group_Member_Delete_Failed;
	m_callback[SC_GROUP_MEMBER_DELETED_NAME] =      &CGameControl::MSG_Group_Member_Delete_Name;
	m_callback[SC_GROUP_MEMBER_POS] =               &CGameControl::MSG_Group_Member_Pos;
	m_callback[SC_GROUP_REQUEST] =					&CGameControl::MSG_Group_Request;
	m_callback[SC_TRADE_ENABLE] =                   &CGameControl::MSG_Trade_Enable;
	m_callback[SC_TRADE_DISABLE] =                  &CGameControl::MSG_Trade_Disable;
	m_callback[SC_TRADE_PLAYER_OBJECT_INFO] =       &CGameControl::MSG_Trade_Player_Object_Info;
	m_callback[SC_TRADE_OBJECT_CONFIRM] =           &CGameControl::MSG_Trade_Object_Confirm;
	m_callback[SC_TRADE_FAILED] =                   &CGameControl::MSG_Trade_Failed;
	m_callback[SC_TRADE_SUCCESS] =                  &CGameControl::MSG_Trade_Success;
	m_callback[SC_TRADE_MONEY_INFO] =               &CGameControl::MSG_Trade_Money_Info;
	m_callback[SC_TRADE_PLAYER_MONEY_INFO] =        &CGameControl::MSG_Trade_Player_Money_Info;
	m_callback[SC_TRADE_PLAYER_OBJECT_TAKEOUT] =    &CGameControl::MSG_Trade_Player_Object_Takeout;
	m_callback[SC_MONSTER_ATTACK_MAGIC] =           &CGameControl::MSG_Monster_Attack_Magic;
	m_callback[SC_MAGIC_EFFECTIVED] =               &CGameControl::MSG_Magic_Effectived;
	m_callback[SC_MAGIC_SKILL_VALUE] =              &CGameControl::MSG_Magic_Skill_Value;
	m_callback[SC_MAGIC_NOEFFECTIVED] =             &CGameControl::MSG_Magic_NoEffectived;
	m_callback[SC_MAGIC_POSITION_SUCCESS] =         &CGameControl::MSG_Magic_Position_Success;
	m_callback[SC_MAGIC_POSITION_FAILED] =          &CGameControl::MSG_Magic_Position_Failed;
	m_callback[SC_MAGIC_CONTINUE_BEGIN] =           &CGameControl::MSG_Magic_Continue_Begin;
	m_callback[SC_MAGIC_CONTINUE_END] =             &CGameControl::MSG_Magic_Continue_End;
	m_callback[SC_MAGIC_QUICK_MOVE] =               &CGameControl::MSG_Magic_Quick_Move;
	m_callback[SC_MAGIC_QUICK_MOVE_POSITION] =      &CGameControl::MSG_Magic_Quick_Move_Position;
	m_callback[SC_MAGIC_MONSTER1_ATTACK] =          &CGameControl::MSG_Magic_Monster1_Attack;	
	m_callback[SC_MAGIC_MONSTER2_ATTACK] =          &CGameControl::MSG_Magic_Monster2_Attack;

	m_callback[SC_GROUP_COMPANY_FAILED] =           &CGameControl::MSG_Group_Company_Failed;
	m_callback[SC_GROUP_MEMBER_ADD_FAILED] =        &CGameControl::MSG_Group_Member_Add_Failed;
	m_callback[SC_TRADE_TAKEIN_FAILED] =            &CGameControl::MSG_Trade_Tackin_Failed;
	m_callback[SC_GUILD_MESSAGE] =                  &CGameControl::MSG_Guild_Message;
	m_callback[SC_GUILD_MESSAGE_FAIL] =             &CGameControl::MSG_Guild_Message_Fail;
	m_callback[SC_GUILD_DELETE_MEMBER_FAILED] =     &CGameControl::MSG_Guild_Delete_Member_Failed;
	m_callback[SC_GUILD_ADDIN_GUILD_FAILED] =       &CGameControl::MSG_Guild_Addin_Guild_Failed;
	m_callback[SC_GUILD_DELETE_MEMBER_SUCCESS] =    &CGameControl::MSG_Guild_Delete_Member_Success;
	m_callback[SC_GUILD_MEMBER_LIST] =              &CGameControl::MSG_Guild_Member_List;
	m_callback[SC_GUILD_NEW_GUILD_LIST] =           &CGameControl::MSG_Guild_Member_List_New;
	m_callback[SC_GUILD_RET_EXP_VAL] =              &CGameControl::MSG_Guild_Ret_Exp_Val;
	m_callback[SC_GUILD_ALLY_SUCCESS] =             &CGameControl::MSG_Guild_Ally_Success;
	m_callback[SC_GUILD_ALIGN_CANCEL_FAILED] =      &CGameControl::MSG_Guild_Align_Cancel_Failed;
	m_callback[SC_GUILD_CREATE_FAIL] =              &CGameControl::MSG_Guild_Create_Fail;
	m_callback[SC_GUILD_CREATE_SUCCESS] =           &CGameControl::MSG_Guild_Create_Success;
	m_callback[SC_CREATE_ACCOUNT_FAIL] =            &CGameControl::MSG_Create_Account_Fail;
	m_callback[SC_CREATE_ACCOUNT_SUCCESS] =         &CGameControl::MSG_Create_Account_Success;
	m_callback[SC_CREATE_CHARACTER_FAIL] =          &CGameControl::MSG_Create_Character_Fail;

	m_callback[SC_EXCHANGE_TALK_MENU] =             &CGameControl::MSG_Exchange_Talk_Menu;
	m_callback[SC_EXCHANGE_BUY_MENU] =              &CGameControl::MSG_Exchange_Buy_Menu;
	m_callback[SC_EXCHANGE_SALE_FAIL] =             &CGameControl::MSG_Exchange_Sale_Fail;
	m_callback[SC_EXCHANGE_SALE_WINDOW] =           &CGameControl::MSG_Exchange_Sale_Window;
	m_callback[SC_EXCHANGE_SALE_OBJECT] =           &CGameControl::MSG_Exchange_Sale_Object;
	m_callback[SC_EXCHANGE_SALE_OK] =               &CGameControl::MSG_Exchange_Sale_Ok;
	m_callback[SC_EXCHANGE_BUY_OK] =                &CGameControl::MSG_Exchange_Buy_Ok;
	m_callback[SC_EXCHANGE_REPAIR_WINDOW] =         &CGameControl::MSG_Exchange_Repair_Window;
	m_callback[SC_EXCHANGE_REPAIR_MONEY] =          &CGameControl::MSG_Exchange_Repair_Money;
	m_callback[SC_EXCHANGE_REPAIR_SUCCESS] =        &CGameControl::MSG_Exchange_Repair_Success;
	m_callback[SC_EXCHANGE_REPAIR_FAIL] =           &CGameControl::MSG_Exchange_Repair_Fail;
	m_callback[SC_EXCHANGE_STORAGE_WINDOW] =        &CGameControl::MSG_Exchange_Storage_Window;
	m_callback[SC_EXCHANGE_STORAGE_OK] =            &CGameControl::MSG_Exchange_Storage_Ok;
	m_callback[SC_EXCHANGE_STORAGE_FAILED] =        &CGameControl::MSG_Exchange_Storage_Failed;
	m_callback[SC_EXCHANGE_GETBACK_WINDOW] =        &CGameControl::MSG_Exchange_Getback_Window;
	m_callback[SC_EXCHANGE_SECOND_BUY_MENU] =       &CGameControl::MSG_Exchange_Second_Buy_Menu;
	m_callback[SC_EXCHANGE_BUY_FAIL] =              &CGameControl::MSG_Exchange_Buy_Fail;
	m_callback[SC_EXCHANGE_TALK_END] =              &CGameControl::MSG_Exchange_Talk_End;
	m_callback[SC_EXCHANGE_GETBACK_OK] =            &CGameControl::MSG_Exchange_Getback_Ok;
	m_callback[SC_EXCHANGE_GETBACK_FAIL] =          &CGameControl::MSG_Exchange_Getback_Fail;
	m_callback[SC_EXCHANGE_POISON_MENU] =           &CGameControl::MSG_Exchange_Poison_Menu;
	m_callback[SC_EXCHANGE_POISON_FAIL] =           &CGameControl::MSG_Make_Poison_Fail;
	m_callback[SC_EXCHANGE_POISON_NEED] =           &CGameControl::MSG_Exchange_Poison_Need;
	m_callback[SC_EXCHANGE_POISON_MONEY] =          &CGameControl::MSG_Exchange_Poison_Money;
	m_callback[SC_BAD_GOOD] =                       &CGameControl::MSG_Bad_Good;
	m_callback[SC_EXCHANGE_FORGE_WINDOW] =          &CGameControl::MSG_Exchange_Forge_Window;
	m_callback[SC_EXCHANGE_TRAY_WND] =              &CGameControl::MSG_Exchange_Tray_Wnd;
	m_callback[SC_RETURN_RESULT] =                  &CGameControl::MSG_Return_Result;

	m_callback[SC_EXCHANGE_FORGE_PERCENT] =         &CGameControl::MSG_Exchange_Forge_Percent;
	m_callback[SC_EXCHANGE_FORGE_RESULT] =          &CGameControl::MSG_Exchange_Forge_Result;
	m_callback[SC_EXCHANGE_BLESS_RESULT] =          &CGameControl::MSG_Exchange_Bless_Result;
	m_callback[SC_EXCHANGE_FORGE_EXPERIENCE] =      &CGameControl::MSG_Exchange_Forge_Experience;
	m_callback[SC_SYSTEM_SKYROCKET] =               &CGameControl::MSG_System_Skyrocket;

	m_callback[SC_PLAYER_SKILL_DELETE] =            &CGameControl::MSG_Player_Skill_Delete;
	m_callback[SC_PLAYER_SKILL_ADD] =               &CGameControl::MSG_Player_Skill_Add;
	m_callback[SC_ERROR_LOGIN] =                    &CGameControl::MSG_Error_Login;
	m_callback[SC_SYSTEM_PLAY_MIDI] =               &CGameControl::MSG_System_Play_Midi;
	m_callback[SC_NPC_MSGBOX] =                     &CGameControl::MSG_Npc_Msgbox;

	m_callback[SC_OPEN_ATTACK_KILL] =               &CGameControl::MSG_Open_Attack_Kill;
	m_callback[SC_CLOSE_ATTACK_KILL] =				&CGameControl::MSG_Close_Attack_Kill;
	m_callback[SC_OPEN_ATTACK_STICK] =              &CGameControl::MSG_Open_Attack_Stick;
	m_callback[SC_CLOSE_ATTACK_STICK] =             &CGameControl::MSG_Close_Attack_Stick;
	m_callback[SC_OPEN_ATTACK_MOON] =               &CGameControl::MSG_Open_Attack_Moon;
	m_callback[SC_CLOSE_ATTACK_MOON] =              &CGameControl::MSG_Close_Attack_Moon;
	m_callback[SC_OPEN_ATTACK_FIRE] =               &CGameControl::MSG_Open_Attack_Fire;
	m_callback[SC_CLOSE_ATTACK_FIRE] =              &CGameControl::MSG_Close_Attack_Fire;
	m_callback[SC_VALID_DIG] =                      &CGameControl::MSG_Valid_Dig;
	m_callback[SC_INTERNET_GOOD] =                  &CGameControl::MSG_Internet_Good;
	m_callback[SC_INTERNET_FAIL] =                  &CGameControl::MSG_Internet_Fail;

	m_callback[SC_OPEN_REMAIN_SHADE] =              &CGameControl::MSG_Open_Remain_Shade;
	m_callback[SC_CLOSE_REMAIN_SHADE] =             &CGameControl::MSG_Close_Remain_Shade;
	m_callback[SC_OPEN_BLOOD_SHADE] =               &CGameControl::MSG_Open_Blood_Shade;
	m_callback[SC_CLOSE_BLOOD_SHADE] =              &CGameControl::MSG_Close_Blood_Shade;
	m_callback[SC_OPEN_WHOLE_MOON] =                &CGameControl::MSG_Open_Whole_Moon;
	m_callback[SC_CLOSE_WHOLE_MOON] =               &CGameControl::MSG_Close_Whole_Noon;
	m_callback[SC_OPEN_THUNDER_FIRE] =              &CGameControl::MSG_Open_Thunder_Fire;
	m_callback[SC_CLOSE_THUNDER_FIRE] =             &CGameControl::MSG_Close_Thunder_Fire;
	m_callback[SC_OPEN_DESTROY_WEAPON] =            &CGameControl::MSG_Open_Destroy_Weapon;
	m_callback[SC_CLOSE_DESTROY_WEAPON] =           &CGameControl::MSG_Close_Destroy_Weapon;

	m_callback[SC_OPEN_DESTROY_SHELL] =             &CGameControl::MSG_Open_Destroy_Shell;
	m_callback[SC_CLOSE_DESTROY_SHELL] =            &CGameControl::MSG_Close_Destroy_Shell;
	m_callback[SC_OPEN_DESTROY_SHIELD] =            &CGameControl::MSG_Open_Destroy_Shield;
	m_callback[SC_CLOSE_DESTROY_SHIELD] =           &CGameControl::MSG_Close_Destroy_Shield;

	m_callback[SC_NEED_UPTO_UUID] =                 &CGameControl::MSG_Need_Upto_Uuid;
	m_callback[SC_SERVER_TEMP_UUID] =               &CGameControl::MSG_Server_Temp_Uuid;
	m_callback[SC_CHANGE_UUID_RESULT] =             &CGameControl::MSG_Change_Uuid_Result;

	m_callback[SC_DYNAMIC_PASSWORD_RESULT] =        &CGameControl::MSG_Dynamic_Password_Result;
	m_callback[SC_UNDYNAMIC_PASSWORD_RESULT] =      &CGameControl::MSG_Undynamic_Password_Result;
	m_callback[SC_ASSERT_BIND] =                    &CGameControl::MSG_Assert_Bind;
	m_callback[SC_INPUT_RESERVE_PASSWORD] =         &CGameControl::MSG_Input_Reserve_Password;
	m_callback[SC_CARD_RESULT] =                    &CGameControl::MSG_Card_Result;
	m_callback[SC_NORMAL_MSGBOX] =                  &CGameControl::MSG_Normal_MessageBox;
	m_callback[SC_WEB_MESSAGEBOX] =                 &CGameControl::MSG_Web_MessageBox;
	m_callback[SC_VERSION_NOMATCH] =                &CGameControl::MSG_Version_NoMatch;
	m_callback[SC_TRAIN_SUCCESS] =                  &CGameControl::MSG_Train_Success;
	m_callback[SC_FRIEND_LIST] =                    &CGameControl::MSG_Friend_List;
	m_callback[SC_ADD_FRIEND_SUCCESS] =             &CGameControl::MSG_Add_Friend_Success;
	m_callback[SC_DEL_FRIEND_SUCCESS] =             &CGameControl::MSG_Del_Friend_Success;
	m_callback[SC_FRIEND_ERROR] =                   &CGameControl::MSG_Friend_Error;
	m_callback[SC_FRIEND_NOTICE] =                  &CGameControl::MSG_Friend_Notice;
	m_callback[SC_FRIEND_REQUEST] =                 &CGameControl::MSG_Friend_Request;
	m_callback[SC_FRIEND_REQUEST_IM] =              &CGameControl::MSG_Friend_Request_IM;
	m_callback[SC_FRIEND_MOQI_CHANGE] =             &CGameControl::MSG_Friend_MoQi_Change;
	m_callback[SC_UNREAD_LEAVE_WORD] =              &CGameControl::MSG_Unread_Leave_Word;
	m_callback[SC_CHAT_NOT_ONLINE] =                &CGameControl::MSG_Chat_Not_Online;
	m_callback[SC_OBJECTS_POSITION] =               &CGameControl::MSG_Objects_Position;
	m_callback[SC_WEATHER] =                        &CGameControl::MSG_Weather;
	m_callback[SC_MONSTER_STOPGO] =                 &CGameControl::MSG_Monster_StopGo;
	m_callback[SC_MONSTER_MOVEPOSITION] =           &CGameControl::MSG_Monster_MovePosition;
	m_callback[SC_RIDE_END] =                       &CGameControl::MSG_Player_RideEnd;
	m_callback[SC_PLAYER_SALUTE] =                  &CGameControl::MSG_Player_Salute;
	m_callback[SC_RUNGATE_ANSWER] =                 &CGameControl::MSG_Rungate_Answer;
	m_callback[SC_STORYBOARD_URL] =                 &CGameControl::MSG_StoryBoard_URL;
	m_callback[SC_BOOTH_REQUEST_RESULT] =           &CGameControl::MSG_Booth_Request_Result;
	m_callback[SC_MONSTERATTACK_EFFECT] =           &CGameControl::MSG_MonsterAttack_Effect;////////////////
	m_callback[SC_SPECIAL_MONSTER_EFFECT] =         &CGameControl::MSG_Special_Monster_Effect;
	m_callback[SC_FU_SHENG_TIME] =                  &CGameControl::MSG_Fu_Sheng_Time;
	m_callback[SC_BOOTH_SHOW_OTHER] =               &CGameControl::MSG_Booth_Show_Other_Stall;
	m_callback[SC_C2C_BOOTH_INFO] =                 &CGameControl::MSG_C2C_Booth_Info;
	m_callback[SC_INPUT_MIBAO_LATER] =              &CGameControl::MSG_Input_MIBAO_LATER;
	m_callback[SC_INPUT_DPWD] =                     &CGameControl::MSG_Input_DPWD;
	m_callback[SC_RUNGATE_DPWD] =                   &CGameControl::MSG_Rungate_DPWD;
	m_callback[SC_EXCHANGE_GEM_RESULT] =            &CGameControl::MSG_Exchange_Gem_Result;
	m_callback[SC_EXCHANGE_BOX_RESULT] =            &CGameControl::MSG_Exchange_Box_Result;
	m_callback[SC_EXCHANGE_LIST_MENU] =             &CGameControl::MSG_Exchange_List_Menu;
	m_callback[SC_SERVER_VERSION] =                 &CGameControl::MSG_ServerVersion;
	m_callback[SC_PACKAGE_CAPACITY] =               &CGameControl::MSG_Package_Capacity;
	m_callback[SC_PERSON_STATE] =                   &CGameControl::MSG_Person_State;
	m_callback[SC_MONSTER_MAGIC_EFFECT] =           &CGameControl::MSG_Monster_Magic_Effect;
	m_callback[SC_ITEM_EFFECT_SWITH] =              &CGameControl::MSG_Item_Effect_Switch;
	m_callback[SC_EXCHANGE_REPAIR_DURATION] =       &CGameControl::MSG_Exchange_Repair_Duration;
	m_callback[SC_SELF_CHARACTER_ACTION] =          &CGameControl::MSG_Self_Character_Action;
	m_callback[SC_ANSWER_SERVER_TIME] =             &CGameControl::MSG_Answer_Server_Time;
	m_callback[SC_PET_OBJECT_GETBACK_RESULT] =      &CGameControl::MSG_Pet_Object_GetBack_Result;
	m_callback[SC_PET_OBJECT_STORAGE_RESULT] =      &CGameControl::MSG_Pet_Object_Storage_Result;
	m_callback[SC_PET_PACKAGE_CAPACITY] =           &CGameControl::MSG_Pet_Package_Capacity;
	m_callback[SC_PET_OBJECT_INFO] =                &CGameControl::MSG_Pet_Object_Info;
	m_callback[SC_PET_OBJECT_ADD] =                 &CGameControl::MSG_Pet_Object_Add;
	m_callback[SC_PET_OBJECT_PICKUP_FAIL] =         &CGameControl::MSG_Pet_Object_Pickup_Fail;
	m_callback[SC_INVESTIGATION] =                  &CGameControl::MSG_Investigation;
	m_callback[SC_PET_STATE_CHANGE] =               &CGameControl::MSG_Pet_State_Change;
	m_callback[SC_PT_STORE_INFO] =                  &CGameControl::MSG_PT_Store_Info;
	m_callback[SC_EXCHANGE_PT_BUY_MENU] =           &CGameControl::MSG_Exchange_PT_Buy_Menu;
	m_callback[SC_WOOLSTORE_TOKEN] =                &CGameControl::MSG_WoolStore_Token;
	m_callback[SC_SUBMIT_GOOD_INFO] =               &CGameControl::MSG_PAIMAI_GOOD_INFO;
	m_callback[SC_SUBMIT_PAIMAI_RESULT] =           &CGameControl::MSG_PAIMAI_RESULT;
	m_callback[SC_PAIMAI_SALE_RESULT] =             &CGameControl::MSG_PAIMAI_SALE_RESULT;
	m_callback[SC_OBJECT_EXTERN_INFO] =             &CGameControl::MSG_Object_Extern_Info;
	m_callback[SC_PT_CHANGSHENGXUANBIN] =           &CGameControl::MSG_ChangshengXuanbin;
	m_callback[SC_PT_MESSAGE_INFO] =                &CGameControl::MSG_Extern_Messgebox_info;
	m_callback[SC_SERVER_SWITCH] =                  &CGameControl::MSG_Server_Switch;
	m_callback[SC_RANGE_SIGN_UP_RESULT] =           &CGameControl::MSG_Range_Sign_Up_Result;
	m_callback[SC_RANGE_REQUEST_RESULT] =           &CGameControl::MSG_Range_Request_Result;
	m_callback[SC_RANGE_FINDPLACE_RESULT] =         &CGameControl::MSG_Range_FindOtherPlace_Result;
	m_callback[SC_RANGE_FRIEND_RESULT] =            &CGameControl::MSG_Range_Friend_Result;
    m_callback[SC_GOOD_INFO_ANSWER] =               &CGameControl::MSG_Good_Info_Answer;
	m_callback[SC_SET_USE_OBJECT_PARA] =            &CGameControl::MSG_Set_Use_Object_Para;
	m_callback[SC_FENSHEN_TAKE_OBJECT]		 =      &CGameControl::MSG_FengShengTakeObject;
    m_callback[SC_FLASH_VOICE_BTN]           =      &CGameControl::MSG_Flash_Voice_Btn;
    m_callback[SC_PET_MERGE]                 =      &CGameControl::MSG_PET_MERGE;
	m_callback[SC_MAZE_INFO]				 =		&CGameControl::MSG_MAZE_INFO;
	m_callback[SC_ZHENBAO_INFO]              =      &CGameControl::MSG_ZHENBAO_INFO;
	m_callback[SC_GUILD_INI_INFO]			 =		&CGameControl::Msg_Guild_Ini_Info;
	m_callback[SC_GUILD_OFF_APPDISS_RESP]	 =		&CGameControl::Msg_Guild_Off_Appoint_Dis;
	m_callback[SC_GUILD_APPLY_TAEL]			 =		&CGameControl::Msg_Guild_Online_Tael;
	m_callback[SC_GUILD_CHANGE_TILE]		 =		&CGameControl::Msg_Guild_Change_Tile;
	m_callback[SC_GUILD_RESOURCE_CONTRIBUTE] =		&CGameControl::Msg_Guild_Resource_Owner;
	m_callback[SC_GUILD_PERSON_TAEL]		 =		&CGameControl::Msg_Guild_Person_Tael;
	m_callback[SC_SHENGONG_FANG]			 =      &CGameControl::MSG_SHENGONGFANG_INFO;
	m_callback[SC_GUILD_STATUS_SWITCH]		 =		&CGameControl::Msg_Guild_Status_Switch;
	m_callback[SC_GUILD_DEVOTERES_RESULT]	 =		&CGameControl::Msg_Guild_DevoteResource_Result;
	m_callback[SC_SPIRT_INFO]                =      &CGameControl::MSG_BOX_INFO;
	m_callback[SC_GUILD_ONOROFF_LINE]		 =		&CGameControl::Msg_Guild_Member_OnOffLine;
	m_callback[SC_GUILD_ASSIGN_TAEL]		 =		&CGameControl::Msg_Guild_Change_Tael;
	m_callback[SC_STELE_PART_LIST]			 =		&CGameControl::MSG_Guild_Stele_Parter;
	m_callback[SC_STELE_REGISTER]			 =		&CGameControl::MSG_Guild_Stele_Register_Result;
	m_callback[SC_STELE_HERO]				 =		&CGameControl::MSG_Guild_Stele_HeroNum;
	m_callback[SC_STELE_BANI]				 =		&CGameControl::MSG_Guild_Stele_Hero_Result;
	m_callback[SC_ASSESS_STELE_LEVEL]		 =		&CGameControl::MSG_Guild_Stele_Level;
	m_callback[SC_GUILD_PHYLE_LIST]			 =		&CGameControl::MSG_Guild_Phyle_List;
	m_callback[SC_GUILD_PHYLE_STATE_MODIFY]  =		&CGameControl::MSG_Phyle_Guild_AddOrExit;
	m_callback[SC_PHYLE_RELATION]			 =		&CGameControl::MSG_Phyle_AddOrExit;
	m_callback[SC_GENERAL_REPLY_CONFIRM]	 =		&CGameControl::MSG_General_Reply_Confirm;
	m_callback[SC_GUILD_PHYLE_STATE]		 =		&CGameControl::MSG_Guild_Phyle_State;
	m_callback[SC_GENERAL_CHAT_MSG]			 =		&CGameControl::MSG_Message_General;
	//m_callback[SC_SELCHAR_CHARINFO]		     =		&CGameControl::MSG_Selchar_CharInfo;
	m_callback[SC_GENERAL_OPERATE_RESULT]	 =		&CGameControl::MSG_General_Operate_Result;
	m_callback[SC_DISASSEMBLE_GOODS]         =      &CGameControl::MSG_DISASSEMBLE_GOODS;
	m_callback[SC_COMBINE_YUANSHI]           =      &CGameControl::MSG_COMBINE_YUANSHI;
	m_callback[SC_JINGLI_VALUE]              =      &CGameControl::MSG_JINGLI_VALUE;
	//m_callback[SC_LIANHUA_VALUE]             =      &CGameControl::MSG_LIANHUA_VALUE;
	//m_callback[SC_LIANHUA_LEVEL]             =      &CGameControl::MSG_LIANHUA_LEVEL;
	m_callback[SC_ELIXIR_INFO]				 =		&CGameControl::MSG_Elixir_Info;
	//m_callback[SC_RELIVE_DEMON]              =      &CGameControl::Msg_Relive_Demon;
	m_callback[SC_SUCCESS_PROBABILTY]        =      &CGameControl::Msg_Success_Probability;
	m_callback[SC_NPC_LOOK_CHANGE]           =      &CGameControl::MSG_NPC_LOOK_CHANGE;
	m_callback[SC_WARM_HEART_PROMPT_INFO]	 =		&CGameControl::Msg_Player_Prompt_Info;
	m_callback[SC_INSTANCEZONE_POPUP]        =      &CGameControl::MSG_INSTANCEZONE_POPUP;
	m_callback[SC_YUANBAOSELL_INFO]			 =      &CGameControl::MSG_YuanbaoSell_Info;
	m_callback[SC_YUANBAOQUERY]              =      &CGameControl::Msg_MyYuanBao_Query;
	m_callback[SC_BAOJIAN_SELECT]            =      &CGameControl::MSG_SELECT_BAOJIAN;

// 	m_callback[SC_TRUSTEESHIP_ARRAY]         =      &CGameControl::MSG_TneupMember;
// 	m_callback[SC_TRUSTEESHIP]               =      &CGameControl::MSG_Trustship;
// 	m_callback[SC_TRUSTEESHIP_SWITCH]        =      &CGameControl::MSG_TrusteeshipSwitch;
// 	m_callback[SC_TRUSTEESHIP_MODE]          =      &CGameControl::MSG_TrusteeshipMode;
// 	m_callback[SC_TRUSTEESHIP_HEAD_TARGET]   =      &CGameControl::MSG_TrusteeshipHeadAttackTarget;
// 	m_callback[SC_TRUSTEESHIP_HEAD_MODE]     =      &CGameControl::MSG_TrusteeshipHeadMode;
// 	m_callback[SC_TRUSTEESHIP_HEAD_POS]      =      &CGameControl::MSG_TrusteeshipHeadPos;
// 	m_callback[SC_TRUSTEESHIP_INVITE_ASK]    =      &CGameControl::MSG_TrusteeshipAsk;
// 	m_callback[SC_TRUSTEESHIP_NEEDJOB]       =      &CGameControl::MSG_Trusteeship_NeedJob;
// 	m_callback[SC_TRUSTEESHIP_HPMP]          =      &CGameControl::MSG_Trusteeship_HPMP; 
// 	m_callback[SC_QUNYING_MEMBER]            =      &CGameControl::MSG_Refresh_QunYing_Members;
// 	m_callback[SC_QUNYING_REPORT]            =      &CGameControl::MSG_QunYing_Reports;

	m_callback[SC_MERITORIOUSNESS_UPDATE]    =      &CGameControl::Msg_Meritoriousness_Update; 
	m_callback[SC_LEASEBOOTH_STATE]          =      &CGameControl::Msg_LeaseBooth_State;
	m_callback[SC_LEASEBOOTH_RESPONSE]       =      &CGameControl::Msg_LeaseBooth_Response;
	m_callback[SC_LEASEBOOTH_STARTRESPONSE]  =      &CGameControl::Msg_LeaseBooth_Start;
	m_callback[SC_LEASEBOOTH_BUYOTHERSRESPONSE]=    &CGameControl::Msg_LeaseBooth_BuyOthers;
	m_callback[SC_LEASEBOOTH_TAKEBACKMONEYRESPONSE]=&CGameControl::Msg_LeaseBooth_TakeMoneyBackResponse;
	m_callback[SC_LEASEBOOTH_OVERDUETIME]    =		&CGameControl::Msg_LeaseBooth_OverdueTime;
	m_callback[SC_FIRM_SHOWPURCHASEITEM]     =		&CGameControl::Msg_GetFirmItemList;
	m_callback[SC_FIRM_CANPURCHASEITEM]      =		&CGameControl::Msg_SelectFirmItem;
	m_callback[SC_FIRM_SENDPURCHASEITEMINFO] =		&CGameControl::Msg_PurchaseItem;

	m_callback[SC_CUSTOM_PIAOHONG]			 =		&CGameControl::MSG_Custom_Piaohong;

	m_callback[SC_SWEAR_LIST]				 =		&CGameControl::MSG_Swear_List;
	m_callback[SC_PLAYERJOINORQUIT_SWEAR]	 =		&CGameControl::MSG_AddDel_Swear_Success;
	m_callback[SC_SWEAR_ADD_CONFIRM]		 =		&CGameControl::MSG_Swear_Request;
	m_callback[SC_SWEAR_ERROR]				 =		&CGameControl::MSG_Swear_Error;

	m_callback[SC_WENPEI_LIST]				 =		&CGameControl::MSG_WenPei_List;

	m_callback[SC_NOTIFY_ADVENTURE_STATUS]	 =		&CGameControl::MSG_Notify_Adventure_Status;
	m_callback[SC_NOTIFY_ADVENTURE_REWARD]	 =		&CGameControl::MSG_Notify_Adventure_Reward;

	m_callback[SC_CONSKILLBUFF]				 =		&CGameControl::MSG_Player_ConSkillBuff;	

	m_callback[SC_NOTIFY_SANWEIZHENHUOINFO]	 =		&CGameControl::MSG_SANWEIZHENHUOINFO;
	m_callback[SC_NPC_LINGWUSANWEIZHENHUO]	 =		&CGameControl::MSG_NPC_LINGWUSANWEIZHENHUO;	

	m_callback[SC_NOTICEIE]					 =		&CGameControl::MSG_NoticeIE;

	m_callback[SC_FIREARTIFICE]				 =		&CGameControl::MSG_FireArtifice;
	m_callback[SC_FIREARTIFICETIME]			 =		&CGameControl::MSG_FireArtificeTime;

	m_callback[SC_NEW_STORAGE_INFO]			 =		&CGameControl::MSG_New_Storage_Info;			//新仓库协议
	m_callback[SC_FABAO_ATTACK]				 =		&CGameControl::MSG_FABAOATTACK;
	m_callback[SC_FABAO_ZHUANGBEI]			 =		&CGameControl::MSG_ZHUANGBEIFABAO;
	m_callback[SC_FABAO_XILING]				 =		&CGameControl::MSG_FABAOAXILING;

	m_callback[SC_BINGINFO]					 =		&CGameControl::MSG_BingInfo;
	m_callback[SC_OPEN_BINGCONTROLWND]		 =		&CGameControl::MSG_OpenBingCtrlWnd;

	//m_callback[SC_YIHUO_REQUEST]			 =		&CGameControl::MSG_YIHUO_REQUEST;

	m_callback[SC_FIRE_SHIELD]				 =		&CGameControl::MSG_Fire_Shield;
	m_callback[SC_DAOZUN_JIANGLIN]			 =		&CGameControl::MSG_DaoZun_JiangLin;
	m_callback[SC_SHADOWKILL8]				 =		&CGameControl::MSG_ShadowKill8;
	m_callback[SC_OPEN_YIHUOZHENGDUOWND]	 =		&CGameControl::MSG_YiHuoZhanMsg;

	m_callback[SC_LIANJI_SHOW]				 =		&CGameControl::MSG_LianJi_Show;

	m_callback[SC_EQUIP_CHAIFEN]			 =		&CGameControl::MSG_Equip_Chaifen;

	m_callback[SC_PLAYER_HORSEMANSHIP_WATCH] =		&CGameControl::MSG_HorseManShip_Watch;

	m_callback[SC_MONSTER_HDNW_LOCK] =              &CGameControl::MSG_Monster_Hdnw_Lock;
	
	m_callback[SC_PLAYER_HORSEMANSHIP_INFO]	 =		&CGameControl::MSG_PLAYER_HORSEMANSHIP_INFO;
	m_callback[SC_PLAYER_HORSEMANSHIP_ACTIONPOINT]=	&CGameControl::MSG_HorseManShip_ActionPoint;

	m_dwJumpTime = 0;
}

CGameControl::~CGameControl(void)
{
	ClearAllRangeData();	//删除所有排名数据
}

void CGameControl::GCL_ClearMsgBuf()
{
	m_msgs.clear(); //清除掉所有的协议
}

bool CGameControl::GCL_AddMsg(const char * pBuf,int iLen)
{
	SMsgBuf msg;

	int i = 0;
	if(m_msgs.size() > 0)
	{
		msg = m_msgs.back();
		i = msg.iPos + msg.iLen;
	}

	//溢出了
	if((i+iLen+1) >= MAX_MESSAGE_BUFFER)
		return false;

	msg.iPos = i;
	msg.iLen = iLen;
	m_msgs.push_back(msg);

	memcpy(&m_cMsgBuf[i],pBuf,iLen);

	return true;
}

void CGameControl::GCL_Run()
{
	for(int i = 0;i < m_msgs.size();i++)
	{
//#ifdef ENABLE_REPLAY
//		if (g_ReplayManager.IsInRecord() && g_pGfx)
//		{
//			g_ReplayManager.SaveMsg(g_pGfx->GetFrameCount(),m_msgs[i].iLen,&m_cMsgBuf[m_msgs[i].iPos]);
//		}
//#endif

		GCL_DoMsg(&m_cMsgBuf[m_msgs[i].iPos],m_msgs[i].iLen);
	}
	m_msgs.clear();
}

void CGameControl::GCL_DoMsg(const char * strMsg,int iLen)
{
	WORD wMsgID = Conv_WORD(strMsg);
	int iStart = 2;
	if (GS_CLI_OLD_MSG == wMsgID)
	{
		//处理不同的网络消息
		if(iLen == 6 )
		{
			WORD wErr = Conv_WORD(strMsg + 2);
			WORD wServer = Conv_WORD(strMsg + 4);

			switch( wErr )
			{
			case SOCK_ERROR_CONNECTFAIL:
				break;
			case SOCK_ERROR_DISCONNECTED:
				break;
			}
			return;
		}
		
		wMsgID = Conv_WORD(strMsg + 2 + 4);
	}


	//处理消息
	map<WORD,lpFNMSGProc>::iterator itr = m_callback.find(wMsgID);
	if(itr != m_callback.end())
	{
// 		CCharacter *pOldSelf = g_pSelf;
// 		bool bOldEnableMagic = g_Config.IsEnableMagic();
// 		bool bEnablePushAction = g_Config.IsEnablePushAction();
//
// 		if (wMsgID == SC_MICROCONTROL_RECEIVEPROTOCOL)
// 		{
// 			DWORD dwID = Conv_DWORD(strMsg + 2);
// 			TneupMember* pMember = g_TrusteeshipData.FindMemberByID(dwID);
// 			if (!pMember)
// 			{
// 				return;
// 			}
// 
// 			g_pSelf = pMember->pCharacter;
// 
// 			
// 			if (!g_pSelf)
// 			{
// 				g_pSelf = pOldSelf;
// 				g_Config.SetEnableMagic(bOldEnableMagic);
// 				g_Config.SetEnablePushAction(bEnablePushAction);
// 				return;
// 			}
// 
// 			if(g_pSelf != g_TrusteeshipData.GetMicroControledCharacter())//微操模式下对于非微操对象收到的协议产生的魔法都不应该创建,否则会看到两个魔法,因为其它人使用魔法微操对象和自己都会收到同样的协议,而且不创建动作,不处理动作,就像传世的元神
// 			{
// 				g_Config.SetEnableMagic(false);
// 				g_Config.SetEnablePushAction(false);
// 			}
// 		}
// 		else
// 		{
// 			g_pSelf = &ORIGINALSELF;
// 			if (g_TrusteeshipData.GetMicroControlPos() >= 0)//微操模式下对于非微操对象收到的协议产生的魔法都不应该创建,否则会看到两个魔法,因为其它人使用魔法微操对象和自己都会收到同样的协议,而且不创建动作,不处理动作,就像传世的元神
// 			{
// 				g_Config.SetEnableMagic(false);
// 				g_Config.SetEnablePushAction(false);
// 			}
// 		}


		TRY_BEGIN;
        (this->*itr->second)(strMsg+iStart,iLen - iStart);

        TRY_END_DO(
			strAddMsg = "协议内容:[";
		    strAddMsg +=  HexDump((const BYTE *)strMsg,iLen,"") + "]";
		);

// 		g_pSelf = pOldSelf;
// 		g_Config.SetEnableMagic(bOldEnableMagic);
// 		g_Config.SetEnablePushAction(bEnablePushAction);
	}
	else
	{
//#ifdef _DEBUG
		output_debug("%s(%d): 没处理的网络协议(0x%04x)\n",__FILE__,__LINE__,wMsgID);
//#endif
	}
}

void CGameControl::GCL_AddErr()
{
	char strMsgBody[CMD_SIZE] = {0};
	Conv_WORD(strMsgBody) = GS_CLI_OLD_MSG;
	Conv_WORD(strMsgBody + 6) = CONNECT_ERROR;
	GCL_AddMsg(strMsgBody,CMD_SIZE);
}


void CGameControl::DoOverMapFindPath()
{
	if (!g_MapFinder.IsWalking())
		return;

	//如果正在跨地图自动寻路
	VPJumpPoint& vJump = g_MapFinder.GetPath();
	if (vJump.size() > 0)
	{
		if(stricmp((*vJump.begin())->strDesMN.c_str(),g_pGameMap->GetMapName()) != 0)
		{
			g_MapFinder.SetWalking(false);
			vJump.clear();				
		}
		else
		{
			vJump.erase(vJump.begin());
		}
	}

	if (g_MapFinder.IsWalking())
	{
		SELF.CleanOppBlock();
		g_MapFinder.DoNextStep();
	}

	//寻路时跨地图,地图按钮闪烁
	g_pControl->MsgToWnd(MSG_CTRL_HANDYMAP_WND,MSG_CTRL_FLASH_BTN_WND,2);
}

bool CGameControl::DealGotoCommand(const std::string & strCommand)
{
	if(strCommand.empty()) return false;
	
	bool bBetweenMap = ((SS_SEARCH_BET_MAP & g_dwServerSwitch) == 0);
	//up 音乐
	g_pAudio->Play(EAT_OTHER,5,g_pAudio->GetRand()++);

	if(strCommand.substr(0,6) == "@@goto")
	{
		std::string strTemp = strCommand.substr(2,strCommand.length() - 2);
		int iPos1 = strTemp.find(" ",0);
		int iPos2 = strTemp.find(" ",iPos1+1);
		std::string strMapName = strTemp.substr(iPos1+1,iPos2-iPos1-1);

		iPos1 = strTemp.find(" ",iPos2+1);
		std::string strNpcName = strTemp.substr(iPos2+1,iPos1-iPos2-1);

		iPos2 = strTemp.find(",",iPos1+1);
		int iX = atoi(strTemp.substr(iPos1+1,iPos2-iPos1-1).c_str());
		int iY = atoi(strTemp.substr(iPos2+1,strTemp.length()-iPos2-1).c_str());
		
		if(bBetweenMap && stricmp(g_pGameMap->GetMapName(),strMapName.c_str()) != 0)
		{
			bool bSucess = g_MapFinder.AutoLookforPath(g_pGameMap->GetMapName(),0,0,strMapName.c_str(),iX,iY,true);
			if (!bSucess)
			{
				char ctemp[256] = {0};
				string str = "";
				StringUtil::toUpperCase(strMapName);
				g_ItemCfgMgr.ParseInfo("MapNpc",strMapName.c_str(),str);
				if (str.empty())
					sprintf(ctemp,"%s","该NPC不在本地图上，无法自动寻路到该NPC处。");
				else sprintf(ctemp,"该NPC不在本地图上，%s",str.c_str());

				g_MsgBoxMgr.PopSimpleAlert(ctemp,0,0);
			}
		}
		else
		{
			g_pGameControl->DealGotoNpcCommand(strCommand);
		}
		return true;
	}
	else if(strCommand.substr(0,9) == "@@goplace")
	{
		std::string strTemp = strCommand.substr(2,strCommand.length() - 2);
		int iPos1 = strTemp.find(" ",0);
		int iPos2 = strTemp.find(" ",iPos1+1);
		std::string strMapName = strTemp.substr(iPos1+1,iPos2-iPos1-1);

		iPos1 = strTemp.find(",",iPos2+1);
		int iX = atoi(strTemp.substr(iPos2+1,iPos1-iPos2-1).c_str());
		int iY = atoi(strTemp.substr(iPos1+1,strTemp.length()-iPos1-1).c_str());

		if(bBetweenMap && stricmp(g_pGameMap->GetMapName(),strMapName.c_str()) != 0)
		{
			bool bSucess = g_MapFinder.AutoLookforPath(g_pGameMap->GetMapName(),0,0,strMapName.c_str(),iX,iY,false);
			if (!bSucess)
			{
				char ctemp[256] = {0};
				string str = "";
				StringUtil::toUpperCase(strMapName);
				g_ItemCfgMgr.ParseInfo("MapNpc",strMapName.c_str(),str);
				if (str.empty())
					sprintf(ctemp,"%s","目标不在本地图上，无法自动寻路到该处。");
				else sprintf(ctemp,"目标不在本地图上，%s",str.c_str());

				g_MsgBoxMgr.PopSimpleAlert(ctemp,0,0);
			}
		}
		else
		{
			g_pGameControl->DealGoPlaceCommand(strCommand);
		}

		return true;
	}

	return false;
}

bool CGameControl::DealGotoNpcCommand(const std::string & strCommand)
{
	if(SELF.IsDead() || SELF.GetBoothState())
		return false;
	//<老兵/@@goto mapName npcName 20,20>\\";
	//strCommand = @@goto mapName npcName 20,20
	TRY_BEGIN
		std::string strTemp = strCommand.substr(2,strCommand.length() - 2);
		int iPos1 = strTemp.find(" ",0);
		int iPos2 = strTemp.find(" ",iPos1+1);
		std::string strMapName = strTemp.substr(iPos1+1,iPos2-iPos1-1);

		iPos1 = strTemp.find(" ",iPos2+1);
		std::string strNpcName = strTemp.substr(iPos2+1,iPos1-iPos2-1);

		iPos2 = strTemp.find(",",iPos1+1);
		int iX = atoi(strTemp.substr(iPos1+1,iPos2-iPos1-1).c_str());
		int iY = atoi(strTemp.substr(iPos2+1,strTemp.length()-iPos2-1).c_str());

		if(stricmp(g_pGameMap->GetMapName(),strMapName.c_str()) != 0)
		{
			char ctemp[256] = {0};
			string str = "";
			StringUtil::toUpperCase(strMapName);
			g_ItemCfgMgr.ParseInfo("MapNpc",strMapName.c_str(),str);
			if (str.empty())
				 sprintf(ctemp,"%s","该NPC不在本地图上，无法自动寻路到该NPC处。");
			else sprintf(ctemp,"该NPC不在本地图上，%s",str.c_str());

			g_MsgBoxMgr.PopSimpleAlert(ctemp,0,0);
			return false;
		}

		//找一个对玩家来说比较合适的点寻路过去
		int iSelfX,iSelfY;
		SELF.GetXY(iSelfX,iSelfY);

		int iLeft = 1,iRight = 5,iTop = 1,iBottom = 5;
		int iPlusX = 1,iPlusY = 1;//往哪个方向找iPlusX<0向左，iPlusY<0向上
		if(iSelfX < iX)
			iPlusX = -1;
		else if(iSelfX == iX)
			iLeft = 0;

		if(iSelfY < iY)
			iPlusY = -1;
		else if(iSelfY == iY)
			iTop = 0;

		if(abs(iSelfX - iX) <= 3 && abs(iSelfY - iY) <= 3)//直接点NPC
		{
			CSimpleCharacter * pChar = MapArray.FindSimpleCharacter(iX,iY);
			if (pChar)
			{
				g_pGameMgr->OnClickChar(pChar->GetID());
			}

			return true;
		}

		bool bFindSpace = false;
		bool bFindOtherSideX = false,bFindOtherSideY = false;//是否查找了自己另外一边的x,y

		for(int i = iLeft*iPlusX; i*iPlusX <= iRight && !bFindSpace; i = i+iPlusX)
		{
			for(int j = iTop*iPlusY; j*iPlusY <= iBottom; j += iPlusY)
			{
				if (!SELF.GetMapPathFinder().IsBlock(iX+i,iY+j))
				{
					if(iSelfX == iX + i && iSelfY == iY + j)//自己已经是在这个位置了直接点NPC
					{
						CSimpleCharacter * pChar = MapArray.FindSimpleCharacter(iX,iY);
						if (pChar)
						{
							g_pGameMgr->OnClickChar(pChar->GetID());
						}
						
						return true;
					}
					else
					{
						if(g_pGameMgr->AI_AutoWalk(iX + i,iY +j))
						{
							g_OtherData.SetFindPathReason(1);//要放在SearchPathLink之后,否则可能会被SearchPathLink给清除掉
							g_OtherData.SetFindPathClickNpcXY(iX,iY);
						}
					}

					bFindSpace = true;
					break;
				}
			}

			if(i*iPlusX >= iRight)
			{
				if(!bFindOtherSideY)//和玩家同一侧Y方向没有找到，到相反的Y方向去找
				{
					iPlusY *= -1;
					if(iTop != 0)
						iTop = 0;//和目标点同一Ｙ值的也要找
					else 
						iTop = 1;//已经找过了不再重复找

					bFindOtherSideY = true;
					i = iLeft*iPlusX;//y换到了另外一侧，x重新再来
				}
				else if(!bFindOtherSideX)//和玩家同一侧X方向没有找到，到相反的X方向去找
				{
					iPlusX *= -1;
					iPlusY *= -1;
					if(iLeft != 0)
						iLeft = 0;//和目标点同一X值的也要找
					else 
						iLeft = 1;//已经找过了不再重复找

					bFindOtherSideX = true;
					bFindOtherSideY = false;
					i = iLeft*iPlusX;//x,y换到了另外一边，x重新再来
				}

			}		

		}

		if(!bFindSpace)
		{
			g_MsgBoxMgr.PopSimpleComfirm("该位置无法自动寻路，或传送点被怪物阻挡。",0,0);
			return false;
		}			

		return true;
	TRY_END_RETURN(false)
return false;
}

bool CGameControl::DealGotoNpc(const char* pszMapName,WORD iX,WORD iY)
{
	//找一个对玩家来说比较合适的点寻路过去
	if(stricmp(g_pGameMap->GetMapName(),pszMapName) != 0)
		return false;

	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	int iLeft = 1,iRight = 5,iTop = 1,iBottom = 5;
	int iPlusX = 1,iPlusY = 1;//往哪个方向找iPlusX<0向左，iPlusY<0向上
	if(iSelfX < iX)
		iPlusX = -1;
	else if(iSelfX == iX)
		iLeft = 0;

	if(iSelfY < iY)
		iPlusY = -1;
	else if(iSelfY == iY)
		iTop = 0;

	TRY_BEGIN

	if(abs(iSelfX - iX) <= 3 && abs(iSelfY - iY) <= 3)//直接点NPC
	{
		CSimpleCharacter * pChar = MapArray.FindSimpleCharacter(iX,iY);
		if (pChar)
		{
			g_pGameMgr->OnClickChar(pChar->GetID());
		}

		return true;
	}

	bool bFindSpace = false;
	bool bFindOtherSideX = false,bFindOtherSideY = false;//是否查找了自己另外一边的x,y

	for(int i = iLeft*iPlusX; i*iPlusX <= iRight && !bFindSpace; i = i+iPlusX)
	{
		for(int j = iTop*iPlusY; j*iPlusY <= iBottom; j += iPlusY)
		{
			if (!SELF.GetMapPathFinder().IsBlock(iX+i,iY+j))
			{
				if(iSelfX == iX + i && iSelfY == iY + j)//自己已经是在这个位置了直接点NPC
				{
					CSimpleCharacter * pChar = MapArray.FindSimpleCharacter(iX,iY);
					if (pChar)
					{
						g_pGameMgr->OnClickChar(pChar->GetID());
					}

					return true;
				}
				else
				{
					if(g_pGameMgr->AI_AutoWalk(iX + i,iY +j))
					{
						g_OtherData.SetFindPathReason(1);//要放在SearchPathLink之后,否则可能会被SearchPathLink给清除掉
						g_OtherData.SetFindPathClickNpcXY(iX,iY);
					}
				}

				bFindSpace = true;
				break;
			}
		}

		if(i*iPlusX >= iRight)
		{
			if(!bFindOtherSideY)//和玩家同一侧Y方向没有找到，到相反的Y方向去找
			{
				iPlusY *= -1;
				if(iTop != 0)
					iTop = 0;//和目标点同一Ｙ值的也要找
				else 
					iTop = 1;//已经找过了不再重复找

				bFindOtherSideY = true;
				i = iLeft*iPlusX;//y换到了另外一侧，x重新再来
			}
			else if(!bFindOtherSideX)//和玩家同一侧X方向没有找到，到相反的X方向去找
			{
				iPlusX *= -1;
				iPlusY *= -1;
				if(iLeft != 0)
					iLeft = 0;//和目标点同一X值的也要找
				else 
					iLeft = 1;//已经找过了不再重复找

				bFindOtherSideX = true;
				bFindOtherSideY = false;
				i = iLeft*iPlusX;//x,y换到了另外一边，x重新再来
			}

		}		

	}

	if(!bFindSpace)
	{
		g_MsgBoxMgr.PopSimpleComfirm("该位置无法自动寻路，或传送点被怪物阻挡。",0,0);
		return false;
	}			

	return true;
	TRY_END_RETURN(false)
		return false;
}

bool CGameControl::DealGotoPlace(const char* pszMapName,WORD iX,WORD iY)
{
	TRY_BEGIN
	if(strcmp(g_pGameMap->GetMapName(),pszMapName) != 0)
		return false;	

	//找一个对玩家来说比较合适的点寻路过去
	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	int iLeft = 0,iRight = 5,iTop = 0,iBottom = 5;
	int iPlusX = 1,iPlusY = 1;//往哪个方向找iPlusX<0向左，iPlusY<0向上
	if(iSelfX < iX)
		iPlusX = -1;

	if(iSelfY < iY)
		iPlusY = -1;

	bool bFindSpace = false;
	bool bFindOtherSideX = false,bFindOtherSideY = false;//是否查找了自己另外一边的x,y

	for(int i = iLeft*iPlusX; i*iPlusX <= iRight && !bFindSpace; i = i+iPlusX)
	{
		for(int j = iTop*iPlusY; j*iPlusY <= iBottom; j += iPlusY)
		{
			if (!SELF.GetMapPathFinder().IsBlock(iX+i,iY+j))
			{
				if(iSelfX == iX + i && iSelfY == iY + j)//自己已经是在这个位置了直接点返回
				{
					return true;
				}
				else
				{
					g_pGameMgr->AI_AutoWalk(iX + i,iY +j);
					g_OtherData.SetFindPathReason(0);
				}

				bFindSpace = true;
				break;
			}
		}

		if(i*iPlusX >= iRight)
		{
			if(!bFindOtherSideY)//和玩家同一侧Y方向没有找到，到相反的Y方向去找
			{
				iPlusY *= -1;
				iTop = 1;//iTop == 0已经找过了不再重复找

				bFindOtherSideY = true;
				i = iLeft*iPlusX;//y换到了另外一侧，x重新再来
			}
			else if(!bFindOtherSideX)//和玩家同一侧X方向没有找到，到相反的X方向去找
			{
				iPlusX *= -1;
				iPlusY *= -1;
				iLeft = 1;//iLeft == 0已经找过了不再重复找

				bFindOtherSideX = true;
				bFindOtherSideY = false;
				i = iLeft*iPlusX;//x,y换到了另外一边，x重新再来
			}
		}
	}

	if(!bFindSpace)
	{
		g_MsgBoxMgr.PopSimpleComfirm("该位置无法自动寻路，或传送点被怪物阻挡。",0,0);
		return false;
	}			

	return true;
	TRY_END_RETURN(false)

	return false;
}

bool CGameControl::DealGoPlaceCommand(const std::string & strCommand)
{
	if(SELF.IsDead() || SELF.GetBoothState())
		return false;
	////<到××处去杀××/@@goplace mapName 20,20>";
	////strCommand = @@goplace mapName 20,20
	TRY_BEGIN
		std::string strTemp = strCommand.substr(2,strCommand.length() - 2);
		int iPos1 = strTemp.find(" ",0);
		int iPos2 = strTemp.find(" ",iPos1+1);
		std::string strMapName = strTemp.substr(iPos1+1,iPos2-iPos1-1);

		iPos1 = strTemp.find(",",iPos2+1);
		int iX = atoi(strTemp.substr(iPos2+1,iPos1-iPos2-1).c_str());
		int iY = atoi(strTemp.substr(iPos1+1,strTemp.length()-iPos1-1).c_str());

		if(strcmp(g_pGameMap->GetMapName(),strMapName.c_str()) != 0)
		{
			char ctemp[256] = {0};
			string str = "";
			StringUtil::toUpperCase(strMapName);
			g_ItemCfgMgr.ParseInfo("MapNpc",strMapName.c_str(),str);
			if (str.empty())
				sprintf(ctemp,"%s","目标不在本地图上，无法自动寻路到该处。");
			else sprintf(ctemp,"目标不在本地图上，%s",str.c_str());

			g_MsgBoxMgr.PopSimpleComfirm(ctemp,0,0);
			return false;
		}

		//找一个对玩家来说比较合适的点寻路过去
		int iSelfX,iSelfY;
		SELF.GetXY(iSelfX,iSelfY);

		int iLeft = 0,iRight = 5,iTop = 0,iBottom = 5;
		int iPlusX = 1,iPlusY = 1;//往哪个方向找iPlusX<0向左，iPlusY<0向上
		if(iSelfX < iX)
			iPlusX = -1;

		if(iSelfY < iY)
			iPlusY = -1;

		bool bFindSpace = false;
		bool bFindOtherSideX = false,bFindOtherSideY = false;//是否查找了自己另外一边的x,y

		for(int i = iLeft*iPlusX; i*iPlusX <= iRight && !bFindSpace; i = i+iPlusX)
		{
			for(int j = iTop*iPlusY; j*iPlusY <= iBottom; j += iPlusY)
			{
				if (!SELF.GetMapPathFinder().IsBlock(iX+i,iY+j))
				{
					if(iSelfX == iX + i && iSelfY == iY + j)//自己已经是在这个位置了直接点返回
					{
						return true;
					}
					else
					{
						g_pGameMgr->AI_AutoWalk(iX + i,iY +j);
						g_OtherData.SetFindPathReason(0);
					}

					bFindSpace = true;
					break;
				}
			}

			if(i*iPlusX >= iRight)
			{
				if(!bFindOtherSideY)//和玩家同一侧Y方向没有找到，到相反的Y方向去找
				{
					iPlusY *= -1;
					iTop = 1;//iTop == 0已经找过了不再重复找

					bFindOtherSideY = true;
					i = iLeft*iPlusX;//y换到了另外一侧，x重新再来
				}
				else if(!bFindOtherSideX)//和玩家同一侧X方向没有找到，到相反的X方向去找
				{
					iPlusX *= -1;
					iPlusY *= -1;
					iLeft = 1;//iLeft == 0已经找过了不再重复找

					bFindOtherSideX = true;
					bFindOtherSideY = false;
					i = iLeft*iPlusX;//x,y换到了另外一边，x重新再来
				}
			}
		}

		if(!bFindSpace)
		{
			g_MsgBoxMgr.PopSimpleComfirm("该位置无法自动寻路，或传送点被怪物阻挡。",0,0);
			return false;
		}			

		return true;
TRY_END_RETURN(false)

return false;
}

bool CGameControl::DealGotoCommand(JumpPoint* pJump)
{
	if(SELF.IsDead() || SELF.GetBoothState() || !pJump)
		return false;
	TRY_BEGIN
	int iX = pJump->wSrcX;
	int iY = pJump->wSrcY;

	if(stricmp(g_pGameMap->GetMapName(),pJump->strSrcMN.c_str()) != 0)
	{
		char ctemp[256] = {0};
		string str = "";
		string str1 = pJump->strSrcMN;
		StringUtil::toUpperCase(str1);
		g_ItemCfgMgr.ParseInfo("MapNpc",str1.c_str(),str);
		if (str.empty())
			sprintf(ctemp,"%s","该NPC不在本地图上，无法自动寻路到该NPC处。");
		else sprintf(ctemp,"该NPC不在本地图上，%s",str.c_str());

		g_MsgBoxMgr.PopSimpleAlert(ctemp,0,0);
		return false;
	}

	//找一个对玩家来说比较合适的点寻路过去
	int iSelfX,iSelfY;
	SELF.GetXY(iSelfX,iSelfY);

	int iLeft = 1,iRight = 5,iTop = 1,iBottom = 5;
	int iPlusX = 1,iPlusY = 1;//往哪个方向找iPlusX<0向左，iPlusY<0向上
	if(iSelfX < iX)
		iPlusX = -1;
	else if(iSelfX == iX)
		iLeft = 0;

	if(iSelfY < iY)
		iPlusY = -1;
	else if(iSelfY == iY)
		iTop = 0;

	bool bFindSpace = false;
	bool bFindOtherSideX = false,bFindOtherSideY = false;//是否查找了自己另外一边的x,y

	if (pJump->bNpc)
	{
		for(int i = iLeft*iPlusX; i*iPlusX <= iRight && !bFindSpace; i = i+iPlusX)
		{
			for(int j = iTop*iPlusY; j*iPlusY <= iBottom; j += iPlusY)
			{
				if (!SELF.GetMapPathFinder().IsBlock(iX+i,iY+j))
				{
					if(g_pGameMgr->AI_AutoWalk(iX + i,iY +j))
					{							
						g_OtherData.SetFindPathReason(1);//要放在SearchPathLink之后,否则可能会被SearchPathLink给清除掉
						g_OtherData.SetFindPathClickNpcXY(iX,iY);							
					}

					bFindSpace = true;
					break;
				}
			}

			if(i*iPlusX >= iRight)
			{
				if(!bFindOtherSideY)////////////////////////和玩家同一侧Y方向没有找到，到相反的Y方向去找
				{
					iPlusY *= -1;
					if(iTop != 0)
						iTop = 0;/////////////////////////和目标点同一Ｙ值的也要找
					else 
						iTop = 1;//已经找过了不再重复找

					bFindOtherSideY = true;
					i = iLeft*iPlusX;//y换到了另外一侧，x重新再来
				}
				else if(!bFindOtherSideX)//和玩家同一侧X方向没有找到，到相反的X方向去找
				{
					iPlusX *= -1;
					iPlusY *= -1;
					if(iLeft != 0)
						iLeft = 0;//和目标点同一X值的也要找
					else 
						iLeft = 1;//已经找过了不再重复找

					bFindOtherSideX = true;
					bFindOtherSideY = false;
					i = iLeft*iPlusX;//x,y换到了另外一边，x重新再来
				}
			}
		}
	}
	else if(g_pGameMgr->AI_AutoWalk(iX,iY))
	{
		bFindSpace = true;
		g_OtherData.SetFindPathReason(2);
	}

	if(!bFindSpace)
	{
		////////////////////////////////////////////////////////////////////////////
		g_MsgBoxMgr.PopSimpleComfirm("该位置无法自动寻路，或传送点被怪物阻挡。",0,0);
		return false;
	}			

	return true;
	TRY_END_RETURN(false)
		return false;
}

bool CGameControl::IsEquipXWSkillSuit() //是否穿着了可以附加技能的圣王套装
{
	CGood &weapon = SELF.GetEquipGood(ITEM_POS_WEAPON);
	if (weapon.GetID() == 0)
	{
		return false;
	}
	if (weapon.GetStdMode() != 5 || weapon.GetShape() != SELF.GetCareer() + 64)
		return false;

	//衣服
	CGood &cloth = SELF.GetEquipGood(ITEM_POS_CLOTH);
	if (cloth.GetID() == 0)
	{
		return false;
	}
	if (cloth.GetStdMode() != SELF.GetSex() + 10 || cloth.GetShape() != SELF.GetCareer() + 25)
		return false;

	return true;
}
