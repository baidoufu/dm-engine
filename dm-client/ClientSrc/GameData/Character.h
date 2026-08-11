#pragma once

#include "GameData/GameDefine.h"
#include "Good.h"
#include "MagicData.h"
#include "Simplecharacter.h"
#include "GameAI/AIMgr.h"
#include "MapArray.h"
#include "GameAI/PathFinder.h"


#define MAX_ZHENBAO_LEVEL 4

#pragma pack(push,1)
typedef struct tagPlayerProperty42		//TMYOAbility 服务端
{
	WORD	wLevel;
	WORD	wAC1;// 物理防御下限
	WORD	wAC2;// 物理防御上限
	WORD	wMAC1;// 魔法防御下限
	WORD	wMAC2;// 魔法防御下限
	WORD	wDC1;// 物理攻击下限
	WORD	wDC2;// 物理攻击上限
	WORD	wMC1;// 魔法攻击下限
	WORD	wMC2;// 魔法攻击上限
	WORD	wSC1;// 道法攻击下限
	WORD	wSC2;// 道法攻击上限

	WORD	wCurHP;//生命
	WORD	wCurMP;//魔法
	WORD	wMaxHP;//生命最大值
	WORD	wMaxMP;//魔法最大值

	// 添加
	WORD wLevelUpCount;	//当前拥有经验
	WORD wNextLevelUpCount;	//下一级需要的经验

	// 删除 传世不需要
	//BYTE	byMove;// 移动
	//BYTE	byHPRenew;// 体力恢复
	//BYTE	byMPRenew;//魔法恢复
	//BYTE	byReserved;

	INT64	i64CurExp;// 当前经验值
	INT64	i64MaxExp;// 升级所需经验	
	WORD	wPackageWeight;// 包裹重量
	WORD	wMaxPackageWeight;// 包裹重量
	BYTE	byEquipWeight;// 负重
	BYTE	byMaxEquipWeight;// 最大负重
	BYTE	byWristWeight;// 腕力
	BYTE	byMaxWristWeight;// 最大腕力
	WORD	wManito;//灵力值
	WORD	wEnergy;//元气最大值
	//[1.978][飞升版本][飞升属性]
	WORD	wCurGodHP;		//元力(神、魔)
	WORD	wMaxGodHP;		//元力最大值(神、魔)
	BYTE	byGodRenew;		//元力恢复(神、魔)
	BYTE    byGodType;		//天人圣殿分类 (0=无, 3=天人神, 4=天人魔, 5=天仙神, 6=天仙魔, 7=上仙神、8=上仙魔)
	BYTE    byGodLevel;		//天人境界重数
	//[1.979装备分拆]
	BYTE	m_wHUMCQL;		//拆卸等级
	DWORD	m_wHUMCQX;		//拆卸经验
	DWORD	m_wHUMCQM;		//拆卸最大经验
	WORD    wStamina;		//精力值     
	WORD    wMaxStamina;	//最大精力值  
	//[1.979][VIP机制]
	BYTE    byVIPState;		//0:不是vip,1:银卡,2:金卡
	//[绑定金币元宝]
	DWORD   dwBindMoney;	//绑定金币数目	//nBDCount
	DWORD   dwBindYuanBao;	//邦定元宝数目	//nBDGOLD

	// 群英阵谱 unKnow1
	BYTE    bySectionTrustee;//允许区域模式
	BYTE    byFreeTrustee;//允许自由模式

	//m_nHMZ
	DWORD   dwZhenBaoValue;//珍宝值

	//btmemo  10字节 btmemo: array[0..9] of Byte;
	BYTE		btmemo[10];

	//荣誉值
	DWORD	dwHonorValue;//荣誉值		//LXHEXP
	BYTE    byVipTradeLevel;//vip商行等级

	//未知
	DWORD LHGTime;
	//[夺宝1.28][翅膀颜色]
	BYTE    byWingColor;//翅膀颜色

	// 传奇世界 NextLHGExp
	DWORD   dwActiveRate;//活跃度
	//未知
	BYTE bt3;

	DWORD   dwPrizeBit;//奖励领取情况,第一位表示
	//未知
	BYTE btm;

}PLAYERPROPERTY42, *PPLAYERPROPERTY42;
#pragma  pack(pop)

