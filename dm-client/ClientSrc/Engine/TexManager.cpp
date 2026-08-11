#include "stdafx.h"
#include "TexManager.h"
#include "BaseClass/DXTn/DXTn.h"
#include "TextureD3D.h"
#include "Global/Interface/CallBackInterface.h"
#include "Stream.h"

#include <string>
#include <shlobj.h>


#include <stdio.h>
#include <algorithm>

#pragma warning(disable:4312)

using namespace std;

#define MAX_BLOCK_SIZE      256*256
#define INVALID_HANDLE_TEX	((LPTexture)0xFFFFFFFF)
#define NULL_AND_DONT_DOWNLOAD	((LPTexture)0xFFFFFFFE)
#define NULL_DOWNLOADING	((LPTexture)0xFFFFFFFD)


CTexPool					CTexManager::m_TexMemPool(FALSE);
CTexManager::MString		CTexManager::m_mSglName;		// sgl名称表
CTexManager::stPQueue		CTexManager::m_pqWillRead;		// 将要读取
CTexManager::VReaded		CTexManager::m_vReaded;			// 已经读取
CTexManager::MAllTex		CTexManager::m_mAllTex;
CTexManager::TexLufoList	CTexManager::m_LufoList;

__int64						CTexManager::m_iCurCacheSize	= 0;
__int64						CTexManager::m_iMaxCacheSize	= 100 * 1024 * 1024;
__int64                     CTexManager::m_iFreeCacheSize   = 0;
HANDLE						CTexManager::m_hThread			= NULL;
DWORD						CTexManager::m_dwThreadID		= 0;
BOOL						CTexManager::m_bExited			= FALSE;
CRITICAL_SECTION			CTexManager::m_AskCriSect;
CRITICAL_SECTION			CTexManager::m_FeedCriSect;


// 纹理读取线程
DWORD WINAPI CTexManager::ThreadLoadTex(LPVOID lpData)
{
	while(TRUE)
	{
		if(m_bExited)
			break;

		if(m_pqWillRead.size() <= 0)
		{
			Sleep(2);
			continue;
		}

		// 取出将要读取纹理的信息
		EnterCriticalSection(&m_AskCriSect);
		DWORD dwID = -1;
		eReadPrior ep = EP_NORMAL;
		m_pqWillRead.pop(dwID,ep);
		LeaveCriticalSection(&m_AskCriSect);

		if(dwID == -1 || dwID == 0)
			continue;

		// 读取
		LPTexture pTex = Inter_LoadTex(dwID,TRUE,ep);
		if(pTex)
			pTex->SetID(dwID);

		// 更新已经读取纹理
		CLock lock(&m_FeedCriSect);
		m_vReaded.push_back(stReaded(dwID,pTex));
	}

	return 0;
}

CTexManager::CTexManager(void)
{
#define	PACKAGE_BEGIN
#define	PACKAGE(lib,num)	m_mSglName[num] = #lib;
#define	PACKAGE_END
#include "SglDefine.inl"

	m_eReadResult	= RR_READING;
	m_iCurFixedSize = 0;
	m_iMaxFixedSize	= 0;


	InitializeCriticalSection(&m_AskCriSect);
	InitializeCriticalSection(&m_FeedCriSect);

	m_bExited = FALSE;
	m_hThread = CreateThread(NULL,0,ThreadLoadTex,NULL,0,&m_dwThreadID);
	SetThreadPriority(m_hThread,THREAD_PRIORITY_ABOVE_NORMAL);

	// 读取sgl文件
	//Init("../data");  //读取sgl文件
}

