#include "GameControl.h"
#include "Global/Global.h"
#include "Global/GlobalMsg.h"
#include "GameControl.h"
#include "GameData\MagicDefine.h"
#include "BaseClass/Control/Control.h"
#include "GameData\GameData.h"
#include "GameData\ObjectLink\MultiLine.h"
#include "GameData/OtherData.h"
#include "GameData/TalkMgr.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/GameGlobal.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/LoginData.h"
#include "GameData/TalkName.h"
#include "GameMap/GameMap.h"
#include "GameData\MagicCtrlMgr.h"
#include "GameData/ObjectLink/StringLine.h"
#include "GameAI/AIMsgFilter.h"
#include "GameAI/AIAutoMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameAI/AIGoodMgr.h"
#include "GameAI/AIAutoEatMgr.h"
#include "GameAI/PathPublic.h"
#include "GameAI/AIMgr.h"
#include "GameAI/AIConfigMgr.h"
#include "GameAI/AutoKillMonsterMgr.h"
#include "GameMap/MinMap.h"
#include "GameData/GuildData.h"
#include "GameData/FindGood.h"
#include "GameData/DirtyWords.h"
#include "GameData/NpcData.h"
#include "GameData/BoothData.h"
#include "Global/Interface/WeatherInterface.h"
#include "Global/Interface/LightInterface.h"
#include "GameData/ConfigData.h"
#include "GameClient/VoiceAdapter.h"
#include "BaseClass/Control/GoodGrid.h"
#include "WndClass/GameWnd/GameManager.h"
#include "GameData/WooolStoreData.h"
#include "GameAI/AIPromptMgr.h"
#include "GameClient/SDOAInterface.h"
#include "GameData/PetData.h"
#include "GameAI/AIBossCfgMgr.h"
#include "GameAI/AIAutoFightMgr.h"
#include "GameData/PhyleData.h"
#include "BaseClass\Misc\cliggproto.h"
#include "GameData\MapFinder.h"
#include "GameData/TipsCfg.h"
#include "GameData/ProtocolCfg.h"

#include "GameClient/ClientInfoCollect.h"


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

#include "GameData/RangeStruct.h"

#include "BaseClass/Misc/cligsproto.h"
#include "BaseClass/Misc/errdefs.h"

#include "GameClient/SDSamplerMgr.h"


using namespace SGDP;


// 收到的消息处理
void CGameControl::SEND_Messagebox_Reply(DWORD dwNpcID,const char * info,bool bForbidDirtyWord) //消息框的返回消息
{
	SET_COMMAND(CS_NPC_MSGBOX,2048);
	ASSIGN_ID(dwNpcID);
	int iLen = strlen(info);

	string strPlayerName = SELF.GetName();
	if(bForbidDirtyWord && !g_DirtyWords.ParseNameIsGM(strPlayerName))
	{
		if(g_DirtyWords.HaveDirtyWords(info,0xFFFFFFFF & (~0x00000001)))
		{
			g_MsgBoxMgr.PopSimpleAlert("你输入的内容中存在不适当字符,请确认后重新输入");
			return;
		}
	}

	ADD_ARRAY(info,iLen);
	SEND_GAME_SERVER();
}
//'<tab1>asdasdassdsa<tab1end><tab2>asdasdassdsa<tab2end>'
void CGameControl::MSG_Game_Greeting(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg + CMD_SIZE,iLen - CMD_SIZE);
	//buf = "<tab1><df/&asdasdassdsa>\\</df/&fdf>asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa\\asdasdassdsa<tab1end><tab2>asdasdassdsa<tab2end>'";

	SELF.SetDead(false);

	//解析Greeting信息
	g_NPC.SetID(*((DWORD *)msg));
	g_TagTextMgr.GetGreetingData().Clear();
	g_TagTextMgr.GetGreetingDownloadData().Clear();
	g_PhyleData.GetPhyle().clear();//清空宗派数据

	int iStartPos = buf.find("<tab1>");
	int iEndPos = buf.find("<tab1end>");
	if(iStartPos != string::npos && iEndPos != string::npos && iEndPos > iStartPos + 6)
	{
		string str = buf.substr(iStartPos + 6,iEndPos - (iStartPos + 6));
		g_TagTextMgr.GetGreetingData().Parse(str,0,"\\\n");

		FILE * fp = fopen("../uptinfo.txt","wb");
		if(fp)
		{
			fwrite(str.c_str(),str.length(),1,fp);
			fclose(fp);
		}	
	}
	else
	{
		if(iLen - CMD_SIZE > 0)
		{
			string str = buf;
			g_TagTextMgr.GetGreetingData().Parse(str,0,"\\\n");
		}
		else
		{
			FILE * fp = fopen("../uptinfo.txt","rb");
			if(fp)
			{
				long length = 0;
				fseek( fp, 0, SEEK_END );
				length = ftell( fp );
				fseek( fp, 0, SEEK_SET );

				char tmpbuf[2048]={0};
				fread(tmpbuf,min(2040,length),1,fp);

				string str = tmpbuf;
				g_TagTextMgr.GetGreetingData().Parse(str,0,"\\\n");
			}
		}
	}

	iStartPos = buf.find("<tab2>");
	iEndPos = buf.find("<tab2end>");
	if(iStartPos != string::npos && iEndPos != string::npos && iEndPos > iStartPos + 6)
	{
		string str = buf.substr(iStartPos + 6,iEndPos - (iStartPos + 6));
		g_TagTextMgr.GetGreetingDownloadData().Parse(str,0,"\\\n");
	}

	//自动登录时要在放完门的动画后才创建游戏窗口
	if(g_Login.GetAutoLoginInType() > 0)
	{
		g_Login.SetHaveAutoLoginIn(true);
	}
	else
	{
		g_pControl->Msg(MSG_CTRL_GAMEWND,OPER_CREATE);
		g_pControl->PopupWindow(MSG_CTRL_GREETING_MSG_WND,OPER_CREATE);
		// 屏蔽今日活动窗口
		//g_pControl->PopupWindow(MSG_CTRL_TODAYACTIVITY_WND,OPER_CREATE);
	}

	BYTE byType = *((BYTE *)(msg + 6));
	if(byType == 1)//下标7一个字节分别表示组号,8,9一个WORD表示区号
	{
		BYTE byInnerGroupNo = *((BYTE *)(msg + 7));
		WORD wInnerAreaNo = *((WORD *)(msg + 8));
		g_Login.SetInnerAreaNo(wInnerAreaNo);
		g_Login.SetInnerGroupNo(byInnerGroupNo);
	}
	else
	{
		int iAreaNo = g_Login.GetAreaNo();
		g_Login.SetInnerAreaNo(iAreaNo);
		g_Login.SetInnerGroupNo(g_Login.GetGroupNo() + 1);
	}

	g_Login.SetLoginTime(g_Timer.GetDateTime("%Y-%m-%d %H:%M:%S"));
	//SEND_Login_Tongji2(false);

	if (SELF.GetLevel() <= 30)
	{
		g_pControl->Msg(MSG_CTRL_UPDATE_TRACK_INFO,MAKELONG(0,8));
	}
}

//// 要发送的消息处理
//bool CGameControl::SEND_Game_Connect()
//{
//	//string str = StringUtil::format("%d",g_Login.GetSessionID());
//	string ver = StringUtil::format("%d",GAME_VERSION);
//
//	SET_COMMAND(0,256);
//	ASSIGN_WORD(0,CS_GAME_CONNECT);
//
//	iBLen = 2;
//	ADD_STR(g_Login.GetLoginID());
//	ADD_BYTE('/');
//	ADD_STR(g_OtherData.GetCharName());
//	ADD_BYTE('/');
//	//ADD_STR(str.c_str());
//	//ADD_BYTE('/');
//	ADD_STR(ver.c_str());
//	ADD_STR("/0");
//
//	SEND_GAME_SERVER();
//	return true;
//}

bool CGameControl::SEND_Game_EnterGame()
{
TRY_BEGIN;

    g_AIAutoMgr.LaunchWaigua(); //进入开启外挂

	SET_COMMAND(CS_GAME_ENTERGAME,256);

	BYTE bySign = 1;//第一个bit表示高速版客户端标志
	if (g_pMagicCtrl && g_pMagicCtrl->GetMagicRoot(MAGICID_ENABLE_CREDIT))
	{
		bySign |= 0x02;//第二个bit表示是否支持信用
	}

	//服务器全局变量也可以控件IGW是否开启
	if((g_dwServerSwitch & SS_IGW) != 0)
	{
		SetIGWInfo();

		g_pControl->MsgToWnd(MSG_CTRL_HANDYMAP_WND,MSG_CTRL_HANDYMAP_WND,1);
	}

	g_pControl->MsgToWnd(MSG_CTRL_SHORTCUTKEY_WND,MSG_CTRL_SHORTCUTKEY_WND);


	if (g_pClientInfoCollect && (g_dwServerSwitch & SS_CLIENT_INFO_COL) !=0)
	{
		unsigned int areaid = g_Login.GetInnerAreaNo();
		unsigned int groupid = g_Login.GetInnerGroupNo();
		unsigned int grade = SELF.GetLevel();
		string userID = SELF.GetName();
		try
		{
			g_pClientInfoCollect->ReportClientQos(WS_GAME_ID,areaid,groupid,grade,userID.c_str());
		}
		catch (...)
		{
		}
	}

	//SendGameInfoForCheck();		//向客户端监控系统发送游戏信息
	ASSIGN_BYTE(6,bySign);
	ASSIGN_BYTE(7,g_iParamGameType);//从什么地方登录的

	int iWidth = g_pGfx->GetWidth();
	int iHeight = g_pGfx->GetHeight();
	BYTE byWH = 0;
	if (iWidth == 800 && iHeight == 600)
		byWH = 0;
	else if (iWidth == 1024 && iHeight == 768)
		byWH = 1;
	else if (iWidth == 1280 && iHeight == 800)
		byWH = 2;
	else if (iWidth == 1600 && iHeight == 900)
		byWH = 3;
	else if (iWidth == 1920 && iHeight == 1080)
		byWH = 4;
	else if (iWidth == 2560 && iHeight == 1440)
		byWH = 5;

	ASSIGN_BYTE(8, byWH); // 分辨率类型序号

	m_dwJumpTime = GetTickCount();
	g_Login.SetAutoLoginInType(0);

	SEND_GAME_SERVER();

	//传世. 发送成功进入地图指令
	//char szPacket[32];
	//wsprintf(szPacket, "%d",m_dwJumpTime); 
	//g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, CS_GAME_ENTERGAME);		//CS_GAME_ENTERGAME

	SEND_Request_Server_Time();
	SEND_SndaMark_Request();


	g_OtherData.SetEnterGameTime(GetTickCount());
	g_OtherData.SetLastMovePosTime(GetTickCount());
	//g_pControl->Msg(MSG_CTRL_UPDATE_HIDESELFMSG_BTN,0);

	//int iSize = SELF.EquipGood().Size();
	//for(int i = 0; i< iSize; i++)
	//{
	//	CheckUseBCSS(SELF.GetEquipGood(i),i);
	//}
	//自动关闭组队
	if(g_pGameData->IsAllowTroop() && g_AICfgMgr.IsCloseTeam())
		SEND_Group_Open_Team(false);

	SEND_Get_Medal_Exp();

	//SEND_Mac_To_GS();

TRY_END
	return true;
}

void CGameControl::SetIGWInfo()
{
	if(!g_pSDOAInterface)
		return;

TRY_BEGIN;

	DWORD dwTest = GetTickCount();
	UserBasicInfo UserInfo;
	WCHAR wszAccount[128] = {0};
	WCHAR wszEky[128] = {0};
	memset(&UserInfo, 0, sizeof(UserBasicInfo));
	UserInfo.cbSize = sizeof(UserBasicInfo);
	MultiByteToWideChar(CP_ACP,0,g_Login.GetLoginID(),-1,wszAccount,128);
	UserInfo.PTAccount = wszAccount;
	UserInfo.PTNumID.QuadPart = _atoi64(g_Login.GetPoptang_str());

	MultiByteToWideChar(CP_ACP,0,g_Login.GetEKey(),-1,wszEky,128);
	UserInfo.EKey = wszEky;
	string strPwd = g_Login.GetPassWord();
	char strTemp[512]={0};
	for(int i = 0;i < strPwd.length();i++)
	{
		strTemp[i] = strPwd.c_str()[i] ^ g_byPwdEncode;
	}
	strPwd = g_pNet->NewDecode(strTemp,strlen(strTemp));

	strcpy((char*)UserInfo.Password, strPwd.c_str());
	if (!g_Login.IsSDOALogin())
		g_pSDOAInterface->Login(&UserInfo,g_Login.GetLoginType());

	UserDetailInfo userdetailinfo;
	WCHAR wszNickName[128] = {0};
	WCHAR wszGameArea[128] = {0};
	WCHAR wszGameServer[128] = {0};

	memset(&userdetailinfo, 0, sizeof(UserDetailInfo));
	userdetailinfo.cbSize = sizeof(UserDetailInfo);
	MultiByteToWideChar(CP_ACP,0,SELF.GetName(),-1,wszNickName,128);
	userdetailinfo.pwcsNickName = wszNickName;
	userdetailinfo.nGender = SELF.IsMale();

	MultiByteToWideChar(CP_ACP,0,g_Login.GetAreaName(),-1,wszGameArea,128);
	userdetailinfo.pwcsGameArea = wszGameArea;
	userdetailinfo.nGameAreaID = (g_Login.GetInnerAreaNo()!= 0)?g_Login.GetInnerAreaNo():1;

	MultiByteToWideChar(CP_ACP,0,g_Login.GetGroupName(),-1,wszGameServer,128);
	userdetailinfo.pwcsGameServer = wszGameServer;
	userdetailinfo.nGameServerID = g_Login.GetInnerGroupNo();//-9999;

	g_pSDOAInterface->ChangePlayerInfo(userdetailinfo);

	wstring strCareer[3] = {L"战士",L"魔法师",L"道士"};
	WCHAR wszCharName[128] = {0};
	WCHAR wszGuildName[128] = {0};
	WCHAR wszGuildTile[128] = {0};

	MultiByteToWideChar(CP_ACP,0,SELF.GetName(),-1,wszCharName,128);
	MultiByteToWideChar(CP_ACP,0,SELF.GetTitle(),-1,wszGuildName,128);
	MultiByteToWideChar(CP_ACP,0,SELF.GetGuildTitle(),-1,wszGuildTile,128);

	UserCharacterInfo	CharInfo = {wszCharName,SELF.GetSex(),strCareer[SELF.GetCareer()].c_str(),SELF.GetLevel(),wszGuildName,wszGuildTile};
	g_pSDOAInterface->SetCharacterInfo(&CharInfo);
	//g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);

	g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);
	g_pSDOAInterface->SetLogicScreen(g_pGfx->GetWidth(),g_pGfx->GetHeight());

	POINT pt;
	pt.x = g_pGfx->GetWidth() - 305; pt.y = 0;
	g_pSDOAInterface->SetTaskBarPosition(&pt);		

	output_debug("igw_start2:%d\r\n",GetTickCount() - dwTest);
TRY_END
}


void CGameControl::SEND_Get_Medal_Exp()
{
	if(g_OtherData.GetUpdateMedalExp(0) != 0) 
		return;

	SET_COMMAND(CS_UPDATE_MEDAL_EXP,12);
	SEND_GAME_SERVER();
}


void CGameControl::MSG_Map_Name(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);

	g_pGameMap->SetMapTitle(buf.c_str());
	g_OtherData.SetMapInTime(GetTickCount());

	UserLocation userLocation;
	WCHAR wszTemp[128] = {0};
	MultiByteToWideChar(CP_ACP,0,buf.c_str(),-1,wszTemp,128);
	memset(&userLocation, 0, sizeof(userLocation));
	userLocation.pwcsPath = wszTemp;
	userLocation.pwcsPathTitle = wszTemp;
	userLocation.pwcsRoom = wszTemp;
	userLocation.pwcsRoomTitle = wszTemp;

	if(g_pSDOAInterface)
		g_pSDOAInterface->ChangeLocation(&userLocation);
}


void CGameControl::MSG_Message_System(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);

	WORD color = Conv_WORD(msg + 6);
	BYTE byType = msg[11];
	DWORD nType = 0;

	if((byType == 2 || byType == 1) && g_MsgFilter.FilterBlack(buf.c_str()))  //过滤黑名单
		return;

	if(g_MsgFilter.FilterRecv(buf.c_str()))  //过滤系统消息字符串，进行相应的处理
		return;

	//有特效的消息
	if(byType == 4)
	{
		g_pMagicCtrl->CreateMagic(MAGICID_PLAYER_CALL_OTHER);
	}
	else if(byType == 5)
	{
		if (!g_pMagicCtrl->FindMagicByAttr(MAGICID_PLAYER_CHUANYUNJIAN1,SELF.GetID(),-1))
		{
			g_pMagicCtrl->CreateMagic(MAGICID_PLAYER_CHUANYUNJIAN2); //穿云箭
		}
		g_TalkMgr.PushDataTalk(TALKTYPE_GUILD,buf.c_str(),buf.size(),0xFB00);	
		g_OtherData.SetTelStr(buf.c_str(),0xDF00);//绿字播报(和千里传音一样,颜色被写死了)
		return;
	}
	else if (byType == 6)
	{
		HorseRunToDead hr;
		hr.eFontType = FONT_YAHEI;
		hr.strText = buf;
		hr.wLastTimes = 1;
		hr.dwColor = COLOR_TEXT_SBU_TITLE;
		hr.iFontSize = 12;
		g_pGameData->AddHorseRunToDead(hr);
		g_pControl->Msg(MSG_CTRL_RUNHORSELAMP_WND,OPER_CREATE);
	}

	bool bIgnoreewLine = true;

	//计算实际类型
	if(byType == 2)
		nType = TALKTYPE_TROOP;
	else if(byType == 1)
		nType = TALKTYPE_PUBLIC;
	else if(byType == 3)//千里传音
	{
		g_OtherData.SetTelStr(buf.c_str(),0xDF00);//绿字播报
		return;
	}
	else if (msg[10] == 6)//传音号角
		nType = TALKTYPE_SYSTEM;
	else if(msg[10] == 10)
	{
		nType = TALKTYPE_SERVER;
		bIgnoreewLine = false;
	}
	else
	{
		nType = TALKTYPE_SYSTEM;
		bIgnoreewLine = false;
	}

	g_TalkMgr.PushDataTalk(nType,buf.c_str(),buf.size(),color,FONT_DEFAULT,FONTSIZE_DEFAULT,bIgnoreewLine);
}

void CGameControl::MSG_Message_Guild(const char * msg,int iLen)
{
	if((g_TalkMgr.GetFilter() & TALKTYPE_GUILD) == 0)
		return;

	char temp[4096] = {0};
	if(iLen > 4000)
		return;

	memcpy(temp,msg + 12,iLen - 12);
	temp[iLen - 12] = 0;

	WORD color = *((WORD *)(msg + 6));
	g_TalkMgr.PushDataTalk(TALKTYPE_GUILD,temp,iLen - 12,color);
}

void CGameControl::MSG_Message_General(const char * msg,int iLen)
{
	DWORD dwType = *((DWORD*)msg);

	if (dwType == 1) 
		dwType = TALKTYPE_PHYLE;

	if((g_TalkMgr.GetFilter() & dwType) == 0)
		return;

	char temp[4096] = {0};
	if(iLen > 4000)
		return;

	memcpy(temp,msg + 12,iLen - 12);
	temp[iLen - 12] = 0;

	if(g_MsgFilter.FilterRecv(temp))  //过滤系统消息字符串，进行相应的处理
		return;

	WORD color = *((WORD *)(msg + 6));
	g_TalkMgr.PushDataTalk(dwType,temp,iLen - 12,color);
}


void CGameControl::MSG_Message_Private(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);

	WORD color = Conv_WORD(msg + 6);
	BYTE byType = msg[10]; //类型

	string name;
	g_MsgFilter.GetNameOfMessage(buf.c_str(),name);
	if(strcmp(name.c_str(),SELF.GetName()) == 0)
		return;

	if (g_BossCfgMgr.IsBlack(name.c_str()))
		return;
	//if(g_MsgFilter.FilterBlack(name.c_str())) //过滤黑名单
	//	return;

	g_TalkName.InsertName(name.c_str());

	string tt;
	if(msg[8] == 1)
	{
		tt = "<会>";
	}
	else if( name.length() < 4)
	{
		tt = "<系统>";
		if(g_DirtyWords.ClearWords1(buf.c_str()))
		{
			return;
		}
		g_DirtyWords.ClearWords(buf);
		tt += buf;

		//加入消息框中
		g_TalkMgr.PushDataTalk(TALKTYPE_SYSTEM,tt.c_str(),tt.size(),TALKCOLOR_RED);
		//g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,tt.c_str(),tt.size(),TALKCOLOR_RED);
	}
	else
	{
		tt = "<生>";
	}
	if (g_pControl->FindWindowByName("OtherBoothWnd") && byType == 2)
	{
		//卖方给买方的消息
		DWORD uID = g_OtherBoothData.GetOtherPlayerID();
		CSimpleCharacterNode* pChar = g_pGameData->FindSimpleCharacter(uID);
		if (pChar && name == pChar->GetName())
		{
			g_TalkMgr.PushDataTalk(TALKTYPE_BUY,buf.c_str(),iLen-12,0xFFFC);
			return;
		}
	}
	else if (byType == 3)
	{//买方给卖方的消息
		g_TalkMgr.PushDataTalk(TALKTYPE_BOOTH,buf.c_str(),iLen - 12,0xFFFC);
		g_BoothTalkName.InsertName(name.c_str());
		return;
	}

	for(int i = 0; i < g_OtherData.GetRelationVector().size(); i++)
	{
		if( name == g_OtherData.GetRelationVector()[i].strName )
		{
			DWORD dwRelationType = g_OtherData.GetRelationVector()[i].iRelType;

			if( (dwRelationType & RT_WIFE) != 0)
				tt = "<妻>";
			else if( (dwRelationType & RT_HUSBAND) != 0)
				tt = "<夫>";
			else if( (dwRelationType & RT_MASTER) != 0)
				tt = "<师>";
			else if( (dwRelationType & RT_PRENTICE) != 0)
				tt = "<徒>";
			else if( (dwRelationType & RT_FRIEND) != 0)
				tt = "<友>";
			else if( (dwRelationType & RT_PHYLE ) != 0)
				tt = "<宗>";

			break;
		}
	}

	if (g_AICfgMgr.IsKeepOutInfo() && stricmp(tt.c_str(),"<生>") == 0)//tt.compare("") == 0)
	{
		return;
	}

	tt += buf;

	if(g_AIAutoMgr.IsEnalbeWaiGua() && g_AICfgMgr.IsMessageAwoke())
	{
		g_pAudio->Play(EAT_OTHER,204,g_pAudio->GetRand()++);
	}

	//加入消息框中
	g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,tt.c_str(),tt.size(),0xFFFC);
	//加入辅助工具的聊天记录窗口
	if(g_AICfgMgr.IsRecordMsg())
	{
		string strPlayerName;
		strPlayerName.assign(SELF.GetName());
		if(!g_DirtyWords.ParseNameIsGM(strPlayerName))
		{
			//脏话过滤
			string szPrivatemsg = tt;
			if(!g_DirtyWords.ClearWords1(szPrivatemsg.c_str()))
			{
				g_DirtyWords.ClearWords(szPrivatemsg);
				szPrivatemsg += g_Timer.GetDateTime("  [%Y-%m-%d %H:%M:%S]");

				g_AICfgMgr.AddPrivateMsg(szPrivatemsg.c_str(),szPrivatemsg.size(),false);
				g_pControl->PopupWindow("AssConfigWnd",MSG_CTRL_RECORD_ADDPRIVATEMSG);//更新窗口滚动条
			}
		}


	}
	//自动回复
	if(g_AICfgMgr.IsAutoReply())
	{
		string ret = "/";
		ret += name;
		ret += " ";
		ret += g_AICfgMgr.GetReplyMsg();
		SEND_Message_Private(ret.c_str(),ret.size());
	}
// 	else if (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain())
// 	{
// 		string ret = "/";
// 		ret += name;
// 		ret += " ";
// 		ret += "群英模式狂赚经验中，回头再聊~~~！";
// 		SEND_Message_Private(ret.c_str(),ret.size());
// 	}
}

//喊话
void CGameControl::MSG_Message_Talk(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);

	DWORD id   = Conv_DWORD(msg);
	WORD color = Conv_WORD(msg + 6);
	BYTE cTalkFont = msg[11]; //类型
	BYTE cTalkSize = msg[9];
	BYTE cTalkColor = msg[10];

	if( cTalkFont > FONT_GDI_ZHONGSONG )
		cTalkFont = 0;

	if(cTalkColor== 0x01 || cTalkFont < 3) 
		cTalkColor = 0xFF;
	else if(cTalkColor == 0xFF)
		cTalkColor = 0x01;

	if(g_MsgFilter.FilterBlack(buf.c_str())) //过滤黑名单
		return;

	if(cTalkFont >= FONT_GDI_XINSONG && FONT_GDI_XINSONG < FONT_GDI_ZHONGSONG)
		cTalkSize = FONTSIZE_BIG;
	else if(cTalkSize == 0)
		cTalkSize = FONTSIZE_DEFAULT;

	g_TalkMgr.PushShoutTalk(buf.c_str(),buf.size(),color,id,cTalkColor,cTalkFont,cTalkSize);
}


void CGameControl::SEND_Message_Confirm()
{
	SET_COMMAND(CS_MESSAGE_CONFIRM,12);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Magic_Ability(const char * msg,int iLen)
{
	int i = CMD_SIZE;
	int iMagicLen = 91;

	SELF.MagicArray().Clear();
	SELF.ProduceMagicArray().Clear();

	for(int j = 0 ; i < iLen; i += iMagicLen,j++)
	{
		if(j >= MAX_MAGIC_SKILL) return;

		BYTE byMagicType = msg[i + 23];//0:普通,1:生产技能

		CMagicData* pMagic = NULL;
		if (byMagicType == 0)
		{
			pMagic = SELF.MagicArray().Add(msg + i,iMagicLen);
		}
		else if (byMagicType == 1)
		{
			pMagic = SELF.ProduceMagicArray().Add(msg + i,iMagicLen);
		}
		else
		{
			continue;
		}

		int iMagicID = pMagic->GetMagicID();
		g_AIMgr.ChangeMagicState(iMagicID,TRUE);

		switch (iMagicID)
		{
		case MAGICID_SWT:
		case MAGICID_LSJ:
		case MAGICID_JYL:
			pMagic->SetDisable(!IsEquipXWSkillSuit());//圣王套装技能是否可用，要看身上的衣服
			break;
		}
	}

	g_AICfgMgr.LoadShortcutConfig();
}

void CGameControl::MSG_Package_Add_Object(const char * msg,int iLen)
{
	int iPos = -1;
	CGood* pGood = NULL;

	CGood addGood;
	addGood.FromBuffer(msg+12,false,iLen - 12);

	//向群英包裹添加
	//第六位
	BYTE type = *(BYTE*)(msg + 8);
	if (type > 0)
	{
// 		TneupMember* pMyself = g_TrusteeshipData.GetSelf();
// 		if (pMyself == NULL)
// 		{
// 			return;
// 		}
// 		//离线托管不要往托管包裹里加东西,因为队长的相关协议里会把这个物品放到群英包裹,不然就会出现两个物品
// 		if (!pMyself->byOffLineMode)
// 		{
// 			pMyself->package.Add(addGood);
// 		}

		return;
	}

	// 临时增加—去重检查
	DWORD dwMakeIndex = addGood.GetID();
	int iExistPos = SELF.PackageGood().FindGoodPos(dwMakeIndex);
	if (iExistPos >= 0)
	{
		// 物品已存在，做更新而非新增
		CGood& existGood = SELF.GetPackageGood(iExistPos);
		existGood.FromBuffer(msg + 12, false, iLen - 12);
		existGood.SetExternString(g_pGameData->GetExternString(dwMakeIndex));
		return;
	}
	
	WORD wLooks = addGood.GetLooks();

	if(g_AIGoodMgr.IsFitBelt(addGood))//药品类在腰带上找个位置放
	{
		//iPos = SELF.PackageGood().FindGoodPos(0,MAX_PACKAGE_ELEMENT - 6,MAX_PACKAGE_ELEMENT - 1);
		iPos = g_AutoEatMgr.FindPanelEmptyPos(wLooks);//如果这个位置有同类物品等待要放到这个位置，或等待自动解包后放到这个位置，如果新来的物品不是同一类，此时不应该占据这个位置
	}

	if(iPos < 0)
		iPos = SELF.PackageGood().FindGoodPos(0); //找到一个空位

	if(iPos >= 0)
	{
		if(iPos >= MAX_PACKAGE_ELEMENT - 6 && !g_AIGoodMgr.IsFitBelt(addGood))//只有腰带上有空位了，而且这个物品不能放到腰带上,找一个能放到腰带上的物品,实在找不到就把这个物品放到腰带上
		{
			int i = 0;
			for(i = 0; i < MAX_PACKAGE_ELEMENT - 6; i++) // 找一个物品放到腰带中
			{
				CGood & tmp = SELF.GetPackageGood(i);
				if(tmp.GetID() > 0 && g_AIGoodMgr.IsFitBelt(tmp))
				{
					SELF.GetPackageGood(iPos) = tmp;
					tmp.SetID(0);
					iPos = i;
					break;
				}
			}

			//if(i == MAX_PACKAGE_ELEMENT - 6)  //包裹里没有可以替换到腰带上的东西,不应该会出现这种情况,否则服务器会判断提示包裹格子或负重已
			//	return;
		}

		pGood = &(SELF.GetPackageGood(iPos));
		pGood->FromBuffer(msg + 12,false,iLen - 12);//生成物品数据
		pGood->SetExternString(g_pGameData->GetExternString(pGood->GetID()));
		g_AutoPickMgr.AddObject(pGood);

		g_FindGood.AddMessage(pGood->GetName(),1);

		//药袋类物品
		if(g_NPC.GetPackGoodID() > 0 && pGood->GetLooks() == 855)
		{
			g_NPC.SetPackGoodID(0);
			GoodAddEffect &goodEffect = pGood->GetGoodAddEffect();
			goodEffect.dwStartID = MAKELONG(1035,PACKAGE_stateitem);
			goodEffect.iFrams = 10;
			goodEffect.RM = RM_ADD1;
			goodEffect.iOffX = -19;
			goodEffect.iOffY = -21;
			goodEffect.iSoundID = 805;
			goodEffect.bFront = false;
			goodEffect.bHideGood = true;
		}

		const char *szGoodName = pGood->GetName();
		//元宝买的东西到达了，马上使用，需要加上使用的条件判断
		CQuickItem& BuyingItem = g_WooolStoreMgr.GetBuyingQuickItem();
		if(BuyingItem.strItemID.length() > 0)
		{
			if( BuyingItem.strName.compare(szGoodName) == 0
				//|| ( BuyingItem.strName.compare("2个白银钥匙") == 0 &&  strcmp(szGoodName,"白银钥匙") == 0 ) 
				|| ( BuyingItem.strName.compare("御兽天袍(包周)") == 0 &&  strcmp(szGoodName,"御兽天袍") == 0 )
				|| ( BuyingItem.strName.compare("御兽天衣(包周)") == 0 &&  strcmp(szGoodName,"御兽天衣") == 0 )
				|| ( BuyingItem.strName.compare("御兽天衣(包月)") == 0 &&  strcmp(szGoodName,"御兽天衣") == 0)
				|| ( BuyingItem.strName.compare("御兽天袍(包月)") == 0 &&  strcmp(szGoodName,"御兽天袍") == 0 )
				)

			{
				if(BuyingItem.iType == 1)//通过快捷消费2.0购买的物品
				{
					CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();
					char strTemp[128];

					sprintf(strTemp,"购买%d件%s成功",QuickBuyData.iBuyCount,BuyingItem.strName.c_str());

					//if(QuickBuyData.iUseAfterBuyType == 0)//买好后不使用，告诉触发快捷消费的地方，
					//{
					//	g_pControl->MsgToWnd(MSG_CTRL_ALLTREASUREBOX_WND,MSG_CTRL_QUICKBUY_OBJ_ARRIVE);
					//}
					//else 
					if(QuickBuyData.iUseAfterBuyType == 2 && QuickBuyData.iObjPos >= 0 && QuickBuyData.iObjPos < MAX_PACKAGE_ELEMENT)
					{
						g_pGameControl->SEND_Use_Object(QuickBuyData.iObjPos);
					}
					else if(QuickBuyData.iUseAfterBuyType == 1)
					{	
						g_pGameControl->SEND_Use_Object(iPos);
					}	
					else if(QuickBuyData.iUseAfterBuyType == 3)
					{
						if((BuyingItem.strName.compare("豹神水") == 0 || BuyingItem.strName.compare("兽神水") == 0)&& QuickBuyData.iObjPos >= 0 && QuickBuyData.iObjPos < MAX_PACKAGE_ELEMENT)
						{
							DWORD id1 = SELF.GetPackageGood(QuickBuyData.iObjPos).GetID();
							DWORD id2 = pGood->GetID();
							g_pGameControl->SEND_Deal_Special_Obj(id1,id2,1);
						}
					}
					else if(QuickBuyData.iUseAfterBuyType == 4)//御兽天袍/御兽天衣，买好后使用购买好了的物品，然后再骑上宠物
					{
						g_pGameControl->SEND_Use_Object(iPos);
						g_pGameControl->SEND_Player_Ride_Horse();
					}
					else if(QuickBuyData.iUseAfterBuyType == 5)
					{
						CGood& goodtemp = g_NPC.GetDamageGood();
						g_pGameControl->SEND_Exchange_Repair_Ok(0,goodtemp.GetID(),goodtemp.GetName(),3);							
						goodtemp.SetID(0);				
						g_NPC.SetSaleGold(0);
					}	
					else if(QuickBuyData.iUseAfterBuyType == 6)
					{
						PET_SENDOUT_MAP& petMap = g_PetData.GetSendOutPetList();
						for(PET_SENDOUT_MAP::iterator it = petMap.begin();it != petMap.end();it++)
						{
							if(it->second.szReserved[0] == 1)
							{
								g_pGameControl->SEND_Feed_Pet_From_Interface(it->first,pGood->GetID());
								SELF.GetUsingTemp().Add(*pGood);
								pGood->SetID(0);
								break;
							}
						}						
					}
					else if(QuickBuyData.iUseAfterBuyType == 7)
					{
						g_pGameControl->SEND_ZHENBAO_INFO(2,(BYTE)QuickBuyData.iObjPos);
					}
					else if(QuickBuyData.iUseAfterBuyType == 8)
					{
						g_pGameControl->SEND_AddObjToObj(SELF.GetPackageGood(QuickBuyData.iObjPos),SELF.GetPackageGood(QuickBuyData.iObjPos));
					}
					else if(QuickBuyData.iUseAfterBuyType == 9)
					{
						g_pGameControl->SEND_Exchange_Menu_Click(g_NPC.GetID(),"@@linghaoshi");
					}
					else if(QuickBuyData.iUseAfterBuyType == 10)
					{
						g_pGameControl->SEND_Use_Object(iPos);
						if(QuickBuyData.iObjPos >= 0 && QuickBuyData.iObjPos < MAX_PACKAGE_ELEMENT)
						{
							g_pGameControl->SEND_Use_Object(QuickBuyData.iObjPos,0,1);
						}
					}

					g_TalkMgr.PushSysTalk(strTemp);
					QuickBuyData.clear();

				}
				else if(pGood->GetStdMode() < 4)
					g_pGameControl->SEND_Use_Object(iPos);

				BuyingItem.clear();
			}
		}



		//第一次进入特定物品闪烁
		if (SELF.GetLevel() < 30)
		{
			if(!g_pGameData->HasPaoPaoStatus(EP_First_GetTaoMuJian) && strcmp(szGoodName,"新手乌木剑") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);

				if(!g_pControl->MsgToWnd(MSG_CTRL_PACKAGEWND,MSG_CTRL_POPUP_ARROWTIP, EP_First_GetTaoMuJian_PaoPao ,(CControl*)"桃木剑"))
				{
					if (g_EutUiType == EUT_CLASSIC)
					{
						g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_GetTaoMuJian_PaoPao1,800 - 710,185 - 158,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
					} 
					else
					{
						g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_GetTaoMuJian_PaoPao1,800 - 393,185 - 150,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
					}
					
				}
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetWaKuangBook) && strcmp(szGoodName,"挖矿") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetWaKuangBook);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetHuanCaiJieZhi) && strcmp(szGoodName,"幻彩戒指") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetHuanCaiJieZhi);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetHuanCaiShouZhuo) && strcmp(szGoodName,"幻彩手镯") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetHuanCaiShouZhuo);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetSkillBook) && (strcmp(szGoodName,"小火球") == 0 || strcmp(szGoodName,"治疗术") == 0 || strcmp(szGoodName,"初级剑法") == 0))
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetSkillBook);
				g_pControl->MsgToWnd(MSG_CTRL_PACKAGEWND,MSG_CTRL_POPUP_ARROWTIP, EP_First_GetAttackSkillBook_PaoPao ,(CControl*)szGoodName);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetQunYingBaoJian) && strcmp(szGoodName,"群英宝鉴") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetQunYingBaoJian);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetLianYuTiYuanKa) && strcmp(szGoodName,"炼狱体验卡") == 0)
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetLianYuTiYuanKa);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetXinShouLingPai) && strcmp(szGoodName,"新手装备令牌") == 0)/////////
			{
				pGood->SetFlashTexID(MAKELONG(21586,PACKAGE_INTERFACE));
				g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,1);
				g_pGameControl->Send_Player_Prompt_Status(EP_First_GetXinShouLingPai);
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetQunYingBaoJian_PaoPao) && strcmp(szGoodName,"群英宝鉴") == 0)
			{
				g_pControl->MsgToWnd(MSG_CTRL_PACKAGEWND,MSG_CTRL_POPUP_ARROWTIP, EP_First_GetQunYingBaoJian_PaoPao ,(CControl*)"群英宝鉴");
			}
			else if(!g_pGameData->HasPaoPaoStatus(EP_First_GetQunYingBaoJian_PaoPao) && strcmp(szGoodName,"群英令") == 0)
			{
				g_pControl->MsgToWnd(MSG_CTRL_PACKAGEWND,MSG_CTRL_POPUP_ARROWTIP, EP_First_GetQunYingBaoJian_PaoPao ,(CControl*)"群英令");
			}


		}



		//自动叠矿,妖翎羽
		int iStdMode = pGood->GetStdMode();
		int iShape = pGood->GetShape();

		if((iStdMode == 43 && iShape == 0) || iStdMode == 48 || iStdMode == 63 || iStdMode == 96 || iStdMode == 46)
		{
			//妖翎羽
			if (iStdMode == 46)
			{
				if (iShape == 5 || iShape == 6)
				{
					int iFindPos = SELF.PackageGood().FindGoodByStdmodeNameLevel(iStdMode,iShape,pGood->GetName(),0,-1,-1,-1,pGood->GetID(),50000 - pGood->GetDura());
					if (iFindPos >= 0)
					{
						g_pGameControl->SEND_AddObjToObj(*pGood,SELF.GetPackageGood(iFindPos));
						return;
					}
				}
			}
			else if (iStdMode == 96)
			{
				int iFindPos = SELF.PackageGood().FindGoodByStdmodeNameLevel(iStdMode,iShape,pGood->GetName(),0,-1,-1,-1,pGood->GetID(),50 - pGood->GetDura());
				if (iFindPos >= 0)
				{
					g_pGameControl->SEND_AddObjToObj(*pGood,SELF.GetPackageGood(iFindPos));
					return;
				}
			}
			else
			{
				CGood &medal = SELF.EquipGood().Get(ITEM_POS_MEDAL);
				if (medal.GetStdMode() == 30 && medal.GetShape() == 6)//生产勋章
				{
					int iFindPos = SELF.PackageGood().FindGoodByStdmodeNameLevel(iStdMode,iShape,pGood->GetName(),0,-1,pGood->GetDura() / 10000,255 - pGood->GetAC(),pGood->GetID());
					if (iFindPos >= 0)
					{
						g_pGameControl->SEND_AddObjToObj(*pGood,SELF.GetPackageGood(iFindPos));
						return;
					}
				}
			}
		}
		//end

	}

}

