#include "GameControl.h"
#include "Global/Interface/StreamInterface.h"
#include "BaseClass/Misc/Internet.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameData.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/GameGlobal.h"
#include "GameData/LoginData.h"
#include "Global/StringUtil.h"
#include "GameAI/AIAutoMgr.h"
#include "GameAI/AIMgr.h"
#include "GameClient/SDOAInterface.h"
#include "GameClient/VoiceAdapter.h"
#include "GameClient/SDError.h"
#include "GameData/OtherData.h"
#include "GameClient/UsbClientSDK.h"
#include "GameAI/AIConfigMgr.h"
#include "GameData/MagicCtrlMgr.h"

#include "BaseClass/Misc/cliggproto.h"
#include "BaseClass/Misc/errdefs.h"

#include "Global/Interface/AudioInterface.h"
#include "GameData/ConfigData.h"
#include "GameClient/SDSamplerMgr.h"

#include "GameClient/WidgetManager.h"
#include "GameClient/ReplayManager.h"

#include "Global/md5.h"

#include "GameClient/OpenLoginClient.h"


//#include "Global/include/openssl.h"
//#pragma comment(lib,"openssl.lib")

//CRsaClient g_RsaClient;

//strClientType:"0"表示普通区版本
void CGameControl::SEND_Login_Tongji(int iSubType, const string& strFrom)
{
    string poststr = StringUtil::format
        ("zone=%d&action=%s&step=3&userid=%s&logintab=%u&loginresult=%d&version=%s",
        g_Login.GetInnerAreaNo(),
		g_Login.GetLoginServer(),
        g_Login.GetLoginID(),
        WS_GAME_ID,
        iSubType,
        g_strVersion);

    g_HttpMgr.PostData("woool.reg.sdo.com","register/wooolloginlog.asp",poststr.c_str());	
}

void CGameControl::SEND_Login_Tongji2(bool bLogoutTime)
{
	output_debug("start SEND_Login_Tongji2");
	
	
	int iSubType = g_pStreamMgr->GetOtherConfigInt("SubType",0);//SubType:0为普通客户端,1为小客户端,
	std::string strFrom = g_pStreamMgr->GetOtherConfigStr("From","official");


	//客户端类型 client_type	数字型(0:未知1:微客户端 2:全客户端 3:网页)
	int client_type = 0;
	if (iSubType == 0)
		client_type = 2;
	else if (g_hParentWnd)
		client_type = 3;
	else
		client_type = 1;

	char szReceiveBuf[257] = "0";
	if(g_hCgpfsDLL && g_hParentWnd)
	{
		LPCgpQueryVariant lpQueryVariant = (LPCgpQueryVariant)GetProcAddress(g_hCgpfsDLL,"CgpQueryVariant");
		if (lpQueryVariant)
		{
			int iRtn = lpQueryVariant(CGP_VARIANT_MACHINECODE,szReceiveBuf,256);
			if (iRtn <= 0)
				strcpy(szReceiveBuf, "0");
		}
	}

	if (!g_strChannelName.empty())
		strFrom = strFrom + " " + g_strChannelName;

	string strLogoutTime;
	if (bLogoutTime)
		strLogoutTime = g_Timer.GetDateTime("%Y-%m-%d %H:%M:%S");

	string postdata = StringUtil::format
		("{\"popt_id\":\"%s\",\"mac\":\"%s\",\"client_type\":%d,\"version\":\"%s\",\"from\":\"%s\",\"user_id\":\"%s\",\"login_time\":\"%s\",\"logout_time\":\"%s\",\"area\":%d,\"server\":\"%s\",\"clientid\":\"%s\"}",
		g_Login.GetLoginID(),
		g_strMac,
		client_type,
		g_strVersion,
		strFrom.c_str(),
		SELF.GetName(),
		g_Login.GetLoginTime(),
		strLogoutTime.c_str(),
		g_Login.GetInnerAreaNo(),
		g_Login.GetLoginServer(),
		szReceiveBuf
		);


	MD5_CTX CMD5;
	postdata = StringUtil::Gb2312ToUtf8(postdata);
	
	string postsign = postdata + "Sdg_WS2011_1111";	
	string postsignMD5 = CMD5.MakePassMD5(postsign);


	postdata = StringUtil::UrlEncoding(postdata);

	string popstr1 = StringUtil::format
		("model=WS_login&data=%s&sign=%s",
		postdata.c_str(),
		postsignMD5.c_str());

	if (rand() % 10 >= 5)
		g_HttpMgr.PostData("tj.game.sdo.com","cgi-bin/receivedata",popstr1.c_str(),8080);
	else
		g_HttpMgr.PostData("tj1.game.sdo.com","cgi-bin/receivedata",popstr1.c_str(),8080);


	output_debug("end SEND_Login_Tongji2");
}


void CGameControl::MSG_Session_ID(const char * msg,int iLen)
{
    //char temp[256];
    //sprintf(temp,"%d %d %d %d %d %d %d %d %d %d %d %d,id %x",
    //	msg[0],msg[1],msg[2],msg[3],msg[4],msg[5],
    //	msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],
    //	*((DWORD*)msg));
    //MessageBox(NULL,temp,NULL,0);

    //g_pGameData->SetSessionID(*((DWORD*)msg));

}

void CGameControl::MSG_Password_Change_Ok(const char * msg,int iLen)
{
    g_MsgBoxMgr.PopSimpleAlert("修改密码成功，新密码将在五分钟后生效！");
}

void CGameControl::MSG_Password_Change_Fail(const char * msg,int iLen)
{
    g_MsgBoxMgr.PopSimpleAlert("修改密码失败！");
}

bool CGameControl::SEND_Login_ServerName()
{
    SET_COMMAND(CS_LOGIN_SERVERNAME,256);
    ADD_STR(g_Login.GetGroupName());

    return (SEND_GAME_SERVER() == iBLen);
}

void CGameControl::SEND_Login_Userpwd_Change(const char * info,DWORD dwState)
{
    SET_COMMAND(CS_LOGIN_USERPWD_CHANGE,256);
    ASSIGN_ID(dwState);
    ADD_STR(info);
    SEND_GAME_SERVER();
}

void CGameControl::SEND_Create_Account(const char * info,bool bBind,DWORD dwState,const char * strValidate)
{
    SET_COMMAND(CS_CREATE_ACCOUNT,1024);
    ASSIGN_ID(dwState);

    if(bBind)
    {
        ASSIGN_BYTE(6,0x01);
    }
    if(strValidate != NULL)
    {
        ASSIGN_BYTE(7,0x01);
        ADD_ARRAY(info,311);
        ADD_STR(strValidate);
        SEND_GAME_SERVER();
    }
    else
    {
        ADD_ARRAY(info,311);
        SEND_GAME_SERVER();
    }
}


bool CGameControl::SEND_Query_Username(const char * str, int iLen,DWORD dwState)
{
    if(strlen(str) > 10||strlen(str)==0)
        return false;

    SET_COMMAND(CS_QUERY_USERNAME,256);
    ASSIGN_ID(dwState);
    ADD_STR(str);
    return ( SEND_GAME_SERVER() == iBLen);
}


void CGameControl::MSG_Create_Account_Fail(const char * msg,int iLen)
{
    string str;
    if(iLen > 12)
    {
        str.assign(msg + 12,iLen-12);
        g_MsgBoxMgr.PopSimpleAlert(str.c_str());
        return;
    }

    DWORD dwCode = *((DWORD*)msg);
    switch(dwCode)
    {
    case 0:
        str = "帐号已存在，请重新创建！";
        break;
    case -1:
        str = "创建帐号失败！";
        break;
    case 0x05:
        str = "该手机号码不能绑定！\n如果想绑定，请重填手机号码。";
        break;
    default:
        str = "创建帐号失败！";
        break;
    }

    g_MsgBoxMgr.PopSimpleAlert(str.c_str());
}

void CGameControl::MSG_Create_Account_Success(const char * msg,int iLen)
{
    if( iLen == 12)
    {
    }
    else
    {
        string txt(msg + 12, iLen - 12);
        if(txt.find('/') != txt.npos)		// 发现/
        {
            return;
        }
        char p[12] = {0};
        memcpy(p,msg + iLen - 11,11);
        char pp[64] = {0};
        memcpy(pp,msg + 12 ,iLen - 24);
        g_MsgBoxMgr.PopSimpleAlert("     您已经成功注册盛大通行证！登陆游戏时请务必选择\n"
            "“盛大通行证登陆”方可输入帐号、密码，否则将无法通过\n"
            "认证。凭借您的盛大通行证还可享受交友、购物、文学等其\n"
            "它POPTANG平台服务。地址：www.sdo.com");
    }
}

void CGameControl::MSG_Create_Character_Fail(const char * msg,int iLen)
{
    if(msg[0] == 0)
    {
        g_MsgBoxMgr.PopSimpleAlert("创建角色失败！");
    }
    else if(msg[0] == 2)
    {
        g_MsgBoxMgr.PopSimpleAlert("创建角色失败：角色名已经存在！");
    }
}

//void CGameControl::MSG_Character_Info(const char * msg,int iLen)
//{
//    SEND_Selchar_CharServer();
//}

void CGameControl::MSG_User_Full(const char * msg,int iLen)
{
    g_MsgBoxMgr.PopSimpleAlert("服务器人数已满，请稍后再试！");
    //g_pControl->Msg(MSG_CTRL_ENTER_GAME_STOP,0,0); 
}

void CGameControl::MSG_Connect_Error(const char * msg,int iLen)
{
    g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CLOSE);

	if(g_MsgBoxMgr.FindMsgBox(MSG_RECONNECT_SERVER) == NULL)
	{
		g_MsgBoxMgr.PopSimpleAlert("连接服务器失败！",MSG_RECONNECT_SERVER);
	}
    //g_pControl->Msg(MSG_CTRL_ENTER_GAME_STOP,0,0);
}

void CGameControl::MSG_Error_Login(const char * msg,int iLen)
{
	if(g_MsgBoxMgr.FindMsgBox(MSG_RECONNECT_SERVER) == NULL)
	{
		g_MsgBoxMgr.PopSimpleAlert("服务器忙，请稍后再试！",MSG_RECONNECT_SERVER);
	}
    //g_pControl->Msg(MSG_CTRL_ENTER_GAME_STOP,0,0);
}

void CGameControl::MSG_Username_Info (const char * msg,int iLen)
{
    DWORD dwCode = *((DWORD *)msg);

    if(dwCode == 0xFFFFFFFF) //DB错误
    { 
        g_MsgBoxMgr.PopSimpleAlert("服务器忙碌，请稍后再试。");
    }
    else if(dwCode == 0x0) //账号不存在，可以建立账户
    {
        g_MsgBoxMgr.PopSimpleAlert("恭喜您，这个帐号名尚无人使用，请您务必详细填写注册资料。");
    }
    else if(dwCode == 0x01) //账号存在，不可以建立账户
    {
        g_MsgBoxMgr.PopSimpleAlert("您好，这个帐号名已经存在，请您另选一个帐号名注册。");
    }
}

