#include "StdAfx.h"
#include "CrashHandler.h"
#include <atomic>
#include <psapi.h>
#include <signal.h>
#include <stdlib.h>
#include <crtdbg.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

std::string CrashHandler::m_dumpPath = "..\\异常\\";
const char* (*CrashHandler::m_additionalInfoCallback)() = nullptr;
CrashHandler::PreCrashSaveCallback CrashHandler::m_preCrashSaveCallback = nullptr;
bool CrashHandler::m_initialized = false;
std::string CrashHandler::m_rtcMessage;

// RTC 错误处理重入保护标志
// 防止在生成 dump 过程中再次触发 RTC 错误导致无限递归
static std::atomic<bool> g_rtcHandling{false};

// 线程安全的时间字符串构建
static VOID BuildTimeStr(char* buf, size_t bufSize)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	_snprintf_s(buf, bufSize, _TRUNCATE, "%04d%02d%02d_%02d%02d%02d",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

// 使用 Win32 API 打开日志文件
static HANDLE OpenLogFile(const char* path)
{
	return CreateFileA(path, GENERIC_WRITE, 0, nullptr,
					   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
}

// 使用 Win32 API 写入日志（不依赖 CRT 的 FILE*）
static VOID LogWrite(HANDLE hFile, const char* str)
{
	DWORD written;
	WriteFile(hFile, str, (DWORD)strlen(str), &written, nullptr);
}

static VOID LogLine(HANDLE hFile, const char* str)
{
	LogWrite(hFile, str);
	LogWrite(hFile, "\r\n");
}

static VOID LogFormat(HANDLE hFile, const char* fmt, ...)
{
	std::array<char, 4096> buf{};
	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
	va_end(args);
	LogWrite(hFile, buf.data());
}

static VOID LogLineFormat(HANDLE hFile, const char* fmt, ...)
{
	std::array<char, 4096> buf{};
	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
	va_end(args);
	LogWrite(hFile, buf.data());
	LogWrite(hFile, "\r\n");
}

// 安全地生成 MiniDump 文件
static BOOL SafeCreateDump(EXCEPTION_POINTERS* pExceptionInfo, const char* dumpPath)
{
	HANDLE hDumpFile = CreateFileA(dumpPath, GENERIC_WRITE, 0, nullptr, 
								   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hDumpFile == INVALID_HANDLE_VALUE)
		return FALSE;

	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pExceptionInfo;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = FALSE;
	
	MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
		MiniDumpWithFullMemory | 
		MiniDumpWithHandleData | 
		MiniDumpWithThreadInfo | 
		MiniDumpWithProcessThreadData |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithUnloadedModules);
	
	BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
						 hDumpFile, dumpType, &dumpInfo, nullptr, nullptr);
	CloseHandle(hDumpFile);
	return result;
}

