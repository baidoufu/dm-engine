#pragma once

#include "Define.h"
#include "Global/Interface/TexManagerInterface.h"
#include "TextureD3D.h"

#include <queue>

class DataStreamInterface;

class CTexManager : public CTexManagerInterface
{
private:
	// 块结构
	struct BlockData
	{
		BYTE		bCompr;			// 是否压缩
		BYTE		ePixelType;		// 纹理格式
		WORD		wReserved;		// 保留
		WORD		wWidth;			// 宽度
		WORD		wHeight;		// 高度
		DWORD		dwSize;			// 像素数据大小

		BYTE *		pData;			// 像素数据
	};

	// 帧结构
	struct FrameData
	{
		WORD		wWidth0;		// 原始宽度
		WORD		wHeight0;		// 原始高度
		WORD		wWidth;			// 宽度
		WORD		wHeight;		// 高度
		SHORT		wOffX;			// X偏移
		SHORT		wOffY;			// Y偏移
		SHORT		wCenterX;		// 中心点X
		SHORT		wCenterY;		// 中心点Y
		WORD		wXBlocks;		// 横向块数
		WORD		wYBlocks;		// 纵向块数

		BlockData*	pBlock;			// BLockData
	};

	// 纹理数据
	struct TexData
	{
		DWORD		dwMagic;		// 魔数
		DWORD		dwChildOff;		// 子数据偏移

		DWORD		dwFrameSize;	// FrameData结构头大小(不包括指针pBlock)
		DWORD		dwBlockSize;	// BlockData结构头大小(不包括指针pData)

		// tex属性
		DWORD		dwOption;		// 属性
		DWORD		dwSpeed;		// 帧速
		DWORD		dwFrames;		// 帧数
	};

	struct stPQueue
	{
		std::queue<DWORD> vID[EP_TYPES];
		int iSize;

		stPQueue()
		{
			iSize = 0;
		}

		int	size(void)
		{
			return iSize;
		}

		void push(DWORD id,eReadPrior ep)
		{
			if(ep < EP_TYPES)
			{
				vID[ep].push(id);
				iSize ++;
			}
		}

		void pop(DWORD &dwID,eReadPrior &ep)
		{
			dwID = -1;
			if(iSize < 0)
				return;

			for(int i = 0;i < EP_TYPES;i++)
			{
				if(vID[i].size() <= 0)
					continue;

				dwID = vID[i].front();
				ep = eReadPrior(i);
				vID[i].pop();
				iSize --;

				//return (dwID == 0) ? -1 : dwID;
				if (dwID == 0)
				{
					dwID = -1;
				}
				break;
			}
		}
	};

	struct stSgl
	{
		stSgl()
		{
			strcpy(sPath,"");
			strcpy(sName,"");
		}
		char	sPath[260];		// 文件夹名称
		char    sName[24];      // 图包名称
	};

	struct stSglWoool
	{
		stSglWoool()
		{
			strcpy(sPath,"");
			strcpy(sName,"");
			bDirType = false;
			hFile = NULL;
			fp1 = fp2 = 0;
			dwFiles = 0;
			vFileOff = NULL;
			vFileSize = NULL;
		}
		char	sPath[260];		// 文件夹名称
		char    sName[24];      // 图包名称
		BOOL	bDirType;		// 是否是文件夹模式

		HANDLE 	hFile;			// sgl文件句柄
		FILE*	fp1;			// 主线程文件的指针
		FILE*	fp2;			// 读线程文件的指针
		DWORD	dwFiles;		// sgl里面的子文件数
		DWORD*	vFileOff;		// 每个子文件的偏移
		DWORD*  vFileSize;      // 每个子文件的大小
	};

	struct stReaded
	{
		DWORD		dwID;
		LPTexture	pTex;

		stReaded(DWORD id = 0,LPTexture p = NULL):dwID(id),pTex(p){}
	};

	typedef std::map<int,std::string>		MString;
	typedef std::map<int,stSglWoool>				MSglFile;
	typedef std::vector<stReaded>			VReaded;
	typedef std::map<DWORD,LPTexture>		MAllTex;
	typedef CLufoList<LPTexture>			TexLufoList;

	eReadResult			m_eReadResult;			// 缓冲模式纹理读取结果
	__int64				m_iCurFixedSize;		// 当前非缓冲读取纹理大小
	__int64				m_iMaxFixedSize;		// 最大非缓冲读取纹理大小
	static __int64		m_iCurCacheSize;		// 当前缓冲读取纹理大小
	static __int64		m_iMaxCacheSize;		// 最大允许缓冲读取纹理大小
	static int			m_iCurChacheSize;		// 当前缓冲读取纹理大小
	static int			m_iMaxChacheSize;		// 最大允许缓冲读取纹理大小
	static __int64      m_iFreeCacheSize;       // 每次Free内存的时候清理的最小内存大小

	static CTexPool		m_TexMemPool;			// 纹理内存池
	static MString		m_mSglName;				// sgl文件名表
	static MSglFile		m_mSglFile;				// sgl文件映射
	static std::string  m_strDataDir;           // data文件夹

	static stPQueue		m_pqWillRead;			// 将要读取
	static VReaded		m_vReaded;				// 已经读取

	static MAllTex		m_mAllTex;				// 所有纹理映射
	static TexLufoList	m_LufoList;				// 最久未用淘汰列表

