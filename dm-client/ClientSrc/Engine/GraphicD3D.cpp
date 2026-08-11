#include "stdafx.h"
#include "GraphicD3D.h"
#include "TextureD3D.h"
#include "Global/DebugTry.h"
#include "Global/Interface/CallBackInterface.h"
#include "TexManager.h"

#include "Global/PerfCheck.h"
#include "BaseClass/Compress/Compr.h"
#include "Global/MathUtil.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include <stdio.h>


#define M_DIS_WIDTH			0
#define M_DIS_HEIGHT		60

#define MAX_WINDOW_WIDTH            2560
#define MAX_WINDOW_HEIGHT           1440


#define ATOM_RECT_SIZE	16
#define MIX_RECT_SIZE	128
#define CLOUDSKIPFRAME	6	//控制cloud材质更新速度
#define STORMSKIPFRAME	3	//控制storm材质更新速度

#define TEX_POOL_TYPE	D3DPOOL_DEFAULT

#define INIT_OLD_POOL(pool,w,h,max)		s_OldPool[pool].D3DFormat	= D3DFMT_A4R4G4B4;\
	s_OldPool[pool].PoolType	= TEX_POOL_TYPE;\
	s_OldPool[pool].bCanCreate	= TRUE;\
	s_OldPool[pool].bCreateFail = FALSE;\
	s_OldPool[pool].iWidth		= w;\
	s_OldPool[pool].iHeight		= h;\
	s_OldPool[pool].iMaxSize	= max;\
	s_OldPool[pool].iCounter	= 0;\
	s_OldPool[pool].Pool.SetCount(&m_dwFrameCount)


#define INIT_VIDEO_POOL(pool,w,h,max)	s_TexPool[pool].D3DFormat	= m_bDXTn ? D3DFMT_DXT3 : D3DFMT_A4R4G4B4;\
	s_TexPool[pool].PoolType	= TEX_POOL_TYPE;\
	s_TexPool[pool].bCanCreate	= TRUE;\
	s_TexPool[pool].bCreateFail = FALSE;\
	s_TexPool[pool].iWidth		= w;\
	s_TexPool[pool].iHeight		= h;\
	s_TexPool[pool].iMaxSize	= max;\
	s_TexPool[pool].iCounter	= 0;\
	s_TexPool[pool].Pool.SetCount(&m_dwFrameCount)

#define INIT_FONT_POOL(pool,w,h,max)	s_FontPool[pool].D3DFormat	= D3DFMT_A4R4G4B4;\
	s_FontPool[pool].PoolType	= TEX_POOL_TYPE;\
	s_FontPool[pool].bCanCreate	= TRUE;\
	s_FontPool[pool].bCreateFail= FALSE;\
	s_FontPool[pool].iWidth		= w;\
	s_FontPool[pool].iHeight	= h;\
	s_FontPool[pool].iMaxSize	= max;\
	s_FontPool[pool].iCounter	= 0;\
	s_FontPool[pool].Pool.SetCount(&m_dwFrameCount)

#define bCanDraw		(m_bFrameDraw && m_bD3dOK && m_bDisplay)

BOOL Combine_DXT3_DXT1(BYTE *pDXT3,BYTE *pDXT1,int w,int h);
BOOL Convert(int iDstType,BYTE *pDst,int pitchs1,int h1,int iSrcType,BYTE *pSrc,int w0,int h0);


CGraphicD3D::CGraphicD3D(void)
{
	hr				= -1;
	m_dwAvailMem	= 0;
	m_dwAlloacMem	= 0;
	m_dwShouldMem	= 0;
	m_iVertexBuffer = 0;

	m_hWnd			= NULL;
	m_iWidth		= 0;
	m_iHeight		= 0;
	m_eDisMode		= DM_WINDOWED;
	m_bGameMode		= TRUE;
	m_bDXTn			= FALSE;
	m_bMultiTex		= FALSE;
	m_bMemSurf		= FALSE;

	m_bD3dOK		= FALSE;
	m_bDisplay		= FALSE;
	m_bFrameDraw	= FALSE;

	m_dwStartTime	= 0;
	m_dwFrameCount	= 1;
	m_dwRealFPS		= 0;
	m_dwTotalRealFps = 0;
	m_iNextTime		= 0;
	m_iPerFmeTime	= 0;
	m_p3d			= NULL;
	m_p3dDev		= NULL;
	m_pBackSurf		= NULL;
	m_pMemSurf		= NULL;
	m_pSprite		= NULL;
	m_pLight		= NULL;
	m_pLightSurf	= NULL;
	m_pTempRenderTarget = NULL;
	m_pTempRenderTargetSurf = NULL;

	m_pRectTex		= NULL;
	m_pMiniMapTex	= NULL;
	m_pTileTex		= NULL;

	for(int i = 0;i < MAX_CLOUDSTEX;i++)
		m_pCloudTex[i] = NULL;

	for(int i = 0;i < MAX_VERTEXBUFFER;i++)
		m_pVertexBuffer[i] = NULL;

	m_rcAtom.left	= 3;
	m_rcAtom.top	= 3;
	m_rcAtom.right	= 4;
	m_rcAtom.bottom	= 4;

	// 默认帧速
	SetFPS(50);
	memset(&m_d3dpp,0,sizeof(m_d3dpp));

	m_hThread		= NULL;
	m_bSaving		= FALSE;
	m_bExited		= FALSE;
	m_bRenderIGW  = false;
	m_bNeedDraw = true;
}

CGraphicD3D::~CGraphicD3D(void)
{
	KillD3D();

	if(m_hThread)
	{
		m_bExited = TRUE;
		ResumeThread(m_hThread);
		WaitForSingleObject(m_hThread,INFINITE);
		CloseHandle(m_hThread);
	}

	m_hThread		= NULL;
	m_bSaving		= FALSE;
	m_bExited		= FALSE;
}

// 创建D3D
BOOL CGraphicD3D::CreateD3D(HWND hWnd,int iW,int iH,DisplayMode eDisplayMode,BOOL bGameMode,bool bRestoreDesktop)
{
	if(!bGameMode)
	{
		RECT rc;
		GetClientRect(hWnd,&rc);
		iW = rc.right - rc.left;
		iH = rc.bottom - rc.top;
	}

	// 保存设置
	m_hWnd		= hWnd;
	m_iWidth	= iW;
	m_iHeight	= iH;
	m_eDisMode	= eDisplayMode;
	m_bGameMode = bGameMode;

	// 创建D3D8接口
	m_p3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_p3d)
		return FALSE;

	// 取得桌面显示信息
	static BOOL bGotDesk = FALSE;
	if(!bGotDesk && !GetDisplayInfo())
		return FALSE;
	bGotDesk = TRUE;

	// 调整窗口和桌面分辨率
	if(eDisplayMode != DM_FULL_TRUE && bGameMode)
	{
		if (bRestoreDesktop)
		{
			// 非真全屏模式时修改桌面分辨率
			if(!RestoreDesktop(FALSE))
			{
				if (m_eDisMode != DM_WINDOWED)
				{	
					char strTemp[256]={0};
					sprintf(strTemp,"你的机器不支持%d*%d分辨率,是否改用窗口模式?",iW,iH);
					if(MessageBox(m_hWnd,strTemp,NULL,MB_YESNO) == IDYES)
					{
						string strPath = g_pCallBack->GetDataDir();
						strPath += "\\Config.ini";
						WritePrivateProfileString("Config","FullScreen","No",strPath.c_str());

						m_eDisMode = DM_WINDOWED;

						if(!RestoreDesktop(FALSE))
							return FALSE;
					}
				}

				return FALSE;
			}
		}
		

		// 调整窗口大小
		AdjustWindow(hWnd,iW,iH,m_eDisMode == DM_FULL_FALSE,TRUE);

	}



	

	// 设置D3D设备参数
	D3DPRESENT_PARAMETERS d3dpp;
	memset(&d3dpp,0,sizeof(d3dpp));
	d3dpp.BackBufferCount	= 1;
	d3dpp.BackBufferWidth	= MAX_WINDOW_WIDTH;
	d3dpp.BackBufferHeight	= MAX_WINDOW_HEIGHT;
	d3dpp.MultiSampleType	= D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect		= D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow		= hWnd;
	d3dpp.Windowed			= (eDisplayMode < DM_FULL_TRUE) ? TRUE : FALSE;
	d3dpp.Flags				= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat	= (m_d3ddm.Format == D3DFMT_X8R8G8B8) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
	//播放的时候为了能够加速,就不限制帧数
	if (g_pCallBack && g_pCallBack->IsInReplay())
	{
		d3dpp.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	if(m_eDisMode == DM_FULL_TRUE)
	{
		if(m_DisInfo_Enter.width >= (DWORD)m_iWidth && m_DisInfo_Enter.height >= (DWORD)m_iHeight)
			d3dpp.FullScreen_RefreshRateInHz = m_DisInfo_Enter.rate;
		else
			d3dpp.FullScreen_RefreshRateInHz = 65;
	}

	// 创建D3D设备
	m_d3dpp = d3dpp;
	hr = m_p3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp,&m_p3dDev);

	if(FAILED(hr))
	{
		d3dpp = m_d3dpp;
		hr = m_p3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_MIXED_VERTEXPROCESSING,&d3dpp,&m_p3dDev);
	}

	if(FAILED(hr))
	{
		d3dpp = m_d3dpp;
		hr = m_p3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&m_p3dDev);
	}

	if(FAILED(hr))
	{
		d3dpp = m_d3dpp;
		hr = m_p3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_REF,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&m_p3dDev);
	}

	if(FAILED(hr))
		return FALSE;

	m_d3dpp = d3dpp;

	//need uncomment


	// D3DSprite
	hr = D3DXCreateSprite(m_p3dDev,&m_pSprite);
	if(hr != D3D_OK)
		return FALSE;

	// 填充矩形纹理
	hr = m_p3dDev->CreateTexture(ATOM_RECT_SIZE,ATOM_RECT_SIZE,1,0,D3DFMT_A4R4G4B4,D3DPOOL_MANAGED,&m_pRectTex,NULL);
	if(hr != D3D_OK)
		return FALSE;
	else
	{
		D3DLOCKED_RECT lr;
		hr = m_pRectTex->LockRect(0,&lr,NULL,0);
		if(hr == D3D_OK && lr.pBits)
			memset(lr.pBits,0xFF,ATOM_RECT_SIZE * ATOM_RECT_SIZE * 2);
		m_pRectTex->UnlockRect(0);
	}

	// 在系统内存离屏表面
	hr = m_p3dDev->CreateOffscreenPlainSurface(MAX_WINDOW_WIDTH,MAX_WINDOW_HEIGHT,m_d3dpp.BackBufferFormat,D3DPOOL_SYSTEMMEM,&m_pMemSurf,NULL);
	if(hr != D3D_OK)
		return FALSE;

	// 查看是否支持多重纹理混合
	hr = m_p3dDev->GetDeviceCaps(&m_caps);
	if(hr == D3D_OK && m_caps.MaxTextureBlendStages > 2)
		m_bMultiTex = TRUE;

	// 查看是否支持DXT3格式
	hr = m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,m_d3dpp.BackBufferFormat,0,D3DRTYPE_TEXTURE,D3DFMT_DXT3);
	if(hr == D3D_OK)
		m_bDXTn = TRUE;

	// 小地图纹理
	hr = m_p3dDev->CreateTexture(MIX_RECT_SIZE,MIX_RECT_SIZE,1,0,D3DFMT_A4R4G4B4,D3DPOOL_MANAGED,&m_pMiniMapTex,NULL);
	if(hr != D3D_OK)
		m_pMiniMapTex = NULL;

	// 大地表混合部分
	if(!m_bMultiTex)
	{
		hr = m_p3dDev->CreateTexture(MIX_RECT_SIZE,MIX_RECT_SIZE,1,0,m_bDXTn ? D3DFMT_DXT3 : D3DFMT_A4R4G4B4,D3DPOOL_MANAGED,&m_pTileTex,NULL);
		if(hr != D3D_OK)
			m_pTileTex = NULL;
	}

	// 获取显卡名称信息等
	m_p3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT,0,&m_AdapterInfo);
	m_dwAvailMem = m_p3dDev->GetAvailableTextureMem();

	if(m_dwAvailMem > 48 * 1024 * 1024)
	{
		//设置图片纹理池参数			  w	  h	 max
		INIT_VIDEO_POOL(TEX_POOL_032_032, 32, 32,300);	// 1024
		INIT_VIDEO_POOL(TEX_POOL_032_064, 32, 64,300);	// 2048
		INIT_VIDEO_POOL(TEX_POOL_032_128, 32,128,300);	// 4096
		INIT_VIDEO_POOL(TEX_POOL_032_256, 32,256,150);	// 8192
		INIT_VIDEO_POOL(TEX_POOL_064_032, 64, 32,300);	// 2048
		INIT_VIDEO_POOL(TEX_POOL_064_064, 64, 64,300);	// 4096
		INIT_VIDEO_POOL(TEX_POOL_064_128, 64,128,200);	// 8192
		INIT_VIDEO_POOL(TEX_POOL_064_256, 64,256,300);	// 16384
		INIT_VIDEO_POOL(TEX_POOL_128_128,128,128,400);	// 16384
		INIT_VIDEO_POOL(TEX_POOL_256_256,256,256,96);	// 
		INIT_VIDEO_POOL(TEX_POOL_512_512,512,512,8);	// 


		INIT_OLD_POOL(OLD_POOL_032_032	,32	, 32,200);
		INIT_OLD_POOL(OLD_POOL_064_064	,64,  64,250);
		INIT_OLD_POOL(OLD_POOL_064_256	,64, 256,200);
		INIT_OLD_POOL(OLD_POOL_128_016	,128, 16, 96);
		INIT_OLD_POOL(OLD_POOL_128_064  ,128, 64, 96);
		INIT_OLD_POOL(OLD_POOL_128_128  ,128,128,250);
		INIT_OLD_POOL(OLD_POOL_256_016	,256, 16, 48);
		INIT_OLD_POOL(OLD_POOL_256_064	,256, 64, 48);
		INIT_OLD_POOL(OLD_POOL_256_128	,256,128, 64);
		INIT_OLD_POOL(OLD_POOL_256_256  ,256,256,100);
		INIT_OLD_POOL(OLD_POOL_512_512  ,512,512,4);

		m_dwShouldMem = 38 * 1024 * 1024;
	}
	else
	{
		//设置图片纹理池参数			  w	  h	 max
		INIT_VIDEO_POOL(TEX_POOL_032_032, 32, 32,150);  // 1024
		INIT_VIDEO_POOL(TEX_POOL_032_064, 32, 64,150);  // 2048
		INIT_VIDEO_POOL(TEX_POOL_032_128, 32,128,150);  // 4096
		INIT_VIDEO_POOL(TEX_POOL_032_256, 32,256,150);  // 8192
		INIT_VIDEO_POOL(TEX_POOL_064_032, 64, 32,200);  // 2048
		INIT_VIDEO_POOL(TEX_POOL_064_064, 64, 64,300);  // 4096
		INIT_VIDEO_POOL(TEX_POOL_064_128, 64,128,200);  // 8192
		INIT_VIDEO_POOL(TEX_POOL_064_256, 64,256,300);  // 16384
		INIT_VIDEO_POOL(TEX_POOL_128_128,128,128,300);  // 16384
		INIT_VIDEO_POOL(TEX_POOL_256_256,256,256,96);	// 
		INIT_VIDEO_POOL(TEX_POOL_512_512,512,512,8);	// 

		INIT_OLD_POOL(OLD_POOL_032_032	, 32, 32,100);
		INIT_OLD_POOL(OLD_POOL_064_064	, 64, 64,200);
		INIT_OLD_POOL(OLD_POOL_064_256	, 64,256,150);
		INIT_OLD_POOL(OLD_POOL_128_016	,128, 16, 64);
		INIT_OLD_POOL(OLD_POOL_128_064	,128, 64, 64);
		INIT_OLD_POOL(OLD_POOL_128_128,  128,128,200);
		INIT_OLD_POOL(OLD_POOL_256_016	,256, 16, 32);
		INIT_OLD_POOL(OLD_POOL_256_064	,256, 64, 32);
		INIT_OLD_POOL(OLD_POOL_256_128	,256,128, 48);
		INIT_OLD_POOL(OLD_POOL_256_256,  256,256, 64);
		INIT_OLD_POOL(OLD_POOL_512_512  ,512,512, 4);

		m_dwShouldMem = 26 * 1024 * 1024;
	}

	INIT_FONT_POOL(FTST_032_032, 32,32,200); //1024
	INIT_FONT_POOL(FTST_064_064, 64,64,250); //2048
	INIT_FONT_POOL(FTST_128_016,128,16,200); //2048
	INIT_FONT_POOL(FTST_128_032,128,32, 32); //4096
	INIT_FONT_POOL(FTST_128_064,128,64, 16); //8192
	INIT_FONT_POOL(FTST_128_128,128,128,16); //16384
	INIT_FONT_POOL(FTST_256_064,256,64,16);  //16384

	strlwr(m_AdapterInfo.Description);

	// 判断是否是Intel的集成显卡
	if(strstr(m_AdapterInfo.Description,"intel"))
	{
		if(strstr(m_AdapterInfo.Description,"810") || strstr(m_AdapterInfo.Description,"815") || strstr(m_AdapterInfo.Description,"830"))
			m_dwShouldMem = m_dwAvailMem - 13 * 1024 * 1024;		// 需要留13M最好
	}
	else if(strstr(m_AdapterInfo.Description,"via"))
	{
		if(m_dwAvailMem > 12 * 1024 * 1024)		// 多余12M的留6M最好
			m_dwShouldMem = m_dwAvailMem - 6 * 1024 * 1024;
	}

	if(InitTexPool(TRUE) == FALSE)
		return FALSE;

	if(InitDXStates() == FALSE)
		return FALSE;

	//m_dwFrameCount	= 1;//这里重置会有问题,比如在int iPastFrame = m_dwFrameCount - ActionNow.tFrameStart;
	m_bD3dOK		= TRUE;
	m_bDisplay		= TRUE;
	m_bFrameDraw	= TRUE;
	m_dwStartTime	= GetTickCount();
	m_iNextTime		= m_dwStartTime;

	m_pThreadBuf	= new BYTE[MAX_WINDOW_WIDTH * MAX_WINDOW_HEIGHT * 4];

	m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0,0,0);

	return TRUE;
}

