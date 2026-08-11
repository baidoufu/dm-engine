inline const char *  CCharacter::GetGuildTitle()	{ return m_strGuildTitle.c_str(); }
inline void  CCharacter::SetLevel(WORD level){m_tagPlayerProperty.wLevel = level;}
inline WORD	 CCharacter::GetLevel(){return m_tagPlayerProperty.wLevel;}

inline WORD  CCharacter::GetAC()			{ return m_tagPlayerProperty.wAC1; }
inline WORD  CCharacter::GetAC2()			{ return m_tagPlayerProperty.wAC2; }
inline WORD  CCharacter::GetDC()			{ return m_tagPlayerProperty.wDC1; }
inline WORD  CCharacter::GetDC2()			{ return m_tagPlayerProperty.wDC2; }
inline WORD  CCharacter::GetMAC()			{ return m_tagPlayerProperty.wMAC1;}
inline WORD  CCharacter::GetMAC2()			{ return m_tagPlayerProperty.wMAC2;}
inline WORD  CCharacter::GetMC()			{ return m_tagPlayerProperty.wMC1; }
inline WORD  CCharacter::GetMC2()			{ return m_tagPlayerProperty.wMC2; }
inline WORD  CCharacter::GetSC()			{ return m_tagPlayerProperty.wSC1; }
inline WORD  CCharacter::GetSC2()			{ return m_tagPlayerProperty.wSC2; }

inline WORD	 CCharacter::GetHP()	{ return m_tagPlayerProperty.wCurHP;}
inline WORD	 CCharacter::GetHPMax()	{return m_tagPlayerProperty.wMaxHP;}
inline WORD	 CCharacter::GetMP()	{ return m_tagPlayerProperty.wCurMP;}
inline WORD	 CCharacter::GetMPMax()	{ return m_tagPlayerProperty.wMaxMP;}

inline UINT64 CCharacter::GetExp()			{ return m_tagPlayerProperty.i64CurExp; }
inline UINT64 CCharacter::GetLevelUpExp()	{ return m_tagPlayerProperty.i64MaxExp; }
inline WORD  CCharacter::GetPackageWeight(){ return m_tagPlayerProperty.wPackageWeight; }
inline WORD  CCharacter::GetPackageWeightMax(){ return m_tagPlayerProperty.wMaxPackageWeight; }
inline WORD  CCharacter::GetWeight()		{ return m_tagPlayerProperty.byEquipWeight;}
inline WORD  CCharacter::GetWeightMax()		{ return m_tagPlayerProperty.byMaxEquipWeight;}
inline WORD  CCharacter::GetWrist()			{ return m_tagPlayerProperty.byWristWeight;}
inline WORD  CCharacter::GetWristMax()		{ return m_tagPlayerProperty.byMaxWristWeight;}
inline DWORD CCharacter::GetPrecision()		{ return m_nPrecision;}
inline DWORD CCharacter::GetSmartness()		{ return m_nSmartness;}
inline BYTE	 CCharacter::GetMagicDef()		{ return m_MagicDef; }
inline BYTE  CCharacter::GetPoisonDef()     { return m_PoisonDef; }
// 删除
//inline BYTE  CCharacter::GetRestoreLife()	{ return m_tagPlayerProperty.byHPRenew;}
//inline BYTE  CCharacter::GetRestoreMana()	{ return m_tagPlayerProperty.byMPRenew;}
inline WORD	 CCharacter::GetFame()          { return m_wFame; }
inline BYTE	 CCharacter::GetLifeRes()		{ return m_LifeRes;		}
inline BYTE	 CCharacter::GetMagicRes()		{ return m_MagicRes;	}

//人物的基本属性Set...
inline void CCharacter::SetAC(WORD nAC)		{ m_tagPlayerProperty.wAC1 = nAC;	}
inline void CCharacter::SetAC2(WORD nAC2)	{ m_tagPlayerProperty.wAC2 = nAC2;}

