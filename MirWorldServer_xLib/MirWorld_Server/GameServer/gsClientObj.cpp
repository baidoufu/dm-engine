#include "StdAfx.h"
#include <string.h>
#include "gsclientobj.h"
#include "gsClientMsgTable.h"
#include "server.h"
#include "humanplayermgr.h"
#include "gameworld.h"
#include "itemmanager.h"
#include "gmmanager.h"
#include "topmanager.h"
#include "systemscript.h"
#include "GroupObject.h"
#include "GuildEx.h"
#include "LogicMap.h"

CClientObj::CClientObj(VOID)
{
	Clean();
}

CClientObj::~CClientObj(VOID)
{
}

VOID CClientObj::Clean()
{
	CClientObject::Clean();
	m_State = GSUM_NOTVERIFIED;
	memset(&m_EnterInfo, 0, sizeof(m_EnterInfo));
	m_pPlayer = nullptr;
	m_iGmLevel = 0;
	m_bScrollTextMode = FALSE;
	m_bNoticeMode = FALSE;
	m_bCompetlyQuit = FALSE;
	m_tmrMsgRate.Savetime();
	m_dwMsgCount = 0;
	m_dwMsgWarnCount = 0;
}

VOID CClientObj::Update()
{
	CClientObject::Update();
}

VOID CClientObj::OnDBMsg(PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	switch (pMsg->wCmd)
	{
	case DM_QUERYTASKINFO:
	{
		if (m_pPlayer)
			m_pPlayer->OnTaskInfo((TASKINFO*)pMsg->data);
	}
	break;
	case DM_QUERYFENGHAO:
	{
		if (m_pPlayer)
			m_pPlayer->OnFengHaoInfo((FenghaoInfo*)pMsg->data);
	}
	break;
	case DM_QUERYUPGRADEITEM:
	{
		DWORD dwKey = *(DWORD*)&pMsg->wParam[0];
		if (dwKey == m_dwClientKey && pMsg->wParam[2] > 0)
		{
			if (m_pPlayer)
				m_pPlayer->SetUpgradeItem(((DBITEM*)pMsg->data)->item);
		}
	}
	break;
	case DM_QUERYMAGIC:
	{
		DWORD key = (pMsg->wParam[1] << 16) | pMsg->wParam[0];
		if (pMsg->wParam[2] & 0x8000 || key != m_dwClientKey)
		{
		}
		else if (m_pPlayer != nullptr)
		{
			m_pPlayer->SetMagic((MAGICDB*)pMsg->data, pMsg->wParam[2]);
			m_pPlayer->SendMagicList();
		}
	}
	break;
	case DM_QUERYITEMS:
	{
		DWORD* pdwArray = (DWORD*)pMsg->data;
		ITEM item;
		if (pMsg->wParam[0] == SE_OK && pdwArray[0] == m_dwClientKey)
		{
			DBITEM* pItems = (DBITEM*)(pMsg->data + sizeof(DWORD));
			OnDBItem(pItems, pMsg->wParam[2], (BYTE)pMsg->wParam[1]);
		}
	}
	break;
	case DM_GETCHARDBINFO:
	{
		CHARDBINFO* pInfo = (CHARDBINFO*)pMsg->data;
		if (pInfo->dwClientKey != m_dwClientKey)
			break;
		if (pMsg->wParam[0] != SE_OK)
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"读取数据库失败, 请联系管理员解决!");
			Disconnect(3000);
			break;
		}
		CREATEHUMANDESC	desc;
		desc.dbinfo = *pInfo;
		desc.pClientObj = this;
		m_pPlayer = CHumanPlayerMgr::GetInstance()->NewPlayer();
		if (m_pPlayer == nullptr)
		{
			Disconnect();
			break;
		}
		if (!m_pPlayer->Init(desc) || !CHumanPlayerMgr::GetInstance()->AddPlayerNameList(m_pPlayer, desc.dbinfo.szName))
		{
			CHumanPlayerMgr::GetInstance()->DeletePlayer(m_pPlayer);
			m_pPlayer = nullptr;
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"初始化失败!");
			Disconnect(3000);
			LG2("玩家 %s 初始化失败\n", desc.dbinfo.szName);
			break;
		}
		m_pPlayer->LoadVars();
		CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
		if (pDI)
		{
			// 加载数据
			pDI->SendQueryMagic(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->SendQueryUpgradeItem(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_BAG, m_pPlayer->GetBag().GetCountLimit());
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_BANK, 100);
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_PETBANK, 10);
			pDI->QueryTaskInfo(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->QueryFengHaoInfo(getId(), m_dwClientKey, m_pPlayer->GetDBId());
		}
		else
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
			Disconnect(3000);
			return;
		}
		m_State = GSUM_WAITINGCONFIRM;
	}
	break;
	case DM_CREATEITEM:
	{
		CREATEITEM* pInfo = (CREATEITEM*)pMsg->data;
		if (pInfo->dwClientKey != m_dwClientKey)
			break;
		OnCreateItem(pInfo->item, pInfo->wPos, pInfo->btFlag);
	}
	break;
	case DM_QUERYCOMMUNITY:
	{
		DWORD dwKey = pMsg->wParam[0] | (pMsg->wParam[1] << 16);
		if (dwKey != m_dwClientKey)
			break;
		if (m_pPlayer)
			m_pPlayer->OnCommunityInfo(pMsg->data);
	}
	break;
	default:
#ifdef _DEBUG
	{
		pServer->OnUnknownMsg(pMsg, datasize);
	}
#endif
	break;
	}
}

VOID CClientObj::OnCreateItem(ITEM& item, int pos, BYTE btFlag)
{
	if (btFlag == IDF_BAG)
		m_pPlayer->AddBagItem(item);
	else
		m_pPlayer->DropItem(item);
}

VOID CClientObj::OnVerifyString(const char* pszString)
{
	//***loginid/角色名字/selectcharid/20161104/0
	//***382198915/天斩/12941735/20161104/0
	// 复制到本地缓冲区，避免修改const数据
	std::array<char, 256> szLocalBuf{};
	strncpy(szLocalBuf.data(), pszString, szLocalBuf.size() - 1);
	char* p = szLocalBuf.data();
	if (p[0] == '*' && p[1] == '*' && p[2] == '*')
		p += 3;
	UINT nLoginId = 0;
	UINT nSelCharId = 0;
	char* Params[5];
	int nParam = SearchParam(p, Params, 5, "/");
	if (nParam == 5)
	{
		if (_stricmp(Params[3], "20161104") != 0) // 这个判断在登录网关也判断了一次,  客户端版本时间
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"你的客户端版本不支持!");
			Disconnect(3000);
			return;
		}
		nLoginId = (UINT)strtoul(Params[0], nullptr, 10);
		nSelCharId = (UINT)strtoul(Params[2], nullptr, 10);
		if (CServer::GetInstance()->GetEnterInfo(nLoginId, nSelCharId, nullptr, m_EnterInfo))
		{
			CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(m_EnterInfo.szName.data());
			if (pPlayer != nullptr) // 进入游戏时，如果玩家已经在游戏中处理
			{
				pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"你的账号已在其他地方登录!");
				// 立即从名称列表移除旧玩家，防止AddPlayerNameList因重名失败
				CHumanPlayerMgr::GetInstance()->RemovePlayerNameList(pPlayer->GetName());
				pPlayer->GetClientObject()->Disconnect();
			}
			CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
			if (!pDI)
			{
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
				Disconnect(3000);
				return;
			}
			else
				pDI->SendQueryDbInfo(getId(), m_dwClientKey, m_EnterInfo.szAccount.data(), CServer::GetInstance()->GetNickName(), Params[1]);
			m_State = GSUM_WAITINGDBINFO;
			return;
		}
	}
	Disconnect(3000);
}

VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	// 消息频率限制
	if (m_tmrMsgRate.IsTimeOut(MSG_RATE_WINDOW))
	{
		// 检查是否超警告阈值
		if (m_dwMsgCount > MSG_RATE_WARN)
			m_dwMsgWarnCount++;
		else
			m_dwMsgWarnCount = 0;
		m_dwMsgCount = 0;
		m_tmrMsgRate.Savetime();
	}
	m_dwMsgCount++;
	// 超过最大限制或连续3次超警告阈值，断开连接
	if (m_dwMsgCount > MSG_RATE_MAX || m_dwMsgWarnCount >= 3)
	{
		Disconnect();
		return;
	}
	if (m_State != GSUM_VERIFIED)
	{
		switch (m_State)
		{
		case GSUM_NOTVERIFIED:
		{
			OnVerifyString((char*)pMsg);
		}
		break;
		case GSUM_WAITINGCONFIRM:
		{
			ProcClientMsg(pMsg, datasize);
		}
		break;
		}
		return;
	}
	if (m_pPlayer == nullptr) return;
	if (m_pPlayer->IsDeath())
	{
		if (pMsg->wCmd != CM_SOFTCLOSE && pMsg->wCmd != CM_SAY && pMsg->wCmd != CM_SETITEMPOSITION && pMsg->wCmd != CM_LEAVESERVER)
		{
			m_pPlayer->SaySystem(getstrings(SD_STDERROR_YOUHAVEALREADYDEAD));
			return;
		}
	}
	ProcClientMsg(pMsg, datasize);
}

VOID CClientObj::SendActionResult(BOOL bSuccess)
{
	if (m_pPlayer == nullptr) return;
	char szMsg[32];
	if (bSuccess)
		snprintf(szMsg, sizeof(szMsg), "#+G/%d/%d!", m_pPlayer->getX(), m_pPlayer->getY());
	else
		snprintf(szMsg, sizeof(szMsg), "#+FL/%d/%d!", m_pPlayer->getX(), m_pPlayer->getY());
	PostMsg(szMsg, (int)strlen(szMsg));
}

VOID CClientObj::OnDisconnect()
{
	// 处理所有排队的数据包，避免断开时丢失数据
	xPacket* pPacket = nullptr;
	while ((pPacket = m_xPacketQueue.pop()) != nullptr)
	{
		OnDataPacket(pPacket);
		getServer()->releasePacket(pPacket);
	}
	if (m_pPlayer != nullptr)
	{
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "LogoutEnv.Logout", FALSE);
		if (m_pPlayer->GetGuild()) // 行会处理
			m_pPlayer->GetGuild()->MemberLogoff(m_pPlayer);
		if (m_pPlayer->GetExchangeObject() != nullptr) // 交易处理
			m_pPlayer->GetExchangeObject()->End(m_pPlayer, ET_CANCEL);
		if (m_pPlayer->GetGroupObject()) // 组队处理
			m_pPlayer->GetGroupObject()->DelMember(m_pPlayer);
		if (m_pPlayer->GetMap() != nullptr) // 地图处理
		{
			DWORD dwParam = 0;
			if (m_pPlayer->GetMap()->IsFlagSeted(MF_NORECONNECT, dwParam))
			{
				if (!m_pPlayer->RandomTeleport(dwParam))
					m_pPlayer->Home();
			}
		}
		CTopManager::GetInstance()->UpdateTopInfo(m_pPlayer); // 更新排行榜
		m_pPlayer->CleanPets();//清除宠物
		m_pPlayer->NoticeFriendOffline();//朋友上下线通知
		m_pPlayer->SaveVars();//保存变量
		m_pPlayer->UpdateToDB();//更新到数据库
		m_pPlayer->UpdateTaskToDB();//更新任务到数据库
		m_pPlayer->UpdateFengHaoToDB();//更新时长封号到数据库
		m_pPlayer->UpdateItemsToDB();//更新物品到数据库
		CGameWorld::GetInstance()->RemoveMapObject(m_pPlayer);//从地图中移除玩家
		CHumanPlayerMgr::GetInstance()->DeletePlayer(m_pPlayer);//释放玩家, 
	}

	if (!m_bCompetlyQuit && this->m_State == GSUM_VERIFIED)
	{
		CSCClientObj* pObj = CServer::GetInstance()->GetSCConnection();
		if (pObj != nullptr)
			pObj->SendMsgAcrossServer(0, MAS_RESTARTGAME, MST_SINGLE, (WORD)m_EnterInfo.dwSelectCharServerId, (char*)&m_EnterInfo, sizeof(m_EnterInfo));
	}
}