void CGameControl::MSG_Version_NoMatch(const char * msg,int iLen)
{
    g_MsgBoxMgr.PopSimpleAlert("您的客户端版本过旧，无法登陆服务器进行游戏。\n"
        "请使用客户端的自动更新功能或者到qyz.sdo.com网站\n下载最新的客户端补丁。");
}

// UUID
void CGameControl::MSG_Need_Upto_Uuid(const char * msg,int iLen)
{
    //取出剩余时间
    DWORD dwTemp = *((DWORD*)msg);
    DWORD dwTemp2 = *((DWORD*)(msg+6));

    //发送消息
    //g_pControl->Msg(MSG_CTRL_NEED_UU, dwTemp, (CControl*)dwTemp2);
}


void CGameControl::MSG_Server_Temp_Uuid(const char * msg,int iLen)
{
    //取出头信息
    DWORD dwResult = *((DWORD*)msg);

    string buf;
    buf.assign(msg+12,iLen-12);

    switch(dwResult)
    {
    case 0x00000000:		//成功并有推荐id返回
        //发送消息
        //g_pControl->Msg(MSG_CTRL_APLLY_UU, UUID_RECOMMEND, (CControl*)msg[11]);
        break;
    case 0xFFFFFFFF:		//系统错
        //发送错误
        //g_pControl->Msg(MSG_CTRL_APLLY_UU, UUID_ERROR, (CControl*)msg[11]);
        break;
    default:
        break;
    }
}


void CGameControl::MSG_Change_Uuid_Result(const char * msg,int iLen)
{
    DWORD dwResult = *((DWORD*)msg);

    string buf;
    buf.assign(msg+12,iLen-12);

    switch(dwResult)
    {
    case 0x00000000:		//修改成功
        //g_pControl->Msg(MSG_CTRL_APLLY_UU, UUID_SUCCESS, NULL);
        break;
    case 0xFFFFFFFF:		//uuid已存在
        //g_pControl->Msg(MSG_CTRL_APLLY_UU, UUID_FAIL, NULL);
        break;
    case 0xFFFFFFFE:		//系统错
        //g_pControl->Msg(MSG_CTRL_APLLY_UU, UUID_ERROR, NULL);
        break;
    }

}


void CGameControl::SEND_Upto_Uuid()
{
    SET_COMMAND(CS_UPTO_UUID,12);
    SEND_GAME_SERVER();
}


void CGameControl::SEND_Change_Uuid(const char *str)
{
    size_t iLen = strlen(str);
    if(iLen > 64 || iLen ==0)
        return;

    SET_COMMAND(CS_CHANGE_UUID,256);
    ADD_STR(str);
    SEND_GAME_SERVER();
}

void CGameControl::MSG_Dynamic_Password_Result(const char * msg,int iLen)
{
    string str;
    DWORD dwResult = *((DWORD*)msg);

    if(dwResult == 0)
    {
        str.assign(msg+12,iLen-12);
        g_MsgBoxMgr.PopSimpleComfirm(str.c_str(),0,0);
        return;
    }
    else if(dwResult == 0xFFFFFFFF)
        str = "用户名不存在！";
    else if(dwResult == 0xFFFFFFFE)
        str = "密码错误！";
    else if(dwResult == 0xFFFFFFFD)
        str = "其他错误！";
    else if(dwResult == 0xFFFFFFFC)
        str = "用户已绑定UUID！";
    else
        str = "未知错误";

    g_MsgBoxMgr.PopSimpleAlert(str.c_str());
}

void CGameControl::MSG_Undynamic_Password_Result(const char * msg,int iLen)
{
    string str;
    DWORD dwResult = *((DWORD*)msg);

    if(dwResult == 0)
    {
        str = "您的帐号已经与盛大密宝解除绑定。目前您的帐号可以仅使用游戏\n"
            "密码登录，您的密宝可以申请与其他帐号进行绑定。如有疑问，\n"
            "请访问网站http://ekey.poptang.com，\n或与我们的客服热线（021-50504729）联系。";
    }
    else if(dwResult == 0xFFFFFFFF)
        str = "帐号不存在";
    else if(dwResult == 0xFFFFFFFE)
        str = "密码错误";
    else if(dwResult == 0xFFFFFFFD)
        str = "序列号错误";
    else if(dwResult == 0xFFFFFFFC)
        str = "序列号与服务号不对应";
    else
        str = "未知错误";

    g_MsgBoxMgr.PopSimpleAlert(str.c_str());
}

void CGameControl::MSG_Assert_Bind(const char * msg,int iLen)
{
    memcpy(g_cBuf,msg + 12,iLen - 12);
    g_cBuf[iLen -12] = 0;
    //g_pControl->Msg(MSG_CTRL_CONFIRM_WND,0);
}

void CGameControl::MSG_Input_Reserve_Password(const char * msg,int iLen)
{
    DWORD dwResult = *((DWORD*)msg);

    if(dwResult == 0xFFFFFFFF)
    {
        g_MsgBoxMgr.PopSimpleAlert("预留口令错误，重新输入预留口令。");
    }

    g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);

    if(msg[6] == 0)	//秘宝预留口令
    {
        g_pControl->PopupWindow(MSG_CTRL_RESERVEPASSWORD_WND,OPER_UPDATE,0);
    }
    else if(msg[6] == 1)		//安全卡预留口令
    {
        g_pControl->PopupWindow(MSG_CTRL_RESERVEPASSWORD_WND,OPER_UPDATE,1);
    }

    //	g_pControl->Msg(MSG_CTRL_LOGIN_OPENWND,2);
}

void CGameControl::MSG_Card_Result(const char * msg,int iLen)
{
    string str;
    DWORD dwResult = *((DWORD*)msg);

    if(dwResult == 0xFFFFFFFF)
        str = "密宝错误";
    else if(dwResult == 0x0)
        str = "您的密宝已到期或已停用";
    else
        str = StringUtil::format("您的密宝还有%d天到期。",dwResult);

    g_MsgBoxMgr.PopSimpleAlert(str.c_str(),0);
}


void CGameControl::SEND_Bind_Dynamic_Password(const char * sUser,const char * sPass,const char * sSerial,const char * sDPass)
{
    SET_COMMAND(CS_BIND_DYNAMIC_PASSWORD,256);
    temp[11] = 1;

    ADD_STR(sUser);
    ADD_BYTE('/');
    ADD_STR(sPass);

    SEND_GAME_SERVER();
}

void CGameControl::SEND_Unbind_Dynamic_Password(const char * sUser,const char * sPass,const char * sSerial,const char * sDPass,const char * sServeID)
{
    SET_COMMAND(CS_UNBIND_DYNAMIC_PASSWORD,256);

    ADD_STR(sUser);
    ADD_BYTE('/');
    ADD_STR(sPass);
    ADD_BYTE('/');
    ADD_STR(sSerial);
    ADD_BYTE('/');
    ADD_STR(sDPass);
    ADD_BYTE('/');
    ADD_STR(sServeID);

    SEND_GAME_SERVER();
}

void CGameControl::SEND_Assert_Bind(int iFlag)
{
    SET_COMMAND(CS_ASSERT_BIND,256);

    temp[0]	= (char)iFlag;
    if(iFlag == 1)
    {
        if(strlen(g_cBuf) < 64)
        {
            ADD_STR(g_cBuf);
        }
    }
    SEND_GAME_SERVER();
}

//void CGameControl::SEND_Assert_Reserve(const char *sReserve,bool bSafeCard)
//{
//    SET_COMMAND(CS_ASSERT_RESERVE,256);
//    ADD_STR(sReserve);
//
//    if(bSafeCard)//安全卡预留口令
//        ASSIGN_BYTE(6,1);
//
//    SEND_GAME_SERVER();
//}

void CGameControl::SEND_Assert_Card()
{
    SET_COMMAND(CS_ASSERT_CARD,12);
    SEND_GAME_SERVER();
}

void CGameControl::MSG_Normal_MessageBox(const char * msg,int iLen)
{
    string str;
    str.assign(msg+12,iLen-12);

    g_pControl->Msg(MSG_CTRL_ENABLELOGINWND,0);

    if(msg[0] == 0x64)
    {
        return;
    }
    else if(*((DWORD *)(msg + 6)) != 0)
    {
        g_MsgBoxMgr.PopSimpleComfirm(str.c_str(),0,0);
    }
    else
    {
		g_MsgBoxMgr.PopSimpleAlert(str.c_str(),0);
    }
}

void CGameControl::MSG_Dynamic_Password_Change(const char * msg,int iLen)
{
    string txt(msg + 12, iLen - 12);
    //[Need Uncomment]
    //g_pControl->Msg(MSG_CTRL_DYNAMIC_PASSWORD_CHANGE,(DWORD)(txt.c_str()),(CControl*)1);
}

void CGameControl::SEND_Next_Dynamic_Password(const char * str)
{
    SET_COMMAND(CS_SEND_NEXT_DYNAMIC_PASSWORD,256);
    ADD_STR(str);
    SEND_GAME_SERVER();
}

void CGameControl::SEND_New_Version()
{
    SET_COMMAND(CS_NEW_VERSION,12);
    SEND_GAME_SERVER();
}

void CGameControl::SEND_Warrantor(const char * strName)
{
    if(strlen(strName) > 0)
    {
        SET_COMMAND(CS_SEND_WARRANTOR,256);
        ADD_STR(strName);
        SEND_GAME_SERVER();
    }
}

void CGameControl::MSG_Input_MIBAO_LATER(const char * msg,int iLen)
{
	int iTime = *( (WORD*)(msg+6));
	g_AIMgr.SetReserveTime(plyMibaoLateTime,iTime);
    if(msg[0] == 1)//普通密宝
    {
        string txt;
        if(iLen > 12)
        {
			if (msg[1] == 0)
			{
				string msg (msg + 12,iLen - 12);
				txt = "请根据随机数“";
				txt += msg;
				txt += "”生成动态密码并输入：";
			}
			else if(msg[1] == 1)
			{
				txt = "请输入您密宝的";
				for(int i = 12; i < iLen; i++)
				{
					txt += "第";
					txt += msg[i];
					txt += "位，";
				}
				txt.erase(txt.size() - 2);
				txt += "：";
			}
        }
        else
        {
            txt = "请输入您的密宝：";
        }
        //弹出预备输入密宝的消息框
        g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_LATER_WND,OPER_CREATE);
        g_pControl->MsgToWnd(MSG_CTRL_INPUT_MIBAO_LATER_WND,MSG_CTRL_INPUT_MIBAO_LATER_WND,1,(CControl*)txt.c_str());

		if (iLen > 12 && msg[1] == 1)
		{
			g_pControl->MsgToWnd(MSG_CTRL_INPUT_MIBAO_LATER_WND,MSG_CTRL_INPUT_MIBAO_LATER_WND,2,(CControl*)(iLen - 12));
		}
    }
	else if(msg[0] == 2)//usb密宝,msg[1] == 0自动密码,msg[1] == 1 手动密码,要按一下usb密宝上的按键
	{
		string strPin(msg + 12,iLen - 12);
		g_Login.SetUsbEKeyChallengeCode(strPin.c_str());
        //弹出预备输入密宝的消息框
        g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_LATER_WND,OPER_CREATE,100 + msg[1]);
	}
}

