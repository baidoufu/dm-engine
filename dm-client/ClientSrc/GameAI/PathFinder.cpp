#include "Global/DebugTry.h"
#include "GameData/ActionStruct.h"
#include "GameData/GameGlobal.h"
#include "GameData/GameData.h"
#include "PathFinder.h"
#include "GameMap/GameMap.h"
#include <math.h>

//#ifdef _DEBUG
//#define AI_DEBUG
//#endif

Node_t  m_NodeList[STEP_LIMIT + 2]; //节点列表

//
//以下的函数不是线程安全的，可能？
//
class CPathFindLock
{
public :
	CPathFindLock()
	{
		INIT_HANDLE(m_hPathFindLock);				// 
	};
	~CPathFindLock()
	{
		DELETE_HANDLE(m_hPathFindLock);
	};
public :
	void Lock()
	{
		LOCK_HANDLE(m_hPathFindLock);
	};
	void UnLock()
	{
		UNLOCK_HANDLE(m_hPathFindLock);
	};
protected :
	DEFINE_HANDLE(m_hPathFindLock);				
};

CPathFindLock g_PathFindLock;


IMPLEMENT_POOL(Node_t,5120,3200);
IMPLEMENT_POOL(Stack_t,1280,160);
IMPLEMENT_POOL(PathPoint_t,5120,640);

Node_t *NewNode()
{
	Node_t *p = 0;

	g_PathFindLock.Lock();
	p = NEW_NODE_T();
	memset(p,0,sizeof(Node_t));
	g_PathFindLock.UnLock();

	return p;
};
void    DeleteNode(Node_t *&p)
{
	g_PathFindLock.Lock();
	DEL_NODE_T(p);
	p = 0;
	g_PathFindLock.UnLock();
};

Stack_t *NewStack()
{
	Stack_t *p = 0;

	g_PathFindLock.Lock();
	p = NEW_STACK_T();
	memset(p,0,sizeof(Stack_t));
	g_PathFindLock.UnLock();

	return p;
};
void    DeleteStack(Stack_t *&p)
{
	g_PathFindLock.Lock();
	DEL_STACK_T(p);
	p = 0;
	g_PathFindLock.UnLock();
};

PathPoint_t *NewPathPoint()
{
	PathPoint_t *p = 0;

	g_PathFindLock.Lock();
	p = NEW_PATHPOINT_T();
	memset(p,0,sizeof(PathPoint_t));
	g_PathFindLock.UnLock();

	return p;
};

void    DeletePathPoint(PathPoint_t *&p)
{
	g_PathFindLock.Lock();
	DEL_PATHPOINT_T(p);
	p = 0;
	g_PathFindLock.UnLock();
};

int AddPathPoint(ListPoint &lP,int x, int y,bool bHead)
{
	PathPoint_t *p = NewPathPoint();
	if(p)
	{
		p->x = x;
		p->y = y;

		if(bHead)
			lP.push_front(p);
		else 
			lP.push_back(p);
	}

	return (int)lP.size();
};

void ReleaseListPoint(ListPoint &lP)
{
	for(ListPoint::iterator itr = lP.begin();	itr != lP.end();	++itr)
	{
		PathPoint_t *p = (*itr);
		DeletePathPoint(p);
	}
	lP.clear();
};

void MovePathPoint(ListPoint &dst,ListPoint &src)
{
	dst.insert(dst.begin(),src.begin(),src.end());
	src.clear();
};


PathPoint_t::PathPoint_t():SPoint()
{
}

