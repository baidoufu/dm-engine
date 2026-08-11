#pragma once

#include "GameDefine.h"
#include "Global/Interface/GraphicInterface.h"

#define  MAX_HOLYWING_PARTS 4

enum GoodState  // Stolen from CVCONST.H in the DIA 2.0 SDK
{
	InOtherPlayer = 1,
	InBoothGrid = 2,	
	InJiPinGrid = 4,
	InTradeGrid = 8,
}; 

//记录物品的位置
struct GoodPos_t
{
	GoodPos_t()
	{
		dwID = 0;
		iPos = -1;
		bDeal = false;
	}
	DWORD dwID;
	int   iPos;
	bool  bDeal;
};
//记录物品的叠加特效属性
struct GoodAddEffect
{
	GoodAddEffect()
	{
		dwStartID = 0;
		iFrams = 0;
		iFramNow = 0;
		iFramsCount = 0;
		RM = RM_ADD2;
		iSpeed = 3;
		iOffX = 0;
		iOffY = 0;
		iSoundID = 0;
		bFront = true;
		bHideGood = false;
	}

	DWORD       dwStartID;//物品特效的起始ID
	int         iFrams;//物品特效的总帧数
	RenderMode  RM;//物品特效的绘制RenderMode
	int iFramsCount;//计数器
	int iFramNow;//当前帧
	int iSpeed;//速度
	int iOffX,iOffY;//绘制时的偏移
	int iSoundID;//第一帧时播放音效的ID
	bool bFront;//绘制在物品上面，否则绘制在物品下面
	bool bHideGood;//绘制特效时是否隐藏物品自身
};


#pragma pack(push,1)
//神巽数据结构,和物品数据对应
typedef struct S_HolyWing
{
	BYTE	byLenOfName;	//物品结构中的名称的长度 0
	char	szName[24];		//物品结构中的名称 1
	BYTE	byStdMode;		//物品结构中的StdMode 25
	BYTE	byShape;		//物品结构中的Shape 26

	BYTE   byHolyWingLevel;//神巽当前等级 27
	BYTE   byHolyWingStrongLevel;//神巽强化等级,也就是镶边类型,镶边颜色根据这个判断 28
	BYTE   byHolyWingAttrType[MAX_HOLYWING_PARTS];//神巽当前等级第i个属性的类型 29 #属性类型 1:生命值 2:魔法值 3:攻击力/魔法攻击/道术攻击 4:防御力 5:魔法命中/命中 6:暴击 7:反弹伤害 8:吸血 9:魔防力 10:躲避
	WORD   wHolyWingAttrValue[MAX_HOLYWING_PARTS];//神巽当前等级第i个属性的属性值 33
	WORD   wHolyWingPartExp[MAX_HOLYWING_PARTS];//神巽每片的经验值 41
	WORD   wHolyWingLevelUpNeedLevel;//神巽当前等级升级所需本体等级 49

	WORD   wHolyWingAddHp;//神巽增加的血量 51
	BYTE   byEmpty;//53 占位用,为了保持dwUniqueID和物品结构在同一位置

	DWORD  dwUniqueID;//物品结构中的dwUniqueID 54

	DWORD  dwHolyWingPartLevelUpExpMax;//神巽每片升级所需要经验值 58

	WORD   wHolyWingAddMp;//神巽增加的魔法值 62
	WORD   wHolyWingAttack;//神巽增加的攻击力 64
	WORD   wHolyWingDefend;//神巽增加的防御力 66
	WORD   wHolyWingMagicHit;//神巽增加的魔法命中 67
	WORD   wHolyWingCruelAttack;//神巽增加的暴击 70
	WORD   wHolyWingReboundHurt;//神巽增加的反弹伤害 72
	WORD   wHolyWingAbsordBlood;//神巽增加的吸血 74
	WORD   wHolyWingMagicDefend;//神巽增加的魔防力 76
	WORD   wHolyWingAttackAvoid;//神巽增加的躲避 78
	WORD   wHolyWingPoFang;//神巽增加的破防 80
	WORD   wHolyWingDiKang;//神巽增加的抵抗 82
	WORD   wHolyWingMagicAvoid;//神巽增加的魔法躲避 84
	BYTE   szExtproperty3[13];//86 占位用,为了保持和物品长度一样


}SHolyWing,*PSHolyWing;

#pragma  pack(pop)


class CGood
{
	friend class CGoodDemonInfo;
public:

	CGood()	{ clear();  }
	CGood(const CGood& r);

