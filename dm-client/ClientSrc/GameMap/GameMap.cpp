#include "GameMap.h"
#include "Global/Interface/TexManagerInterface.h"
#include "Global/Interface/FontInterface.h"
#include "Global/Interface/GraphicInterface.h"
#include "Global/Interface/StreamInterface.h"
#include "Global/Interface/AudioInterface.h"
#include "GameData/GameData.h"
#include "GameData/MapFinder.h"
#include "BaseClass/Control/Control.h"
#include "MapXmlMgr.h"
#include "MinMap.h"
//#include "WndClass/GameWnd/SysConfigWnd.h"
#include "GameData/ConfigData.h"
#include "GameControl/GameControl.h"


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

IMPLEMENT_POOL(STile2,20000,10000);

struct SInfoHeader
{
	DWORD dwSize;	
	int iTemplates;	
	int iBGLayers;
};


//背景相关数据
inline SMapTile * BGLayerData::GetTile(DWORD dwXY)
{
	int iTileX = LOWORD(dwXY);
	int iTileY = HIWORD(dwXY);

	return GetTile(iTileX,iTileY);
}

inline SMapTile* BGLayerData::GetTile(int iX,int iY)
{
	if(iX < 0 || iX >= iWidth || iY < 0 || iY >= iHeight)
		return NULL;

	return &pDataArray[iY*iWidth + iX];
}


//地图数据
CGameMap::CGameMap(void)
{
	m_iWidth		= 0;
	m_iOldWidth		= 0;
	m_iHeight		= 0;
	m_iOldHeight	= 0;
	m_pMap			= NULL;
	m_pOldMap		= NULL;
	//m_pMiniTex		= NULL;
	//m_pOldMiniTex	= NULL;
	m_dwMiniTexID    = 0;
	m_dwOldMiniTexID    = 0;
	m_bPreReadMap	= false;

	m_strMapTitle.clear();
	m_strMapFile.clear();
	m_strOldMapFile.clear();
	m_BgLayers.clear();

	m_dwVersion = 0;
	m_dwPassBlockType = 0;

	m_pDownloadMapDataBuf = 0;
	m_dwDownloadMapDataLen = 0;

	m_pDownloadBgMapDataBuf = 0;
	m_dwDownloadBgMapDataLen = 0;

	InitializeCriticalSection(&m_LoadMapCriSect);


	//载入地图影射关系文件
	char path[MAX_PATH] = {0};
	sprintf(path,"%s\\config\\maplist.xml",GetGameDataDir());

	g_MapXmlMgr.LoadFile(path);
	LoadDynObjData();

	m_bIsEmptyMap = false;
	m_bOldIsEmptyMap = false;
}

CGameMap::~CGameMap(void)
{
	SAFE_DELETE_ARRAY(m_pMap);
	SAFE_DELETE_ARRAY(m_pOldMap);

	SAFE_DELETE_ARRAY(m_pDownloadMapDataBuf);
	SAFE_DELETE_ARRAY(m_pDownloadBgMapDataBuf);

	DeleteCriticalSection(&m_LoadMapCriSect);

	while(!m_BgLayers.empty())
	{
		MBGLayer::iterator itr = m_BgLayers.begin();
		SAFE_DELETE_ARRAY(itr->second.pDataArray);
		m_BgLayers.erase(itr);
	}

	while(!m_OldBgLayers.empty())
	{
		MBGLayer::iterator itr = m_OldBgLayers.begin();
		SAFE_DELETE_ARRAY(itr->second.pDataArray);
		m_OldBgLayers.erase(itr);
	}

	m_MDynObj.clear();

	//g_pTexMgr->ReleaseTex(m_pMiniTex);
	//g_pTexMgr->ReleaseTex(m_pOldMiniTex);
}

void CGameMap::LoadDynMinMap()
{
	g_pGameData->LoadDynMiniMapData();
}

void CGameMap::LoadDynObjData()
{
	m_MDynObj.clear();
	ClearDynamicData();

	char path[MAX_PATH] = {0};

	sprintf(path,"%s\\config\\DynScence.dat",GetGameDataDir());

	FILE *fp = fopen(path,"rb");
	if(!fp)
		return;

	struct sDynObjFileHeader
	{
		DWORD dwMagic;
		DWORD dwCount;
		WORD  wVersion;
		WORD  wRev;
	};

	sDynObjFileHeader header;
	fread(&header,sizeof(header),1,fp);

	DWORD dwIDx = 0;
	BYTE byLeft,byRight,byTop,byBottom;
	WORD wTileCount;
	WORD wObjectH = 0;
	char cTemp;

	for(DWORD i = 0; i < header.dwCount; i++)
	{
		SDynamicObject obj;

		fread(&dwIDx,sizeof(DWORD),1,fp);
		fread(&byLeft,sizeof(BYTE),1,fp);
		fread(&byRight,sizeof(BYTE),1,fp);
		fread(&byTop,sizeof(BYTE),1,fp);
		fread(&byBottom,sizeof(BYTE),1,fp);
		fread(&wTileCount,sizeof(WORD),1,fp);

		for (int j = 0; j < wTileCount; j++)
		{
			SDynamicData dynData;

			if (header.wVersion >= 0x0003)
			{
				fread(&(dynData.sX),sizeof(short),1,fp);
				fread(&(dynData.sY),sizeof(short),1,fp);
			}
			else
			{
				fread(&cTemp,sizeof(BYTE),1,fp);
				dynData.sX = cTemp;
				fread(&cTemp,sizeof(BYTE),1,fp);
				dynData.sY = cTemp;
			}

			fread(&(dynData.dwType),sizeof(DWORD),1,fp);

			if (dynData.dwType & ATO_SMTILES)
			{
				fread(&(dynData.tile.dwSmTile),sizeof(WORD),1,fp);
			}
			if (dynData.dwType & ATO_TILES)
			{
				fread(&(dynData.tile.dwTile),sizeof(WORD),1,fp);
			}
			if (dynData.dwType & ATO_OBJECT)
			{
				fread(&(dynData.tile.uObject),sizeof(DWORD),1,fp);
				fread(&wObjectH,sizeof(WORD),1,fp);
			}
			if (dynData.dwType & ATO_SOUND)
			{
				fread(&(dynData.tile.bySound),sizeof(BYTE),1,fp);
			}
			if (dynData.dwType & ATO_UNDER_OBJ)
			{
				fread(&(dynData.tile.dwUnderObj),sizeof(DWORD),1,fp);
			}

			obj.vData.push_back(dynData);
		}

		m_MDynObj[dwIDx] = obj;
	}

	fclose(fp);

	for (int i = 1;i<13;i++)
	{
		m_MDynAppHide[i] = 0xFF;
	}
}
 
