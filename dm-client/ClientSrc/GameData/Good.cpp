#include "Good.h"
#include "GameAI/AIGoodMgr.h"
#include "Global/Global.h"
#include "BaseClass/Misc/Net.h" //为了使用Decode函数
#include "Global/Timer.h"
#include <assert.h>

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

CGood::CGood(const CGood& r)
{
	this->Assign((CGood&)r); //赋值
}

void CGood::getBuffer(char * buf,size_t & iSize)
{
	if(iSize < PKLength())
	{
		iSize = 0;
		return;
	}

	memcpy(buf,&m_ItemPacket,sizeof(m_ItemPacket));
	iSize = sizeof(m_ItemPacket);

	string szEncode = g_pNet->NewEncode(buf,iSize);
	memcpy(buf,szEncode.c_str(),szEncode.size());
	iSize = szEncode.size();
}

void CGood::handleBuffer(char * lpBuffer,int iSize)
{
	string buf = g_pNet->NewDecode(lpBuffer,iSize);
	lpBuffer = (char *)buf.c_str();
	iSize = buf.size();
	if(iSize < PKLength())
		return;

	FromBuffer(lpBuffer,false,iSize);	
}

void CGood::clear()
{
	memset(&m_ItemPacket,0,sizeof(m_ItemPacket));

	m_ExternString.clear();
	m_dwState = 0;
	m_iPos   = -1;
	m_AddEffect.iFrams = 0;
	m_iFromWnd = 0;
	m_iToWnd = 0;
	m_dwFlashTexID = 0;
	m_dwArrowTipWnd = 0;
	m_iArrowTipMsgID = -1;
}

void CGood::Assign(CGood& tmp)
{
	memcpy(&m_ItemPacket,&(tmp.m_ItemPacket),sizeof(m_ItemPacket));

	m_nFlag2	= tmp.m_nFlag2;
	m_nPayType  = tmp.m_nPayType;
	m_ExternString.assign(tmp.m_ExternString);
	m_iPos      = tmp.m_iPos;
	m_AddEffect.iFrams = tmp.m_AddEffect.iFrams;
	m_iFromWnd  = tmp.m_iFromWnd;
	m_iToWnd  = tmp.m_iToWnd;
	m_dwState  = tmp.m_dwState;
	m_dwFlashTexID = tmp.m_dwFlashTexID;
	m_dwArrowTipWnd = tmp.m_dwArrowTipWnd;
	m_iArrowTipMsgID = tmp.m_iArrowTipMsgID;
}

// p[0] 代表魔1属性
// p[2]&0xC0 代表魔类别
// p[2]&0x07 代表命运石属性
BYTE CGood::GetFateAttr()        //命运石的属性值(pt道具)
{
	DWORD ii = GetFlag();
	char * p =(char *) & ii;
	return (p[2]&0x07) ;
}

unsigned char CGood::GetMedalAtt()        //命运石的属性值(pt道具)
{
	return m_ItemPacket.szReserved[0];
}

void CGood::SetResvEx(const char * str)
{
	memcpy(m_ItemPacket.szReserved,str,10);
}

bool CGood::IsDamaged()
{
	if(m_ItemPacket.byStdMode ==  3 && m_ItemPacket.byShape == 213)
		return false;

    if((GetDemonDark3() & 0x0004))
        return true;

    return false;
}

bool CGood::IsValid()
{
    if( GetDemonDark2() != 0xFFFF && (GetDemonDark3() & 0x0002))//GetDemonDark2()==0xFFFF为商城道具
        return true;

    return false;
}


bool CGood::IsBind()
{
	//特殊物品，该字段被使用了
	if(m_ItemPacket.byStdMode == 46 && m_ItemPacket.byShape == 2) //邪心乾坤咒
		return false;

	if((GetDemonDark3() & 0x0001) != 0)
		return true;
	else
		return false;
}

bool CGood::IsRecorded()
{
	if((GetDemonDark3() & 0x0008) != 0)
		return true;
	else
		return false;
}

bool CGood::IsSafety()
{
	if((m_ItemPacket.szExtproperty3[12] & 0x01) != 0)
		return true;
	else
		return false;
}

