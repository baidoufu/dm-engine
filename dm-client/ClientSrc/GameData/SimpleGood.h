///////////////////////////////////////////////////////////////////////
//文件名：   SimpleGood.h
//文件说明： 本文件是类CSimpleGood的头文件（包含实现）
//			 CSimpleGood用来记录游戏中掉落在地上的物品。
//			 由于每个物品有一个唯一ID，所以CSimpleGood
//			 重载了==，用ID来判断是否相等。
///////////////////////////////////////////////////////////////////////
#pragma once

#include "Global/Global.h"
#include "Global/Template.h"

class CSimpleGood
{
public:
	CSimpleGood();
	~CSimpleGood();

	void    SetID(DWORD nID);
	void	SetName(const char * name)                  { m_strName.assign(name);}
	inline void			SetLooks(DWORD nLooks)			{ m_nLooks = nLooks; }
	inline void			SetXY(int x,int y)				{ m_iX = x; m_iY = y;}

	inline void			GetXY(int &x,int &y)			{ x = m_iX; y =m_iY;}
	inline DWORD		GetID()							{ return m_nID; }
	inline int			GetLooks()						{ return m_nLooks;}
	inline const char *	GetName()						{ return m_strName.c_str();}

	inline DWORD		GetStar()						{ return m_dwStar; }
	inline void			AddStar()						{ m_dwStar++; }
	inline void         SetShowType(DWORD dwShowType)   { m_dwShowType = dwShowType; }
	inline DWORD        GetShowType()                   { return m_dwShowType;       }

	void   FromBuffer(const char* buf,int iLen);
private:
	DWORD	        m_nID;		 // 物品唯一编号
	int		        m_nLooks;	 // 外观
	int		        m_iX;        // 位置X
	int		        m_iY;        // 位置Y
	DWORD	        m_dwStar;    // 星号
	string			m_strName;	 // 物品名
	DWORD           m_dwShowType;// 显示类型
};

//角色列表结点
typedef ListNode<CSimpleGood> CSimpleGoodNode;
