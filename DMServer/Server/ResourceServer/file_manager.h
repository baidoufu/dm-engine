#pragma once
#include "protocol.h"
#include "wpf_archive.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <list>
#include <cstdint>
#include <windows.h>
#include <memory>

class FileManager : public xSingletonClass<FileManager>
{
public:
    FileManager();
    ~FileManager();

    // 根据Hash查找文件
    FileInfo* FindFileByHash(uint64_t hash);
    // 按hash读取文件(统一入口)
    bool ReadFileByHash(uint64_t hash, uint32_t startPos, uint32_t length, std::vector<uint8_t>& buffer);
    // 文件路径Hash计算
    uint64_t PathHash(const std::string& path);

    // ===== wpf模式接口 =====
    // 加载指定目录下的所有wpf文件(Data*.wpf, Texture*.wpf等)及配套hash文件
    // wpfDir: 包含wpf文件的目录路径
    // 返回加载的wpf文件数量
    int LoadWpfFiles(const std::string& wpfDir);
    // 判断hash是否存在于wpf中
    bool IsHashInWpf(uint64_t hash) const;
    // 获取wpf中的文件大小(仅在wpf中存在时有效)
    uint32_t GetWpfFileSize(uint64_t hash) const;
    // 获取文件虚拟路径(从wpf内部目录树)
    std::string GetFilePathByHash(uint64_t hash) const;
private:
    // 初始化加密表
    VOID InitCryptoTable();
    // Hash核心计算
    uint32_t HashString(const std::string& path, int hashType) const;
private:
    uint32_t m_CryptoTable[0x300];  // 仅使用前3列(0-255, 256-511, 512-767)

	// 文件数据缓存：缓存小文件(<=MAX_CACHE_SIZE)到内存，避免重复磁盘IO
	static constexpr uint32_t MAX_CACHE_FILE_SIZE = 64 * 1024;       // 单个文件缓存阈值: 64KB
	static constexpr size_t   MAX_CACHE_TOTAL_BYTES = 128ULL * 1024 * 1024; // 缓存总上限: 128MB
	// LRU缓存: list维护访问顺序(前端=最近使用), map存数据+list迭代器
	mutable std::list<uint64_t> m_cacheLru;
	mutable std::unordered_map<uint64_t, std::pair<std::vector<uint8_t>, std::list<uint64_t>::iterator>> m_fileCache;
	mutable std::shared_mutex m_cacheMutex;
	size_t m_cacheTotalBytes = 0;
	
	// 缓存查询：命中返回数据指针，未命中返回nullptr
	const std::vector<uint8_t>* FindInCache(uint64_t hash);
	// 缓存写入：若文件大小未超阈值则写入缓存, 缓存满时按LRU淘汰最久未访问项
	void InsertCache(uint64_t hash, const std::vector<uint8_t>& data);

	// ===== wpf模式成员 =====
	// wpf归档实例列表(每个wpf文件一个)
	std::vector<std::unique_ptr<WpfArchive>> m_wpfArchives;
	// 全局hash→WpfFileEntry映射(指针指向各WpfArchive内部的entry)
	std::unordered_map<uint64_t, const WpfFileEntry*> m_wpfGlobalMap;
	// wpf文件的合成FileInfo缓存(用于FindFileByHash返回有效指针)
	std::unordered_map<uint64_t, FileInfo> m_wpfFileInfoMap;
	// 查找wpf中的文件条目
	const WpfFileEntry* FindInWpf(uint64_t hash) const;
};

// 判断文件夹是否存在
static bool DirectoryExists(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}
