#pragma once
#pragma warning (disable:4018)

#include "Good.h"
#include "Global/StringUtil.h"
#include "GameData/TagText.h"

#define MAX_GEM_COUNT  6
#define MAX_COMMON_GOOD_COUNT 8
#define MAX_ORE_COUNT 16
#define MAX_LEVELUP_COUNT  7

#define MAX_TREASUER_BOX_NUM      8
#define MAX_TOTAL_BOX_NUM         12

#define MAX_FABAO_GOOD_COUNT	 24
#define MAX_HUFU_GOOD_COUNT		 2


#pragma pack()

struct _StorageData
{
	_StorageData(const char * str,int durS,int durB,int ID,int iflag = 0)
	{
		name = str;
		durSmall = durS;
		durBig = durB;
		id = ID;
		flag = iflag;
	}
	inline bool operator ==(const _StorageData & t)
	{
		if(id == t.id&&durSmall==t.durSmall&&durBig==t.durBig)
			return true;
		else
			return false;
	}

	string name;
	int durSmall;
	int durBig;
	int id;
	int flag; //新增状态：0,无；1:锁，2:封，3：封锁
};

struct _SellData
{
	_SellData(const char * str,int b,int pr,int i,BYTE type = 0)
	{
		name.assign(str);
		price	= pr;
		bNext	= b;
		id		= i;
		byType  = type;
		strId.clear();
		nDura   = 0;
	}
	_SellData(const char * str,const char* ptid,int pr,BYTE type = 0)
	{
		name.assign(str);
		price	= pr;
		strId.assign(ptid);
		bNext	= 0;
		id		= 0;
		byType  = type;
		nDura   = 0;
	}
	inline bool operator ==(const _SellData & t)
	{
		if(id == 0)
		{
			if(id == t.id && price==t.price && bNext==t.bNext && byType == t.byType)
				return true;
			else
				return false;
		}
		else
		{
			return false;
		}
	}
	string		name;
	int				price;
	int				id;
	int				bNext; //在二级列表中表示持久力
	BYTE            byType;//0:普通金币或元宝购买物品，1：绑定金币或绑定元宝购买的物品
	string     strId; //PT物品购买Id
	int        nDura; //一级购买列表中的持久值
};

struct _ACTIVITY
{
	_ACTIVITY(const char* Name,const char* Time,const char* Date)
	{
		strName.assign(Name);
		strTime.assign(Time);			
		strDate.assign(Date);
	}
	string strName;
	string strTime;
	string strDate;
	string content;
	string prize;
};


struct _TreasureData
{
	int Looks;
	BYTE byGoodTyep;//1普通，2为贵重物品，要求闪金光
	bool bTrue;
	char strName[15];
	_TreasureData()
	{
		Looks = 0;
		byGoodTyep = 0;
		bTrue = false;
		memset(strName,0,sizeof(strName));
	}
	_TreasureData(_TreasureData& src)
	{
		Looks = src.Looks;
		byGoodTyep = src.byGoodTyep;
		bTrue = src.bTrue;
		strcpy(strName,src.strName);
	}
	void Clear()
	{
		Looks = 0;
		byGoodTyep = 0;
		bTrue = false;
		strcpy(strName,"");
	}
};

struct _WuXingResult
{
    _WuXingResult()
    {
        dwTime = 0;
        bySuccess = 0xff;
        dwMonster = 0;
        bGameOver = false;	
        dwOverTime = 0;
        bRoundOver = false;
        dwRoundExp = 0;
        dwRound = 0;
        dwRoundOverTm = 0;
    }

    void Clear()
    {
        dwTime = 0;
        bySuccess = 0xff;
        dwMonster = 0;
        bGameOver = false;
        dwOverTime = 0;
        bRoundOver = false;
        dwRoundExp = 0;		
        dwRound = 0;
        dwRoundOverTm = 0;
    }

    DWORD dwRound;		//当前第几轮
    bool  bRoundOver;	//回合结束
    DWORD dwRoundOverTm;//本回合结束时间
    DWORD dwRoundExp;	//回合获得多少万经验数

