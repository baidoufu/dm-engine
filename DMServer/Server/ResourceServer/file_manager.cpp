#include "stdafx.h"
#include "file_manager.h"
#include <cstring>
#include <cstdio>
#include <fileapi.h>

FileManager::FileManager()
{
	InitCryptoTable();
}

FileManager::~FileManager() {}

FileInfo* FileManager::FindFileByHash(uint64_t hash)
{
	auto wpfIt = m_wpfGlobalMap.find(hash);
	if (wpfIt == m_wpfGlobalMap.end())
		return nullptr;

	// 从缓存返回(若不存在则按需创建)
	auto infoIt = m_wpfFileInfoMap.find(hash);
	if (infoIt != m_wpfFileInfoMap.end())
		return &infoIt->second;

	// 惰性创建FileInfo
	FileInfo info{};
	info.hash = hash;
	info.size = wpfIt->second->size;
	info.filePath = GetFilePathByHash(hash);

	auto result = m_wpfFileInfoMap.emplace(hash, std::move(info));
	return &result.first->second;
}

const std::vector<uint8_t>* FileManager::FindInCache(uint64_t hash)
{
	std::shared_lock<std::shared_mutex> lock(m_cacheMutex);
	auto it = m_fileCache.find(hash);
	if (it == m_fileCache.end()) return nullptr;
	m_cacheLru.splice(m_cacheLru.begin(), m_cacheLru, it->second.second);
	return &it->second.first;
}

// ===== wpf模式 =====

int FileManager::LoadWpfFiles(const std::string& wpfDir)
{
	std::string dir = wpfDir;
	if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
		dir += '\\';

	int loadedCount = 0;

	// 按客户端优先级顺序加载: patch优先覆盖
	struct WpfPattern { const char* pattern; };
	WpfPattern patterns[] = {
		{ "TexturePatch*.wpf" },
		{ "Texture*.wpf" },
		{ "DataPatch*.wpf" },
		{ "Data*.wpf" },
	};

	for (const auto& pat : patterns)
	{
		std::string searchPath = dir + pat.pattern;
		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE)
			continue;

		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			std::string fileName = findData.cFileName;
			if (fileName.length() > 5
				&& fileName.compare(fileName.length() - 5, 5, ".hash") == 0)
				continue;

			std::string fullPath = dir + fileName;
			auto archive = std::make_unique<WpfArchive>();
			if (!archive->Open(fullPath.c_str()))
			{
				fprintf(stderr, "[LoadWpfFiles] 跳过无效wpf: %s\n", fullPath.c_str());
				continue;
			}

			const auto& fileMap = archive->GetFileMap();
			for (const auto& pair : fileMap)
				m_wpfGlobalMap[pair.first] = &pair.second;

			fprintf(stderr, "[LoadWpfFiles] 已加载: %s (文件数=%zu, 全局映射=%zu)\n",
				fullPath.c_str(), fileMap.size(), m_wpfGlobalMap.size());

			m_wpfArchives.push_back(std::move(archive));
			loadedCount++;

		} while (FindNextFileA(hFind, &findData));

		FindClose(hFind);
	}

	return loadedCount;
}

bool FileManager::IsHashInWpf(uint64_t hash) const
{
	return m_wpfGlobalMap.find(hash) != m_wpfGlobalMap.end();
}

uint32_t FileManager::GetWpfFileSize(uint64_t hash) const
{
	auto it = m_wpfGlobalMap.find(hash);
	if (it != m_wpfGlobalMap.end())
		return it->second->size;
	return 0;
}

std::string FileManager::GetFilePathByHash(uint64_t hash) const
{
	for (const auto& archive : m_wpfArchives)
	{
		const std::string& path = archive->GetFilePath(hash);
		if (!path.empty())
			return path;
	}
	return std::string();
}

const WpfFileEntry* FileManager::FindInWpf(uint64_t hash) const
{
	auto it = m_wpfGlobalMap.find(hash);
	if (it != m_wpfGlobalMap.end())
		return it->second;
	return nullptr;
}

bool FileManager::ReadFileByHash(uint64_t hash, uint32_t startPos, uint32_t length, std::vector<uint8_t>& buffer)
{
	const WpfFileEntry* wpfEntry = FindInWpf(hash);
	if (!wpfEntry)
		return false;

	for (auto& archive : m_wpfArchives)
	{
		if (archive->GetWpfPath() == wpfEntry->wpfPath)
			return archive->ReadByHash(hash, startPos, length, buffer);
	}
	return false;
}

void FileManager::InsertCache(uint64_t hash, const std::vector<uint8_t>& data)
{
	std::unique_lock<std::shared_mutex> lock(m_cacheMutex);
	auto it = m_fileCache.find(hash);
	if (it != m_fileCache.end())
	{
		m_cacheTotalBytes -= it->second.first.size();
		it->second.first = data;
		m_cacheTotalBytes += data.size();
		m_cacheLru.splice(m_cacheLru.begin(), m_cacheLru, it->second.second);
		return;
	}
	while (m_cacheTotalBytes + data.size() > MAX_CACHE_TOTAL_BYTES && !m_cacheLru.empty())
	{
		uint64_t oldKey = m_cacheLru.back();
		auto oldIt = m_fileCache.find(oldKey);
		if (oldIt != m_fileCache.end())
		{
			m_cacheTotalBytes -= oldIt->second.first.size();
			m_fileCache.erase(oldIt);
		}
		m_cacheLru.pop_back();
	}
	if (m_cacheTotalBytes + data.size() > MAX_CACHE_TOTAL_BYTES)
		return;
	m_cacheLru.push_front(hash);
	m_fileCache.emplace(hash, std::make_pair(data, m_cacheLru.begin()));
	m_cacheTotalBytes += data.size();
}

VOID FileManager::InitCryptoTable()
{
	uint32_t seed = 0x00100001;
	for (int index1 = 0; index1 < 0x100; index1++)
	{
		for (int index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
		{
			seed = (seed * 125 + 3) % 0x2AAAAB;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			if (i < 3)
				m_CryptoTable[index2] = seed & 0xFFFF;
		}
	}
}

uint32_t FileManager::HashString(const std::string& path, int hashType) const
{
	uint32_t seed1 = 0x7FED7FED;
	uint32_t seed2 = 0xEEEEEEEE;
	for (size_t i = 0; i < path.length(); i++)
	{
		char c = toupper(static_cast<unsigned char>(path[i]));
		seed1 = (seed1 + seed2) ^ m_CryptoTable[static_cast<uint8_t>(c) + static_cast<uint32_t>(hashType) * 256];
		seed2 = seed2 * 0x21 + c + 3 + seed1;
	}
	return seed1;
}

uint64_t FileManager::PathHash(const std::string& path)
{
	uint64_t a = static_cast<uint64_t>(HashString(path, 1));
	uint64_t b = static_cast<uint64_t>(HashString(path, 2));
	return (a << 32) | b;
}