bool  CGood::FromBuffer(const char* buf,bool bByBooth,int iGoodLen)
{
	if(buf[0] < 0 || buf[0] > 24)
		return false;

	int itemSize = sizeof(m_ItemPacket);
	memset(&m_ItemPacket,0,itemSize);
	int iLen = min(iGoodLen,itemSize);
	memcpy(&m_ItemPacket,buf,iLen);

	output_debug("Good Size %d\n", itemSize);

	m_ItemPacket.szName[23] = 0;//防止外挂等发来的错误数据

	m_nPayType = buf[31]; //支付类型


	if(bByBooth)
	{
		m_nFlag2 = 0;
	}
	else
	{
		m_nFlag2 = *((WORD*)(buf+52));
	}

	if(m_ItemPacket.byNeed == 7)
		m_nPayType = (m_nPayType & 0x80);
	else
		m_nPayType = (m_nPayType  & 0x81);


	return true;
}
//之前不带长度的协议的物品的长度,以后的协议都会把物品长度带过来,多条物品的协议已经改成了带长度的,单条的协议物品数据都是放在协议最后,所以暂时没改,以后要修改那条协议的时候必须把物品长度带过来
int CGood::PKLength()
{
	//return 78;
	return 87;
}

DWORD CGood::GetRecordTime(bool bFront)
{
	DWORD m;

	if(bFront)
	{
		*((BYTE *)&m)		= m_ItemPacket.byAC1;		
		*((BYTE *)&m + 1)	= m_ItemPacket.byAC2;
		*((BYTE *)&m + 2)	= m_ItemPacket.byMAC1;
		*((BYTE *)&m + 3)	= m_ItemPacket.byMAC2;
	}
	else
	{
		*((BYTE *)&m)		= m_ItemPacket.byDC1;		
		*((BYTE *)&m + 1)	= m_ItemPacket.byDC2;
		*((BYTE *)&m + 2)	= m_ItemPacket.byMC1;
		*((BYTE *)&m + 3)	= m_ItemPacket.byMC2;
	}
	return m;
}


DWORD CGood::GetFrozenTime()
{
	if(GetDemonDark2() == 0)
		return 0;

	//特殊物品，该字段被使用了
	if(m_ItemPacket.byStdMode == 46 && m_ItemPacket.byShape == 2) //邪心乾坤咒
		return 0;

	if(m_ItemPacket.byStdMode == 13)
	{
		return 0;
	}

	if(m_ItemPacket.byStdMode == 3 && m_ItemPacket.byShape == 213)
	{
		return 0;
	}

	//计算基准时间为2007-8-6标准0:00,也就是1186329600
	DWORD dwGoodTime = (1186329600 + GetDemonDark2() * 24 * 3600);

	//今天的日子
	__time32_t tNow;
	_time32(&tNow);
	tNow += g_dwServerTime;

	if(dwGoodTime < tNow)
		return 0;

	return dwGoodTime;
}

//物品数组
CGoodArray::CGoodArray()
{
	m_iSize = 0;
	m_GoodArray = NULL;
}

//物品数组
CGoodArray::CGoodArray(int iSize)
{
	m_iSize = iSize;
	if(m_iSize > 0)
	{
		m_GoodArray = new CGood[iSize];
		//for(int ii = 0;ii < iSize;ii++)
		//	m_GoodArray[ii].clear();
	}
	else
	{
		m_GoodArray = NULL;
	}
}

CGoodArray::~CGoodArray()
{
	SAFE_DELETE_ARRAY(m_GoodArray);
}

CGood& CGoodArray::Get(int i)
{
	if(i < 0 || i >= m_iSize)
	{
		char buf[128];
		sprintf(buf,"GoodArray越界:%d",i); 
		throw exception(buf);
	}

	return m_GoodArray[i];
}

CGood* CGoodArray::GetPtr(int i)
{
	if(i >= 0 && i < m_iSize)
		return &m_GoodArray[i];
	return NULL;
}

void CGoodArray::Clear()
{
	for(int i = 0;i < m_iSize;i++)
	{
		m_GoodArray[i].SetID(0);
	}
}

void CGoodArray::ReSize(int iSize,bool bFill)
{
	if(iSize == m_iSize) //相同时
		return;

	if(iSize == 0)
	{
		SAFE_DELETE_ARRAY(m_GoodArray);
		m_iSize = 0;
	}
	else
	{
		CGood*  NewGoodArray = new CGood[iSize];
		for(int i =  0; i < m_iSize && i < iSize; i++)
		{
			NewGoodArray[i] = m_GoodArray[i];
		}
		SAFE_DELETE_ARRAY(m_GoodArray);

		m_GoodArray = NewGoodArray;

		for(int i = m_iSize; i < iSize; i++)
		{
			m_GoodArray[i].SetID(0);
		}
	}
	m_iSize = iSize;
}