	bool  bGameOver;	//五行任务结束任务
	DWORD dwOverTime;	//五行任务结束时间
	DWORD dwTime;		//耗费时间
	BYTE  bySuccess;	//0失败1成功2顺利3完美4成功发奖并关闭弹出窗口
	DWORD dwMonster;    //逃逸怪物数量
	std::vector<std::string> strPrize;	//奖品名称
};

struct _TreeGift
{
	_TreeGift()
	{
		wLooks = 0;
	}
	int    wLooks;
	string strName;
};

typedef vector<_TreeGift> VTreeGift;

struct _PetMerge
{
    _PetMerge()
    {
        Clear();
    }

    void Clear()
    {
        FirstPet.SetID(0);
        SecondPet.SetID(0);
        Seal.SetID(0);
        byTestResult = byMergeResult = 0xff;
        iLingLi = iMoney = 0;
		result.clear();
    }

    CGood FirstPet;	//主灵兽
    CGood SecondPet;	//副灵兽
    CGood Seal;		//封元印

    byte byTestResult;	//测试结果
    byte byMergeResult;	//幻化结果

    int iLingLi;		//玩家灵力值
    int iMoney;			//玩家金币(单位为万)

	string result;

};

struct ZhenBaoGrid
{
	ZhenBaoGrid()
	{
		clear();
	}

	void clear()
	{
		iType = iState = uLooks = iCount = dwOpenTime = 0;
		strOwnerName = "";
		strPrizeName = "";
		bShine = false;
	}

	int iType;		//格子类型 0 经验 1装备 2图腾 3 未开启的极品阁 4 开启的极品阁
	int iState;     //格子显示状态 0 未打开 1已点击但未失效 2 已打开（既不能点击） 100 点击格子后未发送协议，处于播放特效状态
	DWORD uLooks;
	int iCount;		//表示经验值的数量
	string strOwnerName;  //打开格子的玩家的名字
	string strPrizeName;  //格子的奖励
	DWORD dwOpenTime;	//格子打开的时间
	bool bShine;	//是否贵重物品需要闪光
};

struct JiPinGrid
{
	JiPinGrid()
	{
		Clear();
	}

	void Clear()
	{
		iType = iState = dwOpenTime = 0;
		strOwnerName = "";
		prize.SetID(0);
		bShine = false;
	}

	int iType;		//格子类型 0物品 1骷髅
	int iState;     //格子显示状态 0 未打开 1已点击但未失效 2 已打开（既不能点击）100 点击格子后未发送协议，处于播放特效状态
	string strOwnerName;  //打开格子的玩家的名字
	CGood prize;	//获得的物品
	DWORD dwOpenTime;	//格子打开的时间
	bool bShine;	//是否贵重物品需要闪光
};

struct _ZhenBaoState
{
	_ZhenBaoState()
	{
		bSpecialNode = false;	
		bInViewMode = false;
		iOriginPos = -1;
		strHint.clear();
	}

	void Clear()
	{
		iOriginPos = -1;
		bSpecialNode = false;
		bInViewMode = false;
		
		for(int i = 0;i < 77;i++)
		{
			grid[i].clear();
		}

		for(int i = 0;i < 36;i++)
		{
			jipingrid[i].Clear();
		}
	}

	ZhenBaoGrid grid[77];	//珍宝阁77个格子
	JiPinGrid jipingrid[36];//极品阁36个格子
	bool bSpecialNode;      //是否节点状态	
	int iOriginPos;         //记录进入极品阁所点击的珍宝阁格子的位置
	bool bInViewMode;		//是否处于观赏模式
	string strHint;			//提示语句
};

struct _ShenGongFangGoods
{
	_ShenGongFangGoods()
	{
		bSend = false;
		iResult = -1;		
	}

	void clear()
	{
		bSend = false;
		iResult = -1;		
		m_Shield.clear();
		m_Wood.clear();
		m_Luck.clear();		
	}

	CGood    m_Shield;		//行会盾牌
	CGood    m_Wood;		//炼制盾牌的木材
	CGood    m_Luck;		//幸运符
	bool	 bSend;
	int      iResult;		//1成功0留盾牌扣其它2全部物品扣光3有假物品或物品品质太低4在护宝符的保护下没有收盾牌	
};

