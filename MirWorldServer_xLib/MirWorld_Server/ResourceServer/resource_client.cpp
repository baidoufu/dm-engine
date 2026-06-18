#include "stdafx.h"
#include "resource_client.h"
#include "resource_server.h"

CResourceClient::CResourceClient(VOID)
{
}

CResourceClient::~CResourceClient(VOID)
{
}

VOID CResourceClient::Clean()
{
	CClientObject::Clean();
}

VOID CResourceClient::OnConnection()
{
	EnableBatchMode(TRUE);
	m_TimeOut.Savetime();
}

VOID CResourceClient::OnDisconnect()
{
}

VOID CResourceClient::Update()
{
	if (m_TimeOut.IsTimeOut(HEARTBRATTIME))
	{
		Disconnect();
		return;
	}
	CClientObject::Update();
}

VOID CResourceClient::OnDataPacket(xPacket* pPacket)
{
	HandleDownloadRequest(pPacket->getbuf(), pPacket->getsize());
}

VOID CResourceClient::HandleDownloadRequest(const char* buffer, int len)
{
	size_t offset = sizeof(STDlNetMsgHeader);
	if (len < (int)offset) return;
	SDLProtocalHeader* header = (SDLProtocalHeader*)(buffer + offset);
	offset += sizeof(SDLProtocalHeader);
	if (len < (int)offset) return;
	//协议判断
	switch (header->wProtocal)
	{
	case 0x1001: //请求下载
	{
		if (header->wFileCount == 0) return;
		FileManager* pFM = FileManager::GetInstance();
		if (!pFM) return;
		for (int i = 0; i < header->wFileCount; i++)
		{
			if (offset + sizeof(SDlProtocalBody) > (size_t)len) return;
			SDlProtocalBody* body = (SDlProtocalBody*)(buffer + offset);
			FileInfo* fileInfo = pFM->FindFileByHash(body->i64Hash);
			if (!fileInfo)
			{
				offset += sizeof(SDlProtocalBody);
				continue;
			}
			DPRINT(SUCCESS_GREEN, "%llX  \n%s \n\n", fileInfo->hash, fileInfo->filePath.c_str());
			SendFileToClient(fileInfo, body->dwStartPos, body->dwID, body->iWpf);
			offset += sizeof(SDlProtocalBody);
		}
	}
	break;
	case 0x1002: //心跳
	{
		m_TimeOut.Savetime();
	}
	break;
	case 0x1003: //确认收包
	{

	}
	break;
	}
}

VOID CResourceClient::SendFileToClient(FileInfo* fileInfo, uint32_t startPos, uint32_t requestID, uint32_t wpfIndex)
{
	FileManager* pFM = FileManager::GetInstance();
	if (!pFM) return;
	std::vector<uint8_t> fileData; // 文件数据
	if (!pFM->ReadFileRange(fileInfo->filePath, startPos, fileInfo->size, fileData))
		return;
	if (fileData.empty())return; // 数据等于空时
	//组包数据
	xPacketPool::ScopedPacket packet;
	uint32_t dataLen = sizeof(SDLProtocalHeader) + 1 + (uint32_t)fileInfo->filePath.size() + (uint32_t)fileData.size();
	packet->create(sizeof(STDlNetMsgHeader) + dataLen);
	//消息头
	STDlNetMsgHeader msgHeader{};
	msgHeader.wMsgID = ntohs(0x0BB9); //小端序
	msgHeader.dwDataLen = htonl(dataLen); //小端序
	packet->push(&msgHeader, sizeof(STDlNetMsgHeader));
	//协议头
	SDLProtocalHeader protoHeader{};
	protoHeader.wProtocal = 0x1001;
	protoHeader.wFileCount = 1;
	protoHeader.dwID = requestID;
	protoHeader.i64Hash = fileInfo->hash;
	protoHeader.iWpf = wpfIndex;
	protoHeader.dwAttribute = EFA_FILE;
	protoHeader.dwLen = 0;
	protoHeader.dwStartPos = 0;
	protoHeader.dwLastDWORD = *(uint32_t*)&fileData[fileData.size() - 4];
	protoHeader.dwRev = 0;
	packet->push(&protoHeader, sizeof(SDLProtocalHeader));
	//文件数据
	uint8_t pathLen = (uint8_t)fileInfo->filePath.size();
	packet->push(&pathLen, 1);
	packet->push((LPVOID)fileInfo->filePath.data(), (int)fileInfo->filePath.size());
	packet->push((LPVOID)fileData.data(), (int)fileData.size());

	postSend(packet.get());
}