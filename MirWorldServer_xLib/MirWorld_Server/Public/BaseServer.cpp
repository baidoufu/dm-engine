#include "StdAfx.h"
#include <ws2tcpip.h>
#include "baseserver.h"
#include "tempclient.h"
#include "LogFile.h"

CBaseServer::CBaseServer(VOID) :m_xQTempClient(1024)
{
	m_szNickName.fill(0);
	m_szServerState.fill(0);
	m_Type = ST_UNKNOWN;
	m_Ident.dwId = 0;
	m_iDBServerCount = 0;
	GetLocalTime(&m_stStartTime);
	m_bStateInfoUpdated = FALSE;
	m_nLoopCount = 0;
	m_fLoopTime = 0.0f;
	m_dwLoopTimes = 0;
	m_dwLoopStartTime = 0;
	m_dwConnectedTimes = 0;
	m_dwDisconnectTimes = 0;
	m_dwPreConnections = 0;
	m_xIocpServer.setEventListener(this);
	srand(timeGetTime());
	this->m_szBanIpFile.fill(0);
	this->m_szTrustIpFile.fill(0);
}

CBaseServer::~CBaseServer(VOID)
{
}

BOOL CBaseServer::Start(CSettingFile& sf) //读取Config.ini
{
	const char* pServerName = m_strServerName.c_str();
	//获取服务器类型
	m_Ident.bType = (BYTE)GetServerTypeByName(sf.GetString(pServerName, "Type"));
	if (m_Ident.bType == ST_UNKNOWN)
	{
		m_pIoConsole->OutPut(ERROR_RED, "得到一个未知服务器类型, 请检查配置文件!\n");
		return FALSE;
	}
	//获取group和id
	m_Ident.bGroup = (BYTE)sf.GetInteger(pServerName, "Group");
	m_Ident.bId = (BYTE)sf.GetInteger(pServerName, "Id");
	//获取名字
	SetNickName(sf.GetString(pServerName, "Name", "NONAME"));
	//获取servercenter的地址
	std::array<char, 16> szIpAddr{};
	m_SCAddr.nPort = (UINT)sf.GetInteger("Public", "ScPort", 0);
	if (!GetAddrByName(sf.GetString("Public", "ScAddr", "127.0.0.1"), szIpAddr.data()))
		strncpy(szIpAddr.data(), "127.0.0.1", szIpAddr.size() - 1);
	m_SCAddr.SetAddr(szIpAddr.data());
	//取得服务器的地址和端口
	m_ServerAddr.nPort = (UINT)sf.GetInteger(pServerName, "Port", 0);
	if (!GetAddrByName(sf.GetString(pServerName, "Addr", "127.0.0.1"), szIpAddr.data()))
		strncpy(szIpAddr.data(), "127.0.0.1", szIpAddr.size() - 1);
	m_ServerAddr.SetAddr(szIpAddr.data());
	m_pIoConsole->OutPut(SUCCESS_GREEN, "正在启动服务器...\n");
	//加载IP限制列表
	const char* pszBanIpFile = sf.GetString(pServerName, "BanIpList", ".\\BanIP.txt");
	o_strncpy(m_szBanIpFile.data(), pszBanIpFile, 250);
	this->m_BanList.Load(pszBanIpFile);
	//加载IP信任列表
	const char* pszTrustIpFile = sf.GetString(pServerName, "TrustIpList", ".\\TrustIP.txt");
	o_strncpy(m_szTrustIpFile.data(), pszTrustIpFile, 250);
	this->m_TrustList.Load(pszTrustIpFile);
	//启动服务器
	BOOL bRet = FALSE;
	bRet = m_xIocpServer.start();
	if (bRet)
	{
		std::array<char, 16> cp{};
		strncpy(cp.data(), strcmp(szIpAddr.data(), "127.0.0.1") == 0 ? "127.0.0.1" : "0.0.0.0", cp.size() - 1);
		bRet = m_xIocpServer.postListen(cp.data(), m_ServerAddr.nPort, 64, 100000);
		if (bRet)
		{
			if (!InitServer(sf))
				bRet = FALSE;
			m_dwLoopStartTime = timeGetTime();
			StartIoThread(); // 先启动IO独立线程
			if (!xThread::Start(this))
				bRet = FALSE;
		}
	}
	else
		m_pIoConsole->OutPut(ERROR_RED, m_xIocpServer.getErrorMsg());
	if (bRet)
		m_pIoConsole->OutPut(SUCCESS_GREEN, "启动服务器成功...\n");
	else
		m_pIoConsole->OutPut(ERROR_RED, "(%d) %s\n", m_xIocpServer.getErrorCode(), m_xIocpServer.getErrorMsg());
	return bRet;
}

