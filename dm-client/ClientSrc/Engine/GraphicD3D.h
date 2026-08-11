#pragma once

#include "Define.h"
#include "TextureD3D.h"
#include <vector>
#include "Global/Interface/GraphicInterface.h"

class CGraphicD3D : public CGraphicInterface
{
	friend class CLight;
	// 桌面显示信息
	struct DesktopInfo
	{
		DWORD	bits;
		DWORD	width;
		DWORD	height;
		DWORD	rate;
	};
	// 保存jpg设置信息
	struct JpgSaveInfo
	{
		BOOL			b32Bits;
		char			sJpgFile[256];
		RECT			rc;
		D3DLOCKED_RECT	lr;
	};
	// 定点缓冲，定点索引缓冲
	enum {D3DFVF_CUSTOMVERTEX = D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1};
	struct CUSTOMVERTEX
	{
		float sx, sy, sz, rhw;							// 位置，rhw固定设定为1.0f
		DWORD color;									// 颜色
		float tu, tv;									// 此定点的纹理坐标
	};

protected:
	D3DADAPTER_IDENTIFIER9	m_AdapterInfo;	// 显卡信息
	DWORD					m_dwAvailMem;	// 可用的显存
	DWORD					m_dwAlloacMem;	// 分配的显存
	DWORD					m_dwShouldMem;	// 应该分配的显存

	int m_iVertexBuffer;					// 当前顶点缓冲
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer[MAX_VERTEXBUFFER];		// 顶点缓冲

	HRESULT		hr;							// D3D返回结果
	HWND		m_hWnd;						// 窗口句柄
	int			m_iWidth;					// 宽度
	int			m_iHeight;					// 高度
	DisplayMode	m_eDisMode;					// 是否窗口模式
	BOOL		m_bGameMode;				// 是否是游戏模式

	RECT		m_rcAtom;					// 矩形纹理单元格

	BOOL		m_bD3dOK;					// D3D是否正常
	BOOL		m_bDisplay;					// 主动设置显示开关

	BOOL		m_bFrameDraw;				// 本帧是否显示
	DWORD		m_dwStartTime;				// 开始绘制的时间
	DWORD		m_dwFrameCount;				// 绘制开始的帧计数
	DWORD		m_dwRealFPS;				// 当前的帧速
	DWORD       m_dwTotalRealFps;           // 每帧的fps之和,用来计算平均fps

	int			m_iPerFmeTime;				// 每帧花费的时间
	int			m_iNextTime;				// 下一帧显示时间

	BOOL		m_bDXTn;					// 是否支持DXT3格式
	BOOL		m_bMultiTex;				// 是否支持多重纹理混合
	BOOL		m_bMemSurf;					// m_pMemSurf是否是锁定态
	bool        m_bRenderIGW;               // 是否绘制IGW
	bool        m_bNeedDraw;                // 是否要求绘制

	LPDIRECT3D9				m_p3d;			// D3D8接口
	LPDIRECT3DDEVICE9		m_p3dDev;		// 设备接口
	LPDIRECT3DSURFACE9		m_pBackSurf;	// 背表面(需要重置)
	LPDIRECT3DSURFACE9		m_pMemSurf;		// 系统内存表面(不需要重置)
	LPD3DXSPRITE			m_pSprite;		// 精灵绘制接口(需要重置)
	LPD3DTEX				m_pLight;		// 光照纹理(需要重置)
	LPDIRECT3DSURFACE9		m_pLightSurf;	// 光照纹理表面(需要重置)
	LPD3DTEX				m_pTempRenderTarget;//临时的rendertarget(需要重置)
	LPDIRECT3DSURFACE9		m_pTempRenderTargetSurf;// 临时的rendertarget纹理表面(需要重置)

	LPD3DTEX				m_pRectTex;		// 绘制矩形使用的纹理(不需要重置)
	LPD3DTEX				m_pMiniMapTex;	// 绘制小地图使用的纹理(不需要重置)
	LPD3DTEX				m_pTileTex;		// 绘制地表使用的纹理(不需要重置)
	LPD3DTEX                m_pCloudTex[MAX_CLOUDSTEX]; // 云彩使用的纹理(不需要重置)

