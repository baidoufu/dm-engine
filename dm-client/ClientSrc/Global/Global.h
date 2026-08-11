
//公共模块的头文件,如果改变此文件的内容，将导致
//程序中全部模块重新编译，请尽量不要修改这个文件

#pragma once

#include "Globaldefine.h"
#include "GameData/UnionStruct.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#endif
#include <direct.h>
#include "StdHeaders.h"  //导入stl


#include "GameClient/ISDSampler.h"
#include "GameClient/plugins_export.h"//内嵌到web页面


using namespace SGDP;

#define ENABLE_REPLAY          //是否编译录像功能


#ifdef ENABLE_REPLAY

DWORD MyGetTickCount();//重载了的GetTickCount,如果在放录像=g_dwProStartCount + g_pGfx->GetFrameCount() * 20; 否则为::GetTickCount();
DWORD OrignGetTickCount();//原来系统的GetTickCount();
#define  GetTickCount()  MyGetTickCount()

#else
#define  OrignGetTickCount()  GetTickCount()
#endif


// 设置使用性能测试
#define DOPERFCHECK			1

// 用来测试内存泄漏的new重载
#include "new.h"


// 定义同步机制
#define DEFINE_HANDLE(h)		CRITICAL_SECTION h;
#define INIT_HANDLE(h)			InitializeCriticalSection(&h);
#define LOCK_HANDLE(h)			EnterCriticalSection(&h);
#define UNLOCK_HANDLE(h)		LeaveCriticalSection(&h);
#define DELETE_HANDLE(h)		DeleteCriticalSection(&h);

void	output_debug(char *format,...);					// 调试输出
void	fatal_error(const char * err,DWORD code = 0);	// 发生严重错误，退出程序
void    debug_info_dlg(const char * s);					// 在一个对话框里显示调试信息
void    SaveLogFormat(const char *format,...);//记录日志

// 全局实例
BOOL InitGlobalObject(HWND hWnd);		// 初始化全局变量
void DeleteGlobalObject();				// 摧毁全局变量
bool IsCanRidePet(int iMonster);
//彩虹帮助
bool StartRainBowSpirt();				//启动彩虹帮助
//bool StopRainBowSpirt();                //停止彩虹帮助
//寻找父进程的模块地址
//bool FindParentPath(const DWORD currentProcessID,char* ptrResult,char* sonFileName);

string HexDump(const BYTE* pData, int len, const char *delim);//16进制内容转换成字符串

void  GetDevAccordDir(int iDir,int&ix,int& iy);
WORD  GetGreateMagicID(WORD wMagicID,BYTE byFlyType,int iLevel,bool bDaoZunState,bool IsFightOnLeopard);//大师级技能魔法ID
WORD  GetFightOnLeopardMagicID(WORD wMagicID);
int   GetTexIDByMagicID(WORD wMagicID, int state = 0);
int   GetTexIDByHouseMagicID(WORD wMagicID, bool isZhuDong);
bool  QuickBuy(char* pName,int iCount,int iUseAfterBuy);
void  AssignLooks(LOOKS& Looks,__int64 iLooks);//初始化looks
const char *GetGameDataDir();// Data目录
void FullScreenEventCall(BOOL bFull);//网页版本在全屏和非全屏模式下切换
bool ChangeWindowSize(int iWidth,int iHeight,DisplayMode eNewDisMode);
void ToCommaStr(char* str,int l);//把一个数字变量 100,000,000这样的格式
void OpenWebUrl(const char* szUrl,bool bUseDefaultBrowser = false);//bUseDefaultBrowser:是否使用默认浏览器,否则使用IE打开


//客户端嵌入IE
typedef BOOL  (* LPCgpInitialize)();//CgpInitialize
typedef HWND  (* LPCgpGetActiveWindow)();//CgpGetActiveWindow
typedef void  (* LPCgpUninitialize)();//CgpUninitialize
typedef BOOL  (* LPCgpSetupAppWindow)(HWND hWnd,int nFlag);//CgpSetupAppWindow
typedef void  (* LPCgpListenAppExitEvent)(CgpAppExitEvent_t cb);//CgpListenAppExitEvent
typedef void  (* LPCgpListenFullscreenEvent)(CgpFullscreenEvent_t cb);//CgpListenFullscreenEvent
typedef void  (* LPCgpListenWindowSizeEvent)(CgpWindowSizeEvent_t cb);//CgpListenWindowSizeEvent
typedef UINT  (* LPCgpQueryVariant)(UINT varId, LPSTR pszBuf, UINT nMax);//CgpQueryVariant


#ifdef _DEBUG
#define	DetailOutput(sss)		output_debug("%s(%d):%s\n",__FILE__,__LINE__,sss)
#else
#define	DetailOutput(sss)
#endif

// Global Define
//#define	PI							3.14159f
//#define PREREAD_NONE				0
//#define PREREAD_AUTO				1
//#define PREREAD_FULL				2
//#define	PREREAD_DEBUG				3
//#define WS_GAME_ID                  188

