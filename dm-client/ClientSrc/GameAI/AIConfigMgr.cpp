#include "AIConfigMgr.h"
#include "AIMgr.h"
#include "GameData/GameData.h"
#include "GameData/GameGlobal.h"
#include "Global/Interface/StreamInterface.h"
#include "GameData/TalkMgr.h"
#include "GameData/LoginData.h"
#include "GameData/WooolStoreData.h"
#include "AIAutoEatMgr.h"
#include "GameClient/ReplayManager.h"
#include "BaseClass/TiXml/tinyxml.h"

CAIConfigMgr g_AICfgMgr;

CAIConfigMgr::CAIConfigMgr(void)
{
	m_files.push_back("asstool.ini");
	m_files.push_back("asstool.txt");
	m_files.push_back("bestitem.ini");
	m_files.push_back("boss.ini");

	m_bIsShowBestItem = false; // 只显示极品否
	m_bIsAutoMagicAttack = false; //连续魔法攻击
	m_bIsRunAttack = false;    // 是否追杀
	m_bIsAbsoluteLock = false;
	m_bNoDrawChar = false;
}

CAIConfigMgr::~CAIConfigMgr(void)
{
}

////////////////////////////////////////////////////////////////////////////
//内部函数
char* CAIConfigMgr::ReadConfigString(const char * strKey,const char * strDefault)
{
	DWORD dwSize = 1024;
	static char szReturn[1024] = {0};
	ZeroMemory(szReturn,1024);
	GetPrivateProfileString("Config",strKey,strDefault,szReturn,dwSize,m_strPath.c_str());
	return szReturn;
}

void CAIConfigMgr::WriteConfigString(const char* strKey,const char* str)
{
	WritePrivateProfileString("Config",strKey,str,m_strPath.c_str());
}

int CAIConfigMgr::ReadConfigInt(const char * strKey,int iDefault)
{
	int iRet = (int)GetPrivateProfileInt("Config",strKey,iDefault,m_strPath.c_str());
	return iRet;
}

void CAIConfigMgr::WriteConfigInt(const char* strKey,int i)
{
	static char str[64] = {0};
	sprintf(str,"%d",i);

	WritePrivateProfileString("Config",strKey,str,m_strPath.c_str());
}

int CAIConfigMgr::ReadConfigTime(const char* strKey,int iDefault)
{
	int iRet = (int)GetPrivateProfileInt("Time",strKey,iDefault,m_strPath.c_str());
	return iRet;
}

bool CAIConfigMgr::ReadConfigBoolean(const char* strKey,bool bDefault)
{
	int iReturn = ReadConfigInt(strKey,bDefault?1:0);

	if(iReturn == 1)
		return true;
	else 
		return false;

	return bDefault;
}

void CAIConfigMgr::WriteConfigBoolean(const char* strKey,bool b)
{
	char* str = b?"1":"0";
	WritePrivateProfileString("Config",strKey,str,m_strPath.c_str()); 
}

DWORD CAIConfigMgr::ReadConfigColor(const char* strKey,DWORD dwDefaultColor)
{
	DWORD dwSize = 64;
	char szReturn[64] = {0};
	GetPrivateProfileString("Color",strKey,"",szReturn,dwSize,m_strPath.c_str());

	if(strlen(szReturn) < 8)
		return dwDefaultColor;

	return (strtoul(szReturn,NULL,16));
}

void CAIConfigMgr::WriteConfigColor(const char* strKey,DWORD dwColor)
{
	char str[64] = {0};
	dwColor |= 0xFF000000;
	ltoa(dwColor,str,16);

	WritePrivateProfileString("Color",strKey,str,m_strPath.c_str());
}

int CAIConfigMgr::ReadConfigSection(const char* strSection,vector<string>& strArray)
{
	char szBuffer[4096] = {0};
	DWORD dwSize = 4096;
	GetPrivateProfileSection(strSection,szBuffer,dwSize,m_strPath.c_str());
	int iCount = 0;
	char* szStr = szBuffer;
	while (strlen(szStr)>0)
	{
		strArray.push_back(szStr);
		szStr = szStr+strlen(szStr)+1;
		iCount++;
	}
	return iCount;
}



void CAIConfigMgr::SetConfigPath()
{
	m_id = StringUtil::format("%d_%s_%s",
		g_Login.GetAreaNo(),g_Login.GetGroupName(),SELF.GetName());

#ifdef ENABLE_REPLAY
	if (g_ReplayManager.IsInReplay())
	{
		m_strSelfConfigDir = StringUtil::format("%s\\%s\\config\\%s\\",GetGameDataDir(),g_strReplayDir,m_id.c_str());
	}
	else 
#endif
	{
		m_strSelfConfigDir = StringUtil::format("%s\\config\\%s\\",GetGameDataDir(),m_id.c_str());
	}

	m_strPath = m_strSelfConfigDir + "AssTool.ini";

	string config_path = StringUtil::format("%s\\config\\",GetGameDataDir());

	if(mkdir(m_strSelfConfigDir.c_str()) == 0)
	{
		for(int i = 0; i < m_files.size(); i ++)
		{
			string file = config_path;
			file += "default\\";
			file += m_files[i].c_str();
			CopyFile(file.c_str(),(m_strSelfConfigDir + m_files[i]).c_str(),FALSE);
		}
	}

#ifdef ENABLE_REPLAY
	//录像的时候把配置文件复制一份到录像目录,为了无限次的播放,文件设为只读,播放的时候就用这份原始的文件
	if (g_ReplayManager.IsInRecord())
	{
		if (g_ReplayManager.GetRoleConfigDirMap().find(m_id) == g_ReplayManager.GetRoleConfigDirMap().end())
		{
			string szReplayDir = StringUtil::format("%s\\%s\\config\\%s\\",GetGameDataDir(),g_strReplayDir,m_id.c_str());
			mkdir(szReplayDir.c_str());
			for(int i = 0; i < m_files.size(); i ++)
			{
				string strFilePath = szReplayDir + m_files[i];
				DWORD dwAttr = GetFileAttributes(strFilePath.c_str());
				BOOL b = SetFileAttributes(strFilePath.c_str(),dwAttr & (~FILE_ATTRIBUTE_READONLY));
				CopyFile((m_strSelfConfigDir + m_files[i]).c_str(),strFilePath.c_str(),FALSE);
				b = SetFileAttributes(strFilePath.c_str(),FILE_ATTRIBUTE_READONLY);
			}

			g_ReplayManager.GetRoleConfigDirMap()[m_id] = 1;
		}
	}
#endif

}

