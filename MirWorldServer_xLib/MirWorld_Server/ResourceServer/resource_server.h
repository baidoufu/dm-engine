#pragma once
#include "..\res\resource.h"
#include "resource_client.h"
#include "file_manager.h"

#ifdef _DEBUG
#define DPRINT(color, ...) do { if (ShouldOutputLog(color)) CResourceServer::GetInstance()->GetIoConsole()->OutPut(color, __VA_ARGS__); } while(0)
#define DSYSMSG	SaySystem
#else
inline VOID empty_func(...) {}
#define DPRINT empty_func
#define	DSYSMSG empty_func
#endif
// 日志级别过滤: release模式下自动跳过 TEXT_WHITE/COOL_BLUE/STRING_GREEN/KEYWORD_PINK/FUNC_PURPLE/CYAN/ORANGE
// 使用 do{}while(0) 惰性求值, 当日志级别被过滤时参数表达式不会被执行
#define PRINT(color, ...) do { if (ShouldOutputLog(color)) CResourceServer::GetInstance()->GetIoConsole()->OutPut(color, __VA_ARGS__); } while(0)

#define SERVER_NAME	"资源服务"

class CResourceServer : public xSingletonClass<CResourceServer>, public CBaseServer, public xIndexObjectPool<CResourceClient>
{
public:
	CResourceServer(VOID);
	virtual ~CResourceServer(VOID);
public:
	VOID OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject(){ return newObject(); }
	VOID DeleteClientObject(CClientObject* pObject){ deleteObject((CResourceClient*)pObject); }
	VOID Update();
	//初始化
	BOOL InitServer(CSettingFile&);
	BOOL MakeIndex(CSettingFile&);
protected:
	std::string m_dataRootPath;
	std::string m_fileListPath;
};