#pragma once

// 处理MySQL服务的类
class MySQLProcess
{
public:
	MySQLProcess();
	~MySQLProcess();
	bool Start();
	bool Stop();
	bool IsRunning() const;
	const std::string& GetName() const { return "MySQL"; }
private:
	bool IsMySQLRunning() const;
	// 等待 MySQL 端口就绪(可接受连接)
	// timeoutMs: 总超时时间,内部循环探测端口并检测进程是否已退出
	bool WaitForReady(DWORD timeoutMs = 30000) const;
};
