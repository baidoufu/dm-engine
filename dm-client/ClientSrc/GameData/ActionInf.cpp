#include "ActionInf.h"
#include "Global/Interface/TexManagerInterface.h"
#include "Global/StringUtil.h"
#include "GameData/ConfigData.h"
#include "PetData.h"
#include "GameData/CharacterAttr.h"

#define MAX_MONSTER_NUMS  255
#define MONSTER_ACTION_NUM 36
#define WINGS_ACTION_TYPE_NUM 22

#define MAPMONSTERARM(appr,reb,head,armid) m_mapMonsterArm[((DWORD)appr<<16)|((DWORD)reb<<8)|((DWORD)head)]=armid;

CActionInf g_ActionInf;

//---------------------------------------------------------------------------
//动作信息结构
CActionInf::CActionInf(void)
{
	LoadDATFile();
	m_PackageIndex[PICPACKAGE_HUMAN][0] = PACKAGE_human1;		//human1
	m_PackageIndex[PICPACKAGE_HUMAN][1] = PACKAGE_humanex1;		//humanex1
	m_PackageIndex[PICPACKAGE_HUMAN][2] = PACKAGE_humanexn1;	//huamnexn1
	m_PackageIndex[PICPACKAGE_HUMAN][3] = PACKAGE_humanexo1;	//huamnexo1

	m_PackageIndex[PICPACKAGE_HAIR][0] = PACKAGE_hair1;			//hair1
	m_PackageIndex[PICPACKAGE_HAIR][1] = PACKAGE_hairex1;		//hairex1
	m_PackageIndex[PICPACKAGE_HAIR][2] = PACKAGE_hairexn1;		//hairexn1
	m_PackageIndex[PICPACKAGE_HAIR][3] = PACKAGE_hairexo1;		//hairexo1

	m_PackageIndex[PICPACKAGE_PATTERN][0] = PACKAGE_pattern1;	//pattern1
	m_PackageIndex[PICPACKAGE_PATTERN][1] = PACKAGE_patternex1;	//patternex1
	m_PackageIndex[PICPACKAGE_PATTERN][2] = PACKAGE_patternexn1;//patternexn1
	m_PackageIndex[PICPACKAGE_PATTERN][3] = PACKAGE_patternexo1;//patternexo1
	
	m_PackageIndex[PICPACKAGE_MONSTER][0] = PACKAGE_monster1;	//monster1
	m_PackageIndex[PICPACKAGE_MONSTER][1] = PACKAGE_monsterex1;	//monsterex1
	m_PackageIndex[PICPACKAGE_MONSTER][2] = PACKAGE_monsterexn1;//monsterexn1
	m_PackageIndex[PICPACKAGE_MONSTER][3] = PACKAGE_monsterexo1;//monsterexo1

	m_PackageIndex[PICPACKAGE_WEAPON][0] = PACKAGE_weapon1;		//waepon1
	m_PackageIndex[PICPACKAGE_WEAPON][1] = PACKAGE_weaponex1;	//weaponex1
	m_PackageIndex[PICPACKAGE_WEAPON][2] = PACKAGE_weaponexn1;	//weaponexn1
	m_PackageIndex[PICPACKAGE_WEAPON][3] = PACKAGE_weaponexo1;	//weaponexo1

	m_PackageIndex[PICPACKAGE_WEAPONEFF][0] = PACKAGE_weaponeff1;	//weaponeff1
	m_PackageIndex[PICPACKAGE_WEAPONEFF][1] = PACKAGE_weaponeffex1;	//weaponeffex1
	m_PackageIndex[PICPACKAGE_WEAPONEFF][2] = PACKAGE_weaponeffexn1;//weaponeffexn1
	m_PackageIndex[PICPACKAGE_WEAPONEFF][3] = PACKAGE_weaponeffexo1;//weaponeffexo1

	m_PackageIndex[PICPACKAGE_FABAO][0] = PACKAGE_FaBao1;	//weaponeff1
	m_PackageIndex[PICPACKAGE_FABAOEFF][0] = PACKAGE_FaBaoEff1;	//weaponeff1

	m_PackageIndex[PICPACKAGE_MONSTER_ARM][0] = PACKAGE_monsterarm1;
	m_PackageIndex[PICPACKAGE_MONSTER_ARM][1] = PACKAGE_monsterarmex1;
	m_PackageIndex[PICPACKAGE_MONSTER_ARM][2] = PACKAGE_monsterarmexn1;
	m_PackageIndex[PICPACKAGE_MONSTER_ARM][3] = PACKAGE_monsterarmexo1;
	//怪物盔甲

	MAPMONSTERARM(166,MAT_ZHUAN_1,MAT_BODY,0);	//豹子
	MAPMONSTERARM(167,MAT_ZHUAN_1,MAT_BODY,0);
	MAPMONSTERARM(168,MAT_ZHUAN_1,MAT_BODY,0);

	MAPMONSTERARM(166,MAT_ZHUAN_1,MAT_FIRST_BAO,1);	//豹子
	MAPMONSTERARM(167,MAT_ZHUAN_1,MAT_FIRST_BAO,1);
	MAPMONSTERARM(168,MAT_ZHUAN_1,MAT_FIRST_BAO,1);

	MAPMONSTERARM(166,MAT_ZHUAN_2,MAT_BODY,2);
	MAPMONSTERARM(167,MAT_ZHUAN_2,MAT_BODY,2);
	MAPMONSTERARM(168,MAT_ZHUAN_2,MAT_BODY,2);

	MAPMONSTERARM(185,MAT_ZHUAN_1,MAT_BODY,3);	//狮子
	MAPMONSTERARM(186,MAT_ZHUAN_1,MAT_BODY,3);
	MAPMONSTERARM(187,MAT_ZHUAN_1,MAT_BODY,3);

	MAPMONSTERARM(185,MAT_ZHUAN_2,MAT_BODY,4);
	MAPMONSTERARM(186,MAT_ZHUAN_2,MAT_BODY,4);
	MAPMONSTERARM(187,MAT_ZHUAN_2,MAT_BODY,4);

	MAPMONSTERARM(39,MAT_ZHUAN_1,MAT_BODY,5);	//麒麟
	MAPMONSTERARM(40,MAT_ZHUAN_1,MAT_BODY,5);
	MAPMONSTERARM(41,MAT_ZHUAN_1,MAT_BODY,5);

	MAPMONSTERARM(39,MAT_ZHUAN_2,MAT_BODY,6);
	MAPMONSTERARM(40,MAT_ZHUAN_2,MAT_BODY,6);
	MAPMONSTERARM(41,MAT_ZHUAN_2,MAT_BODY,6);

	MAPMONSTERARM(39,MAT_ZHUAN_2,MAT_HEAD,7);//麒麟头部
	MAPMONSTERARM(40,MAT_ZHUAN_2,MAT_HEAD,7);
	MAPMONSTERARM(41,MAT_ZHUAN_2,MAT_HEAD,7);

	//凤凰占4倍,1-无人骑 2-骑乘时,身体 3-左 4-右
	MAPMONSTERARM(301,MAT_ZHUAN_3,MAT_BODY,8);
	MAPMONSTERARM(302,MAT_ZHUAN_3,MAT_BODY,8);
	MAPMONSTERARM(303,MAT_ZHUAN_3,MAT_BODY,8);

	MAPMONSTERARM(301,MAT_ZHUAN_3,MAT_RIDE_BODY,9);
	MAPMONSTERARM(302,MAT_ZHUAN_3,MAT_RIDE_BODY,9);
	MAPMONSTERARM(303,MAT_ZHUAN_3,MAT_RIDE_BODY,9);

	MAPMONSTERARM(301,MAT_ZHUAN_3,MAT_RIDE_LEFT,10);
	MAPMONSTERARM(302,MAT_ZHUAN_3,MAT_RIDE_LEFT,10);
	MAPMONSTERARM(303,MAT_ZHUAN_3,MAT_RIDE_LEFT,10);

	MAPMONSTERARM(301,MAT_ZHUAN_3,MAT_RIDE_RIGHT,11);
	MAPMONSTERARM(302,MAT_ZHUAN_3,MAT_RIDE_RIGHT,11);
	MAPMONSTERARM(303,MAT_ZHUAN_3,MAT_RIDE_RIGHT,11);

	MAPMONSTERARM(166,MAT_ZHUAN_3,MAT_BODY,12);	//豹子三转
	MAPMONSTERARM(167,MAT_ZHUAN_3,MAT_BODY,12);
	MAPMONSTERARM(168,MAT_ZHUAN_3,MAT_BODY,12);

	MAPMONSTERARM(373,MAT_ZHUAN_3,MAT_BODY,13);	//老虎三转

	MAPMONSTERARM(39,MAT_ZHUAN_3,MAT_BODY,14);	//麒麟三转
	MAPMONSTERARM(40,MAT_ZHUAN_3,MAT_BODY,14);
	MAPMONSTERARM(41,MAT_ZHUAN_3,MAT_BODY,14);

	MAPMONSTERARM(39,MAT_ZHUAN_3,MAT_HEAD,15);	//麒麟三转头部
	MAPMONSTERARM(40,MAT_ZHUAN_3,MAT_HEAD,15);
	MAPMONSTERARM(41,MAT_ZHUAN_3,MAT_HEAD,15);

	MAPMONSTERARM(185,MAT_ZHUAN_3,MAT_BODY,16);	//狮子三转
	MAPMONSTERARM(186,MAT_ZHUAN_3,MAT_BODY,16);
	MAPMONSTERARM(187,MAT_ZHUAN_3,MAT_BODY,16);

	MAPMONSTERARM(185,MAT_ZHUAN_3,MAT_HEAD,17);	//狮子三转头部
	MAPMONSTERARM(186,MAT_ZHUAN_3,MAT_HEAD,17);
	MAPMONSTERARM(187,MAT_ZHUAN_3,MAT_HEAD,17);
}

