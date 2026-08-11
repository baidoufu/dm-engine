#include "GameControl.h"
#include "GameData/GameData.h"
#include "GameData\MagicDefine.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameGlobal.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/OtherData.h"
#include "GameData/LoginData.h"
#include "GameData/NpcData.h"
#include "GameData/PhyleData.h"

#include "GameAI/AIAutoMgr.h"

#include "BaseClass/Misc/cliggproto.h"
#include "BaseClass/Misc/errdefs.h"

// 收到的消息处理
//void CGameControl::MSG_Selchar_CharInfo(const char * msg,int iLen)
//{
// 	//如果第二字节为1表示selgate和rungate是同一台服务器，登录rungate时就不要重连
// 	BYTE byNum = (BYTE)(msg[0]);
// 	//g_Login.SetSelRunGateSame(msg[1] == 1);
// 
// 	//自动进入游戏
// 	if(g_OtherData.IsAutoEnter())
// 	{
// 		SEND_Selchar_SelectChar();
// 		g_OtherData.SetAutoEnter(false);
// 		return;
// 	}
// 
// 	g_OtherData.GetCharList().clear(); //清除之前的数据
// 	g_OtherData.SetReceivedCharList(true); //表示已经接受到数据了
// 
// 	string buf;
// 	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
// 	int i = 0;
// 	int iSize = buf.size();
// 	int ii = 0; //编号
// 	g_OtherData.SetCharNo(0);
// 
// 	while(ii < byNum)
// 	{
// 		_CharInfo info;
// 		string name = StringUtil::toStr(buf,i);
// 		info.iCareer = StringUtil::toInt(buf,i);
// 		info.iHair   = StringUtil::toInt(buf,i);
// 		info.iLevel  = StringUtil::toInt(buf,i);
// 		info.iSex    = StringUtil::toInt(buf,i);
// 		//[协议要修改]
// 		BYTE byNoUse = (BYTE)(StringUtil::toUInt(buf,i));//重数,0表示没有飞升
// 		//info.bPhyle	 = (BYTE)(StringUtil::toUInt(buf,i));//重数,0表示没有飞升
// 
// 		if(name[0] == '*')
// 		{
// 			info.strName = name.c_str()+1;
// 			g_OtherData.SetCharNo(ii);
// 		}
// 		else
// 		{
// 			info.strName = name;
// 		}
// 		g_OtherData.GetCharList().push_back(info);
// 		ii++;
// 	}
// 
// 
// 	if(g_Login.GetAutoLoginInType() > 0 && g_Login.GetAutoLoginInType() <= 4)//自动登录
// 	{
// 		if(g_OtherData.GetCharList().size() == 0)
// 		{
// 			g_MsgBoxMgr.PopSimpleAlert("没有角色!");
// 			return;
// 		}
// 		Sleep(20);
// 		g_pGameControl->SEND_Selchar_SelectChar();
// 		return;
// 	}
// 	else if (g_Login.GetAutoLoginInType() == 5)//浩方竞技,只登录到选角色界面
// 	{
// 		g_Login.SetAutoLoginInType(0);
// 		g_Login.SetInHaoFangJingJi(true);
// 	}
// 
// 	g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);

//}

// void CGameControl::MSG_Selchar_RunServer(const char * msg,int iLen)
// {
// 	string buf;
// 	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
// 
// 	int i = 0;
// 
// 	string strIP = StringUtil::toStr(buf,i);
// 	int iPort = StringUtil::toInt(buf,i);
// 
// 	//连接网络
// 	g_pGameControl->SEND_Game_Connect();
// 	g_OtherData.SetAutoEnter(false);
// }


//// 要发送的消息处理
//bool CGameControl::SEND_Selchar_CharServer()
//{
//	//string str = StringUtil::format("%d",g_Login.GetSessionID());
//
//	SET_COMMAND(CS_SELCHAR_CHARSERVER,256);
//	ASSIGN_BYTE(1,1);//新版客户端支持selgate和rungate合并，这里用第二字节置1告诉selgate是新客户端
//	ASSIGN_BYTE(6,1);//高速版客户端标志
//	ASSIGN_BYTE(7,1);//客户端不管
//
//	ADD_STR(g_Login.GetLoginID());
//	ADD_BYTE('/');
//	//ADD_STR(str.c_str());
//
//	return (SEND_GAME_SERVER() == iBLen);
//}