BOOL CBaseServer::GetAddrByName(const char* pszHostName, char* pszIpAddress)
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result = nullptr;
	if (getaddrinfo(pszHostName, nullptr, &hints, &result) != 0 || result == nullptr)
		return FALSE;

	ULONG lAddr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(result);
	sprintf(pszIpAddress, "%u.%u.%u.%u", lAddr & 0xff, (lAddr & 0xff00) >> 8, (lAddr & 0xff0000) >> 16, (lAddr & 0xff000000) >> 24);
	return TRUE;
}

VOID CBaseServer::GetServerState(SERVERSTATE& state)
{
	state.bUpdated = m_bStateInfoUpdated;
	m_bStateInfoUpdated = FALSE;
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	state.dwRunningSeconds = GetT1toT2Second(m_stStartTime, stNow);
	state.dwRecvBytes = m_xIocpServer.getRecvKBytes();
	state.dwSendBytes = m_xIocpServer.getSendKBytes();
	state.fLoopTime = m_fLoopTime;
	state.numConnected = (int)m_dwConnectedTimes;
	state.numDisconnected = (int)m_dwDisconnectTimes;
	state.numConnection = m_dwConnectedTimes - m_dwDisconnectTimes;
	state.dwLoopTimes = m_dwLoopTimes;
	sprintf(m_szServerState.data(), "%d天%d小时%d分%d秒",
		state.dwRunningSeconds / 86400, (state.dwRunningSeconds / 3600) % 24, (state.dwRunningSeconds / 60) % 60,
		state.dwRunningSeconds % 60);
	state.pServerDescript = m_szServerState.data();
}

BOOL CBaseServer::Stop()
{
	StopIoThread();       // 先停止IO线程
	xThread::Terminate(); // 通知主线程终止
	xThread::WaitFor();   // 等待主线程结束（线程内不再访问网络资源）
	//停止网络
	m_SCClientObj.close(); // 先关闭客户端连接
	m_xIocpServer.stop();  // 再停止IOCP服务器（释放网络资源）
	CleanServer();         // 清理服务器数据（此时网络已完全关闭）
	return TRUE;
}

VOID CBaseServer::Execute(LPVOID lpParam)
{
	while (!IsTerminated())
	{
		CFrameTime::UpdateFrameTime(); // 每帧缓存一次时间
		// ===== 消费IO线程投递的事件（连接/断开） =====
		ProcessIoMessages();
		// ===== 游戏逻辑主循环 =====
		Update(); //主循环（子类CServer重写：包处理 + 游戏世界更新 + Flush + SC/DB）
		// ===== 统计信息（保留在主线程） =====
		if ((m_nLoopCount & 1) == 0)
			UpdateTotalInfo();
		++m_nLoopCount;
		// 避免空转
		if (!m_xIocpServer.hasPendingEvents() && m_IoToMainQueue.getcount() == 0)
			Sleep(1);
	}
}