void CAIConfigMgr::LoadConfigBeforeEnter()
{
	SetConfigPath();
	m_bShowNewHandPrompt = ReadConfigBoolean("ShowNewHandPrompt",true);
	m_bAutoShowActLog    = ReadConfigBoolean("AutoShowActLog",false);
}

void CAIConfigMgr::LoadConfig()
{
	SetConfigPath();

	LoadOtherConfig();
	LoadReadmeConfig();
	//LoadShortcutConfig();
	LoadMessageConfig();

	LoadQuickItemConfig(); //导入快捷消费的列表
}

void CAIConfigMgr::SetQuickItem(int i,CQuickItem& Good)
{
	if(i<0|| i>= QUICKITEM_COUNT) return;

	std::string str;
	char pBuff[32]={0};
	char strKey[128]={0};
	sprintf(pBuff,"QuickConsume%d",i);
	sprintf(strKey,"%d/%s/%s",Good.iItemLooks,Good.strItemID.c_str(),Good.strName.c_str());
	WriteConfigString(pBuff,strKey);
}

bool CAIConfigMgr::LoadQuickItemConfig()
{
	//如果配置文件中有，则加载配置文件中的数据
	char pBuff[100]={0};
	std::string strValue;
	std::string strTemp;
	bool bLoad = false;
	for(int i=0;i< QUICKITEM_COUNT;i++)
	{
		sprintf(pBuff,"QuickConsume%d",i);
		strValue.clear();
		strValue = ReadConfigString(pBuff);
		if( strValue.length()>10)
		{
			CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(i);
			char str[128] = {0};
			sscanf(strValue.c_str(),"%d/%s",&qitem.iItemLooks,str);
			strTemp = str;
			int iPos = strTemp.find_first_of("/");
			if(iPos >1)
			{
				qitem.strItemID =  strTemp.substr(0,iPos);
				qitem.strName = strTemp.substr(iPos+1);
			}
		}
 		else 
 		{
 			CQuickItem& qitem = g_WooolStoreMgr.GetQuickItem(i);
 			std::map<std::string,CQuickItem>& QuickBuyMap = g_WooolStoreMgr.GetQuickBuyItemMap();
 			int iLooks = 0;
			string szName = "";
 
			if(i == 0)
			{
				szName = "传音号角";
				iLooks = 836;
				qitem.strItemID = "10101289";
				qitem.iPrice = 5;
			}
			else if(i == 1)
			{
				szName = "打宝神佑";
				iLooks = 1510;
				qitem.strItemID = "10105553";
				qitem.iPrice = 3;
			}
			else if(i == 2)
			{
				szName = "群英令(捆)";
				iLooks = 5185;
				qitem.strItemID = "10105251";
				qitem.iPrice = 20;
			}
			else if(i == 3)
			{
				szName = "幸运符(捆)";
				iLooks = 975;
				qitem.strItemID = "10101780";
				qitem.iPrice = 20;
			}
			else if(i == 4)
			{
				szName = "龙骧金卡";
				iLooks = 1063;
				qitem.strItemID = "10105549";
				qitem.iPrice = 50;
			}
			else if(i == 5)
			{
				szName = "双倍经验包周卡";
				iLooks = 828;
				qitem.strItemID = "10101272";
				qitem.iPrice = 56;
			} 


			qitem.strName = szName;

			std::map<std::string,CQuickItem>::iterator iter = QuickBuyMap.find(szName.c_str()); 
 			if(iter != QuickBuyMap.end())
 			{
 				qitem = iter->second;
 			}

			qitem.iItemLooks = iLooks;
		}

	}


	if(!bLoad)
		g_WooolStoreMgr.QueryInfo(0);
	return true;
}

