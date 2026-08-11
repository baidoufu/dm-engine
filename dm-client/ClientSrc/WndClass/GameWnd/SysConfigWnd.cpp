#include "sysconfigwnd.h"
#include "GameControl/GameControl.h"
#include "GameData/GameData.h"
#include "BaseClass/Control/ParserTip.h"
#include <Shellapi.h>
#include "Global/Interface/StreamInterface.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/MsgBoxMgr.h"
#include "Global/Interface/WeatherInterface.h"
#include "GameData/ConfigData.h"
#include "GameClient/SDOAInterface.h"
#include "GameMap/GameMap.h"
#include "GameData/OtherData.h"
#include "GameData/MagicCtrlMgr.h"

#define CONFIGPAGECOUNT 5
#define CPAGEINTERNAL  68

#define MASTERCFGSY   60
#define MASTERCFGEY   170

#define MERCFGSY   180
#define MERCFGEY   285

#define SYSCONFIGSY	   293
#define SYSCONFIGEY	   408

#define CONFIGPAGEINTERNAL 69

#define MAX_TRANS 100
#define MAX_SOUND 100

#define MPAGEINTERNAL  68
#define MERPAGECOUNT 3
#define MASTERCFGSX   22
#define MASTERCFGSY   60
#define MASTERCFGEX   50
#define MASTERCFGEY   170

INIT_WND_POSX(CSysConfigWnd,POS_AUTO,POS_AUTO)

CSysConfigWnd::CSysConfigWnd(void)
{
	m_pRD1024 = NULL;
 	m_pRD800  = NULL; 
	m_pRD1280 = NULL;
	m_pRD1600 = NULL;
	m_pRD1920 = NULL;
	m_pRD2560 = NULL;
	m_pRDWinMode = NULL;
	m_pRDScreenMode = NULL;
	m_pRDSpeed = NULL;
	m_pRDHalfSpeed = NULL;
	//m_pRDHighQ = NULL;
	//m_pRDLowQ = NULL;
	m_pRD32Bit = NULL;
	m_pRD16Bit = NULL;
	//m_pRDVSynOpen = NULL;
	//m_pRDVSynClose = NULL;
	m_pRDCursorOpen = NULL;
	m_pRDCursorClose = NULL;
	m_pRDOpenShine = NULL;
	m_bIsTransparence =false;

	//g_TempConfig.Set3DMode		( g_Config.Get3DMode()		);
	g_TempConfig.SetWindowWidth (g_Config.GetWindowWidth());
	g_TempConfig.SetHalfSpeed	( g_Config.GetHalfSpeed()	);


	g_TempConfig.SetAllWndMode	( g_Config.GetAllWndMode()	);
	g_TempConfig.SetSelfAlpha	( g_Config.GetSelfAlpha()	);
	g_TempConfig.SetAutoPath	( g_Config.GetAutoPath()	);
	g_TempConfig.SetColorHP		( g_Config.GetColorHP()		);
	g_TempConfig.SetTexPrepare  ( g_Config.GetTexPrepare()  );
	g_TempConfig.SetSmooth		( g_Config.GetSmooth()		);
	g_TempConfig.SetLight		( g_Config.GetLight()		);
	g_TempConfig.SetRGB			( g_Config.GetRGB()			);
	//g_TempConfig.SetVSync		( g_Config.GetVSync()		);
	g_TempConfig.SetMouse		( g_Config.GetMouse()		);
	g_TempConfig.SetTrans		( g_Config.GetTrans()		);
	g_TempConfig.SetWeather		( g_Config.GetWeather()		);
	//if( g_Config.Get3DMode() )
	//	g_TempConfig.SetCanTrans(true);
	//else
	//	g_TempConfig.SetCanTrans(false);

	//个性化设置
	g_TempConfig.SetSndOn		( g_Config.GetSndOn()		);
	g_TempConfig.SetSndVol		( g_Config.GetSndVol()		);
	g_TempConfig.SetFloodOn		( g_Config.GetFloodOn()		);
	g_TempConfig.SetRainbowOn   ( g_Config.GetRainbowOn()   );
	g_TempConfig.SetOpenShine	( g_Config.GetOpenShine()	);

	g_TempConfig.SetBkMusicOn	( g_Config.GetBkMusicOn()	);
	g_TempConfig.SetBkMusicVol	( g_Config.GetBkMusicVol()	);
	
	g_TempConfig.SetHusbandAndWife(g_Config.GetHusbandAndWife());
	g_TempConfig.SetShaMember(g_Config.GetShaMember());
	g_TempConfig.SetOpenWing(g_Config.GetOpenWing());
	g_TempConfig.SetTempMask(g_Config.GetTempMask());
	g_TempConfig.SetShowUIHelp(g_Config.GetShowUIHelp());

	g_TempConfig.SetTitleStr(g_Config.GetTitleStr());
	string str = g_Config.GetTempMaskStr();
	int iPos = str.find_first_of('/');
	if(iPos < 0) iPos = str.find_first_of('\\');

	if(iPos > 0 && str.size() > 0)
		str = str.substr(0,iPos);
	g_TempConfig.SetTempMaskStr(str.c_str());

	m_EutUiType = g_EutUiType;

	//m_pArrow = g_pTexMgr->LoadTex(PACKAGE_INTERFACE,138);
	//m_p3D = g_pTexMgr->LoadTex(PACKAGE_INTERFACE,10051);


	m_iIndex = 10179;
	m_iPages = 6;
	m_iCurPage = 0;

	m_bIsVol = false;
	m_bIsBkVol = false;


	//新版页签
	m_iVersion = 2;
	S_TabPage* pGameConfigPage = AddTabPage(0,0,MAKELONG(10125,PACKAGE_INTERFACE),0,122,85,86,87,88,"系统设置",NULL,true,0,0,1);

	AddTabPage(0,0,MAKELONG(10125,PACKAGE_INTERFACE),0,0,0,0,0,0,"",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(10125,PACKAGE_INTERFACE),47,36,120,121,122,123,"图像设置",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(10126,PACKAGE_INTERFACE),118,36,120,121,122,123,"声音设置",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(10127,PACKAGE_INTERFACE),190,36,120,121,122,123,"游戏功能",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(10128,PACKAGE_INTERFACE),262,36,120,121,122,123,"客服功能",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);
	AddTabPage(0,0,MAKELONG(10129,PACKAGE_INTERFACE),334,36,120,121,122,123,"个性化设置",pGameConfigPage,false,0,0,0,FONT_YAHEI,FONTSIZE_SMALL);

	AddTabPage(0,0,MAKELONG(10125,PACKAGE_INTERFACE),0,37,85,86,87,88,"游戏设置",NULL,true,0,0,1);

}

CSysConfigWnd::~CSysConfigWnd(void)
{
	//if(m_pArrow)
	//{
	//	g_pTexMgr->ReleaseTex(m_pArrow);
	//	m_pArrow = NULL;
	//}

	//if(m_p3D)
	//{
	//	g_pTexMgr->ReleaseTex(m_p3D);
	//	m_p3D = NULL;
	//}

	// 窗口透明度
	{		
		g_pStreamMgr->SetConfigInt("Trans",g_Config.GetTrans());		
	}

	// 音效开关
	{
		if( g_Config.GetSndOn() )
		{
			g_pStreamMgr->SetConfigStr("Sound","Yes");

			if(g_TempConfig.GetWeather())
			{
				if(g_pWeather->GetParticleType() == PARTICLE_RAIN)
				{
					g_pAudio->Stop(EAT_OTHER,257,g_pAudio->GetAudioRain());
					g_pAudio->GetAudioRain() = g_pAudio->GetRand();
					g_pAudio->Play(EAT_OTHER,256,g_pAudio->GetRand()++,true);
				}
				else if(g_pWeather->GetParticleType() == PARTICLE_STORM)
				{
					g_pAudio->Stop(EAT_OTHER,256,g_pAudio->GetAudioRain());
					g_pAudio->GetAudioRain() = g_pAudio->GetRand();
					g_pAudio->Play(EAT_OTHER,257,g_pAudio->GetRand()++,true);
				}
			}
		}
		else
		{
			g_pStreamMgr->SetConfigStr("Sound","No");
			
		}		
	}

	// 音量	
	{ 		
		g_pStreamMgr->SetConfigInt("SoundVol",g_Config.GetSndVol());		
	}

	{
		if( g_Config.GetBkMusicOn() )
		{
			g_pStreamMgr->SetConfigStr("BkSound","Yes");			
		}
		else
		{
			g_pStreamMgr->SetConfigStr("BkSound","No");			
		}		
	}

	
	{ 		
		g_pStreamMgr->SetConfigInt("BkSoundVol",g_Config.GetBkMusicVol());		
	}
}

bool CSysConfigWnd::OnMouseMove(int iX,int iY)
{
	int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;

	if(iCurPage == PAGE_PICTURE)
	{
		if(m_bIsTransparence)
		{
			if(iX > 240 && iY > 290 && iX < 355 && iY < 310)
			{
				m_iTransparence = iX - 246;
				if(m_iTransparence < 0)
					m_iTransparence = 0;
				if(m_iTransparence > MAX_TRANS)
					m_iTransparence = MAX_TRANS;				
			}
			return true;			
		}
	}
	else if(iCurPage == PAGE_SOUND)
	{
		if(m_bIsVol)
		{
			if(iX >= 285 && iY > 138 && iX < 386 && iY < 153)
			{
				m_iSndVol = iX - 285;
				if(m_iSndVol < 0)
					m_iSndVol =0;
				if(m_iSndVol > MAX_SOUND)
					m_iSndVol = MAX_SOUND;
			}
			return true;
		}

		if (m_bIsBkVol)
		{
			if(iX >= 285 && iY > 138 + 150 && iX < 386 && iY < 153 + 150)
			{
				m_iBkSndVol = iX - 285;
				if(m_iBkSndVol < 0)
					m_iBkSndVol =0;
				if(m_iBkSndVol > MAX_SOUND)
					m_iBkSndVol = MAX_SOUND;
			}
			return true;
		}
	}

	return CCtrlWindowX::OnMouseMove(iX,iY);
}

