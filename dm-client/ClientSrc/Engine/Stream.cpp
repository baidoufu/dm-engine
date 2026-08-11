#include "stdafx.h"
#include "Global/DebugTry.h"
#include "Stream.h"
#include <string>
#include <algorithm>
#include "Global/Interface/CallBackInterface.h"
#include  <io.h>
#include <direct.h>
#include "TexManager.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")


using namespace std;

#ifdef _DEBUG
#include "Global/new.h"
#define new DEBUG_NEW
#endif


void output_debug(char *format,...);


DWORD CStreamManager::m_dwCryptTable[MAX_WPF_HASH_BUFFF_SIZE];

string retString;//动态库中string返回到主程序不能使用函数里面的临时变量
//-----------------------------------------------------------------------
template <typename T> DataStream& DataStream::operator >>(T& val)
{
	read(static_cast<void*>(&val), sizeof(T));
}
//-----------------------------------------------------------------------
const char * DataStream::getLine(bool trimAfter)
{
	std::ostringstream str;
	size_t c = STREAM_TEMP_SIZE-1;
	// Keep looping while not hitting delimiter
	while (c == STREAM_TEMP_SIZE-1)
	{
		c = readLine(mTmpArea, STREAM_TEMP_SIZE-1);
		str << mTmpArea;
	}

	retString.assign(str.str());
	if (trimAfter)
	{
		const string delims = " \t\r";
		retString.erase(retString.find_last_not_of(delims)+1); // trim right
		retString.erase(0, retString.find_first_not_of(delims)); // trim left
	}

	return retString.c_str();
}
//-----------------------------------------------------------------------
CMemoryStream::CMemoryStream(void* pMem, size_t size, bool freeOnClose)
: DataStream()
{
	mData = mPos = static_cast<BYTE*>(pMem);
	mSize = size;
	mEnd = mData + mSize;
	mFreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------
CMemoryStream::CMemoryStream(DataStream& sourceStream, 
							 bool freeOnClose)
							 : DataStream()
{
	// Copy data from incoming stream
	mSize = sourceStream.size();
	mData = new BYTE[mSize];
	sourceStream.read(mData, mSize);
	mPos = mData;
	mEnd = mData + mSize;
	mFreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------
CMemoryStream::~CMemoryStream()
{
	close();
}
//-----------------------------------------------------------------------
size_t CMemoryStream::read(void* buf, size_t count)
{
	size_t cnt = count;
	// Read over end of memory?
	if (mPos + cnt > mEnd)
		cnt = mEnd - mPos;
	if (cnt == 0)
		return 0;

	memcpy(buf, mPos, cnt);
	mPos += cnt;
	return cnt;
}
//-----------------------------------------------------------------------
size_t CMemoryStream::readLine(char* buf, size_t maxCount, 
							   const char* delim)
{
	// Deal with both Unix & Windows LFs
	bool trimCR = false;
	if (delim && strstr(delim,"\n"))
	{
		trimCR = true;
	}

	size_t pos = strcspn((const char*)mPos, delim);
	if (pos > maxCount)
		pos = maxCount;

	// Make sure pos can never go past the end of the data 
	if(mPos + pos > mEnd) pos = mEnd - mPos; 

	if (pos > 0)
	{
		memcpy(buf, (const void*)mPos, pos);
	}

	// reposition pointer
	mPos += pos + 1;

	// Trim off trailing CR if this was a CR/LF entry
	if (trimCR && buf[pos-1] == '\r')
	{
		// terminate 1 character early
		--pos;
	}
	// terminate
	buf[pos] = '\0';


	return pos;
}
//-----------------------------------------------------------------------
size_t CMemoryStream::skipLine(const char * delim)
{
	size_t pos = strcspn( (const char*)mPos, delim);

	// Make sure pos can never go past the end of the data 
	if(mPos + pos > mEnd) pos = mEnd - mPos; 

	mPos += pos + 1;

	return pos;

}
//-----------------------------------------------------------------------
void CMemoryStream::skip(long count)
{
	size_t newpos = (size_t)( ( mPos - mData ) + count );
	assert( mData + newpos <= mEnd );        

	mPos = mData + newpos;
}
//-----------------------------------------------------------------------
void CMemoryStream::seek( size_t pos )
{
	assert( mData + pos <= mEnd );
	mPos = mData + pos;
}
//-----------------------------------------------------------------------
size_t CMemoryStream::tell(void) const
{
	//mData is start, mPos is current location
	return mPos - mData;
}
//-----------------------------------------------------------------------
bool CMemoryStream::eof(void) const
{
	return mPos >= mEnd;
}
//-----------------------------------------------------------------------
void CMemoryStream::close(void)    
{
	if (mFreeOnClose && mData)
	{
		delete [] mData;
		mData = 0;
	}
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
CFileStream::CFileStream(FILE* handle,bool freeOnClose)
: DataStream(), mFileHandle(handle)
{
	// Determine size
	fseek(mFileHandle, 0, SEEK_END);
	mSize = ftell(mFileHandle);
	fseek(mFileHandle, 0, SEEK_SET);
	mFreeOnClose = freeOnClose;
	mOff = mPos = 0;
}

//-----------------------------------------------------------------------
CFileStream::CFileStream(const string& name, FILE* handle,bool freeOnClose)
: DataStream(name), mFileHandle(handle)
{
	// Determine size
	fseek(mFileHandle, 0, SEEK_END);
	mSize = ftell(mFileHandle);
	fseek(mFileHandle, 0, SEEK_SET);
	mFreeOnClose = freeOnClose;
	mOff = mPos = 0;
}

CFileStream::CFileStream(FILE* handle,size_t off,size_t size,bool freeOnClose)
:DataStream(),mFileHandle(handle)
{
	mOff = off;
	mSize = size;
	mFreeOnClose = freeOnClose;
	fseek(mFileHandle,mOff,SEEK_SET); 
	mPos = 0;
}

//-----------------------------------------------------------------------
CFileStream::~CFileStream()
{
	close();
}
//-----------------------------------------------------------------------
size_t CFileStream::read(void* buf, size_t count)
{
	size_t cnt = count;
	// Read over end of memory?
	if (mPos + cnt >= mSize)
		cnt = mSize - mPos;
	if (cnt == 0)
		return 0;

	mPos += cnt;
	return fread(buf, 1,cnt,mFileHandle);
}

size_t CFileStream::readLine(char* buf, size_t maxCount, const char* delim)
{

	if (mSize <= mPos)
	{
		return 0;
	}

	// Have to buffer the data
	// since we have no read up to delimeter method

	// Deal with both Unix & Windows LFs
	bool trimCR = false;
	if (delim && strstr(delim,"\n"))
	{
		trimCR = true;
	}

	memset(buf,0,maxCount);

	size_t chunkSize = min(maxCount, (size_t)STREAM_TEMP_SIZE-1);
	chunkSize = min(chunkSize,mSize - mPos);
	size_t totalCount = 0;
	size_t readCount; 
	while (chunkSize && (readCount = fread(mTmpArea, 1,chunkSize, mFileHandle)))
	{
		// Terminate
		mTmpArea[readCount] = '\0';
		// Find first delimiter
		size_t pos = strcspn(mTmpArea, delim);

		if (pos < readCount)
		{
			// found terminator
			// reposition backwards
			fseek(mFileHandle, pos - readCount + 1, SEEK_CUR);
		}
		mPos += pos + 1;

		if (pos > 0)
		{
			// terminate early if CR found
			if (trimCR && mTmpArea[pos-1] == '\r')
			{
				--pos;
			}

			// Are we genuinely copying?
			if (buf)
			{
				memcpy(buf, (const void*)mTmpArea, pos);
				buf[pos] = '\0';
			}
			totalCount += pos;
		}

		if (pos < readCount || mSize <= mPos)
		{
			break;
		}

		// Adjust chunkSize for next time
		chunkSize = min(maxCount-totalCount, (size_t)STREAM_TEMP_SIZE-1);
		chunkSize = min(chunkSize,mSize - mPos);
	}
	return totalCount;
}

size_t CFileStream::skipLine(const char* delim)
{
	// Re-use readLine, but don't copy data
	char* nullBuf = 0;
	return readLine(nullBuf, 1024, delim);
}
void CFileStream::skip(long count)
{
	mPos += count;
	fseek(mFileHandle, count, SEEK_CUR);
}
void CFileStream::seek( size_t pos )
{
	mPos = pos;

	fseek(mFileHandle, mOff+pos, SEEK_SET);
}

size_t CFileStream::tell(void) const
{
	return (ftell( mFileHandle) - mOff);
}
bool CFileStream::eof(void) const
{
	if(feof(mFileHandle) != 0)
		return true;

	return (mPos >= mSize);
}

void CFileStream::close(void)
{
	if(mFreeOnClose)
	{
		if (mFileHandle)
		{
			fclose(mFileHandle);
			mFileHandle = 0;
		}
	}
	mSize = 0;
}

CWpfFile::CWpfFile()
{
	fp1 = NULL;
	fp2 = NULL;
	strName.clear();
	iDx = 0;

	pWpf = new CWpf();
}

CWpfFile::~CWpfFile()
{
	SAFE_DELETE(pWpf);
	if(fp1)
		fclose(fp1);
	if(fp2)
		fclose(fp2);
}

CStreamManager::CStreamManager()
{
	m_bNeedDownloadFileFromServer = false;
	m_iInDownloadFileCount = 0;
	m_bIsMultiClient = false;

	InitBuffer();

	string strDataDir = "..\\Data";
	if (g_pCallBack)
	{
		m_bIsMultiClient = g_pCallBack->IsMultiClientInSameDir();
		strDataDir = g_pCallBack->GetDataDir();

		sprintf(m_szConfigFile,"%s\\Config.ini",g_pCallBack->GetDataDir());
		sprintf(m_szOtherConfigFile,"%s\\Config\\OtherConfig.ini",g_pCallBack->GetDataDir());

		if (g_pCallBack->IsInReplay())
		{
			sprintf(m_szGameFile,"%s\\%s\\Game.ini",g_pCallBack->GetDataDir(),g_pCallBack->GetReplayDir());
		}
		else
		{
			sprintf(m_szGameFile,"%s\\Game.ini",g_pCallBack->GetDataDir());
			if (g_pCallBack->IsInRecord())
			{
				char szTemp[MAX_PATH];
				sprintf(szTemp,"%s\\%s",g_pCallBack->GetDataDir(),g_pCallBack->GetReplayDir());
				mkdir(szTemp);
				sprintf(szTemp,"%s\\%s\\Game.ini",g_pCallBack->GetDataDir(),g_pCallBack->GetReplayDir());
				DWORD dwAttr = GetFileAttributes(szTemp);
				BOOL b = SetFileAttributes(szTemp,dwAttr & (~FILE_ATTRIBUTE_READONLY));
				CopyFile(m_szGameFile,szTemp,FALSE);
				b = SetFileAttributes(szTemp,FILE_ATTRIBUTE_READONLY);
			}
		}


		//switch(g_pCallBack->GetParamGameType())
		//{
		//case 1:
		//	sprintf(m_szWebsiteFile,"%s/config/websitehf.ini",g_pCallBack->GetDataDir());
		//	break;
		//case 2:
		//case 3://和边锋一样
		//	sprintf(m_szWebsiteFile,"%s/config/websitebf.ini",g_pCallBack->GetDataDir());
		//	break;
		//default:// g_iParamGameType == 0或其它
		sprintf(m_szWebsiteFile,"%s/config/website.ini",g_pCallBack->GetDataDir());
		//break;
		//}
	}
	else
	{
		sprintf(m_szConfigFile,"%s\\Config.ini","..\\Data");
		sprintf(m_szGameFile,"%s\\Game.ini","..\\Data");
		sprintf(m_szOtherConfigFile,"%s\\Config\\OtherConfig.ini","..\\Data");
		sprintf(m_szWebsiteFile,"%s\\config\\website.ini","..\\Data");
	}

	strcpy(SECTION_INI,"");
	strcpy(SECTION_NAME,"Config");


	InitializeCriticalSection(&m_DlFileCriSect);
	InitializeCriticalSection(&m_SavingFileMapCriSect);
	InitializeCriticalSection(&m_WpfLock);

	if (!m_bIsMultiClient)
	{
		TRY_BEGIN
		MeargeDownloadWpf();
		TRY_END
	}
}

CStreamManager::~CStreamManager()
{
	clear();

	DeleteCriticalSection(&m_DlFileCriSect);
	DeleteCriticalSection(&m_SavingFileMapCriSect);
	DeleteCriticalSection(&m_WpfLock);
}

void CStreamManager::Init()
{
	string strDataDir = "..\\Data";
	if (g_pCallBack)
	{
		strDataDir = g_pCallBack->GetDataDir();
	}


	struct _finddata_t file;

	int iDx = 1;
	string strPath;
	intptr_t fhandle = 0;

	if (m_bNeedDownloadFileFromServer)	
	{
		strPath = strDataDir + "\\Texture*.hash";
		fhandle = _findfirst(strPath.c_str(), &file);
		if(fhandle != -1)
		{
			do
			{
				if(!(file.attrib & _A_SUBDIR))
				{ 
					strPath = strDataDir + "\\" + file.name; 
					AddHash(strPath.c_str(),m_VWpf_Texture,iDx);
					iDx ++;
				}
			} while(!_findnext(fhandle, &file));

			_findclose(fhandle);
		}


		strPath = strDataDir + "\\Data*.hash";
		fhandle = _findfirst(strPath.c_str(), &file);
		if(fhandle != -1)
		{
			do
			{
				if(!(file.attrib & _A_SUBDIR))
				{ 
					strPath = strDataDir + "\\" + file.name; 
					AddHash(strPath.c_str(),m_VWpf_Data,iDx);
					iDx ++;
				}
			} while(!_findnext(fhandle, &file));

			_findclose(fhandle);
		}
	}



	strPath = strDataDir + "\\TexturePatch*.wpf";
	fhandle = _findfirst(strPath.c_str(), &file);
	iDx = 100;
	if(fhandle != -1)
	{
		do
		{
			if(!(file.attrib & _A_SUBDIR))
			{ 
				strPath = strDataDir + "\\" + file.name; 
				Add(strPath.c_str(),m_VWpf_Texture,iDx);
				iDx ++;
			}
		} while(!_findnext(fhandle, &file));

		_findclose(fhandle);
	}

	strPath = strDataDir + "\\Texture*.wpf";
	fhandle = _findfirst(strPath.c_str(), &file);
	if(fhandle != -1)
	{
		do
		{
			if(!(file.attrib & _A_SUBDIR))
			{ 
				strPath = strDataDir + "\\" + file.name; 
				Add(strPath.c_str(),m_VWpf_Texture,iDx);
				iDx ++;
			}
		} while(!_findnext(fhandle, &file));

		_findclose(fhandle);
	}


	iDx = 200;//Data*.wpf从100开始
	strPath = strDataDir + "\\DataPatch*.wpf";
	fhandle = _findfirst(strPath.c_str(), &file);
	if(fhandle != -1)
	{
		do
		{
			if(!(file.attrib & _A_SUBDIR))
			{ 
				strPath = strDataDir + "\\" + file.name; 
				Add(strPath.c_str(),m_VWpf_Data,iDx);
				iDx ++;
			}
		} while(!_findnext(fhandle, &file));

		_findclose(fhandle);
	}


	strPath = strDataDir + "\\Data*.wpf";
	fhandle = _findfirst(strPath.c_str(), &file);
	if(fhandle != -1)
	{
		do
		{
			if(!(file.attrib & _A_SUBDIR))
			{ 
				strPath = strDataDir + "\\" + file.name; 
				Add(strPath.c_str(),m_VWpf_Data,iDx);
				iDx ++;
			}
		} while(!_findnext(fhandle, &file));

		_findclose(fhandle);
	}



}

void   CStreamManager::MeargeDownloadWpf()
{
	char szPath[MAX_PATH] = {0};
	_fullpath(szPath,"../",MAX_PATH);

	int iPathLen = strlen(szPath);
	if (iPathLen > 0)
	{
		szPath[iPathLen - 1] = 0;//去掉最后一个"\";
	}
	else
	{
		return;
	}

	string strInstallDir = szPath;


	struct _finddata_t file;
	intptr_t fhandle = 0;

	string strPath = strInstallDir + "\\Data\\DlTemp\\*.wpf";
	fhandle = _findfirst(strPath.c_str(), &file);
	if(fhandle != -1)
	{
		do
		{
			if(!(file.attrib & _A_SUBDIR))
			{ 
				strPath = strInstallDir + "\\Data\\DlTemp\\" + file.name; 


				CWpf *pSource = new CWpf();
				if(!pSource->WpfOpen(strPath.c_str(),0,EWOT_LISTDIR))
				{
					SAFE_DELETE(pSource);
					continue;
				}

				if (pSource->GetWpfType() != WS_GAME_ID && pSource->GetWpfType() != 0)//文件格式不匹配
				{
					pSource->WpfClose();
					SAFE_DELETE(pSource);
					DeleteFile(strPath.c_str());
					continue;
				}

				PWpfFileList pList = pSource->GetRoot();
				if (pList)
				{
					if (pList->MDirs.size() > 0)
					{
						MList::iterator it;
						for (it = pList->MDirs.begin();it != pList->MDirs.end();it++)
						{//所有的文件夹
							std::string::size_type iPos = it->first.find(".wpf");
							if (iPos == std::string::npos)
							{
								pSource->ExportDirFromList(strInstallDir.c_str(),it->second);
							}
							else
							{
								std::string strWpf = strInstallDir + "\\Data\\";
								strWpf += it->first;

								CWpf *pDestWpf = new CWpf();

								if(!pDestWpf->WpfOpen(strWpf.c_str(), 0, EWOT_RDWR))
								{
									if(!pDestWpf->WpfCreate(strWpf.c_str()))
									{
										SAFE_DELETE(pDestWpf);
										pSource->WpfClose();
										SAFE_DELETE(pSource);
										continue;
									}
								}

								pDestWpf->SetExpendLength(20*1024*1024);
								pDestWpf->ImportSubDirFromList(pSource,it->second,pDestWpf->GetRoot(),false);
								pDestWpf->WpfClose();
								SAFE_DELETE(pDestWpf);
							}
						}
					}

					if (pList->MFiles.size() > 0)
					{
						MSFCB::iterator it;
						//所有的文件
						for (it = pList->MFiles.begin();it != pList->MFiles.end();it++)
						{
								string strTemp = strInstallDir + "\\";
								strTemp += it->first.c_str();
								pSource->ExportFile(it->second,strTemp.c_str());		
						}
					}
				}

				pSource->WpfClose();
				SAFE_DELETE(pSource);

				DeleteFile(strPath.c_str());

			}
		} while(!_findnext(fhandle, &file));


		_findclose(fhandle);
	}
}


void CStreamManager::SetNeedDownloadFileFromServer(bool val) 
{ 
	m_bNeedDownloadFileFromServer = val; 
}

DataStreamInterface* CStreamManager::CreateMemoryStream(void* pMem, size_t size, bool freeOnClose)
{
	 return (new CMemoryStream(pMem,size,freeOnClose));
}

DataStreamInterface* CStreamManager::OpenTextureFile(const char * filename,bool bThread,bool bFromDir,DWORD dwID,eReadPrior ePrior)
{
TRY_BEGIN

	DataStream* pStream = NULL;

#ifdef _DEBUG
	string path = "../Data";
	if (g_pCallBack)
	{
		path = g_pCallBack->GetDataDir();
	}

	path = path + "\\" + filename;
	FILE* fp = fopen(path.c_str(),"rb");
	if(fp)
	{
		pStream = new CFileStream(fp,true);
	}

	if (!bFromDir && !pStream)
	{
		VWpfFile::iterator itr;
		CWpfFile* pWpffile = NULL;
		CLock lock(&m_WpfLock);

		for(itr = m_VWpf_Texture.begin();itr != m_VWpf_Texture.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FileOpen(filename,bThread?EFT_THREAD:EFT_THREAD2))
			{
				pStream = new CFileStream(bThread?pWpffile->fp2:pWpffile->fp1,(long)(pWpffile->pWpf->GetFileOffset(bThread?EFT_THREAD:EFT_THREAD2)),pWpffile->pWpf->FileLength(bThread?EFT_THREAD:EFT_THREAD2));
				break;
			}
		}
	}
#else
	if (!bFromDir)
	{
		VWpfFile::iterator itr;
		CWpfFile* pWpffile = NULL;
		CLock lock(&m_WpfLock);

		for(itr = m_VWpf_Texture.begin();itr != m_VWpf_Texture.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FileOpen(filename,bThread?EFT_THREAD:EFT_THREAD2))
			{
				pStream = new CFileStream(bThread?pWpffile->fp2:pWpffile->fp1,(long)(pWpffile->pWpf->GetFileOffset(bThread?EFT_THREAD:EFT_THREAD2)),pWpffile->pWpf->FileLength(bThread?EFT_THREAD:EFT_THREAD2));
				break;
			}
		}
	}
	else
	{
		string path = "../Data";
		if (g_pCallBack)
		{
			path = g_pCallBack->GetDataDir();
		}

		path = path + "\\" + filename;
		FILE* fp = fopen(path.c_str(),"rb");
		if(fp)
		{
			pStream = new CFileStream(fp,true);
		}
	}
#endif


	if(!pStream && m_bNeedDownloadFileFromServer)//要下载文件,要进入临界区
	{
		SendDownloadFile(filename,dwID,0,ePrior);
	}

	return pStream;
TRY_END_RETURN(NULL)
}

DataStreamInterface* CStreamManager::OpenDataFile(const char * filename,bool bThread,bool bFromDir,bool bFullPath,eReadPrior ePrior)
{
TRY_BEGIN


	DataStream* pStream = NULL;

#ifdef _DEBUG
	string path;

	if (!bFullPath)
	{
		path = "../Data";
		if (g_pCallBack)
		{
			path = g_pCallBack->GetDataDir();
		}

		path = path + "\\" + filename;
	}
	else
	{
		path = filename;
	}

	FILE* fp = fopen(path.c_str(),"rb");
	if(fp)
	{
		pStream = new CFileStream(fp,true);
	}

	if (!bFromDir && !pStream)
	{
		VWpfFile::iterator itr;
		CWpfFile* pWpffile = NULL;
		CLock lock(&m_WpfLock);

		for(itr = m_VWpf_Data.begin();itr != m_VWpf_Data.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FileOpen(filename,bThread?EFT_THREAD:EFT_THREAD2))
			{
				pStream = new CFileStream(bThread?pWpffile->fp2:pWpffile->fp1,(long)(pWpffile->pWpf->GetFileOffset(bThread?EFT_THREAD:EFT_THREAD2)),pWpffile->pWpf->FileLength(bThread?EFT_THREAD:EFT_THREAD2));
				break;
			}
		}
	}
#else
	if (!bFromDir)
	{
		VWpfFile::iterator itr;
		CWpfFile* pWpffile = NULL;
		CLock lock(&m_WpfLock);

		for(itr = m_VWpf_Data.begin();itr != m_VWpf_Data.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FileOpen(filename,bThread?EFT_THREAD:EFT_THREAD2))
			{
				pStream = new CFileStream(bThread?pWpffile->fp2:pWpffile->fp1,(long)(pWpffile->pWpf->GetFileOffset(bThread?EFT_THREAD:EFT_THREAD2)),pWpffile->pWpf->FileLength(bThread?EFT_THREAD:EFT_THREAD2));
				break;
			}
		}
	}
	else
	{
		string path;

		if (!bFullPath)
		{
			string path = "../Data";
			if (g_pCallBack)
			{
				path = g_pCallBack->GetDataDir();
			}

			path = path + "\\" + filename;
		}
		else
		{
			path = filename;
		}

		FILE* fp = fopen(path.c_str(),"rb");
		if(fp)
		{
			pStream = new CFileStream(fp,true);
		}
	}
