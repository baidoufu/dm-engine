#include "Net.h"

#include "GameControl/GameControl.h"
#include "Global/DebugTry.h"
#include "Global/PerfCheck.h"
#include "GameData/GameGlobal.h"
#include "Global/StringUtil.h"
#include "GameData/GameData.h"
#include "GameData/TalkMgr.h"
#include "GameClient/ReplayManager.h"

#include "NetData.h"
#include "cligsproto.h"
#include "cliggproto.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

#include <fstream>

#ifdef NETWORK_HOOK
typedef int (__stdcall * lpfnMirRecv) (int ,const char *,int,int);
typedef int (__stdcall * lpfnMirSend) (int ,const char *,int,int);

HMODULE hdll = NULL;			//初始化
lpfnMirRecv mirRecv = NULL;
lpfnMirSend mirSend = NULL;

#endif


CCliGgProto CGameNet::m_soGgProto;
CCliGsProto CGameNet::m_soGsProto;

CGameNet::CGameNet(int iMaxServer,HWND hWnd,DWORD dwMsg,long lEvent)
{
	ZeroMemory(m_cBuf,DATA_BUFSIZE);
	Init(iMaxServer,hWnd,dwMsg,lEvent);

	//m_bNewDynDec = false; //传世不加密
    m_bNewDynDec = true;
	m_bIsGS = false;
	SysCount = 1;

	//被微操及附身对象可以发的协议
	m_bTrusteeshipAllowProtocol[CS_PLAYER_TURN] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_WALK] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_SQUAT] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_RUN] = 0;
	m_bTrusteeshipAllowProtocol[CS_PACKAGE_OBJECT_REJECT] = 0;
	m_bTrusteeshipAllowProtocol[CS_PACKAGE_OBJECT_PICKUP] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_NORMAL_ATTACK] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_FORCE_ATTACK] = 0;	
	m_bTrusteeshipAllowProtocol[CS_PLAYER_DIG] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_MAGIC] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_KILL] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_STICK] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_STOPGO] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_SALUTE] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_MOON] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_FIRE] = 0;
	m_bTrusteeshipAllowProtocol[CS_PLAYER_ATTACK_NEW_FENCE] = 0;
	m_bTrusteeshipAllowProtocol[CS_PACKAGE_OBJECT_USE] = 0;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_GOOD_EXCHANGE] = 0;
	m_bTrusteeshipAllowProtocol[CS_RIDE_HORSE] = 0;
	m_bTrusteeshipAllowProtocol[CS_TRAIN_HORSE] = 0;
	m_bTrusteeshipAllowProtocol[CS_MICROCONTROL_SENDPROTOCOL] = 0;
	m_bTrusteeshipAllowProtocol[CS_RTSINSTANCE_EXCHANGE_BYRESOURCE] = 0;
	m_bTrusteeshipAllowProtocol[CS_WUXING_APPEAR_POINT] = 0;

	//微操及附身时通过原主体能发的协议
	m_bTrusteeshipAllowProtocol[CS_PT_STORE_INFO] = 1;
	m_bTrusteeshipAllowProtocol[CS_GROUP_QUERY] = 1;
	m_bTrusteeshipAllowProtocol[CS_REQUEST_SERVER_TIME] = 1;
	m_bTrusteeshipAllowProtocol[CS_OPEN_MAPVIEW] = 1;
	m_bTrusteeshipAllowProtocol[CS_LOOK_PLAYER_INFO] = 1;
	m_bTrusteeshipAllowProtocol[CS_QUIT_GAME] = 1;
	m_bTrusteeshipAllowProtocol[CS_REAL_QUIT_GAME] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_MODE] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_HEAD_TARGET] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_INVITE_ASK] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_DATA_EXCHANGE] = 1;
	m_bTrusteeshipAllowProtocol[CS_TRUSTEESHIP_HPMP] = 1;
	m_bTrusteeshipAllowProtocol[CS_MICROCONTROL_SELECT] = 1;
	m_bTrusteeshipAllowProtocol[CS_OBJECTS_POSITION] = 1;
	m_bTrusteeshipAllowProtocol[CS_GUILD_ALLY] = 1; // == CS_MESSAGE_PRIVATE,== CS_MESSAGE_SEND
	m_bTrusteeshipAllowProtocol[CS_SHORTCUT_KEY_CHANGE] = 1;
	m_bTrusteeshipAllowProtocol[CS_WARM_HEART_PROMPT_INFO] = 1;
	m_bTrusteeshipAllowProtocol[CS_INSTANCEZONE_POPUP] = 1;
	//m_bTrusteeshipAllowProtocol[CS_MESSAGE_SEND] = 1;
	//m_bTrusteeshipAllowProtocol[CS_MESSAGE_PRIVATE] = 1;



