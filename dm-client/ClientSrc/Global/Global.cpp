#include "Global.h"
#include "DebugTry.h"

#include <stdio.h>
#include <io.h>
#include <math.h>
#include <new>
#include "tchar.h"
#include <ShlObj.h>

#include "PerfCheck.h"

#ifdef _DEBUG
#include "new.h"
#define new DEBUG_NEW
#endif

#include "BaseClass/Misc/Input.h"
#include "BaseClass/Control/Control.h"
#include "BaseClass/Misc/Net.h"
#include "BaseClass/Misc/Internet.h"
#include "GameMap/GameMap.h"
#include "GameMap/MinMap.h"
#include "GameData/GameData.h"
#include "GameControl/GameControl.h"
#include "BaseClass/Control/UiManager.h"
#include "GameData/MagicCtrlMgr.h"
#include "GameData/LoginData.h"
#include "GameClient/SDRSIEClient.h"
//#include "GameClient/IGWLoaderDx9.h"
#include "GameData/WooolStoreData.h"
#include "GameData/TalkMgr.h"
#include "Interface/EngineInterface.h"
#include "EngineCallBack.h"
#include "GameClient/SDOAInterface.h"
#include "Global/Interface/AudioInterface.h"
#include "GameClient/ReplayManager.h"
#include "GameData/TipsCfg.h"
#include "GameData/XmlCfg.h"
#include "GameAI/AIGoodMgr.h"
#include "GameAI/AIConfigMgr.h"
#include "ShareMem.h"
#include <TlHelp32.h>
//#include "Psapi.h"
#include "GameClient/ClientInfoCollect.h"
#include "GameClient/OpenLoginClient.h"
#include "GameClient/Registry.h"
#include <Shellapi.h>

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <winternl.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")



#define DEFAULT_GLOBAL_BUFFER_SIZE			(128 * 1024 + 16 + 64 * 64 * 7 + 16)





typedef CEngineInterface* (*LPCREATEENGINEINTERFACE)(CCallBackInterface*);
typedef ISDRSIEClient* (*LPCREATSDRSINTERFACE)();

typedef BOOL (*LPUSINGIE6INTERFACE)();
HMODULE          a_hEngineDLL       = NULL;
HMODULE          a_hSDRSIEDLL       = NULL;

HMODULE          a_hSDSamplerDLL       = NULL;
HMODULE          a_hIE6WebDll       = NULL;
CEngineCallBack  g_EngileCallBack;






typedef NTSYSAPI 
NTSTATUS (NTAPI* LdrLoadDll_t)(
							   IN PWCHAR PathToFile OPTIONAL,
							   IN ULONG Flags OPTIONAL,
							   IN PUNICODE_STRING ModuleFileName,
							   OUT PHANDLE ModuleHandle
							   );

typedef NTSYSAPI 
VOID (NTAPI* RtlInitUnicodeString_t)(
									 PUNICODE_STRING DestinationString,
									 PCWSTR SourceString
									 );

HMODULE LoadModuleEx(LPCWSTR lpszPath)
{
	WCHAR cbPath[MAX_PATH+32] = {0};
	if (PathIsRelativeW(lpszPath)) {
		GetModuleFileNameW(0, cbPath, MAX_PATH);
		wcscat(cbPath, L"/../");
		wcscat(cbPath, lpszPath);
	} else {
		LPWSTR lpszFile = NULL;
		GetFullPathNameW(lpszPath, MAX_PATH, cbPath, &lpszFile);
	};

	HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
	LdrLoadDll_t LdrLoadDll = (LdrLoadDll_t)GetProcAddress(hNtdll, _T("LdrLoadDll"));
	RtlInitUnicodeString_t RtlInitUnicodeString = (RtlInitUnicodeString_t)GetProcAddress(hNtdll, _T("RtlInitUnicodeString"));

	// make lib search path
	int len = GetEnvironmentVariableW(L"PATH", NULL, 0);
	WCHAR* envPath = (WCHAR*)malloc((len+MAX_PATH*2+1) * sizeof(WCHAR));
	GetModuleFileNameW(0, envPath, MAX_PATH);
	wcscat(envPath, L"/../appstore/core/;");
	wcscat(envPath, cbPath);
	wcscat(envPath, L"/../.;");
	int c = (int)wcslen(envPath);
	GetEnvironmentVariableW(L"PATH", envPath+c, len);

	// load libaray
	HMODULE module = NULL;
	UNICODE_STRING nameDll;
	RtlInitUnicodeString(&nameDll, cbPath);
	LdrLoadDll((WCHAR*)envPath, 0, &nameDll, (HANDLE*)&module);
	free(envPath);

	return module;
};









void output_debug(char *format,...)
{
	char str[8 * 1024] = {0};
	va_list argptr;

	va_start(argptr,format);
	vsprintf(str,format,argptr);
	va_end(argptr);

	OutputDebugString(str);
}


// 严重错误，立即退出程序
void fatal_error(const char * err,DWORD code)
{
	char str[1024];

	strcpy(str,"错误：");
	if(err) 
	{
		strcat(str,err);
		strcat(str,"\n");
	}
	OutputDebugString(str);
	exit(1);
}

void    debug_info_dlg(const char * s)
{
	::MessageBox(g_hWnd,s,"错误",MB_OK);
	exit(1);
}

void SaveLogFormat(const char *format,...)
{
	// 时间
	char strTime[128];
	char tmp_d[64];
	char tmp_t[64];

	_tzset();
	_strdate(tmp_d);
	_strtime(tmp_t);
	sprintf(strTime,"%s\t%s",tmp_d,tmp_t);


	char strLog[8 * 1024] = {0};
	va_list argptr;

	va_start(argptr,format);
	vsprintf(strLog,format,argptr);
	va_end(argptr);


	//记录到文件
	FILE *fp = fopen("../clientlog.txt","a+b");
	if (fp)
	{
		fprintf(fp,"Time:%s,Msg:%s\r\n",strTime,strLog);
		fclose(fp);
	}
}

void SaveLog(const char *buf,int len,const char *header,const char *tail)
{
	if ( !buf)
	{
		return;
	}

	// 时间
	char strTime[128];
	char tmp_d[64];
	char tmp_t[64];

	_tzset();
	_strdate(tmp_d);
	_strtime(tmp_t);
	sprintf(strTime,"%s\t%s",tmp_d,tmp_t);

	//记录到文件
	FILE *fp = fopen("../clientlog.txt","a+b");
	if (fp)
	{
		fprintf(fp,"Time:%s,Msg:%s",strTime,header);
		fwrite(buf,len,1,fp);
		fprintf(fp,"%s",tail);
		fclose(fp);
	}

}

