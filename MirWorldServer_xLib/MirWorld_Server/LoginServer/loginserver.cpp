#include "stdafx.h"
#include "loginserver.h"
#include "server.h"

CServerForm g_Form;
CServer* g_pServer = nullptr;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".936");
	CLogFile::GetInstance()->Init("..\\日志\\登录网关");
	// 组合命令行参数
	std::array<char, 256> cmdLine = {0};
	for (int i = 1; i < argc; i++)
	{
		if (i > 1) strcat(cmdLine.data(), " ");
		strcat(cmdLine.data(), argv[i]);
	}
	g_Form.Create(SERVER_NAME, cmdLine.data());
	g_pServer = CServer::GetInstance();
	g_pServer->SetServerName(SERVER_NAME);
	g_pServer->SetIoConsole(&g_Form);
	g_Form.SetInputListener(g_pServer);
	g_Form.SetServer(g_pServer);
	g_Form.SetStatus(FALSE);
	return g_Form.EnterMessageLoop();
}