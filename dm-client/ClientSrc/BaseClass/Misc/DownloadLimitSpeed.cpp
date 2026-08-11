#include "Net.h"
#include "Global/DebugTry.h"
#include "Global/PerfCheck.h"
#include "GameData/GameGlobal.h"
#include "Global/Interface/StreamInterface.h"
#include "NetData.h"
#include "GameControl/GameControl.h"
#include "GameData/LoginData.h"
#include "Global/md5.h"
#include  <io.h>
#include <fcntl.h>
#include <sys/stat.h>


#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif





CDownloadLimitSpeed::CDownloadLimitSpeed(HWND hWnd,DWORD dwMsg,long lEvent)
:CDownloadNet()
{
	m_bBgLimitedDownload = true;

	InitMember();

	m_iSaveFilePos = -1;

	m_bHaveDownLoadPlist = false;

	m_iSaveintFileHandle = -1;
	m_iSaveingFileLen = 0;
	m_iDlVersion = 0;

	m_pRecBuf = new char[DATA_BUFSIZE];

	Init(1,hWnd,dwMsg,lEvent);

	m_hThread = CreateThread(NULL,0,ProcessBuffer,this,0,&m_dwThreadID);
	//SetThreadPriority(m_hThread,THREAD_PRIORITY_ABOVE_NORMAL);

	m_iDlVersion = (int)GetPrivateProfileInt("Common","Version",0,"./DlVersion.dat");
	m_dlSpeed = (int)GetPrivateProfileInt("Config","BgDlSpeed",512,"./Config.ini");

}

CDownloadLimitSpeed::~CDownloadLimitSpeed()
{
	if (m_iSaveintFileHandle != -1)
	{
		_close(m_iSaveintFileHandle);
		m_iSaveintFileHandle = -1;
	}
}

DWORD WINAPI CDownloadLimitSpeed::ProcessBuffer(LPVOID lpData)
{
	CDownloadLimitSpeed* pThis = reinterpret_cast<CDownloadLimitSpeed*>(lpData);
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

		pThis->SendAsynBuf();

		if (pThis->m_dwRecvBytes * 1000 / (GetTickCount() + 1 - pThis->m_dwStartTime) > pThis->m_dlSpeed*1024)//限速m_dlSpeed K/s
		{
			Sleep(50);
			continue;
		}

		if(!pThis->DealRead())
		{
			Sleep(100);
			continue;
		}


		if (pThis->m_dwAllRecDataBufLen_SaveToDisk > 0)
		{
			CLock lock(&(pThis->m_CriSect));

			if (GetTickCount() - pThis->m_dwStartTime > 10000)//过了10s重新计算
			{
				pThis->m_dwStartTime = GetTickCount();
				pThis->m_dwRecvBytes = 0;
			}

			if (pThis->m_dwAllRecDataBufLen_SaveToDisk > 0)//再判一次,有可能在上面判了以后主线改变了这个值
			{
				//没有下载数据要处理的空闲时间用来保存文件,每次只保存一个文件,防止长时间卡在保存文件里面,导致主线程无法接收协议以及无法处理接收到的文件
				// 从头部开始解析

				if (pThis->m_iSaveingFileLen == -1)//非断点续传有包头,小于包头长度等待
				{
					if (pThis->m_dwAllRecDataBufLen_SaveToDisk < DOWNLOAD_HEADER_LEN)
					{
						continue;					
					}

					STDlNetMsgHeader *pHeader = (STDlNetMsgHeader *)(pThis->m_pRecBuf);
					WORD wMsgID = ::ntohs(pHeader->wMsgID);

					if (wMsgID != 3001)//非法包
					{
						pThis->m_bNeedReConnect = true;
						continue;
					}

					pThis->m_iSaveingFileLen = ::ntohl(pHeader->dwDataLen);
					pThis->m_iSaveingFileLen += sizeof(STDlNetMsgHeader);
				}


				int iBufLen = min(pThis->m_iSaveingFileLen - pThis->m_iSaveFilePos,500000);
				if (iBufLen > 0 && pThis->m_dwAllRecDataBufLen_SaveToDisk >= iBufLen)//包已经收完或到达500000
				{
					pThis->m_iSaveFilePos += iBufLen;

					bool bFinish = false;
					if (pThis->m_iSaveFilePos == pThis->m_iSaveingFileLen)
					{
						bFinish = true;
					}

					bool bRtn = pThis->SaveToFile(pThis->m_pRecBuf, iBufLen,bFinish); 
					if (!bRtn)
					{
						//pThis->m_bNeedReConnect = true;
						//后台下载出错了直接退出,不要重连
						pThis->m_bExited = true;

						output_debug("pThis->SaveToFile Fail,%s",pThis->m_strSavingFileName.c_str());
						continue;
					}


					pThis->m_dwAllRecDataBufLen_SaveToDisk -= iBufLen;
					if(bFinish)
					{
						pThis->m_iReceiveFiles ++;
					}

					pThis->m_iReceiveDataLen += iBufLen;

					if (pThis->m_dwAllRecDataBufLen_SaveToDisk > 0)
					{
						memmove(pThis->m_pRecBuf, pThis->m_pRecBuf + iBufLen, pThis->m_dwAllRecDataBufLen_SaveToDisk);
					}
					else if (pThis->m_dwAllRecDataBufLen_SaveToDisk < 0)//不应该出现
					{
						pThis->m_dwAllRecDataBufLen_SaveToDisk = 0;
					}
				}
			}
		}
	}

	pThis->m_dwThreadID = 0;

	return 0; 
}