//------------------------------------------------------------------------------
// Purpose : PointOnTileLine : 判断n个点在同一条格子直线上
// Input   : 
// Output  :
//------------------------------------------------------------------------------
bool PointOnTileLine(SPoint *pt ,int nCount)
{
	if(!pt || nCount<2)
		return false;
	int dx = pt[1].x - pt[0].x ;
	int dy = pt[1].y - pt[0].y ;
	for(int i = 2;i<nCount;++i)
	{
		if((pt[i].x - pt[i-1].x != dx) || (pt[i].y - pt[i-1].y != dy))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------
// Purpose : AI_Value : 搜索评估函数
// Input   : 起点(sx, sy),终点(dx, dy)
// Output  : 
//------------------------------------------------------------------------------
long AI_Value(int sx, int sy, int dx, int dy)
{
//	long distance = (dx-sx)*(dx-sx) + (dy-sy)*(dy-sy);return (long)(sqrt((double)distance));
//	long distance = max(labs(dx-sx) , labs(dy-sy));return distance;
	long distance = (dx-sx)*(dx-sx) + (dy-sy)*(dy-sy);return distance;
// 	long distance = 20*(abs(dx-sx) + abs(dy-sy));return distance;
//	long distance = max(labs(dx-sx) , labs(dy-sy));return distance;
//	long distance = long(0.5+sqrt((dx-sx)*(dx-sx) + (dy-sy)*(dy-sy)));return distance;
}

//大格ID索引定义
//0,1,2
//7,8,3
//6,5,4
#define AI_GRID_MAX 8
POINT g_ptSearchRgn[AI_GRID_MAX] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
//------------------------------------------------------------------------------
// Purpose : AI_SearchPossibleGrid : 根据起点和终点计算搜索的节点顺序
// Input   : 起点(sx, sy),终点(dx, dy)
// Output  : nOut[9]
//------------------------------------------------------------------------------
bool AI_SearchPossibleGrid(int sx, int sy, int dx, int dy,int nOut[])
{
	int nDir = 0;
	nDir = GetDir8(  dx, dy , sx, sy );
	nOut[0] = (nDir  +(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[1] = (nDir+1+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[2] = (nDir-1+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[3] = (nDir+2+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[4] = (nDir-2+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[5] = (nDir+3+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[6] = (nDir-3+(AI_GRID_MAX)) % (AI_GRID_MAX);
	nOut[7] = (nDir+4+(AI_GRID_MAX)) % (AI_GRID_MAX);

	return true;
};


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

const int MAX_WayPointCounts = 1024;

////////////////////////////////////////////////////////////////////////////////
//                           Constructor Destructor                           //
////////////////////////////////////////////////////////////////////////////////

CPathFinder::CPathFinder()
{
	m_pStack		= NewStack();
	m_bIsPathing	= false;
	m_pListOpen		= NULL;
	m_pListClosed	= NULL;
	m_pPath			= NULL;

	m_SearchDir		= 0;			//1正向；0反向
	m_MaxSearch		= 324;
	m_pBlock		= NULL;
	m_nRows			= 0;		
	m_nCols			= 0;	
	

	m_bDirs8		= 1;
	m_OptimizeStep	= 2;			//

	m_pBitArray     = NULL;

	g_WayPoints			= 0;
	g_WayPointCounts	= 0;
	g_WayPointCurrent	= 0;

	InitWayPoints(MAX_WayPointCounts);


	memset(m_UserPath,0,sizeof(m_UserPath));
	memset(&m_UserPathFindingInfo,0,sizeof(UserPathFindingInfo_t));
}

////////////////////////////////////////////////////////////////////////////////

CPathFinder::~CPathFinder()
{
	FreeNodes();
	DeleteStack(m_pStack);
	InitWayPoints(0);
}

void CPathFinder::InitWayPoints(int iSize)
{
	if(g_WayPoints)
		delete []g_WayPoints;
	g_WayPoints			= 0;
	g_WayPointCounts	= iSize;
	g_WayPointCurrent	= 0;

	if(g_WayPointCounts > 0)
	{
		g_WayPoints = new SPoint[g_WayPointCounts];
	}
};


inline int AI_IsBlock(int nRows,int nCols,DWORD *pBlock,int x, int y)
{
	if( x<0 || y<0 || x>=nCols || y>=nRows || !pBlock ) 
		return 1;
	return (pBlock[y*nCols + x] & TILE_BLOCK_BIT);
};		// returns 1 if we can move on it

//
inline int AI_GoTile(int nRows,int nCols,DWORD *pBlock,int sx,int sy,int dx,int dy)
{
	int deltax = dx - sx;
	int deltay = dy - sy;
	if(deltax == 0 && deltay == 0 )
		return 0;

	int absx = abs(deltax);
	int absy = abs(deltay);
	if(absx && absy && (absy!=absy))	//本地图上不可能!
		return 0;
	int distance = max(absx,absy);

	deltax			= SIGN_INT3(deltax);
	deltay			= SIGN_INT3(deltay);

	for(int i = 0;i<distance;++i)
	{
		sx+=deltax;
		sy+=deltay;

		if(AI_IsBlock(nRows, nCols,pBlock,sx,sy))
			return false;
	}
	return 1;
};


void UserPathFindingInfo_t::SetValue(int _sx,int _sy,int _dx,int _dy)
{
	strncpy(MapID, g_pGameMap->GetMapName(), sizeof(MapID) - 1);
	MapID[sizeof(MapID) - 1] = '\0';
	pBlock= SELF.GetOppBlock();
	SELF.GetOppStartXY(iStartX,iStartY);
	nRows= MAPARR_WIDTH;
	nCols= MAPARR_WIDTH;
	sx=_sx - iStartX;
	sy=_sy - iStartY;
	dx=_dx - iStartX;
	dy=_dy - iStartY;
};

bool UserPathFindingInfo_t::IsOnPath(UserPathFindingInfo_t &ui,SPoint &begin,SPoint &end)
{
	if(strcmp(ui.MapID, MapID) == 0)
	{
		if((begin.x + iStartX) == (ui.sx + ui.iStartX) 
			&& (begin.y + iStartY) == (ui.sy + ui.iStartY)
//			&& (end.x + iStartX) == (ui.dx + ui.iStartX)
//			&& (end.y + iStartY) == (ui.dy + ui.iStartY))
			&& (dx + iStartX) == (ui.dx + ui.iStartX)
			&& (dy + iStartY) == (ui.dy + ui.iStartY))
		{
			return true;
		}
	}
	return false;
};	//

int UserPathFindingInfo_t::GoTile(int _dx,int _dy)
{
	return  AI_GoTile( nRows, nCols,pBlock, sx, sy,_dx,_dy);
};

POINT g_ptDIROff[]=
{
	{  0,-1},
	{ +1,-1},
	{ +1, 0},
	{ +1,+1},
	{  0,+1},
	{ -1,+1},
	{ -1, 0},
	{ -1,-1}
};

bool AI_GetDirStep(int iDir,int &iX,int &iY)
{
	iX = g_ptDIROff[iDir].x;
	iY = g_ptDIROff[iDir].y;
	return true;
};

bool UserPathFindingInfo_t::AI_IsSurroundByMonster(int sx,int sy,int _nRows,int _nCols,DWORD *_pBlock)
{
	for(int i = 0 ;i < ARRAYSIZE(g_ptDIROff);++i)
	{
		if(AI_IsBlock(_nRows,_nCols,_pBlock,sx+g_ptDIROff[i].x,sy+g_ptDIROff[i].y))
			return true;
	}
	return false;
};
UINT UserPathFindingInfo_t::AI_GetDirectionalAttackableMonster(int iSelfX,int iSelfY,int iDir)
{
	//
	iDir-=1;
	iDir%=8;

	UINT id = 0;
	int tx,ty;
	for(int i = 0;i<8 && !id;i++)
	{
		tx	= iSelfX + g_ptDIROff[iDir].x;
		ty	= iSelfY + g_ptDIROff[iDir].y;
		id = AI_GetAttackableMonster(tx,ty);
		++iDir;
		iDir%=8;
	}
	return 0;
};

//默认攻击
bool AI_CanAttack(CSimpleCharacter *pChar)
{
	if(!pChar->IsMonster())
		return false;
	if(pChar->IsDead())
		return false;

	return true;
};

//怪物攻击
UINT UserPathFindingInfo_t::AI_GetAttackableMonster(int iAimX,int iAimY,fnCanAttack fnAttackCB)
{
	if(!fnAttackCB)
		return 0;

	CSimpleCharacterNode *pChar = MapArray.GetSimpleCharacterHead(iAimX,iAimY);
	while(pChar)
	{
		if(fnAttackCB(pChar))
		{
			return pChar->GetID();
		}
		pChar = pChar->m_MapNext;
	}
	return 0;
};

/*
//默认拾取
bool AI_CanPick(CSimpleGood *pGood,int index)
{
//	if(pGood->IsDead())
//		return false;
//	if(pGood->GetLooks() STDMODE_PLANT_START)
	return true;
};	
*/
/*
//物品拾取
//返回
//BUG20060306多个物品或者丢弃的物品位于同一个格子，系统将拾取不该拾取的东西；
BOOL UserPathFindingInfo_t::AI_GetPickableItem(int iAimX,int iAimY,UINT &uid,fnCanPick fnPickCB)
{
	if(!fnPickCB)
		return 0;

	int index =0;
	CSimpleGoodNode *pGood = MapArray.GetSimpleGoodHead(iAimX,iAimY);
	while(pGood)
	{
		if(fnPickCB(pGood,index++))
		{
			uid = pGood->GetID();
			return 1;
		}
		pGood = pGood->m_MapNext;
	}
	return 0;
};
*/
//------------------------------------------------------------------------------
// Purpose : GetPathClosePoint : 寻径并返回第一个可用的转向点
// Input   : 
// Output  :
//------------------------------------------------------------------------------
bool CPathFinder::GetPathClosePoint(UserPathFindingInfo_t &ui,SPoint *ptClose,BOOL bOffDest ,BOOL bForceFind,int *iIntegrity)
{TRY_BEGIN
	int nRows;
	int nCols;
	DWORD *pBlock;
	int sx,  sy,  dx,  dy;

	if(!ptClose)
		return false;

	//如果终点当前存储的路径一致，并且下一点在当前信息表中可以行走
	if(g_WayPointCounts > 1 && ptClose && !bForceFind && g_WayPointCurrent<g_WayPointCounts)
	{
		if(m_UserPathFindingInfo.IsOnPath(ui,g_WayPoints[g_WayPointCurrent],g_WayPoints[g_WayPointCounts-1]))
		{		
			if(GetNextClosePoint(ptClose))
			{
				//本网格转化到新网格
				ptClose->x += m_UserPathFindingInfo.iStartX -ui.iStartX ;
				ptClose->y += m_UserPathFindingInfo.iStartY -ui.iStartY;

				//判断在新网格上是否可以走
				if(ui.GoTile(ptClose->x ,ptClose->y))
				{
#ifdef AI_DEBUG
					output_debug("原路: %d:%d \n",ptClose->x ,ptClose->y);
#endif
					ptClose->x += ui.iStartX;
					ptClose->y += ui.iStartY;
					return true;
				}
			}
		}
	}

	memcpy(&m_UserPathFindingInfo,&ui,sizeof(m_UserPathFindingInfo));

	nRows = ui.nRows;
	nCols = ui.nCols;
	pBlock = ui.pBlock;
	sx = ui.sx;
	sy = ui.sy;
	dx = ui.dx;
	dy = ui.dy;

	int nSearchOut[AI_GRID_MAX];
	bool bRes = CreatePath( nRows, nCols,pBlock, sx, sy,  dx, dy,iIntegrity);
	if(!bRes)
	{
		if(IsBlock(sx, sy))
		{
			//起始点阻挡,可能站在怪物身上,直接寻找周围一个空点走过去
			AI_SearchPossibleGrid( sx,  sy, dx,  dy, nSearchOut);
			for(int i = 0;i<AI_GRID_MAX ;++i)
			{
				int tempx = g_ptSearchRgn[nSearchOut[i]].x+sx;
				int tempy = g_ptSearchRgn[nSearchOut[i]].y+sy;
				if(!IsBlock(tempx, tempy))
				{
					g_WayPoints[0].x = sx;g_WayPoints[0].y = sy;
					g_WayPoints[1].x = tempx;g_WayPoints[1].y = tempy;
					g_WayPointCounts = 2;

					if(GetNextClosePoint(ptClose))
					{
						ptClose->x += ui.iStartX;
						ptClose->y += ui.iStartY;
						return true;
					}
					return false;
				}
			}
			return false;
		}

		if(IsBlock(dx, dy))
		{
			//如果起点和终点在1格范围内，则直接结束
			if((dx-sx) * (dx-sx) + (dy-sy) * (dy-sy) <= 2)
			{
				return false;
			}
		}

		//目的点阻挡,取得一个合适的替换点。
		if(bOffDest)
		{
			AI_SearchPossibleGrid( dx,  dy, sx,  sy, nSearchOut);
			for(int i = 0;i<AI_GRID_MAX && !bRes;++i)
			{
				bRes = CreatePath( nRows, nCols,pBlock, sx, sy,  g_ptSearchRgn[nSearchOut[i]].x+dx, g_ptSearchRgn[nSearchOut[i]].y+dy,iIntegrity);
			}
		}
		bOffDest = 0;
	}
	if(!bRes)
		return false;

	//终点偏移
	if(bOffDest)
	{
		if(g_WayPointCounts > 1)
		{
			SPoint ptLast		= g_WayPoints[g_WayPointCounts - 1];
			SPoint ptLastPre	= g_WayPoints[g_WayPointCounts - 2];
			int deltax			= ptLast.x - ptLastPre.x;
			int deltay			= ptLast.y - ptLastPre.y;
			g_WayPointCounts--;
			if(labs(deltax) >1 || labs(deltay) > 1 )
			{
				deltax			= SIGN_INT3(deltax);
				deltay			= SIGN_INT3(deltay);
				for(int i = 0;i<(m_OptimizeStep - 1) && (g_WayPointCounts < MAX_WayPointCounts);++i)
				{
					g_WayPoints[g_WayPointCounts].x = g_WayPoints[g_WayPointCounts - 1].x + deltax;
					g_WayPoints[g_WayPointCounts].y = g_WayPoints[g_WayPointCounts - 1].y + deltay;
					g_WayPointCounts++;
				}
			}
		}
	}
	if(GetNextClosePoint(ptClose))
	{
		ptClose->x += ui.iStartX;
		ptClose->y += ui.iStartY;
		return true;
	}
	return false;
TRY_END_RETURN(0)};	

//------------------------------------------------------------------------------
// Purpose : CreatePath : 寻径
// Input   : 搜索空间:nRows, nCols,pBlock ;起点(sx,sy);终点(dx, dy)
// Output  : 返回成功或者失败
//------------------------------------------------------------------------------
bool CPathFinder::CreatePath(int nRows,int nCols,DWORD *pBlock,int sx,int sy, int dx,int dy,int *iIntegrity)
{TRY_BEGIN
	SPoint	po;

	if(nRows < 2 || nCols < 2 || !pBlock)
		return false;

	g_WayPointCounts = 0;
	g_WayPointCurrent = 0;

	m_nRows		= nRows;
	m_nCols		= nCols;
	m_pBlock	= pBlock;

	if(IsBlock(dx,dy))
		return (m_bIsPathing = false);

	if(IsBlock(sx,sy))
		return (m_bIsPathing = false);

	if(GetTileIndex(sx,sy) == GetTileIndex(dx,dy))
		return (m_bIsPathing = false);


	FreeNodes();

	if(m_SearchDir)
		FindPath(sx,sy,dx,dy,iIntegrity);	//正向搜索
	else
		FindPath(dx,dy,sx,sy,iIntegrity);	//为了优化搜索，反向；尽量实现跑步优化；形成的点为反向!!!2005-4-22

	m_bIsPathing = true;

	int ptCounts = 0;
	SPoint ptTemp[4];

	while ( !PathOK() )		
	{
		po.x = GetNodeX();
		po.y = GetNodeY();

		ptTemp[ptCounts++] = po;
		if(m_OptimizeStep == 1)
		{
			if(g_WayPointCounts < MAX_WayPointCounts)
				g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);
			ptCounts = 0;
		}
		else if(m_OptimizeStep == 2)
		{
			if (ptCounts >= 3)
			{
				if(g_WayPointCounts < MAX_WayPointCounts)
					g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);

				if(PointOnTileLine(ptTemp,3))
				{
					ptTemp[0] = ptTemp[2];
					ptCounts = 1;
				}
				else
				{
					ptTemp[0] = ptTemp[1];
					ptTemp[1] = ptTemp[2];
					ptCounts = 2;
				}
			}
		}
		else if(m_OptimizeStep == 3)
		{
			if (ptCounts >= 4) 
			{
				if(g_WayPointCounts < MAX_WayPointCounts)
					g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);

				if(PointOnTileLine(ptTemp,4))
				{
					ptTemp[0] = ptTemp[3];
					ptCounts = 1;
				}
				else
				{
					ptTemp[0] = ptTemp[1];
					ptTemp[1] = ptTemp[2];
					ptTemp[2] = ptTemp[3];
					ptCounts = 3;
				}
			}
		}
		PathNextNode();
	}
	po.x = GetNodeX();
	po.y = GetNodeY();
	ptTemp[ptCounts++] = po;

	if(m_OptimizeStep == 1)
	{
		if(g_WayPointCounts<MAX_WayPointCounts)
			g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);
		ptCounts = 0;
	}
	else if(m_OptimizeStep == 2)
	{
		if (ptCounts >=3) 
		{
			if(g_WayPointCounts < MAX_WayPointCounts)
				g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);

			if(PointOnTileLine(ptTemp,3))
			{
				ptTemp[0] = ptTemp[2];
				ptCounts = 1;
			}
			else
			{
				ptTemp[0] = ptTemp[1];
				ptTemp[1] = ptTemp[2];
				ptCounts = 2;
			}
		}
	}
	else if(m_OptimizeStep == 3)
	{
		if (ptCounts >= 4) 
		{
			if(g_WayPointCounts<MAX_WayPointCounts)
				g_WayPoints[g_WayPointCounts++] = (ptTemp[0]);

			if(PointOnTileLine(ptTemp,4))
			{
				ptTemp[0] = ptTemp[3];
				ptCounts = 1;
			}
			else
			{
				ptTemp[0] = ptTemp[1];
				ptTemp[1] = ptTemp[2];
				ptTemp[2] = ptTemp[3];
				ptCounts = 3;
			}
		}
	}

	for(int i = 0;i<ptCounts;++i)
	{
		if(g_WayPointCounts<MAX_WayPointCounts)
			g_WayPoints[g_WayPointCounts++] = (ptTemp[i]);
	}

	if(0 == m_SearchDir)
	{
		//调整顺序
		for(int i = 0;i<g_WayPointCounts/2;++i)
		{
			po = g_WayPoints[i];
			g_WayPoints[i] = g_WayPoints[g_WayPointCounts - 1 - i];
			g_WayPoints[g_WayPointCounts - 1 - i] = po;
		}
	}

	if(g_WayPointCounts > 1)
	{
		SPoint ptFisrt = g_WayPoints[0];
		SPoint ptFisrt2 = g_WayPoints[1];
		if(ptFisrt.x != sx|| ptFisrt.y != sy)
		{
			g_WayPointCounts = 0;
			return (m_bIsPathing = false);
		}
		if(labs(ptFisrt2.x - ptFisrt.x) >m_OptimizeStep || labs(ptFisrt2.y - ptFisrt.y) > m_OptimizeStep )
		{
			g_WayPointCounts = 0;
			return (m_bIsPathing = false);
		}

	}

//#ifdef _DEBUG
//		FILE *fp = fopen("Block.txt","a+t");
//		if(fp)
//		{
//			for(int i = 0;i<g_WayPointCounts;++i)
//			{
//				fprintf(fp," %d(%02d,%02d)",i,g_WayPoints[i].x,g_WayPoints[i].y);
//			}
//			fclose(fp);
//		}
//#endif

	return true;
TRY_END_RETURN(0)};	

//------------------------------------------------------------------------------
// Purpose : GetPath : 返回全路径
// Input   : 
// Output  : 返回成功或者失败;ptList,转向点列表指针;ptCounts,转向点个数
//------------------------------------------------------------------------------
bool CPathFinder::GetPath(SPoint **ptList,int *ptCounts) 
{TRY_BEGIN
	if(g_WayPointCounts >0 && *ptList && ptCounts)
	{
		*ptList = (SPoint *)&g_WayPoints;
		*ptCounts = g_WayPointCounts;
		return true;
	}
	return false;
TRY_END_RETURN(0)};	

bool  CPathFinder::GetPath(UserPathFindingInfo_t &ui,vectorPoint &vP)
{
	vP.clear();
	for(int i = 0;i<g_WayPointCounts;++i)
	{
		vP.push_back(PathPoint_t(g_WayPoints[i]));
	}
	return (vP.size() > 0);
};

bool CPathFinder::GetPath(UserPathFindingInfo_t &ui,ListPoint &lP)
{
	ReleaseListPoint(lP);
	for(int i = 0;i<g_WayPointCounts;++i)
	{
		AddPathPoint(lP,g_WayPoints[i].x+ui.iStartX,g_WayPoints[i].y+ui.iStartY,false);
	}
	return (lP.size() > 0);
};

//------------------------------------------------------------------------------
// Purpose : GetNextClosePoint : 取得下一个转向点
// Input   : 
// Output  : 
//------------------------------------------------------------------------------
bool CPathFinder::GetNextClosePoint(SPoint *ptClose)
{TRY_BEGIN

	g_WayPointCurrent++;
	if(g_WayPointCounts >1  && ptClose && g_WayPointCurrent < g_WayPointCounts)
	{
		*ptClose = g_WayPoints[g_WayPointCurrent];
		return true;
	}
	return false;
TRY_END_RETURN(0)};
//------------------------------------------------------------------------------
// Purpose : SetOptimizeStep : 设置优化步长
// Input   : iStep,目的优化步长
// Output  : 返回成功或者失败;
//------------------------------------------------------------------------------
bool    CPathFinder::SetOptimizeStep(int iStep)
{
	if(iStep == 1 || iStep == 2 || iStep == 3 )
	{
		m_OptimizeStep = iStep;
		return true;
	}
	return false;
};

//根据ID得到最近使用路径搜索的参数；
//indexLRU返回最近不使用的存储槽
UserPath_t *CPathFinder::GetUserPath(DWORD id)
{
	int index	= 0;
	DWORD t		= 0;
	for(int i = 0;i<StatisticID;++i)
	{
		UserPath_t &up = m_UserPath[i];
		if(!t || (t < up.t))
		{
			t = up.t;
			index = i;
		}

		if(up.id == id)
		{
			return &up;
		}
	}

	memset(&m_UserPath[index],0,sizeof(UserPath_t));
	m_UserPath[index].id = id;

	return &m_UserPath[index];
};

inline bool SPointEqual(SPoint &src,SPoint &dst)
{
	if((src.x == dst.x) && (src.y == dst.y))
		return true;
	return false;
}
// 存储用户路径
bool CPathFinder::AddUserPath(DWORD id,int x, int y)
{
	UserPath_t *up = GetUserPath(id);
	SPoint sp;sp.x = x;sp.y = y;
	up->sp[up->nums++] = sp;

	if(up->nums == (StatisticNum - 1) )
	{
		int iMode = StatisticMode - 1;
		//查询模式是否匹配
		for(int i = 2;i<=StatisticMode;i++)
		{
			//仅匹配4个循环：a-b-a-b-a-b-a-b
			int index	= StatisticNum -1 -  ((i-1)<<3);
			int step	= ((i-1)<<1);
			bool exact	= true;
			for(int j = index+step;j<StatisticNum && exact;j+=step)
			{
				for(int k = 0;k<step && exact;++k) 
				{
					if(!SPointEqual(up->sp[j + k - step] , up->sp[j + k]) )
					{
						exact = false;
					}
				}
			}
			if(exact)	//查到匹配模式
				break;
			--iMode;
		}

		//所有模式正常通过
		if(iMode)
		{
			up->nums = 0;
			return false;
		}
		else 
		{
			memmove(&up->sp[0],&up->sp[1],sizeof(SPoint)*(StatisticNum - 1));
		}
	}
	return true;
};				

//------------------------------------------------------------------------------
// Purpose : PathOK : check it's return value before getting the node entries
// Input   : 
// Output  :
//------------------------------------------------------------------------------
bool CPathFinder::PathOK(void)		
{                                     
	if ( !m_bIsPathing ) 
		return true;					

	if (m_pPath && m_pPath->Parent )		
   		return false;					
	else
   		return true;
}

//------------------------------------------------------------------------------
// Purpose : GetTileIndex
// Input   : 
// Output  :
//------------------------------------------------------------------------------
int CPathFinder::GetTileIndex(int x, int y)
{
	
	if ( x<0 || x >= m_nCols || y<0 || y >= m_nRows )
		return 0;

	return y*m_nCols + x; 
}

//------------------------------------------------------------------------------
// Purpose : IsBlock
// Input   : 
// Output  : returns 1 if tile is blocked.
//------------------------------------------------------------------------------
int CPathFinder::IsBlock(int x, int y)
{TRY_BEGIN
	if( x<0 || y<0 || x>=m_nCols || y>=m_nRows ) 
		return true;
	return (m_pBlock[y*m_nCols + x] & TILE_BLOCK_BIT);
TRY_END_RETURN(1)};


//------------------------------------------------------------------------------
// Purpose : GetRows :取得地图tile行数 
// Input   : 
// Output  :
//------------------------------------------------------------------------------
int CPathFinder::GetRows()
{
	return m_nRows;
};		

//------------------------------------------------------------------------------
// Purpose : GetCols :取得地图tile列数 
// Input   : 
// Output  :
//------------------------------------------------------------------------------
int CPathFinder::GetCols()
{
	return m_nCols;
};		

////////////////////////////////////////////////////////////////////////////////
//								      Private Member Functions                //
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Purpose : FreeNodes
// Input   : 
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::FreeNodes()
{TRY_BEGIN
	Node_t *Node, *OldNode;
	
	if ( m_pListOpen )
	{
		Node = m_pListOpen->NextNode;
		while ( Node )
		{
			OldNode = Node;
			Node = Node->NextNode;
			DeleteNode(OldNode);
		}
		DeleteNode(m_pListOpen);
	}

	if ( m_pListClosed )
	{
		Node = m_pListClosed->NextNode;
		while ( Node )
		{
			OldNode = Node;
			Node = Node->NextNode;
			DeleteNode(OldNode);
		}
		DeleteNode(m_pListClosed);
	}
TRY_END}

////////////////////////////////////////////////////////////////////////////////
//                               A* Algorithm                                 //
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Purpose : FindPath:(假定点有效)
// Input   : 起点(sx, sy),终点(dx, dy)
// Output  : iIntegrity--1 路径不完整;
//------------------------------------------------------------------------------
void CPathFinder::FindPath(int sx, int sy, int dx, int dy,int *iIntegrity)
{TRY_BEGIN
	Node_t *Node, *BestNode, *BestNodeOld =0;
	int TileIndexS;

	TileIndexS = GetTileIndex(sx, sy);					// 搜索终止点

	m_pListOpen		= NewNode();
	m_pListClosed	= NewNode();

	Node			= NewNode();						// 第一个节点设为终点
	Node->g = 0;										// 搜索树高度
	Node->h = AI_Value(sx,  sy,  dx,  dy);				// 独立评估函数;(should really use sqrt().)
	Node->f = Node->g + Node->h;						// 节点总评估值
	Node->NodeIndex = GetTileIndex(dx, dy);
	Node->x = dx;
	Node->y = dy;

	int SEARCH = m_MaxSearch ;
	m_pListOpen->NextNode=Node;							// make Open List point to first node

	int i = 0;
	for(i=0;i<SEARCH;++i)
	{
		BestNode = ReturnBestNode();					// 评估
		BestNodeOld = BestNode;
													
		if (BestNode && BestNode->NodeIndex == TileIndexS)			// if we've found the end, break and finish
	   		break;
		GenerateSuccessors(BestNode,sx,sy);				// 以起点(sx,sy)为终止点进行搜索
	}

	if(i==SEARCH)
	{
		if(iIntegrity)
			*iIntegrity = 1;
#ifdef AI_DEBUG
		output_debug("FindPath : overflow! SEARCH = %d\n",i);
#endif
	}
	else
	{
		if(iIntegrity)
			*iIntegrity = 0;
	}

	m_pPath = BestNode;
TRY_END}

//------------------------------------------------------------------------------
// Purpose : ReturnBestNode
// Input   : 
// Output  :
//------------------------------------------------------------------------------
Node_t *CPathFinder::ReturnBestNode()
{TRY_BEGIN
	Node_t *tmpNode;

	if ( m_pListOpen->NextNode == NULL )
	{
		return NULL;
	}

// Pick Node with lowest f, in this case it's the first node in list
// because we sort the m_pListOpen list wrt lowest f. Call it BESTNode_t.
	tmpNode = m_pListOpen->NextNode;				// point to first node on m_pListOpen
	m_pListOpen->NextNode = tmpNode->NextNode;		// Make m_pListOpen point to nextnode or NULL.

// Next take BESTNode_t (or temp in this case) and put it on m_pListClosed
	tmpNode->NextNode = m_pListClosed->NextNode;
	m_pListClosed->NextNode = tmpNode;
	
	return	tmpNode;
TRY_END_RETURN(0)};

//------------------------------------------------------------------------------
// Purpose : GenerateSuccessors
// Input   : 起点:BestNode;终点:dx,dy
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::GenerateSuccessors(Node_t *BestNode, int dx, int dy)
{TRY_BEGIN
	int x, y;

	if (!BestNode) {
		return;
	}

	int nSearchOut[AI_GRID_MAX];
	AI_SearchPossibleGrid( BestNode->x,  BestNode->y,  dx,  dy,nSearchOut);		

	for(int i = 0;i<AI_GRID_MAX ;++i)
	{
		if(i%2 && !m_bDirs8)
			continue;
		if ( !IsBlock(x = BestNode->x + g_ptSearchRgn[nSearchOut[i]].x, y = BestNode->y + g_ptSearchRgn[nSearchOut[i]].y) )
			GenerateSuccessorsForOneDir(BestNode,x,y,dx,dy);
	}
TRY_END}

//------------------------------------------------------------------------------
// Purpose : GenerateSuccessorsForOneDir
// Input   : BestNode: 当前点;x,y: 搜索起点;dx,dy: 搜索终点
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::GenerateSuccessorsForOneDir(Node_t *BestNode,int x, int y, int dx, int dy)
{TRY_BEGIN
	int g, TileIndexS, c = 0;
	Node_t *Old, *Successor;

	g = BestNode->g+1;									// g(Successor)=g(BestNode)+cost of getting from BestNode to Successor
	TileIndexS = GetTileIndex(x,y);						// identification purposes

	if (Old = CheckOpen(TileIndexS))					// if equal to NULL then not in m_pListOpen list, else it returns the Node in Old
	{
		for( c = 0; c < CHILDNUM; c++)
			if( BestNode->Child[c] == NULL )			// Add Old to the list of BestNode's Children (or Successors).
	   			break;
		BestNode->Child[c] = Old;

		if ( g < Old->g )								// if our new g value is < Old's then reset Old's parent to point to BestNode
		{
			Old->Parent = BestNode;
	   		Old->g = g;
	   		Old->f = g + Old->h;
		}
	}
	else if (Old=CheckClosed(TileIndexS))				// if equal to NULL then not in m_pListClosed list, else it returns the Node in Old
	{
		for( c = 0; c < CHILDNUM; c++)
			if ( BestNode->Child[c] == NULL )			// Add Old to the list of BestNode's Children (or Successors).
	   			break;
		BestNode->Child[c] = Old;

		if ( g < Old->g )								// if our new g value is < Old's then reset Old's parent to point to BestNode
		{
			Old->Parent = BestNode;
			Old->g = g;
			Old->f = g + Old->h;
			PropagateDown(Old);							// Since we changed the g value of Old, we need to propagate this new value downwards, i.e. do a Depth-First traversal of the tree!
		}
	}
	else
	{
		Successor = NewNode();
		Successor->Parent = BestNode;
		Successor->g = g;
		Successor->h = AI_Value(x,  y,  dx,  dy);		// should do sqrt(), but since we don't really
		Successor->f = g + Successor->h;				// care about the distance but just which branch looks
		Successor->x = x;								// better this should suffice. Anyayz it's faster.
		Successor->y = y;
		Successor->NodeIndex = TileIndexS;
		Insert(Successor);								// insert Successor on m_pListOpen list wrt f
		for( c =0; c < CHILDNUM; c++)
			if ( BestNode->Child[c] == NULL )			// Add Old to the list of BestNode's Children (or Successors).
				break;
		BestNode->Child[c] = Successor;
	}
TRY_END}

//------------------------------------------------------------------------------
// Purpose : CheckOpen : 
// Input   :
// Output  :
//------------------------------------------------------------------------------
Node_t	*CPathFinder::CheckOpen(int tileindex)
{TRY_BEGIN
	Node_t *tmpNode;

	tmpNode = m_pListOpen->NextNode;
	while ( tmpNode != NULL )
	{
		if ( tmpNode->NodeIndex == tileindex )
	   		return (tmpNode);
		else
			tmpNode = tmpNode->NextNode;
	}
	return	NULL;
TRY_END_RETURN(0)};	

//------------------------------------------------------------------------------
// Purpose : CheckClosed
// Input   :
// Output  :
//------------------------------------------------------------------------------
Node_t	*CPathFinder::CheckClosed(int tileindex)
{TRY_BEGIN
	Node_t *tmpNode;

	tmpNode = m_pListClosed->NextNode;

	while ( tmpNode != NULL )
	{
		if ( tmpNode->NodeIndex == tileindex )
			return(tmpNode);
		else
			tmpNode = tmpNode->NextNode;
	}
	return	NULL;
TRY_END_RETURN(0)};	

//------------------------------------------------------------------------------
// Purpose : Insert，按照评估权f值排序；最小的f在头部
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::Insert(Node_t *Successor)
{TRY_BEGIN
	Node_t *tmpNode1, *tmpNode2;
	int f;

	if ( m_pListOpen->NextNode == NULL )
	{
		m_pListOpen->NextNode = Successor;
		return;
	}

	// insert into m_pListOpen successor wrt f
	f = Successor->f;
	tmpNode1 = m_pListOpen;
	tmpNode2 = m_pListOpen->NextNode;

	while ( (tmpNode2 != NULL) && (tmpNode2->f < f) )
	{
		tmpNode1 = tmpNode2;
		tmpNode2 = tmpNode2->NextNode;
	}

	Successor->NextNode = tmpNode2;
	tmpNode1->NextNode = Successor;
TRY_END};	

//------------------------------------------------------------------------------
// Purpose : PropagateDown
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::PropagateDown(Node_t *Old)
{TRY_BEGIN
	int c, g;
	Node_t *Child, *Father;

	g = Old->g;									// alias.
	for ( c = 0; c < CHILDNUM; c++)
	{
		if ( (Child=Old->Child[c]) == NULL )	// create alias for faster access.
			break;
		if ( g+1 < Child->g)
		{
			Child->g = g+1;
			Child->f = Child->g + Child->h;
			Child->Parent = Old;				// reset parent to new path.
			Push(Child);						// Now the Child's branch need to be checked out. Remember the new cost must be propagated down.
		}
	}

	while ( m_pStack->NextStackPtr != NULL )
	{
		Father = Pop();
		for (c = 0; c < CHILDNUM; c++)
		{
			if ( (Child=Father->Child[c]) == NULL )	// we may stop the propagation 2 ways: either there are no children, or that the g value of the child is equal or better than the cost we're propagating
				break;
			if ( Father->g+1 < Child->g )
			{
				Child->g = Father->g+1;
				Child->f = Child->g+Child->h;
				Child->Parent = Father;
				Push(Child);
			}
		}
	}
TRY_END}

//------------------------------------------------------------------------------
// Purpose : Push
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPathFinder::Push(Node_t *Node)
{TRY_BEGIN
	Stack_t *tmpStack;

	tmpStack = NewStack();
	tmpStack->NodePtr = Node;
	tmpStack->NextStackPtr = m_pStack->NextStackPtr;
	m_pStack->NextStackPtr = tmpStack;
TRY_END}

//------------------------------------------------------------------------------
// Purpose : Pop
// Input   :
// Output  :
//------------------------------------------------------------------------------
Node_t	*CPathFinder::Pop(void)
{TRY_BEGIN
	Node_t	*tmpNode;
	Stack_t *tmpStack;

	tmpStack	= m_pStack->NextStackPtr;
	tmpNode		= tmpStack->NodePtr;

	m_pStack->NextStackPtr = tmpStack->NextStackPtr;
	DeleteStack(tmpStack);
	return	tmpNode;
TRY_END_RETURN(0)};

//------------------------------------------------------------------------------
// Purpose : FindPathEx:(假定点有效)
// Input   : 起点(sx, sy),终点(dx, dy)
// Output  : iIntegrity--1 路径不完整;
//------------------------------------------------------------------------------
void CPathFinder::FindPathEx(int sx, int sy, int dx, int dy, int *iIntegrity)
{TRY_BEGIN
	Node_t *Node, *BestNode;
	int TileIndexS = GetTileIndex(sx, sy);					// 搜索终止点

	UINT32 node_count   = 0;
	UINT32 current_node = 0;

	Node = &m_NodeList[0];								// 第一个节点设为终点
	Node->g = 0;										// 搜索树高度
	Node->h = 0;										// 独立评估函数,都为0
	Node->f = Node->g + Node->h;						// 节点总评估值
	Node->NodeIndex = GetTileIndex(dx, dy);
	Node->x = dx;
	Node->y = dy;

	int SEARCH = (m_nCols - 1)*(m_nRows - 1);
	if(SEARCH > STEP_LIMIT)
		SEARCH = STEP_LIMIT;

	m_pBitArray[Node->NodeIndex] = 1;					// 已经处理过的节点

	int nSearchOut[AI_GRID_MAX];
	nSearchOut[0] = 0;
	nSearchOut[1] = 4;
	nSearchOut[2] = 6;
	nSearchOut[3] = 2;
	nSearchOut[4] = 1;
	nSearchOut[5] = 3;
	nSearchOut[6] = 5;
	nSearchOut[7] = 7;

	int i = 0;
	for(i=0;i < SEARCH;++i)
	{
		if(node_count >= SEARCH || current_node  > node_count) //队列为空
		{
			m_pPath = NULL;
			return;
		}

		BestNode = &m_NodeList[current_node++]; //加入到备份队列

		for(int i = 0;i< AI_GRID_MAX ;++i)
		{
			int x = BestNode->x + g_ptSearchRgn[nSearchOut[i]].x;
			int y = BestNode->y + g_ptSearchRgn[nSearchOut[i]].y;

			int _TileIndexS = GetTileIndex(x,y);						// identification purposes
			if(_TileIndexS == 0)  //超出范围的，结束
				continue;

			if(IsBlock(x,y))  //非阻挡点
			{
				m_pBitArray[_TileIndexS] = 1; //表示该点已经被处理过了
			}
			else if(m_pBitArray[_TileIndexS] == 0) // 还没有处理过该节点
			{
				Node_t *Successor;
				int c = 0;
				int g = BestNode->g+1;									// g(Successor)=g(BestNode)+cost of getting from BestNode to Successor

				m_pBitArray[_TileIndexS] = 1;

				Successor = &m_NodeList[++node_count];
				Successor->Parent = BestNode;
				Successor->g = g;
				Successor->h = 0;
				Successor->f = g + Successor->h;				// care about the distance but just which branch looks
				Successor->x = x;								// better this should suffice. Anyayz it's faster.
				Successor->y = y;
				Successor->NodeIndex = _TileIndexS;

				for(c =0; c < CHILDNUM; c++)
				{
					if ( BestNode->Child[c] == NULL )			// Add Old to the list of BestNode's Children (or Successors).
						break;
				}
				BestNode->Child[c] = Successor;

				if(_TileIndexS == TileIndexS) //找到了路径，结束
				{
					m_pPath = Successor;
					if(iIntegrity)
						*iIntegrity = 0;
					return;
				}
			}
		}
	}
	m_pPath = BestNode; //没有找到路径
TRY_END}


//地图路径
CMapPathFinder::CMapPathFinder():CPathFinder()
{
	m_MaxSearch		= 40960;
//	m_MaxSearch		= 1024;
	m_OnRoute		= false;
	m_SearchDir		= 1;			//1正向；0反向

};

CMapPathFinder::~CMapPathFinder()
{
	ReleasePathPoint();
};

//
PathPoint_t *ListIndex(ListPoint &ls,int index)
{
	int iCounts = 0;
	for(ListPoint::iterator itr = ls.begin();	itr != ls.end();++itr)
	{
		PathPoint_t *p = (*itr);
		if(iCounts == index)
		{
			return p;
		}
		++iCounts;
	}
	return 0;
};

bool ListSplit(ListPoint &ls,PathPoint_t **p1,PathPoint_t **p2,PathPoint_t **p3,PathPoint_t **p4)
{
	int index = (int)ls.size() ;
	if(index < 4)
		return false;

	index/=2;

	*p1 = ls.front();

	int iCounts = 0;
	for(ListPoint::iterator itr = ls.begin();itr != ls.end();++itr)
	{
		PathPoint_t *p = (*itr);
		if(iCounts == index)
		{
			*p2 = p;
			*p3 = *(++itr);
			*p3 = ls.back();
			return 1;
		}
	}
	return false;
}

void	CMapPathFinder::Optimaize(bool bEnable)
{
	if(!bEnable)
		return ;

	PathPoint_t *pStart = 0 ,*pEnd = 0 ;
	int iSize = (int)m_PathPoints.size();

};
bool	CMapPathFinder::ReCreatePath(int sx, int sy,int _MapID)
{
	int iIntegrity = 0;
	if(CreatePath(sx,  sy,  m_DstPathPoint.x,  m_DstPathPoint.y,&iIntegrity))
	{
		if(iIntegrity == 1)
			return false;
		return true;
	}
	return false;
};

long CMapPathFinder::Distance2Dst(int sx, int sy)
{
	if(m_DstPathPoint.x <=0 || m_DstPathPoint.y <=0 )
		return 0;
	return _PointLenAbs(m_DstPathPoint.x,m_DstPathPoint.y,sx, sy);
}

//------------------------------------------------------------------------------
// Purpose : CreatePath,目前采用广度优先算法,使用带Ex的算法
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CMapPathFinder::CreatePath(int sx, int sy, int dx, int dy,int *iIntegrity)
{
	DWORD dwBeginTick = GetTickCount();
	SPoint	po;

	//禁止重复寻路
	if(m_DstPathPoint.x == dx && m_DstPathPoint.y == dy)
	{
		ListPoint *lp = 0;
		if((lp = GetPath()) && (lp->size() > 1))
		{
			PathPoint_t *p = lp->front();
			if(p->x == sx && p->y == sy)
			{
				return true;
			}
		}
	}

	m_DstPathPoint.x = dx;
	m_DstPathPoint.y = dy;

	ReleasePathPoint();

	m_nCols	= g_pGameMap->GetWidth();
	m_nRows	= g_pGameMap->GetHeight();
#ifdef AI_DEBUG
	output_debug("CMapPathFinder::CreatePath ,start(%d,%d) --> end(%d,%d) ;[%d,%d]\n",sx,sy,dx,dy,m_nRows,m_nCols);//[%s,%d] 
#endif

	m_pBlock	= 0;

	if(IsBlock(dx,dy))
	{
#ifdef AI_DEBUG
		output_debug("[%s,%d] CMapPathFinder::CreatePath : (%d,%d) IsBlock;\n",__FILE__,__LINE__,dx,dy);
#endif
		return (m_bIsPathing = false);
	}

	if(IsBlock(sx,sy))
	{
#ifdef AI_DEBUG
		output_debug("[%s,%d] CMapPathFinder::CreatePath : (%d,%d) IsBlock;\n",__FILE__,__LINE__,sx,sy);
#endif
		return (m_bIsPathing = false);
	}

	if(GetTileIndex(sx,sy) == GetTileIndex(dx,dy))
	{
		int i = GetTileIndex(sx,sy);
		int j = GetTileIndex(dx,dy);
#ifdef AI_DEBUG
		output_debug("CMapPathFinder::CreatePath ,start(%d,%d) == end(%d,%d) ;[%d,%d][%d,%d]\n",sx,sy,dx,dy,m_nCols,m_nRows,i,j);//[%s,%d] 
#endif
		return (m_bIsPathing = false);
	}

#ifdef _A_STAR
	FreeNodes();

	if(m_SearchDir)
		FindPath(sx,sy,dx,dy,iIntegrity);	//正向搜索
	else
		FindPath(dx,dy,sx,sy,iIntegrity);	//为了优化搜索，反向；尽量实现跑步优化；形成的点为反向!!!2005-4-22

#else
	FreeNodes();

	TRY_BEGIN
		m_pBitArray = new BYTE[m_nCols*m_nRows];
		memset(m_pBitArray,0,m_nCols*m_nRows);
	TRY_END_RETURN(false)


#ifdef AI_DEBUG
	DWORD dwEndTick1 = GetTickCount();
	output_debug("CreatePath : FreeNodesEx 时间 %d ms\n",(dwEndTick1 - dwBeginTick));
#endif

	if(m_SearchDir)
		FindPathEx(sx,sy,dx,dy,iIntegrity);	//正向搜索
	else
		FindPathEx(dx,dy,sx,sy,iIntegrity);	//为了优化搜索，反向；尽量实现跑步优化

	SAFE_DELETE_ARRAY(m_pBitArray);

#ifdef AI_DEBUG
	DWORD dwEndTick2 = GetTickCount();
	output_debug("CreatePath : FindPathEx 时间 %d ms\n",(dwEndTick2 - dwEndTick1));
#endif

#endif

	if(!m_pPath)
		return false;

	m_bIsPathing = true;

	while ( !PathOK() )		
	{
		po.x = GetNodeX();
		po.y = GetNodeY();
		AddPathPoint(po.x,po.y,!m_SearchDir);//bHead == true;,
		PathNextNode();
	}

	//尾节点
	po.x = GetNodeX();
	po.y = GetNodeY();
	AddPathPoint(po.x,po.y,!m_SearchDir);//,

#ifdef AI_DEBUG
	DWORD dwEndTick = GetTickCount();
	output_debug("CreatePath :从(%d,%d)到(%d,%d) 时间 %d ms\n",sx,sy,dx,dy,(dwEndTick - dwBeginTick) );
#endif



	return true;
};	
//------------------------------------------------------------------------------
// Purpose : 返回全路径
// Input   :
// Output  :
//------------------------------------------------------------------------------
ListPoint *CMapPathFinder::GetPath()
{
	return &m_PathPoints;
};			 

//------------------------------------------------------------------------------
// Purpose : 取得路径的下一个转向点
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool	CMapPathFinder::GetNextClosePoint(SPoint *ptClose)
{
	if(ptClose && m_PathPoints.size())
	{
		return 1;
	}
	return 0;
};	

//------------------------------------------------------------------------------
// Purpose : IsBlock
// Input   :
// Output  :
//------------------------------------------------------------------------------
int CMapPathFinder::IsBlock(int x, int y)
{
//	!m_pGameMap || 
	return (g_pGameMap->IsBlock(x,y) || SELF.IsOppBlock(x,y));
};


//------------------------------------------------------------------------------
// Purpose : 根据方向增加到head或者tail
// Input   :
// Output  :
//------------------------------------------------------------------------------
int CMapPathFinder::AddPathPoint(int x, int y,bool bHead)
{
	return ::AddPathPoint(m_PathPoints,x,  y,bHead);
};


void CMapPathFinder::ReleasePathPoint()
{
	::ReleaseListPoint(m_PathPoints);
}

//------------------------------------------------------------------------------
// Purpose : 取得路径
// Input   : 对原路径进行简化,每16个点存储成队列
// Output  :
//------------------------------------------------------------------------------
void	CMapPathFinder::GetRoute(vectorPoint &vp)
{
	vp.clear();
	//
	int iSize = (int)m_PathPoints.size() - 1;
	int iStep = 0;
	for(ListPoint::iterator itr = m_PathPoints.begin();itr != m_PathPoints.end();++itr)
	{
		PathPoint_t *p = (*itr);	
		if((!(iStep % 6) && (iSize > (iStep + 2))) || iStep==iSize  || iStep==0)
			vp.push_back(*p);
		iStep++;
	};
};

bool	CMapPathFinder::AdvanceRoutePoint()
{
	if(m_CurrentRoute.size() < 1)
	{
		m_CurrentRoutePoint.x	= m_CurrentRoutePoint.y = -1; 
		m_dwCurrentRouteTime	= 0 ;
		return false;
	}

	m_CurrentRoutePoint.x = m_CurrentRoute[0].x ;
	m_CurrentRoutePoint.y = m_CurrentRoute[0].y;
	m_CurrentRoute.erase(m_CurrentRoute.begin());

	m_dwCurrentRouteTime = GetTickCount();

#ifdef AI_DEBUG
	int selfX,selfY;
	SELF.GetXY(selfX,selfY);

	output_debug("AdvanceRoutePoint : (%d,%d) 剩余 %d Self: (%d,%d)\n",m_CurrentRoutePoint.x,m_CurrentRoutePoint.y,m_CurrentRoute.size(),selfX,selfY);
#endif

	return true;
};

//
//处理同地图异常跳转 -- 由于1号地图分割，有可能造成搜索的路径刚好处于“桥”上；由于桥两端都是跳转点；
//造成往复跳动的bug.
//iStep用于控制前进航路点的个数；由于基本的航路点的间隔是6个格子；（4-1）*6 + 3；20格，应该可以满足跳转搜索
//这里假定原始的航路点为直线路径

//简单算法，自己到下一航路点的距离比当前两航路点距离近，那么略过第一个航路点
bool   CMapPathFinder::JumpAdvanceRoutePoint(int iSelfX,int iSelfY,int iStep)
{ 
	if(m_CurrentRoutePoint.x == -1)
		return false;

	int len0 ,len1;
	while(m_CurrentRoute.size()> 1 && iStep>0)
	{
		PathPoint_t &pStart = m_CurrentRoute[0];

		len0 = _PointLenAbs(m_CurrentRoutePoint.x,m_CurrentRoutePoint.y,pStart.x,pStart.y);
		len1 = _PointLenAbs(iSelfX,iSelfY,pStart.x,pStart.y);
		if(len1 <= len0)
		{
			if(!AdvanceRoutePoint())
				return false;
		}
		else
			break;
		--iStep;
	}
	return true;
};

bool	CMapPathFinder::IsRightWalkingSpeed()
{
	if(m_dwCurrentRouteTime &&
		(labs(GetTickCount() - m_dwCurrentRouteTime) > 6000))
	{
		return false;
	}
	return true;
};

//------------------------------------------------------------------------------
// Purpose : 把当前的路径作为航路，同时删除搜索结果
// Input   : 
// Output  :
//------------------------------------------------------------------------------
void	CMapPathFinder::UsePathAsCurrentRoute(const char* _MapID)
{
	if(_MapID == NULL)
		_MapID = g_pGameMap->GetMapName();

	GetRoute(m_CurrentRoute);
//	m_PathPoints.clear();

	AdvanceRoutePoint();
	if(m_CurrentRoute.size() > 0)
		SetOnRoute(1);

	SetMapID(_MapID);
	if(!_MapID)
	{
#ifdef AI_DEBUG	
		output_debug("UsePathAsCurrentRoute : %s\n",_MapID);
#endif
	}
};	

void	CMapPathFinder::GetCurrentRoutePoint(int &x,int &y)
{
	x = m_CurrentRoutePoint.x;
	y = m_CurrentRoutePoint.y;
};


bool    CMapPathFinder::ReachRouteEnd(int x,int y,int limit)
{
	if(_closePoint(x,y,m_DstPathPoint.x,m_DstPathPoint.y,limit))
		return true;
	return false;
}; 

bool    CMapPathFinder::ReachCurrentRoutePoint(int x,int y,int limit)
{
	if(m_CurrentRoutePoint.x == -1 || m_CurrentRoutePoint.y == -1)
	{
		return true;
	}

	if(_closePoint(x,y,m_CurrentRoutePoint.x,m_CurrentRoutePoint.y,limit))
		return true;

	return false;
};
//
bool	CMapPathFinder::IsLeftRoutePoint()
{
	if(m_CurrentRoute.size())
		return true;
	return false;
};

bool	CMapPathFinder::IsOnRoute()
{
	return m_OnRoute;
};

bool    CMapPathFinder::SetOnRoute(bool bOnRoute)
{
	m_OnRoute = bOnRoute;
	if(m_OnRoute == 0)
	{
		m_DstPathPoint.x = -1;
		m_DstPathPoint.y = -1;
	}

	return m_OnRoute;
};

void    CMapPathFinder::SetMapID(const char* _MapID)
{
	m_MapID = _MapID;
};

const char*     CMapPathFinder::GetMapID()
{
	return m_MapID.c_str();
};

const char*	CMapPathFinder::GetCurrentMapID()
{
	return g_pGameMap->GetMapName();
};

CMapPathFinderAgent	   g_MapPathFinderAgent;
//CTrusteeshipData g_TrusteeshipData;//g_TrusteeshipData的构造函数用到CClassCache及CMapPathFinderClient的构造函数,所以放在这个cpp的最后,否则在执行g_TrusteeshipData构造函数时g_PathFindLock还没有执行构造函数初始化