void CSysConfigWnd::Draw(void)
{
	CCtrlWindowX::Draw();
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}
	int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;
	//////////////////////////////////////////////
	g_pFont->DrawText(m_iScreenX + 276,m_iScreenY + 10,"系统设置",COLOR_TEXT_MAIN_TITLE,FONT_YAHEI,FONTSIZE_BIG,DTF_Center);

	if (iCurPage == 0)
	{
		g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 102,"尊敬的用户：",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
	 	g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 132,"      您好，传奇世界分别提供了图像、声音设置以及游戏功能选项供",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 152,"您自由配置，并提供多个客户服务专区方便您的连接。您可以通过点击",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 172,"上方的各项功能按钮，根据自己的喜好和机器硬件设备的具体情况加以",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 192,"设置，以便达到您需要的游戏效果。如果您尚未做过调节，游戏将使用",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 82,m_iScreenY + 212,"默认配置。",0xFFFFFF00,FONT_YAHEI,FONTSIZE_SMALL);
	}
	else if(iCurPage == PAGE_PICTURE)
	{
		int iStartY = 94;
		int iSPan = 18;

		if (!g_hParentWnd)
		{
			g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY,"分辨率",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		}
		g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 2,"窗口模式",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 3,"刷新速度",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 4,"界面设定：",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		//g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 3,"图像质量",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		
		iStartY = 182;
		//g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY,"色彩模式",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		//g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 1,"垂直刷新",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + iStartY + iSPan * 1,"自绘鼠标",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
	
		g_pFont->DrawText(m_iScreenX + 55,m_iScreenY + 292,"窗口透明度",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

		m_pRD1280->SetChecked(m_iWindowWidth == 1280);
		m_pRD1600->SetChecked(m_iWindowWidth == 1600);
		m_pRD1920->SetChecked(m_iWindowWidth == 1920);
		m_pRD2560->SetChecked(m_iWindowWidth == 2560);
		m_pRD1024->SetChecked(m_iWindowWidth == 1024);
 		m_pRD800->SetChecked(m_iWindowWidth == 800);
		m_pRDWinMode->SetChecked(m_bAllWndMode);
		m_pRDScreenMode->SetChecked(!m_bAllWndMode);
		m_pRDHalfSpeed->SetChecked(m_bHalfSpeed);
		m_pRDSpeed->SetChecked(!m_bHalfSpeed);

		//m_pRDHighQ->SetChecked(m_b3DMode);
		//m_pRDLowQ->SetChecked(!m_b3DMode);

		//m_pRD32Bit->SetChecked(m_iRGB32 == 1);
		//m_pRD16Bit->SetChecked(m_iRGB32 == 0);
		//m_pRDVSynOpen->SetChecked(m_bVSync);
		//m_pRDVSynClose->SetChecked(!m_bVSync);
		m_pRDCursorOpen->SetChecked(m_iMouse == 1);
		m_pRDCursorClose->SetChecked(m_iMouse == 0);

		m_pClassicUI->SetChecked(m_EutUiType == EUT_CLASSIC);
		m_pFashionUi->SetChecked(m_EutUiType == EUT_FASHION);

		//m_pRD32Bit->SetShow(m_b3DMode);
		//m_pRD16Bit->SetShow(m_b3DMode);
		//m_pRDVSynOpen->SetShow(m_b3DMode);
		//m_pRDVSynClose->SetShow(m_b3DMode);
		//m_pRDCursorOpen->SetShow(m_b3DMode);
		//m_pRDCursorClose->SetShow(m_b3DMode);

		char ch[26] = {0};
		g_pGfx->DrawTextureNL(m_iScreenX + 234, m_iScreenY + 290, g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,130,EP_UI));
		g_pGfx->DrawTextureNL(m_iScreenX + 236 + m_iTransparence, m_iScreenY + 290, g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,138,EP_UI));
		sprintf(ch,"%d",m_iTransparence);
		g_pFont->DrawText(m_iScreenX + 360,m_iScreenY + 292 ,ch,0xFFFFFF00);

		if(m_iTransparence != g_Config.GetTrans())
		{
			g_Config.SetTrans(m_iTransparence);

			DWORD dw = m_iTransparence;
			if(dw<0)  dw = 0;
			if(dw>50) dw = 50;
			dw = (( (50-dw+25) * 0xFF / 75 ) << 24) | 0x00FFFFFF;
			g_Config.SetTransColor(dw);
		}
	}
	else if(iCurPage == PAGE_SOUND)
	{
		m_pRDSoundOn->SetChecked(m_bSndOn);
		m_pRDSoundOff->SetChecked(!m_bSndOn);

		m_pBkMusicOn->SetChecked(m_bBkSndOn);
		m_pBkMusicOff->SetChecked(!m_bBkSndOn);

// 		m_pRDNearVoiceOn->SetChecked(m_bNearVoice);
// 		m_pRDNearVoiceOff->SetChecked(!m_bNearVoice);
// 
// 		m_pRDTeamVoiceOn->SetChecked(m_bTeamVoice);
// 		m_pRDTeamVoiceOff->SetChecked(!m_bTeamVoice);
// 
// 		m_pRDMicroPhoneOn->SetChecked(m_bMicroPhone);
// 		m_pRDMicroPhoneOff->SetChecked(!m_bMicroPhone);
// 
// 		m_pRDNearVoiceOn->SetShow(false);
// 		m_pRDNearVoiceOff->SetShow(false);
// 		m_pRDTeamVoiceOn->SetShow(false);
// 		m_pRDTeamVoiceOff->SetShow(false);
// 		m_pRDMicroPhoneOn->SetShow(false);
// 		m_pRDMicroPhoneOff->SetShow(false);

		g_pFont->DrawText(m_iScreenX + 50,m_iScreenY + 99,"音效",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 50,m_iScreenY + 142,"音效音量调节",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

		g_pGfx->DrawTextureNL(m_iScreenX + 277 ,m_iScreenY + 140 ,g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,130,EP_UI));
		g_pGfx->DrawTextureNL(m_iScreenX + 279 + m_iSndVol ,m_iScreenY + 140 ,g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,138,EP_UI));


		int iVolum = g_pAudio->GetVolumeMusic();
		g_pFont->DrawText(m_iScreenX + 50,m_iScreenY + 99 + 150,"背景音效",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 50,m_iScreenY + 142 + 150,"背景音效音量调节",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

		g_pGfx->DrawTextureNL(m_iScreenX + 277 ,m_iScreenY + 140 + 150 ,g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,130,EP_UI));
		g_pGfx->DrawTextureNL(m_iScreenX + 279 + m_iBkSndVol ,m_iScreenY + 140 + 150,g_pTexMgr->GetTexImm(PACKAGE_INTERFACE,138,EP_UI));

		char ch[64];
		sprintf(ch,"%d",m_iBkSndVol);
		g_pFont->DrawText(m_iScreenX + 407,m_iScreenY + 141 + 150,ch,0xFFFFFF00);

		sprintf(ch,"%d",m_iSndVol);
		g_pFont->DrawText(m_iScreenX+407,m_iScreenY+141,ch,0xFFFFFF00);

		if(m_bSndOn != g_Config.GetSndOn())
		{
			g_Config.SetSndOn(m_bSndOn);

			if( !m_bSndOn )
			{	
				g_pAudio->StopAll();
			}			

			g_pAudio->EnableSound(m_bSndOn);
		}

		if(m_iSndVol != g_Config.GetSndVol())
		{
			g_Config.SetSndVol( m_iSndVol );			
			g_pAudio->SetVolumeSound(m_iSndVol);
			g_pAudio->SetAllVol();
			if(g_pSDOAInterface)
				g_pSDOAInterface->NodifyAudioChanged();
		}

		if(m_bBkSndOn != g_Config.GetBkMusicOn())
		{
			g_Config.SetBkMusicOn(m_bBkSndOn);//////
			g_pAudio->EnableBkSound(m_bBkSndOn);/////

			if( m_bBkSndOn )
			{
				string str = "";
				string strMapName = g_pGameMap->GetMapName();
				StringUtil::toUpperCase(strMapName);
				g_ItemCfgMgr.ParseInfo("MapMusic",strMapName.c_str(),str);

				if (!str.empty())
				{
					g_pAudio->PlayMusic(EAT_BKMUSIC,atoi(str.c_str()),true);
				}
				else 
				{
					g_pAudio->StopAllMusic();	
				}
			}
			else
			{				
				g_pAudio->StopAllMusic();
			}			
		}

		if(m_iBkSndVol != g_Config.GetBkMusicVol())
		{
			g_Config.SetBkMusicVol( m_iBkSndVol );		
			g_pAudio->SetVolumeMusic(m_iBkSndVol);
		}
	}
	else if(iCurPage == PAGE_GAME)
	{
		int iStartY = 75,iSpan = 30;
		g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + iStartY,"血条显示",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + iStartY + iSpan,"血条显示为名字颜色",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		//g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + iStartY + iSpan * 2,"光影效果",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + iStartY + iSpan * 2,"天气效果",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + 256,"纹理预读",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);

		if (!g_TempConfig.GetHalfSpeed())
		{
			g_pFont->DrawText(m_iScreenX + 97,m_iScreenY + 256 + 30,"平滑模式",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_SMALL);
		}


		m_pRDFloodOn->SetChecked(m_bFloodOn);
		m_pRDFloodOff->SetChecked(!m_bFloodOn);
		m_pRDColorHPOn->SetChecked(m_bColorHP);
		m_pRDColorHPOff->SetChecked(!m_bColorHP);
		m_pRDWeatherOn->SetChecked(m_iWeatherOn == 1);
		m_pRDWeatherOff->SetChecked(m_iWeatherOn == 0);

		//m_pRDLightOn->SetChecked(m_iLightOn == 1);
		//m_pRDLightOff->SetChecked(m_iLightOn == 0);

		m_pRDTexPreOff->SetChecked(m_iTexPrepare == 0);
		m_pRDTexPreHalf->SetChecked(m_iTexPrepare == 1);
		m_pRDTexPreOn->SetChecked(m_iTexPrepare == 2);

		//m_pUIHelpOn->SetChecked(m_bShowUIHelp);
		//m_pUIHelpOff->SetChecked(!m_bShowUIHelp);

		//bool b3D = g_TempConfig.Get3DMode();
		//m_pRDLightOn->SetShow(b3D);
		//m_pRDLightOff->SetShow(b3D);

		bool bSpeed = !g_TempConfig.GetHalfSpeed();
		m_pRDSmoothOff->SetShow(bSpeed);
		m_pRDSmoothHalf->SetShow(bSpeed);
		m_pRDSmoothOn->SetShow(bSpeed);

		m_pRDSmoothOff->SetChecked(m_iSmooth == 0);
		m_pRDSmoothHalf->SetChecked(m_iSmooth == 1);
		m_pRDSmoothOn->SetChecked(m_iSmooth == 2);
	}
	else if(iCurPage == PAGE_CLISERV)
	{
		g_pFont->DrawText(m_iScreenX + 126,m_iScreenY + 108,"从这里，您可以连接到我们的客户服务专区",COLOR_TEXT_NORMAL,FONT_YAHEI,FONTSIZE_MIDDLE);
	}
	else if(iCurPage == PAGE_PERSONALITY)
	{
		g_pFont->DrawText(m_iScreenX + 249,m_iScreenY + 67,"个性化签名",0xFFB46428,FONT_YAHEI,FONTSIZE_MIDDLE);
		g_pFont->DrawText(m_iScreenX + 129,m_iScreenY + 159,"社会关系",0xFFB46428,FONT_YAHEI,FONTSIZE_MIDDLE);
		m_pRDHusband->SetChecked(m_bHusbandAndWife);
		//m_pRDShaMember->SetChecked(m_bShaMember);
		m_pRDOpenWing->SetChecked(m_bOpenWing);
		m_pRDTempMask->SetChecked(m_bTempMask);
		//m_pRDOpenShine->SetChecked(m_bOpenShine);
	}
}