void CGameControl::SEND_Object_Pickup(int x,int y)
{
	SET_COMMAND(CS_PACKAGE_OBJECT_PICKUP,12);
	ASSIGN_WORD(6,x);
	ASSIGN_WORD(8,y);

	SEND_GAME_SERVER();
}


bool CGameControl::SEND_Package_Object_Reject(CGood &Good) // 扔东西
{
	//太长时间没回应清除第一个,免得一直扔不了
	DWORD dwCount = GetTickCount();
	if(!SELF.GetDropTemp().Add(Good))
	{
		if(dwCount - SELF.GetReserveData(plyLastRejectObjectTime) < 5000)
		{
			return false;
		}

		SELF.GetDropTemp().Get(0).SetID(0);
		if(!SELF.GetDropTemp().Add(Good))
		{
			return false;
		}
	}

	SELF.SetReserveData(plyLastRejectObjectTime,dwCount);


	SET_COMMAND(CS_PACKAGE_OBJECT_REJECT,255);

	ASSIGN_ID(Good.GetID());
	ADD_STR(Good.GetName());

	SEND_GAME_SERVER();

	Good.SetID(0);
	return true;
}

void CGameControl::MSG_Package_Reject_Confirm(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	if(SELF.GetDropTemp().FindGood(id))
	{
		SELF.GetDropTemp().DeleteGood(id);
		g_AutoPickMgr.RemoveObject(id);

		g_pGameData->DelExternString(id);
	}
}

void CGameControl::SEND_Drop_Gold(long lDropGood)
{
	SET_COMMAND(CS_DROP_GOLD,12);
	ASSIGN_ID(lDropGood);
	SEND_GAME_SERVER();
}


// 实现了自动放药的功能
void CGameControl::MSG_Object_Use_Success(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	CGood* tmp = NULL;
	if (id != 0)
	{
		tmp = SELF.GetUsingTemp().FindGood(id);
	}

	//如果GS发过来的ID为0,只能取最早使用的
	if (NULL == tmp)
	{
		tmp = &(SELF.GetUsingTemp().Get(0));
	}

	if(tmp->GetID() == 0)
		return;

	int iLooks = tmp->GetLooks();
	if(iLooks >= 792 && iLooks <= 797 || iLooks == 802) //特殊的道具使用声音
	{
		if(iLooks >= 792 && iLooks <= 794)
		{
			int iSoundID = 210+iLooks-792;
			g_pAudio->Play(EAT_OTHER,iSoundID,g_pAudio->GetRand()++);
		}
	}
	else
	{
		//使用物品音乐
		int j = tmp->GetStdMode()+1;
		g_pAudio->Play(EAT_GOODS, j, g_pAudio->GetRand()++);		
	}

	g_AutoPickMgr.RemoveObject(tmp->GetID());
	g_AutoEatMgr.UseSuccess(*tmp);
	SELF.GetUsingTemp().DeleteGood(tmp->GetID());

}

void CGameControl::MSG_Object_Use_Failed(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	CGood* tmp = NULL;
	if (id != 0)
	{
		tmp = SELF.GetUsingTemp().FindGood(id);
	}

	//如果GS发过来的ID为0,只能取最早使用的
	if (NULL == tmp)
	{
		tmp = &(SELF.GetUsingTemp().Get(0));
	}

	if(tmp->GetID() == 0)
		return;

	if(tmp->GetFromWnd() == LingXiBox_Free_Wnd || tmp->GetFromWnd() == LingXiBox_Pay_Wnd)		//取出失败，放回至尊宝盒
	{
		int eFromWnd = tmp->GetFromWnd();

		tmp->SetFromWnd(0);
		tmp->SetToWnd(0);
		if (eFromWnd == LingXiBox_Free_Wnd)
		{
			g_pGameData->GetBoxInfo().freegoods.BackToArray(*tmp,tmp->GetPos());			
		}
		else
		{
			g_pGameData->GetBoxInfo().paygoods.BackToArray(*tmp,tmp->GetPos());			
		}
	}
	else if(tmp->GetFromWnd() == Pet_Package_Wnd)//宠物包裹
	{
		tmp->SetFromWnd(0);
		SELF.PetGood().BackToArray(*tmp,tmp->GetPos());
	}
	else
	{
		//判断是否为托管包裹，取标记，1代表托管包裹
		BYTE type = *(BYTE*)(msg + 8);
		if (type > 0)
		{
			tmp->SetFromWnd(NO_Wnd);

// 			if (g_TrusteeshipData.GetSelf())
// 			{
// 				g_TrusteeshipData.GetSelf()->package.BackToArray(*tmp,tmp->GetPos());
// 			}
		}
		else if(SELF.PackageGood().BackToArray(*tmp,tmp->GetPos()))
		{
			if(CGoodGrid::GetDropGoodFrom().DropGood.GetID() == tmp->GetID())
			{
				CGoodGrid::ClearGoodFrom();
			}
		}

		g_AutoEatMgr.UseFailed(*tmp);
	}

	SELF.GetUsingTemp().DeleteGood(tmp->GetID());
}

bool CGameControl::SEND_AddObjToObj(CGood &srcGood,CGood &destGood,bool bAddSrcToUseTemp)
{
	//太长时间没有回应,清除GetUsingTemp最上面一个,防止出现一直不能使用物品的情况
	DWORD dwCount = GetTickCount();
	if(!SELF.GetUsingTemp().Add(bAddSrcToUseTemp?srcGood:destGood))
	{
		if(dwCount - SELF.GetReserveData(plyLastUseObjectTime) < 3000)
		{
			return false;
		}

		SELF.GetUsingTemp().Get(0).SetID(0);

		if(!SELF.GetUsingTemp().Add(bAddSrcToUseTemp?srcGood:destGood))
		{
			return false;
		}
	}

	SELF.SetReserveData(plyLastUseObjectTime,dwCount);


	SET_COMMAND(CS_PACKAGE_OBJECT_USE,12);
	ASSIGN_ID(srcGood.GetID());
	ASSIGN_DWORD(6,destGood.GetID());
	SEND_GAME_SERVER();  

	if (bAddSrcToUseTemp)
	{
		srcGood.SetID(0);
	}
	else
	{
		destGood.SetID(0);
	}

	return true;
}

bool CGameControl::SEND_Use_Object(int iPos,DWORD dwLinkID,DWORD dwData2)
{
	CGood& tmp = SELF.GetPackageGood(iPos);

	if(tmp.GetID() == 0)
		return false;

	tmp.SetPos(iPos);

	if(g_AIGoodMgr.FilterUseGood(tmp))
		return false;

	if(SELF.IsDead())//////正常的物品死亡时不能使用
		return false;

	int iStdMode = tmp.GetStdMode();
	int iShape = tmp.GetShape();

	if (iStdMode == 3)
	{
		bool bWalking = true;
		if (iShape == 1 || iShape == 2 || iShape == 3 || iShape == 5 ||
			iShape == 15 || iShape == 17)
		{
			bWalking = false;
		}

		if (iShape == 19 && tmp.GetLooks() == 19)
			bWalking = false;

		if (!bWalking) g_MapFinder.SetWalking(false);
	}

	//五行令旗		
	if(iStdMode ==  3 && iShape == 57)
	{
		if(!SELF.GetUsingTemp().Add(tmp))
			return false;

		SET_COMMAND(CS_FLAG_WUXING,CMD_SIZE);
		ASSIGN_ID(tmp.GetID());
		ASSIGN_DWORD(6,g_pControl->GetMouseOnID());
		tmp.SetID(0);
		SEND_GAME_SERVER();
		return true;
	}

	if(strcmp(tmp.GetName(),"群英宝鉴") == 0)
	{
		if(SELF.PackageGood().FindGoodByName("群英令") == -1 && SELF.PackageGood().FindGoodByName("群英令(捆)") == -1)
		{
			g_WooolStoreMgr.SetQuickBuyItem("群英令");
			CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

			if(QuickBuyData.pItem)
			{
				char str[256] = "";
				sprintf(str,"您的群英令已经用完了，您需要马上购买吗？单价%d元宝",QuickBuyData.pItem->iPrice);
				QuickBuyData.strMsg = str;
				QuickBuyData.iType = 1;
				QuickBuyData.iUseAfterBuyType = 1;
				QuickBuyData.iObjPos = iPos;
				g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);						
			}

			return true;
		}
	}

	if(strcmp(tmp.GetName(),"飞来神石") == 0)
	{
		g_pControl->PopupWindow(MSG_CTRL_USE_OBJECT_WND,OPER_CREATE,1);	
		return false;
	}
	else if(strcmp(tmp.GetName(),"回城神石") == 0 || strcmp(tmp.GetName(),"回城神石(大)") == 0 || strcmp(tmp.GetName(),"回城石") == 0 || strcmp(tmp.GetName(),"新手回城神石") == 0)
	{
		char m[8] = {0};
		m[0] = tmp.GetDC();
		m[1] = tmp.GetDC2();
		m[2] = tmp.GetMC();
		m[3] = tmp.GetMC2();
		m[4] = tmp.GetSC();
		m[5] = tmp.GetSC2();

		LocalmapInfo_s* m_pAreaMap = g_pMinMap->GetLocalMap(m);
		if (!m_pAreaMap)
		{
			DWORD tmpID = tmp.GetID();
			g_pControl->PopupWindow(MSG_CTRL_USE_OBJECT_WND,OPER_CREATE,4);
			g_pControl->MsgToWnd(MSG_CTRL_USE_OBJECT_WND,OPER_CREATE,tmpID);
			return false;
		}
	}
	else if(iStdMode == 3 && iShape == 28)//用还魂神符复活别人
	{
		CSimpleCharacterNode* p = g_pGameData->FindSimpleCharacter(g_pControl->GetMouseOnID());
		if(p && p->IsDead() )
		{
			g_pGameControl->SEND_Use_ReliveItem(p->GetID(),tmp.GetID());
			return true;
		}
		return false;
	}
	//双击元神元丹
	else if(iStdMode == 0 && iShape == 3)
	{
		g_TalkMgr.PushSysTalk("此丹只能给分身服用");
		return false;
	}	
	////豹魔石 //快捷消费2.0
	//else if(tmp.GetStdMode() == 49 && tmp.GetShape() == 51)
	//{
	//	map<DWORD,PET_SENDOUT>::const_iterator it;
	//	map<DWORD,PET_SENDOUT>& mPetList = g_PetData.GetSendOutPetList();
	//	it = mPetList.find(tmp.GetID());
	//	//PET_SENDOUT &SendOutPet = g_pGameData->GetPetData().GetSendOutPet();
	//	//if(SendOutPet.dwPetItemID == 0)//已经放出豹子了，此时双击是收豹子则不触发快捷消费
	//	if(it == mPetList.end())
	//	{
	//		//喂食时间
	//		DWORD m = tmp.GetRecordTime();

	//		if( m != 0 )
	//		{
	//			int iDays = g_Timer.GetServerHours(m) / 24;
	//			if(iDays >= 4)
	//			{					
	//				g_WooolStoreMgr.SetQuickBuyItem("兽神水");						

	//				CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

	//				if(!QuickBuyData.pItem)
	//				{
	//					g_WooolStoreMgr.SetQuickBuyItem("豹神水");
	//					
	//					CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();
	//				}
	//				if(QuickBuyData.pItem)
	//				{
	//					char cTemp[128]={0};

	//					sprintf(cTemp,"你的灵兽已经饿死，你需要马上购买并使用兽神水复活你的灵兽吗？单价%d元宝。",QuickBuyData.pItem->iPrice);
	//					QuickBuyData.iUseAfterBuyType = 3;

	//					QuickBuyData.strMsg = cTemp;
	//					QuickBuyData.iType = 1;
	//					QuickBuyData.iObjPos = iPos;

	//					g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);
	//					return false;
	//				}

	//			}
	//		}   
	//	}
	//}
	else if (tmp.GetStdMode() == 96) // 妖翎羽
	{
		if (SELF.GetHolyWingLevel() >= 0)
		{
			if (g_pControl->FindWindowByName("DivinityWingWnd"))
			{
				DWORD dwgoodid = tmp.GetID();
				g_pControl->Msg(MSG_CTRL_DIVINITYWING_WND, 10, (CControl*)&dwgoodid);
			}

			g_pControl->PopupWindow(MSG_CTRL_DIVINITYWING_WND,OPER_CREATE);
		}
		return true;
	}
	else if(tmp.GetStdMode() == 98 && tmp.GetShape() == 0 && dwData2 != 1)//经验宝盒dwData2 == 1 表示是精力药水购买成功以后再次使用该物品,因为精力药水刚刚使用,精力值还没有来得及加上,所以就不判断了
	{
		if (SELF.GetJingLi() < tmp.GetDuraMax() * 50)
		{
			if (tmp.GetDuraMax() <= 2)
			{
				g_WooolStoreMgr.SetQuickBuyItem("精力药水(小)");						
			}
			else if (tmp.GetDuraMax() <= 15)
			{
				g_WooolStoreMgr.SetQuickBuyItem("精力药水(中)");						
			}
			else
			{
				g_WooolStoreMgr.SetQuickBuyItem("精力药水(大)");						
			}


			CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();
			if(QuickBuyData.pItem)
			{
				char cTemp[128]={0};

				sprintf(cTemp,"你的精力值不够，你需要马上购买%s并使用吗？单价%d元宝。",QuickBuyData.pItem->strName.c_str(),QuickBuyData.pItem->iPrice);
				QuickBuyData.iUseAfterBuyType = 10;

				QuickBuyData.strMsg = cTemp;
				QuickBuyData.iType = 1;
				QuickBuyData.iObjPos = iPos;

				g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);
			}
			else
			{
				g_MsgBoxMgr.PopSimpleAlert("你的精力值不够。");
			}

			return false;
		}
	}


	//太长时间没有回应,清除GetUsingTemp最上面一个,防止出现一直不能使用物品的情况
	DWORD dwCount = GetTickCount();
	if(!SELF.GetUsingTemp().Add(tmp))
	{
		if(dwCount - SELF.GetReserveData(plyLastUseObjectTime) < 3000)
		{
			return false;
		}

		SELF.GetUsingTemp().Get(0).SetID(0);

		if(!SELF.GetUsingTemp().Add(tmp))
		{
			return false;
		}
	}

	SELF.SetReserveData(plyLastUseObjectTime,dwCount);

	SET_COMMAND(CS_PACKAGE_OBJECT_USE,256);
	ASSIGN_ID(tmp.GetID());

	if(g_AIGoodMgr.IsLingfu(tmp) || g_AIGoodMgr.IsMiXianLuYing(tmp) || g_AIGoodMgr.IsLingShouXiangQuan(tmp) || g_AIGoodMgr.IsJiGuanYaoShi(tmp)) 
	{
		/////////////////////////骑乘状态下不可使用灵符、迷仙路引、灵兽项圈/////////
		if(SELF.IsOnLepoard() && !SELF.GetFightOnLeopard())
		{
			///////////////
			g_TalkMgr.PushSysTalk("骑乘(非骑战)状态下不可使用此物品！");
			SELF.GetUsingTemp().DeleteGood(tmp.GetID());
			return false;
		}	    
		else
		{
			ASSIGN_DWORD(6,g_pControl->GetMouseOnID());
		}	    
	}
	else if(g_AIGoodMgr.IsLinkGood(tmp))
	{
		if(dwLinkID != 0)
			ASSIGN_DWORD(6,dwLinkID);
		else
			ASSIGN_DWORD(6,g_NPC.GetPackGoodID());
	}
	else if(iStdMode = 3 && (iShape == 206 || iShape == 207))
	{
		ASSIGN_DWORD(6,g_pControl->GetMouseOnID());
	}
	else if (dwData2 != 0)
	{
		ASSIGN_DWORD(6,dwData2);
	}
	else if (g_AIGoodMgr.IsLianHuLu(tmp))
	{
		ASSIGN_DWORD(6,g_pControl->GetMouseOnID());
	}

	ADD_STR(tmp.GetName());
	SEND_GAME_SERVER();

	tmp.SetID(0);
	return true;
}

//离开迷仙阵
void CGameControl::SEND_ExitMazeMap()
{
	SET_COMMAND(CS_EXIT_MAZE,12);
	SEND_GAME_SERVER();  
}

void CGameControl::Send_Elixir_Buy_Apply(int iType,int iCount)
{
	if (iType >= 1 && iType <= 4 && iCount > 0)
	{
		SET_COMMAND(CS_ELIXIR_APPLY_BUY,12);
		ASSIGN_WORD(6,(WORD)iType);
		ASSIGN_WORD(8,(WORD)iCount);
		SEND_GAME_SERVER();
	}
}

void CGameControl::SEND_PetUse_Object(CGood& good)
{
	DWORD dwID = good.GetID();
	if(dwID <= 0)  
		return;

	int iStdMode = good.GetStdMode();
	int iShape = good.GetShape();

	if((iStdMode == 49 && iShape == 52) || 
		(iStdMode == 37 && iShape == 10 ) ||
		iStdMode == 53 && iShape ==0)
	{
		SET_COMMAND(CS_PETPACKAGE_OBJECT_USE,12);
		good.SetFromWnd(Pet_Package_Wnd);
		if(SELF.GetUsingTemp().Add(good))
		{
			ASSIGN_ID(dwID);
			SEND_GAME_SERVER();  
			good.SetID(0);
		}
		else
		{
			good.SetFromWnd(NO_Wnd);
		}
	}	
}

void CGameControl:: SEND_Message_Send(CStringLine* pLine,const char * szHeader)
{
	//非法数据
	int iLen = pLine->getBufLength();
	if(pLine == NULL || iLen == 0 || iLen > 1000)
		return;

	if(g_MsgFilter.FilterSend(pLine->c_str())) //判断是否要过滤发送的消息
		return;

	string strPlayerName = SELF.GetName();
	if(!g_DirtyWords.ParseNameIsGM(strPlayerName))
	{
		if(g_DirtyWords.ClearWords1(pLine)) //清除整个一行
			return;
		g_DirtyWords.ClearWords(pLine);//替换掉内部的字符
	}

	// 发送聊天信息
	//fnMakeDefMessage(&DefMsg, CS_MESSAGE_SEND, 0, 0, 0, 0);
	//g_pNet->SendPacket(&DefMsg, (char*)pLine->getBuf());

	SET_COMMAND(CS_MESSAGE_SEND,1024);
	if (szHeader)
	{
		ADD_STR(szHeader);
	}
	ADD_ARRAY(pLine->getBuf(),iLen);
	if(temp[12] == '#' && (temp[13] == '#' || temp[13] == '1'))
	{
		int i = 14;
		while(temp[i])
		{
			if(temp[i] == ' ')
			{
				if(temp[13] == '#')		// 人名
				{
					if(i > 14 && (i - 14) < 16)
					{
						char sss[16];
						memset(sss,0,16);
						memcpy(sss,temp + 14,i - 14);
						g_TalkName.InsertLWName(sss);
					}
				}
				else					// 手机号码
				{
					if(i == 24)
					{
						char sss[16];
						memset(sss,0,16);
						memcpy(sss,temp + 13,11);
						g_TalkName.InsertLWName(sss);
					}
				}
				break;
			}
			i++;
		}
		g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,temp + 12,strlen(temp) - 12,TALKCOLOR_RED);
	}
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Message_Send(const char * str,int iLen)
{
	if(iLen > 1000)
		return;

	if(g_MsgFilter.FilterSend(str)) //判断是否要过滤发送的消息
		return;

	// 替换掉回车换行
	char *pt = (char *)str;
	while(*pt)
	{
		if((*pt == 0x0D) || (*pt == 0x0A))
			*pt = 0x20;
		pt++;
	}

	int i = 0;
	for(; i < iLen; i++)
	{
		if(str[i] != ' ')
			break;
	}

	if(i == iLen) return;  // 全是空格


	//fnMakeDefMessage(&DefMsg, CS_MESSAGE_SEND, 0, 0, 0, 0);
	//g_pNet->SendPacket(&DefMsg, (char*)str);
	
	SET_COMMAND(CS_MESSAGE_SEND,1024);
	string strPlayerName = SELF.GetName();
	if(!g_DirtyWords.ParseNameIsGM(strPlayerName))
	{
		if(g_DirtyWords.ClearWords1(str)) //清除整个一行
			return;
		g_DirtyWords.ClearWords(str,temp+12);//替换掉内部的字符
	}
	else
	{
		strcpy(temp+12,str);
	}
	if(temp[12] == '#' && (temp[13] == '#' || temp[13] == '1'))
	{
		int i = 14;
		while(temp[i])
		{
			if(temp[i] == ' ')
			{
				if(temp[13] == '#')		// 人名
				{
					if(i > 14 && (i - 14) < 16)
					{
						char sss[16];
						memset(sss,0,16);
						memcpy(sss,temp + 14,i - 14);
						g_TalkName.InsertLWName(sss);
					}
				}
				else					// 手机号码
				{
					if(i == 24)
					{
						char sss[16];
						memset(sss,0,16);
						memcpy(sss,temp + 13,11);
						g_TalkName.InsertLWName(sss);
					}
				}
				break;
			}
			i++;
		}
		g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,temp + 12,strlen(temp) - 12,TALKCOLOR_RED);
	}

	iBLen = CMD_SIZE + strlen(temp + CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Message_Private(CStringLine* pLine)
{
	if(pLine == NULL || pLine->getBufLength() == 0) //非法数据
		return;

	if(strlen(pLine->c_str()) > 1000)
		return;

	string strPlayerName = SELF.GetName();
	if(!g_DirtyWords.ParseNameIsGM(strPlayerName))
	{
		if(g_DirtyWords.ClearWords1(pLine)) //清除整个一行
			return;
		g_DirtyWords.ClearWords(pLine);//替换掉内部的字符
	}
	int iLen = pLine->getBufLength();
	const char* str = pLine->c_str();

	char name[256] = {0};
	const char* pchar = strchr(str,' ');
	if(pchar != NULL)
	{
		strncpy(name,str+1,pchar-str-1);
		g_TalkName.InsertName(name);
	}

	//如果名字超过14字节服务器会踢人
	if((pchar != NULL && strlen(name) > 14) || (pchar == NULL && iLen > 15))
	{
		g_TalkMgr.PushSysTalk("密聊时对方名字不能超7个汉字或14个英文字母");
		return;
	}
	g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,str,iLen, 0xFF04);

	SET_COMMAND(CS_MESSAGE_PRIVATE,1024);
	ADD_ARRAY(pLine->c_str(),strlen(pLine->c_str()));
	SEND_GAME_SERVER();
}

void CGameControl:: SEND_Message_Private(const char* str,int iLen,bool bShow)
{
	if(str == NULL || iLen == 0 || iLen > 1000)
		return;

	char name[256] = {0};
	const char* pchar = strchr(str,' ');
	if(pchar == NULL)
		return;

	SET_COMMAND(CS_MESSAGE_PRIVATE,1024);

	//添加名字
	memcpy(temp + CMD_SIZE,str,iLen);
	//	strncpy(name,pchar,pchar - str);
	strncpy(name,str,pchar - str);
	g_TalkName.InsertName(name);

	string strPlayerName = SELF.GetName();
	if(!g_DirtyWords.ParseNameIsGM(strPlayerName))
	{
		//脏话过滤
		if(g_DirtyWords.ClearWords1(pchar))
			return;
		g_DirtyWords.ClearWords(pchar,temp+12+(pchar - str));
	}
	else
	{
		strcpy(temp+12+(pchar - str),pchar);
	}

	if (bShow)
		g_TalkMgr.PushDataTalk(TALKTYPE_PRIVATE,str,iLen, 0xFF04);
	iBLen = iBLen + iLen;
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Message_Rest(const char * str, int iLen)
{
	//fnMakeDefMessage(&DefMsg, CS_MESSAGE_SEND, 0, 0, 0, 0);
	//g_pNet->SendPacket(&DefMsg, (char*)str);

	SET_COMMAND(CS_MESSAGE_SEND,1024);
	ADD_ARRAY(str,iLen);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Package_Reject_Fail(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);

	CGood* pGood = SELF.GetDropTemp().FindGood(id);
	if(pGood)
	{
		SELF.PackageGood().BackToArray(*pGood,pGood->GetPos());
		SELF.GetDropTemp().DeleteGood(id);
	}
}

void CGameControl::SEND_Object_Keeping(DWORD npcid,DWORD objID,const char * objName)
{
	SET_COMMAND(CS_OBJECT_KEEPING,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,objID);
	ADD_STR(objName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Make_Poison(DWORD npcid,const char * PoisonName)
{
	SET_COMMAND(CS_MAKE_POISON,256);
	ASSIGN_ID(npcid);
	ADD_STR(PoisonName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Object_Getback(DWORD npcid,DWORD objID,const char * objName)
{
	SET_COMMAND(CS_OBJECT_GETBACK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,objID);
	ADD_STR(objName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Open_MapView()
{
	SET_COMMAND(CS_OPEN_MAPVIEW,CMD_SIZE);
	SEND_GAME_SERVER();
}

//i = 1 请求开宝箱 
//i = 2 请求获得物品 
// 3 请求发本回合奖励；  4为请求下一轮抽奖； 5为请求发奖励到包裹
//6请求关闭结束本轮开宝箱
//7告诉服务器玩家是否选择连续开宝箱,1表示是，０表示否
//12,为祈愿树
void CGameControl::SEND_Open_Box_Requese(int i,BYTE byContinueOpenNewBox)
{
	SET_COMMAND(CS_OPEN_BOX_REQUEST,16);

	if(i < 3)
	{
		ASSIGN_WORD(6,i);
	}
	else if(i<= 11 && i != 7)//新宝箱及新宝箱2
	{
		ASSIGN_BYTE(6,i);
		ASSIGN_BYTE(7,g_NPC.GetBoxOpenCycle());
	}
	else if(i == 7 || i == 12)
	{
		ASSIGN_BYTE(6,i);
		ASSIGN_BYTE(7,byContinueOpenNewBox);

		//祈愿树 i == 12
		// 0　　请求祈愿树奖品列表
		// 1　　请求验证及获得力量条等级
		// 2　　请求发奖
		if(i == 12)
		{
			if(byContinueOpenNewBox == 1 || byContinueOpenNewBox == 2)
			{
				ASSIGN_DWORD(8,g_NPC.GetGood().GetID());
				ADD_DWORD(g_NPC.GetPrayCharm().GetID());
			}
		}
	}

	SEND_GAME_SERVER();
}


void CGameControl::SEND_Guild_Member_Title_Edit(const char * content,bool bNew)
{
	size_t iLen = strlen(content);	
	if(iLen > 40960)
		iLen = 40960;	
	SET_COMMAND(CS_GUILD_MEMBER_TITLE_EDIT,40972);

	if (bNew)
		temp[6] = 0x01;

	ADD_STR(content);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Message_Edit(const char * content)
{
	size_t iLen = strlen(content);	
	if(iLen > 40960)
		iLen = 40960;

	SET_COMMAND(CS_GUILD_MESSAGE_EDIT,40972);
	ADD_STR(content);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Look_Message()
{
	SET_COMMAND(CS_GUILD_LOOK_MESSAGE,CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Look_Web()
{
	SET_COMMAND(CS_GUILD_LOOK_WEB,CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Look_Member_List()
{
	WORD wMsgID = 0;
	wMsgID = CS_GUILD_NEW_GUILD_LIST;

	SET_COMMAND(wMsgID,CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Look_Exp_Val()
{
	SET_COMMAND(CS_GUILD_LOOK_EXP_VAL,CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Delete_Member(const char * str)
{
	string tmp = str;
	StringUtil::trim(tmp);
	if(tmp.size() == 0)
		return;

	SET_COMMAND(CS_GUILD_DELETE_MEMBER,256);
	ADD_STR(tmp.c_str());
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Appoint_Dismiss(const char* pName,WORD wJob,bool bAppoint)
{
	string tmp = pName;
	StringUtil::trim(tmp);
	if (tmp.size() == 0)
		return;

	SET_COMMAND(CS_GUILD_OFF_APPDISS,256);
	(*(WORD*)(temp+6)) = wJob;
	temp[8] = (bAppoint?1:0);
	memcpy(temp+12,pName,strlen(pName));
	iBLen += strlen(pName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Online_Tael()
{
	SET_COMMAND(CS_GUILD_APPLY_TAEL,12);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_General_Protocol(WORD wProtocol)
{
	SET_COMMAND(wProtocol,12);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Guild_Add_Member(const char * str)
{
	if(GetTickCount() - SELF.GetReserveData(plyLastInviteAddGuild) < 3000)
	{
		g_TalkMgr.PushSysTalk("每3秒才能发送一次加入行会邀请");
		return;
	}

	string tmp = str;
	StringUtil::trim(tmp);
	if(tmp.size() == 0)
		return;

	SET_COMMAND(CS_GUILD_ADDIN_GUILD,256);
	ADD_STR(tmp.c_str());
	SEND_GAME_SERVER();

	SELF.SetReserveData(plyLastInviteAddGuild,GetTickCount());
}

void CGameControl::MSG_StoryBoard_URL(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
}

void CGameControl::MSG_Special_Monster_Effect(const char * strMsg,int iLen)
{
	UINT oid = 0,tid = 0;
	WORD x,y;
	BYTE  effecttype ; // 特殊效果


	memcpy(&oid,strMsg,4);    // player ID
	memcpy(&x,strMsg + 6,2);  // HI
	memcpy(&y,strMsg + 8,2);  // Low
	tid = MAKELONG(y,x); 
	memcpy(&effecttype,strMsg + 10,1); //攻击类型
	Magic_Show_s * ms = NULL;

	switch (effecttype)
	{
	case 0x01:  // 秒杀效果 100%
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_KILLED_EFFECT,0,oid,tid);
			break;
		}
	case 0x02:  // 秒杀效果 50%
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_KILLED_EFFECT_50,0,oid,tid);
			break;
		}
	case 0x04: //防御 100%
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_RECOVERY_EFFECT,0,oid,tid);
			break;
		}
	case 0x08:  // 防御 50%
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_MON_RECOVERY_EFFECT_50,0,tid,oid);
			break;
		}
	default:
		break;
	}

}


void CGameControl::MSG_MonsterAttack_Effect(const char * msg,int iLen)
{
	UINT tid = 0;
	WORD tx = 0,ty = 0;
	DWORD looks = Conv_DWORD(msg + 12);;

	DWORD oid = Conv_DWORD(msg);
	WORD x = Conv_WORD(msg + 6);  //怪物 X 坐标
	WORD y = Conv_WORD(msg + 8); //怪物 Y 坐标
	WORD attacktype = Conv_WORD(msg + 10); //攻击类型
	if(attacktype == 0 || attacktype == 2)
	{
		tx = Conv_WORD(msg + 12);  // 目标点 X
		ty = Conv_WORD(msg + 14);  // 目标点 Y
	}
	else if(attacktype == 1)
	{
		tid = Conv_DWORD(msg + 12); // 目标点 
	}

	Magic_Show_s * pMagic = NULL;
	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(oid);
	if( !p && attacktype != 2 && attacktype != 3) 
		return;
	else if(p == NULL && attacktype == 2) // 冰锥出现
	{
		looks = looks>>16;
		switch(looks)
		{
		case 231:// 冰锥
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_BZ_APPEAR_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				if(pMagic)
				{
					pMagic->ref->wOriginTileX= x;
					pMagic->ref->wOriginTileY= y;
				}
				break;
			}
		case 255: //闪电
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_AYSD_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				if(pMagic)
				{
					pMagic->ref->wOriginTileX= x;
					pMagic->ref->wOriginTileY= y;
				}
				break;
			}
		case 318://熔岩
			{				
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_RY_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				break;
			}
		case 319://冰锥7
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_BZ7_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				break;
			}
		}
		if(pMagic)
			pMagic->attr &= ~EMP_MAGIC_WAIT_SERVER;

		return;
	}
	else if(p == NULL && attacktype == 3) // 冰锥攻击效果包
	{
		looks = looks>>16;
		switch(looks)
		{
		case 231:// 冰锥
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_BZ_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				if(pMagic)
				{
					pMagic->ref->wOriginTileX= x;
					pMagic->ref->wOriginTileY= y;
				}
				break;
			}
		case 255: //暗影闪电，随机闪电
		case 237: //随机闪电，五雷轰
			{
				if (g_pGameData->FindSimpleCharacter(tid))
					pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MULTI_THUDER,EMP_MAGIC_SERVER_ADVISE,tid);
				else
					pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MULTI_THUDER,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));

				if(pMagic)
				{
					pMagic->attr &= ~EMP_MAGIC_MYSELF;
					pMagic->attr |= EMP_MAGIC_STATIC;

					pMagic->ref->wOriginTileX = x;
					pMagic->ref->wOriginTileY = y;
				}
				break;
			}
		case 238: //随机闪电，单条的雷电术
			{
				if (g_pGameData->FindSimpleCharacter(tid))
					pMagic = g_pMagicCtrl->CreateMagic(MAGICID_THUNDER,EMP_MAGIC_SERVER_ADVISE,oid,tid);
				else
					pMagic = g_pMagicCtrl->CreateMagic(MAGICID_THUNDER,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));

				if(pMagic)
				{
					pMagic->attr &= ~EMP_MAGIC_MYSELF;
					pMagic->attr |= EMP_MAGIC_STATIC;

					pMagic->ref->wOriginTileX = x;
					pMagic->ref->wOriginTileY = y;
				}
				break;
			}
		case 318://熔岩
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_RY_ATTACK,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				break;
			}
		case 319://冰锥7
			{
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_BZ7_ATTACK,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(x,y));
				break;
			}
		}

		if(pMagic)
			pMagic->attr &= ~EMP_MAGIC_WAIT_SERVER;
		return;
	}


	if(!p)
		return;

	switch(p->GetRaceNo())
	{
	case 227: //噩梦之眼
		{
			if(attacktype == 0)
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(tx,ty));
			else if(attacktype == 1)
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE,oid,tid);

			break;
		}
	case 226: // 噩梦铠魔
		{
			if(attacktype == 0)
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_MDS_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE|EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,MAKELONG(x,y),MAKELONG(tx,ty));
			else if(attacktype == 1)
				pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_MDS_ATTACK_EFFECT,EMP_MAGIC_SERVER_ADVISE,oid,tid);

			break;
		}
	case 232: // 被暗影修罗击中的效果
		{
			pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_ANYING_ATTACKED,EMP_MAGIC_SERVER_ADVISE,oid,tid);
			break;
		}
	case 233: // 被魔神修罗击中的效果
		{
			pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_MOSHEN_ATTACKED,EMP_MAGIC_SERVER_ADVISE,oid,tid);
			break;
		}
    case 249://////////////////无相天魔的吞噬效果2
		{
			pMagic = g_pMagicCtrl->CreateMagic(MAGICID_MON_DEMON_LICK_UP2,EMP_MAGIC_SERVER_ADVISE,tid,oid);
			break;
		}


	}

	if(pMagic)
		pMagic->attr &= ~EMP_MAGIC_WAIT_SERVER;

}