bool CAIConfigMgr::LoadOtherConfig()
{
	m_dwBestItemColor = ReadConfigColor("BestItemColor",0xFFFF0000);
	m_dwExpensiveItemColor = ReadConfigColor("ExpensiveItemColor",0xFF00FF00);
	m_dwPickItemColor = ReadConfigColor("PickItemColor",0xFFFFFFFF);
	m_dwShowItemColor = ReadConfigColor("ShowItemColor",0xFFFFFFFF);

	m_bIsAutoFindWay = ReadConfigBoolean("AutoFindWay", true);

	m_bIsMountain = ReadConfigBoolean("Mountain",true);
	m_bIsBury = ReadConfigBoolean("Bury",false);
	m_bShowExtendShortCut = ReadConfigBoolean("ShowExtend",false);
	m_bIsGoPickup = ReadConfigBoolean("GoPickup",true);
	m_bIsAutoPickup = ReadConfigBoolean("AutoPickup",false);
	m_bIsStarObject = ReadConfigBoolean("StarObject",false);
	m_bIsShowObjectName = ReadConfigBoolean("ShowObjectName",true);
	m_bIsShowPlayerName = ReadConfigBoolean("ShowPlayerName",true);
	m_bIsShowArrow = ReadConfigBoolean("ShowArrow",false);
	m_bIsTeamColor = ReadConfigBoolean("TeamColor",false);
	m_bIsAttackColor = ReadConfigBoolean("AttackColor",false);
	m_bIsCloseTeam = ReadConfigBoolean("CloseTeam",true);
	m_bIsDismount = ReadConfigBoolean("Dismount",true);
	m_bIsAntiAttack = ReadConfigBoolean("AntiAttack",false);
	m_bIsAutoSkill = ReadConfigBoolean("AutoSkill",false);
	m_iAutoSkillTime = ReadConfigInt("AutoSkillTime",4000);
	m_bIsPKAlarm = ReadConfigBoolean("PKAlarm",true);
	m_bIsQuickOutGame = ReadConfigBoolean("QuickOutGame",true);
	m_bShowNpcPriority = ReadConfigBoolean("ShowNpcPriority",true);
	m_bShowNewHandPrompt = ReadConfigBoolean("ShowNewHandPrompt",true);
	m_bShowHpChangeEffect = ReadConfigBoolean("ShowHpChangeEffect",true);
	m_bAcceptFrientAndTrade = ReadConfigBoolean("AcceptFrientAndTrade",true);
	//m_bAutoFeed = ReadConfigBoolean("AutoFeed",false);
	m_bAutoTeamVoice = ReadConfigBoolean("AutoTeamVoice",false);
	m_bAutoShowActLog = ReadConfigBoolean("AutoShowActLog",true);

	m_bIsDurationPrompt = ReadConfigBoolean("DurationPrompt",true);
	m_bIsBossPrompt = ReadConfigBoolean("BossAwoke",true);
	m_bIsObjectPrompt = ReadConfigBoolean("BestItemAwoke",true);
	m_bIsBlackFilter = ReadConfigBoolean("BlackFilter",true);

	m_bIsAllowFly = ReadConfigBoolean("AllowFly",true);
	m_bRunNotStop = ReadConfigBoolean("RunNotStop",false);

	m_iDurationPromptTime = ReadConfigTime("DurationPromptTime",4000);
	m_iBossPromptTime = ReadConfigTime("BossAwokeTime",10000);
	m_iObjectPromptTime = ReadConfigTime("BestItemAwokeTime",10000);

	//许猛配置文件
	m_bAutoQuit = ReadConfigBoolean("AutoQuit",false);
	m_iAutoQuitLimit = ReadConfigInt("AutoQuitLimit",0);
	m_iAutoAction = ReadConfigInt("AutoAction",0);
	m_iAutoActionLimit =ReadConfigInt("AutoActionLimit",0);
	m_auto_say.assign(ReadConfigString("AutoSay"));
	m_bIsAutoSay    = ReadConfigBoolean("AutoSayType",0);
	m_iAutoSayTimer = ReadConfigInt("AutoSayTime",0);
	m_bShowMagicTime = ReadConfigBoolean("MagicTime",1);
	m_bRecordMsg = ReadConfigBoolean("RecordMsg",0);
	m_bAutoReply  = ReadConfigBoolean("AutoReply",0);
	m_reply_msg.assign(ReadConfigString("AutoReplyMsg"));
	m_bAutoMogong = ReadConfigBoolean("AutoMogong",1);
	m_bMessageAwoke = ReadConfigBoolean("MessageAwoke",0);
	m_bKeepOutInfo = ReadConfigBoolean("KeepOutInfo",0);
	m_bPkAllAttack = ReadConfigBoolean("PK_AllAttack",0);
	m_bShowSysInfo = ReadConfigBoolean("ShowSysInfo",true);

	m_nHuiChengShiPosition = ReadConfigInt("HuiChengPos",0);
	if(m_nHuiChengShiPosition < 0 || m_nHuiChengShiPosition > 6)
		m_nHuiChengShiPosition = 0;
	m_bIsPetPickup   = ReadConfigBoolean("PetPickup",true);

	//田际军配置文件
	//战士
	m_bAutoSteelProtect = ReadConfigBoolean("AutoSteelProtect",true);
	m_bAutoProtectSkin = ReadConfigBoolean("AutoProtectSkin",false);
	m_bAutoFire = ReadConfigBoolean("AutoFire",true);
	m_bAutoSuperAttack = ReadConfigBoolean("AutoSuperAttack",true);
	m_iAutoWildDelay = ReadConfigInt("AutoWildDelay",1800);

	m_bAutoWildCollide = ReadConfigBoolean("AutoWildCollide ",true);
	m_bDirWild = ReadConfigBoolean("DirWild",true);

	m_bAutoWildRun = ReadConfigBoolean("AutoWildRun",false);
	m_bDoubleFire = ReadConfigBoolean("DoubleFire",false);
	m_bDestroyShield = ReadConfigBoolean("DestroyShield",true);
	m_bDestroyShell = ReadConfigBoolean("DestroyShell",true);
	//道士
	m_bAutoProtectGhost = ReadConfigBoolean("AutoProtectGhost",true);
	m_bAutoProtectArmor = ReadConfigBoolean("AutoProtectArmor",true);
	m_bAutoLionCall = ReadConfigBoolean("AutoLionCall",true);
	m_bSmartLionCall = ReadConfigBoolean("SmartLionCall",true);

	//法师
	m_bAutoMagicProtect = ReadConfigBoolean("AutoMagicProtect ",true);
	m_bAutoDodgeSkill = ReadConfigBoolean("AutoDodgeSkill ",false);
	m_bAutoDispute = ReadConfigBoolean("AutoDispute",true);

	string szKey = ReadConfigString("AutoSkillKey","");
	if(szKey.size() >= 2 && szKey[0] == 'F' && szKey[1]>='1' && szKey[1]<='8')
		m_cAutoSkillKey = 0x30+szKey[1]-'0';
	else
		m_cAutoSkillKey = 0;

	m_bAutoKillShow = ReadConfigBoolean("AutoKill",true);


	m_bIsShowBestItem = false; // 只显示极品否
	m_bIsAutoMagicAttack = false; //连续魔法攻击
	m_bIsRunAttack = false;    // 是否追杀

	m_bUsePTSuperHP = ReadConfigBoolean("UsePTSuperHP",false);//[20060630]
	m_bUsePTSuperMP = ReadConfigBoolean("UsePTSuperMP",false);//[20060630]

	m_bRejectBadItemWhenPickBestItem = ReadConfigBoolean("RejectBadItemWhenPickBestItem",false);


	return true;
}

