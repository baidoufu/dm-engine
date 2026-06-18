#pragma once

// 处理服务的类
class ServerProcess
{
public:
    ServerProcess(const std::string& name, const std::string& path);
    ~ServerProcess();

    bool Start();
    bool Stop();
    bool IsRunning() const;
    const std::string& GetName() const { return m_name; }
    const std::string& GetPath() const { return m_path; }

    bool SendCommand(const std::string& command);

    VOID SetOutputHandle(HANDLE hStdOut) { m_hOutput = hStdOut; }
    VOID SetAutoRestart(bool enable) { m_bAutoRestart = enable; }
    bool IsAutoRestartEnabled() const { return m_bAutoRestart; }
    VOID SetRestartCallback(std::function<VOID(ServerProcess*)> callback) { m_restartCallback = callback; }
    int GetRestartCount() const { return m_restartCount; }
    VOID ResetRestartCount() { m_restartCount = 0; }

private:
    std::string m_name;
    std::string m_path;
    HANDLE m_hProcess;
    HANDLE m_hThread;
    HANDLE m_hOutput;
    HANDLE m_hInput;
    DWORD m_dwProcessId;
    bool m_bRunning;
    bool m_bAutoRestart;
    bool m_bManualStop;
    int m_restartCount;
    std::chrono::steady_clock::time_point m_lastRestartTime;
    std::function<VOID(ServerProcess*)> m_restartCallback;

    static unsigned __stdcall ReadOutputThread(VOID* param);
    static unsigned __stdcall MonitorThread(VOID* param);
    VOID ReadOutput();
    VOID MonitorProcess();
};