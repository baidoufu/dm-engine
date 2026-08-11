#include "BoothData.h"
#include "Global/DebugTry.h"
#include "GameData/GameData.h"
#include "GameControl/GameControl.h"
#include "TalkMgr.h"
#include "GameData/PetData.h"

CBoothData     g_BoothData;
CLepoardBoothData g_PetBoothData;
COtherBoothData  g_OtherBoothData;
CBoothTalkName g_BoothTalkName;

CBoothData::CBoothData()
{
	m_nReadyPackPos = -1;
	m_bAdding = false;
	m_bDeleting = false;
	m_ReadyPrice.clear();
	m_iMaxGoodNum = 10;
}

void CBoothData::SetBoothName(const char* strName)
{
	m_strName.assign(strName); 
}

const char* CBoothData::GetBoothName()
{
	return m_strName.c_str(); 
}

GoodPrice_t& CBoothData::GetPrice(int nPos)
{
	return m_GoodsPrices[nPos];
}

GoodPrice_t& CBoothData::GetDropPrice()
{
	return m_DropPrice;
}

bool CBoothData::GetAddingState()
{
	return m_bAdding;
}

void CBoothData::SetAddingState(bool b)
{
	m_bAdding = b;
}

bool CBoothData::GetDeletingState()
{
	return m_bDeleting;
}

void CBoothData::SetDeletingState(bool b)
{
	m_bDeleting = b;
}

bool CBoothData::IsInBooth(DWORD nGoodID,const char* strName) 
{
	if (nGoodID == 0)
		return false;
	for (int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		if (m_GoodsPrices[i].GetID() == nGoodID &&
			stricmp(m_GoodsPrices[i].GetName(),strName) == 0)
			return true;
	}
	if (nGoodID == m_DropPrice.GetID())
		return true;
	return false;
}


bool CBoothData::GetPrice(int nGoodID,GoodPrice_t& price)
{
	for (int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		if (m_GoodsPrices[i].GetID() == nGoodID)
		{
			price = m_GoodsPrices[i];
			return true;
		}
	}
	return false;
}

void CBoothData::SetReadyGood(DWORD nID,const char* strName,int nPackPos)
{
	m_ReadyPrice.SetID(nID);
	m_ReadyPrice.SetName(strName);
	m_nReadyPackPos = nPackPos; 
}

void CBoothData::SetReadyPrice(DWORD ulMoney,DWORD ulYuanBao)
{
	m_ReadyPrice.SetMoney(ulMoney);
	m_ReadyPrice.SetYuanBao(ulYuanBao);
}

GoodPrice_t& CBoothData::GetReadyPrice()
{
	return m_ReadyPrice;
}

void CBoothData::ClearReadyGood()
{
	m_ReadyPrice.clear();
	m_nReadyPackPos = -1;
	m_bAdding = false;
}

void CBoothData::AddGood()
{
	if (m_ReadyPrice.GetID() == 0 || strlen(m_ReadyPrice.GetName()) == 0)
		return;

	if (m_ReadyPrice.GetMoney() == 0 && m_ReadyPrice.GetYuanBao() == 0)
	{
		m_ReadyPrice.clear();
		return;
	}

	//查找空位放入
	int iReadyPos = -1;
	for(int ig=0;ig<MAX_BOOTH_GOODS_NUM;ig++)
	{
		if(m_GoodsPrices[ig].GetID() ==0)
		{
			iReadyPos = ig;
			break;
		}
	}
	if (iReadyPos < 0)
	{
		m_ReadyPrice.clear();
		return;
	}

	m_GoodsPrices[iReadyPos] = m_ReadyPrice;
	m_nReadyPackPos = -1;
	m_ReadyPrice.clear();
	m_bAdding = false;
}

int CBoothData::GetReadyPackPos()
{
	return m_nReadyPackPos;
}

void CBoothData::Clear()
{
	m_ReadyPrice.clear();
	m_DropPrice.clear();

	m_nReadyPackPos = -1;
	for(int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		m_GoodsPrices[i].SetID(0);
	}
	m_strName.assign("");

	m_iMaxGoodNum = 10;
}

void CBoothData::CleanItem(DWORD id)
{
	for(int ii=0; ii <MAX_BOOTH_GOODS_NUM;ii++)
	{
		if (id == g_BoothData.GetPrice(ii).GetID())
		{
			m_GoodsPrices[ii].SetID(0);
			this->FullFill();
			break;
		}
	}
}

