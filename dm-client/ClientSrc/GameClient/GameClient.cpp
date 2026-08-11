// GameClient.cpp : 定义应用程序的入口点。
#include "Global\Global.h"
#include "Global\DebugTry.h"
#include "Resource.h"
#include "Global\GlobalErr.h"
#include "Global\GlobalMsg.h"
#include "Global\PerfCheck.h"
#include "Global\ShareMem.h"
#include "Global\StringUtil.h"
#include "Global\ExceptionReport.h"
#include "Global/Interface/WeatherInterface.h"
#include "Global/Interface/LightInterface.h"
#include "BaseClass\Misc\Input.h"
#include "Global/Interface/StreamInterface.h"
#include "BaseClass\Control\Control.h"
#include "GameControl\GameControl.h"
#include "BaseClass\Misc\Net.h"
#include "Global/Interface/AudioInterface.h"
#include "WndClass\MainWnd\MainWnd.h"
#include "GameMap\GameMap.h"
#include "GameData\GameData.h"
#include "GameData/MsgBoxMgr.h"
#include "GameData/ConfigData.h"
#include "GameData/LoginData.h"
#include "WidgetManager.h"
#include "SDOAInterface.h"
#include "SDError.h"
#include "GameData/MagicCtrlMgr.h"
//#include "UsbClientSDK.h"
#include "BaseClass/Misc/Internet.h"
#include "Global/Standard3DES.h"
#include "Iphlpapi.h"
#include "Registry.h"
#include "ReplayManager.h"
#include "Global/md5.h"
#include "BaseClass/Misc/cliggproto.h"
#include "GameData/OtherData.h"
#include <shlwapi.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

//引入mimalloc
#include "mimalloc-new-delete.h"
#include "mimalloc-override.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"imm32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"ijl15l.lib")
#pragma comment(lib,"lua5.0.lib")
#ifndef WITHOUT_SD_INTERFACE
#pragma comment(lib,"SDError.lib")
#endif
//#pragma comment(lib,"UsbClient.lib")//usb密宝
#pragma comment(lib,"Iphlpapi.lib")

#ifdef _DEBUG
#pragma comment(lib,"tixmld.lib")
//#pragma comment(lib, "SDRSIEClientD.lib")
#else
#pragma comment(lib,"tixml.lib")
//#pragma comment(lib, "SDRSIEClient.lib")
#endif

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

#pragma comment(lib,"version.lib")
//#pragma comment(lib,"DES4Client.lib")

//#pragma comment(lib,"Psapi.Lib")
//内嵌到web页面
//#pragma comment(lib,"cgpfs.lib")

// VLD + lua5.0 兼容：旧 CRT 缺失符号的桩函数
// 注意：32位 __cdecl 自动在符号名前加 _，故函数名比链接器看到的名字少一个下划线
extern "C" {
	void __cdecl _lock(int) {}          // → 符号 __lock
	void __cdecl _unlock(int) {}        // → 符号 __unlock
	void* __cdecl __iob_func(void) {    // → 符号 ___iob_func
		static struct _iobuf { void* _Placeholder; } iob[3] = {};
		return iob;
	}
}

// 初始化 mimalloc, 替换系统内存分配器 - 基于 mimalloc v3.2.8 官方文档和最佳实践
struct MimallocInitializer {
	MimallocInitializer()
	{
		// 重置统计信息
		mi_stats_reset();

		// ============================================
		// 核心性能选项
		// ============================================

		// Arena 预提交内存：2=仅在 overcommit 系统上启用, 0=禁用, 1=总是启用
		// Windows 默认使用 2, 可以提升性能但会增加内存使用
		// MMORPG 服务端建议使用 2, 减少 VirtualAlloc 调用次数
		mi_option_set(mi_option_e::mi_option_arena_eager_commit, 2);

		// ============================================
		// 内存回收选项
		// ============================================

		// 内存回收延迟：延迟 N 毫秒后再回收内存
		// 0=立即回收, -1=不回收
		// MMORPG 服务端设置为 10000ms（10秒）, 玩家进出频繁, 避免频繁回收导致的性能抖动
		// 配合 purge_decommits=1, 10秒后真正释放内存, 平衡性能和内存使用
		mi_option_set(mi_option_e::mi_option_purge_delay, 10000);

		// 回收时是否解提交内存：1=解提交（释放给OS）, 0=重置内存但保留地址空间
		// 设置为 1 可以真正释放内存, 适合 7×24 小时长时间运行的服务器
		// 配合 purge_delay 延迟回收, 避免频繁系统调用
		mi_option_set(mi_option_e::mi_option_purge_decommits, 1);

		// 在 free 时回收 abandoned 页面：0=不允许, 1=允许（仅当页面来自当前 theap）, 其他=总是允许
		// MMORPG 多线程环境, 玩家对象可能在不同线程分配释放
		// 设置为 1 及时回收其他线程释放的内存, 减少跨线程内存占用
		mi_option_set(mi_option_e::mi_option_page_reclaim_on_free, 1);

		// ============================================
		// 内存保留选项
		// ============================================

		// Arena 初始保留大小（单位：KB）
		// 根据实际内存需求配置：
		// 每个进程独立 Arena, 避免相互影响
		mi_option_set(mi_option_e::mi_option_arena_reserve, 16 * 1024);

		// ============================================
		// NUMA 选项
		// ============================================

		// 使用 NUMA 节点：0=使用所有可用节点, N=最多使用 N 个节点
		// 分布式 MMORPG 服务端, 单机多进程架构, 使用所有 NUMA 节点最大化性能
		// 避免单个 NUMA 节点过载, 充分利用多核 CPU 资源
		mi_option_set(mi_option_e::mi_option_use_numa_nodes, 0);

		// ============================================
		// 大页选项
		// ============================================

		// 允许使用大页（2MB 或 4MB）：0=禁用, 1=启用
		// 大页可以提升 TLB 命中率, 但会增加内存粒度
		// MMORPG 服务端对象大小不固定, 建议禁用（0）, 避免内存浪费和碎片
		mi_option_set(mi_option_e::mi_option_allow_large_os_pages, 0);

		// 预留巨大页（1GB）：N=预留 N 个 1GB 页面
		// MMORPG 服务端一般不需要, 设置为 0
		// 巨大页适合大数据处理场景, 不适合游戏服务端的频繁小对象分配
		mi_option_set(mi_option_e::mi_option_reserve_huge_os_pages, 0);

		// 允许透明大页（THP）：0=禁用, 1=启用（Linux 特有功能）
		// Windows 不支持 THP, 此选项无效, 设置为 0 保持一致性
		// 避免让人误以为启用了某种大页功能
		mi_option_set(mi_option_e::mi_option_allow_thp, 0);

		// ============================================
		// 页面管理选项
		// ============================================

		// 保留的满页数：每个 size class 保留 N 个满页（默认=2）
		// MMORPG 服务端大量玩家对象、怪物、物品频繁创建销毁
		// 提高到 6, 减少页面分配/释放开销, 提升分配性能
		// 内存影响：约 50 size class × 8 × 4KB = 1.6MB, 可忽略不计
		mi_option_set(mi_option_e::mi_option_page_full_retain, 8);

		// 最大候选页面数：分配时考虑的最大候选页面数（默认=4）
		// MMORPG 服务端对象大小相对稳定（玩家、物品等）
		// 保持 4 即可, 在性能和搜索开销间取得最佳平衡
		mi_option_set(mi_option_e::mi_option_page_max_candidates, 4);

		// 最小回收大小：设置最小回收大小（单位：KB）
		// 0=无限制, 允许回收任意大小的页面
		// MMORPG 服务端有大量小对象（装备、道具、技能效果）, 设置为 0 及时释放内存
		// 配合 purge_delay=10000ms, 避免频繁系统调用, 防止碎片积累
		mi_option_set(mi_option_e::mi_option_minimal_purge_size, 0);

		// ============================================
		// 高级选项
		// ============================================

		// Arena 最大对象大小：Arena 中可分配的最大对象大小（单位：KB）
		// 64位系统默认 2GB, MMORPG 服务端保持默认即可
		// 足够处理大地图数据、玩家数据库等大对象
		mi_option_set(mi_option_e::mi_option_arena_max_object_size, 2 * 1024 * 1024);  // 2GB

		// 允许访问 abandoned 的 heap 块：0=不允许, 1=允许
		// 生产环境设置为 0, 提升安全性和性能
		// 用于调试时可以启用 1, 检测内存访问问题
		mi_option_set(mi_option_e::mi_option_visit_abandoned, 0);

		// 通用回收频率：每 N 次通用分配调用回收一次 heap（默认=10000）
		// MMORPG 服务端设置为 5000, 提高回收频率, 减少内存碎片
		// 高频分配场景下, 更频繁的回收可以保持内存健康状态
		mi_option_set(mi_option_e::mi_option_generic_collect, 5000);

		// ============================================
		// 调试选项（生产环境建议关闭）
		// ============================================

		// 显示错误信息：0=不显示
		mi_option_set(mi_option_e::mi_option_show_errors, 0);

		// 显示统计信息：0=不显示（生产环境）
		// 开发调试时可临时启用, 观察内存使用情况
		mi_option_set(mi_option_e::mi_option_show_stats, 0);

		// 详细输出：0=关闭
		mi_option_set(mi_option_e::mi_option_verbose, 0);

		// ============================================
		// 安全选项
		// ============================================

		// 内存重试时间（毫秒）：内存不足时重试的时间, 仅 Windows
		// MMORPG 服务端实时性要求高, 设置为 0 禁用重试
		// 避免内存分配失败时阻塞线程导致游戏卡顿, 快速失败并重启更安全
		// 配合外部监控系统（如进程守护）, 自动重启崩溃的服务器
		mi_option_set(mi_option_e::mi_option_retry_on_oom, 0);
	}
} g_mimalloc_init;


using namespace SGDP;

