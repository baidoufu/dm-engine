#pragma once
#include "changemapevent.h"

class CSCPalaceDoor : public CChangeMapEvent
{
public:
	CSCPalaceDoor(VOID);
	virtual ~CSCPalaceDoor(VOID);
	virtual VOID OnEnter(CMapObject* pObject);
};
