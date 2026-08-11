#include "Stringline.h"
#include "Global/Interface/FontInterface.h"
#include "Global/Global.h"
#include "Global/DebugTry.h"

CStringLine::CStringLine()
{
	m_dwFlag = 0;
	m_dwBack = 0;
	m_dwColor = 0xFFFFFFFF;
	m_iFontType = FONT_DEFAULT;
	m_iFontSize = FONTSIZE_DEFAULT;
}

CStringLine::CStringLine(DWORD color,DWORD back,int iFontType,int iFontSize,DWORD dwFlag)
{
	m_dwFlag = dwFlag;
	m_dwBack = back;
	m_dwColor = color;
	m_iFontType = iFontType;
	m_iFontSize = iFontSize;
}

CStringLine::~CStringLine(void)
{
	remove(0,-1);
}

void CStringLine::insert(CGlyph* glyph,int pos)
{
	if(pos < 0)
		m_words.push_back(glyph);
	else
		m_words.insert(m_words.begin()+pos,glyph);

	updateBuffer(); //生成新的Buffer数据
}

void CStringLine::remove(int pos)
{
	if(m_words[pos])
	{
		delete m_words[pos];
		m_words.erase(m_words.begin()+pos);
		updateBuffer(); //生成新的Buffer数据
	}
}

void CStringLine::assign(CStringLine& r)
{
	m_dwBack = r.m_dwBack;
	m_dwColor = r.m_dwColor;
	m_iFontSize = r.m_iFontSize;
	m_iFontType = r.m_iFontType;
	m_dwFlag = r.m_dwFlag;

	remove(0,-1); //删除原来的全部

	for(size_t i = 0;i < r.m_words.size();i++)
	{
		m_words.push_back(r.m_words[i]->clone());
	}
	updateBuffer();
}

void CStringLine::remove(int begin,int end)
{
	if(m_words.size() == 0)
		return;

	if(end <= 0)
		end = (int)m_words.size();

	for(int i = begin; i < end; i++)
	{
		if(m_words[i])
		{
			delete m_words[i];
			m_words[i] = NULL;
		}
	}
	m_words.erase(m_words.begin()+begin,m_words.begin()+end);

	updateBuffer(); //生成新的Buffer数据
}

int  CStringLine::getGlyphPos(int x) //x为坐标
{
	int iSize = (int)m_words.size();
	if(iSize == 0)
		return -1;

	//int pos = x * 2 / m_iFontSize;
	//if(pos > (int)m_show.size())
	//	return ((int)m_words.size() - 1);

	//int _width = 0;
	//for(size_t i = 0;i < m_words.size(); i++)
	//{
	//	int _size = m_words[i]->size();
	//	if(_width + _size > pos)
	//		return i;
	//	_width += _size;
	//}
	//return -1;

	int iW = 0;
	int iCol = -1;
	int i = 0;
	for(; i < iSize; i++)
	{
		int iGlyphWidth = getGlyphWidth(i);
		if(iW + iGlyphWidth < x)
		{
			iW += iGlyphWidth;
		}
		else
		{
			iCol = i;
			break;
		}
	}

	if(i >= iSize)
		iCol = iSize;

	return iCol;
}

CGlyph * CStringLine::getGlyph(int i)
{ 
	if(i < 0 || i >= m_words.size())
		return NULL;

	return m_words[i]; 
}

int CStringLine::substr(string& str,int begin,int end)
{
	str.clear();

	if(m_words.size() == 0)
		return 0;

	if(end < 0)
		end = m_words.size();

	if(begin >= end)
		return 0;

	int _iBegin = 0;
	int _iSize = 0;
	for(int i = 0;i < (int)m_words.size();i++)
	{
		int _size = (int)m_words[i]->size();
		if(i < begin)
			_iBegin += _size;
		else if(i < end)
			_iSize += _size;
		else
			break;
	}
	str = m_show.substr(_iBegin,_iSize);
	return _iBegin;
}

CGood* CStringLine::getGlyphGood(int x) //x为坐标
{
	int pos = getGlyphPos(x);
	if(pos < 0 || pos >= m_words.size())
		return NULL;

	return m_words[pos]->getGood();
}

void CStringLine::clean(const char* str)
{
	m_show = str;
	int pos = 0;

	bool _bModified = false;

	for(size_t j = 0; j < m_words.size();j++)
	{
		CGlyph* pGlyph = m_words.at(j);

		int _size = pGlyph->size();
		string str = m_show.substr(pos,_size);

		if(str.compare(pGlyph->c_str()) != 0) //有改变过
		{
			delete pGlyph;
			m_words[j] = new CChar(str.c_str(),str.size());
			_bModified = true;
		}
		pos += _size;
	}
	if(_bModified)
		updateBuffer();
}