CActionInf::~CActionInf(void)
{
	clear();
}

void CActionInf::clear()
{
	while(!m_MDrawOrder.empty())
	{
		DrawOrderMap::iterator itr = m_MDrawOrder.begin();
		SAFE_DELETE_ARRAY((itr->second).pData);
		m_MDrawOrder.erase(itr);
	}

	m_vMonster.clear();
	m_vPlayer.clear();
	m_Monster.clear();
	m_Wing.clear();
	m_WingEff.clear();
	m_vNpc.clear();
	m_vFaBao.clear();
}

stActionPic* CActionInf::GetActionPic(int iAction,int iRaceType,int iRaceNo,int iType)
{
	int iDrawAction = ConvertAction(iAction,iType);
	ActionMap::iterator itr;

	switch(iRaceType)
	{
	case CHARACTER_MONSTER:
		if(iRaceNo >= m_Monster.size())
			return NULL;

		itr = m_Monster[iRaceNo].find(iDrawAction);
		if(itr == m_Monster[iRaceNo].end())
			return NULL;

		if(itr->second.iFrameMax == 0 || itr->second.iFrameReal == 0)
		{
			itr = m_vMonster.find(iDrawAction);
			if(itr == m_vMonster.end())
				return NULL;
		}

		break;
	case CHARACTER_NPC:
		itr = m_vNpc.find(iDrawAction);
		if(itr == m_vNpc.end())
			return NULL;
		break;
	default:
		itr = m_vPlayer.find(iDrawAction);
		if(itr == m_vPlayer.end())
			return NULL;
		break;
	}
	return &itr->second;
}