int CGoodArray::FindGoodPos(DWORD id,int iStartPos,int iEndPos)
{
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == id)
			return ii;
	}

	return -1;
}

CGood* CGoodArray::FindGood(DWORD id,int iStartPos,int iEndPos)
{
	int pos = -1;
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int i = iStartPos;i <= iEndPos;i++)
	{
		if(m_GoodArray[i].GetID() == id)
		{
			pos = i;
			break;
		}
	}

	if(pos < 0)
		return NULL;

	return &m_GoodArray[pos];
}

int CGoodArray::FindGoodByName(const char* name,int iStartPos,int iEndPos)
{
	if(!name)
	{
		return -1;
	}

	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(strcmp(m_GoodArray[ii].GetName(),name) == 0)
		{
			return ii;
		}
	}
	return -1;
}

int CGoodArray::CountGoodDuraByName(const char* name,int iStartPos,int iEndPos)
{
	int iDura = 0;
	if(!name)
	{
		return iDura;
	}

	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(strcmp(m_GoodArray[ii].GetName(),name) == 0)
		{
			iDura += m_GoodArray[ii].GetDura();
		}
	}

	return iDura;
}

int CGoodArray::ReverseFindGoodByName(const char* name,int iStartPos,int iEndPos)
{
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = m_iSize-1;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = 0;
	}

	for(int ii = iStartPos;ii >= iEndPos;ii--)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(strcmp(m_GoodArray[ii].GetName(),name) == 0)// && m_GoodArray[ii].GetDura() > 0)//治疗药水的dura为0导致不自动使用治疗药水
		{
			return ii;
		}
	}
	return -1;
}

int CGoodArray::FindGoodByIterator(GoodIterator& itr)
{
	for(int ii = 0;ii < m_iSize;ii++)
	{
		if(itr.DoIterator(m_GoodArray[ii]))
		{
			return ii;
		}
	}
	return -1;
}

int CGoodArray::FindGoodByStdmode(int iStdmode,int iShape,int iStartPos,int iEndPos)
{
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(m_GoodArray[ii].GetStdMode() == iStdmode &&
			(iShape == -1 || (m_GoodArray[ii].GetShape() == iShape)))
		{
			return ii;
		}
	}
	return -1;
}

int CGoodArray::FindGoodByStdmodeNameLevel(int iStdmode,int iShape,const char* name,int iStartPos,int iEndPos,int iLevel,int iMaxAC,DWORD dwExceptID,int iMaxDura)
{
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(m_GoodArray[ii].GetStdMode() == iStdmode &&
			(iShape == -1 || (m_GoodArray[ii].GetShape() == iShape)) &&
			( name == NULL || (strcmp("",name) == 0) || (strcmp(m_GoodArray[ii].GetName(),name) == 0) ) &&
			(iLevel == -1 || (m_GoodArray[ii].GetDura() / 10000) == iLevel) &&
			(iMaxAC == -1 || m_GoodArray[ii].GetAC() <= iMaxAC) && 
			(dwExceptID == -1 || m_GoodArray[ii].GetID() != dwExceptID) &&
			(iMaxDura == -1 || m_GoodArray[ii].GetDura() <= iMaxDura)
			)
		{
			return ii;
		}
	}
	return -1;
}

int CGoodArray::GetNumber(int iStdmode,int iShape,const char* name,int iStartPos,int iEndPos)
{
	int iCount = 0;
	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if( (iStdmode == -1 || m_GoodArray[ii].GetStdMode() == iStdmode) &&
			(iShape == -1   || m_GoodArray[ii].GetShape() == iShape) &&
			(name == NULL || (strcmp("",name) == 0) || (strcmp(m_GoodArray[ii].GetName(),name) == 0) )
			)
		{
			iCount ++;
		}
	}
	return iCount;
}

int CGoodArray::CountGoodDuraByStdmode(int iStdmode,int iShape,int iStartPos,int iEndPos)
{
	int iDura = 0;

	if (iStartPos < 0 || iStartPos >= m_iSize)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	for(int ii = iStartPos;ii <= iEndPos;ii++)
	{
		if(m_GoodArray[ii].GetID() == 0)
			continue;

		if(m_GoodArray[ii].GetStdMode() == iStdmode &&
			(iShape == -1 || (m_GoodArray[ii].GetShape() == iShape)))
		{
			iDura += m_GoodArray[ii].GetDura();
		}
	}

	return iDura;
}

