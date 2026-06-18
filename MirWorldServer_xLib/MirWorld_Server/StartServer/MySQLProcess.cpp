#include "stdafx.h"
#include <tlhelp32.h>
#include "MySQLProcess.h"

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

    // 等待 MySQL 启动
    Sleep(2000);

    if (IsMySQLRunning())
    {
        printf("[MySQL] 启动成功\n");
        return true;
    }
    else
    {
        printf("[MySQL] 启动后检测失败\n");
        return false;
    }
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
        Sleep(1000);
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

    Sleep(1000);

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