inline void CCharacter::SetDC(WORD nDC)		{ m_tagPlayerProperty.wDC1 = nDC;  }
inline void CCharacter::SetDC2(WORD nDC2)	{ m_tagPlayerProperty.wDC2 = nDC2;}

inline void CCharacter::SetMAC(WORD nMAC)	{ m_tagPlayerProperty.wMAC1 = nMAC; }
inline void CCharacter::SetMAC2(WORD nMAC2)	{ m_tagPlayerProperty.wMAC2 = nMAC2;}

inline void CCharacter::SetMC(WORD nMC)		{ m_tagPlayerProperty.wMAC1 = nMC;  }
inline void CCharacter::SetMC2(WORD nMC2)	{ m_tagPlayerProperty.wMAC2 = nMC2;}

inline void CCharacter::SetSC(WORD nSC)		{ m_tagPlayerProperty.wSC1 = nSC;  }
inline void CCharacter::SetSC2(WORD nSC2)	{ m_tagPlayerProperty.wSC2 = nSC2;}

inline void	CCharacter::SetHP(WORD nLife)		{ m_tagPlayerProperty.wCurHP = nLife;}
inline void	CCharacter::SetHPMax(WORD nLifeMax)	{ m_tagPlayerProperty.wMaxHP = nLifeMax;}
inline void	CCharacter::SetMP(WORD nMana)		{ m_tagPlayerProperty.wCurMP = nMana;}
inline void	CCharacter::SetMPMax(WORD nManaMax)	{ m_tagPlayerProperty.wMaxMP = nManaMax;}

inline void CCharacter::SetLevelUpExp(UINT64 nLevelUpExp)	{ m_tagPlayerProperty.i64MaxExp = nLevelUpExp;}
inline void CCharacter::SetPackageWeightMax(WORD nPackageWeightMax){ m_tagPlayerProperty.wMaxPackageWeight = nPackageWeightMax;}
inline void CCharacter::SetWeight(BYTE nWeight)				{ m_tagPlayerProperty.byEquipWeight = nWeight;}
inline void CCharacter::SetWeightMax(BYTE nWeight)			{ m_tagPlayerProperty.byMaxEquipWeight = nWeight;}
inline void CCharacter::SetWrist(BYTE nWrist)				{ m_tagPlayerProperty.byWristWeight = nWrist;}
inline void CCharacter::SetWristMax(BYTE nWrist)			{ m_tagPlayerProperty.byMaxWristWeight = nWrist;}
inline void CCharacter::SetPrecision(DWORD nPrecision)		{ m_nPrecision = nPrecision;}
inline void CCharacter::SetSmartness(DWORD nSmartness)		{ m_nSmartness = nSmartness;}
// 删除
//inline void CCharacter::SetMove(BYTE m)						{ m_tagPlayerProperty.byMove = m; }
//inline void CCharacter::SetRestoreLife(BYTE m)				{ m_tagPlayerProperty.byHPRenew = m;}
//inline void CCharacter::SetRestoreMana(BYTE m)				{ m_tagPlayerProperty.byMPRenew = m;}	

inline void	CCharacter::SetMagicDef(BYTE m)		{ m_MagicDef	= m;	}
inline void CCharacter::SetPoisonDef(BYTE m)	{ m_PoisonDef	= m;	}
inline void CCharacter::SetFame(WORD m)         { m_wFame       = m;    }
inline void CCharacter::SetLifeRes(BYTE m)		{ m_LifeRes		= m;	}
inline void	CCharacter::SetMagicRes(BYTE m)		{ m_MagicRes	= m;	}


//活力值
inline void  CCharacter::SetEnergyMax(int v)    {m_Energy_Max=v;}
inline int   CCharacter::GetEnergyMax()         {return m_Energy_Max;}

inline void CCharacter:: SetEnergy(int v)       {m_Energy=v;}
inline int   CCharacter::GetEnergy()            {return m_Energy;}

inline void CCharacter:: SetEnergyEnable(BOOL v){m_Energy_Enable=v;}
inline BOOL CCharacter:: GetEnergyEnable()      {return m_Energy_Enable;}