struct _YUANSHI_RESULT		//拆解合成原石的测试结果
{
	_YUANSHI_RESULT()
	{
		iResult = -1;
	}

	int iResult;		//0不能1可以-1还没收到结果	
	CTagText tagText;
	CGood m_Good;		//托盘中的物品
	bool bExpensive;	//是否贵重物品
};

struct _DRILL_RESULT	//装备打孔结果
{
	_DRILL_RESULT()
	{
		iResult = -1;
	}

	int iPercent;	//成功百分比
	int iResult;	//0打孔成功1打孔失败2可以打孔3不能打孔	
	CGood m_equip;  //被打孔装备
	CGood m_Fushi;	//符石
	CGood m_Luck;	//辅助道具

	CGood m_FuShiToCombine[5];	//需要合成的符石	
};


struct _BaoJianData		//群英宝鉴数据
{
	struct _BaoJianPrize	//群英宝鉴奖励物品
	{
		string name;	//物品名
		WORD looks;		//物品Looks
		DWORD dwnum;	//数量
		WORD wgrade;	//品阶或个数,持久等
	};

	_BaoJianPrize prize[12];

	BYTE byCurrentRound;
	BYTE byFirstPrize;
	BYTE bySecondPrize;
	BYTE byThirdPrize;
	BYTE byAdditionalPrize;

	DWORD anger;	//铁英之怒

	DWORD dwCloseTime;
};

struct _InstanceZonePrize
{
	bool bOpen[5];	//五扇门有没有被开启
	WORD wlooks[5];	//五扇门奖励的looks
	string name[5]; //五扇门奖励的名字
	BYTE byNeedYuanBao;	//开门需要的元宝
	int iNeedGold;	//开门需要的金币	
};

typedef std::map<DWORD,CGood> MOperateGood;

struct _YuanBaoAllInfo
{
	struct _YuanBaoSellInfo
	{
		string strUserId;
		string strPlayerName;
		DWORD dwYuanBaoCount;
		DWORD dwPrice;
	};
	
	typedef vector<_YuanBaoSellInfo> VYuanBaoSellInfo;

	VYuanBaoSellInfo vecYuanBaoSellInfo;

	DWORD dwMySellNum;
	DWORD dwMySellPrice;
	DWORD dwMoneyToTake;
};

// struct _QunYingMember
// {
// 	string strName;
// 	BYTE byJob;
// 	BYTE byLevel;
// 	BYTE bySex;
// 	BYTE byTimeLevel;	//(byTimeLevel == 1)小于2小时,(byTimeLevel == 2)小于5小时,(byTimeLevel == 3)大于5小时
// 	WORD wPosX;
// 	WORD wPosY;
// };

//商行收购信息
struct _FirmData 
{
	struct _ItemNameID
	{
		string strItemName;
		DWORD  dwItemID;
		DWORD  nLevel;
		bool   bCanOverlap;//可否叠加
	};

	struct _ItemListInfo 
	{
		DWORD	dwRowID;
		string	strPlayerName;
		string	strItemName;
		DWORD	dwItemIdx;
		DWORD	nLevel;
		DWORD	nUnitNum;
		DWORD	nNumPerUnit;
		DWORD	dwPrice;
		BYTE	byTradeType;
	};

	_FirmData::_FirmData():bEnd(1),iCurPage(1){}

	BYTE bEnd;		//是不是最后一页
	WORD  iCurPage;	//当前页数
	
	vector<_ItemNameID>		vecItemNameID;		//收购物品种类
	vector<_ItemListInfo>	vecFirmItemList;	//某一类物品收购信息
};

struct _EquipSoulInfo	// 器魂
{
	_EquipSoulInfo()
	{

	}

	CGood m_Equip;  // 装备
	CGood m_Hunshi;	// 妖魂石
	CGood m_Jiejing[5];	// 器魂结晶
};

struct FireArtificeResult 
{
	FireArtificeResult():wLooks(0),iPercent(0),dwNeedMoney(0){}