void CGameControl::MSG_Map_Change(const char * msg,int iLen)
{
	SELF.SetReserveData(plyJumpMap,1);
	MapArray.Clear();
	SELF.SetReserveData(pubDisableDraw,0);
	SELF.SetUsingMagicType(0);

	if(!SELF.IsDead())
		SELF.InitAction(ACTION_STAND);

	g_pAudio->StopAll();
	g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_EQUIP_LEVEL_UP_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_OTHER_BOOTH,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
	g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_BLESSCOMPOUND_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_COMPOSEEQUIP_WND,OPER_CLOSE);

	g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_FABAOLEVELUP_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_FIREARTIFICE_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_EQUIPCHAIFEN_WND, OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_HUFUFLUSH_WND,OPER_CLOSE);


	SELF.GetMapPathFinder().SetOnRoute(0);
	m_dwJumpTime = GetTickCount();

	g_AutoKillMonsterMgr.Clear();
	g_AutoPickMgr.Clear();
	g_AIPromptMgr.Clear();
	g_OtherData.SetLastMovePosTime(GetTickCount());
}

// 地图描述
void CGameControl::MSG_Map_Desc(const char * msg,int iLen)
{

}

void CGameControl::MSG_Map_Jump_Success(const char * msg,int iLen)
{
	g_pControl->Msg(MSG_CTRL_LOCATE_AREA,msg[0]);

	// 设置跨地图标志
	SELF.SetReserveData(plyJumpMap,0);
	g_OtherData.SetLastMovePosTime(GetTickCount());

// 	g_TrusteeshipData.ChangeTrusteeshipMode(eTrusteeshipModeNone,eTrusteeshipModeFight,NULL);
	
	g_pControl->PopupWindow(MSG_CTRL_COMMONCONDENSE_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_SMALL_GEM_CON_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_EQUIP_LEVEL_UP_WND,OPER_CLOSE);	
	g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_HUANHUA_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_BAOJIAN_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_InstanceZonePopUp_Wnd,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_FUSHI_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_PRODUCE_WND,OPER_CLOSE); //  [4/14/2010 dujun]关闭制炼界面
	g_pControl->PopupWindow(MSG_CTRL_BLESSCOMPOUND_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_COMPOSEEQUIP_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_FABAOLEVELUP_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_FIREARTIFICE_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_EQUIPCHAIFEN_WND, OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_HUFUFLUSH_WND,OPER_CLOSE);

	g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);

	if(strnicmp(g_pGameMap->GetMapName(),"wxxg",4) != 0)
	{
		g_pControl->PopupWindow(MSG_CTRL_WUXING_PRIZE_WND,MSG_CTRL_WUXING_PRIZE_WND_CLOSE);  //关闭发奖窗口        
	}

// 	if(strnicmp(g_pGameMap->GetMapName(),"JINLUOTANCHENG",4) != 0)
// 	{
// 		g_pGameControl->SEND_General_Protocol(CS_HEAVEN_CITY_STATUS);
// 	}
	
	if(strnicmp(g_pGameMap->GetMapName(),"DC_JGMC",7) == 0)	
	{
		g_pMagicCtrl->CreateMagic(MAGICID_TILE_CHANGE,EMP_MAGIC_NOTARGET,0,MAKELONG(40,102));
		g_pMagicCtrl->CreateMagic(MAGICID_TILE_CHANGE,EMP_MAGIC_NOTARGET,0,MAKELONG(64,156)); 
		g_pMagicCtrl->CreateMagic(MAGICID_TILE_CHANGE,EMP_MAGIC_NOTARGET,0,MAKELONG(127,132)); 
		g_pMagicCtrl->CreateMagic(MAGICID_TILE_CHANGE,EMP_MAGIC_NOTARGET,0,MAKELONG(128,70)); 
	}
	
// 	if (strcmp(g_pGameMap->GetMapName(),"SKYCITY3") == 0)
// 	{
// 		CMinMap::MExLocalNpcInfo& localNpc = g_pMinMap->GetExLocalNpc();
// 		vector<NpcInfo_s>& vNpcInfo = localNpc[g_pGameMap->GetMapName()];
// 		for (int i = 0;i<vNpcInfo.size();i++)
// 		{
// 			if (vNpcInfo[i].iExNpcType == 4)
// 			{
// 				vNpcInfo.erase(vNpcInfo.begin() + i);
// 			}
// 		}
// 
// 		MDynMiniMapObj& MiniMapObj = g_pGameData->GetDynMinMapObj();
// 		MDynMiniMapObj::iterator it = MiniMapObj.find(g_pGameMap->GetMapName());
// 		if (it != MiniMapObj.end())
// 		{
// 			std::vector<DynMiniMapObj>& VDynMinMap = it->second;
// 			for (int i = 0;i<VDynMinMap.size();i++)
// 			{
// 				if (VDynMinMap[i].wID >= 1 && VDynMinMap[i].wID <= 10)
// 				{
// 					NpcInfo_s npcInfo;
// 					npcInfo.iX = VDynMinMap[i].wMX;
// 					npcInfo.iY = VDynMinMap[i].wMY;
// 					npcInfo.iExNpcType = 4;
// 					if (VDynMinMap[i].wID >= 1 && VDynMinMap[i].wID <= 5)
// 					{
// 						npcInfo.strDesc = "由玄冰柱控制的道路机关";
// 					}
// 					else if (VDynMinMap[i].wID >= 6 && VDynMinMap[i].wID <= 10)
// 					{
// 						npcInfo.strDesc = "由离火柱控制的铁链机关";
// 					}
// 
// 					localNpc[g_pGameMap->GetMapName()].push_back(npcInfo);
// 				}
// 			}
// 		}
// 			
// 	}
// 	else
	{
		CMinMap::MExLocalNpcInfo& localNpc = g_pMinMap->GetExLocalNpc();

		CMinMap::MExLocalNpcInfo::iterator it = localNpc.find(g_pGameMap->GetMapName());
		if (it != localNpc.end())
		{
			vector<NpcInfo_s>& vNpcInfo = it->second;
			for (int i = 0;i<vNpcInfo.size();i++)
			{
				if (vNpcInfo[i].iExNpcType == 4)
				{
					vNpcInfo.erase(vNpcInfo.begin() + i);
				}
			}
		}		
	}

	//if(strnicmp(g_pGameMap->GetMapName(),"MAZE",4) != 0)	//不是迷仙阵地图
	//{
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE1);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE2);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE3);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE4);	
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE5);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE6);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE7);
	//	g_pMagicCtrl->FinishMagicByID(MAGICID_MIGONG_BGIDGE8);	
	//}

	//if (g_MapFinder.IsWalking())
	//{
	//	VPJumpPoint& vJump = g_MapFinder.GetPath();
	//	if (vJump.size() > 0)
	//	{
	//		if(stricmp((*vJump.begin())->strDesMN.c_str(),g_pGameMap->GetMapName()) != 0)
	//		{
	//			g_MapFinder.SetWalking(false);
	//			vJump.clear();				
	//		}
	//		else
	//		{
	//			vJump.erase(vJump.begin());
	//		}
	//	}

	//	if (g_MapFinder.IsWalking())
	//	{
	//		SELF.CleanOppBlock();
	//		g_MapFinder.DoNextStep();
	//	}
	//}

}
//跨gs
void CGameControl::MSG_Map_Jump(const char * msg,int iLen)
{
	//char strIP[16];
	//int iPort; 
	//char str[256]={0};
	//char temp[256]={0};

	SELF.SetReserveData(pubDisableDraw,0);
	SELF.SetUsingMagicType(0);
	g_OtherData.SetLastMovePosTime(GetTickCount());

	//出现跨地图 去掉香炉特效
	g_OtherData.SetCenserStartTime(0);
	g_OtherData.SetCenserDurTime(0);
	g_OtherData.SetCenserStart(false);

	////跨地图 清除掉摆摊物品缓存
	//g_pGameData->GetMyPetBoothData().ClearGoods();
	////跨地图 清除掉全部物品的附魔信息
	//g_pGameData->ClearDemonGoodInfo();

	//清除排行版本号
	memset(g_RangeVesion,0,16);
	ClearMyRangeData();


	//IM
	SEND_Objects_Position();

	string str;
	str.assign(msg + 12,iLen-12);
	int i = 0;

	string strIP = StringUtil::toStr(str,i);
	int iPort = StringUtil::toInt(str,i);

	m_dwJumpTime = GetTickCount();
	g_pAudio->StopAll();
	g_pWeather->EnableParticle(PARTICLE_NONE);

	g_pGameData->DelAllTroopMember();
	g_pGameData->GetGroupPos().clear();
	g_VoiceAdapter.SetTeamRoomID(0);

	//现在调GS不断开同RunGate的连接
	//g_Login.SetServer(strIP.c_str(),iPort);
	g_Login.SetMyIP(strIP.c_str());
	g_Login.SetMyPort(iPort);

	//g_pGameData->SetJump(true);
	SELF.JumpClear();
	g_pGameData->JumpClear();
	g_OtherData.JumpClear();
	SELF.GetMapPathFinder().SetOnRoute(0);
	CGoodGrid::ClearGoodFrom();

	g_AutoKillMonsterMgr.Clear();
	g_AutoPickMgr.Clear();
	g_AIPromptMgr.Clear();

	g_PetData.GetSendOutPetList().clear();

	g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_OTHER_BOOTH,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_LINGXIBOX_WND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
	g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

	g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);

    //////////清除MagicState,不然第一刀有可能会砍失败,不能直接清0
	SELF.RemoveMagicState(MS_ATTACKKILL);
	SELF.RemoveMagicState(MS_ATTACKSTICK);
	SELF.RemoveMagicState(MS_ATTACKMOON);
	SELF.RemoveMagicState(MS_ATTACKFIRE);
	SELF.RemoveMagicState(MS_REMAINSHADE);
	SELF.RemoveMagicState(MS_BLOODSHADE);
	SELF.RemoveMagicState(MS_WHOOLEMOON);
	SELF.RemoveMagicState(MS_THUNDERFIRE);
	SELF.RemoveMagicState(MS_DESTROYWEAPON);
	SELF.RemoveMagicState(MS_DESTROYSHELL);
	SELF.RemoveMagicState(MS_DESTROYSHIELD);
	SELF.RemoveMagicState(MS_DESTROYSHIELD);
	SELF.RemoveExtraState(ES_DODGEPROTECT);


	g_PetData.GetSendOutPetList().clear();
}

void CGameControl::MSG_Special_Effect(const char * msg,int iLen)
{
	int x = *((WORD*)(msg + 6));
	int y = *((WORD*)(msg + 8));
	int id  = *((WORD*)(msg + 12));	
	Magic_Show_s *ms = NULL;

	if( id != 0) 
	{
		SELF.SetReserveData(plyJumpMap,0);
		if(id == 1152)///////盛宴蛋糕 特殊处理
		{
			ms = g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y));
			if(ms)
				ms->ref->create_id = id;	
		}
		else
		{
			ms = g_pMagicCtrl->CreateMagic(1000 + id,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(x,y),MAKELONG(x,y));  
			if(ms)
				ms->ref->create_id = 1000 + id;	
		}
	}

}

void CGameControl::MSG_GS_Jump_Begin(const char * msg,int iLen)
{
	//if(!g_pGameData->GetMyJump())
	//	return;
	//g_pGameData->SetJump(false);

	g_pControl->PopupWindow(MSG_GUILD_EDIT,OPER_CLOSE);        
	g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_OTHER_BOOTH,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
	g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

	g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);

	//g_pControl->Msg(MSG_CTRL_CLEAR_MINIMAP,0);// 清除小地图

	SELF.GetMapPathFinder().SetOnRoute(0);
	g_AutoKillMonsterMgr.Clear();
	g_AutoPickMgr.Clear();
	g_AIPromptMgr.Clear();
	SELF.SetTuTengState(0);

	//MapArray.Clear();
	//g_pGameData->DelAllTroopMember();
	//g_OtherData.Clear();
	//g_OtherData.JumpClear();
	//SELF.Clear();
	//g_pGameData->DelAllTroopMember();
	g_pGameData->GetGroupPos().clear();
	SELF.SetReserveData(plyDrinkSuperRed,0);
	SELF.SetReserveData(plyDrinkSuperBlue,0);

	if(!SELF.IsDead())
	{
		SELF.InitAction(ACTION_STAND);
		SELF.SetOffset(0,0);
		SELF.SetDrawOffset(0,0);
		SELF.ClearAllNext();
	}

	//g_pGameData->SetServerIP(g_pGameData->GetMyIP());
	//g_pGameData->SetServerPort(g_pGameData->GetMyPort());
	g_Login.SetServer(g_Login.GetMyIP(),g_Login.GetMyPort());

	g_pNet->Close(SERVER_GAME);
	g_pNet->SetServer(SERVER_GAME,g_Login.GetServerIP(),g_Login.GetServerPort());
	g_pNet->Connect(SERVER_GAME);

	SET_COMMAND(0,256);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Magic_Quick_Move(const char * msg,int iLen)
{
	DWORD id = Conv_DWORD(msg);
	int x = Conv_WORD(msg+6);
	int y = Conv_WORD(msg+8);

	CCharacterAttr *pChar = NULL;

	if(id == SELF.GetID())
		pChar = &SELF;
	else
		pChar = g_pGameData->AddSimpleCharacter(x,y,id);

	if(pChar)
	{
		pChar->SetSex(msg[11]);
		pChar->SetLooks(Conv_INT64(msg+12));
		pChar->SetXY(x,y);
		pChar->SetRealXY(x,y);
		pChar->SetDir(msg[10]);
		pChar->SetAttackSpeed(Conv_WORD(msg + 20));
		pChar->SetStatus(Conv_WORD(msg+22));
		pChar->SetOffset(0,0);
		pChar->SetDrawOffset(0,0);
		pChar->InitAction(ACTION_STAND);
	}

	g_pMagicCtrl->CreateMagic(MAGICID_MOVE_QUICK,0,id);

	if(id == SELF.GetID())//主角移动
		g_AIMgr.MasterMoved();

	g_OtherData.SetLastMovePosTime(GetTickCount());
}

void CGameControl::MSG_Magic_Quick_Move_Position(const char * msg,int iLen)
{
	int x = Conv_WORD(msg+6);
	int y = Conv_WORD(msg+8);

	string strMapFile;
	strMapFile.assign(msg+12,iLen-12);

	SELF.SetXY(x,y);
	SELF.SetRealXY(x,y);
	SELF.SetOffset(0,0);
	SELF.SetDrawOffset(0,0);

	//g_pGameData->JumpClear();
	g_AutoKillMonsterMgr.Clear();
	g_AutoPickMgr.Clear();
	g_AIPromptMgr.Clear();
	SELF.GetMapPathFinder().SetOnRoute(0);

	SELF.SetReserveData(pubGradualAppear,0);
	SELF.SetReserveData(pubGradualTime,0);

	if(!SELF.IsDead())
	{
		SELF.InitAction(ACTION_STAND);
		SELF.ClearAllNext();
	}

	if(strMapFile != g_pGameMap->GetMapName()) //切换了新的地图
	{
		//g_pWeather->m_SeaTideList.Clear(); 
		g_pGameData->ChangeMapClear(strMapFile.c_str());//切换了地图
		g_pMagicCtrl->FinishAllMagic(true);

		g_pGameMap->LoadMap(strMapFile.c_str());
		g_pGameControl->DoOverMapFindPath();
		g_pGameControl->SEND_Open_MapView();
	}
	else
	{
		if(strnicmp(g_pGameMap->GetMapName(),"wxxg",4) == 0)	//五行玄关跳转点
		{
			g_pMagicCtrl->CreateMagic(MAGICID_MOVE_QUICK,0,SELF.GetID());
		}

		g_AIMgr.MasterMoved();
	}

	g_OtherData.ClearAllFireShowList();
	g_OtherData.SetLastMovePosTime(GetTickCount());
}

void CGameControl::MSG_Object_Dura(const char * msg,int iLen)
{
	int pos = Conv_WORD(msg + 6);
	DWORD dwDura = Conv_DWORD(msg);
	DWORD dwDuramax = Conv_DWORD(msg + 8);
	char str[256] = {0};

	CGood* pGood = SELF.EquipGood().GetPtr(pos);
	if(pGood)
	{
		if( (pos == 5 || pos == 12) && pGood->GetStdMode() == 34 )		// 护身符
		{
			if(pGood->GetAC() == 0)
			{
				if(pGood->GetDura() > 500 && dwDura <= 500)
					sprintf(str,"您的道符快要用完了！");
			}
			else
			{
				if(pGood->GetDura() > 5 && dwDura <= 5)
					sprintf(str,"您的%s快要用完了！",pGood->GetName());
			}
		}
		else if((pos == 5 || pos == 12) && pGood->GetStdMode() == 25)
		{
			if(pGood->GetAC() == 0)
			{
				if(pGood->GetDura()> 300 && dwDura <= 300)
				{
					if(pGood->GetLooks() < 270 || pGood->GetLooks() == 834 || pGood->GetLooks() == 835)
						sprintf(str,"您的毒粉快要用完了！");
					else
						sprintf(str,"您的稻草人偶快要用完了！");
				}
			}
			else
			{
				if(pGood->GetDura()> 3 && dwDura <= 3)
					sprintf(str,"您的%s快要用完了！",pGood->GetName());
			}
		}
		else if(pos == 2 && pGood->GetStdMode() == 33 && pGood->GetShape() == 1)
		{

		}
		else
		{
			if(pGood->GetDura() >= 1500 && dwDura < 1500)
			{
				//CheckUseBCSS(*pGood,pos);
				sprintf(str,"您的%s持久磨损过度，请速将其进行修理！",pGood->GetName());
			}
			else if(pGood->GetDura() >= 500 && dwDura < 500)
			{
				//CheckUseBCSS(*pGood,pos);
				sprintf(str,"您的%s即将损坏，请速将其进行修理！",pGood->GetName()); 
			}
		}

		pGood->SetDura((WORD)dwDura);
		pGood->SetDuraMax((WORD)dwDuramax);

		if(strlen(str) > 0)
			g_TalkMgr.PushSysTalk(str);
	}
}

// 跳转地图后的刷新
void CGameControl::MSG_Jump_Refresh(const char * msg,int iLen)
{
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)msg;
	DWORD id = lpPacketMsg->stDefMsg.nRecog;
	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	BYTE bDir = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	BYTE bGender = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	TCHARDESC	stCharDesc;
	memcpy(&stCharDesc, lpPacketMsg->szEncodeData, sizeof(TCHARDESC));

	//DWORD id = Conv_DWORD(msg);
	//int x = Conv_WORD(msg+6);
	//int y = Conv_WORD(msg+8);

	CCharacterAttr *pChar = NULL;

	if(id == SELF.GetID())
	{
		pChar = &SELF;
		g_AutoPickMgr.ItemPickup(x,y);
	}
	else
	{
		pChar = g_pGameData->AddSimpleCharacter(x,y,id);
	}

	if(pChar)
	{
		pChar->SetXY(x,y);
		pChar->SetRealXY(x,y);
		pChar->SetDir(bDir);
		pChar->SetSex(bGender);
		pChar->SetLooks(stCharDesc.nFeature);
		if (!pChar->IsOnHorse())
		{
			pChar->SetFightOnLeopard(false);
			pChar->SetRunStepOnLeopard(3);
		}

		//pChar->SetAttackSpeed(Conv_WORD(msg + 20));
		pChar->SetStatus(stCharDesc.nStatus);
	}

	g_pMagicCtrl->CreateMagic(MAGICID_MOVE_QUICK,0,id);

	if(id == SELF.GetID() && !g_MapFinder.IsWalking()) //主角移动
		g_AIMgr.MasterMoved();

	g_pAudio->Play(EAT_OTHER,14,g_pAudio->GetRand()++);
}

void CGameControl::SEND_ShortCut_Key_Change(WORD magID,char key)
{
	SET_COMMAND(CS_SHORTCUT_KEY_CHANGE,12);
	ASSIGN_WORD(0,magID);
	ASSIGN_BYTE(6,key);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Guild_Message_Fail(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("你还没有加入行会，你可直接找寻中州皇宫内的国王申请创建宗族，随后再创建属于你自己的行会！");
}

void CGameControl::MSG_Guild_Message(const char * msg,int iLen)
{
	char * strMsg1 = new char[iLen + 1];
TRY_BEGIN
		memcpy(strMsg1,msg,iLen);
		strMsg1[iLen] = 0;

		string str = strMsg1 + 12;
		int nLength = str.find_first_of(0x0d);

		g_GuildData.SetHeader((strMsg1[12 + nLength + 3] == 0x30)?false:true);

		g_GuildData.SetGuildName(str.substr(0,nLength).c_str());

		int nBegin	= str.find("<Notice>");
		nBegin += 8;
		int nEnd	= str.find("<KillGuilds>",nBegin);


		g_GuildData.GetBulletin().clear();
		g_GuildData.GetWeb().clear();
		g_GuildData.SetBulletin("行会公告");
		g_GuildData.SetBulletin(" ");

		int i = nBegin + 1;
		while(i < nEnd)
		{
			int n = str.find_first_of(0x0d,i);
			if( n > nEnd || n == -1)
			{
				g_GuildData.SetBulletin(str.substr(i,nEnd-i).c_str());
			}
			const char * p = str.substr(i,n-i).c_str();
			g_GuildData.SetBulletin(str.substr(i,n-i).c_str());
			i = n + 1;
		}
		for(int ttt = 0;ttt < g_GuildData.GetBulletin().size(); ttt ++)
		{
			char* str = (char*)g_GuildData.GetBulletin()[ttt].c_str();
			CutByUnicode(str,g_GuildData.GetWeb(),52);
		}
		g_GuildData.SetBulletin(" ");
		nBegin	= nEnd + 12;
		nEnd	= str.find("<AllyGuilds>",nBegin);
		i = nBegin;

		g_GuildData.GetKillAlly().clear();
		g_GuildData.GetAlly().clear();

		while(i < nEnd)
		{
			int n = str.find_first_of(0x0d,i);
			if( n > nEnd || n == -1)
			{
				g_GuildData.SetKillAlly(str.substr(i,nEnd-i).c_str());
			}
			g_GuildData.SetKillAlly(str.substr(i,n-i).c_str());
			i = n + 1;
		}

		nBegin	= nEnd + 12; 
		nEnd = str.size();
		i = nBegin;

		while(i < nEnd)
		{
			int n = str.find_first_of(0x0d,i);
			if( n > nEnd || n == -1)
			{
				g_GuildData.SetAlly(str.substr(i,nEnd-i).c_str());
			}
			g_GuildData.SetAlly(str.substr(i,n-i).c_str());
			i = n + 1;
		}

		g_GuildData.GetTempBulletin().clear();
		{
			for(unsigned int i = 0; i < g_GuildData.GetBulletin().size(); i++)
				g_GuildData.GetTempBulletin().push_back(g_GuildData.GetBulletin()[i]);
		}
		{
			for(unsigned int i = 0; i < g_GuildData.GetAlly().size(); i++)
				g_GuildData.GetTempBulletin().push_back(g_GuildData.GetAlly()[i]);
		}	
TRY_END

	delete[] strMsg1;
}

void CGameControl::MSG_Guild_Create_Success(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("建立行会成功！");

	if(g_pSDOAInterface)
		g_pSDOAInterface->OnCommunityEvent(CE_CREATE_GUILD,SELF.GetTitle());
}

void CGameControl::MSG_Guild_Create_Fail(const char * msg,int iLen)
{
	string str;
	DWORD dwCode = *((DWORD *)msg);

	switch(dwCode)
	{
	case 0xFFFFFFFF:
		str = "已加入行会！";
		break;
	case 0xFFFFFFFE:
		str = "缺少创建费用！";
		break;
	case 0xFFFFFFFD:
		str = "缺少必要物品！";
		break;
	case 0xFFFFFFFC:
		str = "行会名输入不正确或者行会名已存在！";
		break;
	case 0xFFFFFFFB:
		str = "你没有准备好需要的全部物品！";
		break;
	default:
		str = "未知错误！";
		break;
	}
	g_MsgBoxMgr.PopSimpleAlert(str.c_str());

}

void CGameControl::MSG_Guild_Delete_Member_Failed(const char * msg,int iLen)
{
	if(msg[0] == 0x02)
	{
		g_MsgBoxMgr.PopSimpleAlert("不是本行会成员！");
	}
}

void CGameControl::MSG_Guild_Addin_Guild_Failed(const char * msg,int iLen)
{
	string str;
	switch(msg[0])
	{
	case 0x5:
		str = "对方不允许加入行会！";
		break;
	case 0x02:
		str = "想加入行会的成员应该面对掌门人！";
		break;
	case 0x07:
		str = "想加入行会的成员应该在掌门人附近！";
		break;
	case 0x03:
		str = "他已经加入我们行会！";
		break;
	case 0x06:
		str = "对方正在接受邀请！";
		break;
	case 0x04:
		str = "对方已经加入其它行会！";
		break;
	case 0x08:
		str = "不能邀请元神分身加入行会！";
		break;
	}

	if(!str.empty())
	{
		g_MsgBoxMgr.PopSimpleAlert(str.c_str());
	}

	g_GuildData.SetGuildDelName("");
}
void CGameControl::MSG_Phyle_AddOrExit(const char * strMsg,int iLen)
{
	//宗派操作
	int iPos = 0;
	DWORD dwOperate = *(DWORD*)strMsg;
	string strName;
	strName.assign(strMsg + 12,iLen - 12);
	string strPhyleName = StringUtil::toStr(strName,iPos);
	string strOperateName = strName.substr(iPos);
	if (strOperateName.length() <= 0 || strPhyleName.length() <= 0) return;

	string& selfphyleName = g_OtherData.GetPhyleName();
	if(strcmp(selfphyleName.c_str(),strPhyleName.c_str()) == 0)
	{
		bool bFound = false;
		VRelationStruct& relationData = g_OtherData.GetRelationVector();
		for(int i = relationData.size() - 1;i >= 0;i--)
		{		
			DWORD dwMyRelationType = relationData[i].iRelType;
			if (strcmp(relationData[i].strName.c_str(),strOperateName.c_str()) == 0)
			{
				bFound = true;
				if (dwOperate == 1)
				{
					relationData[i].AddRelationType(RT_PHYLE);
					relationData[i].byPhyleType = (BYTE)(*((WORD*)(strMsg + 6)));
				}
				else
				{
					relationData[i].RemoveRelationType(RT_PHYLE);
					if (relationData[i].iRelType == 0)
						relationData.erase(relationData.begin() + i);
				}

				if (dwOperate == 0 && strOperateName.compare(SELF.GetName()) == 0)
				{
					for(int j = relationData.size() - 1;j >= 0;j--)
					{
						relationData[j].RemoveRelationType(RT_PHYLE);
						if (relationData[j].iRelType == 0)
							relationData.erase(relationData.begin() + j);
					}

					selfphyleName.clear();					
				}
				break;
			}
		}

		if(!bFound && dwOperate == 1)//增加
		{
			//增加数据
			_RelationStruct relation;
			relation.strName = strOperateName;
			relation.iJob = *(strMsg + 11);
			relation.iLevel = (BYTE)(*(strMsg + 10));
			relation.iOnline = 1;
			relation.iRelType = RT_PHYLE;
			relation.byPhyleType = (BYTE)(*((WORD*)(strMsg + 6)));
			g_OtherData.GetRelationVector().push_back(relation);

			//新加社会关系,社会关系按钮闪烁
			if (!g_pControl->MsgToWnd("RelationWnd",MSG_CTRL_FLASH_BTN_WND,4))
			{
				g_OtherData.SetFlashRelationTabType(4);
			}
		}
	}

	GuildPhyleEle& guildphylemap = g_PhyleData.GetGuildPhyleMap();
	std::map<std::string,CPhyleData::sPhyleMem>& phyEle = g_PhyleData.GetPhyle();
	if (phyEle.size() <= 0)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,20);
		g_pControl->Msg(MSG_CTRL_RELATION_WND,21);
		return;
	}

	std::map<std::string,CPhyleData::sPhyleMem>::iterator it = phyEle.find(strPhyleName);
	if (it != phyEle.end())
	{
		std::vector<CPhyleData::sPhyleElement>& element = it->second.vPhyleMem;
		if (dwOperate == 0)//退出宗派
		{
			std::vector<CPhyleData::sPhyleElement>::iterator itTemp = element.begin();
			while (itTemp != element.end())
			{
				if (strcmp(itTemp->strName.c_str(),strOperateName.c_str()) == 0)
				{
					it->second.vPhyleMem.erase(itTemp);
					if (it->second.vPhyleMem.size() <= 1)
						phyEle.erase(it);
					break;
				}
				itTemp++;
			}
			GuildPhyleEle::iterator itTemp1 = guildphylemap.find(strOperateName);
			if (itTemp1 != guildphylemap.end()) guildphylemap.erase(itTemp1);
		}
		else if (dwOperate == 1)//加入宗派
		{
			CPhyleData::sPhyleElement ele;
			ele.strName = strOperateName;
			ele.byPhyleType[0] = (BYTE)(*((WORD*)(strMsg + 6)));
			element.push_back(ele);

			PHYLEMEMBER phyle;
			phyle.strName = strPhyleName;
			guildphylemap[strOperateName];
		}
	}

	g_pControl->Msg(MSG_CTRL_RELATION_WND,20);
	g_pControl->Msg(MSG_CTRL_RELATION_WND,21);
}

void CGameControl::MSG_Phyle_Guild_AddOrExit(const char * strMsg,int iLen)
{
	//宗派
	DWORD dwOperate = *(DWORD*)strMsg;
	string strName;
	strName.assign(strMsg + 12,iLen - 12);
	std::map<std::string,CPhyleData::sPhyleMem>& phyEle = g_PhyleData.GetPhyle();
	GuildPhyleEle& guildphylemap = g_PhyleData.GetGuildPhyleMap();
	GuildPhyleEle::iterator itMap;
	if (dwOperate == 0)//删除
	{		
		if (strName.length() > 0)
		{
			std::map<std::string,CPhyleData::sPhyleMem>::iterator it = phyEle.find(strName);
			if (it == phyEle.end()) return;

			std::vector<CPhyleData::sPhyleElement>& phylemem = it->second.vPhyleMem;

			for (int i = 0;i<phylemem.size();i++)
			{
				itMap = guildphylemap.find(phylemem[i].strName);
				if (itMap != guildphylemap.end())
				{
					guildphylemap.erase(itMap);
				}
			}

			if (it!= phyEle.end())
			{//找到				
				phyEle.erase(it);
			}
		}		
	}
	else if (dwOperate == 1)//增加
	{
		int iBegin = 0,iEnd = strName.length();
		string strPhyleName = StringUtil::toStr(strName,iBegin);
		std::map<std::string,CPhyleData::sPhyleMem>::iterator it = phyEle.find(strPhyleName);
		if (it != phyEle.end())
			phyEle.erase(it);

		CPhyleData::sPhyleMem phyele;
		int iFlag = 0;
		while(iBegin < iEnd)
		{			
			CPhyleData::sPhyleElement ele;
			PHYLEMEMBER phyle;
			ele.strName = StringUtil::toStr(strName,iBegin);
			ele.byPhyleType[0] = (BYTE)(StringUtil::toUInt(strName,iBegin) - '0');
			phyele.vPhyleMem.push_back(ele);

			if (iFlag < 2) iFlag++;
			if (iFlag == 1) phyle.byType = 1;
			phyle.strName = strPhyleName;
			guildphylemap[ele.strName] = phyle;
		}

		if (iBegin >= iEnd)
			phyEle[strPhyleName] = phyele;
	}

	g_pControl->Msg(MSG_CTRL_RELATION_WND,21,NULL);
}

