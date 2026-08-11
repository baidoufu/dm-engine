#include "Net.h"
#include "Global/DebugTry.h"
#include "Global/PerfCheck.h"
#include "GameData/GameGlobal.h"
#include "Global/Interface/StreamInterface.h"
#include "NetData.h"
#include "GameControl/GameControl.h"
#include "Global/Interface/TexmanagerInterface.h"
#include "clissproto.h"
#include "GameMap/GameMap.h"
#include "GameMap/MinMap.h"
#include "GameMap/MapXmlMgr.h"


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

 

CDownloadNet::CDownloadNet(HWND hWnd,DWORD dwMsg,bool bBgLimitedDownload,long lEvent)
{
	m_bBgLimitedDownload = bBgLimitedDownload;

	InitMember();

	m_pRecBuf = new char[MAX_RECBUF_LENJ_DOWNLOADFILE];

	Init(1,hWnd,dwMsg,lEvent);
	m_strAllRecDataBuf.reserve(1024*1024);

	m_hThread = CreateThread(NULL,0,ProcessBuffer,this,0,&m_dwThreadID);
	SetThreadPriority(m_hThread,THREAD_PRIORITY_ABOVE_NORMAL);

	m_dlSpeed = (int)GetPrivateProfileInt("Config","BgDlSpeed",512,"./Config.ini");

}

CDownloadNet::CDownloadNet()
{
}

CDownloadNet::~CDownloadNet()
{
	SAFE_DELETE_ARRAY(m_pRecBuf);
	SAFE_DELETE_ARRAY(m_pszPacke);
	SAFE_DELETE(m_pDLoSsProto);

	if(m_hThread)
	{
		m_bExited = TRUE;
		WaitForSingleObject(m_hThread,INFINITE);
		CloseHandle(m_hThread);
	}

	DeleteCriticalSection(&m_CriSect);
	DeleteCriticalSection(&m_CriSect_Send);

	SAFE_DELETE(m_pStSSMsg);
}

void CDownloadNet::InitMember()
{
	m_bExited = FALSE;
	m_hThread = NULL;
	m_dwThreadID = 0;
	m_dwAllRecDataBufLen = 0;
	m_dwAllRecDataBufLen_SaveToDisk = 0;
	m_dwAllRecDataBufPos = 0;
	m_dwAllRecDataBufPos_SaveToDisk = 0;
	m_iReceiveFiles = 0;
	m_iLastReceiveFiles = 0;
	m_iReceiveDataLen = 0;
	m_dwLastSendDataTime = GetTickCount();
	m_dwStartTime = GetTickCount();
	m_bNeedReConnect = false;
	m_bNeedDownloadCanJumpMap = false;

	InitializeCriticalSection(&m_CriSect);
	InitializeCriticalSection(&m_CriSect_Send);

	m_pStSSMsg = new PKG_CLI_SS_MSG();

	m_pszPacke = new char[PACKAGE_LENGTH];

	m_pDLoSsProto = new CCliSsProto();

}