#endif


	if(!pStream && m_bNeedDownloadFileFromServer)//要下载文件,要进入临界区
	{
		SendDownloadFile(filename,0,0,ePrior);
	}


	return pStream;
TRY_END_RETURN(NULL)
}

bool CStreamManager::HasFileInServer(const char * filename)
{
    if (!filename)
    {
		return false;
	}

	return HasFileInServer(HashFileName(filename));
}

bool CStreamManager::HasFileInLocalWpf(const char * filename)
{
	if (!filename)
	{
		return false;
	}

	TRY_BEGIN
		CLock lock(&m_WpfLock);
		return HasFileInLocalWpf(HashFileName(filename));

	TRY_END_RETURN(false);
}

bool CStreamManager::HasFileInServer(__int64 iHash)
{
	TRY_BEGIN

		map<__int64,BYTE>::iterator itr = m_MFileHash.find(iHash);
		if (itr == m_MFileHash.end())
		{
			return false;
		}

		return true;

	TRY_END_RETURN(false)
}

bool CStreamManager::HasFileInLocalWpf(__int64 iHash)
{
	TRY_BEGIN
		VWpfFile::iterator itr;
		CWpfFile* pWpffile = NULL;
		CLock lock(&m_WpfLock);

		for(itr = m_VWpf_Data.begin();itr != m_VWpf_Data.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FindFile_PFCB1(iHash))
			{
				return true;
			}
		}

		for(itr = m_VWpf_Texture.begin();itr != m_VWpf_Texture.end();itr++)
		{
			pWpffile = *itr;
			if(!pWpffile) 
			{
				continue;
			}

			if(pWpffile->pWpf->FindFile_PFCB1(iHash))
			{
				return true;
			}
		}

		return false;

	TRY_END_RETURN(false);
}