//纹理图片
PlayerTex CActionInf::GetPlayerTex(DrawCharInf* pDrawInf)
{
	PlayerTex tex;
	memset(&tex,0,sizeof(PlayerTex));

	WORD wPackage = 0;
	if(pDrawInf == NULL || pDrawInf->pLooks == NULL)
		return tex;

	ActionMap::iterator itr = m_vPlayer.find(pDrawInf->iDrawAction);
	
	if(itr == m_vPlayer.end())
		return tex;

	int iBody  = pDrawInf->wBody;
	//if(iBody == 200)
	//	iBody = 16;

	int iFrame = pDrawInf->iFrame;
	switch(pDrawInf->iAction)
	{
	case ACTION_STAND:
		if(iFrame >= 4) iFrame = 6-iFrame;
		break;
	case ACTION_DEATH:
		if (pDrawInf->iDrawAction == 24 && iFrame > 7)
			iFrame = 0;
		else if(pDrawInf->iDrawAction == 19 && iFrame >= 7)
			iFrame = 7;
		//else if(pDrawInf->iDrawAction != 24 && iFrame > 3)
		//	iFrame = 0;
		break;

	default:
		break;
	}

	int iWeapon = pDrawInf->pLooks->Player.byWeapon;	
	int iHorse = pDrawInf->pLooks->Player.wHorse;

	if(IsPhenix(iHorse))//骑了凤凰,御禽天衣，天袍
	{
		iBody = 24;
	}
	else if (IsLeopard(iHorse))//骑了其它灵兽，豹子，狮子，麒麟，御兽天衣，天袍
	{
		iBody = 16;
	}

	int iHair  = pDrawInf->pLooks->Player.byHairType;
	//黄金甲
	if(iBody == 19 || iBody == 20)
	{
		if(pDrawInf->iSex == 0)
			iHair =  12;
		else
			iHair = 13;
	}

	int iDirOff = pDrawInf->iDir*itr->second.iFrameMax + iFrame;
	int iOffset = itr->second.iStartIndex % 1000 + iDirOff;
	int iIdx = PLAYER_FRAMES * ((iBody*2+pDrawInf->iSex) % 50) + iOffset;


	wPackage = GetPackage(PICPACKAGE_HUMAN,iBody * 2 + pDrawInf->iSex,itr->second.iStartIndex);

	tex.dwClothTex = MAKELONG(iIdx,wPackage);

	// 沙城套装
	bool	bShaChengTao	= (iBody >= 35 && iBody <= 37);
	//圣武天战甲,雷神绝魔袍,九幽灵道袍衣服有特效
	if ( (iBody >= 25 && iBody <= 27) || bShaChengTao)
	{
		//女死亡和男的不一样之外 ,其它的都和男的一样
		if (pDrawInf->iSex == 1 && pDrawInf->iDrawAction != ACTION_DEATH)
		{
			//用男的
			tex.dwClothEffTex = MAKELONG(iIdx - 1000,wPackage + 2000);
		}
		else
		{
			tex.dwClothEffTex = MAKELONG(iIdx,wPackage + 2000);
		}
	}

	if(iBody > 0 && iBody != 19 && iBody != 20 && iBody != 24 && !bShaChengTao) //黄金甲没有pattern 
	{
		wPackage = GetPackage(PICPACKAGE_PATTERN,iBody * 2 + pDrawInf->iSex,itr->second.iStartIndex);
		tex.dwPatternTex = MAKELONG(iIdx,wPackage);
	}

	//御禽天袍,圣武天战甲,雷神绝魔袍,九幽灵道袍衣服上带头盔,不再绘制其它头发
	if(iHair > 0 && (iBody < 24 || iBody > 27))
	{
		if(pDrawInf->iDrawAction >= ACTION_LFSTAND &&  pDrawInf->iDrawAction <=ACTION_LFATTACKED)	
		{
			ActionMap::iterator itrTmp = m_vPlayer.find(pDrawInf->iDrawAction - 5 );
			if(itrTmp == m_vPlayer.end())
				return tex;
			int iDirOffTmp = pDrawInf->iDir*itrTmp->second.iFrameMax + iFrame;
			int iOffsetTmp = itrTmp->second.iStartIndex % 1000 + iDirOffTmp;
			int iIdxTmp = PLAYER_FRAMES*iHair + iOffsetTmp;

			wPackage = GetPackage(PICPACKAGE_HAIR,iHair,itrTmp->second.iStartIndex);
			tex.dwHairTex = MAKELONG(iIdxTmp,wPackage);
		}
		else
		{
			iIdx = PLAYER_FRAMES*iHair + iOffset;
			wPackage = GetPackage(PICPACKAGE_HAIR,iHair,itr->second.iStartIndex);
			tex.dwHairTex = MAKELONG(iIdx,wPackage);
		}
	}

	if ( (iWeapon > 0 || pDrawInf->iPutOnShield/*带有行会盾牌*/) && pDrawInf->iAction != ACTION_CUT && pDrawInf->iAction != ACTION_SALUTE)
	{
		ActionMap::iterator itr_weapon = m_vPlayer.find(pDrawInf->iDrawAction);
		if(itr_weapon != m_vPlayer.end())
		{
			if((pDrawInf->iDrawAction >= ACTION_LFSTAND &&  pDrawInf->iDrawAction <= ACTION_LFJYL) ||
				(pDrawInf->iDrawAction >= ACTION_STAND &&  pDrawInf->iDrawAction <=ACTION_MAGIC) || pDrawInf->iAction == ACTION_DEATH)
			{
				int iDirOff = pDrawInf->iDir*itr_weapon->second.iFrameMax + iFrame;

				if(pDrawInf->iDrawAction == ACTION_DEATH && pDrawInf->iSex == 1)
				{
					iDirOff -= 272;
				}

				int iOffset = itr_weapon->second.iStartIndex % 1000 + iDirOff;					

				if(iWeapon > 0)
				{					
					wPackage = GetPackage(PICPACKAGE_WEAPON,iWeapon,itr_weapon->second.iStartIndex,iBody);
					iIdx = PLAYER_FRAMES*(iWeapon % 60) + iOffset;
					tex.dwWeaponTex = MAKELONG(iIdx,wPackage);
					wPackage = GetPackage(PICPACKAGE_WEAPONEFF,iWeapon,itr_weapon->second.iStartIndex,iBody);
					tex.dwWeaponEffTex = MAKELONG(iIdx,wPackage);
				}

				if(pDrawInf->iPutOnShield && (pDrawInf->iDrawAction < ACTION_SWT || pDrawInf->iDrawAction > ACTION_LFJYL))		//带有行会盾牌//套装技能动作没有盾牌,把盾牌隐掉
				{
					int iShield = 0;
					if(pDrawInf->iPutOnShield == 4)
					{
						iShield = 86 + pDrawInf->iSex;
					}
					else if(pDrawInf->iPutOnShield == 5) //  血月虎王盾
						iShield = 92 - pDrawInf->iSex;
					else
					{
						iShield = 65 + (pDrawInf->iPutOnShield - 1) * 2 + pDrawInf->iSex;
					}

					wPackage = GetPackage(PICPACKAGE_WEAPON,iShield,itr_weapon->second.iStartIndex,iBody);
					iIdx = PLAYER_FRAMES*(iShield % 60) + iOffset;
					tex.dwShieldTex = MAKELONG(iIdx,wPackage);

					int iShieldEff = 71 + pDrawInf->iShieldLevel  * 2 + pDrawInf->iSex;
					wPackage = GetPackage(PICPACKAGE_WEAPON,iShieldEff,itr_weapon->second.iStartIndex,iBody);
					iIdx = PLAYER_FRAMES * (iShieldEff % 60) + iOffset;
					tex.dwShieldEffTex = MAKELONG(iIdx,wPackage);
				}
			}
		}
	}	

	if(iHorse > 0)
	{
		int iRideAction = GetRideAction(pDrawInf->iAction,pDrawInf->iDrawAction);
		//int iMonster = 0;
		int iMonster = iHorse;
		BYTE byReborn = 0;
		//if(iHorse >= 6 && iHorse <= 8)
		//{
		//	iMonster = 160 + iHorse;
		//	byReborn = 1;
		//}
		//else if(iHorse >= 9 && iHorse <= 11)
		//{
		//	iMonster = 166 + iHorse - 9;
		//	byReborn = 2;
		//}
		//else if(iHorse >= 25 && iHorse <= 27)
		//{
		//	iMonster = 185 + iHorse - 25;
		//	byReborn = 1;
		//}
		//else if(iHorse >= 28 && iHorse <= 29)
		//{
		//	iMonster = 185 + iHorse - 28;
		//	byReborn = 2;
		//}
		//else if(iHorse == 24)//二转黑狮子
		//{
		//	iMonster = 185 + 30 - 28;
		//	byReborn = 2;
		//}
		//else if(iHorse >= 14 && iHorse <= 16)
		//{
		//	iMonster = 39 + iHorse - 14;
		//	byReborn = 1;
		//}
		//else if(iHorse >= 17 && iHorse <= 19)
		//{
		//	iMonster = 39 + iHorse - 17;
		//	byReborn = 2;
		//}
		//else if(iHorse >= 30 && iHorse <= 32)
		//{
		//	iMonster = 301 + iHorse - 30;
		//	byReborn = 1;
		//}
		//else if(iHorse >= 33 && iHorse <= 35)
		//{
		//	iMonster = 301 + iHorse - 33;
		//	byReborn = 2;
		//}
		//else if(iHorse > 0) 
		//	iMonster = iHorse + 160;

		if((pDrawInf->iDrawAction >= ACTION_LFSTAND && pDrawInf->iDrawAction <= ACTION_LFMAGICTAO )||
			(pDrawInf->iDrawAction >= ACTION_LFSWT && pDrawInf->iDrawAction <= ACTION_LFJYL)	)
		{
			if (pDrawInf->iDrawAction == ACTION_LFSWT)
			{
				if (iFrame >= 8 && iFrame <= 13)
				{
					iRideAction = ACTION_ADDACTION3;
					iFrame = iFrame - 8;
				}
				else
				{
					iFrame = (iFrame*6/16)%6;//stand 的速度是16,ACTION_LFSWT速度是6

					if(iFrame >= 4) 
					{
						iFrame = 6-iFrame;
					}
				}
			}
			else if (pDrawInf->iDrawAction == ACTION_LFLSJ || pDrawInf->iDrawAction == ACTION_LFJYL)
			{
				iFrame = (iFrame*6/16)%6;//stand 的速度是16,ACTION_LFSWT速度是6

				if(iFrame >= 4) 
				{
					iFrame = 6-iFrame;
				}
			}

			MonsterTex & MTEX = GetMonsterTex(iMonster,iRideAction,iFrame,pDrawInf->iDir,pDrawInf->byArm,byReborn,true);
			tex.dwMShapeTex = MTEX.dwShapeTex;
			tex.dwMArmTex =MTEX.dwArmTex;
			tex.dwMHead = MTEX.dwHead;
			tex.dwMHeadArm = MTEX.dwHeadArm;
			tex.dwMonsterLeft = MTEX.dwMonsterLeft;	
			tex.dwMonsterRight = MTEX.dwMonsterRight;	
			tex.dwMonsterLeftArm = MTEX.dwMonsterLeftArm;
			tex.dwMonsterRightArm = MTEX.dwMonsterRightArm;
		}
		else
		{
			MonsterTex & MTEX = GetMonsterTex(iMonster,iRideAction,iFrame,pDrawInf->iDir,pDrawInf->byArm,byReborn,false);
			tex.dwMShapeTex = MTEX.dwShapeTex;
			tex.dwMArmTex =MTEX.dwArmTex;
			tex.dwMHead = MTEX.dwHead;
			tex.dwMHeadArm = MTEX.dwHeadArm;
			tex.dwMonsterLeft = MTEX.dwMonsterLeft;	
			tex.dwMonsterRight = MTEX.dwMonsterRight;	
			tex.dwMonsterLeftArm = MTEX.dwMonsterLeftArm;
			tex.dwMonsterRightArm = MTEX.dwMonsterRightArm;
		}

	}

	if(pDrawInf->byWing > 0)  //有翅膀
	{
		GetWingsTex(pDrawInf,tex);
	}

	int FaBaoType;
	int ShowFrame;
	int FaBaoState;
	if (pDrawInf->pFaBao && (pDrawInf->pFaBao->iHide == 0) && (pDrawInf->pFaBao->bFaZhen == false ))
	{
		ActionMap::iterator itrFaBao = m_vFaBao.find(pDrawInf->iDrawAction);

		pDrawInf->pFaBao->GetCurShowInfo(FaBaoType,ShowFrame,FaBaoState);
		if (FaBaoType >= 0) //有法宝
		{
			int realFaBaoType = FaBaoType - 1;	//法宝编号从1开始,图片从0开始
			//收起或出现阶段
			if (FaBaoState == 2 && FaBaoType > 0)
			{
				//消失出现同attack
				int iDirOff = pDrawInf->iDir*8 + ShowFrame;
				int iOffset = 305 + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,ShowFrame);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,ShowFrame);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pDrawInf->pFaBao->SetState(2);
			}
			else if ((FaBaoType > 0) && (pDrawInf->iDrawAction == ACTION_WALK 
					|| pDrawInf->iDrawAction == ACTION_RUN
					|| pDrawInf->iDrawAction == ACTION_HWALK
					|| pDrawInf->iDrawAction == ACTION_HRUN
					|| pDrawInf->iDrawAction == ACTION_LWALK
					|| pDrawInf->iDrawAction == ACTION_LRUN
					|| pDrawInf->iDrawAction == ACTION_LFWALK
					|| pDrawInf->iDrawAction == ACTION_LFRUN))//如果在移动
			{
				if (itrFaBao == m_vFaBao.end())
				{
					return tex;
				}
				
				int iDirOff = pDrawInf->iDir*itrFaBao->second.iFrameMax + iFrame *(1+ itrFaBao->second.iInterval);
				int iOffset = itrFaBao->second.iStartIndex + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,itr->second.iStartIndex);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,itr->second.iStartIndex);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pDrawInf->pFaBao->SetState(1);
			}
			else if(FaBaoType > 0)//待机
			{
				itrFaBao = m_vFaBao.find(ACTION_STAND);
				if (itrFaBao == m_vFaBao.end())
				{
					return tex;
				}
				int iDirOff = pDrawInf->iDir*itrFaBao->second.iFrameMax + iFrame;
				int iOffset = itrFaBao->second.iStartIndex % 1000 + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,ShowFrame);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,ShowFrame);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pDrawInf->pFaBao->SetState(0);
			}
			pDrawInf->pFaBao->Update();
		}
	}
	//else if (pDrawInf->pFaBao && (pDrawInf->pFaBao->iHide == 0) && pDrawInf->pFaBao->bFaZhen)
	//{
	//	ActionMap::iterator itrFaBao = m_vFaBao.find(pDrawInf->iDrawAction);

	//	int zhuFaBaoType = pDrawInf->pFaBao->dwCurFaBaoType - 1;
	//	int leftFaBaoType = pDrawInf->pFaBao->dwLeftFaBaoType - 1;
	//	int rightFaBaoType = pDrawInf->pFaBao->dwRightFaBaoType - 1;

	//	pDrawInf->pFaBao->GetCurShowInfo(FaBaoType,ShowFrame,FaBaoState);

	//	if (zhuFaBaoType >= 0 && leftFaBaoType >= 0 && rightFaBaoType >= 0) //三法宝都有
	//	{
	//		if (pDrawInf->iDrawAction == ACTION_WALK 
	//			|| pDrawInf->iDrawAction == ACTION_RUN
	//			|| pDrawInf->iDrawAction == ACTION_HWALK
	//			|| pDrawInf->iDrawAction == ACTION_HRUN
	//			|| pDrawInf->iDrawAction == ACTION_LWALK
	//			|| pDrawInf->iDrawAction == ACTION_LRUN
	//			|| pDrawInf->iDrawAction == ACTION_LFWALK
	//			|| pDrawInf->iDrawAction == ACTION_LFRUN)//如果在移动
	//		{
	//			if (itrFaBao == m_vFaBao.end())
	//			{
	//				return tex;
	//			}
	//			
	//			//主
	//			int iDirOff = pDrawInf->iDir*itrFaBao->second.iFrameMax + iFrame *(1+ itrFaBao->second.iInterval);
	//			int iOffset = itrFaBao->second.iStartIndex + iDirOff;
	//			int iIdx = PLAYER_FRAMES * (zhuFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,zhuFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBao = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,zhuFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);

	//			//左
	//			iDirOff = ((pDrawInf->iDir + 3)%8)*itrFaBao->second.iFrameMax + iFrame *(1+ itrFaBao->second.iInterval);
	//			iOffset = itrFaBao->second.iStartIndex + iDirOff;
	//			iIdx = PLAYER_FRAMES * (leftFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,leftFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBaoLeft = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,leftFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBaoLeftEff = MAKELONG(iIdx,wPackage);

	//			//右
	//			iDirOff = ((pDrawInf->iDir + 5)%8)*itrFaBao->second.iFrameMax + iFrame *(1+ itrFaBao->second.iInterval);
	//			iOffset = itrFaBao->second.iStartIndex + iDirOff;
	//			iIdx = PLAYER_FRAMES * (rightFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,rightFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBaoRight = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,rightFaBaoType,itr->second.iStartIndex);

	//			tex.dwFaBaoRightEff = MAKELONG(iIdx,wPackage);

	//			pDrawInf->pFaBao->SetState(1);
	//		}
	//		else
	//		{
	//			itrFaBao = m_vFaBao.find(ACTION_STAND);
	//			if (itrFaBao == m_vFaBao.end())
	//			{
	//				return tex;
	//			}

	//			//主
	//			int iDirOff = pDrawInf->iDir*itrFaBao->second.iFrameMax + iFrame;
	//			int iOffset = itrFaBao->second.iStartIndex % 1000 + iDirOff;
	//			int iIdx = PLAYER_FRAMES * (zhuFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,zhuFaBaoType,ShowFrame);

	//			tex.dwFaBao = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,zhuFaBaoType,ShowFrame);

	//			tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);

	//			//左
	//			iDirOff = ((pDrawInf->iDir + 3)%8)*itrFaBao->second.iFrameMax + iFrame;
	//			iOffset = itrFaBao->second.iStartIndex % 1000 + iDirOff;
	//			iIdx = PLAYER_FRAMES * (leftFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,leftFaBaoType,ShowFrame);

	//			tex.dwFaBaoLeft = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,leftFaBaoType,ShowFrame);

	//			tex.dwFaBaoLeftEff = MAKELONG(iIdx,wPackage);

	//			//右
	//			iDirOff = ((pDrawInf->iDir + 5)%8)*itrFaBao->second.iFrameMax + iFrame;
	//			iOffset = itrFaBao->second.iStartIndex % 1000 + iDirOff;
	//			iIdx = PLAYER_FRAMES * (rightFaBaoType % 50) + iOffset;


	//			wPackage = GetPackage(PICPACKAGE_FABAO,rightFaBaoType,ShowFrame);

	//			tex.dwFaBaoRight = MAKELONG(iIdx,wPackage);

	//			wPackage = GetPackage(PICPACKAGE_FABAOEFF,rightFaBaoType,ShowFrame);

	//			tex.dwFaBaoRightEff = MAKELONG(iIdx,wPackage);

	//			pDrawInf->pFaBao->SetState(0);
	//		}
	//	}

	//	pDrawInf->pFaBao->Update();
	//}
	

	return tex;
}

