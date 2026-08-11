#include "stdafx.h"
#include "wpf_archive.h"
#include <fileapi.h>
#include <handleapi.h>
#include <stdio.h>
#include <queue>

// wpf_archive模块独立日志(不依赖服务器PRINT/DPRINT宏)
#ifdef _DEBUG
#define WPF_LOG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define WPF_LOG(fmt, ...) ((void)0)
#endif

// 静态断言: 确保结构体大小与客户端一致
static_assert(sizeof(WpfHeader) == 64, "WpfHeader must be 64 bytes");
static_assert(sizeof(WpfFcb1)   == 16, "WpfFcb1 must be 16 bytes");
static_assert(sizeof(WpfFcb2)   == 56, "WpfFcb2 must be 56 bytes");

// 空字符串常量(用于GetFilePath返回引用)
const std::string WpfArchive::s_emptyPath;

WpfArchive::WpfArchive()
	: m_fileHandle(INVALID_HANDLE_VALUE)
{
}

WpfArchive::~WpfArchive()
{
	Close();
}

bool WpfArchive::Open(const char* wpfPath)
{
	if (!wpfPath || !*wpfPath)
		return false;

	Close();

	m_wpfPath = wpfPath;

	m_fileHandle = CreateFileA(
		wpfPath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr
	);

	if (m_fileHandle == INVALID_HANDLE_VALUE)
	{
		WPF_LOG("[WpfArchive] 无法打开文件: %s (错误码=%lu)\n", wpfPath, GetLastError());
		return false;
	}

	if (!ReadHeader())
	{
		WPF_LOG("[WpfArchive] 文件头无效: %s\n", wpfPath);
		Close();
		return false;
	}

	if (!LoadFcbTable())
	{
		WPF_LOG("[WpfArchive] 加载FCB表失败: %s\n", wpfPath);
		Close();
		return false;
	}

	WPF_LOG("[WpfArchive] 已加载: %s (文件数=%u, 目录数=%u, 块大小=%u, 路径映射=%zu)\n",
		wpfPath, m_header.dwFileCount, m_header.dwDirCount, m_header.wBytesPerBlock, m_hashToPath.size());

	// 自动检测并校验配套的.wpf.hash文件
	std::string hashPath = m_wpfPath + ".hash";
	if (GetFileAttributesA(hashPath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		if (!ValidateWithHashFile(hashPath.c_str()))
		{
			WPF_LOG("[WpfArchive] hash校验不匹配: %s (将继续使用wpf数据)\n", hashPath.c_str());
		}
	}

	return true;
}

void WpfArchive::Close()
{
	m_fileMap.clear();
	m_hashToPath.clear();
	m_header.Clear();

	if (m_fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_fileHandle);
		m_fileHandle = INVALID_HANDLE_VALUE;
	}

	m_wpfPath.clear();
}

bool WpfArchive::ReadHeader()
{
	DWORD bytesRead = 0;
	if (!ReadFile(m_fileHandle, &m_header, sizeof(WpfHeader), &bytesRead, nullptr)
		|| bytesRead != sizeof(WpfHeader))
	{
		return false;
	}

	if (m_header.dwMagic != WPF_MAGIC_VALUE)
	{
		WPF_LOG("[WpfArchive] 魔数不匹配: 期望=0x%08X 实际=0x%08X\n",
			WPF_MAGIC_VALUE, m_header.dwMagic);
		return false;
	}

	if (m_header.wBytesPerBlock == 0
		|| m_header.wHeaderSize == 0
		|| m_header.iFatPos == 0)
	{
		return false;
	}

	return true;
}