	void clear()
	{
		wLooks = 0;
		iPercent = 0;
		dwNeedMoney = 0;
		strGoodName.clear();
	}

	WORD wLooks;
	int iPercent;
	DWORD dwNeedMoney;
	std::string strGoodName;
};


#define PERONS_PER_PAGE_VIPTRADEWND   5 //vip商行每页显示多少个人

//一个角色的vip商和数据
struct SVipTradGood_OnewPerson
{
	string strOwner;//出售者
	int iVipExp;//Vip经验
	vector<CGood> VGoods;//出售物品
};

typedef map<int,SVipTradGood_OnewPerson> MVipGood;//每一页商行会员物品数据,int是会员编号,从0开始,每页5个

//所有角色的vip商行数据
struct SVipTradGood
{
	int iVipPersons;//vip商行会员总数
	bool bHaveRequireData;//是否请求过数据
	MVipGood mMVipGood;//每一页商行会员物品数据,int是会员编号,从0开始,每页5个

	void Clear()
	{
		iVipPersons = 0;
		bHaveRequireData = false;
		mMVipGood.clear();
	}
};


#pragma pack(push,1)

//赤金护符
struct SChiJinRetPacket
{
	/*BYTE	byDC1;
	BYTE	byDC2;
	BYTE	byMC1;
	BYTE	byMC2;
	BYTE	bySC1;
	BYTE	bySC2;
	BYTE	byAC1;
	BYTE	byAC2;
	BYTE	byMAC1;
	BYTE	byMAC2;
	BYTE	byBaoJi;
	BYTE	byFanTan;
	BYTE	byXiXie;
	BYTE	byJueFang;
	BYTE	byPoFang;*/
	int	byDC1;
	int	byDC2;
	int	byMC1;
	int	byMC2;
	int	bySC1;
	int	bySC2;
	int	byAC1;
	int	byAC2;
	int	byMAC1;
	int	byMAC2;
	int	byBaoJi;
	int	byFanTan;
	int	byXiXie;
	int	byJueFang;
	int	byPoFang;
};

/*
enum HMSWATCH_EVENT
{
HMSWATCH_EVENT_START = 0,	//开始训练
HMSWATCH_EVENT_BUFF,		//影响事件(有益)
HMSWATCH_EVENT_DEBUFF,		//影响事件(有害)
HMSWATCH_EVENT_UNFILL,		//未充分修炼结束
HMSWATCH_EVENT_FILL,		//充分修炼结束
};
*/
//骑术非值守信息
struct HmsWatchEventProtocal
{
	BYTE	Type;					//事件类型HMSWATCH_EVENT
	DWORD	ActiveTime;				//事件时间
	char	ActivePlayerName[16];	//事件发起人
	char	ToPlayerName[16];		//事件影响人
	char	ItemName[16];			//使用物品名
	int		Value;					//事件影响值
};

#pragma  pack(pop)


//骑术非值守信息
struct HorsemanshipWatchInfo
{
	char	m_szPlayerName[16];			//
	BYTE	m_bFlag;					//当前训练状态 0未训练,1训练中,2已训练,3已领取经验
	WORD	m_wBasePure;				//初始提纯度
	WORD	m_wPure;					//提纯度
	DWORD	m_dwTrainTime;				//修炼时间	
	DWORD 	m_dwActionPoint;			//行动点数

	DWORD	m_dwAllTrainTime;			//修炼总时间
	DWORD	m_dwTrainClientStartTime;	//时间
	
	std::vector<HmsWatchEventProtocal> m_event;	//日志

	HorsemanshipWatchInfo()
	{
		clear();
	}
	void clear()
	{
		memset(m_szPlayerName, 0, 16);
		m_bFlag = 0;
		m_wPure = 0;
		m_wBasePure = 0;
		m_dwTrainTime = 0;
		m_dwAllTrainTime = 8 * 3600 * 1000;
		m_dwActionPoint = 0;
		m_dwTrainClientStartTime = 0;
		m_event.clear();
	}
};


class CNpcData
{
public:
	CNpcData(void);
	~CNpcData(void);

	void Clear();

