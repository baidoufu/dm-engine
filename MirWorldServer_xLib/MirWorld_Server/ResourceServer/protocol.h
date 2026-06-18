#pragma once
#include <cstdint>
#include <string>
#include <vector>

#pragma pack(push, 1)
// 二进制索引文件头
struct FileIndexHeader
{
    uint64_t magic;      // 魔数 FIDX = 0x46494458
    uint64_t count;      // 文件数量
};
// 二进制索引条目（固定大小，便于mmap后直接数组访问）
struct FileIndexEntry
{
    uint64_t hash;       // 路径Hash
    uint32_t size;       // 文件大小
    uint32_t pathOffset; // 路径字符串在文件中的偏移
    uint16_t pathLen;    // 路径字符串长度
};
// 服务器响应消息头
struct STDlNetMsgHeader
{
    uint16_t wMsgID;      // 消息ID = 3000 (CLI_SS_MSG), 网络字节序
    uint32_t dwDataLen;   // 后续数据长度, 网络字节序
};
// 下载文件协议头
struct SDLProtocalHeader
{
    uint16_t wProtocal;     // 协议号0x1001(请求下载)/0x1002(心跳)/0x1003(确认收包)
    uint16_t wFileCount;    // 请求下载的文件个数
    uint32_t dwID;          // id (图片纹理ID, 数据文件为0)
    uint64_t i64Hash;       // 文件路径的Hash值 (第一个文件的hash)
    uint32_t iWpf;          // 所属wpf包索引 (服务器返回用)
    uint32_t dwAttribute;   // 文件属性标志eFctattr类型/如果0x1003则是否成功接收
    uint32_t dwLen;         // 文件长度 (服务器用)/如果0x1003则已接收的数据长度
    uint32_t dwStartPos;    // 发送的起始位置, 断点续传用/如果0x1003则本次确认的起始位置
    uint32_t dwLastDWORD;   // 最后四个字节内容, 用来校验文件数据。如何它等于 -1 则不校验
    uint32_t dwRev;         // 保留
};
// 客户端请求体
struct SDlProtocalBody
{
    uint32_t dwID;          // 纹理ID (图片资源用，数据文件为0)
    uint64_t i64Hash;       // 文件路径的Hash值
    uint32_t iWpf;          // 所属wpf包索引
    uint8_t  byPrior;       // 优先级eReadPrior
    uint8_t  byRev[3];      // 保留
    uint32_t dwStartPos;    // 起始位置, 用于断点续传
    uint32_t dwTickCount;   // 请求的时间戳
};
#pragma pack(pop)
// 文件信息
struct FileInfo
{
    std::string filePath;
    uint64_t hash;
    uint32_t size;
};
// 文件属性标志
enum eFctattr
{
    EFA_BLANK = 0,           //空白
    EFA_DIR = 0x0001,        //目录
    EFA_FILE = 0x0002,       //文件
    EFA_COMPRESS = 0x0004,   //拥有此属性的文件为压缩文件,压缩类型由文件头中的byCompress指定
    EFA_CRYPT = 0x0008,      //拥有此属性的文件为加密文件,压缩类型由文件头中的byCrypt指定
    EFA_MEARGE_UNCOMPRESS = 0x0010,   //拥有此属性的文件,如果已经压缩了,当合并到目标wpf里去时要求自行解压,解压后再合并到目标wpf
};
// 优先级
enum eReadPrior
{
    EP_MOST_HIGH = 0,      //最高
    EP_UI,	               //UI	
    EP_TILES,              //大地表
    EP_NPC,                //NPC
    EP_MONSTER,            //怪物
    EP_CHARACTER,          //角色
    EP_SMTILES,            //小地表及贴地物体
    EP_OBJECT,             //地图中的物体层
    EP_SKILL,              //技能特效
    EP_MAGIC,              //魔法特效
    EP_AUDIO,	           //音效
    EP_NORMAL,             //一般
    EP_DONT_DOWNLOAD,      //如果本地有就读取,没有不下载
    EP_TYPES,	           //优先等级种类数
};