//动作
inline SNextAction& CCharacter::GetCNextAction()  { return m_NextAction; }
inline E_NEXT_STATE	CCharacter::HasNextAction()
{
	if(m_pNextBegin != NULL && m_pNextEnd != NULL)
		return ENS_SERVER_ACTION;
	else if(m_NextAction.wAction != ACTION_STAND)
		return ENS_PLAYER_ACTION;
	else
		return ENS_NO_ACTION;
}

inline bool CCharacter::IsWaitServer()
{
	if(m_bWaitServer && GetTickCount() < m_dwStartWaitServer + 5000)//最大延时5秒
		return true;

	return false;
}

inline bool CCharacter::IsSuccess() {  return m_bSuccess; }

inline void CCharacter::SetWaitServer(bool bWait,bool bSuccess)
{
	m_bWaitServer = bWait;
	m_bSuccess    = bSuccess;
	m_dwStartWaitServer = GetTickCount();
}

//行会相关
inline void CCharacter::SetGuildTitle(const char * str){m_strGuildTitle = str;}
inline void  CCharacter::SetGuildLogoID(DWORD dw){ m_dwLogoID = dw;}
inline DWORD CCharacter::GetGuildLogoID()  {return m_dwLogoID;}
inline bool  CCharacter::IsGettingGuildLogo()			{	return 	m_bGettingTex ;}
inline void  CCharacter::SetGettingGuildLogo(bool b)	{	m_bGettingTex =b;}
inline DWORD CCharacter::GetLogoTexTime()			{ return m_dwGettingTexTime; }
inline void  CCharacter::SetLogoTexTime(DWORD dw)	{ m_dwGettingTexTime = dw; }


//元宝和金币,盛大积分..
inline void  CCharacter::SetYuanBao(DWORD n)		{ m_iYuanBao = n; }
inline void  CCharacter::SetCredit(DWORD n)		    { m_iCredit = n; }
inline void  CCharacter::SetCreditMax(DWORD n)		{ m_iCreditMax = n; }
inline void  CCharacter::SetCreditFeeRate(float f)		{ m_fCreditFeeRate = f; }
inline void  CCharacter::SetCreditArrearage(DWORD n)		{ m_dwCreditArrearage = n; }
inline void  CCharacter::SetCreditState(BYTE byState)		{ m_byCreditState = byState; }
inline void  CCharacter::SetBindYuanBao(DWORD n)   { m_tagPlayerProperty.dwBindYuanBao = n; }
inline void  CCharacter::SetBindGold(DWORD n)   { m_tagPlayerProperty.dwBindMoney = n; }

inline DWORD CCharacter::GetYuanBao()				{ return m_iYuanBao; }
inline DWORD CCharacter::GetCredit()		        { return m_iCredit; }
inline DWORD CCharacter::GetCreditMax()		        { return m_iCreditMax; }
inline float  CCharacter::GetCreditFeeRate()		{ return m_fCreditFeeRate; }
inline DWORD  CCharacter::GetCreditArrearage()		{ return m_dwCreditArrearage;}
inline BYTE  CCharacter::GetCreditState()		{ return m_byCreditState; }
inline DWORD CCharacter::GetBindYuanBao()          { return m_tagPlayerProperty.dwBindYuanBao; }
inline DWORD CCharacter::GetBindGold()          { return m_tagPlayerProperty.dwBindMoney; }
inline DWORD CCharacter::GetGold()                 { return m_lGold; }
inline void  CCharacter::SetGold(DWORD lGold)      { m_lGold = lGold; }
inline void  CCharacter::SetSndaMark(DWORD ul){m_iSndaMark = ul;}	//盛大积分
inline DWORD CCharacter::GetSndaMark(){return m_iSndaMark;}
inline DWORD CCharacter::GetBestItemValue()         { return m_dwBestItemValue; }	//极品修练值
inline void  CCharacter::SetBestItemValue(DWORD v)  { m_dwBestItemValue = v;    }

