#pragma once

#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif

// 解决 C++17 std::byte 与 Windows SDK byte 的冲突
#define _HAS_STD_BYTE 0

#include <WinSock2.h>
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "..\res\resource.h"
#include "..\public\public.h"
