#pragma once

#include "Global/Global.h"
#include "Global/MemPool.h"
#include "GameData/ActionStruct.h"
#include "GameData/SimpleGood.h"
#include "GameData/SimpleCharacter.h"
#include "Global/MathUtil.h"
#include "PathPublic.h"

#include <vector>
#include <list>
#include <deque>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#define STEP_LIMIT (805*805-1)    // 1024 * 1024

#define TILE_BLOCK_BIT		(0x00000003)
#define SIGN_INT3(a)		((a > 0 ? 1 : (a==0 ? 0 : -1)))

#define NEW_PATHPOINT_T()	PathPoint_t::NEW_PathPoint_t()
#define DEL_PATHPOINT_T(p)	PathPoint_t::DEL_PathPoint_t(p)

struct PathPoint_t : public SPoint
{
	DECLARE_POOL(PathPoint_t);		//

	PathPoint_t();
	PathPoint_t(SPoint sp){x =sp.x;y = sp.y;};
};
typedef std::list<PathPoint_t *> ListPoint;
typedef std::vector<PathPoint_t> vectorPoint;
void ReleaseListPoint(ListPoint &lP);
void MovePathPoint(ListPoint &dst,ListPoint &src);

//A*寻路
//2005-4-18
#define NEW_NODE_T()	Node_t::NEW_Node_t()
#define DEL_NODE_T(p)	Node_t::DEL_Node_t(p)

// 树节点
struct Node_t
{	
	DECLARE_POOL(Node_t);			// 缓冲

	long	f, h;
	int		g, tmpg;	//gravity,
	int		x, y;
	int		NodeIndex;

	Node_t *Parent;
	Node_t *Child[8];	// a node may have upto 8+(NULL) children.

	Node_t *NextNode;	// for filing purposes
	Node_t()
	{
		NodeIndex = x = y = tmpg = g = f = h = 0;
		Parent = NextNode = NULL;
		for(int i = 0;i < 8;i++)
			Child[i] = NULL;
	}
};
#define NEW_STACK_T()	Stack_t::NEW_Stack_t()
#define DEL_STACK_T(p)	Stack_t::DEL_Stack_t(p)
// 搜索栈
struct Stack_t 
{	
	DECLARE_POOL(Stack_t);			// 缓冲

	Node_t *NodePtr;
	Stack_t *NextStackPtr;
	Stack_t()
	{
		NodePtr			= NULL;
		NextStackPtr	= NULL;
	}
};

//应用逻辑AI,防止死锁,2005-4-27
#define StatisticMode	4	//最多统计模式点，目的是匹配2点，3点，4点 往复运动模型 1，a-b-a-b-a-b-a-b-;2,a-b-c-b-a-b-c-b;3,a-b-c-d-c-b-a-b-c-d-c-b
#define StatisticNum	((StatisticMode-1)<<3)	
#define StatisticID		4
struct UserPath_t
{
	DWORD id;				//使用路径的ID
	DWORD t;				//最后一次使用路径的时间，用于LRU淘汰算法
	DWORD nums;				//点个数
	SPoint sp[StatisticNum];//需要原始统计地图点
};

//搜索回调

//拾取
typedef bool (*fnCanPick)(CSimpleGood *pGood,int index);
bool AI_CanPick(CSimpleGood *pGood,int index);	//默认拾取

//攻击
typedef bool (*fnCanAttack)(CSimpleCharacter *pChar);
bool AI_CanAttack(CSimpleCharacter *pChar);	//默认攻击
bool AI_GetDirStep(int iDir,int &iX,int &iY);
extern POINT g_ptDIROff[];

//用户应用搜索条件,适用于一屏幕内带怪物的搜索
struct UserPathFindingInfo_t
{
	char MapID[64];				//地图ID
	int iStartX,iStartY;	//网格相对整体地图偏移

	int nRows;				//网格
	int nCols;
	DWORD *pBlock;
	int sx,sy;				//起点
	int dx,dy;				//终点

	void SetValue(int _sx,int _sy,int _dx,int _dy);
	bool IsOnPath(UserPathFindingInfo_t &ui,SPoint &begin,SPoint &end);	//
	int  GoTile(int _dx,int _dy);


