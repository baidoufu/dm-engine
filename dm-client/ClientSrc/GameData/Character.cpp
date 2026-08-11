#include "Character.h"
#include "GameDefine.h"
#include "GameData/MagicDefine.h"
#include "GameAI/AIMgr.h"
#include "GameData/GuildData.h"
#include "GameData.h"
#include "BaseClass/Control/Control.h"
#include "GameControl/GameControl.h"
#include "GameAI/AIConfigMgr.h"
#include "GameMap/GameMap.h"

CCharacter* g_pChar = NULL;

CCharacter::CCharacter(void):

m_objPackageGood(MAX_PACKAGE_ELEMENT),
m_objEquipGood(MAX_EQUIPMENT),
m_objPetGood(MAX_PET_PACKAGE),
m_objMagic(MAX_MAGIC_SKILL),
m_objProduceMagic(MAX_PRODUCEMAGIC_SKILL),
m_objUsingTemp(20),
m_objPetTemp(1),
m_objDropTemp(3),
m_objTrusteeTemp(1),
m_objDevGuildGood(3)
{
	clear();//清数据
	m_MapPathFinder.SetMaxSearch(20000);	//辅助地图寻路程序
	m_MapPathFinder.SetSearchDir(0);
	for (int i = 0; i < MAX_ZHENBAO_LEVEL; i++)
	{
		m_dwZhenBaoLevelMaxExp[i] = 10000000;
	}
}

CCharacter::~CCharacter(void)
{
}


bool CCharacter::PopNextAction(SNextAction* pNextAction)
{
	if(g_AIMgr.NeedFailDelay())		// fail延时
		return false;

	if(PopSNextAction(pNextAction))
		return true;

	memcpy(pNextAction,&m_NextAction,sizeof(SNextAction));
	return true;
}