DWORD WINAPI CDownloadNet::ProcessBuffer(LPVOID lpData)
{
	CDownloadNet* pThis = reinterpret_cast<CDownloadNet*>(lpData);
	if (!pThis)
	{
		return 0;
	}

	while(TRUE)
	{
		if(pThis->m_bExited)
			break;
 

		if (pThis->m_bNeedReConnect)
		{
			pThis->Close(0);
			Sleep(1000);
			pThis->Connect(0);
			pThis->m_bNeedReConnect = false;
		}

		if (pThis->m_bBgLimitedDownload && pThis->m_dwRecvBytes * 1000 / (GetTickCount() + 1 - pThis->m_dwStartTime) > pThis->m_dlSpeed*1024)//限速m_dlSpeed K/s
		{
			Sleep(50);
			continue;
		}


		pThis->DealRead();

		pThis->SendAsynBuf();

		//后台限速下载
		if(pThis->m_bBgLimitedDownload)
		{
			if (pThis->m_bNeedDownloadCanJumpMap)
			{
				pThis->BgDownloadCanJumpMap();
			}


			if (pThis->m_dwAllRecDataBufLen_SaveToDisk >= DOWNLOAD_HEADER_LEN)
			{
				CLock lock(&(pThis->m_CriSect));

				if (GetTickCount() - pThis->m_dwStartTime > 10000)//过了10s重新计算
				{
					pThis->m_dwStartTime = GetTickCount();
					pThis->m_dwRecvBytes = 0;
				}

				if (pThis->m_dwAllRecDataBufLen_SaveToDisk >= DOWNLOAD_HEADER_LEN)//再判一次,有可能在上面判了以后主线改变了这个值
				{
					//没有下载数据要处理的空闲时间用来保存文件,每次只保存一个文件,防止长时间卡在保存文件里面,导致主线程无法接收协议以及无法处理接收到的文件
					// 从头部开始解析
					STDlNetMsgHeader *pHeader = (STDlNetMsgHeader *)(pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos_SaveToDisk);
					WORD wMsgID = ::ntohs(pHeader->wMsgID);
					DWORD dwDataLen = ::ntohl(pHeader->dwDataLen);

					int nCompleteLen = dwDataLen + sizeof(STDlNetMsgHeader);
					if (dwDataLen > 0 && pThis->m_dwAllRecDataBufLen_SaveToDisk >= nCompleteLen )//包已经收完
					{
						const char* buf = pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos_SaveToDisk + sizeof(STDlNetMsgHeader);
						
						__int64 iRequireHash = 0;
						if (dwDataLen > 32)
						{
							PSDLProtocalHeader pHeader = PSDLProtocalHeader(buf);

							BYTE byFilePathLen = BYTE(buf[sizeof(SDLProtocalHeader)]);
							int iPos = sizeof(SDLProtocalHeader) + sizeof(BYTE) + byFilePathLen;

							if (iPos < dwDataLen)
							{
								string szFileName;
								szFileName.assign(buf + sizeof(SDLProtocalHeader) + sizeof(BYTE),byFilePathLen);

								iRequireHash = g_pStreamMgr->HashFileName(szFileName.c_str());
							}
						}


						map<__int64,DlProtocalBody>::iterator itr = pThis->m_MBgInDownloadFile.find(iRequireHash);
						if (itr != pThis->m_MBgInDownloadFile.end())
						{
							g_pStreamMgr->SaveDownloadedFileToWpf(buf, dwDataLen,&(itr->second)); 
							pThis->m_MBgInDownloadFile.erase(itr);

							if(pThis->OnBgSavedFile())
							{
								continue;
							}
						}

						//output_debug("保存文件 %d/%d/%d/%d\r\n",m_dwAllRecDataBufPos,nCompleteLen,m_dwAllRecDataBufLen,m_dwAllRecDataBufLen_SaveToDisk);


						// 取下一个packet地址
						//pThis->m_strAllRecDataBuf.erase(0, nCompleteLen);//并不会释放内存,而且很耗
						//if (pThis->m_dwAllRecDataBufPos >= nCompleteLen)
						//{
						//	pThis->m_dwAllRecDataBufPos -= nCompleteLen;
						//}
						//else
						//{
						//	int i = 3;
						//}

						pThis->m_dwAllRecDataBufPos_SaveToDisk += nCompleteLen;
						pThis->m_dwAllRecDataBufLen_SaveToDisk -= nCompleteLen;


						if (pThis->m_dwAllRecDataBufLen_SaveToDisk == 0)
						{
							pThis->m_strAllRecDataBuf.clear();

							if (pThis->m_dwAllRecDataBufPos_SaveToDisk > 15 * 1024 *1024)//超过了15M清除一次
							{

								pThis->m_strAllRecDataBuf.reserve();
							}

							pThis->m_dwAllRecDataBufPos_SaveToDisk = 0;
							pThis->m_dwAllRecDataBufPos = 0;
						}

						pThis->m_iReceiveFiles ++;
					}
				}
			}

			Sleep(50);//只要没有收到文件,保存完一个文件也休息一下,不然可能会卡住太长时间

			continue;
		}



		//前台正常下载
		if(pThis->m_dwAllRecDataBufLen < DOWNLOAD_HEADER_LEN)
		{
			if (pThis->m_dwAllRecDataBufLen_SaveToDisk >= DOWNLOAD_HEADER_LEN)
			{
				CLock lock(&(pThis->m_CriSect));

				if (pThis->m_dwAllRecDataBufLen < DOWNLOAD_HEADER_LEN && pThis->m_dwAllRecDataBufLen_SaveToDisk >= DOWNLOAD_HEADER_LEN)//再判一次m_dwAllRecDataBufLen < DOWNLOAD_HEADER_LEN,有可能在上面判了以后主线改变了这个值
				{
					//没有下载数据要处理的空闲时间用来保存文件,每次只保存一个文件,防止长时间卡在保存文件里面,导致主线程无法接收协议以及无法处理接收到的文件
					// 从头部开始解析
					STDlNetMsgHeader *pHeader = (STDlNetMsgHeader *)(pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos_SaveToDisk);
					WORD wMsgID = ::ntohs(pHeader->wMsgID);
					DWORD dwDataLen = ::ntohl(pHeader->dwDataLen);

					int nCompleteLen = dwDataLen + sizeof(STDlNetMsgHeader);
					if (dwDataLen > 0 && pThis->m_dwAllRecDataBufLen_SaveToDisk >= nCompleteLen )//包已经收完
					{

						g_pStreamMgr->SaveDownloadedFileToWpf(pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos_SaveToDisk + sizeof(STDlNetMsgHeader), dwDataLen); 

						//output_debug("保存文件 %d/%d/%d/%d\r\n",m_dwAllRecDataBufPos,nCompleteLen,m_dwAllRecDataBufLen,m_dwAllRecDataBufLen_SaveToDisk);


						// 取下一个packet地址
						//pThis->m_strAllRecDataBuf.erase(0, nCompleteLen);//并不会释放内存,而且很耗
						//if (pThis->m_dwAllRecDataBufPos >= nCompleteLen)
						//{
						//	pThis->m_dwAllRecDataBufPos -= nCompleteLen;
						//}
						//else
						//{
						//	int i = 3;
						//}

						pThis->m_dwAllRecDataBufPos_SaveToDisk += nCompleteLen;
						pThis->m_dwAllRecDataBufLen_SaveToDisk -= nCompleteLen;


						if (pThis->m_dwAllRecDataBufLen_SaveToDisk == 0)
						{
							pThis->m_strAllRecDataBuf.clear();

							if (pThis->m_dwAllRecDataBufPos_SaveToDisk > 15 * 1024 *1024)//超过了15M清除一次
							{

								pThis->m_strAllRecDataBuf.reserve();
							}

							pThis->m_dwAllRecDataBufPos_SaveToDisk = 0;
							pThis->m_dwAllRecDataBufPos = 0;
						}

					}
				}
			}

			Sleep(1);//只要没有收到文件,保存完一个文件也休息一下,不然可能会卡住太长时间

			continue;
		}


		CLock lock2(&(pThis->m_CriSect));

		while (pThis->m_dwAllRecDataBufLen >= DOWNLOAD_HEADER_LEN)
		{ 
			// 从头部开始解析
			STDlNetMsgHeader *pHeader = (STDlNetMsgHeader *)(pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos);
			WORD wMsgID = ::ntohs(pHeader->wMsgID);
			DWORD dwDataLen = ::ntohl(pHeader->dwDataLen);

			if (wMsgID != 3001)//非法包
			{
				output_debug("CDownloadNet: wMsgID错误=%04X, dwDataLen=%u", wMsgID, dwDataLen);
				pThis->m_bNeedReConnect = true;
				break;
			}


			int nCompleteLen = dwDataLen + sizeof(STDlNetMsgHeader);
			if (dwDataLen <= 0 || pThis->m_dwAllRecDataBufLen < nCompleteLen )
			{
				Sleep(1);//休息一下
				break;//包还没有收完
			}

			bool bRtn = g_pStreamMgr->OnDownloadedFile(pThis->m_strAllRecDataBuf.c_str() + pThis->m_dwAllRecDataBufPos + sizeof(STDlNetMsgHeader), dwDataLen);
			if (!bRtn)
			{
				pThis->m_bNeedReConnect = true;
				output_debug("OnDownloadedFile Fail");
				break;
			}

		
// 			output_debug("收文件 %d/%d/%d/%d\r\n",m_dwAllRecDataBufPos,nCompleteLen,m_dwAllRecDataBufLen,m_dwAllRecDataBufLen_SaveToDisk);

			// 取下一个packet地址
			pThis->m_dwAllRecDataBufPos += nCompleteLen;
			pThis->m_dwAllRecDataBufLen -= nCompleteLen;
			pThis->m_iReceiveFiles ++;
			pThis->m_iReceiveDataLen += dwDataLen;

		}
	}

	pThis->m_dwThreadID = 0;

	return 0; 
}