	//NPC ID名字，托盘名等
	void SetID(DWORD id);
	DWORD GetID();
    
	void SetName(const char * str);
	const char* GetName();
	void SetTrayTitle(char *str);
	const char* GetTrayTitle();

	//物品的买/卖/存放
	void SetSaleObjectValue(DWORD v);
	DWORD GetSaleObjectValue();

	void SetStorageMaxCount(WORD wCount);
	int GetStorageMaxCount();
	void SetExchangeFromQuickNpcWnd(bool b);
	bool IsExchangeFromQuickNpcWnd();
	vector<_SellData>   & GetVectorSellData();
	vector<_SellData>   & GetVectorPTData();
	vector<CGood> & GetNpcQuickVector();
	SVipTradGood &GetVipTradeGood();
	CGood & GetGood();
	void SetSaleGold(DWORD iMoney);
	DWORD GetSaleGold();
	bool GetStart();
	void SetStart(bool start);
	vector<_StorageData> & GetStorageData();
	void SetSecondBuy(bool b);
	bool GetSecondBuy();
	void SetPoisonFlag(bool b);
	bool GetPoisonFlag();
	vector<CGood> & GetGoodList();
	bool IsRepairWindow();
	void SetRepairWindow(bool b);
	int  GetSecBuyStartPos();
	void SetSecBuyStartPos(int start);
	
	VString& GetLeavWords();

	void SetChangeLogo(bool b);
	bool GetChangeLogo();
	void SetForgeExp(DWORD dw);
	DWORD GetForgeExp();

	//黑暗残留换取装备升级
	CGood& GetEquLUpGood(int i);
	bool IsLUpintEqu();
	void SetIsLUpintEqu(bool b);
	int  GetLUpFrame();
	void SetLUpFrame(int i);

	//宝石的凝练和合成
	CGood& GetOreGood(int i);
	CGood& GetGemGood(int i);
	bool  IsCondensing();
	void  SetIsCondensing(bool b);
	int   GetCondenseFrame();
	void  SetCondenseFrame(int i);
    
	//宝箱的开启
	_TreasureData& GetBoxGood(int iPos);

	int   GetBoxOpenState();
	void  SetBoxOpenState(int i);
	int   GetBoxType();
	void  SetBoxType(int i);
	int   GetBoxOpenCycle();
	void  SetBoxOpenCycle(int i);

	//设定装包裹的物品
	DWORD GetPackGoodID();
	void  SetPackGoodID(DWORD dwID);

	//锁包裹的物品
	DWORD GetSuoPackGoodID();
	void  SetSuoPackGoodID(DWORD dwID);

	//英雄榜
	VString&   GetVectorStrData();
	int  GetVectorStrCount();
	string&  GetTitleStrData();
	void SetTitleStrData(const char* str);

	void SetLastCommand(string& str);
	const char *  GetLastCommand();

	//物品提交界面
	CGood& GetSubmitGood();
	void   SetSubmitGoodType(int iType);
	int    GetSubmitGoodType();

	void   SetSubmitGoodMsg1(const char* str);
	void   SetSubmitGoodMsg2(const char* str);
	const char* GetSubmitGoodMsg1();
	const char* GetSubmitGoodMsg2();


	vector<_TreeGift>* GetGiftsByTreeLevel(int iTreeLevel);//新年树由高到低分为三层
	void  SetPrayTreeLevel(BYTE byLevel);
	BYTE  GetPrayTreeLevel();
	string& GetPrayTreePrize();
	CGood& GetPrayCharm();

	//每周任务
	vector<_ACTIVITY> & GetVectorActData();

	//处理物品
	CGood* FindNpcGood(DWORD id);
	void   BackToPackage();
	_WuXingResult& GetWuXingResult(){ return m_stWuxingResult;}
    CGood& GetDamageGood(){ return m_DamageGood;}
    _PetMerge& GetPetMergeData(){return m_iPetMergeData;}	
	_ZhenBaoState& GetZhenBaoState(){return m_ZhenBaoState;}
	_ShenGongFangGoods& GetShenGongFangGoods(){return m_ShenGongFangGoods;}