bool CCharacter::FromBuffer(const char* buf,int iLen)
{
	// 传世角色信息 SC_PLAYER_ATTRIBUTE 封包
	//_LPTDEFAULTMESSAGE	tdm2 = (_LPTDEFAULTMESSAGE)((char*)buf);	//正确.只有头结构
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)buf;	//正确,全部

	SetGold(lpPacketMsg->stDefMsg.nRecog); // 设置金币数量
	SetCareer(lpPacketMsg->stDefMsg.wParam); // 设置角色职业

	//新的结构
	//fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), (char*)&m_xMyHero.m_stAbility, sizeof(ACTORABILITY));
	//fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&m_tagPlayerProperty, sizeof(tagPlayerProperty42));
	//fnDecode6BitBuf(((char*)buf + DEFBLOCKSIZE), (char*)&m_tagPlayerProperty, sizeof(tagPlayerProperty42));
	//fnDecode6BitBuf(((char*)buf + 12), (char*)&m_tagPlayerProperty, sizeof(tagPlayerProperty42));

	//int m_nGlod	= tdm.nRecog;
	//int m_bJob	= tdm.wParam;

	//[协议要修改]
	//m_iMerCount = buf[8];
	//SetSex(buf[8]);								//wTag	性别
	//SetSex(lpPacketMsg->stDefMsg.nRecog);

	//m_iMerSex = buf[9];
	//m_iMerOwner = buf[7];

	//SetGold(Conv_DWORD(buf));		//nRecog	4
	//SetCareer(buf[6]);							//wParam	= m_bJob7-8 职业

	//SetLevel(Conv_WORD(buf+12));
	//SetAC(Conv_WORD(buf + 14));
	//SetAC2(Conv_WORD(buf + 16));
	//SetMAC(Conv_WORD(buf + 18));
	//SetMAC2(Conv_WORD(buf + 20));
	//SetDC(Conv_WORD(buf + 22));
	//SetDC2(Conv_WORD(buf + 24));
	//SetMC(Conv_WORD(buf + 26));
	//SetMC2(Conv_WORD(buf + 28));
	//SetSC(Conv_WORD(buf + 30));
	//SetSC2(Conv_WORD(buf + 32));

	//SetHP(Conv_WORD(buf+34));
	//SetMP(Conv_WORD(buf+36));
	//SetHPMax(Conv_WORD(buf+38));
	//SetMPMax(Conv_WORD(buf+40));

	//SetMove(buf[42]);
	//SetRestoreLife(buf[43]);
	//SetRestoreMana(buf[44]);

	//SetExp(Conv_DWORD(buf+46));
	//SetLevelUpExp(Conv_DWORD(buf+50));

	//SetPackageWeight(Conv_WORD(buf+54));
	//SetPackageWeightMax(Conv_WORD(buf+56));

	//SetWeight((unsigned char)buf[58]);
	//SetWeightMax((unsigned char)buf[59]);

	//SetWrist((unsigned char)buf[60]);
	//SetWristMax((unsigned char)buf[61]);

	//SetLingLi(Conv_WORD(buf + 62));
	//SetSkillPowerMax(Conv_WORD(buf + 64));

	//if(iLen >= 69)
	//{
	//	//[协议要修改]
	//	//SetSpiritPower(Conv_WORD(buf + 66));
	//	//SetSpiritPowerMax(Conv_WORD(buf + 68));
	//	//SetRestoreSpiritPower((BYTE)buf[70]);
	//	//SetFlyType((BYTE)buf[71]);
	//	//SetFlyLevel((BYTE)buf[72]);
	//	//if(m_byFlyType > 0 && m_byFlyLevel > 0 && m_byFlyLevel < 127)
	//	//{
	//	//	m_strMaskName = g_strFlyMask[m_byFlyLevel];
	//	//}
	//}	

	//if(iLen >= 76)
	//{
	//	//SetLianHuaLevel((BYTE)buf[73]);
	//	//SetLianHuaValue(Conv_DWORD(buf + 74));
	//	//SetLianHuaMax(Conv_DWORD(buf + 78));

	//	SetJingLi(Conv_WORD(buf + 73));
	//	SetJingLiMax(Conv_WORD(buf + 75));
	//	SetVipCardType((BYTE)buf[77]);
	//}

	////绑定金币，绑定元宝
	//if(iLen >= 79)
	//{
	//	SetBindGold(Conv_DWORD(buf + 78));
	//	SetBindYuanBao(Conv_DWORD(buf + 82));
	//}

	//if (iLen >= 88)
	//{
	//	if ((BYTE)buf[86])
	//		AddTrusteeshipFlag(0x02);

	//	if ((BYTE)buf[87])
	//		AddTrusteeshipFlag(0x01);
	//}

	//消息体
	int iCopySize = min(iLen - 12,sizeof(tagPlayerProperty42));//兼容老版本gs
	memcpy(&m_tagPlayerProperty,buf + 12,iCopySize);


	////夺宝1.28版添加
	//if (m_tagPlayerProperty.byWingColor > 0)
	//{
	//	SetCharHolyWingStrongLevel( (int)(m_tagPlayerProperty.byWingColor & 0xF) );
	//	SetCharHolyWingLevel( (int)((m_tagPlayerProperty.byWingColor >> 4) & 0xF) );
	//}
	//else
	//{
	//	SetCharHolyWingLevel( -1 );
	//	SetCharHolyWingStrongLevel( -1 );
	//}

	if (m_tagPlayerProperty.wPackageWeight >= m_tagPlayerProperty.wMaxPackageWeight && !g_pGameData->HasPaoPaoStatus(EP_First_PackageFull_Panel_PaoPao))
	{
		if (g_EutUiType == EUT_CLASSIC)
		{
			g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_PackageFull_Panel_PaoPao,730,100,XAL_TOPLEFT,false,XAL_BOTTOMLEFT,0,0,5000);
		} 
		else
		{
			g_pControl->PopupArrowTip(MSG_CTRL_PANEL_WND,EP_First_PackageFull_Panel_PaoPao,800 - 393,185 - 110,XAL_TOPLEFT,false,XAL_BOTTOMLEFT,0,0,5000);
		}
		
	}

	return true;
}

