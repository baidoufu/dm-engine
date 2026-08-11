#include "TalkViewer.h"
#include "GameData/GameData.h"
#include "GameData/GameGlobal.h"
#include "CtrlScroll.h"
#include "Baseclass/Control/ParserTip.h"

#define  ENLARGE_TALK    122

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif

DTI_IMPLEMENT(CTalkViewer, CControlContainer)

CTalkViewer::CTalkViewer(void)
{
	m_iDisLine = 0;
	m_iLineCount = 0;
	m_iLineHeight = FONTSIZE_DEFAULT + 2;
	m_iTotalCount = 0;

	m_iSelBeginRow = -1;
	m_iSelEndRow = -1;
	m_iSelBeginCol = 0;
	m_iSelEndCol = 0;

	m_iDrawOffX = 0;
	m_iDrawOffY = 0;

	m_bSelecting = false;
	m_bAutoWheel = true;
	m_pTalk = NULL;
	m_pScroll = NULL;
	m_pFocusGood = NULL;
	m_pUpButton = NULL;
	m_pDownButton = NULL;
	m_pScroll = NULL;
	m_pSwitchButton = NULL;
	m_strCommand = "";

	m_iBigTalkBack = 0;
	m_bBigTalkViewer = FALSE;
}

CTalkViewer::~CTalkViewer(void)
{
}

void CTalkViewer::OnReSize()
{
	m_iLineCount = m_iHeight / m_iLineHeight;
}

void CTalkViewer::Draw()
{
	if(!m_bShow)
		return;

	if(m_bBigTalkViewer && m_iBigTalkBack > 0)
	{
		DrawTexture(-8,-10,m_iBigTalkBack);
	}

	CControlContainer::Draw();

	if(!m_pTalk)
		return;

	if(m_bAutoWheel)
	{
		WheelToEnd();
	}
	else
	{
		m_iTotalCount = m_pTalk->size();
		if(m_pScroll)
			m_pScroll->SetRange(m_iTotalCount-1);
	}

	int iSize = m_iTotalCount;

	if(m_iDisLine + m_iLineCount < iSize)
		iSize = m_iDisLine + m_iLineCount;

	int iRowBegin, iColBegin;
	int iRowEnd,iColEnd;
	getSelection(iRowBegin,iColBegin,iRowEnd,iColEnd);

	int x = m_iScreenX + m_iDrawOffX;
	int y = m_iScreenY + m_iDrawOffY;

	for(int i = m_iDisLine;i < iSize;i++,y+=m_iLineHeight)
	{
		CStringLine* pLine = m_pTalk->at(i);
		if(pLine == NULL)
			continue;

		if(IsSelected())
		{
			if(i == iRowBegin)
			{
				pLine->Draw(x,y);

				if(i == iRowEnd)
					pLine->Draw(x,y,iColBegin,iColEnd);
				else
					pLine->Draw(x,y,iColBegin);
			}
			else if(i == iRowEnd)
			{
				pLine->Draw(x,y);

				if(i != iRowBegin)  //相同的情况前面已经绘制了
					pLine->Draw(x,y,0,iColEnd);
			}
			else if(i > iRowBegin && i < iRowEnd)
			{
				pLine->Draw(x,y,0);
			}
			else
			{
				pLine->Draw(x,y);
			}
		}
		else
		{
			pLine->Draw(x,y);
		}
	}
}

bool CTalkViewer::Msg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{
	if(dwMsg == MSG_CTRL_SCROLL_MOVE)
	{
		m_iDisLine = dwData;
		return true;
	}
	else if(dwMsg == MSG_CTRL_BUTTON_CLICK)
	{
		if(pControl == m_pSwitchButton)
		{
			SwitchTalkViewer();
			return true;
		}
		else if(pControl == m_pUpButton)
		{
			if(m_pScroll)
				m_pScroll->GoDown();
			else
			{
				m_iDisLine --;
				if(m_iDisLine < 0)
					m_iDisLine = 0;
			}
			return true;
		}
		else if(pControl == m_pDownButton)
		{
			if(m_pScroll)
				m_pScroll->GoUp();
			else
			{
				m_iDisLine ++;
				if(m_iDisLine >= m_pTalk->size())
					m_iDisLine = m_pTalk->size() -1;
			}
			return true;
		}
	}
	return CControlContainer::Msg(dwMsg,dwData,pControl);
}

bool CTalkViewer::OnWheel(int iWheel)
{
	short iWheelSize = (short)iWheel;
	iWheelSize /= WHEEL_DELTA;
	Wheel(-iWheelSize);
	return true;
}