// 初始化所有sgl文件
/*
bool CTexManager::Init(const char* strDataDir)
{
	std::string path = strDataDir;

	std::replace(path.begin(),path.end(),'\\','/');
	if(path.size() > 0 && path[path.size() - 1] == '/')
		path = path.substr(0,path.size()-1);

	if(m_strDataDir.compare(path) == 0)  //已经设置了，不需要重复打开
		return true;

	m_strDataDir = path;

	while(!m_mSglFile.empty())
	{
		MSglFile::iterator itr1 = m_mSglFile.begin();

		if(itr1->second.bDirType == FALSE)
		{
			if(itr1->second.hFile != INVALID_HANDLE_VALUE)
				CloseHandle(itr1->second.hFile);
			SAFE_DELETE_ARRAY(itr1->second.vFileOff);
			SAFE_DELETE_ARRAY(itr1->second.vFileSize);
		}
		m_mSglFile.erase(itr1);
	}

	MString::iterator itr;
	for(itr = m_mSglName.begin(); itr != m_mSglName.end();itr++)
	{
		UpdateSglFile(itr->first);
	}

	//清除掉原来的图片
	PopThreadReaded();// 取出线程刚刚读取的纹理
	for(MAllTex::iterator itr2 = m_mAllTex.begin();itr2 != m_mAllTex.end();++itr2)
	{
		if(itr2->second && INVALID_HANDLE_TEX != itr2->second)
		{
			m_iCurChacheSize -= itr2->second->GetPixelSize();
			ReleaseTex(itr2->second);
		}
	}
	return true;
}

// 更新sgl图库信息
BOOL CTexManager::UpdateSglFile(int iPack)
{
	MSglFile::iterator itr = m_mSglFile.find(iPack);
	if(itr == m_mSglFile.end())	// 还没有读取过
	{
		MString::iterator it = m_mSglName.find(iPack);
		if(it == m_mSglName.end())
			return FALSE;

		stSglWoool sgl;
		if(it->second.size() >= sizeof(sgl.sPath))
			return FALSE;

		strcpy(sgl.sName,it->second.c_str());
		strlwr(sgl.sName);
		sprintf(sgl.sPath,"%s/%s.sgl",m_strDataDir.c_str(),sgl.sName);
		strlwr(sgl.sPath);
		sgl.hFile = CreateFile(sgl.sPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);
		if(sgl.hFile == INVALID_HANDLE_VALUE)
		{
			sprintf(sgl.sPath,"%s/%s",m_strDataDir.c_str(),it->second.c_str());
			strlwr(sgl.sPath);

			sgl.bDirType	= TRUE;
			sgl.fp1			= NULL;
			sgl.fp2			= NULL;
			sgl.dwFiles		= 0;
			sgl.vFileOff	= NULL;
		}
		else
		{
			sgl.bDirType	= FALSE;
			sgl.fp1			= NULL;
			sgl.fp2			= NULL;
			sgl.dwFiles		= 0;
			sgl.vFileOff	= NULL;

			sgl.fp1 = fopen(sgl.sPath,"rb");
			sgl.fp2 = fopen(sgl.sPath,"rb");
			if(sgl.fp2)
			{
				DWORD dwOff = 0;
				fseek(sgl.fp2,36,SEEK_SET);
				fread(&dwOff,4,1,sgl.fp2);
				fseek(sgl.fp2,dwOff,SEEK_SET);
				fread(&dwOff,4,1,sgl.fp2);
				if(dwOff > 0x0000FFFF)
				{
					CloseHandle(sgl.hFile);
					return FALSE;
				}

				sgl.vFileOff = new DWORD[dwOff+1];
				sgl.vFileSize = new DWORD[dwOff];
				memset(sgl.vFileOff,0,dwOff*sizeof(DWORD));
				memset(sgl.vFileSize,0,dwOff * sizeof(DWORD));

				if(fread(sgl.vFileOff,4,dwOff,sgl.fp2) != dwOff)
				{
					SAFE_DELETE_ARRAY(sgl.vFileOff);
					SAFE_DELETE_ARRAY(sgl.vFileSize);
					CloseHandle(sgl.hFile);
					return FALSE;
				}
				sgl.dwFiles = dwOff;

				//移动到尾部
				fseek(sgl.fp2,0,SEEK_END);
				sgl.vFileOff[dwOff] = ftell(sgl.fp2);

				//计算每个文件的长度
				for(unsigned int ui= 0;ui < dwOff;ui++)
				{
					if(sgl.vFileOff[ui] == 0)
					{
						sgl.vFileSize[ui] = 0;
						continue;
					}

					for(unsigned int uj = ui+1; uj< dwOff+1;uj++)
					{
						if(sgl.vFileOff[uj] == 0)
							continue;

						if(sgl.vFileOff[uj] >= sgl.vFileOff[ui])
						{
							sgl.vFileSize[ui] = sgl.vFileOff[uj] - sgl.vFileOff[ui];
							break;
						}
					}
				}
			}
		}
		m_mSglFile[iPack] = sgl;
	}
	return TRUE;
}
*/

CTexManager::~CTexManager(void)
{
	if(m_hThread)
	{
		m_bExited = TRUE;
		WaitForSingleObject(m_hThread,INFINITE);
		CloseHandle(m_hThread);
	}

	// 释放所有sgl文件
	/*
	while(!m_mSglFile.empty())
	{
		MSglFile::iterator itr1 = m_mSglFile.begin();

		if(itr1->second.bDirType == FALSE)
		{
			if(itr1->second.hFile != INVALID_HANDLE_VALUE)
				CloseHandle(itr1->second.hFile);
			SAFE_DELETE_ARRAY(itr1->second.vFileOff);
			SAFE_DELETE_ARRAY(itr1->second.vFileSize);
		}
		m_mSglFile.erase(itr1);
	}

	PopThreadReaded();// 取出线程刚刚读取的纹理
	for(MAllTex::iterator itr2 = m_mAllTex.begin();itr2 != m_mAllTex.end();++itr2)
	{
		if(itr2->second && INVALID_HANDLE_TEX != itr2->second)
		{
			m_iCurChacheSize -= itr2->second->GetPixelSize();
			ReleaseTex(itr2->second);
		}
	}*/

	//PopThreadReaded();// 取出线程刚刚读取的纹理

	//EnterCriticalSection(&m_FeedCriSect);
	//for(MAllTex::iterator itr2 = m_mAllTex.begin();itr2 != m_mAllTex.end();++itr2)
	//{
	//	if(itr2->second && INVALID_HANDLE_TEX != itr2->second)
	//	{
	//		m_iCurCacheSize -= itr2->second->GetPixelSize();
	//		Inter_ReleaseTex(itr2->second);
	//	}
	//}
	//LeaveCriticalSection(&m_FeedCriSect);

	ClearAllTex();

	DeleteCriticalSection(&m_AskCriSect);
	DeleteCriticalSection(&m_FeedCriSect);

	if(m_iCurCacheSize != 0)
		OutputDebugString("TexMem leak!\n");
}

// 取出线程读取的
void CTexManager::PopThreadReaded(void)
{
	if (m_vReaded.size() > 0)
	{
		CLock lock(&m_FeedCriSect);
		while(m_vReaded.size() > 0)
		{
			LPTexture pTex = m_vReaded.back().pTex;
			LPTexture &pOldTex = m_mAllTex[m_vReaded.back().dwID];
			if(pTex)
			{
				//已经在缓冲区中存在了的
				if(pOldTex && pOldTex != INVALID_HANDLE_TEX && pOldTex != NULL_AND_DONT_DOWNLOAD && pOldTex != NULL_DOWNLOADING)
				{
					if (pTex != NULL_DOWNLOADING)
					{
						Inter_ReleaseTex(pTex);
					}
				}
				else if (pTex == NULL_DOWNLOADING)
				{
					pOldTex = pTex;
				}
				else
				{
					m_iCurCacheSize += pTex->GetPixelSize();
					pOldTex = pTex;
					m_LufoList.push(pTex);
				}
			}
			else
			{
				if(pOldTex == NULL)
					pOldTex = INVALID_HANDLE_TEX;
			}
			m_vReaded.pop_back();
		}
	}
}