// bool CGameControl::SEND_Selchar_SelectChar()
// {
// 	SET_COMMAND(CS_SELCHAR_CHAR,256);
// 
// 	ADD_STR(g_Login.GetLoginID());
// 	ADD_BYTE('/');
// 	ADD_STR(g_OtherData.GetCharName());
// 
// 	g_OtherData.CopyToSelf();
// 
// 
// 	return (SEND_GAME_SERVER() == iBLen);
// }

// void CGameControl::SEND_Delete_Character(const char * name)
// {
// 	SET_COMMAND(CS_DELETE_CHARACTER,256);
// 	ADD_STR(name);
// 	SEND_GAME_SERVER();
// }

// void CGameControl::SEND_Create_Character(const char * info,int iLen)
// {
// 	SET_COMMAND(CS_CREATE_CHARACTER,256);
// 	ADD_ARRAY(info,iLen);
// 	SEND_GAME_SERVER();
// }

// void CGameControl::SEND_Deleted_Char_Info()
// {
// 	//string str = StringUtil::format("%d",g_Login.GetSessionID());
// 
// 	SET_COMMAND(CS_DELETED_CHAR_INFO,256);
// 	ASSIGN_BYTE(7,1);//客户端不管
// 	ADD_STR(g_Login.GetLoginID());
// 	ADD_BYTE('/');
// 	//ADD_STR(str.c_str());
// 
// 	SEND_GAME_SERVER();
// }

//void CGameControl::MSG_Deleted_Char_Info(const char * msg,int iLen)
//{
// 	if(iLen == CMD_SIZE)
// 		return;
// 
// 	string buf;
// 	buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);
// 	int iSize = buf.size();
// 	DWORD dwNum = *((DWORD*)(msg + 8));
// 	int i = 0;
// 	int ii = 0;
// 	g_OtherData.GetDelCharList().clear();
// 
// 	while( ii < dwNum)
// 	{
// 		_CharInfo info;
// 		info.strName = StringUtil::toStr(buf,i);
// 		info.iCareer = StringUtil::toInt(buf,i);
// 		info.iHair   = StringUtil::toInt(buf,i);
// 		info.iLevel  = StringUtil::toInt(buf,i);
// 		info.iSex    = StringUtil::toInt(buf,i);
// 		info.strDeltime = StringUtil::toStr(buf,i);
// 		//[协议要修改]
// 		BYTE byNoUse = BYTE(StringUtil::toInt(buf,i));
// 		//info.bPhyle  = BYTE(StringUtil::toInt(buf,i));
// 
// 		if(info.strDeltime.compare("1899") == 0)
// 		{
// 			info.strDeltime = "未知";
// 		}
// 		g_OtherData.GetDelCharList().push_back(info);
// 
// 		ii++;
// 	}
// 	
//}

// void CGameControl::SEND_Restore_Char(int iRecoverChar)
// {
// 	//调用时已经进行了参数检查
// 	if(iRecoverChar < 0)
// 		return;
// 
// 	string name = g_OtherData.GetDelCharList().at(iRecoverChar).szRoleName;
// 
// 	SET_COMMAND(CS_RESTORE_CHAR,256);
// 	ADD_STR(name.c_str());
// 	SEND_GAME_SERVER();
// }

//void CGameControl::MSG_Restore_Char_Error(const char * msg,int iLen)
//{
//	g_MsgBoxMgr.PopSimpleAlert("已经有两个角色或者未知错误！");
//}

//void CGameControl::MSG_Restore_Char_Ok(const char * msg,int iLen)
//{
//	SEND_Selchar_CharServer();
//}

// void  CGameControl::SEND_SessionId_To_SelGate()
// {
// 	SET_COMMAND(CS_SESSIONID_TO_SELGATE,256);
// 	//ASSIGN_ID(g_Login.GetSessionID());	
// 	ADD_STR(g_Login.GetLoginID());
// 
// 	//g_pNet->SendBuf(SERVER_GAME,temp,iBLen, true,false);
//     g_pNet->SendBuf(SERVER_GAME,temp,iBLen, CS_SESSIONID_TO_SELGATE, true,false);
// }

//////////////////////////////////////////////////////////////////////////

//传世创建角色
void CGameControl::SEND_CreateRole_Req(const char* name, BYTE byJob,BYTE bySex, BYTE byHair)
{
	/*
	PKG_CLI_GG_CreateRole_REQ stReq;
	memset(stReq.stInfo.szPTID,0,PTID_LEN);
	strcpy(stReq.stInfo.szPTID,g_Login.GetLoginID());
	memset(stReq.stInfo.szRoleName,0,ROLENAME_MAX_LEN);
	strcpy(stReq.stInfo.szRoleName,name);
	stReq.stInfo.byHair = byHair;
	stReq.stInfo.byJob = byJob;
	stReq.stInfo.bySex = bySex;
	*/

	char szPacket[64];
	wsprintf( szPacket, "%s/%s/%d/%d/%d", g_Login.GetLoginID(), name, byHair, byJob, bySex );
	g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, CM_NEWCHR, false);
}