#ifdef NETWORK_HOOK
	hdll = LoadLibrary("clientmirm.dll");
	if( hdll )
	{
		mirRecv = (lpfnMirRecv) GetProcAddress(hdll,"MirRecv");
		mirSend = (lpfnMirSend) GetProcAddress(hdll,"MirSend");
	}
#endif
}

CGameNet::~CGameNet()
{

}

void CGameNet::CloseDynCode()
{
    output_debug("CloseDynCode\n");

    if (NULL != m_pDynCode)
    {
        VirtualFree(m_pDynCode,0,MEM_RELEASE);
		m_pDynCode = NULL;
   }
    m_lpDynCode_De = NULL;
    m_lpDynCode_En = NULL;
}

bool CGameNet::InitDynCode(STNetMsgHeader* pHeader)
{
    char szDynCode[PACKAGE_LENGTH] = { 0 };

    int nDecLen = m_soGgProto.Decode(pHeader->wMsgID,  (char*)m_sDynCryBuf.c_str(), 
            pHeader->wDataLen, szDynCode, PACKAGE_LENGTH);
    if (-1 == nDecLen)
    {
        return false;
    }

    PKG_GG_CLI_DynCode_NTF* pNtf = (PKG_GG_CLI_DynCode_NTF*)szDynCode;
    
    //std::ofstream dynFile("CliDynCode.dat", std::ios::out|std::ios::binary);
    //if (dynFile.good())
    //{
    //    dynFile.write((char*)pNtf->abyDataBuf, pNtf->dwDataLen);
    //}
    //dynFile.close();

    return TRUE == SetDynEnDe(DED_New_Old, pNtf->dwDataLen, (const char*)pNtf->abyDataBuf);
}

//传世封包解析t
static bool isOnGameProc = false;
void CGameNet::OnSocketMessageRecieve(char* pszMsg, int nLen)
{
	_TDEFAULTMESSAGE	tdm;
	INT	nChrPos[] = { 350, 280, 350, 250, 320, 390 };
	static char szPacket[PACKAGE_LENGTH] = { 0 };

	fnDecodeMessage(&tdm, pszMsg);
	output_debug("协议 %d, Recog=%d, Param=%d, Tag=%d, Series=%d, IsGS=%d\n", tdm.wIdent, tdm.nRecog, tdm.wParam, tdm.wTag, tdm.wSeries, isOnGameProc);
	if(tdm.wIdent == SC_CONFIG_INFO) //临时解决办法
	{
		isOnGameProc = true;
	}
	if(tdm.wIdent == 1){
		isOnGameProc = false;		//切回登录流程 
	}

	if(!isOnGameProc){
		 //output_debug("LS 协议 %d\n", tdm.wIdent);

		//登录流程
		if (tdm.wIdent <= SD_CLI_GS_MSG_ID_MIN)
		{
			*((unsigned short*)szPacket) = tdm.wIdent;
			int nDecLen = sizeof(tdm.wIdent);
			if (nLen > 0) {
				char	szDecodeMsg[PACKAGE_LENGTH];
				ZeroMemory(szDecodeMsg, sizeof(szDecodeMsg));
				fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), szDecodeMsg, sizeof(szDecodeMsg));
				strcpy(szPacket+2, szDecodeMsg);
				nDecLen += strlen(szDecodeMsg);
			}
			g_pGameControl->GCL_AddMsg(szPacket, nDecLen);   
		}else{	

		}
	}else{

		 //output_debug("GS 协议 %d\n", tdm.wIdent);

		//游戏流程
		int nDecLen = sizeof(tdm.wIdent);
		int		nPos = 0;
		char	szDecMsg[PACKAGE_LENGTH] = { 0 };
		ZeroMemory(szDecMsg, sizeof(szDecMsg));

		*((unsigned short*)szDecMsg) = tdm.wIdent;
		int msgLen = fnDecode6BitBuf(pszMsg, szDecMsg+2, PACKAGE_LENGTH);				//消息本体转成字符串
		int decMsgLen = sizeof(_TDEFAULTMESSAGE);				//消息体长度12字节

		/*
		*((unsigned short*)szPacket) = tdm.wIdent;
		strcpy(szPacket+2, szDecMsg);							//消息体
		*/
		nDecLen += CMD_SIZE;

		if (nLen > 0) {
			char	szDecodeMsg[PACKAGE_LENGTH];
			ZeroMemory(szDecodeMsg, sizeof(szDecodeMsg));
			int decLen = fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), szDecodeMsg, sizeof(szDecodeMsg));
			strcpy(szDecMsg+2+CMD_SIZE, szDecodeMsg);
			//nDecLen += strlen(szDecodeMsg);			// 只能处理字符串,如果发过来的是结构,长度错误
			nDecLen += decLen;
		}
		g_pGameControl->GCL_AddMsg(szDecMsg, nDecLen);   
	}
}

