#include "stdafx.h"
#include "file_manager.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <fileapi.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

FileManager::FileManager()
{
    InitCryptoTable();
}

FileManager::~FileManager() {}

bool FileManager::Initialize(const std::string& rootPath)
{
    m_rootPath = rootPath;
    if (!m_rootPath.empty() && m_rootPath.back() != '/' && m_rootPath.back() != '\\') {
        m_rootPath += '/';
    }
    return DirectoryExists(m_rootPath);
}

FileInfo* FileManager::FindFileByHash(uint64_t hash)
{
    auto it = m_fileMap.find(hash);
    if (it != m_fileMap.end()) return &it->second;
    return nullptr;
}

VOID FileManager::ReserveFileMap(size_t count)
{
    m_fileMap.reserve(count);
}

bool FileManager::LoadFromIndex(const std::string& indexPath)
{
    HANDLE hFile = CreateFileA(indexPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0)
    {
        CloseHandle(hFile);
        return false;
    }

    HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMap) { CloseHandle(hFile); return false; }

    char* data = (char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, (SIZE_T)fileSize.QuadPart);
    if (!data) { CloseHandle(hMap); CloseHandle(hFile); return false; }
    // 解析文件头并校验
    FileIndexHeader* header = (FileIndexHeader*)data;
    if (header->magic != 0x46494458)
    {
        UnmapViewOfFile(data); CloseHandle(hMap); CloseHandle(hFile);
        return false;
    }
    // 校验文件大小是否匹配
    size_t expectedSize = static_cast<size_t>(sizeof(FileIndexHeader) + sizeof(FileIndexEntry) * header->count);
    if (fileSize.QuadPart < (LONGLONG)expectedSize)
    {
        UnmapViewOfFile(data); CloseHandle(hMap); CloseHandle(hFile);
        return false;
    }
    // 预分配
    ReserveFileMap((size_t)header->count);

    // 条目数组紧接在头部后面
    FileIndexEntry* entries = (FileIndexEntry*)(data + sizeof(FileIndexHeader));
    // 字符串池在条目数组后面
    char* stringPool = data + sizeof(FileIndexHeader) + sizeof(FileIndexEntry) * header->count;

    for (uint64_t i = 0; i < header->count; i++)
    {
        FileIndexEntry& entry = entries[i];
        FileInfo info{};
        info.hash = entry.hash;
        info.size = entry.size;
        info.filePath = std::string(stringPool + entry.pathOffset, entry.pathLen);
        m_fileMap.emplace(info.hash, std::move(info));
    }

    UnmapViewOfFile(data);
    CloseHandle(hMap);
    CloseHandle(hFile);
    //std::cout << "[加载文件] 从索引缓存中已加载 " << m_fileMap.size() << " 个文件" << std::endl;
    return true;
}

bool FileManager::SaveIndex(const std::string& indexPath, const std::vector<FileInfo>& infos)
{
    // 计算各部分大小
    size_t headerSize = sizeof(FileIndexHeader);
    size_t entriesSize = sizeof(FileIndexEntry) * infos.size();
    size_t stringPoolSize = 0;
    for (const auto& info : infos)
        stringPoolSize += info.filePath.length();
    size_t totalSize = headerSize + entriesSize + stringPoolSize;

    // 使用普通文件IO写入，比mmap更可靠
    FILE* fp = nullptr;
    fopen_s(&fp, indexPath.c_str(), "wb");
    if (!fp) return false;

    // 分配内存缓冲区
    std::vector<char> buffer;
    buffer.resize(totalSize);
    char* data = buffer.data();

    // 写入头部
    FileIndexHeader* header = (FileIndexHeader*)data;
    header->magic = 0x46494458; // 'FIDX'
    header->count = (uint64_t)infos.size();

    // 写入条目和字符串池
    FileIndexEntry* entries = (FileIndexEntry*)(data + headerSize);
    char* stringPool = data + headerSize + entriesSize;
    size_t poolOffset = 0;

    for (size_t i = 0; i < infos.size(); i++)
    {
        const FileInfo& info = infos[i];
        entries[i].hash = info.hash;
        entries[i].size = info.size;
        entries[i].pathOffset = (uint32_t)poolOffset;
        entries[i].pathLen = (uint16_t)info.filePath.length();

        memcpy(stringPool + poolOffset, info.filePath.c_str(), info.filePath.length());
        poolOffset += info.filePath.length();
    }

    // 一次性写入文件
    size_t written = fwrite(data, 1, totalSize, fp);
    fclose(fp);

    if (written != totalSize)
    {
        std::cerr << "[保存索引] 无法完整写入索引文件" << std::endl;
        return false;
    }

    std::cout << "[保存索引] 索引缓存已保存: " << infos.size() << " 个文件" << std::endl;
    return true;
}