bool CAIConfigMgr::LoadShortcutConfig()
{
	m_strKeyPath = StringUtil::format("%s\\config\\%s\\ShortCut.ini",GetGameDataDir(),m_id.c_str());

	for(int i=0;i<MAX_MAGIC_SKILL;i++)
	{
		CMagicData& temp = SELF.GetMagic(i);
		if(0 == temp.GetMagicID())
			continue;

		DWORD dwSize = 1024;
		char szReturn[1024] = {0};
		GetPrivateProfileString("ShortCut",temp.GetMagicName(),"",szReturn,dwSize,m_strKeyPath.c_str());
		string str;
		str.assign(szReturn);

		temp.DefaultAttr(); //设置默认的锁定，持续状态

		temp.SetShortCutKeyEx(0);

		int iLeftComma = str.find_first_of(',');
		if ( 1!= iLeftComma)
			continue;

		if(str.substr(0,1).compare("1") == 0)
			temp.AddAttr(MATTR_LOCK);
		else
			temp.RemoveAttr(MATTR_LOCK);

		iLeftComma = str.find_first_of(',',2);

		if (3 != iLeftComma)
			continue;

		if(str.substr(2,1).compare("1") == 0)
			temp.AddAttr(MATTR_REPEAT);
		else
			temp.RemoveAttr(MATTR_REPEAT);


		int iRightComma = str.find_first_of(',',iLeftComma+1);
		if(iRightComma>=0)
			iLeftComma = iRightComma;

		string strKey = str.substr(iLeftComma+1,str.size()-iLeftComma-1);
		int iAddPos = strKey.find_first_of("+");
		WORD wHighKey = 0;
		WORD wLowKey = 0;
		if(iAddPos>0)
		{
			if(stricmp(strKey.substr(0,iAddPos).c_str(),"Esc")==0)
				wHighKey = 1;
			strKey = strKey.substr(iAddPos+1,strKey.size()-iAddPos-1);
		}

		if(strKey.size()>=2 && strKey[0] == 'F' && strKey[1] >= '1' && strKey[1] <= '8')
			wLowKey = VK_F1+strKey[1]-'1';
		else if(strKey.size() == 1)
		{
			if(strKey[0]>='A' && strKey[0]<='Z')
				wLowKey = strKey[0];
			else if(strKey[0]>='a' && strKey[0]<='z')
				wLowKey = strKey[0]-'a'+'A';
		}
		temp.SetShortCutKeyEx(MAKELONG(wLowKey,wHighKey));
	}

	char szConSkillID[16] = {0};
	ConSkillMap& lConSkillMap = g_AIMgr.GetConSkills();
	for(ConSkillMap::iterator i=lConSkillMap.begin();i!=lConSkillMap.end();i++)
	{
		ConSkillData* lConSkillData = i->second;
		if (lConSkillData == 0)
			continue;

		lConSkillData->bActive = true;

		// 如果连击技能其中一个技能没有学会，则清除快捷键
		for (int isub = 0; isub < lConSkillData->nMagicCount;++isub)
		{
			ConSubSkill& kConSubSkill = lConSkillData->wMagicIDs[isub];
			CMagicData *pMagic = SELF.FindMagic(kConSubSkill.wMagicID);
			if (pMagic == 0 || pMagic->GetMagicLevel() < 4)
			{
				lConSkillData->dwKey = 0;
				lConSkillData->bActive = false;
				continue;
			}
		}

		// 读取快捷键
		DWORD dwSize = 1024;
		char szReturn[1024] = {0};
		sprintf(szConSkillID, "%d", lConSkillData->iConSkillID);
		GetPrivateProfileString("ConShortCut",szConSkillID,"",szReturn,dwSize,m_strKeyPath.c_str());
		string str;
		str.assign(szReturn);

		lConSkillData->dwKey = 0;

		if (!str.empty())
		{
			string strKey = str;
			int iAddPos = strKey.find_first_of("+");
			WORD wHighKey = 0;
			WORD wLowKey = 0;
			if(iAddPos>0)
			{
				if(stricmp(strKey.substr(0,iAddPos).c_str(),"Esc")==0)
					wHighKey = 1;
				strKey = strKey.substr(iAddPos+1,strKey.size()-iAddPos-1);
			}

			if(strKey.size()>=2 && strKey[0] == 'F' && strKey[1] >= '1' && strKey[1] <= '8')
				wLowKey = VK_F1+strKey[1]-'1';

			lConSkillData->dwKey = MAKELONG(wLowKey,wHighKey);
		}
	}

	//{
	//	// 法决
	//	// 读取快捷键
	//	DWORD dwSize = 1024;
	//	char szReturn[1024] = {0};
	//	sprintf(szConSkillID, "%d", 116);
	//	GetPrivateProfileString("YiHuoShortCut",szConSkillID,"",szReturn,dwSize,m_strKeyPath.c_str());
	//	string str;
	//	str.assign(szReturn);

	//	SELF.SetYiHuoShortCutKeyEx(0);

	//	if (!str.empty())
	//	{
	//		string strKey = str;
	//		int iAddPos = strKey.find_first_of("+");
	//		WORD wHighKey = 0;
	//		WORD wLowKey = 0;
	//		if(iAddPos>0)
	//		{
	//			if(stricmp(strKey.substr(0,iAddPos).c_str(),"Esc")==0)
	//				wHighKey = 1;
	//			strKey = strKey.substr(iAddPos+1,strKey.size()-iAddPos-1);
	//		}

	//		if(strKey.size()>=2 && strKey[0] == 'F' && strKey[1] >= '1' && strKey[1] <= '8')
	//			wLowKey = VK_F1+strKey[1]-'1';
	//		else if(strKey.size() == 1)
	//		{
	//			if(strKey[0]>='A' && strKey[0]<='Z')
	//				wLowKey = strKey[0];
	//			else if(strKey[0]>='a' && strKey[0]<='z')
	//				wLowKey = strKey[0]-'a'+'A';
	//		}
	//		SELF.SetYiHuoShortCutKeyEx(MAKELONG(wLowKey,wHighKey));
	//	}
	//}

	return true;
}

bool CAIConfigMgr::LoadReadmeConfig()
{
	//读取Readme文件
	m_readme.clear();

	DataStreamInterface * stream = g_pStreamMgr->OpenDataFile("config/readme.txt",false,true,false,EP_MOST_HIGH);
	if (!stream)
	{
		return false;
	}

	string str;
	while(!stream->eof())
	{
		str = stream->getLine(false);

		CutByUnicode(str.c_str(),m_readme);
	}

	SAFE_DELETE(stream);

	return true;
}