void CDownloadNet::Close(int iServer)
{
	__super::Close(iServer);

	CLock lock(&m_CriSect);
	m_strAllRecDataBuf.clear();
	m_dwAllRecDataBufLen = 0;
	m_dwAllRecDataBufLen_SaveToDisk = 0;
	m_dwAllRecDataBufPos = 0;
	m_dwAllRecDataBufPos_SaveToDisk = 0;
	m_iReceiveFiles = 0;
	m_iReceiveDataLen = 0;
	m_iLastReceiveFiles = 0;

	//不要清除iSendBuffer,有些地方重连会先调用close(),也有些地方没有连上也会调用close()
	//if(iServer >= 0 && iServer < m_buf.size() && m_buf[iServer])
	//{
	//	 m_buf[iServer]->iSendBuffer = 0;
	//}

}

int CDownloadNet::SendBuf(char* buf,DWORD len)
{	
	CLock lock(&m_CriSect_Send);
	//还没有应答的情况下，不发送新的数据
	SOCKET_INFORMATION * SocketInfo = m_buf[0];

	// 1.构造二进制协议包
	STNetMsgHeader stHeader;

	WORD wMsgID = CLI_SS_MSG;
	stHeader.wMsgID = ::htons(wMsgID);


 	memcpy(m_pStSSMsg->abyMsgData, buf, len);
 
 	m_pStSSMsg->wMsgLen = (WORD)len;
 	int nEncLen = m_pDLoSsProto->Encode(wMsgID, (void*)m_pStSSMsg,
 		m_pszPacke + gnNetMsgHeaderSize, PACKAGE_LENGTH-gnNetMsgHeaderSize);
 
 
 	stHeader.wDataLen = ::htons((WORD)nEncLen);
 	*((STNetMsgHeader*)m_pszPacke) = stHeader;


	if(!SocketInfo->bConnect)
	{
		//第一次请求的时候进行连接
		if (SocketInfo->dwLastConnectTime == 0)
		{
			Connect(0);
		}
		AsynSend(0, m_pszPacke, nEncLen + gnNetMsgHeaderSize);
		return 0;
	}
	else if (SocketInfo->iSendBuffer)
	{
		Send(0,SocketInfo->SendBuffer,SocketInfo->iSendBuffer,true);
	}

	int iRtn = Send(0, m_pszPacke, nEncLen + gnNetMsgHeaderSize);

	return iRtn;
}

