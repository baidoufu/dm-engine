#pragma once

#include "xsupport.h"
class xIocpUnit;

// IOCP_UNIT 验证令牌，用于防止 use-after-free
constexpr DWORD IOCP_UNIT_VALID_TOKEN = 0x10CEA7CA;

typedef struct tagOverlappedEx
{
	tagOverlappedEx() : ol{}, dwFlag(0), dwNumberOfBytes(0), dwError(0) {}
	OVERLAPPED	ol;
	DWORD dwFlag;
	DWORD dwNumberOfBytes;
	DWORD dwError;  // I/O完成错误码：0=成功，非0=GetLastError()错误码
}OVERLAPPED_EX;

typedef struct tag_IocpUnit
{
	tag_IocpUnit() : ol{}, pUnit(nullptr), dwValidToken(0) {}
	OVERLAPPED_EX ol;
	xIocpUnit* pUnit;
	DWORD dwValidToken;  // 验证令牌：IOCP_UNIT_VALID_TOKEN 表示有效，0 表示已失效
}IOCP_UNIT;

enum iocpunit_type
{
	IO_NOTSET,
	IO_READ,
	IO_SEND,
	IO_ACCEPT,
};

class xIocpUnit :public xEventSender
{
public:
	enum eventid
	{
		IUE_ID = 10000,
		IUE_READ,
		IUE_SEND,
		IUE_ACCEPT,
		IUE_ERROR,
	};

	xIocpUnit(VOID);
	virtual ~xIocpUnit(VOID);

	VOID OnComplete(DWORD dwNumberOfBytes, ULONG_PTR dwCompletionKey, DWORD dwError);

	iocpunit_type getType() { return m_type; }
	VOID setType(iocpunit_type type) { m_type = type; }

	LPVOID getData() { return m_lpData; }
	VOID setData(LPVOID lpData) { m_lpData = lpData; }

	// 获取/设置已发送偏移量（用于部分发送时重传剩余数据）
	DWORD getSendOffset() const { return m_dwSendOffset; }
	VOID setSendOffset(DWORD offset) { m_dwSendOffset = offset; }

	OVERLAPPED_EX* getOverlappedEx() { return &m_UnitStruct.ol; }
	OVERLAPPED* getOverlapped() { return &m_UnitStruct.ol.ol; }

	// 获取/设置验证令牌（供 xIocpServer 管理生命周期使用）
	DWORD getValidToken() const { return m_UnitStruct.dwValidToken; }
	VOID setValidToken(DWORD token) { m_UnitStruct.dwValidToken = token; }
private:
	iocpunit_type m_type;
	IOCP_UNIT m_UnitStruct;
	LPVOID m_lpData;
	DWORD m_dwSendOffset;  // 已发送偏移量，用于部分发送时追踪剩余数据
};