//输入动态密码 S 511 MSG = 4880 LEN 4 C->S 2046
void CGameControl::MSG_Input_DPWD(const char * msg,int iLen)
{
	//传世动态密码输入
	string txt;
	txt.append(msg, iLen);
	if(iLen >= 4)
	{
		//g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,0);
		//g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt.c_str());
		//g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,2);	
		//自动发送密码

		g_pNet->SendBuf(SERVER_GAME, (char*)txt.c_str(), 0, 2046);
		return;
	}

	/*
    if(msg[0] == 1)
    {
        if(iLen > 12)
        {
            string msg (msg + 12,iLen - 12);
            txt = "请根据随机数“";
            txt += msg;
            txt += "”生成动态密码并输入：";
        }
        else
        {
            g_Login.SetShowRainBow(false);
            txt = "请输入您的密宝：";
        }

        g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,0);
        g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt.c_str());
		g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,3);
    }
    else if(msg[0] == 0)//登录游戏时输入密保
    {
        if(msg[1] == 0)
        {
            if(iLen > 12)
            {
                txt = "请输入您密宝的";
                for(int i = 12; i < iLen; i++)
                {
                    txt += "第";
                    txt += msg[i];
                    txt += "位，";
                }
                txt.erase(txt.size() - 2);
                txt += "：";
            }
        }
        else if(msg[1] == 1)
        {
            string txt2(msg + 12,iLen - 12);
            txt = "请根据随机数“";
            txt += txt2;
            txt += "”生成动态密码并输入：";
        }

        g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,0);
        g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt.c_str());
		g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,3);

		if (msg[1] == 0 && iLen > 12)
		{
			g_pControl->MsgToWnd(MSG_CTRL_INPUT_MIBAO_WND,MSG_CTRL_INPUT_MIBAO_WND,4,(CControl*)(iLen - 12));
		}
    }
	*/
 //   else if(msg[0] == 2)//使用安全卡(大)
 //   {
 //       string txt2(msg + 12,iLen - 12);		
 //       g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,1);
 //       g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt2.c_str());
 //   }
 //   else if(msg[0] == 3)//使用安全卡(小)
 //   {
 //       string txt2(msg + 12,iLen - 12);		
 //       g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,1);
 //       g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt2.c_str());
 //       g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,2);
 //   }
	//else if(msg[0] ==4)//usb密宝 msg[1] == 0自动密码,msg[1] == 1 手动密码,要按一下usb密宝上的按键
	//{
	//	string strPin(msg + 12,iLen - 12);
	//	g_Login.SetUsbEKeyChallengeCode(strPin.c_str());

	//	//g_pGameControl->SEND_UsbMb(msg[1],1);
	//}
}

//byType == 0获取自动动态密码,byType == 1手动密码
//dwData: 1:服务器发协议来要求输入密宝,读成功后发送SEND_Input_DPWD,
//dwData: 2:延时读取手动密码
//void CGameControl::SEND_UsbMb(BYTE byType,DWORD dwData)
//{
//	char dynpwd[100]={0};			
//	string strPin = g_Login.GetUsbEKeyChallengeCode();
//	UsbClientSDK *pUsbSDK = UsbClientSDK::GetInInstance();
//	int retVal = -1;
//
//    if(dwData == 1)
//	{
//		//获取自动动态密码
//		int retVal = pUsbSDK->GetPWDAuto((unsigned char *)dynpwd, (unsigned char *)(strPin.c_str()));//-8 参数错误,-9 异常,0  成功,-1  没找到设备
//		if(retVal != 0)
//		{
//			g_MsgBoxMgr.PopOkCancelBox("读取USB密宝失败,请正确插入USB密宝后再点击确认按钮",MSG_CTRL_GET_USBMB_CODE,byType);
//		}
//		else
//		{
//			if(byType == 1)//手动密码
//			{
//				g_OtherData.SetReadUsbEkeyDelayStartTime(GetTickCount());
//				g_MsgBoxMgr.PopSimpleAlert("请按下USB密宝按键");
//				CCtrlMsgBox * pMsgBox = g_MsgBoxMgr.GetInstance();
//				if(pMsgBox)
//				{
//					pMsgBox->SetEnable(false);
//				}
//			}
//			else
//			{
//				g_pGameControl->SEND_Input_DPWD(dynpwd,2);
//			}
//		}
//	}
//	else if(dwData == 2)
//	{
//		retVal =pUsbSDK->GetPWDManual((unsigned char *)dynpwd, (unsigned char *)(strPin.c_str()));
//		g_MsgBoxMgr.ClearExistMsgBox("请按下USB密宝按键");
//		if(retVal != 0)
//		{
//			g_MsgBoxMgr.PopOkCancelBox("读取USB密宝失败,请正确插入USB密宝后再点击确认按钮",MSG_CTRL_GET_USBMB_CODE,byType);
//		}
//		else
//		{
//			g_pGameControl->SEND_Input_DPWD(dynpwd,2);
//		}
//	}
//}

////bytype: 0:密宝,1:安全卡,2:usb密宝
//void CGameControl::SEND_Input_DPWD(const char * msg, BYTE byType)
//{
//    if(strlen(msg) == 0)
//        return;
//
//    SET_COMMAND(CS_INPUT_DPWD,256);
//	ASSIGN_BYTE(6,byType);
//
//	ADD_STR(msg);
//	//打开彩虹帮助
//	g_Login.SetShowRainBow(true);

//	if(byType == 0)
//		g_Login.SetEKey(msg);
//
//	if (g_Login.GetLoginType() == LOGINTYPE_PTACCOUNT)
//		g_Login.SetLoginType(LOGINTYPE_PTACCOUNTWITHEKEY);
//	else
//		g_Login.SetLoginType(LOGINTYPE_PTNUMIDWITHEKEY);
//
//	SEND_GAME_SERVER();
//}

void CGameControl::MSG_Rungate_DPWD(const char * msg,int iLen)
{
    //string txt = "请输入您的盛大密宝：";
    //g_pControl->Msg(MSG_CTRL_DYNAMIC_PASSWORD_CHANGE,(DWORD)(txt.c_str()),0);
}

void CGameControl::SEND_Rungate_DPWD(const char * msg)
{
    if(strlen(msg) == 0)
        return;

    SET_COMMAND(CS_RUNGATE_DPWD,256);
    ADD_STR(msg);
    SEND_GAME_SERVER();
}

void CGameControl::SEND_Login_Validate()
{
    SET_COMMAND(CS_LOGIN_VALIDATE,CMD_SIZE);
    SEND_GAME_SERVER();
}

void CGameControl::MSG_Login_Validate(const char * msg,int iLen)
{
    FILE * fp = fopen("tmp.tex","wb");
    fwrite(msg + 12,iLen - 12,1,fp);
    fclose(fp);
}

//void CGameControl::MSG_Level_Check(const char * msg,int iLen)
//{
//    BYTE byType = *((BYTE*)(msg + 6));
//    g_Login.SetEnterCheckType(byType);
//
//    FILE * fp = fopen("tmp1.tex","wb");
//    fwrite(msg + 12,iLen - 12,1,fp);
//    fclose(fp);
//    g_pControl->Msg(MSG_CTRL_ENTER_GAME_STOP,0,0);
//    g_MsgBoxMgr.PopEditBox("",MSG_CTRL_LEVEL_CHECK,0);
//}

void CGameControl::SEND_Level_Check(const char * strName)
{
    //g_pControl->Msg(MSG_CTRL_ENTER_GAME_BEGIN,0,0);

    SET_COMMAND(CS_LEVEL_CHECK,20);
    if(strlen(strName) <= 6)
    {
        ADD_STR(strName);
        SEND_GAME_SERVER();
    }
}

void CGameControl::MSG_Check_User(const char * msg, int iLen)
{
    SEND_Check_User();
}
void CGameControl::SEND_Check_User()
{
    SET_COMMAND(CS_CHECK_USER,12);

    int iSize = strlen(g_Login.GetLoginID()) + strlen(SELF.GetName());

    ASSIGN_WORD(6,((iSize)	^ 0x1907));
    SEND_GAME_SERVER();
}

void CGameControl::MSG_Web_MessageBox(const char* msg,int iLen)
{
    if(iLen > 4096 || iLen < 12)
        return;

    string str;
    str.assign(msg + 12,iLen - 12);
	/////////////////////
	//if(g_Login.GetAutoLoginInType() > 0)//自动登录的时候不弹登录logingate后的那个提示框
	//{
	//	if(str.find("您的盛大通行证帐号是") >= 0)
	//	{
	//		return;
	//	}
	//}

    g_MsgBoxMgr.PopTagAlert(str.c_str());
}


//void CGameControl::SEND_Login_Register_Info(const char* szName,const char* szIdentityCard,int iAdult)
//{
//    SET_COMMAND(CS_LOGIN_REGISTER_INFO,256);
//
//    char cTemp[256] = {0};
//    g_pNet->GetServer(cTemp);
//    if(stricmp(cTemp,g_Login.GetLoginServer()) == 0 &&
//        g_pNet->GetPort() == g_Login.GetLoginServerPort()) //当前连接的是LoginServer
//    {
//        sprintf(temp+12,"%s/%s/%s/%d",g_Login.GetLoginID(),szName,szIdentityCard,iAdult);
//
//        g_pNet->SendBuf(SERVER_GAME,temp,12+strlen(temp+12));
//    }
//    else
//    {
//        //sprintf(temp+12,"%s/%s/%s/%d/%d",g_Login.GetLoginID(),szName,szIdentityCard,iAdult,g_Login.GetSessionID());
//		sprintf(temp+12,"%s/%s/%s/%d",g_Login.GetLoginID(),szName,szIdentityCard,iAdult);
//
//        g_pNet->SetServer(SERVER_LOGIN,g_Login.GetLoginServer(),g_Login.GetLoginServerPort());
//        g_pNet->Connect(SERVER_LOGIN); //连接LoginServer,用于输入用户信息
//
//        Sleep(1000);
//        g_pNet->SendBuf(SERVER_LOGIN,temp,12+strlen(temp+12));
//
//        //协议头第5字节(下标4)：0xE1协议体:游戏账号/姓名/身份证号码
//        Sleep(1000);
//        g_pNet->Close(SERVER_LOGIN);
//    }
//}

void CGameControl::SEND_FCMFillInfo_Req(const char* szName,const char* szIdentityCard,int iAdult)
{
	PKG_CLI_GG_FCMFillInfo_REQ stAck;

	sprintf(stAck.szName,"%s",szName);
	sprintf(stAck.szPTID,"%s",g_Login.GetLoginID());
	sprintf(stAck.szIDCard,"%s",szIdentityCard);
	stAck.byFCMFlag = (BYTE)iAdult;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stAck, 0, CLI_GG_FCMFillInfo_REQ);

}


