#include "StdAfx.h"
#include ".\simpledbconnection.h"

CSimpleDBConnection::CSimpleDBConnection(VOID)
{
	this->m_xRecvPacket.create(65536);
	this->m_xSendPacket.create(65536);
}

CSimpleDBConnection::~CSimpleDBConnection(VOID)
{
	this->m_xRecvPacket.destroy();
	this->m_xSendPacket.destroy();
}

VOID CSimpleDBConnection::Update()
{
	BOOL bSend = FALSE, bRecv = FALSE;
	if (Select(&bRecv, &bSend, nullptr, 10))
	{
		if (bRecv && m_xRecvPacket.getfreesize() > 0)
		{
			int length = Recv((LPVOID)m_xRecvPacket.getfreebuf(), m_xRecvPacket.getfreesize());
			if (length > 0)
				m_xRecvPacket.addsize(length);
		}
		if (m_xRecvPacket.getsize() > 0)
		{
			char* pszMsg = (char*)m_xRecvPacket.getbuf();
			int msgptr = 0;
			int parsesize = 0;
			int allsize = m_xRecvPacket.getsize();
			if (allsize == 0)return;
			do
			{
				parsesize = ParseMessage(pszMsg + msgptr, allsize - msgptr);
				msgptr += parsesize;
			} while (parsesize > 0 && allsize > msgptr);

			if (msgptr >= allsize)
				m_xRecvPacket.clear();
			else
				m_xRecvPacket.free(msgptr);
		}
		if (m_xSendPacket.getsize() > 0)
		{
			int length = m_xSendPacket.getsize();
			if (length > 1024)length = 1024;
			length = Send((LPVOID)m_xSendPacket.getbuf(), length);
			if (length > 0)
			{
				if (length == m_xSendPacket.getsize())
					m_xSendPacket.clear();
				else
					m_xSendPacket.free(length);
			}
		}
	}
}

static thread_local std::array<char, 65536> szTemp{};
VOID CSimpleDBConnection::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int datasize)
{
	int length = EncodeMsg(szTemp.data(), dwFlag, wCmd, w1, w2, w3, lpData, datasize);
	m_xSendPacket.push((LPVOID)szTemp.data(), length);
	length = Send((LPVOID)m_xSendPacket.getbuf(), m_xSendPacket.getsize());
	if (length < 1)return;
	if (length == m_xSendPacket.getsize())
		m_xSendPacket.clear();
	else
		m_xSendPacket.free(length);
}

static thread_local std::array<char, 65536> g_szTempBuffer2{};
int	CSimpleDBConnection::ParseMessage(char* pszMsg, int iSize)
{
	char* pStart = nullptr;
	int ParsedSize = 0;
	for (int i = 0; i < iSize; i++)
	{
		if (pszMsg[i] == '*')
		{
			//if( m_bPingNoRet )
			//{
			//	m_bPingNoRet = FALSE;
			//	OnClientPingRet();
			//}
			ParsedSize = i + 1;
		}
		else if (pszMsg[i] == '#')
		{
			pStart = pszMsg + i + 1;
		}
		else if (pszMsg[i] == '!')
		{
			if (pStart != nullptr)
			{
				pszMsg[i] = 0;
				if (*pStart == '+')
				{
					//
					//if( m_pMsgProcessor )
					//	m_pMsgProcessor->OnStringMsg( this, pStart );
					//else
					//	OnStringMsg( pStart );
				}
				else
				{
					if (*pStart >= '0' && *pStart <= '9')pStart++;
					const int encodedLen = static_cast<int>(pszMsg + i - pStart);
					// 编码数据长度上限检查：防止解码后溢出g_szTempBuffer2
					const int maxEncodedLen = static_cast<int>(g_szTempBuffer2.size()) * 3 / 4;
					if (encodedLen > maxEncodedLen || encodedLen <= 0)
					{
						pStart = nullptr;
						ParsedSize = i + 1;
						break;
					}
					int length = _UnGameCode(pStart, (BYTE*)g_szTempBuffer2.data());
					// 解码后长度安全检查
					if (length <= 0 || length > static_cast<int>(g_szTempBuffer2.size()))
					{
						pStart = nullptr;
						ParsedSize = i + 1;
						break;
					}
					//if( m_pMsgProcessor )
					//	m_pMsgProcessor->OnMsg( this, (MIRMSG*)g_szTempBuffer2.data(), length - sizeof( MIRMSGHEADER ) );
					//else
					OnMsg((MIRMSG*)g_szTempBuffer2.data(), length - sizeof(MIRMSGHEADER));
				}
				pszMsg[i] = '!';
				pStart = nullptr;
			}
			ParsedSize = i + 1;
		}
	}
	return ParsedSize;
}
