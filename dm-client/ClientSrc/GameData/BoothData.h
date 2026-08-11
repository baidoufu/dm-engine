#pragma once

#include "Global/Global.h"
#include "Global/StringUtil.h"
#include "Good.h"


#define MAX_BOOTH_GOODS_NUM 30
#define MAX_BOOTH_NAME_LEN  52
#define PLAYER_LIMIT_BOOTH_GOODS_NUM 10//玩家摊位的数量
#define PET_LIMIT_BOOTH_GOODS_NUM 15//豹子摊位的最大数量
#define MAX_UPLOAD_GOODS_NUM  10  //拍卖行上传物品数目

class GoodPrice_t
{
private:
	string  strName;
	DWORD m_nID;
	DWORD m_ulMoney;
	DWORD m_ulYuanBao;
public:
	GoodPrice_t()
	{
		clear();
	}
	void clear()
	{
		strName.clear();
		m_nID = 0;
		m_ulMoney = 0;
		m_ulYuanBao = 0;
	}
	inline void SetName(const char* name){ strName.assign(name); }
	inline const char* GetName(){ return strName.c_str(); }
	inline DWORD GetID() {   return  m_nID;  }
	inline void SetID(DWORD nID) { m_nID = nID; }
	inline DWORD GetMoney() {  return m_ulMoney;  }
	inline void SetMoney(DWORD l) { m_ulMoney = l; }
	inline DWORD GetYuanBao()  { return m_ulYuanBao; }
	inline void SetYuanBao(DWORD l) {  m_ulYuanBao = l;  }
};

class CBoothData
{
private:
	GoodPrice_t m_GoodsPrices[MAX_BOOTH_GOODS_NUM];

	int         m_nReadyPackPos;   //即将加入物品在包裹中的位置
	string m_strName;         //摊位名
	GoodPrice_t m_ReadyPrice;      //即将加入的价格
	GoodPrice_t m_DropPrice;       //摊位上提起的物品价格
	bool        m_bAdding;         //摆摊时添加物品
	bool        m_bDeleting;       //摆摊时删除物品
	int         m_iMaxGoodNum;
public:
	CBoothData();
	void SetBoothName(const char* strName);
	const char* GetBoothName();
	GoodPrice_t& GetPrice(int nPos);
	GoodPrice_t& GetDropPrice();

	bool IsInBooth(DWORD nGoodID,const char* strName);
	bool GetPrice(int nGoodID,GoodPrice_t& price);

	void SetReadyGood(DWORD nID,const char* strName,int nPackPos);
	void SetReadyPrice(DWORD ulMoney,DWORD ulYuanBao);
	GoodPrice_t& GetReadyPrice();
	void ClearReadyGood();
	void AddGood();
	int  GetReadyPackPos();
	void Clear();
	void FullFill();
	void CleanItem(DWORD id);

	bool GetAddingState();
	void SetAddingState(bool b);
	bool GetDeletingState();
	void SetDeletingState(bool b);
	int GetMaxGoodNum();
	void SetMaxGoodNum(int iMax);

};

class CLepoardBoothData
{
private:
	CGood     m_Goods[MAX_BOOTH_GOODS_NUM]; //商品

	//提交拍卖物品的缓存 如果提交拍卖成功 显示图标 如果拍卖失败 删除缓存
	CGood     m_C2CGoods[MAX_UPLOAD_GOODS_NUM];
	GoodPrice_t m_GoodsPrices[MAX_UPLOAD_GOODS_NUM];

	DWORD     m_dwUploadingID; //正在上传的物品ID

	DWORD      m_ID;//豹子ID
	string    m_strLepoardBoothName;//豹子摊位名称
	int       m_iGoodsLimit;//豹子包裹摊位的最大数量

	string m_strRedirectPage;
	int m_iOpenBoothFromWhere;//1:原来的地方,2:从宠物属性界面的摆摊按钮
	int          m_iPetBoothType;
public:
    CLepoardBoothData();
	void ClearGoods();
	void ClearC2CGoods();