bool CStreamManager::Add(const char* strFilePath,VWpfFile & vWpf,int iDx)
{
TRY_BEGIN

   if(!strFilePath)
   {
 	   return false;
   }

   string szPath = strFilePath;
   for(int i = 0; i < vWpf.size(); i++)
   {
	   if (vWpf[i] && strFilePath == vWpf[i]->strPath)
	   {
		   return false;
	   }
   }		


	FILE* fp1 = fopen(strFilePath,"rb");
	FILE* fp2 = fopen(strFilePath,"rb");

	if(!fp2)
	{
		return false;
	}


	CWpfFile* wpffile = NULL;
	wpffile = new CWpfFile();

	DWORD dwTime = GetTickCount();

	DWORD dwOpenType = EWOT_READONLY;
	if (m_bNeedDownloadFileFromServer && !m_bIsMultiClient)
	{
		dwOpenType = EWOT_RDWR | EWOT_LISTFCBHASH;
	}

	if(!wpffile->pWpf->WpfOpen(strFilePath,0,dwOpenType))
	{
		//打开失败先创建一个空文件
		CWpf *pWpf = new CWpf();
		if (pWpf)
		{
			pWpf->WpfCreate(strFilePath,1024*1024,false);
			pWpf->WpfClose();
			SAFE_DELETE(pWpf);
		}

		if(!wpffile->pWpf->WpfOpen(strFilePath,0,dwOpenType))
		{
			SAFE_DELETE(wpffile);
			fclose(fp1);
			fclose(fp2);
			return false ;
		}
	}

	DWORD dwExandLen = 20 * 1024 *1024;
	wpffile->pWpf->SetExpendLength(dwExandLen);

	char strTemp[128] = {0};
	sprintf(strTemp,"OpenWpfFileTime:%u",GetTickCount() - dwTime);
	OutputDebugString(strTemp);


	string path = strFilePath;
	replace( path.begin(), path.end(), '\\', '/' );
	size_t i = path.find_last_of('/');
	if (i == string::npos)
	{
		wpffile->strName = path;
	}
	else
	{
		wpffile->strName = path.substr(i+1, path.size() - i - 1);
	}

	wpffile->fp1 = fp1;
	wpffile->fp2 = fp2;
	wpffile->strPath.assign(strFilePath);
	wpffile->iDx = iDx;

	vWpf.push_back(wpffile); //插入表


	return true;
TRY_END_RETURN(false)
}

