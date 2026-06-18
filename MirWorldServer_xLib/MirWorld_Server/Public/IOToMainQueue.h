#pragma once
#include <atomic>
#include <array>
#include <memory>
#include <functional>
#include <vector>
#include <cstring>

// ============================================================
// 主线程消息类型定义
// ============================================================
enum class MainThreadMsgType : uint8_t
{
	CLIENT_CONNECT,      // 新客户端连接
	CLIENT_DISCONNECT,   // 客户端断开
	SC_CONNECT_FAILED,   // SC连接失败
	DB_CONNECT_FAILED,   // DB连接失败
	SHUTDOWN,            // 关闭信号
};

struct MainThreadMessage
{
	MainThreadMsgType type;
	uint32_t clientId;       // 客户端ID（连接/断开时使用）
	uint32_t param1;         // 附加参数
	uint32_t param2;

	MainThreadMessage() : type(MainThreadMsgType::SHUTDOWN), clientId(0), param1(0), param2(0) {}

	static MainThreadMessage MakeConnect(uint32_t id)
	{
		MainThreadMessage msg;
		msg.type = MainThreadMsgType::CLIENT_CONNECT;
		msg.clientId = id;
		return msg;
	}

	static MainThreadMessage MakeDisconnect(uint32_t id)
	{
		MainThreadMessage msg;
		msg.type = MainThreadMsgType::CLIENT_DISCONNECT;
		msg.clientId = id;
		return msg;
	}

	static MainThreadMessage MakeShutdown()
	{
		MainThreadMessage msg;
		msg.type = MainThreadMsgType::SHUTDOWN;
		return msg;
	}
};

using IoToMainQueue = xMpscQueue<MainThreadMessage, 8192>;
using IoToMainMsgPool = xObjectPool<MainThreadMessage>;