bool CGameMap::ReadMap(DataStreamInterface* stream,SMapData& info)
{
	if(stream == NULL)
		return false;

	DWORD dwHeaderSize = 0;
	DWORD dwVer = 0;
	int   iWidth = 0;
	int   iHeight = 0;

	stream->read(&dwHeaderSize,sizeof(DWORD));
	stream->read(&dwVer, sizeof(DWORD));
	stream->read(&iWidth, sizeof(DWORD));
	stream->read(&iHeight,sizeof(DWORD));
	stream->read(&info.dwOffset,sizeof(DWORD));

	info.dwVersion = dwVer;
	info.iWidth = iWidth;
	info.iHeight = iHeight;
	info.pDataArray = NULL;

	if(iWidth < 3 || iHeight < 3 || iWidth > 1200 || iHeight > 1200)
	{
		fatal_error("错误的地图尺寸！");
		return false;
	}

	TRY_BEGIN
		info.pDataArray = new SMapTile[iWidth * iHeight];
	TRY_END_DO_JUMP(
		SAFE_DELETE_ARRAY(info.pDataArray);
	    SaveException(__FILE__,__FUNCTION__,__LINE__,pSec,strAddMsg.c_str());
	    return false;
	)

	stream->seek(dwHeaderSize);

	SMapTile* pTile = info.pDataArray;
	BYTE cTemp = 0;
	WORD wExAttr = 0;
	WORD lwObjectH = 0;
	DWORD dwNoUse = 0;

	if(dwVer == 1)
	{
		for (int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&pTile->dwSmTile,sizeof(WORD));
				stream->read(&pTile->dwTile,sizeof(WORD));
				stream->read(&pTile->uObject,sizeof(UINT));
				stream->read(&cTemp,sizeof(BYTE));

				pTile->byAttr = 0;
				if(cTemp & 0x01)
					pTile->byAttr |= AT_USED;

				if(cTemp & 0x02)
					pTile->byAttr |= AT_BLOCK;

				stream->read(&pTile->bySound,sizeof(BYTE));
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&cTemp,sizeof(BYTE));
				pTile++;
			}
		}
	}
	else if(dwVer == 2)
	{
		for (int iY = 0;iY< iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				pTile->byAttr = AT_USED;
				if(cTemp & ATO_BLOCK)
					pTile->byAttr |= AT_BLOCK;
				if(cTemp & ATO_SMTILES)
					stream->read(&pTile->dwSmTile,sizeof(WORD));
				if(cTemp & ATO_TILES)
					stream->read(&pTile->dwTile,sizeof(WORD));
				if(cTemp & ATO_OBJECT)
					stream->read(&pTile->uObject,sizeof(UINT));
				pTile++;
			}
		}
	}
	else if(dwVer == 3)
	{
		for (int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));

				pTile->byAttr = AT_USED;
				if(cTemp & ATO_BLOCK)
					pTile->byAttr |= AT_BLOCK;

				if(cTemp & ATO_SMTILES)
					stream->read(&pTile->dwSmTile,sizeof(WORD));
				if(cTemp & ATO_TILES)
					stream->read(&pTile->dwTile,sizeof(WORD));
				if(cTemp & ATO_OBJECT)
					stream->read(&pTile->uObject,sizeof(DWORD));
				if(cTemp & ATO_SOUND)
					stream->read(&pTile->bySound,sizeof(BYTE));
				pTile++;
			}
		}
	}
	else if(dwVer == 5)
	{
		for (int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));

				pTile->byAttr = AT_USED;
				if(cTemp & ATO_BLOCK)
					pTile->byAttr |= AT_BLOCK;

				if(cTemp & AT_PHENIX_PASS_BLOCK)
					pTile->byAttr |= AT_PHENIX_PASS_BLOCK;

				if(cTemp & ATO_SMTILES)
					stream->read(&pTile->dwSmTile,sizeof(WORD));
				if(cTemp & ATO_TILES)
					stream->read(&pTile->dwTile,sizeof(WORD));
				if(cTemp & ATO_OBJECT)
				{
					stream->read(&pTile->uObject,sizeof(DWORD));
					stream->read(&lwObjectH,sizeof(WORD));
					if(lwObjectH == TILE_HEIGHT) 
					{
						pTile->byAttr |= AT_NEARGROUD;
					}
					else if ((pTile->uObject & (~OT_OBJECT_MASK)) == 0 && lwObjectH <= 0)//非法数据
					{
						pTile->uObject = 0;
					}
				}
				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)		pTile->dwSmTile |= (PACKAGE_SmTiles << 16);

				if(cTemp & ATO_SOUND)
					stream->read(&pTile->bySound,sizeof(BYTE));
				if(cTemp & ATO_UNDER_OBJ)
				{
					//stream->read(&pTile->dwUnderObj,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}

// 				if (pTile->byAttr & AT_NEARGROUD)
// 				{
// 					pTile->byAttr &= ~AT_NEARGROUD;
// 					pTile->uFloor = (pTile->uObject & OT_OBJECT_MASK);
// 					pTile->uObject = 0;					
// 				}

				pTile++;
			}
		}
	}
	else if(dwVer == 6)
	{
		for (int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&wExAttr,sizeof(WORD));

				if(cTemp & ATO_BLOCK)
					pTile->byAttr |= AT_BLOCK;

				if (cTemp & ATO_NEARGROUD)
					pTile->byAttr |= AT_NEARGROUD;

				if (cTemp & ATO_PHENIX_PASS_BLOCK)
					pTile->byAttr |= AT_PHENIX_PASS_BLOCK;

				if(cTemp & ATO_SMTILES)
					stream->read(&pTile->dwSmTile,sizeof(WORD));

				if(cTemp & ATO_TILES)
					stream->read(&pTile->dwTile,sizeof(WORD));

				if(cTemp & ATO_OBJECT)
				{
					stream->read(&pTile->uObject,sizeof(DWORD));
					stream->read(&lwObjectH,sizeof(WORD));
					if(lwObjectH == TILE_HEIGHT) 
					{
						pTile->byAttr |= AT_NEARGROUD;
					}
					else if ((pTile->uObject & (~OT_OBJECT_MASK)) == 0 && lwObjectH <= 0)//非法数据
					{
						pTile->uObject = 0;
					}
				}

				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)		pTile->dwSmTile |= (PACKAGE_SmTiles << 16);

				if(cTemp & ATO_SOUND)
					stream->read(&pTile->bySound,sizeof(BYTE));

				if(cTemp & ATO_UNDER_OBJ)					
				{
					//stream->read(&pTile->dwUnderObj,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}

				if (wExAttr & EXATO_OVER_OBJ)
				{
					//stream->read(&pTile->dwOverObj,sizeof(WORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(WORD));
				}

// 				if (pTile->byAttr & AT_NEARGROUD)
// 				{
// 					pTile->byAttr &= ~AT_NEARGROUD;
// 					pTile->uFloor = (pTile->uObject & OT_OBJECT_MASK);
// 					pTile->uObject = 0;
// 				}

				pTile++;
			}
		}
	}
	else if(dwVer == 7)
	{
		for(int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&wExAttr,sizeof(WORD));

				if(cTemp & ATO_BLOCK)				pTile->byAttr |= AT_BLOCK;
				if (cTemp & ATO_NEARGROUD)			pTile->byAttr |= AT_NEARGROUD;
				if (cTemp & ATO_PHENIX_PASS_BLOCK)	pTile->byAttr |= AT_PHENIX_PASS_BLOCK;
				if(cTemp & ATO_SMTILES)				stream->read(&pTile->dwSmTile,sizeof(DWORD));
				if(cTemp & ATO_TILES)				stream->read(&pTile->dwTile,sizeof(DWORD));
				if(cTemp & ATO_OBJECT)
				{
					stream->read(&pTile->uObject,sizeof(DWORD));
					stream->read(&lwObjectH,sizeof(WORD));
					if(lwObjectH == TILE_HEIGHT) 
					{
						pTile->byAttr |= AT_NEARGROUD;
					}
					else if ((pTile->uObject & (~OT_OBJECT_MASK)) == 0 && lwObjectH <= 0)//非法数据
					{
						pTile->uObject = 0;
					}
				}


				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)		pTile->dwSmTile |= (PACKAGE_SmTiles << 16);
				if(cTemp & ATO_SOUND)					stream->read(&pTile->bySound,sizeof(BYTE));
				if(cTemp & ATO_UNDER_OBJ)					
				{
					//stream->read(&pTile->dwUnderObj,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_OVER_OBJ)			
				{
					//stream->read(&pTile->dwOverObj,sizeof(WORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(WORD));
				}
				if (pTile->dwSmTile > 0)				pTile->dwSmTile |= (PACKAGE_SmTiles << 16);
				if (wExAttr & EXATO_COLORADJT)			
				{
					//stream->read(&pTile->dwTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJST)			
				{
					//stream->read(&pTile->dwSTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJOB)			
				{
					//stream->read(&pTile->dwObColor,sizeof(DWORD));//防止老地图这个地方有无效数据//有些地图dwObColor值有问题,比如skycity3的(146,396),所以暂时还是不用pTile->dwObColor
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJEFF)		
				{
					//stream->read(&pTile->dwUnderObjColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}

// 				if (pTile->byAttr & AT_NEARGROUD)
// 				{
// 					pTile->byAttr &= ~AT_NEARGROUD;
// 					pTile->uFloor = (pTile->uObject & OT_OBJECT_MASK);
// 					pTile->uObject = 0;
// 				}

				pTile++;
			}
		}
	}
	else if (dwVer == 8)
	{
		for(int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&wExAttr,sizeof(WORD));
				if(cTemp & ATO_BLOCK)				pTile->byAttr |= AT_BLOCK;
				//if (cTemp & ATO_NEARGROUD)			pTile->byAttr |= AT_NEARGROUD;//新版本如果有ATO_NEARGROUD,会写入uFloor
				if (cTemp & ATO_PHENIX_PASS_BLOCK)	pTile->byAttr |= AT_PHENIX_PASS_BLOCK;
				if (wExAttr & ATO_BLOCK1)			pTile->wExtraAttr |= ATO_BLOCK1;
				if (wExAttr & ATO_BLOCK2)			pTile->wExtraAttr |= ATO_BLOCK2;
				if (wExAttr & ATO_BLOCK3)			pTile->wExtraAttr |= ATO_BLOCK3;
				if(cTemp & ATO_SMTILES)				stream->read(&pTile->dwSmTile,sizeof(DWORD));
				if(cTemp & ATO_TILES)				stream->read(&pTile->dwTile,sizeof(DWORD));
				if(cTemp & ATO_OBJECT)
				{
					stream->read(&pTile->uObject,sizeof(DWORD));
					stream->read(&lwObjectH,sizeof(WORD));
					if(lwObjectH == TILE_HEIGHT) 
					{
						pTile->byAttr |= AT_NEARGROUD;
					}
					else if ((pTile->uObject & (~OT_OBJECT_MASK)) == 0 && lwObjectH <= 0)//非法数据
					{
						pTile->uObject = 0;
					}
				}
				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)		pTile->dwSmTile |= (PACKAGE_SmTiles << 16);
				if(cTemp & ATO_SOUND)					stream->read(&pTile->bySound,sizeof(BYTE));
				if(cTemp & ATO_UNDER_OBJ)					
				{
					//stream->read(&pTile->dwUnderObj,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_OVER_OBJ)
				{
					//stream->read(&pTile->dwOverObj,sizeof(WORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(WORD));
				}
				if (wExAttr & EXATO_COLORADJT)			
				{
					//stream->read(&pTile->dwTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJST)		
				{
					//stream->read(&pTile->dwSTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJOB)			
				{
					//stream->read(&pTile->dwObColor,sizeof(DWORD));//防止老地图这个地方有无效数据//有些地图dwObColor值有问题,比如skycity3的(146,396),所以暂时还是不用pTile->dwObColor
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJEFF)		
				{
					//stream->read(&pTile->dwUnderObjColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}


				if(cTemp & ATO_NEARGROUD)				stream->read(&pTile->uFloor,sizeof(DWORD));
				if(wExAttr & ATO_GROUP)
				{
					pTile->wExtraAttr |=  ATO_GROUP;
					DWORD dwGroupXY;
					stream->read(&dwGroupXY,sizeof(DWORD));

					WORD wX = LOWORD(dwGroupXY) + iX;
					WORD wY = HIWORD(dwGroupXY) + iY;
					dwGroupXY = MAKELONG(iX,iY);

					SMapTile* pTileEx = (info.pDataArray + (wY * iWidth) + wX);
					pTileEx->pTile2->vGroup.push_back(dwGroupXY);
				}
				pTile++;
			}
		}
	}
	else if (dwVer == 9)
	{
		for(int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&wExAttr,sizeof(WORD));
				if(cTemp & ATO_BLOCK)				pTile->byAttr |= AT_BLOCK;
				//if (cTemp & ATO_NEARGROUD)			pTile->byAttr |= AT_NEARGROUD;//新版本如果有ATO_NEARGROUD,会写入uFloor
				if (cTemp & ATO_PHENIX_PASS_BLOCK)	pTile->byAttr |= AT_PHENIX_PASS_BLOCK;
				if (wExAttr & ATO_BLOCK1)			pTile->wExtraAttr |= ATO_BLOCK1;
				if (wExAttr & ATO_BLOCK2)			pTile->wExtraAttr |= ATO_BLOCK2;
				if (wExAttr & ATO_BLOCK3)			pTile->wExtraAttr |= ATO_BLOCK3;
				if(cTemp & ATO_SMTILES)				stream->read(&pTile->dwSmTile,sizeof(DWORD));
				if(cTemp & ATO_TILES)				stream->read(&pTile->dwTile,sizeof(DWORD));
				if(cTemp & ATO_OBJECT)				stream->read(&pTile->uObject,sizeof(DWORD));
				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)	pTile->dwSmTile |= (PACKAGE_SmTiles << 16);
				if(cTemp & ATO_SOUND)				stream->read(&pTile->bySound,sizeof(BYTE));
				if(cTemp & ATO_UNDER_OBJ)				
				{
					//stream->read(&pTile->dwUnderObj,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_OVER_OBJ)		
				{
					//stream->read(&pTile->dwOverObj,sizeof(WORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(WORD));
				}
				if (wExAttr & EXATO_COLORADJT)		
				{
					//stream->read(&pTile->dwTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJST)		
				{
					//stream->read(&pTile->dwSTColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJOB)		
				{
					//stream->read(&pTile->dwObColor,sizeof(DWORD));//防止老地图这个地方有无效数据//有些地图dwObColor值有问题,比如skycity3的(146,396),所以暂时还是不用pTile->dwObColor
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJFLOOR)	
				{
					//stream->read(&pTile->dwFloorColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}
				if (wExAttr & EXATO_COLORADJEFF)	
				{
					//stream->read(&pTile->dwUnderObjColor,sizeof(DWORD));//防止老地图这个地方有无效数据
					stream->read(&dwNoUse,sizeof(DWORD));
				}

				if(cTemp & ATO_NEARGROUD)			stream->read(&pTile->uFloor,sizeof(DWORD));
				if(wExAttr & ATO_GROUP)
				{
					pTile->wExtraAttr |=  ATO_GROUP;
					DWORD dwGroupXY;
					stream->read(&dwGroupXY,sizeof(DWORD));

					WORD wX = iX - (short)(LOWORD(dwGroupXY));
					WORD wY = iY - (short)(HIWORD(dwGroupXY));
					dwGroupXY = MAKELONG(iX,iY);

					SMapTile* pTileEx = (info.pDataArray + (wY * iWidth) + wX);
					pTileEx->pTile2->vGroup.push_back(dwGroupXY);
				}
				pTile++;
			}
		}
	}
	else if (dwVer == 10 || dwVer == 0xB || dwVer == 0xC) // 增加 0xB 和 0xC
	{
		for(int iY=0;iY<iHeight && !stream->eof();iY++)
		{
			for(int iX = 0;iX<iWidth && !stream->eof();iX++)
			{
				stream->read(&cTemp,sizeof(BYTE));
				stream->read(&wExAttr,sizeof(WORD));
				if(cTemp & ATO_BLOCK)				pTile->byAttr |= AT_BLOCK;
				//if (cTemp & ATO_NEARGROUD)			pTile->byAttr |= AT_NEARGROUD;//新版本如果有ATO_NEARGROUD,会写入uFloor
				if (cTemp & ATO_PHENIX_PASS_BLOCK)	pTile->byAttr |= AT_PHENIX_PASS_BLOCK;
				if (wExAttr & ATO_BLOCK1)			pTile->wExtraAttr |= ATO_BLOCK1;
				if (wExAttr & ATO_BLOCK2)			pTile->wExtraAttr |= ATO_BLOCK2;
				if (wExAttr & ATO_BLOCK3)			pTile->wExtraAttr |= ATO_BLOCK3;
				if(cTemp & ATO_SMTILES)				stream->read(&pTile->dwSmTile,sizeof(DWORD));
				if(cTemp & ATO_TILES)				stream->read(&pTile->dwTile,sizeof(DWORD));
				if(cTemp & ATO_OBJECT)				stream->read(&pTile->uObject,sizeof(DWORD));
				if (pTile->dwTile != 0 && (pTile->dwTile >> 16) == 0)
				{
					if ((pTile->uObject & OT_TYPE_MASK) != OT_NORMAL)
						pTile->dwTile |= (PACKAGE_effect << 16);
					else	
						pTile->dwTile |= (PACKAGE_Tiles << 16);
				}
				if (pTile->dwSmTile != 0 && (pTile->dwSmTile >> 16) == 0)	pTile->dwSmTile |= (PACKAGE_SmTiles << 16);
				if(cTemp & ATO_SOUND)				stream->read(&pTile->bySound,sizeof(BYTE));
				if(cTemp & ATO_NEARGROUD)			stream->read(&pTile->uFloor,sizeof(DWORD));
				if(cTemp & ATO_UNDER_OBJ)				stream->read(&pTile->dwUnderObj,sizeof(DWORD));
				if (wExAttr & EXATO_OVER_OBJ)		stream->read(&pTile->dwOverObj,sizeof(DWORD));

				//为了省内存，夺宝暂时不用颜色
				//if (wExAttr & EXATO_COLORADJT)		stream->read(&pTile->dwTColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJST)		stream->read(&pTile->dwSTColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJOB)		stream->read(&pTile->dwObColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJFLOOR)	stream->read(&pTile->dwFloorColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJEFF)	stream->read(&pTile->dwUnderObjColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLOROVEROBJ)	stream->read(&pTile->dwOverObjColor,sizeof(DWORD));

				if (wExAttr & EXATO_COLORADJT)		stream->read(&dwNoUse,sizeof(DWORD));
				if (wExAttr & EXATO_COLORADJST)		stream->read(&dwNoUse,sizeof(DWORD));

				if (wExAttr & EXATO_COLORADJOB)		stream->read(&dwNoUse,sizeof(DWORD));
				if (wExAttr & EXATO_COLORADJFLOOR)	stream->read(&dwNoUse,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJOB)		stream->read(&pTile->dwObColor,sizeof(DWORD));
				//if (wExAttr & EXATO_COLORADJFLOOR)	stream->read(&pTile->dwFloorColor,sizeof(DWORD));

				if (wExAttr & EXATO_COLORADJEFF)	stream->read(&dwNoUse,sizeof(DWORD));
				if (wExAttr & EXATO_COLOROVEROBJ)	stream->read(&dwNoUse,sizeof(DWORD));

				if(wExAttr & ATO_GROUP)
				{
					pTile->wExtraAttr |=  ATO_GROUP;
					DWORD dwGroupXY;
					stream->read(&dwGroupXY,sizeof(DWORD));

					WORD wX = iX - (short)(LOWORD(dwGroupXY));
					WORD wY = iY - (short)(HIWORD(dwGroupXY));
					dwGroupXY = MAKELONG(iX,iY);

					SMapTile* pTileEx = (info.pDataArray + (wY * iWidth) + wX);
					pTileEx->pTile2->vGroup.push_back(dwGroupXY);
				}
				pTile++;
			}
		}
	}

	
	
	if (dwVer >= 10 && info.dwOffset > 0)
	{
		//从当前文件里直接读取
		//偏移到文件信息区
		stream->seek(info.dwOffset);

		SInfoHeader infoh;
		memset(&infoh,0,sizeof(SInfoHeader));
		stream->read(&infoh.dwSize,sizeof(DWORD));
		stream->read(&infoh.iTemplates,sizeof(int));

		if (infoh.dwSize > 8)
		{
			stream->read(&infoh.iBGLayers,sizeof(int));
		}

		//读取背景的配置信息
		if (infoh.iBGLayers > 0)
		{
			BGLayerData layer;
			//stream->read(&layer.iLayerNo,sizeof(int));
			stream->read(&layer.iTileOffX,sizeof(int));
			stream->read(&layer.iTileOffY,sizeof(int));
			stream->read(&layer.fSpeedRate,sizeof(float));

			layer.iLayerNo = 1;

			//ID已经存在的情况，不再加入
			MBGLayer::iterator itr = m_BgLayers.find(layer.iLayerNo);
			if(itr == m_BgLayers.end())
				m_BgLayers[layer.iLayerNo] = layer;
		}			

	}
	
	
	return true;
}

void CGameMap::CheckLoadMap()
{
	if (m_pDownloadMapDataBuf && m_dwDownloadMapDataLen > 0)
	{
		LockDownloadMap();
		if (m_pDownloadMapDataBuf && m_dwDownloadMapDataLen > 0)
		{
			string strMapName = this->GetMapName();
			LoadMap(strMapName.c_str(),true,m_pDownloadMapDataBuf,m_dwDownloadMapDataLen);
			SAFE_DELETE_ARRAY(m_pDownloadMapDataBuf);
			m_dwDownloadMapDataLen = 0;
		}
		UnLockDownloadMap();
	}

	if (m_pDownloadBgMapDataBuf && m_dwDownloadBgMapDataLen > 0)
	{
		LockDownloadMap();
		if (m_pDownloadBgMapDataBuf && m_dwDownloadBgMapDataLen > 0)
		{
			if (strstr(m_strRealDownloadBgMapFileName.c_str(),g_pGameMap->GetRealMapName()))//看看是不是当前地图的背景地图
			{
				LoadBgMap(m_pDownloadBgMapDataBuf,m_dwDownloadBgMapDataLen);
			}
			SAFE_DELETE_ARRAY(m_pDownloadBgMapDataBuf);
			m_dwDownloadBgMapDataLen = 0;
		}
		UnLockDownloadMap();
	}
}

void CGameMap::LockDownloadMap()
{
	EnterCriticalSection(&m_LoadMapCriSect);
}

void CGameMap::UnLockDownloadMap()
{
	LeaveCriticalSection(&m_LoadMapCriSect);
}

bool CGameMap::LoadMap(const char* sMap,bool bReLoad,const char* pInputDataBuf,DWORD dwLen)
{
	if(!sMap || strlen(sMap) < 1)
		return false;

	if (!g_pGameData->HasPaoPaoStatus(EP_FirstWalkToZhongZhou_PaoPao) && strcmp(sMap,"0") == 0)//第一次到中州
	{
		//if (g_EutUiType == EUT_CLASSIC)
		//{
		//	g_pControl->PopupArrowTip(MSG_CTRL_HANDYMAP_WND,EP_FirstWalkToZhongZhou_PaoPao,9,62,XAL_TOPLEFT,false,XAL_TOPRIGHT,0,0,0,false);
		//} 
		//else
		{
			g_pControl->PopupArrowTip(MSG_CTRL_HANDYMAP_WND,EP_FirstWalkToZhongZhou_PaoPao,142,172,XAL_TOPLEFT,false,XAL_TOPRIGHT,0,0,0,false);
		}		
	}


	//如果地图中有动态的数据,在保存到m_pOldMap之前要还原数据
	ClearDynamicData();

	string strRealMap = g_MapXmlMgr.GetRealMap(sMap);//转化为真实的物理存储地图名

	if(m_strRealMap == strRealMap && !bReLoad) //已经读入
	{
		//StartSceneMusic(NULL);
		m_strMapFile = sMap;
		return true;
	}

	g_pMinMap->GetExLocalNpc().clear();

	// 要读取得地图已经在缓冲中
	if(m_strOldRealMap == strRealMap && !bReLoad)
	{
		std::swap(m_dwVersion,m_dwOldVersion);
		std::swap(m_pOldMap,m_pMap);
		std::swap(m_iOldWidth,m_iWidth);
		std::swap(m_iOldHeight,m_iHeight);
		std::swap(m_strMapFile,m_strOldMapFile);
		std::swap(m_BgLayers,m_OldBgLayers);
		std::swap(m_strOldRealMap,m_strRealMap);
		std::swap(m_bOldIsEmptyMap,m_bIsEmptyMap);

		//		StartSceneMusic(NULL);

		if (g_Config.GetBkMusicOn())
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

		m_strMapFile = sMap;
		return true;
	}

	ClearMap();

	// 读取新的地图
	m_strMapFile = sMap;
	m_strRealMap = strRealMap;

	DataStreamInterface* stream = NULL;
	if (pInputDataBuf && dwLen > 0)
	{
		stream = g_pStreamMgr->CreateMemoryStream((void*)pInputDataBuf,dwLen);
	}
	else
	{
		LockDownloadMap();
		m_strRealDownloadMapFileName = strRealMap;
		SAFE_DELETE_ARRAY(m_pDownloadMapDataBuf);
		m_dwDownloadMapDataLen = 0;
		UnLockDownloadMap();

		//优先读取-3d的地图
		char szPath[MAX_PATH] = {0};

		sprintf(szPath,"map\\%s-3d.nmp",strRealMap.c_str());
		if(g_pGfx->IsSupportTide() && g_pStreamMgr->HasFileInServer(szPath))
		{
			stream = g_pStreamMgr->OpenDataFile(szPath,false,false,false,EP_MOST_HIGH);
		}
		else
		{
			sprintf(szPath,"map\\%s.nmp",strRealMap.c_str());
			stream = g_pStreamMgr->OpenDataFile(szPath,false,false,false,EP_MOST_HIGH);
		}
	}


	////显示正在读取地图
	//TRY_BEGIN;
	//int x,y,w,h;

	//string str = "地图读取中，请稍候...";
	//w = str.length()*6;
	//h = 12;
	//x = (g_pGfx->GetWidth() -w)/2;
	//y = (g_pGfx->GetHeight() - h)/2;

	//BOOL bDisplay = g_pGfx->IsDisplay();

	//g_pGfx->SetDisplay(TRUE);
	////g_pGfx->ClearColor(0xFF000000);
	//g_pGfx->DrawFillRect(x-32,y-16,w+64,h+32,0xFF000000);
	//g_pFont->DrawText(x,y,str.c_str(),0xFFFFFFFF,FONTSIZE_DEFAULT,0,0xFF000000);
	//g_pGfx->EndDraw();
	//g_pGfx->BeginDraw();

	//g_pGfx->SetDisplay(bDisplay);//恢复
	//TRY_END;

	//后台下载这张地图可以跳转过去的地图
	if(g_pBgDownLoadNet && !pInputDataBuf)
	{
		g_pBgDownLoadNet->StartBgDownloadCanJumpMap();
	}


	// 读不到地图，只好构造一张空地图
	if(!stream)
	{
		m_iWidth	= 700;
		m_iHeight	= 700;
		TRY_BEGIN
			m_pMap	= new SMapTile[m_iWidth * m_iHeight];
		TRY_END_RETURN(false)

		for(int x = 0;x < m_iWidth;x++)
		{
			m_pMap[(		  0)*m_iWidth+x].byAttr |= AT_USERBLOCK;
			m_pMap[(		  1)*m_iWidth+x].byAttr |= AT_USERBLOCK;
			m_pMap[(		  2)*m_iWidth+x].byAttr |= AT_USERBLOCK;
			m_pMap[(m_iHeight-3)*m_iWidth+x].byAttr |= AT_USERBLOCK;
			m_pMap[(m_iHeight-2)*m_iWidth+x].byAttr |= AT_USERBLOCK;
			m_pMap[(m_iHeight-1)*m_iWidth+x].byAttr |= AT_USERBLOCK;
		}
		for(int y = 0;y < m_iHeight;y++)
		{
			m_pMap[y*m_iWidth+(0		 )].byAttr |= AT_USERBLOCK;
			m_pMap[y*m_iWidth+(1		 )].byAttr |= AT_USERBLOCK;
			m_pMap[y*m_iWidth+(2		 )].byAttr |= AT_USERBLOCK;
			m_pMap[y*m_iWidth+(m_iWidth-3)].byAttr |= AT_USERBLOCK;
			m_pMap[y*m_iWidth+(m_iWidth-2)].byAttr |= AT_USERBLOCK;
			m_pMap[y*m_iWidth+(m_iWidth-1)].byAttr |= AT_USERBLOCK;
		}
		m_bIsEmptyMap = true;
		return false;
	}

	m_bIsEmptyMap = false;

	//直接读取地图
	SMapData MapDataInfo;
	if(ReadMap(stream,MapDataInfo)) //读取地图
	{
		SAFE_DELETE(stream);

		m_iWidth  = MapDataInfo.iWidth;
		m_iHeight = MapDataInfo.iHeight;
		m_pMap    = MapDataInfo.pDataArray;
		m_dwVersion  = MapDataInfo.dwVersion;
	}
	else //读取地图失败
	{
		SAFE_DELETE(stream);
		return false;
	}

	//读背景层数据,现在只有一个背景层，移动的速度由背景的高度和宽度决定
	LoadBgMap();

	if (g_Config.GetBkMusicOn())
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

	return true;
}

bool CGameMap::LoadBgMap(const char* pInputDataBuf,DWORD dwLen)
{
	//读背景层数据,现在只有一个背景层，移动的速度由背景的高度和宽度决定
	DataStreamInterface* stream = NULL;
	if (pInputDataBuf && dwLen > 0)
	{
		stream = g_pStreamMgr->CreateMemoryStream((void*)pInputDataBuf,dwLen);
	}
	else
	{
		char szPath[MAX_PATH] = {0};
		sprintf(szPath,"map\\%s_bg.nmp",m_strRealMap.c_str());

		LockDownloadMap();
		m_strRealDownloadBgMapFileName = szPath;
		SAFE_DELETE_ARRAY(m_pDownloadBgMapDataBuf);
		m_dwDownloadBgMapDataLen = 0;
		UnLockDownloadMap();

		stream = g_pStreamMgr->OpenDataFile(szPath,false,false,false,EP_MOST_HIGH);
	}

	if (!stream)
	{
		return false;
	}

	SMapData MapDataInfo;
	int iLayerNo = 1;

	if(ReadMap(stream,MapDataInfo))
	{
		SAFE_DELETE(stream);

		int _iBGWidth = MapDataInfo.iWidth;
		int _iBGHeight = MapDataInfo.iHeight;


		MBGLayer::iterator iter = m_BgLayers.find(iLayerNo);

		if(m_BgLayers.end() == iter)
		{
			BGLayerData bglayer;
			bglayer.iLayerNo = iLayerNo;
			bglayer.fSpeedRate = 0.75f;
			bglayer.iTileOffX = 10;
			bglayer.iTileOffY = 15;
			m_BgLayers[iLayerNo] = bglayer;
		}

		iter = m_BgLayers.begin();
		BGLayerData& bglayer = iter->second;

		SAFE_DELETE_ARRAY(bglayer.pDataArray);
		bglayer.iWidth = _iBGWidth;
		bglayer.iHeight = _iBGHeight;
		bglayer.pDataArray = MapDataInfo.pDataArray;
	}
	else
	{
		SAFE_DELETE(stream);
		return false;
	}

	return true;
}

bool CGameMap::LoadMiniMap(WORD wMap)
{
	if(wMap == ((WORD)m_dwMiniTexID))
		return true;

	if(m_dwOldMiniTexID && wMap == (WORD)m_dwOldMiniTexID)
	{
		DWORD dwTex	= m_dwMiniTexID;
		m_dwMiniTexID		= m_dwOldMiniTexID;
		m_dwOldMiniTexID	= dwTex;
		return true;
	}

	ClearMiniMap();

	//m_pMiniTex = g_pTexMgr->LoadTex(PACKAGE_mmap,wMap);
	m_dwMiniTexID = MAKELONG(wMap,PACKAGE_mmap);


	if(!m_dwMiniTexID)
		return false;

	return true;
}

void CGameMap::ClearMap()
{
	if(m_pMap)// 缓冲本次地图
	{
		SAFE_DELETE_ARRAY(m_pOldMap);

		m_pOldMap	= m_pMap;
		m_iOldWidth	= m_iWidth;
		m_iOldHeight= m_iHeight;
		m_strOldMapFile = m_strMapFile;
		m_strOldRealMap = m_strRealMap;
		m_dwOldVersion = m_dwVersion;
		m_bOldIsEmptyMap = m_bIsEmptyMap;

		g_pGameData->ResetDynMiniMapObj(const_cast<char*>(m_strMapFile.c_str()));

		while(!m_OldBgLayers.empty())
		{
			MBGLayer::iterator itr = m_OldBgLayers.begin();
			SAFE_DELETE_ARRAY(itr->second.pDataArray);
			m_OldBgLayers.erase(itr);
		}
		m_OldBgLayers = m_BgLayers;
	}

	m_pMap		= NULL;
	m_iWidth	= 0;
	m_iHeight	= 0;
	m_strMapFile.clear();
	m_strRealMap.clear();
	m_BgLayers.clear();
}

void CGameMap::ClearMiniMap()
{
	//if(m_pMiniTex)
	//{
	//	g_pTexMgr->ReleaseTex(m_pOldMiniTex);
	//	m_pOldMiniTex = m_pMiniTex;
	//	m_pMiniTex = NULL;
	//}

	if (m_dwMiniTexID)
	{
		m_dwOldMiniTexID = m_dwMiniTexID;
		m_dwMiniTexID = 0;
	}
}

SMapTile * CGameMap::GetTile(int iX,int iY)
{
	if(iX < 0 || iY < 0 || iX >= m_iWidth || iY >= m_iHeight)
		return NULL;
	return &(m_pMap[iY * m_iWidth + iX]);
}

bool CGameMap::IsBlock(int iX,int iY)
{
	if(iX < 0 || iY < 0 || iX >= m_iWidth || iY >= m_iHeight)
		return true;

	BYTE byAttr = m_pMap[iY * m_iWidth + iX].byAttr;
	DWORD dwBlock = AT_BLOCK | AT_USERBLOCK | AT_PHENIX_PASS_BLOCK;
	dwBlock &= ~m_dwPassBlockType;

	return ((byAttr & dwBlock) != 0);
}

BYTE CGameMap::GetUsed(int iX,int iY)
{
	if(iX < 0 || iY < 0 || iX >= m_iWidth || iY >= m_iHeight)
		return 0;

	BYTE byAttr = m_pMap[iY * m_iWidth + iX].byAttr;

	return ((byAttr & AT_USED) != 0);
}

//临时设置当前格子阻挡2006-2-6；修正客户端和服务器地图不一致
void CGameMap::SetUserBlock(int iX,int iY)
{
	if(iX < 0 || iY < 0 || iX >= m_iWidth || iY >= m_iHeight)
		return;

	m_pMap[iY * m_iWidth + iX].byAttr |= AT_USERBLOCK;
}

BGLayerData* CGameMap::GetBgLayer(int iLayerNo)
{
	MBGLayer::iterator itr = m_BgLayers.find(iLayerNo);
	if(itr == m_BgLayers.end())
		return NULL;

	return &itr->second;
}

void CGameMap::AddDynamicData(int iX,int iY,DWORD dwType,DWORD dwData)
{
	SMapTile * pTile = GetTile(iX,iY);
	if(!pTile)
		return;

	if(pTile->pDynTile == NULL)
	{
		pTile->pDynTile = new SDynMapTile();
		SDynMapTile * pDynTile = pTile->pDynTile;
		pDynTile->dwSmTile = pTile->dwSmTile;
		pDynTile->dwTile = pTile->dwTile;
		pDynTile->uObject = pTile->uObject;
// 		pDynTile->wObjectH = pTile->wObjectH;
		pDynTile->dwUnderObj = pTile->dwUnderObj;
		pDynTile->bySound = pTile->bySound;
		pDynTile->byAttr = pTile->byAttr;

		pDynTile->uFloor = pTile->uFloor;			//地板，贴地物体
		pDynTile->wExtraAttr = pTile->wExtraAttr;     // 砖块的扩展属性[1978]
		//pDynTile->dwSTColor = pTile->dwSTColor;		// 小地表颜色
		//pDynTile->dwTColor = pTile->dwTColor;		// 大地表颜色
		//pDynTile->dwObColor = pTile->dwObColor;		// 物体颜色	
		//pDynTile->dwFloorColor = pTile->dwFloorColor;   // 贴地物体颜色
		//pDynTile->dwUnderObjColor = pTile->dwUnderObjColor;		// 特效颜色
		//pDynTile->dwOverObjColor = pTile->dwOverObjColor;		// 特效颜色
	}

	pTile->pDynTile->dwParts |= dwType;

	switch(dwType)
	{
	case ATO_SMTILES:
		pTile->dwSmTile = (WORD)(dwData);
// 		pTile->dwSTColor = -1;
		break;
	case ATO_TILES:
		pTile->dwTile = (WORD)(dwData);
// 		pTile->dwTColor = -1;
		break;
	case ATO_OBJECT:
		pTile->uObject = dwData;
// 		pTile->dwObColor = -1;

// 		if((dwData & OT_OBJECT_MASK) != 0)
// 		{
// 			LPTexture pTex = g_pTexMgr->GetTexImm(HIWORD(dwData),LOWORD(dwData));
// 			if(pTex)
// 			{
// 				pTile->wObjectH = pTex->GetHeight0();
// 			}
// 		}
// 		else
// 		{
// 			pTile->wObjectH = 0;
// 		}

		break;
	case ATO_UNDER_OBJ:
		pTile->dwUnderObj = dwData;
		//pTile->dwUnderObjColor = -1;
		break;
	case ATO_SOUND:
		pTile->bySound = (BYTE)(dwData);
		break;
	case ATO_BLOCK:
		pTile->byAttr |= AT_BLOCK;
		break;
	case ATO_CLEAR_BLOCK:
		pTile->byAttr &= ~AT_BLOCK;
		pTile->byAttr &= ~AT_PHENIX_PASS_BLOCK;
		break;
	case ATO_NEARGROUD:
		pTile->byAttr |= AT_NEARGROUD;
		break;
	case ATO_PHENIX_PASS_BLOCK:
		pTile->byAttr |= AT_PHENIX_PASS_BLOCK;
		break;
	case ATO_HIDE_APPEAR:
		{
			pTile->pDynTile->byAppHide = 1;//渐现
			pTile->pDynTile->dwTime = dwData;
		}
		break;
	case ATO_APPERA_HIDE:
		{
			pTile->pDynTile->byAppHide = 2;//渐隐
			pTile->pDynTile->dwTime = dwData;
		}
		break;
	}
}

void CGameMap::DelDynamicData(int iX,int iY,DWORD dwType)
{
	SMapTile * pTile = GetTile(iX,iY);
	if(!pTile || !pTile->pDynTile)
		return;

	switch(dwType)
	{
	case ATO_SMTILES:
		pTile->dwSmTile = pTile->pDynTile->dwSmTile;
// 		pTile->dwSTColor = pTile->pDynTile->dwSTColor;
		break;
	case ATO_TILES:
		pTile->dwTile = pTile->pDynTile->dwTile;
// 		pTile->dwTColor = pTile->pDynTile->dwTColor;
		break;
	case ATO_OBJECT:
		pTile->uObject = pTile->pDynTile->uObject;
// 		pTile->wObjectH = pTile->pDynTile->wObjectH;
// 		pTile->dwObColor = pTile->pDynTile->dwObColor;
		break;
	case ATO_UNDER_OBJ:
		pTile->dwUnderObj = pTile->pDynTile->dwUnderObj;
		//pTile->dwUnderObjColor = pTile->pDynTile->dwUnderObjColor;
		break;
	case ATO_SOUND:
		pTile->bySound = pTile->pDynTile->bySound;
		break;
	case ATO_BLOCK:
	case ATO_CLEAR_BLOCK:
		if(pTile->pDynTile->byAttr & AT_BLOCK)
			pTile->byAttr |= AT_BLOCK;
		else
			pTile->byAttr &= ~AT_BLOCK;

		break; 
	case ATO_NEARGROUD:
	case ATO_PHENIX_PASS_BLOCK:
		if(pTile->pDynTile->byAttr & dwType)
			pTile->byAttr |= dwType;
		else
			pTile->byAttr &= ~dwType;

		break;
	}

	pTile->pDynTile->dwParts &= ~dwType;
	if(pTile->pDynTile->dwParts == 0)
	{
		SAFE_DELETE(pTile->pDynTile);
	}

}

void CGameMap::ClearDynamicData()
{
	if(m_iWidth <= 0 || m_iHeight <= 0 || !m_pMap)
		return;

	DWORD dwLen = m_iWidth * m_iHeight;
	SMapTile *pTile = NULL;
	for(DWORD i = 0; i < dwLen; i++)
	{
		pTile = &(m_pMap[i]);
		if(pTile->pDynTile)
		{
			pTile->dwSmTile = pTile->pDynTile->dwSmTile;
			pTile->dwTile = pTile->pDynTile->dwTile;
			pTile->uObject = pTile->pDynTile->uObject;
// 			pTile->wObjectH = pTile->pDynTile->wObjectH;
			pTile->dwUnderObj = pTile->pDynTile->dwUnderObj;
			pTile->bySound = pTile->pDynTile->bySound;
			pTile->byAttr = pTile->pDynTile->byAttr;

			SAFE_DELETE(pTile->pDynTile);
		}
	}
}

void CGameMap::AddDynObject(DWORD dwIdx,int iX,int iY)
{
	MDynamicObject::iterator itr = m_MDynObj.find(dwIdx);
	if (itr == m_MDynObj.end())
		return;

	SDynamicObject &obj = itr->second;
	SDynamicData *p = NULL;
	int iCount = (int)(obj.vData.size());

	BYTE byAppHide = 0;
	MDynamicAppHide::iterator itTemp = m_MDynAppHide.find(dwIdx);
	if (itTemp != m_MDynAppHide.end())
		byAppHide = itTemp->second;

	DWORD dwTime = GetTickCount();
	for (int i = 0;i < iCount; i++)
	{
		p = &(obj.vData[i]);
		if(!p)
		{
			break;
		}

		DWORD dwType = p->dwType;

		if (dwType & ATO_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_BLOCK,AT_BLOCK);
		}
		if (dwType & ATO_PHENIX_PASS_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_PHENIX_PASS_BLOCK,AT_PHENIX_PASS_BLOCK);
		}
		if (dwType & ATO_SMTILES)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_SMTILES,p->tile.dwSmTile);
		}
		if (dwType & ATO_TILES)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_TILES,p->tile.dwTile);
		}
		if (dwType & ATO_OBJECT)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_OBJECT,p->tile.uObject);
		}
		if (dwType & ATO_SOUND)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_SOUND,p->tile.bySound);
		}
		if (dwType & ATO_UNDER_OBJ)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_UNDER_OBJ,p->tile.dwUnderObj);
		}
		if (dwType & ATO_NEARGROUD)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_NEARGROUD,AT_NEARGROUD);
		}
		if (dwType & ATO_CLEAR_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_CLEAR_BLOCK,AT_CLEAR_BLOCK);
		}

		if (byAppHide)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_APPERA_HIDE,dwTime);//淅现
		}
	}
}

