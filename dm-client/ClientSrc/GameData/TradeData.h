#pragma once

#include "Good.h"
#include "GameDefine.h"

class CTradeData
{
public:
	CTradeData(void);
	~CTradeData(void);

	void Clear();

	CGood&			GetSelfGood(int nPos)					{ return m_SelfGoods[nPos];		}
	CGood&			GetAnotherGood(int nPos)				{ return m_AnotherGoods[nPos];	}
	CGood&			GetTempGood()							{ return m_tempGood;			}

	inline DWORD	GetSelfGolds()							{ return m_dwSelfGolds;			}
	inline DWORD	GetAnotherGolds()						{ return m_dwAnotherGolds;		}
	inline DWORD	GetAnotherYuanBao()						{ return m_dwAnotherYuanBao;	}
	inline DWORD	GetSelfYuanBao()						{ return m_dwSelfYuanBao;		}

	inline const char *		GetAnotherName()						{ return m_AnotherName.c_str(); }
	inline void				SetAnotherName(const char * name)		{ m_AnotherName		= name;		}

	inline void				SetAnotherGolds(DWORD golds)	{ m_dwAnotherGolds	= golds;	}
	inline void				SetAnotherYuanBao(DWORD golds)	{ m_dwAnotherYuanBao= golds;	}
	inline void				SetSelfGolds(DWORD golds)		{ m_dwSelfGolds		= golds;	}
	inline void				SetSelfYuanBao(DWORD golds)		{ m_dwSelfYuanBao	= golds;	}


	void SetTradeFlag(bool b)										{ m_bIsTrade = b;}
	bool GetTradeFlag()												{ return m_bIsTrade;}

	void  BackToPackage();
private:
	CGood	   m_SelfGoods[MAX_TRADE_OBJECTS];
	DWORD	   m_dwSelfGolds;
	DWORD	   m_dwSelfYuanBao;
	DWORD	   m_dwAnotherYuanBao;

	CGood	   m_AnotherGoods[MAX_TRADE_OBJECTS];
	DWORD	   m_dwAnotherGolds;
	
	string		m_AnotherName;
	CGood	   m_tempGood;

	bool	   m_bIsTrade;				//对方是否按下交易按钮
};

extern CTradeData g_TradeData;