void  CTexManager::OnDownloadTex(DWORD dwID,const char* buf,DWORD dwLen)
{
	DataStreamInterface* stream = new CMemoryStream((void*)buf,dwLen);
	if(stream)
	{
		LPTexture pTex = Inter_ReadTex(*stream);
		if(pTex)
		{
			pTex->SetID(dwID);
			//// 更新已经读取纹理
			//CLock lock(&m_FeedCriSect);
			////已经在缓冲区中存在了的
			//LPTexture &pOldTex = m_mAllTex[dwID];
			//if(pOldTex && pOldTex != INVALID_HANDLE_TEX && pOldTex != NULL_AND_DONT_DOWNLOAD && pOldTex != NULL_DOWNLOADING)
			//{
			//	Inter_ReleaseTex(pTex);
			//}
			//else
			//{
			//	m_iCurCacheSize += pTex->GetPixelSize();
			//	pOldTex = pTex;
			//	m_LufoList.push(pTex);
			//}

			//不要直接更新,不然要会导致主线程要lock m_mAllTex以及m_LufoList,性能受影响

			// 更新已经读取纹理
			CLock lock(&m_FeedCriSect);
			m_vReaded.push_back(stReaded(dwID,pTex));
		}

		SAFE_DELETE(stream);
	}
}

void  CTexManager::OnSendDownloadTex(DWORD dwID)
{
	CLock lock(&m_FeedCriSect);
	m_vReaded.push_back(stReaded(dwID,NULL_DOWNLOADING));
}

// 内部根据文件指针读取纹理函数
LPTexture CTexManager::Inter_ReadTex(DataStreamInterface& stream)
{
	LPTexture pTex = NULL;

	// 魔数
	TexData tex;
	memset(&tex,0,sizeof(TexData));

	stream.read(&tex.dwMagic,4);
	//不是新格式图片，就当作老格式图片读取
	if(tex.dwMagic != 'TEX1') 
	{
		pTex = Inter_ReadOldTex(stream,tex.dwMagic);
		return pTex;
	}

	pTex = new CTextureD3D;
	if(!pTex)
		return pTex;

	// 读取tex头
	DWORD	dwSize;
	dwSize = sizeof(TexData) - 8;
	stream.read(&tex.dwChildOff,4);
	dwSize = min(dwSize,tex.dwChildOff);
	stream.read(((BYTE *)&tex) + 8,dwSize);

	if(tex.dwFrames == 0 || tex.dwFrames > 256)
		return pTex;

	pTex->SetOption(tex.dwOption);
	pTex->SetFrames(tex.dwFrames);
	pTex->SetSpeed(tex.dwSpeed);

	// 角色图片，非系统控制帧速
	if(tex.dwOption & 0x00000001)
		pTex->EnableSysAnim(FALSE);

	FrameData framdata;
	BlockData blockdata;
	for(DWORD i = 0;i < tex.dwFrames;i++)
	{
		memset(&framdata,0,sizeof(FrameData));
		stream.read(&framdata,tex.dwFrameSize);

		CTexFrame& frame = *(pTex->GetFrame(i));

		frame.wWidth0	= framdata.wWidth0;
		frame.wHeight0	= framdata.wHeight0;
		frame.wWidth	= framdata.wWidth;
		frame.wHeight	= framdata.wHeight;
		frame.wOffX		= framdata.wOffX;
		frame.wOffY		= framdata.wOffY;
		frame.wCenterX	= framdata.wCenterX;
		frame.wCenterY	= framdata.wCenterY;
		frame.wXBlocks	= framdata.wXBlocks;
		frame.wYBlocks	= framdata.wYBlocks;

		dwSize = frame.wXBlocks * frame.wYBlocks;

		if(dwSize > 0)
		{
			frame.vBlocks	= new CTexBlock[dwSize];
			if(!frame.vBlocks)
				return pTex;
		}
		else
			frame.vBlocks = NULL;

		for(DWORD j = 0;j < dwSize;j++)
		{
			memset(&blockdata,0,sizeof(BlockData));
			stream.read(&blockdata,tex.dwBlockSize);

			CTexBlock& block = frame.vBlocks[j];

			block.bCompr		= blockdata.bCompr;
			block.ePixelType	= blockdata.ePixelType;
			block.wReserved		= blockdata.wReserved;
			block.wWidth		= blockdata.wWidth;
			block.wHeight		= blockdata.wHeight;
			block.dwSize		= blockdata.dwSize;
			if(block.dwSize > 0)
			{
				block.pData	= m_TexMemPool.NewTexMem(blockdata.dwSize);
				if(!block.pData)
					return pTex;

				stream.read(block.pData,blockdata.dwSize);
				pTex->SetPixelSize(pTex->GetPixelSize() + block.dwSize);

				//字体缓冲都是A4R4G4B4
				if(block.ePixelType == TPT_A4R4G4B4)
				{
					FontTexSizeType ePool = PoolData::GetFontPool(block.wWidth,block.wHeight);
					if(ePool != FTST_NULL)
						block.pPoolData	= &(s_FontPool[ePool]);
					else
						block.pPoolData	= NULL;
				}
				else if(block.ePixelType == TPT_A8R8G8B8)
				{
					//TexSizeType ePool = PoolData::GetA32Pool(block.wWidth,block.wHeight);
					//if(ePool != TEX_POOL_NULL)
					//	block.pPoolData = &(s_A32Pool[ePool]);
					//else
					//	block.pPoolData = NULL;
#ifdef _DEBUG
					::MessageBox(::GetActiveWindow(),"存在32位的图片,请检查","提示",MB_OK);
#endif
				}


				//从图片纹理缓冲找
				if(block.pPoolData == NULL)
				{
					TexSizeType ePool = PoolData::GetTexPool(block.wWidth,block.wHeight);
					if(ePool != TEX_POOL_NULL)
						block.pPoolData	= &(s_TexPool[ePool]);
					else
						block.pPoolData	= NULL;
				}
			}
		}
	}
	pTex->SetCurFrame(0);
	return pTex;
}

