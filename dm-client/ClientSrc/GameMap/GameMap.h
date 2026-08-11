///////////////////////////////////////////////////////////////////////
//地图类
///////////////////////////////////////////////////////////////////////
#pragma once

#include "Global/Global.h"
#include "Global/MemPool.h"
#include "Global/Interface/GraphicInterface.h"
#include "Global/Interface/TexManagerInterface.h"
#include "GameData/GameDefine.h"
#include "Global/Interface/StreamInterface.h"

// 一个地图砖块单元的结构
enum E_ATTR
{
	AT_BLOCK      = 0x01,
	AT_USED       = 0x02,
	AT_USERBLOCK  = 0x04,

	AT_BLOCKLIGHT = 0x10,
	AT_NEARGROUD  = 0x40,
	AT_PHENIX_PASS_BLOCK = 0x80,//骑凤凰可以通行,普通行走不能通行的阻挡点

	AT_CLEAR_BLOCK = 0x80000000,//动态场景用到,清除原来的阻挡点
};

enum E_ATTR_OFF
{
	ATO_NONE = 0x00,
	ATO_BLOCK = 0x01,
	ATO_SMTILES = 0x02,
	ATO_TILES = 0x04,
	ATO_OBJECT = 0x08,
	ATO_SOUND = 0x10,
	ATO_UNDER_OBJ = 0x20,
	ATO_NEARGROUD = 0x40,
	ATO_PHENIX_PASS_BLOCK = 0x80,

	ATO_APPERA_HIDE = 0x100,
	ATO_HIDE_APPEAR	= 0x200,

	ATO_CLEAR_BLOCK = 0x80000000,//动态场景用到,清除原来的阻挡点
};

enum E_ExAttr_Grid
{
	EXATO_NONE = 0x0000,
	EXATO_OVER_OBJ = 0x0001,//人物之上的场景

	EXATO_COLORADJT   = 0x0002,//大地表
	EXATO_COLORADJST  = 0x0004,//小地表
	EXATO_COLORADJOB  = 0x0008,//物体
	EXATO_COLORADJEFF = 0x0010,//特效

	ATO_GROUP		  = 0x0020,//打组信息
	ATO_BLOCK1		  = 0x0040,// 属性1
	EXATO_COLOROVEROBJ  = 0x0080,//人物之上的场景颜色

	//EXATO_RENMODEST	  = 0x0100,
	//EXATO_RENMODEOBJ  = 0x0200,
	//EXATO_RENMODEEFF  = 0x0400,

	ATO_BLOCK2		  = 0x0800,// 属性2
	ATO_BLOCK3		  = 0x1000,// 属性3
	EXATO_COLORADJFLOOR  = 0x2000,//贴地物体
};

enum E_OBJTYPE
{
	OT_NORMAL          = 0x00000000,
	OT_WATERTIDE       = 0x01000000,
	OT_LIGHTCIRCLE     = 0x02000000,
	OT_GROUP_MASK      = 0x10000000,
	OT_HIDEATNIGHT_MASK= 0x20000000,
	OT_TYPE_MASK       = 0x0F000000,
	OT_OBJECT_MASK     = 0x00FFFFFF,
};
// 地图砖块单元的结构
typedef std::vector<DWORD> DWVECTOR;

#define NEW_TILE2_T()	STile2::NEW_STile2()
#define DEL_TILE2_T(p)	STile2::DEL_STile2(p)

struct STile2
{
	DECLARE_POOL(STile2);

	STile2()
	{
		clear();
	}

	void clear()
	{
		//dwSTColor = 0xFFFFFFFF;
		//dwTColor  = 0xFFFFFFFF;
		dwObColor = 0xFFFFFFFF;		
		//dwUnderObjColor = 0xFFFFFFFF;
		//dwOverObjColor = 0xFFFFFFFF;
		dwFloorColor = 0xFFFFFFFF;

		vGroup.clear();
	}

	//下面这些字段暂时没用到,为了节省内存先注释掉,在编辑器里及地图里是存在的,
	//DWORD	dwSTColor;		// 小地表颜色
	//DWORD	dwTColor;		// 大地表颜色
	DWORD	dwObColor;		// 物体颜色	
	//DWORD	dwUnderObjColor;// UnderObj颜色
	//DWORD	dwOverObjColor;// OverObj颜色
	DWORD dwFloorColor;   // 贴地物体颜色

	DWVECTOR vGroup;
};