//创建角色成功返回 SM_NEWCHR_SUCCESS 192.168.1.6/7100/25
void CGameControl::MSG_CreateRole_Ack(const char * msg, int iLen)
{
	//传世方法, 创建成功后再查询一边角色列表
	g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);   
	g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);

	g_OtherData.GetCharList().clear(); //清除之前的数据
	g_OtherData.SetReceivedCharList(true); //表示已经接受到数据了
	g_OtherData.GetDelCharList().clear();

	Sleep(100); 
	char szPacket[64];
	wsprintf(szPacket, "%s/%d", g_Login.GetLoginID(), g_Login.GetSessionID());
	g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, 100, false);		//CM_QUERYCHR

	/* 无双代码
	PKG_GG_CLI_CreateRole_ACK* pAck = (PKG_GG_CLI_CreateRole_ACK*)msg;
	if (PROTO_SUCCESS == pAck->byResult)
	{
		g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);   
		g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);

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
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReply.stFail.wErrCode));
	}*/
}

//选择角色, 进入游戏
void CGameControl::SEND_SelRole_Req(const char* ptID, const char* roleName)
{
	/*
    PKG_CLI_GG_SelectRole_REQ stReq;

	memset(stReq.szPTID,0,PTID_LEN);
	strcpy(stReq.szPTID,ptID);
	memset(stReq.szRoleName,0,ROLENAME_MAX_LEN);
	strcpy(stReq.szRoleName,roleName);

	g_OtherData.CopyToSelf();

    g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_SelectRole_REQ);
	*/

	//传世方法选择角色
	g_OtherData.CopyToSelf();
	char szPacket[64];
	wsprintf(szPacket, "%s/%s", ptID, roleName);
	g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, CM_SELCHR, false);
}

//传世: 连接游戏网关 7200 192.168.1.6/72000/2//
void CGameControl::MSG_SelRole_Ack(const char * msg, int iLen)
{
	if(iLen > 0)
	{
		string txt;
		txt.append(msg, iLen);
		VString runGate = StringUtil::split(txt, "/");

		//登录游戏
		//g_pGameControl->SEND_Game_Connect();
		g_OtherData.SetAutoEnter(false);
		g_Login.SetInnerAreaNo(1);
		g_Login.SetInnerGroupNo(1);

		//断开当前的账号网关
		g_Login.SetServer(runGate.at(0).c_str(),StringUtil::toInt(runGate.at(1)));
		g_pNet->Close(SERVER_GAME);
		Sleep(1000);
		//连接到游戏网关
		g_pNet->SetServer(SERVER_GAME,runGate.at(0).c_str(),StringUtil::toInt(runGate.at(1)));
		g_pNet->Connect(SERVER_GAME);
		Sleep(100);

		//尝试进入游戏,没有msg结构直接是消息体
		CHAR				szEncodeMsg[512];
		CHAR				szLoginInfo[512];
		CHAR				szPacket[512];
		wsprintf(szLoginInfo, "***%s/%s/%d/20161104/%d", g_Login.GetLoginID(), g_OtherData.GetCharName(), g_Login.GetSessionID(), 1);

		int nPos = fnEncode6BitBuf((BYTE*)szLoginInfo, szEncodeMsg, lstrlen(szLoginInfo), sizeof(szEncodeMsg));

		szEncodeMsg[nPos] = '\0';

		wsprintf(szPacket, "#%d%s!", 1, szEncodeMsg);

		g_pNet->SetGS(true);

		g_pNet->send(SERVER_GAME,(char*)&szPacket,strlen(szPacket), 0);


		//g_pGameControl->SEND_Game_EnterGame();

		//g_Login.SetInnerAreaNo(pAck->stReply.stSucc.wAreaNo);
		//g_Login.SetInnerGroupNo(pAck->stReply.stSucc.byServerNo);
	}else
	{
		//g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReply.stFail.wErrCode));
	}

	/*
    PKG_GG_CLI_SelectRole_ACK* pAck = (PKG_GG_CLI_SelectRole_ACK*)msg;
    if (PROTO_SUCCESS == pAck->byResult)
    {
		//登录游戏
		//g_pGameControl->SEND_Game_Connect();
		g_OtherData.SetAutoEnter(false);

		g_Login.SetInnerAreaNo(pAck->stReply.stSucc.wAreaNo);
		g_Login.SetInnerGroupNo(pAck->stReply.stSucc.byServerNo);
    }
    else
    {
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReply.stFail.wErrCode));
    }
	*/
}