void CStringLine::updateBuffer()
{
	m_show.clear();
	m_buffer.clear();

	for(unsigned int i = 0;i < m_words.size(); i++)
	{
		m_show += m_words[i]->c_str();
		m_buffer.append(m_words[i]->getBuf(),m_words[i]->getBufLength());
	}
}

int CStringLine::getGlyphWidth(int i)
{
	return m_words[i]->size() * m_iFontSize / 2;
}

int CStringLine::getWidth(int begin,int end)
{
	if(end < 0)
		end = (int)m_words.size();

	int _size = 0; //字节数目
	for(int i = begin;i < end; i++)
	{
		_size += m_words[i]->size();
	}
	return _size*m_iFontSize / 2;
}

bool CStringLine::equal(CStringLine* tmp)
{
	if(tmp == NULL)
		return false;

	if(m_buffer.size() != tmp->m_buffer.size())
		return false;

	if(m_buffer == tmp->m_buffer)
		return true;

	return false;
}

void CStringLine::Draw(int x,int y)
{
	TRY_BEGIN;

	//先画基本的部分
	g_pFont->DrawText(x,y,m_show.c_str(),m_dwColor,m_iFontType,m_iFontSize,m_dwFlag,m_dwBack);
	//再画特殊的部分，叠在上面
	int _x = x;
	if(m_dwFlag & DTF_Center)
	{
		_x -= g_pFont->GetLen(m_show.c_str(),NULL,m_iFontSize) / 2;
	}

	for(size_t i = 0;i < m_words.size(); i++)
	{
		bool bUnderline = false;
		DWORD tmpFont = m_dwColor;

		if(m_words[i]->NeedDraw(tmpFont,m_dwBack,bUnderline))
		{
			DWORD dwFlag = m_dwFlag;
			if(bUnderline)
				dwFlag = (dwFlag | DTF_UnderLine) & (~DTF_Center);//居中已经考虑到_x里面了

			g_pFont->DrawText(_x,y,m_words[i]->c_str(),tmpFont,m_iFontType,m_iFontSize,dwFlag,m_dwBack);
		}
		_x += g_pFont->GetLen(m_words[i]->c_str(),NULL,m_iFontSize);
	}

	TRY_END
}

void CStringLine::Draw(int x,int y,int begin,int end)
{
	TRY_BEGIN;
	string str;
	int _iBegin = substr(str,begin,end);
	if(str.empty())
		return;

	DWORD dwInvColor = ~m_dwColor | 0xFF000000;
	DWORD dwInvBack = ~m_dwBack | 0xFF000000;

	int _x = x;
	if(begin > 0)
	{
		string strFrant;
		_iBegin = substr(strFrant,0,begin);
		_x += g_pFont->GetLen(strFrant.c_str(),NULL,m_iFontSize);
	}

	g_pFont->DrawText(_x,y,str.c_str(),dwInvColor,m_iFontType,m_iFontSize,m_dwFlag,dwInvBack);
	TRY_END
}

void CStringLine::DrawPart(int x,int y,int begin,int end)
{
	TRY_BEGIN;
	string str;
	int _iBegin = substr(str,begin,end);
	if(str.empty())
		return;

	//先画基本的部分
	g_pFont->DrawText(x,y,str.c_str(),m_dwColor,m_iFontType,m_iFontSize,m_dwFlag,m_dwBack);

	if(end < 0)
		end = m_words.size();

	//再画特殊的部分，叠在上面
	int _x = x;
	if(m_dwFlag & DTF_Center)
	{
		_x -= g_pFont->GetLen(m_show.c_str(),NULL,m_iFontSize) / 2;
	}

	for(size_t i = begin;i < end; i++)
	{
		bool bUnderline = false;
		DWORD tmpFont = m_dwColor;

		if(m_words[i]->NeedDraw(tmpFont,m_dwBack,bUnderline))
		{
			DWORD dwFlag = m_dwFlag;
			if(bUnderline)
				dwFlag = (dwFlag | DTF_UnderLine) & (~DTF_Center);//居中已经考虑到_x里面了

			g_pFont->DrawText(_x,y,m_words[i]->c_str(),tmpFont,m_iFontType,m_iFontSize,dwFlag,m_dwBack);
		}
		_x += g_pFont->GetLen(m_words[i]->c_str(),NULL,m_iFontSize);
	}
	TRY_END

}