bool CSysConfigWnd::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	switch (dwMsg)
	{
	case MSG_CTRL_TABPAGE_TABCHANGE:
		{
			if (pControl)
			{
				S_TabPage * pParentTabpage = (S_TabPage * )pControl;
				if (pParentTabpage == &m_TabPage && dwData == 1)//点了系统设置页签
				{
					CloseWindow();
					g_pControl->Msg(MSG_CTRL_ASSCONFIG_WND,OPER_CREATE);
					return true;
				}
			}
		}
		break;
	case MSG_CTRL_BUTTON_CLICK:
		{
			int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;
			if((CControl*)m_pCloseButton == pControl)
			{
				g_pAudio->Play(EAT_OTHER,1,g_pAudio->GetRand()++);
				g_pControl->PopupWindow(MSG_CTRL_SYSCONFIG_WND,OPER_CLOSE);
				return true;
			}

			if(iCurPage == 0)
			{
				if(pControl == m_pOK)
				{
					g_pControl->PopupWindow(MSG_CTRL_SYSCONFIG_WND,OPER_CLOSE);
					return true;
				}
			}else if(iCurPage == PAGE_PICTURE)
			{
				if(DealPictureMsg(dwData,pControl))
					return true;
			}
			else if(iCurPage == PAGE_SOUND)
			{
				if(DealSoundMsg(dwData,pControl))
					return true;
			}else if(iCurPage == PAGE_GAME)
			{
				if(DealGameFuncMsg(dwData,pControl))
					return true;
			}
			else if(iCurPage == PAGE_CLISERV)
			{
				if(DealClientServiceMsg(dwData,pControl))
					return true;
			}else if(iCurPage == PAGE_PERSONALITY)
			{
				if(DealPersionalizeMsg(dwData,pControl))
					return true;
			}
		}
		break;
	default:
		break;
	}
	return CCtrlWindowX::Msg(dwMsg,dwData,pControl);
}


bool CSysConfigWnd::DealPictureMsg(DWORD dwData,CControl* pControl)
{
	if(pControl == m_pRD1280)
	{
		m_iWindowWidth = 1280;
		//m_b1024 = true;
 		m_pRD800->SetChecked(false);
		m_pRD1024->SetChecked(false);
		m_pRD1600->SetChecked(false);
		m_pRD1920->SetChecked(false);
		m_pRD2560->SetChecked(false);
		return true;
	}
	if(pControl == m_pRD1024)
	{
		m_iWindowWidth = 1024;
		//m_b1024 = true;
		m_pRD1280->SetChecked(false);
 		m_pRD800->SetChecked(false);
		m_pRD1600->SetChecked(false);
		m_pRD1920->SetChecked(false);
		m_pRD2560->SetChecked(false);
		return true;
	}
 	else if(pControl == m_pRD800)
 	{
 		//m_b1024 = false;
 		m_iWindowWidth = 800;
 		m_pRD1280->SetChecked(false);
 		m_pRD1024->SetChecked(false);
		m_pRD1600->SetChecked(false);
		m_pRD1920->SetChecked(false);
		m_pRD2560->SetChecked(false);
 		return true;
 	}
 	else if(pControl == m_pRD1600)
 	{
 		m_iWindowWidth = 1600;
 		m_pRD1280->SetChecked(false);
 		m_pRD1024->SetChecked(false);
 		m_pRD800->SetChecked(false);
		m_pRD1920->SetChecked(false);
		m_pRD2560->SetChecked(false);
 		return true;
 	}
 	else if(pControl == m_pRD1920)
 	{
 		m_iWindowWidth = 1920;
 		m_pRD1280->SetChecked(false);
 		m_pRD1024->SetChecked(false);
 		m_pRD800->SetChecked(false);
		m_pRD1600->SetChecked(false);
		m_pRD2560->SetChecked(false);
 		return true;
 	}
 	else if(pControl == m_pRD2560)
 	{
 		m_iWindowWidth = 2560;
 		m_pRD1280->SetChecked(false);
 		m_pRD1024->SetChecked(false);
 		m_pRD800->SetChecked(false);
		m_pRD1600->SetChecked(false);
		m_pRD1920->SetChecked(false);
 		return true;
 	}
	else if(pControl == m_pRDWinMode)
	{
		m_bAllWndMode = true;
		m_pRDScreenMode->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDScreenMode)
	{
		m_bAllWndMode = false;
		m_pRDWinMode->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDSpeed)
	{
		m_bHalfSpeed = false;
		m_pRDHalfSpeed->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDHalfSpeed)
	{
		m_bHalfSpeed = true;
		m_pRDSpeed->SetChecked(false);
		return true;
	}
	//else if(pControl == m_pRDHighQ)
	//{
	//	return true;
	//}
	//else if(pControl == m_pRDLowQ)
	//{
	//	return true;
	//}
	//else if(pControl == m_pRD32Bit)
	//{
	//	m_iRGB32 = 1;
	//	m_pRD16Bit->SetChecked(false);
	//	return true;
	//}
	//else if(pControl == m_pRD16Bit)
	//{
	//	m_iRGB32 = 0;
	//	m_pRD32Bit->SetChecked(false);

	//	return true;
	//}
	//else if(pControl == m_pRDVSynOpen)
	//{
	//	m_bVSync = true;
	//	m_pRDVSynClose->SetChecked(false);
	//	return true;
	//}
	//else if(pControl == m_pRDVSynClose)
	//{
	//	m_bVSync = false;
	//	m_pRDVSynOpen->SetChecked(false);
	//	return true;
	//}
	else if(pControl == m_pRDCursorOpen)
	{
		m_iMouse = 1;
		m_pRDCursorClose->SetChecked(false);

		return true;
	}
	else if(pControl == m_pRDCursorClose)
	{
		m_iMouse = 0;
		m_pRDCursorOpen->SetChecked(false);
		return true;
	}
	else if(pControl == m_pClassicUI)
	{
		m_EutUiType = EUT_CLASSIC;
		return true;
	}
	else if(pControl == m_pFashionUi)
	{
		m_EutUiType = EUT_FASHION;
		return true;
	}
	else if(pControl == m_pOK)
	{
		UpdateDataGraphic(false);
		OnclickOK();
		return true;
	}
	else if(pControl == m_pBestCompatBtn)
	{
		OnClickPerform();
		UpdateDataGraphic(true);
		return true;
	}
	else if(pControl == m_pBestEffectBtn)
	{
		OnClickEffect();
		return true;
	}
	return false;
}

void CSysConfigWnd::OnClickPerform()
{
	//g_TempConfig.Set3DMode		(true);
	//g_TempConfig.Set1024Mode	(false);
	g_TempConfig.SetRGB			(0);
	g_TempConfig.SetAllWndMode	(false);
	g_TempConfig.SetMouse		(0);
	g_TempConfig.SetTrans		(2);
	g_Config.SetTrans		    (2);
	g_TempConfig.SetWeather		(0);
	g_TempConfig.SetLight		(0);
	//g_TempConfig.SetCanTrans	(false);
	//g_TempConfig.SetVSync		(true);
	g_TempConfig.SetHalfSpeed	(true);
	g_TempConfig.SetSmooth		(0);
}

void CSysConfigWnd::OnClickEffect()
{
	//g_TempConfig.Set3DMode		(true);
	//g_TempConfig.Set1024Mode	(true);
	g_TempConfig.SetRGB			(0);
	g_TempConfig.SetMouse		(0);
	g_TempConfig.SetTrans		(0);
	g_Config.SetTrans		    (0);
	g_TempConfig.SetWeather		(1);
	g_TempConfig.SetLight		(1);
	//g_TempConfig.SetCanTrans	(true);
	//g_TempConfig.SetVSync		(true);
	g_TempConfig.SetHalfSpeed	(false);
	g_TempConfig.SetSmooth		(2);

	int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;

	if(iCurPage == PAGE_PICTURE)
	{
		UpdateDataGraphic(true);
	}else if(iCurPage == PAGE_SOUND)
	{
		UpdateDataSound(true);
	}else if(iCurPage == PAGE_GAME)
	{
		UpdateDataGameFunc(true);
	}else if(iCurPage == PAGE_PERSONALITY)
	{
		UpdateDataPerson(true);
	}
}