struct STile
{
	STile()
	{
		dwSmTile = 0;
		dwTile = 0;
		uObject = 0;
		//wObjectH = 0;
		bySound = 0;
		byAttr = 0;
		dwUnderObj = 0;
		dwOverObj = 0;

		uFloor	= 0;
		wExtraAttr = 0;

		pTile2 = NEW_TILE2_T();
	}

	~STile()
	{
		DEL_TILE2_T(pTile2);
		pTile2 = 0;
	}

	STile(const STile& right)
	{
		dwSmTile = right.dwSmTile;
		dwTile = right.dwTile;
		uObject = right.uObject;
		dwUnderObj = right.dwUnderObj;
		dwOverObj = right.dwOverObj;
		bySound = right.bySound;
		byAttr = right.byAttr;
		wEffect1 = right.wEffect1;
		uFloor = right.uFloor;
		wExtraAttr = right.wExtraAttr;

		pTile2 = NEW_TILE2_T();
		*pTile2 = *(right.pTile2);
	}

	DWORD	dwSmTile;		// 此砖块使用的小砖块贴图号码
	DWORD	dwTile;			// 此砖块使用的大砖块贴图号码
	DWORD	uObject;		// 此砖块上的物体图片编号
//	WORD    wObjectH;       // 此砖块上的物体图片的高度
	DWORD   dwUnderObj;		// 原来的uEffect,表示物体之下的场景
	DWORD   dwOverObj;      // 人物之上的场景特效
	BYTE	bySound;		// 地表的音效类型
	BYTE	byAttr;			// 此砖块基本属性
	WORD    wEffect1;//人物之上的场景特效1

	DWORD	uFloor;			//地板，贴地物体
	WORD    wExtraAttr;     // 砖块的扩展属性[1978]

	STile2* pTile2;
};
// 地图砖块单元的结构
struct SDynMapTile:public STile
{
	SDynMapTile():STile()
	{
		dwSmTile = 0;
	}

	BYTE	byAppHide;		// 渐隐渐现0,无,1渐现,2渐隐
	DWORD	dwTime;
	DWORD	dwParts;		// 动态改变了的字段
};
// 一个地图砖块单元的结构
struct SMapTile:public STile
{
	SMapTile():STile()
	{
		pDynTile = NULL;
	}
	~SMapTile()
	{
		SAFE_DELETE(pDynTile);
	}

	SDynMapTile *pDynTile;     //tile动态数据
};

struct SMapData
{
	SMapData()
	{
		iWidth = 0;
		iHeight = 0;
		pDataArray = NULL;
		dwOffset = 0;
	}
	int   iWidth,iHeight;
	SMapTile* pDataArray;
	DWORD dwVersion;
	DWORD dwOffset;
};


struct BGLayerData
{
	BGLayerData()
	{
		iWidth =  iHeight = 0;
		iOffX  =  iOffY = 0; 
		iTileOffX = iTileOffY = 0;
		fSpeedRate = 1.0f;
		pDataArray = NULL ;
		iBGStartX = iBGStartY = 0;
		iBGOffX = iBGOffY = 0;
	}

	SMapTile*  GetTile(DWORD dwXY); //获得背景砖块
	SMapTile*  GetTile(int iX,int iY);//获得背景砖块

	int   iWidth,iHeight;
	int   iOffX,iOffY;
	int   iTileOffX,iTileOffY;
	float fSpeedRate;
	int   iLayerNo;   //背景层编号
	SMapTile* pDataArray;
	int   iBGStartX,iBGStartY; //开始绘制的格子
	int   iBGOffX,iBGOffY;     //开始绘制的偏移
};

typedef map<WORD,BGLayerData>  MBGLayer;

//动态改变了的数据,在跳出地图的时候要还原,比如行会塔改变了tile的object,block,飞升台改变了effect
struct SDynamicData
{
	SDynamicData()
	{
		dwType = 0;
		sX = 0;
		sY = 0;
	}

	short sX,sY;//相对坐标
	DWORD dwType;//E_ATTR_OFF 类型
	SMapTile tile;
};

typedef vector<SDynamicData>  VDynamicData;
//动态物体枚举
enum E_DYN_OBJ
{
	EDO_NONE = 0,
	EDO_GUILDTOWER,//行会塔
	EDO_FLYTOWER,//飞升台


	EDO_BRIDGE1 = 100,//天城争夺战地图桥1
};
//动态物体数据结构
struct SDynamicObject
{
	// 	SDynamicObject()
	// 	{
	// 		dwIDx = 0;
	// 		wTileCount = 0;
	// 		byLeft = byRight = byTop = byBottom = 0;
	// 	}