void CCharacter::clear()
{
	//清除基类数据
	CSimpleCharacter::clear();

	memset(&m_tagPlayerProperty,0,sizeof(tagPlayerProperty42));

	m_bGettingTex = false;
	m_lGold = 0; 
	m_nCareer = 0;

	m_nPrecision = 0;
	m_nSmartness = 0;

	m_MagicDef = 0;
	m_PoisonDef = 0;

	m_LifeRes = 0;
	m_MagicRes = 0;

	m_dwBestItemValue = 0;

	m_tagPlayerProperty.i64MaxExp = 1;
	m_tagPlayerProperty.wMaxPackageWeight = 1;
	m_tagPlayerProperty.byMaxEquipWeight = 1;
	m_tagPlayerProperty.byMaxWristWeight = 1;

	m_bWaitServer = false;
    m_bSuccess = false;
	m_dwStartWaitServer = 0;

	m_iYuanBao = 0;
	m_lGold = 0;
	m_iSndaMark = 0;
	m_iCredit = 0;
	m_iCreditMax = 0;
	m_fCreditFeeRate = 0.0f;
	m_dwCreditArrearage = 0;
	m_byCreditState = 0;

	m_objPackageGood.Clear();
	m_objPetGood.Clear();
	m_objEquipGood.Clear();
	m_objUsingTemp.Clear();
	m_objPetTemp.Clear();
	m_objDropTemp.Clear();
	m_objEquipTemp.clear();
	m_objMagic.Clear();
	m_objProduceMagic.Clear();
	m_strGuildTitle.clear();
	m_dwLogoID = 0;

	m_iEquipTempPos = -1;
	m_nSkillPower = 0;
	m_tagPlayerProperty.wEnergy = 100;
	m_iSndaMark = 0;

	m_dwMyPetID = 0;
	m_iMyPetLevel = 0;
	m_tMyPetTime = 0;
	m_bIsOldPet = true;

	m_byLucky = 0;
	m_byCurse = 0;
	
	m_byMagicRate = m_byPoisonRate = 0;

	m_wJinAttack = 0;
	m_wMuAttack = 0;
	m_wTuAttack = 0;
	m_wShuiAttack = 0;
	m_wHuoAttack = 0;

	m_wJinAttack_Low = 0;
	m_wMuAttack_Low = 0;
	m_wTuAttack_Low = 0;
	m_wShuiAttack_Low = 0;
	m_wHuoAttack_Low = 0;

	m_byJinDefend = 0;	
	m_byMuDefend = 0;	
	m_byTuDefend = 0;	
	m_byShuiDefend = 0;	
	m_byHuoDefend = 0;	
	m_wCruelAttack = 0;  
	m_wAbsordBlood = 0;  
	m_wReboundHurt = 0;  
	m_wAbsDefend = 0;    
	m_wDestroyDefend = 0;
	m_byBloody = 0;    

	m_bHaveOnlinePrize = false;
// 	m_byTrusteeshipFlag = 0;
	m_AiMgr.Clear();
	m_MapData.Clear();
	m_MapPathFinder.SetOnRoute(0);

	m_byMeritoriousnessLevel = 0;
	m_dwMeritoriousnessValue = 0 ;
	m_dwMeritoriousnessAddSpeed = 0;
	m_dwMeritoriousnessExp = 0;
// 	m_bTrustepshipOnlyFriend = false;
// 	m_iSelfTrusteeshipPos = -1;
// 	m_bMicroControled = false;

	CleanOppBlock();

	memset(&m_HolyWing,0,sizeof(S_HolyWing));
	//memset(&m_sYIHUOINFO,0,sizeof(YIHUO_INFO));
	m_dwFullZhenBaoTime = 0;
	m_dwYiHuoShortCutKeyEx = 0;

}

WORD CCharacter::GetMagicDelayTime(WORD wMagicID)
{
	CMagicData* pMagic = m_objMagic.FindMagic(wMagicID);
	if(pMagic)
		return pMagic->GetMagicDelay();

	pMagic = m_objProduceMagic.FindMagic(wMagicID);
	if(pMagic)
		return pMagic->GetMagicDelay();

	return 10000;
}

void CCharacter::JumpClear()
{
	m_bDead	= false;
	m_bWait = false;
	m_bDeal = false;
	SetBianShenLooks(0);

	m_pNextBegin = m_pNextEnd = NULL;

	memset(m_dwReserveData,0,plyNums*sizeof(DWORD));
	//清除神佑护身标志
	WORD cExtraState = SELF.GetExtraState(); //原来身上的状态
	cExtraState &= ~ES_SHENYOU;
	SELF.SetExtraState(cExtraState);

	ClearBubbleUp();
	ClearAllNext();
	ClearMagicCase();
	ClrTexFront();
	ClrTexBack();

	m_objEquipGood.Clear();
	m_objUsingTemp.Clear();
	SetTuTengState(0);

    m_bFightOnLeopard = false;
    m_iRunStepOnLeopard = 3;
	//m_wStatus = 0;
	SetWaitServer(false,true);
}