int GetIEMainVersion(void)
{
	HKEY hKey = NULL;
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Internet Explorer"), 0, KEY_READ, &hKey);
	if (lRet != ERROR_SUCCESS)
	{
		return 0;
	}

	TCHAR buf[1024] = {0};
	DWORD nBufSize = sizeof(buf);
	DWORD dwType = 0;

	lRet = RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (unsigned char *)buf, &nBufSize);
	RegCloseKey(hKey);

	if (lRet != ERROR_SUCCESS)
	{
		return 0;
	}

	return _ttoi(buf);
}


string GetDesktopDir()
{
	LPMALLOC ShellMalloc;
	LPITEMIDLIST DesktopPidl;
	char DesktopDir[MAX_PATH];
	if(FAILED(SHGetMalloc(&ShellMalloc)))
		return "";
	if(FAILED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOPDIRECTORY,&DesktopPidl)))
		return "";
	if(!SHGetPathFromIDList(DesktopPidl, DesktopDir))
	{
		ShellMalloc->Free(DesktopPidl);
		ShellMalloc->Release();
		return "";
	}
	ShellMalloc->Free(DesktopPidl);
	ShellMalloc->Release();

	return string(DesktopDir);
}


bool CreateShortcutToURL(const char* lptstrURL, const char* lptstrLinkFile, int nIconIndex = 0, const char* lptstrIconFile = NULL)
{
	if(NULL == lptstrURL || NULL == lptstrLinkFile)
		return false;

	WritePrivateProfileString(_T("InternetShortcut"), 
		TEXT("URL"), 
		lptstrURL, 
		lptstrLinkFile); 
	if(NULL != lptstrIconFile)
	{
		WritePrivateProfileString(_T("InternetShortcut"), 
			TEXT("IconFile"), 
			lptstrIconFile, 
			lptstrLinkFile);

		char strIconIndex[32]={0};
		sprintf(strIconIndex, "%d", nIconIndex);
		WritePrivateProfileString(_T("InternetShortcut"), 
			TEXT("IconIndex"), 
			strIconIndex, 
			lptstrLinkFile);
	}
	return true;
}

bool CreateShortcutToURL_WS()
{
	string strDesktopDir = GetDesktopDir();
	if (!strDesktopDir.empty())
	{
		char strPath[MAX_PATH]={0};
		GetModuleFileName(NULL,strPath,sizeof(strPath));
		string strTemp = strPath;
		int iPos = strTemp.find_last_of('\\');
		if (iPos != string::npos)
		{
			if (iPos < strTemp.size() - 1)
			{
				strTemp = strTemp.substr(0,iPos);

				iPos = strTemp.find_last_of('\\');
				if (iPos != string::npos)
				{
					if (iPos < strTemp.size() - 1)
					{
						strTemp = strTemp.substr(0,iPos) + "\\ws.exe";
					}
				}
			}
		}

		string strShortcutUrl = g_pStreamMgr->GetWebsite("IEShortcutURL");
		if(strShortcutUrl.empty())
			strShortcutUrl = "http://wsweb.sdo.com";
		strDesktopDir += "\\传奇世界登陆器.url";
		return CreateShortcutToURL(strShortcutUrl.c_str(), strDesktopDir.c_str(), 0, strTemp.c_str());
	}
	return false;
}




//全局变量的定义和初始化
float	    g_fSin[360],g_fCos[360];
HWND		g_hWnd				= NULL;
HWND		g_hParentWnd		= NULL;
int			g_iMainWndX			= 0;
int			g_iMainWndY			= 0;
char		g_cMusic[20]		= {"default"};
DWORD       g_dwVersionTail     = 0x0;
DWORD       g_dwBuildID         = 0x0;
char	    g_strVersion[64]	= {0};
DWORD		g_dwServerVersion	= 9999;
E_SERVERTYPE g_eServerType = EST_COMMON;
DWORD       g_dwServerSwitch    = 0;
int         g_dwServerTime      = 0;
int			g_iPreReadMode		= PREREAD_NONE;
char		g_strCmdLine[MAX_PATH]	= {0};
BOOL		g_bException		= FALSE;
DWORD		g_dwMouseType		= 1;
BOOL		g_bSelfCursor		= FALSE;
int			g_iDXVersion		= 0x888888;
char*		g_cBuf				= NULL;
int         g_iParamGameType    = 0;//登录类型，0:普通，1,浩方，2:边锋，3:茶苑
BYTE        g_byPwdEncode       = 0x7B;
char        g_strMac[36]        = "00-00-00-00-00-00";		//获得MAC地址
DWORD       g_dwProStartCount = 0;
char		g_strReplayDir[MAX_PATH] = "Replay";//录像文件及录像配置文件存放目录
DWORD       g_dwLoadTime = 0;//程序启动花的时间
//float       g_fScale = 1.0f;
E_UITYPE    g_EutUiType = EUT_CLASSIC;//ui类型,0:时尚,1:经典
bool        g_bHasDownLoadInitPackage = true;
bool        g_bNeedScale = false;
POS         g_ScaleRate(1.0f,1.0f);
HMODULE     g_hCgpfsDLL = NULL;
bool        g_bIsMultiClientInSameDir = false;
bool        g_bHasUiInDownloading = false;
string       g_strChannelName = "";



CInput		* g_pInput			= NULL;
CGameNet   	* g_pNet			= NULL;
CDownloadNet * g_pDownLoadNet	= NULL;
CDownloadNet * g_pBgDownLoadNet	= NULL;
CDownloadLimitSpeed * g_pDownloadLimitSpeed	= NULL;

CCtrlMainWin* g_pControl		= NULL;
CGameMap	* g_pGameMap		= NULL;
CGameData	* g_pGameData		= NULL;
CGameControl* g_pGameControl    = NULL;
CUiManager  * g_pUiMgr          = NULL;
CMagicCtrlMgr* g_pMagicCtrl     = NULL;
ISDRSIEClient* g_pRainBowSpirt	= NULL;
CSDOAInterface* g_pSDOAInterface = NULL;

CAudioInterface		* g_pAudio = NULL;
CLightInterface *     g_pLight= NULL;
CFontInterface *      g_pFont= NULL;
CGraphicInterface *   g_pGfx= NULL;
CTexManagerInterface *g_pTexMgr= NULL;
CWeatherInterface *   g_pWeather= NULL;
CStreamManagerInterface *   g_pStreamMgr = NULL;
CCharacter*    g_pSelf = NULL;
ISDSampler*    g_pSDSampler = NULL;
CClientInfoCollect* g_pClientInfoCollect = NULL;
COpenLoginClient* g_pOpenLoginClient = NULL;


