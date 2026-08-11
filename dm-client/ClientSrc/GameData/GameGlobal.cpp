
#include "GameGlobal.h"
#include "GameDefine.h"
#include "Global/Interface/FontInterface.h"
#include "Global/DebugTry.h"

// 根据两个点，取得方向
int GetDir8(int iTX, int iTY, int iOX, int iOY)
{
	int x1,y1,j;

	// 计算距离坐标距离
	x1 = (iTX - iOX);
	y1 = -(iTY - iOY);

	//正方向,比如:0,4,8,12也就是北，东，南，西
	if(x1 == 0)
	{
		if(y1 > 0)
			return 0;
		else
			return 4;
	}
	if(y1 == 0)
	{
		if(x1 > 0)
			return 2;
		else
			return 6;
	}

	//非正方向,比如东北，东南...
	j = labs((int)((float)y1 / (float)x1 * 1000));

	if(x1 > 0)
	{
		if(y1 > 0)
		{
			if(j < 414)
				return 2;
			else if(j < 2414)
				return 1;
			else
				return 0;
		}
		else
		{
			if(j < 414)
				return 2;
			else if(j < 2414)
				return 3;
			else
				return 4;
		}
	}
	else
	{
		if(y1 > 0)
		{
			if(j < 414)
				return 6;
			else if(j < 2414)
				return 7;
			else
				return 0;
		}
		else
		{
			if(j < 414)
				return 6;
			else if(j < 2414)
				return 5;
			else
				return 4;
		}
	}
}
//------------------------------------------------------------------------------
// Purpose : 判断16点方向
// Input   : 目标(iTX,iTY),原点(iOX, iOY)
// Output  :
//------------------------------------------------------------------------------
int GetDir16(int iTX,int iTY,int iOX,int iOY,bool bFlag)
{
	int x1,y1,j;

	// 计算距离目标的象素距离
	x1 = (iTX - iOX);
	y1 = -(iTY - iOY);

	//正方向,比如:0,4,8,12也就是北，东，南，西
	if(x1 == 0)
	{
		if(y1 > 0)
			return 0;
		else
			return 8;
	}
	if(y1 == 0)
	{
		if(x1 > 0)
			return 4;
		else
			return 12;
	}

	//非正方向,比如东北，东南...
	j = labs((int)((float)y1 / (float)x1 * 1000));

	if(x1 > 0)
	{
		if(y1 > 0)
		{
			if(j < 199)
				return 4;
			else if(j < 668)
				return 3;
			else if(j < 1496)
				return 2;
			else if(j < 5027)
				return 1;
			else
				return 0;
		}
		else
		{
			if(j < 199)
				return 4;
			else if(j < 668)
				return 5;
			else if(j < 1496)
				return 6;
			else if(j < 5027)
				return 7;
			else
				return 8;
		}
	}
	else
	{
		if(y1 > 0)
		{
			if(j < 199)
				return 12;
			else if(j < 668)
				return 13;
			else if(j < 1496)
				return 14;
			else if(j < 5027)
				return 15;
			else
				return 0;
		}
		else
		{
			if(j < 199)
				return 12;
			else if(j < 668)
				return 11;
			else if(j < 1496)
				return 10;
			else if(j < 5027)
				return 9;
			else
				return 8;
		}
	}
}