// 安全地打印异常信息
static VOID SafePrintExceptionInfo(HANDLE hFile, EXCEPTION_POINTERS* pExceptionInfo)
{
	LogLine(hFile, "========== 异常信息 ==========");
	
	DWORD code = pExceptionInfo->ExceptionRecord->ExceptionCode;
	LogLineFormat(hFile, "异常代码: 0x%08X", code);
	
	const char* desc;
	switch (code)
	{
		case EXCEPTION_ACCESS_VIOLATION: desc = "访问违规"; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: desc = "数组越界"; break;
		case EXCEPTION_BREAKPOINT: desc = "断点异常"; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT: desc = "数据类型未对齐"; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: desc = "浮点数除零"; break;
		case EXCEPTION_FLT_OVERFLOW: desc = "浮点数溢出"; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION: desc = "非法指令"; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO: desc = "整数除零"; break;
		case EXCEPTION_INT_OVERFLOW: desc = "整数溢出"; break;
		case EXCEPTION_STACK_OVERFLOW: desc = "栈溢出"; break;
		// 自定义异常码（由 CrashHandler 内部各 Handler 触发）
		case 0xE0000001: desc = "CRT无效参数"; break;
		case 0xE0000002: desc = "纯虚函数调用"; break;
		case 0xE0000003: desc = "abort()调用"; break;
		case 0xE0000004: desc = "运行时检查失败(RTC)"; break;
		default: desc = "未知异常"; break;
	}
	LogLineFormat(hFile, "异常描述: %s", desc);
	LogLineFormat(hFile, "异常地址: 0x%p", pExceptionInfo->ExceptionRecord->ExceptionAddress);
	LogLineFormat(hFile, "异常标志: 0x%08X", pExceptionInfo->ExceptionRecord->ExceptionFlags);
	LogLineFormat(hFile, "参数数量: %u", pExceptionInfo->ExceptionRecord->NumberParameters);
	
	for (DWORD i = 0; i < pExceptionInfo->ExceptionRecord->NumberParameters && i < 15; ++i)
	{
		LogLineFormat(hFile, "参数[%u]: 0x%p", i, 
					 (VOID*)pExceptionInfo->ExceptionRecord->ExceptionInformation[i]);
	}
	
	if (code == EXCEPTION_ACCESS_VIOLATION)
	{
		const char* opType = pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0 ? "读取" :
							 pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "写入" : "执行";
		LogLineFormat(hFile, "访问违规: %s 地址 0x%p", opType,
					 (VOID*)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
	}
	LogLine(hFile, "");
}

// 安全地打印调用栈
static VOID SafePrintStackTrace(HANDLE hFile, EXCEPTION_POINTERS* pExceptionInfo)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
	
	// 构建符号搜索路径：exe目录 + 环境变量
	std::array<char, MAX_PATH * 2> symbolPath{};
	std::array<char, MAX_PATH> exePath{};
	GetModuleFileNameA(nullptr, exePath.data(), MAX_PATH);
	char* lastSlash = strrchr(exePath.data(), '\\');
	if (lastSlash) *lastSlash = '\0';
	_snprintf_s(symbolPath.data(), symbolPath.size(), _TRUNCATE, "%s;%s", exePath.data(), 
				getenv("_NT_SYMBOL_PATH") ? getenv("_NT_SYMBOL_PATH") : "");
	
	if (!SymInitialize(hProcess, symbolPath.data(), TRUE))
	{
		LogLine(hFile, "SymInitialize 失败，无法解析符号");
		return;
	}
	
	LogLine(hFile, "========== 调用栈 ==========");
	
	CONTEXT context = *pExceptionInfo->ContextRecord;
	
	STACKFRAME64 stackFrame = {0};
#ifdef _WIN64
	DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Rbp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#else
	DWORD machineType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#endif
	
	IMAGEHLP_MODULE64 moduleInfo = {0};
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	
	std::array<char, sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)> symbolBuffer{};
	SYMBOL_INFO* pSymbol = (SYMBOL_INFO*)symbolBuffer.data();
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	
	IMAGEHLP_LINE64 lineInfo = {0};
	lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD displacement = 0;
	
	int frameCount = 0;
	while (StackWalk64(machineType, hProcess, hThread, &stackFrame, 
					  &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr) && 
		   frameCount < 64)
	{
		LogFormat(hFile, "[%2d] 0x%p", frameCount, (VOID*)stackFrame.AddrPC.Offset);
		
		if (SymFromAddr(hProcess, stackFrame.AddrPC.Offset, nullptr, pSymbol))
		{
			LogFormat(hFile, " %s", pSymbol->Name);
		}
		
		if (SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &displacement, &lineInfo))
		{
			LogFormat(hFile, " (%s:%d)", lineInfo.FileName, lineInfo.LineNumber);
		}
		
		if (SymGetModuleInfo64(hProcess, stackFrame.AddrPC.Offset, &moduleInfo))
		{
			LogFormat(hFile, " [%s]", moduleInfo.ModuleName);
		}
		
		LogLine(hFile, "");
		++frameCount;
	}
	
	if (frameCount == 0)
		LogLine(hFile, "无法获取调用栈信息");
	
	LogLine(hFile, "");
	SymCleanup(hProcess);
}