	_YUANSHI_RESULT& GetYuanShiResult(){return m_YuanShiResult;}
	_DRILL_RESULT& GetDrillResult(){return m_DrillResult;}
	bool DelOperateGood(DWORD dwID);
	void AddOperateGood(CGood& good);
	CGood* GetOperateGood(DWORD dwID);
	bool RetToPkgFromOperateGood(DWORD dwID);

	CGood& GetCharGood();
	CGood& GetCharChangeGood();

	CGood& CNpcData::GetZhenPuGood();

	CGood& CNpcData::GetWuSeGood();

	_EquipSoulInfo& GetEquipSoulInfo();

	_BaoJianData& GetBaoJianData(){return m_BaoJian;}
	_YuanBaoAllInfo& GetYuanBaoSellInfo(){return m_AllYuanBaoInfo;}
	_InstanceZonePrize& GetInstancePrize(){return m_InstancePrize;}
	_FirmData&	GetFirmData(){ return m_FirmData;}


	CGood& CNpcData::GetBlessGoodLeft()		{ return m_kBlessGoodLeft; }
	CGood& CNpcData::GetBlessGoodRight()	{ return m_kBlessGoodRight; }
	CGood& CNpcData::GetBlessGoodFu()		{ return m_kBlessGoodFu; }

	CGood& CNpcData::GetECGoodLeft()		{ return m_kECGoodLeft; }
	CGood& CNpcData::GetECGoodRight()		{ return m_kECGoodRight; }

	CGood& CNpcData::GetChaifenGood()		{ return m_EquipChaifenGood; }


	CGood& CNpcData::GetFaBaoGood(int i)	{ return m_GoodFaBao[i]; }	

	CGood& CNpcData::GetFireArtGood(int i, int j)	{ return m_GoodFireArt[i][j]; }

	CGood& CNpcData::GetHufuGood(int i)		{ return m_GoodHufu[i]; }

	const std::string& GetSelFriendName() const	{ return m_strSelFriendName; }
	void SelFriendName(const std::string& val)	{ m_strSelFriendName = val; }
	CGood& GetGoodHorsemanship()				{ return m_kGoodHorsemanship; }
	HorsemanshipWatchInfo& GetHorsemanshipWatchInfo()		{ return m_kHorsemanshipWatchInfo; }
	HorsemanshipWatchInfo& GetOtherHorsemanshipWatchInfo()	{ return m_kOtherHorsemanshipWatchInfo; }

	
// 	vector<_QunYingMember>& GetSafeQunYingMember(){return m_vecSafeQunYingMember;}
// 	vector<_QunYingMember>& GetFreeQunYingMember(){return m_vecFreeQunYingMember;}
private:
	string m_name;

	bool   m_bSecondBuy;
	CGood  m_Good;						//卖、修理时托盘上的物品
	vector<CGood > m_GoodList;
	DWORD m_SaleGold;	
	bool   m_bStart;						//当把物品刚放上托盘时，不显示  ???? 金币
	bool   m_bMakePoison;
	bool   m_bChangeLogo;
	bool   m_bIsRepairWindow;          
	bool   m_bExchangeFromQuickNpcWnd; //是否通过NPC图形化窗口来交易的
	DWORD m_nNpcID;
	vector<_StorageData>		    m_StorageData;     // NPC二级出售物品列表
	vector<_SellData>				m_vecSellData;     // NPC一级出售物品列表
	vector<_SellData>               m_vecPTData; 
	vector<CGood>                   m_vecQuickData;    // NPC图形化购物的物品列表,也用于仓库物品
	SVipTradGood                    m_VipTradeGood; // vip交易行物品数据
	DWORD						    m_SaleObjectValue; // 物品在出售时的价值，0xffffffff表示不要显示,0表示不适合出售
	int m_iSecBuyStart;
	WORD m_wStorageMaxCount; //仓库的最大容量

	string							m_sTrayTitle;
	DWORD m_dwForgeExp;
   