bool CStreamManager::AddHash(const char* strFilePath,VWpfFile & vWpf,int iDx)
{
TRY_BEGIN
	FILE* fp = fopen(strFilePath,"rb");

	if(!fp)
	{
		return false;
	}

	DWORD dwMagic = 0;
	DWORD dwVerson = 0;
	DWORD dwSize = 0;
	DWORD dwRev = 0;
	fread(&dwMagic,sizeof(DWORD),1,fp);
	if (dwMagic != 'hash')
	{
		return false;
	}
	fread(&dwVerson,sizeof(DWORD),1,fp);
	fread(&dwSize,sizeof(DWORD),1,fp);
	fread(&dwRev,sizeof(DWORD),1,fp);


	char readBuf[8000];

	int iReadItems = 0;
	while((iReadItems = fread(readBuf,sizeof(__int64),1000,fp)) > 0)
	{
		for (int i = 0; i < iReadItems && i < 1000; i++)
		{
			m_MFileHash[Conv_INT64(readBuf + i * sizeof(__int64))] = (BYTE)iDx;
		}
	}

	fclose(fp);

	string szPath = strFilePath;
	size_t pos = szPath.find_last_of(".");
	if (pos != string::npos)
	{
		szPath = szPath.substr(0,pos);
		//如果不存在,先创建一个wpf文件
		CWpf *pWpf = new CWpf();
		if (pWpf)
		{
			pWpf->WpfCreate(szPath.c_str(),1024*1024,true);
			pWpf->WpfClose();
			SAFE_DELETE(pWpf);
		}

		Add(szPath.c_str(),vWpf,iDx);
	}


	return true;
TRY_END_RETURN(false)
}