bool CGameControl::SEND_Login_LoginInServer()
{
    LPCTSTR strIP = (LPCTSTR)g_pStreamMgr->GetGameStr("ServerIP","61.152.97.13"); 
    int iPort = g_pStreamMgr->GetGameInt("ServerPort",7000);
    g_pNet->SetServer(SERVER_GAME,strIP,iPort);
	g_Login.SetLoginServer(strIP,iPort);
    LPCTSTR strAreaNo = (LPCTSTR)g_pStreamMgr->GetGameStr("Area","0");
    g_Login.SetAreaNo(atoi(strAreaNo));
    LPCTSTR strAreaName = (LPCTSTR)g_pStreamMgr->GetGameStr("AreaName","206");
    g_Login.SetAreaName(strAreaName);

    g_pNet->Reset();

    bool bRet = g_pNet->Connect(SERVER_GAME);
    return bRet;
}

// void CGameControl::SEND_Uuid_Login(const char * sUuID,const char *sPassword)
// {
//     g_Login.SetAccount(sUuID,true);
// 
//     SET_COMMAND(CS_UUID_LOGIN,256);
//     ASSIGN_DWORD(6,GAME_VERSION);
// 
//     ADD_STR(sUuID);
//     ADD_BYTE('/');
//     ADD_STR(sPassword);
//     SEND_GAME_SERVER();
// }
// 
// 
// void CGameControl::SEND_Login_UserPwd(const char * strUser,const char * strPwd)
// {
//     g_Login.SetAccount(strUser,false);
//     g_Login.SetLoginID(strUser);
// 
//     SET_COMMAND(CS_LOGIN_USERPWD,256);
//     ASSIGN_BYTE(0,1);
//     ASSIGN_BYTE(3,strlen(strUser) + 1);
//     ASSIGN_DWORD(6,GAME_VERSION);
// 
//     ASSIGN_BYTE(2,strlen(strUser) + strlen(strPwd) + 1);
// 
//     ADD_STR(strUser);
//     ADD_BYTE('/');
//     ADD_STR(strPwd);
// 
//     SEND_GAME_SERVER();
// }
// 
// void CGameControl::MSG_Login_UserPwdOK(const char * msg,int iLen)
// {
// 	DWORD dwCode = g_pStreamMgr->GetOtherConfigInt("ClientType",0);
//     SEND_Login_Tongji(dwCode); //发送登陆成功的统计信息
// 
//     g_pStreamMgr->SetConfigStr("LPoptang",g_Login.IsPoptang()?"Yes":"No");
// 
//     if(g_Login.IsTuiguang())
//     {
//         g_Login.SetTuiguang_str(msg);
//         g_Login.SetTuiguang(false);
//         return;
//     }
// 
//     g_Login.SetAntiCM(msg[10] == 1); //是否开启防沉迷资料补填
// 
//     WORD wDays =  Conv_WORD(msg);
//     WORD wHours = Conv_WORD(msg+2);
// 
//     WORD wIDays = Conv_WORD(msg+6);
//     WORD wIHours = Conv_WORD(msg+8);
// 
//     string sInfo;
// 
//     if(msg[11])
//     {
//         //g_pControl->Msg(MSG_CTRL_NEED_UU, *((DWORD*)msg), (CControl*)wIDays);
//     }
//     else if(wDays != 0)
//     {
//         if(wDays == 1)
//             sInfo = "您的付费将于今天到期！";
//         else
//         {
//             if(wHours != 0 )
//                 sInfo = StringUtil::format("您的账户所剩余的时间：%d天%d小时",wDays,wHours);
//             else
//                 sInfo = StringUtil::format("您的账户所剩余的天数：%d天",wDays);
//         }   
//     }
//     else if(wIDays != 0 )
//     {
//         if(wIDays == 1)
//             sInfo = "当前IP的付费将于今天到期！";
//         else
//         {
//             if(wIHours != 0 )
//                 sInfo = StringUtil::format("当前IP所剩余的时间：%d天%d小时",wIDays,wIHours);
//             else
//                 sInfo = StringUtil::format("当前IP所剩余的天数：%d天",wIDays);
//         }
//     }
//     else if(wHours != 0 )
//     {
//         sInfo = StringUtil::format("你的账户所剩余的时间：%d小时",wHours);
//     }
//     else if(wIHours != 0 )
//     {
//         sInfo = StringUtil::format("当前IP所剩余的时间：%d小时",wIHours);
//     }
// 
//     char poptang_str[64] = {0};
//     string str;
// 
//     if(iLen > 12 && iLen < 64)
//     {
//         str.assign(msg+12,iLen - 12);
//         g_Login.SetLoginID(str.c_str());
//     }
//     else if(iLen > 64 && iLen < 105)
//     {
//         str.assign(msg + 12,iLen - 12 - 41);
//         g_Login.SetLoginID(str.c_str());
// 
//         if(msg[64] == '/')
//             memcpy(poptang_str,msg + 65,iLen - 65);
//         g_Login.SetPoptang_str(poptang_str);
//     }
//     else if(iLen > 105 && iLen < 150)
//     {
//         if(msg[64] == '/')
//             memcpy(poptang_str,msg + 65,40);
//         g_Login.SetPoptang_str(poptang_str);
// 
//         std::string pt;
//         pt.assign(msg + 105,iLen - 105);
//         g_Login.SetPoptang(pt.c_str());
//     }
//     else if(iLen > 160 && iLen <= 200)
//     {
//         char temp[64] = {0};
//         memcpy(temp,msg + 12,64);
//         g_Login.SetLoginID(temp);
// 
//         if(msg[64] == '/')
//             memcpy(poptang_str,msg + 65,40);
//         g_Login.SetPoptang_str(poptang_str);
// 
//         ZeroMemory(temp,64);
//         memcpy(temp,msg + 160,40);
// 
//         int iStart = 0;
//         int iLen = strlen(temp);
// 
//         string day = StringUtil::toStr(temp, iStart);
//         string time = StringUtil::toStr(temp, iStart);
//         string security = StringUtil::toStr(temp, iStart);
// 
//         if(strcmp(security.c_str(),"3") == 0)
//         {
//             security = "低。\n盛大网络强烈建议您更换一张新的安全卡来保护帐号";
//         }
//         if(strcmp(security.c_str(),"2") == 0)
//         {
//             security = "中。\n盛大网络提醒您游戏之余准备一张新的安全卡来保护帐号";
//         }
//         if(strcmp(security.c_str(),"1") == 0)
//         {
//             security = "高。\n盛大网络祝您游戏愉快";
//         }
// 
//         char text[512] = {"0"};
// 
// 		sprintf(text,"盛大安全卡验证通过。\n\n您的帐号绑定这张安全卡已经%s天。\n该卡已经使用了%s次。\n您的帐号安全系数为：%s。",day.c_str(),time.c_str(),security.c_str());
// 
//         sInfo = text;
//     }
// 
// 
//     // 设置 组 信息
//     int iGroupNum = g_pStreamMgr->GetGameInt("GroupNum",0);
//     if(iGroupNum <= 0)
//     {
//         g_MsgBoxMgr.PopSimpleAlert("错误：配置文件中无组信息！");
//         return;
//     }
// 
//     g_Login.GetGroupList().clear();
// 
//     //设置Group的信息
//     char stmp[64] = {0};
//     for(int j=0;j<iGroupNum;j++)
//     {
//         sprintf(stmp,"Group%d",j);
//         string strGroup = g_pStreamMgr->GetGameStr(stmp);
// 
//         sprintf(stmp,"GroupNick%d",j);
//         string strNick = g_pStreamMgr->GetGameStr(stmp);
//         g_Login.GetGroupList().push_back(_GroupInfo(strNick,strGroup));
//     }
// 
//     g_pControl->Msg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);
// 
// 	if(g_Login.GetAutoLoginInType() > 0)
// 	{
// 		g_pGameControl->SEND_Login_ServerName();					
// 		return;
// 	}
// 	else
// 	{
// 		g_pControl->Msg(MSG_CTRL_SELECTSERVERWND,OPER_CREATE);
// 		g_Login.SetShowLicenseWnd(true);
// 
// 		if(sInfo.size() != 0)
// 		{
// 			g_MsgBoxMgr.PopSimpleAlert(sInfo.c_str());
// 		}
// 	}
// }
// 
// void CGameControl::MSG_Login_UserPwdFail(const char * msg,int iLen)
// {
//     g_pStreamMgr->SetConfigStr("LPoptang",g_Login.IsPoptang()?"Yes":"No");
// 
//     if(iLen > 12)
//     {
//         std::string pt;
//         pt.assign(msg + 12,iLen - 12);
//         g_Login.SetPoptang(pt.c_str());
//     }
// 
//     g_Login.SetLoginSend(false);
// 
//     DWORD dwCode = *((DWORD*)msg);
// 
//     g_pControl->Msg(MSG_CTRL_ENABLELOGINWND,0);
// 
//     string str;
// 
//     if(dwCode == 0xFFFFAFFF)
//     {
//         str = "Passcode输入不正确，请重新输入！";
//     }
//     else if(dwCode == 0xFFFFFFFC)
//     {
//         string acc(msg + 12,iLen - 12);
//         str = "您必须用盛大通行证：" + acc + "登陆！";
//     }
//     else if(dwCode== 0xFFFFFFFB)
//     {
//         str.assign(msg + 12,iLen - 12);
//     }
//     else if(dwCode == 0xFFFFBFFF)
//     {
//         str = "需要输入下一个Passcode，请在Passcode更新后重新输入！";
//     }
//     else if(dwCode == 0xFFFFCFFF)
//     {
//         str = "令牌被锁定，请联系管理员！";
//     }
//     else if(msg[0] == -1)
//     {
//         str = "密码不正确，请重新输入密码";
//     }
//     else if(msg[0] == -2)
//     {
//         str = "您的密码连续三次输入错误，此帐号将被暂时封闭！\n请在5分钟后重新登陆";
//     }
//     else if(msg[0] == -3)
//     {
//         str = "这个帐号正在使用，或者被异常的终止锁定了，\n请稍后再试！";
//     }
//     else if(msg[0] == 0)
//     {
//         g_pControl->Msg(MSG_CTRL_CLEARID,0);
//         str = "该帐号不存在，请重新输入帐号。";
//     }
//     else if(dwCode == 0xEEEEEEEE)
//     {
//         g_pControl->Msg(MSG_CTRL_CLEARID,0);
//         str = "安全卡验证码错误，请重新输入。";
//     }
//     else if(dwCode == 0xDDDDDDDD)
//     {
//         g_pControl->Msg(MSG_CTRL_CLEARID,0);
//         str = "        安全卡密码连续验证错误，您的帐号将被暂时锁定，\n            请不要再次尝试输入，以防帐号被盗。";
//     }
// 
//     if(str.size() != 0)
//         g_MsgBoxMgr.PopSimpleAlert(str.c_str());
// 
// 	g_Login.SetAutoLoginInType(0);
// }
// 
// 
// //老协议:  角色服务器IP/端口号/Session ID
// //新协议:  角色服务器IP/端口号/Session ID(加密后) / 加密算法(加密后)
// 
// void CGameControl::MSG_Login_CharServer(const char * msg,int iLen)
// {
//     g_Login.SetCharSend(false); //清掉
// 
//     WORD wIP = Conv_WORD(msg+10);
//     WORD wAccountDay = Conv_WORD(msg);
//     WORD wIPHour = Conv_WORD(msg+8);
//     WORD wAccountHour = Conv_WORD(msg+2);
//     WORD wIPDay =  Conv_WORD(msg+6);
// 
//     string str;
// 
//     if(wIP != 0)  
//     {
//         str = "您享受IP授权服务";
//     }
//     else if(wAccountDay != 0 )
//     {
//         if(wAccountDay == 1 )
//             str = "您的付费将于今天到期！";
//         else
//             str = StringUtil::format("您的账户所剩余的天数：%d天",wAccountDay);
//     }
//     else if(wIPDay != 0 )
//     {
//         if(wIPDay == 1)
//             str = "当前IP的付费将于今天到期！";
//         else
//             str = StringUtil::format("当前IP所剩余的天数：%d天",wIPDay);
//     }
//     else if(wAccountHour != 0 )
//     {
//         str = StringUtil::format("你的账户所剩余的时间：%d小时",wAccountHour);
//     }
//     else if(wIPHour != 0 )
//     {
//         str = StringUtil::format("当前IP所剩余的时间：%d小时",wIPHour);
//     }
// 
//     if(str.size() > 0)
//     {
//         g_MsgBoxMgr.PopSimpleAlert(str.c_str());
//     }
// 
//     string buf;
//     buf.assign(msg + 12,iLen-12);
// 
//     string strIP;
//     int  iPort = 0;
//     UINT uSid  = 0;
//     int  i = 0;
// 
//     if(iLen < 128)
//     {
//         i = 0;
//         strIP = StringUtil::toStr(buf,i);
//         iPort = StringUtil::toInt(buf,i);
//         uSid  = StringUtil::toUInt(buf,i);
//     }
//     else
//     {
//         i = 0;
//         strIP = StringUtil::toStr(buf,i);
//         iPort = StringUtil::toInt(buf,i);
//         string strSid = StringUtil::toStr(buf,i);
// 
//         string strSession = g_pNet->NewDecode(strSid.c_str(),strSid.size());
//         string strTemp = g_pNet->NewDecode(msg + 12 + i,iLen - 12 - i);
// 
//         LPDYNCODE	lpDynCode_En = NULL;
//         LPDYNCODE	lpDynCode_De = NULL;
// 
//         int iStrTempLen = strTemp.length();
//         if(iStrTempLen < MAX_DATA_CRYBUFFER && iStrTempLen > 0)
//         {
//             LPVOID pDynCode = VirtualAlloc(NULL,MAX_DATA_CRYBUFFER,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
//             if(pDynCode)
//             {
//                 memset(pDynCode,0,20 * 1024);
//                 memcpy(pDynCode,strTemp.c_str(),iStrTempLen);
// 
//                 LPGETDYNCODE lpGetDynCode = NULL;
//                 TRY_BEGIN
//                     lpGetDynCode = (LPGETDYNCODE)pDynCode;
//                     __asm mov ebx,GetProcAddress
//                     __asm mov ecx,GetModuleHandle
//                     __asm mov edx,LoadLibrary
//                     lpDynCode_En = lpGetDynCode(1);
//                     lpDynCode_De = lpGetDynCode(2);
//                TRY_END
//             }
// 
//             if(lpDynCode_En && lpDynCode_De)
//             {
//                 char newBuffer[1024];
//                 TRY_BEGIN
//                     memcpy(newBuffer,strSession.c_str(),strSession.length());
//                     lpDynCode_De((LPBYTE)newBuffer,strSession.length());
//                 TRY_END
// 
//                 uSid = atol(newBuffer);
//             }
//             else
//             {
//                 OutputDebugString("玩家登陆加解密算法失败...\n");
//             }
// 
// 			if(pDynCode)
// 			{
// 				VirtualFree(pDynCode,0,MEM_RELEASE);
// 			}
//         }
//         else
//         {
//             OutputDebugString("玩家登陆加解密算法失败...\n");
//         }
// 
// 
//     }
// 
//     g_Login.SetServer(strIP.c_str(),iPort);
//     //g_Login.SetSessionID(uSid);
//     //g_Login.SetCharServer(strIP.c_str(),iPort);
// 
// 
//     g_pNet->Close(SERVER_GAME);
//     Sleep(1000);
//     g_pNet->SetServer(SERVER_GAME,strIP.c_str(),iPort);
//     g_pNet->Connect(SERVER_GAME);
// 	Sleep(100);
// 
// 	//把sessionid发给selgate
// 	SEND_SessionId_To_SelGate();
// 
// 
// 	if(g_Login.GetAutoLoginInType() > 0 && g_Login.GetAutoLoginInType() != 5)
// 	{
// 		g_pGameControl->SEND_Selchar_CharServer();
// 		return;
// 	}
// 	else
// 	{
// 		g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);
// 	}
// }


