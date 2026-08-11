#pragma once

#include "glyph.h"
#include "Global/Interface/FontInterface.h"

class CStringLine :	public CGlyph
{
	friend class CMultiLine;
public:
	CStringLine();
	CStringLine(DWORD color,DWORD back,int iFontType = FONT_DEFAULT,int iFontSize = FONTSIZE_DEFAULT,DWORD m_dwFlag = 0);
	virtual ~CStringLine(void);

	void Draw(int x,int y);
	void Draw(int x,int y,int begin,int end = -1);
	void DrawPart(int x,int y,int begin,int end = -1);

	void insert(CGlyph *,int pos = -1);
	void remove(int pos);
	void remove(int begin,int end);
	void assign(CStringLine& r);
	int  substr(string& str,int begin,int end = -1);
	DWORD getColor(){return m_dwColor;}
	void  setColor(DWORD dwColor){m_dwColor = dwColor;}
	DWORD getBack(){return m_dwBack;}
	void  setBack(DWORD dwBack){m_dwBack = dwBack;}
	DWORD getFlag(){return m_dwFlag;}
	void  setFlag(DWORD dwFlag){m_dwFlag = dwFlag;}

	CGlyph * getGlyph(int i);
	int      getGlyphSize(){ return (int)m_words.size(); }
	int      getGlyphWidth(int i);
	int      getGlyphPos(int x); //x为坐标
	CGood*   getGlyphGood(int x); //x为坐标
	void     clean(const char* str);
	int      getWidth(int begin,int end);
	bool     equal(CStringLine* tmp);
private:
	// 禁止拷贝（防止 vector<CGlyph*> 裸指针被浅拷贝导致双重释放）
	CStringLine(const CStringLine&);
	CStringLine& operator=(const CStringLine&);

	vector<CGlyph *> m_words;
	int      m_iFontSize;
	int      m_iFontType;

	DWORD    m_dwColor;//字体颜色，用字节表示的颜色
	DWORD    m_dwBack;//背景色
	DWORD    m_dwFlag;//绘制时的属性

	void     updateBuffer(); //重新生成Buffer信息
};