int GetDir8Attack(int iTX,int iTY,int iOX,int iOY)
{
	int x1,y1,j;
	static UCHAR cDir[7][7] = { {3,3,4,4,4,5,5},
								{3,3,4,4,4,5,5},
								{2,2,3,4,5,6,6},
								{2,2,2,0,6,6,6},
								{2,2,1,0,7,6,6},
								{1,1,0,0,0,7,7},
								{1,1,0,0,0,7,7}	};

	// 计算距离坐标距离
	x1 = iOX - iTX;
	y1 = iOY - iTY;

	if( x1>=-3 && x1<=3 && y1>=-3 && y1<=3 )
	{
		return (int)(cDir[y1+3][x1+3]);
	}
	else
	{
		x1 = (iTX - iOX);
		y1 = -(iTY - iOY);

		//正方向,比如:0,4,8,12也就是北，东，南，西
		if(x1 == 0)
		{
			if(y1 > 0)
				return 0;
			else
				return 4;
		}
		if(y1 == 0)
		{
			if(x1 > 0)
				return 2;
			else
				return 6;
		}

		//非正方向,比如东北，东南...
		j = labs((int)((float)y1 / (float)x1 * 1000));

		if(x1 > 0)
		{
			if(y1 > 0)
			{
				if(j < 414)
					return 2;
				else if(j < 2414)
					return 1;
				else
					return 0;
			}
			else
			{
				if(j < 414)
					return 2;
				else if(j < 2414)
					return 3;
				else
					return 4;
			}
		}
		else
		{
			if(y1 > 0)
			{
				if(j < 414)
					return 6;
				else if(j < 2414)
					return 7;
				else
					return 0;
			}
			else
			{
				if(j < 414)
					return 6;
				else if(j < 2414)
					return 5;
				else
					return 4;
			}
		}
	}
}

int GetLineDirect(int x1,int y1,int x2,int y2)
{
	int iDir;
	
	//判断点的合法性
	if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
	{
		return -1;
	}

	if ( (x2 - x1) > 0 ) //在左半方 
	{
		if ( (y2 - y1) > 0 ) //在第2象限
		{
			if ( (labs(y2 - y1) - labs(x2 - x1)) > 0 )  //在3区
			{
				if ( labs(x2 - x1) == 1)
					iDir = 4;
				else
					iDir = 3;
			}

			if ( (labs(y2 - y1) - labs(x2 - x1)) <= 0 )   //在2区
				iDir = 3;
		}
		
		if( (y2 -y1) == 0 )
			iDir = 2;  

		if ( (y2 -y1) < 0 )  //在第1象限
		{
			if ( (labs(y2 - y1) - labs(x2 - x1)) > 0 )  //在0区
			{
				if ( labs(x2 - x1) == 1)
					iDir = 0;
				else
					iDir = 1;
			}

			if ( (labs(y2 - y1) - labs(x2 - x1)) == 0 )  
				iDir = 1;
			if ( (labs(y2 - y1) - labs(x2 - x1)) < 0 )  //在1区
			{
				if ( labs(y2 - y1) == 1)
					iDir = 2;
				else
					iDir = 1;
			}	
		}	
	}
	else if ( (x2 -x1) == 0 )							//X坐标相同
	{
		if ( (y2 -y1) > 0 )
			iDir = 4;								//正下方 
		else if ( (y2 -y1) == 0 )
			iDir = -1;							//源和目相同
		else
			iDir = 0;								//正上方
	}
	else if ( (x2 - x1) < 0 )							//在右半方
	{
		if ( (y2 - y1) > 0 )							//在第3象限
		{
			if ( (labs(y2 - y1) - labs(x2 - x1)) > 0 )	//在4区
			{
				if ( labs(x2 - x1) == 1)
					iDir = 4;
				else
					iDir = 5;
			}
			if ( (labs(y2 - y1) - labs(x2 - x1)) <= 0)  
				iDir = 5;
		}
		if( (y2 -y1) == 0 )
			iDir = 6;  
		if ( (y2 -y1) < 0 )								//在第4象限
		{
			if ( (labs(y2 - y1) - labs(x2 - x1)) > 0)		//在7区
			{
				if ( labs(x2 - x1) == 1)
					iDir = 0;
				else
					iDir = 7;
			}
			if ( labs(labs(y2 - y1) - labs(x2 - x1)) == 0)  
				iDir = 7;
			if ( (labs(y2 - y1) - labs(x2 - x1)) < 0 )	//在6区
			{
				if (labs(y2 - y1) == 1)
					iDir = 6;
				else
					iDir = 7;
			}	
		}	
	}
	return iDir;
}