void CStreamManager::clear()
{
TRY_BEGIN
	while(!m_VWpf_Texture.empty())
	{
		CWpfFile* wpffile = m_VWpf_Texture.back();
		m_VWpf_Texture.pop_back();

		if(wpffile)
		{
			SAFE_DELETE(wpffile);
		}
	}
	while(!m_VWpf_Data.empty())
	{
		CWpfFile* wpffile = m_VWpf_Data.back();
		m_VWpf_Data.pop_back();

		if(wpffile)
		{
			SAFE_DELETE(wpffile);
		}
	}
TRY_END
}

bool CStreamManager::InitBuffer()
{
	DWORD seed = 0x00100001;
	DWORD index1 = 0;
	DWORD index2 = 0;
	DWORD temp1 = 0,temp2 = 0;
	int i = 0;

	memset(m_dwCryptTable,0,sizeof(m_dwCryptTable));
	for(index1 = 0; index1 < 0x100; index1++)
	{
		for (index2 = index1,i = 0; i < 5; i++,index2 += 0x100)
		{
			seed = (seed * 125 + 3)  % 0x2AAAAB;
			temp1 = (seed & 0xFFFF) << 0x10;
			seed = (seed * 125 + 3)  % 0x2AAAAB;
			temp1 = (seed & 0xFFFF);
			m_dwCryptTable[index2] = (temp1 | temp2);			
		}
	}

	return true;
}