	static bool AI_IsSurroundByMonster(int sx,int sy,int _nRows,int _nCols,DWORD *_pBlock);
	static UINT AI_GetDirectionalAttackableMonster(int iSelfX,int iSelfY,int iDir);
	static UINT AI_GetAttackableMonster(int iAimX,int iAimY,fnCanAttack fnAttackCB = AI_CanAttack);
	static BOOL AI_GetPickableItem(int iAimX,int iAimY,UINT &uid,fnCanPick fnPickCB = AI_CanPick);
};


class	CPathFinder
{
public:
	CPathFinder();
	virtual ~CPathFinder();

	//寻径并返回第一个可用的转向点
	bool GetPathClosePoint(UserPathFindingInfo_t &ui,SPoint *ptClose,BOOL bOffDest = FALSE,BOOL bForceFind = FALSE,int *iIntegrity = 0);

	//寻径
	bool	CreatePath(int nRows,int nCols,DWORD *pBlock,int sx, int sy, int dx, int dy,int *iIntegrity);
	bool	GetPath(SPoint **ptList,int *ptCounts);			// 返回全路径

	bool    GetPath(UserPathFindingInfo_t &ui,vectorPoint &vP);
	bool    GetPath(UserPathFindingInfo_t &ui,ListPoint &lP);

	bool	GetNextClosePoint(SPoint *ptClose);				// 取得路径的下一个转向点
	bool    SetOptimizeStep(int iStep);						// 设置优化调整步长

	bool    AddUserPath(DWORD id,int x, int y);				// 存储用户路径

	bool	PathOK(); // Call and check this function before using these 3 following
	inline void	PathNextNode();
	inline int	GetNodeX();
	inline int	GetNodeY();

	// other usefull functions (do not change them they are used by the A* algorithm)
	int GetTileIndex(int x, int y);	

	virtual int IsBlock(int x, int y);		// returns 1 if we can move on it

	void SetMaxSearch(int _MaxSearch = 324){m_MaxSearch= _MaxSearch;};
	void SetSearchDir(int _SearchDir = 0){m_SearchDir= _SearchDir;};

protected:
	void FreeNodes();

protected:
	// 搜索空间参数
	UserPathFindingInfo_t m_UserPathFindingInfo;

	int     m_SearchDir;	//1正向；0反向
	int		m_MaxSearch;	//最大搜索步

	int     m_OptimizeStep;	//
	int		m_nRows,		
			m_nCols;		// to allocate memory for the
	DWORD	*m_pBlock;		// pointer to the A* own tilemap data array

	int		GetRows();		
	int		GetCols();		

	// 搜索树
#define		TILESIZE	1  // change this also to reflect tile size. 64x32.
#define		CHILDNUM	(( m_bDirs8 == true ) * 4 + 4)

	Node_t *m_pListOpen;		// the node list pointers
	Node_t *m_pListClosed;
	Node_t *m_pPath;			// pointer to the best path

	BYTE*   m_pBitArray;     //判断点是否已经处理过了

	Stack_t *m_pStack;
	bool	m_bIsPathing;
	bool	m_bDirs8;

	UserPath_t m_UserPath[StatisticID];	
protected:
	UserPath_t *GetUserPath(DWORD id);

protected:
	// The A* Algorithm
	void FindPath(int sx, int sy, int dx, int dy,int *iIntegrity = 0);
	Node_t *ReturnBestNode(void);
	void GenerateSuccessors(Node_t *BestNode, int dx, int dy);
	void GenerateSuccessorsForOneDir(Node_t *BestNode,int x, int y, int dx, int dy);
	Node_t *CheckOpen(int tilenum);
	Node_t *CheckClosed(int tilenum);
	void Insert(Node_t *Successor);
	void PropagateDown(Node_t *Old);

	void Push(Node_t *Node); 
	Node_t *Pop(void);

	// 广度优先算法
	void FindPathEx(int sx,int sy,int dx,int dy,int *iIntegrity = 0);

