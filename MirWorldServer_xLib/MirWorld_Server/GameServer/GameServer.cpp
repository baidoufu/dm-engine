#include "stdafx.h"
#include "GameServer.h"
#include "server.h"
#include "fmttextfile.h"
#include "monsterex.h"
#include "scriptobject.h"
#include "VMProtectHelper.h"
#include "HumanPlayerMgr.h"

CServerForm g_Form;
CServer* g_pServer = nullptr;

// 崩溃前数据保存回调
static VOID OnPreCrashSave()
{
	if (g_pServer)
		g_pServer->OnTerminated(TRUE);
}

static int CheckLicense()
{
#ifndef USE_VMPROTECT
	return 1;
#endif
	if (!VMP_IS_PROTECTED()) return -1;
	if (VMP_IS_DEBUGGER_PRESENT(TRUE)) return -2;
	if (!VMP_IS_VALID_IMAGECRC()) return -2;
	FILE* fp = nullptr;
	if (fopen_s(&fp, "license.key", "r") == 0 && fp)
	{
		std::array<CHAR, 1024> szSerial = { 0 };
		fgets(szSerial.data(), static_cast<int>(szSerial.size()), fp);
		fclose(fp);
		size_t len = strlen(szSerial.data()); // 去掉换行符
		while (len > 0 && (szSerial[len - 1] == '\n' || szSerial[len - 1] == '\r'))
			szSerial[--len] = 0;
		if (VMP_SET_SERIAL_NUMBER(szSerial.data()) != 0) return -1; //序列号无效
	}
	else
		return 1; //未找到序列号文件
	return 0;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".936");
	TRY_INIT
	CLogFile::GetInstance()->Init("..\\日志\\龙路引擎");
	// 组合命令行参数
	std::array<char, 256> cmdLine = { 0 };
	for (int i = 1; i < argc; i++)
	{
		if (i > 1) strcat(cmdLine.data(), " ");
		strcat(cmdLine.data(), argv[i]);
	}
	g_Form.Create(SERVER_NAME, cmdLine.data());

	VMP_PROTECT_BEGIN("许可证");
	std::array<CHAR, 256> szHWSerial = { 0 };
	std::array<CHAR, 512> szMsg = { 0 };
	const char* pszDecryptString = nullptr;
	DWORD dwColor = 0xFF;
	VMP_GET_CURRENT_HWID(szHWSerial.data(), 256);
	int nResult = CheckLicense();
	switch (nResult)
	{
	case 1:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("测试版引擎！\n用户名：测试\n人数：5人\n硬件ID：%s\n");
		snprintf(szMsg.data(), szMsg.size(), pszDecryptString, szHWSerial.data());
		dwColor = 0xFF00;
		CHumanPlayerMgr::GetInstance()->SetTestMode();
	}
	break;
	case 0:
	{
		VMProtectSerialNumberData sd = { 0 };
		VMP_GET_SERIAL_NUMBER_DATA(&sd, sizeof(sd));
		SYSTEMTIME st; // 过期时间校验
		GetLocalTime(&st);
		int nExpire = sd.dtExpire.wYear * 10000 + sd.dtExpire.bMonth * 100 + sd.dtExpire.bDay;
		int nNow = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
		if (nExpire < nNow)
		{
			pszDecryptString = VMP_DECRYPT_STRINGA("错误：许可证已过期！\n硬件ID：%s\n");
			snprintf(szMsg.data(), szMsg.size(), pszDecryptString, szHWSerial.data());
			nResult = -1;
		}
		else
		{
			pszDecryptString = VMP_DECRYPT_STRINGA("引擎已注册！\n用户名：%ls\n人数：无限制\n有效期：%u年-%u月-%u日\n硬件ID：%s\n");
			snprintf(szMsg.data(), szMsg.size(), pszDecryptString, sd.wUserName, sd.dtExpire.wYear, sd.dtExpire.bMonth, sd.dtExpire.bDay, szHWSerial.data());
			dwColor = 0xFF00;
		}
	}
	break;
	case -1:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("错误：许可证验证失败！\n硬件ID：%s\n");
		snprintf(szMsg.data(), szMsg.size(), pszDecryptString, szHWSerial.data());
	}
	break;
	case -2:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("错误：程序被恶意修改！\n硬件ID：%s\n");
		snprintf(szMsg.data(), szMsg.size(), pszDecryptString, szHWSerial.data());
	}
	break;
	}
	g_Form.OutPutStatic(dwColor, szMsg.data());
	VMP_FREE_STRING(pszDecryptString);
	if (nResult < 0) return -1;
	VMP_PROTECT_END();

	g_Form.SetArenaReserve(256 * 1024);
	g_pServer = CServer::GetInstance();
	// 注册崩溃前数据保存回调
	CrashHandler::SetPreCrashSaveCallback(OnPreCrashSave);
	g_pServer->SetServerName(SERVER_NAME);
	g_pServer->SetIoConsole(&g_Form);
	g_Form.SetInputListener(g_pServer);
	g_Form.SetServer(g_pServer);
	g_Form.SetStatus(TRUE);
	return g_Form.EnterMessageLoop();
}