#pragma once

#define MAXSERVER		  1
#define SOCK_ERROR_CONNECTFAIL			0x0002
#define SOCK_ERROR_DISCONNECTED			0x0003

#define ENCRYPT_SEED	0xEB
#define ENCRYPT_BASE	0x3B

#include "Global/Global.h"
#include "Global/Interface/StreamInterface.h"

#define NETWORK_HOOK

#define DEFBLOCKSIZE			16



enum E_SERVER
{
	SERVER_GAME = 0,
	SERVER_LOGIN = 1
};

#include <WinSock2.h>

// 是否发送监视数据
#define DATA_BUFSIZE        1024*1024
#define DATA_SEND_BUFSIZE   10240
#define MAX_DATA_CRYBUFFER  20 * 1024

enum DynEnDec
{
	DED_Old,
	DED_New,
	DED_New_Old,

	DED_Types,
};

typedef BOOL		(WINAPI *LPDYNCODE)		(LPBYTE,DWORD);
typedef LPDYNCODE	(WINAPI *LPGETDYNCODE)	(DWORD);

struct SOCKET_INFORMATION
{
	SOCKET_INFORMATION()
	{
		BytesRECV	= 0;
		Socket		= 0;
		RecvLen     = 0;
		iSendBuffer = 0;
		DataBuf.buf	= 0;
		DataBuf.len = 0;

		strServerName.clear();
		iPort       = 0;
		bConnect    = false;
		bProxyUsed  = false;
		bWaitReply  = false;
		iReConnectTimes = 0;
		dwLastConnectTime = 0;

		Buffer = new char[DATA_BUFSIZE];
		SendBuffer = new char[DATA_BUFSIZE];
	}

	~SOCKET_INFORMATION()
	{
		SAFE_DELETE_ARRAY(Buffer);
		SAFE_DELETE_ARRAY(SendBuffer);
	}

	char*			Buffer;
	char*			SendBuffer;
	string		    RecBuf;
	int             RecvLen;

	int				iSendBuffer;

	WSABUF			DataBuf;
	SOCKET			Socket;
	DWORD			BytesRECV;

	bool            bConnect;             //是否已经连接

	string		    strServerName;        //服务器名称或IP
	int				iPort;                //服务器端口号
	bool            bProxyUsed;           //是否使用了代理
	bool            bWaitReply;           //连接代理后等待应答
	int             iReConnectTimes;//一共重连了多少次
	DWORD           dwLastConnectTime;//最后一次连接的时候

};

//传世封包
//NPC包
typedef struct tagCHARDESC
{
	LONG	nFeature;
	LONG	nStatus;
	LONG	nHp;
	LONG	nTiger;
}CHARDESC, *LPCHARDESC;

//12 个字节
typedef struct tagTCHARDESC
{
	LONG	nFeature;
	LONG	nStatus;
	LONG	nHp;
}TCHARDESC, *LPTCHARDESC;

//MakeLong(MakeWord(btRaceImg, btWeapon), MakeWord(btHair, btDress));
typedef struct tagFEATURE
{
	BYTE	bGender;	//btRaceImg 外形 0标示人形
	BYTE	bWeapon;	//btWeapon 武器
	BYTE	bHair;		//btHair 头发
	BYTE	bDress;		//btDress 衣服
}FEATURE, *LPFEATURE;

typedef struct tag_TDEFAULTMESSAGE
{
	int		nRecog;	//0-3
	WORD	wIdent;		//4-5
	WORD	wParam;	//6-7
	WORD	wTag;		//8-9
	WORD	wSeries;	//10-11
} _TDEFAULTMESSAGE, *_LPTDEFAULTMESSAGE;

typedef struct tagPACKETMSG
{
	_TDEFAULTMESSAGE	stDefMsg;
	CHAR				szEncodeData[MAX_PATH * 4];
}PACKETMSG, *LPPACKETMSG;

typedef struct tagMESSAGEBODYWL
{
	LONG	lParam1;
	LONG	lParam2;
	LONG	nTag1;		
	LONG	nTag2;
}MESSAGEBODYWL, *LPMESSAGEBODYWL;