//宠物丛林豹
inline DWORD CCharacter::GetMyPetID(){   return m_dwMyPetID; }
inline void  CCharacter::SetMyPetID(DWORD id){  m_dwMyPetID = id; }
inline int   CCharacter::GetMyPetLevel()      {    return m_iMyPetLevel; }
inline void  CCharacter::SetMyPetLevel(int i) {    m_iMyPetLevel = i;    }
inline __time32_t CCharacter::GetMyPetTime()      {    return m_tMyPetTime;  }
inline void  CCharacter::SetMyPetTime(__time32_t t){   m_tMyPetTime  = t;    }
inline bool  CCharacter::IsMyOldPet()         {    return m_bIsOldPet;   }
inline void  CCharacter::SetMyOldPet(bool b)  {    m_bIsOldPet = b;      } 

//技能
inline CMagicData& CCharacter::GetMagic(int i){ return m_objMagic.Get(i); }
inline CMagicDataArray& CCharacter::MagicArray(){ return m_objMagic; }

//生产技能
inline CMagicData& CCharacter::GetProduceMagic(int i){ return m_objProduceMagic.Get(i); }
inline CMagicData* CCharacter::GetProduceMagicEx(int MagicID){return m_objProduceMagic.FindMagic(MagicID);}
inline CMagicDataArray& CCharacter::ProduceMagicArray(){ return m_objProduceMagic; }

inline void  CCharacter::SetSkillPower(DWORD lSkillPower)   {m_nSkillPower = lSkillPower;    }
inline void  CCharacter::SetSkillPowerMax(WORD lSkillPowerMax){m_tagPlayerProperty.wEnergy = lSkillPowerMax; }
inline DWORD  CCharacter::GetSkillPower()   { return m_nSkillPower;}
inline DWORD  CCharacter::GetSkillPowerMax(){ return m_tagPlayerProperty.wEnergy;}

//包裹
inline CGood& CCharacter::GetPackageGood(int i){ return m_objPackageGood.Get(i); }
inline CGood& CCharacter::GetEquipGood(int i) {	return m_objEquipGood.Get(i); }
inline CGood& CCharacter::GetPetGood(int i) { return m_objPetGood.Get(i); }

inline CGoodArray& CCharacter::PackageGood()	{ return m_objPackageGood;}
inline CGoodArray& CCharacter::EquipGood()   { return m_objEquipGood; }
inline CGoodArray& CCharacter::PetGood()     { return m_objPetGood;   }

inline CGoodArray& CCharacter::GetUsingTemp()      { return m_objUsingTemp;  }
inline CGoodArray& CCharacter::GetPetTemp()        { return m_objPetTemp;    }
inline CGoodArray& CCharacter::GetDropTemp()       { return m_objDropTemp;  }
inline CGoodArray& CCharacter::GetDevGuildGood()	{ return m_objDevGuildGood;}
inline CGoodArray& CCharacter::GetTrusteeTemp()		{ return m_objTrusteeTemp;}

inline void CCharacter::ResizePackageCapacity(int iOld){	m_objPackageGood.ReSize(MAX_PACKAGE_ELEMENT);}
inline void CCharacter::ResizePetPackageCapacity(int iOld){	m_objPetGood.ReSize(MAX_PET_PACKAGE);}
//应用位置信息
inline void CCharacter::ApplyPackageGoodPos(GoodPos_t* PosArray,int iNum){m_objPackageGood.ApplyGoodPos(PosArray,iNum);}
inline CGood& CCharacter::GetEquipTemp(){ return m_objEquipTemp; }
inline int   CCharacter::GetEquipTempPos(){ return m_iEquipTempPos; }
inline void  CCharacter::SetEquipTempPos(int pos){ m_iEquipTempPos = pos; }
inline BYTE CCharacter::GetCharPhase(){return m_bPhases;}
inline void CCharacter::SetCharPhase(BYTE b){m_bPhases = b;}