	////////////////////////////////////
	static HANDLE			m_hThread;						// 线程句柄
	static DWORD			m_dwThreadID;					// 线程ID
	static BOOL				m_bExited;						// 线程是否需要结束
	static CRITICAL_SECTION	m_AskCriSect;					// 请求临界变量
	static CRITICAL_SECTION	m_FeedCriSect;					// 回应临界变量




	static DWORD WINAPI		ThreadLoadTex(LPVOID lpData);	// 纹理读取线程函数
	////////////////////////////////////

private:
	static BOOL			UpdateSglFile(int iPack);
	static void			PopThreadReaded(void);

	static LPTexture	Inter_LoadTex(DWORD dwID,bool bThread,eReadPrior ePrior);		// 装载
	static LPTexture	Inter_ReadTex(DataStreamInterface&);					// 根据文件指针读取
	static LPTexture    Inter_ReadOldTex(DataStreamInterface&,DWORD dwMagic = 'TEX1'); // 根据文件流读取
	static void			Inter_ReleaseTex(LPTexture& pTex);			// 释放

	void				WashOut(void);			// 淘汰机制

public:
	CTexManager(void);
	~CTexManager(void);

	// 处理sgl文件
	bool        Init(const char* strDataDir);

	LPTexture   LoadImage(const char* path);    // 导入其他格式文件，使用FreeImage

	// 非缓冲模式相关
	LPTexture   LoadTex(int iPack,int iIdx,eReadPrior ePrior);    // 根据图包编号和图片编号
	//LPTexture   LoadTex(FILE* fp);              // 从文件指针读取
	LPTexture	LoadTex(LPCTSTR sTexFile,eReadPrior ePrior);		// 根据文件名读取纹理
	LPTexture	LoadTexFromDiskFile(LPCTSTR sTexFile,eReadPrior ePrior);// 根据文件名读取纹理,不从图包中读取,而是从磁盘文件中读取
	LPTexture	LoadTex(DWORD dwID,eReadPrior ePrior);		    // 根据编号读取纹理(优先读取sgl文件中，没有的话读取文件夹中的)
	LPTexture   LoadMemTex(int iW,int iH,BYTE* pBuf,int iPitch = 0);
	void		ReleaseTex(LPTexture& pTex);	// 释放纹理(上面两种加载的方式都需要调用本函数释放，否者会有内存泄漏)
	void        ClearAllTex();
	void        ClearAllNullTex();

	// 缓冲模式相关
	LPTexture	GetTex(int iPack,int iIdx,eReadPrior ePrior);				// 根据图库包，还有索引读取纹理，缓冲模式取得纹理，可以指定读取的优先等级
	LPTexture	GetTexImm(int iPack,int iIdx,eReadPrior ePrior,int *pIResult = NULL);											// 根据图库包，还有索引读取纹理，缓冲模式立即取得纹理
	LPTexture	GetTexImm(DWORD dwID,eReadPrior ePrior,int *pIResult = NULL);											        // 根据ID，缓冲模式立即取得纹理
	LPTexture	GetTexFromID(DWORD dwID,eReadPrior ePrior,int *pIResult = NULL);				// 根据ID，缓冲模式取得纹理，可以指定读取的优先等级
    bool        HasTexInServer(DWORD dwID);//纹理在服务器是否存在
    bool        HasTexInServer(int iPack,int iIdx);//纹理在服务器是否存在
	bool        HasTexInLocalWpf(DWORD dwID);//纹理在本地是否存在
	bool        HasTexInLocalWpf(int iPack,int iIdx);//纹理在本地是否存在
	bool        GetTexFilePath(int iPack,int iIdx,char *szPath);//根据纹理编号得到文件路径,外部保存szPath足够长

	void		PreLoad(DWORD dwBeginID,DWORD dwEndID,eReadPrior ePrior);	// 根据ID预读纹理
	void        PreLoad(int iPack,int iBeginID,int iEndID,eReadPrior ePrior);// 根据ID预读纹理
	void		PreLoadImm(DWORD dwBeginID,DWORD dwEndID,eReadPrior ePrior);	// 根据ID预读纹理
	void        PreLoadImm(int iPack,int iBeginID,int iEndID,eReadPrior ePrior);// 根据ID预读纹理
	void        DelTexFile(int iPack,int iBeginID,int iEndID);// 根据ID删除对应的图片,删除魔法相关的图片时用到
	int			GetReadingTexNum(void);			// 取得等待线程读取的纹理数
	eReadResult	GetLastReadStatus(void);		// 取得最近一次缓冲模式纹理读取的结果

	void        SetMaxCacheSize(int i);
	__int64     GetCurCacheSize();
	__int64     GetMaxCacheSize();
	__int64     GetCurFixedSize();
	__int64     GetMaxFixedSize();
	bool        IsValidOldTex(UCHAR * pTex,int iLen);

	CTexPool&   GetTexMemPool();
	void        OnDownloadTex(DWORD dwID,const char* buf,DWORD dwLen);//有纹理通过下载服务器下载到本地了
	void        OnSendDownloadTex(DWORD dwID);//准备下载dwID,更新纹理池状态
	void		PreDownLoadTex(int iPack,int iBeginID,int iEndID,eReadPrior ePrior);	// 根据ID预下载纹理

};

extern CTexManager * g_pETexMgr;