// 角色转方向
void GetDirStep(int iDir,int &iX,int &iY)
{
	switch( iDir )
	{
		case DIR_UP:		iX =  0; iY = -1; break;
		case DIR_RIGHTUP:	iX = +1; iY = -1; break;
		case DIR_RIGHT:		iX = +1; iY =  0; break;
		case DIR_RIGHTDOWN:	iX = +1; iY = +1; break;
		case DIR_DOWN:		iX =  0; iY = +1; break;
		case DIR_LEFTDOWN:	iX = -1; iY = +1; break;
		case DIR_LEFT:		iX = -1; iY =  0; break;
		case DIR_LEFTUP:	iX = -1; iY = -1; break;
		default:			iX =  0; iY =  0; break;
	}
}

void GetDir16Step(int iDir,float &fX,float &fY)
{
	int  iDegree = (int)(22.5f * iDir);
	if (iDegree < 0)
	{
		iDegree = 0;
	}
	else if (iDegree >= 360)
	{
		iDegree = 360;
	}

	fX = g_fSin[iDegree];
	fY = -g_fCos[iDegree];
}

const char* GetToDir(int iTX,int iTY,int iOX,int iOY)
{
		int iDeltaX = iOX - iTX;
		int iDeltaY = iOY - iTY;

		if(iDeltaY>0)
		{
			if(iDeltaX>0)
				return "↘";
			else if(iDeltaX<0)
				return "↙";
			else
				return "↓";
		}
		else if(iDeltaY<0)
		{
			if(iDeltaX>0)
				return "↗";
			else if(iDeltaX<0)
				return "↖";
			else
				return "↑";
		}else
		{
			if(iDeltaX>0)
				return "→";
			else if(iDeltaX<0)
				return "←";
			else
				return "↓";
		}

		return "";
}


int	ModifyDir(int iDir,int iDirCount)
{
	if (iDirCount <= 0)
		return iDir;

	int retdir = iDir % iDirCount;
	if (retdir < 0)
		retdir += iDirCount;
	return retdir;
}

UINT STRLEN(const char* str, int iFontSize)
{
	return g_pFont->GetLen(str, NULL, iFontSize);
}

UINT STRLEN(const char* p0,const char* p1, int iFontSize)
{
	return g_pFont->GetLen(p0, p1, iFontSize);
}

const char* GoThroughEnter(const char* szSource, int& iPos)
{
	g_cBuf[0] = 0;
	if(szSource && iPos >= 0)
	{
		const int iOldPos = iPos;
		int iLen = strlen(szSource);
		if(iPos >= iLen) return NULL;
		const char* pStart = szSource + iPos;
		while(pStart && *pStart && *pStart != '\n' && iPos < iLen)
		{
			pStart = szSource+(++iPos);
		}
		int iNum = iPos - iOldPos;
		memcpy(g_cBuf, szSource+iOldPos, iNum);
		g_cBuf[iNum] = 0;
		++iPos;
		return g_cBuf;
	}
	return NULL;
}

BOOL CopyText2Clip(const char *buf,int size)
{
	BOOL bRet = FALSE;

	if(!buf || size <= 0)
		return bRet;

	BOOL bAlloced	= FALSE;
	BOOL bLocked	= FALSE;
	BOOL bOpenClip	= FALSE;

	HGLOBAL hClip = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,size + 1);
	if(hClip == NULL)
		return bRet;

	bAlloced = TRUE;
	char *temp = (char*)GlobalLock(hClip);
	if(temp == NULL)
	{
		goto ClipFail;
	}

	bLocked = TRUE;

	memcpy(temp,buf,size);
	temp[size] = 0;

	if(!OpenClipboard(NULL))
		goto ClipFail;

	bOpenClip = TRUE;
	EmptyClipboard();

	if(SetClipboardData(CF_TEXT,hClip) == NULL)
		goto ClipFail;

	bRet = TRUE;

ClipFail:
	if(bLocked)
		GlobalUnlock(hClip);
	if(bOpenClip)
		CloseClipboard();

	return bRet;
}

