#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#endif

// #ifdef _WIN64
#ifndef WITHOUT_SD_INTERFACE
#define WITHOUT_SD_INTERFACE	// 64位不支持SDOA接口，本来也没用
#endif
// #endif

#pragma warning(disable:4251) // disable: warning C4251: 说类导出了但成员变量中的复杂类型没有导出

// 设置使用性能测试
#define DOPERFCHECK			1

// 用来测试内存泄漏的new重载
#include "new.h"

// Global Macros
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL;		} }		// 安全的释放
#define SAFE_DELETE(p)			{ if(p) { delete (p); (p)=NULL;			} }		// 安全的删除new出来的变量
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete [] (p); (p)=NULL;		} }		// 安全的删除new []出来的数组

// 定义同步机制
#define DEFINE_HANDLE(h)		CRITICAL_SECTION h;
#define INIT_HANDLE(h)			InitializeCriticalSection(&h);
#define LOCK_HANDLE(h)			EnterCriticalSection(&h);
#define UNLOCK_HANDLE(h)		LeaveCriticalSection(&h);
#define DELETE_HANDLE(h)		DeleteCriticalSection(&h);

#define Conv_INT64(s)  *((__int64*)(s))
#define Conv_DWORD(s)  *((DWORD*)(s))
#define Conv_INT(s)    *((int*)(s))
#define Conv_WORD(s)   *((WORD*)(s))
#define Conv_BYTE(s)   *((BYTE*)(s))

// Global Define
#define	PI							3.14159f
#define PREREAD_NONE				0
#define PREREAD_AUTO				1
#define PREREAD_FULL				2
#define	PREREAD_DEBUG				3
#define WS_GAME_ID                  188


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

#pragma warning(disable : 4996)
#pragma warning(disable : 4267)
#pragma warning(disable : 4018)
#pragma warning(disable : 4312)

#define _CRT_SECURE_NO_WARNINGS

// 位置结构
typedef struct POS
{
	POS(float x,float y):fx(x),fy(y){}

	float fx;
	float fy;
}*LPPOS;

// 缓冲模式读取纹理的优先等级
enum eReadPrior
{
	EP_MOST_HIGH	=	0, //最高
	EP_UI,	               //UI	
	EP_TILES,              //大地表
	EP_NPC,                //NPC
	EP_MONSTER,            //怪物
	EP_CHARACTER,          //角色
	EP_SMTILES,            //小地表及贴地物体
	EP_OBJECT,             //地图中的物体层
	EP_SKILL,              //技能特效
	EP_MAGIC,              //魔法特效
	EP_AUDIO,	           //音效
	EP_NORMAL,             //一般
	EP_DONT_DOWNLOAD,      //如果本地有就读取,没有不下载

	EP_TYPES,	           //优先等级种类数
};

// 区类型
enum E_SERVERTYPE
{
	EST_COMMON = 0,      //普通区
	//EST_DIFFERENT1,      //差异化新区1,没有心魔,飞升,等级开放到200级,装备怪物暴,装备融合
};

//界面类型
enum E_UITYPE
{
	EUT_NONE = -1,   //无效,changeui时传这个表示界面轮流切换,否则切换成指定的
	EUT_CLASSIC = 0, //经典界面,传世老玩家
	EUT_FASHION,     //时尚界面
};


// 游戏显示的模式
enum DisplayMode
{
	DM_WINDOWED		= 0	,	// 窗口模式
	DM_FULL_FALSE		,	// 假全屏模式
	DM_FULL_TRUE		,	// 真全屏模式
};


class CLock
{
public:
	CLock(CRITICAL_SECTION *pSec)
	{
		m_pSec = pSec;
		EnterCriticalSection(m_pSec);
	}
	~CLock()
	{
		LeaveCriticalSection(m_pSec);
	}
protected:
	CRITICAL_SECTION *m_pSec;
};


#include "StdHeaders.h"