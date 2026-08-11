#include "SDSamplerMgr.h"
#include "Global/Interface/StreamInterface.h"
#include "GameData/LoginData.h"
#include "GameData/Character.h"
#include "GameData/GameData.h"
#include "GameData/GameGlobal.h"
#include "Global/ShareMem.h"
#include "GameData/OtherData.h"


CSDSamplerMgr g_SdSamplerMgr;


CSDSamplerMgr::CSDSamplerMgr(void)
{
}

CSDSamplerMgr::~CSDSamplerMgr(void)
{
}

bool CSDSamplerMgr::SendGameVersionSample()
{
	if (!g_pSDSampler)
		return false;

	g_pSDSampler->SetHead(WS_GAME_ID,g_Login.GetAreaNo(),g_Login.GetGroupNo(),g_Login.GetLoginID());

#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::GameVersion GameVersionInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	t1.GameVersionInfo.GameId = WS_GAME_ID;
	t1.GameVersionInfo.GameArea = g_Login.GetAreaNo();
	t1.GameVersionInfo.GameGroup = g_Login.GetGroupNo();
	t1.GameVersionInfo.ClientType = 1;
	strcpy(t1.GameVersionInfo.Version,g_strVersion);
	DWORD dwCode = g_pStreamMgr->GetOtherConfigInt("SubType",0);
	ltoa(dwCode,t1.GameVersionInfo.VersionType,10);

	t1.ilen = sizeof(SampleData);
	t1.iMap = 63;
	t1.iType = SAMPLE_GAMEVERSION;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_GAMEVERSION, (char*)&t1, sizeof(SampleData));

	return true;
}

bool CSDSamplerMgr::SendPlayerInfoSample()
{
	if (!g_pSDSampler)
		return false;
#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::Player PlayerInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	t1.PlayerInfo.Level = SELF.GetLevel();
	strcpy_s(t1.PlayerInfo.ID,64,g_Login.GetLoginID());
	strcpy_s(t1.PlayerInfo.RoleName,64,SELF.GetName());
	strcpy_s(t1.PlayerInfo.Race ,32,GetJobName(SELF.GetCareer()));
	strcpy_s(t1.PlayerInfo.Gender,4, SELF.GetSex() == 0?"M":"F");

	output_debug("%s,%s,%s,%s",t1.PlayerInfo.ID,t1.PlayerInfo.RoleName,t1.PlayerInfo.Race,t1.PlayerInfo.Gender);

	t1.ilen = sizeof(SampleData);
	t1.iMap = 31;
	t1.iType = SAMPLE_GAMEINFO;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_GAMEINFO, (char*)&t1, sizeof(SampleData));

	return true;
}

bool CSDSamplerMgr::SendSocietyInfoSample()
{
	if (!g_pSDSampler)
		return false;

	int iRalationNnm[4];
	memset(iRalationNnm,0,sizeof(int) * 4);

	for(UINT i = 0; i < g_OtherData.GetRelationVector().size();i++)
	{		
		DWORD dwMyRelationType = g_OtherData.GetRelationVector()[i].iRelType;

		if (dwMyRelationType & RT_FRIEND)
		{
			iRalationNnm[0] ++;
		}
		else if (dwMyRelationType & RT_MASTER)
		{
			iRalationNnm[1] ++;
		}
		else if (dwMyRelationType & RT_PRENTICE)
		{
			iRalationNnm[2] ++;
		}
		else if (dwMyRelationType & RT_WIFE || dwMyRelationType & RT_HUSBAND)
		{
			iRalationNnm[3] ++;
		}
	}
#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::SocialRelations SocialRelationsInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	strcpy_s(t1.SocialRelationsInfo.Friends,64,iRalationNnm[0] > 0?"Yes":"No");
	strcpy_s(t1.SocialRelationsInfo.Guild,64,SELF.GetGuildTitle());
	strcpy_s(t1.SocialRelationsInfo.Team,64,g_pGameData->GetTroopMembers() > 0?"Yes":"No");
	strcpy_s(t1.SocialRelationsInfo.Teacher,64,(iRalationNnm[1] > 0 || iRalationNnm[2] > 0)?"Yes":"No");
	strcpy_s(t1.SocialRelationsInfo.Marriage,64,iRalationNnm[3] > 0?"Yes":"No");

	t1.ilen = sizeof(SampleData);
	t1.iMap = 31;
	t1.iType = SAMPLE_SOCIETY;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_SOCIETY, (char*)&t1, sizeof(SampleData));

	return true;
}