inline void	CCharacter::SetLingLi(WORD dwLingLi) {m_tagPlayerProperty.wManito = dwLingLi;}
inline DWORD CCharacter::GetLingLi() {return m_tagPlayerProperty.wManito;}
inline BYTE	CCharacter::GetLuckyValue()	{return m_byLucky;}
inline void	CCharacter::SetLuckyValue(BYTE b) { m_byLucky = b;}
inline BYTE	CCharacter::GetCurseValue()	{return m_byCurse;}
inline void	CCharacter::SetCurseValue(BYTE b) {m_byCurse = b;}


inline WORD   CCharacter::GetJingLi(){return m_tagPlayerProperty.wStamina;}
inline void   CCharacter::SetJingLi(WORD wJingLi){m_tagPlayerProperty.wStamina = wJingLi;}
inline WORD   CCharacter::GetJingLiMax(){return m_tagPlayerProperty.wMaxStamina;}
inline void   CCharacter::SetJingLiMax(WORD wJingLiMax){m_tagPlayerProperty.wMaxStamina = wJingLiMax;}
inline BYTE   CCharacter::GetVipCardType() { return m_tagPlayerProperty.byVIPState; }
inline void   CCharacter::SetVipCardType(BYTE val) { m_tagPlayerProperty.byVIPState = val; }
inline BYTE   CCharacter::GetVipTradeLevel() { return m_tagPlayerProperty.byVipTradeLevel; }
inline void   CCharacter::SetVipTradeLevel(BYTE val) { m_tagPlayerProperty.byVipTradeLevel = val; }
inline void CCharacter::SetExp(UINT64 nExp){m_tagPlayerProperty.i64CurExp = nExp;}
inline void CCharacter::SetPackageWeight(WORD nPackageWeight){m_tagPlayerProperty.wPackageWeight = nPackageWeight;}


inline void    CCharacter::SetJinAttack(WORD wJin){m_wJinAttack = wJin;}
inline void    CCharacter::SetMuAttack(WORD wMu){m_wMuAttack = wMu;}
inline void    CCharacter::SetTuAttack(WORD wTu){m_wTuAttack = wTu;}
inline void    CCharacter::SetShuiAttack(WORD wShui){m_wShuiAttack = wShui;}
inline void    CCharacter::SetHuoAttack(WORD wHuo){m_wHuoAttack = wHuo;}

inline void    CCharacter::SetJinAttack_Low(WORD wJin){m_wJinAttack_Low = wJin;}
inline void    CCharacter::SetMuAttack_Low(WORD wMu){m_wMuAttack_Low = wMu;}
inline void    CCharacter::SetTuAttack_Low(WORD wTu){m_wTuAttack_Low = wTu;}
inline void    CCharacter::SetShuiAttack_Low(WORD wShui){m_wShuiAttack_Low = wShui;}
inline void    CCharacter::SetHuoAttack_Low(WORD wHuo){m_wHuoAttack_Low = wHuo;}


inline void    CCharacter::SetCruelAttack(WORD val) { m_wCruelAttack = val; }
inline void    CCharacter::SetAbsordBlood(WORD val) { m_wAbsordBlood = val; }
inline void    CCharacter::SetReboundHurt(WORD val) { m_wReboundHurt = val; }
inline void    CCharacter::SetAbsDefend(WORD val) { m_wAbsDefend = val; }
inline void    CCharacter::SetDestroyDefend(WORD val) { m_wDestroyDefend = val; }
inline void    CCharacter::SetBloody(BYTE val) { m_byBloody = val; }

inline WORD    CCharacter::GetJinAttack(){return m_wJinAttack;}
inline WORD    CCharacter::GetMuAttack(){return m_wMuAttack;}
inline WORD    CCharacter::GetTuAttack(){return m_wTuAttack;}
inline WORD    CCharacter::GetShuiAttack(){return m_wShuiAttack;}
inline WORD    CCharacter::GetHuoAttack(){return m_wHuoAttack;}

inline WORD    CCharacter::GetJinAttack_Low(){return m_wJinAttack_Low;}
inline WORD    CCharacter::GetMuAttack_Low(){return m_wMuAttack_Low;}
inline WORD    CCharacter::GetTuAttack_Low(){return m_wTuAttack_Low;}
inline WORD    CCharacter::GetShuiAttack_Low(){return m_wShuiAttack_Low;}
inline WORD    CCharacter::GetHuoAttack_Low(){return m_wHuoAttack_Low;}