// 退出D3D
void CGraphicD3D::KillD3D(bool bRestoreDesktop)
{
	SAFE_DELETE_ARRAY(m_pThreadBuf);
	ReleaseTexPool(TRUE);
	if(m_pMemSurf && m_bMemSurf)
	{
		m_bMemSurf = FALSE;
		m_pMemSurf->UnlockRect();
	}

	SAFE_RELEASE(m_pRectTex);
	SAFE_RELEASE(m_pMiniMapTex);
	SAFE_RELEASE(m_pTileTex);

	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pMemSurf);
	SAFE_RELEASE(m_pBackSurf);



	SAFE_RELEASE(m_p3dDev);
	SAFE_RELEASE(m_p3d);

	if(g_pCallBack)
		g_pCallBack->OnKillD3D();

	// 恢复为进游戏前的桌面分辨率
	if (bRestoreDesktop)
	{
		RestoreDesktop(TRUE);
	}
}

void CGraphicD3D::CenterWindow()
{
	MoveWindow(m_hWnd, 0, 0, 0, 0, TRUE, TRUE);
}

BOOL CGraphicD3D::MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint, BOOL bCenter)
{
	// 获取窗口当前尺寸（如果你已经知道宽高，也可以直接传值）
	RECT rcWnd;
	GetWindowRect(m_hWnd, &rcWnd);
	int w = rcWnd.right - rcWnd.left;
	int h = rcWnd.bottom - rcWnd.top;
	if (0 < nWidth)
	{
		w = nWidth;
	}
	if (0 < nHeight)
	{
		h = nHeight;
	}

	// 获取屏幕工作区（排除任务栏）或全屏尺寸
	// 方法1：全屏尺寸（含任务栏）
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);

	// 方法2：工作区（不含任务栏）——推荐
	// RECT rcWork;
	// SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
	// int screenW = rcWork.right - rcWork.left;
	// int screenH = rcWork.bottom - rcWork.top;

	int x = X;
	int y = Y;
	if (bCenter)
	{
		x = (screenW - w) / 2;
		y = (screenH - h) / 2;
	}

	return ::MoveWindow(hWnd, x, y, w, h, bRepaint);
}

// 修改游戏窗口的客户区域的大小
void CGraphicD3D::AdjustWindow(HWND hWnd,int iW,int iH,BOOL bPopup, BOOL bCenter)
{
	if(bPopup)
	{
		SetWindowLong(hWnd,GWL_STYLE,WS_POPUP|WS_VISIBLE);
		MoveWindow(hWnd,0,0,iW,iH,TRUE, bCenter);
	}
	else
	{
		if(g_pCallBack && g_pCallBack->GetParentHandle())
		{
			SetWindowLong(hWnd,GWL_STYLE,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
			MoveWindow(hWnd,0,0,iW,iH,TRUE, bCenter);
		}
		else
		{
			RECT	rc,rc1;
			//DWORD	style = WS_OVERLAPPED|WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW|WS_VISIBLE;
			DWORD	style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE;

			SetRect(&rc,0,0,iW,iH);
			AdjustWindowRect(&rc,style,FALSE);

			// 设置窗口类型和位置
			SetWindowLong(hWnd,GWL_STYLE,style);
			GetWindowRect(hWnd,&rc1);
			BOOL bRtn = MoveWindow(hWnd,rc1.left,rc1.top,rc.right - rc.left,rc.bottom - rc.top,TRUE, bCenter);

			if (!bRtn)
			{
				int i = 3;
				return;
			}
		}
	
	}

	//SetActiveWindow(m_hWnd);
}

void CGraphicD3D::RecomputeWindowXY(int & iW, int & iH, int iOW, int iOH, DisplayMode eWinMode)
{
	iW = iOW;
	iH = iOH;

	POINT ptDis[6] = { {800,600},{1024,768},{1280,800},{1600,900},{1920,1080},{2560,1440} };

	int maxDisW = 0;
	int maxDisH = 0;

	DEVMODE DisInfo;
	for (int iMode = 0; EnumDisplaySettings(NULL, iMode, &DisInfo) != 0; ++iMode)
	{
		if (DisInfo.dmPelsWidth > maxDisW)
			maxDisW = DisInfo.dmPelsWidth;
		if (DisInfo.dmPelsHeight > maxDisH)
			maxDisH = DisInfo.dmPelsHeight;
	}

	if (DM_WINDOWED == eWinMode)
	{
		maxDisW -= M_DIS_WIDTH;
		maxDisH -= M_DIS_HEIGHT;
	}

	if (iOW <= maxDisW && iOH <= maxDisH)
	{
		iW = iOW;
		iH = iOH;
		return;
	}

	for (int i = 2; i >= 0; --i)
	{
		if (ptDis[i].x <= maxDisW && ptDis[i].y <= maxDisH)
		{
			//去比桌面最大分辨率小的一个
			iW = ptDis[i].x;
			iH = ptDis[i].y;
			return;
		}
	}

	//取最小的
	iW = ptDis[0].x;
	iH = ptDis[0].y;
}

void CGraphicD3D::RecomputeDesktopXY(int & iDskW, int & iDskH, int iWinW, int iWinH, DisplayMode eWinMode, int iCurDisWidth, int iCurDisHeight)
{
	// *语句1
	if (DM_WINDOWED == eWinMode)
	{
		if ((iCurDisWidth - M_DIS_WIDTH) >= iWinW && (iCurDisHeight - M_DIS_HEIGHT) >= iWinH)
		{
			iDskW = iCurDisWidth;
			iDskH = iCurDisHeight;
			return;
		}

		if ((m_DisInfo_Enter.width - M_DIS_WIDTH) >= iWinW && (m_DisInfo_Enter.height - M_DIS_HEIGHT) >= iWinH)
		{
			iDskW = m_DisInfo_Enter.width;
			iDskH = m_DisInfo_Enter.height;
			return;
		}
	}

	

	// 全屏/窗口模式下需要验证是否分辨率支持
	// 窗口模式之所以也需要验证，是因为对方可能在1024*768的情况下打开一个
	// 1280*800的窗口，而有些情况下玩家的显卡是支持的
	int iSelWidth = (DM_WINDOWED == eWinMode) ? iCurDisWidth : min(iCurDisWidth, 1280);
	int iSelHeight = (DM_WINDOWED == eWinMode) ? iCurDisHeight : min(iCurDisHeight, 1440);

	int iTempWidth;
	int iTempHeight;

	bool bSpt = false;
	int iMode = 0;
	DEVMODE DisInfo;
	int iRet = EnumDisplaySettings(NULL, iMode, &DisInfo);
	while (iRet != 0)
	{
		if (DM_WINDOWED == eWinMode)
		{
			iTempWidth = DisInfo.dmPelsWidth - M_DIS_WIDTH;
			iTempHeight = DisInfo.dmPelsHeight - M_DIS_HEIGHT;
		}
		else
		{
			iTempWidth = DisInfo.dmPelsWidth;
			iTempHeight = DisInfo.dmPelsHeight;
		}

		if (DM_WINDOWED == eWinMode &&
			iTempWidth >= iWinW && iTempHeight >= iWinH &&
			iTempWidth <= 2560 && iTempHeight <= 1440 ||
			DM_WINDOWED != eWinMode &&
			iTempWidth <= min(iWinW, 2560) && iTempHeight <= min(iWinH, 1440) &&
			iTempWidth >= 800 && iTempHeight >= 600) // 最小的全屏支持到800*600
		{
			// 记录下可支持的最接近于设定分辨率的一个分辨率
			// 本while循环是为了找到显卡可支持的分辨率，而1280和800的限定则
			// 是我们游戏可以支持的最大全屏分辨率，1280是窗口模式最大桌面宽
			// 度，800是窗口模式最大桌面高度
			iSelWidth = DisInfo.dmPelsWidth;
			iSelHeight = DisInfo.dmPelsHeight;

			if (iWinW == iTempWidth && iWinH == iTempHeight)
			{
				iDskW = iWinW;
				iDskH = iWinH;
				bSpt = true;
				break;
			}

			// 对于窗口模式而言，找到第一个比窗口大的分辨率就可以跳出循环了
			if (DM_WINDOWED == eWinMode)
				break;
		}
		else if (DM_WINDOWED == eWinMode)
		{
			// 如果跳过了语句1，则说明窗口模式试图扩大，那么就给它找到
			// 最大的可支持分辨率
			// 采用面积算法，取面积最接近于需要窗口的一种分辨率
			//int iTY0 = iWinW * iWinH;
			//int iTY1 = ((DisInfo.dmPelsWidth >= iWinW) ? iWinW : DisInfo.dmPelsWidth) *
			//	((DisInfo.dmPelsHeight >= iWinH) ? iWinH : DisInfo.dmPelsHeight);
			//int iTY2 = ((iSelWidth >= iWinW) ? iWinW : iSelWidth) *
			//	((iSelHeight >= iWinH) ? iWinH : iSelHeight);
			//if (abs(iTY0 - iTY1) < abs(iTY0 - iTY2) && (iTY0 - iTY1) )
			//{
			//	iSelWidth = DisInfo.dmPelsWidth;
			//	iSelHeight = DisInfo.dmPelsHeight;
			//}


			// 取比要设置的分辨率大且面积最小的一个
			if (iTempWidth >= iWinW && iTempHeight >= iWinH)
			{
				if ((iSelWidth - M_DIS_WIDTH) < iWinW || (iSelHeight - M_DIS_HEIGHT) < iWinH)
				{
					iSelWidth = DisInfo.dmPelsWidth;
					iSelHeight = DisInfo.dmPelsHeight;
				}
				else if (DisInfo.dmPelsWidth * DisInfo.dmPelsHeight <  iSelWidth * iSelHeight)
				{
					iSelWidth = DisInfo.dmPelsWidth;
					iSelHeight = DisInfo.dmPelsHeight;
				}
			}
		}

		iMode++;
		iRet = EnumDisplaySettings(NULL, iMode, &DisInfo);
	}

	if (!bSpt)
	{
		iDskW = iSelWidth;
		iDskH = iSelHeight;
	}
}


// 设置桌面的分辨率
BOOL CGraphicD3D::RestoreDesktop(BOOL bOld)
{
	DEVMODE	dm;
	memset(&dm,0,sizeof(DEVMODE));

	if(!bOld)
	{
		//AdjustWindow(m_hWnd,m_iWidth,m_iHeight,m_eDisMode == DM_FULL_FALSE);

		// 窗口模式下需要主动修改桌面分辨率
		if(m_eDisMode < DM_FULL_TRUE)
		{
			DesktopInfo lCurDesktopInfo;
			if (!GetCurScreenInfo(lCurDesktopInfo))
				return FALSE;

			BOOL bChangeDisplay		= FALSE;

			dm.dmSize				= sizeof(DEVMODE);
			dm.dmFields				= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
			dm.dmBitsPerPel			= (lCurDesktopInfo.bits == 32) ? 32 : 16;
			dm.dmPelsWidth			= lCurDesktopInfo.width;
			dm.dmPelsHeight			= lCurDesktopInfo.height;
			dm.dmDisplayFrequency	= lCurDesktopInfo.rate;
			if (dm.dmDisplayFrequency < 60 && (dm.dmDisplayFrequency % 5) != 0)//测评有不少高分辨率时刷新率是59的,变成低分辨率时不支持59了
			{
				dm.dmDisplayFrequency = 60;
			}

			// 查看桌面是否是16或者24或者32位色
			if(lCurDesktopInfo.bits != 32 && lCurDesktopInfo.bits != 16)
			{
				bChangeDisplay			= TRUE;
				if(lCurDesktopInfo.bits < 16)
					dm.dmDisplayFrequency = 60;
			}

			// 窗口模式不一样也让他改回原来桌面的分辨率
			//if(m_eDisMode == DM_WINDOWED && (m_DisInfo_Enter.width != m_iWidth || m_DisInfo_Enter.height != m_iHeight) &&
			//	(m_DisInfo_Enter.width != lCurDesktopInfo.width || m_DisInfo_Enter.height != lCurDesktopInfo.height)
			//	)
			//{
			//	bChangeDisplay	= TRUE;
			//	dm.dmPelsWidth	= m_DisInfo_Enter.width;
			//	dm.dmPelsHeight	= m_DisInfo_Enter.height;
			//}

			// 非窗口模式桌面分辨率低于游戏需求分辨率，修改分辨率
			if(m_eDisMode != DM_WINDOWED && (lCurDesktopInfo.width < (DWORD)m_iWidth || lCurDesktopInfo.height < (DWORD)m_iHeight))
			{
				bChangeDisplay			= TRUE;
				dm.dmPelsWidth			= m_iWidth;
				dm.dmPelsHeight			= m_iHeight;
				dm.dmDisplayFrequency	= 60;
			}

			// 桌面分辨率低于游戏需求分辨率，修改分辨率
			if((m_eDisMode == DM_WINDOWED) && ((lCurDesktopInfo.width - M_DIS_WIDTH) < (DWORD)m_iWidth || (lCurDesktopInfo.height - M_DIS_HEIGHT) < (DWORD)m_iHeight))
			{
				int iDskWidth, iDskHeight;
				RecomputeDesktopXY(iDskWidth, iDskHeight, m_iWidth, m_iHeight, m_eDisMode,lCurDesktopInfo.width,lCurDesktopInfo.height);
				bChangeDisplay			= TRUE;
				dm.dmPelsWidth			= iDskWidth;
				dm.dmPelsHeight			= iDskHeight;
				dm.dmDisplayFrequency	= 60;
			}

			// 在假全屏模式时，桌面分辨率和游戏分辨率不一样也修改
			if(m_eDisMode == DM_FULL_FALSE && (lCurDesktopInfo.width != m_iWidth || lCurDesktopInfo.height != m_iHeight))
			{
				bChangeDisplay	= TRUE;
				dm.dmPelsWidth	= m_iWidth;
				dm.dmPelsHeight	= m_iHeight;
			}



			//// 窗口模式不一样也让他改回原来桌面的分辨率，应该是上次和当前的窗口分辨率不一样时要修改
			//HDC hdc = GetDC(NULL);
			//if(hdc && !bChangeDisplay && m_eDisMode == DM_WINDOWED && (lCurDesktopInfo.width != GetDeviceCaps(hdc,HORZRES) || lCurDesktopInfo.height != GetDeviceCaps(hdc,VERTRES)))
			//{
			//	bChangeDisplay	= TRUE;
			//	dm.dmPelsWidth	= lCurDesktopInfo.width;
			//	dm.dmPelsHeight	= lCurDesktopInfo.height;
			//	ReleaseDC(NULL, hdc);
			//}


			// 正式修改
			if(bChangeDisplay && ChangeDisplaySettings(&dm,0) != DISP_CHANGE_SUCCESSFUL)
			{
				if(dm.dmDisplayFrequency == 60)
				{
					dm.dmDisplayFrequency = 65;
					if(ChangeDisplaySettings(&dm,0) != DISP_CHANGE_SUCCESSFUL)
					{
						dm.dmDisplayFrequency = 70;
						if(ChangeDisplaySettings(&dm,0) != DISP_CHANGE_SUCCESSFUL)
							return FALSE;
					}
				}
			}
		}
	}
	else
	{
		HDC hdc = GetDC(NULL);
		if(hdc)
		{
			DesktopInfo now;

			now.bits	= GetDeviceCaps(hdc,BITSPIXEL);
			now.width	= GetDeviceCaps(hdc,HORZRES);
			now.height	= GetDeviceCaps(hdc,VERTRES);
			now.rate	= GetDeviceCaps(hdc,VREFRESH);

			ReleaseDC(NULL, hdc);

			// 恢复为进游戏前的桌面分辨率
			if(memcmp(&now,&m_DisInfo_Enter,sizeof(DesktopInfo)) != 0)
			{
				dm.dmSize				= sizeof(DEVMODE);
				dm.dmFields				= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
				dm.dmBitsPerPel			= m_DisInfo_Enter.bits;
				dm.dmPelsWidth			= m_DisInfo_Enter.width;
				dm.dmPelsHeight			= m_DisInfo_Enter.height;
				dm.dmDisplayFrequency	= m_DisInfo_Enter.rate;
				ChangeDisplaySettings(&dm,0);
			}
		}
	}


	return TRUE;
}

// 取得显示的信息
BOOL CGraphicD3D::GetDisplayInfo(void)
{
	hr = m_p3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&m_d3ddm);
	if(hr != D3D_OK)
		return FALSE;

	HDC hdc = GetDC(NULL);
	if(!hdc) 
		return FALSE;

	m_DisInfo_Enter.bits		= GetDeviceCaps(hdc,BITSPIXEL);
	m_DisInfo_Enter.width		= GetDeviceCaps(hdc,HORZRES);
	m_DisInfo_Enter.height	= GetDeviceCaps(hdc,VERTRES);
	m_DisInfo_Enter.rate		= GetDeviceCaps(hdc,VREFRESH);

	ReleaseDC(NULL, hdc);
	return TRUE;
}