void CGameControl::MSG_Guild_Delete_Member_Success(const char * msg,int iLen)
{
	const char* p =  g_GuildData.GetGuildDelName();
	if(!p)  return;

	if(strcmp(SELF.GetName(),p) == 0)
	{
		g_GuildData.GetMember().clear();
		g_GuildData.GetWeb().clear();
		g_TalkMgr.GetGuildTalk().clear();
		g_GuildData.GetKillAlly().clear();
		g_GuildData.GetAlly().clear();

		//清除一些相关信息
		g_GuildData.GetGuildMemberOfficer().clear();
		g_GuildData.GetGuildMemberNormal().clear();
		g_GuildData.SetGuildTowerLevel(0);
		g_GuildData.SetGuildName("");
		g_GuildData.SetChairMan("");
		g_GuildData.GetGuildOfficerMap().clear();
		g_GuildData.GetOfficerMemberMap().clear();
		memset( &(g_GuildData.GetGuildBuffer()),0,sizeof(g_GuildData.GetGuildBuffer()));
		g_GuildData.SetGuildTael(0);
		g_GuildData.GetGuildTower().dwTaelPub = 0;
		g_GuildData.SetGuildLevel("0");
		g_GuildData.SetGuildFunc(0);
		memset(&(g_GuildData.GetGuildBuffer()),0,sizeof(g_GuildData.GetGuildBuffer()));

		MiniChar& _chars = MapArray.GetMiniChar();//清除相邻玩家的行会关系
		MiniChar::iterator it;
		for(it = _chars.begin(); it != _chars.end(); it++)
		{
			CSimpleCharacter* pChar = it->second;
			if(NULL == pChar || !pChar->IsHuman()) 
				continue;

			if(strcmp( SELF.GetTitle(), pChar->GetGuildName()) == 0)
			{
				pChar->RemoveRelationType(RT_GUILD);
			}
		}
		g_GuildData.SetGuildDelName("");
		return;
	}

	bool bRet =false;	


	//采用新的行会数据进行处理
	bool bFound = false;
	std::vector<CGuildData::sTileNode>* pNodeList = NULL;
	for (int k = 0;!bFound && k<2;k++)
	{
		if (k ==0)
			pNodeList = &(g_GuildData.GetGuildMemberOfficer());
		else pNodeList = &(g_GuildData.GetGuildMemberNormal());

		for (int i = 0;!bFound && i< pNodeList->size();i++)
		{
			//
			CGuildData::sTileNode& sNode = (*pNodeList)[i];
			for (int j = 0;!bFound && j< sNode.vMember.size();j++)
			{
				CGuildData::_NewMember& newMem = sNode.vMember[j];
				if (newMem.strName.compare(p) == 0)
				{
					bFound = true;
					if (!newMem.bAssist || newMem.strAssist.length() <=0)
					{//可以删除此结点							
						sNode.vMember.erase(sNode.vMember.begin() + j);
					}
				}
				else if (newMem.strAssist.compare(p) == 0)
				{
					newMem.strAssist = "";
					newMem.bAssist = false;
					if (newMem.strName.length() <= 0)
					{//可以删除此结点
						sNode.vMember.erase(sNode.vMember.begin() + j);
					}
				}

				if (sNode.vMember.size() <= 0)
				{//删除封号
					pNodeList->erase(pNodeList->begin() + i);
				}

				CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(const_cast<char*>(p));
				if(pChar) pChar->RemoveRelationType(RT_GUILD);
				break;					
			}
		}
	}			

	g_GuildData.SetGuildDelName("");

	CCtrlWindow* pWnd = g_pControl->FindWindowByName("RelationWnd");
	if (pWnd)
	{
		pWnd->Msg(MSG_CTRL_GUILD_CHANGE_INFO,0,NULL);
	}
}

//旧ID 8 9 
//新ID 6 7
//封号名
//玩家名
void CGameControl::Msg_Guild_Change_Tile(const char* msg,int iLen)
{
	if (g_GuildData.GetGuildMemberOfficer().size() <= 0)
		return;

	//行会成员改变封号协议
	WORD wOldJob = *((WORD*)(msg+8));
	WORD wNewJob = *((WORD*)(msg + 6));
	WORD wPhyle = *((WORD*)(msg + 10));
	int iStart = 0;

	string sPlayerName = StringUtil::toStr(msg + 12,iStart);
	if (sPlayerName.compare(SELF.GetName()) == 0)
		g_GuildData.SetGuildFunc(*((DWORD*)(msg)));

	string sTileName = StringUtil::toStr(msg + 12,iStart);
	string sMasterName = StringUtil::toStr(msg + 12,iStart);

	//////////////////////////////////////////////////////////////////////////
	std::map<int ,CGuildData::sOfficer>::const_iterator it;
	it = g_GuildData.GetGuildOfficerMap().find(wOldJob);
	if (it != g_GuildData.GetGuildOfficerMap().end())
	{
		std::string::size_type iPos = it->second.strTile.find("亲信");
		if (iPos != std::string::npos)
			wOldJob--;
	}

	WORD wNewJob1 = wNewJob;
	it = g_GuildData.GetGuildOfficerMap().find(wNewJob);
	if (it != g_GuildData.GetGuildOfficerMap().end())
	{
		std::string::size_type iPos = it->second.strTile.find("亲信");
		if (iPos != std::string::npos)
			wNewJob1--;
	}

	bool bFound = false;
	//1、删除
	std::vector<CGuildData::sTileNode>* pNodeList = NULL;

	string strTemp;
	bool bOnline = false;
	if (wOldJob > 0)
	{	
		if (wOldJob <= 10) pNodeList = &(g_GuildData.GetGuildMemberOfficer());
		else			   pNodeList = &(g_GuildData.GetGuildMemberNormal());

		for (int i= 0;!bFound && i< pNodeList->size();i++)
		{
			if ((*pNodeList)[i].wNum == wOldJob)
			{//找到						
				for (int j = 0;!bFound && j < (*pNodeList)[i].vMember.size();j++)
				{
					CGuildData::_NewMember& guildMem = (*pNodeList)[i].vMember[j];
					if (guildMem.bAssist)
					{////////////有助手

						if(guildMem.strName.compare(sPlayerName.c_str()) == 0)
						{
							bFound = true;
							bOnline =guildMem.bOnline;							
							if (guildMem.strAssist.length() == 0)
							{
								(*pNodeList)[i].vMember.erase((*pNodeList)[i].vMember.begin() + j);
							}
							else
							{
								guildMem.strName = "";								
							}
						}
						else if (guildMem.strAssist.compare(sPlayerName.c_str()) == 0)
						{//删除的是助手
							bFound = true;
							guildMem.strAssist = "";
							bOnline = guildMem.bAssOnline;
						}					
					}
					else
					{//没有助手
						if (guildMem.strName.compare(sPlayerName.c_str()) == 0)
						{
							bFound = true;
							bOnline = guildMem.bOnline;
							(*pNodeList)[i].vMember.erase((*pNodeList)[i].vMember.begin() + j);
						}					
					}				
				}

				CGuildData::sTileNode& TileNode = (*pNodeList)[i];
				if (TileNode.vMember.size() == 0)
				{//删除					
					pNodeList->erase(pNodeList->begin() + i);
				}
			}
		}
	}

	//如果是新加入的，则在线，否则用以前的
	if (wOldJob == 0 && wNewJob != 0)
		bOnline = true;

	if (wNewJob > 0)
	{
		if (wNewJob <= 10) pNodeList = &(g_GuildData.GetGuildMemberOfficer());
		else			   pNodeList = &(g_GuildData.GetGuildMemberNormal());

		if (wNewJob == 1)//行会会长
			g_GuildData.SetChairMan(sPlayerName);

		bool bFinish = false;
		for (int i = 0;i<pNodeList->size() && !bFinish;i++)
		{
			if((*pNodeList)[i].wNum == wNewJob1)
			{
				string strName;
				if (wNewJob1 == wNewJob)
				{//不是亲信
					if (sTileName.length() > 0)
						(*pNodeList)[i].sTile = sTileName;

					CGuildData::_NewMember newMem;
					newMem.strName = sPlayerName;
					newMem.bOnline = bOnline;
					if (wNewJob < 10)//香主及以后都没有助手
						newMem.bAssist = true;

					newMem.byPhyle = (BYTE)wPhyle;
					(*pNodeList)[i].vMember.push_back(newMem);
					bFinish = true;
				}
				else
				{//亲信
					std::vector<CGuildData::_NewMember>& vMember = (*pNodeList)[i].vMember;
					for (int j = 0;j< vMember.size();j++)
					{
						CGuildData::_NewMember& newMem = vMember[j];
						if (newMem.strName.compare(sMasterName.c_str()) == 0)
						{//找到
							newMem.strAssist = sPlayerName;
							newMem.bAssOnline = bOnline;
							newMem.byPhyle = (BYTE)wPhyle;
							bFinish = true;
						}
					}
				}
				break;
			}
		}

		if (!bFinish)
		{//没有找到，则为新的职位
			int iPos = -1;
			for (int i= 0;i< pNodeList->size();i++)
			{
				if (wNewJob > (*pNodeList)[i].wNum)
					continue;

				bFinish = true;
				iPos = i;
				break;
			}

			CGuildData::sTileNode sNewJob;
			sNewJob.wNum = wNewJob;
			sNewJob.sTile = sTileName;
			CGuildData::_NewMember newMember;
			newMember.bOnline = bOnline;
			newMember.byPhyle = (BYTE)wPhyle;
			newMember.strName = sPlayerName;
			if (wNewJob < 10)
			{
				newMember.bAssist = true;
			}
			sNewJob.vMember.push_back(newMember);
			if (!bFinish)
			{
				pNodeList->push_back(sNewJob);
			}
			else
			{
				pNodeList->insert(pNodeList->begin() + iPos,sNewJob);
			}
		}
	}

	//重新构造数据
	if (g_pControl)
	{
		CCtrlWindow* pWnd = g_pControl->FindWindowByName("RelationWnd");
		if (pWnd)
		{
			pWnd->Msg(MSG_CTRL_GUILD_CHANGE_INFO,0,NULL);
		}
	}
}

void CGameControl::Msg_Guild_DevoteResource_Result(const char* msg,int iLen)
{
	bool bSucess = ((*(WORD*)(msg + 6)) != 0);
	DWORD dwID = *(DWORD*)(msg);
	CGoodArray& good = SELF.GetDevGuildGood();
	CGood* pGood = good.FindGood(dwID);
	if (pGood && bSucess)
	{
		pGood->SetID(0);
	}
}

//在线成员行会银两，按照银两数目从大到小的顺序排列
void CGameControl::Msg_Guild_Online_Tael(const char* msg,int iLen)
{
	typedef std::vector<CGuildData::sGuildTael> VGuildTael;
	VGuildTael& guildTael = g_GuildData.GetGuildOnlineTael();
	guildTael.clear();

	int nBegin = 0;
	while ( nBegin < iLen - 12)
	{
		CGuildData::sGuildTael stael;
		stael.strName = StringUtil::toStr(msg + 12,nBegin);
		stael.dwTael  = StringUtil::toInt(msg + 12,nBegin);

		//插入数据
		VGuildTael::iterator it = guildTael.begin();
		bool bDealed = false;
		while(!bDealed && it != guildTael.end())
		{
			if (stael.dwTael > it->dwTael)
			{
				guildTael.insert(it,stael);
				bDealed = true;
			}
			else it++;
		}

		if (!bDealed)
		{
			guildTael.push_back(stael);
		}
	}
}

void CGameControl::Msg_Guild_Person_Tael(const char* msg,int iLen)
{
	//行会个人银两
	DWORD dwTael = *((DWORD*)msg);
	g_GuildData.SetGuildTael(dwTael);
}

//行会改变银两协议
void CGameControl::Msg_Guild_Change_Tael(const char* msg,int iLen)
{
	int Temp = *((int*)msg);
	if (iLen > 12)
	{
		std::vector<CGuildData::sGuildTael>& guildOnlinTael = g_GuildData.GetGuildOnlineTael();

		for (int i = 0;i<guildOnlinTael.size();i++)
		{
			CGuildData::sGuildTael& guildtael = guildOnlinTael[i];
			if(guildtael.strName.compare(msg + 12) == 0)
			{
				//相同
				int iTael = guildtael.dwTael;
				iTael += Temp;
				if (iTael < 0)
					iTael = 0;
				guildtael.dwTael = iTael;
				char cTemp[256] = {0};
				sprintf(cTemp,"已给%s分配了%d银两",msg+12,Temp);
				g_TalkMgr.PushSysTalk(cTemp);
			}
		}
	}
}
//行会成员上下线协议
void CGameControl::Msg_Guild_Member_OnOffLine(const char* msg,int iLen)
{
	//0-3封号
	//6-7上下线标志 0 下线，1上线
	//12玩家姓名
	CGuildData::sTileNode sNode;
	DWORD dwTile = *((DWORD*)msg);
	WORD  wOnOrOffline = *((WORD*)(msg + 6));
	std::vector<CGuildData::sTileNode>* pNodeList = NULL;
	if (dwTile <= 10) pNodeList = &(g_GuildData.GetGuildMemberOfficer());
	else			  pNodeList = &(g_GuildData.GetGuildMemberNormal());

	string strTemp;
	char ctemp[128] = {0};
	bool bDealed = false;
	TRY_BEGIN
		for (int i= 0;!bDealed && i<pNodeList->size();i++)
		{
			std::vector<CGuildData::_NewMember>& vMember = (*pNodeList)[i].vMember;
			for (int j = 0;!bDealed && j < vMember.size();j++)
			{
				CGuildData::_NewMember& newMem = vMember[j];

				if (newMem.strName.compare(msg + 12) == 0)
				{//找到					
					newMem.bOnline = ((1 - wOnOrOffline) == 0);
					bDealed = true;
					sNode.wNum = (WORD)dwTile;
					CGuildData::_NewMember mem;
					mem.bOnline = newMem.bOnline;
					mem.strName = newMem.strName;
					sNode.vMember.push_back(mem);
				}

				if (!bDealed && newMem.bAssist)
				{
					if (newMem.strAssist.compare(msg + 12) == 0)
					{
						newMem.bAssOnline = ((1 - wOnOrOffline) == 0);
						bDealed = true;
						sNode.wNum = (WORD)dwTile;
						CGuildData::_NewMember mem;
						mem.bAssist = true;
						mem.bAssOnline = newMem.bAssOnline;
						mem.strAssist = newMem.strAssist;
						sNode.vMember.push_back(mem);						
					}
				}
			}
		}
TRY_END

	if (bDealed && g_pControl)
	{
		CCtrlWindow* pWnd = g_pControl->FindWindowByName("RelationWnd");
		if (pWnd)
		{			
			pWnd->Msg(MSG_CTRL_GUILD_CHANGE_INFO,1,(CControl*)&sNode);
		}
	}
}
//行会塔信息
void CGameControl::Msg_Guild_Resource_Owner(const char* msg,int iLen)
{
	if (iLen < 32) return;

	CGuildData::sGuildTower& guildTower = g_GuildData.GetGuildTower();
	guildTower.dwTaelPub =  *((DWORD*)(msg));
	guildTower.wFlag = *((WORD*)(msg + 6));

	for (int i = 0;i<5;i++)
	{
		guildTower.dwResource[i] = *((DWORD*)(msg + 12 + i*4));
		guildTower.dwResourceMax[i] = *((DWORD*)(msg + 32 + i*4));
	}
}

//SC_GUILD_PHYLE_LIST//行会宗派列表，增加一个宗派映射
void CGameControl::MSG_Guild_Phyle_List(const char * msg, int iLen)
{
	string str;
	str.assign(msg + 12,iLen - 12);
	int i = 0;
	int nBegin;
	int nEnd;
	std::map<std::string,CPhyleData::sPhyleMem>& phyleMem = g_PhyleData.GetPhyle();
	GuildPhyleEle& guildPhyleMap = g_PhyleData.GetGuildPhyleMap();
	phyleMem.clear();
	TRY_BEGIN
		while ( i < iLen - 12)
		{
			nBegin	= str.find_first_of('#',i);
			if(nBegin == -1)
				break;
			nBegin ++;
			nEnd	= str.find_first_of('#',nBegin);
			if(nEnd == -1)
				nEnd = str.size();
			string strPhyleName = StringUtil::toStr(str,nBegin);
			CPhyleData::sPhyleMem ele;
			int iFlag = 0;
			while(nBegin < nEnd)
			{
				CPhyleData::sPhyleElement element;
				PHYLEMEMBER phyleMem;
				element.strName = StringUtil::toStr(str,nBegin);
				element.byPhyleType[0] = StringUtil::toUInt(str,nBegin);
				ele.vPhyleMem.push_back(element);
				if(iFlag < 2) iFlag++;
				if (iFlag == 1) phyleMem.byType = 1;
				phyleMem.strName = strPhyleName;
				guildPhyleMap[element.strName] = phyleMem;
			}
			phyleMem[strPhyleName] = ele;
			i = nEnd;
		}
		g_pControl->Msg(MSG_CTRL_RELATION_WND,21,NULL);
		TRY_END
}
//新的行会成员列表
void CGameControl::MSG_Guild_Member_List_New(const char * msg, int iLen)
{
	char * strMsg1 = new char[iLen + 1];
	TRY_BEGIN
		memcpy(strMsg1,msg,iLen);
		strMsg1[iLen] = 0;
		string str = strMsg1 + 12;
		int i = 0;
		int nBegin;
		int nEnd;
		bool bGuildTower = (g_GuildData.GetGuildTowerLevel() > 0);
		string strTemp;
		g_GuildData.GetGuildMemberNormal().clear();
		g_GuildData.GetGuildMemberOfficer().clear();
		g_GuildData.GetOfficerMemberMap().clear();
		std::vector<CGuildData::sTileNode>* pNodeList = NULL;
		g_GuildData.SetNewGuildData(true);

		std::map<int,CGuildData::sOfficer>& mOfficer = g_GuildData.GetGuildOfficerMap();
		while ( i < iLen - 12)
		{
			nBegin	= str.find_first_of('#',i);
			if(nBegin == -1)
				break;
			nBegin ++;
			nEnd	= str.find_first_of('#',nBegin);
			if(nEnd == -1)
				nEnd = str.size();
			CGuildData::sTileNode temp;
			int iTemp = StringUtil::toInt(str,nBegin);
			temp.wNum = iTemp;

			if(bGuildTower)
			{
				if (mOfficer.find(iTemp) != mOfficer.end())
					pNodeList = &(g_GuildData.GetGuildMemberOfficer());
				else pNodeList = &(g_GuildData.GetGuildMemberNormal());
			}
			else
			{
				if (iTemp <= 10)
					pNodeList = &(g_GuildData.GetGuildMemberOfficer());
				else pNodeList = &(g_GuildData.GetGuildMemberNormal());
			}

			(*pNodeList).push_back(temp);
			iTemp = pNodeList->size() - 1;
			nBegin ++;
			(*pNodeList)[iTemp].sTile = StringUtil::toStr(str,nBegin);
			while(nBegin < nEnd)
			{
				CGuildData::_NewMember newMem;
				string strName = StringUtil::toStr(str,nBegin);
				char cOnline = strName[0];
				newMem.bOnline = (('1' - cOnline) == 0);//是否在线
				newMem.byPhyle = (strName[1] > '2'?strName[1] - '2':0);

				string::size_type ipos = strName.find_first_of('*');
				if (ipos != string::npos)
				{//找到，则有助手
					newMem.strName = strName.substr(2,ipos - 2);

					if (ipos + 1 == strName.length())
					{
						newMem.bAssOnline = false;
					}
					else
					{
						newMem.bAssOnline = (('1' - strName[ipos + 1]) == 0 );
						newMem.byAssPhyle = (strName[ipos + 2] > '2'?strName[ipos + 2] - '2':0);
						newMem.strAssist = strName.substr(ipos + 3);

						if (bGuildTower && temp.wNum < 11 && temp.wNum > 0)
						{
							g_GuildData.AddGuildMemeberOfficerMap(temp.wNum+1,newMem.strAssist);
						}
					}
					newMem.bAssist = true;

					if (bGuildTower && temp.wNum < 11 && temp.wNum > 0)
					{
						g_GuildData.AddGuildMemeberOfficerMap(temp.wNum,newMem.strName);
					}
				}
				else
				{//没找到，则没有助手

					newMem.strName = strName.substr(2);

					if (bGuildTower && temp.wNum < 11 && temp.wNum > 0)
					{
						g_GuildData.AddGuildMemeberOfficerMap(temp.wNum,newMem.strName);
					}
				}

				(*pNodeList)[iTemp].vMember.push_back(newMem);
			}
			i = nEnd;

			g_GuildData.SetFirstGuildData(false);
		}
		g_pControl->Msg(MSG_CTRL_RELATION_WND,15,NULL);
		g_pControl->MsgToWnd(MSG_CTRL_KFZ_EDIT_MEMBER_WND,MSG_CTRL_KFZ_EDIT_MEMBER_WND,2);
	TRY_END

	delete[] strMsg1;
}

void CGameControl::MSG_Guild_Member_List(const char * msg,int iLen)
{
	char * strMsg1 = new char[iLen + 1];
	memcpy(strMsg1,msg,iLen);
	strMsg1[iLen] = 0;
	string str = strMsg1 + 12;
	int i = 0;
	int nBegin;
	int nEnd;
	string strTemp;
	if (msg[6] != 1)
	{
		g_GuildData.GetMember().clear();
		while ( i < iLen - 12)
		{
			nBegin	= str.find_first_of('#',i);
			if(nBegin == -1)
				break;
			nBegin ++;
			nEnd	= str.find_first_of('#',nBegin);
			if(nEnd == -1)
				nEnd = str.size();
			CGuildData::_Member temp;
			g_GuildData.GetMember().push_back(temp);
			int nLine = g_GuildData.GetMember().size() -1;

			g_GuildData.GetMember()[nLine].num += StringUtil::toStr(str,nBegin);
			nBegin ++;
			g_GuildData.GetMember()[nLine].title = StringUtil::toStr(str,nBegin);

			while(nBegin < nEnd)
			{
				g_GuildData.GetMember()[nLine].name.push_back(StringUtil::toStr(str,nBegin));			
			}
			i = nEnd;

			g_GuildData.SetFirstGuildData(false);
		}
		g_GuildData.GetMemberTemp().clear();
		{
			for(unsigned int i = 0; i < g_GuildData.GetMember().size(); i++)
			{
				string title = "";
				if(g_GuildData.IsHeader())
				{
					title +="(";
					title += g_GuildData.GetMember()[i].num;
					title += ")"; 
				}
				title += "<";
				title += g_GuildData.GetMember()[i].title;
				title += ">";
				g_GuildData.GetMemberTemp().push_back(title);
				char temp[1024] = {0};
				for(unsigned int j = 0; j < g_GuildData.GetMember()[i].name.size();j++)
				{
					sprintf(temp,"%s%-16s",temp,g_GuildData.GetMember()[i].name[j].c_str());
					if(j % 2 == 1)
					{
						g_GuildData.GetMemberTemp().push_back(temp);                
						temp[0] = 0;
					}
				}
				g_GuildData.GetMemberTemp().push_back(temp);

				if(temp[0]!=0)
				{
					temp[0] =0;
					g_GuildData.GetMemberTemp().push_back(temp);
				}
			}
		}		
		g_pControl->MsgToWnd(MSG_CTRL_KFZ_EDIT_MEMBER_WND,MSG_CTRL_KFZ_EDIT_MEMBER_WND,1);
	}
	else
	{//新的改变封号协议
		bool bOnline = false;
		int iCorNum = -1;
		int iOldCorNum = -1;
		int iPhyle = 0;

		std::vector<CGuildData::sTileNode>* guildMem = NULL;

		while ( i < iLen - 12)
		{
			nBegin	= str.find_first_of('#',i);
			if(nBegin == -1)
				break;
			nBegin ++;
			int iPos = str.find_first_of('<',i);
			int iPos1 = str.find_first_of('>',i);
			if (iPos == - 1 || iPos1 == -1)
				break;
			strTemp = str.substr(nBegin,iPos - nBegin);
			iCorNum = atoi(strTemp.c_str());				
			strTemp = str.substr(iPos + 1,iPos1 - iPos - 1);
			nEnd	= str.find_first_of('#',nBegin);
			if(nEnd == -1)
				nEnd = str.size();

			if (iCorNum > 10)
				guildMem = &(g_GuildData.GetGuildMemberNormal());
			else guildMem = &(g_GuildData.GetGuildMemberOfficer());

			CGuildData::sTileNode* pNewNode = NULL;
			for (int j = 0;j<(*guildMem).size();j++)
			{
				if(iCorNum > (*guildMem)[j].wNum)
					continue;

				if ((*guildMem)[j].wNum == iCorNum)
				{
					(*guildMem)[j].sTile = strTemp;
					pNewNode = &((*guildMem)[j]);
					break;
				}
				else
				{
					CGuildData::sTileNode sNode;
					sNode.sTile = strTemp;
					sNode.wNum = iCorNum;
					(*guildMem).insert((*guildMem).begin() + j,sNode);
					pNewNode = &((*guildMem)[j]);
					break;
				}
			}

			if (pNewNode == NULL)
			{
				CGuildData::sTileNode sNode;
				sNode.sTile = strTemp;
				sNode.wNum = iCorNum;
				(*guildMem).push_back(sNode);
				pNewNode = &((*guildMem)[(*guildMem).size() - 1]);
			}

			nBegin = iPos1 + 1;
			while(nBegin < nEnd)
			{
				//先删除旧的数据
				bOnline = false;
				strTemp = StringUtil::toStr(str,nBegin);
				iOldCorNum = StringUtil::toInt(str,nBegin);
				iPhyle = StringUtil::toUInt(str,nBegin);

				if (iOldCorNum > 10)
					guildMem = &(g_GuildData.GetGuildMemberNormal());
				else guildMem = &(g_GuildData.GetGuildMemberOfficer());


				bool bFinish = false;
				for (int j = 0;!bFinish && j<(*guildMem).size();j++)
				{
					if ((*guildMem)[j].wNum == iOldCorNum)
					{
						CGuildData::sTileNode& mem = (*guildMem)[j];
						for (int k = 0;!bFinish && k < mem.vMember.size();k++)
						{
							if (strTemp.compare(mem.vMember[k].strName.c_str()) == 0)
							{
								bOnline = mem.vMember[k].bOnline;
								mem.vMember.erase(mem.vMember.begin() + k);
								if (mem.vMember.size() <= 0 && pNewNode != &((*guildMem)[j]))
									(*guildMem).erase((*guildMem).begin() + j);

								bFinish = true;
								break;
							}
						}							
					}
				}
				//再将新的数据加上去
				if (iCorNum > 10)
					guildMem = &(g_GuildData.GetGuildMemberNormal());
				else guildMem = &(g_GuildData.GetGuildMemberOfficer());

				for (int j = 0;j<(*guildMem).size();j++)
				{
					if ((*guildMem)[j].wNum == iCorNum)
					{
						CGuildData::_NewMember newMem;
						newMem.strName = strTemp;
						newMem.bOnline = bOnline;
						newMem.byPhyle = (BYTE)iPhyle;
						(*guildMem)[j].vMember.push_back(newMem);
						break;
					}
				}
			}
			i = nEnd;
		}
		g_pControl->Msg(MSG_CTRL_RELATION_WND,15,NULL);
	}

	delete[] strMsg1;
}

void CGameControl::MSG_Guild_Ret_Exp_Val(const char * msg,int iLen)
{
	//   行会名称/行会等级/行会经验值/个人贡献行会经验值/个人行会能量 
	string sDisplay;
	string temp;

	string buf;
	buf.assign(msg+12,iLen-12);

	int j =0; 	

	WORD wMember = *((WORD*)(msg + 6));
	WORD wMaxMember = *((WORD*)(msg + 8));

	sDisplay = "本行会“";
	int iSize = (int)buf.size();

	if(j < iSize)
	{
		temp = StringUtil::toStr(buf,j);
		sDisplay += temp;
	}

	if(j < iSize)
	{
		sDisplay += "”目前为";
		temp = StringUtil::toStr(buf,j);	
		sDisplay += temp;
		g_GuildData.SetGuildLevel(temp.c_str());
	}

	if(j < iSize)
	{
		sDisplay += "级行会，\n拥有行会经验值";
		temp = StringUtil::toStr(buf,j);	
		sDisplay += temp;
		g_GuildData.SetGuildExp(temp.c_str());
	}

	if(j < iSize)
	{
		sDisplay += "，你已经为本行会贡献了";
		temp = StringUtil::toStr(buf,j);	
		sDisplay += temp;
	}

	if(j < iSize)
	{
		sDisplay += "行会经验值,\n目前你拥有的行会能量为";
		temp = StringUtil::toStr(buf,j);	
		sDisplay += temp;
	}
	if(wMember > 0 && wMaxMember > 0)
	{
		temp = StringUtil::format("，\n行会成员 %d/%d。",wMember,wMaxMember);
		sDisplay += temp;
	}
	else
	{
		sDisplay += "。";
	}

	//g_MsgBoxMgr.PopSimpleAlert(sDisplay.c_str());
}

void CGameControl::SEND_Guild_Ally(const char * str)
{
	SET_COMMAND(CS_GUILD_ALLY,256);
	ADD_STR(str);
	SEND_GAME_SERVER();

}

void CGameControl::MSG_Guild_War_Message(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg + 12,iLen - 12);

	g_MsgBoxMgr.PopSimpleAlert(buf.c_str());
}

void CGameControl::MSG_Guild_Member_Get_Title(const char * msg,int iLen)
{
	int i = 0;
	string strMsg = msg+12;
	string temp; 

	//更新行会房间数据
	g_VoiceAdapter.RequestRoom();

	if(iLen == 12)//退出行会
	{
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_QUIT_GUILD,SELF.GetTitle());

		SELF.SetTitle("");
		g_GuildData.GetMember().clear();
		g_GuildData.SetGuildName("");
		SELF.SetGuildTitle("");

		g_SdSamplerMgr.SendSocietyInfoSample();

		SELF.SetGuildLogoID(0);

		VRelationStruct& vrl = g_OtherData.GetRelationVector();
		for (int i = 0;i<vrl.size();i++)
		{
			if(vrl[i].strName.compare(SELF.GetName()) == 0)
			{
				vrl[i].strGuild = "";
				break;
			}
		}

		//清除一些相关信息
		g_GuildData.GetGuildMemberOfficer().clear();
		g_GuildData.GetGuildMemberNormal().clear();
		g_GuildData.SetGuildTowerLevel(0);
		g_GuildData.SetGuildName("");
		g_GuildData.GetGuildTower().dwTaelPub = 0;
		g_GuildData.SetGuildLevel("0");
		g_GuildData.SetGuildFunc(0);
		memset(&(g_GuildData.GetGuildBuffer()),0,sizeof(g_GuildData.GetGuildBuffer()));

		MiniChar& _chars = MapArray.GetMiniChar();
		MiniChar::iterator it;
		for(it = _chars.begin(); it != _chars.end(); it++)
		{
			CSimpleCharacter* pChar = it->second;
			if(NULL == pChar || !pChar->IsHuman()) 
				continue;

			pChar->RemoveRelationType(RT_GUILD);
		}

		g_GuildData.SetGuildFlag(0);//行会旗帜
		CCtrlWindow* pWnd = g_pControl->FindWindowByName("RelationWnd");
		if (pWnd)
		{
			pWnd->Msg(MSG_CTRL_GUILD_CHANGE_INFO,0,NULL);
		}

		return;
	}

	//g_pControl->Msg(MSG_CTRL_RELATION_WND,17);

	const char* p =  g_GuildData.GetGuildDelName();//这里借用了，增加行会成员时也使用该数据保存被加入行会的成员姓名
	temp = StringUtil::toStr(strMsg,i);
	std::string strOldTitle = SELF.GetGuildTitle();
	g_GuildData.SetGuildFlag(*((int *)(msg+6)));//行会旗帜

	VRelationStruct& vrl = g_OtherData.GetRelationVector();
	for (int i = 0;i<vrl.size();i++)
	{

		if(vrl[i].strName.compare(SELF.GetName()) == 0)
		{
			vrl[i].strGuild = temp;
			break;
		}
	}

	////会长我上线，成员我上线，我被会长增加,会长改变我的封号,改变会长自己的封号,都要更新封号
	if( !(g_GuildData.IsHeader() && strlen(p) > 0 ) )//除了是会长加了其它人这种情况外，都要更新封号
	{
		SELF.SetTitle(temp.c_str());
		g_GuildData.SetGuildName(temp.c_str());

		if(g_OtherData.GetEnterGameTime() > 0 && strOldTitle.empty())
		{
			if(g_pSDOAInterface)
				g_pSDOAInterface->OnCommunityEvent(CE_JOIN_GUILD,temp.c_str());
		}

		//temp = strMsg.substr(i,strMsg.length() - i);
		//SELF.SetGuildTitle(temp.c_str());
		char strGuildTitle[128] = {0};
		memcpy(strGuildTitle,msg + 12 + i,iLen - 12 - i);
		SELF.SetGuildTitle(strGuildTitle);

		g_SdSamplerMgr.SendSocietyInfoSample();

		SELF.SetGuildLogoID(*((DWORD *)(msg+6)));//		

		if(strOldTitle.empty())
		{
			if(g_pSDOAInterface)
				g_pSDOAInterface->OnCommunityEvent(CE_SET_GUILD_TITLE,temp.c_str());
		}
		else if(strcmp(strOldTitle.c_str(),temp.c_str()) != 0)
		{
			if(g_pSDOAInterface)
				g_pSDOAInterface->OnGuildTitleChanged(strOldTitle.c_str(),temp.c_str(),0);
		}
	}


	if( strOldTitle.length() <= 0)//会长我上线，成员我上线，我被会长增加
	{
		if(1)//我被会长增加,新加入行会,更新附近玩家的关系,上线收到这条协议时时g_pGameData->GetMapArray().GetMiniChar()必定为空
		{
			MiniChar& _chars = MapArray.GetMiniChar();
			MiniChar::iterator it;
			for(it = _chars.begin(); it != _chars.end(); it++)
			{
				CSimpleCharacter* pChar = it->second;
				if(NULL == pChar || !pChar->IsHuman()) 
					continue;

				if(strlen(pChar->GetGuildName()) > 0 && strcmp( SELF.GetTitle(), pChar->GetGuildName()) == 0)
				{
					pChar->AddRelationType(RT_GUILD);
				}
			}
		}
	}
	else
	{
		if(g_GuildData.IsHeader())//会长加了其它人或者改变会长自己的封号
		{
			if(strlen(p) > 0)//会长加了其它人
			{
				CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter((char*)p);
				if(pChar) pChar->AddRelationType(RT_GUILD);
			}
			else//改变会长自己的封号
			{
			}
		}
		else//会长改变我的封号
		{
		}
	}

	g_GuildData.SetGuildDelName("");
}


void CGameControl::MSG_Guild_Member_Title_Edit_Failed(const char * msg,int iLen)
{
	DWORD dwCode = *((DWORD *)msg);
	string str;

	switch(dwCode)
	{
	case 0xFFFFFFFA:
		str = "不能添加/删除成员！";
		break;
	case 0xFFFFFFF9:
		str = "职位重复或出错！";
		break; 
	case 0xFFFFFFFE:
		str = "掌门人位置是空的！";
		break;
	case 0xFFFFFFFC:  
		str = "一个行会只能有两个掌门人！";
		break;
	case 0x00000000:
		g_GuildData.SetGuildFlag(*(DWORD *)(msg+6));
		SELF.SetGuildLogoID(*(DWORD *)(msg+6));
		break;
	case 0xFFFFFFFB:
		str = "行会会旗设定失败！";
		break;
	default:
		str = "编辑封号失败！";
		break;
	}
	if(!str.empty())
	{
		g_MsgBoxMgr.PopSimpleAlert(str.c_str());
	}
}

