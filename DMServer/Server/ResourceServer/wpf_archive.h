#pragma once
// WpfArchive: wpf归档文件轻量只读解析器
// 不依赖客户端CWpf类,仅解析wpf二进制格式实现按hash读取文件数据
// wpf内部结构(类FAT32):
//   [S_WpfHeader 文件头 64B] [文件头备份 64B] [文件数据区...] [空闲块描述] [FCB表=FCB1数组+FCB2数组]
// 文件定位公式: offset = wHeaderSize + dwStart * wBytesPerBlock

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <windows.h>

#pragma pack(push, 4)

// wpf文件头结构(64字节) -- 与客户端WpfInterface.h中S_WpfHeader完全一致
struct WpfHeader
{
	WpfHeader() { Clear(); }

	void Clear()
	{
		dwMagic = 0;
		wHeaderSize = 0;
		wBytesPerBlock = 0;
		iBlankBlockPos = 0;
		dwBlankBlockSize = 0;
		iFatPos = 0;
		dwDirCount = 0;
		dwFileCount = 0;
		dwTotalBlocks = 0;
		iWpfSize = 0;
		byRev1 = 0;
		byCrypt = 0;
		byCompress = 0;
		byRev2 = 0;
		dwWpfType = 0;
		dwRev = 0;
		memset(byRev3, 0, sizeof(byRev3));
		bySavingHeader = 0;
	}

	DWORD     dwMagic;              // 魔数,固定为'WPF\x01'
	WORD      wHeaderSize;          // 头的大小,也是第一个文件或目录的起始位置
	WORD      wBytesPerBlock;       // 每个块的字节数

	__int64   iBlankBlockPos;       // 空闲块描述信息所在位置
	DWORD     dwBlankBlockSize;     // 有多少条空闲块信息
	__int64   iFatPos;              // 文件分配表(FCB)所在位置
	DWORD     dwDirCount;           // 目录总数
	DWORD     dwFileCount;          // 文件个数
	DWORD     dwTotalBlocks;        // 划分的总块数
	__int64   iWpfSize;             // 整个wpf文件内容大小(不含头/空闲块/分配表)
	BYTE      byRev1;               // 作废
	BYTE      byCrypt;              // 加密类型
	BYTE      byCompress;           // 压缩类型
	BYTE      byRev2;               // 保留
	DWORD     dwWpfType;            // wpf的类型标识
	DWORD     dwRev;                // 保留
	BYTE      byRev3[3];            // 保留
	BYTE      bySavingHeader;       // 是否正在保存文件头
};

// FCB第一部分结构(16字节) -- 与客户端WpfInterface.h中S_Fcb1完全一致
// 用于只读模式下的文件定位
struct WpfFcb1
{
	WpfFcb1()
		: dwStart(0)
		, dwSize(0)
		, iHashKey(0)
	{
	}

	DWORD   dwStart;        // 文件:起始块号  目录:子项起始FCB索引
	DWORD   dwSize;         // 文件:字节长度  目录:子项数量
	__int64 iHashKey;       // 文件全路径的hash值
};

// FCB第二部分结构(56字节) -- 与客户端WpfInterface.h中S_Fcb2完全一致
// 包含文件名和属性信息,用于重建目录树
struct WpfFcb2
{
	WpfFcb2()
	{
		memset(strName, 0, sizeof(strName));
		memset(strMd5, 0, sizeof(strMd5));
		dwAttribute = 0;
		dwRev = 0;
	}

	char   strName[32];      // 文件名+扩展名(目录则为目录名), GBK编码
	char   strMd5[16];       // 文件MD5码
	DWORD  dwAttribute;      // 属性 EFA_DIR=0x0001 EFA_FILE=0x0002 EFA_COMPRESS=0x0004
	DWORD  dwRev;            // 当有EFA_COMPRESS属性时表示压缩前大小
};

#pragma pack(pop)

// wpf魔数
#define WPF_MAGIC_VALUE  'WPF\x01'

// 文件属性标志(与客户端E_FCTATTR一致)
#define EFA_DIR     0x0001   // 目录
#define EFA_FILE    0x0002   // 文件

// .wpf.hash文件头结构(16字节) -- 与客户端Wpf.h中HashFileHeader完全一致
struct HashFileHeader
{
	HashFileHeader()
		: dwMagic(0)
		, dwVerson(0)
		, dwSize(0)
		, dwRev(0)
	{
	}

	DWORD dwMagic;     // 魔数固定为'hash'
	DWORD dwVerson;    // 版本号0x01
	DWORD dwSize;      // hash数量
	DWORD dwRev;       // 保留
};