	~SDynamicObject()
	{
		vData.clear();
	}

	//DWORD dwIDx;//编号,唯一标识
	//BYTE byLeft,byRight,byTop,byBottom;//范围,服务器用到
	//WORD wStartX,wStartY;//物体坐标原点X,Y坐标,pData中ix,iy就是和坐标原点的相对偏移,编辑器中用到,
	//WORD wTileCount;//动态数据个数,编辑文件中用到

	VDynamicData vData;
};

typedef map<DWORD,SDynamicObject>  MDynamicObject;
typedef map<DWORD,BYTE>			   MDynamicAppHide;

// 地图组织类，提供地图数据的读写接口
class CGameMap
{
private:
	string      m_strMapTitle;          // 地图名称
	string		m_strMapFile;   		// 地图编号
	string      m_strRealMap;           // 实际地图文件名	
	int			m_iWidth;				// 地图宽度
	int			m_iHeight;				// 地图高度

	//LPTexture	m_pMiniTex;				// 小地图
	DWORD       m_dwMiniTexID;           // 小地图

	string	    m_strOldMapFile;	    // 上次地图名称
	string      m_strOldRealMap;        // 上次实际地图文件名
	int			m_iOldWidth;			// 上次地图宽度
	int			m_iOldHeight;			// 上次地图高度
	bool        m_bOldIsEmptyMap;       // 上次地图是否为空白地图

	SMapTile *	m_pMap;					// 地图砖块数组
	SMapTile*	m_pOldMap;				// 上次地图的砖块数组
	//LPTexture	m_pOldMiniTex;			// 上次地图小地图
	DWORD      m_dwOldMiniTexID;        // 上次地图小地图

	BOOL		m_bPreReadMap;			// 预读地图

	MBGLayer    m_BgLayers;
	MBGLayer    m_OldBgLayers;          //上次地图的背景

	DWORD       m_dwVersion;
	DWORD       m_dwOldVersion;
	MDynamicObject m_MDynObj;
	MDynamicAppHide m_MDynAppHide;

	bool      m_bIsEmptyMap;//是否为没有读到地图构建的一张空白地图

	DWORD      m_dwPassBlockType;		//允许通行的block类型,见E_ATTR
	CRITICAL_SECTION	m_LoadMapCriSect;//多线程读取地图,有可能另外一个线程下载完了地图以后立即读取


	string      m_strRealDownloadMapFileName;   // 实际地图文件名
	char* m_pDownloadMapDataBuf;	
	unsigned int m_dwDownloadMapDataLen;
	
	string      m_strRealDownloadBgMapFileName;   // 实际地图文件名
	char* m_pDownloadBgMapDataBuf;
	unsigned int m_dwDownloadBgMapDataLen;

public:
	CGameMap();
	~CGameMap();

	// 大地图相关
	DWORD       GetVersion()			{ return m_dwVersion; }
	bool		LoadMap(const char* sMap,bool bReLoad = false,const char* pInputDataBuf = NULL,DWORD dwLen = 0);	// 读取一幅地图，参数为：地图文件名，是否重新加载,传入的地图数据,数据长度
	bool		LoadBgMap(const char* pInputDataBuf = NULL,DWORD dwLen = 0);	// 读取背景地图
	bool		ReadMap(DataStreamInterface* stream,SMapData& info);
	void		ClearMap();				// 清除当前地图数据
	void		CheckLoadMap();
	void		LockDownloadMap();
	void		UnLockDownloadMap();
	SMapTile *	GetTile(int iX,int iY);	// 根据所提供的坐标，返回指定的砖块结点
	bool		IsEnable(void)			{ return (m_pMap != NULL);}
	const char*	GetMapName()			{ return m_strMapFile.c_str();}	// 地图名称
	const char*	GetRealMapName()		{ return m_strRealMap.c_str();}	// 客户端实际的地图名称
	const char* GetMapTitle()           { return m_strMapTitle.c_str(); }
	void        SetMapTitle(const char* title){ m_strMapTitle.assign(title); }
	SMapTile *	GetMap()				{ return m_pMap;		}	// 获取整个地图砖块数字
	int			GetWidth()				{ return m_iWidth;		}	// 获取地图宽度
	int			GetHeight()				{ return m_iHeight;		}	// 获取地图高度

