#pragma once

//表示一个段落，每次服务器发送过来的一段话就是一个段落，
//段落是分频道的
#include "StringLine.h"
#include <deque>

class CMultiLine
{
public:
	CMultiLine(DWORD color,DWORD back,DWORD dwType,int limit = 0,int iFontType = FONT_DEFAULT,int iFontSize = FONTSIZE_DEFAULT,bool bIgnoreewLine = false);
	~CMultiLine(void);

	void  setType(DWORD t){  m_dwType = t; }
	DWORD getType()       {  return m_dwType; }

	CStringLine * getLine(int i){ return m_lines.at(i); }
	int  getLineCount(){ return (int)m_lines.size(); }

	void          handleBuffer(const char* lpBuff,int iLen);
	DWORD         getColor(){  return m_dwColor; }
	void          setColor(DWORD dwColor){m_dwColor = dwColor;}
	DWORD         getBack(){ return m_dwBack; }
	void          setBack(DWORD dwBack){m_dwBack = dwBack; }
	DWORD         getFlag(){return m_dwFlag;}
	void          setFlag(DWORD dwFlag){m_dwFlag = dwFlag;}
	int           getAlpha(){ return m_iAlpha; }
	void          setAlpha(int iAlpha){ m_iAlpha = iAlpha; }
	int           getFont(){  return m_iFontType; }
	int           getFontSize(){return m_iFontSize;}

	void setLimit(int iLimit){ m_iLimit = iLimit; }

	const char*   c_str();

	void          clear();
	void          clean(const char* str); //长度应该是一样的，处理脏话
private:
	// 禁止拷贝（防止 vector<CStringLine*> 裸指针被浅拷贝导致双重释放）
	CMultiLine(const CMultiLine&);
	CMultiLine& operator=(const CMultiLine&);

	vector<CStringLine * > m_lines;
	DWORD         m_dwColor; //字体颜色，用字节表示的颜色
	DWORD         m_dwBack;  //背景色
	int           m_iLimit;  //限制字符数
	string		  m_show;    //显示的文字内容
	DWORD         m_dwType;  //类型
	int           m_iAlpha;//用于渐隐绘制
	DWORD         m_dwFlag;//绘制时的属性
	int           m_iFontType;
	int           m_iFontSize;
	bool          m_bIgnoreewLine;//是否清除回车换行及"\\"
};

typedef std::deque<CMultiLine*> TalkType1;
typedef std::deque<CStringLine*> TalkType;
