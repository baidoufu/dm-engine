#pragma once

#include <d3dx9.h>
#include "TextureInterface.h"

#define MAX_VERTEXBUFFER 4
#define MAX_CLOUDSTEX    5


#define DXSPRITE_FLAG  D3DXSPRITE_ALPHABLEND


// 渲染模式
enum RenderMode
{
	RM_ALPHA	= 0	,
	RM_ADD1			,
	RM_ADD2			,
	RM_ALPHAADD		,
	RM_MODULATE		,
	RM_RGB			,
	RM_REALPHA		,

	RM_POINT		,
	RM_LINEAR		,

	RM_ADD3         ,

	RM_WRAP         ,
	RM_CLAMP        ,
};

class CGraphicInterface
{
public:
	virtual ~CGraphicInterface(){}
	// DX相关
	virtual BOOL		CreateD3D(HWND hWnd,int iW,int iH,DisplayMode eDisplayMode,BOOL bGameMode = TRUE,bool bRestoreDesktop = true) = 0;
	virtual void		KillD3D(bool bRestoreDesktop = true) = 0;
	virtual BOOL		ValidateDX(void) = 0;
	virtual BOOL		ReCreateDX(void) = 0;
	virtual BOOL        ResizeWindow(int iW,int iH) = 0;
	virtual void		AdjustWindow(HWND hWnd,int iW,int iH,BOOL bPopup = FALSE, BOOL bCenter = FALSE) = 0;
	virtual void		CenterWindow() = 0;
	virtual BOOL		MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint, BOOL bCenter = FALSE) = 0;

	// 其他
	virtual BOOL		WillShowFrame(void) = 0;
	virtual BOOL		IsNoDraw(void) = 0;
	virtual BOOL		IsDisplay(void) = 0;
	virtual BOOL		RestoreDesktop(BOOL bOld) = 0;
	virtual void		SetWidth(int i) = 0;
	virtual void 		SetHeight(int i) = 0;
	virtual int			GetWidth(void) = 0;
	virtual int			GetHeight(void) = 0;
	virtual void		GetDisSize(int &x,int &y) = 0;
	virtual BOOL        IsBig() = 0; //是否显示大窗口模式
	virtual void		RecomputeWindowXY(int & iW, int & iH, int iOW, int iOH, DisplayMode eWinMode) = 0;
	virtual void		RecomputeDesktopXY(int & iDskW, int & iDskH, int iWinW, int iWinH, DisplayMode eWinMode, int iCurDisWidth, int iCurDisHeight) = 0;

	// 取得的信息
	virtual DWORD		GetFPS(void) = 0;
	virtual DWORD		GetTotalFPS(void) = 0;
	virtual DWORD		GetLastSetFPS(void) = 0;
	virtual DWORD		GetFrameCount(void) = 0;
	virtual void        SetFrameCount(DWORD dwCount) = 0;
	virtual DWORD       _GetTimeCount() = 0; //当前帧应该绘制的时间
	virtual void        SetDisMode(DisplayMode e) = 0;
	virtual DisplayMode	GetDisMode(void) = 0;
	virtual void		GetAdapterInfo(char* str) = 0;

	// 设置的信息
	virtual void		SetFPS(DWORD dwFPS) = 0;
	virtual void		SetDisplay(BOOL b) = 0;
	virtual void        SetFrameDraw(BOOL b) = 0;

	virtual BOOL        IsSupportTide() = 0; //是否支持海浪
	virtual BOOL        IsSupportCloud() = 0; //是否支持云彩

	virtual int         GetPerFmeTime() = 0;
	virtual void        SetPerFmeTime(int val) = 0;
	// 应用相关
	virtual void		BeginDraw(void) = 0;
	virtual void		EndDraw(HWND hWnd = NULL) = 0;
	virtual void		ClearColor(DWORD dwColor = 0) = 0;
	virtual void		SetRenderMode(RenderMode eMode = RM_ALPHA) = 0;

	// 绘制纹理
	virtual void		DrawTextureNL(int x,int y,LPTexture pTex,DWORD dwColor = 0xFFFFFFFF) = 0;
	virtual void		DrawTextureFX(int x,int y,LPTexture pTex,DWORD dwColor = 0xFFFFFFFF,LPRECT pRect = NULL,LPPOS pScale = NULL,float fRotX = 0.0f,float fRotY = 0.0f,float fRotz = 0.0f) = 0;
	virtual void		DrawPartTexture(int x,int y,LPTexture pTex,int iLeft = 0,int iTop = 0,int iRight = -1,int iBottom = -1,DWORD dwColor = 0xFFFFFFFF,LPPOS pScale = NULL) = 0;
	virtual void        DrawArcTexture(int px,int py,LPTexture pTex,float fBeginArc,float fEndArc,DWORD color=0xFFFFFFFF) = 0;
	virtual void        DrawTextureNL_Trans(int px,int py,LPTexture pTex,float fScaleX = 1.0,float fScaleY = 1.0,float fYTrans = 0,DWORD dwColor = 0xFFFFFFFF) = 0;

	// 绘制3宫格(特殊绘制)
	virtual void		DrawTetxure3G(int x,int y,LPTexture pTex,int iWPages,int iBX,int iBY,int iGW,DWORD dwColor = 0xFFFFFFFF) = 0;
	// 绘制9宫格(特殊绘制)
	virtual void		DrawTetxure9G(int x,int y,LPTexture pTex,int iWPages,int iHPages,int iBX,int iBY,int iGW,int iGH,DWORD dwColor = 0xFFFFFFFF) = 0;
	// 只允许在绘制大地表的时候调用(特殊绘制)
	virtual void		DrawTextureTL(int x,int y,LPTexture pColorTex,LPTexture pAlphaTex,DWORD dwColor = 0xFFFFFFFF) = 0;
	// 只允许在绘制小地图的时候调用(特殊绘制)
	virtual void		DrawTextureMM(int x,int y,LPTexture pMiniMapTex,LPRECT pMiniMapRect,LPTexture pAlphaTex,DWORD dwColor = 0xFFFFFFFF) = 0;

	// 绘制海浪图片
	virtual void		DrawTideTexture(int px,int py,LPTexture tex,float size_,BYTE cDir_,DWORD dwColor) = 0;
	virtual void        DrawCloudTexture(int iX,int iY,int iOffX,int iOffY,LPTexture* pTex,DWORD dwColor,DWORD dwDensity,BOOL bCloud = TRUE) = 0;

	// 绘制几何图形
	virtual void		DrawRect(int x,int y,int w,int h,DWORD dwColor,int iFrameW = 1) = 0;
	virtual void		DrawFillRect(int x,int y,int w,int h,DWORD dwColor) = 0;
	virtual void        DrawRectangleWithFrame(DWORD frame_color,DWORD fill_color,int left,int top,int right,int bottom,int frame_width=1) = 0;//画带边框的填充矩形
	virtual void		DrawLine(int pos,int p0,int p1,DWORD dwColor,BOOL bHorizontal = TRUE) = 0;
	virtual void        DrawGouraudLine(int x0,int y0,int x1,int y1,DWORD color0 = 0xFFFFFFFF,DWORD color1 = 0xFFFFFFFF) = 0;
	virtual void        DrawBatchLine(std::vector<POINT>& points,std::vector<DWORD>& colors) = 0;
	virtual void        DrawD3DTexture(int left,int top,int right,int bottom,LPTexture pTex,DWORD color = 0xFFFFFFFF) = 0;										// 显示带灯光效果的纹理
	virtual void        DrawTriangle(int x0,int y0,int x1,int y1,int x3,int y3,DWORD color = 0xFFFFFFFF) = 0;

	// 线程保存截图
	virtual BOOL		SaveSnapShot(LPCTSTR sJpgFile,LPRECT pRect = NULL) = 0;	

	virtual D3DPRESENT_PARAMETERS *	GetD3DPP() = 0;
	virtual LPDIRECT3DDEVICE9		GetD3DDev() = 0;
	virtual LPDIRECT3D9		        GetD3D() = 0;
	virtual LPD3DXSPRITE            GetSprite() = 0;
	virtual LPD3DTEX                GetTempRenderTarget() const = 0;
	virtual LPDIRECT3DSURFACE9      GetTempRenderTargetSurf() const = 0;

	//是否绘制igw
	virtual void SetRenderIGW(bool b) = 0;
	virtual bool IsRenderIGW() = 0;
};

extern CGraphicInterface *   g_pGfx;