// 小退时重置游戏流程标志，使后续角色服消息走登录流程分支
void CGameNet::ResetGameProcFlag()
{
	isOnGameProc = false;
}

//封包处理
void CGameNet::ProcessBuffer(SOCKET_INFORMATION* SocketInfo)
{
	if(SocketInfo->BytesRECV <= 0)
		return;

	DWORD	dwLen	= 0;
    DWORD	i		= 0;

	PERF_SEG_ALL(GameNetProcess,FALSE);

    int bytesRecv = SocketInfo->BytesRECV;
    
    if (!SocketInfo->RecBuf.empty())
    {
        if (bytesRecv+SocketInfo->RecvLen < gnNetMsgHeaderSize)
        {
            // 头部不完整，继续收包。note: 没有使用<=因为有些包消息体可能为空!
            output_debug("incomplete header, continue\n");
            SocketInfo->RecBuf.append(SocketInfo->Buffer, bytesRecv);
            SocketInfo->RecvLen += bytesRecv;
            return;
        }
    }
    
    // 收集当前收到的字节，第二个封包开始需要递加1字节
    SocketInfo->RecBuf.append(SocketInfo->Buffer, bytesRecv);
    SocketInfo->RecvLen += bytesRecv;

    bytesRecv = SocketInfo->RecvLen;

	//传世解析
	char	*pszFirst = (char*)SocketInfo->RecBuf.c_str();
	char	*pszEnd;
	// 跳过前导残留字节，确保从 '#' 开始解析。否则分隔符 '#' 会被误并入头部编码，
	// 导致 fnDecode6BitBuf 用含 '#' 的 16 字符解码，wIdent 变成乱码（如 27904 而非 504）
	while (*pszFirst && *pszFirst != '#')
		pszFirst++;
	while (pszEnd = strchr(pszFirst, '!'))
	{
		*pszEnd = '\0';

		OnSocketMessageRecieve(pszFirst + 1, pszEnd - pszFirst - 1 - DEFBLOCKSIZE);

		if (*(pszEnd + 1) == '#')
			pszFirst = pszEnd + 1;
		else{
			//处理完整后清空包
			SocketInfo->RecBuf.clear();
			SocketInfo->RecvLen = 0;
			return;
		}
	}

    while (bytesRecv > 0)
    { 

        // 从头部开始解析
        char* pNextPacket = (char*)SocketInfo->RecBuf.c_str();

        STNetMsgHeader* pHeader = (STNetMsgHeader*)pNextPacket;
        pHeader->wMsgID = ::ntohs(pHeader->wMsgID);
        pHeader->wDataLen = ::ntohs(pHeader->wDataLen);

        int nCompleteLen = pHeader->wDataLen+gnNetMsgHeaderSize;

        if ( bytesRecv < nCompleteLen )
        {
            if (nCompleteLen > 64*1024) // 16k, just for test
            {
                output_debug("msgid=[%u|0x%x], invalid complen=%d\n", 
                    pHeader->wMsgID, pHeader->wMsgID, nCompleteLen);
                
                pHeader->wMsgID = ::htons(pHeader->wMsgID);
                pHeader->wDataLen = ::htons(pHeader->wDataLen);

                SocketInfo->RecBuf.clear();
                SocketInfo->RecvLen = 0;

                return;
            }

            pHeader->wMsgID = ::htons(pHeader->wMsgID);
            pHeader->wDataLen = ::htons(pHeader->wDataLen);

            return;
        }

        if (pHeader->wMsgID == 0)
        {
            output_debug("Allert! msgid == 0");
        }
        
        // 对LoginGate过来的包关闭掉动态加解密
        if (LG_CLI_GroupList_NTF == pHeader->wMsgID) {
            g_pNet->CloseDynCode();
        }

        // 1.静态解密
        if (m_bNewDynDec)
        {
            // 对协议体进行静态解密
            std::string tmp = NewDecode(pNextPacket+gnNetMsgHeaderSize,
                                        pHeader->wDataLen);
			//output_debug("静态解密: %s %s\n", tmp.c_str(), pNextPacket+gnNetMsgHeaderSize);

            m_sDynCryBuf.clear();
            m_sDynCryBuf.append(tmp);
            memset(m_cBuf, 0, DATA_BUFSIZE);

        }

        // 2.动态解密:
        // 如果是动态密码通知消息，进行动态解密设置。
        if (GG_CLI_DynCode_NTF == pHeader->wMsgID)
        {
            if (false == InitDynCode(pHeader))
            {
                output_debug("init dynamic code buffer failed!\n"); 
                m_bNewDynDec = false;
            }
            else
            {
                output_debug("init dynamic code buffer successfully!\n");
            }
            
            SocketInfo->RecBuf.erase(0, nCompleteLen);            
            SocketInfo->RecvLen -= nCompleteLen;
            bytesRecv -= nCompleteLen;            

            return;
        }
        else
        {
            if(m_lpDynCode_De && m_bNewDynDec)
            {
                TRY_BEGIN
                memcpy(m_cBuf, m_sDynCryBuf.c_str(), m_sDynCryBuf.size());
                if (FALSE == m_lpDynCode_De((LPBYTE)m_cBuf, m_sDynCryBuf.size()))
                {
                    output_debug("dyn-decode msgid=[%u|0x%x] failed!\n", 
                        pHeader->wMsgID, pHeader->wMsgID);

                    SocketInfo->RecBuf.erase(0, nCompleteLen);
                    SocketInfo->RecvLen -= nCompleteLen;
                    bytesRecv -= nCompleteLen;

                    return;
                }
                TRY_END
            } 
        }

        // 3.协议解包
        // 首先判断是和Gate间的消息还是和GS间的消息
        static char szPacket[PACKAGE_LENGTH] = { 0 };
        int nDecLen = -1;
        bool isOldMsg = true;

        if (pHeader->wMsgID >= SD_CLI_GS_MSG_ID_MIN)
        {
            // Client和GS的消息包
            if (m_lpDynCode_De && m_bNewDynDec)
            {
                // 动态解密后,内容放在m_cBuf里
                nDecLen = m_soGsProto.Decode(pHeader->wMsgID,
                    m_cBuf, 
                    m_sDynCryBuf.size(),
                    szPacket+2, 
                    PACKAGE_LENGTH-2);

				//output_debug("动态解密后2: %s\n", m_cBuf);
            }
            else
            {
                // 静态解密后，内容放在m_sDynCryBuf里
                nDecLen = m_soGsProto.Decode(pHeader->wMsgID,
                    (char*)m_sDynCryBuf.c_str(), 
                    m_sDynCryBuf.size(),
                    szPacket+2,
                    PACKAGE_LENGTH-2);

				//output_debug("静态解密后2: %s\n", m_sDynCryBuf.c_str());
            }
        }
        else
        {   
            // Client和Gate间的消息包
            if (m_lpDynCode_De && m_bNewDynDec)
            {
                // 动态解密后,内容放在m_cBuf里
                nDecLen = m_soGgProto.Decode(pHeader->wMsgID,
                    m_cBuf, 
                    m_sDynCryBuf.size(),
                    szPacket+2,
                    PACKAGE_LENGTH-2);         
				//output_debug("动态解密后3: %s\n", m_cBuf);
            }
            else
            {
                // 静态解密后，内容放在m_sDynCryBuf里
                nDecLen = m_soGgProto.Decode(pHeader->wMsgID, 
                    (char*)m_sDynCryBuf.c_str(), 
                    m_sDynCryBuf.size(), 
                    szPacket+2, 
                    PACKAGE_LENGTH-2);    
				//output_debug("静态解密后3: %s\n", m_sDynCryBuf.c_str());
            }
            
            
            isOldMsg = false;
        }

        if (-1 == nDecLen)
        {
            // 解包失败
            output_debug("protocol decode [%u|0x%x] failed\n", pHeader->wMsgID, pHeader->wMsgID);
        }
        else
        {
            // 解包成功，加入消息队列: 2是msgid的长度
            if (pHeader->wMsgID >= SD_CLI_GS_MSG_ID_MIN)
            {
                PKG_GS_CLI_OLD_MSG* pMsg = (PKG_GS_CLI_OLD_MSG*)(szPacket+2);

                int wMsgLen = pMsg->wMsgLen;
                if (wMsgLen == 0)
                {
                    output_debug("empty packet\n");
                }
                else
                {
                    pMsg->wMsgLen = pHeader->wMsgID;
					WORD wMsgID = Conv_WORD((const char*)pMsg + 2 + 4);
					if (wMsgID > 3)//过滤掉和EN_CliGgProto_MessageID中冲突的gs协议
					{
						output_debug("OfficeServerPacket Dec : %s\n", pMsg->abyMsgData);
						g_pGameControl->GCL_AddMsg((const char*)pMsg, wMsgLen+2);
					}
					else
					{
						output_debug("错误的gs协议(< 3 )");
						FILE *fp = fopen("../ErrProt.dat","wb");
						if (fp)
						{
							fwrite((const char*)pMsg,wMsgLen+2,1,fp);
							fclose(fp);
						}
					}

#ifdef NETWORK_HOOK
					if (mirRecv)
					{
						string bbb = NewEncode((const char*)pMsg + 2,wMsgLen);//前面丙个字节是新格式加上的头,==协议号
						DoMirRecv(bbb.c_str(),bbb.size());
					}
#endif				
				}
            }
            else
            {
                *((unsigned short*)szPacket) = pHeader->wMsgID;
				char* s2=(char*)szPacket;
				char**s3=&s2;
				int len = sizeof(szPacket);
				output_debug("OfficeServerPacket Dec2 : Len %s %s\n", s3, s2);
                g_pGameControl->GCL_AddMsg(szPacket, nDecLen+2);    


#ifdef NETWORK_HOOK
				if (mirRecv)
				{
					string bbb = NewEncode((const char*)szPacket + 2,nDecLen);//前面丙个字节是新格式加上的头,==协议号
					DoMirRecv(bbb.c_str(),bbb.size());
				}
#endif
            }
        }
        
        // 取下一个packet地址
        SocketInfo->RecBuf.erase(0, nCompleteLen);
        SocketInfo->RecvLen -= nCompleteLen;
        bytesRecv -= nCompleteLen;
        m_sDynCryBuf.clear();
        memset(m_cBuf, 0, DATA_BUFSIZE);
        
        if (!SocketInfo->RecBuf.empty())
        {
            if (bytesRecv+SocketInfo->RecvLen < gnNetMsgHeaderSize)
            {
                return;
            }
        }
    }   

    return; 
}