// Data目录
inline const char *GetGameDataDir()
{
	static char sDataDir[MAX_PATH]  = {0};//"../Data";
	if (!sDataDir[0])
	{
		char strPath[MAX_PATH]={0};
		GetCurrentDirectory(MAX_PATH,strPath);//有的时候外部CreateProcess可能传入不正确的参数
		std::string filename = std::string(strPath) + "\\Texture3.wpf";
        if (_access(filename.c_str(), 0) != 0) // 如果当前目录下没有Texture3.wpf文件，则尝试获取程序所在目录
		{
			GetModuleFileName(NULL, strPath, sizeof(strPath));
			filename = std::string(strPath) + "\\Texture3.wpf";
		}

		string strTemp = strPath;
		int iPos = strTemp.find_last_of('\\');
		if (iPos != string::npos)
		{
            if (iPos < strTemp.size() - 1 && _access(filename.c_str(), 0) != 0) // 当前就在Data目录下，不能再往上找了
			{
				strTemp = strTemp.substr(0,iPos);
			}

			SetCurrentDirectory(strTemp.c_str());
			//sprintf(sDataDir,"%s",strTemp.c_str());//不能用绝对目录，如果用绝对目录，如果目录中有中文，有些地方会出错，比如CPromptInfoMgr::LoadPromptInfo中的	std::ifstream fin(strPath);	如果std::locale::global(std::locale(""));也会有其它问题，所以这里还是用相对目录,并且保证中间不能有中文
			sprintf(sDataDir,"%s","..\\Data");

			GetCurrentDirectory(MAX_PATH,strPath);
			output_debug("CurrentDir:%s",strPath);

		}

		output_debug("DataDir:%s",sDataDir);
	}

	return sDataDir;
}