// Win32消息处理函数声明
LRESULT CALLBACK	WndProcStart(HWND,UINT,WPARAM,LPARAM);			// 游戏主程序处理函数
LRESULT CALLBACK	WndProc(HWND,UINT,WPARAM,LPARAM);			// 游戏主程序处理函数
LRESULT CALLBACK	ExcepDlgProc(HWND,UINT,WPARAM,LPARAM);		// 异常处理对话框处理函数
void  AnalyzeCommandLine();
DWORD GetGameVersion(const char * p);
int GetTexCacheMemorySize(int * pMode);
void WINAPI SetAudioSoundVolume(DWORD NewVolume);
DWORD WINAPI GetAudioSoundVolume();
void WINAPI SetAudioEffectVolume(DWORD NewVolume);
DWORD WINAPI GetAudioEffectVolume();

// 多个游戏运行时焦点相关
BOOL		a_bRestored				= FALSE;	// 是否是复员状态
DWORD		a_dwSetFocus			= 0;		// 设置为焦点的时间

// 启动窗口
HWND        a_hWndStart             = NULL;     // 启动窗口句柄
HBITMAP     a_hBmp                  = NULL;     // 启动窗口位图

// 游戏变量
HWND		a_hWnd					= NULL;		// 窗口句柄
HINSTANCE	a_hInst					= NULL;		// 当前实例
TCHAR		a_szTitle[256]			= {0};		// 标题栏文本
TCHAR       a_szWindowClass[100]	= {0};      // 主窗口类名
int         a_iW = 0,a_iH = 0,a_iDisMode = 0;


BOOL		a_bPerf					= FALSE;
BOOL		a_bFps					= FALSE;
DWORD		a_dwLastShowCheckTime	= 0;
DWORD		a_dwLastPerfCheckTime	= 0;
DWORD       a_dwLastChecParentWndTime = 0;//上一次检查父窗口时间

// 带参数运行游戏
BOOL		a_bAutoGo				= FALSE;
int			a_nEnableOO				= 0;
int			a_nFollowIn				= 0;
int			a_sGatePort				= 0;
char		a_sGateIP[16]			= {0};
char		a_sAreaName[64]			= {0};
char		a_sServerName[32]		= {0};
char		a_sUser[32]				= {0};
char		a_sPassWord[32]			= {0};


SGDP::ISDError* g_pSDError = NULL;

//像vector取下标[]越界等不会抛出任何异常,直接crash,但可以_set_invalid_parameter_handler,自行处理参数非法的情况
void myInvalidParameterHandler(const wchar_t* expression,const wchar_t* function,const wchar_t* file,unsigned int line,uintptr_t pReserved)
{
	/////////////////
	if(expression && function && file)
	{
		wchar_t wstr[512] = {0};
		char tmp[1024] = {0};
		swprintf(wstr,L"Invalid parameter detected in function %s.File: %s Line: %d,Expression: %s", function, file, line,expression);
		int iLen = WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,wstr,wcslen(wstr),tmp,1023,NULL,NULL);
		throw exception(tmp);
	}
	else
	{
		throw exception("invalid_parameter_handler:参数非法");
	}
}


BOOL CreateStartWnd(HINSTANCE hInstance)
{
	a_hWndStart = NULL;
	WNDCLASSEX wcex;
	char szWindowClassStart[100] = {0};
	strcpy(szWindowClassStart,"WSSTART");

	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProcStart;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_GAMECLIENT);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClassStart;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_GAMECLIENT);

	RegisterClassEx(&wcex);

	int w = 381;
	int h = 372;
	HWND hWnd = GetDesktopWindow();
	RECT rc = {0,0,800,600};
	if(hWnd)
		GetWindowRect(hWnd,&rc);


	a_hWndStart = CreateWindow(szWindowClassStart, "传奇世界", WS_POPUP|WS_BORDER, (rc.right-w)/2, (rc.bottom-h)/2, w, h, NULL, NULL, hInstance, NULL);
	if(!a_hWndStart)
		return FALSE;

	//嵌入页面不要显示启动中窗口,但是要创建这个窗口,不然输入法无法加载,无法切换输入法
	if (g_hParentWnd)
	{
		MoveWindow(a_hWndStart,-1000,-1000,0,0,FALSE);
	}
	else
	{
		ShowWindow(a_hWndStart, SW_SHOW);
	}

	UpdateWindow(a_hWndStart);

	return TRUE;
}

// 游戏主窗口
BOOL CreateGameWnd(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize				= sizeof(WNDCLASSEX); 
	wcex.style				= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc		= (WNDPROC)WndProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hInstance;
	wcex.hIcon				= LoadIcon(hInstance, (LPCTSTR)IDI_GAMECLIENT);
	wcex.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground		= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName		= NULL;
	wcex.lpszClassName		= a_szWindowClass;
	wcex.hIconSm			= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_GAMECLIENT);

	RegisterClassEx(&wcex);


	if(IsWindow(g_hParentWnd))
	{		
		RECT rc;
		GetClientRect(g_hParentWnd,&rc);
		a_hWnd = CreateWindow(a_szWindowClass,a_szTitle,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rc.left,rc.top,rc.right,rc.bottom,g_hParentWnd,NULL,hInstance,NULL);
	}
	else	
	{
		a_hWnd = CreateWindow(a_szWindowClass,a_szTitle,WS_POPUP|WS_CLIPCHILDREN,0,0,800,600,NULL,NULL,hInstance,NULL);
		g_hParentWnd = NULL;
	}


	//a_hWnd = CreateWindow(a_szWindowClass,a_szTitle,WS_POPUP|WS_CLIPCHILDREN,0,0,800,600,NULL,NULL,hInstance,NULL);
	//a_hWnd = CreateWindow(a_szWindowClass,a_szTitle,WS_OVERLAPPED|WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW|WS_VISIBLE ,0,0,a_iW,a_iH,NULL,NULL,hInstance,NULL);
	if(!a_hWnd)
		return FALSE;

	ShowWindow(a_hWnd,SW_SHOW);
	UpdateWindow(a_hWnd);

	return TRUE;
}

BOOL AfterCreateGameWnd(void)
{
	// 检查DX版本
	g_iDXVersion = 0x888888;
	HMODULE	hDXdll = LoadLibrary("..\\GetDXV.dll");
	if(hDXdll)
	{
		typedef int (__stdcall * lpfnGetDXV) (void);
		lpfnGetDXV GetDXV = (lpfnGetDXV)GetProcAddress(hDXdll,"_GetDXV@0");
		if(GetDXV)
			g_iDXVersion = GetDXV();
		FreeLibrary(hDXdll);
	}  

	return TRUE;
} 