void CGameControl::SEND_DelRole_Req(const char* ptID, const char* roleName)
{
	//lzhez  删除角色 注释掉上面的 只发送角色名称到服务端
    PKG_CLI_GG_DelRole_REQ stReq;
	memset(stReq.szPTID,0,ROLENAME_MAX_LEN);
	strcpy(stReq.szPTID,roleName);
	//memset(stReq.szPTID,0,PTID_LEN);
	//strcpy(stReq.szPTID,ptID);
	//memset(stReq.szRoleName,0,ROLENAME_MAX_LEN);
	//strcpy(stReq.szRoleName,roleName);
    g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_DelRole_REQ);
}

void CGameControl::MSG_DelRole_Ack(const char * msg, int iLen)
{
    PKG_GG_CLI_DelRole_ACK* pAck = (PKG_GG_CLI_DelRole_ACK*)msg;
    if (PROTO_SUCCESS == pAck->byResult)
    {
		int idx = g_OtherData.GetDelCharSelectIdx();
		VCharInfo &charlist = g_OtherData.GetCharList();
		if (idx >= 0 && idx < charlist.size())
		{
			charlist[idx].byDelete = 1;
			memcpy(charlist[idx].szDeleteTime,g_Timer.GetDateTime("%Y-%m-%d %H:%M:%S"),TIME_STRING_LEN);
			g_OtherData.GetDelCharList().push_back(charlist[idx]);
			charlist.erase(charlist.begin() + idx);
		}
    }
    else
    {
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReason.wErrCode));
    }

	g_OtherData.SetDelCharSelectIdx(-1);
}

void CGameControl::SEND_UnDelRole_Req(const char* ptID, const char* roleName)
{
	PKG_CLI_GG_UnDelRole_REQ stReq;

	memset(stReq.szPTID,0,PTID_LEN);
	strcpy(stReq.szPTID,ptID);
	memset(stReq.szRoleName,0,ROLENAME_MAX_LEN);
	strcpy(stReq.szRoleName,roleName);


	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_UnDelRole_REQ);
}

void CGameControl::MSG_UnDelRole_Ack(const char * msg, int iLen)
{
	PKG_GG_CLI_UnDelRole_ACK* pAck = (PKG_GG_CLI_UnDelRole_ACK*)msg;
	if (PROTO_SUCCESS == pAck->byResult)
	{
		int idx = g_OtherData.GetUnDelCharSelectIdx();
		VCharInfo &delcharlist = g_OtherData.GetDelCharList();
		if (idx >= 0 && idx < delcharlist.size())
		{
			delcharlist[idx].byDelete = 0;
			g_OtherData.GetCharList().push_back(delcharlist[idx]);
			delcharlist.erase(delcharlist.begin() + idx);
		}
	}
	else
	{
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReason.wErrCode));
	}

	g_OtherData.SetUnDelCharSelectIdx(-1);
}


void CGameControl::MSG_Verify_Req(const char * msg, int iLen)
{
	PKG_GG_CLI_Verify_REQ* pAck = (PKG_GG_CLI_Verify_REQ*)msg;

	//BYTE byType = *((BYTE*)(msg + 6));
	//g_Login.SetEnterCheckType(byType);
	string strPath = GetGameDataDir();
	strPath += "\\tmp1.tex";

	FILE * fp = fopen(strPath.c_str(),"wb");
	fwrite(pAck->abyPicture,pAck->wPicLen,1,fp);
	fclose(fp);
	//g_pControl->Msg(MSG_CTRL_ENTER_GAME_STOP,0,0);
	g_MsgBoxMgr.PopEditBox("请输入验证码:",MSG_CTRL_LEVEL_CHECK,0);
}

void CGameControl::SEND_Verify_Ack(const char * answer)
{
	tagPKG_CLI_GG_Verify_ACK stReq;

	sprintf(stReq.szAnswer,"%s",answer);

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Verify_ACK);
}

void CGameControl::MSG_DynCode_NTF(const char * msg, int iLen)
{
	PKG_GG_CLI_DynCode_NTF* pAck = (PKG_GG_CLI_DynCode_NTF*)msg;
	TRY_BEGIN
	g_pNet->SetDynEnDe(DED_New_Old,pAck->dwDataLen,(const char *)(pAck->abyDataBuf));
	TRY_END	
}