VOID CClientObj::OnCommand(const char* pszCommand)
{
	if (m_pPlayer == nullptr) return;
	CGmManager::GetInstance()->ExecGameCmd(pszCommand, m_pPlayer);
	return;
}

//增加包裹物品需要转换为客户端物品
VOID CClientObj::SendAddItem(ITEM& item)
{
	if (m_pPlayer == nullptr) return;
	ITEMCLIENT clientItem{};
	memcpy(&clientItem, &item, sizeof(ITEMCLIENT));
	m_pPlayer->SendMsg(m_pPlayer->GetId(), SM_ADDBAGITEM, 0, 0, 0, &clientItem, sizeof(ITEMCLIENT));
}

static HandlerTable g_handlerTable;
VOID CClientObj::ProcClientMsg(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer) return;
	HandlerTable::MsgHandler handler = g_handlerTable.GetHandler(pMsg->wCmd);
	if (handler != nullptr)
	{
		(this->*handler)(pMsg, datasize);
	}
#ifdef _DEBUG
	else
	{
		CServer* pServer = CServer::GetInstance();
		pServer->OnUnknownMsg(pMsg, datasize);
	}
#endif
}

VOID CClientObj::SendEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex)
{
	WORD wCmd = SM_TAKEON_OK;
	DWORD dwFeather = m_pPlayer->GetFeather();
	if (!bSuccess)
	{
		wCmd = SM_TAKEON_FAIL;
		dwFeather = 0xffffffff;
		m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
		return;
	}
	m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
	m_pPlayer->SendFeatureChanged();//41外观信息
	m_pPlayer->UpdateProp();  //更新52人物信息
	m_pPlayer->UpdateSubProp(); //更新人物附加属性752
	m_pPlayer->SendStatusChanged();//657
	CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_EQUIPMENT, pos);
}

VOID CClientObj::SendUnEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex)
{
	WORD wCmd = SM_TAKEOFF_OK;
	DWORD dwFeather = m_pPlayer->GetFeather();
	if (!bSuccess)
	{
		wCmd = SM_TAKEOFF_FAIL;
		dwFeather = 0xffffffff;
		m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
		return;
	}
	m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
	m_pPlayer->SendMsg(0, 0x26c, 0, 0, 0);
	m_pPlayer->SendFeatureChanged();
	m_pPlayer->UpdateProp();
	m_pPlayer->UpdateSubProp();
	m_pPlayer->SendStatusChanged();
	CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_BAG, pos);
}

VOID CClientObj::SendClientNewMail(WORD Parm1, WORD Parm2, WORD Parm3)
{
	xPacketPool::ScopedPacket packet(65535);
	switch (Parm2)
	{
	case 0:
	{
	}
	break;
	case 1:
	{
	}
	break;
	case 2:
	{
	}
	break;
	case 3:
	{
	}
	break;
	case 4:
	{
	}
	break;
	case 5:
	{
	}
	break;
	}
	m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9A2, Parm1, Parm2, Parm3, (LPVOID)packet->getbuf(), packet->getsize());
}