typedef struct tagMESSAGEBODYWL_INT
{
	INT	lParam1;
	INT	lParam2;
	INT	nTag1;		
	INT	nTag2;
}MESSAGEBODYWL_INT, *LPMESSAGEBODYWL_INT;

typedef struct tagMESSAGEBODYW
{
	WORD	wParam1;
	WORD	wParam2;
	WORD	wTag1;	
	WORD	wTag2;
}MESSAGEBODYW, *LPMESSAGEBODYW;

typedef struct tag_TSHORTMSSEAGE
{
	WORD	wIdent;
	WORD	wMsg;
} _TSHORTMSSEAGE, *_LPTSHORTMSSEAGE;

//传世
void WINAPI fnMakeDefMessage(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries);
int  WINAPI fnEncode6BitBuf(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen);
int  WINAPI fnDecode6BitBuf(char *pszSrc, char *pszDest, int nDestLen);
int  WINAPI fnEncodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf, int nLen);
int  WINAPI fnDecodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf);


//传世End

//网络通信使用Windows的消息机制
typedef std::vector<SOCKET_INFORMATION*> SocketInfoVector;


class CNet
{
protected:
	CNet(void){}
	long				m_lEvent;
	HWND				m_hWnd;
	DWORD				m_dwMsg;
	SocketInfoVector	m_buf;
	sockaddr_in			m_stSockAddr;            //套接字连接信息
	int					m_nMaxServer;
	int					m_nTimer;

	DWORD				m_dwSendBytes,m_dwRecvBytes;
	DWORD				m_dwTotalSendBytes,m_dwTotalRecvBytes;

	DWORD				m_dwDynType;
	LPVOID				m_pDynCode;
	UINT				m_iDynLen;
	LPDYNCODE			m_lpDynCode_En;
	LPDYNCODE			m_lpDynCode_De;
	DWORD               m_dwDynCodeLen;//服务器端动态加解密算法的长度，如果最后收到的长度不对就会重新请求

	//使用代理服务器
	string              m_strProxyName;
	int                 m_nProxyPort;

	void                LoginProxyReq(int i);
	void                ProcessProxyReply(int i);

public:
	CNet(int iMaxServer,HWND hWnd,DWORD dwMsg,long lEvent = FD_CONNECT | FD_READ | FD_WRITE|FD_CLOSE);
	virtual void Close(int iServer);
	virtual ~CNet(void);
	void OnCommand(DWORD wParam,DWORD lParam);	//消息处理。
	bool Connect(int iServer,BOOL bProxy = FALSE);                                       //连接套接字
	int  Send(int iServer,char * buf,DWORD len);                                   //发送信息
	void SetServer(int iServer,LPCTSTR strServerName,int nPort);		
	void GetServer(char * str) { strcpy(str,m_buf[0]->strServerName.c_str()); }
	int	GetPort() { return m_buf[0]->iPort;}
	bool IsConnected(){return m_buf[0]->bConnect;}
	void ResetLastConnectTime(){m_buf[0]->dwLastConnectTime = 0;}
	void SetDynCodeLen(DWORD dwLen){m_dwDynCodeLen = dwLen;}

	SOCKET_INFORMATION&	GetSocketInfoVector(){return *(m_buf[0]);}		//为了在需要时,获取IP地址

    BOOL	SetDynEnDe(DWORD dwType,DWORD len,const char *buf);
    DWORD	GetDynType();
    BOOL	SetDynEnDe(const char *buf,UINT len);

