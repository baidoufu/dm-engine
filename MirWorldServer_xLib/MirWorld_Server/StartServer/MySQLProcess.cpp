#include "stdafx.h"
#include <tlhelp32.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "MySQLProcess.h"

// MySQL 监听端口,与 my.ini 及 Stop() 中 mysqladmin 使用的端口保持一致
static const USHORT MYSQL_LISTEN_PORT = 8306;

MySQLProcess::MySQLProcess()
{
}

MySQLProcess::~MySQLProcess()
{
	if (IsRunning()) Stop();
}

bool MySQLProcess::IsMySQLRunning() const
{
	// 检查 mysqld 进程是否存在
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 pe32{};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	bool found = false;
	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			if (_stricmp(pe32.szExeFile, "mysqld.exe") == 0)
			{
				found = true;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
	return found;
}

bool MySQLProcess::Start()
{
	if (IsMySQLRunning())
	{
		printf("[MySQL] 已在运行中\n");
		return false;
	}

	printf("[MySQL] 正在启动...\n");

	// 启动 MySQL
	char cmdLine[MAX_PATH];
	sprintf_s(cmdLine, sizeof(cmdLine), "\".\\mysqld.exe\" --defaults-file=\".\\my.ini\"");

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // 隐藏窗口

	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessA(nullptr, cmdLine, nullptr, nullptr, FALSE, 
		CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
	{
		DWORD error = GetLastError();
		printf("[MySQL] 启动失败: %d\n", error);
		return false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// 等待 MySQL 端口就绪(可接受连接)
	// 仅进程存在不代表端口已监听,用 TCP 探测确保真正可用
	if (!WaitForReady())
	{
		printf("[MySQL] 启动失败: 端口 %u 在超时30秒内未就绪\n", MYSQL_LISTEN_PORT);
		return false;
	}

	printf("[MySQL] 启动成功\n");
	return true;
}

bool MySQLProcess::WaitForReady(DWORD timeoutMs) const
{
	// 初始化 Winsock(独立管理进程,启动时调用一次)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("[MySQL] WSAStartup 失败: %d\n", WSAGetLastError());
		return false;
	}

	// 使用 steady_clock 计时
	using clock = std::chrono::steady_clock;
	auto startTime = clock::now();
	auto timeout = std::chrono::milliseconds(timeoutMs);
	bool ready = false;

	while (true)
	{
		// 总超时检查
		if (clock::now() - startTime >= timeout)
		{
			printf("[MySQL] 等待端口就绪超时(%lu ms)\n", timeoutMs);
			break;
		}

		// 进程已退出则无需继续等待(避免端口永远不就绪时空等)
		if (!IsMySQLRunning())
		{
			printf("[MySQL] 等待就绪期间进程已退出\n");
			break;
		}

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}

		// 非阻塞连接,配合 select 实现可控超时
		u_long nonBlock = 1;
		ioctlsocket(sock, FIONBIO, &nonBlock);

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
		addr.sin_port = htons(MYSQL_LISTEN_PORT);

		connect(sock, (sockaddr*)&addr, sizeof(addr)); // 非阻塞,立即返回

		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(sock, &writeSet);

		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int ret = select(0, nullptr, &writeSet, nullptr, &tv);
		if (ret > 0)
		{
			// 连接可写,需进一步确认无错误(连接成功 vs 拒绝)
			int sockErr = 0;
			int errLen = sizeof(sockErr);
			getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&sockErr, &errLen);
			if (sockErr == 0)
			{
				ready = true;
			}
		}
		closesocket(sock);
		if (ready) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	WSACleanup();
	return ready;
}

bool MySQLProcess::Stop()
{
	if (!IsMySQLRunning())
	{
		printf("[MySQL] 未运行\n");
		return false;
	}

	printf("[MySQL] 正在停止...\n");

	// 先尝试优雅关闭：通过mysqladmin发送shutdown命令,账号密码root,root
	char cmdLine[256];
	sprintf_s(cmdLine, sizeof(cmdLine), "\".\\mysqladmin.exe\" -u root -p123456 -h 127.0.0.1 -P 8306 shutdown");

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessA(nullptr, cmdLine, nullptr, nullptr, FALSE, 
		CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, 10000);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	// 等待mysqld进程自行退出（最多15秒）
	for (int i = 0; i < 15; i++)
	{
		if (!IsMySQLRunning())
		{
			printf("[MySQL] 已优雅停止\n");
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	// 优雅关闭超时，使用taskkill作为最后手段
	printf("[MySQL] 优雅关闭超时，强制终止...\n");
	sprintf_s(cmdLine, sizeof(cmdLine), "taskkill /im mysqld.exe /f");

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessA(nullptr, cmdLine, nullptr, nullptr, FALSE, 
		CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
	{
		DWORD error = GetLastError();
		printf("[MySQL] 停止失败: %d\n", error);
		return false;
	}

	WaitForSingleObject(pi.hProcess, 5000);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	if (!IsMySQLRunning())
	{
		printf("[MySQL] 已停止\n");
		return true;
	}
	else
	{
		printf("[MySQL] 停止后检测失败, 可能仍在运行\n");
		return false;
	}
}

bool MySQLProcess::IsRunning() const
{
	return IsMySQLRunning();
}