	BOOL		IsPreReadMap()			{ return m_bPreReadMap;	}
	void		SetPreReadMap(BOOL b)	{ m_bPreReadMap = b;	}
	// 小地图相关
	bool		LoadMiniMap(WORD wMap);	// 读取小地图
	void		ClearMiniMap();			// 清除当前地图数据
	//LPTexture	GetMiniTex()			{ return m_pMiniTex;				}
	DWORD	GetMiniTex()			{ return m_dwMiniTexID;				}
	//WORD		GetMiniMapNo()			{ return m_pMiniTex ? (WORD)m_pMiniTex->GetID()		: 0;}
	//WORD		GetMiniMapWidth()		{ return m_pMiniTex ? (WORD)m_pMiniTex->GetWidth0()	: m_iWidth*2;}
	//WORD		GetMiniMapHeight()		{ return m_pMiniTex ? (WORD)m_pMiniTex->GetHeight0(): m_iHeight;}

	//阻挡信息
	BYTE		GetUsed(int iX,int iY);				// 判断给定坐标的砖块是否被使用
	bool		IsBlock(int iX,int iY);				// 判断给定坐标的砖块是否阻挡
	void		SetUserBlock(int iX,int iY);		// 设置阻挡点
	//动态改变了的数据,在跳出地图的时候要还原,比如行会塔改变了tile的object,block
	//1:wSmTile,2:wTile,3:uObject,4:wObjectH,5;dwUnderObj,6:bySound,7:byAttr
	void        AddDynamicData(int iX,int iY,DWORD dwType,DWORD dwData);//dwData:该tile新的数据,跨地图时会自动复原成老的数据
	void        DelDynamicData(int iX,int iY,DWORD dwType);//返回原来的数据
	void        ClearDynamicData();
	MDynamicObject &GetDynObject(){return m_MDynObj;}
	void        AddDynObject(DWORD dwIdx,int iX,int iY);//在指定位置动态增加一个物体
	void        DelDynObject(DWORD dwIdx,int iX,int iY);//在指定位置动态增加一个物体
	void		ModifyObject(DWORD dwIdx,int iX,int iY,DWORD dwType,DWORD dwData);
	void        LoadDynObjData();
	void		LoadDynMinMap();

	void       SetPassBlockType(DWORD dwType){m_dwPassBlockType = dwType;}
	DWORD      GetPassBlockType(){return m_dwPassBlockType;}
	void       AddPassBlockType(DWORD dwType){m_dwPassBlockType |= dwType;}
	void       DelPassBlockType(DWORD dwType){m_dwPassBlockType &= ~dwType;}

	// 其他属性
	//void		StartSceneMusic(char *src = NULL);	// 场景音乐

	//背景数据
	BGLayerData* GetBgLayer(int iLayerNo);
	inline MBGLayer&  GetBgLayers(){ return m_BgLayers;}

	bool IsEmptyMap() const { return m_bIsEmptyMap; }
	void SetIsEmptyMap(bool val) { m_bIsEmptyMap = val; }

	const std::string& GetRealDownloadMapFileName() const { return m_strRealDownloadMapFileName; }
	void SetRealDownloadMapFileName(const std::string& val) { m_strRealDownloadMapFileName = val; }
	char* GetDownloadMapDataBuf() const { return m_pDownloadMapDataBuf; }
	void SetDownloadMapDataBuf(char* val) { m_pDownloadMapDataBuf = val; }
	void SafeDeleteDownloadMapDataBuf() { SAFE_DELETE_ARRAY(m_pDownloadMapDataBuf); }
	unsigned int GetDownloadMapDataLen() const { return m_dwDownloadMapDataLen; }
	void SetDownloadMapDataLen(unsigned int val) { m_dwDownloadMapDataLen = val; }

	const std::string& GetRealDownloadBgMapFileName() const { return m_strRealDownloadBgMapFileName; }
	void SetRealDownloadBgMapFileName(const std::string& val) { m_strRealDownloadBgMapFileName = val; }
	char* GetDownloadBgMapDataBuf() const { return m_pDownloadBgMapDataBuf; }
	void SetDownloadBgMapDataBuf(char* val) { m_pDownloadBgMapDataBuf = val; }
	void SafeDeleteDownloadBgMapDataBuf() { SAFE_DELETE_ARRAY(m_pDownloadBgMapDataBuf); }
	unsigned int GetDownloadBgMapDataLen() const { return m_dwDownloadBgMapDataLen; }
	void SetDownloadBgMapDataLen(unsigned int val) { m_dwDownloadBgMapDataLen = val; }

	int CanThrough(int iStartX,int iStartY, int iEndX,int iEndY);

};