#pragma once
#define	LGERR CLogFile::GetInstance()->LogTextWithDate( "´íÎó @ %s (%d) => ", __FILE__, __LINE__ );\
			CLogFile::GetInstance()->LogTextRt
#define	LG1 CLogFile::GetInstance()->LogText
#define	LG2 CLogFile::GetInstance()->LogTextWithDate

class CLogFile : public xSingletonClass<CLogFile>
{
public:
	CLogFile(VOID);
	virtual ~CLogFile(VOID);
	BOOL Init(const char* pszPath);
	VOID NextLine();
	VOID LogDate();
	VOID LogText(const char* pszString, ...);
	VOID LogTextRt(const char* pszString, ...);
	VOID LogTextWithDate(const char* pszString, ...);
	VOID ChangeLogFile();
private:
	std::unique_ptr<FILE, decltype(&fclose)> m_fpLog{nullptr, &fclose};
	SYSTEMTIME	m_stFileTime;
	std::array<char, 1024> m_szFile;
	std::array<char, 16384> m_szCache;
	THREAD_PROTECT_DEFINE;
};