bool WpfArchive::LoadFcbTable()
{
	uint32_t dwCount = m_header.dwDirCount + m_header.dwFileCount;
	if (dwCount == 0)
		return true;

	LARGE_INTEGER liPos;
	liPos.QuadPart = m_header.iFatPos;
	if (!SetFilePointerEx(m_fileHandle, liPos, nullptr, FILE_BEGIN))
		return false;

	// 一次性读取FCB1 + FCB2
	// FCB1: dwCount * 16B, FCB2紧跟其后: dwCount * 56B
	size_t fcb1Size = static_cast<size_t>(dwCount) * sizeof(WpfFcb1);
	size_t fcb2Size = static_cast<size_t>(dwCount) * sizeof(WpfFcb2);
	size_t totalFcbSize = fcb1Size + fcb2Size;

	std::vector<uint8_t> fcbBuf(totalFcbSize);
	DWORD bytesRead = 0;
	if (!ReadFile(m_fileHandle, fcbBuf.data(), static_cast<DWORD>(totalFcbSize), &bytesRead, nullptr)
		|| bytesRead != totalFcbSize)
	{
		WPF_LOG("[WpfArchive] 读取FCB表失败: 期望=%zu 实际=%lu\n", totalFcbSize, bytesRead);
		return false;
	}

	// 解析为FCB1和FCB2数组
	const WpfFcb1* fcb1List = reinterpret_cast<const WpfFcb1*>(fcbBuf.data());
	const WpfFcb2* fcb2List = reinterpret_cast<const WpfFcb2*>(fcbBuf.data() + fcb1Size);

	// 构建FileInfo映射(仅文件,不含目录)
	m_fileMap.reserve(m_header.dwFileCount);

	for (uint32_t i = 0; i < dwCount; i++)
	{
		const WpfFcb1& fcb1 = fcb1List[i];
		if (fcb1.iHashKey == 0)
			continue;

		// 文件数据偏移 = 文件头大小 + 起始块号 * 每块字节数
		// 目录项也加入映射(目录的dwStart指向子FCB索引,不指向数据,但客户端也可能用hash查目录)
		uint32_t offset = static_cast<uint32_t>(m_header.wHeaderSize)
			+ fcb1.dwStart * m_header.wBytesPerBlock;

		WpfFileEntry entry;
		entry.wpfPath = m_wpfPath;
		entry.offset = offset;
		entry.size = fcb1.dwSize;

		m_fileMap.emplace(static_cast<uint64_t>(fcb1.iHashKey), std::move(entry));
	}

	// 重建目录树 → hashToPath映射
	if (!RebuildDirectoryTree(
			std::vector<WpfFcb1>(fcb1List, fcb1List + dwCount),
			std::vector<WpfFcb2>(fcb2List, fcb2List + dwCount)))
	{
		WPF_LOG("[WpfArchive] 目录树重建失败\n");
		// 不致命,继续(仅影响文件路径获取)
	}

	return true;
}

// 目录树重建: 从FCB1/FCB2数组中用BFS遍历目录结构,为每个文件生成完整虚拟路径
// FCB布局: FCB[0]=根目录,其dwStart指向子项起始索引,dwSize=子项数量
//   子项占据连续范围[dwStart, dwStart+dwSize),其中:
//     EFA_DIR  → 子目录(递归处理)
//     EFA_FILE → 文件(记录hash→路径)
bool WpfArchive::RebuildDirectoryTree(const std::vector<WpfFcb1>& fcb1List, const std::vector<WpfFcb2>& fcb2List)
{
	uint32_t dwCount = static_cast<uint32_t>(fcb1List.size());
	if (dwCount == 0)
		return true;

	m_hashToPath.reserve(m_header.dwFileCount);

	// BFS队列: {FCB索引, 父目录路径}
	// 根目录索引0,父路径为空
	struct DirItem { uint32_t fcbIndex; std::string parentPath; };
	std::queue<DirItem> dirQueue;
	dirQueue.push({ 0, std::string() });

	while (!dirQueue.empty())
	{
		DirItem dir = dirQueue.front();
		dirQueue.pop();

		const WpfFcb1& dirFcb1 = fcb1List[dir.fcbIndex];

		// 子项起始索引和数量
		uint32_t childStart = dirFcb1.dwStart;
		uint32_t childCount = dirFcb1.dwSize;

		// 边界检查
		if (childStart >= dwCount || childStart + childCount > dwCount)
			continue;

		for (uint32_t i = childStart; i < childStart + childCount; i++)
		{
			const WpfFcb1& childFcb1 = fcb1List[i];
			const WpfFcb2& childFcb2 = fcb2List[i];

			// 构建当前项的完整路径
			std::string fullPath;
			if (dir.parentPath.empty())
				fullPath = childFcb2.strName;
			else
				fullPath = dir.parentPath + "\\" + childFcb2.strName;

			if (childFcb2.dwAttribute & EFA_DIR)
			{
				// 子目录: 加入BFS队列
				dirQueue.push({ i, fullPath });
			}
			else if (childFcb2.dwAttribute & EFA_FILE)
			{
				// 文件: 记录hash→路径映射
				if (childFcb1.iHashKey != 0)
				{
					m_hashToPath.emplace(
						static_cast<uint64_t>(childFcb1.iHashKey),
						fullPath
					);
				}
			}
			// 忽略其他属性(空白项等)
		}
	}

	return m_hashToPath.size() > 0;
}