inline WORD    CCharacter::GetCruelAttack() { return m_wCruelAttack; }
inline WORD    CCharacter::GetAbsordBlood() { return m_wAbsordBlood; }
inline WORD    CCharacter::GetReboundHurt() { return m_wReboundHurt; }
inline WORD    CCharacter::GetAbsDefend() { return m_wAbsDefend; }
inline WORD    CCharacter::GetDestroyDefend() { return m_wDestroyDefend; }
inline BYTE    CCharacter::GetBloody() { return m_byBloody; }

inline void    CCharacter::SetJinDef(BYTE byJin){m_byJinDefend = byJin;}
inline void    CCharacter::SetMuDef(BYTE byMu){m_byMuDefend = byMu;}
inline void    CCharacter::SetTuDef(BYTE byTu){m_byTuDefend = byTu;}
inline void    CCharacter::SetShuiDef(BYTE byShui){m_byShuiDefend = byShui;}
inline void    CCharacter::SetHuoDef(BYTE byHuo){m_byHuoDefend = byHuo;}

inline BYTE    CCharacter::GetJinDef(){return m_byJinDefend;}
inline BYTE    CCharacter::GetMuDef(){return m_byMuDefend;}
inline BYTE    CCharacter::GetTuDef(){return m_byTuDefend;}
inline BYTE    CCharacter::GetShuiDef(){return m_byShuiDefend;}
inline BYTE    CCharacter::GetHuoDef(){return m_byHuoDefend;}

inline bool    CCharacter::IsHaveOnlinePrize() { return m_bHaveOnlinePrize; }
inline void    CCharacter::SetHaveOnlinePrize(bool val) { m_bHaveOnlinePrize = val; }

// inline void    CCharacter::SetFreeTrusteeship(BYTE val) { m_tagPlayerProperty.byFreeTrustee = val;}
// inline void    CCharacter::SetAreaTrusteeship(BYTE val) { m_tagPlayerProperty.bySectionTrustee = val;}
// inline bool    CCharacter::IsFreeTrusteeship(){ return (m_tagPlayerProperty.byFreeTrustee != 0);}
// inline bool    CCharacter::IsAreaTrusteeship(){ return (m_tagPlayerProperty.bySectionTrustee != 0);}

inline CAIMgr &CCharacter::AiMgr(){ return m_AiMgr; }
inline DWORD  *CCharacter::GetOppBlock(){return m_bBlock;}
inline CMapArray& CCharacter::GetMapArray(){ return m_MapData;}
inline CMapPathFinderClient& CCharacter::GetMapPathFinder(){return m_MapPathFinder;}

inline void    CCharacter::SetMeritoriousnessLevel(BYTE byLevel){m_byMeritoriousnessLevel = byLevel;}
inline void    CCharacter::SetMeritoriousnessValue(DWORD dwValue){m_dwMeritoriousnessValue = dwValue ;}
inline void    CCharacter::SetMeritoriousnessAddSpeed(DWORD dwValue){m_dwMeritoriousnessAddSpeed = dwValue;}
inline void    CCharacter::SetMeritoriousnessExp(DWORD dwValue){m_dwMeritoriousnessExp = dwValue;}

inline BYTE    CCharacter::GetMeritoriousnessLevel(){return m_byMeritoriousnessLevel;}
inline DWORD   CCharacter::GetMeritoriousnessValue(){return m_dwMeritoriousnessValue;}
inline DWORD   CCharacter::GetMeritoriousnessAddSpeed(){return m_dwMeritoriousnessAddSpeed;}
inline DWORD   CCharacter::GetMeritoriousnessExp(){return m_dwMeritoriousnessExp;}