bool CAIConfigMgr::LoadMessageConfig()
{
	//读取Readme文件
	char szPath[1024] = {0};
	sprintf(szPath,"%s\\config\\%s\\msg.txt",GetGameDataDir(),GetIDPath());
	m_strRecordPath = szPath;

	g_TalkMgr.Clear(TALKTYPE_ASSPRIVATE);

	FILE* fp = fopen(m_strRecordPath.c_str(),"rt");
	if(fp == NULL)
		return false;

	int iTotal = 0;
	char szLine[2048] = {0};
	while(fgets(szLine,2000,fp) != NULL)
	{
		StringUtil::trim(szLine);
		g_TalkMgr.PushDataTalk(TALKTYPE_ASSPRIVATE,szLine,strlen(szLine),TALKCOLOR_WHITE); //插入数据
		iTotal++;
		memset(szLine,0,2048);
	}
	fclose(fp);

	//清除掉多余的部分，不然太大的话会导致程序崩溃
	int iSize = g_TalkMgr.GetAssPrivateTalk().size();
	if(iSize < iTotal)
	{
		fp = fopen(m_strRecordPath.c_str(),"wt");
		if(fp != NULL)
		{
			iTotal = iSize;
			for(int i = 0;i < iTotal;i++)
			{
				CStringLine* pLine = g_TalkMgr.GetAssPrivateTalk().at(i);
				if(pLine == NULL)
					continue;

				fprintf(fp,"%s\n",pLine->c_str());
			}
		}
		fclose(fp);
	}
	return true;
}

bool CAIConfigMgr::LoadHelpCfg(const char* path, map<std::string, std::string>& mapHelps)
{
	TiXmlDocument xmlDoc;
	if(!xmlDoc.LoadFile(path)) //读取失败
		return false;

	mapHelps.clear();

	TiXmlElement* rootNode = xmlDoc.RootElement();
	TiXmlElement* node = rootNode->FirstChildElement();


	for(;node;node = node->NextSiblingElement())
	{
		if(stricmp(node->Value(),"Helps") == 0)
		{
			TiXmlElement* child = node->FirstChildElement();
			for(;child;child = child->NextSiblingElement())
			{
				if(stricmp(child->Value(),"Help") == 0)
				{					
					string name;
					string val;

					for(TiXmlAttribute* attr = child->FirstAttribute();attr;attr = attr->Next())
					{
						if(stricmp(attr->Name(),"name") == 0)
						{
							name = attr->Value();
						}
						else if(stricmp(attr->Name(),"value") == 0)
						{
							val = attr->Value();
						}						
					}

					if (!name.empty())
					{						
						mapHelps[name] = val;
					}
				}
			}
		}
	}

	return true;
}

string CAIConfigMgr::GetHelpByName(map<std::string, std::string>& mapHelps, const string& name)
{
	if (name.empty())
		return "";

	map<std::string, std::string>::iterator i = mapHelps.find(name);
	if (i != mapHelps.end())
	{
		return i->second;
	}
	return "";
}

bool CAIConfigMgr::LoadFireLianHuaHelpCfg()
{
	char path[MAX_PATH] = {0};
	sprintf(path,"%s\\config\\FireLianHuaCfg.xml",GetGameDataDir());

	return LoadHelpCfg(path, m_kFireLianHuaHelps);
}

string CAIConfigMgr::GetFireLianHuaHelpByName(const string& name)
{
	return GetHelpByName(m_kFireLianHuaHelps, name);
}

bool CAIConfigMgr::LoadQiShuHelpCfg()
{
	char path[MAX_PATH] = {0};
	sprintf(path,"%s\\config\\QiShuHelpCfg.xml",GetGameDataDir());

	return LoadHelpCfg(path, m_kQiShuHelps);
}

string CAIConfigMgr::GetQiShuHelpByName(const string& name)
{
	return GetHelpByName(m_kQiShuHelps, name);
}

void  CAIConfigMgr::SetAutoFindWay(bool b)
{
	m_bIsAutoFindWay=b;
	WriteConfigBoolean("AutoFindWay",m_bIsAutoFindWay);
}

void  CAIConfigMgr::SetIsGoPickup(bool b)
{
	m_bIsGoPickup = b;
	WriteConfigBoolean("GoPickup",m_bIsGoPickup);
}

void CAIConfigMgr::SetShowObjectName(bool b)
{
	m_bIsShowObjectName = b;
	WriteConfigBoolean("ShowObjectName",b);
}

void CAIConfigMgr::SetShowPlayerName(bool b)
{
	m_bIsShowPlayerName = b;
	WriteConfigBoolean("ShowPlayerName",b);
}

void CAIConfigMgr::SetShowArrow(bool b)
{
	m_bIsShowArrow = b;
	WriteConfigBoolean("ShowArrow",b);
}

void CAIConfigMgr::SetShowSysInfo(bool b)
{
	m_bShowSysInfo = b;
	WriteConfigBoolean("ShowSysInfo",b);
}

void CAIConfigMgr::SetQuickOutGame(bool b)
{
	m_bIsQuickOutGame = b;
	WriteConfigBoolean("QuickOutGame",b);
}

void CAIConfigMgr::SetShowNpcPriority(bool b)
{
	m_bShowNpcPriority = b;
	WriteConfigBoolean("ShowNpcPriority",b);
}

void CAIConfigMgr::SetShowNewHandPrompt(bool b)
{
	m_bShowNewHandPrompt = b;
	WriteConfigBoolean("ShowNewHandPrompt",b);
}

void CAIConfigMgr::SetShowHpChangeEffect(bool b)
{
	m_bShowHpChangeEffect = b;
	WriteConfigBoolean("ShowHpChangeEffect",b);
}

void CAIConfigMgr::SetAcceptFrientAndTrade(bool b)
{
	m_bAcceptFrientAndTrade = b;
	WriteConfigBoolean("AcceptFrientAndTrade",b);
}

//void CAIConfigMgr::SetAutoFeed(bool b)
//{
//	m_bAutoFeed = b;
//	WriteConfigBoolean("AutoFeed",b);
//}

void CAIConfigMgr::SetAutoTeamVoice(bool b)
{
	m_bAutoTeamVoice = b;
	WriteConfigBoolean("AutoTeamVoice",b);
}

void CAIConfigMgr::SetAutoShowActLog(bool b)
{
	m_bAutoShowActLog = b;
	WriteConfigBoolean("AutoShowActLog",b);
}

void CAIConfigMgr::SetCloseTeam(bool b)
{
	m_bIsCloseTeam = b;
	WriteConfigBoolean("CloseTeam",b);
}