void CGameControl::MSG_Login_TuiGuang(const char * msg,int iLen)
{
    g_Login.SetTuiguang(true);
}


//动态加解密算法
void CGameControl::MSG_NEED_DYN_CRYPT(const char * msg, int iLen)
{
    if(msg[6] == 0)
    {
		if((msg[1] & 0x01) != 0)
		{
			g_pNet->SetDynCodeLen(*((DWORD *)(msg + 8)));
		}
		else
		{
			g_pNet->SetDynCodeLen(0);
		}
        SEND_DYN_CRYPT_OK(0);
    }
}
//重新请求动态加解密
void CGameControl::SEND_ReRequest_DynCode()
{
	SET_COMMAND(CS_RE_REQUEST_DYN_CRYPT,CMD_SIZE);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_DYN_CRYPT_OK(DWORD n)
{
    SET_COMMAND(CS_DYN_CRYPT_OK,CMD_SIZE);
    ASSIGN_ID(n);
    SEND_GAME_SERVER();
}
//大退
void CGameControl::SEND_Quit_Game(bool bSendQuit)
{
	g_SdSamplerMgr.SendPlayerActionSample();
	g_SdSamplerMgr.SendQualitySample();

	g_pSelf = &ORIGINALSELF;

	if(SELF.GetID() != 0)
	{
		//SEND_Login_Tongji2(true);

		if (bSendQuit)
		{
			SEND_Objects_Position();
		}

		g_pGameData->Clear();
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

		if(g_pSDOAInterface)
		{
			g_pSDOAInterface->Logout();

			g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);
			POINT pt;
			pt.x = g_pGfx->GetWidth() - 305; pt.y = 0;
			g_pSDOAInterface->SetTaskBarPosition(&pt);
		}

		SET_COMMAND(CS_REAL_QUIT_GAME,CMD_SIZE);
		ASSIGN_ID(g_Login.GetSessionID());
		SEND_GAME_SERVER();

		//if (bSendQuit)
		//{
		//	SEND_Logout_Ntf(SELF.GetName());
		//}
	}

   g_pNet->Reset();
   g_pNet->Close(0);
}

//bOpenSelCharWnd为true时是小退,为false是立即复活
void CGameControl::SEND_Out_Game(bool bOpenSelCharWnd)
{
	g_pSelf = &ORIGINALSELF;

	g_PetData.GetSendOutPetList().clear();

    g_pGameControl->SEND_Objects_Position(); //发送包裹位置
    if(bOpenSelCharWnd)
	{
		//SEND_Login_Tongji2(true);

		g_pGameData->Clear();

		// 小退时清除自动登录/体验服/浩方竞技状态，防止MSG_Login_Ack误走自动登录分支直接进入游戏
		g_Login.SetAutoLoginInType(0);
		g_Login.SetLoginInExpType(0);
		g_Login.SetInLoginInExp(false);
		g_Login.SetInHaoFangJingJi(false);

		// 小退时重置游戏流程标志，使角色服消息走登录流程分支而非游戏流程分支
		g_pNet->ResetGameProcFlag();

		if(g_pSDOAInterface)
		{
			//g_pSDOAInterface->Logout();

			g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);
			POINT pt;
			pt.x = g_pGfx->GetWidth() - 305; pt.y = 0;
			g_pSDOAInterface->SetTaskBarPosition(&pt);
		}
	}
// 	else//立即复活
// 	{
// 		if (g_TrusteeshipData.GetSelf())
// 		{
// 			g_TrusteeshipData.GetSelf()->ClearTaoistDataEx();
// 		}
// 	}

	g_OtherData.GetTimeOut().clear();
	g_AIMgr.Clear();

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

    //g_pWeather->EnableParticle(PARTICLE_NONE);
    g_AIAutoMgr.SetEnalbeWaiGua(false);


    //发送小退出信息
    SET_COMMAND(CS_QUIT_GAME,12);
    SEND_GAME_SERVER();

	// 断开游戏服，连接角色服
	g_pNet->Close(SERVER_GAME);
	Sleep(1000);
	g_pNet->SetServer(SERVER_GAME, g_Login.GetRoleGateIP(), g_Login.GetRoleGatePort());
	g_pNet->Connect(SERVER_GAME);
	Sleep(100);

	GCL_ClearMsgBuf(); //清掉之前的全部消息

	if(bOpenSelCharWnd)
	{
		g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);   
		g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);
	}

	char szPacket[64];
	wsprintf(szPacket, "%s/%d", g_Login.GetLoginID(), g_Login.GetSessionID());
	g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, 100, false);		//CM_QUERYCHR
}


//void  CGameControl::SEND_Login_Req_Public_Key() //向登陆服务器请求公钥
//{
//    g_dwReqPubKeyTime = GetTickCount(); //记录发送请求公钥的时刻
//    SET_COMMAND(CS_LOGIN_REQ_PUBLIC_KEY,12);
//	//告诉logingate新客户端支持动态加解密前移到logingate
//	ASSIGN_BYTE(0,1);
//    SEND_GAME_SERVER();
//}

//void  CGameControl::MSG_Login_Public_Key(const char* strMsg2,int iLen)
//{
//    g_dwReqPubKeyTime = 0;
//	g_Login.SetLoginGateType(strMsg2[0]);
//
//    int iKeyLen = *((WORD*)(strMsg2+6));
//
//    if(iLen - 12 < iKeyLen)
//        iKeyLen = iLen - 12;
//
//    g_RsaClient.Init(strMsg2+12,iKeyLen); //初始化RSA加密的封装类
//    g_Login.GetPubKey().assign(strMsg2+12,iKeyLen);
//}

