///////////////////////////////////////////////////////////////////////
//在主缓冲的头部写入指针和尺寸，形成一个freelist,当有一个 buffer被释放时，按顺序
//加入到原有的freelist中，并对相邻的空闲区域进行合并；
///////////////////////////////////////////////////////////////////////
#pragma once
extern bool g_bGlobalValid;
//primary pool
#define NUM_POOL 1
#define MAX_BYTE 0x1000 
//second pool 
#define NUM_LIST 10

typedef unsigned int size_t ;

//#ifdef _DEBUG
//#define TEMP_MEM_CHECK
//#endif

#ifdef TEMP_MEM_CHECK
	#define DEBUG_NEW new(__FILE__, __LINE__)

	void* __cdecl operator new(size_t nSize, const char * lpszFileName, int nLine);
	void* __cdecl operator new[](size_t);
	void* __cdecl operator new[](size_t nSize, const char * lpszFileName, int nLine);
	void* __cdecl operator new(size_t nSize, int nType,  const char * lpszFileName, int nLine);
	void* __cdecl operator new[](size_t nSize, int nType, const char * lpszFileName, int nLine);

	void  __cdecl operator delete(void* p, const char * lpszFileName, int nLine);
	void  __cdecl operator delete[](void* p, const char * lpszFileName, int nLine);
	void  __cdecl operator delete[](void *);

	void PrintMem();
#else
	#define DEBUG_NEW new
#endif