MonsterTex CActionInf::GetMonsterTex(DrawCharInf* pDrawInf) //获得怪物图片
{
	int iMonster = pDrawInf->pLooks->Char.wShape;

	if(iMonster == 205)//铜人	的图放到71怪物编号的位置了	
		iMonster = 71;

	//int iAction  = pDrawInf->iAction;
	//if(((iMonster >= 166 && iMonster <= 168) || (iMonster >= 185 && iMonster <= 187)) && pDrawInf->pLooks->Player.byHorse != 0 )
	//	iAction  = pDrawInf->iDrawAction;

	return GetMonsterTex(iMonster,pDrawInf->iDrawAction,pDrawInf->iFrame,pDrawInf->iDir,pDrawInf->byArm,pDrawInf->iSex,false,pDrawInf->pFaBao);  //monster的性别作为转生次数
}

MonsterTex CActionInf::GetMonsterTex(int iRealMonster,int iAction,int iFrame,char cDir,BYTE byArm,BYTE byReborn,bool bFightOnLeopard,FABAOStruct* pFaBao)
{	
	MonsterTex tex;
	memset(&tex,0,sizeof(MonsterTex));

	WORD wPkg = 0;

	int iMonster = GetDrawMonsterAppr(iRealMonster,byReborn);

	if(iMonster > m_Monster.size())
		return tex;

	ActionMap::iterator itr = m_Monster[iMonster].find(iAction);
	if(itr == m_Monster[iMonster].end())
		return tex;

	stActionPic* pPicInfo = &(itr->second);
	if (iMonster == 209 || iMonster == 310 || iMonster == 311 || iMonster == 329 || iMonster == 330 || (iMonster >= 323 && iMonster <= 326))
	{
		cDir = 0;
	}

	if(pPicInfo->iFrameMax == 0)
	{
		ActionMap::iterator itr2 = m_vMonster.find(iAction);
		if(itr2 != m_vMonster.end())
		{
			pPicInfo = &(itr2->second);
		}
	}

	if(pPicInfo->iFrameMax <= 0)
	{
		return tex;
	}
	else if(iFrame >= pPicInfo->iFrameReal)
	{
		iFrame = pPicInfo->iFrameReal - 1;
	}

	if (iMonster >= 256 && iMonster <= 259)//行会怪，方向始终为0
	{
		cDir = 0;
	}
	int iOff = pPicInfo->iStartIndex % 1000 + cDir*pPicInfo->iFrameMax + iFrame;	

	wPkg = GetPackage(PICPACKAGE_MONSTER,iMonster,pPicInfo->iStartIndex);
	
	

	if (iMonster == 249)//无相天魔
		wPkg = PACKAGE_monsterex1;

	int iIdx = MONSTER_FRAMES * (iMonster % 100) + iOff;
	tex.dwShapeTex = MAKELONG(iIdx,wPkg);		
	tex.dwMonsterEff = MAKELONG(iIdx,wPkg + 1000);

	if(iMonster >= 301 && iMonster <= 303)	//凤凰分成三部分
	{
		//部分凤凰的左右部分放在monster8里面
		if(bFightOnLeopard)
		{		
			tex.dwShapeTex = MAKELONG(iOff + ((iMonster - 301) * 3 + 3) * 500,wPkg + 4);
			tex.dwMonsterRight = MAKELONG(iOff + ((iMonster - 301) * 3 + 4) * 500,wPkg + 4);
			tex.dwMonsterLeft = MAKELONG(iOff + ((iMonster - 301) * 3 + 5) * 500,wPkg + 4);
		}

		if(byReborn == 2)
		{
			if(bFightOnLeopard)
			{
				tex.dwArmTex = MAKELONG(iOff + 13 * 500,wPkg + 4);
			}
			else
			{
				tex.dwArmTex = MAKELONG(iOff + 12 * 500,wPkg + 4);
			}
		}
	}
	//else			//凤凰没有铠甲
	{
		if(cDir == 4)
		{
			if(iMonster >= 39 && iMonster <= 41)//三种麒麟4号方向还有头部,放在PACKAGE_MONSTER8,PACKAGE_MONSTEREX8,PACKAGE_MONSTEREXN8
			{
				tex.dwHead = MAKELONG(iOff + (iMonster - 39)*500,wPkg + 7);
			}
			else if(iMonster == 437)//圣武羊驼
			{
				tex.dwHead = MAKELONG(iOff + (iMonster%100 + 1)*500,wPkg);
			}
		}

		if(/*byArm > 0 && */byReborn >= 1)
		{
			for (int i = 0; i != (MAT_ARM_TYPE_NUMBER - MAT_BODY); ++i)
			{
				std::map <DWORD, DWORD> :: const_iterator armit;
				int iArmIdx = 0;
				DWORD MonsterArmID = -1;
				DWORD ArmTex = 0;
				WORD wArmPkg = 0;

				int MonsterArmType = MAT_BODY + i;
				//if(i == 0 && iMonster >= 166 && iMonster <= 168 && byArm == 2) //天下第一豹
				//{
				//	MonsterArmType = MAT_FIRST_BAO;
				//}
				//else 
					if (i == 0 && iMonster >= 301 && iMonster <= 303 && bFightOnLeopard)
				{
					MonsterArmType = MAT_RIDE_BODY;
				}

				armit = m_mapMonsterArm.find(((DWORD)iMonster<<16)|((DWORD)byReborn<<8)|((DWORD)MonsterArmType));
				
				if(armit == m_mapMonsterArm.end())
					continue;
				
				MonsterArmID = armit->second;				

				if (MonsterArmID == -1)
					continue;
				
				wArmPkg = GetPackage(PICPACKAGE_MONSTER_ARM,MonsterArmID,pPicInfo->iStartIndex);
				iArmIdx = MONSTER_FRAMES * (MonsterArmID % 100) + iOff;
				ArmTex = MAKELONG(iArmIdx,wArmPkg);
				
				switch (MonsterArmType)
				{
				case MAT_BODY:
				case MAT_FIRST_BAO:
				case MAT_RIDE_BODY:
					{
						tex.dwArmTex = ArmTex;
					}
					break;
				case MAT_HEAD:
					{
						tex.dwHeadArm = ArmTex;
					}
					break;
				case MAT_RIDE_LEFT:
					{
						tex.dwMonsterLeftArm = ArmTex;
					}
					break;
				case MAT_RIDE_RIGHT:
					{
						tex.dwMonsterRightArm = ArmTex;
					}
					break;
				}
			}
		}
	}

	int FaBaoType;
	int ShowFrame;
	int FaBaoState;
	WORD wPackage = 0;
	if (pFaBao && (pFaBao->iHide == 0))
	{
		ActionMap::iterator itrFaBao = m_vFaBao.find(iAction == ACTION_STAND?ACTION_STAND:ACTION_RUN);

		pFaBao->GetCurShowInfo(FaBaoType,ShowFrame,FaBaoState);
		if (FaBaoType >= 0) //有法宝
		{
			int realFaBaoType = FaBaoType - 1;	//法宝编号从1开始,图片从0开始
			//收起或出现阶段
			if (FaBaoState == 2 && FaBaoType > 0)
			{
				//消失出现同attack
				int iDirOff = cDir*8 + ShowFrame;
				int iOffset = 305 + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,ShowFrame);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,ShowFrame);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pFaBao->SetState(2);
			}
			else if ((FaBaoType > 0) && (iAction == ACTION_WALK 
				|| iAction == ACTION_RUN
				|| iAction == ACTION_HWALK
				|| iAction == ACTION_HRUN
				|| iAction == ACTION_LWALK
				|| iAction == ACTION_LRUN
				|| iAction == ACTION_LFWALK
				|| iAction == ACTION_LFRUN))//如果在移动
			{
				if (itrFaBao == m_vFaBao.end())
				{
					return tex;
				}

				int iDirOff = cDir*itrFaBao->second.iFrameMax + iFrame *(1+ itrFaBao->second.iInterval);
				int iOffset = itrFaBao->second.iStartIndex + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,itr->second.iStartIndex);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,itr->second.iStartIndex);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pFaBao->SetState(1);
			}
			else if(FaBaoType > 0)//待机
			{
				itrFaBao = m_vFaBao.find(ACTION_STAND);
				if (itrFaBao == m_vFaBao.end())
				{
					return tex;
				}
				int iDirOff = cDir*itrFaBao->second.iFrameMax + iFrame%4;
				int iOffset = itrFaBao->second.iStartIndex % 1000 + iDirOff;
				int iIdx = PLAYER_FRAMES * (realFaBaoType % 50) + iOffset;


				wPackage = GetPackage(PICPACKAGE_FABAO,realFaBaoType,ShowFrame);

				tex.dwFaBao = MAKELONG(iIdx,wPackage);

				wPackage = GetPackage(PICPACKAGE_FABAOEFF,realFaBaoType,ShowFrame);

				tex.dwFaBaoEff = MAKELONG(iIdx,wPackage);
				pFaBao->SetState(0);
			}
			pFaBao->Update();
		}
	}


	return tex;
}