	inline string NewEncode(const char* buf, int len)
	{	
		string sResult;
		int no = 2;
		BYTE remainder = 0, temp, c;
		BYTE bySeed = ENCRYPT_SEED;
		BYTE byBase = ENCRYPT_BASE;

		for (int i = 0; i < len; i++)
		{
			c = buf[i] ^ bySeed;
			if (no == 6)
			{
				sResult += (c & 0x3F) + byBase;
				remainder = remainder | ((c >> 2) & 0x30);
				sResult += remainder + byBase;
				remainder = 0;
			}
			else
			{
				temp = c >> 2;
				sResult += ((temp & 0x3C) | (c & 0x3)) + byBase;
				remainder = (remainder << 2) | (temp & 0x3);
			}
			no = no % 6 + 2;
		}
		if ( no != 2 )
		{
			sResult += remainder + byBase;
		}

		return sResult;
	}

	//传世封包解密
	inline int DecodeMsg(const BYTE *inbuf,BYTE *outbuf,int len)   
	{   
		int outcount=0;   
		BYTE b1,b2,b3,b4;   
		int bcount=0;   
		int kk = len / 4;   
		int ll = len % 4;   
		int  index;   
		for(int i=0;i<kk;i++)    // 先按照四个字节一组处理   
		{   index = i << 2;   
		b1 = inbuf[index] - 0x3b;   
		b2 = inbuf[index + 1] - 0x3b;   
		b3 = inbuf[index + 2] - 0x3b;   
		b4 = inbuf[index + 3] - 0x3b;   

		outbuf[outcount] = (((b1<<2) & 0xF0) | (b1 & 0x03) | (b4 & 0x0C)) ^ 0xeb;   
		outcount++;   

		outbuf[outcount] = (((b2<<2) & 0xF0) | (b2 & 0x03) | ((b4<<2) & 0x0C)) ^ 0xeb;   
		outcount++;   

		outbuf[outcount] = (b3 | ((b4<<2) & 0xC0)) ^ 0xeb;   
		outcount++;   
		}   
		index = kk * 4;   
		if(ll == 2)   
		{   b1 = inbuf[index]     - 0x3b;   
		b4 = inbuf[index + 1] - 0x3b;   

		outbuf[outcount] = (((b1<<2) & 0xF0) | (b1 & 0x03) | ((b4<<2) & 0x0C)) ^ 0xeb;   
		outcount++;   
		}   
		else if(ll == 3)   
		{   b1 = inbuf[index]     - 0x3b;   
		b2 = inbuf[index + 1] - 0x3b;   
		b4 = inbuf[index + 2] - 0x3b;   

		outbuf[outcount] = (((b1<<2) & 0xF0) | (b1 & 0x03) | (b4 & 0x0C)) ^ 0xeb;   
		outcount++;   

		outbuf[outcount] = (((b2<<2) & 0xF0) | (b2 & 0x03) | ((b4<<2) & 0x0C)) ^ 0xeb;   
		outcount++;   
		}   
		outbuf[outcount] = 0;   
		return outcount;   
	}   

	//传世封包加密
	inline int EncodeMsg(const BYTE *inbuf,BYTE *outbuf,int len)   
	{   
		int outcount=0;   
		BYTE b1,b4=0;   
		int bcount=0;   
		for(int i=0;i<len;i++)   
		{   
			b1=inbuf[i]^0xeb;   
			if(bcount==2)   
			{   
				outbuf[outcount]=(b1&0x3f)+0x3b;   
				outcount++;   
				b4|=((b1>>2)&0x30);   
				outbuf[outcount]=b4+0x3b;   
				outcount++;   
				bcount=0;   
				b4=0;   
			}   
			else   
			{   
				outbuf[outcount]=((b1&0xF0)>>2|(b1&3))+0x3b;   
				outcount++;   
				b4<<=2;   
				b4|=(b1>>2)&3;   
				bcount++;   
			}   
		}   
		if(bcount!=0)   
		{   
			outbuf[outcount]=b4+0x3b;   
			outcount++;   
		}   
		outbuf[outcount] = 0;   
		return outcount;   
	}   