//struct YIHUO_INFO	//异火数据
//{
//	DWORD dwLevel;			//0－3异火等级
//	UINT64 i64CurExp;		//12－19：异火经验
//	UINT64 i64CurMaxExp;		//12－19：当前等级最大异火经验
//	BYTE	AttackAddJin;		//金加成
//	BYTE	AttackAddMu;		//木加成
//	BYTE	AttackAddTu;		//土加成
//	BYTE	AttackAddShui;		//水加成
//	BYTE	AttackAddHuo;		//火加成
//	WORD wLucy;				//24－25：幸运
//	WORD wAttackUpper;		//26－27：攻击上限
//	WORD wAttackLower;		//28－29：攻击下限
//	WORD wHit;				//30－31：命中
//	WORD wCruelAttack;		//32－33：暴击
//	WORD wAbsDefend;		//34－35：无视防御
//	float fQiHeDu;			//36-39:契合度
//	BYTE byShortCut;		//40： 快捷键
//	char strName[64];//后面：名字
//};


class CCharacter : public CSimpleCharacter
{
public:
	CCharacter(void);
	~CCharacter(void);

	void clear();
	void JumpClear();

	//人物的基本属性Get...
	virtual void     SetLevel(WORD level);
	virtual WORD	 GetLevel();

	const char *  GetGuildTitle();	
	const char *  GetGuildOfficer();
	WORD  GetAC();			
	WORD  GetAC2();			
	WORD  GetDC();			
	WORD  GetDC2();		
	WORD  GetMAC();			
	WORD  GetMAC2();		
	WORD  GetMC();				
	WORD  GetMC2();			
	WORD  GetSC();			
	WORD  GetSC2();	

	virtual WORD	GetHP();
	virtual WORD	GetHPMax();
	virtual WORD	GetMP();
	virtual WORD	GetMPMax();
	virtual BYTE    GetVipCardType();
	virtual BYTE    GetVipTradeLevel();

	UINT64 GetExp();			
	UINT64 GetLevelUpExp();	
	WORD  GetPackageWeight();
	WORD  GetPackageWeightMax();
	WORD  GetWeight();
	WORD  GetWeightMax();
	WORD  GetWrist();
	WORD  GetWristMax();
	DWORD GetPrecision();
	DWORD GetSmartness();
    BYTE  GetMagicDef();
    BYTE  GetPoisonDef();
    BYTE  GetRestoreLife();
    BYTE  GetRestoreMana();
	WORD  GetFame();
	BYTE  GetLifeRes();
	BYTE  GetMagicRes();

	//人物的基本属性Set...
	void SetAC(WORD nAC);
	void SetAC2(WORD nAC2);

	void SetDC(WORD nDC);
	void SetDC2(WORD nDC2);

	void SetMAC(WORD nMAC);
	void SetMAC2(WORD nMAC2);

	void SetMC(WORD nMC);
	void SetMC2(WORD nMC2);

	void SetSC(WORD nSC);
	void SetSC2(WORD nSC2);

	virtual void SetHP(WORD nLife);
	virtual void SetHPMax(WORD nLifeMax);
	virtual void SetMP(WORD nMana);
	virtual void SetMPMax(WORD nManaMax);
	virtual void SetVipCardType(BYTE val);
	virtual void SetVipTradeLevel(BYTE val);

	void SetExp(UINT64 nExp);
	void SetLevelUpExp(UINT64 nLevelUpExp);
	void SetPackageWeight(WORD nPackageWeight);
	void SetPackageWeightMax(WORD nPackageWeightMax);
	void SetWeight(BYTE nWeight);
	void SetWeightMax(BYTE nWeight);
	void SetWrist(BYTE nWrist);
	void SetWristMax(BYTE nWrist);
	void SetPrecision(DWORD nPrecision);
	void SetSmartness(DWORD nSmartness);
	void SetMove(BYTE m);
	void SetRestoreLife(BYTE m);
	void SetRestoreMana(BYTE m);	

	void SetMagicDef(BYTE m);
	void SetPoisonDef(BYTE m);
    void SetFame(WORD m);

	void SetLifeRes(BYTE m);
	void SetMagicRes(BYTE m);

	virtual bool FromBuffer(const char* buf,int iLen);