//void  CGameControl::SEND_Login_Key_UserPwd(bool bPT,const char * strUser,const char *strPwd)
//{
//    g_Login.SetAccount(strUser,bPT);
//
//	//登录跨服战服务器,天绝魔域或从跨服战服务器,天绝魔域返回时用pt帐号
//	if(g_Login.GetAutoLoginInType() >= 1 && g_Login.GetAutoLoginInType() <= 5)
//	{
//		bPT = true;
//	}
//
//
//	if(strPwd)
//	{
//		string strSavePwd = g_pNet->NewEncode(strPwd,strlen(strPwd));
//		char strTemp[512]={0};
//		for(int i = 0;i < strSavePwd.length();i++)
//		{
//			strTemp[i] = strSavePwd.c_str()[i] ^ g_byPwdEncode;
//		}
//		g_Login.SetPassWord(strTemp);
//	}
//
//	if (bPT)
//		g_Login.SetLoginType(LOGINTYPE_PTACCOUNT);
//	else
//		g_Login.SetLoginType(LOGINTYPE_PTNUMID);
//
//    //拼装老的协议
//    char tmp[256] = {0};
//    if(bPT)
//    {
//        *((WORD *)(tmp+4)) = CS_UUID_LOGIN;
//        tmp[2] = strlen(strUser) + strlen(strPwd) + 1;
//        *((DWORD*)(tmp + 6)) = GAME_VERSION;
//        sprintf(tmp + 12,"%s/%s",strUser,strPwd);
//    }
//    else
//    {
//        tmp[0] = 1;
//        tmp[3] = strlen(strUser) + 1;
//        *((WORD *)(tmp+4)) = CS_LOGIN_USERPWD;
//        *((DWORD*)(tmp + 6)) = GAME_VERSION;
//        sprintf(tmp + 12,"%s/%s",strUser,strPwd);
//        g_Login.SetLoginID(strUser);
//    }
//
//    g_Login.GetAutoPassword().clear();
//    int iLen = CMD_SIZE+strlen(tmp+CMD_SIZE);
//
//    SET_COMMAND(0,256);
//
//    if(g_Login.GetPubKey().size() != 0)
//    {
//        //RSA加密协议体内容
//        ASSIGN_WORD(6,iLen);
//        ASSIGN_WORD(4,CS_LOGIN_KEY_USERPWD);
//        ASSIGN_BYTE(11,1);//高速版客户端标志
//
//        int iEncryptLen = g_RsaClient.PubEncrypt((BYTE*)tmp,iLen,(BYTE*)(temp+12)); //公共的钥匙加密
//        iBLen = iEncryptLen + CMD_SIZE;
//    }
//    else
//    {
//        tmp[11] = 1;//高速版客户端标志
//        memcpy(temp,tmp,iLen);
//        iBLen = iLen;
//    }
//    SEND_GAME_SERVER();
//}

//////////////////////////////////////////////////////////////////////////

void CGameControl::MSG_GroupList_Ntf(const char * msg,int iLen)
{
    PKG_LG_CLI_GroupList_NTF* pNtf = (PKG_LG_CLI_GroupList_NTF*)msg;

	g_Login.GetGroupList().clear();

	//不知道为什么,有极少的客户端一直无法收到窗口的FD_CONNECT消息,无双开区就有5,6个这要的玩家,但实际上这些客户端已经连上了logingate
	if (g_pNet && !g_pNet->IsNeed())
	{
		g_pNet->DealOnConnect(0);
	}

	//设置Group的信息
	_GroupInfo groupInfo;
	//for(int i = 0;i < pNtf->byCount;i++)//组名 + "\0" + 显示名 + '\0'
	for(int i = 0;i < 1;i++)
	{
		groupInfo.strName = (char *)(pNtf->astNameList[i].abyName);//登录用
		groupInfo.strName_Show = groupInfo.strName;//显示用
		
		if (groupInfo.strName.size() < pNtf->astNameList[i].wNameLen - 1)
		{
			char *p = (char *)(pNtf->astNameList[i].abyName) + groupInfo.strName.size() + 1;
			groupInfo.strName_Show.assign(p,pNtf->astNameList[i].wNameLen - groupInfo.strName.size() - 1);
		}

		//传世测试
		groupInfo.strName = msg;
		groupInfo.strName_Show.assign(msg);
		

		output_debug("MSG_GroupList_Ntf:%s,%s\r\n",groupInfo.strName.c_str(),groupInfo.strName_Show.c_str());

		g_Login.GetGroupList().push_back(groupInfo);
	}

	g_pControl->Msg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);

	if(g_Login.GetAutoLoginInType() > 0 || !g_strChannelName.empty() || g_Login.GetLoginInExpType() > 0)//从平台登录一个区就是不一个组,不要让玩家看到玩组界面
	{
		//g_pGameControl->SEND_Login_ServerName();					
		g_pGameControl->SEND_SelGroup_Req(g_Login.GetGroupName());
		return;
	}
	else
	{
		g_pControl->Msg(MSG_CTRL_SELECTSERVERWND,OPER_CREATE);
		//g_pControl->Msg(MSG_CTRL_ACTIVITYLOG_WND,OPER_CREATE);
	}
}

void CGameControl::MSG_DlServerIP_Ntf(const char * msg,int iLen)
{
	PKG_LG_CLI_SS_INFO* pAck = (PKG_LG_CLI_SS_INFO*)msg;
	if (PROTO_SUCCESS == pAck->byResult)
	{       
		struct in_addr saddr;
		SIPAddr& ggAddr = pAck->stReply.stSucc;
		saddr.S_un.S_addr = ggAddr.dwIP;
		std::string ggSvrIP = ::inet_ntoa(saddr);

		output_debug("SourceServer ip:%s,port:%d",ggSvrIP.c_str(),ggAddr.wPort);

		if (!g_ReplayManager.IsInReplay())
		{
			int iPort = g_pStreamMgr->GetConfigInt("DownloadServerPort",0);
			if (g_pDownLoadNet && iPort == 0 && !g_pDownLoadNet->IsConnected())//如果配置了指定IP和端口(iPort != 0),连接到配置指定的IP
			{
				g_pDownLoadNet->Close(0);
				g_pDownLoadNet->ResetLastConnectTime();
				g_pDownLoadNet->SetServer(0,ggSvrIP.c_str(),ggAddr.wPort);
				//g_pDownLoadNet->Connect(0);//放到第一次请求资源的时候连接
			}
			if (g_pBgDownLoadNet && iPort == 0 && !g_pBgDownLoadNet->IsConnected())//如果配置了指定IP和端口(iPort != 0),连接到配置指定的IP
			{
				g_pBgDownLoadNet->Close(0);
				g_pBgDownLoadNet->ResetLastConnectTime();
				g_pBgDownLoadNet->SetServer(0,ggSvrIP.c_str(),ggAddr.wPort);
				//g_pBgDownLoadNet->Connect(0);//放到第一次请求资源的时候连接
			}
			if (g_pDownloadLimitSpeed && iPort == 0 && !g_pDownloadLimitSpeed->IsConnected())//如果配置了指定IP和端口(iPort != 0),连接到配置指定的IP
			{
				g_pDownloadLimitSpeed->Close(0);
				g_pDownloadLimitSpeed->ResetLastConnectTime();
				g_pDownloadLimitSpeed->SetServer(0,ggSvrIP.c_str(),ggAddr.wPort);
				g_pDownloadLimitSpeed->Connect(0);
				g_pDownloadLimitSpeed->SendDownLoadFile("plist.txt",true);
			}
		}

	}
}

//服务器选择 不返回服务器列表,直接使用传入的服务器IP,返回登录界面
void CGameControl::SEND_SelGroup_Req(const char * groupName)
{
	/*
    PKG_CLI_LG_SelGroup_REQ stReq;
    strncpy(stReq.szName, groupName, sizeof(stReq.szName));

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, 104);
	*/
	g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CREATE);
	g_pControl->PushMsg(MSG_CTRL_SELECTSERVERWND,OPER_CLOSE);
}

//链接角色游戏网关 192.168.1.6/7100/3
void CGameControl::MSG_SelGroup_Ack(const char * msg, int iLen)
{
	//传世角色网关处理
	if(iLen > 0)
	{
		string txt; 
		txt.append(msg, iLen);
		VString roleGate = StringUtil::split(txt, "/");

		g_Login.SetSessionID(StringUtil::toInt(roleGate.at(2)));

		//断开当前的账号网关
		g_Login.SetServer(roleGate.at(0).c_str(),StringUtil::toInt(roleGate.at(1)));
		g_Login.SetRoleGate(roleGate.at(0).c_str(),StringUtil::toInt(roleGate.at(1)));	//保存角色服地址，小退时用
		g_pNet->Close(SERVER_GAME);
		Sleep(1000);
		//连接到角色网关
		g_pNet->SetServer(SERVER_GAME,roleGate.at(0).c_str(),StringUtil::toInt(roleGate.at(1)));
		g_pNet->Connect(SERVER_GAME);
		Sleep(100);

		//打开角色列表
		g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);   
		g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);

		//请求角色列表
		char szPacket[64];
		wsprintf(szPacket, "%s/%d", g_Login.GetLoginID(), g_Login.GetSessionID());
		g_pNet->SendBuf(SERVER_GAME, (char*)&szPacket, 0, 100, false);		//CM_QUERYCHR

		return;
	}

    PKG_LG_CLI_SelGroup_ACK* pAck = (PKG_LG_CLI_SelGroup_ACK*)msg;
    if (PROTO_SUCCESS == pAck->byResult)
    {       
        struct in_addr saddr;
        SIPAddr& ggAddr = pAck->stReply.stSucc;
        saddr.S_un.S_addr = ggAddr.dwIP;
        std::string ggSvrIP = ::inet_ntoa(saddr);
        
		g_Login.SetServer(ggSvrIP.c_str(),ggAddr.wPort);
		g_Login.SetRoleGate(ggSvrIP.c_str(),ggAddr.wPort);	//保存角色服地址，小退时用

		g_pNet->Close(SERVER_GAME);
		Sleep(1000);
		g_pNet->SetServer(SERVER_GAME,ggSvrIP.c_str(),ggAddr.wPort);
		g_pNet->Connect(SERVER_GAME);
		Sleep(100);

        SEND_DynCode_Req();

		g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CREATE);
		g_pControl->PushMsg(MSG_CTRL_SELECTSERVERWND,OPER_CLOSE);
		
    }
    else
    {
		g_MsgBoxMgr.PopSimpleAlert(SDGetErrDesc(pAck->stReply.stFail.wErrCode),MSG_RECONNECT_SERVER);
    }

	//如果服务器这个时候都没有发来sourceserver的ip和端口,把下载net释放掉
	if (g_pDownLoadNet && g_pDownLoadNet->GetPort() == 0)
	{
		SAFE_DELETE(g_pDownLoadNet);
		if (g_pStreamMgr)
		{
			g_pStreamMgr->SetNeedDownloadFileFromServer(false);
			g_pStreamMgr->ClearDownloadingFilesList();
		}
	}
	if (g_pBgDownLoadNet && g_pBgDownLoadNet->GetPort() == 0)
	{
		SAFE_DELETE(g_pBgDownLoadNet);
	}

	if (g_pDownloadLimitSpeed && g_pDownloadLimitSpeed->GetPort() == 0)
	{
		SAFE_DELETE(g_pDownloadLimitSpeed);
	}
}
//发给oa的区组信息,收到这条协议就表示gs支持oa登录
void CGameControl::MSG_GroupInfo_NTF(const char * msg,int iLen)
{
	PKG_GG_CLI_GroupInfo_NTF* pAck = (PKG_GG_CLI_GroupInfo_NTF*)msg;

	if (g_pMagicCtrl && g_pMagicCtrl->GetMagicRoot(MAGICID_ENABLE_SDOA))
	{
		g_Login.SetSDOALogin(true);
	}
	else
	{
		g_Login.SetSDOALogin(false);
	}

	if (g_Login.IsSDOALogin())
	{
		g_Login.SetInnerAreaNo(pAck->wAreaId);
		g_Login.SetInnerGroupNo(pAck->wGroupId);
		g_pGfx->SetRenderIGW(true);
	}

#ifdef _DEBUG
	g_Login.SetSDOALogin(false);
	g_pGfx->SetRenderIGW(false);
#endif

	if(!g_pSDOAInterface && g_pMagicCtrl->GetMagicRoot(MAGICID_SHOW_IGW) 
		&& g_strChannelName.empty() && (g_Login.GetLoginInExpType() == 0) )
	{
		g_pSDOAInterface = new CSDOAInterface;
		if(g_pSDOAInterface->OpenIGW())
		{
			if (g_pSDOAInterface->GetSDOADx9())
			    g_pSDOAInterface->GetSDOADx9()->Initialize(g_pGfx->GetD3DDev(),g_pGfx->GetD3DPP(),FALSE);
		}
		else
		{
			SAFE_DELETE(g_pSDOAInterface);
		}
	}

	if (g_pSDOAInterface)
	{
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


		g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);
		g_pSDOAInterface->SetLogicScreen(g_pGfx->GetWidth(),g_pGfx->GetHeight());

		POINT pt;
		pt.x = g_pGfx->GetWidth() - 40; pt.y = 168;
		g_pSDOAInterface->SetTaskBarPosition(&pt);	


		g_pSDOAInterface->SetShowUserLoginDialog(FALSE);			
		g_pControl->PushMsg(MSG_CTRL_SELECTSERVERWND,OPER_CLOSE);
		g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);
		g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CREATE);

	}
	else
	{
		g_Login.SetSDOALogin(false);
	}

}
void CGameControl::SEND_DynCode_Req()
{
    PKG_CLI_GG_DynCode_REQ stReq;

	BYTE byClientType = (unsigned char)g_pStreamMgr->GetOtherConfigInt("SubType",0);//SubType:0为普通客户端,1为小客户端,;
    //logingate 以及gs对于stReq.byClientType = 1表示是原来群英传flash的客户端,里面有不同的处理,这里避免stReq.byClientType == 1
	if (byClientType + 100 > 255)
    {
		byClientType = 255;
    }
	else
	{
		byClientType += 100;
	}
	stReq.byClientType = byClientType;
    
    g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_DynCode_REQ);
}

