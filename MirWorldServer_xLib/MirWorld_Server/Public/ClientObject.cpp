#include "StdAfx.h"
#include ".\clientobject.h"

CClientObject::CClientObject(VOID)
{
	m_xParsePacket.create(65536);
	m_xSendPacket.create(65536);
	m_xDecodePacket.create(65536);
	SetMsgProcessor(this);
}

CClientObject::~CClientObject(VOID)
{
	m_xParsePacket.destroy();
	m_xSendPacket.destroy();
	m_xDecodePacket.destroy();
}

VOID CClientObject::Clean(VOID)
{
	xClientObject::Clean();
	m_dwClientKey = CFrameTime::GetFrameTime();
	m_Pingtimer.Savetime();
	m_bPingNoRet = FALSE;
	m_dwPing = 0;
	m_xParsePacket.clear();
	m_xSendPacket.clear();
	m_xDecodePacket.clear();
}

BOOL CClientObject::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int datasize)
{
	m_xSendPacket.clear();
	char* pBuf = const_cast<char*>(m_xSendPacket.getfreebuf());
	int length = EncodeMsg(pBuf, dwFlag, wCmd, w1, w2, w3, lpData, datasize);
	m_xSendPacket.addsize(length);
	return PostMsg(m_xSendPacket.getbuf(), length);
}

BOOL CClientObject::SendMsg(PMIRMSG pMsg, int datasize)
{
	m_xSendPacket.clear();
	char* pBuf = const_cast<char*>(m_xSendPacket.getfreebuf());
	pBuf[0] = '#';
	int length = _CodeGameCode((BYTE*)pMsg, datasize + sizeof(MIRMSGHEADER), (BYTE*)(pBuf + 1));
	pBuf[length + 1] = '!';
	m_xSendPacket.addsize(length + 2);
	return PostMsg(m_xSendPacket.getbuf(), length + 2);
}

BOOL CClientObject::PostMsg(const char* pszCodedMsg, int size)
{
	return postSendBatch((LPVOID)pszCodedMsg, size);
}

VOID CClientObject::Update()
{
	UpdateStarPing();
	xClientObject::Update();
}

VOID CClientObject::OnClientPingRet()
{
	m_dwPing = CFrameTime::GetFrameTime() - m_Pingtimer.GetSavedTime();
}

VOID CClientObject::OnSCMsg_T(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_MSGACROSSSERVER:
	{
		OnMASMsg(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	}
	break;
	default:
		OnSCMsg(pMsg, datasize);
		break;
	}
}

VOID CClientObject::OnDataPacket(xPacket* pPacket)
{
	if (!m_xParsePacket.push((LPVOID)pPacket->getbuf(), static_cast<int>(pPacket->getsize())))
	{
		Disconnect();
		return;
	}
	char* pszMsg = (char*)m_xParsePacket.getbuf();
	int msgptr = 0;
	int parsesize = 0;
	const int allsize = static_cast<int>(m_xParsePacket.getsize());
	if (allsize == 0) return;
	do
	{
		parsesize = ParseMessage(pszMsg + msgptr, allsize - msgptr);
		msgptr += parsesize;
	} while (parsesize > 0 && allsize > msgptr);

	if (msgptr >= allsize)
		m_xParsePacket.clear();
	else
		m_xParsePacket.free(msgptr);
}

int	CClientObject::ParseMessage(char* pszMsg, int iSize)
{
	m_xDecodePacket.clear();
	char* szDecodeBuffer = const_cast<char*>(m_xDecodePacket.getfreebuf());
	char* pStart = nullptr;
	int ParsedSize = 0;
	for (int i = 0; i < iSize; ++i)
	{
		const char ch = pszMsg[i];
		if (ch == '*')
		{
			if (m_bPingNoRet)
			{
				m_bPingNoRet = FALSE;
				OnClientPingRet();
			}
			ParsedSize = i + 1;
		}
		else if (ch == '#')
		{
			pStart = pszMsg + i + 1;
		}
		else if (ch == '!')
		{
			if (pStart != nullptr)
			{
				pszMsg[i] = 0;
				if (*pStart == '+')
				{
					if (m_pMsgProcessor)
						m_pMsgProcessor->OnUnCodeMsg(this, pStart, static_cast<int>(pszMsg + i - pStart));
				}
				else
				{
					if (*pStart >= '0' && *pStart <= '9') ++pStart;
					const int encodedLen = static_cast<int>(pszMsg + i - pStart);
					// 编码数据长度上限检查：编码后长度不可能小于原始数据的1/2，
					// 如果编码长度超过缓冲区容量的3/4，解码后极可能溢出
					const int maxEncodedLen = static_cast<int>(m_xDecodePacket.getfreesize()) * 3 / 4;
					if (encodedLen > maxEncodedLen || encodedLen <= 0)
					{
						// 编码数据异常，丢弃此消息防止缓冲区溢出
						pStart = nullptr;
						ParsedSize = i + 1;
						break;
					}
					const int length = static_cast<int>(_UnGameCode(pStart, (BYTE*)szDecodeBuffer));
					// 解码后长度安全检查
					if (length <= 0 || length > static_cast<int>(m_xDecodePacket.getfreesize()))
				{
					// 解码数据异常，丢弃此消息
					pStart = nullptr;
					ParsedSize = i + 1;
					break;
				}
				// 消息长度必须至少包含消息头, 防止短包导致整数下溢和堆越界读
				int headerLen = sizeof(MIRMSGHEADER);
				if (length < headerLen)
				{
					pStart = nullptr;
					ParsedSize = i + 1;
					break;
				}
				if (m_pMsgProcessor)
					m_pMsgProcessor->OnCodedMsg(this, (MIRMSG*)szDecodeBuffer, length - headerLen);
				}
				pszMsg[i] = '!';
				pStart = nullptr;
			}
			ParsedSize = i + 1;
		}
	}
	return ParsedSize;
}
