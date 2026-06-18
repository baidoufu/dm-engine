#include "StdAfx.h"
#include "weathersystem.h"
#include "gameworld.h"
#include "TimeSystem.h"

CWeatherSystem::CWeatherSystem(VOID)
{
}

CWeatherSystem::~CWeatherSystem(VOID)
{
}

VOID CWeatherSystem::OnMinuteChange(CSystemTime& curTime)
{
	WORD wTime = curTime.GetHour() * 4 + curTime.GetMinute() / 15;
	CTimeSystem* timeSystem = CTimeSystem::GetInstance();
	if (timeSystem && wTime != timeSystem->GetCurrentlyTime())
	{
		CGameWorld::GetInstance()->AddGlobeProcess(EP_TIMECHANGED, wTime);
		timeSystem->SetCurrentlyTime(wTime);
	}
}