#include "StdAfx.h"
#include ".\groupobject.h"
#include ".\humanplayer.h"
#include ".\groupobjectmgr.h"
#include "gameworld.h"
#include "GuildEx.h"
#include "GuildManager.h"

CGroupObject::CGroupObject(VOID)
{
	m_xMembersArray.Create(CGameWorld::GetInstance()->GetVar(EVI_MAXGROUPMEMBER));
	m_xMembersArray.Clean();
}

CGroupObject::~CGroupObject(VOID)
{
}

BOOL CGroupObject::Create(CHumanPlayer* pLeader, CHumanPlayer* pFirstMember)
{
	UINT index = m_xMembersArray.Add(pLeader);
	if (index == (UINT)-1)return FALSE;
	pLeader->SetGroupObject(this);
	index = m_xMembersArray.Add(pFirstMember);
	if (index == (UINT)-1)
	{
		DestroyGroup();
		return FALSE;
	}
	pFirstMember->SetGroupObject(this);
	UpdateNameList();
	SaySystemAttrib(CC_GROUPTIPS, "-%s 加入编组", pLeader->GetName());
	SaySystemAttrib(CC_GROUPTIPS, "-%s 加入编组", pFirstMember->GetName());
	return TRUE;
}

BOOL CGroupObject::AddMember(CHumanPlayer* pMember)
{
	UINT idx = m_xMembersArray.Add(pMember);
	if (idx != (UINT)-1)
	{
		pMember->SetGroupObject(this);
		UpdateNameList();
		SaySystemAttrib(CC_GROUPTIPS, "-%s 加入编组", pMember->GetName());
		return TRUE;
	}
	if (m_xMembersArray[(UINT)0] != nullptr)
		m_xMembersArray[(UINT)0]->SaySystem("编组已满, 无法添加新成员!");
	return FALSE;
}

BOOL CGroupObject::IsMemberDBId(DWORD dwId)
{
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] != nullptr && m_xMembersArray[i]->GetDBId() == dwId)
			return TRUE;
	}
	return FALSE;
}

BOOL CGroupObject::IsMember(CHumanPlayer* pPlayer)
{
	return (m_xMembersArray[pPlayer] != (UINT)-1);
}

BOOL CGroupObject::IsLeader(CHumanPlayer* pPlayer)
{
	return (m_xMembersArray[(UINT)0] == pPlayer);
}

VOID CGroupObject::LeaveMember(CHumanPlayer* pMember)
{
	if (pMember == m_xMembersArray[(UINT)0])
	{
		SaySystemAttrib(CC_GROUPTIPS, "-%s 离开小组", pMember->GetName());
		DestroyGroup();
		return;
	}
	SaySystemAttrib(CC_GROUPTIPS, "-%s 离开小组", pMember->GetName());
	if (m_xMembersArray.Del(pMember))
	{
		pMember->SendMsg(0, SM_GROUPDESTROYED, 0, 0, 0);
		pMember->SetGroupObject(nullptr);
		//pMember->SaySystem( "你被开除出编组!" );
		//	删除到只剩下队长
		if (m_xMembersArray.GetCount() <= 1)
			DestroyGroup();
		else
			UpdateNameList();
	}
}

VOID CGroupObject::DelMember(CHumanPlayer* pMember)
{
	//	删除队长
	if (pMember == m_xMembersArray[(UINT)0])
	{
		DestroyGroup();
		return;
	}
	SaySystemAttrib(CC_GROUPTIPS, "-%s 退出小组", pMember->GetName());
	if (m_xMembersArray.Del(pMember))
	{
		pMember->SendMsg(0, SM_GROUPDESTROYED, 0, 0, 0);
		pMember->SetGroupObject(nullptr);
		// 删除到只剩下队长
		if (m_xMembersArray.GetCount() <= 1)
			DestroyGroup();
		else
			UpdateNameList();
	}
}

VOID CGroupObject::DestroyGroup()
{
	SendMsg(nullptr, 0, SM_GROUPDESTROYED, 0, 0, 0);
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] == nullptr) continue;
		m_xMembersArray[i]->SetGroupObject(nullptr);
		m_xMembersArray[i]->SaySystemAttrib(CC_GROUPTIPS, "-小组被解散了");
	}
	m_xMembersArray.Clean();
	CGroupObjectMgr::GetInstance()->DestroyGroup(this);
}