	inline string NewDecode(const char* buf, int len)
	{
		string sResult;
		int		nCycles		= len / 4;
		int		nBytesLeft	= len % 4;
		BYTE	bySeed		= ENCRYPT_SEED;
		BYTE	byBase		= ENCRYPT_BASE;
		int		index		= 0;
		BYTE	temp, remainder, c;

		for (int i = 0; i < nCycles; i++)
		{
			remainder = buf[index + 3] - byBase;

			temp = buf[index++] - byBase;
			c = ((temp << 2) & 0xF0) | (remainder & 0x0C) | (temp & 0x3);
			sResult += c ^ bySeed;

			temp = buf[index++] - byBase;
			c = ((temp << 2) & 0xF0) | ((remainder << 2) & 0x0C) | (temp & 0x3);
			sResult += c ^ bySeed;

			temp = buf[index++] - byBase;
			c = temp | ((remainder << 2) & 0xC0);
			sResult += c ^ bySeed;

			index++;
		}

		if (nBytesLeft == 2)
		{
			remainder = buf[len - 1] - byBase;
			temp = buf[index] - byBase;
			c = ((temp << 2) & 0xF0) | ((remainder << 2) & 0x0C) | (temp & 0x3);
			sResult += c ^ bySeed;
		}
		else if (nBytesLeft == 3)
		{
			remainder = buf[len - 1] - byBase;
			temp = buf[index++] - byBase;
			c = ((temp << 2) & 0xF0) | (remainder & 0x0C) | (temp & 0x3);
			sResult += c ^ bySeed;

			temp = buf[index] - byBase;
			c = ((temp << 2) & 0xF0) | ((remainder << 2) & 0x0C) | (temp & 0x3);
			sResult += c ^ bySeed;
		}

		return sResult;
	}

	// 属性处理
	DWORD GetSendBytes()		{ return m_dwSendBytes; }
	DWORD GetRecvBytes()		{ return m_dwRecvBytes; }
	DWORD GetTotalSendBytes()	{ return m_dwTotalSendBytes; }
	DWORD GetTotalRecvBytes()	{ return m_dwTotalRecvBytes; }
	void  ResetSendBytes()		{ m_dwSendBytes = 0; }
	void  ResetRecvBytes()		{ m_dwRecvBytes = 0; }

	// 添加一个代理服务器
	void SetProxy(LPCTSTR strProxyName,int nPort);      //添加一个代理服务器

	void Reset()      {	m_dwDynType = DED_Old;	}

	void OnReadForReplay(const char * strMsg,int iLen);

protected:

	void Init(int iMaxServer,HWND hWnd,DWORD dwMsg,long lEvent);
	int  AsyncSelect(SOCKET s);										 //设置异步通信
	int	 GetSocketInfo(SOCKET s);

	virtual void OnClose(DWORD wParam,DWORD lParam);
	virtual void OnWrite(DWORD wParam,DWORD lParam);
	virtual void OnConnect(DWORD wParam,DWORD lParam);
	virtual void OnError(DWORD wParam);

	virtual void ProcessBuffer(SOCKET_INFORMATION*) {}
	virtual void OnRead(DWORD wParam,DWORD lParam);
	inline  bool IsValidSocket(SOCKET hSocket)
	{
		for(size_t j = 0; j < m_buf.size(); j++)
		{
			if(hSocket == m_buf[j]->Socket)
				return true;
		}
		return false;
	}
};

class CCliGgProto;
class CCliGsProto;
class CCliSsProto;
struct STNetMsgHeader;
struct tagPKG_CLI_SS_MSG;

class CGameNet : public CNet
{
protected:
	_TDEFAULTMESSAGE	DefMsg;
	CHAR				m_szBuff[8192 * 4];				// For Receive
	char				m_szEncodeDefMsg[32];			// For Send
	char				m_szEncodeBody[8192];			
	char				m_szPacket[8192 * 2];

public:	
	CGameNet(int iMaxServer,HWND hWnd,DWORD dwMsg,long lEvent = FD_CONNECT | FD_READ | FD_WRITE|FD_CLOSE);
	~CGameNet();

	//直接发送封包体
	int send(int iServer,
					const char FAR * buf,
					int len,
					int flags);

	//直接发送_TDEFAULTMESSAGE结构
	int SendPacket(_TDEFAULTMESSAGE* lpDefMsg, char *pszData = NULL, int iServerType = SERVER_GAME);