void CBoothData::FullFill()
{

	for(int ii = 0;ii < MAX_BOOTH_GOODS_NUM;ii++)
	{
		if(m_GoodsPrices[ii].GetID() != 0)
			continue;

		int j = ii+1;
		for(;j < MAX_BOOTH_GOODS_NUM;j++)
		{
			if(m_GoodsPrices[j].GetID() != 0)
				break;
		}

		if(j  == MAX_BOOTH_GOODS_NUM)
			break;

		m_GoodsPrices[ii] = m_GoodsPrices[j];
		m_GoodsPrices[j].clear();
	}
}

int CBoothData::GetMaxGoodNum()
{
	return m_iMaxGoodNum;
}

void CBoothData::SetMaxGoodNum(int iMax)
{
	m_iMaxGoodNum = iMax;
}

///////////////////////////////////////////////////////////////////////////
CLepoardBoothData::CLepoardBoothData()
{
	m_iGoodsLimit = 0;
	m_ID = 0;
	m_dwUploadingID = 0;
	m_iPetBoothType = 0;
	m_strRedirectPage.clear();
	ClearGoods();
}

void CLepoardBoothData::ClearGoods()
{	
	for(int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		m_Goods[i].SetID(0);
	}
}
void CLepoardBoothData::ClearC2CGoods()
{
	for(int i=0;i<MAX_UPLOAD_GOODS_NUM;i++)
	{
		m_C2CGoods[i].SetID(0);
		m_GoodsPrices[i].clear();
	}
}

GoodPrice_t& CLepoardBoothData::GetPrice(int nPos)
{
	return m_GoodsPrices[nPos];
}

void  CLepoardBoothData::SetUploadingID(DWORD id)
{
	m_dwUploadingID = id;
}

void  CLepoardBoothData::BackToPackage() //全部还到包裹中去
{
	for(int ii = 0;ii < MAX_UPLOAD_GOODS_NUM;ii++)
	{
		if(m_C2CGoods[ii].GetID() == 0)
			continue;

		SELF.PackageGood().BackToArray(m_C2CGoods[ii]);
		m_C2CGoods[ii].SetID(0);
		m_GoodsPrices[ii].clear();
	}
}

void  CLepoardBoothData::BackToPackage(DWORD id)
{
	for(int ii = 0;ii < MAX_UPLOAD_GOODS_NUM;ii++)
	{
		if(m_C2CGoods[ii].GetID() != id)
			continue;

		SELF.PackageGood().BackToArray(m_C2CGoods[ii]);
		m_C2CGoods[ii].SetID(0);
		m_GoodsPrices[ii].clear();
	}
	C2CFullFill();
}

void CLepoardBoothData::C2CFullFill()
{
	for(int ii = 0;ii < MAX_UPLOAD_GOODS_NUM;ii++)
	{
		if(m_C2CGoods[ii].GetID() != 0)
			continue;

		int j = ii+1;
		for(;j < MAX_UPLOAD_GOODS_NUM;j++)
		{
			if(m_C2CGoods[j].GetID() != 0)
				break;
		}

		if(j  == MAX_UPLOAD_GOODS_NUM)
			break;

		m_C2CGoods[ii] = m_C2CGoods[j];
		m_GoodsPrices[ii] = m_GoodsPrices[j];
		m_C2CGoods[j].SetID(0);
		m_GoodsPrices[j].clear();
	}
}

void CLepoardBoothData::AddC2CGood()
{
	GoodPrice_t& readyPrice = g_BoothData.GetReadyPrice();
	if(readyPrice.GetID() == 0)
		return;

	if(readyPrice.GetMoney() == 0 && readyPrice.GetYuanBao() == 0)
	{
		readyPrice.clear();
		return;
	}

	//包裹中有这样的物品
	CGood* pGood = SELF.PackageGood().FindGood(readyPrice.GetID());
	if(pGood)
	{
		for(int i = 0; i < MAX_UPLOAD_GOODS_NUM;i++)
		{
			if(m_C2CGoods[i].GetID() == 0)
			{
				m_C2CGoods[i].Assign(*pGood);
				pGood->SetID(0);
				m_GoodsPrices[i] = g_BoothData.GetReadyPrice();
				break;
			}
		}
	}
	readyPrice.clear();
}

///////////////////////////////////////////////////////////////////////////
//COtherBoothData
//
COtherBoothData::COtherBoothData()
{
	m_nOtherPlayerID = 0;
	m_nDaysRemain = 0;
	m_iSelected = -1;
	m_bUsedByLeaseBoothWnd = false;
	m_bAdding = false;
	m_ReadyGood.clear();
}
void COtherBoothData::SetBoothName(const char* strName)
{
	m_strName.assign(strName); 
}
const char* COtherBoothData::GetBoothName()
{   
	return m_strName.c_str();
}
CGood& COtherBoothData::GetGoods(int nPos)
{ 
	return m_Goods[nPos];
}