DWORD CCharacter::GetNeedMP(WORD wMagicID)
{
	CMagicData* pMagic = m_objMagic.FindMagic(wMagicID);
	if(pMagic)
		return pMagic->GetNeedManaValue();

	pMagic = m_objProduceMagic.FindMagic(wMagicID);
	if(pMagic)
		return pMagic->GetNeedManaValue();

	return 0;
}

DWORD CCharacter::GetMagicColor(WORD wMagicID)
{
	DWORD dwColor = 0xFFFFFFFF;
	int pos = FindMagicPos(wMagicID);
	if(pos >= 0)
		dwColor = GetMagic(pos).GetColor();

	return dwColor;
}

bool CCharacter::IsLearnMagic(WORD wMagicID)
{
	int pos = m_objMagic.FindMagicPos(wMagicID);
	if(pos >= 0)
	{
		return true;
	}

	pos = m_objProduceMagic.FindMagicPos(wMagicID);
	if(pos >= 0)
	{
		return true;
	}

	return false;
}

int CCharacter::GetWearGold()
{
	if(!(m_Looks.Player.wBody == 19 || m_Looks.Player.wBody == 20))
	{
		return -1;
	}

	if(m_iLevel <= 22)
	{
		return -1;
	}

	for(int i = 3;i <= 10;i++)
	{
		if(m_objEquipGood.Get(i).GetID() == 0)
		{
			return -1;
		}
	}

	for(int i = 0;i < 3;i++)
	{
		//3项链4头盔5 6左右手镯7 8左右戒指9鞋子10腰带
		if(m_objEquipGood.Get(3).GetLooks() ==1201 + i * 6
			&& m_objEquipGood.Get(4).GetLooks() ==1200 + i * 6
			&& m_objEquipGood.Get(5).GetLooks() ==1202 + i * 6
			&& m_objEquipGood.Get(6).GetLooks() ==1202 + i * 6
			&& m_objEquipGood.Get(7).GetLooks() ==1203 + i * 6
			&& m_objEquipGood.Get(8).GetLooks() ==1203 + i * 6
			&& m_objEquipGood.Get(9).GetLooks() ==1205 + i * 6
			&& m_objEquipGood.Get(10).GetLooks() ==1204 + i * 6)
		{
			return i;
		}
	}

	return -1;
}

int CCharacter::CalStepCount(WORD wAction)
{
	// m_tagPlayerProperty.wPackageWeight > m_tagPlayerProperty.wMaxPackageWeight 错误导致不能跑步
	if(m_tagPlayerProperty.wCurHP < 9 || m_tagPlayerProperty.wPackageWeight > m_tagPlayerProperty.wMaxPackageWeight || IsBianShen())
		return 1;

	int iStep = CCharacterAttr::CalStepCount(wAction);
	if((m_dwTuTengState & ETTS_GRAVITY) && iStep > 1)
	{
		iStep -= 1;
	}

	return iStep;
}

const char * CCharacter::GetGuildOfficer()
{
	const char * title = GetGuildTitle();
	std::map<int,CGuildData::sOfficer>& officer = g_GuildData.GetGuildOfficerMap();
	for(std::map<int,CGuildData::sOfficer>::iterator it = officer.begin();it != officer.end();it++)
	{
		if(strcmp(title,it->second.strTile.c_str()) == 0)
		{
			return title;
		}
	}

	return NULL;
}

//包裹查找函数
int CCharacter::FindMagicPos(WORD wMagicID)
{
	int iPos = m_objMagic.FindMagicPos(wMagicID);
	if (iPos == -1)
	{
		iPos = m_objProduceMagic.FindMagicPos(wMagicID);
	}

	return iPos;
}

CMagicData * CCharacter::FindMagic(WORD wMagicID)
{
	CMagicData *pMagic = m_objMagic.FindMagic(wMagicID);
	if (NULL == pMagic)
	{
		pMagic = m_objProduceMagic.FindMagic(wMagicID);
	}

	return pMagic;
}