BOOL CGraphicD3D::GetCurScreenInfo(DesktopInfo& desinfo)
{
	HDC hdc = GetDC(NULL);
	if(hdc)
	{
		desinfo.bits		= GetDeviceCaps(hdc,BITSPIXEL);
		desinfo.width		= GetDeviceCaps(hdc,HORZRES);
		desinfo.height		= GetDeviceCaps(hdc,VERTRES);
		desinfo.rate		= GetDeviceCaps(hdc,VREFRESH);

		ReleaseDC(NULL, hdc);
		return TRUE;
	}
	return FALSE;
}

void CGraphicD3D::GetAdapterInfo(char* str)
{
	if(!m_p3d) 
		strcpy(str,"D3D设备未创建！");
	else
	{
		//sprintf(str,"%sVMem(F/T):%dKB/%dKB\r\n",m_AdapterInfo.Description,GetFreeVideoMemory()/1024,GetVideoMemory()/1024);
		sprintf(str,"%s\r\n",m_AdapterInfo.Description);

		HRESULT hr;
		char sss[256] = {0};

		hr = m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,D3DFMT_R5G6B5,D3DUSAGE_RENDERTARGET,D3DRTYPE_TEXTURE,D3DFMT_A8R8G8B8);
		if(hr != S_OK)
		{
			sprintf(sss,"D3DFMT_A8R8G8B8:D3DFMT_R5G6B5 RenderTarget not support!\r\n");
			strcat(str,sss);
		}
		hr = m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,D3DFMT_R5G6B5,0,D3DRTYPE_TEXTURE,D3DFMT_DXT1);
		if(hr != S_OK)
		{
			sprintf(sss,"D3DFMT_DXT1:D3DFMT_R5G6B5 not support!\r\n");
			strcat(str,sss);
		}
		hr = m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,D3DFMT_R5G6B5,0,D3DRTYPE_TEXTURE,D3DFMT_DXT3);
		if(hr != S_OK)
		{
			sprintf(sss,"D3DFMT_DXT3:D3DFMT_R5G6B5 not support!\r\n");
			strcat(str,sss);
		}
		hr = m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,D3DFMT_R5G6B5,0,D3DRTYPE_TEXTURE,D3DFMT_DXT5);
		if(hr != S_OK)
		{
			sprintf(sss,"D3DFMT_DXT5:D3DFMT_R5G6B5 not support!\r\n");
			strcat(str,sss);
		}
	}
}

// 是否需要显示本帧
BOOL CGraphicD3D::WillShowFrame(void)
{
	if(m_iPerFmeTime <= 1)						// 不进行帧控速
	{
		m_bFrameDraw	= TRUE;
		return TRUE;
	}

	int	iDisTime = GetTickCount() - m_iNextTime;

	if(iDisTime < 0)
	{
		if(!m_bFrameDraw && iDisTime > -10)
		{
			m_bFrameDraw = TRUE;
			m_iNextTime	+= m_iPerFmeTime;
			return TRUE;
		}
		else
		{
			m_bFrameDraw = TRUE;
			return FALSE;
		}
	}
	else
	{
		m_iNextTime	+= m_iPerFmeTime;
		if(iDisTime < m_iPerFmeTime)
			m_bFrameDraw = TRUE;
		else
		{
			m_bFrameDraw = FALSE;
			if(iDisTime > 2000)
			{
				//m_bFrameDraw = TRUE;,防止卡住,不要绘制
				//////////////////////////////////////////
				if (g_pCallBack && g_pCallBack->IsInReplay())//////////////
				{
					m_bFrameDraw = TRUE;
				}
				m_iNextTime = (int)GetTickCount();
			}
		}
		return TRUE;
	}
}

// 是否不显示
BOOL CGraphicD3D::IsNoDraw(void)
{
	return !bCanDraw;
}

// 
BOOL CGraphicD3D::IsDisplay(void)
{
	return m_bDisplay;
}

// 设置帧速
void CGraphicD3D::SetFPS(DWORD dwFPS)
{
	if (dwFPS == 0)
	{
		dwFPS = 1;
	}
	m_iPerFmeTime	= 1000 / dwFPS;
	if (m_iPerFmeTime <= 0)
	{
		m_iPerFmeTime = 1;
	}
}

DWORD CGraphicD3D::GetLastSetFPS(void)
{
	if (m_iPerFmeTime == 0)
	{
		return 1;
	}
	return 	(1000 / m_iPerFmeTime);
}

void CGraphicD3D::SetDisplay(BOOL b)
{
	m_bDisplay = b;
}

int CGraphicD3D::GetPerFmeTime()
{ 
	return m_iPerFmeTime; 
}

void CGraphicD3D::SetPerFmeTime(int val) 
{ 
	m_iPerFmeTime = val;
	if (m_iPerFmeTime <= 0)
	{
		m_iPerFmeTime = 0;
	}
	else if (m_iPerFmeTime > 1000)
	{
		m_iPerFmeTime = 1000;
	}
}

void CGraphicD3D::SetFrameDraw(BOOL b)
{
	m_bFrameDraw = b;
}

BOOL CGraphicD3D::IsSupportTide() //是否支持海浪
{
	if (!m_p3dDev)
		return FALSE;

	if (m_caps.MaxTextureBlendStages < 2 ) return FALSE;
	if (0 == (m_caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR)) return FALSE;
	if (0 == (m_caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)) return FALSE;
	if (0 == (m_caps.SrcBlendCaps&D3DPBLENDCAPS_SRCALPHA)) return FALSE;
	if (0 == (m_caps.DestBlendCaps&D3DPBLENDCAPS_ONE)) return FALSE;

	return TRUE;
}

BOOL CGraphicD3D::IsSupportCloud() //是否支持云彩
{
	if (!m_p3dDev)
		return FALSE;

	if (0 == (m_caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR)) return FALSE;
	if (0 == (m_caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)) return FALSE;
	if (0 == (m_caps.SrcBlendCaps&D3DPBLENDCAPS_SRCALPHA)) return FALSE;
	if (0 == (m_caps.SrcBlendCaps&D3DPBLENDCAPS_SRCCOLOR)) return FALSE;
	if (0 == (m_caps.SrcBlendCaps&D3DPBLENDCAPS_ONE)) return FALSE;
	if (0 == (m_caps.SrcBlendCaps&D3DPBLENDCAPS_INVSRCCOLOR)) return FALSE;

	if (0 == (m_caps.DestBlendCaps&D3DPBLENDCAPS_SRCALPHA)) return FALSE;
	if (0 == (m_caps.DestBlendCaps&D3DPBLENDCAPS_SRCCOLOR)) return FALSE;
	if (0 == (m_caps.DestBlendCaps&D3DPBLENDCAPS_ONE)) return FALSE;
	if (0 == (m_caps.DestBlendCaps&D3DPBLENDCAPS_INVSRCCOLOR)) return FALSE;

	D3DDISPLAYMODE mode;
	if (FAILED(m_p3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&mode)))
		return FALSE;

	if (FAILED(m_p3d->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,mode.Format,
		D3DUSAGE_RENDERTARGET,D3DRTYPE_TEXTURE,D3DFMT_A8R8G8B8)))
		return FALSE;

	return TRUE;
}

