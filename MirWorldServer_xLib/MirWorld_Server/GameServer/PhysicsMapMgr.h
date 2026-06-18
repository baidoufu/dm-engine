#pragma once
#include "findfile.h"
#include <array>

class CPhysicsMap;

class CPhysicsMapMgr : public xSingletonClass<CPhysicsMapMgr>
{
public:
	CPhysicsMapMgr(VOID);
	virtual ~CPhysicsMapMgr(VOID);

	CPhysicsMap* Load(const char* pszName);

	VOID Init(const char* pszPath, const char* pszCachePath);

	CPhysicsMap* GetPhysicsMapByName(const char* pszMapName);
private:
	std::array<char, 1024> m_szPhysicsMapPath{};
	std::array<char, 1024> m_szPhysicsCachePath{};
	xStringList<128> m_xMapList;
	BOOL m_bUseCache;
};