	//活力值
	void  SetEnergyMax(int v) ;
	int   GetEnergyMax() ;

	void  SetEnergy(int v);
	int   GetEnergy()   ;

	void  SetEnergyEnable(BOOL v)  ;
	BOOL  GetEnergyEnable()  ;

	//行会相关
	void SetGuildTitle(const char * str);
	void  SetGuildLogoID(DWORD dw);
	DWORD GetGuildLogoID();
	bool  IsGettingGuildLogo();
	void  SetGettingGuildLogo(bool b);
	DWORD GetLogoTexTime();
	void  SetLogoTexTime(DWORD dw);


	//元宝和金币.盛大积分..
	void  SetYuanBao(DWORD n);
	void  SetCredit(DWORD n);
	void  SetCreditMax(DWORD n);
	void  SetCreditFeeRate(float f);
	void  SetCreditArrearage(DWORD n);
	void  SetCreditState(BYTE byState);
	void  SetBindYuanBao(DWORD n);
	void  SetBindGold(DWORD n);
	DWORD GetYuanBao()	;
	DWORD GetCredit();
	DWORD GetCreditMax();
	float GetCreditFeeRate();
	DWORD GetCreditArrearage();
	BYTE  GetCreditState();
	DWORD GetBindYuanBao() ;
	DWORD GetBindGold() ;
	DWORD GetGold()   ;
	void  SetGold(DWORD lGold)  ;
	void  SetRainbowNum(DWORD ul);
	DWORD GetRainbowNum();
	void  SetSndaMark(DWORD ul);	//盛大积分
	DWORD GetSndaMark();
	DWORD GetBestItemValue() ;	//极品修练值
	void  SetBestItemValue(DWORD v) ;

	//动作
	SNextAction& GetCNextAction();
	bool PopNextAction(SNextAction* pNextAction);
	E_NEXT_STATE HasNextAction();
	void SetWaitServer(bool bWait,bool bSuccess = false);
	bool IsWaitServer();
	bool IsSuccess();
	

	//技能
	CMagicData& GetMagic(int i);
	CMagicDataArray& MagicArray();
	int   FindMagicPos(WORD wMagicID);  //是否学习了某个技能
	CMagicData * FindMagic(WORD wMagicID);  //是否学习了某个技能
	bool  DeleteMagic(WORD wMagicID);
	DWORD  GetMagicColor(WORD wMagicID);

	CMagicData& GetProduceMagic(int i);
	CMagicData* GetProduceMagicEx(int MagicID);
	CMagicDataArray& ProduceMagicArray();

	void  SetSkillPower(DWORD lSkillPower);
	void  SetSkillPowerMax(WORD lSkillPowerMax);
	DWORD  GetSkillPower();
	DWORD  GetSkillPowerMax();

	//包裹
	CGoodArray& PackageGood();
	CGoodArray& EquipGood();
	CGoodArray& PetGood();

	CGood& GetPackageGood(int i);
	CGood& GetEquipGood(int i);
	CGood& GetPetGood(int i);

	CGoodArray& GetUsingTemp(); // 正在使用的物品
	CGoodArray& GetPetTemp();
	CGoodArray& GetDropTemp();
	CGoodArray& GetDevGuildGood();
	CGoodArray& GetTrusteeTemp();

	DWORD  GetNeedMP(WORD wMagicID);
	bool   IsLearnMagic(WORD wMagicID);

    //正在装备或卸载的物品
    CGood& GetEquipTemp();
	int    GetEquipTempPos();
	void   SetEquipTempPos(int pos);
 
	//逻辑的函数
	void   ResizePetPackageCapacity(int iOld);
	void   ResizePackageCapacity(int iOld);
	void   ApplyPackageGoodPos(GoodPos_t* PosArray,int iNum); //应用位置信息
	void   CalDemonAttr();      //计算黑暗属性

	DWORD  GetMyPetID();
	void   SetMyPetID(DWORD id);
	int   GetMyPetLevel();
	void  SetMyPetLevel(int i);
	__time32_t GetMyPetTime();
	void  SetMyPetTime(__time32_t t);
	bool  IsMyOldPet();
	void  SetMyOldPet(bool b);

	//角色所处阶段
	BYTE GetCharPhase();
	void SetCharPhase(BYTE b);