DWORD CActionInf::GetNPCTex(DrawCharInf* pDrawInf)
{
	if(pDrawInf == NULL || pDrawInf->pLooks == NULL)
		return 0;


	return GetNPCTex(pDrawInf->pLooks->Char.wShape,pDrawInf->iAction,pDrawInf->iFrame,pDrawInf->iDir);
}

// 获取NPC贴图
DWORD CActionInf::GetNPCTex(int iNPC,int iAction,int iFrame,char cDir)
{
	ActionMap::iterator itr = m_vNpc.find(iAction);
	if(itr == m_vNpc.end())
		return 0;

	int iIdx = NPC_FRAMES * (iNPC % 300)
		+ itr->second.iStartIndex
		+ cDir * itr->second.iFrameMax
		+ iFrame;

	DWORD dwID = MAKELONG(iIdx,PACKAGE_npc1 + (iNPC / 300));
	return dwID;
}


BOOL CActionInf::IsRideLepoard(DrawCharInf* pDrawInf)
{
	//if(pDrawInf->wBody != 16)
	//	return FALSE;

	return IsLeopard(pDrawInf->pLooks->Player.wHorse);
}

int CActionInf::GetRideAction(int iAction,int iDrawAction)
{
	int iRide = iAction;
	switch(iDrawAction)
	{
	case ACTION_HSTAND:
	case ACTION_LSTAND:
	case ACTION_LFSTAND:
		iRide = ACTION_STAND;
		break;
	case ACTION_HWALK:
	case ACTION_LWALK:
	case ACTION_LFWALK:
		iRide = ACTION_WALK;
		break;
	case ACTION_HRUN:
	case ACTION_LRUN:
	case ACTION_LFRUN:
		iRide = ACTION_RUN;
		break;
	case ACTION_HDEATH:
	case ACTION_LDEATH:
	case ACTION_LFDEATH:
		iRide = ACTION_DEATH;
		break;
	case ACTION_HATTACKED:
	case ACTION_LATTACKED:
	case ACTION_LFATTACKED:
		iRide = ACTION_ATTACKED;
		break;
	case ACTION_LSALUTE:
		iRide = ACTION_ATTACK3;
		break;
	case ACTION_LFATTACK1:
	case ACTION_LFMAGIC:
	case ACTION_LFMAGICFGH:
	case ACTION_LFMAGICRAB:
	case ACTION_LFMAGICTAO:
		iRide = ACTION_ADDACTION3 + iDrawAction - ACTION_LFATTACK1;
		break;
	case ACTION_LFSWT:
	case ACTION_LFLSJ:
	case ACTION_LFJYL:
		iRide = ACTION_STAND;
		break;
	default:
		break;
	}

	return iRide;
}

int CActionInf::GetWingsAction(DrawCharInf* pDrawInf,bool bEffected)
{
	int iWingAction = pDrawInf->iDrawAction;

	//重新影射动作
	if(!bEffected)
	{
		if(IsRideLepoard(pDrawInf))
		{
			switch(iWingAction)
			{
			case ACTION_HSTAND:
			case ACTION_LSTAND:
			case ACTION_LFSTAND:
				iWingAction = ACTION_LSTAND;
				break;
			case ACTION_HWALK:
			case ACTION_LWALK:
			case ACTION_LFWALK:
				iWingAction = ACTION_LWALK;
				break;
			case ACTION_HRUN:
			case ACTION_LRUN:
			case ACTION_LFRUN:
				iWingAction = ACTION_LRUN;
				break;
			case ACTION_HDEATH:
			case ACTION_LDEATH:
			case ACTION_LFDEATH:
				iWingAction = ACTION_LDEATH;
				break;
			case ACTION_HATTACKED:
			case ACTION_LATTACKED:
			case ACTION_LFATTACKED:
				iWingAction = ACTION_LATTACKED;
				break;
			}
		}
	}
	else
	{
		//元神特效动作
		if(IsRideLepoard(pDrawInf))
		{
			switch(iWingAction)
			{
			case ACTION_LSTAND:
			case ACTION_LFSTAND:
				iWingAction = ACTION_LFSTAND;
				break;
			case ACTION_HWALK:
			case ACTION_LWALK:
			case ACTION_LFWALK:
				iWingAction = ACTION_LFWALK;
				break;
			case ACTION_HRUN:
			case ACTION_LRUN:
			case ACTION_LFRUN:
				iWingAction = ACTION_LFRUN;
				break;
			case ACTION_HDEATH:
			case ACTION_LDEATH:
			case ACTION_LFDEATH:
				iWingAction = ACTION_LFDEATH;
				break;
			case ACTION_HATTACKED:
			case ACTION_LATTACKED:
			case ACTION_LFATTACKED:
				iWingAction = ACTION_LFATTACKED;
				break;
			}
		}
	}


	//女人死亡翅膀特殊
	if(iWingAction == ACTION_DEATH && pDrawInf->iSex == 1)
		iWingAction = ACTION_ADDACTION1;

	return iWingAction;
}