void CSysConfigWnd::UpdateDataGraphic(bool bSaveAndValidate)
{
	if(bSaveAndValidate)
	{
		m_iWindowWidth = g_TempConfig.GetWindowWidth();
		m_bAllWndMode = g_TempConfig.GetAllWndMode();
		m_bHalfSpeed = g_TempConfig.GetHalfSpeed();
		//m_b3DMode = g_TempConfig.Get3DMode();


		m_iTransparence = g_Config.GetTrans();  //获得现有的图像设置
		//m_bCanTrans = g_TempConfig.GetCanTrans();
		//m_bVSync = g_TempConfig.GetVSync();
		m_iMouse = g_TempConfig.GetMouse();
		m_iRGB32 = g_TempConfig.GetRGB();
	}
	else
	{
		//g_TempConfig.SetTrans(m_iTransparence);  //设置现有的图像设置
		g_TempConfig.SetHalfSpeed(m_bHalfSpeed);
		//g_TempConfig.SetVSync(m_bVSync);
		//g_TempConfig.Set3DMode(m_b3DMode);
		g_TempConfig.SetWindowWidth(m_iWindowWidth);
		g_TempConfig.SetRGB(m_iRGB32);
		g_TempConfig.SetAllWndMode(m_bAllWndMode);
		g_TempConfig.SetMouse(m_iMouse);
		g_TempConfig.SetDefault(m_bDefault);
	}
}

void CSysConfigWnd::UpdateDataSound(bool bSaveAndValidate)
{
	if(bSaveAndValidate)
	{
		m_iSndVol = g_Config.GetSndVol(); //得到当前音效音量大小
		m_bSndOn= g_Config.GetSndOn();

		//m_iBkSndVol = GetVolume(1);
		m_iBkSndVol = g_Config.GetBkMusicVol();
		m_bBkSndOn = g_Config.GetBkMusicOn();
		
	}
	else
	{
		//g_TempConfig.SetSndVol(m_iSndVol); //设置当前音效音量大小
		//g_TempConfig.SetSndOn(m_bSndOn);

		//g_TempConfig.SetBkMusicOn(m_bBkSndOn);
		//g_TempConfig.SetBkMusicVol(m_iBkSndVol);			
	}
}

void CSysConfigWnd::UpdateDataPerson(bool bSaveAndValidate)
{
	if(bSaveAndValidate)
	{
		m_bHusbandAndWife = g_TempConfig.GetHusbandAndWife();
		m_bShaMember = g_TempConfig.GetShaMember();
		m_bOpenWing = g_TempConfig.GetOpenWing();
		m_bOpenShine = g_TempConfig.GetOpenShine();
		m_bTempMask = g_TempConfig.GetTempMask();
		m_strTitleStr = g_TempConfig.GetTitleStr();
		m_strTempMaskStr = g_TempConfig.GetTempMaskStr();
	}
	else
	{
		g_TempConfig.SetHusbandAndWife(m_bHusbandAndWife);
		g_TempConfig.SetShaMember(m_bShaMember);
		g_TempConfig.SetOpenWing(m_bOpenWing);
		g_TempConfig.SetOpenShine(m_bOpenShine);
		g_TempConfig.SetTempMask(m_bTempMask);
		g_TempConfig.SetTitleStr(m_strTitleStr.c_str());
		g_TempConfig.SetTempMaskStr(m_strTempMaskStr.c_str());
	}
}

void CSysConfigWnd::UpdateDataGameFunc(bool bSaveAndValidate)
{
	if(bSaveAndValidate)
	{
		m_bFloodOn = g_TempConfig.GetFloodOn();  //得到当前血条状态
		m_iLightOn = g_TempConfig.GetLight();	//得到当前光影状态
		m_iWeatherOn = g_TempConfig.GetWeather(); //得到当前天气效果
		m_iTexPrepare = g_TempConfig.GetTexPrepare();
		m_bColorHP = g_TempConfig.GetColorHP();
		m_iSmooth = g_TempConfig.GetSmooth();
		m_bAutoPath = g_TempConfig.GetAutoPath();
		m_bShowUIHelp = g_TempConfig.GetShowUIHelp();
	}
	else
	{
		g_TempConfig.SetFloodOn(m_bFloodOn);  //设置当前血条状态
		g_TempConfig.SetColorHP(m_bColorHP);
		g_TempConfig.SetSmooth(m_iSmooth);
		g_TempConfig.SetAutoPath(m_bAutoPath);
		g_TempConfig.SetLight(m_iLightOn);	//设置当前光影状态
		g_TempConfig.SetWeather(m_iWeatherOn); //设置当前天气效果
		g_TempConfig.SetTexPrepare(m_iTexPrepare); //
		g_TempConfig.SetShowUIHelp(m_bShowUIHelp);
	}
}

bool CSysConfigWnd::DealSoundMsg(DWORD dwData,CControl* pControl)
{
	if(pControl == m_pRDSoundOn)
	{
		m_bSndOn = true;
		m_pRDSoundOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDSoundOff)
	{
		m_bSndOn = false;
		m_pRDSoundOn->SetChecked(false);
		return true;
	}
	else if (pControl == m_pBkMusicOn)
	{
		m_bBkSndOn = true;
		m_pBkMusicOff->SetChecked(false);
		return true;
	}
	else if (pControl == m_pBkMusicOff)
	{
		m_bBkSndOn = false;
		m_pBkMusicOn->SetChecked(false);
	}
	else if(pControl == m_pRDNearVoiceOn)
	{
		m_bNearVoice = true;
		m_pRDNearVoiceOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDNearVoiceOff)
	{
		m_bNearVoice = false;
		m_pRDNearVoiceOn->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDTeamVoiceOn)
	{
		m_bTeamVoice = true;
		m_pRDTeamVoiceOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDTeamVoiceOff)
	{
		m_bTeamVoice = false;
		m_pRDTeamVoiceOn->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDMicroPhoneOn)
	{
		m_bMicroPhone = true;
		m_pRDMicroPhoneOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDMicroPhoneOff)
	{
		m_bMicroPhone = false;
		m_pRDMicroPhoneOn->SetChecked(false);
		return true;
	}
	if(pControl == m_pOK)
	{
		UpdateDataSound(false);
		OnclickOK();
		return true;
	}
	else if(pControl == m_pBestCompatBtn)
	{
		OnClickPerform();
		return true;
	}
	else if(pControl == m_pBestEffectBtn)
	{
		OnClickEffect();
		return true;
	}

	return false;
}

bool CSysConfigWnd::DealGameFuncMsg(DWORD dwData,CControl* pControl)
{
	if(pControl == m_pUIHelpOn)
	{
		m_bShowUIHelp = true;
		m_pUIHelpOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pUIHelpOff)
	{
		m_bShowUIHelp = false;
		m_pUIHelpOn->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDFloodOn)
	{
		m_bFloodOn = true;
		m_pRDFloodOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDFloodOff)
	{
		m_bFloodOn = false;
		m_pRDFloodOn->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDColorHPOn)
	{
		m_bColorHP = true;
		m_pRDColorHPOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDColorHPOff)
	{
		m_bColorHP = false;
		m_pRDColorHPOn->SetChecked(false);
		return true;
	}
	//else if(pControl == m_pRDLightOn)
	//{
	//	m_iLightOn = 1;
	//	m_pRDLightOff->SetChecked(false);
	//	return true;
	//}
	//else if(pControl == m_pRDLightOff)
	//{
	//	m_iLightOn = 0;
	//	m_pRDLightOn->SetChecked(false);
	//	return true;
	//}
	else if(pControl == m_pRDWeatherOn)
	{
		m_iWeatherOn = 1;
		m_pRDWeatherOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDWeatherOff)
	{
		m_iWeatherOn = 0;
		m_pRDWeatherOn->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDTexPreOff)
	{
		m_iTexPrepare = 0;
		m_pRDTexPreOn->SetChecked(false);
		m_pRDTexPreHalf->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDTexPreHalf)
	{
		m_iTexPrepare = 1;
		m_pRDTexPreOn->SetChecked(false);
		m_pRDTexPreOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDTexPreOn)
	{
		m_iTexPrepare = 2;
		m_pRDTexPreOff->SetChecked(false);
		m_pRDTexPreHalf->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDSmoothOff)
	{
		m_iSmooth = 0;
		m_pRDSmoothOn->SetChecked(false);
		m_pRDSmoothHalf->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDSmoothHalf)
	{
		m_iSmooth = 1;
		m_pRDSmoothOn->SetChecked(false);
		m_pRDSmoothOff->SetChecked(false);
		return true;
	}
	else if(pControl == m_pRDSmoothOn)
	{
		//////////////////////////////////////////////
		//if( !g_TempConfig.Get3DMode() )
		//{
		//	g_MsgBoxMgr.PopSimpleAlert("您现在使用的是低画质模式，此模式下不支持最优平滑效果。");
		//	return true;
		//}
		//else
			m_iSmooth = 2;

		m_pRDSmoothOff->SetChecked(false);
		m_pRDSmoothHalf->SetChecked(false);
		return true;
	}
	else if(pControl == m_pOK)
	{
		//点击确定
		UpdateDataGameFunc(false);
		OnclickOK();
		return true;
	}
	else if(pControl == m_pBestCompatBtn)
	{
		OnClickPerform();
		return true;
	}
	else if(pControl == m_pBestEffectBtn)
	{
		OnClickEffect();
		return true;
	}
	return false;
}

