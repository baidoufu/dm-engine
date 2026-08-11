#pragma once
#include <atomic>
#include <deque>
#include <vector>
#include "protocol.h"

constexpr auto HEARTBRATTIME = 30 * 1000;	// 心跳检测时间
constexpr uint32_t MAX_SEND_BYTES_PER_TICK = 512 * 1024;	// 每tick最多发送字节数,防止突发流量堵死客户端

class CResourceClient : public CClientObject
{
public:
	CResourceClient(VOID);
	virtual ~CResourceClient(VOID);
	VOID Clean();
	VOID Update();
	VOID OnConnection();
	VOID OnDisconnect();
	VOID OnDataPacket(xPacket* pPacket);
private:
	//下载资源请求
	VOID HandleDownloadRequest(const char* buffer, int len);
	//发送文件到客户端
	VOID SendFileToClient(FileInfo* fileInfo, uint32_t startPos, uint32_t requestID, uint32_t wpfIndex);
private:
	CServerTimer m_TimeOut; // 心跳时间
	std::deque<std::vector<char>> m_pendingSendQueue; // 待发送数据队列(限速发送)
};