void CActionInf::GetWingsTex(DrawCharInf* pDrawInf, PlayerTex& tex)
{
	//参数检查
	if(!pDrawInf || !pDrawInf->pLooks || pDrawInf->byWing == 0 || (!g_Config.GetOpenWing() && pDrawInf->byWing < 100))//100以上的为神翼,不可以关闭
		return;

	int iBody = pDrawInf->pLooks->Player.wBody;

	//if(pDrawInf->pLooks->Player.wHorse >= 30 && pDrawInf->pLooks->Player.wHorse <= 35)	//骑凤凰没有翅膀
	if(IsPhenix(pDrawInf->pLooks->Player.wHorse))//骑凤凰没有翅膀
		return;

	iBody	= iBody * 2 + pDrawInf->iSex;
	if(pDrawInf->byWing < 100 &&(iBody < 18 || iBody == 24 || iBody == 25 || iBody == 34 || iBody == 35) )//100以上的为神翼,和衣服没有关系
		return ;

	int iWing = pDrawInf->byWing % 100 - 1;
	if(iWing >= m_Wing.size())
		return;

	int iWingAction = GetWingsAction(pDrawInf);

	WingMap::iterator itr = m_Wing[iWing].find(iWingAction);
	if(itr == m_Wing[iWing].end())
		return;

	stWingPic& PicInfo = itr->second;

	int iFrame = pDrawInf->iFrame;
	switch(pDrawInf->iAction)
	{
	case ACTION_STAND:
		if(iFrame >= 4) iFrame = 6-iFrame;
		break;
	case ACTION_DEATH:
		if (pDrawInf->iDrawAction == 24 && iFrame > 7) 
			iFrame = 0;
		else if (pDrawInf->iDrawAction == 19 && iFrame >= 7)
			iFrame = 7;
		//else if(pDrawInf->iDrawAction != 24 && iFrame > 3)
		//	iFrame = 0;
		break;
	default:
		break;
	}

	int iFrameMax = PicInfo.iFrameLeft+PicInfo.iFrameRight+PicInfo.iFrameShadow;

	DWORD dwID = 0;
	int iOffset = PicInfo.iStartIndex + pDrawInf->iDir*iFrameMax;


	tex.dwWingShadowTex =  MAKELONG(iOffset+PicInfo.iFrameLeft + PicInfo.iFrameRight+iFrame,PicInfo.iPackage+PACKAGE_magic1);
	tex.dwWingLeftTex = MAKELONG(iOffset+iFrame,PicInfo.iPackage+PACKAGE_magic1);
	if(PicInfo.iFrameRight > 0)
		tex.dwWingRightTex = MAKELONG(iOffset + PicInfo.iFrameLeft + iFrame,PicInfo.iPackage+PACKAGE_magic1);
	else
		tex.dwWingRightTex = 0;

	if(pDrawInf->byWingEff > 0 && pDrawInf->byWingEff < m_WingEff.size())
	{
		iWingAction = GetWingsAction(pDrawInf,true);
		WingMap::iterator itrEff = m_WingEff[pDrawInf->byWingEff - 1].find(iWingAction);
		if(itrEff == m_WingEff[pDrawInf->byWingEff - 1].end())
			return;

		stWingPic& PicInfoEff = itrEff->second;
		iFrameMax = PicInfoEff.iFrameLeft + PicInfoEff.iFrameRight + PicInfoEff.iFrameShadow;
		int iOffsetEff = PicInfoEff.iStartIndex + pDrawInf->iDir*iFrameMax;


		if(PicInfoEff.iFrameLeft > 0)
			tex.dwWingLeftEffTex = MAKELONG(iOffsetEff+iFrame,PicInfoEff.iPackage+PACKAGE_magic1);
		else
			tex.dwWingLeftEffTex = 0;		

		if (PicInfoEff.iFrameRight > 0)
			tex.dwWingRightEffTex = MAKELONG(iOffsetEff+iFrame + PicInfoEff.iFrameLeft,PicInfoEff.iPackage+PACKAGE_magic1);
		else
			tex.dwWingRightEffTex = 0;

		if(PicInfoEff.iFrameShadow > 0)
			tex.dwWingShadowEffTex = MAKELONG(iOffsetEff+iFrame + PicInfoEff.iFrameLeft+PicInfoEff.iFrameRight,PicInfoEff.iPackage+PACKAGE_magic1);
		else
			tex.dwWingShadowEffTex = 0;
	}
}
//缓冲
void CActionInf::CacheHumanLooks(const PlayerTex &TEX,int iFrameNow,int iFrameCount,eReadPrior ePrior)
{
	if(TEX.dwClothTex != 0)
		g_pTexMgr->PreLoad(TEX.dwClothTex - iFrameNow,TEX.dwClothTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwPatternTex != 0)
		g_pTexMgr->PreLoad(TEX.dwPatternTex - iFrameNow,TEX.dwPatternTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwHairTex != 0)
		g_pTexMgr->PreLoad(TEX.dwHairTex - iFrameNow,TEX.dwHairTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwWeaponTex != 0)
		g_pTexMgr->PreLoad(TEX.dwWeaponTex - iFrameNow,TEX.dwWeaponTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwShieldTex != 0)
		g_pTexMgr->PreLoad(TEX.dwShieldTex - iFrameNow,TEX.dwShieldTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwMShapeTex != 0)
		g_pTexMgr->PreLoad(TEX.dwMShapeTex - iFrameNow,TEX.dwMShapeTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwMArmTex != 0)
		g_pTexMgr->PreLoad(TEX.dwMArmTex - iFrameNow,TEX.dwMArmTex - iFrameNow + iFrameCount,ePrior);

	if(TEX.dwWingShadowTex != 0)
		g_pTexMgr->PreLoad(TEX.dwWingShadowTex - iFrameNow,TEX.dwWingShadowTex - iFrameNow + iFrameCount,ePrior);
	if(TEX.dwWingLeftTex != 0)
		g_pTexMgr->PreLoad(TEX.dwWingLeftTex - iFrameNow,TEX.dwWingLeftTex - iFrameNow + iFrameCount,ePrior);
	if(TEX.dwWingRightTex != 0)
		g_pTexMgr->PreLoad(TEX.dwWingRightTex - iFrameNow,TEX.dwWingRightTex - iFrameNow + iFrameCount,ePrior);

	//case CHARACTER_NPC:
	//	{
	//		ActionMap::iterator itr = m_vNpc.find(pDrawInf->iAction);
	//		if(itr == m_vNpc.end())
	//			return;

	//		int iStart = itr->second.iStartIndex;
	//		int iCount = itr->second.iFrameMax * DIR_NUMS;

	//		int iNpc = pDrawInf->pLooks->Char.wShape;

	//		iIdx = NPC_FRAMES*+iStart;
	//		g_pTexMgr->PreLoad(PACKAGE_npc1,iIdx,iIdx+iCount);
	//		break;
	//	}
	//}
}

void CActionInf::CacheMonsterLooks(const MonsterTex &MTEX,int iFrameNow,int iFrameCount,eReadPrior ePrior)
{
	if(MTEX.dwShapeTex != 0)
		g_pTexMgr->PreLoad(MTEX.dwShapeTex - iFrameNow,MTEX.dwShapeTex - iFrameNow + iFrameCount,ePrior);

	if(MTEX.dwArmTex != 0)
		g_pTexMgr->PreLoad(MTEX.dwArmTex - iFrameNow,MTEX.dwArmTex - iFrameNow + iFrameCount,ePrior);	

	if(MTEX.dwMonsterEff != 0)
		g_pTexMgr->PreLoad(MTEX.dwMonsterEff - iFrameNow,MTEX.dwMonsterEff - iFrameNow + iFrameCount,ePrior);
}

void CActionInf::GetBoothInfo(int iDir,DrawBoothInf* pInf)
{
	if(pInf == NULL)
		return;

	pInf->iOffX = pInf->iOffY = 0;
	pInf->iFlagOffX = pInf->iFlagOffY = 0;

	switch (iDir)
	{
	case DIR_RIGHTUP:
	case DIR_RIGHT:
		{
			pInf->iOffY = -14;
			pInf->iFlagOffX = -90;
			pInf->iFlagOffY = -95;
			if(pInf->iBoothType == 3)
			{
				pInf->iOffX = 47;
				pInf->iOffY = -69;
			}	
			pInf->iDir = 3;
			break;
		}
	case DIR_RIGHTDOWN:
	case DIR_DOWN:
		{	
			pInf->iFlagOffX = -100;
			pInf->iFlagOffY = -31;
			if(pInf->iBoothType == 3)
			{
				pInf->iOffX = 30;
				pInf->iOffY = -36;
			}
			pInf->bBoothFirst = FALSE;
			pInf->iDir = 0;
			break;
		}
	case DIR_LEFTDOWN:
	case DIR_LEFT:
		{
			pInf->iFlagOffX = -100;
			pInf->iFlagOffY = -80;
			if(pInf->iBoothType == 3)
			{
				pInf->iOffX = 5;
				pInf->iOffY = -10;
			}
			pInf->bFlagFirst = TRUE;
			pInf->bBoothFirst = FALSE;
			pInf->iDir = 2;
			break;
		}
	case DIR_LEFTUP:
	case DIR_UP:
		{
			pInf->iOffY = -14;
			pInf->iFlagOffX = -110;
			pInf->iFlagOffY = -60;
			if(pInf->iBoothType == 3)
			{
				pInf->iOffX = -15;
				pInf->iOffY = -80;
			}
			pInf->iDir = 1;
			break;
		}
	}
	if(pInf->iBoothType == 0)
		pInf->bBoothFirst = TRUE;

	pInf->iOffX -= 64;
	pInf->iOffY -= 53;
	pInf->iFlagOffX -= 34;
	pInf->iFlagOffY -= 74;
}

