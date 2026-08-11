///////////////////////////////////////////////////////////////////////
//文件说明：传世商城数据管理类
///////////////////////////////////////////////////////////////////////

#pragma once

#include "global/global.h"
#include "Global/StringUtil.h"

#define IN
#define OUT

#define CHARHEADER '&'
#define CHARMIDDLE '|'
#define	MAINGROUPCOUNT 7
#define HELPINDEX 6
#define PAYINDEX 7
#define HOTITEMCOUNT 10
#define RECVCOUNT   10
#define QUICKITEM_PET 5

enum
{
	//WOOOLSTORE_REAL = 1,  // 易宝商城
	WOOOLSTORE_PAY = 2 ,   // 充值页面
	//WOOOLSTORE_ORDER ,  // 排名系统 网页
	//WOOOLSTORE_PAIMAI , // 拍卖行 C2C商城 免费网页
	//WOOOLSTORE_PAIMAI2 , // 拍卖行 C2C商城 收费网页
	//WOOOLSTORE_JIFEN ,  // 积分商城
	//WOOOLSTORE_REGISTER ,//注册用户
	WOOOLSTORE_FENGHONG = 8,//分红商城
	WOOOLSTORE_NUM      // 代表IE网页数量,放在最后
};

extern const char*	MAINGROUPNAME[MAINGROUPCOUNT];

struct CWooolStoreItem
{// 对商城中单件商品的抽象
	CWooolStoreItem(){clear();}
	string	m_dwItemID;
	int		m_dwItemLooks;
	int		m_iPrice;	
	int		m_dwPreview;	// 当物品选中时的预览	暂时保留
	int		m_iTotalNum;
	string m_strName;
	string	m_strDesc;	// 对物品的描述
	int     m_iRealGood;    //是否为真实物品 0 虚拟物品 1 真实物品
    int     m_iLimitCount;  //是否限制销售 0 不限制 >0 限制销售的数量
	int     m_iLimitSaleRemain;//如果是限制销售的商品 代表剩余商品数量
	int     m_iType;// & 0x1:热销, & 0x2:新品
	void	clear()
	{ 
		m_iTotalNum = 1;
		m_dwItemID.clear();
		m_dwItemLooks = 0; 
		m_iPrice = 0; 
		m_dwPreview = 0;
		m_strName.clear(); 
		m_strDesc.clear();
		m_iRealGood = 0;
		m_iLimitCount = 0;//默认为不限制销售
		m_iLimitSaleRemain = -1;
		m_iType = 0;
	}

	const char*	VisitChild();
};

struct CWoolStoreSendData
{//赠送物品功能所需要的基本信息
	CWoolStoreSendData(){clear();}
	string m_strOriginUser;
	string m_strTerminalUser;
	string m_strSendWords;
	string m_strItemID;
	string m_strItemName;
	string m_strItemPrice;
	string m_strItemLooks;
	string m_strItemGrid;
	string m_strItemCount;//购买物品数量
	string m_strItemReal;//是否为实物道具 0 虚拟道具 1实物道具
	void clear()
	{
		m_strOriginUser.clear();
		m_strTerminalUser.clear();
		m_strSendWords.clear();
		m_strItemID.clear();
		m_strItemName.clear();
		m_strItemPrice.clear();
		m_strItemLooks.clear();
		m_strItemGrid.clear();
		m_strItemCount.clear();
		m_strItemReal.clear();
	}

};

struct SubGroup
{// 对一个子类(小类)的抽象
	SubGroup(){clear();}
	bool		m_bAlreadyQuery;
	int			m_dwSubGroupID;
	string m_strSubGroupName;
	vector<CWooolStoreItem>	m_vSubItem;	// 下面的具体的物品
	void		clear(){m_dwSubGroupID = 0; m_strSubGroupName.clear(); m_vSubItem.clear();m_bAlreadyQuery = false;}

	CWooolStoreItem*	VisitChild(unsigned int iNum);
};

struct MainGroup
{// 对一个大类的抽象
	MainGroup(){m_bAlreadyQuery = false;}
	bool		m_bAlreadyQuery;
	int			m_iMainGroupID;
	string m_strMainGroupName;
	vector<SubGroup>	m_vSubGroup;	// 下面的二级子目录
	