VOID CClientObj::SendBagItems(DBITEM* pItems, int count)
{
	// 背包物品数据
	if (count > BIGBAG_SLOT) count = BIGBAG_SLOT;
	std::vector<ITEMCLIENT> items(BIGBAG_SLOT);
	for (int i = 0; i < count; i++)
	{
		items[i] = *(ITEMCLIENT*)&pItems[i].item;
	}
	m_pPlayer->SendMsg(m_pPlayer->GetId(), SM_BAGINFO, 0, 0, count, items.data(), sizeof(ITEMCLIENT) * count);
	// 背包物品位置数据 - 使用与count一致的截断值
	std::vector<BAGITEMPOS> itempos(BIGBAG_SLOT);
	for (int i = 0; i < count; i++)
	{
		itempos[i].ItemId = pItems[i].item.dwMakeIndex;
		itempos[i].wPos = pItems[i].pos;
	}
	if (m_pPlayer) m_pPlayer->SetBagItemPos(itempos.data(), count);
	m_pPlayer->SendMsg(0, SM_SETITEMPOSITION, 0, 0, count, (LPVOID)itempos.data(), sizeof(BAGITEMPOS) * count);
}

VOID CClientObj::SendEquipments()
{
	EQUIPMENT equipments[_U_MAX];
	int count = m_pPlayer->GetEquipments(equipments);
	m_pPlayer->SendMsg(0, SM_EQUIPMENTS, 0, 0, 0, (LPVOID)equipments, sizeof(EQUIPMENT) * count);
	m_pPlayer->SendFeatureChanged();
	m_pPlayer->UpdateProp();
	m_pPlayer->UpdateSubProp();
	m_pPlayer->SendStatusChanged();
}

static int qsort_comp_dbitem(const DBITEM* p1, const DBITEM* p2)
{
	if (p1->pos > p2->pos)return 1;
	if (p1->pos < p2->pos)return -1;
	return 0;
}

VOID CClientObj::OnDBItem(DBITEM* pItemArray, int nCount, BYTE btFlag)
{
	ITEM item;
	if (nCount > 1)
	{
		qsort((VOID*)pItemArray, nCount, sizeof(DBITEM), (int(*)(const VOID*, const VOID*))qsort_comp_dbitem);
	}
	switch (btFlag)
	{
	case IDF_BANK:
	{
		for (int i = 0; i < nCount; i++)
		{
			m_pPlayer->AddBankItem(pItemArray[i].item, FALSE);
		}
	}
	break;
	case IDF_BAG:
	{
		m_pPlayer->SetSystemFlag(SF_BAGLOADED, TRUE);
		for (int i = 0; i < nCount; i++)
		{
			item = pItemArray[i].item;
			m_pPlayer->ItemToClient(item); // 处理数据
			m_pPlayer->AddBagItem(pItemArray[i].item, TRUE, FALSE, FALSE);
			pItemArray[i].item = item;
		}
		//	发送背包信息和位置信息.
		SendBagItems(pItemArray, nCount);
		m_pPlayer->SendWeightChanged();
	}
	break;
	case IDF_EQUIPMENT:
	{
		m_pPlayer->SetSystemFlag(SF_EQUIPMENTLOADED, TRUE);
		for (int i = 0; i < nCount; i++)
		{
			item = pItemArray[i].item;
			m_pPlayer->ItemToClient(item); // 处理数据
			m_pPlayer->EquipItem(pItemArray[i].pos, pItemArray[i].item, TRUE);
			pItemArray[i].item = item;
		}
		//	发送装备信息
		//	发送属性更改消息
		SendEquipments();
	}
	break;
	case IDF_PETBANK:
	{
		m_pPlayer->OnPetBank(pItemArray, nCount);
	}
	break;
	}
}

VOID CClientObj::OnConnection()
{
	EnableBatchMode(TRUE);
}