void CGameMap::ModifyObject(DWORD dwIdx,int iX,int iY,DWORD dwType,DWORD dwData)
{
	MDynamicObject::iterator itr = m_MDynObj.find(dwIdx);
	if (itr == m_MDynObj.end())
		return;

	SDynamicObject &obj = itr->second;
	SDynamicData *p = NULL;
	int iCount = (int)(obj.vData.size());

	BYTE byAppHide = 0;
	MDynamicAppHide::iterator itTemp = m_MDynAppHide.find(dwIdx);
	if (itTemp != m_MDynAppHide.end())
		byAppHide = itTemp->second;

	for (int i = 0;i < iCount; i++)
	{
		p = &(obj.vData[i]);
		if(!p) break;

		if (dwType & ATO_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_BLOCK,AT_BLOCK);
		}
		if (dwType & ATO_PHENIX_PASS_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_PHENIX_PASS_BLOCK,AT_PHENIX_PASS_BLOCK);
		}
		if (dwType & ATO_SMTILES)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_SMTILES,p->tile.dwSmTile);
		}
		if (dwType & ATO_TILES)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_TILES,p->tile.dwTile);
		}
		if (dwType & ATO_OBJECT)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_OBJECT,p->tile.uObject);
		}
		if (dwType & ATO_SOUND)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_SOUND,p->tile.bySound);
		}
		if (dwType & ATO_UNDER_OBJ)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_UNDER_OBJ,p->tile.dwUnderObj);
		}
		if (dwType & ATO_NEARGROUD)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_NEARGROUD,AT_NEARGROUD);
		}
		if (dwType & ATO_CLEAR_BLOCK)
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_CLEAR_BLOCK,AT_CLEAR_BLOCK);
		}

		if (byAppHide && (dwType & ATO_APPERA_HIDE))
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_APPERA_HIDE,dwData);//淅隐
		}

		if (byAppHide && (dwType & ATO_HIDE_APPEAR))
		{
			AddDynamicData(iX + p->sX,iY + p->sY,ATO_HIDE_APPEAR,dwData);//渐现
		}
	}
}