LPTexture CTexManager::Inter_ReadOldTex(DataStreamInterface& stream,DWORD dwMagic)
{
	// 查看文件是否存在
	LPTexture pTex = NULL;

	BYTE  cVersion = 0;
	BYTE  cOption  = 0;
	WORD  wFrames  = 0;
	WORD  wSpeed   = 0;
	DWORD dwSize = 0;

	if(dwMagic == 'TEX1')
	{
		stream.read(&cVersion,1);
		stream.read(&cOption,1);
		stream.read(&wFrames,2);
	}
	else
	{
		cVersion = (BYTE)(dwMagic & 0x000000FF);
		cOption  = (BYTE)((dwMagic & 0x0000FF00) >> 8);
		wFrames  = (WORD)((dwMagic & 0xFFFF0000) >> 16);
	}
	stream.read(&wSpeed,2);

	if(cVersion != 0x03)
		return pTex;

	const BYTE  cSupport = TEX_RLE | TEX_4444 | TEX_S3TC | TEX_S3TCALPHA;

	//不能处理的图片
	if((cOption & ~cSupport) != 0) 
		return pTex;

	pTex = new CTextureD3D;
	if(!pTex)
		return pTex;

	if(wFrames == 0 || wFrames > 256)
		return pTex;

	pTex->SetOption(0);
	pTex->SetFrames(wFrames);
	pTex->SetSpeed(wSpeed);

	for(DWORD i = 0;i < wFrames;i++)
	{
		BYTE	byXBlocks,byYBlocks;
		WORD	wWidth,wHeight,wOffX,wOffY;

		stream.read(&wWidth,2);
		stream.read(&wHeight,2);
		stream.read(&wOffX,2);
		stream.read(&wOffY,2);
		stream.read(&byXBlocks,1);
		stream.read(&byYBlocks,1);

		//if(wWidth == 0 || wHeight == 0 || byXBlocks > 16 || byYBlocks > 16)
		if(byXBlocks > 16 || byYBlocks > 16)//序列帧里可能有空的
		{
			return pTex;
		}

		CTexFrame& frame = *(pTex->GetFrame(i));

		frame.wWidth0	= wWidth;
		frame.wHeight0	= wHeight;

		// 因为顶点纹理映射精度的问题，所有图像必须能被8整除
		if( (wWidth % 256) < 128 )
			wWidth  = (int)((wWidth  +  7)/ 8) * 8;
		else
			wWidth  = (int)((wWidth  + 15)/16) * 16;

		if( (wHeight%256) < 128 )
			wHeight = (int)((wHeight +  7)/ 8) * 8;
		else
			wHeight = (int)((wHeight + 15)/16) * 16;

		frame.wWidth	= wWidth;
		frame.wHeight	= wHeight;

		frame.wOffX		= wOffX;
		frame.wOffY		= wOffY;
		frame.wCenterX	= 0;
		frame.wCenterY	= 0;
		frame.wXBlocks	= byXBlocks;
		frame.wYBlocks	= byYBlocks;

		dwSize = frame.wXBlocks * frame.wYBlocks;
		frame.vBlocks	= new CTexBlock[dwSize];
		if(!frame.vBlocks)
			return pTex;

		for(DWORD j = 0;j < dwSize;j++)
		{
			WORD byW = 0,byH = 0;
			A4R4G4B4 pSrc[MAX_BLOCK_SIZE];
			stream.read(&byW,1);
			byW++;
			stream.read(&byH,1);
			byH++;

			if(cOption & TEX_RLE)  //RLE压缩
			{
				memset(pSrc,0,byW * byH * 2);

				A4R4G4B4 buf0[MAX_BLOCK_SIZE];
				memset(buf0,0,sizeof(buf0));

				DWORD  dwBlockSize = 0;
				stream.read(&dwBlockSize,sizeof(DWORD));
				if(dwBlockSize > sizeof(buf0)) //异常数据
					return pTex;

				stream.read(buf0,dwBlockSize);

				Apply_RLE((BYTE*)pSrc,(BYTE*)buf0,dwBlockSize); //RLE压缩
			}
			else if(cOption & TEX_S3TC)
			{
				memset(pSrc,0,byW * byH * 2);

				A4R4G4B4 buf1[MAX_BLOCK_SIZE];
				memset(buf1,0,sizeof(buf1));

				DWORD  dwBlockSize = 0;
				stream.read(&dwBlockSize,sizeof(DWORD));
				stream.read(buf1,dwBlockSize);

				Apply_S3TC((BYTE*)pSrc,(BYTE*)buf1,dwBlockSize,byW); //解出S3TC的图片
			}
			else
			{
				stream.read(pSrc,byW * byH * 2);
			}

			if(cOption & TEX_S3TCALPHA) //S3TC同时带Alpha,用于物体
			{
				BYTE buf2[MAX_BLOCK_SIZE];
				memset(buf2,0,sizeof(buf2));

				DWORD dwAlphaSize = 0;
				stream.read(&dwAlphaSize,sizeof(DWORD));
				if(dwAlphaSize > MAX_BLOCK_SIZE) //异常数据
					return pTex;
				stream.read(buf2,dwAlphaSize);

				Apply_Alpha((WORD*)pSrc,buf2,dwAlphaSize);
			}

			CTexBlock& block = frame.vBlocks[j];

			block.bCompr		= FALSE;
			block.ePixelType	= TPT_A4R4G4B4;
			block.wReserved		= 0;
			block.wWidth		= byW;
			block.wHeight		= byH;
			block.dwSize		= byW * byH * 2;
			if(block.dwSize > 0)
			{
				block.pData	= m_TexMemPool.NewTexMem(block.dwSize);
				if(!block.pData)
					return pTex;

				memcpy(block.pData,pSrc,block.dwSize);
				pTex->SetPixelSize(pTex->GetPixelSize() + block.dwSize);

				//从老图片缓冲找
				OldTexSizeType ePool = PoolData::GetOldPool(block.wWidth,block.wHeight);
				if(ePool != OLD_POOL_NULL)
					block.pPoolData	= &(s_OldPool[ePool]);
				else
					block.pPoolData	= NULL;

				//从字体缓冲区找
				if(block.pPoolData == NULL)
				{
					FontTexSizeType ePool = PoolData::GetFontPool(block.wWidth,block.wHeight);
					if(ePool != FTST_NULL)
						block.pPoolData	= &(s_FontPool[ePool]);
					else
						block.pPoolData	= NULL;
				}
			}
		}
	}
	pTex->SetCurFrame(0);

	return pTex;
}