//[6]:0表示删除,1表示添加,12字节以后表示名字
void CGameControl::MSG_Guild_Ally_Success(const char * msg,int iLen)
{
	BYTE byType = msg[6];
	string strGuildName;
	strGuildName.assign(msg + 12,iLen - 12);
	if(byType == 0)
	{
		g_pControl->MsgToWnd(MSG_CTRL_RELATION_WND,MSG_CTRL_RELATION_WND,18,(CControl*)(strGuildName.c_str()));
	}
	else if(byType == 1)
	{
		g_pControl->MsgToWnd(MSG_CTRL_RELATION_WND,MSG_CTRL_RELATION_WND,19,(CControl*)(strGuildName.c_str()));
	}
}

void CGameControl::MSG_Guild_Align_Failed(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("联盟失败！");
}

void CGameControl::MSG_Guild_Align_Cancel_Failed(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("取消联盟失败！");
}

void CGameControl::SEND_Look_Player_Info(DWORD id,int x, int y,char dir)
{
	SET_COMMAND(CS_LOOK_PLAYER_INFO,12);
	*((DWORD *)temp) = id;
	*((WORD*)(temp + 6)) = x;
	*((WORD*)(temp + 8)) = y;
	temp[10] = dir;

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Player_Dig_Corpse(DWORD id,int x, int y,char dir)
{
	SET_COMMAND(CS_PLAYER_DIG_CORPSE,12);
	*((DWORD *)temp) = id;
	*((WORD*)(temp + 6)) = x;
	*((WORD*)(temp + 8)) = y;
	temp[10] = dir;

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Map_No_Small_Map(const char * msg,int iLen)
{
	//g_TalkMgr.PushSysTalk("没有可用地图");
	g_pGameMap->LoadMiniMap(0);
}

void CGameControl::MSG_Map_Small_Map(const char * msg,int iLen)
{
	WORD num = Conv_WORD(msg + 6);
	g_pGameMap->LoadMiniMap(num);
}

void CGameControl::MSG_Magic_Skill_Value(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg);
	int  iLevel   = Conv_WORD(msg + 8);

	CMagicData* pMagic = SELF.FindMagic(wMagicID);
	if(pMagic == NULL)
		return;

	int iOldLevel = pMagic->GetMagicLevel();

	pMagic->SetMagicLevel(msg[6]);
	pMagic->SetSkillValue(iLevel);

	g_AIMgr.ChangeMagicState(wMagicID,TRUE);

	if (iOldLevel != pMagic->GetMagicLevel())
		g_AIMgr.UpdateConSkillActive();
}

void CGameControl::MSG_Player_Skill_Delete(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg);

	g_AIMgr.ChangeMagicState(wMagicID,FALSE);
	SELF.DeleteMagic(wMagicID);

	g_AIMgr.UpdateConSkillActive();
	g_pControl->Msg(MSG_CTRL_UPDATE_SKILLWND,0,0);
	g_AICfgMgr.SaveShortCutConfig();
}

void CGameControl::MSG_Player_Skill_Add(const char * msg,int iLen)
{
	WORD wMagicID = Conv_WORD(msg + 20);
	BYTE byMagicType = msg[12 + 23];//0:普通,1:生产技能

	CMagicData* pMagic = NULL;
	if (byMagicType == 0)
	{
		pMagic = SELF.MagicArray().FindMagic(wMagicID);
		if (pMagic)
			pMagic->FromBuffer(msg+12,iLen-12);
		else
			pMagic = SELF.MagicArray().Add(msg+12,iLen-12);

		if(!g_pGameData->HasPaoPaoStatus(EP_First_StudySkill_PaoPao))
		{
			if(!g_pControl->MsgToWnd(MSG_CTRL_SKILLWND,MSG_CTRL_POPUP_ARROWTIP,EP_First_StudySkill_PaoPao))
			{
				if (g_EutUiType == EUT_CLASSIC)
				{
					g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_StudySkill_PaoPao,800 - 755,185 - 113,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
				} 
				else
				{
					g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_StudySkill_PaoPao,138+234,38+48,XAL_BOTTOMRIGHT,false,XAL_BOTTOMRIGHT);
				}
			}
			else
			{
				g_pControl->PopupArrowTip(MSG_CTRL_SKILLWND,EP_First_StudySkill_PaoPao,348,94,XAL_TOPLEFT,false,XAL_TOPRIGHT);
			}
		}
	}
	else if (byMagicType == 1)
	{
		pMagic = SELF.ProduceMagicArray().FindMagic(wMagicID);
		if (pMagic)
			pMagic->FromBuffer(msg+12,iLen-12);
		else
			pMagic = SELF.ProduceMagicArray().Add(msg+12,iLen-12);

		if (!g_pControl->MsgToWnd("SkillWnd",MSG_CTRL_FLASH_BTN_WND,2))
		{
			g_OtherData.SetNeedFlashProduceTab(true);
		}
	}


	g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,2);
	

	if(pMagic)
	{
		pMagic->DefaultAttr();
		switch (pMagic->GetMagicID())
		{
		case MAGICID_SWT:
		case MAGICID_LSJ:
		case MAGICID_JYL:
			pMagic->SetDisable(!IsEquipXWSkillSuit());//圣王套装技能是否可用，要看身上的衣服
			break;
		}

		pMagic->SetFlashTexID(MAKELONG(21589,PACKAGE_INTERFACE));
	}



	g_AIMgr.UpdateConSkillActive();
	g_pControl->Msg(MSG_CTRL_UPDATE_SKILLWND,0,0);
	g_AICfgMgr.SaveShortCutConfig();

}

void CGameControl::MSG_Open_Attack_Kill(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_ATTACKKILL);
}

void CGameControl::MSG_Close_Attack_Kill(const char* msg, int iLen)
{
	SELF.RemoveMagicState(MS_ATTACKKILL);
}

void CGameControl::MSG_Open_Attack_Stick(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_ATTACKSTICK);
}

void CGameControl::MSG_Close_Attack_Stick(const char * msg,int iLen)
{
	SELF.RemoveMagicState(MS_ATTACKSTICK);
}

void CGameControl::MSG_Open_Attack_Moon(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_ATTACKMOON);
}

void CGameControl::MSG_Close_Attack_Moon(const char * msg,int iLen)
{
	SELF.RemoveMagicState(MS_ATTACKMOON);
}

void CGameControl::MSG_Open_Attack_Fire(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_ATTACKFIRE);
	g_AutoFightMgr.SetAutoFire(1);
}

void CGameControl::MSG_Close_Attack_Fire(const char * msg,int iLen)
{
	SELF.RemoveMagicState(MS_ATTACKFIRE);
	g_AutoFightMgr.SetAutoFire(0);
}

void CGameControl::MSG_Valid_Dig(const char * msg,int iLen)
{
	SELF.AddMagicState(MS_VALIDDIG);    
}

void CGameControl::MSG_Internet_Good(const char * msg,int iLen)
{

}

void CGameControl::MSG_Internet_Fail(const char * msg,int iLen)
{

}

void CGameControl::MSG_Chat_Not_Online(const char * msg,int iLen)
{
	string str;
	string temp;
	string temp8;
	int j =0; 

	str.assign(msg+12,iLen-12);

	//	temp = str;
	temp = StringUtil::toStr(str,j);
	g_OtherData.GetOffLinePrivate().bLeaveWords = false;
	g_OtherData.GetOffLinePrivate().strCharName = temp;

	//取出文本的内容
	temp8.assign(msg + 12 + j,iLen-12-j);
	StringUtil::AutoCut(temp8,MAX_NOTE_BYTES);
	g_OtherData.GetOffLinePrivate().strMessage = temp8;

	if(g_OtherData.IsAutoLeaveWord())//自动留言
	{
		temp = "";
		temp = temp + "##" + g_OtherData.GetOffLinePrivate().strCharName + " ";
		temp = temp + g_OtherData.GetOffLinePrivate().strMessage;
		StringUtil::AutoCut(temp,MAX_NOTE_BYTES);		
		SEND_Message_Send(temp.c_str(),temp.length());
	}
	else
		g_pControl->PopupWindow(MSG_CTRL_AUTOLEAVEWORD_WND,OPER_CREATE);	//自动留言窗口

} 

void CGameControl::MSG_Unread_Leave_Word(const char * msg,int iLen)
{
	char temp[2048] = {0};
	int j =0; 

	_LeaveWords t;
	string strTemp;
	strTemp.assign(msg + 12,iLen - 12);

	t.strCharName = StringUtil::toStr(strTemp,j);
	t.strDate = StringUtil::toStr(strTemp,j);
	t.strDate.assign(t.strDate.c_str(),0,10);

	t.strTime = StringUtil::toStr(strTemp,j);
	t.strMessage = StringUtil::toStr(strTemp,j);
	t.strPic = StringUtil::toStr(strTemp,j);
	t.strMusic = StringUtil::toStr(strTemp,j);
	t.bIsReaded = false;

	g_OtherData.GetLeaveWordsVector().push_back(t);
	if(g_OtherData.GetLeaveWordsVector().size() > 20)
		g_OtherData.GetLeaveWordsVector().erase(g_OtherData.GetLeaveWordsVector().begin());
} 

void CGameControl::MSG_Friend_Notice(const char * msg,int iLen)
{
	char temp[100]={0};
	char tempName[50] = {0};
	char tempMoQi[50]={0};
	DWORD dwMoQi = 0;
	char tempGuild[50]={0};
	char cType;
	char cFlag;
	int iLevel = 0,iJob=0;
	int itemp = 12;
	
	memcpy(&cFlag,msg,1);
	memcpy(temp,msg + itemp,iLen-itemp);
	memcpy(&cType,msg + 6,1);
	char* pTemp  = temp + strlen(temp);
	//if(strlen(temp) +itemp >= iLen)
	//{
	//	memcpy(tempName,temp,strlen(temp));
	//}
	//else
	//{
	//	memcpy(tempName,temp,strlen(temp));			
	//	memcpy(tempGuild,pTemp+1,strlen(pTemp+1));		
	//}

	memcpy(tempName,temp,strlen(temp));
	int ipos = itemp + strlen(temp);
	if (cType == 0)
	{
		memcpy(tempMoQi,pTemp+1,strlen(pTemp+1));
		dwMoQi = atol(tempMoQi);
		ipos = itemp + strlen(temp) + 1 + strlen(pTemp+1);
		pTemp  = pTemp+1 + strlen(pTemp+1);
	}
	if(ipos < iLen)
	{		
		memcpy(tempGuild,pTemp+1,strlen(pTemp+1));
	}

	temp[iLen-itemp] =0;
	bool bFind = false;
	DWORD dwNewRelationType = 0;

	char temp1[256] = {0};
	if( cType == 3)
	{
		if(SELF.IsMale())
		{
			strcpy(temp1,"你的妻子");
			dwNewRelationType = RT_WIFE;//妻
		}
		else
		{
			strcpy(temp1,"你的丈夫");
			dwNewRelationType = RT_HUSBAND;//夫
		}
	}
	else if(cType == 0)
	{
		strcpy(temp1,"你的好友");
		dwNewRelationType = RT_FRIEND;//好友
	}
	else if(cType == 1)
	{
		strcpy(temp1,"你的师父");
		dwNewRelationType = RT_MASTER;//师
	}
	else if(cType == 2)
	{
		strcpy(temp1,"你的徒弟");
		dwNewRelationType = RT_PRENTICE;//徒
	}
	else if (cType == 4)//宗派成员
	{		
		strcpy(temp1,"你的宗族成员");
		dwNewRelationType = RT_PHYLE;//徒
	}
	else if (cType == 5)
	{
		strcpy(temp1,"你的结义好友");
		dwNewRelationType = RT_JIEYI;//结义
	}

	for(UINT i = 0; !bFind && i < g_OtherData.GetRelationVector().size();i++)
	{		
		DWORD dwMyRelationType = g_OtherData.GetRelationVector()[i].iRelType;
		string strName = g_OtherData.GetRelationVector()[i].strName;
		bFind = (strcmp(temp,strName.c_str()) == 0);
		if(bFind && ((dwMyRelationType & dwNewRelationType) != 0))
		{
			g_OtherData.GetRelationVector()[i].strGuild = tempGuild;
			g_OtherData.GetRelationVector()[i].iMuoQi = dwMoQi;			
			g_OtherData.GetRelationVector()[i].iJob =*(msg + 11); 
			g_OtherData.GetRelationVector()[i].iLevel = (BYTE)(*(msg + 10));

			g_OtherData.GetRelationVector()[i].AddRelationType(dwNewRelationType);
			g_OtherData.GetRelationVector()[i].iOnline = cFlag % 2;
			g_OtherData.GetRelationVector()[i].byJinJie = *(msg + 8);

			if( (dwMyRelationType & dwNewRelationType) != 0 && cFlag < 2)
			{				
				if(strcmp(SELF.GetName(),strName.c_str()) == 0)
					break;

				strcat(temp1,temp);
				if( cFlag == 0)
				{
					strcat(temp1,"下线了");
					g_TalkMgr.PushSysTalk(temp1,TALKCOLOR_PINK);						
				}
				else if( cFlag == 1)
				{
					strcat(temp1,"上线了");
					g_TalkMgr.PushSysTalk(temp1,TALKCOLOR_PINK);						
				}
			}			
			break;
		}
	}

	if(!bFind)
	{
		//增加数据
		_RelationStruct relation;
		relation.strName = tempName;
		relation.strGuild = tempGuild;
		relation.iMuoQi = dwMoQi;
		relation.iJob = *(msg + 11);
		relation.iLevel = (BYTE)(*(msg + 10));
		relation.iRelType = dwNewRelationType;
		relation.iOnline = cFlag % 2;
		relation.byJinJie = *(msg + 8);
		g_OtherData.GetRelationVector().push_back(relation);
	}

	if(cType == 0)		
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,10);
		g_pControl->Msg(MSG_CTRL_YUSHOULEVELUP_WND,10);
	}
	else if(cType == 1 || cType == 2) 	
		g_pControl->Msg(MSG_CTRL_RELATION_WND,13);
	else if(cType == 3)				
		g_pControl->Msg(MSG_CTRL_RELATION_WND,14);
	else if (cType == 4)
		g_pControl->Msg(MSG_CTRL_RELATION_WND,20);
	else if (cType == 5)
		g_pControl->Msg(MSG_CTRL_RELATION_WND,30);
} 

void CGameControl::MSG_General_Reply_Confirm(const char * msg,int iLen)
{
	char temp[256]={0};
	memcpy(temp,msg+12,iLen-12);
	temp[iLen-12] = 0;
	WORD wType = *(WORD*)(msg + 6);

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(temp);
	if(!p) return;

	S_CommonConfirm CommonConfirm;	
	CommonConfirm.dwLastTime = 30000;
	CommonConfirm.dwRequestPersonID = p->GetID();
	if (wType == 1)
	{
		CommonConfirm.iType = 7;
		CommonConfirm.strMsg = CommonConfirm.strMsg + temp + "申请将你加入他的宗族";
	}
	g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);
	g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
}

void CGameControl::MSG_Friend_Request(const char * msg,int iLen)
{
	char temp[256]={0};
	memcpy(temp,msg+12,iLen-12);
	temp[iLen-12] =0;

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(temp);
	if(!p)
		return;

	if (!g_AICfgMgr.IsAcceptFrientAndTrade())
	{
		g_pGameControl->SEND_Answer_Friend_Request(temp,false);
		return;
	}

	S_CommonConfirm CommonConfirm;
	CommonConfirm.iType = 4;
	CommonConfirm.dwLastTime = 30000;
	CommonConfirm.dwRequestPersonID = p->GetID();
	CommonConfirm.strMsg = CommonConfirm.strMsg + temp + "申请将你加入好友名单。";
	g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);
	g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
} 

void CGameControl::MSG_Friend_Request_IM(const char * msg,int iLen)
{
	char temp[256]={0};
	memcpy(temp,msg+12,iLen-12);
	temp[iLen-12] =0;
} 

void CGameControl::MSG_Friend_MoQi_Change(const char * msg,int iLen)
{
	int j = 0;
	string buf;
	buf.assign(msg+12,iLen-12);
	const char* sbuf = buf.c_str();

	while(j < iLen-13)
	{
		// 好友名字
		string strName = StringUtil::toStr(buf,j);

		// 默契度
		DWORD dwMoQi = StringUtil::toUInt(buf,j);		

		bool bFind = false;
		for(UINT i = 0; !bFind && i < g_OtherData.GetRelationVector().size();i++)
		{
			bFind = (g_OtherData.GetRelationVector()[i].strName == strName);
			if(bFind)	
			{
				g_OtherData.GetRelationVector()[i].iMuoQi = dwMoQi;
				break;
			}
		}
	}

	g_pControl->Msg(MSG_CTRL_RELATION_WND,10,NULL);
}


void CGameControl::MSG_Friend_List(const char * msg,int iLen)
{
	int j = 0;
	char cType = msg[6];
	char cChangeName = msg[8];

	string buf;
	buf.assign(msg+12,iLen-12);

	DWORD dwNewRelationType = 0;
	if( cType == 3)
	{
		if(SELF.IsMale())
			dwNewRelationType = RT_WIFE;//妻
		else
			dwNewRelationType = RT_HUSBAND;//夫
	}
	else if(cType == 0)
		dwNewRelationType = RT_FRIEND;//好友
	else if(cType == 1)
		dwNewRelationType = RT_MASTER;//师
	else if(cType == 2)
		dwNewRelationType = RT_PRENTICE;//徒
	else if (cType == 4)
	{
		g_OtherData.SetPhyleName(StringUtil::toStr(buf,j));
		g_OtherData.SetPhyleMaster(StringUtil::toStr(buf,j));
		g_OtherData.SetPhyleType(StringUtil::toUInt(buf,j));
		dwNewRelationType = RT_PHYLE;
	}

	if(cChangeName == 1)
	{
		string strOrgName = StringUtil::toStr(buf,j);

		for(int i = 0;i< g_OtherData.GetRelationVector().size();i++)
		{
			DWORD dwMyRelationType = g_OtherData.GetRelationVector()[i].iRelType;

			if( (dwMyRelationType & dwNewRelationType) != 0 && g_OtherData.GetRelationVector()[i].strName == strOrgName)
			{
				string strNewName = StringUtil::toStr(buf,j);
				g_OtherData.GetRelationVector()[i].strName = strNewName;

				CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strNewName.c_str());
				if(p) 
					p->AddRelationType(dwNewRelationType);

				break;
			}
		}

		if( cType == 3)
			g_pControl->Msg(MSG_CTRL_RELATION_WND,14,NULL);
		else if(cType == 0)
		{
			g_pControl->Msg(MSG_CTRL_RELATION_WND,10,NULL);
			g_pControl->Msg(MSG_CTRL_YUSHOULEVELUP_WND,10);
		}
		else if(cType == 1 || cType == 2)
			g_pControl->Msg(MSG_CTRL_RELATION_WND,13,NULL);
		else if (cType == 4)
			g_pControl->Msg(MSG_CTRL_RELATION_WND,20,NULL);

		return;
	}

	while(j < iLen-13)
	{
		DWORD dwPhyleType = 0;
		if (cType == 4)
		{
			dwPhyleType = StringUtil::toUInt(buf,j);
		}
		string strName = StringUtil::toStr(buf,j);

		DWORD dwMoQi = 0;
		if (cType == 0)
		{
			dwMoQi = StringUtil::toUInt(buf,j);
		}

		bool bFind = false;
		for(UINT i = 0; !bFind && i < g_OtherData.GetRelationVector().size();i++)
		{
			bFind = (g_OtherData.GetRelationVector()[i].strName == strName);
			if(bFind)	
			{
				DWORD dwRelation = g_OtherData.GetRelationVector()[i].iRelType;
				dwRelation |=  dwNewRelationType;
				g_OtherData.GetRelationVector()[i].iRelType = dwRelation;
				g_OtherData.GetRelationVector()[i].iMuoQi = dwMoQi;
				break;
			}
		}

		if(!bFind)
		{
			//增加数据
			_RelationStruct relation;
			relation.strName = strName;
			relation.iRelType = dwNewRelationType;
			relation.iMuoQi = dwMoQi;
			relation.strGuild = "";
			relation.iJob = -1;
			relation.iLevel = -1;
			relation.byPhyleType = (BYTE)dwPhyleType;
			g_OtherData.GetRelationVector().push_back(relation);
		}

		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strName.c_str());
		if(p) 
			p->AddRelationType(dwNewRelationType);
	}
	//获取好友等级排名趋势
	//Send_Range_Friend(0x0100);[qyz]趋势已经没有用了，故不用申请

	if( cType == 3)
		g_pControl->Msg(MSG_CTRL_RELATION_WND,14,NULL);
	else if(cType == 0)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,10,NULL);
		g_pControl->Msg(MSG_CTRL_YUSHOULEVELUP_WND,10);
	}
	else if(cType == 1 || cType == 2)
		g_pControl->Msg(MSG_CTRL_RELATION_WND,13,NULL);
	else if (cType == 4)
		g_pControl->Msg(MSG_CTRL_RELATION_WND,20,NULL);
}

void CGameControl::MSG_Add_Friend_Success(const char * msg,int iLen)
{
	char temp[100]={0};
	char tempName[50] = {0};
	char tempGuild[50]={0};
	char cType;
	char cFlag;
	int iLevel = 0,iJob=0;
	memcpy(&cFlag,msg,1);
	memcpy(temp,msg + 12,iLen-12);
	char* pTemp  = temp + strlen(temp);	
	if(strlen(temp) +12 >= iLen)
	{
		memcpy(tempName,temp,strlen(temp));
	}
	else
	{
		memcpy(tempName,temp,strlen(temp));
		memcpy(tempGuild,pTemp+1,strlen(pTemp+1));
	}

	memcpy(&cType,msg + 6,1);
	temp[iLen-12] =0;
	bool bFind = false;

	DWORD dwNewRelationType = 0;
	if( cType == 3)
	{
		if(SELF.IsMale())
			dwNewRelationType = RT_WIFE;//妻
		else
			dwNewRelationType = RT_HUSBAND;//夫
	}
	else if(cType == 0)
		dwNewRelationType = RT_FRIEND;//好友
	else if(cType == 1)
		dwNewRelationType = RT_MASTER;//师
	else if(cType == 2)
		dwNewRelationType = RT_PRENTICE;//徒

	//每种关系的人的个数,如果只有一个,就是之前没有,现在有了,要发送收集信息
	int iRalationNnm[4];
	memset(iRalationNnm,0,sizeof(int) * 4);


	for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
	{		
		DWORD dwMyRelationType = g_OtherData.GetRelationVector()[i].iRelType;
		if (dwMyRelationType & RT_FRIEND)
		{
			iRalationNnm[0] ++;
		}
		else if (dwMyRelationType & RT_MASTER)
		{
			iRalationNnm[1] ++;
		}
		else if (dwMyRelationType & RT_PRENTICE)
		{
			iRalationNnm[2] ++;
		}
		else if (dwMyRelationType & RT_WIFE || dwMyRelationType & RT_HUSBAND)
		{
			iRalationNnm[3] ++;
		}

		if (!bFind)
		{
			bFind = (g_OtherData.GetRelationVector()[i].strName == temp);
			if(bFind)
			{
				g_OtherData.GetRelationVector()[i].strGuild = tempGuild;
				g_OtherData.GetRelationVector()[i].iJob =*(msg + 11); 
				g_OtherData.GetRelationVector()[i].iLevel = (BYTE)(*(msg + 10));
				g_OtherData.GetRelationVector()[i].byJinJie = *(msg + 8);
				g_OtherData.GetRelationVector()[i].AddRelationType(dwNewRelationType);
			}
		}
	}

	//某类关系从无到有的时候发送收集信息
	if ((iRalationNnm[0] == 0 && cType == 0) ||
		(iRalationNnm[1] == 0 && cType == 1) ||
		(iRalationNnm[2] == 0 && cType == 2) ||
		(iRalationNnm[3] == 0 && cType == 3))
	{
		g_SdSamplerMgr.SendSocietyInfoSample();
	}


	if(!bFind)
	{
		//增加数据
		_RelationStruct relation;
		relation.strName = tempName;
		relation.strGuild = tempGuild;
		relation.iJob = *(msg + 11);
		relation.iLevel = (BYTE)(*(msg + 10));
		relation.byJinJie = *(msg + 8);
		relation.iOnline = 1;
		relation.iRelType = dwNewRelationType;
		g_OtherData.GetRelationVector().push_back(relation);

		//新加社会关系,社会关系按钮闪烁
		if (!g_pControl->MsgToWnd("RelationWnd",MSG_CTRL_FLASH_BTN_WND,cType))
		{
			g_OtherData.SetFlashRelationTabType(cType);
		}

		g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,6);
	}

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(temp);
	if(p) p->AddRelationType(dwNewRelationType);

	if(cType == 0)
	{
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_ADD_FRIEND,tempName);

		strcat(temp," 已经成功加入你的关系列表。");
		g_TalkMgr.PushSysTalk(temp);	
	}

	if( cType == 3)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,14,NULL);
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_MARRIED,tempName);
	}
	else if(cType == 0)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,10,NULL);
		g_pControl->Msg(MSG_CTRL_YUSHOULEVELUP_WND,10);
	}
	else if(cType == 1 || cType == 2)
	{
		g_pControl->Msg(MSG_CTRL_RELATION_WND,13,NULL);
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_JOIN_MASTER,tempName);
	}
}

void CGameControl::MSG_Del_Friend_Success(const char * msg,int iLen)
{
	char temp[256] = {0};
	int	 iFlag;
	char cType;

	memcpy(&iFlag,msg,4);
	memcpy(&cType,msg + 6,1);
	memcpy(temp,msg + 12,iLen - 12);
	temp[iLen - 12 ] = 0;

	DWORD dwNewRelationType = 0;

	if( cType == 3)
	{
		if(SELF.IsMale())
			dwNewRelationType = RT_WIFE;//妻
		else
			dwNewRelationType = RT_HUSBAND;//夫
	}
	else if(cType == 0)
		dwNewRelationType = RT_FRIEND;//好友
	else if(cType == 1)
		dwNewRelationType = RT_MASTER;//师
	else if(cType == 2)
		dwNewRelationType = RT_PRENTICE;//徒

	//每种关系的人的个数,如果只有一个,删除之后就没有了,要发送收集信息
	int iRalationNnm[4];
	memset(iRalationNnm,0,sizeof(int) * 4);

	for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
	{
		_RelationStruct &SRelation = g_OtherData.GetRelationVector()[i];

		if (SRelation.iRelType & RT_FRIEND)
		{
			iRalationNnm[0] ++;
		}
		else if (SRelation.iRelType & RT_MASTER)
		{
			iRalationNnm[1] ++;
		}
		else if (SRelation.iRelType & RT_PRENTICE)
		{
			iRalationNnm[2] ++;
		}
		else if (SRelation.iRelType & RT_WIFE || SRelation.iRelType & RT_HUSBAND)
		{
			iRalationNnm[3] ++;
		}


		if(SRelation.strName == temp && (dwNewRelationType & SRelation.iRelType) !=0 )
		{
			SRelation.RemoveRelationType(dwNewRelationType);

			if(SRelation.iRelType == 0)//没有任何关系了则删除
			{
				g_OtherData.GetRelationVector().erase(g_OtherData.GetRelationVector().begin() + i);
				i --;
			}

			////////////////////////////
			CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(temp);
			if(p) 
				p->RemoveRelationType(dwNewRelationType);

			if(cType == 0)			
			{
				g_pControl->Msg(MSG_CTRL_RELATION_WND,10,NULL);
				g_pControl->Msg(MSG_CTRL_YUSHOULEVELUP_WND,10);
			}
			else if(cType == 1 || cType == 2) 	
				g_pControl->Msg(MSG_CTRL_RELATION_WND,13,NULL);
			else if(cType == 3)				
				g_pControl->Msg(MSG_CTRL_RELATION_WND,14,NULL);

			//break;
		}
	}


	//某类关系从无到有的时候发送收集信息
	if ((iRalationNnm[0] == 1 && cType == 0) ||
		(iRalationNnm[1] == 1 && cType == 1) ||
		(iRalationNnm[2] == 1 && cType == 2) ||
		(iRalationNnm[3] == 1 && cType == 3))
	{
		g_SdSamplerMgr.SendSocietyInfoSample();
	}



	g_OtherData.SetDelFriendNo(-1);

	if(cType == 0)
	{
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_REMOVE_FRIEND,temp);

		if( iFlag == 2)
		{
			strcat(temp,"已经被你从关系列表中删除。");
			g_TalkMgr.PushSysTalk(temp);
		}
		else if( iFlag == 1)
		{
			char temp1[256] = {0};
			sprintf(temp1,"你已经被%s从关系列表中删除。",temp);
			g_TalkMgr.PushSysTalk(temp1);
		}
	}
	else if (cType == 3)
	{
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_DIVORCED,temp);
	}
	else if (cType == 1 || cType == 2)
	{
		if(g_pSDOAInterface)
			g_pSDOAInterface->OnCommunityEvent(CE_LEAVE_MASTER,temp);
	}
}

void CGameControl::MSG_Friend_Error(const char * msg,int iLen)
{
	int iFlag;
	char temp[256] = {0};

	memcpy(&iFlag,msg,4);

	switch(iFlag)
	{
	case 1:
		{
			strcpy(temp,"很抱歉，你的好友数量已满，无法通过验证。");
			break;
		}
	case 2:
		{
			memcpy(temp,msg + 12,iLen - 12);

			strcat(temp,"不存在或不在线。");
			break;
		}
	case 3:
		{
			memcpy(temp,msg + 12,iLen - 12);
			strcat(temp,"超过其好友的规定数。");
			break;
		}
	case 4:
		{
			memcpy(temp,msg + 12,iLen - 12);
			strcat(temp,"拒绝了你的好友申请。");
			break;
		}
	case 5:
		{
			memcpy(temp,msg + 12,iLen - 12);
			strcat(temp,"已经是你的好友。");
			break;
		}
	case 6:
		{
			strcpy(temp,"添加好友出错。");
			break;
		}
	case 7:
		{
			strcpy(temp,"删除好友出错。");
			break;
		}
   case 8:
		{
			strcpy(temp,"对不起，您的级别不够，还不能添加好友。");
			break;
		}
	case 9:
		{
			memcpy(temp,msg + 12,iLen - 12);
			strcat(temp,"的级别不够，还不能成为您的好友。");
			break;
		}
	case 10:
		{
			memcpy(temp,msg + 12,iLen - 12);
			strcat(temp,"玩家暂时不能被添加为好友，添加失败。");
			break;
		}
	}

	if(iFlag > 0)
	{
		g_TalkMgr.PushSysTalk(temp);
	}
}
//Operate == 0,招收，1开除,2退出,3解散
void CGameControl::SEND_Phyle_Operate(const char * name,BYTE byOperate)
{	
	SET_COMMAND(CS_PHYLE_RELATION,256);
	ASSIGN_BYTE(6,byOperate);

	if (name && strlen(name) > 0)
	{
		string _name = name;
		StringUtil::trim(_name);
		if(_name.size() == 0)
			return;
		ADD_STR(_name.c_str());
	}

	int i = SEND_GAME_SERVER();

	if (i > 0 && byOperate == 0)
	{
		g_TalkMgr.PushSysTalk("已经发出加入宗族请求，请耐心等待回应");
	}
}

void CGameControl::SEND_Add_Friend(const char * name)
{
	string _name = name;
	StringUtil::trim(_name);
	if(_name.size() == 0)
		return;

	SET_COMMAND(CS_ADD_FRIEND,256);
	ADD_STR(_name.c_str());
	int i = SEND_GAME_SERVER();

	if (i > 0)
	{
		g_TalkMgr.PushSysTalk("已经发出交友请求，请耐心等待回应");
	}
}