inline DWORD CStreamManager::HashString(const char * pStr,DWORD type)
{
	DWORD seed1 = 0x7FED7FED;
	DWORD seed2 = 0xEEEEEEEE;
	DWORD ch;			/* One key character */

	while(*pStr != 0)
	{
		ch = toupper(*pStr++);
		seed1 = m_dwCryptTable[(type << 8) + ch] ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}

	return seed1;	
}

inline __int64 CStreamManager::HashFileName(const char * filename)
{
	DWORD o1 = HashString(filename,1);
	DWORD o2 = HashString(filename,2);

	return ( (((__int64)(o1)) << 32) + o2);
}

void  CStreamManager::PreDownLoadTex(const char *strDir,int iPack,int iBeginID,int iEndID,eReadPrior ePrior)
{
	if (!m_bNeedDownloadFileFromServer || !strDir || ePrior == EP_DONT_DOWNLOAD)
	{
		return;
	}

	EnterCriticalSection(&m_DlFileCriSect);

	char sTexFile[256] = {0};

	VDlProtocalBody VBody;
	DlProtocalBody body;
	for(int i = iBeginID; i < iEndID;i++)
	{
		sprintf(sTexFile,"%s\\%03d\\%05d.tex",strDir,i/100,i);

		__int64 iRequireHash = HashFileName(sTexFile);

		//如果这个文件本身就不存在的,不要向下载服务器请求
		map<__int64,BYTE>::iterator itr = m_MFileHash.find(iRequireHash);
		if (itr == m_MFileHash.end())
		{
			continue;
		}

		//正在请求中
		if (m_MInDownloadFile.find(iRequireHash) != m_MInDownloadFile.end())
		{
			continue;
		}

		body.dwID = MAKELONG(i,iPack);
		body.i64Hash = iRequireHash;
		body.iWpf = itr->second;
		body.byPrior = (BYTE)ePrior;
		body.dwTickCount = GetTickCount();

		m_MInDownloadFile[iRequireHash] = body;

		VBody.push_back(body);
	}

	LeaveCriticalSection(&m_DlFileCriSect);

	if (VBody.size() <= 0)
	{
		return;
	}

	SendDownloadFile(VBody);
}

void  CStreamManager::SendDownloadFile(const char* strFileName,DWORD dwID,int iRev,eReadPrior ePrior)
{
	if (!m_bNeedDownloadFileFromServer || !strFileName || ePrior == EP_DONT_DOWNLOAD)
	{
		return;
	}

	CLock lock(&m_DlFileCriSect);

	__int64 iRequireHash = HashFileName(strFileName);

	//如果这个文件本身就不存在的,不要向下载服务器请求
	map<__int64,BYTE>::iterator itr = m_MFileHash.find(iRequireHash);
	if (itr == m_MFileHash.end())
	{
		return;
	}
	//正在请求中
	if (m_MInDownloadFile.find(iRequireHash) != m_MInDownloadFile.end())
	{
		return;
	}


	VDlProtocalBody VBody;
	DlProtocalBody body;

	body.dwID = dwID;
	body.i64Hash = iRequireHash;
	body.iWpf = itr->second;
	body.byPrior = (BYTE)ePrior;
	body.dwTickCount = GetTickCount();

	m_MInDownloadFile[iRequireHash] = body;

	m_iInDownloadFileCount = m_MInDownloadFile.size();

	VBody.push_back(body);

	//大地表处理下载中的时候使用一张默认的图代替,免得黑屏
	if (dwID > 0)
	{
		int iPack = HIWORD(dwID);
		if (iPack >= PACKAGE_Tiles && iPack <= PACKAGE_Tiles10)
		{
			g_pETexMgr->OnSendDownloadTex(dwID);
		}
	}


	//char str[128];
	//char tmp_d[64];
	//char tmp_t[64];
	//_tzset();
	//_strdate(tmp_d);
	//_strtime(tmp_t);
	//sprintf(str,"%s\t%s",tmp_d,tmp_t);

	//output_debug("Send:%s %I64d %d %s\r\n",str,body.i64Hash,ePrior,strFileName);


	SendDownloadFile(VBody);
}

void  CStreamManager::SendDownloadFile(VDlProtocalBody &VBody)
{
	if (VBody.size() <= 0)
	{
		return;
	}

	static char buf[2048] = {0};

	PSDLProtocalHeader pHeader = (PSDLProtocalHeader)buf;

	memset(pHeader,0,sizeof(SDLProtocalHeader));

	pHeader->wFileCount = VBody.size();//文件个数
	pHeader->wProtocal = 0x1001;//协议号
	pHeader->dwID = 0;//id
	pHeader->i64Hash = 0;//第一个hash值
	pHeader->iWpf = 0;

	PSDlProtocalBody pBody = (PSDlProtocalBody)(buf + sizeof(SDLProtocalHeader));

	for (size_t i = 0; i < VBody.size(); i++)
	{
		pBody->dwID = VBody[i].dwID;
		pBody->i64Hash = VBody[i].i64Hash;
		pBody->iWpf = VBody[i].iWpf;
		pBody->dwStartPos = VBody[i].dwStartPos;
		pBody->byPrior = VBody[i].byPrior;

		pBody ++;
	}

	g_pCallBack->SendDownloadFile(buf,sizeof(SDLProtocalHeader) + sizeof(SDlProtocalBody) * VBody.size());
}