void CTalkViewer::Wheel(int i)
{
	m_iDisLine += i;

	if(m_iDisLine >= m_iTotalCount)
		m_iDisLine = m_iTotalCount - 1;

	if(m_iDisLine < 0)
		m_iDisLine = 0;

	if(m_pScroll)
		m_pScroll->SetPos(m_iDisLine);
}


void CTalkViewer::WheelPage(int i)
{
	Wheel(i * m_iLineCount);
}

void CTalkViewer::WheelToEnd()
{
	if(!m_pTalk)
		return;

	int iOldDisLine = m_iDisLine;

	if(m_pTalk->size() == m_iTotalCount) //行数没有变化时不动
		return;

	int iOldTotalCount = m_iTotalCount;
	m_iTotalCount = m_pTalk->size();

	if(m_iDisLine + m_iLineCount < m_iTotalCount)
	{
		m_iDisLine = m_iTotalCount - m_iLineCount;
	}
	else if(m_iDisLine >= m_iTotalCount)
	{
		//m_iDisLine = m_iTotalCount - m_iLineCount;
		if(iOldTotalCount-iOldDisLine >= m_iLineCount)
			m_iDisLine = m_iTotalCount - m_iLineCount;
		else
			m_iDisLine = m_iTotalCount - (iOldTotalCount-iOldDisLine) - 1;//为了每次在上一次的基础上多一行
	}

	if(m_iDisLine < 0)
	{
		m_iDisLine = 0;
	}
	else if(m_iDisLine >= m_iTotalCount)
	{
		m_iDisLine = m_iTotalCount - m_iLineCount;
	}

	if(m_pScroll)
	{
		m_pScroll->SetRange(m_iTotalCount-1);
		m_pScroll->SetPos(m_iDisLine);
	}
}

bool CTalkViewer::OnLeftButtonDown(int iX,int iY)
{
	m_iSelBeginRow = m_iSelEndRow = -1;
	m_iSelBeginCol = m_iSelEndCol = 0;

	int col = 0;
	int row = getLineByXY(iX,iY,col);
	if(row < 0)
		return false;

	m_iSelBeginRow = row;
	m_iSelBeginCol = col;

	m_bSelecting = true;
	return true;		
}

bool CTalkViewer::OnLeftButtonUp(int iX,int iY)
{
	m_bSelecting = false;

	int col = 0;
	int row = getLineByXY(iX,iY,col);
	if(row < 0)
		return false;

	m_iSelEndRow = row;
	m_iSelEndCol = col;

	m_strCommand.clear();
	if(row < m_pTalk->size())
	{
		CStringLine* pLine = m_pTalk->at(row);
		if(pLine)
		{
			CGlyph* pGlyph = pLine->getGlyph(col);
			if(pGlyph)
			{
				const char* szCommand = pGlyph->getCommand();
				if(szCommand)
					m_strCommand = szCommand;
			}
		}
	}

	if(m_iSelEndRow == m_iSelBeginRow && m_iSelEndCol == m_iSelBeginCol && m_pParent)
		m_pParent->Msg(MSG_CTRL_TALKVIEW_LCLICK,m_iSelBeginRow);

	return true;
}

bool CTalkViewer::OnMouseMove(int iX,int iY)
{
	m_pFocusGood = NULL;
	CParserTip *pTip = g_pControl->GetTipWnd();
	pTip->SetShow(false);

	if(m_bSelecting)
	{
		int col = 0;
		int row = getLineByXY(iX,iY,col);
		if(row < 0)
		{
			m_bSelecting = false;
			return true;
		}

		m_iSelEndRow = row;
		m_iSelEndCol = col;

		return true;
	}
	else
	{
		m_pFocusGood = getGoodByXY(iX,iY);
	}

	if(m_pFocusGood)
	{
		pTip->Parse(*m_pFocusGood);

		int x = m_iScreenX + iX;
		int y = m_iScreenY + iY - 10;
		pTip->AdjustXY(x,y);
		pTip->Move(x,y);
		pTip->SetShow(true);
	}

	return true;
}

const char* CTalkViewer::GetLine(int x,int y)
{
	int col = 0;
	int row = getLineByXY(x,y,col);
	if(row < 0 || !m_pTalk)
		return "";

	if(m_pScroll)
		row += m_pScroll->GetPos();
	if(row >= m_pTalk->size())
		return "";

	CStringLine* pLine = m_pTalk->at(row);
	if(!pLine)
		return "";

	return pLine->c_str();
}