void CAIConfigMgr::SetDurationPrompt(bool b)
{
	m_bIsDurationPrompt = b;
	WriteConfigBoolean("DurationPrompt",b);
}

void CAIConfigMgr::SetTeamColor(bool b)
{
	m_bIsTeamColor = b;
	WriteConfigBoolean("TeamColor",b);
}

void CAIConfigMgr::SetDismount(bool b)
{
	m_bIsDismount = b;
	WriteConfigBoolean("Dismount",b);
}

void CAIConfigMgr::SetAutoPickup(bool b)
{
	m_bIsAutoPickup = b;
	WriteConfigBoolean("AutoPickup",b);
}

void CAIConfigMgr::SetShowMagicTime(bool b)
{
	m_bShowMagicTime = b;
	WriteConfigBoolean("MagicTime",b);
}

void CAIConfigMgr::SetBury(bool b)
{
	m_bIsBury = b;
	WriteConfigBoolean("Bury",b);
}

void CAIConfigMgr::SetShowExtendShortCut(bool b)
{
	m_bShowExtendShortCut = b;
	WriteConfigBoolean("ShowExtend",b);
}

void CAIConfigMgr::SetMountain(bool b)
{
	m_bIsMountain = b;
	WriteConfigBoolean("Mountain",b);
}

void CAIConfigMgr::SetStarObject(bool b)
{
	m_bIsStarObject = b;
	WriteConfigBoolean("StarObject",b);
}

void CAIConfigMgr::SetAutoMogong(bool b)
{
	m_bAutoMogong = b;
	WriteConfigBoolean("AutoMogong",b);
}

void CAIConfigMgr::SetAllowFly(bool b)
{
	m_bIsAllowFly = b;
	WriteConfigBoolean("AllowFly",b);
}

void CAIConfigMgr::SetRunNotStop(bool b)
{
	m_bRunNotStop = b;
	WriteConfigBoolean("RunNotStop",b);
}

void  CAIConfigMgr::SetAutoSteelProtect(bool v)
{
	m_bAutoSteelProtect=v;
	WriteConfigBoolean("AutoSteelProtect",m_bAutoSteelProtect);
}

void  CAIConfigMgr::SetAutoProtectSkin(bool v)
{
	m_bAutoProtectSkin=v;
	WriteConfigBoolean("AutoProtectSkin",m_bAutoProtectSkin);
}

void CAIConfigMgr::SetAutoFire(bool v)
{
	m_bAutoFire=v;
	WriteConfigBoolean("AutoFire",m_bAutoFire);
}

 void CAIConfigMgr::SetAutoSuperAttack(bool b)
 {
 	m_bAutoSuperAttack = b;
 	WriteConfigBoolean("AutoSuperAttack",m_bAutoSuperAttack);
 }

void CAIConfigMgr::SetAutoWildCollide(bool v)
{
	m_bAutoWildCollide=v;
	WriteConfigBoolean("AutoWildCollide",m_bAutoWildCollide);
}

void  CAIConfigMgr::SetDirWild(bool v)
{
	m_bDirWild=v;
	WriteConfigBoolean("DirWild",m_bDirWild);
}

void CAIConfigMgr::SetDoubleFire(bool v)
{
	m_bDoubleFire=v;
	WriteConfigBoolean("DoubleFire",m_bDoubleFire);
}

void CAIConfigMgr::SetDestroyShield(bool v)
{
	m_bDestroyShield=v;
	WriteConfigBoolean("DestroyShield",m_bDestroyShield);
}

void CAIConfigMgr::SetDestroyShell(bool v)
{
	m_bDestroyShell=v;
	WriteConfigBoolean("DestroyShell",m_bDestroyShell);
}

void CAIConfigMgr::SetAutoProtectGhost(bool v)
{
	m_bAutoProtectGhost=v;
	WriteConfigBoolean("AutoProtectGhost",m_bAutoProtectGhost);
}

void CAIConfigMgr::SetAutoProtectArmor(bool v)
{
	m_bAutoProtectArmor=v;
	WriteConfigBoolean("AutoProtectArmor",m_bAutoProtectArmor);
}

void  CAIConfigMgr::SetAutoLionCall(bool v)
{
	m_bAutoLionCall=v;
	WriteConfigBoolean("AutoLionCall",m_bAutoLionCall);
}

void  CAIConfigMgr::SetSmartLionCall(bool v)
{
	m_bSmartLionCall=v;
	WriteConfigBoolean("SmartLionCall",m_bSmartLionCall);
}

void  CAIConfigMgr::SetAutoMagicProtect(bool v)
{
	m_bAutoMagicProtect=v;
	WriteConfigBoolean("AutoMagicProtect",m_bAutoMagicProtect);
}

void  CAIConfigMgr::SetAutoDodgeSkill(bool v)
{
	m_bAutoDodgeSkill=v;
	WriteConfigBoolean("AutoDodgeSkill",m_bAutoDodgeSkill);
}

void  CAIConfigMgr::SetAutoDispute(bool v)
{
	m_bAutoDispute=v;
	WriteConfigBoolean("AutoDispute",m_bAutoDispute);
}
void  CAIConfigMgr::SetAntiAttack(bool v)
{
	m_bIsAntiAttack = v;
	WriteConfigBoolean("AntiAttack",v);
}

void  CAIConfigMgr::SetAutoSkill(bool v)
{
	m_bIsAutoSkill = v;
	WriteConfigBoolean("AutoSkill",v);
}
void  CAIConfigMgr::SetAutoSkillKey(char key)
{
	m_cAutoSkillKey = key;

	char str[64] = {0};
	if(key > 0)
		sprintf(str,"F%d",key - 0x30);
	else
		strcpy(str,"");

	WriteConfigString("AutoSkillKey",str);
}

void CAIConfigMgr::SetAutoSkillTime(int iTime)
{
	m_iAutoSkillTime = iTime;
	WriteConfigInt("AutoSkillTime",iTime);
}

void  CAIConfigMgr::SetAutoKillShow(bool v)
{
	m_bAutoKillShow = v;
	WriteConfigBoolean("AutoKill",v);
}

//保护
void  CAIConfigMgr::SetAutoQuit(bool b)
{
	m_bAutoQuit = b;
	WriteConfigBoolean("AutoQuit",b);
}

