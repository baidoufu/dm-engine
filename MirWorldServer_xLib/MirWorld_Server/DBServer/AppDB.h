#pragma once
#include "virtualdatabase.h"
#include <memory>
#include <vector>
#include <string>
#include <mutex>

class CMySQLConnectionPool;

// ============================================================================
// CDeferredWriteBuffer — 延迟写入缓冲区
//
// 将可延迟的写操作 SQL 缓存起来，定时批量刷盘（事务提交）
// 减少高频写入对数据库的压力
// ============================================================================
class CDeferredWriteBuffer
{
public:
	// 将一条 SQL 加入缓冲
	VOID Enqueue(std::string sql);
	// 刷盘：在事务中执行所有缓冲的 SQL
	SERVER_ERROR Flush(CVirtualDBConnection* pConnection);
	// 获取待处理数量
	int GetPendingCount() const;
	// 缓冲是否为空
	BOOL IsEmpty() const;
private:
	mutable std::mutex m_mutex;
	std::vector<std::string> m_pendingSQL;
};

// ============================================================================
// 密码哈希辅助函数
// 使用 MD5(盐+密码) 格式存储，格式为 "盐$哈希"
// 盐为8字节随机十六进制字符串，哈希为32字节MD5十六进制字符串
// ============================================================================
namespace PasswordHash {
	// 生成密码哈希字符串，格式: "salt$md5(salt+password)"
	std::string HashPassword(const char* pszPassword);
	// 验证密码是否匹配哈希
	bool VerifyPassword(const char* pszPassword, const char* pszStoredHash);
}

class CAppDB
{
public:
	CAppDB(VOID);
	virtual ~CAppDB(VOID);
	// 传统方式：创建独立连接
	SERVER_ERROR OpenDataBase(const char* pszServer, const char* pszPort, const char* pszDBName, const char* pszId, const char* pszPassword);
	// 连接池方式：从池中获取连接
	SERVER_ERROR OpenDataBase(CMySQLConnectionPool& pool);

	SERVER_ERROR CreateAccount(const char* pszAccount, const char* pszPassword, const char* pszName, const char* pszBirthday,
		const char* pszQ1, const char* pszA1, const char* pszQ2, const char* pszA2, const char* pszEmail,
		const char* pszPhoneNumber, const char* pszMobilePhoneNumber, const char* pszIdCard);
	SERVER_ERROR CheckAccount(const char* pszAccount, const char* pszPassword);
	SERVER_ERROR ChangePassword(const char* pszAccount, const char* pszOldPass, const char* pszNewPass);
	SERVER_ERROR CheckAccountExist(const char* pszAccount);

	SERVER_ERROR GetCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult);
	SERVER_ERROR GetDelCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult);

	SERVER_ERROR CreateCharacter(CREATECHARDESC* pDesc);
	SERVER_ERROR CheckCharacterExist(const char* pszAccount, const char* pszServerName, const char* pszName);
	SERVER_ERROR DelCharacter(const char* pszAccount, const char* pszServerName, const char* pszName);
	SERVER_ERROR RestoreCharacter(const char* pszAccount, const char* pszServerName, const char* pszName);

	SERVER_ERROR GetMapPosition(const char* pszAccount, const char* pszServerName, const char* pszName, tQueryMapPosition_Result* pResult);
	SERVER_ERROR GetCharDBInfo(const char* pszAccount, const char* pszServerName, const char* pszName, CHARDBINFO* pInfo);
	SERVER_ERROR PutCharDBInfo(CHARDBINFO* pInfo);

	SERVER_ERROR FindItemId(DWORD dwOwner, BYTE btFlag, WORD wPos, DWORD dwFindKey, DWORD& dwItemIndex);

	SERVER_ERROR CreateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR CreateItemEx(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR UpdateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR DeleteItem(DWORD dwItemIndex);
	SERVER_ERROR UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos);
	SERVER_ERROR UpdateItemPosEx(BYTE btFlag, WORD wCount, BAGITEMPOS* pItemPos);

	SERVER_ERROR UpdateItemOwner(DWORD dwItemIndex, DWORD dwOwner, BYTE btFlag, WORD wPos);

	SERVER_ERROR QueryItems(DWORD dwOwner, BYTE	btFlag, DBITEM* pItemBuffer, int& count);

	SERVER_ERROR UpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade);

	SERVER_ERROR UpdateMagic(DWORD dwOwner, MAGICDB* pMagic, int count);
	SERVER_ERROR QueryMagic(DWORD dwOwner, MAGICDB* pMagicArray, int& count);

	SERVER_ERROR UpdateCommunity(DWORD dwOwner, const char* pszCommunity);
	SERVER_ERROR QueryCommunity(DWORD dwOwner, char* pszCommunity, int& iBufferSize);
	SERVER_ERROR BreakFriend(const char* friend1, const char* friend2);
	SERVER_ERROR RestoreGuild(const char* name, const char* guildname);

	SERVER_ERROR DeleteMarriage(const char* pszName, const char* pszMarriage);
	SERVER_ERROR DeleteTeacher(const char* pszName, const char* pszTeacher);
	SERVER_ERROR DeleteStudent(const char* pszTeacher, const char* pszStudent);

	SERVER_ERROR ExecSqlCommand(DWORD dwTransId, const char* pszCommand, EXECSQLRECORD* pRecordDef, xPacket& packet);
	SERVER_ERROR AddCredit(const char* pszName, UINT nCount);
	SERVER_ERROR UpdateCharacterGuildName(const char* pszName, const char* pszGuildName);
	SERVER_ERROR DeleteMagic(DWORD dwOwner, WORD wId);
	//任务系统数据
	SERVER_ERROR QueryTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	SERVER_ERROR UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	SERVER_ERROR InstertTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	//时长封号系统数据
	SERVER_ERROR QueryFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);
	SERVER_ERROR UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);
	SERVER_ERROR InstertFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);

	VOID DoInit();

	// 获取底层连接（用于心跳保活等）
	CVirtualDBConnection* GetConnection() { return m_pDBConnection; }

	// 延迟写入相关
	void EnableDeferredWrite(bool bEnable);
	SERVER_ERROR FlushDeferredWrites();
	int GetDeferredWriteCount() const;
	// 安全重置：归还连接池连接、清空缓冲区，使对象回到初始状态
	// 用于对象池复用场景，替代手动析构+placement new
	void Reset();
private:
	// SQL 注入防护：转义字符串
	std::string Escape(const char* pszInput);

	std::unique_ptr<CVirtualDatabase> m_pDatabase;
	CVirtualDBConnection* m_pDBConnection;

	// 连接池相关
	CMySQLConnectionPool* m_pPool;		// 非拥有指针，连接池模式时使用
	bool m_bUsingPool;					// 是否使用连接池模式

	// 延迟写入缓冲
	CDeferredWriteBuffer m_writeBuffer;
	bool m_bDeferredWrite;				// 是否启用延迟写入
};