void CActionInf::LoadDATFile()
{
	clear();

	//人物动作
	m_vPlayer[ACTION_STAND] = stActionPic(1,6,4,16);
	m_vPlayer[ACTION_WALK]  = stActionPic(49,16,16,2);
	m_vPlayer[ACTION_RUN] = stActionPic(177,16,16,2);
	m_vPlayer[ACTION_ATTACK1] = stActionPic(305,8,6,6);
	m_vPlayer[ACTION_ATTACKED] = stActionPic(369,5,3,4);
	m_vPlayer[ACTION_MAGIC] = stActionPic(409,10,8,4);
	m_vPlayer[ACTION_HSTAND] = stActionPic(489,6,4,16);
	m_vPlayer[ACTION_HWALK] = stActionPic(537,10,8,4);
	m_vPlayer[ACTION_HRUN] = stActionPic(617,10,8,4);
	m_vPlayer[ACTION_HDEATH] = stActionPic(697,8,8,8);
	m_vPlayer[ACTION_DEATH] = stActionPic(761,5,4,8);
	m_vPlayer[ACTION_CUT] = stActionPic(801,5,3,8);
	m_vPlayer[ACTION_HATTACKED] = stActionPic(841,5,3,8);
	m_vPlayer[ACTION_SALUTE] = stActionPic(881,8,6,6);
	//m_vPlayer[ACTION_HSALUTE] = stActionPic(945,16,16,8);

	//以下动作图片放在HumanEx1,2...图包中
	m_vPlayer[ACTION_LSTAND] = stActionPic(1001,6,4,16);
	m_vPlayer[ACTION_LWALK]  = stActionPic(1049,10,8,4);
	m_vPlayer[ACTION_LRUN] = stActionPic(1129,10,8,4);
	m_vPlayer[ACTION_LDEATH] = stActionPic(1209,8,8,4);
	m_vPlayer[ACTION_LATTACKED] = stActionPic(1275,5,3,8);
	m_vPlayer[ACTION_LSALUTE] = stActionPic(1315,16,16,8);
	m_vPlayer[ACTION_LFSTAND] = stActionPic(1443,4,4,16);
	m_vPlayer[ACTION_LFWALK]  = stActionPic(1475,8,8,4);
	m_vPlayer[ACTION_LFRUN]   = stActionPic(1539,8,8,4);
	m_vPlayer[ACTION_LFDEATH] = stActionPic(1603,8,8,8);
	m_vPlayer[ACTION_LFATTACKED] = stActionPic(1667,3,3,8);
	m_vPlayer[ACTION_LFATTACK1]  = stActionPic(1691,6,6,6);
	m_vPlayer[ACTION_LFMAGIC]	 = stActionPic(1739,8,8,6);
	m_vPlayer[ACTION_LFMAGICFGH] = stActionPic(1803,14,14,4);
	m_vPlayer[ACTION_LFMAGICRAB] = stActionPic(1915,8,8,4);


	////以下动作图片放在HumanExn1,2...图包中
	m_vPlayer[ACTION_LFMAGICTAO] = stActionPic(2001,12,12,4);
	m_vPlayer[ACTION_JYL] = stActionPic(2098,28,28,4);
	m_vPlayer[ACTION_LFSWT] = stActionPic(2324,18,18,6);
	m_vPlayer[ACTION_LFLSJ] = stActionPic(2470,13,13,6);
	m_vPlayer[ACTION_LFJYL] = stActionPic(2576,18,18,4);
	////以下动作图片放在HumanExo1,2...图包中
	m_vPlayer[ACTION_SWT] = stActionPic(3098,18,18,6);
	m_vPlayer[ACTION_LSJ] = stActionPic(3244,13,13,4);

	//法宝动作
	m_vFaBao[ACTION_STAND] = stActionPic(1,6,4,16);
	m_vFaBao[ACTION_WALK]  = stActionPic(49,16,16,2);
	m_vFaBao[ACTION_RUN] = stActionPic(49,16,16,2);

	m_vFaBao[ACTION_HSTAND] = stActionPic(1,6,4,16);
	m_vFaBao[ACTION_HWALK] = stActionPic(49,16,8,4,1);
	m_vFaBao[ACTION_HRUN] = stActionPic(49,16,8,4,1);

	m_vFaBao[ACTION_LSTAND] = stActionPic(1,6,4,16);
	m_vFaBao[ACTION_LWALK]  = stActionPic(49,16,8,4,1);
	m_vFaBao[ACTION_LRUN] = stActionPic(49,16,8,4,1);

	m_vFaBao[ACTION_LFSTAND] = stActionPic(1,6,4,16);
	m_vFaBao[ACTION_LFWALK]  = stActionPic(49,16,8,4,1);
	m_vFaBao[ACTION_LFRUN]   = stActionPic(49,16,8,4,1);

	//NPC动作
	m_vNpc[ACTION_STAND] = stActionPic(1,10,4,16);
	m_vNpc[ACTION_WALK] = stActionPic(81,8,8,4);
	m_vNpc[ACTION_STAND2] = stActionPic(146,9,8,8);
	m_vNpc[ACTION_STAND3] = stActionPic(146,18,12,8);

	//怪物动作
	m_vMonster[ACTION_STAND] = stActionPic(1,6,4,16);
	m_vMonster[ACTION_WALK] = stActionPic(49,8,8,4);
	m_vMonster[ACTION_ATTACK1] = stActionPic(113,8,8,6);
	m_vMonster[ACTION_ATTACKED] = stActionPic(177,5,4,4);
	m_vMonster[ACTION_DEATH] = stActionPic(217,8,8,8);
	m_vMonster[ACTION_RUN] = stActionPic(281,8,8,4);
	m_vMonster[ACTION_APPEAR] = stActionPic(345,8,5,8);
	m_vMonster[ACTION_ATTACK2]=stActionPic(345,8,8,4);

	//ex
	m_vMonster[ACTION_ATTACK3]=stActionPic(1001,16,16,4);
	m_vMonster[ACTION_ATTACK4]=stActionPic(1130,4,4,4);
	m_vMonster[ACTION_ATTACK5]=stActionPic(1163,12,12,4);
	m_vMonster[ACTION_ATTACK6]=stActionPic(1260,4,4,4);
	m_vMonster[ACTION_ADDACTION2]=stActionPic(1293,4,4,4);
	m_vMonster[ACTION_ADDACTION3]=stActionPic(1388,6,6,4);
	m_vMonster[ACTION_ADDACTION4]=stActionPic(1436,8,8,4);

	//exn
	m_vMonster[ACTION_ADDACTION1]=stActionPic(2001,24,24,4);
	m_vMonster[ACTION_ADDACTION5]=stActionPic(2193,14,14,4);
	m_vMonster[ACTION_ADDACTION6]=stActionPic(2306,8,8,4);
	m_vMonster[ACTION_ADDACTION7]=stActionPic(2371,12,12,4);

	//怪物装备动作
	/*m_vMonsterArm[ACTION_STAND] = stActionPic(1,6,4,16);
	m_vMonsterArm[ACTION_WALK] = stActionPic(49,8,8,4);
	m_vMonsterArm[ACTION_ATTACK1] = stActionPic(113,8,8,6);
	m_vMonsterArm[ACTION_ATTACKED] = stActionPic(177,5,3,4);
	m_vMonsterArm[ACTION_DEATH] = stActionPic(217,8,8,8);
	m_vMonsterArm[ACTION_RUN] = stActionPic(281,8,8,4);
	m_vMonsterArm[ACTION_APPEAR] = stActionPic(345,8,5,8);
	m_vMonsterArm[ACTION_ATTACK2]=stActionPic(345,16,16,4);
	m_vMonsterArm[ACTION_ATTACK3]=stActionPic(539,16,16,4);
	m_vMonsterArm[ACTION_ATTACK4]=stActionPic(668,4,4,4);
	m_vMonsterArm[ACTION_ATTACK5]=stActionPic(701,12,12,4);
	m_vMonsterArm[ACTION_ATTACK6]=stActionPic(798,4,4,4);
	m_vMonsterArm[ACTION_ADDACTION1]=stActionPic(978,24,24,4);
	m_vMonsterArm[ACTION_ADDACTION2]=stActionPic(831,4,4,4);
	m_vMonsterArm[ACTION_ADDACTION3]=stActionPic(864,6,6,4);
	m_vMonsterArm[ACTION_ADDACTION4]=stActionPic(913,8,8,4);
	m_vMonsterArm[ACTION_ADDACTION5]=stActionPic(1171,14,14,4);
	m_vMonsterArm[ACTION_ADDACTION6]=stActionPic(1284,8,8,4);
	m_vMonsterArm[ACTION_ADDACTION7]=stActionPic(1349,12,12,4);*/

	char path[255] = {0};
	sprintf_s(path,"%s\\config\\ActionCfg.dat",GetGameDataDir());

	FILE* fp = fopen(path,"rb");
	if(fp)
	{
		//读入绘制顺序表
		char buf[33] = {0};
		DWORD dwVer = 0;
		int count = 0;
		int tmp = 0;

		fread(buf,1,32,fp);
		fread(&dwVer,sizeof(DWORD),1,fp);
		fread(&count,sizeof(int),1,fp);
		fread(&tmp,sizeof(int),1,fp);
		fread(&tmp,sizeof(int),1,fp);

		for(int ii = 0;ii < count;ii++)
		{
			int iAction = 0,iFrame = 0;

			fread(&iAction,sizeof(int),1,fp);
			fread(&iFrame,sizeof(int),1,fp);

			if(iFrame > 0)
			{
				SDrawOrder_Dir_Frame* pData = new SDrawOrder_Dir_Frame[8*iFrame];
				fread(pData,sizeof(SDrawOrder_Dir_Frame),8*iFrame,fp);

				// 规范化 byOrder: 将 <24 的有效绘制层提到前面, 追加 24 终止标记, 其余填 255
				for (int j = 0; j < 8 * iFrame; j++)
				{
					BYTE byTemp[DP_DrawPartMaxNum] = {};
					int iPos = 0;

					// 第一遍: 收集所有有效绘制层 (< 24)
					for (int k = 0; k < DP_DrawPartMaxNum; k++)
					{
						if (pData[j].byOrder[k] < DP_DrawPartEffectNum)
							byTemp[iPos++] = pData[j].byOrder[k];
					}

					// 追加终止标记 24
					byTemp[iPos++] = DP_DrawPartEffectNum;

					// 剩余填 255
					while (iPos < DP_DrawPartMaxNum)
						byTemp[iPos++] = 255;

					memcpy(pData[j].byOrder, byTemp, DP_DrawPartMaxNum);
				}

				m_MDrawOrder[iAction] = SDrawOrder(iFrame,pData);				
			}
		}

		//读入怪物动作
		fread(buf,1,32,fp);
		fread(&dwVer,sizeof(DWORD),1,fp);
		fread(&count,sizeof(int),1,fp);
		fread(&tmp,sizeof(int),1,fp);
		fread(&tmp,sizeof(int),1,fp);

		for (int i = 0;i< count;i++)
		{
			int iActionNum = 0;
			fread(&iActionNum,sizeof(int),1,fp);

			ActionMap ActionInfo;
			for (int j = 0;j < iActionNum;j++)
			{
				int iAction = 0;
				int iStartIndex = 0;
				int mmx = 0,realx = 0,speedx = 0;

				fread(&iAction,sizeof(int),1,fp);
				fread(&iStartIndex,sizeof(int),1,fp);
				fread(&mmx,sizeof(int),1,fp);
				fread(&realx,sizeof(int),1,fp);
				fread(&speedx,sizeof(int),1,fp);

				//怪物在原有配置的基础上自动乘以2,因为老传世是以1/25为单位的
				//而现在是以1/50为单位的
				ActionInfo[iAction] = stActionPic(iStartIndex,mmx,realx,speedx*2);
			}
			m_Monster.push_back(ActionInfo);
		}

		//读入翅膀配置
		int wingcount = 0;
		int wingeffcount = 0;
		memset(buf,0,32);
		fread(buf,1,32,fp);
		fread(&dwVer,sizeof(DWORD),1,fp);
		fread(&wingcount,sizeof(int),1,fp);
		fread(&wingeffcount,sizeof(int),1,fp);
		fread(&tmp,sizeof(int),1,fp);

		for(int ii = 0;ii < wingcount;ii++)
		{
			int iActionNum = 0;
			fread(&iActionNum,sizeof(int),1,fp);

			WingMap mWing;

			for (int i=0;i < iActionNum;i++)
			{
				int iAction = 0;
				int iPackage = 0,iStartIndex = 0;
				int iFrameLeft = 0,iFrameRight = 0,iFrameShadow = 0;

				fread(&iAction,sizeof(int),1,fp);
				fread(&iPackage,sizeof(int),1,fp);
				fread(&iStartIndex,sizeof(int),1,fp);
				fread(&iFrameLeft,sizeof(int),1,fp);
				fread(&iFrameRight,sizeof(int),1,fp);
				fread(&iFrameShadow,sizeof(int),1,fp);

				mWing[iAction] = stWingPic(iPackage,iStartIndex,iFrameLeft,iFrameRight,iFrameShadow);
			}
			m_Wing.push_back(mWing);
		}

		for (int i = 0;i < wingeffcount;i++)
		{
			int iActionNum = 0;
			fread(&iActionNum,sizeof(int),1,fp);

			WingMap mWingEff;
			for (int i = 0;i < iActionNum;i++)
			{	
				int iAction = 0;
				int iPackage = 0,iStartIndex = 0;
				int iFrameLeft = 0,iFrameRight = 0,iFrameShadow = 0;

				fread(&iAction,sizeof(int),1,fp);
				fread(&iPackage,sizeof(int),1,fp);
				fread(&iStartIndex,sizeof(int),1,fp);
				fread(&iFrameLeft,sizeof(int),1,fp);
				fread(&iFrameRight,sizeof(int),1,fp);
				fread(&iFrameShadow,sizeof(int),1,fp);

				mWingEff[iAction] = stWingPic(iPackage,iStartIndex,iFrameLeft,iFrameRight,iFrameShadow);
			}
			m_WingEff.push_back(mWingEff);
		}


		fclose(fp);
	}
}