// inline bool    CCharacter::IsTrustepshipOnlyFriend(){ return m_bTrustepshipOnlyFriend; }
// inline void    CCharacter::SetTrustepshipOnlyFriend(bool val) { m_bTrustepshipOnlyFriend = val; }
// inline int     CCharacter::GetSelfTrusteeshipPos() { return m_iSelfTrusteeshipPos; }
// inline bool    CCharacter::IsMicroControled(){return (m_iSelfTrusteeshipPos >= 0 && m_bMicroControled);}
// inline void    CCharacter::SetMicroControled(bool b){m_bMicroControled = b;}

inline BYTE    CCharacter::GetHolyWingAttrType(int i){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAttrType(i);}
inline WORD    CCharacter::GetHolyWingAttrValue(int i){	return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAttrValue(i);}
inline WORD    CCharacter::GetHolyWingAttrExp(int i){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAttrExp(i);}
inline int     CCharacter::GetHolyWingLevel(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingLevel();}
inline WORD    CCharacter::GetHolyWingLevelUpNeedLevel(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingLevelUpNeedLevel();}
inline int     CCharacter::GetHolyWingStrongLevel(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingStrongLevel();}
inline DWORD   CCharacter::GetHolyWingPartLevelUpExpMax(){ return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingPartLevelUpExpMax(); }
inline WORD    CCharacter::GetHolyWingAddHp(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAddHp();}
inline WORD    CCharacter::GetHolyWingAddMp(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAddMp();}
inline WORD    CCharacter::GetHolyWingAttack(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAttack();}
inline WORD    CCharacter::GetHolyWingDefend(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingDefend();}
inline WORD    CCharacter::GetHolyWingMagicHit(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingMagicHit();}
inline WORD    CCharacter::GetHolyWingCruelAttack(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingCruelAttack();}
inline WORD    CCharacter::GetHolyWingReboundHurt(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingReboundHurt();}
inline WORD    CCharacter::GetHolyWingAbsordBlood(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAbsordBlood();}
inline WORD    CCharacter::GetHolyWingMagicDefend(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingMagicDefend();}
inline WORD    CCharacter::GetHolyWingAttackAvoid(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingAttackAvoid();}
inline WORD	   CCharacter::GetHolyWingPoFang(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingPoFang();}
inline WORD    CCharacter::GetHolyWingDiKang(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingDiKang();}
inline WORD    CCharacter::GetHolyWingMagicAvoid(){return m_objEquipGood.Get(ITEM_POS_WING).GetHolyWingMagicAvoid();}

inline DWORD  CCharacter::GetFullZhenBaoTime(){ return m_dwFullZhenBaoTime; }
inline void   CCharacter::SetFullZhenBaoTime(DWORD val) { m_dwFullZhenBaoTime = val; }
inline DWORD  CCharacter::GetZhenBaoValue(){ return m_tagPlayerProperty.dwZhenBaoValue; }
inline void   CCharacter::SetZhenBaoValue(DWORD val) { m_tagPlayerProperty.dwZhenBaoValue = val; }
inline void   CCharacter::SetHonorValue(DWORD val)	{ m_tagPlayerProperty.dwHonorValue = val; }
inline DWORD  CCharacter::GetHonorValue()			{ return m_tagPlayerProperty.dwHonorValue; }
inline DWORD  CCharacter::GetZhenBaoLevelMaxExp(int i)
{ 
	if(i >= 0 && i < MAX_ZHENBAO_LEVEL) 
		return m_dwZhenBaoLevelMaxExp[i]; 

	return -1;
}
inline void   CCharacter::SetZhenBaoLevelMaxExp(int i,DWORD val){ if(i >= 0 && i < MAX_ZHENBAO_LEVEL) m_dwZhenBaoLevelMaxExp[i] = val;}

inline DWORD CCharacter::GetActiveRate() { return m_tagPlayerProperty.dwActiveRate; }
inline DWORD  CCharacter::GetPrizeBit() { return m_tagPlayerProperty.dwPrizeBit; }
//inline YIHUO_INFO&   CCharacter::GetYIHUOINFO(){ return m_sYIHUOINFO;}