bool CDownloadNet::OnBgSavedFile()
{
	//没有要下载的文件那么断开连接,下次要求下载时再连接
	if(m_MBgInDownloadFile.size() == 0)
	{
		SOCKET_INFORMATION * SocketInfo = m_buf[0];
		SocketInfo->dwLastConnectTime = 0;
		Close(0);
		return true;
	}

	return false;
}

void CDownloadNet::SendAsynBuf()
{
	if(m_buf.size() <= 0)
	{
		return;
	}

	SOCKET_INFORMATION * SocketInfo = m_buf[0];
	if (!SocketInfo->bConnect)
	{
		if(m_bBgLimitedDownload)//后台下载断开了连接不要重连,等到有新的下载请求时会重新连接
		{
			Sleep(3000);
		}
		else if (SocketInfo->dwLastConnectTime != 0 && GetTickCount() - SocketInfo->dwLastConnectTime > 3000)
		{
			if (SocketInfo->iReConnectTimes < 18)
			{
				Connect(0);
				SocketInfo->iReConnectTimes ++;
				SocketInfo->dwLastConnectTime = GetTickCount();
			}
			else
			{
				int iSubType = g_pStreamMgr->GetOtherConfigInt("SubType",0);//SubType:0为普通客户端,1为小客户端,
				if (iSubType == 1 && g_pStreamMgr->GetDownloadingFiles() > 0)
				{
					g_pGameControl->GCL_AddErr();
				}
				
				SocketInfo->dwLastConnectTime = 0;
				SocketInfo->iReConnectTimes = 0;
			}

		}
		return;
	}

	SendReceiveFileCountToSourceServer();

	if (SocketInfo->iSendBuffer)
	{
		CLock lock(&m_CriSect_Send);
		Send(0,SocketInfo->SendBuffer,SocketInfo->iSendBuffer,true);
		return;
	}

	//如果长时间没有发送数据,每过一段时间发一个心跳包
	if (GetTickCount() - m_dwLastSendDataTime > 20000)
	{
		SendEmptyProtocal();
	}

}