BOOL CBaseServer::ConnectSCServer()
{
	if (m_SCClientObj.IsConnected())
		m_SCClientObj.close();
	m_SCClientObj.Clean();
	m_SCClientObj.setServer(&m_xIocpServer);
	m_SCClientObj.SetMsgProcessor(this);
	if (m_xIocpServer.postConnection(m_SCAddr.addr.data(), m_SCAddr.nPort, m_SCClientObj))//OpenConnection(m_SCClientObj,m_SCAddr.addr, m_SCAddr.nPort );
	{
		m_SCClientObj.postRecv();
		return TRUE;
	}
	return FALSE;
}

BOOL CBaseServer::ConnectDBServer()
{
	for (int i = 0; i < m_iDBServerCount && i < 2; i++)
	{
		if (m_DBClientObjs[i].IsConnected())
			continue;
		m_DBClientObjs[i].Clean();
		m_DBClientObjs[i].setServer(&m_xIocpServer);
		m_DBClientObjs[i].SetMsgProcessor(this);
		if (!m_xIocpServer.postConnection(m_DBAddrs[i].addr.data(), m_DBAddrs[i].nPort, m_DBClientObjs[i]))
			return FALSE;
		m_DBClientObjs[i].postRecv();
	}
	return TRUE;
}

VOID CBaseServer::UpdateSCServer()
{
	PLMIRMSG pmsg = nullptr;
	if (m_dwPreConnections != (m_dwConnectedTimes - m_dwDisconnectTimes))
	{
		m_dwPreConnections = (m_dwConnectedTimes - m_dwDisconnectTimes);
		m_SCClientObj.UpdateInfo(m_dwPreConnections);
	}
	if (m_SCClientObj.IsConnected())
	{
		m_SCClientObj.Update();
	}
	else
	{
		if (!m_scReconnectTimer.IsTimeOut(1000)) return;
		m_scReconnectTimer.Savetime();
		if (!ConnectSCServer()) //重新连接
		{
			m_pIoConsole->OutPut(ERROR_RED, "%s|连接服务器中心(%s:%u)失败...\n",
				GetServerTypeNameByType((servertype)m_Ident.bType), m_SCAddr.addr.data(), m_SCAddr.nPort);
		}
		else
		{
			m_SCClientObj.setId(ID_SERVERCENTERCONNECTION);
			m_SCClientObj.RegisterServer(m_ServerAddr, m_Ident, m_szNickName.data());
			OnSCServerReady();
		}
	}
}

VOID CBaseServer::UpdateDBServer()
{
	for (int i = 0; i < m_iDBServerCount && i < 2; i++)
	{
		if (m_DBClientObjs[i].IsConnected())
		{
			m_DBClientObjs[i].Update();
		}
		else
		{
			if (!m_dbReconnectTimers[i].IsTimeOut(1000)) continue;
			m_dbReconnectTimers[i].Savetime();
			m_DBClientObjs[i].Clean();
			m_DBClientObjs[i].setServer(&m_xIocpServer);
			m_DBClientObjs[i].SetMsgProcessor(this);
			if (m_xIocpServer.postConnection(m_DBAddrs[i].addr.data(), m_DBAddrs[i].nPort, m_DBClientObjs[i]))
			{
				m_DBClientObjs[i].postRecv();
				m_pIoConsole->OutPut(SUCCESS_GREEN, "%s|连接数据库服务器%d(%s:%u)成功...\n", 
					GetServerTypeNameByType((servertype)m_Ident.bType), i, m_DBAddrs[i].addr.data(), m_DBAddrs[i].nPort);
			}
			else
			{
				m_pIoConsole->OutPut(ERROR_RED, "%s|连接数据库服务器%d(%s:%u)失败...\n",
					GetServerTypeNameByType((servertype)m_Ident.bType), i, m_DBAddrs[i].addr.data(), m_DBAddrs[i].nPort);
			}
		}
	}
}