void CGameNet::Close(int iServer)
{
	m_bNeedSocket	= false;
	__super::Close(iServer);
}

void CGameNet::Clean()
{
	m_buf.clear();
}


void CGameNet::AddMsg(const  char * buf,int nLen)
{
	g_pGameControl->GCL_AddMsg(buf,nLen);
}

int CGameNet::send(int iServer,
		  const char * buf,
		  int len,
		  int flags)
{ 
	SOCKET_INFORMATION * SocketInfo = m_buf[iServer];
	return Send(iServer,(char *)buf, len);
}

int CGameNet::SendPacket( _TDEFAULTMESSAGE* lpDefMsg, char *pszData, int iServerType)
{
	fnEncodeMessage(lpDefMsg, m_szEncodeDefMsg, sizeof(m_szEncodeDefMsg));

	if(IsGS()){
		((SysCount >= 9) ? SysCount = 1 : SysCount++);
	}

	if (pszData)
	{
		fnEncode6BitBuf((unsigned char *)pszData, m_szEncodeBody, strlen(pszData), sizeof(m_szEncodeBody));
		sprintf(m_szPacket, "#%d%s%s!", SysCount, m_szEncodeDefMsg, m_szEncodeBody);
	}
	else
		sprintf(m_szPacket, "#%d%s!", SysCount, m_szEncodeDefMsg);

	//send(m_sockClient, m_szPacket, strlen(m_szPacket), 0);
	return Send(iServerType,m_szPacket,strlen(m_szPacket));;
}

