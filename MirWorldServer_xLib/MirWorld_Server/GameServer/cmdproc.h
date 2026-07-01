#pragma once
#include <string>

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		ÃèÊö£º
//		×¢ÊÍ£º
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CCommandManager : public xSingletonClass<CCommandManager>
{
public:
	CCommandManager();
	BOOL AddCommand(const char* pszCommand, fnCommandProc proc);
	fnCommandProc GetCommandProc(const char* pszCommand);
	BOOL ChangeCommandName(const char* pszCommand, const char* pszNewName);
protected:
	xStringList<1024> m_xCommandList;
	SmallFlatMap<std::string, fnCommandProc, 256> m_xCache;  // GMÃüÁî»º´æ (Õ»´æ´¢, Ìæ´ú unordered_map)
};