VOID CBaseServer::ProcConnectionEvent()
{
	const int count = static_cast<int>(m_xQTempClient.getcount());
	if (count == 0) return;
	for (int i = 0; i < count; ++i)
	{
		CTempClient* pTClient = m_xQTempClient.pop();
		if (pTClient->IsTimeOut(3000))
		{
			pTClient->close();
			m_xTempClients.deleteObject(pTClient);
			continue;
		}
		m_xQTempClient.push(pTClient);
	}
}

VOID CBaseServer::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_MSGACROSSSERVER:
	{
		OnMASMsg(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	}
	break;
	case SCM_REGISTERSERVER:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			REGISTER_SERVER_RESULT* presult = (REGISTER_SERVER_RESULT*)pMsg->data;
			m_pIoConsole->OutPut(SUCCESS_GREEN, "注册服务器成功,index = %d\n",
				presult->Id.bIndex);
			m_Ident = presult->Id;
			m_iDBServerCount = presult->nDbCount;
			if (m_iDBServerCount > 2)m_iDBServerCount = 2;
			// 正确使用DbAddr[0]和DbAddr[1]分别赋值
			for (int i = 0; i < m_iDBServerCount && i < 2; i++)
			{
				m_DBAddrs[i] = presult->DbAddr[i];
				m_pIoConsole->OutPut(STRING_GREEN, "数据库服务器%d %s:%d\n",
					i, m_DBAddrs[i].addr.data(), m_DBAddrs[i].nPort);
			}
		}
		else
		{
			m_pIoConsole->OutPut(ERROR_RED, "注册服务器失败,错误 = %d\n",
				pMsg->wParam[0]);
		}
	}
	break;
	}
}

VOID CBaseServer::Update()
{
}

VOID CBaseServer::UpdateTotalInfo()
{
	++m_dwLoopTimes;
	if (m_dwLoopTimes >= 5000)
	{
		DWORD dwCurTime = CFrameTime::GetFrameTime();
		m_dwLoopTimes = 0;
		m_fLoopTime = (dwCurTime - m_dwLoopStartTime) / 1000.0f;
		m_dwLoopStartTime = dwCurTime;
		m_bStateInfoUpdated = TRUE;
	}
}