	//Set...
	void	SetID(DWORD nID);
	void	SetName(const char * name) ;
	void	SetStdMode(int stdmode);
	void	SetShape(BYTE nShape);
	void	SetWeight(int nWeight);
	void	SetLooks(int nLooks);
	void	SetDura(WORD nDura);
	void	SetDuraMax(WORD nDuraMax);
	void	SetOriDuraMax(WORD nDuraMax);
	void	SetAC(BYTE nAC);
	void	SetAC2(BYTE nAC2);
	void	SetMAC(BYTE nMAC);
	void	SetMAC2(BYTE nMAC2)	;
	void	SetDC(BYTE nDC);
	void	SetDC2(BYTE nDC2);
	void	SetMC(BYTE nMC);
	void	SetMC2(BYTE nMC2);
	void	SetSC(BYTE nSC);
	void	SetSC2(BYTE nSC2);
	void	SetNeed(BYTE nNeed);
	void	SetNeedLevel(BYTE nNeedLevel);
	void	SetPrice(DWORD nPrice);
	void    SetFlag(DWORD flag);
	void	SetFlag1(WORD nFlag1);
	void	SetFlag2(int nFlag2);
	void    SetAttachSkill(BYTE uAttachSkill);
	void    SetAttackSkillLevel(BYTE uSkillLevel);

	void	SetDemondark1(WORD wDemonDark1);
	void	SetDemondark2(WORD wDemonDark2);
	void	SetDemondark3(WORD wDemonDark3);
	void	SetResvEx(const char *);
	void    SetResvEx(int i , BYTE& c);
	void	SetPayType(BYTE pay);

	//1.907商城道具 
	void	SetExternString(string text);

	DWORD	GetID();
	const char * GetName();
	int		GetStdMode();
	BYTE	GetShape();
	BYTE	GetWeight();
	int		GetLooks();	
	WORD	GetDura();
	WORD	GetDuraMax();
	WORD	GetOriDuraMax();
	BYTE	GetAC();
	BYTE	GetAC2();
	BYTE	GetMAC();
	BYTE	GetMAC2();
	BYTE	GetDC()	;
	BYTE	GetDC2();
	BYTE	GetMC();
	BYTE	GetMC2();
	BYTE	GetSC()	;
	BYTE	GetSC2();
	BYTE	GetNeed();
	BYTE	GetNeedLevel();
	DWORD	GetPrice();
	DWORD	GetFlag();
	DWORD   GetFrozenTime();
	WORD	GetFlag1();
	int		GetFlag2();
	BYTE    GetPayType(); //支付方式金币还是元宝
	BYTE    GetLevelUpTimes();//升级了的次数
	BYTE    GetStarCount();//天生极品的星星数

	string	GetExternString();

	//心魔增加的物品属性
	WORD     GetDemonDark1() ;  //黑暗属性1
	WORD     GetDemonDark2() ;  //黑暗属性2
	WORD     GetDemonDark3() ;  //黑暗属性3

	BYTE&       GetResvEx(int i);
	BYTE*       GetResvExBuf();
	BYTE&       GetResvEx3(int i);
	BYTE*       GetResvEx3Buf();

	BYTE        GetAttachSkill();
	BYTE        GetAttachSkillLevel();

	//辅助的物品判断函数
	bool      IsBind();
	bool      IsRecorded();//是否已经记录,用于跨服战记录装备的显示
	bool      IsSafety();//该装备是否已经被保险

    bool		IsDamaged();//是否损坏
    bool		IsValid();//是否失效
    int			GetRarity();		//获得封元印稀有度

	BYTE      GetFateAttr();        //命运石的属性值(pt道具)
	unsigned char      GetMedalAtt();		  //获得勋章属性

	DWORD     GetRecordTime(bool bFront = true);

	void      getBuffer(char * buf,size_t & iSize);
	void      handleBuffer(char * lpBuffer,int iSize);
	void	  clear();
	bool      FromBuffer(const char* buf,bool bByBooth,int iGoodLen);
	void      Assign(CGood& tmp);

	DWORD     GetItemState();
	void      AddItemState(GoodState goodData);
	void      RemoveItemState(GoodState goodData);
	void      SetPos(int pos);
	int       GetPos();
	void      SetFromWnd(int iWnd);
	int       GetFromWnd();
	void      SetToWnd(int iWnd);
	int       GetToWnd();

	GoodAddEffect &GetGoodAddEffect();

	bool IsUpgrading();//武器是否正在升级中

