#include "stdafx.h"
#include "resource_client.h"
#include "resource_server.h"
#include <vector>

CResourceClient::CResourceClient(VOID)
{
}

CResourceClient::~CResourceClient(VOID)
{
}

VOID CResourceClient::Clean()
{
	m_pendingSendQueue.clear();
	CClientObject::Clean();
}

VOID CResourceClient::OnConnection()
{
	EnableBatchMode(TRUE);
	m_TimeOut.Savetime();
	DPRINT(SUCCESS_GREEN, "[%p][新连接] 客户端已连接\r\n", this);
}

VOID CResourceClient::OnDisconnect()
{
	DPRINT(WARN_YELLOW, "[%p][断开] 客户端断开连接\r\n", this);
}

VOID CResourceClient::Update()
{
	if (m_TimeOut.IsTimeOut(HEARTBRATTIME))
	{
		Disconnect();
		return;
	}

	// 限速发送: 每tick最多发送 MAX_SEND_BYTES_PER_TICK 字节, 防止突发流量堵死客户端TCP窗口
	uint32_t sentBytes = 0;
	while (!m_pendingSendQueue.empty() && sentBytes < MAX_SEND_BYTES_PER_TICK)
	{
		auto& buf = m_pendingSendQueue.front();
		postSend((LPVOID)buf.data(), (int)buf.size());
		sentBytes += (uint32_t)buf.size();
		m_pendingSendQueue.pop_front();
	}

	xClientObject::Update();  // 跳过CClientObject的UpdateStarPing(*保活),避免污染TCP流
}

VOID CResourceClient::OnDataPacket(xPacket* pPacket)
{
	m_TimeOut.Savetime();
	int iLen = pPacket->getsize();
	DPRINT(CYAN, "[%p][数据到达] len=%d\r\n", this, iLen);
	if (iLen == 0) return;
	HandleDownloadRequest(pPacket->getbuf(), iLen);
}

VOID CResourceClient::HandleDownloadRequest(const char* buffer, int len)
{
	// 线格式: STNetMsgHeader(4B) + Encoded_wMsgLen(2B) + SDLProtocalHeader + SDlProtocalBody[]
	size_t offset = 6; // STNetMsgHeader(4B) + Encoded_wMsgLen(2B)
	if (len < (int)(offset + sizeof(SDLProtocalHeader))) return;
	SDLProtocalHeader* header = (SDLProtocalHeader*)(buffer + offset);
	offset += sizeof(SDLProtocalHeader);
	if (len < (int)offset) return;
	switch (header->wProtocal)
	{
	case 0x1001:
	{
		m_TimeOut.Savetime();
		if (header->wFileCount == 0) return;
		FileManager* pFM = FileManager::GetInstance();
		if (!pFM) return;
		uint64_t dwReqTick = GetSteadyTimeMS64();
		DPRINT(COOL_BLUE, "[%p][C->S][%llu] 收到下载请求, 文件数=%d\r\n", this, dwReqTick, header->wFileCount);
		struct BatchItem { FileInfo* fileInfo = nullptr; std::vector<uint8_t> fileData; uint32_t requestID = 0; uint32_t wpfIndex = 0; uint32_t startPos = 0; };
		std::vector<BatchItem> batchItems;
		batchItems.reserve(header->wFileCount);
		for (int i = 0; i < header->wFileCount; i++)
		{
			if (offset + sizeof(SDlProtocalBody) > (size_t)len) return;
			SDlProtocalBody* body = (SDlProtocalBody*)(buffer + offset);
			FileInfo* fileInfo = pFM->FindFileByHash(body->i64Hash);
			if (!fileInfo)
			{
				PRINT(WARN_YELLOW, "[%p][C->S] 文件未找到 Hash=%016llX\r\n", this, body->i64Hash);
				offset += sizeof(SDlProtocalBody);
				continue;
			}
			DPRINT(SUCCESS_GREEN, "[%p]  [找到] Hash=%016llX (%u 字节)\r\n", this, body->i64Hash, fileInfo->size);
			BatchItem item;
			item.fileInfo = fileInfo;
			item.requestID = body->dwID;
			item.wpfIndex = body->iWpf;
			item.startPos = body->dwStartPos;
			uint32_t actualLen = (body->dwStartPos < fileInfo->size) ? (fileInfo->size - body->dwStartPos) : 0;
			if (actualLen > 0 && pFM->ReadFileByHash(body->i64Hash, body->dwStartPos, actualLen, item.fileData)
				&& item.fileData.size() >= 4)
			{
				batchItems.push_back(std::move(item));
			}
			offset += sizeof(SDlProtocalBody);
		}
		if (batchItems.empty())
		{
			PRINT(WARN_YELLOW, "[%p][C->S] 所有文件均未找到, 不发送响应\r\n", this);
			return;
		}
		DPRINT(SUCCESS_GREEN, "[%p][C->S] 成功读取 %d 个文件, 准备发送\r\n", this, (int)batchItems.size());
		static constexpr uint32_t MAX_BATCH_BYTES = 1024 * 1024;
		size_t fileIdx = 0;
		while (fileIdx < batchItems.size())
		{
			uint32_t batchBytes = 0;
			size_t batchStart = fileIdx;
			while (fileIdx < batchItems.size())
			{
				uint32_t itemBytes = sizeof(STDlNetMsgHeader) + sizeof(SDLProtocalHeader) + 1
					+ (uint32_t)batchItems[fileIdx].fileInfo->filePath.size()
					+ (uint32_t)batchItems[fileIdx].fileData.size();
				if (batchBytes > 0 && batchBytes + itemBytes > MAX_BATCH_BYTES) break;
				batchBytes += itemBytes;
				fileIdx++;
			}
			std::vector<char> rawBuf(batchBytes);
			size_t wpos = 0;
			for (size_t k = batchStart; k < fileIdx; k++)
			{
				auto& item = batchItems[k];
				uint32_t dataLen = sizeof(SDLProtocalHeader) + 1
					+ (uint32_t)item.fileInfo->filePath.size()
					+ (uint32_t)item.fileData.size();
				STDlNetMsgHeader msgHeader{};
				msgHeader.wMsgID = htons(0x0BB9);
				msgHeader.dwDataLen = htonl(dataLen);
				memcpy(&rawBuf[wpos], &msgHeader, sizeof(msgHeader)); wpos += sizeof(msgHeader);
				SDLProtocalHeader protoHeader{};
				protoHeader.wProtocal = 0x1001;
				protoHeader.wFileCount = 1;
				protoHeader.dwID = item.requestID;
				protoHeader.i64Hash = item.fileInfo->hash;
				protoHeader.iWpf = item.wpfIndex;
				protoHeader.dwAttribute = EFA_FILE;
				protoHeader.dwLen = item.fileInfo->size;       // 文件总长度, 供客户端显示下载进度
				protoHeader.dwStartPos = item.startPos;        // 本次发送启始位置, 支持断点续传
				protoHeader.dwLastDWORD = *(uint32_t*)&item.fileData[item.fileData.size() - 4];
				protoHeader.dwRev = 0;
				memcpy(&rawBuf[wpos], &protoHeader, sizeof(protoHeader)); wpos += sizeof(protoHeader);
				uint8_t pathLen = (uint8_t)item.fileInfo->filePath.size();
				memcpy(&rawBuf[wpos], &pathLen, 1); wpos += 1;
				memcpy(&rawBuf[wpos], item.fileInfo->filePath.data(), item.fileInfo->filePath.size()); wpos += item.fileInfo->filePath.size();
				memcpy(&rawBuf[wpos], item.fileData.data(), item.fileData.size()); wpos += item.fileData.size();
			}
			uint64_t dwResTick = GetSteadyTimeMS64();
			DPRINT(SUCCESS_GREEN, "[%p][S->C][%llu] 发送响应: %d文件 %u字节 (耗时%llums)\r\n", this, dwResTick, (int)(fileIdx - batchStart), batchBytes, dwResTick - dwReqTick);
			// 不再直接postSend, 改为入队由Update()限速发送, 避免突发流量堵死客户端
			m_pendingSendQueue.push_back(std::move(rawBuf));
		}
	}
	break;
	case 0x1002:
	{
		m_TimeOut.Savetime();
	}
	break;
	case 0x1003:
	{
	}
	break;
	}
}