// 加载.wpf.hash文件,返回文件内所有hash值列表
// hashFilePath: .wpf.hash文件的全路径
// outHashes: 输出的hash值列表
// 返回true表示加载成功
inline bool LoadWpfHashFile(const char* hashFilePath, std::vector<uint64_t>& outHashes)
{
	if (!hashFilePath || !*hashFilePath)
		return false;

	FILE* fp = nullptr;
	fopen_s(&fp, hashFilePath, "rb");
	if (!fp)
		return false;

	HashFileHeader header;
	if (fread(&header, sizeof(HashFileHeader), 1, fp) != 1)
	{
		fclose(fp);
		return false;
	}

	if (header.dwMagic != 'hash')
	{
		fclose(fp);
		return false;
	}

	outHashes.clear();
	outHashes.reserve(header.dwSize);

	// 批量读入,每次最多1000个
	constexpr int kBatchSize = 1000;
	__int64 batchBuf[kBatchSize];
	uint32_t remaining = header.dwSize;
	while (remaining > 0)
	{
		uint32_t toRead = remaining < kBatchSize ? remaining : kBatchSize;
		size_t itemsRead = fread(batchBuf, sizeof(__int64), toRead, fp);
		if (itemsRead == 0)
			break;
		for (size_t i = 0; i < itemsRead; i++)
			outHashes.push_back(static_cast<uint64_t>(batchBuf[i]));
		remaining -= static_cast<uint32_t>(itemsRead);
	}

	fclose(fp);
	return outHashes.size() == header.dwSize;
}

// 单个文件在wpf中的定位信息
struct WpfFileEntry
{
	std::string wpfPath;     // wpf文件全路径
	uint32_t    offset;      // 文件数据在wpf中的偏移
	uint32_t    size;        // 文件大小(字节)
};

// WpfArchive: 单个wpf文件的只读解析器
// 可同时提供: hash→数据(hash→offset+size) 和 hash→虚拟路径(通过FCB2目录树重建)
// 线程安全: 只读操作天然线程安全,多个客户端可并发读取同一wpf的不同文件
//           但同一WpfArchive实例的不同方法调用需外部加锁
class WpfArchive
{
public:
	WpfArchive();
	~WpfArchive();

	// 打开wpf文件并解析其全部元数据(header+FCB1+FCB2+目录树),自动检测配套.wpf.hash文件做校验
	// wpfPath: wpf文件全路径
	// 返回true表示成功
	bool Open(const char* wpfPath);

	// 使用配套的.wpf.hash文件校验wpf内容完整性
	bool ValidateWithHashFile(const char* hashFilePath);

	// 关闭wpf文件,释放资源
	void Close();

	// 是否已成功打开
	bool IsOpen() const { return m_fileHandle != INVALID_HANDLE_VALUE; }

	// 根据hash查找文件定位信息
	const WpfFileEntry* FindByHash(uint64_t hash) const;

	// 根据hash获取文件虚拟路径(如 "map\001\00001.map"),不存在返回空字符串
	const std::string& GetFilePath(uint64_t hash) const;

	// 根据hash读取文件完整内容
	bool ReadByHash(uint64_t hash, std::vector<uint8_t>& outBuf);

	// 根据hash读取文件的部分内容(支持断点续传)
	bool ReadByHash(uint64_t hash, uint32_t startPos, uint32_t length, std::vector<uint8_t>& outBuf);

	const std::string& GetWpfPath() const { return m_wpfPath; }
	uint32_t GetFileCount() const { return m_header.dwFileCount; }
	const std::unordered_map<uint64_t, WpfFileEntry>& GetFileMap() const { return m_fileMap; }
	const WpfHeader& GetHeader() const { return m_header; }

private:
	bool ReadHeader();
	bool LoadFcbTable();

	// 从FCB1/FCB2数组重建目录树,填充m_hashToPath映射
	// fcb1List: 已加载的FCB1数组(dwCount项)
	// fcb2List: 已加载的FCB2数组(dwCount项)
	bool RebuildDirectoryTree(const std::vector<WpfFcb1>& fcb1List, const std::vector<WpfFcb2>& fcb2List);

	std::string  m_wpfPath;
	HANDLE       m_fileHandle;
	WpfHeader    m_header;
	std::unordered_map<uint64_t, WpfFileEntry> m_fileMap;       // hash → (wpfPath,offset,size)
	std::unordered_map<uint64_t, std::string>  m_hashToPath;    // hash → 虚拟路径

	// 空字符串常量,用于GetFilePath未找到时返回引用
	static const std::string s_emptyPath;
};
