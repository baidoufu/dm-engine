#include "StdAfx.h"
#include ".\market.h"
#include ".\submarket.h"
#include ".\humanplayer.h"

CMarket::CMarket(UINT id)
{
	m_nSubMarketCount = 0;
	this->m_Id = id;
}

CMarket::~CMarket(VOID)
{
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n])
		{
			delete this->m_pSubMarketArray[n];
			this->m_pSubMarketArray[n] = nullptr;
		}
	}
}

CSubMarket* CMarket::AddSubMarket(UINT nId, const char* pszName)
{
	if (this->m_nSubMarketCount >= 32)
		return nullptr;
	std::array<char, 256> szFilename{};

	this->m_pSubMarketArray[this->m_nSubMarketCount] = new CSubMarket(this->m_Id, nId, pszName);

	sprintf(szFilename.data(), ".\\Data\\Market\\%02u%02u.txt", this->m_Id, nId);
	if (!this->m_pSubMarketArray[this->m_nSubMarketCount]->LoadSubMarket(szFilename.data()))
	{
		delete this->m_pSubMarketArray[this->m_nSubMarketCount];
		this->m_pSubMarketArray[this->m_nSubMarketCount] = nullptr;
		return nullptr;
	}
	return this->m_pSubMarketArray[this->m_nSubMarketCount++];
}

CSubMarket* CMarket::GetSubMarket(UINT nId)
{
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n] && this->m_pSubMarketArray[n]->GetId() == nId)
			return this->m_pSubMarketArray[n];
	}
	return nullptr;
}

VOID CMarket::SendSubMarket(CHumanPlayer* pPlayer)
{
	xPacketPool::ScopedPacket packet(65536);
	sprintf((char*)packet->getbuf(), "%02u&", m_Id);
	packet->addsize((int)strlen(packet->getbuf()));
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n])
		{
			sprintf((char*)packet->getfreebuf(), "%02u|%s&", this->m_pSubMarketArray[n]->GetId(), this->m_pSubMarketArray[n]->GetName());
			packet->addsize((int)strlen(packet->getfreebuf()));
		}
	}
	pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 2, 0, 0, (LPVOID)packet->getbuf(), packet->getsize());
	if (this->m_nSubMarketCount > 0 && m_pSubMarketArray[0])
		m_pSubMarketArray[0]->QueryItems(pPlayer, GetId());
}