// 确保目录存在（递归创建所有中间目录）
static BOOL EnsureDirectoryExists(const char* path)
{
	// 复制一份用于修改
	std::array<char, MAX_PATH> dirPath{};
	strncpy_s(dirPath.data(), dirPath.size(), path, _TRUNCATE);
	dirPath[MAX_PATH - 1] = '\0';  // 确保字符串零终止符
	
	// 找到最后一个反斜杠，截断为目录路径
	char* lastSlash = strrchr(dirPath.data(), '\\');
	if (lastSlash != nullptr)
	{
		*lastSlash = '\0';
	}
	else
	{
		// 没有路径分隔符，说明是当前目录，无需创建
		return TRUE;
	}
	
	// 如果目录已存在，直接返回成功
	DWORD attr = GetFileAttributesA(dirPath.data());
	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;
	
	// 递归创建父目录
	// 先找到上一层目录
	char* prevSlash = strrchr(dirPath.data(), '\\');
	if (prevSlash != nullptr && prevSlash != dirPath.data())
	{
		*prevSlash = '\0';
		// 跳过盘符根目录（如 C:\）
		if (prevSlash[-1] != ':')
		{
			if (!EnsureDirectoryExists(dirPath.data()))
				return FALSE;
		}
		*prevSlash = '\\';
	}
	
	// 创建当前目录
	return CreateDirectoryA(dirPath.data(), nullptr) != 0 || 
		   GetLastError() == ERROR_ALREADY_EXISTS;
}

// ========== 类公开方法 ==========

// 生成MiniDump和崩溃日志
VOID CrashHandler::CreateMiniDump(EXCEPTION_POINTERS* pExceptionInfo)
{
	std::array<char, 64> timeStr{};
	BuildTimeStr(timeStr.data(), timeStr.size());
	
	std::array<char, MAX_PATH> logPath{};
	_snprintf_s(logPath.data(), logPath.size(), _TRUNCATE, "%scrash_%s.log", m_dumpPath.c_str(), timeStr.data());

	// 确保目录存在
	EnsureDirectoryExists(logPath.data());

	// 生成详细日志
	__try
	{
		HANDLE hLogFile = OpenLogFile(logPath.data());
		if (hLogFile != INVALID_HANDLE_VALUE)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			LogLineFormat(hLogFile, "崩溃时间: %04d-%02d-%02d %02d:%02d:%02d",
						  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			LogLine(hLogFile, "");
			// 安全打印异常信息
			SafePrintExceptionInfo(hLogFile, pExceptionInfo);
			// RTC 错误的详细信息（仅当异常码为 0xE0000004 时输出）
			if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE0000004 && !m_rtcMessage.empty())
			{
				LogLine(hLogFile, "========== RTC 详细信息 ==========");
				LogLine(hLogFile, m_rtcMessage.c_str());
				LogLine(hLogFile, "");
			}
			// 安全打印调用栈
			__try
			{
				SafePrintStackTrace(hLogFile, pExceptionInfo);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				LogLine(hLogFile, "调用栈解析过程中发生异常");
			}
			// 打印额外信息
			if (m_additionalInfoCallback)
			{
				__try
				{
					const char* additionalInfo = m_additionalInfoCallback();
					if (additionalInfo)
					{
						LogLine(hLogFile, "========== 额外信息 ==========");
						LogLine(hLogFile, additionalInfo);
						LogLine(hLogFile, "");
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					LogLine(hLogFile, "[额外信息回调异常]");
				}
			}
			// 打印内存信息
			LogLine(hLogFile, "========== 内存信息 ==========");
			PROCESS_MEMORY_COUNTERS pmc;
			if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			{
				LogLineFormat(hLogFile, "页面错误数: %lu", pmc.PageFaultCount);
				LogLineFormat(hLogFile, "工作集大小: %Iu KB", pmc.WorkingSetSize / 1024);
				LogLineFormat(hLogFile, "峰值工作集: %Iu KB", pmc.PeakWorkingSetSize / 1024);
				LogLineFormat(hLogFile, "分页池大小: %Iu KB", pmc.QuotaPagedPoolUsage / 1024);
				LogLineFormat(hLogFile, "非分页池: %Iu KB", pmc.QuotaNonPagedPoolUsage / 1024);
				LogLineFormat(hLogFile, "页面文件使用: %Iu KB", pmc.PagefileUsage / 1024);
				LogLineFormat(hLogFile, "峰值页面文件: %Iu KB", pmc.PeakPagefileUsage / 1024);
			}
			LogLine(hLogFile, "");
			
			CloseHandle(hLogFile);
			printf("崩溃日志已生成: %s\n", logPath.data());
		}
		else
		{
			printf("无法创建崩溃日志文件: %s (错误: %d)\n", logPath.data(), GetLastError());
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("生成崩溃日志时发生二次异常\n");
	}
}

// 未处理异常的回调函数
LONG WINAPI CrashHandler::UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
	printf("\n========== 程序崩溃 ==========\n");
	printf("捕获到未处理的异常, 正在生成崩溃报告...\n");
	
	// 先尝试保存数据（行会、沙城、玩家等）
	if (m_preCrashSaveCallback)
	{
		printf("正在尝试保存数据...");
		__try
		{
			m_preCrashSaveCallback();
			printf("数据保存完成.");
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("数据保存过程中发生二次异常, 已跳过.");
		}
	}

	// 生成MiniDump和详细日志
	CreateMiniDump(pExceptionInfo);
	
	printf("崩溃报告已生成, 程序即将退出.\n");
	printf("=============================\n\n");

	return EXCEPTION_EXECUTE_HANDLER;
}