bool CDownloadLimitSpeed::SaveToFile(const char *buf,int iLen,bool bFinish)
{
	int iPos = 0;
	if (m_iSaveintFileHandle == -1)
	{
		if (iLen < sizeof(SDLProtocalHeader))
		{
			return false;
		}

		iPos += sizeof(STDlNetMsgHeader);
		
		PSDLProtocalHeader pHeader = PSDLProtocalHeader(buf + iPos);
		iPos += sizeof(SDLProtocalHeader);

		BYTE byFilePathLen = BYTE(buf[iPos]);
		iPos += 1;

		if (iPos + byFilePathLen >= iLen)
		{
			return false;
		}

		//string szFileName;
		//szFileName.assign(buf + sizeof(SDLProtocalHeader) + sizeof(BYTE),byFilePathLen);
		iPos += byFilePathLen;


		string name = "";
		size_t lastsplashpos = m_strSavingFileName.rfind("\\");
		if (lastsplashpos == string::npos)
		{
			name = m_strSavingFileName;			
		}
		else
		{
			name = m_strSavingFileName.substr(lastsplashpos + 1,m_strSavingFileName.size() - lastsplashpos - 1);
		}


		string szFilePath = "../Data/DlTemp";
		CreateDirectory(szFilePath.c_str(),NULL);
		szFilePath = szFilePath + "/";

		szFilePath = szFilePath + name;

		//把总长度及文件在包中的起始位置保存到一个信息文件中,用于断点继传
		string strInfoPath = szFilePath + ".info";
		FILE *fp = fopen(strInfoPath.c_str(),"wb");
		if (!fp)
		{
			return false;
		}
		fwrite(&m_iSaveingFileLen,sizeof(int),1,fp);
		fwrite(&iPos,sizeof(int),1,fp);
		fclose(fp);


		szFilePath += ".dl";
		errno_t err = _sopen_s(&m_iSaveintFileHandle, szFilePath.c_str(), _O_BINARY | _O_RDWR | _O_CREAT | _O_TRUNC , _SH_DENYWR,  _S_IREAD | _S_IWRITE );
		if(err != 0 || m_iSaveintFileHandle == -1)
		{
			return false;
		}
		_lseeki64(m_iSaveintFileHandle,0,SEEK_END);

	}

	DWORD dwRtn = _write(m_iSaveintFileHandle,buf + iPos,iLen - iPos);

	if (bFinish)
	{
		if (m_iSaveintFileHandle != -1)
		{
			_close(m_iSaveintFileHandle);
			m_iSaveintFileHandle = -1;
		}

		OnFileDownedFinish();
	}

	return true;
}