// 内部根据ID纹理读取函数
LPTexture CTexManager::Inter_LoadTex(DWORD dwID,bool bThread,eReadPrior ePrior)
{
	LPTexture	pTex	= NULL;
	int			iPack	= HIWORD(dwID);
	int			iIdx	= LOWORD(dwID);

	// 传世资源读取
	/*
	MSglFile::iterator itr = m_mSglFile.find(iPack);
	if(itr == m_mSglFile.end())
		return pTex;

	DataStream* stream = NULL;
	DWORD   dwSize = 0;
	stSglWoool&	sgl = itr->second;

	if(sgl.bDirType)
	{
		char sTexFile[256];
		sprintf(sTexFile,"%s\\%03d\\%05d.tex",sgl.sName,iIdx/100,iIdx);

		stream = g_StreamMgr.Open(sTexFile,bThread); //在打包文件中没有这个文件

		if(!stream)
		{
			sprintf(sTexFile,"%s/%03d/%05d.tex",sgl.sPath,iIdx/100,iIdx);

			FILE* fp = fopen(sTexFile,"rb");
			if(fp)
			{
				try
				{
					stream = new CFileStream(fp,true);
				}
				catch(...)
				{
					fclose(fp);
					return pTex;
				}
			}
		}
	}
	else
	{
		if(bThread)
		{
			if(sgl.fp2 && sgl.vFileOff && iIdx < (int)sgl.dwFiles && sgl.vFileOff[iIdx] > sizeof(TexData))
			{
				try
				{
					stream = new CFileStream(sgl.fp2,sgl.vFileOff[iIdx],sgl.vFileSize[iIdx]);
				}
				catch(...)
				{
					return pTex;
				}
			}
		}
		else
		{
			if(sgl.fp1 && sgl.vFileOff && iIdx < (int)sgl.dwFiles && sgl.vFileOff[iIdx] > sizeof(TexData))
			{
				try
				{
					stream = new CFileStream(sgl.fp1,sgl.vFileOff[iIdx],sgl.vFileSize[iIdx]);
				}
				catch(...)
				{
					return pTex;
				}
			}
		}
	}

	if(stream)
	{
		pTex = Inter_ReadTex(*stream);
		SAFE_DELETE(stream);
	}

	if (pTex && (pTex->GetOption() & OF_RAND_PLAY))
	{
		DWORD dwFrames = pTex->GetFrames();
		DWORD dwCurFrames = (rand() % dwFrames);
		pTex->SetCurFrame(dwCurFrames);
	}*/

	// 无双资源读取
	/////////////////////////

	MString::iterator itr = m_mSglName.find(iPack);
	if(itr == m_mSglName.end())
		return pTex;

	DataStreamInterface* stream = NULL;
	DWORD   dwSize = 0;

	char sTexFile[256];
	sprintf(sTexFile,"%s\\%03d\\%05d.tex",itr->second.c_str(),iIdx/100,iIdx);
	stream = g_pEStreamMgr->OpenTextureFile(sTexFile,bThread,false,dwID,ePrior); 

	if(stream)
	{
		pTex = Inter_ReadTex(*stream);
		SAFE_DELETE(stream);
	}

	return pTex;
}

bool CTexManager::HasTexInServer(DWORD dwID)
{
	int			iPack	= HIWORD(dwID);
	int			iIdx	= LOWORD(dwID);

	return HasTexInServer(iPack,iIdx); 
}

bool CTexManager::HasTexInServer(int iPack,int iIdx)
{
	MString::iterator itr = m_mSglName.find(iPack);
	if(itr == m_mSglName.end())
		return false;

	char sFindTexFile[256];
	sprintf(sFindTexFile,"%s\\%03d\\%05d.tex",itr->second.c_str(),iIdx/100,iIdx);
	return g_pEStreamMgr->HasFileInServer(sFindTexFile); 
}

bool CTexManager::GetTexFilePath(int iPack,int iIdx,char *szPath)
{
	if(!szPath)
		return false;

	MString::iterator itr = m_mSglName.find(iPack);
	if(itr == m_mSglName.end())
		return false;

	sprintf(szPath,"%s\\%03d\\%05d.tex",itr->second.c_str(),iIdx/100,iIdx);
	return true; 
}

bool CTexManager::HasTexInLocalWpf(DWORD dwID)
{
	int			iPack	= HIWORD(dwID);
	int			iIdx	= LOWORD(dwID);

	return HasTexInLocalWpf(iPack,iIdx); 
}

bool CTexManager::HasTexInLocalWpf(int iPack,int iIdx)
{
	MString::iterator itr = m_mSglName.find(iPack);
	if(itr == m_mSglName.end())
		return false;

	char sFindTexFile[256];
	sprintf(sFindTexFile,"%s\\%03d\\%05d.tex",itr->second.c_str(),iIdx/100,iIdx);
	return g_pEStreamMgr->HasFileInLocalWpf(sFindTexFile); 
}


// 内部释放纹理函数
void CTexManager::Inter_ReleaseTex(LPTexture& pTex)
{
	if(!pTex)
		return;

	if(pTex == INVALID_HANDLE_TEX || pTex == NULL_AND_DONT_DOWNLOAD || pTex == NULL_DOWNLOADING)
	{
		pTex = NULL;
		return;
	}

	for(DWORD j = 0;j < pTex->GetFrames();j++)
	{
		for(int i = 0;i < pTex->GetFrame(j)->wXBlocks * pTex->GetFrame(j)->wYBlocks;i++)
		{
			CTexBlock& block = pTex->GetFrame(j)->vBlocks[i];

			if(block.pData)
			{
				m_TexMemPool.DelTexMen(block.pData);
				block.pData = NULL;
			}
		}
		SAFE_DELETE_ARRAY(pTex->GetFrame(j)->vBlocks);
	}
	pTex->ClearFrame();
	SAFE_DELETE(pTex);
}