void CDownloadNet::SendEmptyProtocal()
{
	SDLProtocalHeader pHeader;

	pHeader.wFileCount = 0;//文件个数,为0服务器不会处理
	pHeader.wProtocal = 0x1002;//心跳包协议号
	pHeader.dwID = 0;//id
	pHeader.i64Hash = 0;//第一个hash值
	pHeader.iWpf = 0;

	SendBuf((char*)(&pHeader),sizeof(SDLProtocalHeader));
}

void CDownloadNet::SendReceiveFileCountToSourceServer()
{
	if (m_iReceiveFiles <= 0 || m_iLastReceiveFiles == m_iReceiveFiles)
	{
		return;
	}

	SDLProtocalHeader pHeader;

	pHeader.wFileCount = 0;//文件个数,为0服务器不会处理
	pHeader.wProtocal = 0x1003;//告诉资源服务器一共接受到了多少个文件
	pHeader.dwID = m_iReceiveFiles;
	pHeader.i64Hash = 0;
	pHeader.iWpf = m_iReceiveDataLen;

	m_iLastReceiveFiles = m_iReceiveFiles;

	SendBuf((char*)(&pHeader),sizeof(SDLProtocalHeader));
}

void CDownloadNet::BgSendDownloadFileRequest(__int64 iHash,DWORD dwID,eReadPrior ePrior)
{
	static char buf[2048] = {0};

	map<__int64,BYTE>& hashmap = g_pStreamMgr->GetAllFileHashMap();
	map<__int64,BYTE>::iterator itr = hashmap.find(iHash);
	if (itr == hashmap.end())
	{
		return;
	}

	if (m_MBgInDownloadFile.find(iHash) != m_MBgInDownloadFile.end())
	{
		return;
	}


	PSDLProtocalHeader pHeader = (PSDLProtocalHeader)buf;

	memset(pHeader,0,sizeof(SDLProtocalHeader));

	pHeader->wFileCount = 1;//文件个数
	pHeader->wProtocal = 0x1001;//协议号
	pHeader->dwID = dwID;//id
	pHeader->i64Hash = iHash;//第一个hash值
	pHeader->iWpf = 0;

	PSDlProtocalBody pBody = (PSDlProtocalBody)(buf + sizeof(SDLProtocalHeader));

	pBody->dwID = dwID;
	pBody->i64Hash = iHash;
	pBody->iWpf = itr->second;
	pBody->dwStartPos = 0;
	pBody->byPrior = ePrior;

	m_MBgInDownloadFile[iHash] = *pBody;


	SendBuf(buf,sizeof(SDLProtocalHeader) + sizeof(SDlProtocalBody) * 1);
}