//账号密码验证
void CGameControl::SEND_PTLogin_Req(const char * name, const char * passWd)
{
	/*
    PKG_CLI_GG_Login_REQ stReq;

	if (strstr(name,"@") != NULL)
	{
		stReq.byType = LOGIN_MAIL;
		memset(stReq.stInfo.stMailLoginReq.szMailID, 0,MAIL_LEN);
		memset(stReq.stInfo.stMailLoginReq.szPasswd, 0, PASSWORD_LEN);
		strcpy(stReq.stInfo.stMailLoginReq.szMailID, name);
		strcpy(stReq.stInfo.stMailLoginReq.szPasswd, passWd);
		stReq.stInfo.stMailLoginReq.dwClientVersion = GAME_VERSION;
	}
	else
	{
		stReq.byType = LOGIN_PTSDK;
		memset(stReq.stInfo.stPTLoginReq.szPTID, 0,PTID_LEN);
		memset(stReq.stInfo.stPTLoginReq.szPasswd, 0, PASSWORD_LEN);
		strcpy(stReq.stInfo.stPTLoginReq.szPTID, name);
		strcpy(stReq.stInfo.stPTLoginReq.szPasswd, passWd);
		stReq.stInfo.stPTLoginReq.dwClientVersion = GAME_VERSION;
	}

	g_Login.SetLoginID(name);
	if(passWd)
	{
		string strSavePwd = g_pNet->NewEncode(passWd,strlen(passWd));
		char strTemp[512]={0};
		for(int i = 0;i < strSavePwd.length();i++)
		{
			strTemp[i] = strSavePwd.c_str()[i] ^ g_byPwdEncode;
		}
		g_Login.SetPassWord(strTemp);
	}*/

	//传世封包
	char				szPacket[64];
	memset(szPacket, 0 , 64);
	wsprintf(szPacket, "%s/%s", name, passWd);
    g_pNet->SendBuf(SERVER_GAME, szPacket, 0, CM_IDPASSWORD, false);
}

void CGameControl::SEND_OALogin_Req(const char * token,const char * loginid)
{
    PKG_CLI_GG_Login_REQ stReq;
    stReq.byType = LOGIN_SDOA;
	memset(stReq.stInfo.stSDOALoginReq.abyToken,0,TOKEN_LEN);
	memset(stReq.stInfo.stSDOALoginReq.szDigitID,0,PTID_LEN);

    strcpy((char*)stReq.stInfo.stSDOALoginReq.abyToken, token);
	strcpy(stReq.stInfo.stSDOALoginReq.szDigitID,loginid);

    stReq.stInfo.stSDOALoginReq.dwClientVersion = GAME_VERSION;
	stReq.stInfo.stSDOALoginReq.wTokenLen = strlen(token);

    g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Login_REQ);
}

void CGameControl::SEND_APLogin_Req(const char* sendtype, const char * auth)
{
	PKG_CLI_GG_Login_REQ stReq;
	stReq.byType = LOGIN_AP;
	memset(stReq.stInfo.stAPLoginReq.szAuthAct, 0, sizeof(stReq.stInfo.stAPLoginReq.szAuthAct));
	memset(stReq.stInfo.stAPLoginReq.szAuthStr, 0, AP_AUTH_STR_LEN);

	strcpy(stReq.stInfo.stAPLoginReq.szAuthAct, sendtype);
	strcpy(stReq.stInfo.stAPLoginReq.szAuthStr, auth);
	stReq.stInfo.stAPLoginReq.dwClientVersion = GAME_VERSION;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Login_REQ);

	output_debug("SEND_APLogin_Req %s:%s\n",sendtype,auth);
}

void CGameControl::SEND_EXPLogin_Req()
{
	PKG_CLI_GG_Login_REQ stReq;
	stReq.byType = LOGIN_EXP;
	
	stReq.stInfo.stEXPLoginReq.dwClientVersion = GAME_VERSION;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Login_REQ);

	output_debug("SEND_EXPLogin_Req \n");
}

void CGameControl::SEND_SGILogin_Req(const char * code)
{
	PKG_CLI_GG_Login_REQ stReq;
	memset(stReq.stInfo.stSGILoginReq.szPlatform, 0, sizeof(stReq.stInfo.stSGILoginReq.szPlatform));
	memset(stReq.stInfo.stSGILoginReq.szAuthCode, 0, sizeof(stReq.stInfo.stSGILoginReq.szAuthCode));

	stReq.byType = LOGIN_SGI;

	strcpy(stReq.stInfo.stSGILoginReq.szPlatform, "sd");
	strcpy(stReq.stInfo.stSGILoginReq.szAuthCode, code);
	stReq.stInfo.stSGILoginReq.dwClientVersion = GAME_VERSION;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_Login_REQ);

	output_debug("SEND_SGILogin_Req: %s\n",stReq.stInfo.stSGILoginReq.szAuthCode);
}


void CGameControl::SEND_MergePT_Req(const char * account, const char * passWd)
{
	PKG_CLI_GG_MERGE_PT_REQ stReq;

	memset(stReq.szAccount, 0, sizeof(stReq.szAccount));
	memset(stReq.szPasswd, 0, sizeof(stReq.szPasswd));

	strcpy(stReq.szAccount, account);
	strcpy(stReq.szPasswd, passWd);
	stReq.dwClientVersion = GAME_VERSION;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_MERGE_PT_REQ);

	output_debug("SEND_MergePT_Req: %s,%s \n", stReq.szAccount, stReq.szPasswd);
}

void CGameControl::MSG_MergePT_Ack(const char * msg,int iLen)
{
	PKG_CLI_GG_MERGE_PT_ACK* pAck = (PKG_CLI_GG_MERGE_PT_ACK*)msg;

	output_debug("MSG_MergePT_Ack %d\n",pAck->nResult);

	if (pAck->nResult == PROTO_SUCCESS)
	{
		g_Login.SetLoginInExpType(0);
		g_Login.SetInLoginInExp(false);

		g_pControl->PopupWindow(MSG_CTRL_BINDPTWND_WND,OPER_CLOSE);
		g_MsgBoxMgr.PopSimpleAlert("绑定成功");
	}
	else
	{
		g_pControl->MsgToWnd("BindPTWnd", MSG_CTRL_BINDPTWND_WND, 10);

		string url = g_pStreamMgr->GetWebsite("NewUser");
		string str = "                        错  误\\";
		str += "您输入的盛大通行证或密码有误，请重新输入或<注册新的盛大通行证/@@OpenUrl " + url + ">。";	
		g_MsgBoxMgr.PopTagAlert(str.c_str());
	}
	
}

//EC_TYPE_Ekey_Common   = 0,
//EC_TYPE_Ekey_Random   = 1,
//EC_TYPE_Ekey_Usb_auto = 2,
//EC_TYPE_Ekey_Usb_manu = 3,
//EC_TYPT_ECard_Mobile  = 4,
//EC_TYPE_ECard_Common  = 5,
//EC_TYPE_Reserve       = 6,
//EC_TYPE_ECardno       = 7,
void CGameControl::MSG_Challenge_Req(const char * msg,int iLen)
{
    PKG_GG_CLI_Challenge_REQ* pReq = (PKG_GG_CLI_Challenge_REQ*)msg;

	string txt;
	if(pReq->byType == 0 || pReq->byType == 1)
	{
		int iChallengeLen = 0;
		if (pReq->byType == 0)
		{
			txt = "请输入您密宝的";
			iChallengeLen = strlen(pReq->szChallenge);

			for(int i = 0; i < iChallengeLen; i++)
			{
				txt += "第";
				txt += pReq->szChallenge[i];
				txt += "位，";
			}
			txt.erase(txt.size() - 2);
			txt += "：";
		}
		else
		{
			txt = "请根据随机数“";
			txt += pReq->szChallenge;
			txt += "”生成动态密码并输入：";
		}

		g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,0);
		g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)txt.c_str());

		if (pReq->byType == 0)
		{
			g_pControl->MsgToWnd(MSG_CTRL_INPUT_MIBAO_WND,MSG_CTRL_INPUT_MIBAO_WND,4,(CControl*)(iChallengeLen));
		}

	}
	else if(pReq->byType == 2 || pReq->byType == 3)
	{

	}
	else if(pReq->byType == 4)//小安全卡
	{
		g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,2);
		g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)(pReq->szChallenge));
		
	}
	else if(pReq->byType == 5)//大安全卡
	{
		g_pControl->PopupWindow(MSG_CTRL_INPUT_MIBAO_WND,OPER_CREATE,1);
		g_pControl->Msg(MSG_CTRL_INPUT_MIBAO_WND,1,(CControl*)(pReq->szChallenge));
	}
	else if(pReq->byType == 6 || pReq->byType == 7)//秘宝预留口令,安全卡预留口令
	{
		g_pControl->PopupWindow(MSG_CTRL_SOFT_KEYBOARD_CREATE,OPER_CLOSE);

		if(pReq->byType == 6)	//秘宝预留口令
		{
			g_pControl->PopupWindow(MSG_CTRL_RESERVEPASSWORD_WND,OPER_UPDATE,0);
		}
		else//安全卡预留口令
		{
			g_pControl->PopupWindow(MSG_CTRL_RESERVEPASSWORD_WND,OPER_UPDATE,1);
		}
	}
}