bool WpfArchive::ValidateWithHashFile(const char* hashFilePath)
{
	std::vector<uint64_t> hashes;
	if (!LoadWpfHashFile(hashFilePath, hashes))
		return false;

	if (hashes.size() != static_cast<size_t>(m_header.dwFileCount))
	{
		WPF_LOG("[WpfArchive] hash文件数量(%zu)与wpf文件数(%u)不一致: %s\n",
			hashes.size(), m_header.dwFileCount, hashFilePath);
	}

	uint32_t missingCount = 0;
	for (uint64_t h : hashes)
	{
		if (m_fileMap.find(h) == m_fileMap.end())
		{
			missingCount++;
			if (missingCount <= 5)
				WPF_LOG("[WpfArchive] hash文件中存在但wpf中未找到: %016llX\n", h);
		}
	}

	if (missingCount > 0)
	{
		WPF_LOG("[WpfArchive] hash校验: %u个文件在wpf中未找到 (共%zu项)\n",
			missingCount, hashes.size());
	}
	else
	{
		WPF_LOG("[WpfArchive] hash文件校验通过: %s (%zu项)\n",
			hashFilePath, hashes.size());
	}

	return missingCount == 0;
}

const WpfFileEntry* WpfArchive::FindByHash(uint64_t hash) const
{
	auto it = m_fileMap.find(hash);
	if (it != m_fileMap.end())
		return &it->second;
	return nullptr;
}

const std::string& WpfArchive::GetFilePath(uint64_t hash) const
{
	auto it = m_hashToPath.find(hash);
	if (it != m_hashToPath.end())
		return it->second;
	return s_emptyPath;
}

bool WpfArchive::ReadByHash(uint64_t hash, std::vector<uint8_t>& outBuf)
{
	const WpfFileEntry* entry = FindByHash(hash);
	if (!entry)
		return false;

	outBuf.resize(entry->size);

	LARGE_INTEGER liPos;
	liPos.QuadPart = entry->offset;
	if (!SetFilePointerEx(m_fileHandle, liPos, nullptr, FILE_BEGIN))
		return false;

	DWORD bytesRead = 0;
	if (!ReadFile(m_fileHandle, outBuf.data(), entry->size, &bytesRead, nullptr)
		|| bytesRead != entry->size)
	{
		outBuf.clear();
		return false;
	}

	return true;
}

bool WpfArchive::ReadByHash(uint64_t hash, uint32_t startPos, uint32_t length, std::vector<uint8_t>& outBuf)
{
	const WpfFileEntry* entry = FindByHash(hash);
	if (!entry)
		return false;

	if (startPos >= entry->size)
		return false;

	uint32_t actualLen = length;
	if (startPos + length > entry->size)
		actualLen = entry->size - startPos;

	outBuf.resize(actualLen);

	LARGE_INTEGER liPos;
	liPos.QuadPart = static_cast<__int64>(entry->offset) + startPos;
	if (!SetFilePointerEx(m_fileHandle, liPos, nullptr, FILE_BEGIN))
		return false;

	DWORD bytesRead = 0;
	if (!ReadFile(m_fileHandle, outBuf.data(), actualLen, &bytesRead, nullptr)
		|| bytesRead != actualLen)
	{
		outBuf.clear();
		return false;
	}

	return true;
}
