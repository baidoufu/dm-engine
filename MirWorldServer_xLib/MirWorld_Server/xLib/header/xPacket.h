#include "xinc.h"
#include <memory>
#include <array>

class xPacket
{
public:
	xPacket()
	{
		m_pBuf = nullptr;
		m_pExternalBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
	}
	xPacket(int nSize)
	{
		m_pBuf = nullptr;
		m_pExternalBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		create(nSize);
	}
	xPacket(char* pbuf, int nSize)
	{
		m_pBuf = nullptr;
		m_pExternalBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		create(pbuf, nSize);
	}
	virtual ~xPacket(VOID)
	{
		destroy();
	}
	BOOL create(char* pbuf, int nSize)
	{
		destroy();
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		m_pExternalBuf = pbuf;
		m_pBuf = pbuf;
		m_iMaxSize = nSize;
		return TRUE;
	}
	BOOL create(int nSize)
	{
		m_iSize = 0;
		// 复用已有缓冲区，避免重复分配
		if (m_bBuildInBuffer && m_pBuf != nullptr && m_iMaxSize >= nSize)
			return TRUE;
		destroy();
		if (nSize > 0)
		{
			m_bBuildInBuffer = TRUE;
			// 使用16字节对齐的内存分配, 提升缓存性能
			char* alignedBuf = (char*)_aligned_malloc(nSize, 16);
			if (alignedBuf)
			{
				// 使用自定义删除器管理对齐内存
				m_pBufAligned = std::unique_ptr<char, AlignedDeleter>(alignedBuf, AlignedDeleter{});
				m_pBuf = alignedBuf;
				m_iMaxSize = nSize;
				m_bAligned = TRUE;
			}
			else
			{
				// 向上对齐到4KB页边界，减少内存碎片
				int alignedSize = (nSize + 4095) & ~4095;
				m_pBufNormal = std::make_unique<char[]>(alignedSize);
				m_pBuf = m_pBufNormal.get();
				m_iMaxSize = alignedSize;
				m_bAligned = FALSE;
			}
		}
		return TRUE;
	}
	BOOL notcreated()const
	{
		return (m_pBuf == nullptr);
	}
	VOID destroy()
	{
		if (m_bBuildInBuffer)
		{
			// 根据分配方式释放内存
			if (m_bAligned)
				m_pBufAligned.reset();
			else
				m_pBufNormal.reset();
		}
		m_pExternalBuf = nullptr;
		m_pBuf = nullptr;
		m_iMaxSize = 0;
		m_iSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
	}
	BOOL push(LPVOID lpData, int iDatasize)
	{
		if (m_iMaxSize - m_iSize < iDatasize)return FALSE;
		memcpy(m_pBuf + m_iSize, lpData, iDatasize);
		m_iSize += iDatasize;
		return TRUE;
	}
	BOOL push(const char* pszString)
	{
		size_t len = strlen(pszString);
		return push((LPVOID)pszString, (int)len);
	}
	BOOL pop(LPVOID lpData, int iDatasize)
	{
		if (m_iSize < iDatasize)return FALSE;
		memcpy(lpData, m_pBuf, iDatasize);
		return free(iDatasize);
	}
	BOOL push(int nZero)
	{
		if (m_iMaxSize - m_iSize < nZero) return FALSE;
		memset(m_pBuf + m_iSize, 0, nZero);
		m_iSize += nZero;
		return TRUE;
	}
	BOOL peek(LPVOID lpData, int iDatasize)const
	{
		if (m_iSize < iDatasize)return FALSE;
		memcpy(lpData, m_pBuf, iDatasize);
		return TRUE;
	}
	BOOL free(int iDatasize)
	{
		if (m_iSize < iDatasize)return FALSE;
		m_iSize -= iDatasize;
		if (m_iSize != 0)
			memmove(m_pBuf, m_pBuf + iDatasize, m_iSize);
		return TRUE;
	}
	VOID clear(){ m_iSize = 0; }
	int	getsize()const { return m_iSize; }
	const char* getbuf()const { return m_pBuf; }
	const char* getfreebuf()const { return (m_pBuf + m_iSize); }
	int	getmaxsize()const { return m_iMaxSize; }
	int	getfreesize()const { return m_iMaxSize - m_iSize; }
	VOID setsize(int nSize) { m_iSize = nSize; }
	BOOL addsize(int nSize)
	{
		if (nSize > getfreesize())return FALSE;
		m_iSize += nSize;
		return TRUE;
	}
	// 禁用拷贝构造函数和拷贝赋值运算符
	xPacket(const xPacket&) = delete;
	xPacket& operator=(const xPacket&) = delete;
private:
	// 对齐内存的自定义删除器
	struct AlignedDeleter {
		void operator()(char* p) const { if (p) ::_aligned_free(p); }
	};
	std::unique_ptr<char[]> m_pBufNormal;       // 标准内存智能指针
	std::unique_ptr<char, AlignedDeleter> m_pBufAligned; // 对齐内存智能指针
	char* m_pExternalBuf;                        // 外部缓冲区(不管理生命周期)
	char* m_pBuf;                                // 当前工作缓冲区指针
	int	m_iSize;
	int	m_iMaxSize;
	BOOL m_bBuildInBuffer;
	BOOL m_bAligned;  // 标记是否使用对齐内存分配
};

//封包对象池
class xPacketPool
{
public:
	static constexpr int POOL_SIZE = 128; // 线程本地缓存
	static constexpr int DEFAULT_PACKET_SIZE = 8192;
	static xPacket* Alloc(int size = DEFAULT_PACKET_SIZE);
	static xPacket* Alloc(char* pbuf, int size = DEFAULT_PACKET_SIZE);
	static VOID Free(xPacket* pkt);
	// 必须在可能退出的工作线程(如DB查询线程)退出前调用
	static VOID DrainThreadLocal();
	// RAII 包装类
	class ScopedPacket
	{
		xPacket* m_pkt;
	public:
		ScopedPacket(char* pbuf, int size = DEFAULT_PACKET_SIZE) : m_pkt(Alloc(pbuf, size)) {}
		ScopedPacket(int size = DEFAULT_PACKET_SIZE) : m_pkt(Alloc(size)) {}
		~ScopedPacket() { if (m_pkt) Free(m_pkt); }
		xPacket* operator->() { return m_pkt; }
		xPacket& operator*() { return *m_pkt; }
		xPacket* get() const { return m_pkt; }
		xPacket* release() { xPacket* p = m_pkt; m_pkt = nullptr; return p; }
	};
private:
	static thread_local std::array<xPacket*, POOL_SIZE> tl_FreeList;
	static thread_local int tl_FreeCount;
};