void  CAIConfigMgr::SetAutoQuitLimit(int i)
{
	m_iAutoQuitLimit = i;
	WriteConfigInt("AutoQuitLimit",i);
}

void  CAIConfigMgr::SetAutoAction(int i)
{
	m_iAutoAction = i;
	WriteConfigInt("AutoAction",i);
}

void  CAIConfigMgr::SetAutoActionLimit(int i)
{
	m_iAutoActionLimit = i;
	WriteConfigInt("AutoActionLimit",i);
}

void  CAIConfigMgr::SetBossPrompt(bool b)
{
	m_bIsBossPrompt = b;
	WriteConfigBoolean("BossAwoke",b);
}

void  CAIConfigMgr::SetObjectPrompt(bool b)
{
	m_bIsObjectPrompt = b;
	WriteConfigBoolean("BestItemAwoke",b);
}

void  CAIConfigMgr::SetPkAllAttack(bool b)
{
	m_bPkAllAttack = b;
	WriteConfigBoolean("PK_AllAttack",b);
}
void  CAIConfigMgr::SetBlackFilter(bool b)
{
	m_bIsBlackFilter = b;
	WriteConfigBoolean("BlackFilter",b);
}
void  CAIConfigMgr::SetPKAlarm(bool b)
{
	m_bIsPKAlarm = b;
	WriteConfigBoolean("PKAlarm",b);
}

void  CAIConfigMgr::SetAutoSay(bool b)
{
	m_bIsAutoSay = b;
	WriteConfigBoolean("AutoSayType",b);
}
void  CAIConfigMgr::SetAutoSayMsg(const char* str)
{
	m_auto_say = "";
	for(int i=0;i<strlen(str);i++)
	{
		if(str[i] !='\r' && str[i] !='\n')
		{
			m_auto_say+=str[i];
		}
	}
	WriteConfigString("AutoSay",str);
}

void  CAIConfigMgr::SetAutoReply(bool b)
{
	m_bAutoReply = b;
	WriteConfigBoolean("AutoReply",b);
}

void  CAIConfigMgr::SetAutoReplyMsg(const char* str)
{
	m_reply_msg.assign(str);

	WriteConfigString("AutoReplyMsg",str);
}

void  CAIConfigMgr::SetAutoSayTimer(int i)
{
	m_iAutoSayTimer = i;
	WriteConfigInt("AutoSayTime",i);
}

void  CAIConfigMgr::SetRecordMsg(bool b)
{
	m_bRecordMsg = b;
	WriteConfigBoolean("RecordMsg",b);
}

void  CAIConfigMgr::SetMessageAwoke(bool b)
{
	m_bMessageAwoke = b;
	WriteConfigBoolean("MessageAwoke",b);
}

void CAIConfigMgr::SetKeepOutInfo(bool b)
{
	m_bKeepOutInfo = b;
	WriteConfigBoolean("KeepOutInfo",b);
}

void  CAIConfigMgr::SetBestItemColor(DWORD dwColor)
{
	m_dwBestItemColor = dwColor;
	WriteConfigColor("BestItemColor",dwColor);
}
void  CAIConfigMgr::SetExpensiveItemColor(DWORD dwColor)
{
	m_dwExpensiveItemColor = dwColor;
	WriteConfigColor("ExpensiveItemColor",dwColor);

}
void  CAIConfigMgr::SetPickItemColor(DWORD dwColor)
{
	m_dwPickItemColor = dwColor;
	WriteConfigColor("PickItemColor",dwColor);

}
void  CAIConfigMgr::SetShowItemColor(DWORD dwColor)
{
	m_dwShowItemColor = dwColor;
	WriteConfigColor("ShowItemColor",dwColor);
}

void CAIConfigMgr::SetUsePTSuperHP(bool b)
{
	m_bUsePTSuperHP = b;
	WriteConfigBoolean("UsePTSuperHP",b);
}

void CAIConfigMgr::SetUsePTSuperMP(bool b)
{
	m_bUsePTSuperMP  = b;
	WriteConfigBoolean("UsePTSuperMP",b);
}

void  CAIConfigMgr::SetRejectBadItemWhenPickBestItem(bool val) 
{
	m_bRejectBadItemWhenPickBestItem = val;
	WriteConfigBoolean("RejectBadItemWhenPickBestItem",val);
}

void CAIConfigMgr::AddPrivateMsg(const char* strMsg,int iLen,bool bNeedClearWords)
{
	g_TalkMgr.PushDataTalk(TALKTYPE_ASSPRIVATE,strMsg,iLen,0xFFFC,FONT_SONGTI,FONTSIZE_DEFAULT,true,bNeedClearWords); //插入数据

	FILE * fp = fopen(m_strRecordPath.c_str(),"a+");
	if(fp == NULL)
		return;

	fwrite(strMsg,strlen(strMsg),1,fp);
	fwrite("\n",strlen("\n"),1,fp);
	fclose(fp);
}

void CAIConfigMgr::ClearPrivateMsg()
{
	g_TalkMgr.GetAssPrivateTalk().clear();

	FILE * fp = fopen(m_strRecordPath.c_str(),"wt");
	if(fp == NULL)
		return;
	fprintf(fp,"");
	fclose(fp);
}

string CAIConfigMgr::GetReadme(int i)
{
	if(i >= 0 && i < (int)m_readme.size())
		return m_readme[i];
	else
		return "";
}