VOID CBaseServer::OnUnknownMsg(PMIRMSG pMsg, int datasize, int type)
{
	DWORD dwColor = 0;
	switch (type)
	{
	case 0:
	{
		dwColor = ERROR_RED;
		LG1("未知消息 Ident:%#x\n", pMsg->wCmd);
		m_pIoConsole->OutPut(dwColor, "未知消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	case 1:
	{
		dwColor = ORANGE;
		LG1("接收消息 Ident:%#x\n", pMsg->wCmd);
		m_pIoConsole->OutPut(dwColor, "接收消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	case 2:
	{
		dwColor = WARN_YELLOW;
		LG1("发送消息 Ident:%#x\n", pMsg->wCmd);
		m_pIoConsole->OutPut(dwColor, "发送消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	}
	LG1("F:%d I:%d P1:%d P2:%d P3:%d D:%s S:%d\n", pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	LG1("[");
	{
		const BYTE* pBytes = reinterpret_cast<const BYTE*>(pMsg->data);
		for (int i = 0; i < datasize; ++i)
		{
			LG1("%d,", pBytes[i] & 0xff);
		}
	}
	LG1("]\n\n");
	m_pIoConsole->OutPut(dwColor, "F:%d I:%d P1:%d P2:%d P3:%d D:%s S:%d\n\n", pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
}

VOID CBaseServer::StartIoThread()
{
	if (m_bIoRunning) return;
	m_bIoRunning = TRUE;
	m_IoThread = std::thread([this]() {
		// IO独立线程：专门处理IOCP事件
		// - 偶数帧：xIocpServer::update() 处理连接/断开事件
		// - 奇数帧：ProcConnectionEvent() 清理临时客户端
		DWORD ioFrameCounter = 0;
		while (m_bIoRunning)
		{
			if ((ioFrameCounter & 1) == 0)
			{
				// 偶数帧：处理IOCP连接/断开事件
				// xIocpServer::update() 内部调用 OnEvent()
				// OnEvent 只做网络层初始化，业务回调通过队列投递
				m_xIocpServer.update();
			}
			else
			{
				// 奇数帧：清理临时客户端
				if (m_xIocpServer.hasPendingEvents())
					SwitchToThread();
				else
					Sleep(1);
				ProcConnectionEvent();
				// 注意：SC/DB客户端的Update()由主线程在UpdateSCServer()/UpdateDBServer()中调用
				// 不在IO线程中调用，避免与主线程的竞态条件
			}
			++ioFrameCounter;
		}
	});
}

VOID CBaseServer::StopIoThread()
{
	if (!m_bIoRunning) return;
	m_bIoRunning = false;
	// 投递关闭信号唤醒IO线程
	{
		MainThreadMessage* pMsg = m_IoToMainMsgPool.newObject();
		*pMsg = MainThreadMessage::MakeShutdown();
		m_IoToMainQueue.push(pMsg);
	}
	if (m_IoThread.joinable())
		m_IoThread.join();
}

VOID CBaseServer::ProcessIoMessages()
{
	// 主线程调用：消费IO线程投递的连接/断开事件
	MainThreadMessage* pMsg;
	while ((pMsg = m_IoToMainQueue.pop()) != nullptr)
	{
		switch (pMsg->type)
		{
		case MainThreadMsgType::CLIENT_CONNECT:
		{
			// 在主线程上下文执行OnConnection回调
			CClientObject* pObj = GetClientObject(pMsg->clientId);
			if (pObj && pObj->IsConnected())
			{
				pObj->OnConnection();
				m_pIoConsole->OutPut(STRING_GREEN, "%s|来自(%s:%u)的连接成功...\n", 
					GetServerTypeNameByType((servertype)m_Ident.bType), pObj->getAddress(), pObj->getPort());
			}
			break;
		}
		case MainThreadMsgType::CLIENT_DISCONNECT:
		{
			// 在主线程上下文执行断开处理
			CClientObject* pObj = GetClientObject(pMsg->clientId);
			if (pObj)
			{
				pObj->OnDisconnect();
				CClientObject* pObject = GetClientObject(pMsg->clientId);
				if (pObject == nullptr)
				{
					if (pMsg->clientId == ID_SERVERCENTERCONNECTION)
						m_pIoConsole->OutPut(ERROR_RED, "%s与服务器中心失去连接!\n", GetServerTypeNameByType((servertype)m_Ident.bType));
				}
				else if (pObject == pObj)
				{
					m_pIoConsole->OutPut(ERROR_RED, "%s|来自(%s:%u)的连接断开...\n", 
						GetServerTypeNameByType((servertype)m_Ident.bType), pObject->getAddress(), pObject->getPort());
					DeleteClientObject(pObject);
					++m_dwDisconnectTimes;
				}
				else
				{
					// 对象ID在OnDisconnect期间被复用：原对象已被替换为新对象
					// 必须删除当前占用该ID的新对象，否则资源泄漏
					m_pIoConsole->OutPut(ERROR_RED, "%s|连接断开时检测到对象复用(clientId=%u)，强制清理\n",
						GetServerTypeNameByType((servertype)m_Ident.bType), pMsg->clientId);
					DeleteClientObject(pObject);
					++m_dwDisconnectTimes;
				}
			}
			break;
		}
		case MainThreadMsgType::SHUTDOWN:
			break;
		default:
			break;
		}
		m_IoToMainMsgPool.deleteObject(pMsg);
	}
}

VOID CBaseServer::OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam)
{
	// 注意：此方法在IOCP工作线程中回调
	// 只做网络层初始化，业务回调通过消息队列投递到主线程
	switch (iEvent)
	{
	case xIocpServer::ISE_ONCONNECTION:
	{
		xTempClient* pTempClient = (xTempClient*)lpParam;

		if (!(m_TrustList.IsEmpty() || m_TrustList.AddressIn(pTempClient->getAddress())))
		{
			m_pIoConsole->OutPut(ERROR_RED, "非信任IP %s 连接被阻止...\n", pTempClient->getAddress());
		}
		else if (!m_BanList.IsEmpty() && m_BanList.AddressIn(pTempClient->getAddress()))
		{
			m_pIoConsole->OutPut(ERROR_RED, "被禁止IP %s 连接被阻止...\n", pTempClient->getAddress());
		}
		else
		{
			// 在IO线程中创建客户端对象（仅网络层初始化）
			CClientObject* pObj = NewClientObject();
			if (pObj != nullptr)
			{
				pObj->setServer(&m_xIocpServer);
				if (!pObj->steelSocket(*(xSocket*)lpParam))
				{
					DeleteClientObject(pObj);
				}
				else
				{
					++m_dwConnectedTimes;
					pObj->postRecv();
					// 投递连接事件到主线程（OnConnection在主线程执行）
					MainThreadMessage* pMsg = m_IoToMainMsgPool.newObject();
					*pMsg = MainThreadMessage::MakeConnect(pObj->getId());
					m_IoToMainQueue.push(pMsg);
				}
			}
		}
	}
	break;
	case xIocpServer::ISE_ONDISCONNECTION:
	{
		CClientObject* pObj = (CClientObject*)lpParam;
		UINT id = pObj->getId();
		// 投递断开事件到主线程（OnDisconnect/DeleteObject在主线程执行）
		MainThreadMessage* pMsg = m_IoToMainMsgPool.newObject();
		*pMsg = MainThreadMessage::MakeDisconnect(id);
		m_IoToMainQueue.push(pMsg);
	}
	break;
	}
}

VOID CBaseServer::OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size)
{
	//++++
}

VOID CBaseServer::OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize)
{
	if (pObject == &this->m_SCClientObj)
	{
		if (pMsg->dwFlag == 0)
			OnSCMsg(pMsg, datasize);
		else
		{
			CClientObject* pClientObj = (CClientObject*)GetClientObject(pMsg->dwFlag);
			if (pClientObj != nullptr)
				pClientObj->OnSCMsg_T(pMsg, datasize);
		}
	}
	else if (m_iDBServerCount > 0 &&
		(pObject == &this->m_DBClientObjs[0] || (m_iDBServerCount > 1 && pObject == &this->m_DBClientObjs[1])))
	{
		if (pMsg->dwFlag == 0)
			OnDBMsg(pMsg, datasize);
		else
		{
			CClientObject* pClientObj = GetClientObject(pMsg->dwFlag);
			if (pClientObj != nullptr)
				pClientObj->OnDBMsg(pMsg, datasize);
		}
	}
}

VOID CBaseServer::OnInput(const char* pszString)
{
	std::array<char, 256> szLine{};
	o_strncpy(szLine.data(), pszString, szLine.size());
	xStringsExtracter<16> cmd(szLine.data(), " \t,", " \t");
	if (static_cast<int>(cmd.getCount()) > 0)
	{
		if (_stricmp(cmd[0], "reloadbanlist") == 0)
		{
			m_BanList.Load(m_szBanIpFile.data());
			m_pIoConsole->OutPut(SUCCESS_GREEN, "Ban列表 %s 已加载!\n", m_szBanIpFile.data());
		}
		else if (_stricmp(cmd[0], "reloadtrustlist") == 0)
		{
			m_TrustList.Load(m_szTrustIpFile.data());
			m_pIoConsole->OutPut(SUCCESS_GREEN, "Trust列表 %s 已加载!\n", m_szTrustIpFile.data());
		}
		else if (_stricmp(cmd[0], "exit") == 0 || _stricmp(cmd[0], "quit") == 0)
		{
			m_pIoConsole->OutPut(SUCCESS_GREEN, "收到关闭命令, 开始优雅关闭服务器...\n");
			Stop();
		}
	}
}