void CDownloadLimitSpeed::SendDownLoadFile(const char *filepath,bool bReLoad)
{
	CLock lock(&m_CriSect);

	if (!filepath || !m_strSavingFileName.empty())
	{
		return;
	}

	char buf[128] = {0};

	__int64 iHash = g_pStreamMgr->HashFileName(filepath);
	PSDLProtocalHeader pHeader = (PSDLProtocalHeader)buf;

	memset(pHeader,0,sizeof(SDLProtocalHeader));

	pHeader->wFileCount = 1;//文件个数
	pHeader->wProtocal = 0x1001;//协议号
	pHeader->dwID = 0;//id
	pHeader->i64Hash = iHash;//第一个hash值
	pHeader->iWpf = 0;

	m_strSavingFileName = filepath;
	m_iSaveingFileLen = -1;
	m_iSaveFilePos = 0;

	if (!bReLoad)
	{
		string name = "";
		size_t iPos = m_strSavingFileName.rfind("\\");
		if (iPos == string::npos)
		{
			name = m_strSavingFileName;			
		}
		else
		{
			name = m_strSavingFileName.substr(iPos + 1,m_strSavingFileName.size() - iPos - 1);
		}

		string szFilePath = "../Data/DlTemp/";
		szFilePath = szFilePath + name;
		//总长度及文件在包中的起始位置保存在一个信息文件中,用于断点继传
		string strInfoPath = szFilePath + ".info";
		FILE *fp = fopen(strInfoPath.c_str(),"rb");
		if (fp)
		{
			fread(&m_iSaveingFileLen,sizeof(int),1,fp);
			fread(&m_iSaveFilePos,sizeof(int),1,fp);
			fclose(fp);

			szFilePath = szFilePath + ".dl";

			fp = fopen(szFilePath.c_str(),"rb");
			if(fp)
			{
				fseek(fp,0,SEEK_END);
				m_iSaveFilePos += ftell(fp);
				fclose(fp);
			}
			else
			{
				m_strSavingFileName.clear();
				DeleteFile(strInfoPath.c_str());
				return;
			}

			if (m_iSaveFilePos >= m_iSaveingFileLen)
			{
				OnFileDownedFinish();
				return;
			}

			//m_pSaveintFile = fopen(szFilePath.c_str(),"a+b");
			errno_t err = _sopen_s(&m_iSaveintFileHandle, szFilePath.c_str(), _O_BINARY | _O_RDWR, _SH_DENYWR,  _S_IREAD | _S_IWRITE );
			if(err != 0 || m_iSaveintFileHandle == -1)
			{
				return;
			}

			_lseeki64(m_iSaveintFileHandle,0,SEEK_END);
		}
	}


	PSDlProtocalBody pBody = (PSDlProtocalBody)(buf + sizeof(SDLProtocalHeader));

	pBody->dwID = 0;
	pBody->i64Hash = iHash;
	pBody->iWpf = 0;
	pBody->byPrior = EP_MOST_HIGH;
	pBody->dwStartPos =  m_iSaveFilePos;//起始下载位置,断点继传

	m_dwStartTime = GetTickCount();
	m_dwRecvBytes = 0;


	SendBuf(buf,sizeof(SDLProtocalHeader) + sizeof(SDlProtocalBody));
}