	D3DPRESENT_PARAMETERS	m_d3dpp;		// 设备参数
	D3DDISPLAYMODE			m_d3ddm;		// 原始显示模式
	DesktopInfo				m_DisInfo_Enter;		// 显示模式信息,
	D3DCAPS9                m_caps;         // 显示能力

///////////////////////////////////////////////////////////
// 保存截图线程相关
	static HANDLE			m_hThread;		// 线程句柄
	static BOOL				m_bSaving;		// 是否正在保存
	static BOOL				m_bExited;		// 是否结束线程
	static JpgSaveInfo		m_JSI;			// 保存jpg设置信息
	static BYTE*			m_pThreadBuf;	// 线程buffer
	// 线程函数
	static	DWORD WINAPI	ThreadSaveJpg(LPVOID lpData);	
///////////////////////////////////////////////////////////
private:
	BOOL		InitDXStates(void);
	BOOL		InitTexPool(BOOL bForce);
	void		ReleaseTexPool(BOOL bForce);

	BOOL		GetDisplayInfo(void);
	BOOL		GetCurScreenInfo(DesktopInfo& desinfo);

public:
				CGraphicD3D(void);
				~CGraphicD3D(void);

	DWORD&		GetAlloacMem();	// 分配的显存
	DWORD&      GetShouldMem();	// 应该分配的显存