void CGameMap::DelDynObject(DWORD dwIdx,int iX,int iY)
{
	MDynamicObject::iterator itr = m_MDynObj.find(dwIdx);
	if (itr == m_MDynObj.end())
		return;

	SDynamicObject &obj = itr->second;
	SDynamicData *p = NULL;

	BYTE byAppHide = 0;
	MDynamicAppHide::iterator itTemp = m_MDynAppHide.find(dwIdx);
	if (itTemp != m_MDynAppHide.end())
		byAppHide = itTemp->second;

	int iCount = (int)(obj.vData.size());
	for (int i = 0;i < iCount; i++)
	{
		p = &(obj.vData[i]);
		SMapTile * pTile = GetTile(iX + p->sX,iY + p->sY);
		if(!pTile || !pTile->pDynTile)
		{
			break;
		}

		pTile->dwSmTile = pTile->pDynTile->dwSmTile;
		pTile->dwTile = pTile->pDynTile->dwTile;
		pTile->uObject = pTile->pDynTile->uObject;
// 		pTile->wObjectH = pTile->pDynTile->wObjectH;
		pTile->dwUnderObj = pTile->pDynTile->dwUnderObj;
		pTile->bySound = pTile->pDynTile->bySound;
		pTile->byAttr = pTile->pDynTile->byAttr;

		SAFE_DELETE(pTile->pDynTile);

	}
}


//////////////////////////////////////////////////////////////////////////////////
// 参数   ptStartPoint: 起始坐标
//        ptEndPoint:   终止坐标
// 返回值 0:  传入的两点重合
//        <0: 两点之间超过3个阻挡点，返回第三个阻挡点与起始点的偏移
//        >0: 两点之间无阻挡
int CGameMap::CanThrough(int iStartX,int iStartY, int iEndX,int iEndY)
{
	int ptPosX;
	int ptPosY;
	int nSteps = max(abs(iEndX - iStartX), abs(iEndY - iStartY));
	if (nSteps == 0)
		return 0;

	double nXOffset = ((double)(iEndX - iStartX)) / nSteps;
	double nYOffset = ((double)(iEndY - iStartY)) / nSteps;

	int nCount = 0;
	for (int i = 1; i < nSteps; i++)
	{
		ptPosX = (WORD)(iStartX + i * nXOffset + 0.5);
		ptPosY = (WORD)(iStartY + i * nYOffset + 0.5);
		if ((IsBlock(ptPosX, ptPosY)) && (++nCount >= 3))
			return (-i);
	}
	return nSteps;
}
