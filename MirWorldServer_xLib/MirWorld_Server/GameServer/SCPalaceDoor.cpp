#include "StdAfx.h"
#include ".\scpalacedoor.h"
#include "SandCity.h"
#include "HumanPlayer.h"

CSCPalaceDoor::CSCPalaceDoor(VOID)
{
	Clean();
}

CSCPalaceDoor::~CSCPalaceDoor(VOID)
{
}

VOID CSCPalaceDoor::OnEnter(CMapObject* pObject)
{
	CHumanPlayer* pPlayer = nullptr;
	if (pObject->GetType() == OBJ_PLAYER)
	{
		pPlayer = (CHumanPlayer*)pObject;
		if (pPlayer->GetGuild() != nullptr)
		{
			// 只有沙城成员才能出入
			if (pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
				CChangeMapEvent::OnEnter(pObject);
		}
	}
}