	// DX相关
	BOOL		CreateD3D(HWND hWnd,int iW,int iH,DisplayMode eDisplayMode,BOOL bGameMode = TRUE,bool bRestoreDesktop = true);
	void		KillD3D(bool bRestoreDesktop = true);
	BOOL		ValidateDX(void);
	BOOL		ReCreateDX(void);
	BOOL        ResizeWindow(int iW,int iH);
	void		AdjustWindow(HWND hWnd,int iW,int iH,BOOL bPopup = FALSE, BOOL bCenter = FALSE);
	void		CenterWindow();
	BOOL		MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint, BOOL bCenter = FALSE);

	// 其他
	BOOL		WillShowFrame(void);
	BOOL		IsNoDraw(void);
	BOOL		IsDisplay(void);
	BOOL		RestoreDesktop(BOOL bOld);
	void		SetWidth(int i);
	void 		SetHeight(int i);
	int			GetWidth(void);
	int			GetHeight(void);
	void		GetDisSize(int &x,int &y);
	BOOL        IsBig(); //是否显示大窗口模式	
	void		RecomputeWindowXY(int & iW, int & iH, int iOW, int iOH, DisplayMode eWinMode);
	void		RecomputeDesktopXY(int & iDskW, int & iDskH, int iWinW, int iWinH, DisplayMode eWinMode, int iCurDisWidth, int iCurDisHeight);

	// 取得的信息
	DWORD		GetFPS(void);
	DWORD		GetTotalFPS(void);
	DWORD		GetLastSetFPS(void);
	DWORD		GetFrameCount(void);
	void        SetFrameCount(DWORD dwCount);
	DWORD       _GetTimeCount(); //当前帧应该绘制的时间
	void        SetDisMode(DisplayMode e){m_eDisMode = e;}
	DisplayMode	GetDisMode(void);
	void		GetAdapterInfo(char* str);

	// 设置的信息
	void		SetFPS(DWORD dwFPS);
	void		SetDisplay(BOOL b);
	void        SetFrameDraw(BOOL b);

	BOOL        IsSupportTide(); //是否支持海浪
	BOOL        IsSupportCloud(); //是否支持云彩

	int         GetPerFmeTime();
	void        SetPerFmeTime(int val);
	// 应用相关
	void		BeginDraw(void);
	void		EndDraw(HWND hWnd = NULL);
	void		ClearColor(DWORD dwColor = 0);
	void		SetRenderMode(RenderMode eMode = RM_ALPHA);

	// 绘制纹理
	void		DrawTextureNL(int x,int y,LPTexture pTex,DWORD dwColor = 0xFFFFFFFF);
	void		DrawTextureFX(int x,int y,LPTexture pTex,DWORD dwColor = 0xFFFFFFFF,LPRECT pRect = NULL,LPPOS pScale = NULL,float fRotX = 0.0f,float fRotY = 0.0f,float fRotz = 0.0f);
	void		DrawPartTexture(int x,int y,LPTexture pTex,int iLeft = 0,int iTop = 0,int iRight = -1,int iBottom = -1,DWORD dwColor = 0xFFFFFFFF,LPPOS pScale = NULL);
	void        DrawArcTexture(int px,int py,LPTexture pTex,float fBeginArc,float fEndArc,DWORD color=0xFFFFFFFF);
	void        DrawTextureNL_Trans(int px,int py,LPTexture pTex,float fScaleX = 1.0,float fScaleY = 1.0,float fYTrans = 0,DWORD dwColor = 0xFFFFFFFF);

	// 绘制3宫格(特殊绘制)
	void		DrawTetxure3G(int x,int y,LPTexture pTex,int iWPages,int iBX,int iBY,int iGW,DWORD dwColor = 0xFFFFFFFF);
	// 绘制9宫格(特殊绘制)
    void		DrawTetxure9G(int x,int y,LPTexture pTex,int iWPages,int iHPages,int iBX,int iBY,int iGW,int iGH,DWORD dwColor = 0xFFFFFFFF);
	// 只允许在绘制大地表的时候调用(特殊绘制)
	void		DrawTextureTL(int x,int y,LPTexture pColorTex,LPTexture pAlphaTex,DWORD dwColor = 0xFFFFFFFF);
	// 只允许在绘制小地图的时候调用(特殊绘制)
	void		DrawTextureMM(int x,int y,LPTexture pMiniMapTex,LPRECT pMiniMapRect,LPTexture pAlphaTex,DWORD dwColor = 0xFFFFFFFF);

	// 绘制海浪图片
	void		DrawTideTexture(int px,int py,LPTexture tex,float size_,BYTE cDir_,DWORD dwColor);
	void        DrawCloudTexture(int iX,int iY,int iOffX,int iOffY,LPTexture* pTex,DWORD dwColor,DWORD dwDensity,BOOL bCloud = TRUE);

	// 绘制几何图形
	void		DrawRect(int x,int y,int w,int h,DWORD dwColor,int iFrameW = 1);
	void		DrawFillRect(int x,int y,int w,int h,DWORD dwColor);
	void        DrawRectangleWithFrame(DWORD frame_color,DWORD fill_color,int left,int top,int right,int bottom,int frame_width=1);//画带边框的填充矩形
	void		DrawLine(int pos,int p0,int p1,DWORD dwColor,BOOL bHorizontal = TRUE);
	void        DrawGouraudLine(int x0,int y0,int x1,int y1,DWORD color0 = 0xFFFFFFFF,DWORD color1 = 0xFFFFFFFF);
	void        DrawBatchLine(std::vector<POINT>& points,std::vector<DWORD>& colors);
	void        DrawD3DTexture(int left,int top,int right,int bottom,LPTexture pTex,DWORD color = 0xFFFFFFFF);										// 显示带灯光效果的纹理
	void        DrawTriangle(int x0,int y0,int x1,int y1,int x3,int y3,DWORD color = 0xFFFFFFFF);

	// 线程保存截图
	BOOL		SaveSnapShot(LPCTSTR sJpgFile,LPRECT pRect = NULL);	

	D3DPRESENT_PARAMETERS *	GetD3DPP() {return &m_d3dpp;}
	LPDIRECT3DDEVICE9		GetD3DDev(){return m_p3dDev;}
	LPDIRECT3D9		        GetD3D(){return m_p3d;}
	LPD3DXSPRITE            GetSprite(){return m_pSprite;}
	LPD3DTEX                GetTempRenderTarget() const { return m_pTempRenderTarget; }
	LPDIRECT3DSURFACE9      GetTempRenderTargetSurf() const { return m_pTempRenderTargetSurf; }

	//是否绘制igw
	virtual void SetRenderIGW(bool b);
	virtual bool IsRenderIGW();

};

extern CGraphicD3D * g_pEGfx;

#include "GraphicD3D.inl"