bool CSysConfigWnd::DealClientServiceMsg(DWORD dwData,CControl* pControl)
{
	string url;
	if(pControl == m_pWebsite)
	{
		url = g_pStreamMgr->GetWebsite("Website","");
		ShellExecute(NULL,"open","iexplore.exe",url.c_str(),NULL,SW_SHOW);
		return true;
	}
	else if(pControl == m_pSupport)
	{
		url = g_pStreamMgr->GetWebsite("Support","");
		ShellExecute(NULL,"open","iexplore.exe",url.c_str(),NULL,SW_SHOW);
		return true;
	}
	else if(pControl == m_pAct)
	{
		g_pControl->PopupWindow(MSG_CTRL_ACTIVITY_LOG_WND,OPER_CREATE,0);
		return true;
	}
	else if(pControl == m_pHelp)
	{
		g_pControl->PopupWindow(MSG_CTRL_MEMOIRE_WND,OPER_CREATE);
		return true;
	}
	else if(pControl == m_pOK)
	{
		//点击确定
		OnclickOK();
		return true;
	}

	return false;
}

bool CSysConfigWnd::DealPersionalizeMsg(DWORD dwData,CControl* pControl)
{
	if(pControl == m_pRDHusband)
	{
		m_bHusbandAndWife = m_pRDHusband->IsChecked();
		return true;
	}
	else if(pControl == m_pRDShaMember)
	{
		m_bShaMember = m_pRDShaMember->IsChecked();
		return true;
	}
	else if(pControl == m_pRDOpenWing)
	{
		m_bOpenWing = m_pRDOpenWing->IsChecked();
		return true;
	}
	else if(pControl == m_pRDOpenShine)
	{
		m_bOpenShine = m_pRDOpenShine->IsChecked();
		return true;
	}
	else if(pControl == m_pRDTempMask)
	{
		m_bTempMask = m_pRDTempMask->IsChecked();
		return true;
	}
	else if(pControl == m_pOK)
	{
		//点击确定
		m_strTitleStr = m_pMultiEdit->GetText();
		m_strTempMaskStr = m_pMBTempMask->GetSelText();

		UpdateDataPerson(false);
		OnclickOK();
		return true;
	}

	return false;
}

