#pragma once
#include "protocol.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <mutex>
#include <cstdint>
#include <windows.h>

class FileManager : public xSingletonClass<FileManager>
{
public:
    FileManager();
    ~FileManager();

    // 初始化文件管理器
    bool Initialize(const std::string& rootPath);
    // 根据Hash查找文件
    FileInfo* FindFileByHash(uint64_t hash);
    // 加载文件列表（仅加载二进制索引缓存）
    bool LoadFileList(const std::string& plistPath);
    // 生成索引缓存
    bool MakeFileIndex(const std::string& plistPath);
    // 读取文件片段 (断点续传)
    bool ReadFileRange(const std::string& filePath, uint32_t startPos, uint32_t length, std::vector<uint8_t>& buffer) const;
    // 文件路径Hash计算
    uint64_t PathHash(std::string& path);
private:
    // 初始化加密表
    VOID InitCryptoTable();
    // Hash核心计算
    uint32_t HashString(std::string& path, int hashType) const;
    // 预分配容量，减少 rehash
    VOID ReserveFileMap(size_t count);
    // 从二进制索引文件加载（mmap方式）
    bool LoadFromIndex(const std::string& indexPath);
    // 从List.txt解析并生成二进制索引缓存
    bool BuildAndSaveIndex(const std::string& plistPath, const std::string& indexPath);
    // 保存文件列表到二进制索引
    bool SaveIndex(const std::string& indexPath, const std::vector<FileInfo>& infos);
private:
    std::string m_rootPath;
    std::unordered_map<uint64_t, FileInfo> m_fileMap;
    uint32_t m_CryptoTable[0x300];  // 仅使用前3列(0-255, 256-511, 512-767)
};

// 判断文件夹是否存在
static bool DirectoryExists(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}
// 判断文件是否存在
static bool FileExists(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}
// 通过路径找到资源并获取文件大小
static uint64_t GetFileSizeLocal(const std::string& path) {
    WIN32_FILE_ATTRIBUTE_DATA fileInfo{};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileInfo)) {
        return 0;
    }
    return ((uint64_t)fileInfo.nFileSizeHigh << 32) | fileInfo.nFileSizeLow;
}