    //bNeed是否要加密,bNeedDyn是否要动态加密(在bNeed为false是无效)
	int	SendBuf(int iServer,
                char * buf,
                DWORD len, 
                WORD wMsgID= 1000/*CLI_GS_OLD_MSG*/,
                /*bool bNeed = true,			//传世不加密
                bool bNeedDyn = true,		//传世不加密*/
				bool	bIsSGS = true	);		//默认为SEND_GAME_SERVER函数

	virtual void Close(int iServer);
	virtual void Clean();
	void SetNeed(bool b)	{ m_bNeedSocket = b; }
	bool	IsNeed()	{ return m_bNeedSocket; }
	int AsynSend(int iServer,char * buf,DWORD len);

    void CloseDynCode();
	void DealOnConnect(int iServer);

	void SetGS(bool isGS){m_bIsGS = isGS;}
	bool IsGS(){return m_bIsGS;}

	// 小退时重置游戏流程标志，使消息走登录流程分支
	void ResetGameProcFlag();

	struct STR2MSG_ENTRY
	{
		char* str;
		WORD  msg;
	};


	virtual void DealRead();//OnRead如果一次读不完socket的缓冲区,其它的数据就有可能永远发不过来

protected:
	//传世
	virtual void OnSocketMessageRecieve(char* pszMsg, int nLen);
	//


	virtual void ProcessBuffer(SOCKET_INFORMATION*);
	virtual void AddMsg(const char * buf,int nLen);
	virtual void OnConnect(DWORD wParam,DWORD lParam);
	virtual void OnClose(DWORD wParam,DWORD lParam);
	virtual void OnError(DWORD wParam);
	void		 ProcessAsterisk();
	virtual void OnRead(DWORD wParam,DWORD lParam);

    bool         InitDynCode(STNetMsgHeader* pHeader);


#ifdef NETWORK_HOOK
	void    DoMirRecv(const char* buf,int iLen);
	void    DoMirSend(const char* buf,int iLen);
#endif

private:
	bool m_bIsGS;
	int SysCount;
	bool	m_bNeedSocket;
	BOOL	m_bDynDec;
	bool	m_bNewDynDec;
	string m_sDynCryBuf;
	char	m_cBuf[DATA_BUFSIZE];
	map<WORD,BYTE> m_bTrusteeshipAllowProtocol;//托管模式下,微操及附身时所允许使用的协议号

	static CCliGgProto m_soGgProto;
    static CCliGsProto m_soGsProto;

};




#pragma pack(1)


struct STDlNetMsgHeader
{
	UINT16  wMsgID;
	DWORD  dwDataLen;
};

#pragma pack()


#define MAX_RECBUF_LENJ_DOWNLOADFILE   3 * 1024 * 1024
#define DOWNLOAD_HEADER_LEN    (sizeof(STDlNetMsgHeader) + sizeof(SDLProtocalHeader))  //下载文件协议头的长度


class CDownloadNet : public CNet
{
public:	
	CDownloadNet(HWND hWnd,DWORD dwMsg,bool bBgLimitedDownload = false,long lEvent = FD_CONNECT | FD_READ | FD_WRITE|FD_CLOSE);
	CDownloadNet();
	virtual ~CDownloadNet();

	int	SendBuf(char * buf,DWORD len);

	virtual void Close(int iServer);
	int AsynSend(int iServer,char * buf,DWORD len);
	int Send(int iServer,char * buf,DWORD len,bool bSendAsynBuf = false);//发送信息
	virtual void SendAsynBuf();
	DWORD GetProcessBufferThreadID() const { return m_dwThreadID; }
	int GetReceiveFiles() const { return m_iReceiveFiles; }
	int GetReceiveBuffSize() const { return m_strAllRecDataBuf.size(); }