// 导入其他格式文件，使用FreeImage
LPTexture CTexManager::LoadImage(const char* path)
{
	LPTexture pTex = NULL;
	return pTex;
}

//// 根据文件句柄读取纹理
//LPTexture CTexManager::LoadTex(FILE* fp)
//{
//	LPTexture pTex = NULL;
//	if(!fp)
//		return pTex;
//
//	CFileStream* stream = NULL;
//	try
//	{
//		stream = new CFileStream(fp);
//	}
//	catch(...)
//	{
//		return pTex;
//	}
//
//	pTex = Inter_ReadTex(*stream);
//
//	SAFE_DELETE(stream);
//
//	if(pTex)
//	{
//		pTex->m_dwID = 0;
//		m_iCurFixedSize += pTex->GetPixelSize();
//		if(m_iCurFixedSize > m_iMaxFixedSize)
//			m_iMaxFixedSize = m_iCurFixedSize;
//	}
//	return pTex;
//}

// 根据文件名读取纹理,不从图包中读取,而是从磁盘文件中读取
LPTexture CTexManager::LoadTexFromDiskFile(LPCTSTR sTexFile,eReadPrior ePrior)
{
	LPTexture pTex = NULL;

	DataStreamInterface* stream = g_pEStreamMgr->OpenTextureFile(sTexFile,false,true,0,ePrior);
	if(stream)
	{
		pTex = Inter_ReadTex(*stream);

		SAFE_DELETE(stream);

		if(pTex)
		{
			pTex->SetID(0);
			m_iCurFixedSize += pTex->GetPixelSize();
			if(m_iCurFixedSize > m_iMaxFixedSize)
				m_iMaxFixedSize = m_iCurFixedSize;
		}
	}
	return pTex;
}
// 根据文件名称读取纹理函数
LPTexture CTexManager::LoadTex(LPCTSTR sTexFile,eReadPrior ePrior)
{
	LPTexture pTex = NULL;

	DataStreamInterface* stream = g_pEStreamMgr->OpenTextureFile(sTexFile,false,false,0,ePrior);
	if(stream)
	{
		pTex = Inter_ReadTex(*stream);

		SAFE_DELETE(stream);

		if(pTex)
		{
			pTex->SetID(0);
			m_iCurFixedSize += pTex->GetPixelSize();
			if(m_iCurFixedSize > m_iMaxFixedSize)
				m_iMaxFixedSize = m_iCurFixedSize;
		}
	}
	return pTex;
}
// 根据ID纹理读取函数
LPTexture CTexManager::LoadTex(DWORD dwID,eReadPrior ePrior)
{
	LPTexture pTex = Inter_LoadTex(dwID,FALSE,ePrior);
	if(pTex)
	{
		pTex->SetID(dwID);
		m_iCurFixedSize += pTex->GetPixelSize();
		if(m_iCurFixedSize > m_iMaxFixedSize)
			m_iMaxFixedSize = m_iCurFixedSize;
	}
	return pTex;
}

// 根据图包和编号读取函数
LPTexture CTexManager::LoadTex(int iPack,int iIdx,eReadPrior ePrior)
{
	return LoadTex(MAKELONG(iIdx,iPack),ePrior);
}

LPTexture CTexManager::LoadMemTex(int iW,int iH,BYTE* buf,int iPitch)
{
	LPTexture pTex = NULL;
	pTex = new CTextureD3D();

	if(!pTex->AddFrame(iW,iH,buf,iPitch))
	{
		ReleaseTex(pTex);
		return NULL;
	}
	return pTex;
}

// 释放纹理函数
void CTexManager::ReleaseTex(LPTexture& pTex)
{
	if(pTex && pTex != INVALID_HANDLE_TEX && pTex != NULL_AND_DONT_DOWNLOAD && pTex != NULL_DOWNLOADING)
		m_iCurFixedSize -= pTex->GetPixelSize();

	Inter_ReleaseTex(pTex);
}

// 使用最久未用淘汰算法
void CTexManager::WashOut(void)
{
	LPTexture pTex = NULL;

	while(m_iCurCacheSize > m_iMaxCacheSize)	// 已经到了最大缓冲了，使用最久未用淘汰
	{
		CLock lock(&m_FeedCriSect);
		if(m_LufoList.size() <= 0)
		{
			break;
		}

		pTex = m_LufoList.back();

		if(pTex && pTex != INVALID_HANDLE_TEX && pTex != NULL_AND_DONT_DOWNLOAD && pTex != NULL_DOWNLOADING)
		{
			m_mAllTex.erase(pTex->GetID());
			m_iCurCacheSize -= pTex->GetPixelSize();
		}

		m_LufoList.pop();
		Inter_ReleaseTex(pTex);
	}
}

// 缓冲模式读取纹理
LPTexture CTexManager::GetTex(int iPack,int iIdx,eReadPrior ePrior)
{
	return GetTexFromID(MAKELONG(iIdx,iPack),ePrior);
}

// 立即缓冲模式，根据图包和Idx读取
LPTexture CTexManager::GetTexImm(int iPack,int iIdx,eReadPrior ePrior,int *pIResult)
{
	return GetTexImm(MAKELONG(iIdx,iPack),ePrior,pIResult);
}