void CGameControl::SEND_Challenge_Ack(const char * challenge)
{
    PKG_CLI_GG_Challenge_ACK stAck;
    stAck.byType = 0;
	sprintf(stAck.szDPWD,"%s",challenge);

	if (g_Login.GetLoginType() == LOGINTYPE_PTACCOUNT)
		g_Login.SetLoginType(LOGINTYPE_PTACCOUNTWITHEKEY);

    g_pNet->SendBuf(SERVER_GAME, (char*)&stAck, 0, CLI_GG_Challenge_ACK);
}

void CGameControl::SEND_VerifyStoreEkey_Ack(const char * challenge)
{
	PKG_CLI_GG_VerifyStoreEkey_ACK stAck;
	sprintf(stAck.szRoleName,"%s",SELF.GetName());
	sprintf(stAck.szPTID,"%s",g_Login.GetLoginID());
	sprintf(stAck.szDPWD,"%s",challenge);

	g_pNet->SendBuf(SERVER_GAME, (char*)&stAck, 0, CLI_GG_VerifyStoreEkey_ACK);
}

void CGameControl::SEND_OfflineTrustee(BYTE byType)
{
	tagPKG_CLI_GG_OfflineTrustee_NTF stAck;
	stAck.byTrusteeMode = byType;
	g_pNet->SendBuf(SERVER_GAME, (char*)&stAck, 0, CLI_GG_OfflineTrustee_NTF);
}

void CGameControl::MSG_Login_Error_Ack(const char * msg,int iLen)
{
	g_pControl->Msg(MSG_CTRL_ENABLELOGINWND,0);
	g_MsgBoxMgr.PopSimpleAlert("登录失败,账号或密码错误! "); 
}

//传世选择服务器,自动返回选择服务器
void CGameControl::MSG_Login_Succ(const char * msg,int iLen)
{
	string txt = g_Login.GetGroupName(); // 获取选择的服务器组名
	// 设置玩家登录成功的登录ID
	g_Login.SetLoginID(msg+1);
	//txt.append(msg, iLen);
	// 告诉服务端寻找选择角色服务器
	g_pNet->SendBuf(SERVER_GAME, (char*)txt.c_str(), 0, 104, false);
}

//传世方法,返回成功,直接进入角色相关界面, 到这里表已经登陆成功了
void CGameControl::MSG_Login_Ack(const char * msg,int iLen)
{
	//for 传世 iLen 长度为0即没有角色, 进入角色创建界面
	//unsigned long long qwDigitID = 0;
	//LoginData        *stLoginData = NULL;
	SRoleList        *stRoleList = NULL;
	VString roleList;

	bool success = false;

	g_Login.SetThirdChannel("");
	g_Login.SetThirdUserID("");

	//char poptang_str[66] = {0};
	//sprintf(poptang_str,"%I64d",qwDigitID);
	//g_Login.SetPoptang_str(poptang_str);

	g_pControl->Msg(MSG_CTRL_CREATECHARWND,OPER_CLOSE);   
	g_pControl->Msg(MSG_CTRL_SELECTCHARWND,OPER_CREATE);

	// 解析角色列表
	//*11WAD/0/225/1/0/ 5
	//s40:=s40 + sChrName + '/' + sJob + '/' + sHair + '/' + sLevel + '/' + IntToStr(btSex) + '/'+inttostr(ChrRecord.data.wTRJJ)+'/'+sTrfl+'/';
	//mysql 版本 s40 := sChrName + '/' + sJob + '/' + sHair + '/' + sLevel + '/' + IntToStr(btSex) + '/2014-6-30/0/';
	roleList = StringUtil::split(msg, "/");
	stRoleList = new SRoleList();
	stRoleList->byRoleCount = roleList.size() / 7;
	for (int i =0; i < stRoleList->byRoleCount; i ++)
	{
		int index = i * 7;
		strcpy(stRoleList->astRoleList[i].szRoleName, roleList[index].c_str());  //lzhez 修正只能显示一个角色
		stRoleList->astRoleList[i].byJob = StringUtil::toInt(roleList[index+1]);
		stRoleList->astRoleList[i].byHair = StringUtil::toInt(roleList[index+2]);
		stRoleList->astRoleList[i].wLevel = StringUtil::toInt(roleList[index+3]);
		stRoleList->astRoleList[i].bySex = StringUtil::toInt(roleList[index+4]);
		strcpy(stRoleList->astRoleList[i].szDeleteTime, roleList[index+5].c_str());
		stRoleList->astRoleList[i].byDelete = StringUtil::toInt(roleList[index+6]);
	}
	g_OtherData.SetCharNo(stRoleList->bySelect);

	//自动进入游戏
	if(g_OtherData.IsAutoEnter())
	{
		//SEND_Selchar_SelectChar();
		SEND_SelRole_Req(g_Login.GetLoginID(),g_OtherData.GetCharName());
		g_OtherData.SetAutoEnter(false);
		return;
	}

	g_OtherData.GetCharList().clear(); //清除之前的数据
	g_OtherData.GetDelCharList().clear();
	g_OtherData.SetReceivedCharList(true); //表示已经接受到数据了

	for(int i = 0; i < stRoleList->byRoleCount; i++)
	{
		_CharInfo info;
		memcpy(&info,&(stRoleList->astRoleList[i]),sizeof(info));
		if (info.byDelete == 1)	//传世,这个可能不对
		{
			g_OtherData.GetDelCharList().push_back(info);
		}
		else
		{
			g_OtherData.GetCharList().push_back(info);
		}
	}


	if (g_Login.GetLoginInExpType() > 0)
	{
		g_Login.SetInLoginInExp(true);
	}
	else if(g_Login.GetAutoLoginInType() > 0 && g_Login.GetAutoLoginInType() <= 4)//自动登录
	{
		if(g_OtherData.GetCharList().size() == 0)
		{
			g_MsgBoxMgr.PopSimpleAlert("没有角色!");
			return;
		}
		Sleep(20);
		//g_pGameControl->SEND_Selchar_SelectChar();
		SEND_SelRole_Req(g_Login.GetLoginID(),g_OtherData.GetCharName());
		return;
	}
	else if (g_Login.GetAutoLoginInType() == 5)//浩方竞技,只登录到选角色界面
	{
		g_Login.SetAutoLoginInType(0);
		g_Login.SetInHaoFangJingJi(true);
	}		

	if (iLen <= 0)	//小于等于0 说明没有创建角色，第一次登陆，弹自由对战窗口
	{
		g_pControl->Msg(MSG_CTRL_LICENSE2WND_WND, OPER_RECREATE);//弹窗口操作
	}

	WIN32_FIND_DATA ffd;
	string strFileName = GetGameDataDir();
	strFileName += "\\license.txt";

	HANDLE h = FindFirstFile(strFileName.c_str(),&ffd);  
	if(h != INVALID_HANDLE_VALUE)   
	{ 
		FindClose(h);  

		char strTemp[32]={0};
		sprintf(strTemp,"%u%u",ffd.ftLastWriteTime.dwHighDateTime,ffd.ftLastWriteTime.dwLowDateTime);

		strFileName = GetGameDataDir();
		strFileName += "\\config\\LoginIdCfg.ini";

		char szReturn[32] = {0};
		GetPrivateProfileString("LicenseTime",g_Login.GetLoginID(),"0",szReturn,32,strFileName.c_str());
		if(strcmp(strTemp,szReturn) != 0)
		{
			//WritePrivateProfileString("LicenseTime",g_Login.GetAccount(),strTemp,strFileName.c_str());

			g_pControl->Msg(MSG_CTRL_LICENSEWND_WND,OPER_CREATE);
			//g_pControl->MsgToWnd(MSG_CTRL_LICENSEWND_WND,MSG_CTRL_LICENSEWND_WND,1,(CControl*)(szMsg.c_str()));
		}
	}
	//output_debug("end SDError::RuntimeData");
}


void CGameControl::SENDNeedAuthen_Ack()
{
	output_debug("SENDNeedAuthen_Ack");
	PKG_CLI_GG_NeedAuthen_ACK stReq;

	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_NeedAuthen_ACK);

	g_pControl->PopupWindow(MSG_CTRL_AUTHEN_WND,OPER_CLOSE);
}

void CGameControl::MSG_NeedAuthen_Req(const char * msg,int iLen)
{
	output_debug("MSG_NeedAuthen_Req");

	PKG_GG_CLI_NeedAuthen_REQ* pAck = (PKG_GG_CLI_NeedAuthen_REQ*)msg;
	if (!pAck->szAuthenURL)
	{
		return;
	}
	g_WidgetMgr.SetAuthenURL(pAck->szAuthenURL);

	output_debug(pAck->szAuthenURL);

	g_pControl->Msg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_AUTHEN_WND,OPER_RECREATE);
}

void CGameControl::Msg_Login_Check_Mask(const char* msg,int iLen)
{
	tagPKG_GG_CLI_LoginVerify_REQ* pAck = (tagPKG_GG_CLI_LoginVerify_REQ*)msg;

	if (!pAck->szVerifyURL)
	{
		return;
	}

	string str;
	str.assign(pAck->szVerifyURL);
	g_pGameData->SetLoginCheckMask(str.c_str());

	if (g_pControl->FindWindowByName("LoginCheckMaskWnd"))
		g_pControl->MsgToWnd("LoginCheckMaskWnd",MSG_CTRL_LOGIN_CHECK_MASK_UPDATE);
	else g_pControl->PopupWindow(MSG_CTRL_LOGIN_CHECK_MASK_WND,OPER_CREATE);
}

void CGameControl::Send_Login_Check_Mask(const char* pData)
{
	PKG_CLI_GG_LoginVerify_ACK stReq;

	sprintf(stReq.szAnswer,"%s",pData);

	if (strlen(stReq.szAnswer) <= 0)
	{
		return;
	}

	g_pControl->PopupWindow(MSG_CTRL_LOGIN_CHECK_MASK_WND,OPER_CLOSE);
	g_pNet->SendBuf(SERVER_GAME, (char*)&stReq, 0, CLI_GG_LoginVerify_ACK);
}