// 初始化DX的状态
BOOL CGraphicD3D::InitDXStates(void)
{
	m_p3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE			,TRUE					);
	m_p3dDev->SetRenderState(D3DRS_SRCBLEND					,D3DBLEND_SRCALPHA		);
	m_p3dDev->SetRenderState(D3DRS_DESTBLEND				,D3DBLEND_INVSRCALPHA	);
	m_p3dDev->SetRenderState(D3DRS_LIGHTING					,FALSE					);
	m_p3dDev->SetRenderState(D3DRS_CULLMODE					,D3DCULL_NONE			);
	m_p3dDev->SetRenderState(D3DRS_ALPHATESTENABLE   		,FALSE					);
	m_p3dDev->SetRenderState(D3DRS_ALPHAFUNC				,D3DCMP_GREATER			);
	m_p3dDev->SetRenderState(D3DRS_ALPHAREF					,0xAA					);

	m_p3dDev->SetTextureStageState( 0,	D3DTSS_TEXCOORDINDEX,0						);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_ALPHAOP		,D3DTOP_MODULATE		);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_ALPHAARG1	,D3DTA_TEXTURE			);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_ALPHAARG2	,D3DTA_CURRENT			);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_COLOROP		,D3DTOP_MODULATE		);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_COLORARG1	,D3DTA_TEXTURE			);
	m_p3dDev->SetTextureStageState( 0,	D3DTSS_COLORARG2	,D3DTA_CURRENT			);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_TEXCOORDINDEX,0						);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_ALPHAOP		,D3DTOP_SELECTARG1		);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_ALPHAARG1	,D3DTA_TEXTURE			);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_ALPHAARG2	,D3DTA_CURRENT			);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_COLOROP		,D3DTOP_SELECTARG2		);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_COLORARG1	,D3DTA_TEXTURE			);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_COLORARG2	,D3DTA_CURRENT			);
	m_p3dDev->SetTextureStageState( 1,	D3DTSS_COLOROP		,D3DTOP_DISABLE			);

	SetRenderMode(RM_POINT);
	SetRenderMode(RM_CLAMP);

	// 在16bit模式下，打开色彩抖动
	if(m_d3ddm.Format != D3DFMT_X8R8G8B8)
	{
		m_p3dDev->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	}

	hr = m_p3dDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&m_pBackSurf);
	if(hr != D3D_OK)
		return FALSE;

	m_pSprite->OnResetDevice();

	SetRenderMode();

	return TRUE;
}

// 创建纹理缓冲池
BOOL CGraphicD3D::InitTexPool(BOOL bForce)
{
	// 创建光照贴图
	hr = m_p3dDev->CreateTexture(256,256,1,D3DUSAGE_RENDERTARGET,m_d3dpp.BackBufferFormat,D3DPOOL_DEFAULT,&m_pLight,NULL);
	if(hr == D3D_OK && m_pLight)
	{
		m_pLight->GetSurfaceLevel(0,&m_pLightSurf);
	}
	else
	{
		m_pLight		= NULL;
		m_pLightSurf	= NULL;
	}
	// 创建临时rendertarget贴图
	//hr = m_p3dDev->CreateTexture(512,512,1,D3DUSAGE_RENDERTARGET,m_d3dpp.BackBufferFormat,D3DPOOL_DEFAULT,&m_pTempRenderTarget,NULL);
	//hr = m_p3dDev->CreateTexture(MAX_WINDOW_WIDTH,MAX_WINDOW_HEIGHT,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pTempRenderTarget,NULL);
	hr = m_p3dDev->CreateTexture(512,512,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pTempRenderTarget,NULL);
	if(hr == D3D_OK && m_pTempRenderTarget)
	{
		m_pTempRenderTarget->GetSurfaceLevel(0,&m_pTempRenderTargetSurf);
	}
	else
	{
		m_pTempRenderTarget		= NULL;
		m_pTempRenderTargetSurf	= NULL;
	}

	// 创建顶点缓冲
	m_iVertexBuffer = 0;
	for(int i = 0;i < MAX_VERTEXBUFFER;i++)
	{
		hr = m_p3dDev->CreateVertexBuffer(sizeof(CUSTOMVERTEX)*192,0,D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,&m_pVertexBuffer[i],NULL);
		if(hr != D3D_OK)
			return FALSE;
	}
	m_p3dDev->SetVertexShader(NULL);

	// 创建天气纹理
	if(!m_pCloudTex[0] && IsSupportCloud()) //因为需要用到是否支持的判断
	{
		hr = m_p3dDev->CreateTexture(32 ,32 ,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pCloudTex[0],NULL);
		hr = m_p3dDev->CreateTexture(64 ,64 ,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pCloudTex[1],NULL);
		hr = m_p3dDev->CreateTexture(128,128,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pCloudTex[2],NULL);
		hr = m_p3dDev->CreateTexture(256,256,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pCloudTex[3],NULL);
		hr = m_p3dDev->CreateTexture(256,256,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pCloudTex[4],NULL);
	}

	if(!bForce && TEX_POOL_TYPE == D3DPOOL_MANAGED)
		return TRUE;

	// 创建图形显示系统纹理
	if(TEX_POOL_TYPE == D3DPOOL_DEFAULT && bForce)
	{
		for(int i = 0;i < OLD_POOL_TYPES;i++)
		{
			hr = m_p3dDev->CreateTexture(s_OldPool[i].iWidth,s_OldPool[i].iHeight,1,0,
				s_OldPool[i].D3DFormat,D3DPOOL_SYSTEMMEM,&(s_OldPool[i].pMemTex),NULL);

			if(hr != D3D_OK)
			{
				MessageBox(NULL,"CreateTexture s_OldPool D3DPOOL_SYSTEMMEM",NULL,0);
				return FALSE;
			}
		}

		for(int i = 0;i < TEX_POOL_TYPES;i++)
		{
			hr = m_p3dDev->CreateTexture(s_TexPool[i].iWidth,s_TexPool[i].iHeight,1,0,
				s_TexPool[i].D3DFormat,D3DPOOL_SYSTEMMEM,&(s_TexPool[i].pMemTex),NULL);

			if(hr != D3D_OK)
			{
				MessageBox(NULL,"CreateTexture s_TexPool D3DPOOL_SYSTEMMEM",NULL,0);
				return FALSE;
			}
		}

	}

	// 创建字体显示系统纹理
	if(TEX_POOL_TYPE == D3DPOOL_DEFAULT && bForce)
	{
		for(int i = 0;i < FTST_NUMS;i++)
		{
			hr = m_p3dDev->CreateTexture(s_FontPool[i].iWidth,s_FontPool[i].iHeight,1,0,
				s_FontPool[i].D3DFormat,D3DPOOL_SYSTEMMEM,&(s_FontPool[i].pMemTex),NULL);

			if(hr != D3D_OK)
			{
				MessageBox(NULL,"CreateTexture s_FontPool D3DPOOL_SYSTEMMEM",NULL,0);
				return FALSE;
			}
		}
	}

	return TRUE;
}

// 释放纹理池
void CGraphicD3D::ReleaseTexPool(BOOL bForce)
{
	SAFE_RELEASE(m_pLight);
	SAFE_RELEASE(m_pLightSurf);
	SAFE_RELEASE(m_pTempRenderTarget);
	SAFE_RELEASE(m_pTempRenderTargetSurf);
	SAFE_RELEASE(m_pBackSurf);

	//释放顶点缓冲
	for(int i = 0;i < MAX_VERTEXBUFFER;i++)
	{
		SAFE_RELEASE(m_pVertexBuffer[i]);
	}

	//释放天气效果纹理
	for(int i = 0;i < MAX_CLOUDSTEX;i++)
	{
		SAFE_RELEASE(m_pCloudTex[i]);
	}

	if(!bForce && TEX_POOL_TYPE == D3DPOOL_MANAGED)
		return;

	for(int i = 0;i < OLD_POOL_TYPES;i++)
	{
		if(bForce)
			SAFE_RELEASE(s_OldPool[i].pMemTex);
		s_OldPool[i].Pool.ReleaseAllSrc();
		s_OldPool[i].bCanCreate = TRUE;
	}

	for(int i = 0;i < TEX_POOL_TYPES;i++)
	{
		if(bForce)
			SAFE_RELEASE(s_TexPool[i].pMemTex);
		s_TexPool[i].Pool.ReleaseAllSrc();
		s_TexPool[i].bCanCreate = TRUE;
	}

	for(int i = 0;i < FTST_NUMS;i++)
	{
		if(bForce)
			SAFE_RELEASE(s_FontPool[i].pMemTex);
		s_FontPool[i].Pool.ReleaseAllSrc();
		s_FontPool[i].bCanCreate = TRUE;
	}
	m_dwAlloacMem = 0;
}

// 查看设备是否丢失
BOOL CGraphicD3D::ValidateDX(void)
{
	if(!m_p3dDev)
		goto D3D_ERROR;

	hr = m_p3dDev->TestCooperativeLevel();
	if(hr == D3DERR_DEVICELOST)
	{
		if(g_pCallBack)
			g_pCallBack->OnLostDevice();

		goto D3D_ERROR;
	}
	else if(hr == D3DERR_DEVICENOTRESET)
	{
		ReleaseTexPool(FALSE);

		hr = m_pSprite->OnLostDevice();
		if(hr != D3D_OK)
			goto D3D_ERROR;

		hr = m_p3dDev->Reset(&m_d3dpp);
		if(hr != D3D_OK)
		{
			if(ReCreateDX())
			{
				m_bD3dOK = TRUE;
		
				if(g_pCallBack)
					g_pCallBack->OnDeviceNotReset();

				if (g_pETexMgr)
				{
					g_pETexMgr->ClearAllTex();
				}

				return TRUE;
			}
			goto D3D_ERROR;
		}

		if (g_pETexMgr)
		{
			g_pETexMgr->ClearAllTex();
		}

		if(g_pCallBack)
			g_pCallBack->OnDeviceReset((void*)(&m_d3dpp));

		if(InitTexPool(FALSE) == FALSE)
			goto D3D_ERROR;

		if(InitDXStates() == FALSE)
			goto D3D_ERROR;

	}
	m_bD3dOK = TRUE;
	return TRUE;

D3D_ERROR:
	m_bD3dOK = FALSE;
	return FALSE;
}

BOOL CGraphicD3D::ReCreateDX(void)
{
	KillD3D(false);
	return CreateD3D(m_hWnd,m_iWidth,m_iHeight,m_eDisMode,m_bGameMode,false);
}

BOOL CGraphicD3D::ResizeWindow(int iW,int iH)
{
	if(iW == m_iWidth && iH == m_iHeight)
		return true;

	KillD3D(false);
	return CreateD3D(m_hWnd,iW,iH,m_eDisMode,m_bGameMode);
}

// 开始渲染
void CGraphicD3D::BeginDraw(void)
{
	if(!bCanDraw)
		return;

	//加速的时候不要每帧都绘制,加速10倍就每10帧绘制一次
	if (m_iPerFmeTime < 20 && g_pCallBack && g_pCallBack->IsInReplay() && (m_dwFrameCount % (20 / m_iPerFmeTime) != 0))
	{
		m_bNeedDraw = false;
	}
	else
	{
		m_bNeedDraw = true;

		PoolData::ResetCounter(); //重置计数器

		m_p3dDev->BeginScene();
		m_pSprite->Begin();
	}
}

// 结束渲染
void CGraphicD3D::EndDraw(HWND hWnd)
{
	if(!bCanDraw) //没有绘制的时候帧数也要继续
	{
		/////////////播放的时候以帧作为驱动,不绘制就不要加帧,不然在处理过程有可能要取mouseonid,但没有绘制就设置不了MouseOnID,导致整个逻辑不对//////////
		if (g_pCallBack && g_pCallBack->IsInReplay())
		{
			return;
		}

		m_dwFrameCount++;
		return;
	}

	if(m_bNeedDraw)
	{
		if (m_bRenderIGW && g_pCallBack)
			g_pCallBack->IgwDraw();

		PERF_SEG_BEGIN(EndScene,FALSE);
		m_pSprite->End();
		m_p3dDev->EndScene();
		PERF_SEG_END(EndScene);

		RECT rect = {0,0,m_iWidth,m_iHeight};

		PERF_SEG_BEGIN(Present,FALSE);
		m_p3dDev->Present(&rect,&rect,hWnd,NULL);
		PERF_SEG_END(Present);
	}


	PERF_SEG_BEGIN(FrameCount,FALSE);
	if(!hWnd || hWnd == m_hWnd)
	{
		m_dwFrameCount++;
		m_dwTotalRealFps += m_dwRealFPS;

		// 计算帧速
		static DWORD dwLastTime = 0;
		static DWORD dwFrames	= 0;
		if(dwLastTime == 0)
		{
			dwLastTime  = GetTickCount();
			dwFrames	= 0;
		}
		else
		{
			int iTime = (int)(GetTickCount() - dwLastTime);
			if(iTime >= 1000)
			{
				m_dwRealFPS = dwFrames;
				dwLastTime  = GetTickCount();
				dwFrames	= 0;
			}
			dwFrames++;
		}
	}
	PERF_SEG_END(FrameCount);
}

// 清除底图
void CGraphicD3D::ClearColor(DWORD dwColor)
{
	if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
		return;

	m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,dwColor,0,0);
}

