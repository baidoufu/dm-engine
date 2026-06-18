#pragma once
#include "vmap.h"
#include <array>
#include <memory>

class CPhysicsMap
{
public:
	CPhysicsMap(VOID);
	virtual ~CPhysicsMap(VOID);
	BOOL LoadMap(const char* pszFilename);
	BOOL LoadCache(const char* pszCacheFilename);
	BOOL SaveCache(const char* pszPath);
	BOOL IsBlocked(int x, int y);
	int	GetHeight()const { return m_iHeight; }
	int	GetWidth()const { return m_iWidth; }
	DWORD* GetBlockLayer() { return m_pBlockLayer.get(); }
	const char* GetName() { return m_szMapName.data(); }
private:
	BOOL VerifyPos(int x, int y)const
	{
		if (x < 0 || x >= m_iWidth)return FALSE;
		if (y < 0 || y >= m_iHeight)return FALSE;
		return TRUE;
	}
	DWORD m_dwVer;
	int m_iWidth;
	int m_iHeight;
	std::unique_ptr<DWORD[]> m_pBlockLayer;
	DWORD m_iMaxBlockElements;
	std::array<char, 32> m_szMapName{};
};