VOID CGroupObject::SendMsg(CHumanPlayer* pSender, DWORD dwFlag, WORD wCmd, WORD wParam1, WORD wParam2, WORD wParam3, LPVOID lpData, int datasize)
{
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] != nullptr && pSender != m_xMembersArray[i])
			m_xMembersArray[i]->SendMsg(dwFlag, wCmd, wParam1, wParam2, wParam3, lpData, datasize);
	}
}

VOID CGroupObject::UpdateNameList()
{
	// 使用安全拼接，防止栈缓冲区溢出
	std::string szNameList;
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] == nullptr) continue;
		if (i > 0) szNameList += '/';
		szNameList += m_xMembersArray[i]->GetName();
	}
	SendMsg(nullptr, 0, SM_GROUPMEMBERLIST, 0, 0, 0, (LPVOID)szNameList.c_str(), static_cast<int>(szNameList.size()));
}

VOID CGroupObject::UpdateMemberPosition(CHumanPlayer* p)
{
	std::array<WORD, 20> wArray{};
	int ptr = 0;
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] == nullptr || m_xMembersArray[i] == p) continue;
		if (ptr + 2 > (int)wArray.size()) break; // 防止数组越界
		wArray[ptr++] = m_xMembersArray[i]->getX();
		wArray[ptr++] = m_xMembersArray[i]->getY();
	}
	p->SendMsg(0, SM_UPDATEMEMBERINFO, 0, 0, 0, (LPVOID)wArray.data(), ptr * sizeof(WORD));
}

VOID CGroupObject::AdjustGroupExp(CHumanPlayer* p, DWORD dwExp, DWORD dwId)
{
	int	sumlevel = 0;
	int	sumaround = 0;
	const int MAX_AROUND_MEMBERS = 20;
	std::array<CHumanPlayer*, MAX_AROUND_MEMBERS> pAoundList{};
	int px = p->getX();
	int py = p->getY();
	CLogicMap* pMap = p->GetMap();
	CGuildEx* pGuild = p->GetGuild();
	BOOL IsGuildMembers = TRUE;
	for (UINT i = 0; i < m_xMembersArray.GetCount(); i++)
	{
		if (m_xMembersArray[i] == nullptr) continue;
		if (m_xMembersArray[i]->GetMap() == pMap)
		{
			if (DISTANCE(px, py, m_xMembersArray[i]->getX(), m_xMembersArray[i]->getY()) <= 12 &&
				!m_xMembersArray[i]->IsDeath())
			{
				if (sumaround < MAX_AROUND_MEMBERS)
					pAoundList[sumaround++] = m_xMembersArray[i];
				sumlevel += m_xMembersArray[i]->GetPropValue(PI_LEVEL);
			}
			if (m_xMembersArray[i]->GetGuild() != pGuild)
				IsGuildMembers = FALSE;
		}
		else
			IsGuildMembers = FALSE;
	}
	if (sumlevel <= 0)
		sumlevel = 1;
	if (sumaround <= 1)
		p->WinExp(dwExp, FALSE, dwId);
	else
	{
		BOOL IsGuildExp = FALSE;
		DWORD dwAddGuildExp = 0;
		if (IsGuildMembers && m_xMembersArray.GetCount() == sumaround)
			IsGuildExp = TRUE;
		for (int i = 0; i < sumaround; i++)
		{
			DWORD dwWinExp = dwExp * pAoundList[i]->GetPropValue(PI_LEVEL) / sumlevel;
			pAoundList[i]->WinExp(dwWinExp, FALSE, dwId);
			if (pGuild && IsGuildExp)
			{
				DWORD dwExtraExp = dwWinExp * sumaround / 10;
				pAoundList[i]->SaySystem("%s 行会增加%d行会经验, 额外增加%d行会经验.", pGuild->GetName(), dwWinExp, dwExtraExp);
				dwAddGuildExp += dwWinExp + dwExtraExp;
			}
		}
		if (pGuild && dwAddGuildExp != 0)
			pGuild->AddExp(dwAddGuildExp);
	}
}

VOID CGroupObject::SaySystemAttrib(DWORD dwAttrib, const char* pszMsg, ...)
{
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	SendMsg(nullptr, 0, SM_SYSCHAT, LOWORD(dwAttrib), HIWORD(dwAttrib), 0, szBuff);
}