bool  CStreamManager::OnDownloadedFile(const char *buf,int iLen)
{
	if (!buf || iLen < sizeof(SDLProtocalHeader))
	{
		return false;
	}

	PSDLProtocalHeader pHeader = PSDLProtocalHeader(buf);

	BYTE byFilePathLen = BYTE(buf[sizeof(SDLProtocalHeader)]);
	int iPos = sizeof(SDLProtocalHeader) + sizeof(BYTE) + byFilePathLen;

	if (iPos >= iLen)
	{
		return false;
	}

	string szFileName;
	szFileName.assign(buf + sizeof(SDLProtocalHeader) + sizeof(BYTE),byFilePathLen);
	//output_debug("OnDownloadedFile  %s:%d/%d\r\n",szFileName.c_str(),iLen - iPos,iLen);

	//校验一下hash值
	__int64 iHash = HashFileName(szFileName.c_str());
	if (iHash != pHeader->i64Hash)
	{
		output_debug("Hash校验错误:%I64d,%I64d,%s",iHash,pHeader->i64Hash,szFileName.c_str());
		return false;
	}
	//校验一下最后四个字节
	if (pHeader->dwLastDWORD != -1 && iLen - iPos >= 4)
	{
		DWORD dwLoastDWORD = *((DWORD*)(buf + iLen - 4));
		if (pHeader->dwLastDWORD != dwLoastDWORD)
		{
			output_debug("数据校验错误:%I64d,%s",iHash,szFileName.c_str());
			return false;
		}
	}


	DWORD dwID = 0;
	int iWpf = -1;


	EnterCriticalSection(&m_DlFileCriSect);

	map<__int64,DlProtocalBody>::iterator itr_hash = m_MInDownloadFile.find(pHeader->i64Hash);
	if (itr_hash == m_MInDownloadFile.end())//收到无效的或重复的
	{
		output_debug("收到的文件Hash错误:%I64d,",pHeader->i64Hash);
		LeaveCriticalSection(&m_DlFileCriSect);
		return true;
	}
	else
	{
		EnterCriticalSection(&m_SavingFileMapCriSect);
		m_MInSavingFile[pHeader->i64Hash] = itr_hash->second;
		LeaveCriticalSection(&m_SavingFileMapCriSect);

		dwID = itr_hash->second.dwID;
		iWpf = itr_hash->second.iWpf;
		m_MInDownloadFile.erase(itr_hash);
	}


	//char str[128];
	//char tmp_d[64];
	//char tmp_t[64];
	//_tzset();
	//_strdate(tmp_d);
	//_strtime(tmp_t);
	//sprintf(str,"%s\t%s",tmp_d,tmp_t);

	//output_debug("Receive:%s %I64d %s\r\n",str,pHeader->i64Hash,szFileName.c_str());


	m_iInDownloadFileCount = m_MInDownloadFile.size();

	LeaveCriticalSection(&m_DlFileCriSect);


TRY_BEGIN

	if (dwID > 0)//图片
	{
		g_pETexMgr->OnDownloadTex(dwID,buf + iPos,iLen - iPos);
	}
	else//mask资源要优先保存到图包,因为有其它地方急着要用图包,mask要在背景图片之前读到,读的时候是到图包里去读的,map文件不保存,但要先更新地图,为什么不保存到图包里再更新地图,是多开的时候后面的无法保存到图包
	{
		if(strstr(szFileName.c_str(),"mask\\") || strstr(szFileName.c_str(),"map\\"))
		{
			CWpfFile* pWpffile = NULL;
			for(VWpfFile::iterator itr = m_VWpf_Data.begin();itr != m_VWpf_Data.end();itr++)
			{
				pWpffile = *itr;
				if(pWpffile && pWpffile->iDx == iWpf) 
				{
					if (!strstr(szFileName.c_str(),"map\\"))
					{
						CLock lock(&m_WpfLock);
						pWpffile->pWpf->CreateFileFromBuff(szFileName.c_str(),buf + iPos,iLen - iPos,true,false,(pHeader->dwAttribute & EFA_COMPRESS) != 0 && (pHeader->dwAttribute & EFA_MEARGE_UNCOMPRESS) != 0);
					}

					g_pCallBack->OnDownloadedDataFile(iWpf,dwID,pHeader->i64Hash,szFileName.c_str(),buf + iPos,iLen - iPos,(pHeader->dwAttribute & EFA_COMPRESS) != 0);
					break;
				}
			}
		}
	}

TRY_END_RETURN(false)

	return true;
}