void CSysConfigWnd::OnCreate()
{
	m_bDown = false;
	SetCloseButton(496,4,80);
	m_bIsTransparence = false;

	SetMask(10125);

	m_pRD1280 = NULL;
	m_pRD1600 = NULL;
	m_pRD1920 = NULL;
	m_pRD2560 = NULL;
	m_pRD1024 = NULL;
 	m_pRD800  = NULL; 
	m_pRDWinMode = NULL;
	m_pRDScreenMode = NULL;
	m_pRDSpeed = NULL;
	m_pRDHalfSpeed = NULL;
	//m_pRDHighQ = NULL;
	//m_pRDLowQ = NULL;
	m_pRD32Bit = NULL;
	m_pRD16Bit = NULL;
	//m_pRDVSynOpen = NULL;
	//m_pRDVSynClose = NULL;
	m_pRDCursorOpen = NULL;
	m_pRDCursorClose = NULL;
	m_pRDOpenShine = NULL;
	m_bIsTransparence =false;

	m_pRDSoundOn = NULL;
	m_pRDSoundOff = NULL;
	m_pRDNearVoiceOn = NULL;
	m_pRDNearVoiceOff = NULL;
	m_pRDTeamVoiceOn = NULL;
	m_pRDTeamVoiceOff = NULL;
	m_pRDMicroPhoneOn = NULL;
	m_pRDMicroPhoneOff = NULL;

	m_pUIHelpOn = NULL;
	m_pUIHelpOff = NULL;

	m_pWebsite = NULL;
	m_pSupport = NULL;
	m_pAct = NULL;
	m_pHelp = NULL;

	m_pMultiEdit = NULL;
	m_pRDHusband = NULL;
	m_pRDTeacher = NULL;
	m_pRDShaMember = NULL;
	m_pRDOpenWing = NULL;
	m_pRDOpenShine = NULL;
	m_pRDTempMask = NULL;
	m_pMBTempMask = NULL; 

	m_pOK = NULL;
	m_pBestCompatBtn = NULL;
	m_pBestEffectBtn = NULL;








	int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;

	if(iCurPage == PAGE_PICTURE)
	{
		UpdateDataGraphic(true);

		int iX1 = 189,iX2 = 327 - 40,iY = 90,iSPan = 18;

 		m_pRD800 = new CCtrlRadio();
 		AddControl(m_pRD800);
 		m_pRD800->Create(this,iX1,iY,125,126,127,128,110,20);
 		m_pRD800->SetText("800*600",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
 		m_pRD800->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式。");
		
		m_pRD1024 = new CCtrlRadio();
		AddControl(m_pRD1024);
		m_pRD1024->Create(this,iX2,iY,125,126,127,128,110,20);
		m_pRD1024->SetText("1024*768",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRD1024->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式。");
	
	
		m_pRD1280 = new CCtrlRadio(); 
		AddControl(m_pRD1280);
		m_pRD1280->Create(this,iX2 + 90,iY,125,126,127,128,110,20);
		m_pRD1280->SetText("1280*800",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRD1280->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式，推荐宽屏用户使用。");

		m_pRD1600 = new CCtrlRadio();
		AddControl(m_pRD1600);
		m_pRD1600->Create(this,iX1,iY + iSPan,125,126,127,128,110,20);
		m_pRD1600->SetText("1600*900",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRD1600->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式。");

		m_pRD1920 = new CCtrlRadio();
		AddControl(m_pRD1920);
		m_pRD1920->Create(this,iX2,iY + iSPan,125,126,127,128,110,20);
		m_pRD1920->SetText("1920*1080",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRD1920->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式。");

		m_pRD2560 = new CCtrlRadio();
		AddControl(m_pRD2560);
		m_pRD2560->Create(this,iX2 + 90,iY + iSPan,125,126,127,128,110,20);
		m_pRD2560->SetText("2560*1440",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRD2560->SetTips("选择高分辨率在游戏中能看到更大的地图范围，\n但是会消耗很多的显示资源，请根据您的机器配\n置决定采用何种分辨率模式。");

		if (g_hParentWnd)
		{
			m_pRD800->SetEnable(false);
			m_pRD800->SetShow(false);
			m_pRD1024->SetEnable(false);
			m_pRD1024->SetShow(false);
			m_pRD1280->SetEnable(false);
			m_pRD1280->SetShow(false);
			m_pRD1600->SetEnable(false);
			m_pRD1600->SetShow(false);
			m_pRD1920->SetEnable(false);
			m_pRD1920->SetShow(false);
			m_pRD2560->SetEnable(false);
			m_pRD2560->SetShow(false);
		}

		m_pRDWinMode = new CCtrlRadio();
		AddControl(m_pRDWinMode);
		m_pRDWinMode->Create(this,iX1,iY + iSPan*2,125,126,127,128,110,20);
		m_pRDWinMode->SetText("窗口",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDWinMode->SetTips("窗口模式便于您在游戏的同时运行别的Windows\n程序，比如浏览器，QQ等等。");

		m_pRDScreenMode = new CCtrlRadio();
		AddControl(m_pRDScreenMode);
		m_pRDScreenMode->Create(this,iX2,iY + iSPan*2,125,126,127,128,110,20);
		m_pRDScreenMode->SetText("全屏",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDScreenMode->SetTips("窗口模式便于您在游戏的同时运行别的Windows\n程序，比如浏览器，QQ等等。");


		m_pRDSpeed = new CCtrlRadio();
		AddControl(m_pRDSpeed);
		m_pRDSpeed->Create(this,iX1,iY + iSPan*3,125,126,127,128,110,20);
		m_pRDSpeed->SetText("快",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		m_pRDSpeed->SetTips("打开此选项会降低游戏的刷新速度，对配置较差\n的机器能有效的避免跳帧的感觉。");

		m_pRDHalfSpeed = new CCtrlRadio();
		AddControl(m_pRDHalfSpeed);
		m_pRDHalfSpeed->Create(this,iX2,iY + iSPan*3,125,126,127,128,110,20);
		m_pRDHalfSpeed->SetText("慢",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDHalfSpeed->SetTips("打开此选项会降低游戏的刷新速度，对配置较差\n的机器能有效的避免跳帧的感觉。");

		m_pFashionUi = new CCtrlRadio();
		AddControl(m_pFashionUi);
		m_pFashionUi->Create(this,iX2,iY + iSPan*4,125,126,127,128,70,20);
		m_pFashionUi->SetText("时尚版界面",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		
		m_pClassicUI = new CCtrlRadio();
		AddControl(m_pClassicUI);
		m_pClassicUI->Create(this,iX1,iY + iSPan*4,125,126,127,128,70,20);
		m_pClassicUI->SetText("经典版界面",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pRDHighQ = new CCtrlRadio();
		//AddControl(m_pRDHighQ);
		//m_pRDHighQ->Create(this,iX1,iY + iSPan*3,125,126,127,128,110,20);
		//m_pRDHighQ->SetText("高",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRDHighQ->SetTips("高质量图像模式将使用显卡的3D渲染功能显示游\n戏画面，能提供最好的图像显示效果，但是对机\n器的配置有一定的要求。低质量图像模式使用显\n卡的2D渲染功能显示游戏画面，图像质量略低于\n高画质模式，但是明显降低对机器配置的要求。");

		//m_pRDLowQ = new CCtrlRadio();
		//AddControl(m_pRDLowQ);
		//m_pRDLowQ->Create(this,iX2,iY + iSPan*3,125,126,127,128,110,20);
		//m_pRDLowQ->SetText("低",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRDLowQ->SetTips("高质量图像模式将使用显卡的3D渲染功能显示游\n戏画面，能提供最好的图像显示效果，但是对机\n器的配置有一定的要求。低质量图像模式使用显\n卡的2D渲染功能显示游戏画面，图像质量略低于\n高画质模式，但是明显降低对机器配置的要求。");

		iY = 180;
		//m_pRD32Bit = new CCtrlRadio();
		//AddControl(m_pRD32Bit);
		//m_pRD32Bit->Create(this,iX1,iY,125,126,127,128,110,20);
		//m_pRD32Bit->SetText("32位色",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRD32Bit->SetTips("16位色模式是游戏的标准显示模式，32位色模式\n能略微提升图像质量，但是会极大的降低显示速\n度。除非您的机器配置极高，否则不推荐打开32\n位色显示模式。");

		//m_pRD16Bit = new CCtrlRadio();
		//AddControl(m_pRD16Bit);
		//m_pRD16Bit->Create(this,iX2,iY,125,126,127,128,110,20);
		//m_pRD16Bit->SetText("16位色",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRD16Bit->SetTips("16位色模式是游戏的标准显示模式，32位色模式\n能略微提升图像质量，但是会极大的降低显示速\n度。除非您的机器配置极高，否则不推荐打开32\n位色显示模式。");

		//m_pRDVSynOpen = new CCtrlRadio();
		//AddControl(m_pRDVSynOpen);
		//m_pRDVSynOpen->Create(this,iX1,iY + iSPan*1,125,126,127,128,110,20);
		//m_pRDVSynOpen->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRDVSynOpen->SetTips("打开垂直刷新选项能有效避免画面撕裂的现象，\n但是会略微降低显示速度。除非您确实了解此选\n项的含义，否则不推荐关闭此选项。");

		//m_pRDVSynClose = new CCtrlRadio();
		//AddControl(m_pRDVSynClose);
		//m_pRDVSynClose->Create(this,iX2,iY + iSPan*1,125,126,127,128,110,20);
		//m_pRDVSynClose->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		//m_pRDVSynClose->SetTips("打开垂直刷新选项能有效避免画面撕裂的现象，\n但是会略微降低显示速度。除非您确实了解此选\n项的含义，否则不推荐关闭此选项。");

		m_pRDCursorOpen = new CCtrlRadio();
		AddControl(m_pRDCursorOpen);
		m_pRDCursorOpen->Create(this,iX1,iY + iSPan*1,125,126,127,128,110,20);
		m_pRDCursorOpen->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDCursorOpen->SetTips("在使用某些较旧的显卡时，由于显卡驱动兼容性\n方面的原因，会出现鼠标闪烁的情况，此时可以\n打开此选项。");

		m_pRDCursorClose = new CCtrlRadio();
		AddControl(m_pRDCursorClose);
		m_pRDCursorClose->Create(this,iX2,iY + iSPan*1,125,126,127,128,110,20);
		m_pRDCursorClose->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDCursorClose->SetTips("在使用某些较旧的显卡时，由于显卡驱动兼容性\n方面的原因，会出现鼠标闪烁的情况，此时可以\n打开此选项。");




		//// 窗口透明度
		//m_strTip[7][0] ="拖动滚动条可以调整窗口的透明度。";
		//m_strTip[7][1] ="";
		//m_strTip[7][2] ="";
		//m_strTip[7][3] ="";
		//m_strTip[7][4] ="";

	}
    else if(iCurPage == PAGE_SOUND)
	{
		UpdateDataSound(true);

		m_pRDSoundOn = new CCtrlRadio();
		AddControl(m_pRDSoundOn);
		m_pRDSoundOn->Create(this,275,97,125,126,127,128,110,20);
		m_pRDSoundOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDSoundOn->SetTips("音效开关");

		m_pRDSoundOff = new CCtrlRadio();
		AddControl(m_pRDSoundOff);
		m_pRDSoundOff->Create(this,390,97,125,126,127,128,110,20);
		m_pRDSoundOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDSoundOff->SetTips("音效开关");

		m_pBkMusicOn = new CCtrlRadio();
		AddControl(m_pBkMusicOn);
		m_pBkMusicOn->Create(this,275,97 + 150,125,126,127,128,110,20);
		m_pBkMusicOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pBkMusicOn->SetTips("背景音效开关");

		m_pBkMusicOff = new CCtrlRadio();
		AddControl(m_pBkMusicOff);
		m_pBkMusicOff->Create(this,390,97 + 150,125,126,127,128,110,20);
		m_pBkMusicOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pBkMusicOff->SetTips("背景音效开关");


		//m_pRDNearVoiceOn = new CCtrlRadio();
		//AddControl(m_pRDNearVoiceOn);
		//m_pRDNearVoiceOn->Create(this,309,252,0,55);

		//m_pRDNearVoiceOff = new CCtrlRadio();
		//AddControl(m_pRDNearVoiceOff);
		//m_pRDNearVoiceOff->Create(this,422,252,0,55);

		//m_pRDTeamVoiceOn = new CCtrlRadio();
		//AddControl(m_pRDTeamVoiceOn);
		//m_pRDTeamVoiceOn->Create(this,309,280,0,55);

		//m_pRDTeamVoiceOff = new CCtrlRadio();
		//AddControl(m_pRDTeamVoiceOff);
		//m_pRDTeamVoiceOff->Create(this,422,280,0,55);

		//m_pRDMicroPhoneOn = new CCtrlRadio();
		//AddControl(m_pRDMicroPhoneOn);
		//m_pRDMicroPhoneOn->Create(this,309,305,0,55);

		//m_pRDMicroPhoneOff = new CCtrlRadio();
		//AddControl(m_pRDMicroPhoneOff);
		//m_pRDMicroPhoneOff->Create(this,422,305,0,55);

		//// 音效音量
		//m_strTip[1][0] ="拖动滚动条可以调整音效的音量。";
		//m_strTip[1][1] ="";
		//m_strTip[1][2] ="";
		//m_strTip[1][3] ="";
		//m_strTip[1][4] ="";

	}
	else if(iCurPage == PAGE_GAME)
	{
		UpdateDataGameFunc(true);

		int iX1 = 219,iX2 = 338,iX3 = 396,iY = 73,iSpan = 30;
		m_pRDFloodOn = new CCtrlRadio();
		AddControl(m_pRDFloodOn);
		m_pRDFloodOn->Create(this,iX1,iY,125,126,127,128,110,20);
		m_pRDFloodOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDFloodOn->SetTips("显示玩家和怪物的血条");

		m_pRDFloodOff = new CCtrlRadio();
		AddControl(m_pRDFloodOff);
		m_pRDFloodOff->Create(this,iX2,iY,125,126,127,128,110,20);
		m_pRDFloodOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDFloodOff->SetTips("显示玩家和怪物的血条");

		m_pRDColorHPOn = new CCtrlRadio();
		AddControl(m_pRDColorHPOn);
		m_pRDColorHPOn->Create(this,iX1,iY + iSpan,125,126,127,128,110,20);
		m_pRDColorHPOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDColorHPOn->SetTips("显示玩家的血条时使用其名字的颜色（名字无特\n殊颜色时血条显示为红色）。");

		m_pRDColorHPOff = new CCtrlRadio();
		AddControl(m_pRDColorHPOff);
		m_pRDColorHPOff->Create(this,iX2,iY + iSpan,125,126,127,128,110,20);		
		m_pRDColorHPOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDColorHPOff->SetTips("显示玩家的血条时使用其名字的颜色（名字无特\n殊颜色时血条显示为红色）。");

		//m_pRDLightOn = new CCtrlRadio();
		//AddControl(m_pRDLightOn);
		//m_pRDLightOn->Create(this,iX1,iY + iSpan*2,125,126,127,128,110,20);
		//m_pRDLightOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pRDLightOff = new CCtrlRadio();
		//AddControl(m_pRDLightOff);
		//m_pRDLightOff->Create(this,iX2,iY+iSpan*2,125,126,127,128,110,20);
		//m_pRDLightOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		m_pRDWeatherOn = new CCtrlRadio();
		AddControl(m_pRDWeatherOn);
		m_pRDWeatherOn->Create(this,iX1,iY+iSpan*2,125,126,127,128,110,20);
		m_pRDWeatherOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		m_pRDWeatherOff = new CCtrlRadio();
		AddControl(m_pRDWeatherOff);
		m_pRDWeatherOff->Create(this,iX2,iY+iSpan*2,125,126,127,128,110,20);
		m_pRDWeatherOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pUIHelpOn = new CCtrlRadio();
		//AddControl(m_pUIHelpOn);
		//m_pUIHelpOn->Create(this,iX1,iY+iSpan*5,125,126,127,128,110,20);
		//m_pUIHelpOn->SetText("开",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pUIHelpOff = new CCtrlRadio();
		//AddControl(m_pUIHelpOff);
		//m_pUIHelpOff->Create(this,iX2,iY+iSpan*5,125,126,127,128,110,20);
		//m_pUIHelpOff->SetText("关",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		iY = 257;
		m_pRDTexPreOff = new CCtrlRadio();
		AddControl(m_pRDTexPreOff);
		m_pRDTexPreOff->Create(this,iX1 - 15,iY,125,126,127,128,110,20);
		m_pRDTexPreOff->SetText("关闭",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDTexPreOff->SetTips("进入一张地图前预先读取此地图所用到的图像。\n如果选择关闭则不预读地图；选择智能将根据您\n的机器配置决定读取多大范围的地图；选择完全\n将预读整张地图。打开预读在进入一张地图时会\n耗费更多的时间，但将减少跑动时的延迟时间。");

		m_pRDTexPreHalf = new CCtrlRadio();
		AddControl(m_pRDTexPreHalf);
		m_pRDTexPreHalf->Create(this,iX2 - 30,iY,125,126,127,128,110,20);
		m_pRDTexPreHalf->SetText("智能",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDTexPreHalf->SetTips("进入一张地图前预先读取此地图所用到的图像。\n如果选择关闭则不预读地图；选择智能将根据您\n的机器配置决定读取多大范围的地图；选择完全\n将预读整张地图。打开预读在进入一张地图时会\n耗费更多的时间，但将减少跑动时的延迟时间。");

		m_pRDTexPreOn = new CCtrlRadio();
		AddControl(m_pRDTexPreOn);
		m_pRDTexPreOn->Create(this,iX3,iY,125,126,127,128,110,20);
		m_pRDTexPreOn->SetText("完全",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDTexPreOn->SetTips("进入一张地图前预先读取此地图所用到的图像。\n如果选择关闭则不预读地图；选择智能将根据您\n的机器配置决定读取多大范围的地图；选择完全\n将预读整张地图。打开预读在进入一张地图时会\n耗费更多的时间，但将减少跑动时的延迟时间。");

		m_pRDSmoothOff = new CCtrlRadio();
		AddControl(m_pRDSmoothOff);
		m_pRDSmoothOff->Create(this,iX1 - 15,iY + 30,125,126,127,128,110,20);
		m_pRDSmoothOff->SetText("关闭",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);
		m_pRDSmoothOff->SetTips("平滑显示选项将在跑动中明显降低屏幕图像晃动\n引起的不适感。选择关闭将使用原有模式；选择\n标准模式，将有很大改善；选择最优模式将完全\n消除晃动感，但是此模式对机器性能要求较高，\n只推荐机器配置较高的玩家使用此模式。");

		m_pRDSmoothHalf = new CCtrlRadio();
		AddControl(m_pRDSmoothHalf);
		m_pRDSmoothHalf->Create(this,iX2 - 30,iY + 30,125,126,127,128,110,20);
		m_pRDSmoothHalf->SetText("标准",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		m_pRDSmoothOn = new CCtrlRadio();
		AddControl(m_pRDSmoothOn);
		m_pRDSmoothOn->Create(this,iX3,iY + 30,125,126,127,128,110,20);
		m_pRDSmoothOn->SetText("最优",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);


		//// 光影效果
		//m_strTip[2][0] ="显示日夜交替引起的明暗变化。光影效果将消耗";
		//m_strTip[2][1] ="一定的显示资源，配置较低的玩家推荐关闭此选";
		//m_strTip[2][2] ="项。";
		//m_strTip[2][3] ="";
		//m_strTip[2][4] ="";

		//// 天气效果
		//m_strTip[3][0] ="显示雨雾沙尘暴等天气效果。";
		//m_strTip[3][1] ="";
		//m_strTip[3][2] ="";
		//m_strTip[3][3] ="";
		//m_strTip[3][4] ="";

		//// 智能寻路
		//m_strTip[4][0] ="自动搜索到达目的地的路径。";
		//m_strTip[4][1] ="";
		//m_strTip[4][2] ="";
		//m_strTip[4][3] ="";
		//m_strTip[4][4] ="";

	}else if(iCurPage == PAGE_CLISERV)
	{
		m_pSupport = new CCtrlButton(); 
		AddControl(m_pSupport);
		m_pSupport->CreateEx(this, 160,158, 95, 96, 97, 98);
		m_pSupport->SetText("客户服务", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

		m_pWebsite = new CCtrlButton();
		AddControl(m_pWebsite);
	 	m_pWebsite->CreateEx(this, 160,194, 95, 96, 97, 98);
		m_pWebsite->SetText("官方网站", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

		m_pAct = new CCtrlButton();
		AddControl(m_pAct);
		m_pAct->CreateEx(this, 317,158, 95, 96, 97, 98);
		m_pAct->SetText("当前活动", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

		m_pHelp = new CCtrlButton();
		AddControl(m_pHelp);
		m_pHelp->CreateEx(this, 317,194, 95, 96, 97, 98);
		m_pHelp->SetText("游戏帮助", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

	}
	else if(iCurPage == PAGE_PERSONALITY)
	{
		UpdateDataPerson(true);

		m_pRDHusband = new CCtrlRadio();
		AddControl(m_pRDHusband);
		m_pRDHusband->Create(this,109,212,125,126,127,128,110,20);
		m_pRDHusband->SetText("显示夫妻封号",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pRDShaMember = new CCtrlRadio();
		//AddControl(m_pRDShaMember);
		//m_pRDShaMember->Create(this,109,254,125,126,127,128,110,20);
		//m_pRDShaMember->SetText("沙城行会名称",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		m_pRDOpenWing = new CCtrlRadio();
		AddControl(m_pRDOpenWing);
		m_pRDOpenWing->Create(this,342,191,125,126,127,128,110,20);
		m_pRDOpenWing->SetText("显示未觉醒翅膀",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);

		//m_pRDOpenShine = new CCtrlRadio();
		//AddControl(m_pRDOpenShine);
		//m_pRDOpenShine->Create(this,423,251,0,55);

		m_pRDTempMask = new CCtrlRadio();
		AddControl(m_pRDTempMask);
		m_pRDTempMask->Create(this,342,232,125,126,127,128,110,20);
		m_pRDTempMask->SetText("临时封号",COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,COLOR_TEXT_NORMAL,FONTSIZE_SMALL,0,FONT_YAHEI,8);


		m_pMBTempMask = new CCtrlMenuButton();
		AddControl(m_pMBTempMask);
		m_pMBTempMask->Create(this,344,254,100,80);
		m_pMBTempMask->SetMenuTextColor(0xFFFFFFFF);
		m_pMBTempMask->SetFont(FONT_YAHEI);


		std::string str = g_Config.GetTempMaskStr();
		vector<string> vec;
		g_Config.ParseStrToken(str,vec);
		for(int i=0;i<vec.size();i++)
		{
			m_pMBTempMask->AddString(vec[i].c_str());
		}
		
		m_pMBTempMask->SetCurSel(m_strTempMaskStr.c_str());

		m_pMultiEdit = new CCtrlMultiEdit(80,72,2);
		AddControl(m_pMultiEdit);
		m_pMultiEdit->Create(this,12,52,87,448,57);
		m_pMultiEdit->SetFocus();
		m_pMultiEdit->SetText(m_strTitleStr.c_str(),FONT_YAHEI);


	}
	//公共部分
	m_pOK = new CCtrlButton();
	AddControl(m_pOK);
	m_pOK->CreateEx(this, 412,343, 95, 96, 97, 98);
	m_pOK->SetText("确定", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

	if(iCurPage < PAGE_CLISERV && iCurPage >= PAGE_PICTURE)
	{
		m_pBestCompatBtn = new CCtrlButton();
		AddControl(m_pBestCompatBtn);
		m_pBestCompatBtn->CreateEx(this, 95,343, 95, 96, 97, 98);
		m_pBestCompatBtn->SetText("最佳兼容", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

		m_pBestEffectBtn = new CCtrlButton(); 
		AddControl(m_pBestEffectBtn);
		m_pBestEffectBtn->CreateEx(this, 177,343, 95, 96, 97, 98);
		m_pBestEffectBtn->SetText("最佳效果", COLOR_BTN_NORMAL ,COLOR_BTN_MOUSEON, COLOR_BTN_PRESS, COLOR_BTN_DISABLE, FONTSIZE_SMALL,0,FONT_YAHEI);

	}

	CCtrlWindowX::OnCreate();
}

bool CSysConfigWnd::OnLeftButtonDown(int iX, int iY)
{
	m_bDown =true;
	int iCurPage = m_TabPage.vSubTabPage[0].iCurPage;

	if(iCurPage == PAGE_PICTURE)
	{

		if(iX > 240 && iX < 355 && iY > 290 && iY < 310)
			m_bIsTransparence = true;

		if(iX> 240 && iX < 355 && iY > 290 && iY < 310)
		{
			if(iX - 246 < m_iTransparence)
				--m_iTransparence;
			else
				++m_iTransparence;

			if(m_iTransparence < 0)
				m_iTransparence = 0;
			else if(m_iTransparence > MAX_TRANS)
				m_iTransparence = MAX_TRANS;
		}

	}else if(iCurPage == PAGE_SOUND)
	{	
		if(iX > 280 && iX < 395 && iY > 138 && iY < 153)
			m_bIsVol = true;
		if(iX > 280 && iX< 390  && iY > 138 && iY < 153)
		{
			if(iX - 285 < m_iSndVol)
				--m_iSndVol;
			else
				++m_iSndVol;
			if(m_iSndVol < 0) m_iSndVol = 0;
			if(m_iSndVol > 100) m_iSndVol = 100;
		}

		if(iX > 280 && iX < 395 && iY > 138 + 150 && iY < 153 + 150)
			m_bIsBkVol = true;
		if(iX > 280 && iX< 390  && iY > 138 + 150 && iY < 153 + 150)
		{
			if(iX - 285 < m_iBkSndVol)
				--m_iBkSndVol;
			else
				++m_iBkSndVol;
			if(m_iBkSndVol < 0) m_iBkSndVol = 0;
			if(m_iBkSndVol > 100) m_iBkSndVol = 100;
		}
	}

	return CCtrlWindowX::OnLeftButtonDown(iX,iY);
}

bool CSysConfigWnd::OnLeftButtonUp(int iX, int iY)
{
	m_bDown = false;
	m_bIsVol = false;
	m_bIsBkVol = false;
	m_bIsTransparence = false;

	return CCtrlWindowX::OnLeftButtonUp(iX,iY);
}

bool CSysConfigWnd::OnclickOK()
{
	//对系统进行设置
	g_pAudio->Play(EAT_OTHER,1,g_pAudio->GetRand()++);


	E_UITYPE oldUiType = g_EutUiType;
	if (m_EutUiType != g_EutUiType)
	{
		if (g_TempConfig.GetWindowWidth() == 800 && m_EutUiType == EUT_FASHION)
		{
			g_MsgBoxMgr.PopSimpleAlert("时尚版界面不支持800*600分辨率。");
			return false;
		}

		if(!g_pControl->ChangeUi(m_EutUiType,false))
			return false;
	}


	bool bResetDisplay = false;

	//分辨率1024还是800x600
	if( g_TempConfig.GetWindowWidth() != g_Config.GetWindowWidth() )
	{
		g_Config.SetWindowWidth(g_TempConfig.GetWindowWidth());
		bResetDisplay = true;
	}

	// color
	if( g_TempConfig.GetRGB() != g_Config.GetRGB())
	{
		g_Config.SetRGB(g_TempConfig.GetRGB());

		if(g_Config.GetRGB() == 1)
			g_pStreamMgr->SetConfigInt("ColorDepth",32);
		else
			g_pStreamMgr->SetConfigInt("ColorDepth",16);

		bResetDisplay = true;
	}


	////垂直刷新
	//if( g_TempConfig.Get3DMode() && 
	//	g_TempConfig.GetVSync() !=  g_Config.GetVSync())
	//{
	//	g_Config.SetVSync(g_TempConfig.GetVSync());

	//	if(g_Config.GetVSync() == 1)
	//		g_pStreamMgr->SetConfigStr("VSync","Yes");
	//	else
	//		g_pStreamMgr->SetConfigStr("VSync","No");

	//	bReLogin = true;
	//}

	// 平滑模式
	if( g_TempConfig.GetHalfSpeed() != g_Config.GetHalfSpeed() )
	{
		g_Config.SetHalfSpeed( g_TempConfig.GetHalfSpeed() );
		if( g_Config.GetHalfSpeed() )
			g_pStreamMgr->SetConfigStr("HalfSpeed","Yes");
		else
			g_pStreamMgr->SetConfigStr("HalfSpeed","No");

		//g_pGfx->SetHalfSpeed( g_Config.GetHalfSpeed() );
	}

	// 窗口还是全屏幕
	if( g_TempConfig.GetAllWndMode() != g_Config.GetAllWndMode() )
	{
		g_Config.SetAllWndMode( g_TempConfig.GetAllWndMode() );
		bResetDisplay = true;
	}	

	// 自绘鼠标否？
	if( g_TempConfig.GetMouse()!=2 && g_TempConfig.GetMouse()!=g_Config.GetMouse() )
	{
		g_Config.SetMouse( g_TempConfig.GetMouse() );
		if( g_TempConfig.GetMouse() )
		{
			g_pStreamMgr->SetConfigStr("SelfCursor","Yes");
			g_pGameData->EnableSelfCursor(true);
		}
		else
		{
			g_pStreamMgr->SetConfigStr("SelfCursor","No");
			g_pGameData->EnableSelfCursor(false);
		}
	}

	//彩虹帮助
	if(g_TempConfig.GetRainbowOn() != g_Config.GetRainbowOn() )
	{
		g_Config.SetRainbowOn( g_TempConfig.GetRainbowOn() );

		if( g_TempConfig.GetRainbowOn() )
		{
//			g_pControl->Msg(MSG_CTRL_RAINBOW_BTN_SHOW,1);
			g_pStreamMgr->SetConfigStr("RainbowOn","Yes");
		}
		else 
		{
//			g_pControl->Msg(MSG_CTRL_RAINBOW_BTN_SHOW,0);
			g_pStreamMgr->SetConfigStr("RainbowOn","No");
		}
	}
	// 血条
	if( g_TempConfig.GetFloodOn() != g_Config.GetFloodOn() )
	{
		g_Config.SetFloodOn( g_TempConfig.GetFloodOn() );
		if( g_TempConfig.GetFloodOn() )
			g_pStreamMgr->SetConfigStr("ShowHP","Yes");
		else
			g_pStreamMgr->SetConfigStr("ShowHP","No");
	}

	g_Config.SetShowUIHelp(g_TempConfig.GetShowUIHelp());

	// 血条同名字颜色
	if( g_TempConfig.GetColorHP() != g_Config.GetColorHP() )
	{
		g_Config.SetColorHP( g_TempConfig.GetColorHP() );
		if( g_TempConfig.GetColorHP() )
			g_pStreamMgr->SetConfigStr("ColorHP","Yes");
		else
			g_pStreamMgr->SetConfigStr("ColorHP","No");
	}

	// 光影
	if( g_TempConfig.GetLight()!=2 && g_TempConfig.GetLight()!=g_Config.GetLight() )
	{
		g_Config.SetLight( g_TempConfig.GetLight() );
		if( g_TempConfig.GetLight() )
		{
			g_pStreamMgr->SetConfigStr("Light","Yes");
//			g_pLight->EnableLight(true);
		}
		else
		{
			g_pStreamMgr->SetConfigStr("Light","No");
//			g_pLight->EnableLight(false);
		}
	}

	// 天气效果
	if( g_TempConfig.GetWeather()!=2 && g_TempConfig.GetWeather()!=g_Config.GetWeather() )
	{
		g_Config.SetWeather( g_TempConfig.GetWeather() );

		if( g_TempConfig.GetWeather() )
		{
			g_pStreamMgr->SetConfigStr("Weather","Yes");

			if(g_pWeather->GetParticleType() == PARTICLE_RAIN)
			{
				g_pAudio->Stop(EAT_OTHER,257,g_pAudio->GetAudioRain());
				g_pAudio->GetAudioRain() = g_pAudio->GetRand();
				g_pAudio->Play(EAT_OTHER,256,g_pAudio->GetRand()++,true);
			}
			else if(g_pWeather->GetParticleType() == PARTICLE_STORM)
			{
				g_pAudio->Stop(EAT_OTHER,256,g_pAudio->GetAudioRain());
				g_pAudio->GetAudioRain() = g_pAudio->GetRand();
				g_pAudio->Play(EAT_OTHER,257,g_pAudio->GetRand()++,true);
			}
		}
		else
		{ 
			g_pStreamMgr->SetConfigStr("Weather","No");
			g_pAudio->Stop(EAT_OTHER,256,g_pAudio->GetAudioRain());
			g_pAudio->Stop(EAT_OTHER,257,g_pAudio->GetAudioRain());
		}
	}

	// 纹理预读
	if( g_TempConfig.GetTexPrepare() != g_Config.GetTexPrepare() )
	{
		g_Config.SetTexPrepare( g_TempConfig.GetTexPrepare() );
		if( g_TempConfig.GetTexPrepare() == 0 )
		{
			g_pStreamMgr->SetConfigStr("MemoryCost","No");
			g_iPreReadMode = PREREAD_NONE;
		} 
		else if( g_TempConfig.GetTexPrepare() == 1 )
		{
			g_pStreamMgr->SetConfigStr("MemoryCost","Auto");
			g_iPreReadMode = PREREAD_AUTO;
		}
		else
		{
			g_pStreamMgr->SetConfigStr("MemoryCost","Normal");
			g_iPreReadMode = PREREAD_FULL;
		}
	}

	// 平滑
	if( g_TempConfig.GetSmooth() != g_Config.GetSmooth() )
	{
		int j = g_Config.GetSmooth();
		g_Config.SetSmooth( g_TempConfig.GetSmooth() );
		if( g_TempConfig.GetSmooth() == 0 )
		{
			g_pStreamMgr->SetConfigStr("Smooth","0");
			if( j == 2 )
				g_pControl->Msg(MSG_CTRL_DOUBLEMAGIC,0);
		} 
		else if( g_TempConfig.GetSmooth() == 1)
		{
			g_pStreamMgr->SetConfigStr("Smooth","1");
			if( j == 2 )
				g_pControl->Msg(MSG_CTRL_DOUBLEMAGIC,0);
		}
		else
		{
			g_pStreamMgr->SetConfigStr("Smooth","2");
			if( j < 2 )
				g_pControl->Msg(MSG_CTRL_DOUBLEMAGIC,1);
		}
	}

	//个性化设置
	string str = g_Config.GetTempMaskStr();
	int iPos = str.find_first_of('/');
	if(iPos < 0) iPos = str.find_first_of('\\');

	if(iPos > 0 && str.size() > 0)
		str = str.substr(0,iPos);

	bool bTempMask = g_TempConfig.GetTempMask();

	if(g_TempConfig.GetHusbandAndWife() != g_Config.GetHusbandAndWife() ||
		g_TempConfig.GetShaMember() != g_Config.GetShaMember() ||
		g_TempConfig.GetOpenWing() != g_Config.GetOpenWing() ||
		strcmp(g_TempConfig.GetTitleStr(),g_Config.GetTitleStr()) != 0 ||
		bTempMask != g_Config.GetTempMask() ||
		(bTempMask && strcmp(g_TempConfig.GetTempMaskStr(),str.c_str()) != 0)||
		g_TempConfig.GetOpenShine() != g_Config.GetOpenShine())
	{
		g_pGameControl->SEND_Person_Setting();
	}


	if(bResetDisplay)
	{
		int iWidth = g_Config.GetWindowWidth();
		//if (iWidth <= 800 && g_EutUiType == EUT_FASHION)
		//{
		//	g_MsgBoxMgr.PopSimpleAlert("时尚版界面不支持800*600分辨率，要使用800*600分辨率请先切换成经典版界面。");
		//	g_Config.SetWindowWidth(iOldWidth);
		//	g_pStreamMgr->SetConfigInt("ScreenMode",iOldWidth); 
		//	g_pControl->ChangeUi(oldUiType);
		//	return false;
		//}

		int iHeight = g_pGfx->GetHeight();

		if (iWidth == 1024)
		{
			iHeight = 768;
		}
		else if (iWidth == 1280)
		{
			iHeight = 800;
		}
		else if (iWidth == 1600)
		{
			iHeight = 900;
		}
		else if (iWidth == 1920)
		{
			iHeight = 1080;
		}
		else if (iWidth == 2560)
		{
			iHeight = 1440;
		}
		else
		{
			iWidth = 800;
			iHeight = 600;
		}


		DisplayMode eNewDisMode = DM_WINDOWED;
		if (!g_Config.GetAllWndMode())
		{
			eNewDisMode = DM_FULL_FALSE;;
		}


		if(!ChangeWindowSize(iWidth,iHeight,eNewDisMode))
		{
			g_pControl->ChangeUi(oldUiType);
		}

	}

	g_pControl->PopupWindow(MSG_CTRL_SYSCONFIG_WND,OPER_CLOSE);

	return true;
}