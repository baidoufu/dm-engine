#pragma once

class CWeatherSystem : public xSingletonClass<CWeatherSystem>, public CTimeEventObject
{
public:
	CWeatherSystem(VOID);
	virtual ~CWeatherSystem(VOID);
	VOID OnMinuteChange(CSystemTime& curTime);
};