//发送信息
int CDownloadNet::Send( int iServer,char * buf,DWORD len,bool bSendAsynBuf)
{
	m_dwLastSendDataTime = GetTickCount();

	WSABUF	DataBuf;
	DataBuf.buf		= buf;
	DataBuf.len		= len;
	DWORD SendBytes = 0;
	DWORD total		= 0;

	SOCKET_INFORMATION * SocketInfo = m_buf[iServer];

	while(total < len)
	{
		if(WSASend(SocketInfo->Socket, &DataBuf,1,&SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
		{
			//int iErr = WSAGetLastError();
			//output_debug("CDownloadNet::WSASend Error:%d\r\n",iErr);

			if(!bSendAsynBuf)
			{
				if ((SocketInfo->iSendBuffer + len) < DATA_BUFSIZE)
				{
					memcpy(&(SocketInfo->SendBuffer[SocketInfo->iSendBuffer]),buf,len);
					SocketInfo->iSendBuffer += len;
				}
				else
				{
					output_debug("CDownloadNet::WSASend Error:Buff溢出\r\n",WSAGetLastError());
				}
			}

			return -1;
		} 
		else // No error so update the byte count
		{
			total += SendBytes;

			if (bSendAsynBuf)
			{
				if (SendBytes < SocketInfo->iSendBuffer)
				{
					SocketInfo->iSendBuffer -= SendBytes;
					memmove(SocketInfo->SendBuffer, SocketInfo->SendBuffer + SendBytes, SocketInfo->iSendBuffer);
				}
				else
				{
					SocketInfo->iSendBuffer = 0;
				}
			}
		}
	}

	m_dwSendBytes		+= total;
	m_dwTotalSendBytes	+= total;



	return total;
}

void CDownloadNet::BgDownloadCanJumpMap()
{
	MapJumpInfo &mapjumpInfo = g_MapFinder.GetMapJumInfo();//mapjumpInfo游戏启动时初始化,以后不会改变

	string strCurMap = g_pGameMap->GetRealMapName();
	MapJumpInfo::iterator it = mapjumpInfo.find(strCurMap);
	if(it == mapjumpInfo.end())//这张地图没有任何出口
		return;

	//先把这张地图的所有出口加入队列
	VJumpPoint &vJumPonit = it->second;
	char szPath[MAX_PATH] = {0};

	for (size_t i = 0; i < vJumPonit.size(); i++)
	{	
		string strRealMap = g_MapXmlMgr.GetRealMap(vJumPonit[i].strDesMN.c_str());//转化为真实的物理存储地图名

		sprintf(szPath,"map\\%s-3d.nmp",strRealMap.c_str());
		__int64 iRequireHash = g_pStreamMgr->HashFileName(szPath);

		//优先读取-3d的地图,没有3D的则用普通的
		if(!g_pGfx->IsSupportTide() || !g_pStreamMgr->HasFileInServer(iRequireHash))
		{
			sprintf(szPath,"map\\%s.nmp",strRealMap.c_str());
			iRequireHash = g_pStreamMgr->HashFileName(szPath);
		}

		//不是本地图,而且没有请求过下载
		if(strRealMap != strCurMap && !g_pStreamMgr->IsInDownloadList(iRequireHash) && 
			g_pStreamMgr->HasFileInServer(iRequireHash) && !g_pStreamMgr->HasFileInLocalWpf(iRequireHash) &&
			m_MBgInDownloadFile.find(iRequireHash) == m_MBgInDownloadFile.end()
			)
		{
			BgSendDownloadFileRequest(iRequireHash,0,EP_NORMAL);
			//下载这张地图对应的小地图
			int iMinMapID = g_pMinMap->GetMinMapID(vJumPonit[i].strDesMN.c_str());
			bool bRtn = g_pTexMgr->GetTexFilePath(PACKAGE_timap,iMinMapID,szPath);

			if(bRtn)
			{
				iRequireHash = g_pStreamMgr->HashFileName(szPath);
				if(g_pStreamMgr->HasFileInServer(iRequireHash) && !g_pStreamMgr->HasFileInLocalWpf(iRequireHash))
				{
					BgSendDownloadFileRequest(iRequireHash,MAKELONG(PACKAGE_timap,iMinMapID),EP_NORMAL);
				}
				else
				{
					bRtn = g_pTexMgr->GetTexFilePath(PACKAGE_mmap,iMinMapID,szPath);
					iRequireHash = g_pStreamMgr->HashFileName(szPath);
					if(g_pStreamMgr->HasFileInServer(iRequireHash) && !g_pStreamMgr->HasFileInLocalWpf(iRequireHash))
					{
						BgSendDownloadFileRequest(iRequireHash,MAKELONG(PACKAGE_mmap,iMinMapID),EP_NORMAL);
					}
				}
			}
		}
	}

	m_bNeedDownloadCanJumpMap = false;
}


void CDownloadNet::OnClose(DWORD wParam,DWORD lParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;

	output_debug("CDownloadNet::OnClose");

	bool bConnected = m_buf[i]->bConnect;
	m_buf[i]->bConnect = false;

	Close(0);

	g_pStreamMgr->ClearDownloadingFilesList();
	g_pTexMgr->ClearAllNullTex();
	//return;

	//重连
	if (bConnected)
	{
		Sleep(200);
		Connect(0);
	}

	//MessageBox(NULL,"OnClose","",MB_OK);
}
void CDownloadNet::OnError(DWORD wParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;

	output_debug("CDownloadNet::OnError");

	bool bConnected = m_buf[i]->bConnect;
	m_buf[i]->bConnect = false;

	Close(i); //关闭连接
	return;

	//重连
	if (bConnected)
	{
		Sleep(200);
		Connect(0);
	}
	//MessageBox(NULL,"OnError","",MB_OK);
}

void CDownloadNet::OnConnect(DWORD wParam,DWORD lParam)
{
	//static int iTimes = 0;
	//static DWORD dwTime = GetTickCount();
	if( !WSAGETSELECTERROR(lParam) ) 
	{
		int i = GetSocketInfo(wParam);
		if(i == -1)
			return;

		//iTimes ++;
		//Close(i);
		//Connect(0);

		//char *strHeaders = "Accept:*/*\r\n""Content-Type:application/x-www-form-urlencoded\r\n";
		//for (int j = 0; j < 1000; j++)
		//{
		//	SendBuf(strHeaders,strlen(strHeaders));
		//}

		//if (iTimes >= 1000)
		//{
		//	DWORD dwTemp = GetTickCount() - dwTime;
		//	output_debug("%d,%d\r\n",iTimes,dwTemp);
		//	int j = 3;
		//}
		//return;
		output_debug("CDownloadNet::OnConnect");

		SOCKET_INFORMATION* SocketInfo = m_buf[i];
		SocketInfo->bConnect = true;

		if(SocketInfo->iSendBuffer )
		{
			Send(i,SocketInfo->SendBuffer,SocketInfo->iSendBuffer);
			SocketInfo->iSendBuffer = 0;
		}

		SocketInfo->dwLastConnectTime = GetTickCount();
		SocketInfo->iReConnectTimes = 0;
	}
}

int CDownloadNet::AsynSend(int iServer,char * buf,DWORD len)
{
	if(m_buf[iServer]->iSendBuffer + len < DATA_BUFSIZE)
	{
		memcpy(m_buf[iServer]->SendBuffer + m_buf[iServer]->iSendBuffer,buf,len);
		m_buf[iServer]->iSendBuffer += len;
	}

	return 0;
}

bool CDownloadNet::DealRead()
{
	DWORD RecvBytes = 0;
	DWORD Flags = 0;


	SOCKET_INFORMATION * SocketInfo = m_buf[0];
	if (!SocketInfo->bConnect)
	{
		return false;
	}

	SocketInfo->DataBuf.buf = m_pRecBuf;
	if(m_bBgLimitedDownload)
	{
		SocketInfo->DataBuf.len = min(MAX_RECBUF_LENJ_DOWNLOADFILE - m_dwAllRecDataBufLen_SaveToDisk, 256 * 1024);
	}
	else
	{
		SocketInfo->DataBuf.len = MAX_RECBUF_LENJ_DOWNLOADFILE;
	}

	Flags = 0;
	if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	{
		int iError = WSAGetLastError();
		if(iError != WSAEWOULDBLOCK)
		{
			//FILE *fp = fopen("DownLoadNetError.log","a+");
			//if(fp)
			//{
			//	fprintf(fp,"%s CDownloadNet WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
			//	fclose(fp);
			//}
			//output_debug("%s CDownloadNet WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
		}

		return false;
	} 
	else
	{
		if(RecvBytes == 0)
		{
			OnClose(0,0);
		}

		m_dwRecvBytes		+= RecvBytes;
		m_dwTotalRecvBytes	+= RecvBytes;

		SocketInfo->BytesRECV = RecvBytes;

		//ProcessBuffer(SocketInfo);

		// 收集当前收到的字节
		CLock lock(&m_CriSect);
		m_strAllRecDataBuf.append(m_pRecBuf, RecvBytes);
		m_dwAllRecDataBufLen += RecvBytes;
		m_dwAllRecDataBufLen_SaveToDisk += RecvBytes;

	}

	return true;
}
void CDownloadNet::OnRead(DWORD wParam,DWORD lParam)
{
 	return;

	DWORD RecvBytes = 0;
	DWORD Flags = 0;

	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;

	SOCKET_INFORMATION * SocketInfo = m_buf[i];
	SocketInfo->DataBuf.buf = m_pRecBuf;
	SocketInfo->DataBuf.len = MAX_RECBUF_LENJ_DOWNLOADFILE;

	Flags = 0;
	if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	{
		int iError = WSAGetLastError();
		if(iError != WSAEWOULDBLOCK)
		{
			//FILE *fp = fopen("DownLoadNetError.log","a+");
			//if(fp)
			//{
			//	fprintf(fp,"%s CDownloadNet WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
			//	fclose(fp);
			//}
			//output_debug("%s CDownloadNet WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
		}
	} 
	else
	{
		if(RecvBytes == 0)
		{
			OnClose(0,0);
		}

		m_dwRecvBytes		+= RecvBytes;
		m_dwTotalRecvBytes	+= RecvBytes;

		SocketInfo->BytesRECV = RecvBytes;

		//ProcessBuffer(SocketInfo);

		// 收集当前收到的字节
		CLock lock(&m_CriSect);
		m_strAllRecDataBuf.append(m_pRecBuf, RecvBytes);
		m_dwAllRecDataBufLen += RecvBytes;
		m_dwAllRecDataBufLen_SaveToDisk += RecvBytes;

	}
}