SYSTEMTIME GetModifyTime(char* appname)
{
	WIN32_FIND_DATA ffd ;
	SYSTEMTIME stUTC, stLocal;
	memset(&stLocal,0,sizeof(stLocal));
	HANDLE hFind = FindFirstFile(appname,&ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FileTimeToSystemTime(&(ffd.ftLastWriteTime), &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		FindClose(hFind);
	}
	else
	{
		output_debug("GetModifyTime_Fail:%s",appname);
	}

	return stLocal;
}

// 初始化全局变量
BOOL InitGlobalObject(HWND hWnd)
{
	g_hWnd = hWnd;

	//GetGameDataDir();//SetCurrentDirectory
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


	if(PathFileExists("../Data/Data1.wpf"))
	{
		FILE *fp = fopen("../Data/Data1.wpf","r+b");
		if (fp)
		{
			fclose(fp);
		}
		else
		{
			g_bIsMultiClientInSameDir = true;
		}
	}


	////获得强制更新引导程序选择的区组信息
	//SYSTEMTIME timeGame = GetModifyTime("game.ini");//game.ini中配置的区组，老引导程序会修改这里的配置，新的引导程序自己修改game.ini
	//SYSTEMTIME timeAutopatch = GetModifyTime("..\\usetting.ini");//强制更新引导程序选择的区组

	////output_debug("wYear:%d,wMonth:%d,wDay:%d,wHour:%d,wMinute:%d,wSecond:%d",timeGame.wYear,timeGame.wMonth,timeGame.wDay,timeGame.wHour,timeGame.wMinute,timeGame.wSecond);
	////output_debug("wYear:%d,wMonth:%d,wDay:%d,wHour:%d,wMinute:%d,wSecond:%d",timeAutopatch.wYear,timeAutopatch.wMonth,timeAutopatch.wDay,timeAutopatch.wHour,timeAutopatch.wMinute,timeAutopatch.wSecond);

	//timeGame.wDayOfWeek = timeAutopatch.wDayOfWeek;//忽略wDayOfWeek字段
	



	//char strServerID[128];
	int iP[4] = {0};
	char szTemp[256] = {0};
	int iPort = 0;
	if(g_hCgpfsDLL && g_hParentWnd)//微端
	{
		char szReceiveBuf[513] = {0};
		LPCgpQueryVariant lpQueryVariant = (LPCgpQueryVariant)GetProcAddress(g_hCgpfsDLL,"CgpQueryVariant");
		//123.122.111.111:7000[channelname:2133]-无双一区(全国电信)-areaid:1           //[channelname]对于官服是没有的,是指其它AP的名字,和Oauth.xml对应
		if (lpQueryVariant)
		{
			TRY_BEGIN
			int iRtn = lpQueryVariant(CGP_VARIANT_SERVINFO,szReceiveBuf,512);
			if (iRtn > 0)
			{

				output_debug("CGP_VARIANT_SERVINFO:%s",szReceiveBuf);
				sscanf_s(szReceiveBuf,"%d.%d.%d.%d:%d",&(iP[0]),&(iP[1]),&(iP[2]),&(iP[3]),&iPort);

				sprintf(szTemp,"%d.%d.%d.%d",iP[0],iP[1],iP[2],iP[3]);
				WritePrivateProfileString("Config","ServerIP",szTemp,strGameIniPath.c_str());

				sprintf(szTemp,"%d",iPort);
				WritePrivateProfileString("Config","ServerPort",szTemp,strGameIniPath.c_str());

				char *pChannelName = strstr(szReceiveBuf,"[channelname:");
				if (pChannelName)
				{
					char *pEndChannelName = strstr(pChannelName,"]");
					if (pEndChannelName && pEndChannelName - pChannelName > 13)
					{
						g_strChannelName.assign(pChannelName + 13,pEndChannelName - pChannelName - 13);

						string strUnionLoginPath = GetGameDataDir();
						strGameIniPath += "\\UnionLogin.ini";
						WritePrivateProfileString("LoginMode","exclusive","1",strGameIniPath.c_str());
						WritePrivateProfileString("LoginMode","LoginType",g_strChannelName.c_str(),strGameIniPath.c_str());
					}
				}
			}

			if(g_Login.GetLoginInExpType() != 1)
			{
				memset(szReceiveBuf,0,512);
				iRtn = lpQueryVariant(CGP_VARIANT_AUTHCODE,szReceiveBuf,512);

				output_debug("CGP_VARIANT_AUTHCODE:%d,%s",iRtn,szReceiveBuf);

				if (iRtn > 0 && !strstr(szReceiveBuf,"INVALIDATEA4353COLLATE"))//页面测试code:INVALIDATEA4353COLLATE
				{
					g_Login.SetLoginInExpType(2);
					g_Login.SetLoginInExp2AuthCode(szReceiveBuf);
				}
			}


			TRY_END
		}
	}
	//else //if(memcmp(&timeGame, &timeAutopatch,sizeof(SYSTEMTIME)) < 0)//如果timeAutopatch比如新说明肯定是从新的引导程序进入，要客户端自行设置ip和port
	//{
	//	GetPrivateProfileString("Setup","SelServerID","NOFIND",strServerID,128,"..\\usetting.ini");
	//	output_debug("ServerID:%s",strServerID);

	//	FILE* file = fopen("..\\dengluweb\\skin\\default\\config.js", "rb" );
	//	if ( file )
	//	{
	//		long length = 0;
	//		fseek( file, 0, SEEK_END );
	//		length = ftell( file );
	//		fseek( file, 0, SEEK_SET );

	//		if ( length > 0 )
	//		{
	//			string data;
	//			data.reserve( length);

	//			const int BUF_SIZE = 2048;
	//			char buf[BUF_SIZE + 1]={0};

	//			while( fgets( buf, BUF_SIZE, file ) )
	//			{
	//				data += buf;
	//			}

	//			//id : 'qyz_2',
	//			//aid : '2',
	//			//name : '雷霆万钧(2区全国网通)',
	//			//addr : '125.39.128.156:7000',		//服务器地址信息
	//			//isnew : '0',
	//			//type : '0'
	//			char szFind[128] = {0};
	//			int iType = 0;
	//			sprintf(szFind,"'%s'",strServerID);

	//			size_t iPos = data.find(szFind);
	//			if(iPos != string::npos)
	//			{
	//				size_t iPos2 = data.find("addr : '",iPos);
	//				if (iPos2 != string::npos)
	//				{
	//					memset(szTemp,0,128);

	//					TRY_BEGIN
	//						sscanf_s(data.c_str() + iPos2 + 8,"%d.%d.%d.%d:%d",&(iP[0]),&(iP[1]),&(iP[2]),&(iP[3]),&iPort);
	//					sprintf(szTemp,"%d.%d.%d.%d",iP[0],iP[1],iP[2],iP[3]);
	//					WritePrivateProfileString("Config","ServerIP",szTemp,strGameIniPath.c_str());
	//					output_debug("ServerIP:%s",szTemp);

	//					sprintf(szTemp,"%d",iPort);
	//					WritePrivateProfileString("Config","ServerPort",szTemp,strGameIniPath.c_str());
	//					output_debug("ServerPort:%s",szTemp);

	//					iPos2 = data.find("type : '",iPos);
	//					if (iPos2 != string::npos)
	//					{
	//						sscanf_s(data.c_str() + iPos2 + 8,"%d",&iType);
	//					}

	//					memset(szTemp,0,128);
	//					sprintf(szTemp,"%d",iType);
	//					WritePrivateProfileString("Config","GroupType",szTemp,strGameIniPath.c_str());

	//					TRY_END
	//				}

	//			}
	//		}

	//		fclose( file );
	//	}
	//}

	////end 获得强制更新引导程序选择的区组信息


	//DWORD dwRet = GetPrivateProfileInt("Config","GroupType",0,strGameIniPath.c_str());
	//if(dwRet == 1)
	//{
	//	g_eServerType = EST_DIFFERENT1;
	//}




	int iEnableDownLoadFile = 0;

	// 生成全局缓冲
	//加载引擎
	if (!a_hEngineDLL)
	{	
		SYSTEMTIME timeDat = GetModifyTime("WpfSys.dll");
		SYSTEMTIME timeUpdate = GetModifyTime("WpfSys.dll.update");

		//timeUpdate.wDayOfWeek = timeDat.wDayOfWeek;//忽略wDayOfWeek字段
		//if(memcmp(&timeDat, &timeUpdate,sizeof(SYSTEMTIME)) < 0)
		//因为玩家有可能会修改系统时间,所以判断一旦时间不一样就覆盖
		if(memcmp(&timeDat, &timeUpdate,sizeof(SYSTEMTIME)) != 0 && timeUpdate.wYear != 0)
		{
			DeleteFile("WpfSys.dll");
			CopyFile("WpfSys.dll.update", "WpfSys.dll", FALSE);
		}

		timeDat = GetModifyTime("Engine.dll");
		timeUpdate = GetModifyTime("Engine.dll.update");

		//timeUpdate.wDayOfWeek = timeDat.wDayOfWeek;//忽略wDayOfWeek字段
		//if(memcmp(&timeDat, &timeUpdate,sizeof(SYSTEMTIME)) < 0)
		//因为玩家有可能会修改系统时间,所以判断一旦时间不一样就覆盖
		if(memcmp(&timeDat, &timeUpdate,sizeof(SYSTEMTIME)) != 0 && timeUpdate.wYear != 0)
		{
			DeleteFile("Engine.dll");
			CopyFile("Engine.dll.update", "Engine.dll", FALSE);
		}

#ifdef _DEBUG
		a_hEngineDLL = LoadLibrary("EngineD.dll"); 
#else
		a_hEngineDLL = LoadLibrary("Engine.dll"); 
#endif

		if (!a_hEngineDLL)
		{
			output_debug("加载Engine.dll失败_1！%u",GetLastError());
			char szTemp[MAX_PATH] = {0};
#ifdef _DEBUG
			sprintf(szTemp,"%s\\EngineD.dll",GetGameDataDir());
			a_hEngineDLL = LoadLibrary(szTemp); 
#else
			sprintf(szTemp,"%s\\Engine.dll",GetGameDataDir());
			a_hEngineDLL = LoadLibrary(szTemp); 
#endif
		}

		if(!a_hEngineDLL)
		{
			DWORD dw = GetLastError(); 

			//LPVOID lpMsgBuf;
			//FormatMessage(
			//	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			//	FORMAT_MESSAGE_FROM_SYSTEM,
			//	NULL,
			//	dw,
			//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			//	(LPTSTR) &lpMsgBuf,
			//	0, NULL );

			//output_debug("加载Engine.dll失败_2！%u,%s",dw,lpMsgBuf);

			output_debug("加载Engine.dll失败_2！%u",dw);

			MessageBox(g_hWnd,"加载Engine.dll失败！",NULL,0);
			return FALSE;
		}

		LPCREATEENGINEINTERFACE lpSetGlobal = (LPCREATEENGINEINTERFACE)GetProcAddress(a_hEngineDLL,"CreateInstance");
		CEngineInterface *pEngineInterface = lpSetGlobal(&g_EngileCallBack);
		if (!pEngineInterface)
		{
			output_debug("加载Engine.dll失败_3！%u",GetLastError());
			MessageBox(g_hWnd,"加载Engine.dll失败！",NULL,0);
			if (a_hEngineDLL)
			{
				FreeLibrary(a_hEngineDLL);
			}
			return FALSE;
		}


		g_pStreamMgr = pEngineInterface->GetStreamManagerInterface();


		iEnableDownLoadFile = g_pStreamMgr->GetConfigInt("EnableDownload",1);
		//无双完整包也能让连接资源服务器,以后强更的升级包中也不带资源文件了
		//int iSubType = g_pStreamMgr->GetOtherConfigInt("SubType",0);//SubType:0为普通客户端,1为小客户端,
		//if (iSubType == 0)
		//	iEnableDownLoadFile = 0;

		if(iEnableDownLoadFile && !g_ReplayManager.IsInReplay())
		{
			g_pStreamMgr->SetNeedDownloadFileFromServer(true);
		}

		g_pStreamMgr->Init();


		g_pAudio = pEngineInterface->GetAudioInterface();
		g_pLight = pEngineInterface->GetLightInterface();
		g_pFont = pEngineInterface->GetFontInterface();
		g_pGfx = pEngineInterface->GetGraphicInterface();
		g_pTexMgr = pEngineInterface->GetTexMgrInterface();
		g_pWeather = pEngineInterface->GetWeatherInterface();

		g_dwProStartCount = GetTickCount();
	}


	g_cBuf = new char[DEFAULT_GLOBAL_BUFFER_SIZE];
	memset(g_cBuf,0,DEFAULT_GLOBAL_BUFFER_SIZE);


	g_pGameData		= new CGameData;
	g_pSelf         = &ORIGINALSELF;
	g_pInput		= new CInput;
	g_pNet			= new CGameNet(1,hWnd,MSG_SOCKET);	
	g_pGameMap		= new CGameMap;
	g_pGameControl	= new CGameControl;
	//导入UI管理器
	g_pUiMgr        = new CUiManager();
	g_pMagicCtrl    = new CMagicCtrlMgr();

	g_pMinMap       = new CMinMap();

	//g_pClientInfoCollect = new CClientInfoCollect; //信息采集，屏蔽可以阻止加载 cliqos.dll
	

	if(iEnableDownLoadFile && !g_ReplayManager.IsInReplay())
	{
		if (!PathFileExists("../Data/font/simkai.ttf"))
		{
			g_bHasDownLoadInitPackage = false;
		} 

		g_pDownLoadNet	= new CDownloadNet(hWnd,MSG_SOCKET_DOWNLOADFILE);
		g_pBgDownLoadNet = new CDownloadNet(hWnd,MSG_SOCKET_BGDOWNLOADFILE,true);

		int iIsFirstRun = (int)GetPrivateProfileInt("Config","FirstRun",1,"./Config.ini");
		if (iIsFirstRun)
		{
			char str[32] = "0";
			WritePrivateProfileString("Config","FirstRun",str,"./Config.ini");

			if (g_hParentWnd)
				CreateShortcutToURL_WS();
		}
		else
		{
			//多开以后,后面开的不要后台下载
			if(!g_bIsMultiClientInSameDir)
			{
				g_pDownloadLimitSpeed = new CDownloadLimitSpeed(hWnd,MSG_SOCKET_DOWNLOADFILE_LIMITSPEED);
			}
		}
	}




	if (false && g_pMagicCtrl->GetMagicRoot(MAGICID_ENABLE_SAMPLER)) // 加上false 跳过，不去加载 SDSampler.dll
	{
		if(!a_hSDSamplerDLL)
		{
			a_hSDSamplerDLL = LoadLibrary("SDSampler.dll"); 
			if (a_hSDSamplerDLL)
			{
				PFN_SDSamplerGetModule lpSDSamplerGetModule = (PFN_SDSamplerGetModule)GetProcAddress(a_hSDSamplerDLL,"SDSamplerGetModule");
				SDSamplerVersion version = SDSAMPLE_VERSION;
				g_pSDSampler = lpSDSamplerGetModule(&version);
			}
		}
	}

	if(!Init_GameData())
	{
		DeleteGlobalObject();
		return FALSE;
	}

	// 预生成360度的con和sin值
	for(int i = 0;i < 360;i++)
	{
		g_fCos[i] = (float)cos(i*PI/180);
		g_fSin[i] = (float)sin(i*PI/180);
	}

	g_TipsCfg.ReLoadGoodsTipsCfg();
	g_TipsCfg.ReLoadSkillTipsCfg();
	g_TipsCfg.ReLoadOtherTipsCfg();
	g_XmlCfg.ReloadXmgCfg();

	g_AIGoodMgr.LoadSoulLevelUpExp();	
	g_PromptInfoMgr.LoadPromptInfo();

	g_AICfgMgr.LoadFireLianHuaHelpCfg();
	g_AICfgMgr.LoadQiShuHelpCfg();

	//IE9内嵌ie补丁
	if (GetIEMainVersion() >= 9)
	{
		char szTemp[MAX_PATH] = {0};
		sprintf(szTemp,"%s\\SDO\\Base\\WidgetData\\ie6\\web.dll",GetGameDataDir());
		WCHAR wszWebDllPath[2 * 1024] = {0};
		MultiByteToWideChar(CP_ACP,0,szTemp,-1,wszWebDllPath,4 * 1024);

		a_hIE6WebDll = LoadModuleEx(wszWebDllPath); 
		if (a_hIE6WebDll)
		{
			LPUSINGIE6INTERFACE lpUseIE9 = (LPUSINGIE6INTERFACE)GetProcAddress(a_hIE6WebDll,"UsingIE6");
			if (lpUseIE9)
			{
				BOOL b = lpUseIE9();
				int i = 3;
			}
		}
	}

	return TRUE;
}

// 删除全局变量
void DeleteGlobalObject()
{
	TRY_BEGIN
		SAFE_DELETE(g_pControl);
	SAFE_DELETE_ARRAY(g_cBuf);
	SAFE_DELETE(g_pMagicCtrl);
	SAFE_DELETE(g_pGameData);
	SAFE_DELETE(g_pNet);
	SAFE_DELETE(g_pDownLoadNet);
	SAFE_DELETE(g_pBgDownLoadNet);
	SAFE_DELETE(g_pDownloadLimitSpeed);
	SAFE_DELETE(g_pGameMap);
	SAFE_DELETE(g_pGameControl);
	SAFE_DELETE(g_pInput);
	SAFE_DELETE(g_pUiMgr);
	SAFE_DELETE(g_pMinMap);
	SAFE_DELETE(g_pSDOAInterface);
	SAFE_DELETE(g_pClientInfoCollect);
	SAFE_DELETE(g_pOpenLoginClient);

	Release_GameData();

	if (g_pAudio)
	{
		g_pAudio->StopAllMusic();
		g_pAudio->StopAll();
	}

	g_pStreamMgr = NULL;
	g_pAudio = NULL;
	g_pLight = NULL;
	g_pFont = NULL;
	g_pGfx = NULL;
	g_pTexMgr = NULL;
	g_pWeather = NULL;

	typedef void (*LPRELEASE)();
	if(a_hEngineDLL)
	{
		LPRELEASE lpRelease = (LPRELEASE)GetProcAddress(a_hEngineDLL,"ReleaseEngine");
		lpRelease();			
		FreeLibrary(a_hEngineDLL);
		a_hEngineDLL = NULL;
	}

	if(a_hSDRSIEDLL)
	{
		FreeLibrary(a_hSDRSIEDLL);
		a_hSDRSIEDLL = NULL;
	}

	if (g_pSDSampler)
	{
		g_pSDSampler->Release();
		if (a_hSDSamplerDLL)
		{
			FreeLibrary(a_hSDSamplerDLL);
			a_hSDSamplerDLL = NULL;
		}
	}


	if(a_hIE6WebDll)
	{
		FreeLibrary(a_hIE6WebDll);
		a_hIE6WebDll = NULL;
	}

	TRY_END
}

int RSGameCB( int nMsg, const char* szMsg)
{
	PostMessage(g_hWnd, 0x3001, 0, 0);
	return 0;
}

void  GetDevAccordDir(int iDir,int&ix,int& iy)
{
	int iDis = 8;
	int iDis2 = 30;
	switch(iDir)
	{
	case 4:
		ix = iDis;
		iy = -iDis2/2;
		break;
	case 5:
		ix = ((int)(iDis / 1.414) + iDis2 / 2);
		iy = ((int)(iDis / 2.828) - iDis2/4);
		break;
	case 6:
		ix = 0;
		iy = (iDis/2 + iDis2/2);
		break;
	case 7:
		ix = -((int)(iDis / 1.414) - iDis2 / 2);
		iy = ((int)(iDis / 2.828) + iDis2 / 4);
		break;
	case 0:
		ix = -iDis;
		iy = iDis2/2;
		break;
	case 1:
		ix = -((int)(iDis / 1.414) + iDis2/2);
		iy = -((int)(iDis / 2.828) - iDis2/4);
		break;
	case 2:
		ix = -iDis2;
		iy = -iDis/2;
		break;
	case 3:
		ix = ((int)(iDis / 1.414) - iDis2/2);
		iy = -((int)(iDis / 2.828) + iDis2/4);
		break;
	default:
		ix = 0;
		iy = 0;
		break;
	}
}

bool IsCanRidePet(int iMonster)
{
	bool bCanRide = false;
	if(iMonster >= 166 && iMonster <= 168)
		bCanRide = true;
	else if(iMonster >= 185 && iMonster <= 187)
		bCanRide = true;
	else if(iMonster >= 39 && iMonster <= 41)
		bCanRide = true;
	else if(iMonster >= 4 && iMonster <= 6)
		bCanRide = true;
	else if(iMonster >= 39 && iMonster <= 41)
		bCanRide = true;
	else if(iMonster >= 301 && iMonster <= 303)
		bCanRide = true;

	return bCanRide;
}

string HexDump(const BYTE* pData, int len, const char *delim)
{
	static BYTE hextable[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	std::string sResult, tmp;
	BYTE result;
	for (int i = 0; i < len; i++)
	{
		result = BYTE(pData[i]);
		sResult += hextable[(result >> 4) & 0x0F];
		sResult += hextable[result & 0x0F];
		sResult += delim;
	}
	return sResult;
}

WORD GetGreateMagicID(WORD wMagicID,BYTE byFlyType,int iLevel,bool bDaoZunState,bool IsFightOnLeopard)
{
	WORD wRtnMagicID = wMagicID;

	switch (wMagicID)
	{
	case MAGICID_FIREBALL:
	case MAGICID_HEAL:
	case MAGICID_POISON_MAGIC:
	case MAGICID_THUNDER:
	case MAGICID_DISPUTE_FIRE:
	case MAGICID_PROTECT_SYMBOL:
	case MAGICID_PROTECT_SKIN:
	case MAGICID_CURSE:
	case MAGICID_ICE_ARROW:
	case MAGICID_STEEL_PROTECT:
	case MAGICID_ATTACK_KILL:
	case MAGICID_BLOOD_SHADE:
	case MAGICID_HELL_FIRE:
	case MAGICID_LURE_LIGHT:
	case MAGICID_WHOLE_MOON:	
	case MAGICID_ADV_FIREBALL:	
	case MAGICID_PROTECT_ARMOR:
	case MAGICID_WILD_COLLIDE:
	case MAGICID_PROTECT_GHOST:
	case MAGICID_TRACE_THUNDER:
	case MAGICID_MAGIC_PROTECT:
	case MAGICID_REPLACE_MAGIC:
	case MAGICID_ATTACK_MOON:
	case MAGICID_HELL_THUNDER:
	case MAGICID_BLOW_FIRE:
	case MAGICID_ATTACK_STICK:
	case MAGICID_ROTATE_FIRE:
	case MAGICID_FIRE_WALL:
	case MAGICID_HIDE_ALL:
	case MAGICID_CONTROL_CORPSE:
	case MAGICID_ICE_DRAG:
	case MAGICID_ICE_ARRAY:
	case MAGICID_CALL_MONSTER:
	case MAGICID_ATTACK_FIRE:
	case MAGICID_THUNDER_FIRE:
	case MAGICID_ICE_STORM:
	case MAGICID_MULTI_THUDER:
	case MAGICID_ICE_WHIRLWIND:
	case MAGICID_GOD_SAY:
	case MAGICID_TEAM_HEAL:
	case MAGICID_LOCK_MONSTER:
	case MAGICID_FIRE_RAIN:
	case MAGICID_DRAGON_LIGHT:
	case MAGICID_DESTROY_SHELL:
	case MAGICID_DESTROY_SHIELD:
	case MAGICID_DODGE_SKILL:
	case MAGICID_FIRE_JUJU:
		{			
			if (iLevel >= 4)
			{
				//if((byFlyType - 1) % 2 + 1 == 1)
				wRtnMagicID = wMagicID + 10000;	
				//else if((byFlyType - 1) % 2 + 1 == 2)
				//wRtnMagicID = wMagicID + 11000;	
			}

			if (bDaoZunState)
			{
				if (wMagicID == MAGICID_PROTECT_SYMBOL)
				{
					wRtnMagicID = MAGICID_DAOZUN_JIANGLIN_ATTACK;
				}				
				else if (!IsFightOnLeopard && wMagicID == MAGICID_FIRE_JUJU)
				{
					wRtnMagicID = MAGICID_DAOZUN_JIANGLIN_ATTACK;
				}				
			}
		}
		break;
	case MAGICID_PROTECT_SKIN_END:
		{
			if (iLevel >= 4)
			{
				//if(byFlyType == 1)
				wRtnMagicID = MAGICID_SHEN_PROTECT_SKIN_END;	
				//else if(byFlyType == 2)
				//wRtnMagicID = MAGICID_MO_PROTECT_SKIN_END;	
			}
		}
		break;		
		//case MAGICID_FIGHTER_FIGHTER_KILL:
		//case MAGICID_ENCHANTER_ENCHANTER_THUNDER:
		//case MAGICID_DAO_DAO_FLOWER:
		//case MAGICID_FIGHTER_ENCHANTER_FIRE:
		//case MAGICID_DAO_FIGHTER_POISON:
		//case MAGICID_ENCHANGER_DAO_BOSS:
		//	{
		//		if (iLevel >= 4)
		//		{//大师级技能书				
		//			wRtnMagicID += 15000;
		//		}
		//	}
		//	break;
		//case MAGICID_LONGXIAOJIUTIAN: //龙啸九天的等级:0-3,4-7.8-10
		//	{
		//		if(iLevel >= 4 && iLevel <= 7)
		//			wRtnMagicID = MAGICID_LONGXIAOJIUTIAN_LEVEL2;
		//		else if(iLevel >= 8 && iLevel <= 10)
		//			wRtnMagicID = MAGICID_LONGXIAOJIUTIAN_LEVEL3;
		//	}
		//	break;

	case MAGICID_JUDUZHAOZE:
		{
			if (iLevel >= 3)
			{	
				wRtnMagicID = MAGICID_JUDUZHAOZE_DA;		
			}
			else
			{
				wRtnMagicID = MAGICID_JUDUZHAOZE_XIAO;
			}
		}
		break;	
	default:
		break;
	}

	return wRtnMagicID;
}

WORD GetFightOnLeopardMagicID(WORD wMagicID)
{
	WORD wRtnMagicID = wMagicID;

	switch (wMagicID % 1000)
	{
	case MAGICID_DRAGON_LIGHT:
	case MAGICID_FIRE_JUJU:
	case MAGICID_SWT:
	case MAGICID_LSJ:
		wRtnMagicID = wMagicID + 3000;
		break;
	default:
		break;
	}

	return wRtnMagicID;
}

int GetTexIDByMagicID(WORD wMagicID, int state)
{	
	if(wMagicID < 58)
		return (wMagicID -1) * 2 + 300;
	else if(wMagicID < 95)
	{
		if (state == 0)
			return (wMagicID - 58) + 2612;
		else
			return (wMagicID - 58) + 2660;
	}
	else if (wMagicID < 113)
	{
		if (state == 0)
			return (wMagicID - 95) * 2 + 17200;
		else
			return (wMagicID - 95) * 2 + 17201;
	}
	else if (wMagicID >= 113 && wMagicID <= 114)
	{
		if (state == 0)
			return (wMagicID - 113) * 2 + 17244;
		else
			return (wMagicID - 113) * 2 + 17245;
	}
	else if (wMagicID >= 115 && wMagicID <= 117)
	{
		if (state == 0)
			return (wMagicID - 115) * 2 + 17236;
		else
			return (wMagicID - 115) * 2 + 17237;
	}

	return 0;
}

int GetTexIDByHouseMagicID(WORD wMagicID, bool isZhuDong)
{
	if (isZhuDong)
		return (wMagicID - 58) + 2612;
	else
		return 23230 + wMagicID;

	return 0;
}

bool QuickBuy(char* pName,int iCount,int iUseAfterBuy)
{
	g_WooolStoreMgr.SetQuickBuyItem(pName);
	CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

	if(QuickBuyData.pItem)
	{		
		QuickBuyData.iUseAfterBuyType = iUseAfterBuy;
		QuickBuyData.strMsg = "";
		QuickBuyData.iType = 1;
		QuickBuyData.iBuyCount = iCount;

		bool bUse = iUseAfterBuy == 0?false:true;
		g_WooolStoreMgr.BuyQuickItem(*QuickBuyData.pItem,bUse,iCount);

		return true;
	}
	else
	{
		char ctemp[128] = {0};
		sprintf(ctemp,"购买%s失败！",pName);
		g_TalkMgr.PushSysTalk(ctemp);

		return false;
	}
}

//bool StartRainBowSpirt()
//{
//	int iAreacode = g_Login.GetAreaNo();
//
//	if (!PathFileExists("../Data/SDRSIEClient.dll") || !PathFileExists("../Data/RSSDKDLL.dll"))
//	{
//		return false;
//	}
//
//	// 彩虹帮助IE版初始化
//	//if(!g_pGameData->IsShowRainBow()) return false;
//	int iServercode = g_Login.GetInnerGroupNo() + 1;
//	if(g_pRainBowSpirt != NULL )
//	{		
//		g_pRainBowSpirt->Show(0x01,iServercode,true);
//		return true;
//	}
//	//g_pRainBowSpirt = SDRSIEClientGetModule();
//
//	a_hSDRSIEDLL = LoadLibrary("SDRSIEClient.dll"); 
//
//	if(a_hSDRSIEDLL)
//	{
//		LPCREATSDRSINTERFACE lpGetSDRSI = (LPCREATSDRSINTERFACE)GetProcAddress(a_hSDRSIEDLL,"SDRSIEClientGetModule");
//		g_pRainBowSpirt = lpGetSDRSI();
//	}
//
//	if( g_pRainBowSpirt == NULL) 
//		return false;
//
//	long lSytle = GetWindowLong(g_hWnd, GWL_STYLE);
//	if(! (WS_CLIPCHILDREN & lSytle))
//	{
//		SetWindowLong(g_hWnd, GWL_STYLE, lSytle | WS_CLIPCHILDREN);
//	}
//
//	int nErrCode = g_pRainBowSpirt->CreateRSWindow( g_hWnd , SDRSIECLIENT_INGAME);
//	if(nErrCode != SDRSIECLIENT_ERROR_OK)
//	{
//		g_pRainBowSpirt->Release();
//		g_pRainBowSpirt = NULL;
//		return false;
//	}
//	int iIDType = 1;
//	char cAccount[64]={0},cRole[64]={0};
//
//	iIDType = 1;
//	strcpy(cAccount,g_Login.GetLoginID());
//
//	strcpy(cRole, SELF.GetName());
//
//	nErrCode = g_pRainBowSpirt->SetGameInfo(WS_GAME_ID, iAreacode, iIDType, cAccount, cRole, &RSGameCB);
//	if(nErrCode != SDRSIECLIENT_ERROR_OK)
//	{
//		g_pRainBowSpirt->Release();
//		g_pRainBowSpirt = NULL;
//		return false;
//	}
//	g_pRainBowSpirt->SetMovable(true, true);
//	g_pRainBowSpirt->Show( 0x01, iServercode, true);
//
//	return true;
//}


//bool StopRainBowSpirt()
//{
//	if (!g_pRainBowSpirt)
//	{
//		return false;
//	}
//
//	int iServercode = g_Login.GetInnerGroupNo() + 1;
//	g_pRainBowSpirt->Show( 0x01, iServercode, false);
//	g_pRainBowSpirt->Release();
//	g_pRainBowSpirt = NULL;
//
//	return true;
//}

bool StartRainBowSpirt()
{
	if(g_pSDOAInterface && g_pSDOAInterface->OpenWidget("sdoHelper") == SDOA_OK)
	{

	}
	else
	{
		//调用新的接口彩虹帮助
		g_pControl->PopupWindow(MSG_CTRL_CAIHONGHELP_WND,OPER_CREATE);
	}

	return true;
}

void AssignLooks(LOOKS& Looks,__int64 iLooks)
{
	memset(&Looks,0,sizeof(LOOKS));

	Looks.Player.iLooks = iLooks;

	BYTE type = (BYTE)(iLooks & 0x00000000000000FF);

	if(type == 0 || type == 254)
	{
		// 修正角色不显示
		Looks.Player.byType = CHARACTER_HUMAN;
		Looks.Char.byType = CHARACTER_HUMAN;

		// 服务端 btRaceImg, btDress, btWeapon, btHair: Byte
		WORD wShape = LOWORD(iLooks);	//(btRaceImg, btWeapon)
		WORD wAppr = HIWORD(iLooks);		//byHairType，wBody
		BYTE btRaceImg  = LOBYTE(wShape);
		BYTE btWeapon = HIBYTE(wShape);
		BYTE byHairType = LOBYTE(wAppr);
		BYTE wBody = HIBYTE(wAppr);

		Looks.Player.byWeapon = btWeapon;
		Looks.Player.byHairType = byHairType;
		Looks.Player.wBody = wBody;

		//Looks.Player.wHorse = (WORD)((iLooks >> 8) & 0x000000000000FFFF);
		//Looks.Player.byWeapon = (BYTE)((iLooks >> 24) & 0x00000000000000FF);
		//Looks.Player.byHairType = (BYTE)((iLooks >> 32) & 0x000000000000000F);
		//Looks.Player.byHairColor = (BYTE)((iLooks >> 36) & 0x000000000000000F);
		//Looks.Player.wBody = (WORD)((iLooks >> 40) & 0x000000000000FFFF);
		//Looks.Player.byColor = (BYTE)((iLooks >> 56) & 0x00000000000000FF);


		if(type == 254)
			Looks.Player.bySubType = CHARACTER_DEMON;
	}
	else
	{
		if(type == 0x32)	//50
		{
			// 这里只有look里面的类型改变了
			Looks.Char.byType = CHARACTER_NPC;// NPC
			Looks.Player.byType = CHARACTER_NPC;
			WORD wShape = LOWORD(iLooks);	//(btRaceImg, btWeapon)
			WORD wAppr = HIWORD(iLooks);		//wAppr
			BYTE btRaceImg  = LOBYTE(wShape);
			BYTE btWeapon = HIBYTE(wShape);

			Looks.Char.wShape = wAppr;

			//修正NPC显示
			//Looks.Char.wShape = ((WORD)((iLooks >> 24 ) & 0x000000000000FF00)) | ((WORD)((iLooks >> 16 ) & 0x00000000000000FF));
			//Looks.Char.wShape = ((WORD)((iLooks >> 16 ) & 0x000000000000FFFF));
			//output_debug("Looks %I64d Npc wAppr 1 %d\n", iLooks, ((WORD)((iLooks >> 8 ) & 0x000000000000FF00)) );
			//output_debug("Looks %I64d Npc wAppr 2 %d\n",iLooks,  ((WORD)((iLooks >> 16 ) & 0x000000000000FFFF)));
			//output_debug("Looks %I64d Npc wAppr 3 %d\n", iLooks, ((WORD)((iLooks >> 24 ) & 0x000000000000FFFF)));
			//output_debug("Looks %I64d Npc wAppr 4 %d\n", iLooks, ((WORD)((iLooks >> 32 ) & 0x000000000000FFFF)));
			//Looks.Char.wReserve = (Looks.Char.wReserve & 0xFF00) | ((WORD)((iLooks >> 24 ) & 0x00000000000000FF));
		}
		else
		{
			Looks.Char.byType = CHARACTER_MONSTER;// 怪物
			Looks.Player.byType = CHARACTER_MONSTER;
			//Looks.Char.wShape = (WORD)((iLooks >> 16 ) & 0x000000000000FFFF);
			Looks.Char.wShape = HIWORD(iLooks);	
		}
	}
}

/*
寻找父进程的模块地址
*/
/*
bool FindParentPath(const DWORD currentProcessID,char* ptrResult,char* sonFileName)
{
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, currentProcessID);

	if (handle == INVALID_HANDLE_VALUE)
	{
		::OutputDebugString("createToolHelp32Snapshot FAILED!\n");
		return false;
	} 

	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(PROCESSENTRY32);

	DWORD parentID = 0;

	if (!Process32First(handle, &pEntry))
	{
		::OutputDebugString("getEntry FAILED!\n");
		return false;
	}
	else
	{
		if (currentProcessID == pEntry.th32ProcessID)
		{
			parentID = pEntry.th32ParentProcessID;

		}else
		{
			while (Process32Next(handle,&pEntry))
			{
				if (currentProcessID == pEntry.th32ProcessID)
				{
					parentID = pEntry.th32ParentProcessID;
					break;
				}
			}
		}
	}
	//根据ID获得子进程模块地址
	char sonProcessName[MAX_PATH] = "unknown";
	HANDLE sonProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, currentProcessID);   

	if( sonProcess)   
	{   
		HMODULE hMod;   
		DWORD cbNeeded;   

		if(EnumProcessModules(sonProcess, &hMod, sizeof(hMod), &cbNeeded))   
		{   
			GetModuleFileNameEx(sonProcess, hMod, sonProcessName, sizeof(sonProcessName));   
		}   
	}   

	CloseHandle( sonProcess); 

	memcpy(sonFileName, sonProcessName, sizeof(sonProcessName));

	//根据ID获得父进程模块地址
	char szProcessName[MAX_PATH] = "unknown";   

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, parentID);   

	if( hProcess)   
	{   
		HMODULE hMod;   
		DWORD cbNeeded;   

		if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))   
		{   
			GetModuleFileNameEx(hProcess, hMod, szProcessName, sizeof(szProcessName));   
		}   
	}   

	CloseHandle( hProcess);   

	memcpy(ptrResult,szProcessName,sizeof(szProcessName));

	return true;
}
*/

void OpenWebUrl(const char* szUrl,bool bUseDefaultBrowser)
{
	if(!szUrl)
	{
		return;
	}

	bool bFind = false;
	if (bUseDefaultBrowser)
	{
		char szDefaultBrowser[MAX_PATH * 2] = {0}; //定义命令行
		bool bRtn = EnumRegString(0,szDefaultBrowser,"htmlfile\\shell\\open\\command",HKEY_CLASSES_ROOT);
		if(bRtn)
		{
			//ShellExecute(NULL,"open",szDefaultBrowser,szUrl,NULL,SW_SHOW);
			strcat(szDefaultBrowser," ");
			strcat(szDefaultBrowser,szUrl);
			int iRtn = WinExec(szDefaultBrowser,SW_SHOW);
			if(iRtn > 31)
			{
				bFind = true;
			}
		}
	}
	
	if(!bFind)
	{
		ShellExecute(NULL,"open","iexplore.exe",szUrl,NULL,SW_SHOW);
	}
}






#ifdef ENABLE_REPLAY


#ifdef GetTickCount

#undef GetTickCount
DWORD MyGetTickCount()
{
	if (g_ReplayManager.IsInReplay() && g_pGfx)
	{
		return g_dwProStartCount + g_pGfx->GetFrameCount() * 20;
	}

	return ::GetTickCount();
}

DWORD OrignGetTickCount()
{
	return ::GetTickCount();
}
#define GetTickCount() MyGetTickCount()

#endif

#endif //#ifdef ENABLE_REPLAY