// 立即缓冲模式，根据ID读取
LPTexture CTexManager::GetTexImm(DWORD dwID,eReadPrior ePrior,int *pIResult)
{
	// 已经读取或者正在读取中
	PopThreadReaded();// 取出线程刚刚读取的纹理

	MAllTex::iterator itr = m_mAllTex.find(dwID);
	if(itr != m_mAllTex.end())
	{
		if(itr->second == INVALID_HANDLE_TEX)
		{
			m_eReadResult = RR_NOFILE;
			return NULL;
		}
		else if(itr->second == NULL_AND_DONT_DOWNLOAD)
		{
			if (ePrior == EP_DONT_DOWNLOAD)
			{
				m_eReadResult = RR_NOFILE;
				return NULL;
			}
			//else//如果不是EP_DONT_DOWNLOAD,要求重新读取
			//{

			//}
		}
		else if(itr->second == NULL_DOWNLOADING)//下载中
		{
			m_eReadResult = RR_DOWNLOADING;
			if(pIResult)
			{
				*pIResult = m_eReadResult;
			}
			return NULL;
		}
		else if(itr->second)
		{
			m_LufoList.reflesh(itr->second);
			m_eReadResult = RR_SUCCESS;
			return itr->second;
		}
	}

	// 立即读取
	LPTexture pTex = LoadTex(dwID,ePrior);
	if(pTex)
	{
		m_iCurCacheSize += pTex->GetPixelSize();
		m_mAllTex[dwID] = pTex;
		m_LufoList.push(pTex);
		m_eReadResult = RR_SUCCESS;
	}
	else
	{
		if (ePrior == EP_DONT_DOWNLOAD)
		{
			m_mAllTex[dwID] = NULL_AND_DONT_DOWNLOAD;
		}
		else
		{	
			m_mAllTex[dwID] = INVALID_HANDLE_TEX;
		}
		m_eReadResult = RR_NOFILE;
	}

	return pTex;
}

// 取得纹理(缓冲池模式，不需要主动删除的)
LPTexture CTexManager::GetTexFromID(DWORD dwID,eReadPrior ePrior,int *pIResult)
{
	// 非法的ID
	if(dwID == -1)
	{
		m_eReadResult = RR_NOFILE;
		return NULL;
	}

	PopThreadReaded();// 取出线程读取的

	// 已经读取或者正在读取中
	MAllTex::iterator itr = m_mAllTex.find(dwID);
	if(itr != m_mAllTex.end())
	{
		if(itr->second)
		{
			if(itr->second == INVALID_HANDLE_TEX)
			{
				m_eReadResult = RR_NOFILE;
				return NULL;
			}
			else if(itr->second == NULL_AND_DONT_DOWNLOAD)
			{
				if (ePrior == EP_DONT_DOWNLOAD)
				{
					m_eReadResult = RR_NOFILE;
					return NULL;
				}
				//else//如果不是EP_DONT_DOWNLOAD,要求重新读取
				//{

				//}
			}
			else if(itr->second == NULL_DOWNLOADING)//下载中
			{
				m_eReadResult = RR_DOWNLOADING;
				if(pIResult)
				{
					*pIResult = m_eReadResult;
				}
				return NULL;
			}
			else
			{
				m_LufoList.reflesh(itr->second);
				m_eReadResult = RR_SUCCESS;
				return itr->second;
			}
		}
		else
		{
			m_eReadResult = RR_READING;
			return itr->second;
		}
	}

	// 查看是否需要淘汰
	WashOut();

	// 放入线程读取队列，让线程去读取
	EnterCriticalSection(&m_AskCriSect);
	m_pqWillRead.push(dwID,ePrior);
	LeaveCriticalSection(&m_AskCriSect);

	if (ePrior == EP_DONT_DOWNLOAD)
	{
		m_mAllTex[dwID] = NULL_AND_DONT_DOWNLOAD;
	}
	else
	{
		m_mAllTex[dwID] = NULL;
	}

	m_eReadResult = RR_READING;
	return NULL;
}

// 预读纹理
void CTexManager::PreLoad(DWORD dwBeginID,DWORD dwEndID,eReadPrior ePrior)
{
	for(DWORD i = dwBeginID;i < dwEndID;i++)
		GetTexFromID(i,ePrior);
}

void CTexManager::PreLoadImm(DWORD dwBeginID,DWORD dwEndID,eReadPrior ePrior)
{
	for(DWORD i = dwBeginID;i < dwEndID;i++)
		GetTexImm(i,ePrior);
}

void CTexManager::PreLoad(int iPack,int iBeginID,int iEndID,eReadPrior ePrior)
{
	for(int i = iBeginID; i < iEndID;i++)
	{
		GetTexFromID(MAKELONG(i,iPack),ePrior);
	}
}

void CTexManager::PreLoadImm(int iPack,int iBeginID,int iEndID,eReadPrior ePrior)
{
	for(int i = iBeginID; i < iEndID;i++)
	{
		GetTexImm(MAKELONG(i,iPack),ePrior);
	}
}

void CTexManager::PreDownLoadTex(int iPack,int iBeginID,int iEndID,eReadPrior ePrior)
{
	if (GetTexImm(iPack,iBeginID,ePrior))
	{
		return;
	}

	MString::iterator itr = m_mSglName.find(iPack);
	if(itr == m_mSglName.end())
		return;
	
	g_pEStreamMgr->PreDownLoadTex(itr->second.c_str(),iPack,iBeginID,iEndID,ePrior);
}


void CTexManager::DelTexFile(int iPack,int iBeginID,int iEndID)
{
	for(int i = max(iBeginID,1); i < iEndID;i++)
	{
		MString::iterator itr = m_mSglName.find(iPack);
		if(itr == m_mSglName.end())
			return;
		
		char sTexFile[256];
		char szDestPath[256];
		//sprintf(sTexFile,"%s\\%s\\%03d\\%05d.tex",g_pCallBack->GetDataDir(),itr->second.c_str(),i/100,i);
		//DeleteFile(sTexFile);

		sprintf(sTexFile,"F:\\NewWoool\\Data\\%s\\%03d\\%05d.tex",itr->second.c_str(),i / 100,i);
		sprintf(szDestPath,"F:\\暂时不放的图\\%s\\%03d\\%05d.tex",itr->second.c_str(),i/100,i);//目录名
		string szDP = szDestPath;
		SHCreateDirectoryEx(NULL,szDP.substr(0,szDP.length() - 9).c_str(),NULL);
		CopyFile(sTexFile,szDestPath,TRUE);

	}
}

// 正在读取的纹理张数
int	CTexManager::GetReadingTexNum(void)
{
	return m_pqWillRead.size();
}

