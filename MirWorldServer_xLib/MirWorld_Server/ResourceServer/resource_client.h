#pragma once
#include <atomic>
#include "protocol.h"

constexpr auto HEARTBRATTIME = 30 * 1000;	// 心跳检测时间

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
};