void CTalkViewer::CopyToClipboard()
{
	int iRowBegin, iColBegin;
	int iRowEnd,iColEnd;

	if(!IsSelected())
		return;

	getSelection(iRowBegin,iColBegin,iRowEnd,iColEnd);
	std::string strTmp,str;

	for(int i = iRowBegin;i <= iRowEnd && i < m_iTotalCount;i++)
	{
		CStringLine* pLine = m_pTalk->at(i);;
		if(pLine == NULL)
			continue;

		if(i == iRowBegin)
		{
			if(i == iRowEnd)
				pLine->substr(str,iColBegin,iColEnd);
			else
				pLine->substr(str,iColBegin,-1);
		}
		else if(i == iRowEnd)
		{
			if(i != iRowBegin)
				pLine->substr(str,0,iColEnd);
		}
		else if(i > iRowBegin && i < iRowEnd)
		{
			str = pLine->c_str();
		}
		strTmp += str + "\n";
	}
	if(strTmp.size() != 0)
	{
		CopyText2Clip(strTmp.c_str(),strTmp.size());
	}
}

bool CTalkViewer::IsSelected()
{
	if(m_iSelBeginRow < 0 || m_iSelEndRow < 0)
		return false;

	if(m_iSelBeginRow != m_iSelEndRow || (m_iSelBeginCol != m_iSelEndCol))
		return true;

	return false;
}

bool CTalkViewer::getLineHeader(std::string& strLine)
{
	if(m_iSelBeginRow < 0 || m_iSelBeginRow >= m_pTalk->size())
		return false;

	CStringLine* pLine = m_pTalk->at(m_iSelBeginRow);
	if(!pLine)
		return false;

	strLine = pLine->c_str();

	bool bFlag = false;
	char sTemp[512] = "/";

	if(strLine[0] == '/' || strLine[0] == '#')
	{
		bFlag = true;
		strcat(sTemp,strLine.c_str() + 1);
	}
	else if(strLine.substr(0,3) == "(!)")
	{
		bFlag = true;
		strcat(sTemp,strLine.c_str() + 3);
	}
	else if(strLine.size() > 5 && strLine[0] == '//' && strLine[1] == '<'  && strLine[4] == '>')
	{
		strcat(sTemp,strLine.c_str() + 5);
	}
	else if(strLine.size() > 4 && strLine[0] == '<'  && strLine[3] == '>')
	{
		strcat(sTemp,strLine.c_str() + 4);
	}
	else
	{
		strcat(sTemp,strLine.c_str());
	}
	int nPos = strcspn(sTemp,": =");
	if(nPos < strlen(sTemp))
	{
		sTemp[nPos] = '\0';
		bFlag = true;
	}
	if(bFlag)
	{
		strcat(sTemp," ");
		strLine = sTemp;
		return true;
	}
	return false;
}

int CTalkViewer::getLineByXY(int x,int y,int & col)
{
	x -= m_iDrawOffX;
	y -= m_iDrawOffY;

	int iRow = y / m_iLineHeight;
	if(iRow < 0 || iRow >= m_iLineCount)
		return -1;

	if(m_iDisLine + iRow >= m_iTotalCount && m_iTotalCount > m_iDisLine)
		iRow = m_iTotalCount - m_iDisLine - 1;

	iRow += m_iDisLine;

	if(iRow >= m_pTalk->size())
		return -1;

	CStringLine* pLine = m_pTalk->at(iRow);
	if(pLine == NULL)
		return -1;

	col = pLine->getGlyphPos(x);
	if(col < 0)
		return -1;

	return iRow;
}

CGood* CTalkViewer::getGoodByXY(int x,int y)
{
	if(!m_pTalk)
		return NULL;

	x -= m_iDrawOffX;
	y -= m_iDrawOffY;

	int iRow = y / m_iLineHeight;
	if(iRow < 0 || iRow >= m_iLineCount)
		return NULL;

	if(m_iDisLine + iRow >= m_iTotalCount && m_iTotalCount > m_iDisLine)
		iRow = m_iTotalCount - m_iDisLine - 1;

	iRow += m_iDisLine;
	WORD wColor = 0;

	if(iRow >= m_pTalk->size())
		return NULL;

	return (m_pTalk->at(iRow)->getGlyphGood(x));
}

void CTalkViewer::getSelection(int& rowBegin,int & colBegin,int & rowEnd,int & colEnd)
{
	if(m_iSelBeginRow < m_iSelEndRow || (m_iSelBeginRow == m_iSelEndRow  &&  m_iSelBeginCol < m_iSelEndCol))
	{
		rowBegin = m_iSelBeginRow;
		colBegin = m_iSelBeginCol;

		rowEnd = m_iSelEndRow;
		colEnd = m_iSelEndCol;
	}
	else
	{
		rowBegin = m_iSelEndRow;
		colBegin = m_iSelEndCol;

		rowEnd = m_iSelBeginRow;
		colEnd = m_iSelBeginCol;
	}
}