// 设置渲染模式
void CGraphicD3D::SetRenderMode(RenderMode eMode)
{
	TRY_BEGIN
	if(!bCanDraw || !m_bNeedDraw)
			return;

	switch(eMode)
	{
	case RM_ALPHA:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_SRCALPHA		);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA	);
		break;
	case RM_ADD1:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ONE			);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR	);
		break;
	case RM_ADD2:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ONE			);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE			);
		break;
	case RM_ADD3:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ONE			);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA	);
		break;
	case RM_ALPHAADD:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_SRCALPHA		);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE			);
		break;
	case RM_MODULATE:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ZERO			);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR		);
		break;
	case RM_RGB:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ONE			);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO			);
		break;
	case RM_REALPHA:
		m_p3dDev->SetRenderState(D3DRS_SRCBLEND	, D3DBLEND_ZERO);
		m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA		);
		break;
	case RM_POINT:
		m_p3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT	);
		m_p3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT	);
		break;
	case RM_LINEAR:
		m_p3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
		m_p3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		break;
	case RM_WRAP:
		m_p3dDev->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
		m_p3dDev->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
		break;
	case RM_CLAMP:
		m_p3dDev->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
		m_p3dDev->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
	default:
		break;
	}
	TRY_END
}

// 纹理的简单绘制
void CGraphicD3D::DrawTextureNL(int x,int y,LPTexture pTex,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !pTex || !(pTex->GetCurFrame()))
			return;

	pTex->UpdateFrame(m_dwFrameCount);

	TEX_FRAME & frame	= *(pTex->GetCurFrame());
	LPD3DTEX	pD3DTex	= NULL;
	int			x0		= x - frame.wCenterX + frame.wOffX;
	int			y0		= y - frame.wCenterY + frame.wOffY;
	for(int j = 0;j < frame.wYBlocks;j++)
	{
		x0 = x - frame.wCenterX + frame.wOffX;
		for(int i = 0;i < frame.wXBlocks;i++)
		{
			TEX_BLOCK &block = frame.vBlocks[j * frame.wXBlocks + i];
			if(pD3DTex = block.GetVideoTex(m_p3dDev))
				m_pSprite->Draw(pD3DTex,NULL,NULL,NULL,0,&D3DXVECTOR2((float)x0,(float)y0),dwColor);
			x0 += block.wWidth;
		}
		y0 += frame.vBlocks[j * frame.wXBlocks].wHeight;
	}
	TRY_END
}
//画弧形，要求纹理只有一块，分成多块的纹理将只处理第一块
void CGraphicD3D::DrawArcTexture(int px,int py,LPTexture pTex,float fBeginArc,float fEndArc,DWORD color)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !pTex || !(pTex->GetCurFrame()))
			return;

	pTex->UpdateFrame(m_dwFrameCount);

	int w,h,h0,w0;
	CUSTOMVERTEX *v;
	HRESULT hr;
	TEX_FRAME * tn = pTex->GetCurFrame();
	LPD3DTEX	pD3DTex	= NULL;

	//角度超出范围
	if(fEndArc <= 0.0f || fEndArc > 360.0f || fBeginArc < 0.0f || fBeginArc > 360.0f)
		return;

	if(fabs(fEndArc - fBeginArc) < 1.0f) //小于1度不画了
		return;

	if(tn->vBlocks)
		pD3DTex = tn->vBlocks[0].GetVideoTex(m_p3dDev);
	if(!pD3DTex)
		return;

	px += pTex->GetOffX();
	py += pTex->GetOffY();

	w = tn->vBlocks[0].wWidth;
	h = tn->vBlocks[0].wHeight;
	w0 = tn->wWidth;
	h0 = tn->wHeight;

	// 更换顶点缓冲区，以实现并行操作
	m_iVertexBuffer++;
	if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	if( hr != D3D_OK ) 
		return;

	// 锁定
	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*6,(void**)&v, 0);
	if( hr != D3D_OK ) 
		return;

	int iVertexNum = 1;

	//中心点
	v[0].sx = (float)px - 0.25f;
	v[0].sy = (float)py - 0.25f;
	v[0].tu = 0.5f * w0 / w;
	v[0].tv = 0.5f * h0 / h;

	float fAng = fBeginArc;

	int iMul = 1;
	if(fEndArc >= fBeginArc)
	{
		for(fAng = fBeginArc;fAng <= fEndArc;fAng += 90.0f)
		{
			float px2 = g_fCos[(450 - (int)fAng) % 360] * w0/2;
			float py2 = g_fSin[(450 - (int)fAng) % 360] * h0/2;
			v[iVertexNum].sx = (float)px + px2 - 0.25f;
			v[iVertexNum].sy = (float)py - py2 - 0.25f;
			v[iVertexNum].tu = v[0].tu + px2/w;
			v[iVertexNum].tv = v[0].tv - py2/h;
			iVertexNum++;
		}
		iMul = 1;
	}
	else
	{
		for(fAng = fBeginArc;fAng >= fEndArc;fAng -= 90.0f)
		{
			float px2 = g_fCos[(450 - (int)fAng) % 360] * w0/2;
			float py2 = g_fSin[(450 - (int)fAng) % 360] * h0/2;
			v[iVertexNum].sx = (float)px + px2 - 0.25f;
			v[iVertexNum].sy = (float)py - py2 - 0.25f;
			v[iVertexNum].tu = v[0].tu + px2/w;
			v[iVertexNum].tv = v[0].tv - py2/h;
			iVertexNum++;
		}
		iMul = -1;
	}

	if(fabs(fEndArc - (fAng - iMul*90.0f)) > 1.0f) //超过一度,要绘制最后一个点
	{
		float px2 = g_fCos[(450 - (int)fEndArc) % 360] * w0/2;
		float py2 = g_fSin[(450 - (int)fEndArc) % 360] * h0/2;
		v[iVertexNum].sx = (float)px + px2 - 0.25f;
		v[iVertexNum].sy = (float)py - py2 - 0.25f;
		v[iVertexNum].tu = v[0].tu + px2/w;
		v[iVertexNum].tv = v[0].tv - py2/h;
		iVertexNum++;
	}

	for(int i = 0;i < iVertexNum;i++)
	{
		v[i].color = color;
		v[i].rhw = 1.0f;
		v[i].sz =  0.5f;
	}

	// 显示纹理
	m_pVertexBuffer[m_iVertexBuffer]->Unlock();

	hr = m_p3dDev->SetTexture(0,pD3DTex);
	if( hr != D3D_OK ) return;

	DWORD dwFVF = 0;
	hr = m_p3dDev->GetFVF(&dwFVF);
	if(hr != D3D_OK) return;

	m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iVertexNum-2);
	m_p3dDev->SetFVF(dwFVF);

	if( hr != D3D_OK ) return;
	TRY_END
}

void CGraphicD3D::DrawTetxure3G(int x,int y,LPTexture pTex,int iWPages,int iBX,int iBY,int iGW,DWORD dwColor)
{
	if(!bCanDraw || !m_bNeedDraw || !pTex)
		return;

	pTex->UpdateFrame(m_dwFrameCount);

	x -= pTex->GetOffX();
	y -= pTex->GetOffY();

	RECT vRC[3];
	vRC[0].left		= pTex->GetOffX();
	vRC[0].top		= pTex->GetOffY();
	vRC[0].right	= iBX;
	vRC[0].bottom	= pTex->GetOffY() + pTex->GetHeight();

	vRC[1].left		= iBX;
	vRC[1].top		= pTex->GetOffY();
	vRC[1].right	= iBX + iGW;
	vRC[1].bottom	= pTex->GetOffY() + pTex->GetHeight();

	vRC[2].left		= iBX + iGW;
	vRC[2].top		= pTex->GetOffY();
	vRC[2].right	= pTex->GetOffX() + pTex->GetWidth();
	vRC[2].bottom	= pTex->GetOffY() + pTex->GetHeight();

	int idx = x + vRC[0].left - vRC[0].right;
	DrawTextureFX(idx,y,pTex,dwColor,&vRC[0]);
	idx += iGW;
	for(int i = 0;i < iWPages;i++)
	{
		DrawTextureFX(idx - iGW,y,pTex, dwColor,&vRC[1]);
		idx += iGW;
	}
	DrawTextureFX(idx - iGW * 2,y,pTex, dwColor,&vRC[2]);
}

// 绘制9宫格
void CGraphicD3D::DrawTetxure9G(int x,int y,LPTexture pTex,int iWPages,int iHPages,int iBX,int iBY,int iGW,int iGH,DWORD dwColor)
{
	if(!bCanDraw || !m_bNeedDraw || !pTex)
		return;

	pTex->UpdateFrame(m_dwFrameCount);

	// 取得9个格子的矩形区域
	RECT vRC[9];
	int x0,y0,x1,y1;
	for(int j = 0;j < 3;j++)
	{
		if(j == 0)
		{
			y0 = pTex->GetOffY();
			y1 = iBY + iGH;
		}
		else if(j == 1)
		{
			y0 = iBY + iGH;
			y1 = y0 + iGH;
		}
		else
		{
			y0 = iBY + iGH * 2;
			y1 = pTex->GetOffY() + pTex->GetHeight();
		}
		for(int i = 0;i < 3;i++)
		{
			if(i == 0)
			{
				x0 = pTex->GetOffX();
				x1 = iBX + iGW;
			}
			else if(i == 1)
			{
				x0 = iBX + iGW;
				x1 = x0 + iGW;
			}
			else
			{
				x0 = iBX + iGW * 2;
				x1 = pTex->GetOffX() + pTex->GetWidth();
			}
			vRC[j * 3 + i].left		= x0;
			vRC[j * 3 + i].top		= y0;
			vRC[j * 3 + i].right	= x1;
			vRC[j * 3 + i].bottom	= y1;
		}
	}

	int tw,th,ix,iy;
	iy = y + pTex->GetOffY() - iBY;
	for(int h = 0;h < iHPages;h++)
	{
		ix = x + pTex->GetOffX() - iBX;
		if(h == 0)
			th = 0;
		else if(h == iHPages - 1)
			th = 2;
		else
			th = 1;
		for(int w = 0;w < iWPages;w++)
		{
			if(w == 0)
				tw = 0;
			else if(w == iWPages - 1)
				tw = 2;
			else
				tw = 1;

			DrawTextureFX(ix - tw * iGW,iy - th * iGH,pTex,dwColor,&vRC[th * 3 + tw]);
			ix += iGW;
		}
		iy += iGH;
	}
}

//////////////////////////////////////////////////////////////////
// 海水大地表的绘制(只允许在绘制绘制大地表的时候调用)
// pColorTex[DXT1格式]
// pMiniMapRect[大小必须为128×128]
// pAlphaTex[DXT3格式，大小必须为128×128]
//////////////////////////////////////////////////////////////////
void CGraphicD3D::DrawTextureTL(int x,int y,LPTexture pColorTex,LPTexture pAlphaTex,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !pColorTex || !pAlphaTex || pColorTex->GetWidth0() != 128 || pAlphaTex->GetWidth0() != 128)
			return;
	if(pColorTex->GetWidth0() != pAlphaTex->GetWidth0() || pColorTex->GetHeight0() != pAlphaTex->GetHeight0())
		return;
	if(!(pColorTex->GetCurFrame()) || !(pAlphaTex->GetCurFrame()) || !(pColorTex->GetCurFrame()->vBlocks) || !(pAlphaTex->GetCurFrame()->vBlocks))
		return;

	pColorTex->UpdateFrame(m_dwFrameCount);
	pAlphaTex->UpdateFrame(m_dwFrameCount);

	CTexFrame* pAlphaCurFrame =	pAlphaTex->GetCurFrame();
	if(!pAlphaCurFrame || !(pColorTex->GetCurFrame()))
		return;

	TEX_BLOCK* pColorBlock	= pColorTex->GetCurFrame()->vBlocks;
	TEX_BLOCK* pAlphaBlock	= pAlphaCurFrame->vBlocks;

	int	iRx	= x - pAlphaCurFrame->wCenterX + pAlphaCurFrame->wOffX;
	int	iRy	= y - pAlphaCurFrame->wCenterY + pAlphaCurFrame->wOffY;
	if(m_bMultiTex)
	{
		LPD3DTEX pD3DColorTex = pColorBlock->GetVideoTex(m_p3dDev);
		LPD3DTEX pD3DAlphaTex = pAlphaBlock->GetVideoTex(m_p3dDev);

		if(pD3DColorTex && pD3DAlphaTex)
		{
			m_p3dDev->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			m_p3dDev->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_SELECTARG2);

			m_p3dDev->SetTexture(1,pD3DAlphaTex);
			m_pSprite->Draw(pD3DColorTex,NULL,NULL,NULL,0,&D3DXVECTOR2((float)iRx,(float)iRy),dwColor);

			m_p3dDev->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
			m_p3dDev->SetTexture(1,NULL);
		}
	}
	else if(m_pTileTex && pColorBlock->pData && pAlphaBlock->pData)
	{
		BYTE *pDXT3,*pDXT1;
		BYTE buf0[128 * 128] = {0};
		BYTE buf1[128 * 128] = {0};

		pDXT3 = buf0;
		if(pAlphaBlock->bCompr)
		{
			DWORD dwDestSize = sizeof(buf0);
			Uncompress(buf0,&dwDestSize,pAlphaBlock->pData,pAlphaBlock->dwSize);
		}
		else
			memcpy(buf0,pAlphaBlock->pData,pAlphaBlock->dwSize);

		pDXT1 = pColorBlock->pData;
		if(pColorBlock->bCompr)
		{
			DWORD dwDestSize = sizeof(buf1);
			Uncompress(buf1,&dwDestSize,pColorBlock->pData,pColorBlock->dwSize);
			pDXT1 = buf1;
		}

		Combine_DXT3_DXT1(pDXT3,pDXT1,128,128);

		D3DLOCKED_RECT lr = {0,NULL};
		m_pTileTex->LockRect(0,&lr,NULL,0);
		Convert(m_bDXTn ? TPT_DXT3 : TPT_A4R4G4B4,(BYTE *)lr.pBits,lr.Pitch,128,TPT_DXT3,pDXT3,128,128);
		m_pTileTex->UnlockRect(0);

		m_pSprite->Draw(m_pTileTex,NULL,NULL,NULL,0,&D3DXVECTOR2((float)iRx,(float)iRy),dwColor);
	}
	TRY_END
}