	SubGroup*	VisitChild(unsigned int iNum);
};

#define QUICKITEM_COUNT   6
#define PETQUICKITEM_COUNT   5

struct CQuickItem
{
	CQuickItem()
	{
		clear();
	}
	void clear()
	{
		strItemID.clear();
		iItemLooks = 0;
		strName.clear();
		iPrice = 1;
		iType = 0;
		dwTime = 0;
		iBuyType = 0;
	}
	CQuickItem& operator=(CQuickItem& qi)
	{
		if(&qi == this) return *this;

		dwTime = qi.dwTime;
		strName = qi.strName;
		iType = qi.iType;
		iPrice = qi.iPrice;
		iItemLooks = qi.iItemLooks;
		strItemID = qi.strItemID;
		iBuyType = qi.iBuyType;
		return *this;
	}
	std::string  strItemID;
	int  iItemLooks;
	int  iPrice;
	int  iType;//1为通过快捷消费2.0购买的物品，购买成功后要求一个红字提示,0为快捷商城
	int  iBuyType;//0:用普通元宝购买,1:用绑定元宝购买
	std::string strName;
	unsigned int dwTime;
};

struct CQuickBuyData
{
	CQuickBuyData()
	{
        clear();
	}

	void clear()
	{
		strMsg.clear();
		iType = 1;
		pItem = NULL;
		iUseAfterBuyType = 1;
		iObjPos = -1;
		iBuyCount = 1;
	}

	int  iType;
	int  iObjPos;//使用那一件物品触发了快捷消费？ 购买成功后再次使用这个物品，比如赤金宝箱双击后购买了钥匙要求再次使用赤金宝箱;
	int  iUseAfterBuyType;//0,不使用，1使用购买好了的物品，2,使用原先点击或使用的物品,3两个物品一起使用，如豹魔石和豹神水
	                      //4,御兽天袍/御兽天衣，买好后使用购买好了的物品，然后再骑上宠物 5，使用五色晶石，6喂豹粮,7使用灵皓石进入极品阁
	                      //8,开启魂珠,9,使用灵皓石进入仙灵密室
	                      //10,购买成功后立即使用,并且再次使用原来触发快捷消息的物品
	int  iBuyCount;//买多少件？默认为一件
	string strMsg;
	CQuickItem *pItem;
};

struct COpenStorePara//从消息公告点击商城道具后直接打开对应的商城页面的参数
{
	COpenStorePara()
	{
		iMainGroup = 0;
		iSubGroup = 0;
		iIndex = 0;
		id = 0;
	}

	int iMainGroup;//大类
	int iSubGroup;//子类
	int id;//商品id
	int iIndex;//该子类中的那一个物品
};
//vip 商城数据结构
struct CVipStoreItem
{
	CVipStoreItem()
	{
		m_wItemLooks = 0;
		m_wPrice = 0;
		m_wGoldCardPrice = 0;
		m_wSilverCardPrice = 0;
		m_iItemCount = 0;
		m_iLimitRemain = 0;
	}


	WORD m_wItemLooks;//外观
	WORD m_wPrice;//价格
	WORD m_wGoldCardPrice;//金卡会员价格
	WORD m_wSilverCardPrice;//银卡会员价格
	int  m_iItemCount;//购买物品数量
	int  m_iLimitRemain;//剩余商品数量
	string  m_strName;//名称
	string	m_strItemID;//编号
	string	m_strDesc;// 对物品的描述
};

struct CVipStoreData
{
	CVipStoreData()
	{
		clear();
	}

	void clear()
	{
		tOverdueTime = 0;
		tLastRequireTime = 0;
		strServiceMsg.clear();
		strTips.clear();
		vSellItems.clear();
		vPresentItems.clear();
	}

	__time32_t tOverdueTime;//到期时间
	string strServiceMsg;//上面的服务内容
	string strTips;//下面的小贴士
	vector<CVipStoreItem> vSellItems;//出售的物品
	vector<CVipStoreItem> vPresentItems;//赠送的物品
	__time32_t tLastRequireTime;//是一次请数据的时间,如果不在同一天要求重新请求

};