	SPoint		*g_WayPoints;
	int			g_WayPointCounts;
	int			g_WayPointCurrent;

protected:
	void		InitWayPoints(int iSize = 512);
};

inline void	CPathFinder::PathNextNode()
{
	if (m_pPath) 
	{
		m_pPath = m_pPath->Parent; 
	}
}

inline int	CPathFinder::GetNodeX()
{ 
	if (!m_pPath) 
	{
		return 0;
	}
	return m_pPath->x; 
}

inline int	CPathFinder::GetNodeY()
{
	if (!m_pPath) 
	{
		return 0;
	}
	return m_pPath->y; 
}



//地图寻路
class	CMapPathFinder : public CPathFinder 
{
public:
	CMapPathFinder();
	~CMapPathFinder();

	//**寻路
	//是否为阻挡点
	virtual int IsBlock(int x, int y);		// returns 1 if we can move on it
	//创建地图路径
	bool	CreatePath(int sx, int sy, int dx, int dy,int *iIntegrity = 0);
	//返回全路径点队列
	ListPoint *GetPath();		
	//取得路径的下一个转向点
	bool	GetNextClosePoint(SPoint *ptClose);	
	//保存一个路径点
	int		AddPathPoint(int x, int y ,bool bHead = true);
	//清空路径点队列
	void	ReleasePathPoint();
	//优化
	void	Optimaize(bool bEnable = false);
	//重新创建路径
	bool	ReCreatePath(int sx, int sy,int _MapID = 0);
	//到终点的直线Tile距离
	long	Distance2Dst(int sx, int sy);

	//**航路应用
	//Route -- 长路经点，对创建出来的tile点路径进行删简，减少路径点的存储；
	//			具体应用时，还可以在一个屏幕内使用更加细化和结合怪物等动态阻挡信息的路径搜索，以便越过这些阻挡点。
	void	GetRoute(vectorPoint &vp);
	void	UsePathAsCurrentRoute(const char* _MapID = 0);	//把当前的路径作为航路，同时删除搜索结果
	bool    ReachRouteEnd(int x,int y,int limit = 3); //

	bool	IsLeftRoutePoint();
	bool    ReachCurrentRoutePoint(int x,int y,int limit = 3); //
	void	GetCurrentRoutePoint(int &x,int &y);
	bool	AdvanceRoutePoint();
	bool    JumpAdvanceRoutePoint(int iSelfX,int iSelfY,int iStep = 4);
	bool	IsOnRoute();
	bool	IsRightWalkingSpeed();
	bool    SetOnRoute(bool bOnRoute);
	void    SetMapID(const char* _MapID);
	const char* GetMapID();
	const char*	GetCurrentMapID();
	vectorPoint& GetCurrentRoute(){return m_CurrentRoute;}
	int GetDestX(){return m_DstPathPoint.x;}
	int GetDestY(){return m_DstPathPoint.y;}

protected:
	//Route -- 航路点
	string	m_MapID;					//  当前的地图ID
	bool	m_OnRoute;					//	判定是否在当前的路径上
	vectorPoint m_CurrentRoute;			//	存储当前的航路点；每16格为航路点；如果为空表示已经走到
	PathPoint_t m_CurrentRoutePoint;	//	当前航路点

	ListPoint m_PathPoints;				//	存储搜索的航路点
	PathPoint_t m_DstPathPoint;			//  禁止重复寻路
	DWORD		m_dwCurrentRouteTime;	//	每个路经点的行走时间；用于判断超时太多，则重新生成路径
};


//代理，申请者公用的任务标记
#define PFT_ERROR		-1	//
#define PFT_IDLE		0	//[代理]当前搜索代理处于空闲，可以执行任务；[申请]无
#define PFT_RUNNING		1	//[代理]当前搜索代理处于搜索状态，不可以执行任务；[申请]同
#define PFT_FINISHED	2	//[申请]搜索请求已经被完成；
#define PFT_WAITING		3	//[申请]搜索请求需要等待代理的PFT_IDLE；[代理]任务已经完成，但无法提交