	static  int PKLength();//之前不带长度的协议的物品的长度,以后的协议都会把物品长度带过来,多条物品的协议已经改成了带长度的,单条的协议物品数据都是放在协议最后,所以暂时没改,以后要修改那条协议的时候必须把物品长度带过来

	DWORD     GetFlashTexID();
	void      SetFlashTexID(DWORD dwID);
	DWORD     GetArrowTipWnd();
	void      SetArrowTipWnd(DWORD val);
	int       GetArrowTipMsgID();
	void      SetArrowTipMsgID(int val);

	BYTE     GetSoulLevel();
	void     SetSoulLevel(BYTE val);
	DWORD    GetSoleExp();
	void     SetSoleExp(DWORD val);
	bool     HaveWakeUpSole();//是否魂器觉醒

	//神翼用到的一些属性
	BYTE   GetHolyWingAttrType(int i);//神巽当前等级第i个属性的类型
	WORD   GetHolyWingAttrValue(int i);//神巽当前等级第i个属性的属性值
	WORD   GetHolyWingAttrExp(int i);//神巽当前等级第i个属性的经验值
	int    GetHolyWingLevel();//神巽当前等级, < 0表示没有觉醒神翼
	WORD   GetHolyWingLevelUpNeedLevel();//神巽当前等级升级所需要本体等级
	DWORD  GetHolyWingPartLevelUpExpMax();//神巽每片升级所需要经验值
	int    GetHolyWingStrongLevel();//神巽强化等级,也就是镶边类型,镶边颜色根据这个判断 < 0表示没有觉醒神翼
	WORD   GetHolyWingAddHp();//神巽增加的血量
	WORD   GetHolyWingAddMp();//神巽增加的魔法值
	WORD   GetHolyWingAttack();//神巽增加的攻击力
	WORD   GetHolyWingDefend();//神巽增加的防御力
	WORD   GetHolyWingMagicHit();//神巽增加的魔法命中
	WORD   GetHolyWingCruelAttack();//神巽增加的暴击
	WORD   GetHolyWingReboundHurt();//神巽增加的反弹伤害
	WORD   GetHolyWingAbsordBlood();//神巽增加的吸血
	WORD   GetHolyWingMagicDefend();//神巽增加的魔防力
	WORD   GetHolyWingAttackAvoid();//神巽增加的躲避
	WORD   GetHolyWingPoFang();//神巽增加的破防
	WORD   GetHolyWingDiKang();//神巽增加的抵抗
	WORD   GetHolyWingMagicAvoid();//神巽增加的魔法躲避
	//end 神巽


	//玉兔神佑物品属性
	BYTE   GetBlessAC();	//攻击
	BYTE   GetBlessMC();	//魔法攻击
	BYTE   GetBlessDC();	//道术攻击
	BYTE   GetBlessAddHP();	//生命
	BYTE   GetBlessDefend();//防御
	BYTE   GetBlessMDefend();//魔法防御
	BYTE   GetBlessAttackAvoid();//躲避
	BYTE   GetBlessAttackHit();//命中
	BYTE   GetBlessMagicHit();//魔法命中
	BYTE   GetBlessMagicAvoid();//魔法躲避
	//end 玉兔神佑物品属性


	//纹佩配件物品属性
	static int GetSubWenPeiLooks(int shape, int n);
	int		GetSubWenPeiType();
	//end 纹佩配件物品属性

	//纹佩物品属性
	int   GetWenPeiDLooks();	//底纹looks
	int   GetWenPeiZLooks();	//主纹looks
	int   GetWenPeiFLooks();	//辅纹looks
	int   GetWenPeiBLooks();	//边纹looks	
	int   GetWenPeiSubLooks(int i); // i:0-3 按绘制顺序
	int   GetWenPeiSubCount();  //子部件数量
	int	  GetWenPeiExtAttr(int i);//属性 0：血量 1：全攻击下限 2：全攻击上限 3：全防御上限 4：命中 5：躲避 6：魔法命中 7：魔法躲避 8：暴击 9：反弹伤害 10：吸血 11：抵抗 13：破防
	int	  GetWenPeiGuDian();
	//end 纹佩物品属性

	//紫灵玉佩
	WORD GetAddHP();

	//法宝
	int      GetFaBaoLingGenLevel();//法宝灵根的等级
	int      GetFaBaoLingGenExp();//法宝灵根的当前经验
	int      GetFaBaoLingGenMaxExp();//法宝灵根的当前等级的最大经验
	bool     HaveOpenFaBaoLingGen();//是否开启灵根
	//end法宝