void CGameControl::SEND_Del_Friend(const char * name)
{
	SET_COMMAND(CS_DEL_FRIEND,256);
	ADD_STR(name);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Answer_Friend_Request(const char * name, bool bApprove)
{
	SET_COMMAND(CS_ANSWER_FRIEND_REQUEST,256);
	ASSIGN_BYTE(0,bApprove ? 0x01 : 0x02);//approve 
	ADD_STR(name);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Weather(const char * msg,int iLen)
{
	DWORD dwColor = Conv_DWORD(msg);
	WORD wTime	= Conv_WORD(msg+6);
	BYTE byWeather= msg[8];
	BYTE byDensity= msg[9];
	BYTE bySpeed  = msg[10];

	DWORD dwWeatherColor = 0xFFFFFFFF;

	if( iLen >= 16 )
		dwWeatherColor = Conv_DWORD(msg+12);

	int byNewWeather = PARTICLE_NONE;

	switch(byWeather)
	{
	case 1:
		byNewWeather = PARTICLE_RAIN;
		break;
	case 2:
		byNewWeather = PARTICLE_CLOUD;
		g_pWeather->SetDensity(byDensity);
		g_pWeather->SetWeatherColor(dwWeatherColor);
		break;
	case 3:
		byNewWeather = PARTICLE_STORM;
		g_pWeather->SetStormSpeed(bySpeed);
		g_pWeather->SetDensity(byDensity);
		g_pWeather->SetWeatherColor(dwWeatherColor);
		break;
	case 4:
		byNewWeather = PARTICLE_FLOWER1;
		break;
	case 5:
		byNewWeather = PARTICLE_FLOWER2;
		break;
	case 6:
		byNewWeather = PARTICLE_FLOWER3;
		break;
	case 7:
		byNewWeather = PARTICLE_FLOWER4;
		break;
	case 8:
		byNewWeather = PARTICLE_FLOWER5;
		break;
	case 9:
		byNewWeather = PARTICLE_FLOWER6;
		break;
	case 10:
		byNewWeather = PARTICLE_FLOWER7;
		break;
	case 11:
		byNewWeather = PARTICLE_FLOWER8;
		break;
	case 12:
		byNewWeather = PARTICLE_FLOWER9;
		break;
	default:
		break;
	}

	if(g_pWeather->GetParticleType() != byNewWeather || byNewWeather == PARTICLE_FLOWER9)
	{
		g_pWeather->EnableParticle(byNewWeather);
	}

	if(byNewWeather == PARTICLE_RAIN)
	{
		g_pAudio->Stop(EAT_OTHER,257,g_pAudio->GetAudioRain());
		if(g_Config.GetWeather())
		{
			g_pAudio->GetAudioRain() = g_pAudio->GetRand();
			g_pAudio->Play(EAT_OTHER,256,g_pAudio->GetRand()++,true);
		}
	}
	else if(byNewWeather == PARTICLE_STORM)
	{
		g_pAudio->Stop(EAT_OTHER,256,g_pAudio->GetAudioRain());
		if(g_Config.GetWeather())
		{
			g_pAudio->GetAudioRain() = g_pAudio->GetRand();
			g_pAudio->Play(EAT_OTHER,257,g_pAudio->GetRand()++,true);
		}
	}
	else
	{
		g_pAudio->Stop(EAT_OTHER,256,g_pAudio->GetAudioRain());
		g_pAudio->Stop(EAT_OTHER,257,g_pAudio->GetAudioRain());
	}

	if(wTime == 0xFFFF || wTime < 28 || wTime > 64 )
		g_OtherData.SetDay(false);
	else
		g_OtherData.SetDay(true);


	// 天气
	DWORD dwLightColor = 0xFFFFFFFF;
	//sColor = (SColor *)(&dwWeatherColor);
	BYTE *pColor = (BYTE *)(&dwLightColor);
	if( wTime != 0xFFFF )
	{
		int iTime,i;
		iTime = wTime = wTime % 96;
		if( wTime > 60 )
			wTime = 95 - wTime;

		WORD w = 0x45 + (wTime * 0xBA / 24);
		w = min(w,0xFF);
		if(w > 0xDD && byWeather == 1)
			w = max(w - 0x22,0xDD);

		//sColor->r = (UCHAR)w;
		*(pColor + 2) = (BYTE)w;
		if(iTime >= 64 && iTime < 80)
		{
			// 黄昏
			i = 8 - abs( 8 - (iTime-64) );
			//sColor->g = (UCHAR)(w - i*0x23/8);
			//sColor->b = (UCHAR)(w - i*0x3C/8);
			*(pColor + 1) = (BYTE)(w - i*0x23/8);
			*pColor = (BYTE)(w - i*0x3C/8);
		}
		else
		{
			//sColor->g = (UCHAR)w;
			//sColor->b = (UCHAR)w;
			*(pColor + 1) = (BYTE)w;
			*pColor = (BYTE)w;
		}
	}


	//天气颜色
	if(dwColor != 0xFFFFFFFF )
	{
		//SColor * c1;
		//c1 = (SColor *)(&dwColor);
		BYTE *p1 = (BYTE *)(&dwColor);

		//sColor->r = sColor->r * c1->r / 0xFF;
		//sColor->g = sColor->g * c1->g / 0xFF;
		//sColor->b = sColor->b * c1->b / 0xFF;
		*(pColor + 2) = (*(pColor + 2)) * (*(p1 + 2)) / 0xFF;
		*(pColor + 1) = (*(pColor + 1)) * (*(p1 + 1)) / 0xFF;
		*pColor = (*pColor) * (*p1) / 0xFF;
	}

	g_pLight->SetLightColor(dwLightColor);
}

void CGameControl::SEND_Change_hair(UCHAR cHair)
{
	SET_COMMAND(CS_CHANGE_HAIR,12);
	temp[0] = cHair;
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Change_BodyColor(UCHAR cColor)
{
	SET_COMMAND(CS_CHANGE_BODYCOLOR,12);
	temp[6] = cColor;

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Kick_Out(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("与服务器的连接被强行中断！\n"
		"连接时间可能超过限制，\n"
		"或者用户请求重新连接。",MSG_RECONNECT_SERVER,0);

	output_debug("与服务器的连接被强行中断!");

	g_AIAutoMgr.ExitWaigua();

#ifdef _CHAT
	if(g_VoiceAdapter.IsInRoom())
	{
		g_VoiceAdapter.SetLoginOutAfterLeaveRoom(true);
		g_VoiceAdapter.LeaveRoom(false);
	}
	else
		g_VoiceAdapter.ExitAdapter();
#endif
}

void CGameControl::MSG_KickOff_NTF(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("与服务器的连接被强行中断！\n"
		"连接时间可能超过限制，\n"
		"或者用户请求重新连接。",MSG_RECONNECT_SERVER,0);

	g_AIAutoMgr.ExitWaigua();

#ifdef _CHAT
	if(g_VoiceAdapter.IsInRoom())
	{
		g_VoiceAdapter.SetLoginOutAfterLeaveRoom(true);
		g_VoiceAdapter.LeaveRoom(false);
	}
	else
		g_VoiceAdapter.ExitAdapter();
#endif
}

void CGameControl::MSG_Package_Update_Object(const char * msg,int iLen)
{
	DWORD dwGoodID = *((DWORD*)(msg + 56)); // 是物品数据上的 dwMakeIndex 值
	if(dwGoodID == 0)
		return;
	
	CGood* pGood = 0;
	//如果物品是托管包裹里物品，则直接在托管包裹里寻找
	BYTE type = *(BYTE*)(msg + 8);

	if (type > 0)
	{
// 		for (int i = 0; i < MAX_TRUSTEESHIP_NUM; ++i)
// 		{
// 			pGood = g_TrusteeshipData.GetMemberPackage(i).FindGood(dwGoodID);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 
// 			pGood = SELF.GetUsingTemp().FindGood(dwGoodID);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 
// 			if (dwGoodID == CGoodGrid::GetDropGoodFrom().DropGood.GetID())
// 			{
// 				pGood = &(CGoodGrid::GetDropGoodFrom().DropGood);
// 			}
// 		}
	} 
	else
	{
		int iFromWnd = NO_Wnd;
		pGood = g_pGameData->SearchGoodByID(dwGoodID,iFromWnd);
	}

	
	if(pGood)
	{
		int nlock =  pGood->GetFlag2();
		int iStdMode = pGood->GetStdMode();
		int iShape = pGood->GetShape();

		pGood->FromBuffer(msg + 12,false,iLen - 12);
		pGood->SetExternString(g_pGameData->GetExternString(dwGoodID));

		int new_lock = pGood->GetFlag2();
		int iNewStdMode = pGood->GetStdMode();
		int iNewShape = pGood->GetShape();

		//使用乾坤锁特效
		if(g_AIGoodMgr.IsCanLock(*pGood) &&  new_lock > 0 && new_lock > nlock)
		{
			GoodAddEffect &goodEffect = pGood->GetGoodAddEffect();
			goodEffect.dwStartID = MAKELONG(6905,PACKAGE_stateitem);
			goodEffect.iFrams = 12;
			goodEffect.RM = RM_ADD2;
			goodEffect.iOffX = -32;
			goodEffect.iOffY = -32;
			goodEffect.iSoundID = 203;
			return;
		}

		//持久为0的雪莲类物品，自动仍掉
		if(type == 0 && iStdMode == 0 && (iShape == 200 || iShape == 201 || iShape == 202) && pGood->GetDura() == 0)
		{
			g_AutoEatMgr.SetGoodInf(pGood->GetPos() - MAX_PACKAGE_NUM,g_AIGoodMgr.GetObjectAttr(*pGood));
			g_pGameControl->SEND_Package_Object_Reject(*pGood);
			return;
		}
		//为了显示鉴定时的特效，作的特殊处理
		if((iStdMode == 82 && iNewStdMode == 81) || (iStdMode == 83 && iNewStdMode == 58))
		{
			GoodAddEffect &goodEffect = pGood->GetGoodAddEffect();
			if(pGood->GetLooks() == 855)
				goodEffect.dwStartID = MAKELONG(1035,PACKAGE_stateitem);
			else
				goodEffect.dwStartID = MAKELONG(1020,PACKAGE_stateitem);

			goodEffect.iFrams = 10;
			goodEffect.RM = RM_ADD2;
			goodEffect.iOffX = -18;
			goodEffect.iOffY = -20;
			goodEffect.iSoundID = 203;
			return;
		}

		if(iStdMode == 49 && iShape == 51) //豹魔石的属性更新了
		{
			SELF.SetMyPetTime(pGood->GetRecordTime());
			SELF.SetMyPetLevel(pGood->GetDC());
			return;
		}
	}
	else if(dwGoodID == g_NPC.GetShenGongFangGoods().m_Shield.GetID())		//神工坊升级盾牌品质
	{
		g_NPC.GetShenGongFangGoods().m_Shield.FromBuffer(msg + 12,false,iLen - 12);
		g_NPC.GetShenGongFangGoods().bSend = false;
	}
	else if(dwGoodID == g_NPC.GetShenGongFangGoods().m_Luck.GetID())		//神工坊升级更新祈愿符捆
	{
		g_NPC.GetShenGongFangGoods().m_Luck.FromBuffer(msg + 12,false,iLen - 12);
	}
	else if(dwGoodID == g_NPC.GetYuanShiResult().m_Good.GetID())
	{
		g_NPC.GetYuanShiResult().m_Good.FromBuffer(msg + 12,false,iLen - 12);
	}	
}

void CGameControl::MSG_Weapon_Break(const char * msg,int iLen)
{
	DWORD uID = *((DWORD*)msg);

	CCharacterAttr* pChar = g_pGameData->FindCharacterByID(uID);
	if(pChar)
	{
		pChar->AddMagicState(MS_WEAPONBREAK);
	}
}

void CGameControl::MSG_Rungate_Answer(const char * msg,int iLen)
{
	int iTemp = SELF.GetReserveData(plySendRungateNums);
	SELF.SetReserveData(plyRungateLastAskTime,GetTickCount());
	SELF.SetReserveData(plyRungateNotAnswer,0);

	DWORD dwUsedTime = GetTickCount() - Conv_DWORD(msg);


	if(iTemp <= 24)			// 卡了一秒
	{
		if(dwUsedTime <= 100)
		{
			g_OtherData.SetAlarmState(1,4);// 网络
			g_OtherData.SetAlarmState(2,2);// 服务器
		}
		else
		{
			g_OtherData.SetAlarmState(1,2);// 网络
			g_OtherData.SetAlarmState(2,5);// 服务器
		}
	}
	else					// 超过一秒
	{
		if(dwUsedTime <= 300)
		{
			g_OtherData.SetAlarmState(1,4);// 网络
			g_OtherData.SetAlarmState(2,1);// 服务器
		}
		else
		{
			g_OtherData.SetAlarmState(1,1);// 网络
			g_OtherData.SetAlarmState(2,5);// 服务器
		}
	}
}

void CGameControl::SEND_Test_Rungate_Answer()
{
	DWORD dwCount = GetTickCount();
	int iTemp = (int)(dwCount - SELF.GetReserveData(plyRungateLastAskTime));

	if(SELF.GetReserveData(plyRungateLastAskTime) == 0 || iTemp > 3000)		// 3秒一上才发送一次
	{
		SET_COMMAND(CS_SENDTO_RUNGATE,12);
		ASSIGN_DWORD(0,dwCount);

		SEND_GAME_SERVER();

		if(SELF.GetReserveData(plyRungateNotAnswer))	// Rungate不支持该协议的情况(没回应)
			SELF.SetReserveData(plyRungateLastAskTime,dwCount);

		SELF.AddReserveData(plySendRungateNums);	

		int iState;
		int iCount = SELF.GetReserveData(plySendRungateNums);
		if(iCount < 20)
			iState = 2;
		else
			iState = 1;

		g_OtherData.SetAlarmState(1,iState);// 网络
		g_OtherData.SetAlarmState(2,4);// 服务器

		SELF.SetReserveData(plyRungateNotAnswer,1);
	}
}

void CGameControl::MSG_System_Skyrocket(const char * msg,int iLen)
{
	BYTE cType,cTemp,cTemp1;
	DWORD dwTime,dwTime1;

	memcpy(&dwTime,msg,4);
	memcpy(&cType,msg + 6,1);
	memcpy(&cTemp,msg + 7,1);

	switch(cType)
	{
	case 1:	// 圣诞
	case 2:	// 元旦
	case 3:	// 春节
		{
			if(cTemp == 0)	
				cTemp = 4;
			if(dwTime <= 1000)								
			{
				dwTime = 2000;
			}

			memcpy(&dwTime1,msg + 8,4);	
			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_FESTAL_SKYROCKET, 0, SELF.GetID());
			if(ms)
			{
				ms->iData2 = cType;              //节日类型
				ms->iData3 = dwTime;             //祝贺焰火时间间隔
				ms->iData4 = dwTime1;            //背景焰火持续时间
				ms->iCycles = cTemp;             ////////////////////////祝贺焰火播放次数
				ms->tFrameStart = GetTickCount();//起始时间
			}

			break;
		}
	case 4:	// 背景焰火
		{
			memcpy(&cTemp1,msg + 8,1);
			if(cTemp1 == 0)
				cTemp1 = 30;				
			if(dwTime <= 1000)
				dwTime = 20000;

			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_STREW_SKYROCKET, 0, SELF.GetID());
			if(ms)
			{
				ms->iData2 = cTemp;              //类型
				ms->iData3 = cTemp1;             //稠密程度(0 - 100)
				ms->iData4 = dwTime;             //背景焰火持续时间
				ms->tFrameStart = GetTickCount();//起始时间
			}

			break;
		}
	default:
		break;
	}
}


void CGameControl::SEND_Midi_For_Other(char *sName,char *sBody,DWORD dwMidiID)
{
	if(strlen(sName) + strlen(sBody) + 18 >= 2048)
		return;

	static char cDivide = 0x0B;
	SET_COMMAND(CS_MIDI_FOR_OTHER,2048);

	ADD_STR(sName);
	ADD_BYTE(cDivide);
	ADD_STR(sBody);
	ADD_BYTE(cDivide);
	ADD_DWORD(dwMidiID);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_System_Play_Midi(const char * msg,int iLen)
{
	DWORD dwMidiID;
	char sName[16] = {0};
	char sBody[1024] = {0};

	char *p1;
	char *p0 = (char*)(msg + 12);

	// 取得名字
	p1 = p0;
	while(1)
	{
		if(*p1 == '\0')
			return;
		if(*p1 == 0x0B)		
			break;
		p1++;
	}
	if(p1 > p0 && (p1 - p0) <= 16)
		memcpy(sName,p0,p1 - p0);
	p1++;
	p0 = p1;

	// 取得附语
	while(1)
	{
		if(*p1 == '\0')
			return;
		if(*p1 == 0x0B)		
			break;
		p1++;
	}
	if(p1 > p0 && (p1 - p0) <= 1024)
		memcpy(sBody,p0,p1 - p0);
	p1++;
	p0 = p1;

	// Midi编号
	dwMidiID = atoi(p1);

	if(dwMidiID > 0)
	{
		//播放Midi
		TRY_BEGIN
			g_pAudio->PlayMidi(dwMidiID,false);
		TRY_END_DO(g_pAudio->EnableMidi(false);)
	}
}


void CGameControl::MSG_Object_Detail(const char * msg,int iLen)
{
	char szTemp[CMD_SIZE] = {0};
	MSG_Object_Use_Failed(szTemp,CMD_SIZE); 

	string str(msg + CMD_SIZE,iLen-CMD_SIZE);
	if(str.size() > 0)
	{
		string::size_type p = str.find('\\');
		while( p != string::npos)
		{
			str[p] = '\n';
			p = str.find('\\');
		}
		g_MsgBoxMgr.PopSimpleAlert(str.c_str());
	}
}


void CGameControl::MSG_Fu_Sheng_Time(const char * msg,int iLen)
{
	DWORD dwDelayTime = Conv_DWORD(msg + 6);
	SELF.SetReserveData(plyShenYouLastTime,dwDelayTime);
}

void CGameControl::SEND_User_Real_Info(const char* szName,bool bMale,const char* szTelphone,
									   const char* szAddress,const char* szPostCode)
{
	const char* szGender = bMale?"男":"女";

	SET_COMMAND(CS_USER_REAL_INFO,1024);
	ASSIGN_ID(g_NPC.GetID());

	ADD_STR(szName);
	ADD_STR("\n");
	ADD_STR(szGender);
	ADD_STR("\n");
	ADD_STR(szTelphone);
	ADD_STR("\n");
	ADD_STR(szAddress);
	ADD_STR("\n");
	ADD_STR(szPostCode);

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Leave_Message_Info(const char* szName,const char* szAddr,const char* szContent)
{
	SET_COMMAND(CS_LEAVE_MESSAGE_INFO,512);
	ASSIGN_ID(g_NPC.GetPackGoodID());
	ADD_STR(szName);
	ADD_BYTE(0);
	ADD_STR(szAddr);
	ADD_BYTE(0);
	ADD_STR(szContent);
	ADD_BYTE(0);
	ASSIGN_WORD(6,iBLen - 12);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Rungate_Encrypt(const char * msg,int iLen)
{
	//原封不动发回去
	g_pNet->SendBuf(SERVER_GAME,(char *)msg,iLen);
}


void CGameControl::MSG_Server_Switch(const char* strMsg,int iLen)
{
	g_dwServerSwitch = Conv_DWORD(strMsg + 6);

	if (!g_pMagicCtrl || !g_pMagicCtrl->GetMagicRoot(MAGICID_ENABLE_CREDIT))
	{
		g_dwServerSwitch &= ~SS_CREDIT;
	}

	//服务器全局变量也可以控件IGW是否开启
	if(g_pSDOAInterface && (g_dwServerSwitch & SS_IGW) == 0)
	{
		TRY_BEGIN
		SAFE_DELETE(g_pSDOAInterface);
		TRY_END
		g_pSDOAInterface = NULL;
	}
}

void CGameControl::SEND_Ready_Quit(int iQuitType)
{
	SET_COMMAND(CS_READY_QUIT,CMD_SIZE);
	ASSIGN_BYTE(10,iQuitType);
	SEND_GAME_SERVER();
}

/////好友队列信息中徒弟是否收到师尊令的标志
//x = byFlag; //个数
//y = strTokeyInfo.size(); //字符串长度
//direction = byType; //类型：0-所有获得授权徒弟信息 1－某个徒弟获得授权 2-某个徒弟
//剥夺授权
void CGameControl::MSG_Prentice_Flag(const char* msg,int iLen)
{

	WORD wCount =  *((WORD*)(msg+6));
	WORD iLength   =  *((WORD*)(msg+8));
	WORD iType  =  *((WORD*)(msg+10));

	string strTemp;
	strTemp.clear();
	if(iLength>256)
		return;

	VRelationStruct::iterator itr;
	if(iType == 0 && wCount>0 && wCount <=3 && iLength>0)
	{

		string name[3];

		strTemp.assign(msg+12,iLength);
		for(int i=0;i<3;i++)
		{
			name[i].clear();
			int first =strTemp.find('\0');
			if( first != string::npos )
			{
				name[i] = strTemp.substr(0,first);
				strTemp = strTemp.substr(first+1);

			}
		}

		//更新徒弟信息
		for(itr = g_OtherData.GetRelationVector().begin();itr!=g_OtherData.GetRelationVector().end();itr++)
		{
			if( (*itr).iRelType == 2 )
			{
				if( (*itr).strName == name[0] ||
					(*itr).strName == name[1] ||
					(*itr).strName == name[2]
				)
				{
					(*itr).bHasXuanTieFlag  = true;
				}
			}
		}
	}
	if(iType == 1)
	{
		strTemp.assign(msg+12,iLength);
		//更新徒弟信息
		for(itr= g_OtherData.GetRelationVector().begin();itr!= g_OtherData.GetRelationVector().end();itr++)
		{
			if( (*itr).iRelType == 2 && (*itr).strName == strTemp)
			{
				(*itr).bHasXuanTieFlag  = true;
				char strTemp1[256];
				memset(strTemp1,0,sizeof(strTemp1));
				sprintf(strTemp1,"师尊令已经授予徒弟%s",strTemp.c_str());
				g_TalkMgr.PushSysTalk(strTemp1);
			}
		}
	}
	if(iType ==2 )
	{
		strTemp.assign(msg+12,iLength);
		//更新徒弟信息
		for(itr=g_OtherData.GetRelationVector().begin();itr!=g_OtherData.GetRelationVector().end();itr++)
		{
			if( (*itr).iRelType == 2 && (*itr).strName == strTemp)
			{
				(*itr).bHasXuanTieFlag  = false;
				char strTemp1[512] = {0};
				sprintf(strTemp1,"你的徒弟%s的师尊令已被收回，快去找武官教头领取该师尊令。",strTemp.c_str());
				g_TalkMgr.PushSysTalk(strTemp1);
			}
		}
	}


}

void CGameControl::SEND_Use_ReliveItem(DWORD dwTargetID,DWORD dwGoodID)
{
	SET_COMMAND(CS_USE_RELIVE_ITEM,12);
	ASSIGN_ID(dwTargetID);
	ASSIGN_DWORD(6,dwGoodID);
	SEND_GAME_SERVER();
}


//第6字节 为1表示是天绝魔域服务器
void CGameControl::MSG_Player_Connected(const char * strMsg,int iLen)
{
	g_Login.SetIsTianJueMoYu(strMsg[6] == 1?true:false);
}

void CGameControl::MSG_ServerVersion(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
	g_dwServerVersion = 0;

	VString vStr = StringUtil::split(buf,",");
	for(int i = 0; i < vStr.size() && i < 4;i++)
	{
		g_dwServerVersion = g_dwServerVersion* 10 + atoi(vStr[i].c_str());
	}

	SELF.SetDead(false);
}

void  CGameControl::Msg_Update_Medal_Attribute(const char * msg,int iLen)
{
	CGood& good = SELF.GetEquipGood(ITEM_POS_MEDAL);
	DWORD dwID = Conv_DWORD(msg);
	if(good.GetID() == dwID)
	{
		for(int i= 0;i<4;i++)
		{
			good.GetResvEx(1+i) = msg[6+i];
		}
	}
}

void  CGameControl::Msg_Update_Medal_Exp(const char * msg,int iLen)
{
	int iNum = msg[6];
	if( iNum < 8 )
	{
		for(int i = 0;i<iNum;i++)
		{
			DWORD dwData = Conv_DWORD(msg + 12 + i * 4);
			g_OtherData.SetUpdateMedalExp(i,dwData);
		}		
	}
}

void CGameControl::Msg_Take_Put_Seal(const char* msg,int iLen)
{
	//SELF.GetUsingTemp().SetID(0);
}

//五行令旗，使用成功时的消息处理
void CGameControl::Msg_WuXing_Flag(const char* msg,int iLen)
{
	WORD wType = *((WORD*)(msg+10));
	DWORD dwValue = *((DWORD*)(msg + 6));
	if(wType == 1)//怪物ID
	{
		g_pMagicCtrl->FinishMagic(g_pMagicCtrl->FindMagicByID(MAGICID_WUXING_FALG_MONSTER_STATE));
		g_pMagicCtrl->CreateMagic(MAGICID_WUXING_FLAG_START,0,SELF.GetID(),dwValue);

		DWORD dwDir = *((DWORD*)(msg));
		char cDir = (char)dwDir;
		g_OtherData.GetWuXingFlag().dwFlagInMonstID = dwValue;
		g_OtherData.GetWuXingFlag().bFinishFlag = false;
		g_OtherData.GetWuXingFlag().bUseFlag = true;
		g_OtherData.GetWuXingFlag().bMonsterStart = false;

		//增加一个魔法
		SELF.InitAction(ACTION_MAGIC);
		SAction& ActionNow = SELF.GetAction();

		int iX, iY;
		g_pGameMgr->GetMouseTile(iX,iY);
		ActionNow.iAimX = iX;
		ActionNow.iAimY = iY;
		ActionNow.uTarget = g_pControl->GetMouseOnID();        

		int offx,offy;
		SELF.GetOffset(offx,offy);
		SELF.SetDir(cDir);		
	}
	else if(wType == 2)	//回合结束，获得经验值
	{
		g_NPC.GetWuXingResult().dwRound = HIWORD(Conv_DWORD(msg));
		g_NPC.GetWuXingResult().bySuccess = (BYTE)LOWORD(Conv_DWORD(msg));
		g_NPC.GetWuXingResult().dwRoundExp = Conv_DWORD(msg + 6) / 10000;
		g_NPC.GetWuXingResult().bRoundOver = true;
		g_NPC.GetWuXingResult().dwRoundOverTm = GetTickCount();
	}
	else if(wType == 3)//守卫上限
	{
		g_OtherData.GetWuXingFlag().dwGuarderNum = dwValue;

		if(Conv_DWORD(msg) != 1)	//表明守卫数发生了改变，1为初始值
		{
			g_OtherData.GetWuXingFlag().dwGuardChangeTm = GetTickCount();
		}
	}
	else if(wType == 4)//逃跑的怪物数目
	{
		g_OtherData.GetWuXingFlag().dwEscapeMon = dwValue;

		if(dwValue != 0)	//表明有怪物逃逸了，0为初始值
		{
			g_OtherData.GetWuXingFlag().dwEscapeMonTm = GetTickCount();
		}
	}
	else if(wType == 5)//香炉时间
	{
		g_OtherData.SetCenserStart(true);
		g_OtherData.SetCenserStartTime(GetTickCount());
		g_OtherData.SetCenserDurTime(dwValue * 1000 - 10000);

		g_OtherData.GetWuXingFlag().dwLeftTime = 10;				//倒计时20秒
		g_OtherData.GetWuXingFlag().dwStartTime = GetTickCount();

		_TimeOut _tout("",GetTickCount(),10000,1,true,10);//倒计时20秒
		g_OtherData.AddTimeOut(_tout);

		g_OtherData.GetWuXingFlag().dwCurrentRound = 1;
		g_NPC.GetWuXingResult().bRoundOver = false;
		g_NPC.GetWuXingResult().bGameOver = false;

		g_pMagicCtrl->FinishAllMagic(true);
		SELF.SetReserveData(pubDisableDraw,0);
		SELF.SetUsingMagicType(0);
	}
	else if(wType == 6)//地表魔法，封元印区性 67 x, 89 y WORD值
	{
		DWORD dwData = *((DWORD*)(msg + 6));
		g_pMagicCtrl->CreateMagic(MAGICID_CALL_MONSTER_WXXG,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,dwData);
	}
	else if(wType == 7)     //怪物消失
	{
		if(g_OtherData.GetWuXingFlag().dwFlagInMonstID == dwValue)
		{
			g_OtherData.GetWuXingFlag().dwFlagInMonstID = 0;
		}
		CSimpleCharacterNode * pChar = g_pGameData->FindSimpleCharacter(dwValue);

		if(pChar)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_PLAYER_QUIT_YUANSHEN,0,dwValue,0);
		}        
	}
	else if(wType == 10)	//玩家尝试在高台上召唤近战怪物
	{
		g_MsgBoxMgr.PopSimpleAlert("\n         此怪物只有在高台下召出才能发挥其战斗能力。");
	}
	else if(wType == 11)	//玩家的封印灵力不足
	{
		g_MsgBoxMgr.PopSimpleAlert("\n封元神力不足，无法召唤此怪物。消灭逃跑怪物可以累积更多的封元神力。");
	}
}

void  CGameControl::SEND_WuXing_Apply_Monster(DWORD dwID,int iX,int iY)
{	
	SET_COMMAND(CS_WUXING_APPEAR_POINT,CMD_SIZE);
	ASSIGN_DWORD(0,dwID);
	ASSIGN_WORD(6,iX);
	ASSIGN_WORD(8,iY);  
	SEND_GAME_SERVER();
}
//前四个字节为存或取的封元印 == dwID
//下标6为 1=存  2=取
//下表8、9、10、11为风元册 == dwConID
void CGameControl::SEND_Seal_Info(DWORD dwID,int iType,DWORD dwConID)
{
	SET_COMMAND(CS_SEAL_TAKE_PUT,CMD_SIZE);
	ASSIGN_DWORD(0,dwID);
	char c = (CHAR)iType;
	ASSIGN_BYTE(6,c);
	ASSIGN_DWORD(8,dwConID);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_WuXing_Request_Prize()	//五行玄关完成后请求服务器发奖
{
	SET_COMMAND(CS_FLAG_WUXING,CMD_SIZE);
	ASSIGN_BYTE(10,1);
	SEND_GAME_SERVER();
}


//void CGameControl::CheckUseBCSS(CGood temp,int iPos)
//{
//	if(temp.GetID() == 0 || temp.GetStdMode() == 25 || temp.GetStdMode() == 34	||
//		temp.GetStdMode() == 59 || temp.GetStdMode() == 60 ||
//		temp.GetStdMode() == 61 || temp.GetStdMode() == 13 )
//		return;
//
//	if( temp.GetDura() < 1500 )
//	{
//		//快捷消费2.0
//		DWORD dwGoodID = 0;
//		int iSize = SELF.PackageGood().Size();
//		for(int i = 0; i < iSize; i++)
//		{
//			CGood& good = SELF.GetPackageGood(i);
//
//			if(good.GetID() == 0)
//				continue;
//
//			if( (strcmp(good.GetName(),"超级冰泉圣水") == 0 && good.GetDura() > 0 ) || strcmp(good.GetName(),"冰泉圣水") == 0  )
//			{
//				dwGoodID = good.GetID();
//				break;
//			}
//		}
//
//		if(dwGoodID == 0)
//		{
//			g_WooolStoreMgr.SetQuickBuyItem("超级冰泉圣水");
//			CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();
//
//			if(QuickBuyData.pItem)
//			{
//				char cTemp[128]={0};
//				sprintf(cTemp,"您的%s即将损坏，需要立即购买超级冰泉圣水并使用吗？单价%d元宝",
//					temp.GetName(),QuickBuyData.pItem->iPrice);
//
//				QuickBuyData.strMsg = cTemp;
//				QuickBuyData.iType = 1;
//				QuickBuyData.iUseAfterBuyType = 1;
//				QuickBuyData.iObjPos = iPos;
//
//				g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);
//			}
//		}
//	}
//}

//0:关闭窗口,1:再来一次,2:请求兑换离火令
void CGameControl::SEND_PLAY_WXXG(BYTE byType)
{
	SET_COMMAND(CS_FLAG_WUXING,CMD_SIZE); 
	ASSIGN_BYTE(10,10);

	ASSIGN_BYTE(11,byType);

	SEND_GAME_SERVER();
}

void  CGameControl::MSG_Flash_Voice_Btn(const char* strMsg2,int iLen)
{    
	if(strMsg2[0] == 1)
		g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_VOICE_BTN_FLASH,0);
	else
		g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_VOICE_BTN_FLASH,1);

}

//第8位是发送人名的个数,0-3为战斗值 12位后格式为 人名14字节＋是否激活(1位)＋人名长度（14字节）＋是否激活(1字节).....
void  CGameControl::MSG_Guild_Stele_Parter(const char* strMsg2,int iLen)
{    
	std::vector<CGuildData::_GuildStele>& vGuildStelMem = g_GuildData.GetGuildSteleMember();
	vGuildStelMem.clear();

	BYTE byNum = strMsg2[8];

	int ioff = 12;
	string strName;
	BYTE byState = 0;
	g_GuildData.SetGuildSteleFghValue(*((DWORD*)(strMsg2)));

	//参赛人员
	CGuildData::_GuildStele member;
	for(int i = 0; i < byNum; i++)
	{
		member.strName.assign(strMsg2+ioff,14);
		ioff += 14;
		member.bPart = (*((BYTE*)(strMsg2+ioff)) != 0);
		ioff += 1;

		vGuildStelMem.push_back(member);
	}

	g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_RECREATE,MAKELONG(2,16));
}

//第8位是发送人名的个数,12位后格式为 行会名30字节+人名14字节＋是否激活(1位)＋人名长度（14字节）＋是否激活(1字节).....
void  CGameControl::MSG_Kfz_Join_Member(const char* strMsg2,int iLen)
{    
	_KfzGuild &KfzGuild = g_OtherData.GetKfzGuildData();
	vector<_KfzMember> & VMemberList = KfzGuild.VKfzMemberList;
	VMemberList.clear();

	BYTE byNum = strMsg2[8];

	int ioff = 12;
	string strName;
	WORD wNameLen = 0;
	BYTE byState = 0;

	//行会名字
	KfzGuild.strGuildName.assign(strMsg2+ioff,30);
	ioff += 30;

	///////////////////////////////////参赛人员
	for(int i = 0; i < byNum; i ++)
	{
		strName.assign(strMsg2+ioff,14);
		ioff += 14;
		byState = *((BYTE*)(strMsg2+ioff));
		ioff += 1;

		VMemberList.push_back(_KfzMember(strName.c_str(),-1,-1,-1,byState));
	}
	////////////////
	g_pControl->Msg(MSG_CTRL_KFZ_MEMBER_LIST_WND,OPER_RECREATE);/////////

}
/////报名参赛人员,下标6为人员个数,下标7为类型(0=观众，1=参赛者),下标12后为参赛名单,14字节一个人名
void CGameControl::SEND_Kfz_Confirm_Member()
{
	_KfzGuild &KfzGuild = g_OtherData.GetKfzGuildData();
	vector<_KfzMember> & VMemberList = KfzGuild.VKfzMemberList;
	size_t iNum = VMemberList.size();

	SET_COMMAND(CS_KFZ_CONFIRM_MEMBER,CMD_SIZE + iNum*14); 
	ASSIGN_BYTE(6,iNum);
	ASSIGN_BYTE(7,1);


	for(size_t i = 0; i < iNum; i ++)
	{
		_KfzMember & member = VMemberList[i];
		ADD_ARRAY(member.strName.c_str(),14);
	}


	SEND_GAME_SERVER();
}
//请求本服参加跨服战人员名单
void CGameControl::SEND_Request_Kfz_Join_Member()
{
	SET_COMMAND(CS_KFZ_JOIN_MEMBER,CMD_SIZE); 
	SEND_GAME_SERVER();
}

//观众报名观看跨服战
void  CGameControl::MSG_Kfz_Visitor_Join(const char* strMsg2,int iLen)
{   
	SEND_Kfz_Activate();
}
//下标12以后为密码。
void CGameControl::SEND_Guild_Stele_Register()
{
	SET_COMMAND(CS_STELE_REGISTER,CMD_SIZE);
	string strPwd = g_Login.GetPassWord();
	char strTemp[512]={0};
	for(int i = 0;i < strPwd.length();i++)
	{
		strTemp[i] = strPwd.c_str()[i] ^ g_byPwdEncode;
	}
	strPwd = g_pNet->NewDecode(strTemp,strlen(strTemp));

	ADD_STR(strPwd.c_str());
	SEND_GAME_SERVER();
}
//下标12以后为密码。
void CGameControl::SEND_Kfz_Activate()
{
	SET_COMMAND(CS_KFZ_ACTIVATE,CMD_SIZE); 
	string strPwd = g_Login.GetPassWord();
	char strTemp[512]={0};
	for(int i = 0;i < strPwd.length();i++)
	{
		strTemp[i] = strPwd.c_str()[i] ^ g_byPwdEncode;
	}
	strPwd = g_pNet->NewDecode(strTemp,strlen(strTemp));

	ADD_STR(strPwd.c_str());
	SEND_GAME_SERVER();
}
//仙灵碑精英最大数目
void CGameControl::MSG_Guild_Stele_HeroNum(const char* strMsg2,int iLen)//返回精英战将人数
{
	WORD wNum = *((WORD*)(strMsg2 + 6));
	g_GuildData.SetGuildSteleHeroNum(wNum);
}
void CGameControl::DealHeavenCityStatus(DWORD dwType,DWORD dwPos,DWORD dwStatus)
{
	if (dwType == 1)//初始化
	{
		//阻挡点清除+对象可见

	}
	else if (dwType == 2)
	{
		//对象可见+特效+阻挡点清除与设置

	}
}

void CGameControl::MSG_HeavenCity_Status(const char* msg,int iLen)
{
	WORD wType = *((WORD*)msg);
	int iPos = 12;
	BYTE byPassBlock = 0;
	if (wType == 1)
	{
		//初始化状态
		while (iPos < iLen)
		{
			//路通的点，即死的怪
			byPassBlock = *((BYTE*)(msg + iPos));
			iPos++;

			if (byPassBlock > 14)
				continue;

			DealHeavenCityStatus(wType,byPassBlock,0);//0是通的			
		}
	}
	else if (wType == 2)
	{
		//改变的状态
		while (iPos < iLen)
		{
			byPassBlock = *((BYTE*)(msg + iPos));
			DealHeavenCityStatus(wType,byPassBlock,0);
			iPos++;
			byPassBlock = *((BYTE*)(msg + iPos));
			DealHeavenCityStatus(wType,byPassBlock,1);
			iPos++;
		}
	}
}

void CGameControl::MSG_General_Operate_Result(const char* msg,int iLen)
{
	WORD wType = *((WORD*)(msg + 6));
	WORD bResult = *((WORD*)(msg + 8));

	switch (wType)
	{
	case 1://创建宗派成功
		{
			if (bResult == 0)//成功
			{
				if(!g_pControl->FindWindowByName("RelationWnd"))
				{
					g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_CREATE);
					g_pControl->Msg(MSG_CTRL_RELATION_WND,22);
				}
			}
		}
		break;
	case 2://淬炼结果
	case 3://拆卸宝石结果
		{
			if(bResult == 0)//成功
			{
				g_pControl->Msg(MSG_CTRL_EQUIP_LEVEL_UP_WND,10);
			}
			else
			{
				g_pControl->Msg(MSG_CTRL_EQUIP_LEVEL_UP_WND,11);
			}
		}
		break;	
	case 4://打孔
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("恭喜您打孔成功！");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)	//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)	//该物品不能用来打孔
			{
				g_MsgBoxMgr.PopSimpleAlert("该物品不能用来打孔");
			}
			else if(bResult == 3)	//固化中的物品不能打孔
			{
				g_MsgBoxMgr.PopSimpleAlert("固化中的物品不能打孔");
			}
			else if(bResult == 4)	//请用打孔钻头来打孔
			{
				g_MsgBoxMgr.PopSimpleAlert("请用打孔钻头来打孔");
			}
			else if(bResult == 5)	//已经达到孔数的上限
			{
				g_MsgBoxMgr.PopSimpleAlert("已经达到孔数的上限");
			}			
			else if(bResult == 7)	//打孔失败
			{
				//g_MsgBoxMgr.PopSimpleAlert("    这次似乎不太幸运，打孔失败了，打孔材料也破碎掉了。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 1);
			}
		}
		break;
	case 5://镶嵌符石
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("    一阵宝光闪过，您的装备被成功的镶嵌了一颗符石，它的能力将得到极大的提升！");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)	//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)	//该物品不能用来镶嵌符石
			{
				g_MsgBoxMgr.PopSimpleAlert("该物品不能用来镶嵌符石");
			}
			else if(bResult == 3)	//请用符石来镶嵌
			{
				g_MsgBoxMgr.PopSimpleAlert("请用符石来镶嵌");
			}
			else if(bResult == 4)	//请使用符石镶嵌符
			{
				g_MsgBoxMgr.PopSimpleAlert("请使用符石镶嵌符");
			}
			else if(bResult == 5)	//该属性孔不存在，无法镶嵌符石
			{
				g_MsgBoxMgr.PopSimpleAlert("没有该属性的孔洞，请换1颗符石或者选择转换孔洞的属性。");
			}
			else if(bResult == 6)	//该孔已经镶嵌了符石，所以无法再镶嵌
			{
				g_MsgBoxMgr.PopSimpleAlert("该属性的孔已经全部镶嵌了符石，请换1颗符石或者选择转换孔洞的属性。");
			}
			else if(bResult == 7)	//某某符石无法镶嵌到该装备
			{
				g_MsgBoxMgr.PopSimpleAlert("该符石无法镶嵌到该装备");
			}
			else if(bResult == 8)	//符石镶嵌失败	
			{
				//g_MsgBoxMgr.PopSimpleAlert("    镶嵌失败了！符石也消失了。放入符石镶嵌符（高级）可以保证您的镶嵌100%成功。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 1);
			}
			else if(bResult == 9)	//非法符石等级
			{
				g_MsgBoxMgr.PopSimpleAlert("非法符石等级");
			}
			else if(bResult == 10)	//非法符石镶嵌符
			{
				g_MsgBoxMgr.PopSimpleAlert("非法符石镶嵌符");
			}
		}
		break;
	case 6://符石摘除
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("    叮叮叮，一颗符石被您成功的从装备上摘除了下来。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)	//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)	//请放入镶嵌过符石的装备
			{
				g_MsgBoxMgr.PopSimpleAlert("请放入镶嵌过符石的装备");
			}
			else if(bResult == 3)	//固化中的物品不能摘除
			{
				g_MsgBoxMgr.PopSimpleAlert("固化中的物品不能摘除");
			}
			else if(bResult == 4)	//包裹空间不足，请先整理出足够空位
			{
				g_MsgBoxMgr.PopSimpleAlert("包裹空间不足，请先整理出足够空位");
			}
			else if(bResult == 5)	//只有符石摘除符才能摘除符石
			{
				g_MsgBoxMgr.PopSimpleAlert("只有符石摘除符才能摘除符石");
			}
			else if(bResult == 6)	//非法符石摘除符等级
			{
				g_MsgBoxMgr.PopSimpleAlert("非法符石摘除符等级");
			}
			else if(bResult == 7)	//该孔没有镶嵌过符石
			{
				g_MsgBoxMgr.PopSimpleAlert("该孔没有镶嵌过符石");
			}
			else if(bResult == 8)	//符石摘除符和符石的等级不一致
			{
				g_MsgBoxMgr.PopSimpleAlert("    符石摘除符和符石的等级不一致,请放入与所摘除符石等级一致的符石摘除符");
			}
			
		}
		break;
	case 7://雕琢
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("    您的符石雕琢成功！一颗闪亮的新符石诞生，它的属性得到了极大的提升。赶快去镶嵌吧！");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)//包裹空间不足，请先整理出足够空位
			{
				g_MsgBoxMgr.PopSimpleAlert("包裹空间不足，请先整理出足够空位");
			}
			else if(bResult == 3)	//请用非纯净符石来雕琢
			{
				g_MsgBoxMgr.PopSimpleAlert("请用非纯净符石来雕琢");
			}
			else if(bResult == 4)	//只有符石雕琢符才能雕琢符石
			{
				g_MsgBoxMgr.PopSimpleAlert("只有符石雕琢符才能雕琢符石");
			}
			else if(bResult == 5)//符石雕琢符和符石的等级不一致
			{
				g_MsgBoxMgr.PopSimpleAlert("符石雕琢符和符石的等级不一致");
			}
			else if(bResult == 6)	//非法符石等级
			{
				g_MsgBoxMgr.PopSimpleAlert("非法符石等级");
			}
			
		}
		break;
	case 8://合成
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("    英雄今天真是吉星高照！您的符石合成成功啦！");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)	//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)	//只有符石才能合成
			{
				g_MsgBoxMgr.PopSimpleAlert("只有符石才能合成");
			}
			else if(bResult == 3)	//你放入的不是符石合成符
			{
				g_MsgBoxMgr.PopSimpleAlert("请放入符石合成符来合成符石");
			}
			else if(bResult == 4)	//非法符石的等级
			{
				g_MsgBoxMgr.PopSimpleAlert("非法符石等级");
			}
			else if(bResult == 5)	//符石已经是最高级，不能继续合成
			{
				g_MsgBoxMgr.PopSimpleAlert("符石已经是最高级，无法继续合成");
			}
			else if(bResult == 6)	//符石合成符和符石等级不一致
			{
				g_MsgBoxMgr.PopSimpleAlert("符石合成符和符石等级不一致");
			}
			else if(bResult == 7)//有处于固化状态的符石，所以无法合成
			{
				g_MsgBoxMgr.PopSimpleAlert("处于固化状态的符石无法合成");
			}
			else if(bResult == 8)//符石的种类不一致，所以无法合成
			{
				g_MsgBoxMgr.PopSimpleAlert("符石的种类不一致，所以无法合成");
			}
			else if(bResult == 9)	//符石的等级不一致，所以无法合成
			{
				g_MsgBoxMgr.PopSimpleAlert("符石的等级不一致，所以无法合成");
			}
			else if(bResult == 10)	//符石合成失败
			{
				//g_MsgBoxMgr.PopSimpleAlert("    符石合成失败！参与合成的符石数量与正确等级的符石合成符是影响符石合成成败的关健。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 1);
			}
		}
		break;
	case 9://转换孔属性
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("    您成功的更改了装备上孔洞的属性，现在去找符合孔洞属性的符石来镶嵌吧！");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)	//失败
			{
				g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
			}
			else if(bResult == 2)	//固化中的装备不能转换孔属性
			{
				/////////////////////////////////////////////////////////////////////////
				g_MsgBoxMgr.PopSimpleAlert("固化中的装备不能转换孔属性");
			}
			else if(bResult == 3)//////////////////////////////////////选择的孔洞属性和转换的属性相同，无需再转换
			{
				g_MsgBoxMgr.PopSimpleAlert("选择的孔洞属性和转换的属性相同，无需再转换");
			}
			else if(bResult == 4)	//该孔不存在，无法转换孔属性
			{
				g_MsgBoxMgr.PopSimpleAlert("该孔不存在，无法转换孔属性");
			}			
			else if(bResult == 6)	//只有转换乾坤才能进行孔属性转化
			{
				g_MsgBoxMgr.PopSimpleAlert("只有转换乾坤才能进行孔属性转化");
			}
			else if(bResult == 7)	//该孔已经镶嵌过符石，无法转换孔属性
			{
				g_MsgBoxMgr.PopSimpleAlert("该孔已经镶嵌过符石，无法转换孔属性");
			}
			else if(bResult == 8)	//请放入打过孔的装备
			{
				g_MsgBoxMgr.PopSimpleAlert("请放入打过孔的装备");
			}
		}
		break;
	case 10:
		{
			g_NPC.GetDrillResult().iResult = bResult;

			if(bResult == 0)//成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("一阵宝光闪过，您的装备强化成功！装备上的星星增加了一颗。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
			}
			else if(bResult == 1)
			{
				g_MsgBoxMgr.PopSimpleAlert("装备无法强化。");
			}
			else if(bResult == 2)
			{
				g_MsgBoxMgr.PopSimpleAlert("非法物品");
			}
			else if(bResult == 3)	//绑定物品
			{
				g_MsgBoxMgr.PopSimpleAlert("绑定的装备无法强化。");
			}
			else if(bResult == 6)	//钱不够
			{
				g_MsgBoxMgr.PopSimpleAlert("金币不足。");
			}
			else if(bResult == 7)	//不能强化
			{
				g_MsgBoxMgr.PopSimpleAlert("本装备无法强化。");
			}
			else if(bResult == 9)	//放入的升级符与装备等级不符
			{
				g_MsgBoxMgr.PopSimpleAlert("强化符的品级与装备品级不符");
			}
			else if(bResult == 10)	//带幸运符成功
			{
				//g_MsgBoxMgr.PopSimpleAlert("在幸运符的护佑下，一阵宝光闪过，您的装备强化成功！装备上的星星增加了一颗。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 10);
			}
			else if(bResult == 11)	/////////////////失败，有护宝符
			{
				//g_MsgBoxMgr.PopSimpleAlert("这次似乎不太幸运啊。强化失败了，但是在护宝符的护佑下，你的装备完好无损。");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 11);
			}
			else if(bResult == 12)	//失败，没护宝符
			{
				//g_MsgBoxMgr.PopSimpleAlert("这次似乎不太幸运啊。强化失败了，没有护宝符的保护，装备也破碎了（符石完好无损但变为绑定状态还回您的包裹里）");
				g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM + 1);
			}
		}
		break;
	case 11:
	case 12:
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				{
					str = "恭喜你炼制成功！";
					if (wType == 12) str = "恭喜你铸造成功！";					
				}
				break;
			case 1:
				str = "系统忙！";
				break;
			case 2:
				str = "魔法等级非法";
				break;
			case 3:
				str = "没学过技能！";
				break;
			case 4:
				str = "物品非法！";
				break;
			case 5:
				str = "物品绑定！";
				break;
			case 6:
				str = "不是图纸或配方！";
				break;
			case 7:
				str = "图纸(配方)不能制炼(铸造)";
				break;
			case 8:
				str = "包裹里有复制物品！";
				break;
			case 9:
				str = "图纸(配方)名称不对";
				break;
			case 10:
				{
					g_WooolStoreMgr.SetQuickBuyItem("精力药水(大)");
					CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

					if(QuickBuyData.pItem)
					{
						char cTemp[128]={0};
						sprintf(cTemp,"精力值不够，是否购买精力药水补充精力值？单价%d元宝",QuickBuyData.pItem->iPrice);
						QuickBuyData.iUseAfterBuyType = 6;
						QuickBuyData.strMsg = cTemp;
						QuickBuyData.iType = 1;
						g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);						
					}
				}
				return;
			case 11:
				str = "材料不够，请在包裹中准备所需的材料！";
				break;
			case 12:
				str = "收物品失败！";
				break;
			case 13:
				str = "概率没到！";
			default:
				{
					str = "本次制炼失败。制炼技能等级越高，每次制炼物品成功概率就越高。";
					if (wType == 12)
						str = "本次铸造失败。铸造技能等级越高，每次铸造装备成功概率就越高！";///////////////////////		
					
					str+="\r\n \r\n注：镶嵌过符石的宝石不可再作为制作装备的材料（如有需要，可以先摘除掉宝石上的符石再进行生产制作）";
				}
				break;				
			}
			g_pControl->MsgToWnd("ProduceWnd",MSG_CTRL_CLEAN_PROCUDE_GOOD_INFO,bResult);
			if (bResult != 0)
				g_MsgBoxMgr.PopSimpleAlert(str.c_str());
			//g_TalkMgr.PushSysTalk(str.c_str());			
		}
		break;
	case 13://幸运珍宝阁
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				break;
			case 1:
				str = "系统忙，请稍后再试！";
				break;
			case 2:
				str = "您的等级小于20级，只有等级20级以上才能来抽奖。";
				break;
			case 3:
				str = "您今天抽奖机会已经用完。";
				break;
			default:
				str = "系统忙，请稍后再试。";
			}
			if(bResult!=0)
				g_TalkMgr.PushSysTalk(str.c_str());
		}
		break;
	case 14://天佑中州字符转换
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "转换成功！";
				break;
			case 1:
				str = "系统忙，请稍后再试！";
				break;
			case 2:
				str = "每天只有一次转换字符的机会，你今天已经转换过了，无法再次转换。";
				break;
			case 3:
				str = "必须使用转换乾坤来转换字符。";
				break;
			case 4:
				str = "你放入的不是“天佑中州”四个字符之一！";
				break;
			case 5:
				str = "请选择不同的字符来转换。";
				break;
			case 6:
				str = "转换失败";
				break;
			case 7:
				str = "您的等级小于10级，只有等级10级以上才能转换字符。";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}
			g_TalkMgr.PushSysTalk(str.c_str());
		}
		break;
	case 15:
		{
			if(bResult == 1)	
			{
				g_MsgBoxMgr.PopSimpleAlert("系统正忙，请再次尝试！");
			}
			else if(bResult == 2)
			{
				g_MsgBoxMgr.PopSimpleAlert("请先领取了你的奖品或卖出元宝所得的金币后再来吧。");
			}
			else if(bResult == 3)
			{
				////////////////////////////////////////////////////////////////
				g_MsgBoxMgr.PopSimpleAlert("输入的元宝数量不正确，请重新输入。");
			}
			else if(bResult == 4)
			{
				g_MsgBoxMgr.PopSimpleAlert("输入的元宝数量不能大于200，请重新输入。");
			}
			else if(bResult == 5)
			{
				g_MsgBoxMgr.PopSimpleAlert("输入的单价数量不正确，请重新输入。");
			}
			else if(bResult == 6)
			{
				g_MsgBoxMgr.PopSimpleAlert("输入的单价数量不能大于500万，请重新输入。");
			}
			else if(bResult == 7)
			{
				g_MsgBoxMgr.PopSimpleAlert("很抱歉，您每隔一分钟才能操作元宝一次！");
			}
			else if(bResult == 8)
			{
				g_MsgBoxMgr.PopSimpleAlert("元宝系统已经关闭！");
			}
			else if(bResult == 9)
			{
				g_MsgBoxMgr.PopSimpleAlert("个人的元宝系统已经关闭！");
			}
			else if(bResult == 10)
			{
				g_MsgBoxMgr.PopSimpleAlert("很抱歉，你没有足够的元宝！");
			}
			else if(bResult == 11)
			{
				g_MsgBoxMgr.PopSimpleAlert("你的元宝正在寄售中！");
			}
		}
		break;
	case 16:
		{
			if(bResult == 1)	
			{
				g_MsgBoxMgr.PopSimpleAlert("系统正忙，请再次尝试！");
			}
			else if(bResult == 2)
			{
				g_MsgBoxMgr.PopSimpleAlert("输入的元宝数量不正确，请重新输入");
			}
			else if(bResult == 3)
			{
				g_MsgBoxMgr.PopSimpleAlert("输入的元宝数量不能大于200，请重新输入。");
			}
			else if(bResult == 4)
			{
				g_MsgBoxMgr.PopSimpleAlert("你似乎没有那么多钱啊，还是先少买点元宝吧！");
			}
			else if(bResult == 5)
			{
				g_MsgBoxMgr.PopSimpleAlert("抱歉，您每隔一分钟才能操作元宝一次！");
			}
			else if(bResult == 6)
			{
				g_MsgBoxMgr.PopSimpleAlert("元宝系统已经关闭！");
			}
			else if(bResult == 7)
			{
				g_MsgBoxMgr.PopSimpleAlert("个人寄售元宝系统已经关闭！");
			}
			else if(bResult == 8)
			{
				g_MsgBoxMgr.PopSimpleAlert("你已经拥有太多元宝了，还是用完之后再来买吧！");
			}
			else if(bResult == 9)
			{
				g_MsgBoxMgr.PopSimpleAlert("寄售元宝数量不足或者已经售出，请重新购买。");
			}
			else if(bResult == 10)
			{
				g_MsgBoxMgr.PopSimpleAlert("我没有办法取走你的钱，你还是先去仓库管理员那里换取足够金币吧");
			}			
		}
		break;
	case 17:
		{
			if(bResult == 0)	//取消寄售成功
			{
				g_NPC.GetYuanBaoSellInfo().dwMySellNum = 0;
				g_NPC.GetYuanBaoSellInfo().dwMySellPrice = 0;
				g_MsgBoxMgr.PopSimpleAlert("取消寄售成功");
			}
			else if(bResult == 1)
			{
				g_MsgBoxMgr.PopSimpleAlert("系统正忙，请再次尝试！");
			}
			else if(bResult == 2)
			{
				g_MsgBoxMgr.PopSimpleAlert("很抱歉，您每隔一分钟才能操作元宝一次！");
			}
			else if(bResult == 3)
			{
				g_MsgBoxMgr.PopSimpleAlert("元宝系统已经关闭！");
			}
			else if(bResult == 4)
			{
				g_MsgBoxMgr.PopSimpleAlert("个人寄售元宝系统已经关闭！");
			}
			else if(bResult == 5)
			{
				g_MsgBoxMgr.PopSimpleAlert("你包裹中的元宝数量已经超出上限，请整理包裹后再来！");
			}
			else if(bResult == 6)
			{
				g_MsgBoxMgr.PopSimpleAlert("对不起，你并没有在我这里寄售元宝！");
			}
		}
		break;
	case 18:
		{
			if(bResult == 0)	//领取金币成功
			{
				g_NPC.GetYuanBaoSellInfo().dwMoneyToTake = 0;
				g_MsgBoxMgr.PopSimpleAlert("领取金币成功");				
			}
			else if(bResult == 1)
			{
				g_MsgBoxMgr.PopSimpleAlert("你整理包裹以后再来吧，已经装不下那么多钱了！");
			}
			else if(bResult == 2)
			{
				g_MsgBoxMgr.PopSimpleAlert(" 对不起，你无法领取奖品，请确认下列可能：\n  1、你并没有寄售元宝！\n  2、你寄售的元宝并未售出\n  3、你已经领取过金币了");
			}
		}
		break;
	case 19://材料转换
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "转换成功！";
				break;
			case 1:
				str = "系统忙，请稍后再试！";
				break;
			case 2:
				str = "必须使用转换乾坤来转换材料。";
				break;
			case 3:
				str = "转换失败";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}
			g_TalkMgr.PushSysTalk(str.c_str());
		}
		break;
	case 20:
		g_NPC.GetDrillResult().iResult = bResult;

		if(bResult == 0)//成功
		{
			g_pControl->Msg(MSG_FUSHI_WND,OPER_CUSTOM);
		}
		else if(bResult == 1)
		{
			
		}
		else if(bResult == 2)
		{
			g_MsgBoxMgr.PopSimpleAlert("你似乎没有足够的金币来支付这次费用");
		}
		else if(bResult == 3)	//
		{
			g_MsgBoxMgr.PopSimpleAlert("只有纯净符石才能还原。");
		}
		else if(bResult == 4)	//
		{
			g_MsgBoxMgr.PopSimpleAlert("非法符石等级");
		}
		else if(bResult == 5)	//
		{
			g_MsgBoxMgr.PopSimpleAlert("只有符石还原符才能还原符石。");
		}
		else if(bResult == 6)	//
		{
			g_MsgBoxMgr.PopSimpleAlert("符石还原符和符石的等级不一致");
		}		
		break;
	case 21:
		{
// 			DWORD dwGoodID = *((DWORD*)(msg));
// 			if (g_pControl->FindWindowByName("ZhenPuCuiLianWnd"))
// 			{
// 				g_pControl->Msg(MSG_CTRL_ZHENPUCUILIAN_WND,10,(CControl*)&dwGoodID);
// 			}
		}
		break;
	case 22://魂器升级
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "恭喜您升级器魂成功！";
				break;
			case 1:
				str = "物品非法！";
				break;
			case 2:
				str = "请放入正确的装备。";
				break;
			case 3:
				str = "固化中的装备不能进行器魂升级！";
				break;
			case 4:
				str = "材料非法！";
				break;
			case 5:
				str = "该装备没有觉醒！请先带着它觉醒后再来找我升级。";
				break;
			case 7:
				str = "您没有足够的金币来支付此项操作的费用。";
				break;
			case 8:
				str = "该装备的器魂经验值已经升到顶级，无法再进行提升！";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}

			g_pControl->MsgToWnd("EquipSoulWnd",MSG_CTRL_EQUIPSOUL_WND,MAKELONG(wType,bResult),(CControl*)(str.c_str()));
		}
		break;
	case 23://剥离
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "剥离成功！您获得了绑定器魂结晶。";
				break;
			case 1:
				str = "物品非法！";
				break;
			case 2:
				str = "请放入正确的装备。";
				break;
			case 3:
				str = "固化中的装备无法进行器魂剥离！";
				break;
			case 4:
				str = "该装备还未觉醒，无器魂结晶可供剥离。";
				break;
			case 6:
				str = "您没有足够的金币来支付此项操作的费用。";
				break;
			case 7:
				str = "您的包裹已满！请先清理后再来找我。";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}

			g_pControl->MsgToWnd("EquipSoulWnd",MSG_CTRL_EQUIPSOUL_WND,MAKELONG(wType,bResult),(CControl*)(str.c_str()));
		}
		break;
	case 24://魂器觉醒
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "觉醒成功！您装备的潜力得到了大大的提供！";
				break;
			case 1:
				str = "物品非法！";
				break;
			case 2:
				str = "请放入正确的装备。";
				break;
			case 3:
				str = "固化中的装备无法进行觉醒操作。";
				break;
			case 4:
				str = "材料非法！";
				break;
			case 5:
				str = "该装备已经觉醒！无需反复操作。带着它去升级器魂吧！";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}

			g_pControl->MsgToWnd("EquipSoulWnd",MSG_CTRL_EQUIPSOUL_WND,MAKELONG(wType,bResult),(CControl*)(str.c_str()));
		}
		break;
	case 25://装备拆分
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "分解成功！您获得了绑定的器魂结晶。";
				break;
			case 1:
				str = "物品非法！";
				break;
			case 2:
				str = "请放入正确的装备。";
				break;
			case 3:
				str = "固化中的装备无法进行分解！";
				break;
			case 4:
				str = "镶嵌有符石的装备无法分解！可先至英雄岛符石镶嵌商人处摘除符石后再进行分解操作。";
				break;
			case 5:
				str = "拥有器魂经验值装备不可分解，可至NPC器魂精炼师处剥离器魂后再进行分解操作！";
				break;
			case 6:
				str = "您的精力值不够无法分解装备，可通过商城购买精力药水补充精力值后再进行分解操作。";
				break;
			case 7:
				str = "该装备不能分解！";
				break;
			default:
				str = "系统忙，请稍后再试！";
			}

			g_MsgBoxMgr.PopSimpleAlert(str.c_str());
		}
		break;
	case 26://神佑融合
		{
			g_pControl->MsgToWnd("BlessCompoundWnd",MSG_CTRL_BLESSCOMPOUND_WND,MAKELONG(wType,bResult));
		}
		break;
	case 27://神佑修复
		{
			g_pControl->MsgToWnd("BlessCompoundWnd",MSG_CTRL_BLESSCOMPOUND_WND,MAKELONG(wType,bResult));
		}
		break;
	case 28://纹佩精炼
		{
			g_pControl->MsgToWnd("BlessCompoundWnd",MSG_CTRL_BLESSCOMPOUND_WND,MAKELONG(wType,bResult));
		}
		break;
	case 29://装备升级
	case 30://圣战之魂升级
		{
			g_pControl->MsgToWnd("ComposeEquipWnd",MSG_CTRL_COMPOSEEQUIP_WND,MAKELONG(wType,bResult));
		}
		break;
	case 31:
		g_pControl->MsgToWnd("FireArtificeWnd",MSG_CTRL_FIREARTIFICE_WND,MAKELONG(wType,bResult));
		break;
	case 32:
		{
			if (bResult == 0)
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else if (bResult == 8)
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,11);
			}
			else
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				std::string str;
				switch (bResult)
				{
				case 1:
					str = "非法物品。";
					break;
				case 2:
					str = "固化中的物品无法强化。";
					break;
				case 3:
					str = "法宝已升到最大等级。";
					break;
				case 4:
					str = "不是法宝强化符。";
					break;
				case 5:
					str = "法宝强化符不可用，将被系统回收。";
					break;
				case 6:
					str = "法宝的品阶或者等级不匹配。";
					break;
				case 7:
					str = "金币不足。";
					break;
				default:
					str = "系统忙，请稍后再试！";
				}

				g_MsgBoxMgr.PopSimpleAlert(str.c_str());
			}
		}		
		break;		
	case 33://开启灵识
		{
			if(bResult == 0)//成功
			{				
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else if(bResult == 7)//打孔失败
			{				
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,11);
			}
			else
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
							}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("该物品不能用来开启灵识");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("固化中的物品不能开启灵识");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("请用打孔钻头来开启灵识");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("已经达到灵识的上限");
				}
			}
		}
		break;
	case 34://圣灵注入
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else if(bResult == 8)
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,11);
			}
			else 
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("该物品不能用来注入圣灵");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("请用圣灵来注入");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("请使用圣灵注入符");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("没有该属性的灵识，请换1颗圣灵或者选择转换灵识的属性。");
				}
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("该属性的灵识已经全部注入了圣灵，请换1颗圣灵或者选择转换灵识的属性。");
				}
				else if(bResult == 7)
				{
					g_MsgBoxMgr.PopSimpleAlert("该圣灵无法注入到该装备");
				}
				else if(bResult == 9)
				{
					g_MsgBoxMgr.PopSimpleAlert("非法圣灵等级");
				}
				else if(bResult == 10)
				{
					//g_MsgBoxMgr.PopSimpleAlert("非法圣灵注入符");
					g_MsgBoxMgr.PopSimpleAlert("注入符等级与圣灵等级不一致");
				}
			}
		}
		break;
	case 35://圣灵提取
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else 
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)//失败
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("请放入注入过圣灵的装备");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("固化中的物品不能提取");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("包裹空间不足，请先整理出足够空位");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有圣灵提取符才能提取圣灵");
				}
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("非法圣灵提取符等级");
				}
				else if(bResult == 7)
				{
					g_MsgBoxMgr.PopSimpleAlert("该孔没有注入过圣灵");
				}
				else if(bResult == 8)
				{
					g_MsgBoxMgr.PopSimpleAlert("    圣灵提取符和圣灵的等级不一致,请放入与所提取圣灵等级一致的圣灵提取符");
				}
			}

		}
		break;
	case 36://圣灵净化
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("包裹空间不足，请先整理出足够空位");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("请用非纯净圣灵来净化");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有圣灵净化符才能净化圣灵");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵净化符和圣灵的等级不一致");
				}
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("非法圣灵等级");
				}
			}

		}
		break;
	case 37://圣灵融合
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else if(bResult == 10)
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,11);
			}
			else 
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有圣灵才能融合");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("请放入圣灵融合符来融合圣灵");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("非法圣灵等级");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵已经是最高级，无法继续融合");
				}
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵融合符和圣灵等级不一致");
				}
				else if(bResult == 7)
				{
					g_MsgBoxMgr.PopSimpleAlert("处于固化状态的圣灵无法融合");
				}
				else if(bResult == 8)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵的种类不一致，所以无法融合");
				}
				else if(bResult == 9)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵的等级不一致，所以无法融合");
				}
			}			
		}
		break;
	case 38://更改灵识属性
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{
					g_MsgBoxMgr.PopSimpleAlert("系统忙，请稍候再试");
				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("固化中的法宝不能转换灵识属性");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("选择的灵识属性和转换的属性相同，无需再转换");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("该灵识不存在，无法转换灵识属性");
				}			
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有转换乾坤才能进行灵识属性转化");
				}
				else if(bResult == 7)
				{
					g_MsgBoxMgr.PopSimpleAlert("该灵识已经注入过圣灵，无法转换灵识属性");
				}
				else if(bResult == 8)
				{
					g_MsgBoxMgr.PopSimpleAlert("请放入开启过灵识的法宝");
				}
			}
		}
		break;
	case 39://圣灵还原
		{
			if(bResult == 0)//成功
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,10);
			}
			else
			{
				g_pControl->MsgToWnd("FaBaoLevelUpWnd",MSG_CTRL_FABAOLEVELUP_WND,20);

				if(bResult == 1)
				{

				}
				else if(bResult == 2)
				{
					g_MsgBoxMgr.PopSimpleAlert("你似乎没有足够的金币来支付这次费用");
				}
				else if(bResult == 3)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有纯净圣灵才能还原。");
				}
				else if(bResult == 4)
				{
					g_MsgBoxMgr.PopSimpleAlert("非法圣灵等级");
				}
				else if(bResult == 5)
				{
					g_MsgBoxMgr.PopSimpleAlert("只有圣灵还原符才能还原圣灵。");
				}
				else if(bResult == 6)
				{
					g_MsgBoxMgr.PopSimpleAlert("圣灵还原符和圣灵的等级不一致");
				}
			}
		}
		break;
	case 40://纹佩融合
		g_pControl->MsgToWnd("BlessCompoundWnd",MSG_CTRL_BLESSCOMPOUND_WND,MAKELONG(wType,bResult));
		break;
	case 41://装备拆分
		{
			std::string str;
			switch (bResult)
			{
			case 0:
				str = "<color=graygolden 拆分成功！>";
				break;
			case 1:
				str = "<color=graygolden 非法物品！>";
				break;
			case 2:
				str = "<color=graygolden 金币不足！>";
				break;
			case 3:
				str = "<color=graygolden 固化中的装备无法拆分！>";
				break;
			case 4:
				str = "<color=graygolden 系统忙，请稍后再试！>";
				break;
			case 5:
				str = "<color=graygolden 该物品不能分解成魂！>";
				break;
			default:
				str = "<color=graygolden 系统忙，请稍后再试！>";
			}

			g_pControl->MsgToWnd("EquipChaifenWnd",MSG_CTRL_EQUIPCHAIFEN_WND,100,(CControl*)(str.c_str()));
		}
		break;
	case 42:// 赤金护符
		if (bResult == 0)
		{
			g_pControl->MsgToWnd("HuFuFlushWnd", MSG_CTRL_HUFUFLUSH_WND, 11);
		}
		else if (bResult == 1)
		{
			g_MsgBoxMgr.PopSimpleAlert("绑定元宝不足");
		}
		else if (bResult == 2)
		{
			g_MsgBoxMgr.PopSimpleAlert("非法物品");
		}
		else if (bResult == 3)
		{
			g_MsgBoxMgr.PopSimpleAlert("该物品不能保留刷新属性");
		}
		else if (bResult == 4)
		{
			g_MsgBoxMgr.PopSimpleAlert("固化中的物品不能保留刷新属性");
		}
		else if (bResult == 5)
		{
			g_MsgBoxMgr.PopSimpleAlert("没有刷新过属性");
		}
		break;
	default:
		break;
	}
}