#define PFT_RESULT_ERROR 1
//存储搜索任务和结果
struct MapPathTask_t
{
	int		tag;				//  任务标记	
	DWORD	begin;				//  开始任务请求时间，如果超过5分钟，抛弃等待任务
	DWORD   result;				//  存储路径查找结果，ｅｒｒｏｒ　＝１

	MapPoint_t	src;
	MapPoint_t	dst;
	ListPoint	m_PathPoints;	//	原始路径点

	MapPathTask_t(){Clear();};
	void Clear(bool bRoute = false);
	void Clone(MapPathTask_t &mpt,bool bRoute = false,bool ClearSrc = false);
	bool IsWaitingTask(const char* MapID,int sx, int sy, int dx, int dy);
	int  RelateTask(MapPathTask_t &mpt);	//判断任务关系
};

inline bool TaskIsWaitTooManyTime(DWORD a)
{
	return (labs(GetTickCount() - a) >= 120000);
};

inline bool QueueIsWaitTooManyTime(DWORD a)
{
	return (labs(GetTickCount() - a) >= 200);//在200ms内，没有申请者提走，则放弃
};

/*
**	地图寻路代理类[线程代理]
	当地图请求线程寻路(通常，复杂的寻路都需要通过线程进行寻路，目的是防止阻塞主线程)时；
	应当等待线程执行操作完成
*/
class	CMapPathFinderAgent : public CMapPathFinder 
{
public:
	CMapPathFinderAgent();
	~CMapPathFinderAgent();

	bool HavePathFindTask();	//如果有未完成的任务
	bool ExcutePathFindTask();	//执行任务，并且把结果返回给m_CurrentTask
	bool FinishPathFindTask();
	bool AcceptTask(MapPathTask_t &mpt);//代理接受任务
	int  RelayTaskRusult(MapPathTask_t &mpt);//代理判断并转发任务结果

	void InitPathFindingThread();
	void BeginPathFindingThread();
	void ExitPathFindingThread();
	BOOL IsMapPathFindingThreadRunning()	{ return m_bMapPathFindingThreadRunning;}
	BOOL IsRunMapPathFindingThread()		{ return m_bRunMapPathFindingThread;	}
	void Lock();
	void UnLock();

protected:
	HANDLE m_hMapPathFindingThread		;		//线程句柄
	DWORD m_dwMapPathFindingThreadId	;		//线程句柄标记
	BOOL m_bMapPathFindingThreadRunning ;		//运行标记
	BOOL m_bRunMapPathFindingThread		;		//是否打开运行标记

	DEFINE_HANDLE(m_hMapPathLock);				


protected:
	MapPathTask_t	m_CurrentTask;		//	[代理，申请者]当前任务
	MapPathTask_t	m_LastTask;			//	[代理]已经完成的任务，等待申请者轮寻，如果超时，那么不等待
										//  处于PFT_WAITING的申请，如果代理空闲，那么，可以提交任务；并且
										//  处于PFT_WAITING的申请
};

/*
**	地图寻路客户类[线程寻路应用的基类]
	请求CMapPathFinderAgent寻路，并存储返回结果
*/
class	CMapPathFinderClient : public CMapPathFinder 
{
public:
	CMapPathFinderClient();
	~CMapPathFinderClient();
public:
	virtual void UseTaskPathAsCurrentRoute();	//把m_CurrentTask作为航路，同时删除搜索结果
	virtual int  TaskRunning();					//判断当前是否有寻路任务；
	virtual bool PushPathFindTask(int sx, int sy, int dx, int dy);//地图路径寻找
	bool    ImmediatePathFindTask(int sx,int sy,int dx,int dy,int iLimit = 10); //地图立即查找
	bool    ReFindPathImm(int sx,int sy);//重新提交寻路的任务,立即查找
	
	bool    SubmitTask();							//申请者向代理提交任务
	bool    ReSubmitFindTask(int sx,int sy);      //重新提交寻路的任务
	MapPoint_t& GetPathFinderTarget(){return m_CurrentTask.dst;}
protected:
	MapPathTask_t	m_CurrentTask;		//	[代理，申请者]当前任务
};

extern	CMapPathFinderAgent	   g_MapPathFinderAgent;