	static DWORD WINAPI		ProcessBuffer(LPVOID lpData);	// 纹理处理协议
	void StartBgDownloadCanJumpMap(){m_bNeedDownloadCanJumpMap = true;}//线程后台下载当前地图可以跳转过去的地图及其小地图

protected:
	virtual void OnConnect(DWORD wParam,DWORD lParam);
	virtual void OnClose(DWORD wParam,DWORD lParam);
	virtual void OnError(DWORD wParam);
	virtual void OnRead(DWORD wParam,DWORD lParam);

	void InitMember();

	void SendEmptyProtocal();
	void SendReceiveFileCountToSourceServer();//发送接受到的文件个数到资源服务器
	bool DealRead();
	bool OnBgSavedFile();//后台下载并且保存文件完成
	void BgDownloadCanJumpMap();//线程后台下载当前地图可以跳转过去的地图及其小地图
	void BgSendDownloadFileRequest(__int64 iHash,DWORD dwID,eReadPrior ePrior);//该函数现不支持多线程调用



protected:
	char * m_pRecBuf;//当次接受数据buf
	HANDLE			m_hThread;						// 线程句柄
	DWORD			m_dwThreadID;					// 线程ID
	BOOL     m_bExited;//线程是否退出
	CRITICAL_SECTION	m_CriSect;					// 处理临界变量
	CRITICAL_SECTION	m_CriSect_Send;					// 处理临界变量
	string	 m_strAllRecDataBuf;//所有接受到的数据Buf
	DWORD    m_dwAllRecDataBufLen;//所有接受到的数据Buf的长度
	DWORD    m_dwAllRecDataBufLen_SaveToDisk;//所有接受到的数据Buf的长度
	DWORD    m_dwAllRecDataBufPos;//所有接受到的数据Buf现在读取的位置
	DWORD    m_dwAllRecDataBufPos_SaveToDisk;//所有接受到的数据Buf现在保存的位置
	int      m_iReceiveFiles;//接收到的文件的个数
	int      m_iReceiveDataLen;//接收到的文件数据的长度,不是所有数据,不包括协议头
	int      m_iLastReceiveFiles;//上一次接收到的文件的个数,如果现在不一样就通知一次服务器
	char     *m_pszPacke;
	DWORD    m_dwStartTime;
	int      m_dlSpeed;//下载限速,K/s
	bool     m_bNeedReConnect;
	bool     m_bBgLimitedDownload;//是否后台限速下载,如后台下载地图等
	bool     m_bNeedDownloadCanJumpMap;//是否开始线程后台下载当前地图可以跳转过去的地图及其小地图
	map<__int64,DlProtocalBody> m_MBgInDownloadFile;//正在下载的文件列表


	CCliSsProto *m_pDLoSsProto;


	tagPKG_CLI_SS_MSG* m_pStSSMsg;
	DWORD  m_dwLastSendDataTime;//最后一次发送数据的时间,收来发送心跳包保持连接

};





class CDownloadLimitSpeed : public CDownloadNet
{
public:	
	CDownloadLimitSpeed(HWND hWnd,DWORD dwMsg,long lEvent = FD_CONNECT | FD_READ | FD_WRITE|FD_CLOSE);
	~CDownloadLimitSpeed();

	static DWORD WINAPI		ProcessBuffer(LPVOID lpData);	// 纹理处理协议
	virtual void OnRead(DWORD wParam,DWORD lParam);
	virtual void SendAsynBuf();
	void SendDownLoadFile(const char *filepath,bool bReLoad = false);


protected:
	//FILE *m_pSaveintFile;
	int   m_iSaveintFileHandle;//文件句柄

	int  m_iSaveFilePos;
	int  m_iSaveingFileLen;
	string m_strSavingFileName;
	VDlFileList m_vDlFileList;
	int m_iDlVersion;//当前下载到的版本
	bool m_bHaveDownLoadPlist;


	virtual void OnClose(DWORD wParam,DWORD lParam);
	virtual void OnError(DWORD wParam);
	bool DealRead();
	bool SaveToFile(const char *buf,int iLen,bool bFinish);
	void ParseFileList(const char *filepath);
	void OnFileDownedFinish();
};