VOID CResourceClient::SendFileToClient(FileInfo* fileInfo, uint32_t startPos, uint32_t requestID, uint32_t wpfIndex)
{
	FileManager* pFM = FileManager::GetInstance();
	if (!pFM) return;
	std::vector<uint8_t> fileData;
	uint32_t actualLen = (startPos < fileInfo->size) ? (fileInfo->size - startPos) : 0;
	if (actualLen == 0) return;
	if (!pFM->ReadFileByHash(fileInfo->hash, startPos, actualLen, fileData))
		return;
	if (fileData.empty()) return;
	xPacketPool::ScopedPacket packet;
	uint32_t dataLen = sizeof(SDLProtocalHeader) + 1 + (uint32_t)fileInfo->filePath.size() + (uint32_t)fileData.size();
	packet->create(sizeof(STDlNetMsgHeader) + dataLen);
	STDlNetMsgHeader msgHeader{};
	msgHeader.wMsgID = htons(0x0BB9);
	msgHeader.dwDataLen = htonl(dataLen);
	packet->push(&msgHeader, sizeof(STDlNetMsgHeader));
	SDLProtocalHeader protoHeader{};
	protoHeader.wProtocal = 0x1001;
	protoHeader.wFileCount = 1;
	protoHeader.dwID = requestID;
	protoHeader.i64Hash = fileInfo->hash;
	protoHeader.iWpf = wpfIndex;
	protoHeader.dwAttribute = EFA_FILE;
	protoHeader.dwLen = fileInfo->size;          // 文件总长度
	protoHeader.dwStartPos = startPos;        // 本次发送启始位置, 支持断点续传
	protoHeader.dwLastDWORD = *(uint32_t*)&fileData[fileData.size() - 4];
	protoHeader.dwRev = 0;
	packet->push(&protoHeader, sizeof(SDLProtocalHeader));
	uint8_t pathLen = (uint8_t)fileInfo->filePath.size();
	packet->push(&pathLen, 1);
	packet->push((LPVOID)fileInfo->filePath.data(), (int)fileInfo->filePath.size());
	packet->push((LPVOID)fileData.data(), (int)fileData.size());
	postSend(packet.get());
}