bool CSDSamplerMgr::SendPlayerActionSample()
{
	if (!g_pSDSampler || g_OtherData.GetEnterGameTime() == 0)
		return false;
#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::Action::PlayerAction PlayerActionInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	t1.PlayerActionInfo.Duration = GetTickCount() - g_OtherData.GetEnterGameTime();

	t1.ilen = sizeof(SampleData);
	t1.iMap = 32;
	t1.iType = SAMPLE_ACTION;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_ACTION, (char*)&t1, sizeof(SampleData));

	return true;
}

bool CSDSamplerMgr::SendClientInfoSample()
{
	if (!g_pSDSampler)
		return false;
#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::GameSetting::ClientStartup ClientStartupInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	int iScreenMode = 0;
	char* sRet = g_pStreamMgr->GetConfigStr("FullScreen","Yes");
	if(stricmp(sRet,"Yes") != 0)
		iScreenMode = 1;

	t1.ClientStartupInfo.ScreenMode = iScreenMode;

	int vApps[3];
	g_ShareMem.GetShare(vApps,sizeof(vApps));
	t1.ClientStartupInfo.Double = vApps[0] > 1?1:0;

	if (g_pSDOAInterface && g_Login.IsSDOALogin())
	{
		strcat_s(t1.ClientStartupInfo.ExternalSoftware,64,"SDOA,");
	}
	strcat_s(t1.ClientStartupInfo.ExternalSoftware,64,"SDError,");
	strcat_s(t1.ClientStartupInfo.ExternalSoftware,64,"RainBowSpirt");
	if(g_pGfx->GetWidth() == 800)
		strcpy_s(t1.ClientStartupInfo.Resolution,16,"800*600");
	else if(g_pGfx->GetWidth() == 1024)
		strcpy_s(t1.ClientStartupInfo.Resolution,16,"1024*768");
	else
		strcpy_s(t1.ClientStartupInfo.Resolution,16,"1280*800");
	
	TCHAR szPath[MAX_PATH] = {0};
	_fullpath(szPath,"../",MAX_PATH);

	strcpy_s(t1.ClientStartupInfo.InstallPath,128,szPath);
	strcpy_s(t1.ClientStartupInfo.RegistryIntegrity,128,"Yes");

	t1.ilen = sizeof(SampleData);
	t1.iMap = 63;
	t1.iType = SAMPLE_CLIENTINFO;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_CLIENTINFO, (char*)&t1, sizeof(SampleData));

	return true;
}

bool CSDSamplerMgr::SendQualitySample()
{
	if (!g_pSDSampler)
		return false;
#pragma pack(1)
	struct SampleData
	{
		short ilen;			
		int  iMap;			
		UINT iType;			
		short iVersion;		
		InGameInfo::Quality QualityInfo;		
	};
#pragma pack()
	SampleData t1;
	memset(&t1, 0, sizeof(SampleData));

	DWORD dwDelay = 0;
	if (SELF.GetReserveData(plyNoWaitServerTimes) > 0)
	{
		dwDelay = (DWORD)((double)SELF.GetReserveData(plyWaitServerTimes) / SELF.GetReserveData(plyNoWaitServerTimes) * 10000);
	}

	t1.QualityInfo.NetworkDelay = dwDelay;
	t1.QualityInfo.StartUp = g_dwLoadTime;
	int iFrameRate = 0;
	if (g_pGfx->GetFrameCount() > 0)
	{
		iFrameRate = (int)((double)g_pGfx->GetTotalFPS() / g_pGfx->GetFrameCount());
	}
	t1.QualityInfo.RenderingFrameRate = iFrameRate;


	t1.ilen = sizeof(SampleData);
	t1.iMap = 13;
	t1.iType = SAMPLE_GAMEQUALITY;      
	t1.iVersion = 1;

	g_pSDSampler->LogStruct(SAMPLE_GAMEQUALITY, (char*)&t1, sizeof(SampleData));

	return true;
}