	CGood& GetGoods(int nPos){return m_Goods[nPos];}
	CGood& GetC2CGood(int nPos){return m_C2CGoods[nPos];}
	GoodPrice_t& GetPrice(int nPos);

	DWORD  GetUploadingID(){    return m_dwUploadingID; }
	void   SetUploadingID(DWORD id);

	void   C2CFullFill();
	void   BackToPackage(); //全部还到包裹中去
	void   BackToPackage(DWORD id);
	void   AddC2CGood();

	DWORD GetID(){return m_ID;}
	void   SetID(DWORD nID){ m_ID = nID;}
	void   SetGoodsLimit(int i){m_iGoodsLimit = i;}
	int    GetGoodsLimit(){return m_iGoodsLimit;}

	const char*  GetRedirectPage(){ return m_strRedirectPage.c_str(); }
	void   SetRedirectPage(const char* str){ m_strRedirectPage.assign(str); }

	bool   IsPaimaiClosed();
};

struct S_LeaseBoothType
{
	S_LeaseBoothType()
	{
		wDays = 0;
		wPrice = 0;
	}

	WORD wDays;//租用天数
	WORD wPrice;//价格
};

class COtherBoothData
{
private:
	string     m_strName; //摊位名
	CGood     m_Goods[MAX_BOOTH_GOODS_NUM]; //商品

	DWORD   m_nOtherPlayerID;
	WORD	m_nDaysRemain;						//租赁商铺还剩天数
	int       m_iSelected;      //选中哪个格子
	CGood     m_ReadyBuyGood;    //准备购买的物品
	CGood     m_AlreadyBuyGood;		//已经向服务器发出购买请求的物品
	CGood	  m_ReadyGood;		 //租赁摊位中准备出售的物品
	bool	  m_bUsedByLeaseBoothWnd;		//当自己租赁的窗口打开时,为true,需要客户端维护窗口内商品数据
	bool        m_bAdding;         //租赁摊位时添加物品
	bool        m_bDeleting;       //租赁摊位时删除物品
	vector<S_LeaseBoothType> m_VLeaseBoothType;//租赁摊位类型信息
public:
	COtherBoothData();
	void SetBoothName(const char* strName);
	const char* GetBoothName();
	CGood& GetGoods(int nPos);

	DWORD GetOtherPlayerID();
	void SetOtherPlayerID(DWORD nID);

	void Clear();
	bool IsInBooth(DWORD nGoodID,const char* strName);

	CGood& GetReadyBuy(){ return m_ReadyBuyGood; }
	CGood& GetAlreadyBuy(){ return m_AlreadyBuyGood;}
	void SetLeaseBoothWndOpen(bool open){ m_bUsedByLeaseBoothWnd = open;}
	bool IsMyLeaseBoothWndOpen(){ return m_bUsedByLeaseBoothWnd;}
	void SetReadyGood(DWORD nID,const char* strName,int nPackPos);
	CGood& GetReadyGood(){ return m_ReadyGood;}
	WORD GetDaysRemain(){ return m_nDaysRemain;}
	void SetDaysRemain(WORD days){ m_nDaysRemain = days;}
	void ClearReadyGood();
	bool GetAddingState();
	void SetAddingState(bool b);
	void AddGood();//将readygood放入goods数组中
	vector<S_LeaseBoothType> &GetLeaseBoothTypeVector() { return m_VLeaseBoothType; }

};

//记录在摆摊时跟自己说话的买主姓名
class CBoothTalkName
{
public:
	void InsertName(const char* strName);
	bool IsInList(const char* strName);
private:
	ListString m_List;
};

extern CBoothData     g_BoothData;
extern CLepoardBoothData g_PetBoothData;
extern COtherBoothData  g_OtherBoothData;
extern CBoothTalkName g_BoothTalkName;