	void	SetLingLi(WORD dwLingLi);
	DWORD	GetLingLi();
	BYTE	GetLuckyValue();
	void	SetLuckyValue(BYTE b);
	BYTE	GetCurseValue();
	void	SetCurseValue(BYTE b);

	void    SetMagicRate(BYTE m){m_byMagicRate = m;}
	BYTE    GetMagicRate(){return m_byMagicRate;}
	void    SetPoisonRate(BYTE m){m_byPoisonRate = m;}
	BYTE    GetPoisonRate(){return m_byPoisonRate;}

	int     GetWearGold();		//穿戴蟠龙金甲及全套金甲首饰,0为穿戴傲天套装，1为穿戴幻天套装，2为穿戴玄天套装,-1为未穿戴

	WORD	GetMagicDelayTime(WORD wMagicID);
	virtual int     CalStepCount(WORD wAction);

	
	WORD    GetJingLi();
	void    SetJingLi(WORD wJingLi);
	WORD    GetJingLiMax();
	void    SetJingLiMax(WORD wJingLiMax);
	//DWORD   GetLianHuaValue();
	//void    SetLianHuaValue(DWORD dwLainHua);
	//DWORD   GetLianHuaMax();
	//void    SetLianHuaMax(DWORD dwLainHuaMax);
	//BYTE    GetLianHuaLevel();
	//void    SetLianHuaLevel(BYTE byLevel);

	void    SetJinAttack(WORD);
	void    SetMuAttack(WORD);
	void    SetTuAttack(WORD);
	void    SetShuiAttack(WORD);
	void    SetHuoAttack(WORD);
	void    SetJinAttack_Low(WORD);
	void    SetMuAttack_Low(WORD);
	void    SetTuAttack_Low(WORD);
	void    SetShuiAttack_Low(WORD);
	void    SetHuoAttack_Low(WORD);

	void    SetCruelAttack(WORD val);
	void    SetAbsordBlood(WORD val);
	void    SetReboundHurt(WORD val);
	void    SetAbsDefend(WORD val);
	void    SetDestroyDefend(WORD val);
	void    SetBloody(BYTE val);

	WORD    GetJinAttack();
	WORD    GetMuAttack();
	WORD    GetTuAttack();
	WORD    GetShuiAttack();
	WORD    GetHuoAttack();
	WORD    GetJinAttack_Low();
	WORD    GetMuAttack_Low();
	WORD    GetTuAttack_Low();
	WORD    GetShuiAttack_Low();
	WORD    GetHuoAttack_Low();


	WORD    GetCruelAttack();
	WORD    GetAbsordBlood();
	WORD    GetReboundHurt();
	WORD    GetAbsDefend();
	WORD    GetDestroyDefend();
	BYTE    GetBloody();

	void    SetJinDef(BYTE byJin);
	void    SetMuDef(BYTE byMu);
	void    SetTuDef(BYTE byTu);
	void    SetShuiDef(BYTE byShui);
	void    SetHuoDef(BYTE byHuo);

	BYTE    GetJinDef();
	BYTE    GetMuDef();
	BYTE    GetTuDef();
	BYTE    GetShuiDef();
	BYTE    GetHuoDef();
	bool    IsHaveOnlinePrize();
	void    SetHaveOnlinePrize(bool val);

	void    SetMeritoriousnessLevel(BYTE byLevel);
	void    SetMeritoriousnessValue(DWORD dwValue);
	void    SetMeritoriousnessAddSpeed(DWORD dwValue);
	void    SetMeritoriousnessExp(DWORD dwValue);

	BYTE    GetMeritoriousnessLevel();
	DWORD   GetMeritoriousnessValue();
	DWORD   GetMeritoriousnessAddSpeed();
	DWORD   GetMeritoriousnessExp();


// 	bool    IsTrustepshipOnlyFriend();
// 	void    SetTrustepshipOnlyFriend(bool val);
// 	int     GetSelfTrusteeshipPos();
// 	void    SetSelfTrusteeshipPos(int val);
// 	bool    IsMicroControled();//自己是否被微操
// 	void    SetMicroControled(bool b);

// 	void   SetFreeTrusteeship(BYTE byFlag);
// 	void   SetAreaTrusteeship(BYTE byFlag);
// 	bool   IsFreeTrusteeship();
// 	bool   IsAreaTrusteeship();


