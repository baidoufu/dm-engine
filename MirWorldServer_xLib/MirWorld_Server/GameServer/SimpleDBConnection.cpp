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

static thread_local std::array<char, 65536> s_szTemp{};
VOID CSimpleDBConnection::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int datasize)
{
	// 长度校验: _CodeGameCode编码每3字节输入产生4字节输出(膨胀率4/3), MIRMSGHEADER=12字节
	// 估算编码后长度 = 1('#') + ceil((12+datasize)*4/3) + 1('!') + 1('\0') + 对齐余量
	// 若超过szTemp容量则拒绝发送, 避免EncodeMsg缓冲区溢出
	if (lpData != nullptr)
	{
		int actualDataSize = (datasize < 0) ? (int)strlen((char*)lpData) : datasize;
		const int ENCODE_OVERHEAD = 24; // '#' '!' '\0' + header(12字节)编码膨胀 + 尾部对齐余量
		int estimatedCodedSize = (int)((12LL + actualDataSize) * 4 / 3) + ENCODE_OVERHEAD;
		if (estimatedCodedSize > (int)s_szTemp.size())
		{
			PRINT(ERROR_RED, "CSimpleDBConnection::SendMsg 拒绝发送: 编码后估算长度(%d)超过缓冲区(%zu), wCmd=0x%X datasize=%d\n",
				estimatedCodedSize, s_szTemp.size(), wCmd, actualDataSize);
			return;
		}
	}
	int length = EncodeMsg(s_szTemp.data(), dwFlag, wCmd, w1, w2, w3, lpData, datasize);
	// 双保险: 校验EncodeMsg返回长度合法, 避免后续push越界
	if (length <= 0 || length > (int)s_szTemp.size())
	{
		PRINT(ERROR_RED, "CSimpleDBConnection::SendMsg 编码长度非法: length=%d, wCmd=0x%X\n", length, wCmd);
		return;
	}
	m_xSendPacket.push((LPVOID)s_szTemp.data(), length);
	length = Send((LPVOID)m_xSendPacket.getbuf(), m_xSendPacket.getsize());
	if (length < 1)return;
	if (length == m_xSendPacket.getsize())
		m_xSendPacket.clear();
	else
		m_xSendPacket.free(length);
}

static thread_local std::array<char, 65536> s_szTempBuffer2{};
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
					// 编码数据长度上限检查：防止解码后溢出s_szTempBuffer2
					const int maxEncodedLen = static_cast<int>(s_szTempBuffer2.size()) * 3 / 4;
					if (encodedLen > maxEncodedLen || encodedLen <= 0)
					{
						pStart = nullptr;
						ParsedSize = i + 1;
						break;
					}
					int length = _UnGameCode(pStart, (BYTE*)s_szTempBuffer2.data());
					// 解码后长度安全检查
					if (length <= 0 || length > static_cast<int>(s_szTempBuffer2.size()))
					{
						pStart = nullptr;
						ParsedSize = i + 1;
						break;
					}
					//if( m_pMsgProcessor )
					//	m_pMsgProcessor->OnMsg( this, (MIRMSG*)s_szTempBuffer2.data(), length - sizeof( MIRMSGHEADER ) );
					//else
					OnMsg((MIRMSG*)s_szTempBuffer2.data(), length - sizeof(MIRMSGHEADER));
				}
				pszMsg[i] = '!';
				pStart = nullptr;
			}
			ParsedSize = i + 1;
		}
	}
	return ParsedSize;
}
