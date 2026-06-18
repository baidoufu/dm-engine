#pragma once
class CInputListener
{
public:
	virtual VOID OnInput(const char* pszString) = 0;
};

enum e_ColorType
{
	TEXT_WHITE, // 柔白色
	COOL_BLUE, // 静谧蓝
	STRING_GREEN, // 活力绿
	WARN_YELLOW, // 明艳黄
	KEYWORD_PINK, // 珊瑚橙
	FUNC_PURPLE, // 淡雅紫
	CYAN, //荧光青
	ORANGE, //低饱和橙
	SUCCESS_GREEN, // 鲜绿色
	ERROR_RED, // 纯红色
};

// 日志级别过滤: release模式下跳过调试级别日志, 避免不必要的参数求值
inline bool ShouldOutputLog(DWORD dwColor)
{
#ifdef _DEBUG
	return true;
#else
	// release模式仅输出 WARN_YELLOW / SUCCESS_GREEN / ERROR_RED
	return (dwColor == WARN_YELLOW || dwColor == SUCCESS_GREEN || dwColor == ERROR_RED);
#endif
}

class CIOConsole
{
public:
	CIOConsole(VOID);
	virtual ~CIOConsole(VOID);
	// TEXT_WHITE 柔白色 
	// COOL_BLUE 静谧蓝 
	// STRING_GREEN 活力绿 
	// WARN_YELLOW 明艳黄 
	// KEYWORD_PINK 珊瑚橙 
	// FUNC_PURPLE 淡雅紫 
	// CYAN 荧光青 
	// ORANGE 低饱和橙
	// SUCCESS_GREEN 鲜绿色
	// ERROR_RED 纯红色
	VOID OutPut(DWORD dwColor, const char* pszString, ...);
	DWORD GetColor(DWORD index);
public:
	virtual VOID OutPutStatic(DWORD dwColor, const char* pszString) {};
	virtual VOID Input(const char* pszString) { if (m_pInputListener != nullptr)m_pInputListener->OnInput(pszString); }
	CInputListener* GetInputListener() { return m_pInputListener; }
	VOID	SetInputListener(CInputListener* pInputListener) { m_pInputListener = pInputListener; }
protected:
	CInputListener* m_pInputListener;
};