void CTalkViewer::AddUpButton(int iX,int iY,WORD wTex,WORD wMTex,WORD wCTex,int iAlignType)
{
	if(m_pUpButton)
		RemoveControl((CControl**)&m_pUpButton);

	m_pUpButton = new CCtrlButton();
	m_pUpButton->SetAlignType(iAlignType);
	AddControl(m_pUpButton);
	m_pUpButton->CreateEx(this,iX,iY,wTex,wMTex,wCTex);
}

void CTalkViewer::AddDownButton(int iX,int iY,WORD wTex,WORD wMTex,WORD wCTex,int iAlignType)
{
	if(m_pDownButton)
		RemoveControl((CControl**)&m_pDownButton);

	m_pDownButton = new CCtrlButton();
	m_pDownButton->SetAlignType(iAlignType);
	AddControl(m_pDownButton);
	m_pDownButton->CreateEx(this,iX,iY,wTex,wMTex,wCTex);
}

void CTalkViewer::AddSwitchButton(int iX,int iY,WORD wTex,WORD wMTex,WORD wCTex,int iAlignType)
{
	if(m_pSwitchButton)
		RemoveControl((CControl**)&m_pSwitchButton);

	m_pSwitchButton = new CCtrlButton();
	m_pSwitchButton->SetAlignType(iAlignType);
	AddControl(m_pSwitchButton);
	m_pSwitchButton->CreateEx(this,iX,iY,wTex,wMTex,wCTex);
}

void CTalkViewer::AddScroll(int iX,int iY,int iW,int iH,int iScrollIdx,int iAlignType)
{
	if(m_pScroll)
		RemoveControl((CControl**)&m_pScroll);

	m_pScroll = new CCtrlScroll();
	m_pScroll->SetAlignType(iAlignType);
	AddControl(m_pScroll);
	m_pScroll->Create(this,iX,iY,iW,iH,iScrollIdx);
}

void CTalkViewer::AddScrollEx(int iX,int iY,int iW,int iH,bool bShow,int iStep,int iAlignType)
{
	if(m_pScroll)
		RemoveControl((CControl**)&m_pScroll);

	m_pScroll = new CCtrlScroll();
	m_pScroll->SetAlignType(iAlignType);
	AddControl(m_pScroll);
	m_pScroll->CreateEx(this,iX,iY,iW,iH);
	m_pScroll->SetShow(bShow);
	if (iStep > 0)
	{
		m_pScroll->SetStep(iStep);
	}
}

void CTalkViewer::SwitchTalkViewer()
{
	m_bBigTalkViewer = !m_bBigTalkViewer;

	//聊天背景
	if(m_bBigTalkViewer)
	{
		Move(POS_NOMOVE,GetY()-ENLARGE_TALK);
		ReSize(POS_NORESIZE,GetOriginalHeight()+ENLARGE_TALK);
		if(m_pScroll)
		{
			m_pScroll->Move(POS_NOMOVE,m_pScroll->GetY() - 7);
			m_pScroll->ReSize(POS_NORESIZE,m_pScroll->GetOriginalHeight() + ENLARGE_TALK + 7);
		}
		if(m_pSwitchButton)
			m_pSwitchButton->Move(POS_NOMOVE,m_pSwitchButton->GetY() - 7);
	}
	else
	{
		Move(POS_NOMOVE,GetY()+ENLARGE_TALK);
		ReSize(POS_NORESIZE,GetOriginalHeight()-ENLARGE_TALK);
		if(m_pScroll)
		{
			m_pScroll->Move(POS_NOMOVE,m_pScroll->GetY() + 7);
			m_pScroll->ReSize(POS_NORESIZE,m_pScroll->GetOriginalHeight()-ENLARGE_TALK - 7);
		}
		if(m_pSwitchButton)
			m_pSwitchButton->Move(POS_NOMOVE,m_pSwitchButton->GetY() + 7);
	}
    if(m_pScroll)
    {
        m_pScroll->SetPos(0);       //重新设置拖动框位置
        m_pScroll->SetPos(m_iDisLine);
    }
    
}

bool CTalkViewer::OnKeyDown(WORD wState,UCHAR cKey)
{
	switch(cKey)
	{
	case 'C':
		if(g_pInput->IsCtrl())
		{
			CopyToClipboard();
			return true;
		}
		break;
	case VK_UP:                                       
		{
			Wheel(-1);
			m_pScroll->SetPos(m_iDisLine);
			return true;
		}
	case VK_DOWN:
		{
			Wheel(1);
			m_pScroll->SetPos(m_iDisLine);
			return true;
		}
	case VK_PRIOR:
		{
			WheelPage(-1);
			m_pScroll->SetPos(m_iDisLine);
			return true;
		}
	case VK_NEXT:
		{
			WheelPage(1);
			m_pScroll->SetPos(m_iDisLine);
			return true;
		}
	}
	return CControlContainer::OnKeyDown(wState,cKey);
}