eReadResult CTexManager::GetLastReadStatus(void)
{
	return m_eReadResult;
}
void    CTexManager::SetMaxCacheSize(int i)
{
	m_iMaxCacheSize = i * 1024 * 1024;
	m_iFreeCacheSize = m_iMaxCacheSize / 4;
}

__int64 CTexManager::GetCurCacheSize()
{
	return m_iCurCacheSize;
}

__int64 CTexManager::GetMaxCacheSize()
{
	return m_iMaxCacheSize;
}

__int64 CTexManager::GetCurFixedSize()
{
	return m_iCurFixedSize;
}

__int64 CTexManager::GetMaxFixedSize()
{
	return m_iMaxFixedSize;
}

CTexPool&  CTexManager::GetTexMemPool()
{
	return m_TexMemPool;
}

void CTexManager::ClearAllTex()
{
	PopThreadReaded();// 取出线程刚刚读取的纹理

	for(MAllTex::iterator itr2 = m_mAllTex.begin();itr2 != m_mAllTex.end();++itr2)
	{
		if(itr2->second && INVALID_HANDLE_TEX != itr2->second && NULL_AND_DONT_DOWNLOAD != itr2->second && NULL_DOWNLOADING != itr2->second)
		{
			m_iCurCacheSize -= itr2->second->GetPixelSize();
			ReleaseTex(itr2->second);
		}
	}

	m_mAllTex.clear();
	while(m_LufoList.size())
	{
		m_LufoList.pop();
	}
}

void CTexManager::ClearAllNullTex()
{
	PopThreadReaded();// 取出线程刚刚读取的纹理

	for(MAllTex::iterator itr2 = m_mAllTex.begin();itr2 != m_mAllTex.end();)
	{
		if(!itr2->second || INVALID_HANDLE_TEX == itr2->second || NULL_AND_DONT_DOWNLOAD == itr2->second || NULL_DOWNLOADING == itr2->second)
		{
			itr2 = m_mAllTex.erase(itr2);
		}
		else
		{
			itr2 ++;
		}
	}
}

bool CTexManager::IsValidOldTex(UCHAR * pTex,int iLen)
{

#define ADDCOUNT(a)		count += a; if(count > iLen) return false;

	WORD w;
	short sw;
	WORD wFrames;
	UCHAR cProp,cPageX,cPageY;
	int i,x,y,bw,bh,count,j,k,a;
	DWORD dw;

	if( !pTex ) return false;

	// 版本号
	if( pTex[0] != 0x03 ) return false;	
	// 属性
	cProp = pTex[1];
	if( (cProp & 0x03) == 0 ) return false;							
	// 总帧数
	wFrames = *((WORD *)(pTex+2)); 
	if( wFrames<1 || wFrames>512 )	return false;
	// 动画速度
	w = *((WORD *)(pTex+4));
	if( w<0 || w>512 ) return false;

	// 实际读取
	count = 6;
	for(i=0;i<wFrames;i++)
	{
		// 图片宽度
		w = *((WORD *)(pTex+count));
		ADDCOUNT(2);
		if( w<1 || w>2048 ) return false;
		// 图片高度
		w = *((WORD *)(pTex+count));
		ADDCOUNT(2);
		if( w<1 || w>2048 ) return false;
		// X方向偏移量
		sw = *((short *)(pTex+count));
		ADDCOUNT(2);
		if( sw<-1024 || sw>1024 ) return false;
		// Y方向偏移量
		sw = *((short *)(pTex+count));
		ADDCOUNT(2);
		if( sw<-1024 || sw>1024 ) return false;	
		// X方向分割块数
		cPageX = pTex[count];
		ADDCOUNT(1);
		if( cPageX<1 || cPageX>8 ) return false;
		// Y方向分割块数
		cPageY = pTex[count];
		ADDCOUNT(1);
		if( cPageY<1 || cPageY>8 ) return false;

		for(y=0;y<cPageY;y++)
		{
			for(x=0;x<cPageX;x++)
			{
				bw = pTex[count] + 1; ADDCOUNT(1);
				bh = pTex[count] + 1; ADDCOUNT(1);

				if( cProp & TEX_RLE )
				{
					dw = *((DWORD *)(pTex+count));
					ADDCOUNT(4);
					if( dw > 256*256*4 ) return false;
					if( (dw+count) > iLen ) return false;

					j = 0;
					a = 0;
					while( j < dw )
					{
						if( pTex[count+j] & 0x80 )
						{
							// n 个象素不相同
							k = pTex[count+j] - 0x80;
							j += k*2 + 1;
						}
						else
						{
							// n 个象素相同
							k = pTex[count+j];
							j += 3;							
						}
						a += k;
					}

					if( a != (bw*bh) ) return false;
					ADDCOUNT(dw);
				}
				else if( cProp & TEX_S3TC )
				{
					dw = *((DWORD *)(pTex+count));
					ADDCOUNT(4);
					if( dw > 256*256*4 ) return false;
					if( (dw+count) > iLen ) return false;
					if( (float)dw != ((float)bw*bh*2/32*7) ) return false;
					ADDCOUNT(dw);
				}
				else
				{
					dw = bw * bh * 2;
					ADDCOUNT(dw);
				}

				if( (cProp&TEX_S3TC) && (cProp&TEX_S3TCALPHA) )
				{
					dw = *((DWORD *)(pTex+count));
					ADDCOUNT(4);
					if( dw > 256*256 ) return false;
					if( (dw+count) > iLen ) return false;

					j = 0;
					a = 0;
					while( j < dw )
					{
						if( pTex[count+j] & 0x80 )
						{
							// n 个象素不相同
							k = pTex[count+j] - 0x80;
							j += k + 1;
						}
						else
						{
							// n 个象素相同
							k = pTex[count+j];
							j += 2;							
						}
						a += k*2;
					}

					if( a != (bw*bh) ) return false;
					ADDCOUNT(dw);
				}
			}
		}
	}

	return true;
}