	CAIMgr &AiMgr();
	CMapArray& GetMapArray();
	CMapPathFinderClient& GetMapPathFinder();
	DWORD* GetOppBlock();
	void   ReSetOppBlock(int iPlayerX,int iPlayerY);
	void   CleanOppBlock();
	BOOL   IsOppBlock(int iX,int iY);
	void   SetOppBlock(int iX,int iY,BOOL b);
	BOOL   BlockSwitch(int x0,int y0,BOOL b);
	void   GetOppStartXY(int &iStartX,int &iStartY);

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

	DWORD  GetFullZhenBaoTime();
	void   SetFullZhenBaoTime(DWORD val);
	DWORD  GetZhenBaoValue();
	void   SetZhenBaoValue(DWORD val);
	void   SetHonorValue(DWORD val);
	DWORD  GetHonorValue();
	DWORD  GetZhenBaoLevelMaxExp(int i);
	void   SetZhenBaoLevelMaxExp(int i,DWORD val);


	void   Reset_MicroControl();
	//YIHUO_INFO& GetYIHUOINFO();
	//void SetYiHuoShortCutKey(BYTE key)	{ m_sYIHUOINFO.byShortCut = key; }
	//BYTE GetYiHuoShortCutKey()			{ return m_sYIHUOINFO.byShortCut; }
	//void SetYiHuoShortCutKeyEx(DWORD key)	{ m_dwYiHuoShortCutKeyEx = key; }
	//DWORD GetYiHuoShortCutKeyEx()			{ return m_dwYiHuoShortCutKeyEx; }
	//void SetYiHuoName(const char* name){memset(m_sYIHUOINFO.strName,0,64); memcpy(m_sYIHUOINFO.strName,name,strlen(name));}
	//char* GetYiHuoName()			{ return m_sYIHUOINFO.strName;}

	DWORD GetActiveRate();
	DWORD  GetPrizeBit();


private:

	//主角身上的基本属性，跟SimpleCharacter都有的属性放在CharacterAttr中

	std::string m_strGuildTitle;	    // 

	tagPlayerProperty42 m_tagPlayerProperty;//角色信息





	DWORD			m_nPrecision;	// 精确
	DWORD			m_nSmartness;	// 敏捷

	BYTE            m_LifeRes;		// 生命恢复
	BYTE            m_MagicRes;		// 魔法恢复

	BYTE		    m_MagicDef;    //魔法躲避
	BYTE		    m_PoisonDef;   //中毒躲避
	BYTE		    m_byLucky;		//幸运值
	BYTE		    m_byCurse;		//诅咒值
	BYTE            m_byMagicRate;//魔法命中
	BYTE            m_byPoisonRate;//中毒命中

    WORD		    m_wFame;       //声望
	int             m_Energy;  	 //活力值
	int             m_Energy_Max;  //活力值上限
	BOOL            m_Energy_Enable;//开启活力条

	//动作
	SNextAction     m_NextAction;   //下一步动作
	bool            m_bWaitServer;  //等待服务器响应
	bool            m_bSuccess;     //服务器返回成功
	DWORD           m_dwStartWaitServer; //开始等待服务器返回的时间
	bool			m_bTechSinging; //吟唱状态

	//金币和元宝
	DWORD           m_iYuanBao;       //元宝数目
	DWORD           m_iCredit;        //可用信用额度
	DWORD           m_iCreditMax;     //最大信用额度
	float           m_fCreditFeeRate; //信用交易手续费
	DWORD           m_dwCreditArrearage;//信用欠款
	BYTE            m_byCreditState;//信用开通状态,0:未开通,1:开通没有通过手机验证,2:开通且通过了手机验证
	DWORD   		m_lGold;          //金币数量
	DWORD           m_iSndaMark ;	  //玩家盛大积分
	DWORD           m_dwBestItemValue; //极品修炼值
	//行会相关
	DWORD		    m_dwLogoID;
	bool		    m_bGettingTex;
	DWORD		    m_dwGettingTexTime;
	//人物物品
	CGoodArray	    m_objEquipGood;	// 装备
	CGoodArray	    m_objPackageGood;               // 包裹栏
	CGoodArray      m_objPetGood;                   // 宠物包裹栏