DWORD COtherBoothData::GetOtherPlayerID() 
{
	return m_nOtherPlayerID;
}
void COtherBoothData::SetOtherPlayerID(DWORD nID)
{  
	m_nOtherPlayerID = nID; 
}

void COtherBoothData::Clear()
{
	map<DWORD,CGoodPetInfo>& mapGoodPetInfo = g_PetData.GetGoodPetInfoMap();
	for (int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		if(m_Goods[i].GetID() != 0)
		{
			mapGoodPetInfo.erase(m_Goods[i].GetID());
			m_Goods[i].SetID(0);
		}
	}
	m_bAdding = false;
	m_VLeaseBoothType.clear();
}
bool COtherBoothData::IsInBooth(DWORD nGoodID,const char* strName)
{
	if (nGoodID == 0)
		return false;
	for (int i=0;i<MAX_BOOTH_GOODS_NUM;i++)
	{
		if (m_Goods[i].GetID() == nGoodID &&
			stricmp(m_Goods[i].GetName(),strName) == 0)
			return true;
	}

	return false;
}

void COtherBoothData::SetReadyGood(DWORD nID,const char* strName,int nPackPos)
{
	m_ReadyGood.SetID(nID);
	m_ReadyGood.SetName(strName);
	m_ReadyGood.SetPos(nPackPos);
}
bool COtherBoothData::GetAddingState()
{
	return m_bAdding;
}
void COtherBoothData::SetAddingState(bool b)
{
	m_bAdding = b;
}
void COtherBoothData::ClearReadyGood()
{
	m_ReadyGood.clear();
	m_ReadyGood.SetPos(-1);
	m_bAdding = false;
}
void COtherBoothData::AddGood()
{
	/*
	if (m_ReadyPrice.GetID() == 0 || strlen(m_ReadyPrice.GetName()) == 0)
	return;

	if (m_ReadyPrice.GetMoney() == 0 && m_ReadyPrice.GetYuanBao() == 0)
	{
	m_ReadyPrice.clear();
	return;
	}

	//查找空位放入
	int iReadyPos = -1;
	for(int ig=0;ig<MAX_BOOTH_GOODS_NUM;ig++)
	{
	if(m_GoodsPrices[ig].GetID() ==0)
	{
	iReadyPos = ig;
	break;
	}
	}
	if (iReadyPos < 0)
	{
	m_ReadyPrice.clear();
	return;
	}

	m_GoodsPrices[iReadyPos] = m_ReadyPrice;
	m_nReadyPackPos = -1;
	m_ReadyPrice.clear();
	m_bAdding = false;
	*/
	if (m_ReadyGood.GetID() ==0 || m_ReadyGood.GetName() == 0)
	{
		return;
	}
	if (m_ReadyGood.GetPrice() == 0)
	{
		m_ReadyGood.clear();
		return;
	}
	//查找空位放入
	int iReadyPos = -1;
	for(int ig=0;ig<MAX_BOOTH_GOODS_NUM;ig++)
	{
		if(m_Goods[ig].GetID() ==0)
		{
			iReadyPos = ig;
			break;
		}
	}
	if (iReadyPos < 0)
	{
		m_ReadyGood.clear();
		return;
	}
	m_Goods[iReadyPos] = m_ReadyGood;
	m_ReadyGood.SetPos(-1);
	m_ReadyGood.clear();
	m_bAdding = false;
}
bool CLepoardBoothData::IsPaimaiClosed()
{
	//服务器版本限制 + 客户端开关限制
	if((g_dwServerSwitch & SS_C2C) == 0)
	{
		g_TalkMgr.PushSysTalk("拍卖行即将开放,敬请关注");
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////
//记录摆摊中跟自己说话的买主名字

void CBoothTalkName::InsertName(const char* strName)
{
	if(strlen(strName) == 0)
		return;

	ListString::iterator it;
	for(it = m_List.begin();it != m_List.end(); it++)
	{
		if(0 == it->compare(strName))
			return;
	}
	m_List.push_back(strName);
}

bool CBoothTalkName::IsInList(const char* strName)
{
	ListString::iterator it;
	for (it = m_List.begin(); it != m_List.end(); it++)
	{
		if (0 == it->compare(strName))
			return true;
	}
	return false;
}