	//赤金护符
	int		GetChiLianBaoji();//暴击
	int		GetChiLianFanTan();//反弹
	int		GetChiLianXiXie();//吸血
	int		GetChiLianJueFang();//抵抗
	int		GetChiLianPoFang();//破防
	//end赤金护符

private:

#pragma pack(push,1)

	typedef struct tagItemPacket
	{
		BYTE	byLenOfName;	//名称的长度 0
		CHAR	szName[14];		//名称 1-13
		BYTE	byStdMode;		//StdMode 25
		BYTE	byShape;		//Shape 26
		BYTE	byWeight;		//Weight 27

		//WORD	wReserved1;		//保留 28
		//BYTE	byUnUsed;		// 30
		//BYTE	byReserved2; //31

		//DWORD   dwFlag;//28
		BYTE byAniCount;
		BYTE wSource;
		BYTE byReserved;
		BYTE byNeedIdentify;
		WORD	wLooks; //32
		WORD	wDuraMax; //34

		BYTE	byAC1; //36
		BYTE	byAC2; //37
		BYTE	byMAC1; //38
		BYTE	byMAC2;//39
		BYTE	byDC1;//40
		BYTE	byDC2;//41
		BYTE	byMC1;//42
		BYTE	byMC2;//43
		BYTE	bySC1;//44
		BYTE	bySC2;//45
		BYTE	byNeed;//46
		BYTE	byNeedLevel;//47		NeedLevel
		
		WORD	wFlags1;//48		//b1 b2
		DWORD	dwPrice;//50		Price

		DWORD	dwUniqueID;//54		//MakeIndex
		WORD	wItemDura;//58
		WORD	wMaxItemDura;//60

		//TDarkProperty 总共16 字节

		WORD	wEvilAttr[3];//		6字节
		BYTE	szReserved[10];//68
		BYTE    byWakeUpLevel; //装备存放觉醒的等级 78
		DWORD   dwTotalExp;  //装备存放经验 79
		//BYTE	szExtproperty3[16];
		BYTE	szExtproperty3[14];

	}ITEMPACKET_EX68;

#pragma  pack(pop)

private:
	//DWORD	    m_nID;
	//string		m_strName; // string m_strName;
	//int			m_stdMode;
	//BYTE	    m_nShape;	// 形状
 //   int  	    m_nWeight;	// 重量
	//int		    m_nLooks;	// 外观
	//DWORD	    m_nDura;	// 持久力
	//DWORD	    m_nDuraMax; // 持久力上限
	//DWORD	    m_nOriDuraMax; // 原有持久力上限
	//BYTE	    m_nAC;		// 
	//BYTE	    m_nAC2;
	//BYTE	    m_nMAC;
	//BYTE	    m_nMAC2;
	//BYTE	    m_nDC;
	//BYTE	    m_nDC2;
	//BYTE	    m_nMC;
	//BYTE	    m_nMC2;
	//BYTE	    m_nSC;
	//BYTE	    m_nSC2;
	//DWORD	    m_nNeed;		// 
	//DWORD	    m_nNeedLevel;	// 需要等级
	//DWORD	    m_nPrice;
	//DWORD	    m_lFlag;
	//int		    m_nFlag1;

	////1.907增加商城道具属性

 //   WORD        m_wDemonDark1;
	//WORD        m_wDemonDark2;
	//WORD        m_wDemonDark3;

	//BYTE        m_cResvEx[10];
	//BYTE        m_bySoulLevel;//魂器等级,0级0经验表示没有激活,0级1经验表示第一级
	//DWORD       m_dwSoleExp;//魂器经验,0级0经验表示没有激活,0级1经验表示第一级

	tagItemPacket m_ItemPacket;

	BYTE        m_nPayType;     // 元宝还是金币
	int	        m_nFlag2;
	string		m_ExternString; //扩展文字
	DWORD       m_dwState;    //物品的状态，可能是摊位中的物品或者其他玩家的物品
	int         m_iPos;      //编号,可能是在此Good所属的GoodArray中的编号(如包括中的物品)，也可能是原来源地的编号(如Usingtemp)
	int         m_iFromWnd;  //来自哪个窗口
	int         m_iToWnd;  //准备放到哪个窗口
	GoodAddEffect  m_AddEffect;//物品特效的起始ID，如鉴定物品,祝福油回到祝福油罐时上面的特效
	DWORD        m_dwFlashTexID;////是否要闪烁,新手帮助有时候要闪烁特定的按钮,为0表示不要闪烁,< 0x0000FFFF表示闪烁的类型,>0x0000FFFF表示闪烁纹理ID,客户端自己的逻辑控制用
	DWORD        m_dwArrowTipWnd;//这个物品对应冒泡所在窗口msg,
	int          m_iArrowTipMsgID;//这个物品对应冒泡的内容,点击后关闭冒泡,根据ID来查找泡泡
};

