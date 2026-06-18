#include "..\header\xPacket.h"

thread_local std::array<xPacket*, xPacketPool::POOL_SIZE> xPacketPool::tl_FreeList = {};
thread_local int xPacketPool::tl_FreeCount = 0;

xPacket* xPacketPool::Alloc(int size)
{
	if (tl_FreeCount > 0)
	{
		xPacket* pkt = tl_FreeList[--tl_FreeCount];
		if (pkt->notcreated() || pkt->getmaxsize() < size)
			pkt->create(size);
		return pkt;
	}
	return new xPacket(size);
}

xPacket* xPacketPool::Alloc(char* pbuf, int size)
{
	if (tl_FreeCount > 0)
	{
		xPacket* pkt = tl_FreeList[--tl_FreeCount];
		pkt->create(pbuf, size);
		return pkt;
	}
	return new xPacket(pbuf, size);
}

VOID xPacketPool::Free(xPacket* pkt)
{
	if (!pkt) return;
	pkt->clear();
	if (tl_FreeCount < POOL_SIZE)
		tl_FreeList[tl_FreeCount++] = pkt;
	else
		delete pkt;
}
