# ResourceServer 项目代码 Wiki

## 目录

1. [项目概述](#1-项目概述)
2. [项目整体架构](#2-项目整体架构)
3. [目录结构](#3-目录结构)
4. [依赖关系](#4-依赖关系)
5. [编译与运行](#5-编译与运行)
6. [核心模块详解](#6-核心模块详解)
7. [关键类与函数说明](#7-关键类与函数说明)
8. [协议规范](#8-协议规范)
9. [数据流与运行机制](#9-数据流与运行机制)
10. [性能优化设计](#10-性能优化设计)

---

## 1. 项目概述

**ResourceServer** 是游戏服务器集群中的**资源分发服务**，负责为游戏客户端提供游戏资源（如纹理、地图、音频、NPC 数据等）的下载服务。

### 核心职责

- 管理游戏资源文件的元数据（路径、大小、Hash）
- 响应客户端的资源下载请求
- 支持断点续传
- 通过二进制索引缓存加速资源查找
- 心跳保活与连接管理

### 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++ |
| 平台 | Windows (Win32) |
| 构建工具 | Visual Studio (MSBuild, v143 工具集) |
| 网络模型 | IOCP（重叠 IO 完成端口） |
| 内存管理 | mimalloc |
| 字符编码 | GBK (.936) |

---

## 2. 项目整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                         进程入口 (main.cpp)                      │
│                        CServerForm 窗口控制台                     │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    CResourceServer (单例)                        │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ 继承: CBaseServer + xIndexObjectPool<CResourceClient>     │  │
│  │ 职责: 服务器生命周期管理、配置加载、对象池管理              │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────┬───────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌───────────────────────┐      ┌───────────────────────────────┐
│  CResourceClient × N  │      │     FileManager (单例)        │
│  ┌─────────────────┐  │      │  ┌─────────────────────────┐  │
│  │ 心跳超时检测    │  │      │  │ 资源元数据管理           │  │
│  │ 协议解析分发    │  │      │  │ .txt → .idx 缓存管理     │  │
│  │ 文件分块发送    │  │      │  │ mmap 零拷贝加载索引      │  │
│  └─────────────────┘  │      │  │ 文件 Hash 查找           │  │
└───────────────────────┘      │  │ 断点续传读取             │  │
                               │  │ MPQ 风格加密 Hash 算法   │  │
                               │  └─────────────────────────┘  │
                               └───────────────────────────────┘
```

---

## 3. 目录结构

```
ResourceServer/
├── main.cpp                  # 程序入口，初始化服务器和消息循环
├── resource_server.h/cpp     # 资源服务器核心类
├── resource_client.h/cpp     # 客户端连接处理类
├── file_manager.h/cpp        # 文件管理与索引缓存系统
├── protocol.h                # 协议结构体与枚举定义
├── stdafx.h/cpp              # 预编译头
├── ResourceServer.vcxproj    # Visual Studio 项目文件
├── ResourceServer.vcxproj.filters
└── ResourceServer.vcxproj.user
```

---

## 4. 依赖关系

### 4.1 内部依赖

| 依赖项 | 说明 |
|--------|------|
| `public` 库 | 提供 CBaseServer、CClientObject、CLogFile、CServerForm 等基础服务器设施 |
| `xLib` 库 | 提供 xSingletonClass、xIndexObjectPool、xPacket、CServerTimer 等通用工具 |
| `mimalloc` | 高性能内存分配器 |

### 4.2 外部依赖

| 依赖项 | 用途 |
|--------|------|
| WinSock2 | 网络通信（IOCP） |
| Windows API | 文件操作、内存映射（CreateFileMapping / MapViewOfFile） |
| shlwapi.lib | 路径处理工具 |

### 4.3 类继承关系

```
CResourceServer
├── xSingletonClass<CResourceServer>   // 单例模式
├── CBaseServer                        // 服务器基类（网络 IOCP、配置加载）
└── xIndexObjectPool<CResourceClient>  // 对象池管理客户端连接

CResourceClient
└── CClientObject                      // 客户端连接基类

FileManager
└── xSingletonClass<FileManager>       // 单例模式

FileInfo                               // 纯数据结构
```

### 4.4 模块调用关系

```
main.cpp
└── CResourceServer::GetInstance()
    ├── InitServer() → FileManager::Initialize()
    │                  FileManager::LoadFileList()
    │                  xIndexObjectPool::create()
    └── Update() → CResourceClient::Update() (遍历对象池)
                 └── CResourceClient::HandleDownloadRequest()
                     └── FileManager::FindFileByHash()
                     └── FileManager::ReadFileRange()
                     └── CResourceClient::SendFileToClient()
```

---

## 5. 编译与运行

### 5.1 环境要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows |
| IDE | Visual Studio 2022 (v143 工具集) |
| SDK | Windows SDK 10.0 |
| 架构 | Win32 (x86) |
| 运行时库 | 静态链接 (MT / MTd) |

### 5.2 编译配置

| 配置项 | Debug | Release |
|--------|-------|---------|
| 输出目录 | `../../../Work/Bin/` | `../../../Work/Bin/` |
| 优化 | 禁用 | 启用 |
| 调试信息 | EditAndContinue | ProgramDatabase |
| 预定义宏 | `WIN32;_DEBUG;_CONSOLE` | `WIN32;NDEBUG;_CONSOLE` |
| 运行时库 | `/MTd` | `/MT` |

### 5.3 编译步骤

```powershell
# 方法一：通过 Visual Studio IDE
# 打开 MirWorld_Server.sln，选择 ResourceServer 项目，按 Ctrl+Shift+B 编译

# 方法二：通过命令行 (Developer PowerShell)
cd "d:\DM Engine\MirWorldServer_xLib\MirWorld_Server"
msbuild MirWorld_Server.sln /p:Configuration=Release /p:Platform=Win32 /t:ResourceServer
```

### 5.4 运行方式

```powershell
# 编译产物路径
..\..\..\Work\Bin\ResourceServer.exe

# 运行
cd "d:\DM Engine\MirWorldServer_xLib\Work\Bin"
.\ResourceServer.exe

# 运行后程序会：
# 1. 初始化日志系统，日志输出到 ..\日志\资源服务器\
# 2. 从配置文件中读取 DataRootPath 和 FileListPath
# 3. 加载 .idx 索引缓存
# 4. 监听网络端口等待客户端连接
# 5. ResourceServer -make 生成索引缓存文件
```

### 5.5 配置文件

服务器通过 `CSettingFile` 读取配置，配置项位于 `[资源服务]` 段：

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `DataRootPath` | `.\Res\` | 资源文件根目录 |
| `FileListPath` | `.\List\` | 文件列表目录（.txt 文件所在） |
| `MaxConnection` | `1024` | 最大并发连接数 |

### 5.6 文件列表格式

`FileListPath` 目录下的 `.txt` 文件（如 `Data.txt`）格式：

```text
# 注释行以 # 或 ; 开头
# 每行为相对于 DataRootPath 的资源文件路径
Data/Texture/player.wil
Data/Map/map001.wil
Data/Audio/bgm.mp3
```

ResourceServer -make 来生成对应的 `.idx` 二进制索引缓存文件（如 `Data.idx`）。

---

## 6. 核心模块详解

### 6.1 进程入口模块 (main.cpp)

```cpp
INT main(INT argc, CHAR* argv[])
{
    setlocale(LC_ALL, ".936");                              // 设置 GBK 中文编码
    CLogFile::GetInstance()->Init("..\\日志\\资源服务器");   // 日志系统初始化
    g_Form.Create(SERVER_NAME, cmdLine);                    // 创建控制台窗口
    g_Form.SetArenaReserve(128 * 1024);                     // 预分配内存池 128KB
    g_pServer = CResourceServer::GetInstance();             // 获取服务器单例
    g_pServer->SetServerName(SERVER_NAME);                  // 设置服务器名称
    g_pServer->SetIoConsole(&g_Form);                       // 绑定控制台输出
    g_Form.SetInputListener(g_pServer);                     // 设置输入监听
    g_Form.SetServer(g_pServer);                            // 关联服务器对象
    g_Form.SetStatus(FALSE);                                // 初始状态为未就绪
    return g_Form.EnterMessageLoop();                       // 进入消息循环
}
```

**关键流程：**
1. 设置本地化编码为 GBK（代码页 936）
2. 初始化日志系统
3. 解析命令行参数
4. 创建服务器控制台窗口（CServerForm）
5. 预分配内存池（Arena 128KB）
6. 获取/创建 ResourceServer 单例
7. 进入 Windows 消息循环

### 6.2 服务器核心模块 (resource_server.h/cpp)

#### CResourceServer 类

```cpp
class CResourceServer : public xSingletonClass<CResourceServer>, 
                        public CBaseServer, 
                        public xIndexObjectPool<CResourceClient>
```

**多重继承说明：**

| 基类 | 作用 |
|------|------|
| `xSingletonClass<CResourceServer>` | 提供单例模式支持（`GetInstance()`） |
| `CBaseServer` | 提供 IOCP 网络通信、配置读取、服务器状态管理 |
| `xIndexObjectPool<CResourceClient>` | 提供基于索引的对象池，管理 CResourceClient 实例 |

**核心方法：**

| 方法 | 功能 |
|------|------|
| `InitServer(CSettingFile&)` | 初始化服务器：读取配置、创建对象池、初始化 FileManager、加载文件列表 |
| `Update()` | 每帧更新：遍历所有客户端对象调用 Update，调用基类 UpdateSCServer |
| `CleanServer()` | 清理服务器资源（当前为空实现） |
| `GetClientObject(UINT)` | 通过 ID 获取客户端对象 |
| `NewClientObject()` | 从对象池分配新客户端对象 |
| `DeleteClientObject(CClientObject*)` | 归还客户端对象到对象池 |

**成员变量：**

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_dataRootPath` | `std::string` | 资源数据根目录路径 |
| `m_fileListPath` | `std::string` | 文件列表目录路径 |

**调试宏：**

```cpp
#ifdef _DEBUG
#define DPRINT CResourceServer::GetInstance()->GetIoConsole()->OutPut  // 调试输出
#define DSYSMSG SaySystem                                               // 系统消息
#else
#define DPRINT empty_func                                              // 发布版空操作
#define DSYSMSG empty_func
#endif
#define PRINT CResourceServer::GetInstance()->GetIoConsole()->OutPut   // 始终输出
```

#### InitServer 详细流程

```
InitServer()
├── 读取配置: DataRootPath, FileListPath, MaxConnection
├── 创建对象池: create(maxconnection)
├── 初始化 FileManager: FileManager::Initialize(m_dataRootPath)
├── 扫描 FileListPath 目录下所有文件
└── 对每个文件调用 FileManager::LoadFileList()
    ├── 检查 .idx 缓存是否存在且有效
    ├── 有效 → LoadFromIndex() (mmap 零拷贝加载)
    └── 无效 → BuildAndSaveIndex() (解析 txt 并生成 .idx)
```

### 6.3 客户端处理模块 (resource_client.h/cpp)

#### CResourceClient 类

```cpp
class CResourceClient : public CClientObject
```

**职责：** 管理单个客户端连接的生命周期，处理协议解析和资源下发。

**核心方法：**

| 方法 | 功能 |
|------|------|
| `OnConnection()` | 连接建立时记录心跳时间 |
| `OnDisconnect()` | 连接断开时的清理（当前为空） |
| `Update()` | 心跳超时检测，超时则断开连接 |
| `OnDataPacket(xPacket*)` | 接收到数据包后的处理入口 |
| `HandleDownloadRequest()` | 解析下载请求协议 |
| `SendFileToClient()` | 读取文件并打包发送给客户端 |

**心跳机制：**

```cpp
constexpr auto HEARTBRATTIME = 30 * 1000;    // 心跳超时阈值: 30 秒
```

**心跳逻辑：**
- 连接建立时保存当前时间
- Update 循环中检查是否超过 30 秒未活动，超时则主动断开

**协议处理流程：**

```
OnDataPacket(xPacket*)
└── HandleDownloadRequest(buffer, len)
    ├── 解析 STDlNetMsgHeader (网络消息头)
    ├── 解析 SDLProtocalHeader (协议头)
    └── switch (wProtocal):
        ├── 0x1001 (请求下载)
        │   └── 遍历 wFileCount 个 SDlProtocalBody
        │       ├── FileManager::FindFileByHash(i64Hash)
        │       └── SendFileToClient(fileInfo, startPos, id, wpfIndex)
        ├── 0x1002 (心跳) → 空处理
        └── 0x1003 (确认收包) → 空处理
```

#### SendFileToClient 数据包构建

```
数据包结构:
┌─────────────────────┬───────────────────────┬───────────────┬────────────┐
│ STDlNetMsgHeader    │ SDLProtocalHeader     │ 路径长度(1B)  │ 文件路径   │
│ (6 bytes)           │ (40 bytes)            │               │ (变长)     │
└─────────────────────┴───────────────────────┴───────────────┴────────────┘
└──────────────────────────────────────────────┬───────────────────────────┘
                                               │ 文件数据 (变长)           │
                                               └───────────────────────────┘

网络字节序:
- wMsgID: ntohs(0x0BB9) = 0x0BB9 (3000)
- dwDataLen: htonl(dataLen)
```

### 6.4 文件管理模块 (file_manager.h/cpp)

#### FileManager 类

```cpp
class FileManager : public xSingletonClass<FileManager>
```

**职责：** 资源文件元数据管理、索引缓存系统、文件读取。

**核心方法：**

| 方法 | 功能 |
|------|------|
| `Initialize(rootPath)` | 初始化，验证资源根目录是否存在 |
| `FindFileByHash(hash)` | 通过 Hash 值查找文件信息（线程安全） |
| `LoadFileList(plistPath)` | 加载文件列表（自动选择 .idx 缓存） |
| `MakeFileIndex(plistPath)` | 生成索引缓存文件 |
| `ReadFileRange(filePath, startPos, length, buffer)` | 读取文件片段（支持断点续传） |
| `PathHash(path)` | 计算文件路径的 64 位 Hash 值 |

**成员变量：**

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_rootPath` | `std::string` | 资源根目录 |
| `m_fileMap` | `unordered_map<uint64_t, FileInfo>` | Hash → FileInfo 映射表 |
| `m_mutex` | `std::mutex` | 保护 fileMap 的互斥锁 |
| `m_CryptoTable[0x500]` | `uint32_t[]` | MPQ 风格加密 Hash 表 |

#### 索引缓存系统

```
┌─────────────────────────────────────────────────────────────────┐
│                    .idx 二进制索引文件格式                        │
├────────────────────┬────────────────────────────────────────────┤
│ FileIndexHeader    │ 魔数: 0x46494458 ('FIDX')                  │
│ (16 bytes)         │ 版本: 1                                    │
│                    │ 文件数量: uint32                           │
│                    │ 时间戳: uint64 (毫秒)                      │
├────────────────────┼────────────────────────────────────────────┤
│ FileIndexEntry[]   │ hash: uint64 (路径Hash)                    │
│ (24 bytes × count) │ size: uint32 (文件大小)                    │
│                    │ pathOffset: uint32 (路径字符串偏移)        │
│                    │ pathLen: uint32 (路径长度)                 │
├────────────────────┼────────────────────────────────────────────┤
│ String Pool        │ 所有文件路径字符串连续存储（无分隔符）      │
│ (变长)             │ 通过 pathOffset + pathLen 定位            │
└────────────────────┴────────────────────────────────────────────┘
```

**索引加载策略（三级缓存）：**

```
LoadFileList("Data.idx")
│
├── 加载索引缓存路径: LoadFromIndex()
│   ├── CreateFile + CreateFileMapping (mmap)
│   ├── 校验魔数 (0x46494458) 和版本 (1)
│   ├── 校验文件大小
│   ├── MapViewOfFile 零拷贝映射
│   └── 直接解析内存中的条目和字符串池
│
└── 
MakeFileIndex("Data.txt")
│
├── 构建索引缓存路径: BuildAndSaveIndex()
    ├── mmap 映射 .txt 文件
    ├── 逐行解析路径（跳过 # 和 ; 注释行）
    ├── 对每个路径: FindFirstFile 验证文件存在并获取大小
    ├── 计算路径 Hash (MPQ 风格算法)
    ├── 批量插入 m_fileMap
    └── SaveIndex() 生成 .idx 缓存文件
```

#### Hash 算法 (MPQ 风格)

```cpp
// 初始化加密表
InitCryptoTable():
    seed = 0x00100001
    生成 0x500 (1280) 个 16 位伪随机数

// 字符串 Hash 计算 (32 位)
HashString(path, hashType):
    seed1 = 0x7FED7FED
    seed2 = 0xEEEEEEEE
    for each CHAR c in path (大写):
        seed1 = (seed1 + seed2) ^ CryptoTable[toupper(c) + hashType * 256]
        seed2 = seed2 * 0x21 + c + 3 + seed1
    return seed1

// 64 位路径 Hash
PathHash(path):
    a = HashString(path, 1)  // 低 32 位
    b = HashString(path, 2)  // 高 32 位
    return (a << 32) | b
```

此算法源自暴雪 MPQ 文件系统的 Hash 算法，具有碰撞率低、计算快速的特点。

---

## 7. 关键类与函数说明

### 7.1 全局对象

| 对象 | 类型 | 作用 |
|------|------|------|
| `g_Form` | `CServerForm` | 服务器控制台窗口，处理消息循环和 IO |
| `g_pServer` | `CResourceServer*` | 全局服务器实例指针 |

### 7.2 协议相关宏

| 宏 | 值 | 含义 |
|----|-----|------|
| `SERVER_NAME` | `"资源服务"` | 服务器名称标识 |

### 7.3 协议号

| 协议号 | 名称 | 方向 | 说明 |
|--------|------|------|------|
| `0x1001` | 请求下载 | C→S / S→C | 客户端请求下载文件，服务器响应文件数据 |
| `0x1002` | 心跳 | C→S | 客户端发送心跳保活 |
| `0x1003` | 确认收包 | C→S | 客户端确认已接收数据包（用于流控） |

### 7.4 文件属性标志 (eFctattr)

| 标志 | 值 | 含义 |
|------|-----|------|
| `EFA_BLANK` | `0x0000` | 空白 |
| `EFA_DIR` | `0x0001` | 目录 |
| `EFA_FILE` | `0x0002` | 文件 |
| `EFA_COMPRESS` | `0x0004` | 压缩文件 |
| `EFA_CRYPT` | `0x0008` | 加密文件 |
| `EFA_MEARGE_UNCOMPRESS` | `0x0010` | 合并时需解压 |

### 7.5 下载优先级 (eReadPrior)

| 优先级 | 值 | 用途 |
|--------|-----|------|
| `EP_MOST_HIGH` | 0 | 最高优先级 |
| `EP_UI` | 1 | UI 资源 |
| `EP_TILES` | 2 | 大地表 |
| `EP_NPC` | 3 | NPC |
| `EP_MONSTER` | 4 | 怪物 |
| `EP_CHARACTER` | 5 | 角色 |
| `EP_SMTILES` | 6 | 小地表及贴地物体 |
| `EP_OBJECT` | 7 | 地图中的物体层 |
| `EP_SKILL` | 8 | 技能特效 |
| `EP_MAGIC` | 9 | 魔法特效 |
| `EP_AUDIO` | 10 | 音效 |
| `EP_NORMAL` | 11 | 一般 |
| `EP_DONT_DOWNLOAD` | 12 | 本地有则读，无则不下载 |
| `EP_TYPES` | 13 | 优先级种类数 |

### 7.6 工具函数

| 函数 | 说明 |
|------|------|
| `DirectoryExists(path)` | 判断目录是否存在（Windows API） |
| `FileExists(path)` | 判断文件是否存在（Windows API） |
| `GetFileSizeLocal(path)` | 获取文件大小（支持 > 4GB） |

---

## 8. 协议规范

### 8.1 网络层消息头 (STDlNetMsgHeader)

```cpp
struct STDlNetMsgHeader {
    uint16_t wMsgID;      // 消息ID: 0x0BB9 (3000), 网络字节序
    uint32_t dwDataLen;   // 后续协议数据长度, 网络字节序
};
```

**大小：** 6 字节

**说明：** 
- 所有网络数据使用网络字节序（大端）
- `ntohs()` 和 `htonl()` 用于字节序转换

### 8.2 协议头 (SDLProtocalHeader)

```cpp
struct SDLProtocalHeader {
    uint16_t wProtocal;     // 协议号: 0x1001/0x1002/0x1003
    uint16_t wFileCount;    // 请求/响应文件数量
    uint32_t dwID;          // 资源ID（纹理ID，数据文件为0）
    uint64_t i64Hash;       // 文件路径Hash
    uint32_t iWpf;          // 所属WPF包索引
    uint32_t dwAttribute;   // 文件属性标志
    uint32_t dwLen;         // 文件长度 / 已接收长度(0x1003)
    uint32_t dwStartPos;    // 起始位置(断点续传) / 确认起始位置(0x1003)
    uint32_t dwLastDWORD;   // 文件末尾4字节校验值
    uint32_t dwRev;         // 保留
};
```

**大小：** 40 字节

**字段多义性说明：**

| 协议号 | dwLen | dwStartPos | dwAttribute |
|--------|-------|------------|-------------|
| `0x1001` 下载 | 文件长度 | 发送起始位置 | 文件属性标志 |
| `0x1003` 确认 | 已接收长度 | 本次确认起始位置 | 是否成功接收 |

### 8.3 请求体 (SDlProtocalBody)

```cpp
struct SDlProtocalBody {
    uint32_t dwID;          // 纹理ID（图片资源用，数据文件为0）
    uint64_t i64Hash;       // 文件路径Hash
    uint32_t iWpf;          // 所属WPF包索引
    uint8_t  byPrior;       // 优先级 (eReadPrior)
    uint8_t  byRev[3];      // 保留
    uint32_t dwStartPos;    // 起始位置（断点续传）
    uint32_t dwTickCount;   // 请求时间戳
};
```

**大小：** 32 字节（1 字节对齐）

### 8.4 请求-响应流程

```
客户端                                      服务器
   │                                          │
   │  ┌─────────────────────────────────┐    │
   │  │ 0x1001 下载请求                  │    │
   │  │ STDlNetMsgHeader                │    │
   │  │   wMsgID = 0x0BB9               │    │
   │  │ SDLProtocalHeader               │    │
   │  │   wProtocal = 0x1001            │    │
   │  │   wFileCount = N                │    │
   │  │ SDlProtocalBody × N             │    │
   │  │   i64Hash, dwStartPos, ...      │    │
   │  └─────────────────────────────────┘    │
   │─────────────────────────────────────────>│
   │                                          │ 查找 Hash → FileInfo
   │                                          │ 读取文件内容
   │                                          │
   │  ┌─────────────────────────────────┐    │
   │  │ 0x1001 响应 (每个文件一个包)     │    │
   │  │ STDlNetMsgHeader                │    │
   │  │ SDLProtocalHeader               │    │
   │  │   wProtocal = 0x1001            │    │
   │  │   wFileCount = 1                │    │
   │  │ 路径长度 (1 byte)               │    │
   │  │ 文件路径 (变长)                  │    │
   │  │ 文件数据 (变长)                  │    │
   │  └─────────────────────────────────┘    │
   │<─────────────────────────────────────────│
   │                                          │
   │  ┌─────────────────────────────────┐    │
   │  │ 0x1003 确认收包                  │    │
   │  │ dwAttribute = 成功/失败          │    │
   │  │ dwLen = 已接收长度               │    │
   │  │ dwStartPos = 确认起始位置        │    │
   │  └─────────────────────────────────┘    │
   │─────────────────────────────────────────>│
   │                                          │
```

### 8.5 心跳流程

```
客户端                                      服务器
   │                                          │
   │  ┌─────────────────────────────────┐    │
   │  │ 0x1002 心跳                      │    │
   │  │ SDLProtocalHeader               │    │
   │  │   wProtocal = 0x1002            │    │
   │  └─────────────────────────────────┘    │
   │─────────────────────────────────────────>│
   │                                          │ 收到心跳，更新时间戳
   │                                          │ (当前未做特殊处理)
```

---

## 9. 数据流与运行机制

### 9.1 启动流程

```
main()
  │
  ├─ 设置 GBK 编码
  ├─ 初始化日志系统
  ├─ 创建 CServerForm 控制台窗口
  ├─ 预分配 Arena 内存池 (128KB)
  ├─ 获取 CResourceServer 单例
  ├─ 设置服务器名称、IO 控制台、输入监听
  ├─ CResourceServer::InitServer()
  │   ├─ 读取配置文件 → DataRootPath, FileListPath, MaxConnection
  │   ├─ 创建对象池 → create(maxconnection)
  │   ├─ FileManager::Initialize() → 验证资源目录
  │   └─ 扫描 List 目录 → 加载/重建 .idx 索引
  │       ├─ 索引有效 → mmap 零拷贝加载 (快速)
  │       └─ 索引无效 → 解析 txt → 生成 .idx (慢速)
  │
  └─ EnterMessageLoop() → Windows 消息循环
      │
      ├─ 网络事件 (IOCP 完成端口)
      │   ├─ 新连接 → NewClientObject() → OnConnection()
      │   ├─ 断开连接 → OnDisconnect() → DeleteClientObject()
      │   └─ 数据到达 → OnDataPacket()
      │
      └─ 定时器事件
          └─ Update() → 遍历客户端对象池
              └─ CResourceClient::Update() → 心跳检测
```

### 9.2 下载请求处理流程

```
收到数据包 (IOCP)
  │
  ▼
CResourceClient::OnDataPacket(xPacket*)
  │
  ├─ 心跳检查/更新
  │
  ▼
HandleDownloadRequest(buffer, len)
  │
  ├─ 解析 STDlNetMsgHeader
  ├─ 解析 SDLProtocalHeader
  │
  ▼
switch (wProtocal)
  │
  ├─ 0x1001 (下载请求)
  │   │
  │   ├─ for i = 0 to wFileCount-1:
  │   │   ├─ 解析 SDlProtocalBody
  │   │   ├─ FileManager::FindFileByHash(i64Hash)
  │   │   │   └─ lock_guard<mutex>
  │   │   │   └─ m_fileMap.find(hash)
  │   │   │
  │   │   ├─ 找到文件?
  │   │   │   ├─ 是 → SendFileToClient()
  │   │   │   │   ├─ FileManager::ReadFileRange()
  │   │   │   │   │   ├─ 打开文件
  │   │   │   │   │   ├─ seekg(startPos)
  │   │   │   │   │   └─ read(buffer, length)
  │   │   │   │   │
  │   │   │   │   ├─ 构建响应包
  │   │   │   │   │   ├─ STDlNetMsgHeader
  │   │   │   │   │   ├─ SDLProtocalHeader
  │   │   │   │   │   ├─ 路径长度 + 路径
  │   │   │   │   │   └─ 文件数据
  │   │   │   │   │
  │   │   │   │   └─ postSend(packet) → IOCP 异步发送
  │   │   │   │
  │   │   │   └─ 否 → 跳过
  │   │   │
  │   │   └─ offset += sizeof(SDlProtocalBody)
  │   │
  │   └─ end for
  │
  ├─ 0x1002 (心跳) → 无操作
  │
  └─ 0x1003 (确认收包) → 无操作
```

### 9.3 心跳检测流程

```
CResourceClient::Update() (每帧调用)
  │
  ├─ m_TimeOut.IsTimeOut(HEARTBRATTIME = 30s)?
  │   ├─ 是 → Disconnect() → 断开连接
  │   └─ 否 → CClientObject::Update() → 继续处理
  │
  └─ (在 OnDataPacket 中)
      └─ m_TimeOut.IsTimeOut(UPDATEHEARTBRAT = 20s)?
          └─ 是 → m_TimeOut.Savetime() → 刷新心跳时间
```

---

## 10. 性能优化设计

### 10.1 内存映射文件 (mmap)

```cpp
// LoadFromIndex 中使用 mmap 零拷贝
HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
CHAR* data = (CHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, fileSize.QuadPart);
// 直接访问 data 指针，无需额外内存分配
```

**优点：**
- 操作系统自动管理页面缓存
- 避免用户态与内核态之间的数据拷贝
- 内存使用按需分配（页面级）

### 10.2 对象池管理

```cpp
xIndexObjectPool<CResourceClient>
```

**优点：**
- 避免频繁 new/delete 造成的内存碎片
- 客户端对象复用，减少分配开销
- 基于索引的查找比指针查找更友好（缓存局部性）

### 10.3 哈希查找

```cpp
std::unordered_map<uint64_t, FileInfo> m_fileMap;
```

- 使用 64 位 MPQ 风格 Hash 作为键，碰撞率极低
- `unordered_map` 提供 O(1) 平均查找复杂度
- `reserve()` 预分配减少 rehash

### 10.4 线程安全

```cpp
std::mutex m_mutex;  // 保护 m_fileMap

FileInfo* FindFileByHash(uint64_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);  // RAII 自动解锁
    auto it = m_fileMap.find(hash);
    ...
}
```

### 10.5 内存分配优化

- 使用 **mimalloc** 替代默认分配器，提供更快的分配/释放性能
- Arena 预分配 128KB 内存池，减少小对象分配开销
- `reserve()` 预分配 unordered_map 容量，减少 rehash 开销

### 10.6 网络字节序处理

```cpp
msgHeader.wMsgID = ntohs(0x0BB9);    // 转换为网络字节序
msgHeader.dwDataLen = htonl(dataLen); // 转换为网络字节序
```

确保跨平台兼容性，所有网络传输数据统一使用大端字节序。

---

## 附录 A：术语表

| 术语 | 说明 |
|------|------|
| IOCP | I/O Completion Port，Windows 高性能异步 I/O 模型 |
| mmap | Memory-mapped file，内存映射文件 |
| WPF | 游戏资源包格式（类似于资源归档文件） |
| MPQ | Blizzard 的资源文件格式，本项目借鉴其 Hash 算法 |
| Arena | 内存池，预分配大块内存供小对象使用 |
| Hash | 文件路径的哈希值，用于快速查找 |

## 附录 B：常见问题

**Q: 为什么 .idx 索引文件比 .txt 加载更快？**

A: `.idx` 是二进制格式，使用内存映射（mmap）可直接将文件映射到内存地址空间，操作系统自动管理页面缓存。而 `.txt` 需要逐行读取、字符串解析、文件验证等操作。

**Q: 断点续传是如何实现的？**

A: 客户端在请求中携带 `dwStartPos` 字段，服务器从该偏移位置读取文件内容并发送。客户端通过 `0x1003` 确认协议告知已接收的数据长度，支持从中断位置继续传输。

**Q: 心跳超时时间是多少？**

A: 心跳超时阈值为 30 秒。如果客户端 30 秒内没有发送任何数据，服务器将主动断开连接。

**Q: 如何更新资源列表？**

A: 将新的资源路径添加到 `.txt` 文件中（或修改现有路径），删除对应的idx文件。再使用命令：ResourceServer -make 来自动重新解析并生成新的 `.idx` 缓存。