bool CAIConfigMgr::SaveShortCutConfig()
{
	m_strKeyPath = StringUtil::format("%s\\config\\%s\\ShortCut.ini",GetGameDataDir(),m_id.c_str());

	char szBuffer[4096] = {0};
	char* szBuf = szBuffer;

	for(int i=0;i<MAX_MAGIC_SKILL;i++)
	{
		CMagicData& temp = SELF.GetMagic(i);
		if(temp.GetMagicID() == 0)
			continue;

		string szMagicName = temp.GetMagicName();
		sprintf(szBuf,"%s=%d,%d,",szMagicName.c_str(),temp.HasAttr(MATTR_LOCK)?1:0,temp.HasAttr(MATTR_REPEAT)?1:0);
		szBuf+=szMagicName.size() + 5;

		char cKey = temp.GetShortCutKey();
		if(cKey>0)
		{
			cKey-=0x30;
			sprintf(szBuf,"F%d",cKey);
			szBuf+=2;
		}
		*szBuf = ',';
		szBuf++;

		string strExKey = GetShortCutKeyExStr(temp.GetShortCutKeyEx());
		strcpy(szBuf,strExKey.c_str());
		szBuf += strExKey.length();

		*szBuf=0;
		szBuf++;
	}
	*szBuf = 0;
	WritePrivateProfileSection("ShortCut",szBuffer,m_strKeyPath.c_str());

	memset(szBuffer, 0, 4096);
	szBuf = szBuffer;

	char sztemp[64] = {0};
	ConSkillMap& lConSkillMap = g_AIMgr.GetConSkills();
	for(ConSkillMap::iterator i=lConSkillMap.begin();i!=lConSkillMap.end();i++)
	{
		ConSkillData* lConSkillData = i->second;
		if (lConSkillData == 0 || lConSkillData->dwKey == 0)
			continue;

		sprintf(sztemp, "%d", lConSkillData->iConSkillID);		
		sprintf(szBuf,"%s=",sztemp);
		szBuf+=strlen(sztemp) + 1;


		WORD wLowKey = LOWORD(lConSkillData->dwKey);
		if(HIWORD(lConSkillData->dwKey) == 1)
		{
			if(wLowKey>=VK_F1 && wLowKey<=VK_F8)
			{
				sprintf(sztemp,"Esc+F%d",wLowKey-VK_F1+1);
			}
		}
		else
		{
			if(wLowKey>=VK_F1 && wLowKey<=VK_F8)
			{
				sprintf(sztemp,"F%d",wLowKey-VK_F1+1);
			}
		}

		strcpy(szBuf,sztemp);
		szBuf += strlen(sztemp);

		*szBuf=0;
		szBuf++;
	}

	*szBuf = 0;
	WritePrivateProfileSection("ConShortCut",szBuffer,m_strKeyPath.c_str());


	memset(szBuffer, 0, 4096);
	szBuf = szBuffer;
	//法决
	/*if (SELF.GetYiHuoShortCutKeyEx() != 0)	
	{
		sprintf(sztemp, "%d", 116);		
		sprintf(szBuf,"%s=",sztemp);
		szBuf+=strlen(sztemp) + 1;		

		string strExKey = GetShortCutKeyExStr(SELF.GetYiHuoShortCutKeyEx());
		strcpy(szBuf,strExKey.c_str());
		szBuf += strExKey.length();

		*szBuf=0;
	}*/
	
	//WritePrivateProfileSection("YiHuoShortCut",szBuffer,m_strKeyPath.c_str());


	return true;
}

string CAIConfigMgr::GetShortCutKeyExStr(DWORD dwKey)
{
	char str[64]={0};
	WORD wLowKey = LOWORD(dwKey);
	if(HIWORD(dwKey) == 1)
	{
		if(wLowKey>=VK_F1 && wLowKey<=VK_F8)
		{
			sprintf(str,"Esc+F%d",wLowKey-VK_F1+1);
		}
	}
	else
	{
		if(wLowKey>='A' && wLowKey <='Z')
		{
			sprintf(str,"%c",wLowKey);
		}
	}
	return str;
}

//自动金刚
bool  CAIConfigMgr::IsAutoSteelProtect()
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoSteelProtect;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoSteelProtect;
}
//自动护身真气
bool  CAIConfigMgr::IsAutoProtectSkin() 
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoProtectSkin;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoProtectSkin; 
} 
//自动烈火
bool  CAIConfigMgr::IsAutoFire()        
{
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoFire;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoFire;        
} 
//自动强化攻杀
bool  CAIConfigMgr::IsAutoSuperAttack() 
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoSuperAttack;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoSuperAttack; 
} 

//自动突斩
bool  CAIConfigMgr::IsAutoWildCollide() 
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoSuddenKill;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoWildCollide; 
} 
//定向突斩
bool  CAIConfigMgr::IsDirWild()         
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bDirWild;
// 		}
// 
// 		return false;
// 	}

	return m_bDirWild;        
}
//自动双烈火
bool  CAIConfigMgr::IsDoubleFire()      
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoDoubleFire;
// 		}
// 
// 		return false;
// 	}

	return m_bDoubleFire;      
} 
//持续破盾
bool  CAIConfigMgr::IsDestroyShield()   
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bDestroyShield;
// 		}
// 
// 		return false;
// 	}

	return m_bDestroyShield;  
} 
//持续破击
bool  CAIConfigMgr::IsDestroyShell()    
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bDestroyShell;
// 		}
// 
// 		return false;
// 	}

	return m_bDestroyShell;   
} 
//持续幽灵盾
bool  CAIConfigMgr::IsAutoProtectGhost()
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoProtectGhost;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoProtectGhost;
} 
//持续神圣战甲术
bool  CAIConfigMgr::IsAutoProtectArmor()
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoProtectArmor;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoProtectArmor;
} 
//自动狮子吼
bool  CAIConfigMgr::IsAutoLionCall()    
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoLionCall;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoLionCall;    
} 
//定向狮子吼
bool  CAIConfigMgr::IsSmartLionCall()   
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bSmartLionCall;
// 		}
// 
// 		return false;
// 	}

	return m_bSmartLionCall;  
} 
//持续魔法盾
bool  CAIConfigMgr::IsAutoMagicProtect()
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoMagicProtect;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoMagicProtect;
} 
//持续风影盾
bool  CAIConfigMgr::IsAutoDodgeSkill()  
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{		
// 		return true;
// 	}

	return m_bAutoDodgeSkill; 
} 
//自动抗拒
bool  CAIConfigMgr::IsAutoDispute()    
{ 
// 	if (g_pSelf != &(ORIGINALSELF) || (g_TrusteeshipData.IsTrusteeship() && !g_TrusteeshipData.IsCaptain()))//队长微操及附身模式或队员托管模式
// 	{
// 		TneupMember *pSelf = g_TrusteeshipData.GetSelf();
// 		if (pSelf)
// 		{
// 			return pSelf->fightConfig.bAutoDispute;
// 		}
// 
// 		return false;
// 	}

	return m_bAutoDispute;    
} 