BOOL PasteFromClipboard(string& str)
{
	if(!OpenClipboard(NULL))
		return FALSE;

	HGLOBAL hClip=NULL;

	if(!(hClip=GetClipboardData(CF_TEXT)))
	{
		CloseClipboard();
		return FALSE;
	}
	char *temp = (char*)GlobalLock(hClip);
	str.assign(temp);

	GlobalUnlock(hClip);
	CloseClipboard();

	return TRUE;
}

const char* GetJobName(int iJob)
{
	if(iJob == 0)
		return "战士";
	else if(iJob == 1)
		return "魔法师";
	else if(iJob == 2)
		return "道士";
	else
		return "未知";
}

const char* GetGenderName(int iSex)
{
	if(iSex == 0)
		return "男";
	else
		return "女";
}

const char* GetHairStyle(int i)
{
	if(i< 0 || i>= NUM_HAIR_STYLE)
		return "";

	static char* HAIR_STYLE[] = 
	{
		"清凉型",
		"豪放型",
		"名士型",
		"侠客型",
		"洒脱型",
		"淑女型",
		"活泼型",
		"高贵型",
	};
	return HAIR_STYLE[i];
}

const char* GetHairColor(int i)
{
	if(i< 0 || i>= MAX_HAIR_COLOR)
		return "";

	static char* HAIR_COLOR[] = 
	{
		"白银色",
		"铅灰",
		"黑色",
		"赭绿",
		"咖啡",
		"宝石蓝",
		"墨绿",
		"苍竹",
		"灰蓝",
		"朱砂",
		"蝴蝶蓝",
		"竹叶青",
		"藤黄",
		"紫罗兰",
		"芭蕉绿",
		"玄铁",
	};
	return HAIR_COLOR[i];
}

float CalRate(int val,int vmax)
{
	float rate = 0.0f;
	if(vmax != 0)
		rate = (float) val / vmax;

	if(rate > 1.0f)
		rate = 1.0f;

	return rate;
}

//如果方向映射了，那么图片也需要绕Y轴映射！
void DirMapping(int dirCur,int dirMax,int &dirOut,int &bMapping)
{
	if(dirMax == 8 || dirMax == 16 )
	{
		dirOut = dirCur;
		return;
	}
	dirOut = 0;	//强制到0
};

void CutByUnicode(const char* strMsg,VString& vec,UINT iLimit,bool bCutByEnter,char token)
{
	string _str;

	char* p = (char*)strMsg;
	char* pBegin = (char*)strMsg;

	while(*p != '\0')
	{
		char* q = ::CharNext(p);

		if(bCutByEnter && (*p == '\r' || *p == '\n' || *p == token))
		{
			int l = (int)(p - pBegin);
			if(l > 0)
			{
				_str.assign(pBegin,l);
				vec.push_back(_str);
				_str.clear();
			}
			pBegin = q;
		}
		else
		{
			int l = (int)(q - pBegin);
			if(l >= iLimit)
			{
				_str.assign(pBegin,l);
				vec.push_back(_str);
				_str.clear();
				pBegin = q;
			}
		}
		p = q;
	}

	if(p != pBegin)
	{
		_str.assign(pBegin,(int)(p - pBegin));
		vec.push_back(_str);
	}
}

char GetDirByTile(int iDeltaX,int iDeltaY)
{
	if(iDeltaY>0)
	{
		if(iDeltaX>0)
			return DIR_RIGHTDOWN;
		else if(iDeltaX<0)
			return DIR_LEFTDOWN;
		else
			return DIR_DOWN;
	}
	else if(iDeltaY<0)
	{
		if(iDeltaX>0)
			return DIR_RIGHTUP;
		else if(iDeltaX<0)
			return DIR_LEFTUP;
		else
			return DIR_UP;
	}else
	{
		if(iDeltaX>0)
			return DIR_RIGHT;
		else if(iDeltaX<0)
			return DIR_LEFT;
		else
			return DIR_DOWN;
	}
	return DIR_NO;
}