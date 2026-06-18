#include "stdafx.h"
#include "resource_server.h"

CServerForm g_Form;
CResourceServer* g_pServer = nullptr;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".936");
	CLogFile::GetInstance()->Init("..\\日志\\资源服务器");
	std::array<char, 256> cmdLine = {0};
	for (int i = 1; i < argc; i++)
	{
		if (i > 1) strcat(cmdLine.data(), " ");
		strcat(cmdLine.data(), argv[i]);
	}
	g_Form.Create(SERVER_NAME, cmdLine.data());
	g_Form.SetArenaReserve(128 * 1024);
	//检测 -make 参数
	if (_stricmp(cmdLine.data(), "-make") == 0)
	{
		CSettingFile sf;
		if (!sf.Open("Config.ini"))
		{
			fprintf(stderr, "错误：找不到配置文件Config.ini!\n");
			return 1;
		}
		g_pServer = CResourceServer::GetInstance();
		return g_pServer->MakeIndex(sf);
	}
	g_pServer = CResourceServer::GetInstance();
	g_pServer->SetServerName(SERVER_NAME);
	g_pServer->SetIoConsole(&g_Form);
	g_Form.SetInputListener(g_pServer);
	g_Form.SetServer(g_pServer);
	g_Form.SetStatus(FALSE);
	return g_Form.EnterMessageLoop();
}
