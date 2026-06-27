#pragma once
#pragma comment( lib, "USER32" )

#include <crtdbg.h>
#include <array>
#define	MTASSERT(a)	_ASSERTE(a)

#define	MTVERIFY(a) if( !(a) )PrintError( #a, __FILE__, __LINE__, GetLastError() );

__inline VOID PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
{
	LPSTR	lpBuffer;
	std::array<char, 256> errbuf = {};
#ifdef _WINDOWS
	std::array<char, MAX_PATH> modulename = {};
#else
	DWORD	numread;
#endif
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errnum, LANG_NEUTRAL, (LPTSTR)&lpBuffer, 0, nullptr);
	wsprintf(errbuf.data(), "\n在文件 %s 的 %d 行进行的下列调用失败: \n\n"
		"     %s\n\n原因: %s\n", filename, lineno, linedesc, lpBuffer);
#ifndef _WINDOWS
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), errbuf.data(), (int)strlen(errbuf.data()), &numread, FALSE);
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
#else
	GetModuleFileName(nullptr, modulename.data(), MAX_PATH);
	MessageBox(nullptr, errbuf.data(), modulename.data(),
		MB_ICONWARNING | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#endif
	exit(EXIT_FAILURE);
}