class CWooolStoreMgr
{// 商城管理器
public:
	CWooolStoreMgr(void);
	~CWooolStoreMgr(void);
	void	WSMFacade(const char* Msg, int iLen);					// 统一由该函数调度服务端协议对应的响应行为

    void    ParseResvItem(IN const char* pData, IN int iLen);			//分析接收赠品的信息
	void    ParseSendResult(IN const char* pData, IN int iLen,IN int iType);		//分析赠送物品的结果
	void    ParseRecvResult(IN const char* pData, IN int iLen,int iType);		//分析赠送物品的结果

	int		ParseSubMenu(IN const char* pData, IN int iLen);		// 分析服务端传来的小类名称
	int		ParseConcreteItem(IN const char* pData, IN int iLen);	// 分析出服务端传来的具体物品的数据
	void	ParseItemDesc(IN const char* pData, IN int iLen);		// 分析某个物品的具体描述
	int		ParseHotItem(IN const char* pData, IN int iLen);		// 分析热门商品
	void	ParseTradeResult(IN const char* pData, IN int iLen,bool bShow = true);	// 分析购买结果
	void	ParseScrollText(IN const char* pData, IN int iLen);		// 分析传来的滚动文字
	int		QueryInfo(IN unsigned int iMain, IN unsigned int iSub = -1, IN unsigned int iItem = -1);// 向服务器请求指定位置的数据,如果本机已有则直接返回
	int     QueryRecvItemInfo();//向服务端发送查询接收赠品的指令
	bool	IsHaveRequestPetQuick(){return m_bIsRequesPetQuick;}

	MainGroup*			GetMainGroupByID(string& ID, int& iMainIndex);				// 根据1级groupID得到对应指针,得到主项的索引
	SubGroup*			GetSubGroupByID(string& ID, int& iMainIndex, int& iSubIndex);// 根据2级groupID得到对应指针
	CWooolStoreItem*	GetItemByID(string& ID, int& iMainIndex, int& iSubIndex, int& iItemIndex);	// 根据物品ID,得到物品指针
	int GetItemIndexByID(int iMainIndex, int iSubIndex,int id);//通过ID获得一个商品在子类中的序号

	MainGroup*			GetMainGroupByIndex(unsigned int iMain)
	{// 通过索引得到主类的指针
		if (iMain < MAINGROUPCOUNT)
			return (&(m_MainGroup[iMain]));
		else 
			return NULL;
	}

	SubGroup*			GetSubGroupByIndex(unsigned int iMain, unsigned int iSub)
	{// 通过索引得到子类的指针
		MainGroup* p = GetMainGroupByIndex(iMain);
		if (p != NULL && iSub < p->m_vSubGroup.size())
			return (&(p->m_vSubGroup.operator [](iSub)));
		else 
			return NULL;
	}

	CWooolStoreItem*	GetItemByIndex(unsigned int iMain, unsigned int iSub, unsigned int iItem)
	{// 通过索引得到物品的指针
		SubGroup* p = GetSubGroupByIndex(iMain, iSub);
		if (p != NULL && iItem < p->m_vSubItem.size())
			return (&(p->m_vSubItem.operator [](iItem)));
		else
			return NULL;
	}

	CWooolStoreItem* GetHotItem(size_t iIndex)
	{// 通过某个热销物品指针
		if (iIndex < HOTITEMCOUNT)
			return (&(m_HotItems[iIndex]));
		else
			return NULL;
	}
	CWoolStoreSendData* GetSendData()
	{//直接返回赠送物品信息
		return &m_SendData;
	}

	CWoolStoreSendData* GetRecvData(size_t iIndex)
	{//直接返回接收物品信息
		return (&(m_RecvData[iIndex]));
	}

	CWoolStoreSendData* GetBuyData()
	{//直接返回接收物品信息
		return &m_BuyData;
	}