void CGameControl::MSG_Guild_Phyle_State(const char* msg,int iLen)//行会修神还是修魔 0是凡人1是神人2是魔
{
	//行会修神还是修魔
	BYTE byState =*((BYTE*)(msg + 6));
	g_GuildData.SetGuildPhyleState(byState);

	//行会没有供奉神或魔,没有神魔buf
	if(byState == 0 || iLen < 12 + 28)
	{
		return;
	}


	//Buffer加成
	CGuildData::_GuildBuffer& guildBuff = g_GuildData.GetGuildBuffer();
	BYTE* pBuff = (BYTE*)(msg + 12);
	//主体Buffer加成
	guildBuff.m_byAttackFgh_Phyle[0] = *pBuff;
	guildBuff.m_byAttackFgh_Phyle[1] = *(pBuff + 1);
	guildBuff.m_byAttackRab_Phyle[0] = *(pBuff + 2);
	guildBuff.m_byAttackRab_Phyle[1] = *(pBuff + 3);
	guildBuff.m_byAttackDao_Phyle[0] = *(pBuff + 4);
	guildBuff.m_byAttackDao_Phyle[1] = *(pBuff + 5);
	guildBuff.m_byPhyDef_Phyle[0]	   = *(pBuff + 6);
	guildBuff.m_byPhyDef_Phyle[1]	   = *(pBuff + 7);
	guildBuff.m_byMagicDef_Phyle[0]  = *(pBuff + 8);
	guildBuff.m_byMagicDef_Phyle[1]  = *(pBuff + 9);
	guildBuff.m_byMagicHit_Phyle     = *(pBuff + 10);
	guildBuff.m_byPhyHit_Phyle       = *(pBuff + 11);
	guildBuff.m_byMagicAvoid_Phyle   = *(pBuff + 12);	
	guildBuff.m_byPhyAvoid_Phyle     = *(pBuff + 13);
	//元神Buffer加成
	guildBuff.m_byMAttackFgh_Phyle[0] = *(pBuff + 14);
	guildBuff.m_byMAttackFgh_Phyle[1] = *(pBuff + 15);
	guildBuff.m_byMAttackRab_Phyle[0] = *(pBuff + 16);
	guildBuff.m_byMAttackRab_Phyle[1] = *(pBuff + 17);
	guildBuff.m_byMAttackDao_Phyle[0] = *(pBuff + 18);
	guildBuff.m_byMAttackDao_Phyle[1] = *(pBuff + 19);
	guildBuff.m_byMPhyDef_Phyle[0]    = *(pBuff + 20);
	guildBuff.m_byMPhyDef_Phyle[1]    = *(pBuff + 21);
	guildBuff.m_byMMagicDef_Phyle[0]  = *(pBuff + 22);
	guildBuff.m_byMMagicDef_Phyle[1]  = *(pBuff + 23);
	guildBuff.m_byMMagicHit_Phyle     = *(pBuff + 24);	
	guildBuff.m_byMPhyHit_Phyle       = *(pBuff + 25);
	guildBuff.m_byMMagicAvoid_Phyle   = *(pBuff + 26);
	guildBuff.m_byMPhyAvoid_Phyle     = *(pBuff + 27);

}

void CGameControl::MSG_Guild_Stele_Level(const char* strMsg2,int iLen)
{	
	g_GuildData.SetGuildSteleFghValue(*((DWORD*)(strMsg2)));
	DWORD dwValue = 0;
	WORD wNum = *((WORD*)(strMsg2 + 6));
	WORD wLevel = *(strMsg2 + 8);
	g_GuildData.SetGuildSteleLevelNum(MAKELONG(wLevel,wNum));
	g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_RECREATE,MAKELONG(2,17));
}