#ifdef DOPERFCHECK
	#define PERF_STARTCHECK(a)		
	#define PERF_ENDCHECK(a)		
	#define PERF_GETCHECKVALUE(a)	0
	#define PERF_RESETCHECK(a)
#else
	#define	TEX_CHECK(pool,w,h)

	#define PERF_STARTCHECK(a)		
	#define PERF_ENDCHECK(a)		
	#define PERF_GETCHECKVALUE(a)	0
	#define PERF_RESETCHECK(a)
#endif

#include "Timer.h"

#pragma warning(disable : 4996)
#pragma warning(disable : 4267)
#pragma warning(disable : 4018)
#pragma warning(disable : 4312)

#define _CRT_SECURE_NO_WARNINGS
#define	MAX_FLY_LEVEL	130


// 全局变量
extern HWND			g_hWnd;					// 主窗口句柄
extern HWND			g_hParentWnd;           // 父窗口句柄,用于ie内嵌游戏
extern int			g_iMainWndX;
extern int			g_iMainWndY;
extern char	*		g_cBuf;
extern char			g_strVersion[64];       // 版本名称
extern DWORD        g_dwBuildID;            // Build号 
extern DWORD		g_dwServerVersion;      // 服务器版本号
extern E_SERVERTYPE g_eServerType;          // gs类型
extern DWORD        g_dwServerSwitch;       // 服务器开关
extern int          g_dwServerTime;         //服务器时间，需要客户端请求才能更新
extern int			g_iPreReadMode;			// 预读模式
extern char			g_strCmdLine[MAX_PATH];
extern DWORD		g_dwMouseType;
extern BOOL			g_bException;
extern BOOL			g_bSelfCursor;
extern int          g_iDXVersion;
extern char         *g_cBuf;
extern int          g_iParamGameType;        //登录类型，0:普通，1,浩方，2:边锋，3:茶苑
extern BYTE         g_byPwdEncode;           //密码加密后再和g_byPwdEncode做异或，g_byPwdEncode启动后随机产生
extern char         g_strMac[36];//MAC地址
extern DWORD        g_dwProStartCount;//程序启动时间
extern char			g_strReplayDir[MAX_PATH];//录像文件及录像配置文件存放目录
extern DWORD        g_dwLoadTime;//程序启动花的时间
//extern float        g_fScale;//缩放比例
extern E_UITYPE     g_EutUiType;//ui类型,0:时尚,1:经典
extern bool         g_bHasDownLoadInitPackage;//小客户端是否下载过初始包
extern bool         g_bNeedScale;//是否缩放,处理内嵌IE版本,大小由外面控件决定
extern POS          g_ScaleRate;//缩放比例,这个比例是相对于800*600的标准情况,处理内嵌IE版本,大小由外面控件决定,//界面缩放用,由于适应不同的分辨率,一般窗口不缩放,只有像panelwnd之类的窗口才缩放,因为事件及消息处理的需要,缩放比例改变时会重新设置所有控件的位置,大小等,所以绘制时还是从m_iScreenX,m_iScreenY开始,不要再考虑缩放比例
extern HMODULE      g_hCgpfsDLL;//客户端嵌入IE用到的动态库
extern bool         g_bIsMultiClientInSameDir;//是否同目录下的多开客户端
extern bool         g_bHasUiInDownloading;//是否有UI正在下载中
extern string       g_strChannelName;//接入的外部平台的名称,为空表示非外部平台


// 全局对象
class CInput;
class CGameNet;
class CDownloadNet;
class CDownloadLimitSpeed;
class CCtrlMainWin;
class CGameMap;
class CGameData;
class CGameControl;
class CUiManager;
class CMagicCtrlMgr;
class ISDRSIEClient;
class ISDRSIEClient;
//class CIGWLoaderDx9;
class CSDOAInterface;
class CCharacter;
class CClientInfoCollect;
class COpenLoginClient;


extern CInput		* g_pInput;             //DInput
extern CGameNet     * g_pNet;               //网络
extern CDownloadNet * g_pDownLoadNet;       //资源下载网络
extern CDownloadNet * g_pBgDownLoadNet;       //资源下载网络,后台限速下载地图等资源
extern CDownloadLimitSpeed * g_pDownloadLimitSpeed;       //资源下载网络,后台下载音效,字体等资源包
extern CCtrlMainWin * g_pControl;           //窗口
extern CGameMap     * g_pGameMap;           //地图信息
extern CGameData    * g_pGameData;          //数据
extern CGameControl * g_pGameControl;
extern CUiManager   * g_pUiMgr;             //UI管理器
extern CMagicCtrlMgr* g_pMagicCtrl;
extern ISDRSIEClient* g_pRainBowSpirt;		//新版彩虹帮助
//extern CIGWLoaderDx9* g_pSDOAInterface;
extern CSDOAInterface* g_pSDOAInterface;
extern CCharacter*    g_pSelf;
extern ISDSampler*    g_pSDSampler;//客户端信息收集接口
extern CClientInfoCollect* g_pClientInfoCollect;//客户端信息收集
extern COpenLoginClient* g_pOpenLoginClient;	// AP