	void ClearData()
	{// 删除客户端暂存的商城物品的数据
		for (int i = 0; i < HOTITEMCOUNT; ++i)
			m_HotItems[i].clear();
		for (int i = 0; i < MAINGROUPCOUNT; ++i)
		{
			m_MainGroup[i].m_bAlreadyQuery = false;
			m_MainGroup[i].m_vSubGroup.clear();
		}
		m_TmpQuickItem.clear();
		m_BuyingQuickItem.clear();
		m_DropQuickItem.clear();
		m_VipStoreData.clear();
	}

	int GetScrollTextLines() {return (int)m_vScrollText.size();}
	const char* GetScrollTextContent(size_t iLine)
	{
		if (iLine < m_vScrollText.size()) 
		{
			return m_vScrollText[iLine].c_str();
		}
		return NULL;
	}

	void UseQuickItem(int iIndex ,bool bUse = true);
	void UsePetQuickItem(int iIndex,bool bUse = true);
	void BuyQuickItem(CQuickItem qitem,bool bUse = true,int iBuyCount = 1,bool bInputPetPkg = false,bool bQuickBuy = true);//bQuickBuy:是否快捷购买

	CQuickItem& GetQuickItem(int i) {return m_aQuickPkg[i]; };
	CQuickItem& GetTmpQuickItem() { return m_TmpQuickItem; }
	CQuickItem& GetBuyingQuickItem() { return m_BuyingQuickItem; }
	CQuickItem& GetQuickPetItem(int i){return m_aQuickPet[i];}

	CQuickItem& GetDropQuickItem(){ return m_DropQuickItem; }
	
	CQuickBuyData&  GetQuickBuyData(){return m_QuickBuyData;}
	void  SetQuickBuyItem(string strName);
	std::map<string,CQuickItem>&   GetQuickBuyItemMap(){return m_MQuickBuyItem;}

	COpenStorePara&  GetOpenStorePara(){return m_OpenStorePara;}

	void SetWooolStorePage(int iPage){ m_iWooolStorePage = iPage; }
	int  GetWooolStorePage() { return m_iWooolStorePage; }

	void SetPaiMaiStoreType(int iType){ m_iPaiMaiStoreType = iType; }
	int  GetPaiMaiStoreType(){ return m_iPaiMaiStoreType; }

	const char* GetWooolStoreGUID(){ return m_strWooolStoreGUID.c_str(); }
	void SetWooolStoreGUID(const char* str){ m_strWooolStoreGUID.assign(str); }
	CVipStoreData& GetVipStoreData(){return m_VipStoreData;}

protected:
	bool		GetMainString(IN const char*& pData, OUT string& strTarget);	// 获得@  @中的字符串,pData must Null terminated
	bool		GetSubString(IN const char*& pData, OUT int& iTarget);			// 获得一个|前的数字
	bool		GetSubString(IN const char*& pData, OUT string& strTarget);// 获得一个|前的字符
	MainGroup	m_MainGroup[MAINGROUPCOUNT];
	CWooolStoreItem m_HotItems[HOTITEMCOUNT];
	bool		m_bIsRequesPetQuick;//宠物快捷消费

	VString m_vScrollText;
	map<string,CQuickItem> m_MQuickBuyItem;
	CWoolStoreSendData  m_SendData;//赠送物品的消息结构
	CWoolStoreSendData  m_RecvData[RECVCOUNT];//接收物品的消息结构
	CWoolStoreSendData  m_BuyData;//购买物品的消息结构

	//快捷消费清单
	CQuickItem          m_aQuickPkg[QUICKITEM_COUNT];
	CQuickItem          m_TmpQuickItem;
	CQuickItem          m_BuyingQuickItem;
	CQuickItem          m_DropQuickItem;
	CQuickItem			m_aQuickPet[QUICKITEM_PET];

	//快捷消费2.0
	CQuickBuyData       m_QuickBuyData;
	COpenStorePara      m_OpenStorePara;

	int                 m_iWooolStorePage; //当前处理的内嵌IE页面编号
	int                 m_iPaiMaiStoreType;
	string              m_strWooolStoreGUID;
	//vip商城数据
	CVipStoreData       m_VipStoreData;
};

extern CWooolStoreMgr g_WooolStoreMgr;