	CGoodArray      m_objUsingTemp;                 // 正在使用的物品
	CGood   		m_objEquipTemp;                 // 将要装备的或是将要取下装备栏的
	int             m_iEquipTempPos;                // 在装备栏的位置
	CGoodArray	    m_objDropTemp;
	CGoodArray      m_objPetTemp;
	CGoodArray      m_objTrusteeTemp;				//托管包裹物品交换临时变量
	CGoodArray		m_objDevGuildGood;				//行会捐献的物品

	//学习的技能
	CMagicDataArray	m_objMagic;
	CMagicDataArray	m_objProduceMagic;//生产技能

	DWORD           m_dwMyPetID;//丛林豹的ID
	int         m_iMyPetLevel; //丛林豹的级别
	__time32_t  m_tMyPetTime;  //丛林豹的喂豹粮时间
	bool        m_bIsOldPet;   //是否是没有升级过的豹子

	DWORD       m_nSkillPower;//元气值

	BYTE		m_bPhases;//角色所处阶段0为新手期，1为真元期，2为结丹期，3为元婴期，4为元神期，5为分神期

	
	
	//DWORD       m_dwLianHua;			//炼化值
	//DWORD       m_dwLianHuaMax;			//最大炼化值
	//BYTE        m_byLianHuaLevel;		//炼化等级

	WORD        m_wJinAttack;			//金攻
	WORD        m_wMuAttack;			//木攻
	WORD        m_wTuAttack;			//土攻
	WORD        m_wShuiAttack;			//水攻
	WORD        m_wHuoAttack;			//火攻

	WORD        m_wJinAttack_Low;			//金攻下限
	WORD        m_wMuAttack_Low;			//木攻下限
	WORD        m_wTuAttack_Low;			//土攻下限
	WORD        m_wShuiAttack_Low;			//水攻下限
	WORD        m_wHuoAttack_Low;			//火攻下限

	WORD        m_wCruelAttack;         //暴击
	WORD        m_wAbsordBlood;         //吸血
	WORD        m_wReboundHurt;         //反弹伤害
	WORD        m_wAbsDefend;           //绝对防御
	WORD        m_wDestroyDefend;       //破防
	BYTE        m_byBloody;             //嗜血 百分数

	BYTE        m_byJinDefend;			//金防
	BYTE        m_byMuDefend;			//木防
	BYTE        m_byTuDefend;			//土防
	BYTE        m_byShuiDefend;		    //水防
	BYTE        m_byHuoDefend;			//火防

	bool        m_bHaveOnlinePrize;     //下次上线是否有在线奖励领取
	BYTE        m_byMeritoriousnessLevel; //当前功勋等级
	DWORD       m_dwMeritoriousnessValue; //功勋值
	DWORD       m_dwMeritoriousnessAddSpeed;//当前等级获取功勋值上限：XXX\小时
	DWORD       m_dwMeritoriousnessExp;//当前可兑换经验

// 	bool        m_bTrustepshipOnlyFriend;//是否只允许好友组自己进入群英模式
// 	int			m_iSelfTrusteeshipPos;	 //角色自己在托管阵谱中的位置m_TneupMembers
// 	bool        m_bMicroControled;  //自己是否被微操
	
	int         m_iOppStartX,m_iOppStartY;//和GetPlayerXY不一样,因为有可能GameManager 先调用SELF.GetPlayerXY(m_iPlayerX,m_iPlayerY);,再调试DealPlayer(),调用DealPlayer的时候有可能会改变PlayerXY,导致绘制的时候取得的就不一样
	DWORD       m_dwFullZhenBaoTime;//满珍贵值效果到什么时候结束
	DWORD       m_dwZhenBaoLevelMaxExp[MAX_ZHENBAO_LEVEL];


	CAIMgr      m_AiMgr;
	CMapArray	m_MapData; //地图数据
	DWORD		m_bBlock[MAPARR_WIDTH*MAPARR_WIDTH];
	CMapPathFinderClient   m_MapPathFinder;

	S_HolyWing  m_HolyWing;
	//YIHUO_INFO  m_sYIHUOINFO;	//异火数据
	DWORD  m_dwYiHuoShortCutKeyEx;   //扩展快捷键
};

#include "Character.inl"

extern CCharacter* g_pChar; 
