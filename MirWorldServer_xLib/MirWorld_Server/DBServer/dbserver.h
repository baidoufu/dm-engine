#pragma once
#include "../res/resource.h"
constexpr const char* SERVER_NAME = "数据服务";

#ifdef _DEBUG
#define DPRINT(color, ...) do { if (ShouldOutputLog(color)) CServer::GetInstance()->GetIoConsole()->OutPut(color, __VA_ARGS__); } while(0)
#define DSYSMSG	SaySystem
#else
inline VOID empty_func(...) {}
#define DPRINT empty_func
#define	DSYSMSG empty_func
#endif
// 日志级别过滤: release模式下自动跳过 TEXT_WHITE/COOL_BLUE/STRING_GREEN/KEYWORD_PINK/FUNC_PURPLE/CYAN/ORANGE
// 使用 do{}while(0) 惰性求值, 当日志级别被过滤时参数表达式不会被执行
#define PRINT(color, ...) do { if (ShouldOutputLog(color)) CServer::GetInstance()->GetIoConsole()->OutPut(color, __VA_ARGS__); } while(0)