int CGameNet::SendBuf(int iServer, 
                      char* buf,
                      DWORD len, 
                      WORD wMsgID,
                      /*bool bNeed, 
                      bool bNeedDyn,*/
					  bool bIsSGS)
{
#ifdef ENABLE_REPLAY
	if(g_ReplayManager.IsInReplay())
	{
		return 0;
	}
#endif

	//还没有应答的情况下，不发送新的数据
	SOCKET_INFORMATION * SocketInfo = m_buf[iServer];

	if(SocketInfo->bProxyUsed && SocketInfo->bWaitReply)
	{
		return 0;
	}

	//传世建包 后期方法类里面
	//_TDEFAULTMESSAGE DefMsg;
	//char				m_szEncodeDefMsg[32];			// For Send
	//char				m_szEncodeBody[8192];			
	//char				m_szPacket[8192 * 2];
	fnMakeDefMessage(&DefMsg, wMsgID, 0, 0, 0, 0);

	//SEND_GAME_SERVER 发送
	if(bIsSGS){
		//复制buf前12位到DefMsg		(_TDEFAULTMESSAGE)
		memcpy(&DefMsg, buf, 12);

		if(wMsgID != DefMsg.wIdent){
			//复制错误
			int jjjEr = 0;
		}
	}

	if(IsGS()){
		((SysCount >= 9) ? SysCount = 1 : SysCount++);
	}

	fnEncodeMessage(&DefMsg, m_szEncodeDefMsg, sizeof(m_szEncodeDefMsg));
	if (buf)
	{
		if(bIsSGS){
			//SEND_GAME_SERVER 后12位为MsgBody
			// 必须用真实 body 长度(len-12)编码, 不能用 strlen(buf+12):
			// 二进制结构体(如 REGISTERACCOUNT)内含有 '\0', strlen 会在第一个 '\0' 处截断,
			// 导致服务端收到的 datasize 远小于 226 而注册失败。len 即 iBLen(12字节头+数据)。
			int nBodyLen = (len > 12) ? (int)(len - 12) : 0;
			fnEncode6BitBuf((unsigned char *)buf+12, m_szEncodeBody, nBodyLen, sizeof(m_szEncodeBody));
			sprintf(m_szPacket, "#%d%s%s!", SysCount, m_szEncodeDefMsg, m_szEncodeBody);
		}else
		{
			fnEncode6BitBuf((unsigned char *)buf, m_szEncodeBody, strlen(buf), sizeof(m_szEncodeBody));
			sprintf(m_szPacket, "#%d%s%s!", SysCount, m_szEncodeDefMsg, m_szEncodeBody);
		}
	}
	else
		sprintf(m_szPacket, "#%d%s!", SysCount, m_szEncodeDefMsg);
	return Send(iServer,m_szPacket,strlen(m_szPacket));;

	/*
#ifdef NETWORK_HOOK
	if(mirSend)
	{
		std::string bbb = NewEncode(buf,len);
		DoMirSend(bbb.c_str(),bbb.size());
	}
#endif

	bool bForceNotAddTrusteeshipHeader = false;//是否强制不使用托管协议头而使用原来主体的普通协议发送
// 	//微操或附身模式下不允许使用的协议
// 	if (g_TrusteeshipData.IsTrusteeship() && g_pSelf != &ORIGINALSELF)
// 	{
// 		WORD wMsgID = Conv_WORD(buf + 4);
// 		map<WORD,BYTE>::iterator itr = m_bTrusteeshipAllowProtocol.find(wMsgID);
// 		if (itr == m_bTrusteeshipAllowProtocol.end())
// 		{
// 			g_TalkMgr.PushSysTalk("操控其他队员时不能使用此功能！");
// 			return 0;
// 		}
// 		else if(itr->second == 1)
// 		{
// 			if (wMsgID == CS_MESSAGE_SEND && strstr(buf + 12,"@Rest"))//微操时让宠物停止,用微操协议发,其它情况用主体协议发
// 			{
// 				bForceNotAddTrusteeshipHeader = false;
// 			}
// 			else
// 			{
// 				bForceNotAddTrusteeshipHeader = true;
// 			}
// 		}
// 	}

	if(bNeed)		// 需要加密的消息
	{		
       // 1.构造二进制协议包
        STNetMsgHeader stHeader;
        stHeader.wMsgID = ::htons(wMsgID);

        static char szPacket[PACKAGE_LENGTH] = { 0 };
        int nEncLen = -1;
        if (wMsgID == CLI_GS_OLD_MSG)
        {
            // 为了兼容老的Client和GS间的协议，在此处封装消息体
            PKG_CLI_GS_OLD_MSG stMsg;
// 			if (!bForceNotAddTrusteeshipHeader && g_pSelf != &ORIGINALSELF)//托管模式下的微操模式或附身模式
// 			{
// 				SET_COMMAND(CS_MICROCONTROL_SENDPROTOCOL,12);
// 				if (g_pSelf)
// 				{
// 					ASSIGN_ID(g_pSelf->GetID());
// 				}
// 				else
// 				{
// 					return 0;
// 				}
// 
// 				memcpy(stMsg.abyMsgData, temp, 12);
// 				memcpy(stMsg.abyMsgData + 12, buf, len);
// 				len += CMD_SIZE;
// 			}
// 			else
			{
				memcpy(stMsg.abyMsgData, buf, len);
			}

            stMsg.wMsgLen = (WORD)len;
            nEncLen = m_soGsProto.Encode(wMsgID, (void*)&stMsg,
                szPacket+gnNetMsgHeaderSize, PACKAGE_LENGTH-gnNetMsgHeaderSize);
        }
        else
        {
            // 新协议都是在外部构造消息体，在此处直接组包
            nEncLen = m_soGgProto.Encode(wMsgID, (void*)buf,
                szPacket+gnNetMsgHeaderSize, PACKAGE_LENGTH-gnNetMsgHeaderSize);
        }

        if (-1 == nEncLen)
        {
            output_debug("protocol encode failed for [%u|0x%x]\n", wMsgID, wMsgID);
            return 0;
        }
        
        // 2.动态加密
        if(bNeedDyn && DED_New_Old == m_dwDynType && m_lpDynCode_En)
        {
            TRY_BEGIN
            if (FALSE == m_lpDynCode_En((LPBYTE)szPacket+gnNetMsgHeaderSize,nEncLen))
            {
                output_debug("dyn-encode [%u|0x%x] failed\n", wMsgID, wMsgID);
                return 0;
            }
            TRY_END_NOTHROW
        }

        // 3.静态加密
        string sendBuf = NewEncode(szPacket+gnNetMsgHeaderSize, nEncLen);
        memcpy(szPacket+gnNetMsgHeaderSize, sendBuf.c_str(), sendBuf.size());

        stHeader.wDataLen = ::htons(sendBuf.size());
        *((STNetMsgHeader*)szPacket) = stHeader;

        if(!SocketInfo->bConnect)
        {
            AsynSend(iServer, szPacket, sendBuf.size()+gnNetMsgHeaderSize);
            return 0;
        }

        return Send(iServer, szPacket, sendBuf.size()+gnNetMsgHeaderSize);
	}
	else
	{
		// 不需要加密或者已经处理过加密的消息
		if(!SocketInfo->bConnect)
		{
			AsynSend(iServer,buf,len);
			return 0;
		}
		return Send(iServer,buf,len);
	}*/
}