bool CGoodArray::DeleteGood(DWORD id,bool bFill)
{
	bool bRtn = false;
	if(id == 0)
		return bRtn;

	for(int i = 0; i < m_iSize;i++)
	{
		if( id == m_GoodArray[i].GetID())
		{
			//后面的物品往前面移动
			if(bFill)
			{
				for(int j = i+1; j < m_iSize;j++)
				{
					m_GoodArray[j-1].Assign(m_GoodArray[j]);
				}
				m_GoodArray[m_iSize-1].SetID(0);
				//i --;
			}
			else
			{
				m_GoodArray[i].SetID(0);
			}
			bRtn = true;
		}
	}

	return bRtn;
}

void CGoodArray::ApplyGoodPos(GoodPos_t* PosArray,int iNum) //应用位置信息
{
	if(m_iSize == 0 || iNum == 0)
		return;

	CGood* tGood = new CGood[m_iSize]; //建立一个临时的物品数组
	for(int i = 0;i < m_iSize;i++)
	{
		tGood[i].SetID(0);
	}

	// 正确的位置就使用
	for(int ii = 0; ii < iNum; ii++)
	{
		int pos = PosArray[ii].iPos;
		if(pos < 0 || pos >= m_iSize)
			continue;

		CGood* pGood = FindGood(PosArray[ii].dwID);
		if(pGood)
		{
			if(tGood[pos].GetID() == 0)
			{
				tGood[pos].Assign(*pGood);
				tGood[pos].SetPos(pos);
				PosArray[ii].bDeal = true;
			}
		}
	}

	for(int ii = 0; ii < iNum; ii++) //非正确位置的
	{
		if(!PosArray[ii].bDeal)
		{
			CGood* pGood = FindGood(PosArray[ii].dwID);
			if(pGood)
			{
				for(int j = 0; j < m_iSize; j++)
				{
					if(tGood[j].GetID() == 0)
					{
						tGood[j].Assign(*pGood);
						tGood[j].SetPos(j);
						PosArray[ii].bDeal = true;
						break;
					}
				}
			}
		}
	}

	SAFE_DELETE_ARRAY(m_GoodArray);
	m_GoodArray = tGood;
}

CGood* CGoodArray::Add(const char* buf,int iGoodLen)
{
	int pos = FindGoodPos(0);
	if(pos < 0)
		return NULL;

	m_GoodArray[pos].FromBuffer(buf,false,iGoodLen);
	m_GoodArray[pos].SetPos(pos);

	return &m_GoodArray[pos];
}

CGood* CGoodArray::Add(CGood& tmp)
{
	int pos = FindGoodPos(0);
	if(pos < 0)
		return NULL;

	m_GoodArray[pos].Assign(tmp);
	//m_GoodArray[pos].SetPos(pos);//usingtemp 会调用add,这样导致放回去的位置不对
	return &m_GoodArray[pos];
}

int CGoodArray::GetGoodNumber()
{
	int iCount = 0;
	for(int i = 0;i < m_iSize;i++)
	{
		if(m_GoodArray[i].GetID() != 0)
			iCount++;
	}
	return iCount;
}

int CGoodArray::GetBlankNumber()
{
	int iCount = 0;
	for(int i = 0;i < m_iSize;i++)
	{
		if(m_GoodArray[i].GetID() == 0)
			iCount++;
	}
	return iCount;
}

bool CGoodArray::BackToArray(CGood& tmp,int pos)
{
	if(tmp.GetID() == 0)
		return false;

	if(pos >= 0 && pos < m_iSize)
	{
		if(m_GoodArray[pos].GetID() == 0)
		{
			m_GoodArray[pos].Assign(tmp);
			return true;
		}
	}

	CGood* pGood = FindGood(0);
	if(pGood)
	{
		pGood->Assign(tmp);
		return true;
	}
	return false;
}

