#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>
#include <string>

#pragma comment(lib, "dbghelp.lib")

// 崩溃处理器类
class CrashHandler
{
public:
	// 初始化崩溃处理器
	static VOID Initialize();
	// 设置崩溃转储文件保存路径
	static VOID SetDumpPath(const char* path);
	// 设置额外的崩溃信息回调
	static VOID SetAdditionalInfoCallback(const char* (*callback)());
	// 设置崩溃前数据保存回调（如保存玩家数据、行会数据等）
	typedef VOID (*PreCrashSaveCallback)();
	static VOID SetPreCrashSaveCallback(PreCrashSaveCallback callback);
	// 处理 RTC (Run-Time Check) 错误：捕获当前上下文并生成崩溃报告
	// 由 RTC Report Hook 在检测到 RTC 错误时调用
	static VOID HandleRtcFailure(const char* rtcMessage);
private:
	// 未处理异常的回调函数
	static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo);
	// 生成MiniDump
	static VOID CreateMiniDump(EXCEPTION_POINTERS* pExceptionInfo);
private:
	static std::string m_dumpPath;
	static const char* (*m_additionalInfoCallback)();
	static PreCrashSaveCallback m_preCrashSaveCallback;
	static bool m_initialized;
	// RTC 错误的原始消息文本（用于在崩溃日志中记录详细信息）
	static std::string m_rtcMessage;
};
// 便利宏定义
#define CRASH_HANDLER_INIT() CrashHandler::Initialize()
#define CRASH_HANDLER_SETPATH(path) CrashHandler::SetDumpPath(path)

// ============================================================
// 统一异常安全宏 - 用于关键代码路径的防护
// ============================================================

// 安全指针检查：检查指针是否为空，为空则返回指定值
#define SAFE_CHECK_PTR(ptr, retval) \
	do { if ((ptr) == nullptr) return (retval); } while(0)

// 安全指针检查：检查指针是否为空，为空则返回void
#define SAFE_CHECK_PTR_VOID(ptr) \
	do { if ((ptr) == nullptr) return; } while(0)

// 安全指针检查：检查指针是否为空，为空则跳过（用于循环体内）
#define SAFE_CHECK_PTR_CONTINUE(ptr) \
	do { if ((ptr) == nullptr) continue; } while(0)

// 安全指针检查：检查指针是否为空，为空则中断循环
#define SAFE_CHECK_PTR_BREAK(ptr) \
	do { if ((ptr) == nullptr) break; } while(0)

// 安全数组边界检查
#define SAFE_CHECK_INDEX(idx, maxsize) \
	do { if ((idx) < 0 || (idx) >= (maxsize)) return FALSE; } while(0)

#define SAFE_CHECK_INDEX_VOID(idx, maxsize) \
	do { if ((idx) < 0 || (idx) >= (maxsize)) return; } while(0)

// SEH异常保护包装器 - 用于保护可能产生访问违例的代码段
// 用法: SEH_PROTECT_BEGIN() ... SEH_PROTECT_END()
#define SEH_PROTECT_BEGIN() \
	__try {

#define SEH_PROTECT_END() \
	} __except(EXCEPTION_EXECUTE_HANDLER) { \
		PRINT(ERROR_RED, "[SEH] 异常捕获于 %s:%d\n", __FILE__, __LINE__); \
	}

// SEH异常保护包装器 - 带返回值
#define SEH_PROTECT_BEGIN_RET(retval) \
	__try {

#define SEH_PROTECT_END_RET(retval) \
	} __except(EXCEPTION_EXECUTE_HANDLER) { \
		PRINT(ERROR_RED, "[SEH] 异常捕获于 %s:%d\n", __FILE__, __LINE__); \
		return (retval); \
	}

// 安全delete - 删除后置空指针
#define SAFE_DELETE(p) \
	do { if ((p) != nullptr) { delete (p); (p) = nullptr; } } while(0)

#define SAFE_DELETE_ARRAY(p) \
	do { if ((p) != nullptr) { delete[] (p); (p) = nullptr; } } while(0)

// 安全释放COM/Windows句柄
#define SAFE_RELEASE(p) \
	do { if ((p) != nullptr) { (p)->Release(); (p) = nullptr; } } while(0)

// 实例Key验证 - 用于检测对象引用是否过期
// 在获取远端对象引用时，先验证Key是否匹配，防止引用已释放的对象
#define VALIDATE_INSTANCE_KEY(obj, key) \
	((obj) != nullptr && (obj)->GetKey() == (key))