void CDownloadLimitSpeed::OnFileDownedFinish()
{
	string name = "";
	size_t iPos = m_strSavingFileName.rfind("\\");
	if (iPos == string::npos)
	{
		name = m_strSavingFileName;			
	}
	else
	{
		name = m_strSavingFileName.substr(iPos + 1,m_strSavingFileName.size() - iPos - 1);
	}


	string szFilePath = "../Data/DlTemp";
	szFilePath = szFilePath + "/";
	szFilePath = szFilePath + name;

	string szNewName = szFilePath;

	szFilePath += ".dl";


	DeleteFile(szNewName.c_str());
	string szInfo = szNewName + ".info";
	DeleteFile(szInfo.c_str());

	if (name == "plist.txt")
	{
		m_bHaveDownLoadPlist = true;
		MoveFile(szFilePath.c_str(), szNewName.c_str());
		TRY_BEGIN
		ParseFileList(szNewName.c_str());
		TRY_END
	}
	else
	{
		bool bFind = false;
		for (int i = 0; i < m_vDlFileList.size(); i++)
		{
			if (m_vDlFileList[i].szPathName == m_strSavingFileName)
			{
				bFind = true;
				char str[32] = {0};
				m_iDlVersion = m_vDlFileList[i].iDestVersion;

				char szMD5[33] = {0};
				MD5_CTX CMD5;
				CMD5.MD5File32(szMD5,szFilePath.c_str());

				//如果md校验错误,把这个文件删除
				if (strcmp(szMD5,m_vDlFileList[i].szMD5.c_str()) != 0)
				{
					DeleteFile(szFilePath.c_str());
					m_vDlFileList.clear();//清除,这次不下载了,下次启动的时候重下
				}
				else
				{
					sprintf(str,"%d",m_vDlFileList[i].iDestVersion);
					WritePrivateProfileString("Common","Version",str,"./DlVersion.dat");
					m_vDlFileList.erase(m_vDlFileList.begin() + i);
					MoveFile(szFilePath.c_str(), szNewName.c_str());
				}

				break;
			}
		}

		//下下来的文件不是我要的
		if (!bFind)
		{
			DeleteFile(szFilePath.c_str());
		}
	}

	m_strSavingFileName.clear();
}

