// stdafx.h : 标准系统包含文件的包含文件, 
// 或是常用但不常更改的项目特定的包含文件
#pragma once

#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif

// 解决 C++17 std::byte 与 Windows SDK byte 的冲突
#define _HAS_STD_BYTE 0

#include <WinSock2.h>
// Windows 头文件:
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <tchar.h>
#include "..\res\resource.h"
// TODO: 在此处引用程序要求的附加头文件
#include "..\public\public.h"