	CGood m_EquLvlUpGood[MAX_COMMON_GOOD_COUNT];                //黑暗残留换取装备升级物品
	CGood m_OreGood[MAX_ORE_COUNT];      //小宝石合成的时候 放置的矿石列表(第一个位置是幸运符的位置)
	CGood m_GemGood[MAX_COMMON_GOOD_COUNT];     //宝石凝练或合成时物品列表
	bool  m_bIsCondensing;              //正在凝练或合成宝石
	bool  m_bIsLUpintEqu;               //正在升级武器
	int   m_iCondenseFrame;             //凝练/合成物品帧
	int   m_iLUpFrame;                  //武器升级帧

	DWORD m_dwPackGoodID;               //打包物品ID  比如乾坤药袋装物品的时候保存所装物品的ID

	DWORD m_dwSuoPackGoodId;			//乾坤锁锁定物品ID
	
	VString   m_vecStrData;     //显示在NPC List窗口中的文字内容
	string      m_strTitleData;   //NPC List窗口的标题栏

	string      m_strCommand;      // 最后一条命令
	VString   m_vecLeavWords; //NPC留言信息

	_TreasureData    m_BoxGood[MAX_TOTAL_BOX_NUM];
	int m_iBoxType;
	int m_iBoxOpenState;// -1 箱子未开启状态 0 箱子开启状态 1 箱子请求状态 2请求获得物品并关闭窗口
	int m_iBoxOpenCycle;//开宝箱的当前回合数

	CGood m_SubmitGood;//通过SubmitWnd提交的物品
	string m_strSubmitGoodMsg1,m_strSubmitGoodMsg2;//提交物品时显示的信息，m_strSubmitGoodMsg1为提示信息，m_strSubmitGoodMsg２为：请放入。。。
    int m_iSubmitGoodType;//提交物品的类型，点击确定全根据这个来发做不同的事

	vector<_ACTIVITY>	m_vecActData;//每周任务列表

	VTreeGift			m_VecTreeGift[3];
	BYTE				m_byPrayTreeLevel;
	string				m_strPrize;
	CGood				m_PrayCharm;

	_WuXingResult m_stWuxingResult;
    CGood m_DamageGood;     //使用五色晶石修复的物品
    _PetMerge m_iPetMergeData;	
	_ZhenBaoState m_ZhenBaoState;  //珍宝阁状态
	_ShenGongFangGoods m_ShenGongFangGoods;		//神工坊炼制涉及到的物品
	_YUANSHI_RESULT m_YuanShiResult;	
	_DRILL_RESULT m_DrillResult;
	MOperateGood	 m_mOperateGood;
	CGood m_GoodChar;  //天佑中州字符道具
	CGood m_GoodChange; //天佑中州变换乾坤道具

	CGood m_GoodZhenPu; // 阵谱淬炼
	_EquipSoulInfo m_EquipSoulInfo; // 器魂

	CGood m_GoodWuSe; // 五色任务

	_BaoJianData m_BaoJian;	

	_YuanBaoAllInfo m_AllYuanBaoInfo;

	_InstanceZonePrize m_InstancePrize;
	_FirmData		m_FirmData;


	// 神佑融合
	CGood m_kBlessGoodLeft;
	CGood m_kBlessGoodRight;
	CGood m_kBlessGoodFu;

	// 装备合成
	CGood m_kECGoodLeft;
	CGood m_kECGoodRight;

	// 装备拆分
	CGood m_EquipChaifenGood;


	CGood m_GoodFaBao[MAX_FABAO_GOOD_COUNT];//第1个为主法宝，第2个为幸运符，第3-5个为辅法宝

	CGood m_GoodFireArt[6][6];	

	//赤金护符
	CGood m_GoodHufu[MAX_HUFU_GOOD_COUNT];

	//骑术非值守
	string m_strSelFriendName;
	CGood m_kGoodHorsemanship;
	HorsemanshipWatchInfo m_kHorsemanshipWatchInfo;
	HorsemanshipWatchInfo m_kOtherHorsemanshipWatchInfo;

// 	vector<_QunYingMember> m_vecSafeQunYingMember;
// 	vector<_QunYingMember> m_vecFreeQunYingMember;
};
extern CNpcData  g_NPC; //NPC数据

#include "NpcData.inl"