bool FileManager::BuildAndSaveIndex(const std::string& plistPath, const std::string& indexPath)
{
    auto start = std::chrono::steady_clock::now();

    // 先走原来的mmap解析流程
    HANDLE hFile = CreateFileA(plistPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0)
    { CloseHandle(hFile); return false; }

    HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMap) { CloseHandle(hFile); return false; }

    char* data = (char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, (SIZE_T)fileSize.QuadPart);
    if (!data) { CloseHandle(hMap); CloseHandle(hFile); return false; }

    size_t estimatedLines = (size_t)(fileSize.QuadPart / 30) + 1024;
    ReserveFileMap(estimatedLines);

    std::vector<FileInfo> currentFiles;
    currentFiles.reserve(estimatedLines);

    char* p = data;
    char* end = data + fileSize.QuadPart;

    while (p < end)
    {
        if (*p == '\r' || *p == '\n' || *p == '#' || *p == ';')
        {
            while (p < end && *p != '\r' && *p != '\n') p++;
            while (p < end && (*p == '\r' || *p == '\n')) p++;
            continue;
        }

        char* lineStart = p;
        while (p < end && *p != '\r' && *p != '\n') p++;
        char* lineEnd = p;

        std::string pathName(lineStart, lineEnd - lineStart);
        size_t first = pathName.find_first_not_of(" \t");
        if (first == std::string::npos)
        {
            while (p < end && (*p == '\r' || *p == '\n')) p++;
            continue;
        }
        size_t last = pathName.find_last_not_of(" \t\r\n");
        pathName = pathName.substr(first, last - first + 1);

        if (!pathName.empty())
        {
            std::string fullPath = m_rootPath + pathName;
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA(fullPath.c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                FindClose(hFind);
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    FileInfo info{};
                    info.filePath = std::move(pathName);
                    info.hash = PathHash(info.filePath);
                    info.size = (uint32_t)(((uint64_t)findData.nFileSizeHigh << 32) | (uint64_t)findData.nFileSizeLow);

                    currentFiles.emplace_back(std::move(info));
                }
            }
        }
        while (p < end && (*p == '\r' || *p == '\n')) p++;
    }

    // 保存索引缓存（必须在 move 进 m_fileMap 之前执行！）
    if (!SaveIndex(indexPath, currentFiles))
    {
        std::cerr << "[保存索引] 无法保存索引缓存" << std::endl;
    }

    // 批量插入全局索引
    for (auto& info : currentFiles)
        m_fileMap.emplace(info.hash, std::move(info));

    UnmapViewOfFile(data);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return true;
}

bool FileManager::LoadFileList(const std::string& plistPath)
{
    if (plistPath.length() > 4 && plistPath.compare(plistPath.length() - 4, 4, ".txt") == 0)
        return false;
    if (LoadFromIndex(plistPath)) return true;
    std::cerr << "[加载索引缓存] 文件已损坏..." << std::endl;
    return false;
}

bool FileManager::MakeFileIndex(const std::string& plistPath)
{
    std::string indexPath = plistPath;
    if (indexPath.length() > 4 && indexPath.compare(indexPath.length() - 4, 4, ".txt") == 0)
        indexPath.replace(indexPath.length() - 4, 4, ".idx");
    if (FileExists(indexPath))
        return false;
    // 解析txt并生成索引缓存
    return BuildAndSaveIndex(plistPath, indexPath);
}

bool FileManager::ReadFileRange(const std::string& filePath, uint32_t startPos, uint32_t length, std::vector<uint8_t>& buffer) const
{
    std::string fullPath = m_rootPath + filePath;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) return false;
    file.seekg(startPos, std::ios::beg);
    buffer.resize(length);
    file.read(reinterpret_cast<char*>(buffer.data()), length);
    return file.good();
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

uint32_t FileManager::HashString(std::string& path, int hashType) const
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

uint64_t FileManager::PathHash(std::string& path)
{
    uint64_t a = static_cast<uint64_t>(HashString(path, 1));
    uint64_t b = static_cast<uint64_t>(HashString(path, 2));
    return (a << 32) | b;
}