//////////////////////////////////////////////////////////////////
// 圆形小地图的绘制(只允许在绘制小地图的时候调用)
// pMiniMapTex[DXT1格式]
// pMiniMapRect[大小必须为128×128]
// pAlphaTex[必须为DXT3格式,大小必须为128×128]
//////////////////////////////////////////////////////////////////
void CGraphicD3D::DrawTextureMM(int x,int y,LPTexture pMiniMapTex,LPRECT pRect,LPTexture pAlphaTex,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !m_pMiniMapTex || !pMiniMapTex || !pRect || !pAlphaTex || pMiniMapTex->GetFrames() == 0 || pAlphaTex->GetFrames() == 0)
			return;

	if(pAlphaTex->GetWidth0() != 128 || pAlphaTex->GetHeight0() != 128)
		return;

	// 上次更新的记录
	static DWORD	dwMiniMapTexID	= 0;
	static RECT		rcRect			= {0,0,0,0};

	// 查看是否需要更新Tex
	if(dwMiniMapTexID != pMiniMapTex->GetID() || memcmp(pRect,&rcRect,sizeof(RECT)) != 0)
	{
		WORD Alpha[128 * 128]		= {0};
		WORD MMRGB[128 * 128]		= {0};
		BYTE buf1[128 * 128 * 2]	= {0};
		RECT		rc				= {0,0,0,0};
		TEX_FRAME*	frame	= pMiniMapTex->GetFrame(0);
		if (!frame)
		{
			return;
		}

		// 取得alpha数据
		if(pAlphaTex->GetFrame(0)->vBlocks && pAlphaTex->GetFrame(0)->vBlocks[0].pData)
		{
			TEX_BLOCK &block = pAlphaTex->GetFrame(0)->vBlocks[0];
			if(block.bCompr)
			{
				DWORD dwSize = sizeof(Alpha);
				memset(Alpha,0,dwSize);
				Uncompress((BYTE *)Alpha,&dwSize,block.pData,block.dwSize);
			}
			else
				memcpy(Alpha,block.pData,block.dwSize);
		}

		// 取得MiniMapTex纹理需要用到块数据
		int x0 = 0,y0 = 0;
		for(int j = 0;j < frame->wYBlocks;j++)
		{
			x0 = 0;
			for(int i = 0;i < frame->wXBlocks;i++)
			{
				if(x0 > pRect->right && y0 > pRect->bottom)
					goto UPDATE_MM;

				TEX_BLOCK &block = frame->vBlocks[j * frame->wXBlocks + i];
				if(pRect && (pRect->right <= x0 || pRect->bottom <= y0 || pRect->left >= (x0 + block.wWidth) || pRect->top >= (y0 + block.wHeight)))
				{
					x0 += block.wWidth;
					continue;
				}
				else
				{
					// 得到像素数据
					LPBYTE pBuf = block.pData;
					if(block.bCompr && block.pData)
					{
						DWORD dwSize = sizeof(buf1);
						memset(buf1,0,dwSize);
						Uncompress(buf1,&dwSize,block.pData,block.dwSize);
						pBuf = buf1;
					}
					memset(MMRGB,0,sizeof(MMRGB));
					Convert(TPT_A4R4G4B4,(BYTE *)MMRGB,block.wWidth * 2,block.wHeight,block.ePixelType,pBuf,block.wWidth,block.wHeight);

					// 计算绘制区域
					if(pRect->left > x0)
						rc.left = pRect->left - x0;
					else
						rc.left = 0;
					if(pRect->top > y0)
						rc.top = pRect->top - y0;
					else
						rc.top = 0;
					if(pRect->right < x0 + block.wWidth)
						rc.right = pRect->right - x0;
					else
						rc.right = block.wWidth;
					if(pRect->bottom < y0 + block.wHeight)
						rc.bottom = pRect->bottom - y0;
					else
						rc.bottom = block.wHeight;

					// 混合数据
					for(long n = rc.top;n < rc.bottom;n++)
					{
						for(long m = rc.left;m < rc.right;m++)
						{
							Alpha[128 * (y0 + n - pRect->top) + (x0 + m - pRect->left)] &= (0xF000 | MMRGB[n * block.wWidth + m]);
						}
					}
				}
				x0 += block.wWidth;
			}
			y0 += frame->vBlocks[j * frame->wXBlocks].wHeight;
		}

UPDATE_MM:

		D3DLOCKED_RECT lr = {0,NULL};
		m_pMiniMapTex->LockRect(0,&lr,NULL,0);
		Convert(TPT_A4R4G4B4,(BYTE *)lr.pBits,lr.Pitch,128,TPT_A4R4G4B4,(BYTE *)Alpha,128,128);
		m_pMiniMapTex->UnlockRect(0);

		dwMiniMapTexID	= pMiniMapTex->GetID();
		rcRect			= *pRect;
	}

	m_pSprite->Draw(m_pMiniMapTex,NULL,NULL,NULL,0,&D3DXVECTOR2((float)x,(float)y),dwColor);
	TRY_END
}

/////////////////////////////////////////////////////////////////
// 绘制纹理的一部分
/////////////////////////////////////////////////////////////////
void CGraphicD3D::DrawPartTexture(int x,int y,LPTexture pTex,int iLeft,int iTop,int iRight,int iBottom,DWORD dwColor,LPPOS pScale)
{
	TRY_BEGIN
	PERF_SEG_ALL(DrawPartTexture,TRUE);
	if(!bCanDraw || !m_bNeedDraw || !pTex || !(pTex->GetCurFrame()))
		return;

	RECT rc;
	rc.left = iLeft;
	rc.top = iTop;
	rc.right = iRight;
	rc.bottom = iBottom;

	DrawTextureFX(x,y,pTex,dwColor,&rc,pScale);
	TRY_END
}

//////////////////////////////////////////////////////////////////
// 纹理的复杂绘制
// pRect				:绘制区域
// pScale				:缩放倍数
// fRotX，fRotY，fRotZ	:X,Y,Z轴旋转的度数
//////////////////////////////////////////////////////////////////
void CGraphicD3D::DrawTextureFX(int x,int y,LPTexture pTex,DWORD dwColor,LPRECT pRect,LPPOS pScale,float fRotX,float fRotY,float fRotZ)
{
	TRY_BEGIN
		PERF_SEG_ALL(DrawTextureFX,TRUE);
	if(!bCanDraw || !m_bNeedDraw || !pTex)
		return;

	pTex->UpdateFrame(m_dwFrameCount);

	if(!(pTex->GetCurFrame()))
		return;

	D3DXMATRIX mat,mat1;
	LPD3DTEX	pD3DTex	= NULL;
	TEX_FRAME & frame	= *(pTex->GetCurFrame());
	float	fCenterX	= (float)(frame.wOffX - frame.wCenterX);
	float	fCenterY	= (float)(frame.wOffY - frame.wCenterY);

	D3DXMatrixIdentity(&mat);
	if(pScale)			// 缩放图片
		D3DXMatrixScaling(&mat,pScale->fx,pScale->fy,1.0f);

	if(fRotX != 0.0f)	// 翻转图片(绕X轴)
	{
		D3DXMatrixRotationX(&mat1,fRotX * D3D_PI / 180.0f);
		D3DXMatrixMultiply(&mat,&mat,&mat1);
	}
	if(fRotY != 0.0f)	// 翻转图片(绕Y轴)
	{
		D3DXMatrixRotationY(&mat1,fRotY * D3D_PI / 180.0f);
		D3DXMatrixMultiply(&mat,&mat,&mat1);
	}
	if(fRotZ != 0.0f)	// 旋转图片(绕Z轴)
	{
		D3DXMatrixRotationZ(&mat1,fRotZ * D3D_PI / 180.0f);
		D3DXMatrixMultiply(&mat,&mat,&mat1);
	}

	// 移动位置
	D3DXMatrixTranslation(&mat1,(float)x,(float)y,0.0f);
	D3DXMatrixMultiply(&mat,&mat,&mat1);

	// 绘制每一块
	if(pRect)
	{
		if(pRect->top < 0)
			pRect->top = 0;
		if(pRect->left < 0)
			pRect->left = 0;
		if(pRect->bottom > pTex->GetHeight() || pRect->bottom < 0)
			pRect->bottom = pTex->GetHeight();
		if(pRect->right > pTex->GetWidth() || pRect->right < 0)
			pRect->right = pTex->GetWidth();
	}

	RECT *prc,rc;
	int x0 = 0;
	int y0 = 0;
	for(int j = 0;j < frame.wYBlocks;j++)
	{
		x0 = 0;
		for(int i = 0;i < frame.wXBlocks;i++)
		{
			TEX_BLOCK &block = frame.vBlocks[j * frame.wXBlocks + i];
			if(pRect && (pRect->right <= x0 || pRect->bottom <= y0 || pRect->left >= (x0 + block.wWidth) || pRect->top >= (y0 + block.wHeight)))
			{
				x0 += block.wWidth;
				continue;
			}
			else if(pD3DTex = block.GetVideoTex(m_p3dDev))
			{
				if(pRect)
				{
					// 计算绘制区域
					if(pRect->left > x0)
						rc.left = pRect->left - x0;
					else
						rc.left = 0;
					if(pRect->top > y0)
						rc.top = pRect->top - y0;
					else
						rc.top = 0;
					if(pRect->right < x0 + block.wWidth)
						rc.right = pRect->right - x0;
					else
						rc.right = block.wWidth;
					if(pRect->bottom < y0 + block.wHeight)
						rc.bottom = pRect->bottom - y0;
					else
						rc.bottom = block.wHeight;

					prc = &rc;
					D3DXMatrixTranslation(&mat1,(float)(x0 + fCenterX + rc.left),(float)(y0 + fCenterY + rc.top),0.0f);
				}
				else
				{
					rc.left = rc.top = 0;
					rc.right = block.wWidth;
					rc.bottom = block.wHeight;
					prc = &rc;
					D3DXMatrixTranslation(&mat1,(float)(x0 + fCenterX),(float)(y0 + fCenterY),0.0f);
				}

				// 计算每张纹理所在的位置
				D3DXMatrixMultiply(&mat1,&mat1,&mat);

				// 绘制纹理
				m_pSprite->DrawTransform(pD3DTex,prc,&mat1,dwColor);
			}
			x0 += block.wWidth;
		}
		y0 += frame.vBlocks[j * frame.wXBlocks].wHeight;
	}
	TRY_END
}

// 不使用D3D的灵活顶点格式是因为每次锁定顶点缓冲会有比较大的开销，反而缩放绘制纹理少些
void CGraphicD3D::DrawRect(int x,int y,int w,int h,DWORD dwColor,int iFrameW)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
			return;

	// 分别是上，下，左，右矩形
	m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)w,(float)iFrameW),NULL,0.0f,&D3DXVECTOR2((float)x,(float)y),dwColor);
	m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)w,(float)iFrameW),NULL,0.0f,&D3DXVECTOR2((float)x,(float)(y + h - iFrameW)),dwColor);
	m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)iFrameW,(float)(h - 2 * iFrameW)),NULL,0.0f,&D3DXVECTOR2((float)x,(float)(y + iFrameW)),dwColor);
	m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)iFrameW,(float)(h - 2 * iFrameW)),NULL,0.0f,&D3DXVECTOR2((float)(x + w - iFrameW),(float)(y + iFrameW)),dwColor);
	TRY_END

}

// 不使用D3D的灵活顶点格式是因为每次锁定顶点缓冲会有比较大的开销，反而缩放绘制纹理少些
void CGraphicD3D::DrawFillRect(int x,int y,int w,int h,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
			return;

	m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)w,(float)h),NULL,0.0f,&D3DXVECTOR2((float)x,(float)y),dwColor);
	TRY_END
}

// 画填充的带宽边框的矩形
void CGraphicD3D::DrawRectangleWithFrame(DWORD frame_color,DWORD fill_color,int left,int top,int right,int bottom,int frame_width)
{
	TRY_BEGIN
		if( frame_width == 1 )
			DrawRect(left,top,right-left,bottom-top,frame_color);
		else
		{
			DrawFillRect(left,top,right-left,frame_width,frame_color);//上面边框
			DrawFillRect(left,bottom-frame_width,right-left,frame_width,frame_color);//下面边框
			DrawFillRect(left,top,frame_width,bottom-top,frame_color);//左面边框
			DrawFillRect(right-frame_width,top,frame_width,bottom-top,frame_color);//右面边框
		}

		DrawFillRect(left+frame_width,top+frame_width,right-frame_width-left-frame_width,bottom-frame_width-top-frame_width,fill_color);
		TRY_END
}

// 绘制直线(目前只支持横线和竖线)
void CGraphicD3D::DrawLine(int pos,int p0,int p1,DWORD dwColor,BOOL bHorizontal)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
			return;

	if(bHorizontal)		// 横线
		m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2((float)(p1 - p0),1.0f),NULL,0.0f,&D3DXVECTOR2((float)p0,(float)pos),dwColor);
	else				// 竖线
		m_pSprite->Draw(m_pRectTex,&m_rcAtom,&D3DXVECTOR2(1.0f,(float)(p1 - p0)),NULL,0.0f,&D3DXVECTOR2((float)pos,(float)p0),dwColor);
	TRY_END
}

void CGraphicD3D::DrawGouraudLine(int x0, int y0, int x1, int y1, DWORD color0, DWORD color1)
{
	if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
		return;

	CUSTOMVERTEX *v;
	HRESULT hr;

	m_iVertexBuffer++;
	if(m_iVertexBuffer >= MAX_VERTEXBUFFER) m_iVertexBuffer = 0;

	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	if(hr != D3D_OK) return;

	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
	if(hr != D3D_OK) return;

	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
	v[0].sz = v[1].sz =  v[2].sz = v[3].sz = 0.5f;

	v[0].color = color0;
	v[1].color = color1;
	v[2].color = color0;
	v[3].color = color1;

	v[0].sx = (float)x0;
	v[0].sy = (float)y0;
	v[1].sx = (float)x1;
	v[1].sy = (float)y1;

	if ((x0 < x1 && y0 < y1) || (x0 > x1 && y0 > y1))
	{
		v[2].sx = (float)x0+1;
		v[2].sy = (float)y0-1;
		v[3].sx = (float)x1+1;
		v[3].sy = (float)y1-1;
	}
	else
	{
		v[2].sx = (float)x0+1;
		v[2].sy = (float)y0+1;
		v[3].sx = (float)x1+1;
		v[3].sy = (float)y1+1;
	}

	m_pVertexBuffer[m_iVertexBuffer]->Unlock();

	hr = m_p3dDev->SetTexture(0,NULL);
	if( hr != D3D_OK ) return;

	DWORD dwFVF = 0;
	hr = m_p3dDev->GetFVF(&dwFVF);
	if(hr != D3D_OK) return;

	m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_p3dDev->SetFVF(dwFVF);

	if( hr != D3D_OK ) return;
}

