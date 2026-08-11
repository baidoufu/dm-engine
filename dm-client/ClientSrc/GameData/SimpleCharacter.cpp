#include "simplecharacter.h"
#include "Global/StringUtil.h"
#include "GameData.h"
#include "GameData/MagicCtrlMgr.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

CSimpleCharacter::CSimpleCharacter()
{
	clear();
	m_dwPetLeaveWordsTime = 0;
	m_iSilkwormFrame = 0;
}

CSimpleCharacter::~CSimpleCharacter()
{
}

bool CSimpleCharacter::IsCanRideLeopard()
{
	int iRaceNo = GetRaceNo();
	if((iRaceNo >= 166 && iRaceNo <= 168) || (iRaceNo >= 185 && iRaceNo <= 187)
		|| (iRaceNo >= 39 && iRaceNo <= 41) || (iRaceNo >= 301 && iRaceNo <= 303)
		|| (iRaceNo == 373)
		|| (iRaceNo >= 452 && iRaceNo <= 477))
		return true;

	return false;
}

bool CSimpleCharacter::IsLeopard()
{
	int iRaceNo = GetRaceNo();
	if((iRaceNo >= 166 && iRaceNo <= 168) || (iRaceNo >= 185 && iRaceNo <= 187)
		|| iRaceNo == 2 || iRaceNo == 20 || (iRaceNo >= 38 && iRaceNo <= 41) || (iRaceNo >= 300 && iRaceNo <= 303)
		||(iRaceNo == 373)
		|| (iRaceNo >= 452 && iRaceNo <= 477))
		return true;

	return false;
}

//   225 + (appr-161)
bool CSimpleCharacter::IsHuman()
{
	return (GetRaceType() == CHARACTER_HUMAN);
}

// 这里获取的类型全为CHARACTER_MONSTER, 导致NPC显示不出来, 因该是消息接收结构错误
bool CSimpleCharacter::IsNpc()
{
	return (GetRaceType() == CHARACTER_NPC);
}

bool CSimpleCharacter::IsMonster()
{
	return (GetRaceType() == CHARACTER_MONSTER);
}

bool CSimpleCharacter::IsHorse()
{
	int iRaceNo = GetRaceNo();
	if((iRaceNo >= 151 && iRaceNo <= 168) || (iRaceNo >= 185 && iRaceNo <= 187) || (iRaceNo >= 38 && iRaceNo <= 41) || (iRaceNo >= 301 && iRaceNo <= 303) || iRaceNo == 357 || iRaceNo == 359 || iRaceNo == 360)
		return true;

	return false;
}

bool CSimpleCharacter::IsMyPet()	
{
	//if(!IsHorse())
	//	return false;

	char name[64] = {0};
	sprintf(name,"(%s)",SELF.GetName());
	return (strstr(m_strName.c_str(),name) != NULL );
}

bool CSimpleCharacter::IsYourPet(const char * strName)
{
	if( !IsMonster() ) 
		return false;

	char name[64] = {0};
	sprintf(name,"(%s)",strName);
	return ( strstr(m_strName.c_str(),name) != NULL );
}

bool CSimpleCharacter::IsNoNormalAttack()
{
	if(m_Looks.Char.wShape == 115 )
		return true;
	else
		return false;
}

bool CSimpleCharacter::IsPetOfHeartMonster()
{
	if(!IsMonster())
		return false;

	int iRaceNo = GetRaceNo();
	if(iRaceNo == 113 || iRaceNo == 114 || iRaceNo == 117)
		return true;

	return false;
}

bool CSimpleCharacter::IsPet()
{
	if(!IsMonster())
		return false;

	int iPos = m_strName.find('(');
	if(iPos < 0)
		return false;
	return true;
}