SDrawOrder_Dir_Frame * CActionInf::Get_Draw_Order_Dir_Frame(int iAction,int iDir,int iFrame)
{
	DrawOrderMap::iterator itr = m_MDrawOrder.find(iAction);
	if(itr == m_MDrawOrder.end())
	{
		return NULL;
	}
	else
	{
		return &(itr->second.pData[itr->second.iFrames * iDir + iFrame]);
	}
}

int CActionInf::GetPackage(int iType,int iIndex,int iStartIndex,int iBody)
{
	if(iType <= PICPACKAGE_NULL) return 0;	

	if(iType == PICPACKAGE_HUMAN || iType == PICPACKAGE_PATTERN || iType == PICPACKAGE_HAIR)
	{
		return m_PackageIndex[iType][iStartIndex / 1000] + iIndex / 50;
	}
	else if(iType == PICPACKAGE_WEAPON || iType == PICPACKAGE_WEAPONEFF)
	{
		if(iStartIndex > 1000 && iBody == 24)
		{
			return m_PackageIndex[iType][iStartIndex / 1000] + iIndex / 60 + PACKAGE_weapon8 - PACKAGE_weapon1;
		}
		else
		{
			return m_PackageIndex[iType][iStartIndex / 1000] + iIndex / 60;
		}

	}
	else if(iType == PICPACKAGE_MONSTER)
	{		
		return m_PackageIndex[iType][iStartIndex / 1000] + iIndex / 100;

	}
	else if (iType == PICPACKAGE_FABAO)
	{
		return m_PackageIndex[iType][0] + iIndex / 50;	
	}
	else if (iType == PICPACKAGE_FABAOEFF)
	{
		return m_PackageIndex[iType][0] + iIndex / 50;	
	}
	else if (iType == PICPACKAGE_MONSTER_ARM)
	{
		return m_PackageIndex[iType][iStartIndex / 1000] + iIndex / 100;	
	}

	return 0;
}

int CActionInf::ConvertAction(int iAction,int iType)
{
	if(iType == APT_RIDE_ATTACK_LEAPOARD)
	{
		switch(iAction)
		{
		case ACTION_STAND:
			iAction = ACTION_LFSTAND;
			break;
		case ACTION_WALK:
			iAction = ACTION_LFWALK;
			break;
		case ACTION_RUN:
			iAction = ACTION_LFRUN;
			break;
		case ACTION_ATTACK1:
			iAction = ACTION_LFATTACK1;
			break;
		case ACTION_ATTACKED:
			iAction = ACTION_LFATTACKED;
			break;
		case ACTION_MAGIC:
			iAction = ACTION_LFMAGIC;
			break;
		case ACTION_DEATH:
			iAction = ACTION_LFDEATH;
			break;
		case ACTION_SALUTE:
			iAction = ACTION_LSALUTE;
			break;
		case ACTION_SWT:
			iAction = ACTION_LFSWT;
			break;
		case ACTION_LSJ:
			iAction = ACTION_LFLSJ;
			break;
		case ACTION_JYL:
			iAction = ACTION_LFJYL;
			break;
		}
	}
	else if (iType == APT_RIDE_LEPOARD)
	{
		switch(iAction)
		{
		case ACTION_STAND:
			iAction = ACTION_LSTAND;
			break;
		case ACTION_WALK:
			iAction = ACTION_LWALK;
			break;
		case ACTION_RUN:
			iAction = ACTION_LRUN;
			break;
		case ACTION_ATTACK1:
			iAction = ACTION_LFATTACK1;
			break;
		case ACTION_ATTACKED:
			iAction = ACTION_LATTACKED;
			break;
		case ACTION_DEATH:
			iAction = ACTION_LDEATH;
			break;
		case ACTION_SALUTE:
			iAction = ACTION_LSALUTE;
			break;
		case ACTION_SWT:
			iAction = ACTION_LFSWT;
			break;
		case ACTION_LSJ:
			iAction = ACTION_LFLSJ;
			break;
		case ACTION_JYL:
			iAction = ACTION_LFJYL;
			break;
		}
	}
	else if(iType == APT_RIDE_HORSE)
	{
		switch(iAction)
		{
		case ACTION_STAND:
			iAction = ACTION_HSTAND;
			break;
		case ACTION_WALK:
			iAction = ACTION_HWALK;
			break;
		case ACTION_RUN:
			iAction = ACTION_HRUN;
			break;
		case ACTION_ATTACKED:
			iAction = ACTION_HATTACKED;
			break;
		case ACTION_DEATH:
			iAction = ACTION_HDEATH;
			break;
		}
	}

	return iAction;
}

int	CActionInf::GetDrawMonsterAppr(int iMonsterAppr ,BYTE& byReborn)
{
	//三转 + 13 = 一转

	int ResultAppr = iMonsterAppr;

	switch (iMonsterAppr)
	{
	case 452:	//豹子三转
	case 453:
	case 454:
	case (452+13):	
	case (453+13):
	case (454+13):
		{
			int t = (iMonsterAppr - 452)/13;

			if (t ==1 )
				byReborn = 0;
			else
				byReborn = 3;

			ResultAppr = (iMonsterAppr - 452)%13 + 166;
		}
		break;
	case 455:	//老虎三转
	case (455+13):
		{
			int t = (iMonsterAppr - 455)/13;

			if (t ==1 )
				byReborn = 0;
			else
				byReborn = 3;

			ResultAppr = (iMonsterAppr - 455)%13 + 373;
		}
		break;
	case 456:	//麒麟三转
	case 457:
	case 458:
	case (456+13):	
	case (457+13):
	case (458+13):
		{
			int t = (iMonsterAppr - 456)/13;

			if (t ==1 )
				byReborn = 0;
			else
				byReborn = 3;

			ResultAppr = (iMonsterAppr - 456)%13 + 39;
		}
		break;
	case 459:	//狮子三转
	case 460:
	case 461:
	case (459+13):	
	case (460+13):
	case (461+13):
		{
			int t = (iMonsterAppr - 459)/13;

			if (t ==1 )
				byReborn = 0;
			else
				byReborn = 3;

			ResultAppr = (iMonsterAppr - 459)%13 + 185;
		}
		break;
	case 462:	//凤凰三转
	case 463:
	case 464:
	case (462+13):	
	case (463+13):
	case (464+13):
		{
			int t = (iMonsterAppr - 462)/13;

			if (t ==1 )
				byReborn = 0;
			else
				byReborn = 3;

			ResultAppr = (iMonsterAppr - 462)%13 + 301;
		}
		break;
	}

	return ResultAppr;
}