void CGraphicD3D::DrawBatchLine(std::vector<POINT>& points,std::vector<DWORD>& colors)
{
	if(!bCanDraw || !m_bNeedDraw || !m_pRectTex || points.size() == 0)
		return;

	//设置颜色
	DWORD dwColor = 0xFFFFFFFF;
	for(size_t ii = colors.size(); ii < points.size();ii++)
	{
		colors.push_back(dwColor);
	}

	int iPos = 0;
	while(iPos < points.size()) //绘制
	{
		//要绘制的点的个数
		int iNums = (points.size() - iPos) / 2;
		if(iNums > 64) iNums = 64;
		else if(iNums == 0)
			return;

		CUSTOMVERTEX *v;
		HRESULT hr;

		m_iVertexBuffer++;
		if(m_iVertexBuffer >= MAX_VERTEXBUFFER)
			m_iVertexBuffer = 0;

		hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
		if(hr != D3D_OK) return;

		hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*iNums*3,(void**)&v, 0);
		if(hr != D3D_OK) return;

		for(int i = 0; i < iNums;i++)
		{
			int k = i * 3;
			POINT pt1 = points.at(iPos + i*2);
			POINT pt2 = points.at(iPos + i*2+1);

			DWORD color1 = colors.at(iPos + i*2);
			DWORD color2 = colors.at(iPos + i*2+1);

			v[k].rhw = v[k+1].rhw = v[k+2].rhw = 1.0f;
			v[k].sz = v[k+1].sz = v[k+2].sz = 0.5f;

			v[k+0].color = color1;
			v[k+1].color = color2;
			v[k+2].color = color2;

			v[k+0].sx = (float)pt1.x;			v[k+0].sy = (float)pt1.y;
			v[k+1].sx = (float)pt2.x;			v[k+1].sy = (float)pt2.y;
			v[k+2].sx = (float)pt2.x+2.0f;		v[k+2].sy = (float)pt2.y;
		}
		m_pVertexBuffer[m_iVertexBuffer]->Unlock();

		hr = m_p3dDev->SetTexture(0,NULL);
		if( hr != D3D_OK ) return;

		DWORD dwFVF = 0;
		hr = m_p3dDev->GetFVF(&dwFVF);
		if(hr != D3D_OK) return;

		m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);
		hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLELIST,0, iNums);
		m_p3dDev->SetFVF(dwFVF);

		if( hr != D3D_OK ) return;
		iPos += iNums * 2;
	}
}

void CGraphicD3D::DrawTideTexture(int px,int py,LPTexture tex,float size_,BYTE cDir_,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !tex)
			return;

	px += tex->GetOffX();
	py += tex->GetOffY();

	CUSTOMVERTEX *v;
	HRESULT hr;

	TEX_FRAME * tn = tex->GetCurFrame();
	LPD3DTEX	pD3DTex	= tn->vBlocks[0].GetVideoTex(m_p3dDev);

	if(pD3DTex == NULL)
		return;

	m_iVertexBuffer++;
	if(m_iVertexBuffer >= MAX_VERTEXBUFFER)
		m_iVertexBuffer = 0;

	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
	if( hr != D3D_OK ) return;

	//计算SeaTide的Polygon
	v[0].color = v[1].color = v[2].color = v[3].color = dwColor;
	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

	size_*=0.5f;
	switch (cDir_)
	{
	case 0:
		v[1].sx = px+size_;							
		v[3].sx = px-size_;
		v[2].sx = px-size_;							
		v[0].sx = px+size_;
		v[1].sy = (float)((int)(py+size_*0.5f)) -0.25f;
		v[3].sy = (float)((int)(py+size_*0.5f)) -0.25f;
		v[2].sy = (float)((int)(py-size_*0.5f)) +0.25f;
		v[0].sy = (float)((int)(py-size_*0.5f)) +0.25f;
		break;
	case 1:
		v[1].sx = (float)((int)(px + 0.7071f*size_*0.5f)) -0.25f;
		v[1].sy = (float)((int)(py + 0.7071f*size_*1.5f)) -0.25f;
		v[3].sx = (float)((int)(px - 0.7071f*size_*1.5f)) +0.25f;
		v[3].sy = (float)((int)(py - 0.7071f*size_*0.5f)) +0.25f;
		v[2].sx = (float)((int)(px - 0.7071f*size_*0.5f)) +0.25f;
		v[2].sy = (float)((int)(py - 0.7071f*size_*1.5f)) +0.25f;
		v[0].sx = (float)((int)(px + 0.7071f*size_*1.5f)) -0.25f;
		v[0].sy = (float)((int)(py + 0.7071f*size_*0.5f)) -0.25f;
		break;
	case 2:
		v[3].sx = (float)((int)(px-size_*0.5f)) +0.25f;
		v[2].sx = (float)((int)(px+size_*0.5f)) -0.25f;
		v[1].sx = (float)((int)(px-size_*0.5f)) +0.25f;
		v[0].sx = (float)((int)(px+size_*0.5f)) -0.25f;
		v[3].sy = py-size_;
		v[2].sy = py-size_;
		v[1].sy = py+size_;
		v[0].sy = py+size_;
		break;
	case 3:
		v[0].sx = (float)((int)(px - 0.7071f*size_*0.5f)) +0.25f;
		v[0].sy = (float)((int)(py + 0.7071f*size_*1.5f)) -0.25f;
		v[1].sx = (float)((int)(px - 0.7071f*size_*1.5f)) +0.25f;
		v[1].sy = (float)((int)(py + 0.7071f*size_*0.5f)) -0.25f;
		v[3].sx = (float)((int)(px + 0.7071f*size_*0.5f)) -0.25f;
		v[3].sy = (float)((int)(py - 0.7071f*size_*1.5f)) +0.25f;
		v[2].sx = (float)((int)(px + 0.7071f*size_*1.5f)) -0.25f;
		v[2].sy = (float)((int)(py - 0.7071f*size_*0.5f)) +0.25f;
		break;
	case 4:
		v[1].sx = px-size_;
		v[3].sx = px+size_;
		v[2].sx = px+size_;
		v[0].sx = px-size_;
		v[1].sy = (float)((int)(py-size_*0.5f)) +0.25f;
		v[3].sy = (float)((int)(py-size_*0.5f)) +0.25f;
		v[2].sy = (float)((int)(py+size_*0.5f)) -0.25f;
		v[0].sy = (float)((int)(py+size_*0.5f)) -0.25f;
		break;
	case 5:
		v[2].sx = (float)((int)(px + 0.7071f*size_*0.5f)) -0.25f;
		v[2].sy = (float)((int)(py + 0.7071f*size_*1.5f)) -0.25f;
		v[0].sx = (float)((int)(px - 0.7071f*size_*1.5f)) +0.25f;
		v[0].sy = (float)((int)(py - 0.7071f*size_*0.5f)) +0.25f;
		v[1].sx = (float)((int)(px - 0.7071f*size_*0.5f)) +0.25f;
		v[1].sy = (float)((int)(py - 0.7071f*size_*1.5f)) +0.25f;
		v[3].sx = (float)((int)(px + 0.7071f*size_*1.5f)) -0.25f;
		v[3].sy = (float)((int)(py + 0.7071f*size_*0.5f)) -0.25f;
		break;
	case 6:
		v[3].sx = (float)((int)(px+size_*0.5f)) -0.25f;
		v[2].sx = (float)((int)(px-size_*0.5f)) +0.25f;
		v[1].sx = (float)((int)(px+size_*0.5f)) -0.25f;
		v[0].sx = (float)((int)(px-size_*0.5f)) +0.25f;
		v[3].sy = py+size_;
		v[2].sy = py+size_;
		v[1].sy = py-size_;
		v[0].sy = py-size_;
		break;
	case 7:
		v[3].sx = (float)((int)(px - 0.7071f*size_*0.5f)) +0.25f;
		v[3].sy = (float)((int)(py + 0.7071f*size_*1.5f)) -0.25f;
		v[2].sx = (float)((int)(px - 0.7071f*size_*1.5f)) +0.25f;
		v[2].sy = (float)((int)(py + 0.7071f*size_*0.5f)) -0.25f;
		v[0].sx = (float)((int)(px + 0.7071f*size_*0.5f)) -0.25f;
		v[0].sy = (float)((int)(py - 0.7071f*size_*1.5f)) +0.25f;
		v[1].sx = (float)((int)(px + 0.7071f*size_*1.5f)) -0.25f;
		v[1].sy = (float)((int)(py - 0.7071f*size_*0.5f)) +0.25f;
		break;
	}

	v[0].tu = 0.0f;	v[0].tv = 0.5f;
	v[1].tu = 0.0f;	v[1].tv = 0.0f;
	v[2].tu = 1.0f;	v[2].tv = 0.5f;
	v[3].tu = 1.0f;	v[3].tv = 0.0f;

	m_pVertexBuffer[m_iVertexBuffer]->Unlock();

	SetRenderMode(RM_LINEAR);
	SetRenderMode(RM_ALPHAADD);

	DWORD dwFVF = 0;
	m_p3dDev->GetFVF(&dwFVF);
	m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);

	hr = m_p3dDev->SetTexture(0,pD3DTex);
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	m_p3dDev->SetFVF(dwFVF);

	SetRenderMode(RM_POINT);
	SetRenderMode();
	TRY_END
}