bool CDownloadLimitSpeed::DealRead()
{
	if (m_dwAllRecDataBufLen_SaveToDisk >= DATA_BUFSIZE - 1000)
	{
		return true;//让上面处理已经接收的
	}

	DWORD RecvBytes = 0;
	DWORD Flags = 0;


	SOCKET_INFORMATION * SocketInfo = m_buf[0];
	if (!SocketInfo->bConnect)
	{
		return false;
	}
	SocketInfo->DataBuf.buf = m_pRecBuf + m_dwAllRecDataBufLen_SaveToDisk;
	SocketInfo->DataBuf.len = min(DATA_BUFSIZE - m_dwAllRecDataBufLen_SaveToDisk,10000);

	Flags = 0;
	if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	{
		int iError = WSAGetLastError();
		if(iError != WSAEWOULDBLOCK)
		{
			//output_debug("%s CDownloadLimitSpeed WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
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


		// 收集当前收到的字节
		EnterCriticalSection(&m_CriSect);
		m_dwAllRecDataBufLen_SaveToDisk += RecvBytes;
		LeaveCriticalSection(&m_CriSect);

		return true;
	}
}

void CDownloadLimitSpeed::SendAsynBuf()
{
	if (m_buf.size() <= 0 || !m_buf[0] || !(m_buf[0]->bConnect))
	{
		return;
	}

	CDownloadNet::SendAsynBuf();

	if (m_bHaveDownLoadPlist)
	{
		if (m_strSavingFileName.empty())
		{
			if (m_vDlFileList.size() > 0)
			{
				for (int i = 0; i < m_vDlFileList.size(); i++)
				{
					if (m_vDlFileList[i].iBeginiVersion == m_iDlVersion)
					{
						return SendDownLoadFile(m_vDlFileList[i].szPathName.c_str());
					}
				}
			}

			m_vDlFileList.clear();
			m_bExited = true;
			return;
		}
	}
}

void CDownloadLimitSpeed::ParseFileList(const char *filepath)
{
	DataStreamInterface * stream = g_pStreamMgr->OpenDataFile(filepath,false,true,true);
	if (!stream)
	{
		return;
	}

	string str;
	sDlFileList dlfile;
	char szTemp[256] = {0};
	char szAreaName[32] = {0};
	sprintf(szAreaName,"[%d]",g_Login.GetInnerAreaNo());

	while(!stream->eof())
	{
		str = stream->getLine(false);
		memset(szTemp,0,256);
		if(sscanf(str.c_str(),"%d=%d,%s",&dlfile.iBeginiVersion,&dlfile.iDestVersion,szTemp) != 3)
		{
			continue;
		}

		if (dlfile.iBeginiVersion >= m_iDlVersion)
		{
			dlfile.szPathName = szTemp;
			size_t iPos1 = dlfile.szPathName.find(",");
			if (iPos1 == string::npos)
			{
				continue;
			}

			size_t iPos2 = dlfile.szPathName.find(",",iPos1 + 1);
			if (iPos1 == string::npos)
			{
				continue;
			}

			dlfile.szMD5 = dlfile.szPathName.substr(iPos1 + 1,iPos2 - iPos1 - 1);
			dlfile.szArea = dlfile.szPathName.substr(iPos2 + 1,dlfile.szPathName.length() - iPos2 - 1);
		
			//szPathName最后改写,上面要用到
			dlfile.szPathName = dlfile.szPathName.substr(0,iPos1);

			if (dlfile.szArea == "ALL" || dlfile.szArea.find(szAreaName))
			{
				m_vDlFileList.push_back(dlfile);

				//为了保证下载内容合并时候的顺序以及启动的速度,每次启动只下载一个文件.
				break;
			}
		}
	}

	m_bHaveDownLoadPlist = true;

	SAFE_DELETE(stream);
}

void CDownloadLimitSpeed::OnRead(DWORD wParam,DWORD lParam)
{
	return;
	//DWORD RecvBytes = 0;
	//DWORD Flags = 0;

	//int i = GetSocketInfo(wParam);
	//if(i == -1)
	//	return;

	//SOCKET_INFORMATION * SocketInfo = m_buf[i];
	//SocketInfo->DataBuf.buf = m_pRecBuf;
	//SocketInfo->DataBuf.len = MAX_RECBUF_LENJ_DOWNLOADFILE;

	//Flags = 0;
	//if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	//{
	//	int iError = WSAGetLastError();
	//	if(iError != WSAEWOULDBLOCK)
	//	{
	//		//FILE *fp = fopen("DownLoadNetError.log","a+");
	//		//if(fp)
	//		//{
	//		//	fprintf(fp,"%s CDownloadLimitSpeed WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
	//		//	fclose(fp);
	//		//}
	//		//output_debug("%s CDownloadLimitSpeed WSARecv ERROR_NO:%d\n",g_Timer.GetDateTime(),iError);
	//	}
	//} 
	//else
	//{
	//	if(RecvBytes == 0)
	//	{
	//		OnClose(0,0);
	//	}

	//	m_dwRecvBytes		+= RecvBytes;
	//	m_dwTotalRecvBytes	+= RecvBytes;

	//	SocketInfo->BytesRECV = RecvBytes;

	//	//ProcessBuffer(SocketInfo);

	//	// 收集当前收到的字节
	//	EnterCriticalSection(&m_CriSect);
	//	m_strAllRecDataBuf.append(m_pRecBuf, RecvBytes);
	//	m_dwAllRecDataBufLen += RecvBytes;
	//	m_dwAllRecDataBufLen_SaveToDisk += RecvBytes;
	//	LeaveCriticalSection(&m_CriSect);

	//}
}

void CDownloadLimitSpeed::OnClose(DWORD wParam,DWORD lParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;

	bool bConnected = m_buf[i]->bConnect;
	m_buf[i]->bConnect = false;

	Close(0);
}

void CDownloadLimitSpeed::OnError(DWORD wParam)
{
	int i = GetSocketInfo(wParam);
	if(i == -1)
		return;

	bool bConnected = m_buf[i]->bConnect;
	m_buf[i]->bConnect = false;

	Close(i); //关闭连接
}