void CGameNet::OnClose(DWORD wParam,DWORD lParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;
	m_buf[i]->bConnect = false;

	if(m_bNeedSocket)
	{
		g_pGameControl->GCL_AddErr();
	}
}
void CGameNet::OnError(DWORD wParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;
	m_buf[i]->bConnect = false;

	g_pGameControl->GCL_AddErr();
}

void CGameNet::OnConnect(DWORD wParam,DWORD lParam)
{
	if( WSAGETSELECTERROR(lParam) ) 
	{
		g_pGameControl->GCL_AddErr();
	}
	else
	{
		int i = GetSocketInfo(wParam);
		if(i == -1)
			return;

		DealOnConnect(i);
	}
}

void CGameNet::DealOnConnect(int iServer)
{
	if(iServer < 0 || iServer >= m_buf.size())
		return;

	SOCKET_INFORMATION* SocketInfo = m_buf[iServer];
	SocketInfo->bConnect = true;

	if(SocketInfo->iSendBuffer )
	{
		Send(iServer,SocketInfo->SendBuffer,SocketInfo->iSendBuffer);
		SocketInfo->iSendBuffer = 0;
	}
	m_bNeedSocket = true;
}


void CGameNet::ProcessAsterisk()
{
	char temp[6] = {0};
	temp[0] = '*';
	this->Send(0,temp,6);
}

