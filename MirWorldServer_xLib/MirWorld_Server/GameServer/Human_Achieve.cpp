#include "stdafx.h"
#include "TimeAchieve.h"
#include "HumanPlayer.h"
#include "systemscript.h"

VOID CHumanPlayer::InitAchievement(int nCount)
{
	auto* ac = GetAchieveComp();
	if (!ac) return; // 构造期间ECS组件尚未创建, 安全跳过
	ac->Data.btLevel = 0;
	ac->Data.dwExp = 0;
	ac->Data.btStatus.assign(nCount, 0);
	ac->Data.btRecentCount.assign(nCount, 0);
	ac->Data.dwCompleteTime.assign(nCount, 0);
}

BOOL CHumanPlayer::ChangeAchieveGroupExp(BYTE btGroupId, BYTE btType, DWORD btRecentCount)
{
	if (btType>2) return FALSE;
	// 获取该组的所有成就索引
	const std::vector<int>* pIndexList = CTimeAchieve::GetInstance()->FindAchieveGroupById(btGroupId);
	if (pIndexList == nullptr || pIndexList->empty()) return FALSE;
	DWORD dwNow = GetUnixTimeSec();
	// 遍历该组所有成就，增加进度值
	for (int nIndex : *pIndexList)
	{
		const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->GetAchieve(nIndex);
		const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(pAchieveItem->nId);
		if (pAchieveItem == nullptr || pIndex == -1) continue;
		// 检查成就是否已完成或已领取
		if (_achievement().btStatus[pIndex] >= 1) continue;
		DWORD& refCount = _achievement().btRecentCount[pIndex];
		switch (btType)
		{
		case 0:
			if (refCount > (0xffffffff - btRecentCount)) refCount = 0xffffffff;
			else refCount += btRecentCount;
		break;
		case 1:
			if (refCount < btRecentCount) refCount = 0;
			else refCount -= btRecentCount;
		break;
		case 2:
			refCount = btRecentCount;
		break;
		}
		// 检查是否达到完成条件
		if (pAchieveItem->nMaxExp > 0 && _achievement().btRecentCount[pIndex] >= (DWORD)pAchieveItem->nMaxExp)
		{
			_achievement().dwExp += pAchieveItem->nPoint; // 增加成就点
			_achievement().btStatus[pIndex] = 1; // 标记为可领取
			_achievement().dwCompleteTime[pIndex] = dwNow; // 设置完成时间戳
			setSParam(0, pAchieveItem->szName.data()); // 成就名
			setVParam(1, nIndex); // 成就ID
			CSystemScript::GetInstance()->Execute(GetScriptTarget(), "成就系统.完成奖励");
		}
		CheckAchieveLevelUp();
	}
	return TRUE;
}

BOOL CHumanPlayer::ChangeAchieveExp(WORD wId, BYTE btType, DWORD btRecentCount)
{
	if (btType > 2) return FALSE;
	// 获取成就对象
	const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(wId);
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pAchieveItem == nullptr || pIndex == -1) return FALSE;
	// 检查成就是否已完成或已领取
	if (_achievement().btStatus[pIndex] >= 1) return FALSE;
	DWORD& refCount = _achievement().btRecentCount[pIndex];
	switch (btType)
	{
	case 0:
		if (refCount > (0xffffffff - btRecentCount)) refCount = 0xffffffff;
		else refCount += btRecentCount;
		break;
	case 1:
		if (refCount < btRecentCount) refCount = 0;
		else refCount -= btRecentCount;
		break;
	case 2:
		refCount = btRecentCount;
		break;
	}
	// 检查是否达到完成条件
	if (pAchieveItem->nMaxExp > 0 && refCount >= (DWORD)pAchieveItem->nMaxExp)
	{
		_achievement().dwExp += pAchieveItem->nPoint; // 增加成就点
		_achievement().btStatus[pIndex] = 1; // 标记为可领取
		DWORD dwNow = GetUnixTimeSec();
		_achievement().dwCompleteTime[pIndex] = dwNow; // 设置完成时间戳
		setSParam(0, pAchieveItem->szName.data()); // 成就名
		setVParam(1, wId); // 成就ID
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "成就系统.完成奖励");
	}
	CheckAchieveLevelUp();
	return TRUE;
}