bool  CStreamManager::SaveDownloadedFileToWpf(const char *buf,int iLen,PSDlProtocalBody pBody)
{
	if (!buf || iLen < 32)
	{
		return false;
	}


	PSDLProtocalHeader pHeader = PSDLProtocalHeader(buf);

	BYTE byFilePathLen = BYTE(buf[sizeof(SDLProtocalHeader)]);
	int iPos = sizeof(SDLProtocalHeader) + sizeof(BYTE) + byFilePathLen;

	if (iPos >= iLen)
	{
		return false;
	}


	string szFileName;
	szFileName.assign(buf + sizeof(SDLProtocalHeader) + sizeof(BYTE),byFilePathLen);


	//output_debug("SaveDownloadedFileToWpf %s:%d/%d\r\n",szFileName.c_str(),iLen - iPos,iLen);
	DWORD dwID = 0;
	int iWpf = -1;

	if (pBody)
	{
		dwID = pBody->dwID;
		iWpf = pBody->iWpf;
	}
	else
	{
		CLock lock(&m_SavingFileMapCriSect);

		map<__int64,DlProtocalBody>::iterator itr_hash = m_MInSavingFile.find(pHeader->i64Hash);
		if (itr_hash == m_MInSavingFile.end())
		{
			return true;
		}
		else
		{
			dwID = itr_hash->second.dwID;
			iWpf = itr_hash->second.iWpf;
			m_MInSavingFile.erase(itr_hash);
		}
	}


	TRY_BEGIN

	CWpfFile* pWpffile = NULL;
	if (dwID > 0)//图片资源
	{
		for(VWpfFile::iterator itr = m_VWpf_Texture.begin();itr != m_VWpf_Texture.end();itr++)
		{
			pWpffile = *itr;
			if(pWpffile && pWpffile->iDx == iWpf) 
			{
				CLock lock(&m_WpfLock);
				pWpffile->pWpf->CreateFileFromBuff(szFileName.c_str(),buf + iPos,iLen - iPos,false,false,(pHeader->dwAttribute & EFA_COMPRESS) != 0 && (pHeader->dwAttribute & EFA_MEARGE_UNCOMPRESS) != 0);
				break;
			}
		}
	}
	else if(!strstr(szFileName.c_str(),"mask\\"))//mask资源在OnDownloadedFile的时候已经及时保存到wpf里了
	{
		CWpfFile* pWpffile = NULL;
		for(VWpfFile::iterator itr = m_VWpf_Data.begin();itr != m_VWpf_Data.end();itr++)
		{
			pWpffile = *itr;
			if(pWpffile && pWpffile->iDx == iWpf) 
			{
				CLock lock(&m_WpfLock);
				pWpffile->pWpf->CreateFileFromBuff(szFileName.c_str(),buf + iPos,iLen - iPos,false,false,(pHeader->dwAttribute & EFA_COMPRESS) != 0 && (pHeader->dwAttribute & EFA_MEARGE_UNCOMPRESS) != 0);
				break;
			}
		}
	}

	TRY_END_RETURN(false)

	return true;
}


//获得正在下载的文件个数
int CStreamManager::GetDownloadingFiles()
{
	//CLock lock(&m_DlFileCriSect);	
	return (int)(m_MInDownloadFile.size());
}

//获得正在保存的文件个数
int CStreamManager::GetSavingFiles()
{
	//CLock lock(&m_SavingFileMapCriSect);	
	return (int)(m_MInSavingFile.size());
}

bool CStreamManager::IsInDownloadList(const char* strFileName)
{
	return IsInDownloadList(HashFileName(strFileName));
}

bool CStreamManager::IsInDownloadList(__int64 iHash)
{
	CLock lock(&m_DlFileCriSect);
	if (m_MInDownloadFile.find(iHash) != m_MInDownloadFile.end())
	{
		return true;
	}

	return false;
}

void CStreamManager::ClearDownloadingFilesList()
{
	CLock lock(&m_DlFileCriSect);	
	m_MInDownloadFile.clear();
	CLock lock2(&m_SavingFileMapCriSect);	
	m_MInSavingFile.clear();
}


// 根据下载优先级获取超时时间(毫秒)
// 高优先级资源(地图/UI/地表)快速重试，低优先级资源可容忍较长等待
static DWORD GetTimeoutByPrior(BYTE byPrior)
{
	switch (byPrior)
	{
	case EP_MOST_HIGH:	return 2000;	// 最高优先级(地图等):2秒
	case EP_UI:			return 3000;	// UI纹理:3秒
	case EP_TILES:		return 5000;	// 大地表:5秒
	case EP_NPC:
	case EP_MONSTER:
	case EP_CHARACTER:	return 7000;	// NPC/怪物/角色:7秒
	case EP_SMTILES:
	case EP_OBJECT:		return 9000;	// 小地表/物体:9秒
	case EP_SKILL:
	case EP_MAGIC:		return 10000;	// 技能/魔法特效:10秒
	case EP_AUDIO:		return 20000;	// 音效:20秒
	default:			return 30000;	// 一般/未知:30秒
	}
}

int CStreamManager::CheckDownLoadList()
{
	int iRtn = 0;
	if (m_MInDownloadFile.size() == 0)
	{
		return iRtn;
	}

	VDlProtocalBody VBody;

	CLock lock(&m_DlFileCriSect);
	DWORD dwTime = GetTickCount();
	// 按优先级分级超时：高优先级快速重试，低优先级可容忍较长等待
	// byRev[0]记录重试次数，超过最大重试次数(3次)放弃，避免对服务器不存在的文件无限请求
	for (map<__int64,DlProtocalBody>::iterator itr = m_MInDownloadFile.begin();itr != m_MInDownloadFile.end();)
	{
		if (itr->second.byPrior <= EP_TILES)
		{
			iRtn = 1;
		}

		DWORD dwTimeout = GetTimeoutByPrior(itr->second.byPrior);
		const BYTE byMaxRetry = 5; // 最大重试5次

		if (dwTime - itr->second.dwTickCount > dwTimeout)
		{
			// 超过最大重试次数，放弃该文件，不再请求
			if (itr->second.byRev[0] >= byMaxRetry)
			{
				output_debug("放弃下载[%d次超时]: %I64d Prior=%d\r\n",
					itr->second.byRev[0], itr->second.i64Hash, itr->second.byPrior);
				itr = m_MInDownloadFile.erase(itr);
			}
			else
			{
				itr->second.dwTickCount = dwTime;
				itr->second.byRev[0] ++;
				VBody.push_back(itr->second);

				output_debug("重试下载[%d/%d]: %I64d Prior=%d Timeout=%dms\r\n",
					itr->second.byRev[0], byMaxRetry,
					itr->second.i64Hash, itr->second.byPrior, dwTimeout);

				itr = m_MInDownloadFile.erase(itr);
			}
		}
		else
		{
			itr ++;
		}
	}

	// 超时未响应的重新请求下载
	if (VBody.size() > 0)
	{
		SendDownloadFile(VBody);
		// 重试请求需重新加入追踪列表，否则OnDownloadedFile中查找不到hash会丢弃响应
		for (size_t i = 0; i < VBody.size(); i++)
		{
			m_MInDownloadFile[VBody[i].i64Hash] = VBody[i];
		}
		m_iInDownloadFileCount = m_MInDownloadFile.size();
	}

	return iRtn;
}
