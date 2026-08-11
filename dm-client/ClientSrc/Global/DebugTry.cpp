#include "DebugTry.h"
#include "stdio.h"
#include <time.h>
#include "BaseClass/Misc/Internet.h"
#include "Interface/StreamInterface.h"
/*#include <dbghelp.h>


#pragma warning(disable : 4996)

inline LONG __stdcall excep_filter( LPEXCEPTION_POINTERS lpEP, DWORD dwExpCode,char *strCallStack)
{
	// init dbghelp.dll
	if( !SymInitialize( GetCurrentProcess(), NULL, TRUE ) )
	{
		//TRACE( "Init dbghelp faile .\n" );
		return 0;
	}


	CONTEXT *context = lpEP->ContextRecord;
	DWORD dwTickCount;
	dwTickCount = GetTickCount();
	STACKFRAME64 sf;
	char buf[512]={0};
	memset( &sf, 0, sizeof( STACKFRAME ) );

	sf.AddrPC.Offset = context->Eip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Offset = context->Esp;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = context->Ebp;
	sf.AddrFrame.Mode = AddrModeFlat;

	DWORD machineType = IMAGE_FILE_MACHINE_I386;

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	for( int i = 0; i < 10; i++)
	{
		if( !StackWalk64(machineType, hProcess, hThread, &sf, context, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0 ) )
		{
			break;
		}

		if( sf.AddrFrame.Offset == 0 )
		{
			break;
		}

		//if(i < 2)
		//{
		//	continue;
		//}

		sprintf(buf,"Address:%08X  ", sf.AddrPC.Offset);
		strcat(strCallStack,buf);

		IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
		DWORD dwLineDisplacement;
		if( SymGetLineFromAddr64( hProcess, sf.AddrPC.Offset,&dwLineDisplacement, &lineInfo ))
		{
			sprintf(buf,"%s (%u):",lineInfo.FileName, lineInfo.LineNumber);
			strcat(strCallStack,buf);
		}

		BYTE symbolBuffer[ sizeof( SYMBOL_INFO ) + 1024 ];
		PIMAGEHLP_SYMBOL64 pSymbol = ( PIMAGEHLP_SYMBOL64 ) symbolBuffer;

		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		pSymbol->MaxNameLength = 1024;
		DWORD64 symDisplacement = 0;
		if( SymGetSymFromAddr64( hProcess, sf.AddrPC.Offset, 0, pSymbol) )
		{
			sprintf(buf," %s", pSymbol->Name);
			strcat(strCallStack,buf);
		}

		strcat(strCallStack,"\r\n");
	}

	if( !SymCleanup( GetCurrentProcess() ) )
	{
		//TRACE( "Cleanup dbghelp faile.\n" );
		return 0;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}


inline void GetCallStack(char *strCallStack)
{
	__try
	{
		throw 1;
	}
	__except(excep_filter(GetExceptionInformation(), GetExceptionCode(),strCallStack))
	{
	}
}
*/
void SaveException(char * file,char *fun,int line,const CSeException* pException,const char *msg)
{
	if(g_iCatchTimes++ > 10)
		return;

	// 时间
	char strTime[128];
	char tmp_d[64];
	char tmp_t[64];

	_tzset();
	_strdate(tmp_d);
	_strtime(tmp_t);
	sprintf(strTime,"%s\t%s",tmp_d,tmp_t);


	char strTemp[2048]={0};
	if(pException)
		sprintf(strTemp,"Exception=Time:%s,Version:%s,File:%s,Function:%s,Line:%d,Address:0x%08x,Exception:%s,Msg:%s\r\n",strTime,g_strVersion,file,fun,line,(DWORD)((__int64)pException->Address()),pException->What(),msg?msg:"");
	else
		sprintf(strTemp,"Exception=Time:%s,Version:%s,File:%s,Function:%s,Line:%d,Msg:%s\r\n",strTime,g_strVersion,file,fun,line,msg?msg:"");
	
	//char strCallStack[5120]={0};
	//GetCallStack(strCallStack);
	//strcat(strTemp,strCallStack);

	//记录到文件
	FILE * fp = fopen("../clientexception.log","a+"/*(g_iCatchTimes == 1 ? "w" : "a+")*/);
	if(fp)
	{		
		fprintf(fp,"%s",strTemp);
		fclose(fp);
	}

	if(g_pStreamMgr && g_iCatchTimes <= 2 && !g_bAlertException)
	{
		/* 自动上传日志
		strcat(strTemp,"&type=1");

		string URL = g_pStreamMgr->GetWebsite("ExceptionReport");
		if(URL.empty())
			URL = "http://home.woool.sdo.com/project/saveexception.asp";

		int iPos = URL.find("//");
		int iPos2 = URL.find("/",iPos + 2);
		if(iPos != string::npos && iPos2 != string::npos)
		{
			g_HttpMgr.PostData(URL.substr(iPos + 2,iPos2 - (iPos + 2)).c_str(),URL.substr(iPos2 + 1,URL.length() - (iPos2 + 1)).c_str(),strTemp);
		}*/
	}

	if(g_bAlertException)
	{
		strcat(strTemp,"\r\n\r\n程序出现异常,请联系开发人员");
		::MessageBox(::GetActiveWindow(),strTemp,"错误",MB_OK);
	}
}