BOOL CHumanPlayer::SetAchieveState(WORD wId, BYTE btStatu)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return FALSE;
	_achievement().btStatus[pIndex] = btStatu;
	return TRUE;
}

BOOL CHumanPlayer::SetAchieveTime(WORD wId, DWORD dwTime)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return FALSE;
	_achievement().dwCompleteTime[pIndex] = dwTime;
	return TRUE;
}

BOOL CHumanPlayer::SendGotAchieve(WORD wId)
{
	// 获取成就对象
	const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(wId);
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pAchieveItem == nullptr || pIndex == -1) return FALSE;

	xPacketPool::ScopedPacket packet;
	int nValue = 1;
	packet->push(&nValue, 1);
	packet->push(4);
	nValue = 1;
	packet->push(&nValue, 4);
	nValue = pAchieveItem->nId;
	packet->push(&nValue, 4);
	nValue = 1;
	packet->push(&nValue, 4);
	packet->push(4);
	nValue = 1;
	packet->push(&nValue, 4);
	packet->push(4);
	packet->push(4);
	packet->push(4);
	packet->push(5);
	SendMsg(GetId(), 0x959, 0, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
	return TRUE;
}

BOOL CHumanPlayer::ChangeAchievePoint(BYTE btType, DWORD dwExp)
{
	
	DWORD dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(_achievement().btLevel);
	if (dwMaxExp == -1) return FALSE;
	switch (btType)
	{
	case 0:
		_achievement().dwExp += dwExp;
		break;
	case 1:
		_achievement().dwExp -= dwExp;
		break;
	case 2:
		_achievement().dwExp = dwExp;
		break;
	}
	// 检查是否达到升级条件
	if (_achievement().dwExp >= dwMaxExp)
	{
		_achievement().dwExp = 0;
		_achievement().btLevel++;
		SaySystem("恭喜您，成就等级提升到 %u 级!", _achievement().btLevel);
	}
	return TRUE;
}

DWORD CHumanPlayer::GetAchieveExpById(WORD wId)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return _achievement().btRecentCount[pIndex];
}

BYTE CHumanPlayer::GetAchieveStateById(WORD wId)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return _achievement().btStatus[pIndex];
}

DWORD CHumanPlayer::GetAchieveCompleteTimeById(WORD wId)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return _achievement().dwCompleteTime[pIndex];
}

VOID CHumanPlayer::CheckAchieveLevelUp() 
{
	BYTE& refLevel = _achievement().btLevel;
	DWORD& refExp = _achievement().dwExp;
	DWORD dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(refLevel);
	if (dwMaxExp == -1) return;
	while (refExp >= dwMaxExp)
	{
		refExp -= dwMaxExp;
		refLevel++;
		SaySystem("恭喜您，成就等级提升到 %u 级!", refLevel);
		dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(refLevel);
		if (dwMaxExp == -1) break;
	}
}

VOID CHumanPlayer::PacketAchieve(xPacket& packet, BYTE btType, DWORD nAchieveCount)
{
	switch (btType)
	{
	case 0:// i=0时,状态值(0未完成,1已完成/可领取,2已领取)
	{
		for (DWORD i = 0; i < nAchieveCount; i++)
		{
			DWORD dwStatus = (DWORD)_achievement().btStatus[i];
			packet.push(&dwStatus, 4);
		}
	}
	break;
	case 1:// i=1时,近期获得成就
	{
		for (DWORD i = 0; i < nAchieveCount; i++)
			packet.push(&_achievement().btRecentCount[i], 4);
	}
	break;
	case 2:// i=2时,完成时间戳
	{
		for (DWORD i = 0; i < nAchieveCount; i++)
			packet.push(&_achievement().dwCompleteTime[i], 4);
	}
	break;
	}
}