bool CCharacter::DeleteMagic(WORD wMagicID)
{
	bool bRtn = m_objMagic.DeleteMagic(wMagicID);
	if (false == bRtn)
	{
		bRtn = m_objProduceMagic.DeleteMagic(wMagicID);
	}

	return bRtn;
}

void CCharacter::Reset_MicroControl()
{
	m_iX = m_iRealX;
	m_iY = m_iRealY;
	m_iOffX = m_iOffY = 0;
	m_MapData.ResetAllCharacters();
	ClearAllNext();
	SetWaitServer(false);
	InitAction(ACTION_STAND);
	SetReserveData(plyAttackLockID,0);
	SetReserveData(plyMagicLockID,0);
}

// void    CCharacter::SetSelfTrusteeshipPos(int val)
// {
// 	m_iSelfTrusteeshipPos = val;
// 	if (m_iSelfTrusteeshipPos >= 0 && m_iSelfTrusteeshipPos < MAX_TRUSTEESHIP_NUM)
// 	{
// 		TneupMember &member = g_TrusteeshipData.GetTneupMember(m_iSelfTrusteeshipPos);
// 
// 		string strPath = g_AICfgMgr.GetConfigDir();
// 		strPath += "tgEatCfg.dat";
// 		FILE *fp = fopen(strPath.c_str(),"rb");
// 		if (fp)
// 		{
// 			fread(&(member.eatConfig),1,sizeof(STrusteeshipEatConfig),fp);
// 			fclose(fp);
// 		}
// 		else//没有设置过设一个默认值
// 		{
// 			g_TrusteeshipData.SetDeaultEatConfig(member.eatConfig, m_nCareer,m_nHPMax,m_nMPMax);
// 		}
// 
// 		strPath = g_AICfgMgr.GetConfigDir();
// 		strPath += "tgFightCfg.dat";
// 		fp = fopen(strPath.c_str(),"rb");
// 		if (fp)
// 		{
// 			fread(&(member.fightConfig),1,sizeof(STrusteeshipFightConfig),fp);
// 			fclose(fp);
// 		}
// 	}
// }	

void   CCharacter::GetOppStartXY(int &iStartX,int &iStartY)
{
	iStartX = m_iOppStartX;
	iStartY = m_iOppStartY;
}

//角色阻挡相关信息
void CCharacter::ReSetOppBlock(int iPlayerX,int iPlayerY)
{
	m_iOppStartX = iPlayerX - MAPARR_WIDTH / 2;
	m_iOppStartY = iPlayerY - MAPARR_WIDTH / 2;

	for(int i = 0;i<MAPARR_WIDTH;++i)
	{
		for(int j = 0;j<MAPARR_WIDTH;++j)
		{
			BOOL bBlock = g_pGameMap->IsBlock(m_iOppStartX+i,m_iOppStartY+j);
			m_bBlock[j* MAPARR_WIDTH+i] = bBlock?0x01:0x0;
		}
	}
}

void CCharacter::CleanOppBlock()
{
	memset(m_bBlock,0,MAPARR_WIDTH * MAPARR_WIDTH * 4);
}

void CCharacter::SetOppBlock(int iX,int iY,BOOL b)
{
	int x = iX - m_iOppStartX;
	int y = iY - m_iOppStartY;

	if(x < 0 || y < 0 || x >= MAPARR_WIDTH || y >= MAPARR_WIDTH)
		return;

	m_bBlock[y * MAPARR_WIDTH + x] |= b?0x02:0x0;
}

BOOL CCharacter::IsOppBlock(int iX,int iY)
{
	int x = iX - m_iOppStartX;
	int y = iY - m_iOppStartY;

	if(x < 0 || y < 0 || x >= MAPARR_WIDTH || y >= MAPARR_WIDTH)
		return FALSE;

	return (m_bBlock[y * MAPARR_WIDTH + x] != 0);
}

BOOL CCharacter::BlockSwitch(int x0,int y0,BOOL b)
{
	BOOL bBlock = IsOppBlock(x0,y0);
	SetOppBlock(x0,y0,b);
	return bBlock;
}