bool CSimpleCharacter::FromBuffer(const char* buf,int iLen)
{
	// NPC外观
	LPPACKETMSG lpPacketMsg = (LPPACKETMSG)buf;

	int x = lpPacketMsg->stDefMsg.wParam;
	int y = lpPacketMsg->stDefMsg.wTag;
	SetXY(x,y);
	SetRealXY(x,y);
	SetDir(LOBYTE(lpPacketMsg->stDefMsg.wSeries));
	SetSex(HIBYTE(lpPacketMsg->stDefMsg.wSeries));

	CHARDESC	stCharDesc;
	FEATURE		stFeature;
	memcpy(&stCharDesc, lpPacketMsg->szEncodeData, sizeof(CHARDESC));
	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));

	//SetLooks(Conv_INT64(buf+12));
	SetLooks(stCharDesc.nFeature);

	SetAttackSpeed(Conv_WORD(buf+20));
	SetLevel((BYTE)buf[30]);
	//[协议要修改]
	//SetReserveData(pubHoldDemon,(buf[26] == 101)?1:0);

	SetMonsterArmLevel(buf[27]);
	SetHP(LOWORD(stCharDesc.nHp));
	SetHPMax(HIWORD(stCharDesc.nHp));	

	//int j = 0;
	//string sStr;
	//sStr.assign(lpPacketMsg->szEncodeData+sizeof(CHARDESC), lpPacketMsg->szEncodeData-sizeof(CHARDESC));
	//SetName(StringUtil::toStr(sStr,j).c_str());
	//SetNameColor(StringUtil::toInt(sStr,j,'\0'));

	int iReserveLen = 14;
	//if (g_dwServerVersion >= 1280)//1.28新增加64字节
	//	iReserveLen = 78;

	//name+"/"+color(iColorLen字节)+"\0"(1字节)+其它(iRevLen字节)+职业(1字节)+行会名+"\0";
	// 删除
	int j = 0;
	string str;
	str.assign(buf+28,iLen-28);
	SetName(StringUtil::toStr(str,j).c_str());
	SetNameColor(StringUtil::toInt(str,j,'\0'));

	const BYTE * tempbuf = (const BYTE *)(str.c_str() + j);
	if( iLen - 28 > j + iReserveLen)
	{
		//第1字节表示第二元神等级
		BYTE byTemp = tempbuf[0];
		//SetSecondYuanShen(byTemp);
		//第2字节低2位表示骑了跑几格的豹子
		byTemp = tempbuf[1] & 0x3;
		if(byTemp == 1)	//骑战且豹子跑两格
		{
			m_bFightOnLeopard = true;
			m_iRunStepOnLeopard = 2;
		}
		else if(byTemp == 3)	//骑战且豹子跑三格
		{
			m_bFightOnLeopard = true;
			m_iRunStepOnLeopard = 3;            
		}
		else
		{
			m_bFightOnLeopard = false;
			m_iRunStepOnLeopard = 3;            
		}

		//第2字节低高6bit表示带了什么盾牌,发过来的是盾牌的shape: 战士:1,2,3,4;法师:5,6,7,8;道士:9,10,11,12,虎王盾:13,14,15,16
		byTemp = (tempbuf[1] >> 2) & 0x3F;
		if (byTemp > 0)
		{
			SetShield(byTemp);
		}
		else
		{
			SetShield(0);
		}


// 		//第三字节第一个bit表示是否托管,非零表示已经托管
// 		BYTE byTrusteeshipFlag = ((BYTE)tempbuf[2]) & 0x01;
// 		if (byTrusteeshipFlag > 0)
// 		{
// 			if (g_TrusteeshipData.FindMemberPosByID(GetID()) >= 0)
// 			{
// 				SetTrusteeshipFlag(1);
// 			}
// 			else
// 			{
// 				SetTrusteeshipFlag(2);
// 			}
// 		}
// 		else
// 		{
// 			SetTrusteeshipFlag(0);
// 		}

		SetVipTradeLevel(tempbuf[3]);
		
		SetSanWeiFireLevel(tempbuf[4]);

		//
		//这里是传世飞升相关数据,传奇世界已经不要了
		//

		//神翼等级 + 1,0表示没有神翼
		if (tempbuf[8] == 0)
		{
			SetCharHolyWingLevel(-1);
			//强化等级
			SetCharHolyWingStrongLevel(-1);
		}
		else
		{
			SetCharHolyWingLevel(tempbuf[8] - 1);
			//强化等级
			SetCharHolyWingStrongLevel(tempbuf[9]);

			//夺宝1.28
			//BYTE byWingColor = tempbuf[8];
			//SetCharHolyWingStrongLevel( (int)(byWingColor & 0xF) );
			//SetCharHolyWingLevel( (int)((byWingColor >> 4) & 0xF) );
		}
	
		//
		SetLiveTokenNumber(tempbuf[10]); 
		SetTypeBufferQQ(tempbuf[11]);
		////法宝类型,等级
		SetFaBaoType(tempbuf[12],tempbuf[13]);

		if (iReserveLen > 14)
		{
			BYTE daodun = (BYTE)tempbuf[14];
			if (daodun == 0)
			{
				Set8DunState(false);
				SetDaoZunJiangLinState(false);
			}
			else if (daodun == 1)
			{
				Set8DunState(true);
				SetDaoZunJiangLinState(false);
			}
			else if (daodun == 2)
			{
				Set8DunState(false);
				SetDaoZunJiangLinState(true);
			}

		//	BYTE leftFabao = tempbuf[16];
		//	SetLeftFaBaoType(leftFabao);
		//	BYTE rightFabao = tempbuf[17];
		//	SetRightFaBaoType(rightFabao);

		//	StartFaZhen(tempbuf[15],tempbuf[16],tempbuf[17]);

		//	Set12GongTitle((BYTE)tempbuf[18]);
		}


	}

	if(IsHuman())
	{
		//[协议要修改]
		//第5位低四位表示神魔属性
		//SetFlyType(tempbuf[4] & 0x0F);
		//SetFlyLevel(tempbuf[5]);
		SetGreateMagicState(*((WORD*)(tempbuf + 6)));

		//设置职业
		if(iLen >  28 + j + iReserveLen)//职业位置
			SetCareer(tempbuf[iReserveLen]);
		//设置行会名
		//name+"/"+color(iColorLen字节)+"\0"(1字节)+其它(iRevLen字节)+职业(1字节)+行会名+"\0";
		j = j + iReserveLen + 1;//行会名起始位置
		if(iLen > j + 28)
		{
			SetGuildName(StringUtil::toStr(str,j,'\0').c_str());
		}
	}

	//SetStatus(Conv_WORD(buf+22));//要放在SetGreateMagicState(*((WORD*)(tempbuf + 6)));之后	
	SetStatus(stCharDesc.nStatus);

	InitAction(ACTION_STAND);
	SetOffset(0,0);
	SetDrawOffset(0,0);

	return true;
}

//void CSimpleCharacter::StartFaZhen(BYTE flag,BYTE leftfabao,BYTE rightfabao)
//{
//	if (flag == 1 && !IsUsingFaZhen())
//	{
//		BYTE zhuFabao = GetFaBaoType()->dwCurFaBaoType;
//		UseFaZhen(true);
//
//		if (zhuFabao != 0 && leftfabao !=0 && rightfabao != 0)
//		{
//			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_FABAO_LIANJIE,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,GetID());
//			if (ms)
//			{
//				ms->byRev1 = zhuFabao;
//				ms->wSound = ((WORD)leftfabao) << 8 | ((WORD)rightfabao);
//			}	
//		}
//	}
//	else if(flag == 0 && IsUsingFaZhen())
//	{
//		BYTE zhuFabao = GetFaBaoType()->dwCurFaBaoType;
//		UseFaZhen(false);
//	}
//}