BOOL LoadConfig(int &iW,int &iH,int &iDisMode)
{
	iW				= 800;
	iH				= 600;
	iDisMode		= DM_WINDOWED;

	char* sRet = g_pStreamMgr->GetConfigStr("FullScreen","Yes");
	if(stricmp(sRet,"Yes") == 0)
		iDisMode = DM_FULL_FALSE;
	else
		iDisMode = DM_WINDOWED;

	sRet = g_pStreamMgr->GetConfigStr("AlertException","No");
	if(stricmp(sRet,"Yes") == 0)
		g_bAlertException = true;
	else
		g_bAlertException = false;

	//sRet = g_pStreamMgr->GetConfigStr("SaveLog","No");
	//if(stricmp(sRet,"Yes") == 0)
	//{
	//	if (g_pFileSaveLog == NULL)
	//	{
	//		g_pFileSaveLog = fopen("../clientlog.txt","a+b");
	//	}
	//}
	//else
	//{
	//	g_pFileSaveLog = NULL;
	//}

#ifdef _DEBUG
	g_bAlertException = true;
#endif

	g_Config.SetAllWndMode(iDisMode == DM_WINDOWED);

	int iRet = g_pStreamMgr->GetConfigInt("ScreenMode",800);
	if(iRet == 2560)
	{
		iW = 2560;
		iH = 1440;
	}
	else if(iRet == 1920)
	{
		iW = 1920;
		iH = 1080;
	}
	else if(iRet == 1600)
	{
		iW = 1600;
		iH = 900;
	}
	else if(iRet == 1280)
	{
		iW = 1280;
		iH = 800;
	}
	else if(iRet == 1024)
	{
		iW = 1024;
		iH = 768;
	}

    g_Config.SetWindowWidth(iRet);

	iRet = g_pStreamMgr->GetConfigInt("ColorDepth",16);
	if( iRet == 16 )
		g_Config.SetRGB(0);
	else
		g_Config.SetRGB(1);

	//sRet = g_pStreamMgr->GetConfigStr("VSync","Yes");
	//if( stricmp(sRet,"Yes") == 0 )
	//	g_Config.SetVSync(true);
	//else
	//	g_Config.SetVSync(false);

	sRet = g_pStreamMgr->GetConfigStr("HalfSpeed","No");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetHalfSpeed(true);
	else
		g_Config.SetHalfSpeed(false);

	if( g_Config.GetHalfSpeed() )
	{
		g_Config.SetSmooth(0);
	}
	else
	{
		iRet = g_pStreamMgr->GetConfigInt("Smooth",0);
		if( iRet == 1 )
			g_Config.SetSmooth(1);
		else if( iRet == 2 )
			g_Config.SetSmooth(2);
		else if( iRet == -1 )
			g_Config.SetSmooth(-1);
		else
			g_Config.SetSmooth(0);
	}

	// 读取初始设置
	sRet = g_pStreamMgr->GetConfigStr("Shadow2D","No");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.Set2DShadow(true);
	else
		g_Config.Set2DShadow(false);

	g_Config.SetTrans(g_pStreamMgr->GetConfigInt("Trans",0));
	DWORD dwRet = g_pStreamMgr->GetConfigInt("Trans",0);
	if(dwRet < 0 )  dwRet = 0;
	if(dwRet > 50)  dwRet = 50;
	dwRet = (( (50-dwRet+25) * 0xFF / 75 ) << 24) | 0x00FFFFFF;
	g_Config.SetTransColor(dwRet);

	sRet = g_pStreamMgr->GetConfigStr("Sound","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetSndOn(true);
	else
		g_Config.SetSndOn(false);

	g_Config.SetSndVol(g_pStreamMgr->GetConfigInt("SoundVol",80));
	if(g_pAudio && g_Config.GetSndVol() > 0)
		g_pAudio->SetVolumeSound(g_Config.GetSndVol());

	sRet = g_pStreamMgr->GetConfigStr("BkSound","Yes");
	if( stricmp(sRet,"Yes") == 0 )
	{
		g_Config.SetBkMusicOn(true);
		g_pAudio->EnableBkSound(true);
	}
	else
	{
		g_Config.SetBkMusicOn(false);
		g_pAudio->EnableBkSound(false);
	}

	g_Config.SetBkMusicVol(g_pStreamMgr->GetConfigInt("BkSoundVol",80));
	if(g_Config.GetBkMusicVol() > 0)
		g_pAudio->SetVolumeMusic(g_Config.GetBkMusicVol());
	
	sRet = g_pStreamMgr->GetConfigStr("Weather","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetWeather(1);
	else
		g_Config.SetWeather(0);

	sRet = g_pStreamMgr->GetConfigStr("ShowHP","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetFloodOn(true);
	else
		g_Config.SetFloodOn(false);

	sRet = g_pStreamMgr->GetConfigStr("RainbowOn","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetRainbowOn(true);
	else
		g_Config.SetRainbowOn(false);

	sRet = g_pStreamMgr->GetConfigStr("Light","Yes");
	if( stricmp(sRet,"Yes") == 0 )
	{
		g_Config.SetLight(1);
//		g_pLight->EnableLight(true);
	}
	else
	{
		g_Config.SetLight(0);
//		g_pLight->EnableLight(false);
	}

	sRet = g_pStreamMgr->GetConfigStr("SelfAlpha","No");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetSelfAlpha(true);
	else
		g_Config.SetSelfAlpha(false);

	sRet = g_pStreamMgr->GetConfigStr("AutoPath","No");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetAutoPath(true);
	else
		g_Config.SetAutoPath(false); 

	sRet = g_pStreamMgr->GetConfigStr("SkipCheck","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetSkipCheck(true);
	else
		g_Config.SetSkipCheck(false);

	sRet = g_pStreamMgr->GetConfigStr("ColorHP","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetColorHP(true);
	else
		g_Config.SetColorHP(false);

	sRet = g_pStreamMgr->GetConfigStr("CheckQuick","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetCheckQuick(true);
	else
		g_Config.SetCheckQuick(false);

	sRet = g_pStreamMgr->GetConfigStr("Chase","Yes");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetChase(true);
	else
		g_Config.SetChase(false);

	sRet = g_pStreamMgr->GetConfigStr("GM","No");
	if( stricmp(sRet,"Yes") == 0 )
		g_Config.SetGM(true);
	else
		g_Config.SetGM(false);

	g_pAudio->EnableSound(g_Config.GetSndOn());

	sRet = g_pStreamMgr->GetConfigStr("SelfCursor","No");
	if( stricmp(sRet,"Yes") == 0 )
	{
		g_pGameData->EnableSelfCursor(true);
		g_Config.SetMouse(TRUE); 
	}
	else
	{
		g_pGameData->EnableSelfCursor(false);
		g_Config.SetMouse(FALSE);
	}

	sRet = g_pStreamMgr->GetConfigStr("AutoShowActivityLog","No");
	if( stricmp(sRet,"Yes") == 0 )
	{
		g_Config.SetAutoShowActivityLog(TRUE);
	}
	else
	{
		g_pGameData->EnableSelfCursor(false);
		g_Config.SetMouse(FALSE);
	}

	int iMemory = GetTexCacheMemorySize(&g_iPreReadMode);
	switch( g_iPreReadMode )
	{
	case PREREAD_NONE:
	case PREREAD_DEBUG:
		g_Config.SetTexPrepare(0);
		break;
	case PREREAD_AUTO:
		g_Config.SetTexPrepare(1);
		break;
	case PREREAD_FULL:
		g_Config.SetTexPrepare(2);
		break;
	default:
		g_Config.SetTexPrepare(2);
		break;
	}
	iMemory = g_pStreamMgr->GetConfigInt("MaxTexPoolCacheSize",iMemory);
	g_pTexMgr->SetMaxCacheSize(iMemory);

	//字体设置
	float fWeight = float(atof(g_pStreamMgr->GetConfigStr("FontWeight","1.5")));
	float fContrast = float(atof(g_pStreamMgr->GetConfigStr("FontContrast","1.5")));
	g_pFont->SetFontConfig(fWeight,fContrast);


	////自动合并矿石等,内部使用
	//sRet = g_pStreamMgr->GetConfigStr("AutoCombine","No");
	//if( stricmp(sRet,"Yes") == 0 )
	//{
	//	g_Config.SetAutoCombine(true); 
	//}
	//else
	//{
	//	g_Config.SetAutoCombine(false); 
	//}


	return TRUE;
}

int GetTexCacheMemorySize(int * pMode)
{
	MEMORYSTATUS ms;
	int i;

	GlobalMemoryStatus(&ms);
	i = ms.dwTotalPhys / (1024*1024);

	// 根据内存量的不同设置不同的纹理缓冲区大小
	if( i > 600)
		i = 125;
	else if( i > 500)
		i = 80;
	else if( i > 200)
		i = 60;
	else if( i > 138)
		i = 50;
	else if( i > 100)
		i = 40;
	else
		i = i / 3;

	char* sRet = g_pStreamMgr->GetConfigStr("MemoryCost","Auto");
	if( stricmp(sRet,"normal") == 0 )
	{
		i = (int)((float)i * 0.8f);
		if(pMode) *pMode = PREREAD_FULL;
	}
	else if( stricmp(sRet,"auto") == 0 )
	{
		i = (int)((float)i * 0.8f);
		if(pMode) *pMode = PREREAD_AUTO;
	}
	else if( stricmp(sRet,"large") == 0 )
	{
		i = i;
		if(pMode) *pMode = PREREAD_FULL;
	}
	else if( stricmp(sRet,"small") == 0 )
	{
		i = (int)((float)i * 0.6f);
		if(pMode) *pMode = PREREAD_FULL;
	}
	else if( stricmp(sRet,"debug") == 0 )
	{
		i = (int)((float)i * 0.8f);
		i = min( i , 128);
		if(pMode) *pMode = PREREAD_DEBUG;
	}
	else 
	{
		i = (int)((float)i * 0.8f);
		if(pMode) *pMode = PREREAD_NONE;
	}

	i = min(i,150);

	return i;
}

//退出通知
void AppExitEventCall()
{
	//退出游戏	
	output_debug("AppExitEventCall");
	PostMessage(g_hWnd,WM_DESTROY,0,0);
}

//全屏/恢复通知
void FullScreenEventCall(BOOL bFull)
{
	if (!g_hParentWnd || !g_pGfx)
	{
		return;
	}

	RECT rc;
	GetClientRect(g_hParentWnd,&rc);
	int iWidth = rc.right - rc.left;
	int Height = rc.bottom - rc.top;

	output_debug("FullScreenEventCall:%d[%d,%d]",bFull,iWidth,Height);

	ChangeWindowSize(g_pGfx->GetWidth(),g_pGfx->GetHeight(),bFull?DM_FULL_FALSE:DM_WINDOWED);
}

//尺寸变化通知
void SizeEventCall(SIZE size)
{
	if (!g_hParentWnd || !g_pGfx)
	{
		return;
	}

	output_debug("SizeEventCall[%d,%d]",size.cx,size.cy);

	ChangeWindowSize(size.cx,size.cy,g_pGfx->GetDisMode());	
}




bool ChangeWindowSize(int iWidth,int iHeight,DisplayMode eNewDisMode)
{
	int iOldWidth = g_pGfx->GetWidth();
	int iOldHeight = g_pGfx->GetHeight();
	DisplayMode eOldDisMode = g_pGfx->GetDisMode();


	if (eNewDisMode == DM_FULL_FALSE || eNewDisMode == DM_FULL_TRUE)
	{
		if (!((iWidth == 800 && iHeight == 600) || (iWidth == 1024 && iHeight == 768) || (iWidth == 1280 && iHeight == 800) || (iWidth == 1600 && iHeight == 900) || (iWidth == 1920 && iHeight == 1080) || (iWidth == 2560 && iHeight == 1440)))
		{
			//iWidth = iOldWidth;
			//iHeight = iOldHeight;
			iWidth = 1024;
			iHeight = 768;
		}
	}
	else if (g_hParentWnd)
	{
		RECT rc;
		GetClientRect(g_hParentWnd,&rc);
		iWidth = rc.right - rc.left;
		iHeight = rc.bottom - rc.top;
	}
	else if (a_iDisMode == DM_WINDOWED)
	{
		int iRetWidth;
		int iRetHeight;
		g_pGfx->RecomputeWindowXY(iRetWidth,iRetHeight, iWidth,iHeight,(DisplayMode)a_iDisMode);
		if ((iRetWidth != iWidth) || (iRetHeight != iHeight))
		{
			g_Config.SetWindowWidth(iOldWidth);
			if (g_pControl)
				g_MsgBoxMgr.PopSimpleAlert("您的显示器无法设置此分辨率！");
			return false;
		}

		iWidth = iRetWidth;
		iHeight = iRetHeight;
	}
	

	g_pGfx->SetWidth(iWidth);
	g_pGfx->SetHeight(iHeight);
	g_pGfx->SetDisMode(eNewDisMode);
	if(!g_pGfx->RestoreDesktop(false))
	{
		g_pGfx->SetWidth(iOldWidth);
		g_pGfx->SetHeight(iOldHeight);
		g_pGfx->SetDisMode(eOldDisMode);
		g_Config.SetWindowWidth(iOldWidth);

		if (g_pControl)
			g_MsgBoxMgr.PopSimpleAlert("分辨率设置失败！");
		//MessageBox(g_hWnd,"分辨率设置失败.","提示",MB_OK);
		return false;
	}

	if(eNewDisMode == DM_WINDOWED)
	{
		g_pStreamMgr->SetConfigStr("FullScreen","No");
		g_Config.SetAllWndMode(true);
	}
	else
	{
		g_pStreamMgr->SetConfigStr("FullScreen","Yes");
		g_Config.SetAllWndMode(false);
	}

	if ((iWidth == 800 && iHeight == 600) || (iWidth == 1024 && iHeight == 768) || (iWidth == 1280 && iHeight == 800) || (iWidth == 1600 && iHeight == 900) || (iWidth == 1920 && iHeight == 1080) || (iWidth == 2560 && iHeight == 1440))
	{
		g_pStreamMgr->SetConfigInt("ScreenMode",iWidth);
		g_Config.SetWindowWidth(iWidth);
	}


	if (eNewDisMode == DM_FULL_FALSE || eNewDisMode == DM_FULL_TRUE)
	{
		if (g_hParentWnd)
		{
			SetParent(g_hWnd,NULL);
		}

		g_bNeedScale = false;
		g_ScaleRate.fx = g_ScaleRate.fy = 1.0f;
	}
	else
	{
		if (!((iWidth == 800 && iHeight == 600) || (iWidth == 1024 && iHeight == 768) || (iWidth == 1280 && iHeight == 800) || (iWidth == 1600 && iHeight == 900) || (iWidth == 1920 && iHeight == 1080) || (iWidth == 2560 && iHeight == 1440)))
		{
			g_bNeedScale = true;
			g_ScaleRate.fx = (float(iWidth)/800);
			g_ScaleRate.fy = (float(iHeight)/600);
		}
		else
		{
			g_bNeedScale = false;
			g_ScaleRate.fx = g_ScaleRate.fy = 1.0f;
		}

		if (g_hParentWnd)
		{
			SetParent(g_hWnd,g_hParentWnd);
		}
	}


	g_pGfx->AdjustWindow(g_hWnd,iWidth,iHeight,eNewDisMode == DM_FULL_FALSE);
	g_pControl->ReSize(iWidth,iHeight);


	g_pControl->ReSetAllWndPos();

	//香炉新点
	if (g_OtherData.GetCenserStartTime() > 0)
	{
		g_pMagicCtrl->FinishMagicByID(MAGICID_SENCER_EFFECT);
		g_OtherData.SetCenserStart(true);
	}

	if (g_pSDOAInterface)
	{
		g_pSDOAInterface->SetLogicScreen(iWidth,iHeight);

		POINT pt;
		pt.x = g_pGfx->GetWidth() - 305; pt.y = 0;
		g_pSDOAInterface->SetTaskBarPosition(&pt);		
	}

	//如果有父窗口,并且有进行全屏与窗口切换,DIRECTSOUND8要重初始化,不然会没有声音
	if (g_hParentWnd && eOldDisMode != eNewDisMode && g_pAudio)
	{
		g_pAudio->ReCreateSoundManager();
	}

	return true;
}


BOOL InitGame(void)
{
TRY_BEGIN

	// 初始化随机种子
#ifdef ENABLE_REPLAY
	if(g_ReplayManager.IsInReplay())
	{
		srand(g_ReplayManager.GetStartTime());
		//为了使时间和录像当时的一致,这里通过改变g_dwServerTime来实现
		__time32_t tNow;
		_time32(&tNow);
		g_dwServerTime = g_ReplayManager.GetStartTime() - tNow;
	}
	else
#endif
	{
		__time32_t tNow;
		_time32(&tNow);
#ifdef ENABLE_REPLAY
		g_ReplayManager.SaveStartTime(tNow);
#endif
		srand(tNow);
	}


	if(!InitGlobalObject(a_hWnd))
		return FALSE;

	if(!LoadConfig(a_iW,a_iH,a_iDisMode))
		return FALSE;
	

#ifndef WITHOUT_SD_INTERFACE
 	LoadLibrary("SDError.dll");
 	g_pSDError = SDErrorGetModule(&(SDERROR_VERSION));
#endif



	if (g_hCgpfsDLL && g_hParentWnd)
	{
		RECT rc;
		GetClientRect(g_hParentWnd,&rc);
		a_iW = rc.right - rc.left;
		a_iH = rc.bottom - rc.top;

		//告诉插件游戏已经启动
		LPCgpSetupAppWindow lpSetupAppWin = (LPCgpSetupAppWindow)GetProcAddress(g_hCgpfsDLL,"CgpSetupAppWindow");
		if (lpSetupAppWin)
		{
			lpSetupAppWin(g_hWnd,1);
		}
		//设置通知回调
		LPCgpListenAppExitEvent lpSetExitEvent = (LPCgpListenAppExitEvent)GetProcAddress(g_hCgpfsDLL,"CgpListenAppExitEvent");
		if (lpSetExitEvent)
		{
			lpSetExitEvent(AppExitEventCall);
		}

		LPCgpListenFullscreenEvent lpSetFullscreenEvent = (LPCgpListenFullscreenEvent)GetProcAddress(g_hCgpfsDLL,"CgpListenFullscreenEvent");
		if (lpSetFullscreenEvent)
		{
			lpSetFullscreenEvent(FullScreenEventCall);
		}

		LPCgpListenWindowSizeEvent lpSetWinSizeEvent = (LPCgpListenWindowSizeEvent)GetProcAddress(g_hCgpfsDLL,"CgpListenWindowSizeEvent");
		if (lpSetWinSizeEvent)
		{
			lpSetWinSizeEvent(SizeEventCall);
		}

		output_debug("W:%d,H:%d\r\n",a_iW,a_iH);


		a_iDisMode = DM_WINDOWED;
		g_pStreamMgr->SetConfigStr("FullScreen","No");
	}
	else if (a_iDisMode == DM_WINDOWED)
	{		
		g_pGfx->RecomputeWindowXY(a_iW,a_iH, a_iW,a_iH,(DisplayMode)a_iDisMode);
		g_pStreamMgr->SetConfigInt("ScreenMode",a_iW);
		g_Config.SetWindowWidth(a_iW);
	}
	
	// CreateD3D
	if(!g_pGfx->CreateD3D(a_hWnd,a_iW,a_iH,(DisplayMode)a_iDisMode))
	{
		MessageBox(a_hWnd,"CreateDevice错误，确认显卡是否支持D3D加速！可\n能可以通过尝试重装显卡驱动或者DirectX解决！",NULL,0);
		return FALSE;
	}

	// 加载截包工具
	//LoadLibrary("../pview/PV.dll");

	g_pGfx->SetFPS(50);

	// CControl
	g_pControl = new CMainWnd;
	g_pControl->Create(a_iW,a_iH);

	if (g_hParentWnd)
	{
		//不是标准的大小要缩放某些控件
		if (!((a_iW == 800 && a_iH == 600) || (a_iW == 1024 && a_iH == 768) || (a_iW == 1280 && a_iH == 800) || (a_iW == 1600 && a_iH == 900) || (a_iW == 1920 && a_iH == 1080) || (a_iW == 2560 && a_iH == 1440)))
		{
			g_bNeedScale = true;
			g_ScaleRate.fx = (float(a_iW)/800);
			g_ScaleRate.fy = (float(a_iH)/600);
		}
	}


	// 创建窗口
	g_pControl->Msg(MSG_CTRL_LOGINWND,OPER_CREATE);

	// CWeather
	g_pWeather->Create(a_iW,a_iH);

	// CInput 
	g_pInput->EnableCharMsg();
	g_pInput->EnableMoveMsg();
	g_pInput->EnableDClickMsg();
	g_pInput->Acquire();

	//// CLight
	g_pLight->CreateLight();
	g_pLight->SetLightColor(0xFFFFFFFF);

	g_WidgetMgr.Init();

	if(g_Login.GetAutoLoginInType() == 0)//
	{
	    // 开启多个游戏的时候的提示
		int vApps[3];
		g_ShareMem.GetShare(vApps,sizeof(vApps));

#ifndef _DEBUG
		if(vApps[0] > 1)
		{
#ifdef ENABLE_REPLAY
			if (!g_ReplayManager.IsInReplay())
#endif
			{
				g_MsgBoxMgr.PopSimpleAlert("由于您开启多个游戏程序，将有可能会影响游戏正常运行速度，建议您关闭其他无关程序以保持游戏正常运行速度！");
			}
		}
#endif
	}

	return TRUE;
TRY_END_RETURN(FALSE);
}



void DeleteGame()
{
 	if(g_pSDError)
 	{
 		g_pSDError->Release();
 		g_pSDError = NULL;
 	}
	g_WidgetMgr.Release();
	g_pGfx->KillD3D();

	if (g_pSDOAInterface)
	{
		g_pSDOAInterface->Logout();
		g_pSDOAInterface->CloseIGW();
	}

	if (g_hCgpfsDLL)
	{
		LPCgpUninitialize lpUninit = (LPCgpUninitialize)GetProcAddress(g_hCgpfsDLL,"CgpUninitialize");
		if (lpUninit)
		{
			lpUninit();
		}
		//FreeLibrary(g_hCgpfsDLL);
	}


	DeleteGlobalObject();
}

void ShowDetail(void)
{

	if(g_pGfx->IsNoDraw())
		return;

	char str[256] = {0};

	// 显示帧速
	if(a_bFps)
	{
		sprintf(str,"固定纹理池: %u/%u 缓冲纹理池: %u/%u\n",(DWORD)g_pTexMgr->GetCurFixedSize(),(DWORD)g_pTexMgr->GetMaxFixedSize(),
			(DWORD)g_pTexMgr->GetCurCacheSize(),(DWORD)g_pTexMgr->GetMaxCacheSize());

		g_pFont->DrawText(g_pGfx->GetWidth()/2,10,str,0xFF00FF00,FONT_DEFAULT,12,DTF_BlackFrame|DTF_Center);

		sprintf(str,"FPS:%d",g_pGfx->GetFPS());
		g_pFont->DrawText(g_pGfx->GetWidth()-g_pFont->GetLen(str)-10,10,str,0xFFFFFFFF,FONT_DEFAULT,12,DTF_BlackFrame);
	}

	// 显示性能
	if(a_bPerf)
	{
		for(int i = 0;i < g_PerfCheck.GetResultNums();i++)
		{
			PerfResult& pr = g_PerfCheck.GetResult(i);

			memset(str,' ',pr.dwLevel * 2);
			sprintf(str + pr.dwLevel * 2,"%-16s %d %-8d(%0.2f%%)",pr.sName,pr.bActived,pr.dwTimeCount,pr.fPercent);
			g_pFont->DrawText(100,100 + 12 * i,str,0xFFFFFFFF,FONT_DEFAULT,12,0,0x80000000);
		}
	}
}

DWORD GetGameVersion(const char * p)
{
	TCHAR   szFullPath[MAX_PATH];   
	DWORD   dwVerInfoSize = 0;   
	DWORD   dwVerHnd;   
	VS_FIXEDFILEINFO*  pFileInfo;

	DWORD   dwVersion = 0x0;

	GetModuleFileName(NULL,   szFullPath,   sizeof(szFullPath));   
	dwVerInfoSize  =  GetFileVersionInfoSize(szFullPath, &dwVerHnd);   
	if(dwVerInfoSize) 
	{   
		HANDLE     hMem;   
		LPVOID     lpvMem;   
		unsigned   int   uInfoSize   =   0;   

		hMem   =   GlobalAlloc(GMEM_MOVEABLE,   dwVerInfoSize);   
		lpvMem   =   GlobalLock(hMem);   
		GetFileVersionInfo(szFullPath,   dwVerHnd,   dwVerInfoSize,   lpvMem);   

		::VerQueryValue(lpvMem,   (LPTSTR)("\\"),   (void**)&pFileInfo,   &uInfoSize);   

		WORD   nProdVer[4];
		nProdVer[0]   =   HIWORD(pFileInfo->dwProductVersionMS);     
		nProdVer[1]   =   LOWORD(pFileInfo->dwProductVersionMS);   
		nProdVer[2]   =   HIWORD(pFileInfo->dwProductVersionLS);   
		nProdVer[3]   =   LOWORD(pFileInfo->dwProductVersionLS);

		for(int ii = 0;ii < 4;ii++)
		{
			dwVersion = (dwVersion << 8) + (BYTE)nProdVer[ii];
		}

		WORD dwVersionTail = HIWORD(pFileInfo->dwFileVersionMS);
		if(dwVersionTail == 0)
		{
			sprintf(g_strVersion,"%d.%d.%d.%d",nProdVer[0],nProdVer[1],nProdVer[2],nProdVer[3]);

		}
		else
		{
			sprintf(g_strVersion,"%d.%d.%d.%d.%d",nProdVer[0],nProdVer[1],nProdVer[2],nProdVer[3],dwVersionTail);
		}

		g_dwBuildID = pFileInfo->dwFileVersionLS;

		GlobalUnlock(hMem);   
		GlobalFree(hMem);
	}
	else
	{
		strcpy(g_strVersion,p);
		VString vStr = StringUtil::split(p,".");
		for(int ii = 0; ii < vStr.size() && ii < 4;ii++)
		{
			dwVersion |= atoi(vStr[ii].c_str()) << ((3 - ii) * 8);
		}
	}

	sprintf(a_szTitle,"传奇世界 v%s",g_strVersion);

	return dwVersion;
}



void AnalyzeCommandLine()
{
	static char* key_name[8] = 
	{
		"-user:",
		"-password:",
		"-token:",
		"-parent:",
		"-ip ",
		"-port ",
		"-dist ",
		"-explogin"
	};

	output_debug("%s",g_strCmdLine);

	string strGameIniPath = GetGameDataDir();
#ifdef ENABLE_REPLAY
	if (g_ReplayManager.IsInReplay())
	{
		strGameIniPath += "\\";
		strGameIniPath += g_strReplayDir;
		strGameIniPath += "\\game.ini";
	}
	else
#endif
	{
		strGameIniPath += "\\game.ini";
	}
	//-ip 123.122.111.111 -port 7000 -dist ws_1[channelname:2133] ********
	//[channelname]对于官服是没有的,是指其它AP的名字,和Oauth.xml对应

	char temp[512] = {0};

	for(int ii  = 0;ii < 8;ii++)
	{
		memset(temp,0,512);
		char* key = key_name[ii];
		char* pdest = strstr(g_strCmdLine,key); //查找这个Key
		if(pdest == NULL)
			continue;

		pdest += strlen(key);

		char* pend = strstr(pdest," ");
		if(pend)
			strncpy(temp,pdest,(int)(pend - pdest));
		else
			strcpy(temp,pdest);

		switch(ii)
		{
		case 0:
			g_Login.GetAutoUser().assign(temp);
			break;
		case 1:
			g_Login.GetAutoPassword().assign(temp);
			break;
		case 2:
			{
				std::string s = g_pNet->NewDecode(temp,strlen(temp));
				std::vector<std::string> VStr;

				int pos = s.find_first_of('/');
				while (pos != -1)
				{
					std::string strSub = s.substr(0,pos);			           
					VStr.push_back(strSub);
					s = s.substr(pos+1);
					pos = s.find_first_of('/');
				}
				VStr.push_back(s);

				if (VStr.size() == 3)
				{
					g_Login.GetAutoUser() = VStr[0];

					if(VStr[1].compare("") != 0)
						g_Login.GetAutoPassword() = VStr[1];

					if (VStr[2].compare("frombf") == 0)
						g_iParamGameType = 2;
					else//fromtea
						g_iParamGameType = 3;
				}
				else
				{
					g_Login.GetAutoUser() = VStr[0];
					if (VStr.size() == 2)
						g_Login.GetAutoPassword() = VStr[1];

					g_iParamGameType = 1;
				}

				//int pos = s.find_first_of('/');
				//if(pos != -1)
				//{
				//	g_Login.GetAutoUser() = s.substr(0,pos);
				//	g_Login.GetAutoPassword() = s.c_str()+pos+1;
				//}
				//else
				//{
				//	g_Login.GetAutoUser() = s;
				//}
				//g_ParamGame = true;
			}
			break;
		case 3:
			{
				char   *stopstring;
				g_hParentWnd = (HWND)(strtoul(temp,&stopstring,16));
			}
			break;
		case 4:
			WritePrivateProfileString("Config","ServerIP",temp,strGameIniPath.c_str());
			//g_Login.SetAutoLoginInType(5);//从浩方竞技登录游戏,直接到选角色界面
			break;
		case 5:
			WritePrivateProfileString("Config","ServerPort",temp,strGameIniPath.c_str());
			break;
		case 6:
			{
				char *pChannelName = strstr(temp,"[channelname:");
				if (pChannelName)
				{
					string strUnionLoginPath = GetGameDataDir();
					strUnionLoginPath += "\\UnionLogin.ini";

					string szServerID;//ws_1
					if(pChannelName > temp)
					{
						szServerID.assign(temp,pChannelName - temp);
					}
					else
					{
						szServerID = temp;
					}
					WritePrivateProfileString("LoginMode","LoginServer",szServerID.c_str(),strUnionLoginPath.c_str());


					char *pEndChannelName = strstr(pChannelName,"]");
					if (pEndChannelName && pEndChannelName - pChannelName > 13)
					{
						g_strChannelName.assign(pChannelName + 13,pEndChannelName - pChannelName - 13);

						WritePrivateProfileString("LoginMode","exclusive","1",strUnionLoginPath.c_str());
						WritePrivateProfileString("LoginMode","LoginType",g_strChannelName.c_str(),strUnionLoginPath.c_str());
					}
				}

			}
			break;
		case 7:
			{
				// 免注册登录
				g_Login.SetLoginInExpType(1);
			}
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////
// 主函数
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	// Debug 模式下启用 CRT 内存泄漏检测
	// 进程退出时自动在"输出"窗口打印泄漏块信息
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(xxx);  // 取消注释并填入块号以定位特定泄漏
#endif

	GetGameDataDir();//初始包工作路径

	g_hCgpfsDLL = LoadLibrary("cgpfs_l.dll"); 
	
	if (g_hCgpfsDLL)
	{
		//CgpInitialize();
		LPCgpInitialize pCgpInit = (LPCgpInitialize)GetProcAddress(g_hCgpfsDLL,"CgpInitialize");
		if (pCgpInit)
		{
			pCgpInit();
		}
		//取得插件窗口句柄
		LPCgpGetActiveWindow pCgpGetActiveWindow = (LPCgpGetActiveWindow)GetProcAddress(g_hCgpfsDLL,"CgpGetActiveWindow");
		if (pCgpGetActiveWindow)
		{
			g_hParentWnd = pCgpGetActiveWindow();
		}
	}

	strcpy(g_strCmdLine,lpCmdLine);

//#ifndef _DEBUG	
//	if(!g_hParentWnd && strstr(g_strCmdLine,"-loader") == NULL)
//	{
//		MessageBox(NULL,"请从ws.exe进入游戏!","提示",MB_OK|MB_ICONERROR);
//		return 0;
//	}
//#endif	

	//看一下自己是多开的第几个客户端
	int vApps[3];
	g_ShareMem.GetShare(vApps,sizeof(vApps));
	vApps[0]++;
	g_ShareMem.SetShare(vApps,sizeof(vApps));


#ifndef _DEBUG	
	//	//限多开
	//	//// 开启多个游戏的时候的提示
	//	//if (vApps[0] >= 3)
	//	//{
	//	//	::MessageBox(GetActiveWindow(),"最多同时开启3个客户端。","提示",MB_OK|MB_ICONERROR);
	//	//	return 0;
	//	//}
	//

	int iClients = 0;
	HWND hFindWnd = NULL;

	while(hFindWnd = ::FindWindowEx(NULL,hFindWnd,"WSCLIENT",NULL))
	{
		iClients ++;
	}

	// 开启多个游戏的时候的提示
	if (iClients >= 3)
	{
		::MessageBox(GetActiveWindow(),"最多同时开启3个客户端。","提示",MB_OK|MB_ICONERROR);
		return 0;
	}

	////寻找父进程的模块地址
	//DWORD currentProcessID = GetCurrentProcessId();
	//char szProcessName[MAX_PATH] = {'\0'};
	//char sonProcessName[MAX_PATH]= {'\0'};


	//FindParentPath(currentProcessID,szProcessName,sonProcessName);

	//output_debug("%s",szProcessName);

	//BYTE qyzexeMD5[128] = {0};

	//if (stricmp(szProcessName,"unknown") != 0)
	//{
	//	MD5_CTX::MakePassMD5(qyzexeMD5,szProcessName);
	//}

	//string szMD5 = HexDump(qyzexeMD5,16,"");

	//output_debug("%s",szMD5.c_str());

	//if (qyzexeMD5[0] != 0 && stricmp((char*)(szMD5.c_str()),"E0B683C2EF34490BBA436190D755E308") != 0)
	//{
	//	MessageBox(NULL,"请从ws.exe进入游戏，并确认您的版本是否最新。","提示",MB_OK|MB_ICONERROR);
	//	return 0;				
	//}

#endif



	if(vApps[0] > 1)
	{
		sprintf(g_strReplayDir,"Replay%d",vApps[0]);
	}

	
	AnalyzeCommandLine(); //


	g_dwLoadTime = GetTickCount();

	CSeException::Install_Handler();
	_set_invalid_parameter_handler(myInvalidParameterHandler);

	MSG			msg;
	DWORD		dwMsg,dwData;

	// 初始化
	g_ExceptionReport.SetLogFileName("../Exception.prt");

	GetGameVersion("2.0.0.0");

	a_hInst = hInstance;
	g_byPwdEncode = (BYTE)(rand()%256);

	::LoadString(hInstance,IDC_GAMECLIENT,a_szWindowClass,100);


#ifdef ENABLE_REPLAY

	if(strstr(g_strCmdLine,"-replay") != NULL)
	{
		g_ReplayManager.StartReplay(true);
	}
	else
	{
		GetGameDataDir();
		char szReturn[32] = {0};
		int r = GetPrivateProfileString("Config","SaveReplay","No",szReturn,32,".\\Config.ini");
		if(stricmp(szReturn,"Yes") == 0)
		{	
			g_ReplayManager.StartRecord(true);
		}
	}
#endif


	//创建启动窗口
	CreateStartWnd(hInstance); 

	BOOL bRet = CreateGameWnd(hInstance);
	if(!bRet)
		return FALSE;

	if(!AfterCreateGameWnd() || !InitGame())
	{
		DeleteGlobalObject();
		return FALSE;
	}

	if(a_hWndStart && !g_hParentWnd)//如果有父窗口,不要关闭这个窗口,否则无法切换输入法
		DestroyWindow(a_hWndStart);


	//录像的时候画鼠标,而且不隐藏系统鼠标
#ifdef ENABLE_REPLAY
	if(g_ReplayManager.IsInReplay())
	{
		g_pGameData->EnableSelfCursor(true);
		::ShowCursor(true);
	}
#endif


////for test
//
//	PKG_LG_CLI_SS_INFO pAck;
//	pAck.byResult = PROTO_SUCCESS;
//	pAck.stReply.stSucc.dwIP = 1 << 24 | 127;//127.0.0.1
//    pAck.stReply.stSucc.wPort = 8400;
//
//	g_pGameControl->MSG_DlServerIP_Ntf((const char *)(&pAck),123);
//
//
////end for test


	//强制更新引导程序
	//HANDLE hFile;
	//WIN32_FIND_DATA fd;
	//DWORD len1 = 0 , len2 = 0;
	//string strSrcFileName = "../ws.update";
	//string strDesFileName = "../ws.exe";
	//if (g_iParamGameType == 1)
	//{
	//	strSrcFileName = "../wooolhf.update";
	//	strDesFileName = "../wooolhf.exe";
	//}
	//else if (g_iParamGameType == 2)
	//{
	//	strSrcFileName = "../wooolbf.update";
	//	strDesFileName = "../wooolbf.exe";
	//}
	//else if (g_iParamGameType == 3)
	//{
	//	strSrcFileName = "../woooltea.update";
	//	strDesFileName = "../woooltea.exe";
	//}

	//hFile = FindFirstFile(strSrcFileName.c_str(),&fd);
	//if(hFile != INVALID_HANDLE_VALUE )
	//{
	//	len1 = fd.nFileSizeLow;
	//	FindClose(hFile);
	//	hFile = FindFirstFile(strDesFileName.c_str(),&fd);
	//	if(hFile != INVALID_HANDLE_VALUE)
	//	{
	//		len2 = fd.nFileSizeLow;
	//		FindClose(hFile);
	//	}
	//	if( len1 && len1!=len2 )	// 强制更新
	//	{
	//		CopyFile(strSrcFileName.c_str(),strDesFileName.c_str(),false);
	//	}
	//}
	//end 强制更新引导程序


	if(!g_pSDOAInterface && g_pMagicCtrl->GetMagicRoot(MAGICID_SHOW_IGW) 
		&& g_strChannelName.empty() && (g_Login.GetLoginInExpType() == 0) )
	{
		DWORD dwTest = GetTickCount();
		g_pSDOAInterface = new CSDOAInterface;

		if(g_pSDOAInterface->OpenIGW())
		{
			if (g_pSDOAInterface->GetSDOADx9())
				g_pSDOAInterface->GetSDOADx9()->Initialize(g_pGfx->GetD3DDev(),g_pGfx->GetD3DPP(),FALSE);

			g_pSDOAInterface->SetScreenStatus(SDOA_SCREENMINI);
			g_pSDOAInterface->SetLogicScreen(g_pGfx->GetWidth(),g_pGfx->GetHeight());

			POINT pt;
			pt.x = g_pGfx->GetWidth() - 305; pt.y = 0;
			g_pSDOAInterface->SetTaskBarPosition(&pt);
		}
		else
		{
			SAFE_DELETE(g_pSDOAInterface);
		}

		output_debug("igw_start1:%d\r\n",GetTickCount() - dwTest);
	}

	////收集5分钟之内的unhandle异常日志
	//WIN32_FIND_DATA ffd;
	//FILETIME  lft;

	//HANDLE h = FindFirstFile("../Exception.prt",&ffd);  
	//if(h != INVALID_HANDLE_VALUE)   
	//{   
	//	FileTimeToLocalFileTime( &( ffd.ftLastWriteTime),&lft);//修改时间
	//	FindClose(h);  

	//	SYSTEMTIME stimefile,stimenow;
	//	FileTimeToSystemTime(&lft,&stimefile);//文件时间转换成系统时间
	//	//GetSystemTime(&stimenow);//当前系统时间
	//	GetLocalTime(&stimenow);

	//	//只收集5分钟之内的未捕获异常,一般玩家崩溃了以后会重新再登录的
	//	if(stimenow.wYear == stimefile.wYear && stimenow.wMonth == stimefile.wMonth && stimenow.wDay == stimefile.wDay && stimenow.wHour == stimefile.wHour
	//		&& stimenow.wMinute - stimefile.wMinute < 5)
	//	{
	//		FILE* file = fopen("../Exception.prt", "rb" );
	//		if ( file )
	//		{
	//			long length = 0;
	//			fseek( file, 0, SEEK_END );
	//			length = ftell( file );
	//			fseek( file, 0, SEEK_SET );

	//			if ( length > 0 )
	//			{
	//				string data;
	//				data.reserve( length + 10);
	//				data = "Exception=";

	//				const int BUF_SIZE = 2048;
	//				char buf[BUF_SIZE + 1]={0};

	//				while( fgets( buf, BUF_SIZE, file ) )
	//				{
	//					data += buf;
	//				}

	//				data += "&type=2";

	//				string URL = g_pStreamMgr->GetWebsite("ExceptionReport");
	//				if(URL.empty())
	//					URL = "http://home.ws.sdo.com/project/saveexception.asp";

	//				int iPos = URL.find("//");
	//				int iPos2 = URL.find("/",iPos + 2);
	//				if(iPos != string::npos && iPos2 != string::npos)
	//				{
	//					g_HttpMgr.PostData(URL.substr(iPos + 2,iPos2 - (iPos + 2)).c_str(),URL.substr(iPos2 + 1,URL.length() - (iPos2 + 1)).c_str(),data.c_str());
	//				}
	//			}

	//			fclose( file );
	//		}
	//	}
	//}  
	////end 收集5分钟之内的unhandle异常日志


	//获得MAC地址
	PMIB_IFTABLE ifTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	ifTable = (MIB_IFTABLE*) malloc(sizeof(MIB_IFTABLE));

	// Make an initial call to GetIfTable to get the necessary size into the dwSize variable 首先调用GetIfTable函数，将所需大小赋值给dwSize变量
	if (GetIfTable(ifTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) 
	{
		free(ifTable);
		ifTable = (MIB_IFTABLE *) malloc (dwSize);
	}

	// Make a second call to GetIfTable to get the  actual data we want 再次调用GetIfTable以获取我们想要的实际数据
	if ((dwRetVal = GetIfTable(ifTable, &dwSize, 0)) == NO_ERROR) 
	{
		if (ifTable->dwNumEntries > 0 && ifTable->table[0].dwPhysAddrLen > 0 && ifTable->table[0].dwPhysAddrLen < 10)
		{
			for (int i = 0; i < ifTable->table[0].dwPhysAddrLen; i++)
			{
				sprintf(g_strMac + i * 3,"%02X-",ifTable->table[0].bPhysAddr[i]);
			}
			g_strMac[ifTable->table[0].dwPhysAddrLen * 3 - 1] = 0;
		}
	}
	else 
	{
		printf("\tGetIfTable failed.\n");
	}

	free(ifTable);
	//end 获得MAC地址


	////判断分红用的SDGGameIdentity.dat文件是否存在,不存在的话就创建一个
	//h = FindFirstFile("../SDO178/SDGGameIdentity.dat",&ffd);  
	//if(h == INVALID_HANDLE_VALUE)   
	//{	
	//	CreateDirectory("../SDO178",NULL);	
	//	FILE *fp = fopen("../SDO178/SDGGameIdentity.dat","wb");
	//	if (fp)
	//	{
	//		char szTemp[1024] = {0};
	//		char szDir[256] = {0};
	//		GetCurrentDirectory(256,szDir);
	//		std::string strDir = szDir;
	//		size_t pos = strDir.find_last_of("\\");
	//		if (pos != string::npos)
	//		{
	//			strDir = strDir.substr(0,pos);
	//		}

	//		sprintf(szTemp,"1^$^传奇世界^$^%s^$^%s",strDir.c_str(),g_strMac);
	//		output_debug(szTemp);

	//		CDES *pObj = CreateDES();

	//		string strKey = "456#$%iu";//密钥

	//		char pEncryptResult[2048] = {0};
	//		Encrypt(pObj,szTemp,strlen(szTemp),strKey.c_str(),strKey.length(),pEncryptResult);//加密
	//		DestroyDES(pObj);

	//		fwrite(pEncryptResult,strlen(pEncryptResult),1,fp);
	//		fclose(fp);
	//	}
	//}
	////end 红用


	RegistryAsRun();  //运行注册表检查


	g_dwLoadTime = GetTickCount() - g_dwLoadTime;
	a_dwLastChecParentWndTime = GetTickCount();


	// 主消息循环
	while(true)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) 
		{
			if(!GetMessage(&msg,NULL,0,0)) 
				return (int) msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		else
		{
			// 查看是否需要显示下一帧
			if(!g_pGfx->WillShowFrame())
			{
				Sleep(1);
				continue;
			}

			//检查父窗口是否有效
			if (g_hParentWnd && a_dwLastChecParentWndTime != 0)
			{
				if(GetTickCount() - a_dwLastChecParentWndTime > 3000)
				{
					a_dwLastChecParentWndTime = GetTickCount();
					if (!IsWindow(g_hParentWnd))
					{
						if(g_pGameControl)
							g_pGameControl->SEND_Quit_Game();

						DeleteGame();
						a_dwLastChecParentWndTime = 0;
						break;
					}
				}
			}


			g_Timer.OnIdle();

			TRY_BEGIN        
			PERF_SEG_ALL(ROOT,FALSE);

#ifdef ENABLE_REPLAY
			if(g_ReplayManager.IsInReplay())
			{				
				g_ReplayManager.Replay(g_pGfx->GetFrameCount());
			}
#endif

			PERF_SEG_BEGIN(DealNet,FALSE);
			g_pGameControl->GCL_Run();
			PERF_SEG_END(DealNet);

// 			if (g_TrusteeshipData.GetMicroControlPos() >= 0 || g_TrusteeshipData.GetCaptionSubstitutePos() >= 0)
// 			{
// 				g_pSelf = g_TrusteeshipData.GetMicroControledCharacter();
// 			}
// 			else
// 			{
// 				g_pSelf = &ORIGINALSELF;
// 			}


			if (g_pNet)
			{
				g_pNet->DealRead();
			}

			//后台下载线程已经退出了,把g_pDownloadLimitSpeedd delete掉
			if (g_pDownloadLimitSpeed && g_pDownloadLimitSpeed->GetProcessBufferThreadID() == 0)
			{
				SAFE_DELETE(g_pDownloadLimitSpeed);
			}

			PERF_SEG_BEGIN(DealInput,FALSE)
			if(g_pInput)					// 游戏主窗口输入
			{
				// 更新鼠标状态
				//g_pInput->UpdateMouseState(g_iMainWndX,g_iMainWndY);
				while(g_pInput->GetMsgBuffer(&dwMsg,&dwData)) 
					g_pControl->Msg(dwMsg,dwData);
			}
			PERF_SEG_END(DealInput);

			// 下面是绘制
			// 查看设备是否丢失
			g_pGfx->ValidateDX();

			// 开始绘图
			PERF_SEG_BEGIN(BeginDraw,FALSE);
			g_pGfx->BeginDraw();
			g_pGfx->ClearColor();
			PERF_SEG_END(BeginDraw);

			// 显示控件
			PERF_SEG_BEGIN(DrawControl,FALSE);
			g_pControl->Draw();
			//g_pControl->UpdateLayOut();
			PERF_SEG_END(DrawControl);

			// 显示细节信息
			PERF_SEG_BEGIN(ShowDetail,FALSE);
  			ShowDetail();
			PERF_SEG_END(ShowDetail);

			// 结束绘图
			PERF_SEG_BEGIN(EndDraw,FALSE);
			g_pGfx->EndDraw();
			PERF_SEG_END(EndDraw);

			TRY_END
		}
	}


	return 0;
}

// 启动窗口
LRESULT CALLBACK WndProcStart(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message )
	{
	case WM_CREATE:
		{
			char szName[128] = "";
			sprintf(szName,"%s\\logo.bmp",GetGameDataDir());
			a_hBmp = (HBITMAP)LoadImage(a_hInst,szName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		}
		break;
	case WM_DESTROY:
		DeleteObject(a_hBmp);
		a_hBmp = NULL;
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc,hdc2;
			hdc = BeginPaint(hWnd, &ps);
			if(a_hBmp )
			{
				hdc2 = CreateCompatibleDC(hdc);
				SelectObject(hdc2,a_hBmp);
				StretchBlt(hdc,0,0,381,372,hdc2,0,0,381,372,SRCCOPY);
				DeleteDC(hdc2);
			}
			EndPaint(hWnd, &ps);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////
//  函数：WndProc(HWND, unsigned, WORD, LONG)
//  目的：处理主窗口的消息。
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
/////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

TRY_BEGIN
    LRESULT lResult; 
	if (g_pGfx && g_pGfx->IsRenderIGW() && g_pSDOAInterface && g_pSDOAInterface->GetSDOADx9())
	{
		if (SDOA_WM_CLIENT_RUN == message)
		{
			if (g_Login.IsSDOALogin() && !g_pSDOAInterface->IsShowUserLoginDialog() && g_pSDOAInterface->GetSDOAApp())
			{
				g_pSDOAInterface->GetSDOAApp()->ShowLoginDialog(CSDOAInterface::OnLogin,0,0);
				g_pSDOAInterface->SetShowUserLoginDialog(TRUE);
			}
		}
		else
		{
			if(g_pSDOAInterface->GetSDOADx9()->OnWindowProc ( hWnd, message, wParam, lParam , &lResult ) == SDOA_OK)
			   return lResult;
		}
	}
TRY_END


TRY_BEGIN
	//判断是否是Widget可以处理的消息，如果是的话，主消息就不处理了
	LRESULT lRet = NULL;
    bool bClickedControl = false;
	if (g_pControl)
	{
		CControl * pFocusNow = g_pControl->GetFocusCtrl();
		if (pFocusNow && (pFocusNow->IsClick() || pFocusNow->IsRBClick()))
		{
			bClickedControl = true;
		}
	}
	
	if(!bClickedControl && g_WidgetMgr.OnWindowMessage(message,wParam,lParam,lRet))
		return lRet;
TRY_END

TRY_BEGIN
	//下面才是游戏自己处理的消息
	if(g_pInput)
	{
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			// 更新鼠标消息的获取窗口
			g_pInput->WndMessage(message,wParam,lParam);
		}
	}

	switch(message) 
	{
	case WM_LBUTTONDOWN:
		{
			break;
		}
	case WM_SIZE:
		{
			if(SIZE_RESTORED == wParam)			// 恢复
			{
				a_bRestored = TRUE;
				//下面两行有可能会导致循环的音效一直没法结束，比如魔法结束时已经结束掉了的循环音效，如果再Restartall的话就再也没有机会结束了,然后就一直响
				//传世一中也已经注释掉了
				//if(g_pAudio)
				//	g_pAudio->RestartAll();
				if (g_pGfx)
				{
					g_pGfx->SetDisplay(TRUE);
				}

				{
					int vApps[3];
					g_ShareMem.GetShare(vApps,sizeof(vApps));
					vApps[1]--;
					if(vApps[1] < 0)
						vApps[1] = 0;
					g_ShareMem.SetShare(vApps,sizeof(vApps));
				}
			}
			else if(SIZE_MINIMIZED == wParam)	// 最小化
			{
				a_bRestored = FALSE;
				//if(g_pAudio)
				//	g_pAudio->PauseAll();
				if (g_pGfx)
				{
					g_pGfx->SetDisplay(FALSE);
				}

				{
					int vApps[3];
					g_ShareMem.GetShare(vApps,sizeof(vApps));
					vApps[1]++;
					if(vApps[1] > vApps[0])
						vApps[1] = vApps[0];
					g_ShareMem.SetShare(vApps,sizeof(vApps));
				}
			}
			else if(SIZE_MAXIMIZED == wParam) //窗口模式最大化1024x768
			{
// 				if(g_pControl)
// 				{
// 					if (g_pGfx->GetWidth() >= 1280)
// 					{
// 						return DefWindowProc(hWnd, message, wParam, lParam);
// 					}
// 					else if (g_pGfx->GetWidth() >= 1024)
// 					{
// 						g_pGfx->SetWidth(1280);
// 						g_pGfx->SetHeight(800);
// 
// 						g_pGfx->SetDisMode(DM_FULL_FALSE);
// 						g_pGfx->AdjustWindow(g_hWnd,1280,800,true);
// 						if(!g_pGfx->RestoreDesktop(false))
// 						{
// 							MessageBox(g_hWnd,"尝试设置分辨率失败.","提示",MB_OK);
// 							return DefWindowProc(hWnd, message, wParam, lParam);;
// 						}	
// 
// 						g_pControl->ReSize(1280,800);
// 						g_pControl->SetScaleX(float(1280)/1024);
// 						g_pControl->SetScaleY(float(800)/768);
// 						g_pControl->ReSetAllWndPos();
// 					}
// 					else if (g_pGfx->GetWidth() >= 800)
// 					{
// 						g_pGfx->SetWidth(1024);
// 						g_pGfx->SetHeight(768);
// 
// 						g_pGfx->SetDisMode(DM_FULL_FALSE);
// 						g_pGfx->AdjustWindow(g_hWnd,1024,768,true);
// 						if(!g_pGfx->RestoreDesktop(false))
// 						{
// 							MessageBox(g_hWnd,"尝试设置分辨率失败.","提示",MB_OK);
// 							return DefWindowProc(hWnd, message, wParam, lParam);;
// 						}	
// 
// 						g_pControl->ReSize(1024,768);
// 						g_pControl->SetScaleX(float(1024)/1024);
// 						g_pControl->SetScaleY(float(768)/768);
// 						g_pControl->ReSetAllWndPos();
// 					}
// 				}
			}
			break;
		}
	case WM_MOVE:		
		{
			//if (g_hWnd && g_hParentWnd)//嵌入IE根本收不到WM_MOVE
			//{
			//	RECT rc; 
			//	GetWindowRect(g_hWnd,&rc);
			//	POINT pt;
			//	pt.x = rc.left;
			//	pt.y = rc.top;
			//	ClientToScreen(g_hWnd,&pt);
			//	g_iMainWndX = pt.x;
			//	g_iMainWndY = pt.y;
			//	output_debug("WM_MOVE:%d,%d,\r\n",pt.x,pt.y);
			//}
			//else
			{
				g_iMainWndX = (int)(short)LOWORD(lParam);
				g_iMainWndY = (int)(short)HIWORD(lParam);
			}


		}
		break;
		

	case WM_NCHITTEST:
		{
			LRESULT lRes = DefWindowProc(hWnd,message,wParam,lParam);
			if(lRes != HTCLIENT)
			{
				//g_pControl->SetMouseOnID(MOUSE_ON_INVALID);
				//g_pControl->SetHideCursor(TRUE);
				if(g_bSelfCursor)
					while(ShowCursor(TRUE) < 0);
			}
			else
			{
				//g_pControl->SetHideCursor(FALSE);
				if(g_bSelfCursor)
					while(ShowCursor(FALSE) >= 0);
			}
			return lRes;
		}
	case WM_KILLFOCUS:
		
		if(g_pGameData && !g_Config.GetAllWndMode())
		{
			//g_pGfx->SwitchDisplay();
			//ShowWindow(g_hWnd,SW_MINIMIZE);
			//if(g_pAudio)
			//	g_pAudio->PauseAll();
			//g_pGfx->SetDisplay(FALSE);
		}
		break;

	case WM_SETFOCUS:
		if(g_pGameData && !g_Config.GetAllWndMode())
		{
			//g_pGfx->SwitchDisplay();
			//ShowWindow(g_hWnd,SW_RESTORE);
			//if( g_pAudio ) 
			//	g_pAudio->RestartAll();
			//g_pGfx->SetDisplay(TRUE);
		}

		{
			a_dwSetFocus = GetTickCount();
			int vApps[3];
			g_ShareMem.GetShare(vApps,sizeof(vApps));
			vApps[2] = (int)a_dwSetFocus;
			g_ShareMem.SetShare(vApps,sizeof(vApps));
		}
		break;
	case MSG_SOCKET: // 网络消息
		{
			if(g_pNet != NULL)
				g_pNet->OnCommand(wParam,lParam);
			break;
		}
	case MSG_SOCKET_DOWNLOADFILE://下载服务器的网络消息
		{
			if (g_pDownLoadNet)
			{
				g_pDownLoadNet->OnCommand(wParam,lParam);
			}
			break;
		}
	case MSG_SOCKET_BGDOWNLOADFILE://下载服务器的网络消息
		{
			if (g_pBgDownLoadNet)
			{
				g_pBgDownLoadNet->OnCommand(wParam,lParam);
			}
			break;
		}
	case MSG_SOCKET_DOWNLOADFILE_LIMITSPEED://下载服务器的网络消息
		{
			if (g_pDownloadLimitSpeed)
			{
				g_pDownloadLimitSpeed->OnCommand(wParam,lParam);
			}
			break;
		}
	case MSG_IGW_APP:
		{
			g_WidgetMgr.OnHostApp(wParam,lParam);
			break;
		}
	case SDOA_WM_CLIENT_LOGIN:
		{
			if (g_pSDOAInterface)
			{
				LoginResultEx& loginResult = g_pSDOAInterface->GetLoginResult();
				g_pGameControl->SEND_OALogin_Req(loginResult.strSessionId.c_str(),loginResult.strSndaid.c_str());
				g_pControl->PushMsg(MSG_CTRL_USERLOGINWND,OPER_CLOSE);
			}
			break;
		}
	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId)
			{
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
	case WM_FLASH_WND:
		if(GetFocus() != a_hWnd)
		{
			FLASHWINFO fi;
			memset(&fi,0,sizeof(FLASHWINFO));

			fi.cbSize	= sizeof(FLASHWINFO);
		//	fi.dwFlags	= FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.hwnd		= a_hWnd;

			FlashWindowEx(&fi);
		}
		break;

	case WM_KEYDOWN:
#ifdef ENABLE_REPLAY
		if (wParam == VK_RIGHT)
		{
			if (g_ReplayManager.IsInReplay())
			{
				if (g_pGfx)
				{
					g_pGfx->SetFPS(g_pGfx->GetLastSetFPS() * 2);
					return true;				
				}
			}
			//else
			//{
			//	g_fScale *= 1.1f;
			//}
		}
		else if (wParam == VK_LEFT)
		{
			if (g_ReplayManager.IsInReplay())
			{
				if (g_pGfx)
				{
					g_pGfx->SetFPS(g_pGfx->GetLastSetFPS() / 2);
					return true;				
				}
			}
			//else
			//{
			//	g_fScale /= 1.1f;
			//}
		}
		else if (wParam == VK_DOWN && g_ReplayManager.IsInReplay())
		{
			if (g_pGfx)
			{
				g_pGfx->SetFPS(50);
				return true;				
			}
		}
		else if (wParam == VK_UP && g_ReplayManager.IsInReplay())
		{
			if (g_pControl)
			{
				g_ReplayManager.StartReplay(true);

				srand(g_ReplayManager.GetStartTime());
				//为了使时间和录像当时的一致,这里通过改变g_dwServerTime来实现
				__time32_t tNow;
				_time32(&tNow);
				g_dwServerTime = g_ReplayManager.GetStartTime() - tNow;

				g_pGameData->Clear();
				g_pGfx->SetFrameCount(0);

				SAFE_DELETE(g_pControl);
				g_pControl = new CMainWnd;
				g_pControl->Create(a_iW,a_iH);

				g_dwProStartCount = OrignGetTickCount();

				// 创建窗口
				g_pControl->Msg(MSG_CTRL_LOGINWND,OPER_CREATE);

				//g_pControl->Msg(MSG_CTRL_QUIT_WND,OPER_CUSTOM);
				//DeleteGame();
				//InitGame();
				return true;				
			}
		}
#endif
		break;
	case WM_KEYUP:
		break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
        return true;

	case WM_SYSCOMMAND:
		{
			if(wParam == SC_CLOSE)	// 截获关闭按钮
			{
				//g_pControl->Msg(MSG_SHOPPINGMALL_WND,0);
				//return true;
			}
			break;
		}
	case WM_CLOSE:
		{
			if(g_pControl && g_pControl->GetGameState() == MSG_CTRL_GAMEWND)
			{
				if((g_dwServerSwitch & SS_QUIT_DELAY) && SELF.GetID() != 0 && !SELF.IsDead())//读秒退出
				{
					g_pControl->PopupWindow(MSG_CTRL_READ_QUIT_WND,OPER_CREATE,0);
					return true;
				}
			}
			PostMessage(g_hWnd,WM_DESTROY,0,0);
			return true;
		}

	case WM_DESTROY:
		{
			if (g_pSDOAInterface)
			{
				g_pSDOAInterface->SetHaveClosedWindow(TRUE);
			}

			ShowWindow(g_hWnd,SW_HIDE);

			//退弹
			/*
			if (!g_hParentWnd)//微端不要退弹广告
			{
				PROCESS_INFORMATION pinfo;
				STARTUPINFO sf;
				ZeroMemory(&sf,sizeof(STARTUPINFO));

				string strPopUrl = g_pStreamMgr->GetGameStr("pop","http://client.ws.sdo.com/ingame/exit.asp");

				//strPopUrl = "http://adrs.sdo.com/ADRefererSystem/adjump.aspx?SndaADID=00-qyz-hfrym-1222-02&RedirectUrl=http%3a%2f%2fws.sdo.com%2fproject%2fingame%2fqyzexit.asp%3ftype%3d1";

				size_t pos = strPopUrl.find("http%3a%2f%2f");
				if (pos != string::npos)//肯定是网吧加了监控代码的版本
				{
					pos = strPopUrl.find("%3ftype%3d");// ?type=
					if (pos != string::npos)
					{
						strPopUrl = strPopUrl.substr(0,pos);
					}

					if (SELF.IsHaveOnlinePrize())
					{
						strPopUrl += "%3ftype%3d1";
					}
					else
					{
						strPopUrl += "%3ftype%3d0";
					}
				}
				else//正常版本
				{
					pos = strPopUrl.find("?");
					if (pos != string::npos)
					{
						strPopUrl = strPopUrl.substr(0,pos);
					}

					if (SELF.IsHaveOnlinePrize())
					{
						strPopUrl += "?type=1";
					}
					else
					{
						strPopUrl += "?type=0";
					}
				}


				g_pStreamMgr->SetGameStr("pop",strPopUrl.c_str());

				CreateProcess("popup.dat",NULL,NULL,NULL,FALSE,0,NULL,NULL,&sf,&pinfo);
			}
			*/
			//end退弹


			if(g_pGameControl)
				g_pGameControl->SEND_Quit_Game();

			DeleteGame();
			PostQuitMessage(0);

			int vApps[2];
			g_ShareMem.GetShare(vApps,sizeof(vApps));
			vApps[0]--;
			g_ShareMem.SetShare(vApps,sizeof(vApps));
		}

		break;		 
    
	case WM_INTERNET_ERROR:
		if(g_pGameControl)
			g_pGameControl->MSG_Internet_Error(wParam);
		break;
	case WM_ADDMSG:
		if(g_pGameControl)
			g_pGameControl->MSG_ADD_Internet_Msg((const char *)wParam,lParam);
		break;
	case WM_SHOWFPS:
		a_bFps = !a_bFps;
		a_dwLastShowCheckTime = GetTickCount();
		break;
		
	case WM_SHOWPERF:
		
		a_bPerf = !a_bPerf;
		a_dwLastPerfCheckTime = GetTickCount();
		break;
		

	//防止系统刷屏
	case WM_ERASEBKGND:
		return true;
		break;
	default:
		break;
	}
TRY_END

	return DefWindowProc(hWnd, message, wParam, lParam);
}  

void WINAPI SetAudioSoundVolume(DWORD NewVolume)
{
	if(g_pAudio)
		g_pAudio->SetVolumeSound(NewVolume);
}

DWORD WINAPI GetAudioSoundVolume()
{
	if(g_pAudio)
		return g_pAudio->GetVolumeSound();

	return 0;
}

void WINAPI SetAudioEffectVolume(DWORD NewVolume)
{
	if(g_pAudio)
		g_pAudio->SetVolumeMusic(NewVolume);
}

DWORD WINAPI GetAudioEffectVolume()
{
	if(g_pAudio)
		return g_pAudio->GetVolumeMusic();

	return 0;
}