//物品迭代器
class GoodIterator
{
public:
	virtual bool DoIterator(CGood& tmp){ return false; }
};

//物品数组
class CGoodArray
{
public:
	CGoodArray();
	CGoodArray(int iSize);
	~CGoodArray();

	int    Size(){ return m_iSize; }
	void   Clear();
	void   ReSize(int iSize,bool bFill = false);
	CGood* GetPtr(int i);
	CGood& Get(int i);
	int    FindGoodPos(DWORD id,int iStartPos = 0,int iEndPos = -1);
	CGood* FindGood(DWORD id,int iStartPos = 0,int iEndPos = -1);
	int    FindGoodByName(const char* name,int iStartPos = 0,int iEndPos = -1);
	int    ReverseFindGoodByName(const char* name,int iStartPos = -1,int iEndPos = 0);
	int    FindGoodByStdmode(int iStdmode,int iShape = -1,int iStartPos = 0,int iEndPos = -1);
	int    FindGoodByStdmodeNameLevel(int iStdmode,int iShape = -1,const char* name = NULL,int iStartPos = 0,int iEndPos = -1,int iLevel = -1,int iMaxAC = -1,DWORD dwExceptID = -1,int iMaxDura = -1);
	int    GetNumber(int iStdmode = -1,int iShape = -1,const char* name = NULL,int iStartPos = 0,int iEndPos = -1);//获得符合条件所有物品个数之和
	int    CountGoodDuraByStdmode(int iStdmode,int iShape = -1,int iStartPos = 0,int iEndPos = -1);//获得所有指定物品持久之和
	int    CountGoodDuraByName(const char* name,int iStartPos = 0,int iEndPos = -1);//获得所有指定物品持久之和
	bool   DeleteGood(DWORD id,bool bFill = true);
	void   ApplyGoodPos(GoodPos_t* PosArray,int iNum); //应用位置信息
	int    FindGoodByIterator(GoodIterator& itr);
	CGood* Add(const char* buf,int iGoodLen);
	CGood* Add(CGood& tmp);
	int    GetGoodNumber();
	int    GetBlankNumber();
	bool   BackToArray(CGood& tmp,int pos = -1);
	void   SortGoods(int iStartPos = 0,int iEndPos = -1);
protected:
	CGood* m_GoodArray;
	int    m_iSize;
	static CGood BLANK;
};

enum eGoodFrom		//物品窗口来源
{
    NO_Wnd = 0,		//没窗口来源
    Package_Wnd,	//包裹窗口
    Panel_Wnd,		//面板窗口
    Arm_Wnd,		//装备窗口
    Repair_Wnd,		//修理窗口
    Trade_Wnd,		//交易窗口
    NpcSale_Wnd,	//NPC卖、修理窗口
    Booth_Wnd,      //摊位窗口
	LeaseBooth_Wnd,	//离线摆摊窗口
    Gem_Wnd,        //宝石凝练合成窗口
    Pet_Package_Wnd, //宠物包裹窗口
    Upload_Wnd,      //上传物品的窗口
	NpcQuickSale_Wnd, //图形化买卖窗口
	NpcQuickStorage_Wnd, //图形化仓库
	SplitStone_Wnd,      //符石拆分窗口
	SealBook_Wnd,		//封元册窗口
	LingXiBox_Free_Wnd,		//至尊宝盒窗口免费格子
	LingXiBox_Pay_Wnd,		//至尊宝盒窗口免费格子
	TrusteeshipPackage_Wnd_1,	//托管窗口，左数第一个
	TrusteeshipPackage_Wnd_2,
	TrusteeshipPackage_Wnd_3,
	TrusteeshipPackage_Wnd_4,
	TrusteeshipPackage_Wnd_5,
	VipTrade_Wnd, //VIP交易行窗口
};

struct stGoodFrom
{
    eGoodFrom eFromWnd;
    CGood	DropGood;	//物品类
    int		iWndPos;	//来源窗口的位置

    void operator =(stGoodFrom &GoodFrom) 
    {
        eFromWnd	= GoodFrom.eFromWnd;
        DropGood	= GoodFrom.DropGood;
        iWndPos		= GoodFrom.iWndPos;
    }
};

#include "Good.inl"