int CGameNet::AsynSend(int iServer,char * buf,DWORD len)
{
	if(len < DATA_BUFSIZE)
	{
		memcpy(m_buf[iServer]->SendBuffer,buf,len);
		m_buf[iServer]->iSendBuffer = len;
	}

	return 0;
}

void CGameNet::OnRead(DWORD wParam,DWORD lParam)
{
	return;
}

void CGameNet::DealRead()
{
	if (m_buf.size() > 0 && m_buf[0]/* && m_buf[0]->bConnect*/)
	{
		CNet::OnRead((DWORD)(m_buf[0]->Socket),0);
	}
}

#ifdef NETWORK_HOOK
void CGameNet::DoMirRecv(const char* buf,int iLen)
{
	if(mirRecv)
	{
		auto_ptr<char> pBuf(new char[iLen + 2]);
		*pBuf.get() = '#';
		memcpy(pBuf.get() + 1,buf,iLen);
		*(pBuf.get() + iLen + 1) = '!';
		mirRecv(0,pBuf.get(),iLen + 2,0);
	}
}
#endif

#ifdef NETWORK_HOOK
void CGameNet::DoMirSend(const char* buf,int iLen)
{
	if(mirSend)
	{
		static char counter = '1';

		auto_ptr<char> pBuf(new char[iLen + 3]);
		*pBuf.get()  = '#';
		*(pBuf.get() + 1) = counter;
		memcpy(pBuf.get() + 2,buf,iLen);
		*(pBuf.get() + iLen + 2) = '!';

		if(counter > '9') 
			counter = '1';

		mirSend(0,pBuf.get(),iLen + 3,0);
	}
}
#endif