void CGraphicD3D::DrawCloudTexture(int iX,int iY,int iOffX,int iOffY,LPTexture* pTex,DWORD dwColor,DWORD dwDensity,BOOL bCloud)
{
	TRY_BEGIN
		//参数检查
		if(!bCanDraw || !m_bNeedDraw || !pTex)
			return;

	for(int i = 0;i < 4;i++)
	{
		if(pTex[i] == NULL)	return;
		pTex[i]->EnableSysAnim(FALSE);
	}

	DWORD dwFVF;
	hr = m_p3dDev->GetFVF(&dwFVF);
	if( dwFVF != D3DFVF_CUSTOMVERTEX )
		hr = m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);

	D3DVIEWPORT9 vp,OldVP;
	LPDIRECT3DSURFACE9 pOldTarget = NULL;
	CUSTOMVERTEX *v;
	int inter;
	DWORD c;
	LPDIRECT3DSURFACE9 pSurfaceCloud[5];

	int loop,i,frame1,frame2;
	int size[4] = {32,64,128,256};
	int time[4] = {1000,1000,1000,1000};
	if(bCloud)
	{
		time[0] = 32000;time[1] = 32000;time[2] = 64000;time[3] = 64000;
	}

	int weight[4] = {2,4,8,16};
	static float frt[4] = {0.0f,0.0f,1.0f,1.0f};
	static int iSkipCloud=0;
	DWORD dwTimePast_ = GetTickCount();
	LPD3DTEX pD3DTex = NULL;


	SetRenderMode(RM_LINEAR);
	SetRenderMode(RM_WRAP);

	ZeroMemory(pSurfaceCloud,sizeof(pSurfaceCloud));
	for(i = 0;i < MAX_CLOUDSTEX;i++)
	{
		if( m_pCloudTex[i]->GetSurfaceLevel(0,&pSurfaceCloud[i]) != D3D_OK ) 
			goto FINISH_CLOUD;
	}

	DWORD dwCount = GetTickCount();
	int iDist_ = (dwCount / 100) % 2048;

	if( iSkipCloud == 0 )
	{
		// 初始化
		hr = m_p3dDev->GetViewport(&OldVP);
		hr = m_p3dDev->GetRenderTarget(0,&pOldTarget);
		if( hr != D3D_OK ) goto FINISH_CLOUD;

		// step 1
		vp.X = vp.Y = 0;
		vp.MaxZ = 1.0f;
		vp.MinZ = 0.0f;
		for(loop=0;loop<4;loop++)
		{
			inter = (dwTimePast_%time[loop])*255 / time[loop];
			vp.Width	= size[loop];
			vp.Height	= size[loop];
			hr = m_p3dDev->SetRenderTarget(0,pSurfaceCloud[loop]);
			if( hr != D3D_OK ) goto FINISH_CLOUD;
			m_p3dDev->SetViewport(&vp);

			SetRenderMode(RM_RGB);

			frame1	= (dwTimePast_%(time[loop]+time[loop]+time[loop]+time[loop])) / time[loop];
			frame2	= (frame1+1) % 4;

			// render frame1
			pTex[loop]->SetCurFrame(frame1);
			pD3DTex = pTex[loop]->GetCurFrame()->vBlocks[0].GetVideoTex(m_p3dDev);

			if( !pD3DTex ) goto FINISH_CLOUD;

			c = 0xFF000000 | ((255-inter)<<16) | ((255-inter)<<8) | (255-inter);

			m_iVertexBuffer++;
			if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
			hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer],0,sizeof(CUSTOMVERTEX));
			hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			v[0].color = c; v[1].color = c; v[2].color = c; v[3].color = c;
			v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
			v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

			v[1].tu = 0.0f;					v[1].tv = 0.0f;
			v[3].tu = 1.0f;					v[3].tv = 0.0f;
			v[2].tu = 1.0f;					v[2].tv = 1.0f;
			v[0].tu = 0.0f;					v[0].tv = 1.0f;
			v[0].sx = 0.0f;					v[0].sy = float(size[loop]);
			v[1].sx = 0.0f;					v[1].sy = 0.0f;
			v[2].sx = float(size[loop]);	v[2].sy = float(size[loop]);
			v[3].sx = float(size[loop]);	v[3].sy = 0.0f;

			hr = m_pVertexBuffer[m_iVertexBuffer]->Unlock();
			hr = m_p3dDev->SetTexture(0,pD3DTex);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			SetRenderMode(RM_ADD2);

			// render frame2
			pTex[loop]->SetCurFrame(frame2);
			pD3DTex = pTex[loop]->GetCurFrame()->vBlocks[0].GetVideoTex(m_p3dDev);
			if( !pD3DTex ) goto FINISH_CLOUD;

			c = 0xFF000000 | (inter<<16) | (inter<<8) | inter;

			m_iVertexBuffer++;
			if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
			hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer],0, sizeof(CUSTOMVERTEX));
			hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			v[0].color = c; v[1].color = c; v[2].color = c; v[3].color = c;
			v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
			v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

			v[1].tu = 0.0f;					v[1].tv = 0.0f;
			v[3].tu = 1.0f;					v[3].tv = 0.0f;
			v[2].tu = 1.0f;					v[2].tv = 1.0f;
			v[0].tu = 0.0f;					v[0].tv = 1.0f;
			v[0].sx = 0.0f;					v[0].sy = float(size[loop]);
			v[1].sx = 0.0f;					v[1].sy = 0.0f;
			v[2].sx = float(size[loop]);	v[2].sy = float(size[loop]);
			v[3].sx = float(size[loop]);	v[3].sy = 0.0f;

			hr = m_pVertexBuffer[m_iVertexBuffer]->Unlock();
			hr = m_p3dDev->SetTexture(0,pD3DTex);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			if( hr != D3D_OK ) goto FINISH_CLOUD;
		}

		// step2  4to1
		m_p3dDev->SetRenderTarget(0,pSurfaceCloud[4]);
		m_p3dDev->SetViewport(&vp);

		SetRenderMode(RM_ADD2);

		m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0,0,0);
		for(loop=0;loop<4;loop++)
		{
			//compute combination frame
			m_iVertexBuffer++;
			if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
			hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer],0,sizeof(CUSTOMVERTEX));
			hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			inter = 255 / weight[loop];
			c = 0xFF000000 | (inter<<16) | (inter<<8) | inter;

			v[0].color = v[1].color = v[2].color = v[3].color = c;
			v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
			v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

			v[1].tu = 0.0f;				v[1].tv = 0.0f;
			v[3].tu = 1.0f;				v[3].tv = 0.0f;
			v[2].tu = 1.0f;				v[2].tv = 1.0f;
			v[0].tu = 0.0f;				v[0].tv = 1.0f;
			v[0].sx = 0.0f;				v[0].sy = float(size[3]);
			v[1].sx = 0.0f;				v[1].sy = 0.0f;
			v[2].sx = float(size[3]);	v[2].sy = float(size[3]);
			v[3].sx = float(size[3]);	v[3].sy = 0.0f;

			m_pVertexBuffer[m_iVertexBuffer]->Unlock();
			m_p3dDev->SetTexture(0,m_pCloudTex[loop]);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			if( hr != D3D_OK ) goto FINISH_CLOUD;
		}

		if(bCloud)
		{
			// step 3	
			//compute square + 1-B
			m_p3dDev->SetRenderTarget(0,pSurfaceCloud[3]);
			m_p3dDev->SetViewport(&vp);
			m_iVertexBuffer++;
			if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
			hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer],0,sizeof(CUSTOMVERTEX));
			hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			c = 0xFFFFFFFF;
			v[0].color = c; v[1].color = c; v[2].color = c; v[3].color = c;
			v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
			v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

			v[1].tu = 0.0f;				v[1].tv = 0.0f;
			v[3].tu = 1.0f;				v[3].tv = 0.0f;
			v[2].tu = 1.0f;				v[2].tv = 1.0f;
			v[0].tu = 0.0f;				v[0].tv = 1.0f;
			v[0].sx = 0.0f;				v[0].sy = float(size[3]);
			v[1].sx = 0.0f;				v[1].sy = 0.0f;
			v[2].sx = float(size[3]);	v[2].sy = float(size[3]);
			v[3].sx = float(size[3]);	v[3].sy = 0.0f;

			m_pVertexBuffer[m_iVertexBuffer]->Unlock();
			inter = 255 - dwDensity;

			c = 0xFF000000 | (inter<<16) | (inter<<8) | inter;

			m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,c,0,0);

			m_p3dDev->SetTexture(0,m_pCloudTex[4]);

			SetRenderMode(RM_ADD2);

			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			//compute B-square
			m_p3dDev->SetRenderTarget(0,pSurfaceCloud[4]);
			m_p3dDev->SetTexture(0,m_pCloudTex[3]);
			m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0xFFFFFFFF,0,0);
			m_p3dDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			if( hr != D3D_OK ) goto FINISH_CLOUD;

			//get energy back
			m_p3dDev->SetRenderTarget(0,pSurfaceCloud[3]);
			m_p3dDev->SetTexture(0,m_pCloudTex[4]);
			m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0xFFFFFFFF,0,0);
			m_p3dDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

			//square
			m_p3dDev->SetRenderTarget(0,pSurfaceCloud[4]);
			m_p3dDev->SetTexture(0,m_pCloudTex[3]);
			m_p3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0xFFFFFFFF,0,0);
			m_p3dDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_p3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		}

		// step 4
		m_p3dDev->SetRenderTarget(0,pOldTarget);
		m_p3dDev->SetViewport(&OldVP);
		SAFE_RELEASE(pOldTarget);
	}
	iSkipCloud++;
	if( iSkipCloud > CLOUDSKIPFRAME ) iSkipCloud = 0;	// 避免过多的不必要的刷新

	SetRenderMode(RM_ADD1);
	m_p3dDev->SetTexture(0,m_pCloudTex[4]);	

	m_iVertexBuffer++;
	if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*4 , (void**)&v, 0);
	if( hr != D3D_OK ) goto FINISH_CLOUD;

	if(bCloud)//云
	{
		frt[0] = float(iDist_)/2048.0f + float((iX*4)%128)*0.0078125f;
		frt[1] = float(iDist_)/2048.0f + float((iY*2)%128)*0.0078125f;
	}
	else//狂砂
	{
		frt[0] = float(dwDensity)/2048.0f + float((iX*4)%128)*0.0078125f;
		frt[1] = float(dwDensity)/2048.0f + float((iY*2)%128)*0.0078125f;
	}

	frt[2] = frt[0]+1.0f;
	frt[3] = frt[1]+1.0f;

	v[0].color = v[1].color = v[2].color = v[3].color = dwColor;
	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

	v[0].sx = float(iOffX);			v[0].sy = float(iOffY)+2048.0f-8.0f;
	v[0].tu = frt[0];				v[0].tv = frt[3];
	v[1].sx = float(iOffX);			v[1].sy = float(iOffY);
	v[1].tu = frt[0];				v[1].tv = frt[1];
	v[2].sx = float(iOffX)+2048.0f;	v[2].sy = float(iOffY)+2048.0f-8.0f;
	v[2].tu = frt[2];				v[2].tv = frt[3];
	v[3].sx = float(iOffX)+2048.0f;	v[3].sy = float(iOffY);
	v[3].tu = frt[2];				v[3].tv = frt[1];

	m_pVertexBuffer[m_iVertexBuffer]->Unlock();
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	if(!bCloud)
		hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);


	// 清理环境
FINISH_CLOUD:
	if( pOldTarget )
	{
		m_p3dDev->SetRenderTarget(0,pOldTarget);
		m_p3dDev->SetViewport(&OldVP);
		SAFE_RELEASE(pOldTarget);
	}
	for(i=0;i<5;i++)
		SAFE_RELEASE(pSurfaceCloud[i]);

	SetRenderMode(RM_CLAMP);
	SetRenderMode(RM_POINT);
	SetRenderMode();

	if( dwFVF != D3DFVF_CUSTOMVERTEX )
		hr = m_p3dDev->SetFVF(dwFVF);

	TRY_END
}


void CGraphicD3D::DrawTextureNL_Trans(int px,int py,LPTexture pTex,float fScaleX,float fScaleY,float fYTrans,DWORD dwColor)
{
	TRY_BEGIN
		if(!bCanDraw || !m_bNeedDraw || !pTex || !(pTex->GetCurFrame()))
			return;

	pTex->UpdateFrame(m_dwFrameCount);

	TEX_FRAME & frame	= *(pTex->GetCurFrame());
	LPD3DTEX	pD3DTex	= NULL;
	int			x0		= px - frame.wCenterX + frame.wOffX;
	int			y0		= py - frame.wCenterY + frame.wOffY;
	RECT rc;
	int iBlockWidth = frame.vBlocks[0].wWidth;
	int iBlockHeight = frame.vBlocks[0].wHeight;

	for(int j = 0;j < frame.wYBlocks;j++)
	{
		x0 = px - frame.wCenterX + frame.wOffX;
		for(int i = 0;i < frame.wXBlocks;i++)
		{
			TEX_BLOCK &block = frame.vBlocks[j * frame.wXBlocks + i];
			if(pD3DTex = block.GetVideoTex(m_p3dDev))
			{
				//m_pSprite->Draw(pD3DTex,NULL,NULL,NULL,0,&D3DXVECTOR2((float)x0,(float)y0),dwColor);
				rc.left		= 0;
				rc.top		= 0;
				rc.right	= block.wWidth;
				rc.bottom	= block.wHeight;

				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);

				mat._11 = (float)fScaleX;//x scale   1.0
				mat._22 = (float)fScaleY;//y scale   1.0
				mat._12 = (float)fYTrans;//y方向的拉伸(切变) 0.5
				mat._13 = 0;
				mat._41 = (float)x0 - i*(1- fScaleX)*iBlockWidth;
				mat._42 = (float)(y0-iBlockHeight*mat._22) - j*(1-fScaleY)*iBlockHeight;
				m_pSprite->DrawTransform(pD3DTex,&rc,&mat,dwColor);
			}

			x0 += iBlockWidth;
		}
		y0 += iBlockHeight;
	}
	TRY_END
}

void CGraphicD3D::DrawD3DTexture(int left,int top,int right,int bottom,LPTexture pTex,DWORD dwColor)
{
	TRY_BEGIN
	if(!bCanDraw || !m_bNeedDraw || !pTex || !m_p3dDev || !pTex->GetCurFrame())
		return;


	TEX_FRAME & frame	= *(pTex->GetCurFrame());
	if(!frame.vBlocks)
		return;

	TEX_BLOCK &block = frame.vBlocks[0];
	LPD3DTEX pD3DTex	= block.GetVideoTex(m_p3dDev);
	if(!pD3DTex)
		return;


	CUSTOMVERTEX *v;
	HRESULT hr;

	// 更换顶点缓冲区，以实现并行操作

	m_iVertexBuffer++;
	if( m_iVertexBuffer >= MAX_VERTEXBUFFER ) m_iVertexBuffer = 0;
	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*6 , (void**)&v, 0);

	if( hr != D3D_OK ) return;

	v[0].color = v[1].color = v[2].color = v[3].color = dwColor;
	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.5f;

	v[0].sx = (float)left;
	v[0].sy = (float)top;
	v[0].tu = 0.0f;
	v[0].tv = 0.0f;
	v[1].sx = (float)right;
	v[1].sy = (float)top;
	v[1].tu = 1.0f;
	v[1].tv = 0.0f;

	v[2].sx = (float)left;
	v[2].sy = (float)bottom;
	v[2].tu = 0.0f;
	v[2].tv = 1.0f;

	v[3].sx = (float)right;
	v[3].sy = (float)bottom;
	v[3].tu = 1.0f;
	v[3].tv = 1.0f;



	//如果顶点格式不对则设置顶点格式
	DWORD dwFVF;
	m_p3dDev->GetFVF(&dwFVF);
	if( dwFVF != D3DFVF_CUSTOMVERTEX )
		m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);
	// 显示纹理
	m_pVertexBuffer[m_iVertexBuffer]->Unlock();
	hr = m_p3dDev->SetTexture(0,pD3DTex);
	if( hr != D3D_OK ) return;
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	if( hr != D3D_OK ) return;
	//还原顶点格式
	if( dwFVF != D3DFVF_CUSTOMVERTEX )
		m_p3dDev->SetFVF(dwFVF);

	TRY_END
}

void CGraphicD3D::SetRenderIGW(bool b)
{
	m_bRenderIGW = b;
}

bool CGraphicD3D::IsRenderIGW()
{
	return m_bRenderIGW;
}

void CGraphicD3D::DrawTriangle( int x0,int y0,int x1,int y1,int x3,int y3,DWORD color /*= 0xFFFFFFFF*/ )
{
	if(!bCanDraw || !m_bNeedDraw || !m_pRectTex)
		return;

	CUSTOMVERTEX *v;
	HRESULT hr;

	m_iVertexBuffer++;
	if(m_iVertexBuffer >= MAX_VERTEXBUFFER) m_iVertexBuffer = 0;

	hr = m_p3dDev->SetStreamSource(0, m_pVertexBuffer[m_iVertexBuffer], 0,sizeof(CUSTOMVERTEX));
	if(hr != D3D_OK) return;

	hr = m_pVertexBuffer[m_iVertexBuffer]->Lock(0, sizeof(CUSTOMVERTEX)*3 , (void**)&v, 0);
	if(hr != D3D_OK) return;

	v[0].rhw = v[1].rhw = v[2].rhw  = 1.f;
	v[0].sz = v[1].sz =  v[2].sz = 0.5f;

	v[0].color = color;
	v[1].color = color;
	v[2].color = color;	

	v[0].sx = (float)x0;
	v[0].sy = (float)y0;
	v[1].sx = (float)x1;
	v[1].sy = (float)y1;
	v[2].sx = (float)x3;
	v[2].sy = (float)y3;	

	m_pVertexBuffer[m_iVertexBuffer]->Unlock();

	hr = m_p3dDev->SetTexture(0,NULL);
	if( hr != D3D_OK ) return;

	DWORD dwFVF = 0;
	hr = m_p3dDev->GetFVF(&dwFVF);
	if(hr != D3D_OK) return;

	m_p3dDev->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = m_p3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 1);
	m_p3dDev->SetFVF(dwFVF);

	if( hr != D3D_OK ) return;
}