void   CGoodArray::SortGoods(int iStartPos,int iEndPos)
{
	if (iStartPos >= m_iSize || iStartPos < 0)
	{
		iStartPos = 0;
	}
	if (iEndPos < 0 || iEndPos >= m_iSize)
	{
		iEndPos = m_iSize - 1;
	}

	//排序
	for (int i = iStartPos; i < iEndPos; i++)
	{
		for (int j = i + 1; j < iEndPos; j++)
		{
			if (m_GoodArray[j].GetID() == 0)//后面的为空,不动
			{
				continue;
			}

			if (m_GoodArray[i].GetID() == 0 || //前面空的往后排
				g_AIGoodMgr.CompareInPackage(&m_GoodArray[i], &m_GoodArray[j]) < 0)//优先级低的往后排
			{
				CGood temp = m_GoodArray[i];
				m_GoodArray[i] = m_GoodArray[j];
				m_GoodArray[j] = temp;
			}
		}
	}

}

void	CGood::SetID(DWORD nID) 
{
	m_ItemPacket.dwUniqueID = nID; 
	if (m_ItemPacket.dwUniqueID == 0)
	{
		clear();
	}

}

int CGood::GetRarity()
{
    int iRarity = 0;
    if(m_ItemPacket.dwUniqueID != 0 && m_ItemPacket.byStdMode == 49 && m_ItemPacket.byShape == 55)
    {
        if(m_ItemPacket.byAC1 == 0)
        {
            switch(m_ItemPacket.byAC2)
            {
            case 1:
            case 2:
            case 3:
            case 4:
                iRarity = 1;
                break;
            case 5:
            case 6:
            case 7:
                iRarity = 2;
                break;
            case 8:
            case 9:
                iRarity = 3;
                break;
            case 10:
                iRarity = 4;
                break;
            }
        }
        else if(m_ItemPacket.byAC1 == 2)
        {
            switch(m_ItemPacket.byAC2)
            {
            case 1:
            case 2:
            case 3:
                iRarity = 4;
                break;
            }
        }

        return iRarity;
    }
    else
    {
        return 0;
    }
}


int CGood::GetLooks()
{
	if(m_ItemPacket.byStdMode == 100 && m_ItemPacket.byShape >= 1 && m_ItemPacket.byShape <= 4 && m_ItemPacket.wLooks > 0)
	{
		//纹佩配件looks;
		return GetSubWenPeiLooks(m_ItemPacket.byShape, m_ItemPacket.wLooks);
	}
	return m_ItemPacket.wLooks;
}

//shape: 1:边纹,2:底纹,3:主纹,4:辅纹
int CGood::GetSubWenPeiLooks(int shape, int n)
{
	if (shape < 1 || shape > 4)
		return 0;

	return shape * 100 + n + 6000;
}


//纹佩looks
int  CGood::GetWenPeiBLooks()
{
	return GetSubWenPeiLooks(1, m_ItemPacket.szReserved[3]);
}

int	 CGood::GetWenPeiDLooks() 
{
	return GetSubWenPeiLooks(2, m_ItemPacket.szReserved[4]);
}

int  CGood::GetWenPeiZLooks() 
{ 
	return GetSubWenPeiLooks(3, m_ItemPacket.szReserved[5]);
}

int  CGood::GetWenPeiFLooks()
{
	return GetSubWenPeiLooks(4, m_ItemPacket.szReserved[6]);
}

int CGood::GetWenPeiSubLooks(int i)
{
	switch (i)
	{	
	case 0:
		return GetSubWenPeiLooks(2, m_ItemPacket.szReserved[4]);
	case 1:
		return GetSubWenPeiLooks(4, m_ItemPacket.szReserved[6]);
	case 2:
		return GetSubWenPeiLooks(3, m_ItemPacket.szReserved[5]);
	case 3:
		return GetSubWenPeiLooks(1, m_ItemPacket.szReserved[3]);
	default:
		break;
	}
	return 0;
}

int CGood::GetFaBaoLingGenLevel()
{
	return m_ItemPacket.szExtproperty3[7];
}

int CGood::GetFaBaoLingGenExp()
{
	return ((WORD)(m_ItemPacket.szExtproperty3[8])) | ((WORD)(m_ItemPacket.szExtproperty3[9]) << 8);
}

int CGood::GetFaBaoLingGenMaxExp()
{
	return ((WORD)(m_ItemPacket.szExtproperty3[10])) | ((WORD)(m_ItemPacket.szExtproperty3[11]) << 8);
}

bool CGood::HaveOpenFaBaoLingGen()
{
	return (GetFaBaoLingGenMaxExp() > 0);
}