// 无效参数处理回调
static VOID InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, 
									 const wchar_t* file, UINT line, uintptr_t reserved)
{
	printf("[CRT] 无效参数: %S 函数: %S 文件: %S:%u\n", 
		   expression ? expression : L"", 
		   function ? function : L"", 
		   file ? file : L"", line);
	// 手动触发异常，走 UnhandledExceptionFilter
	RaiseException(0xE0000001, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

// 纯虚函数调用回调
static VOID PureCallHandler()
{
	printf("[CRT] 纯虚函数调用\n");
	RaiseException(0xE0000002, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

// abort信号处理
static VOID AbortHandler(int signal)
{
	printf("[CRT] abort() 被调用\n");
	RaiseException(0xE0000003, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

#ifdef _DEBUG
// ============================================================================
// RTC (Run-Time Check) 错误拦截
// ============================================================================
// RTC 错误（如 ESP 不匹配、栈损坏、变量未初始化等）在 Debug 构建下通过
// _CrtDbgReport 报告，默认会弹出对话框。此 Hook 在报告时拦截 RTC 错误，
// 转而调用 CrashHandler::HandleRtcFailure 生成崩溃报告（dump + 日志），
// 避免 RTC 错误现场丢失。
//
// RTC 错误消息特征：以 "Run-Time Check Failure" 开头，例如：
//   "Run-Time Check Failure #0 - The value of ESP was not properly saved..."
//   "Run-Time Check Failure #2 - Stack around the variable 'xxx' was corrupted."
//   "Run-Time Check Failure #3 - The variable 'xxx' is being used without..."
//   "Run-Time Check Failure #4 - Stack pointer corruption."
// ============================================================================
static int __cdecl RtcReportHook(int reportType, char* message, int* returnValue)
{
	// 只处理包含 "Run-Time Check Failure" 的报告（RTC 错误特征字符串）
	if (message == nullptr || strstr(message, "Run-Time Check Failure") == nullptr)
	{
		return FALSE; // 非 RTC 错误，交给默认处理流程
	}

	// 防止重入：生成 dump 过程中可能再次触发 RTC 错误
	if (g_rtcHandling.exchange(true))
	{
		// 已在处理中，直接吞掉避免无限递归
		*returnValue = 0;
		return TRUE;
	}

	printf("\n========== RTC 错误捕获 ==========\n");
	printf("检测到运行时检查失败, 正在生成崩溃报告...\n");
	printf("RTC 消息: %s\n", message);

	// 调用 CrashHandler 处理：捕获当前上下文 + 生成 dump + 终止进程
	// 注意：此时仍在触发 RTC 错误的线程上下文中，RtlCaptureContext 能
	//       捕获到错乱但真实的寄存器状态（ESP/EIP），供事后分析
	CrashHandler::HandleRtcFailure(message);

	// 不会到达这里，HandleRtcFailure 会调用 TerminateProcess
	*returnValue = 0;
	return TRUE;
}
#endif // _DEBUG

VOID CrashHandler::Initialize()
{
	if (m_initialized) return;

	// 1. 设置未处理异常过滤器（SEH异常）
	SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	// 2. 处理CRT无效参数（如传NULL给需要非NULL的函数）
	_set_invalid_parameter_handler(InvalidParameterHandler);

	// 3. 处理纯虚函数调用
	_set_purecall_handler(PureCallHandler);

	// 4. 处理abort()信号
	signal(SIGABRT, AbortHandler);

	// 5. 禁用abort的错误报告弹窗
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

	// 6. 禁用CRT断言弹窗（只输出到stderr）
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

#ifdef _DEBUG
	// 7. 拦截 RTC (Run-Time Check) 错误，让 RTC 错误也走崩溃报告流程
	// RTC 错误（如 ESP 不匹配 #0、栈损坏 #2、变量未初始化 #3 等）默认
	// 弹窗且不生成 dump，导致错误现场丢失。通过 Report Hook 拦截后
	// 转而调用 HandleRtcFailure 生成 dump，保留寄存器和线程上下文
	_CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, RtcReportHook);
#endif

	m_initialized = true;
}

VOID CrashHandler::SetDumpPath(const char* path)
{
	m_dumpPath = path;
	// 确保路径以反斜杠结尾
	if (!m_dumpPath.empty() && m_dumpPath.back() != '\\')
	{
		m_dumpPath += '\\';
	}
}

VOID CrashHandler::SetAdditionalInfoCallback(const char* (*callback)())
{
	m_additionalInfoCallback = callback;
}

VOID CrashHandler::SetPreCrashSaveCallback(PreCrashSaveCallback callback)
{
	m_preCrashSaveCallback = callback;
}

VOID CrashHandler::HandleRtcFailure(const char* rtcMessage)
{
	// 保存 RTC 消息，供 CreateMiniDump 写入日志
	m_rtcMessage = (rtcMessage != nullptr) ? rtcMessage : "";

	// 捕获当前线程上下文
	// 此时仍在触发 RTC 错误的线程中，RtlCaptureContext 能捕获到
	// 错乱但真实的寄存器状态（ESP/EIP），供事后分析
	CONTEXT ctx;
	RtlCaptureContext(&ctx);

	// 构造异常记录（自定义异常码 0xE0000004 = RTC 错误）
	EXCEPTION_RECORD er = {};
	er.ExceptionCode = 0xE0000004;
	er.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#ifdef _WIN64
	er.ExceptionAddress = (PVOID)ctx.Rip;
#else
	er.ExceptionAddress = (PVOID)ctx.Eip;
#endif

	EXCEPTION_POINTERS ep;
	ep.ExceptionRecord = &er;
	ep.ContextRecord = &ctx;

	printf("\n========== 程序崩溃 (RTC 错误) ==========\n");
	printf("捕获到运行时检查失败, 正在生成崩溃报告...\n");

	// 先尝试保存数据（行会、沙城、玩家等）
	if (m_preCrashSaveCallback)
	{
		printf("正在尝试保存数据...");
		__try
		{
			m_preCrashSaveCallback();
			printf("数据保存完成.");
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("数据保存过程中发生二次异常, 已跳过.");
		}
	}

	// 生成 MiniDump 和详细日志
	CreateMiniDump(&ep);

	printf("崩溃报告已生成, 程序即将退出.\n");
	printf("========================================\n\n");

	// 强制终止进程，避免继续执行导致更严重的损坏
	// 使用 TerminateProcess 而非 exit()，避免触发 atexit/全局析构导致二次崩溃
	TerminateProcess(GetCurrentProcess(), 0xE0000004);
}