void CGameControl::MSG_Elixir_Info(const char* msg,int iLen)
{
	Elixir& elixir = g_pGameData->GetElixir();
	elixir.dwLeftTm = *((DWORD*)msg);

	WORD wInfo = *((WORD*)(msg + 6));
	BYTE* ptemp = (BYTE*)(msg + 12);
	if (wInfo == 0)
	{
		//上线
		for (int i = 0;i<4;i++)
		{
			elixir.wPrice[i] = *ptemp;
			ptemp++;
			elixir.wTimes[i] = *ptemp;
			ptemp++;
			elixir.wHour[i] = *ptemp;
			ptemp++;
			elixir.dwExp[i] = *((DWORD*)ptemp);
			ptemp += 4;
		}

		PromptInfo prom;
		prom.wLooks = 16510;
		prom.eMsgWnd = MSG_CTRL_ELIXIR_WND;
		VPROMPTINFO& vprom = g_pGameData->GetPromptInfo();
		vprom.push_back(prom);
	}
}

void CGameControl::MSG_Guild_Stele_Hero_Result(const char* strMsg2,int iLen)
{
	g_pGameControl->SEND_Exchange_Menu_Click(DWORD(-6),"@liaojieXLDL");
}

//收到客户端激活的协议0x0361后，还以0x0361返回结果，第一个字节表示现在的激活状态,0表示未激活也就是失败，非0表示成功
void  CGameControl::MSG_Guild_Stele_Register_Result(const char* strMsg2,int iLen)
{
	BYTE byResult = strMsg2[0];
	g_GuildData.SetGuildSteleFghValue(*((DWORD*)(strMsg2 + 6)));
	std::vector<CGuildData::_GuildStele>& guildSteleMember = g_GuildData.GetGuildSteleMember();

	size_t iNum = guildSteleMember.size();
	std::string strName = "";
	strName.assign(strMsg2 + 12,iLen - 12);

	for(size_t i = 0; i < iNum; i++)
	{
		CGuildData::_GuildStele & member = guildSteleMember[i];
		if(strcmp(strName.c_str(),member.strName.c_str()) == 0)
		{
			member.bPart = (byResult != 0);					
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////收到客户端激活的协议0x0336后 还以0x0336返回结果 第一个字节表示现在的激活状态,0表示未激活也就是失败 1表示激活中,2成功//////////////
void  CGameControl::MSG_Kfz_Activate_Result(const char* strMsg2,int iLen)
{    
	BYTE byResult = strMsg2[0];

	_KfzGuild &KfzGuild = g_OtherData.GetKfzGuildData();
	vector<_KfzMember> & VMemberList = KfzGuild.VKfzMemberList;
	size_t iNum = VMemberList.size();

	for(size_t i = 0; i < iNum; i ++)
	{
		_KfzMember & member = VMemberList[i];
		if(strcmp(SELF.GetName(),member.strName.c_str()) == 0)
		{
			member.byState = byResult;					
			break;
		}
	}
}
//读秒倒计时
//时间(秒)四字节 + 协议号(0x0339) + 类型(2字节) + 显示精度(0表示以分钟为单位,1表示以秒为单位,1字节) + 12字节以后协议体显示的信息(可以没有)
//类型:1-20000客户端自己用,20001-30000:图形倒计时,30001-40000普通倒计时,要显示内容,40001-50000:图形(小图)加普通文字显示
void  CGameControl::MSG_Second_TimeOut(const char* strMsg2,int iLen)
{   
	DWORD dwTime = Conv_DWORD(strMsg2);
	WORD wType = Conv_WORD(strMsg2 + 6);
	//VTimeOut& vTimeout = g_OtherData.GetTimeOut();
	if(wType == 0)
	{
		_TimeOut timeout("",GetTickCount(),dwTime * 1000,2,true,(int)dwTime);
		//vTimeout.push_back(timeout);
		g_OtherData.AddTimeOut(timeout);
	}
	else if(wType == 1)
	{
		_TimeOut timeout("离下一场比赛开始还有%s，请速至跨服争霸管理员处发起挑战，角逐最强行会！",GetTickCount(),dwTime * 1000,10001,false,(int)dwTime);
		//vTimeout.push_back(timeout);
		g_OtherData.AddTimeOut(timeout);
	}
	else
	{
		string strMsg;
		if (iLen > 12)
		{
			strMsg.assign(strMsg2 + 12,iLen - 12);
		}

		_TimeOut timeout(strMsg,GetTickCount(),dwTime * 1000,wType,(strMsg2[8] == 1),(int)dwTime);
		//vTimeout.push_back(timeout);
		g_OtherData.AddTimeOut(timeout);
	}

}
//跨服战结果 下标是6的表示胜负标志0:负，1:胜，2:平,3:观众,不显示获胜或失败
//下标为7的一个字节表示类型,0表示原来的放蛋糕,1表示天绝魔域,放烟
void  CGameControl::MSG_Kfz_Result(const char* msg,int iLen)
{   
	string strMsg;
	strMsg.assign(msg + 12,iLen - 12);

	BYTE byResult = msg[6];
	BYTE byType = msg[7];

	g_pControl->PopupWindow(MSG_CTRL_KFZ_RESULT,OPER_RECREATE);
	g_pControl->MsgToWnd(MSG_CTRL_KFZ_RESULT,MSG_CTRL_KFZ_RESULT,byResult * 10000 + iLen - 12,(CControl*)strMsg.c_str());

	//放几个蛋糕特效
	if(byType == 0)
	{
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(22,35),MAKELONG(22,35));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(30,44),MAKELONG(30,44));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(39,53),MAKELONG(39,53));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(47,45),MAKELONG(47,45));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(38,36),MAKELONG(38,36));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(30,26),MAKELONG(30,26));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(40,17),MAKELONG(40,17));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(48,25),MAKELONG(48,25));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
		g_pMagicCtrl->CreateMagic(MAGICID_REGALEMENT_CAKE,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(57,35),MAKELONG(57,35));//,-1,-1,0,0,g_OtherData.GetFireShowList(x,y));
	}
	else if(byType == 1)
	{
		g_pMagicCtrl->CreateMagic(MAGICID_TIANJUE_END_EFF,EMP_MAGIC_NOTARGET | EMP_MAGIC_NOOWNER,MAKELONG(112,465),MAKELONG(112,465));
	}
}
/////////跨服战成员状态byResult:1发来成员状态,2:告诉最新的不在线成员并询问是否以此状态记录
//////12字节后:成员名字(14字节)+成员状态(1字节,0表示不在场,1表示在场)+元神名字(14字节)+元神状态(1字节,0表示不在场,1表示在场,2表示没有元神)
void  CGameControl::MSG_Kfz_MemberState(const char* msg,int iLen)
{   
	string strMsg;
	strMsg.assign(msg + 12,iLen - 12);

	BYTE byResult = msg[6];
	if(byResult == 1)
	{
		g_pControl->PopupWindow(MSG_CTRL_KFZ_MEMBER_STATE_WND,OPER_RECREATE);
		g_pControl->MsgToWnd(MSG_CTRL_KFZ_MEMBER_STATE_WND,MSG_CTRL_KFZ_MEMBER_STATE_WND,iLen - 12,(CControl*)strMsg.c_str());
	}
	else if(byResult == 2)
	{
		g_MsgBoxMgr.PopOkCancelBox(strMsg.c_str(),MSG_CTRL_CONFIRM_KFZ_MEMBER_STATE,0);
	}
}
//跨服战擂台列表,0x033D
////////////12字节以后协议体:擂台号(WORD) + 行会名(32字节) + 区服(22字节) + 保证金(DWORD) +  剩余多少分钟(BYTE) + 地图编号(15字节)
//下标为6的一个WORD表示所有记录条数,下标为8开始的一个word表示起始编号,下标为10开始的一个word表示结束编号,编号为从0开始闭区间
void  CGameControl::MSG_Kfz_ArenaList(const char* msg,int iLen)
{   
	WORD wNum = Conv_WORD(msg + 6);
	WORD wStart = Conv_WORD(msg + 8);
	WORD wEnd = Conv_WORD(msg + 10);

	if(wStart == 0)
	{
		g_pControl->PopupWindow(MSG_CTRL_KFZ_ARENA_WND,OPER_RECREATE);
	}

	g_pControl->MsgToWnd(MSG_CTRL_KFZ_ARENA_WND,MSG_CTRL_KFZ_ARENA_WND,1,(CControl*)msg);
}
//确认跨服战成员状态,1为确认成员状态,2为确认记录
void CGameControl::SEND_Kfz_MemberState(BYTE byType)
{
	SET_COMMAND(CS_KFZ_MEMBER_CONFIRM_STATE,12);
	ASSIGN_BYTE(6,byType);
	SEND_GAME_SERVER();
}
//下标为6的那个字节表示类型,1为扩包裹,2为扩钱袋
void CGameControl::SEND_EnlargePackage(BYTE byType)
{
	SET_COMMAND(SC_ENLARGE_PACKAGE,12);
	ASSIGN_BYTE(6,byType);
	SEND_GAME_SERVER();
}
// 请求跨服战擂台列表,0x033D,下标为6开始一个word表示开始行号,下标为8开始的一个WORD表示结束行号,行号为从0开始的闭区间
void CGameControl::SEND_Request_Kfz_ArenaList(int iStart,int iEnd)
{
	SET_COMMAND(CS_KFZ_ARENA_LIST,12);
	ASSIGN_WORD(6,iStart);
	ASSIGN_WORD(8,iEnd);
	SEND_GAME_SERVER();
}
//请求攻擂,0x033F,协议体(12)字节以后内容为地图编号
void CGameControl::SEND_Kfz_Arena_Join(const char * strMapName)
{
	if(!strMapName || strlen(strMapName) == 0)
		return ;

	SET_COMMAND(CS_KFZ_ARENA_JOIN,12 + strlen(strMapName));
	ADD_STR(strMapName);
	SEND_GAME_SERVER();
}
//第一个word:点了确认后执行的命令类型:
//第二个word:弹框的类型
void CGameControl::MSG_MsgBox_Command(const char* msg,int iLen)
{
	WORD wCommandType = Conv_WORD(msg);
	WORD wMsgBoxType = Conv_WORD(msg + 2);

	DWORD dwMsg = 0;
	if(wCommandType == 1)//退出到登录界面
	{
		dwMsg = MSG_CTRL_QUIT_TO_LOGINGWND;
	}
	else if(wCommandType == 2)//退出客户端
	{
		dwMsg = MSG_CTRL_CLOSE_CLIENT;
	}

	if(wMsgBoxType == 0)
		g_MsgBoxMgr.PopTagAlert(msg + 12,dwMsg);
	else if (wMsgBoxType == 1)
		g_MsgBoxMgr.PopSimpleComfirm(msg + 12,dwMsg,0);
	else if (wMsgBoxType == 2)
		g_MsgBoxMgr.PopOkCancelBox(msg + 12,dwMsg,0);

}


// 0-3 类型编号
// 4-5 协议号:0x0392
// 6-7 x
// 8-9 y
// 10-11 预留
// 12-15 指针
void CGameControl::MSG_DynObj_Appear(const char* msg,int iLen)
{
	DWORD dwObjID = Conv_DWORD(msg);
	WORD wX = Conv_WORD(msg + 6);
	WORD wY = Conv_WORD(msg + 8);

	DWORD id = Conv_DWORD(msg + 12);

	char* pMapName = const_cast<char*>(g_pGameMap->GetMapName());
	bool bShow = false;
	if (dwObjID == 11 || dwObjID == 12)
	{
		bShow = true;
	}
	g_pGameData->SetDynMiniMapObj(pMapName,(WORD)dwObjID,bShow);

	g_pGameMap->AddDynObject(dwObjID,wX,wY);
}
// 0-3 类型编号
// 4-5 协议号:0x0393
// 6-7 x
// 8-9 y
void CGameControl::MSG_DynObj_DisAppear(const char* msg,int iLen)
{
	DWORD dwObjID = Conv_DWORD(msg);
	WORD wX = Conv_WORD(msg + 6);
	WORD wY = Conv_WORD(msg + 8);
	//VSDynRecover& vDynRev = g_pGameData->GetVDynRecover();
	//SDynRecover dyn;
	//dyn.dwID = dwObjID;
	//dyn.wStartX = wX;
	//dyn.wStartY = wY;
	//dyn.dwTime = GetTickCount();
	//vDynRev.push_back(dyn);

	char* pMapName = const_cast<char*>(g_pGameMap->GetMapName());
	bool bShow = true;
	if (dwObjID == 11 || dwObjID == 12)
	{
		bShow = false;
	}
	g_pGameData->SetDynMiniMapObj(pMapName,(WORD)dwObjID,bShow);

	//g_pGameMap->ModifyObject(dwObjID,wX,wY,ATO_APPERA_HIDE,dyn.dwTime);//渐隐
	g_pGameMap->DelDynObject(dwObjID,wX,wY);
}
void CGameControl::Send_Del_Skill(DWORD dwGoodID,string& strSkillName)
{
	SET_COMMAND(CS_DEL_SKILL,32);

	ASSIGN_DWORD(0,dwGoodID);

	ADD_STR(strSkillName.c_str());

	SEND_GAME_SERVER();
}

//特殊物品处理协议,(x=1为用豹神水复活豹子,dwObjId1为豹子，dwObjId2为神水）
void  CGameControl::SEND_Deal_Special_Obj(DWORD dwObjId1,DWORD dwObjId2,BYTE x)
{
	SET_COMMAND(CS_SPECIALITEMACT,12);
	ASSIGN_ID(dwObjId1);
	ASSIGN_BYTE(6,x);
	ASSIGN_DWORD(8,dwObjId2);
	SEND_GAME_SERVER();
}

//特殊物品处理协议,
void  CGameControl::MSG_Deal_Special_Obj(const char* strMsg,int iLen)
{
	BYTE x = *((BYTE*)(strMsg + 6));
	DWORD dwId = *((DWORD*)(strMsg));

	if(x == 1)//豹子复活后自动放出豹子
	{
		int i = SELF.PackageGood().FindGoodPos(dwId);
		if(i >= 0)
		{
			g_pGameControl->SEND_Use_Object(i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 小退 重新请求角色
void CGameControl::SEND_ReselRole_Req(const char* ptID, const char* roleName)
{
	PKG_CLI_GG_ReselRole_REQ stReq;
	memset(stReq.szPTID, 0, PTID_LEN);
	memset(stReq.szRoleName, 0, ROLENAME_MAX_LEN);

	memcpy(stReq.szPTID, ptID, strlen(ptID));
	memcpy(stReq.szRoleName, roleName, strlen(roleName));

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_ReselRole_REQ, false);
}

//返回角色列表
void CGameControl::MSG_ReselRole_Ack(const char * msg,int iLen)
{
	PKG_GG_CLI_ReselRole_ACK* pAck = (PKG_GG_CLI_ReselRole_ACK*)msg;
	if (PROTO_SUCCESS == pAck->byResult)
	{
		BYTE byNum = pAck->stReply.stSucc.byRoleCount;

		g_OtherData.GetCharList().clear(); //清除之前的数据
		g_OtherData.SetReceivedCharList(true); //表示已经接受到数据了
		g_OtherData.GetDelCharList().clear();

		g_OtherData.SetCharNo(pAck->stReply.stSucc.bySelect);

		SRoleList &roleList = pAck->stReply.stSucc;

		for(int i = 0; i < byNum; i++)
		{
			_CharInfo info;
			memcpy(&info,&(roleList.astRoleList[i]),sizeof(info));
			if (info.byDelete)
			{
				g_OtherData.GetDelCharList().push_back(info);
			}
			else
			{
				g_OtherData.GetCharList().push_back(info);
			}
		}
	}
	else
	{
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReply.stFail.wErrCode),MSG_RECONNECT_SERVER);
	}
}

void CGameControl::SEND_Logout_Ntf(const char* ptID)
{
	PKG_CLI_GG_Logout_NTF stReq;
	memset(stReq.szPTID, 0, PTID_LEN);
	memcpy(stReq.szPTID, ptID, strlen(ptID));
	//memcpy(stReq.szRoleName, roleName, ROLENAME_MAX_LEN);

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Logout_NTF, false);
}

void CGameControl::SEND_INSTANCEZONE_POPUP(byte byType,byte bySelect)
{
	SET_COMMAND(CS_INSTANCEZONE_POPUP,32);
	ASSIGN_BYTE(6,byType);
	ASSIGN_BYTE(7,bySelect);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_INSTANCEZONE_POPUP( const char * msg, int iLen )
{
	int iInstanceType = msg[6];
	int iWndType = msg[7];	//1弹出开始框2弹出领奖框

	if(iWndType == 1)
	{
		if(iInstanceType >= 1 && iInstanceType <= 5)
		{
			g_pControl->PopupWindow(MSG_CTRL_InstanceZonePopUp_Wnd,OPER_RECREATE,iInstanceType);
		}

		if (iInstanceType == 5)
		{//告知红蓝阵营
			SELF.SetCamp(Conv_BYTE(msg + 8));
		}
	}
	else if(iWndType == 2)
	{
		g_pControl->PopupWindow(MSG_CTRL_InstanceZonePopUp_Wnd,OPER_RECREATE,100);

		g_NPC.GetInstancePrize().byNeedYuanBao = msg[8];	//元宝
		g_NPC.GetInstancePrize().iNeedGold = msg[9] * 10000;		//金币

		for(int i = 0;i < 5;i++)
		{
			g_NPC.GetInstancePrize().bOpen[i] = false;
		}
	}
	else if(iWndType == 3)
	{
		int iOpenCardNum = msg[8] - 1;		//成功开启的牌1，2，3，4，5；

		if(iOpenCardNum >= 0 && iOpenCardNum <= 4)
		{
			g_NPC.GetInstancePrize().bOpen[iOpenCardNum] = true;
			g_NPC.GetInstancePrize().wlooks[iOpenCardNum] = Conv_WORD(msg + 10);
			g_NPC.GetInstancePrize().name[iOpenCardNum].assign(msg + 12,iLen - 12);
		}		
	}
	else if (iWndType == 4)
	{
		g_pControl->PopupWindow(MSG_CTRL_InstanceZonePopUp_Wnd,OPER_RECREATE,101);
		if (iLen > 12)
		{
			string strTemp;
			strTemp.assign(msg+12,iLen-12);
			g_pControl->MsgToWnd(MSG_CTRL_InstanceZonePopUp_Wnd,MSG_CTRL_InstanceZonePopUp_Wnd,msg[8],(CControl*) (strTemp.c_str()));
		}
	}
	else if (iWndType == 5)
	{
		SELF.SetCamp(0);
	}
}

void CGameControl::SEND_Txzg_GetPrize()
{
	SET_COMMAND(CS_MSG_TXZG,32);
	ASSIGN_BYTE(6,2);//下标6　　2　玩家领取经验
	SEND_GAME_SERVER();
}

//下标6　1:  铁血战歌战斗状态
//下标0　1　个人资源数
//下标　　7　本人是红方0　或蓝方　1　　　
//下标8　9　红方资源数　
//下标10　11　蓝方资源数 

//下标6　2　　铁血战歌战斗结果
//下标7　胜利方　0　红方胜利　1　蓝方胜利　255:平局
//下标8　9　获胜经验（千为单位） 
//下标10　11  个人贡献经验（千为单位）

//下标6 3:表示RTS副本战斗状态
////////////////////0-1:红方中立怪物击杀数，
//2-3:蓝方中立怪物击杀数,
//7：高4位表示公共资源归属方，低4位表示自己所在阵营,   0：无，1:红方 2：蓝方
//8－9： 红方资源值
//10－11： 蓝方资源值

void CGameControl::Msg_Txzg(const char* msg,int iLen)
{	
	BYTE byType = msg[6];
	sTXZG &txzg = g_OtherData.GetTXZG();

	if (byType == 1)
	{
		txzg.byType = 1;
		txzg.bStarted = true;
		txzg.wContribute = Conv_WORD(msg);
		txzg.bySide = msg[7];
		txzg.wCurRes_Red = Conv_WORD(msg + 8);
		txzg.wCurRes_Blue = Conv_WORD(msg + 10);
	}
	else if (byType == 2)
	{
		txzg.byType = 1;
		txzg.byVictorySide = msg[7];
		txzg.wGetExp = Conv_WORD(msg + 8);
		txzg.wContributeExp = Conv_WORD(msg + 10);

		g_pControl->PopupWindow(MSG_CTRL_WUXING_PRIZE_WND,OPER_RECREATE,3);
	}
	else if (byType == 3)
	{
		//弹出RTS副本资源交换界面
		if (!txzg.bStarted)
		{
			g_pControl->PopupWindow(MSG_CTRL_RTSINSTANCE_WND,OPER_RECREATE);
		}
		//
		txzg.byType = 2;
		txzg.bStarted = true;
		txzg.wKillMonsterNum_Red = Conv_WORD(msg);
		txzg.wKillMonsterNum_Blue = Conv_WORD(msg + 2);
		txzg.bySide = BYTE(msg[7]) & 0x0F;
		txzg.byResSide = (BYTE(msg[7]) >> 4) & 0x0F;
		txzg.wCurRes_Red = Conv_WORD(msg + 8);
		txzg.wCurRes_Blue = Conv_WORD(msg + 10);
	}
	else if (byType == 4)//世界杯
	{
		sTSJB& tsjb = g_OtherData.GetTSJB();
		tsjb.bStarted = true;
		tsjb.byMyTeam = msg[7];
		tsjb.FirstTeamScore = Conv_WORD(msg + 8);
		tsjb.SecondTeamScore = Conv_WORD(msg + 10);
		
	}
}

//
// bool CGameControl::SEND_Trusteeship_Use_Object(int iPos)
// {
// 	TneupMember* pSelf = g_TrusteeshipData.GetSelf();
// 	if (!pSelf)
// 	{
// 		return false;
// 	}
// 
// 	CGood& tmp = pSelf->package.Get(iPos);
// 
// 	if(tmp.GetID() == 0)
// 		return false;
// 
// 	tmp.SetPos(iPos);
// 
// 	//太长时间没有回应,清除GetUsingTemp最上面一个,防止出现一直不能使用物品的情况
// 	DWORD dwCount = GetTickCount();
// 	if(!SELF.GetUsingTemp().Add(tmp))
// 	{
// 		if(dwCount - SELF.GetReserveData(plyLastUseObjectTime) < 3000)
// 		{
// 			return false;
// 		}
// 
// 		SELF.GetUsingTemp().Get(0).SetID(0);
// 		if(!SELF.GetUsingTemp().Add(tmp))
// 		{
// 			return false;
// 		}
// 	}
// 
// 	SELF.SetReserveData(plyLastUseObjectTime,dwCount);
// 
// 	SET_COMMAND(CS_PACKAGE_OBJECT_USE,256);
// 	ASSIGN_ID(tmp.GetID());
// 	ASSIGN_BYTE(10,1);//增加标记：1代表托管包裹
// 	ADD_STR(tmp.GetName());
// 	SEND_GAME_SERVER();
// 
// 	tmp.SetID(0);
// 	return true;
// }

void CGameControl::SendGameInfoForCheck()
{
TRY_BEGIN
	g_SdSamplerMgr.SendPlayerInfoSample();
  	g_SdSamplerMgr.SendSocietyInfoSample();
TRY_END
}

void CGameControl::SEND_TestProto(WORD id)
{
	sProtocolCfg * info = g_ProtosCfg.GetProtoCfg(id);
	if (!info)
	{
		return;
	}

	SET_COMMAND(info->id, info->length);

	for (int i = 0; i < (int)info->data.size(); ++i)
	{
		sProtocolData& ldata = info->data[i];
		
		if (ldata.strtype == "ASSIGN_ID")
		{
			ASSIGN_ID(strtoul(ldata.strvar.c_str(), 0, 10));
		}
		else if (ldata.strtype == "ASSIGN_WORD")
		{
			ASSIGN_WORD(ldata.pos, (WORD)atol(ldata.strvar.c_str()));
		}
		else if (ldata.strtype == "ASSIGN_DWORD")
		{
			ASSIGN_DWORD(ldata.pos, strtoul(ldata.strvar.c_str(), 0, 10));
		}
		else if (ldata.strtype == "ASSIGN_BYTE")
		{
			ASSIGN_BYTE(ldata.pos, (BYTE)atol(ldata.strvar.c_str()));
		}		
		else if (ldata.strtype == "ADD_STR")
		{
			ADD_STR(ldata.strvar.c_str());
		}
		else if (ldata.strtype == "ADD_WORD")
		{
			ADD_WORD((WORD)atol(ldata.strvar.c_str()));
		}
		else if (ldata.strtype == "ADD_DWORD")
		{
			ADD_DWORD(strtoul(ldata.strvar.c_str(), 0, 10));
		}
		else if (ldata.strtype == "ADD_BYTE")
		{
			ADD_BYTE((BYTE)atol(ldata.strvar.c_str()));
		}
		else if (ldata.strtype == "ADD_ARRAY")
		{
			ADD_ARRAY(ldata.strvar.c_str(), ldata.len);
		}
	}

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Swear_List(const char * msg,int iLen)
{	
	string buf;
	buf.assign(msg+12,iLen-12);

	int j = 0;
	string strMainName = StringUtil::toStr(buf,j);
	g_OtherData.SetJieYiLeader(strMainName);

	int per = StringUtil::toInt(buf,j);
	g_OtherData.SetJieYiExp(per);
	
	while(j < iLen-13)
	{		
		string strName = StringUtil::toStr(buf,j);

		DWORD iYesterLive = StringUtil::toUInt(buf,j);
		DWORD iCurLive = StringUtil::toUInt(buf,j);		

		bool bFind = false;
		for(UINT i = 0; !bFind && i < g_OtherData.GetRelationVector().size();i++)
		{
			_RelationStruct& lRelation = g_OtherData.GetRelationVector()[i];
			bFind = (lRelation.strName == strName);
			if(bFind)	
			{
				DWORD dwRelation = lRelation.iRelType;
				dwRelation |=  RT_JIEYI;
				lRelation.iRelType = dwRelation;
				lRelation.iCurLive = iCurLive;
				lRelation.iYesterLive = iYesterLive;
				break;
			}
		}

		if(!bFind)
		{
			//增加数据
			_RelationStruct relation;
			relation.strName = strName;
			relation.iRelType = RT_JIEYI;
			relation.iCurLive = iCurLive;
			relation.iYesterLive = iYesterLive;
			relation.strGuild = "";
			relation.iJob = -1;
			relation.iLevel = -1;
			relation.byPhyleType = 0;
			g_OtherData.GetRelationVector().push_back(relation);
		}

		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strName.c_str());
		if(p) 
			p->AddRelationType(RT_JIEYI);
	}

	g_pControl->Msg(MSG_CTRL_RELATION_WND,30,NULL);
}

void CGameControl::SEND_Add_Swear(const char * name)
{
	string _name = name;
	StringUtil::trim(_name);
	if(_name.size() == 0)
		return;

	SET_COMMAND(CS_ADDORDEL_SWEARMEM,256);
	ASSIGN_BYTE(6,1);
	ADD_STR(_name.c_str());
	int i = SEND_GAME_SERVER();

	if (i > 0)
	{
		g_TalkMgr.PushSysTalk("已经发出结义请求，请耐心等待回应");
	}
}

void CGameControl::SEND_Del_Swear(const char * name)
{
	SET_COMMAND(CS_ADDORDEL_SWEARMEM,256);
	ASSIGN_BYTE(6,0);
	ADD_STR(name);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Swear_Request(const char * msg,int iLen)
{
	char temp[256]={0};
	memcpy(temp,msg+12,iLen-12);
	temp[iLen-12] = 0;

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(temp);
	if(!p)
		return;	

	S_CommonConfirm CommonConfirm;
	CommonConfirm.iType = 9;
	CommonConfirm.dwLastTime = 30000;
	CommonConfirm.dwRequestPersonID = p->GetID();
	CommonConfirm.strMsg = CommonConfirm.strMsg + temp + "邀请您进行结义，是否确认？";
	g_OtherData.GetCommonConfirmVector().push_back(CommonConfirm);
	g_pControl->PopupWindow(MSG_CTRL_COMMON_CONFIRM_WND,OPER_CREATE);
}

void CGameControl::SEND_Answer_Swear_Request(const char * name, bool bApprove)
{
	SET_COMMAND(CS_ADD_SWEAR_CONFIRM,256);
	ASSIGN_BYTE(6,bApprove ? 1 : 0);
	ADD_STR(name);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_AddDel_Swear_Success(const char * msg,int iLen)
{
	DWORD bval = Conv_DWORD(msg);
	
	if (bval == 1)
	{		
		MSG_Add_Swear_Success(msg, iLen);
	}
	else if (bval == 0)
	{
		MSG_Del_Swear_Success(msg, iLen);
	}
}

void CGameControl::MSG_Add_Swear_Success(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen-12);

	int j = 0;
	string strMainName = StringUtil::toStr(buf,j);
	g_OtherData.SetJieYiLeader(strMainName);

	string strName;

	if(j < iLen-13)
	{
		strName = StringUtil::toStr(buf,j);
	}	

	bool bFind = false;

	for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
	{
		if (!bFind)
		{
			_RelationStruct& lRelation = g_OtherData.GetRelationVector()[i];
			bFind = (lRelation.strName == strName);
			if(bFind)
			{
				lRelation.AddRelationType(RT_JIEYI);
			}
		}
	}


	if(!bFind)
	{
		//增加数据
		_RelationStruct relation;
		relation.strName = strName;
		relation.iRelType = RT_JIEYI;
		g_OtherData.GetRelationVector().push_back(relation);

		//新加社会关系,社会关系按钮闪烁
		if (!g_pControl->MsgToWnd("RelationWnd",MSG_CTRL_FLASH_BTN_WND,6))
		{
			g_OtherData.SetFlashRelationTabType(6);
		}

		g_pControl->MsgToWnd(MSG_CTRL_PANEL_WND,MSG_CTRL_FLASH_BTN,6);
	}

	CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strName.c_str());
	if(p) p->AddRelationType(RT_JIEYI);

	strName += " 已经成功加入你的关系列表。";	
	g_TalkMgr.PushSysTalk(strName.c_str());

	g_pControl->Msg(MSG_CTRL_RELATION_WND,30,NULL);
}

void CGameControl::MSG_Del_Swear_Success(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen-12);

	int j = 0;
	string strMainName = StringUtil::toStr(buf,j);
	g_OtherData.SetJieYiLeader(strMainName);

	string strName;

	if(j < iLen-13)
	{
		strName = StringUtil::toStr(buf,j);
	}	

	if (strcmp(SELF.GetName(), strName.c_str()) == 0)
	{
		for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
		{
			_RelationStruct &SRelation = g_OtherData.GetRelationVector()[i];

			if((RT_JIEYI & SRelation.iRelType) != 0 )
			{
				SRelation.RemoveRelationType(RT_JIEYI);

				if(SRelation.iRelType == 0)//没有任何关系了则删除
				{
					g_OtherData.GetRelationVector().erase(g_OtherData.GetRelationVector().begin() + i);
					i --;
				}
				
				CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strName.c_str());
				if(p) 
					p->RemoveRelationType(RT_JIEYI);				
			}
		}

		g_pControl->Msg(MSG_CTRL_RELATION_WND,30,NULL);

		char temp1[256] = {0};
		sprintf(temp1,"%s已经从结义关系列表中被删除。",strName.c_str());
		g_TalkMgr.PushSysTalk(temp1);

	}
	else
	{
		for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
		{
			_RelationStruct &SRelation = g_OtherData.GetRelationVector()[i];

			if(SRelation.strName == strName && (RT_JIEYI & SRelation.iRelType) != 0 )
			{
				SRelation.RemoveRelationType(RT_JIEYI);

				if(SRelation.iRelType == 0)//没有任何关系了则删除
				{
					g_OtherData.GetRelationVector().erase(g_OtherData.GetRelationVector().begin() + i);
					i --;
				}
				
				CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(strName.c_str());
				if(p) 
					p->RemoveRelationType(RT_JIEYI);
			}
		}

		g_pControl->Msg(MSG_CTRL_RELATION_WND,30,NULL);

		string temp = strName + "已经从结义关系列表中被删除。";
		g_TalkMgr.PushSysTalk(temp.c_str());
	}	
}

void CGameControl::MSG_Swear_Error(const char * msg,int iLen)
{
	BYTE by = Conv_BYTE(msg+6);
	
	char temp[256] = {0};
	switch(by)
	{
	case 1:
		{
			strcpy(temp,"请1分钟后再进行结义相关操作。");
			break;
		}
	case 2:
		{
			strcpy(temp,"你的等级没到30级。");
			break;
		}
	case 3:
		{			
			strcpy(temp,"已达结义人数上限。");
			break;
		}
	case 4:
		{			
			strcpy(temp,"对方正忙，请稍候再试。");
			break;
		}
	case 5:
		{			
			strcpy(temp,"服务器忙。");
			break;
		}
	case 6:
		{
			strcpy(temp,"对方不在线。");
			break;
		}
	case 7:
		{
			strcpy(temp,"你不是结义发起者。");
			break;
		}
	case 8:
		{
			strcpy(temp,"对方不是结义成员。");
			break;
		}
	case 9:
		{
			strcpy(temp,"对方已经和他人结义。");
			break;
		}
	case 10:
		{
			strcpy(temp,"你是结义发起者，不能退出结义。");
			break;
		}
	default:
		return;
	}

	g_TalkMgr.PushSysTalk(temp);
}

void CGameControl::SEND_Heart_Beat()
{
	SET_COMMAND(CS_HEART_BEAT,CMD_SIZE + 128);//后面64字节给庞路用
	iBLen = CMD_SIZE + 128;
	SEND_GAME_SERVER();	
}


void CGameControl::MSG_InGameVerify_Req(const char * msg, int iLen)
{
	tagPKG_GG_CLI_InGameVerify_REQ* pAck = (tagPKG_GG_CLI_InGameVerify_REQ*)msg;


	string strPath = GetGameDataDir();
	strPath += "\\tmp1.tex";

	FILE * fp = fopen(strPath.c_str(),"wb");
	fwrite(pAck->abyPicture,pAck->wPicLen,1,fp);
	fclose(fp);


	if (pAck->wPopupTime > 0 && pAck->wTotalTime > 0)
	{
		g_AIMgr.SetReserveTime(plyVerifyLateTime,pAck->wPopupTime);
		g_AIMgr.SetReserveTime(plyVerifyTotalTime,pAck->wTotalTime);

		//g_MsgBoxMgr.PopEditBox("请输入验证码:",MSG_CTRL_LEVEL_CHECK,1,NULL,false,24,0,0,NULL,false);
		g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_LATER_WND,OPER_CREATE,2);
	}
	else
	{
		g_pControl->MsgToWnd(MSG_CTRL_INPUT_MIBAO_LATER_WND,MSG_CTRL_INPUT_MIBAO_LATER_WND,3);
	}

}

void CGameControl::SEND_InGameVerify_Ack(const char * answer)
{
	tagPKG_CLI_GG_InGameVerify_ACK stReq;

	sprintf(stReq.szAnswer,"%s",answer);

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_InGameVerify_ACK);
}

void CGameControl::SEND_FreshVerify_Ack()
{
	tagPKG_CLI_GG_FreshVerify_Req stReq;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_FreshVerify_Req);
}

void CGameControl::MSG_NoticeIE(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg + 12,iLen - 12);

	if (!buf.empty())
	{
		g_pGameData->SetNoticeIEURL(buf);
		g_pGameData->SetNoticeIEState(1);
		g_pGameData->SetNoticeFrame(0);
	}
}

void CGameControl::MSG_Activerate_Prize(const char * msg,int iLen)
{
	BYTE by = Conv_BYTE(msg+6);
}

void CGameControl::SEND_Activerate_Prize(BYTE byType,BYTE bySelPrize)
{
	SET_COMMAND(CS_ACTIVERATE_PRIZE,CMD_SIZE + 64);

	ASSIGN_BYTE(6,byType);
	ASSIGN_BYTE(8,bySelPrize);

	SEND_GAME_SERVER();	
}

