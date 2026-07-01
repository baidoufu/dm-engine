#include "StdAfx.h"
#include ".\appdb.h"
#include "mysqldatabase.h"
#include "DBConnectionPool.h"
#include "server.h"
#include <string>
#include "dbserver.h"
#include <wincrypt.h>
#include <chrono>

// ============================================================================
// PasswordHash 实现
// ============================================================================
namespace PasswordHash {

static std::string MD5Hash(const std::string& input)
{
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbHash[16];
	DWORD cbHash = 16;
	std::string result;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return result;
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		CryptReleaseContext(hProv, 0);
		return result;
	}
	if (!CryptHashData(hHash, (BYTE*)input.c_str(), (DWORD)input.size(), 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return result;
	}
	if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return result;
	}

	char hex[33]{};
	for (DWORD i = 0; i < cbHash; i++)
		sprintf(hex + i * 2, "%02x", rgbHash[i]);
	hex[32] = '\0';
	result = hex;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return result;
}

static std::string GenerateSalt()
{
	HCRYPTPROV hProv = 0;
	BYTE randomBytes[4]{};
	std::string salt;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		// 使用 steady_clock 毫秒作为种子
		unsigned uSeed = static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count());
		srand(uSeed);
		sprintf(reinterpret_cast<char*>(randomBytes), "%04x", rand() & 0xFFFF);
	}
	else
	{
		CryptGenRandom(hProv, 4, randomBytes);
		CryptReleaseContext(hProv, 0);
	}

	char hex[9]{};
	for (int i = 0; i < 4; i++)
		sprintf(hex + i * 2, "%02x", randomBytes[i]);
	hex[8] = '\0';
	salt = hex;
	return salt;
}

std::string HashPassword(const char* pszPassword)
{
	if (pszPassword == nullptr || pszPassword[0] == '\0') return "";
	std::string salt = GenerateSalt();
	std::string combined = salt + pszPassword;
	std::string hash = MD5Hash(combined);
	return salt + "$" + hash;
}

bool VerifyPassword(const char* pszPassword, const char* pszStoredHash)
{
	if (pszPassword == nullptr || pszStoredHash == nullptr) return false;
	// 检查是否为哈希格式 "salt$hash"
	const char* delim = strchr(pszStoredHash, '$');
	if (delim == nullptr)
	{
		// 兼容旧版明文密码：直接比较
		return (strcmp(pszStoredHash, pszPassword) == 0);
	}
	std::string salt(pszStoredHash, delim - pszStoredHash);
	std::string storedHashOnly(delim + 1);
	std::string combined = salt + pszPassword;
	std::string computedHash = MD5Hash(combined);
	return (computedHash == storedHashOnly);
}

} // namespace PasswordHash

// ============================================================================
// CDeferredWriteBuffer 实现
// ============================================================================
void CDeferredWriteBuffer::Enqueue(std::string sql)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_pendingSQL.push_back(std::move(sql));
}

SERVER_ERROR CDeferredWriteBuffer::Flush(CVirtualDBConnection* pConnection)
{
	std::vector<std::string> localSQL;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		localSQL.swap(m_pendingSQL);
	}
	if (localSQL.empty()) return SE_OK;
	if (pConnection == nullptr) return SE_DB_NOTINITED;
	// 最多重试3次，应对临时网络抖动
	constexpr int maxRetries = 3;
	for (int attempt = 0; attempt < maxRetries; attempt++)
	{
		CVirtualDataUnit* pDataUnit = pConnection->GetDataUnit();
		if (pDataUnit == nullptr)
		{
			// GetDataUnit 内部会尝试重连，如果仍失败则放弃
			if (attempt < maxRetries - 1) continue;
			return SE_ALLOCMEMORYFAIL;
		}
		// 开启事务
		SERVER_ERROR ret = pDataUnit->Operation("START TRANSACTION");
		if (ret != SE_OK)
		{
			pConnection->DelDataUnit(pDataUnit);
			if (attempt < maxRetries - 1) continue;
			return ret;
		}
		int nExecuted = 0;
		bool bFailed = false;
		for (const auto& sql : localSQL)
		{
			ret = pDataUnit->Operation(sql.c_str());
			if (ret != SE_OK && ret != SE_DB_NOMOREDATA)
			{
				pDataUnit->Operation("ROLLBACK");
				pConnection->DelDataUnit(pDataUnit);
				PRINT(ERROR_RED, "批量写入失败(第%d条，第%d次尝试)，已回滚。SQL: %.100s\n", nExecuted, attempt + 1, sql.c_str());
				bFailed = true;
				break;
			}
			nExecuted++;
		}
		if (bFailed)
		{
			if (attempt < maxRetries - 1) continue;
			// 所有重试均失败，记录丢失的SQL以便后续恢复
			PRINT(ERROR_RED, "延迟写入彻底失败！共%d条SQL丢失，请检查数据库连接！\n", (int)localSQL.size());
			for (const auto& lostSql : localSQL)
			{
				LG2("[DATA_LOSS] %s\n", lostSql.c_str());
			}
			return ret;
		}

		ret = pDataUnit->Operation("COMMIT");
		pConnection->DelDataUnit(pDataUnit);
		if (ret == SE_OK) return SE_OK;
		// COMMIT 失败，重试
		if (attempt < maxRetries - 1) continue;
		return ret;
	}
	return SE_FAIL;
}

int CDeferredWriteBuffer::GetPendingCount() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return (int)m_pendingSQL.size();
}

BOOL CDeferredWriteBuffer::IsEmpty() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_pendingSQL.empty();
}

// ============================================================================
// CAppDB 实现
// ============================================================================
CAppDB::CAppDB(VOID) : m_pDBConnection(nullptr), m_pPool(nullptr), m_bUsingPool(false), m_bDeferredWrite(false)
{
}

CAppDB::~CAppDB(VOID)
{
	// 析构前先刷盘
	if (m_bDeferredWrite && !m_writeBuffer.IsEmpty())
		FlushDeferredWrites();
	if (m_pDBConnection != nullptr)
	{
		if (m_bUsingPool && m_pPool) // 连接池模式：归还连接
			m_pPool->Release(static_cast<CMySQLDBConnection*>(m_pDBConnection));
		else if (m_pDatabase)
			m_pDatabase->DelConnection(m_pDBConnection);
		m_pDBConnection = nullptr;
	}
	m_pDatabase.reset();
	m_pPool = nullptr;
	m_bUsingPool = false;
}

std::string CAppDB::Escape(const char* pszInput)
{
	if (pszInput == nullptr) return "";
	if (m_pDBConnection == nullptr) return pszInput;
	return m_pDBConnection->EscapeString(pszInput);
}

SERVER_ERROR CAppDB::OpenDataBase(const char* pszServer, const char* pszPort, const char* pszDBName, const char* pszId, const char* pszPassword)
{
	SERVER_ERROR ret = SE_OK;
	// 先断开旧连接，防止悬垂指针
	if (m_pDBConnection != nullptr)
	{
		if (m_bUsingPool && m_pPool)
			m_pPool->Release(static_cast<CMySQLDBConnection*>(m_pDBConnection));
		else if (m_pDatabase)
			m_pDatabase->DelConnection(m_pDBConnection);
		m_pDBConnection = nullptr;
	}
	m_pDatabase.reset();
	m_bUsingPool = false;
	m_pPool = nullptr;

	m_pDatabase = std::make_unique<CMySQLDatabase>();
	if (!m_pDatabase) return SE_ALLOCMEMORYFAIL;
	ret = m_pDatabase->Init();
	if (ret != SE_OK) return ret;
	m_pDBConnection = m_pDatabase->GetConnection();
	if (m_pDBConnection == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = m_pDBConnection->Connect(pszServer, pszPort, pszDBName, pszId, pszPassword);
	if (ret != SE_OK)
	{
		m_pDatabase->DelConnection(m_pDBConnection);
		m_pDBConnection = nullptr;
		return ret;
	}
	return SE_OK;
}

SERVER_ERROR CAppDB::OpenDataBase(CMySQLConnectionPool& pool)
{
	// 先断开旧连接
	if (m_pDBConnection != nullptr)
	{
		if (m_bUsingPool && m_pPool)
			m_pPool->Release(static_cast<CMySQLDBConnection*>(m_pDBConnection));
		else if (m_pDatabase)
			m_pDatabase->DelConnection(m_pDBConnection);
		m_pDBConnection = nullptr;
	}
	m_pDatabase.reset();
	// 从连接池获取
	m_pPool = &pool;
	m_bUsingPool = true;
	CMySQLDBConnection* pConn = pool.Acquire();
	if (pConn == nullptr)
		return SE_ALLOCMEMORYFAIL;
	m_pDBConnection = pConn;
	return SE_OK;
}

void CAppDB::EnableDeferredWrite(bool bEnable)
{
	m_bDeferredWrite = bEnable;
}

SERVER_ERROR CAppDB::FlushDeferredWrites()
{
	return m_writeBuffer.Flush(m_pDBConnection);
}

int CAppDB::GetDeferredWriteCount() const
{
	return m_writeBuffer.GetPendingCount();
}

void CAppDB::Reset()
{
	// 刷盘延迟写入
	if (m_bDeferredWrite && !m_writeBuffer.IsEmpty())
		FlushDeferredWrites();
	// 归还连接
	if (m_pDBConnection != nullptr)
	{
		if (m_bUsingPool && m_pPool)
			m_pPool->Release(static_cast<CMySQLDBConnection*>(m_pDBConnection));
		else if (m_pDatabase)
			m_pDatabase->DelConnection(m_pDBConnection);
		m_pDBConnection = nullptr;
	}
	m_pDatabase.reset();
	m_pPool = nullptr;
	m_bUsingPool = false;
	m_bDeferredWrite = false;
}

SERVER_ERROR CAppDB::CreateAccount(const char* pszAccount, const char* pszPassword, const char* pszName, const char* pszBirthday,
	const char* pszQ1, const char* pszA1, const char* pszQ2, const char* pszA2, const char* pszEmail,
	const char* pszPhoneNumber, const char* pszMobilePhoneNumber, const char* pszIdCard)
{
	CVirtualDataUnit* pDataUnit = nullptr;
	SERVER_ERROR ret = SE_OK;

	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;

	if (!xCheckAccount(pszAccount))
		return SE_REG_INVALIDACCOUNT;
	if (!xCheckPassword(pszPassword))
		return SE_REG_INVALIDPASSWORD;
	if (!xCheckNormalString(pszName))
		return SE_REG_INVALIDNAME;
	if (pszBirthday[0] != 0 && !xCheckBirthday(pszBirthday))
		return SE_REG_INVALIDBIRTHDAY;
	if (!xCheckIdCard(pszIdCard))
		return SE_REG_INVALIDIDCARD;
	if (!xCheckNormalString(pszQ1))
		return SE_REG_INVALIDQUESTION;
	if (!xCheckNormalString(pszA1))
		return SE_REG_INVALIDANSWER;
	if (!xCheckNormalString(pszQ2))
		return SE_REG_INVALIDQUESTION;
	if (!xCheckNormalString(pszA2))
		return SE_REG_INVALIDANSWER;
	if (!xCheckTelephone(pszPhoneNumber))
		return SE_REG_INVALIDPHONENUMBER;
	if (!xCheckMobilePhone(pszMobilePhoneNumber))
		return SE_REG_INVALIDMOBILEPHONE;

	if (pszEmail[0] != 0 && !xCheckEmail(pszEmail)) return SE_REG_INVALIDEMAIL;
	if ((ret = CheckAccountExist(pszAccount)) == SE_LOGIN_ACCOUNTNOTEXIST)
	{
		// 转义所有字符串参数防止 SQL 注入
		std::string sAccount = Escape(pszAccount);
		// 密码使用加盐哈希存储，不再存储明文
		std::string sPasswordHash = PasswordHash::HashPassword(pszPassword);
		std::string sEscapedPasswordHash = Escape(sPasswordHash.c_str());
		std::string sName = Escape(pszName);
		std::string sBirthday = Escape(pszBirthday);
		std::string sQ1 = Escape(pszQ1);
		std::string sA1 = Escape(pszA1);
		std::string sQ2 = Escape(pszQ2);
		std::string sA2 = Escape(pszA2);
		std::string sPhoneNumber = Escape(pszPhoneNumber);
		std::string sMobilePhoneNumber = Escape(pszMobilePhoneNumber);
		std::string sIdCard = Escape(pszIdCard);
		std::string sEmail = Escape(pszEmail);

		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("Insert into TBL_ACCOUNT(ACCOUNT,PASSWORD,NAME,BIRTHDAY,Q1,A1,Q2,A2,PHONENUMBER,MOBILEPHONENUMBER,IDCARD,EMAIL,CREATEDATE)"
			"VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',CURRENT_TIMESTAMP)",
			sAccount.c_str(), sEscapedPasswordHash.c_str(), sName.c_str(), sBirthday.c_str(),
			sQ1.c_str(), sA1.c_str(), sQ2.c_str(), sA2.c_str(),
			sPhoneNumber.c_str(), sMobilePhoneNumber.c_str(), sIdCard.c_str(), sEmail.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::CheckAccount(const char* pszAccount, const char* pszPassword)
{
	CHAR szPassword[256] = ""; // 增大缓冲区以容纳哈希值（8+1+32=41字符）及可能的额外数据
	int bufferSize = sizeof(szPassword) - 1; // 缓冲区大小，预留结尾\0
	int dataSize = 0;         // 接收实际数据大小

	if (!xCheckAccount(pszAccount)) return SE_LOGIN_ACCOUNTNOTEXIST;
	if (!xCheckPassword(pszPassword)) return SE_LOGIN_PASSWORDERROR;

	if (strlen(pszAccount) > 50) return SE_LOGIN_ACCOUNTNOTEXIST;

	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;

	SERVER_ERROR ret = CheckAccountExist(pszAccount);
	if (ret != SE_LOGIN_ACCOUNTEXIST)return ret;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	ret = pDataUnit->Operation("select PASSWORD from TBL_ACCOUNT where ACCOUNT = '%s'", Escape(pszAccount).c_str());
	if (ret == SE_OK)
	{
		dataSize = bufferSize;
		ret = pDataUnit->GetData(1, SQL_C_CHAR, szPassword, dataSize);
		if (ret == SE_OK)
		{
			szPassword[bufferSize - 1] = '\0';
			// 使用哈希验证（兼容旧版明文密码）
			if (PasswordHash::VerifyPassword(pszPassword, szPassword))
				ret = SE_OK;
			else
				ret = SE_LOGIN_PASSWORDERROR;
		}
		else
			ret = SE_LOGIN_ACCOUNTNOTEXIST;
	}
	else if (ret == SE_DB_NOMOREDATA)
		ret = SE_LOGIN_ACCOUNTNOTEXIST;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::ChangePassword(const char* pszAccount, const char* pszOldPass, const char* pszNewPass)
{
	if (!xCheckPassword(pszNewPass)) return SE_LOGIN_PASSWORDERROR;
	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;
	SERVER_ERROR ret = CheckAccount(pszAccount, pszOldPass);
	if (ret == SE_OK)
	{
		// 新密码使用加盐哈希存储
		std::string sNewPassHash = PasswordHash::HashPassword(pszNewPass);
		std::string sEscapedNewPassHash = Escape(sNewPassHash.c_str());
		std::string sAccount = Escape(pszAccount);

		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_ACCOUNT set password = '%s' where account = '%s'", sEscapedNewPassHash.c_str(), sAccount.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::CheckAccountExist(const char* pszAccount)
{
	if (!xCheckAccount(pszAccount)) return SE_LOGIN_ACCOUNTNOTEXIST;
	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select account from TBL_ACCOUNT where account = '%s'", Escape(pszAccount).c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA) return SE_LOGIN_ACCOUNTNOTEXIST;
	if (ret == SE_OK) return SE_LOGIN_ACCOUNTEXIST;
	return ret;
}

SERVER_ERROR CAppDB::GetCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult)
{
	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select NAME,CLASS,SEX,VLEVEL,HAIR,ODATE from TBL_CHARACTER_INFO where ACCOUNT = '%s' and SERVER = '%s' AND DELFLAG = 0", Escape(pszAccount).c_str(), Escape(pszServerName).c_str());
	int count = 0;
	if (ret == SE_OK)
	{
		SELECT_CHAR_LIST* clist = pResult->charlist.data();
		CHAR szDate[200] = "";
		CSystemTime	stTime[2];
		do
		{
			int size = 19;
			pDataUnit->GetData(1, SQL_C_CHAR, (LPVOID)clist[count].szName, size);
			size = 0;
			pDataUnit->GetData(2, SQL_TINYINT, (LPVOID)&clist[count].btClass, size);
			size = 0;
			pDataUnit->GetData(3, SQL_TINYINT, (LPVOID)&clist[count].btSex, size);
			size = 0;
			pDataUnit->GetData(4, SQL_SMALLINT, (LPVOID)&clist[count].wLevel, size);
			size = 0;
			pDataUnit->GetData(5, SQL_TINYINT, (LPVOID)&clist[count].btHair, size);
			size = 200;
			pDataUnit->GetData(6, SQL_C_CHAR, (LPVOID)szDate, size);
			stTime[count] = szDate;
			pResult->charlist[count] = clist[count];
			count++;
			if (count >= 2)
				break;
		} while (pDataUnit->MoveNext() == SE_OK);

		if (count == 1)
		{
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
		}
		else if (count == 2)
		{
			if (stTime[0] > stTime[1])
				clist[0].date.bflag = 1, clist[1].date.bflag = 0;
			else
				clist[0].date.bflag = 0, clist[1].date.bflag = 1;
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
			clist[1].date.year = stTime[1].GetYear();
			clist[1].date.month = stTime[1].GetMonth();
			clist[1].date.day = stTime[1].GetDay();
			clist[1].date.hour = stTime[1].GetHour();
			clist[1].date.minute = stTime[1].GetMinute();
		}
	}
	if (ret == SE_DB_NOMOREDATA)
	{
		ret = SE_OK;
		count = 0;
	}
	//ret = SE_OK ,count = 0;
	pResult->count = count;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CreateCharacter(CREATECHARDESC* pDesc)
{
	if (m_pDBConnection == nullptr) return SE_DB_NOTINITED;
	SERVER_ERROR ret = CheckCharacterExist(pDesc->szAccount, pDesc->szServer, pDesc->szName);
	if (!xCheckAccount(pDesc->szAccount))
	{
		PRINT(ERROR_RED, "检测szAccount返回错误\n");
		return SE_CREATECHARACTER_INVALID_CHARNAME;
	}
	if (!xCheckCharname(pDesc->szName)) {
		PRINT(ERROR_RED, "检测szName返回错误\n");
		return SE_CREATECHARACTER_INVALID_CHARNAME;
	}
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		PRINT(ERROR_RED, "SE_SELCHAR_CHAREXIST返回错误\n");
		return ret;
	}
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
	{
		PRINT(ERROR_RED, "pDataUnit == NULL返回错误\n");
		return SE_ALLOCMEMORYFAIL;
	}

	// 转义字符串参数
	std::string sAccount = Escape(pDesc->szAccount);
	std::string sServer = Escape(pDesc->szServer);
	std::string sName = Escape(pDesc->szName);

	ret = pDataUnit->Operation("insert into TBL_CHARACTER_INFO(ACCOUNT,SERVER,NAME,CLASS,SEX,VLEVEL,HAIR,CREATEDATE) "
		"VALUES('%s', '%s', '%s', %u, %u, %u, %u, CURRENT_TIMESTAMP)",
		sAccount.c_str(), sServer.c_str(), sName.c_str(), pDesc->btClass, pDesc->btSex, pDesc->btLevel, pDesc->btHair);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CheckCharacterExist(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (!xCheckAccount(pszAccount))return SE_SELCHAR_NOTEXIST;
	if (!xCheckCharname(pszName))return SE_SELCHAR_NOTEXIST;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select ID from TBL_CHARACTER_INFO where server = '%s' and name = '%s'", Escape(pszServerName).c_str(), Escape(pszName).c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK) return SE_SELCHAR_CHAREXIST;
	return SE_SELCHAR_NOTEXIST;
}

SERVER_ERROR CAppDB::DelCharacter(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		// 转义参数
		std::string sAccount = Escape(pszAccount);
		std::string sServer = Escape(pszServerName);
		std::string sName = Escape(pszName);

		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_CHARACTER_INFO set DELFLAG = 1, deldate = CURRENT_TIMESTAMP where account = '%s' and server = '%s' and name = '%s'", sAccount.c_str(), sServer.c_str(), sName.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetDelCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult)
{
	if (!xCheckAccount(pszAccount))
	{
		pResult->count = 0;
		return SE_OK;
	}
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select NAME,CLASS,SEX,VLEVEL,HAIR,ODATE from TBL_CHARACTER_INFO where ACCOUNT = '%s' and SERVER = '%s' AND DELFLAG = 1", Escape(pszAccount).c_str(), Escape(pszServerName).c_str());
	int count = 0;
	if (ret == SE_OK)
	{
		SELECT_CHAR_LIST* clist = nullptr;
		CHAR szDate[200] = { 0 };
		CSystemTime	stTime[MAX_DELCHARLISTCOUNT];
		do
		{
			clist = &pResult->charlist[count];
			int size = 19;
			pDataUnit->GetData(1, SQL_C_CHAR, (LPVOID)clist[0].szName, size);
			size = 0;
			pDataUnit->GetData(2, SQL_TINYINT, (LPVOID)&clist[0].btClass, size);
			size = 0;
			pDataUnit->GetData(3, SQL_TINYINT, (LPVOID)&clist[0].btSex, size);
			size = 0;
			pDataUnit->GetData(4, SQL_SMALLINT, (LPVOID)&clist[0].wLevel, size);
			size = 0;
			pDataUnit->GetData(5, SQL_TINYINT, (LPVOID)&clist[0].btHair, size);
			size = 200;
			pDataUnit->GetData(6, SQL_C_CHAR, (LPVOID)szDate, size);
			stTime[0] = szDate;
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
			count++;
			if (count >= MAX_DELCHARLISTCOUNT)break;
		} while (pDataUnit->MoveNext() == SE_OK);
	}
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	pResult->count = count;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::RestoreCharacter(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		// 转义参数
		std::string sAccount = Escape(pszAccount);
		std::string sServer = Escape(pszServerName);
		std::string sName = Escape(pszName);

		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_CHARACTER_INFO set DELFLAG = 0 where account = '%s' and server = '%s' and name = '%s'", sAccount.c_str(), sServer.c_str(), sName.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetMapPosition(const char* pszAccount, const char* pszServerName, const char* pszName, tQueryMapPosition_Result* pResult)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		int size = 32;
		ret = pDataUnit->Operation("select MAPNAME,POSX,POSY from TBL_CHARACTER_INFO where account = '%s' and server = '%s' and name = '%s'", Escape(pszAccount).c_str(), Escape(pszServerName).c_str(), Escape(pszName).c_str());
		if (ret == SE_OK)
		{
			pDataUnit->GetData(1, SQL_C_CHAR, pResult->szName.data(), size);
			size = 0;
			pDataUnit->GetData(2, SQL_SMALLINT, (LPVOID)&pResult->x, size);
			size = 0;
			pDataUnit->GetData(3, SQL_SMALLINT, (LPVOID)&pResult->y, size);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetCharDBInfo(const char* pszAccount, const char* pszServerName, const char* pszName, CHARDBINFO* pInfo)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("select "
			"ID, CLASS, SEX,"
			"VLEVEL, MAPNAME, POSX, POSY, DIR, ATTACKMODE, HAIR,"
			"CUREXP, HP, MP, MAXHP, MAXMP, MINDC, MAXDC,"
			"MINMC, MAXMC, MINSC, MAXSC, MINAC, MAXAC,"
			"MINMAC, MAXMAC,WEIGHT, HANDWEIGHT, BODYWEIGHT,"
			"GAMETIME, GOLD, MAPID, YUANBAO, BIGGOLD, BIGBAG,"
			"PERSONCODE, PERSONSIGN, TEMPRANK,"
			"FLAG1, FLAG2, FLAG3, FLAG4, GUILDNAME, FORGEPOINT,"
			"PROP1, PROP2, PROP3, PROP4, PROP5, PROP6, PROP7, PROP8"
			" from TBL_CHARACTER_INFO where account = '%s' and server = '%s' and name = '%s'", Escape(pszAccount).c_str(), Escape(pszServerName).c_str(), Escape(pszName).c_str());
		if (ret == SE_OK)
		{
			strncpy(pInfo->szName, pszName, 18);
			pInfo->szName[18] = 0;
			pDataUnit->GetValue(pInfo->dwDBId);
			pDataUnit->GetValue(pInfo->btClass);
			pDataUnit->GetValue(pInfo->btSex);

			pDataUnit->GetValue(pInfo->wLevel);
			pDataUnit->GetValue(pInfo->szStartPoint, 16);
			pDataUnit->GetValue(pInfo->x);
			pDataUnit->GetValue(pInfo->y);
			pDataUnit->GetValue(pInfo->dir);
			pDataUnit->GetValue(pInfo->btAttackMode);
			pDataUnit->GetValue(pInfo->btHair);

			pDataUnit->GetValue(pInfo->dwCurExp);
			pDataUnit->GetValue(pInfo->hp);
			pDataUnit->GetValue(pInfo->mp);
			pDataUnit->GetValue(pInfo->maxhp);
			pDataUnit->GetValue(pInfo->maxmp);
			pDataUnit->GetValue(pInfo->mindc);
			pDataUnit->GetValue(pInfo->maxdc);

			pDataUnit->GetValue(pInfo->minmc);
			pDataUnit->GetValue(pInfo->maxmc);
			pDataUnit->GetValue(pInfo->minsc);
			pDataUnit->GetValue(pInfo->maxsc);
			pDataUnit->GetValue(pInfo->minac);
			pDataUnit->GetValue(pInfo->maxac);

			pDataUnit->GetValue(pInfo->minmac);
			pDataUnit->GetValue(pInfo->maxmac);
			pDataUnit->GetValue(pInfo->weight);
			pDataUnit->GetValue(pInfo->handweight);
			pDataUnit->GetValue(pInfo->bodyweight);

			pDataUnit->GetValue(pInfo->nGameTime);
			pDataUnit->GetValue(pInfo->dwGold);
			pDataUnit->GetValue(pInfo->mapid);
			pDataUnit->GetValue(pInfo->dwYuanbao);
			pDataUnit->GetValue(pInfo->bBigGold);
			pDataUnit->GetValue(pInfo->bBigBag);

			pDataUnit->GetValue(pInfo->wPersonCode);
			pDataUnit->GetValue(pInfo->szPersonSign, 20);
			pDataUnit->GetValue(pInfo->szTempRank, 60);

			pDataUnit->GetValue(pInfo->dwFlag[0]);
			pDataUnit->GetValue(pInfo->dwFlag[1]);
			pDataUnit->GetValue(pInfo->dwFlag[2]);
			pDataUnit->GetValue(pInfo->dwFlag[3]);
			pDataUnit->GetValue(pInfo->szGuildName, 20);
			pDataUnit->GetValue(pInfo->dwForgePoint);

			pDataUnit->GetValue(pInfo->dwProp[0]);
			pDataUnit->GetValue(pInfo->dwProp[1]);
			pDataUnit->GetValue(pInfo->dwProp[2]);
			pDataUnit->GetValue(pInfo->dwProp[3]);
			pDataUnit->GetValue(pInfo->dwProp[4]);
			pDataUnit->GetValue(pInfo->dwProp[5]);
			pDataUnit->GetValue(pInfo->dwProp[6]);
			pDataUnit->GetValue(pInfo->dwProp[7]);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::PutCharDBInfo(CHARDBINFO* pInfo)
{
	SERVER_ERROR ret = SE_FAIL;

	// 转义字符串参数
	std::string sStartPoint = Escape(pInfo->szStartPoint);
	std::string sPersonSign = Escape(pInfo->szPersonSign);
	std::string sTempRank = Escape(pInfo->szTempRank);
	std::string sGuildName = Escape(pInfo->szGuildName);

	char szSQL[4096];
	sprintf_s(szSQL, sizeof(szSQL),
		"update TBL_CHARACTER_INFO "
		"set VLEVEL = %d, MAPNAME = '%s', POSX = %d, POSY = %d, DIR = %u, ATTACKMODE = %u, HAIR = %u,"
		"CUREXP = %d, HP = %d, MP = %d, MAXHP = %d, MAXMP = %d, MINDC = %u, MAXDC = %u,"
		"MINMC = %u, MAXMC = %u, MINSC = %u, MAXSC = %u, MINAC = %u, MAXAC = %u,"
		"MINMAC = %u, MAXMAC = %u,WEIGHT = %d, HANDWEIGHT = %u, BODYWEIGHT = %u,"
		"GAMETIME = %d, GOLD = %d, MAPID = %d, ODATE = CURRENT_TIMESTAMP, YUANBAO = %d, BIGGOLD = %d, BIGBAG = %d,"
		"PERSONCODE = %d, PERSONSIGN = '%s', TEMPRANK = '%s',"
		"FLAG1 = %d, FLAG2 = %d, FLAG3 = %d, FLAG4 = %d, GUILDNAME = '%s',FORGEPOINT = %d,"
		"PROP1 = %d, PROP2 = %d, PROP3 = %d, PROP4 = %d, PROP5 = %d, PROP6 = %d, PROP7 = %d, PROP8 = %d"
		" where ID = %u",
		pInfo->wLevel, sStartPoint.c_str(), pInfo->x, pInfo->y, pInfo->dir, pInfo->btAttackMode, pInfo->btHair,
		pInfo->dwCurExp, pInfo->hp, pInfo->mp, pInfo->maxhp, pInfo->maxmp, pInfo->mindc, pInfo->maxdc,
		pInfo->minmc, pInfo->maxmc, pInfo->minsc, pInfo->maxsc, pInfo->minac, pInfo->maxac,
		pInfo->minmac, pInfo->maxmac, pInfo->weight, pInfo->handweight, pInfo->bodyweight,
		pInfo->nGameTime, pInfo->dwGold, pInfo->mapid, pInfo->dwYuanbao, pInfo->bBigGold, pInfo->bBigBag,
		pInfo->wPersonCode, sPersonSign.c_str(), sTempRank.c_str(),
		pInfo->dwFlag[0], pInfo->dwFlag[1], pInfo->dwFlag[2], pInfo->dwFlag[3], sGuildName.c_str(), pInfo->dwForgePoint,
		pInfo->dwProp[0], pInfo->dwProp[1], pInfo->dwProp[2], pInfo->dwProp[3], pInfo->dwProp[4], pInfo->dwProp[5], pInfo->dwProp[6], pInfo->dwProp[7],
		pInfo->dwDBId
	);

	// PutCharDBInfo是角色核心数据保存，始终立即执行，不走延迟写入
	// 延迟写入可能导致崩溃时数据丢失
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::FindItemId(DWORD dwOwner, BYTE btFlag, WORD wPos, DWORD dwFindKey, DWORD& dwItemIndex)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select ID from TBL_CHARACTER_ITEM where "
		"OWNERID = %u and Flag = %u and POS = %hd and FINDKEY = %u LIMIT 1",
		dwOwner, btFlag, wPos, dwFindKey);
	if (ret == SE_OK)
	{
		int size = 0;
		ret = pDataUnit->GetData(1, SQL_INTEGER, &dwItemIndex, size);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
	{
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		pDataUnit->Operation("update TBL_CHARACTER_ITEM "
			"set FINDKEY = 0 "
			"where ID = %u", dwItemIndex);
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::UpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade)
{
	char szSQL[512];
	sprintf_s(szSQL, sizeof(szSQL), "update TBL_CHARACTER_ITEM "
		"set NEEDIDENTIFY = %d, FLAG = %u, FINDKEY = %u "
		"where ID = %u", 1, IDF_UPGRADE, dwUpgrade, dwMakeIndex);

	// 延迟写入
	if (m_bDeferredWrite)
	{
		m_writeBuffer.Enqueue(szSQL);
		return SE_OK;
	}

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CreateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	DWORD FindKey = static_cast<DWORD>(Getrand(0x7fffffff)) | CFrameTime::GetFrameTime();
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}
	// 转义物品名称
	std::string sName = Escape(szName);
	std::string sFullName = Escape(szFullName);

	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_ITEM( "
		"OWNERID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,FLAG,POS,FINDKEY,IMAGEINDEX,EXT,CREATEDATE)"
		" VALUES("
		"%u,'%s','%s',%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,UNHEX('%s'),CURRENT_TIMESTAMP)",
		dwOwner, sName.c_str(), sFullName.c_str(), pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType,
		pItem->baseitem.wFeature, btFlag, wPos, FindKey, pItem->baseitem.wImageIndex, szExtHex
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
		ret = FindItemId(dwOwner, btFlag, wPos, FindKey, pItem->dwMakeIndex);
	return ret;
}

SERVER_ERROR CAppDB::CreateItemEx(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}
	// 转义物品名称
	std::string sName2 = Escape(szName);
	std::string sFullName2 = Escape(szFullName);

	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_ITEM("
		"OWNERID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,FLAG,POS,FINDKEY,IMAGEINDEX,EXT,CREATEDATE)"
		" VALUES("
		"%u,'%s','%s',%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,UNHEX('%s'),CURRENT_TIMESTAMP)",
		dwOwner, sName2.c_str(), sFullName2.c_str(), pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType, pItem->baseitem.wFeature,
		btFlag, wPos, pItem->dwParam[0], pItem->baseitem.wImageIndex, szExtHex
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItem(DWORD	dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}

	// 转义物品名称
	std::string sName = Escape(szName);
	std::string sFullName = Escape(szFullName);

	char szSQL[4096];
	sprintf_s(szSQL, sizeof(szSQL), "update TBL_CHARACTER_ITEM "
		"set OWNERID = %u, NAME = '%s', FULLNAME = '%s', MINDC = %u, MAXDC = %u, MINMC = %u, MAXMC = %u, MINSC = %u, MAXSC = %u, MINAC = %u, MAXAC = %u, "
		"MINMAC = %u, MAXMAC = %u, DURA = %u, CURDURA = %u, MAXDURA = %u, NEEDTYPE = %u, NEEDLEVEL = %u, SPECIALPOWER = %u, NEEDIDENTIFY = %u, "
		"WEIGHT = %u, STDMODE = %d, SHAPE = %u, PRICE = %u, UNKNOWN_1 = %u, UNKNOWN_2 = %u, FLAG = %u, POS = %u, FINDKEY = %u, IMAGEINDEX = %u, EXT = UNHEX('%s'), DELFLAG = 0 "
		"where ID = %u",
		dwOwner, sName.c_str(), sFullName.c_str(), pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType, pItem->baseitem.wFeature,
		btFlag, wPos, pItem->dwParam[0], pItem->baseitem.wImageIndex, szExtHex, pItem->dwMakeIndex
	);

	// 延迟写入
	if (m_bDeferredWrite)
	{
		m_writeBuffer.Enqueue(szSQL);
		m_pDBConnection->DelDataUnit(pDataUnit);
		return SE_OK;
	}

	SERVER_ERROR ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteItem(DWORD dwItemIndex)
{
	char szSQL[256];
	sprintf_s(szSQL, sizeof(szSQL), "DELETE FROM TBL_CHARACTER_ITEM where ID = %u", dwItemIndex);

	// 延迟写入
	if (m_bDeferredWrite)
	{
		m_writeBuffer.Enqueue(szSQL);
		return SE_OK;
	}

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos)
{
	char szSQL[256];
	sprintf_s(szSQL, sizeof(szSQL), "update TBL_CHARACTER_ITEM "
		"set FLAG = %u, POS = %u where ID = %u", btFlag, wPos, dwItemIndex);

	// 延迟写入
	if (m_bDeferredWrite)
	{
		m_writeBuffer.Enqueue(szSQL);
		return SE_OK;
	}

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItemPosEx(BYTE btFlag, WORD wCount, BAGITEMPOS* pItemPos)
{
	for (int i = 0; i < wCount; i++)
	{
		UpdateItemPos(pItemPos[i].ItemId, btFlag, pItemPos[i].wPos);
	}
	return SE_OK;
}

SERVER_ERROR CAppDB::UpdateItemOwner(DWORD dwItemIndex, DWORD dwOwner, BYTE btFlag, WORD wPos)
{
	char szSQL[256];
	sprintf_s(szSQL, sizeof(szSQL), "update TBL_CHARACTER_ITEM "
		"set OWNERID = %u,FLAG = %u, POS = %u where ID = %u", dwOwner, btFlag, wPos, dwItemIndex);

	// 延迟写入
	if (m_bDeferredWrite)
	{
		m_writeBuffer.Enqueue(szSQL);
		return SE_OK;
	}

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation(szSQL);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::QueryItems(DWORD dwOwner, BYTE	btFlag, DBITEM* pItemBuffer, int& count)
{
	// 防止恶意传入超大count导致内存越界
	if (count > 100) count = 100;
	if (count <= 0) { count = 0; return SE_OK; }
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	int getcount = 0;
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select "
		"ID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,POS,FINDKEY,IMAGEINDEX,HEX(EXT) "
		"from TBL_CHARACTER_ITEM "
		"where OWNERID = %u and FLAG = %u and DELFLAG = 0 LIMIT %d", dwOwner, btFlag, count);
	if (ret == SE_OK)
	{
		memset(pItemBuffer, 0, sizeof(DBITEM) * count);  // 清空缓冲区, 避免旧数据残留
		do
		{
			DBITEM* p = &pItemBuffer[getcount];
			char szName[32];
			char szFullName[32];
			char szExtHex[721] = "";  // 360 * 2 + 1
			pDataUnit->GetValue(p->item.dwMakeIndex);
			pDataUnit->GetValue(szName, 32);
			pDataUnit->GetValue(szFullName, 32);
			pDataUnit->GetValue(p->item.baseitem.btMinAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinMagAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxMagAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinSouAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxSouAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinDef);
			pDataUnit->GetValue(p->item.baseitem.btMaxDef);
			pDataUnit->GetValue(p->item.baseitem.btMinMagDef);
			pDataUnit->GetValue(p->item.baseitem.btMaxMagDef);
			pDataUnit->GetValue(p->item.baseitem.wMaxDura);
			pDataUnit->GetValue(p->item.wCurDura);
			pDataUnit->GetValue(p->item.wMaxDura);
			pDataUnit->GetValue(p->item.baseitem.needtype);
			pDataUnit->GetValue(p->item.baseitem.needvalue);
			pDataUnit->GetValue(p->item.baseitem.btSpecialpower);
			pDataUnit->GetValue(p->item.baseitem.bNeedIdentify);
			pDataUnit->GetValue(p->item.baseitem.btWeight);
			pDataUnit->GetValue(p->item.baseitem.btStdMode);
			pDataUnit->GetValue(p->item.baseitem.btShape);
			pDataUnit->GetValue(p->item.baseitem.nPrice);
			pDataUnit->GetValue(p->item.baseitem.btPriceType);
			pDataUnit->GetValue(p->item.baseitem.wFeature);
			pDataUnit->GetValue(p->pos);
			pDataUnit->GetValue(p->item.dwParam[0]);
			pDataUnit->GetValue(p->item.baseitem.wImageIndex);
			pDataUnit->GetValue(szExtHex, 721);
			// 将 HEX 字符串转换回二进制数据
			int extLen = 360; // btItemExt数组大小
			for (int i = 0; i < extLen && szExtHex[i * 2] != '\0' && szExtHex[i * 2 + 1] != '\0'; i++)
			{
				char high = szExtHex[i * 2];
				char low = szExtHex[i * 2 + 1];
				BYTE val = 0;
				// 处理高位
				if (high >= '0' && high <= '9')
					val |= (high - '0') << 4;
				else if (high >= 'A' && high <= 'F')
					val |= (high - 'A' + 10) << 4;
				else if (high >= 'a' && high <= 'f')
					val |= (high - 'a' + 10) << 4;
				// 处理低位
				if (low >= '0' && low <= '9')
					val |= (low - '0');
				else if (low >= 'A' && low <= 'F')
					val |= (low - 'A' + 10);
				else if (low >= 'a' && low <= 'f')
					val |= (low - 'a' + 10);
				p->item.btItemExt[i] = val;
			}
			p->btFlag = btFlag;

			if (p->item.baseitem.wMaxDura < p->item.wMaxDura)
				p->item.baseitem.wMaxDura = p->item.wMaxDura;
			//	设置名字
			p->item.baseitem.btNameLength = static_cast<BYTE>(strlen(szName));
			strncpy(p->item.baseitem.szName, szName, 14);
			strncpy(p->item.szFullName, szFullName, 16);
			getcount++;
		} while (pDataUnit->MoveNext() == SE_OK && getcount < count);
	}
	count = getcount;
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	return ret;
}

SERVER_ERROR CAppDB::UpdateMagic(DWORD dwOwner, MAGICDB* pMagic, int count)
{
	CVirtualDataUnit* pDataUnit = nullptr;
	SERVER_ERROR ret = SE_OK;
	for (int i = 0; i < count; i++)
	{
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("select * "
			"from tbl_character_magic "
			"where CHARID = %u AND MAGICID = %u LIMIT 1", dwOwner, pMagic[i].wId);
		m_pDBConnection->DelDataUnit(pDataUnit);

		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		if (ret == SE_OK)
		{
			ret = pDataUnit->Operation("update tbl_character_magic "
				"set USERKEY = %u, CURLEVEL = %u, CURTRAIN = %u ,LEARNDATE = CURRENT_TIMESTAMP "
				"where CHARID = %u AND MAGICID = %u", pMagic[i].btKey, pMagic[i].btLevel, pMagic[i].dwCurTrain, dwOwner, pMagic[i].wId);
		}
		else
		{
			ret = pDataUnit->Operation("insert into tbl_character_magic("
				"CHARID, USERKEY, CURLEVEL, MAGICID, CURTRAIN, LEARNDATE) "
				"VALUES(%u, %u, %u, %u, %u, CURRENT_TIMESTAMP)",
				dwOwner, pMagic[i].btKey, pMagic[i].btLevel, pMagic[i].wId, pMagic[i].dwCurTrain);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

static int	qsort_cmpmagic(MAGICDB* p1, MAGICDB* p2)
{
	if (p1->wId > p2->wId)return -1;
	if (p1->wId == p2->wId)return 0;
	return 1;
}

SERVER_ERROR CAppDB::QueryMagic(DWORD dwOwner, MAGICDB* pMagicArray, int& count)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	int getcount = 0;
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select USERKEY,CURLEVEL,MAGICID,CURTRAIN "
		"FROM TBL_CHARACTER_MAGIC "
		"where CHARID = %d  LIMIT %d", dwOwner, count);
	memset(pMagicArray, 0, sizeof(MAGICDB) * count); // 清空缓冲区, 避免旧数据残留
	if (ret == SE_OK)
	{
		do
		{
			pDataUnit->GetValue(pMagicArray[getcount].btKey);
			pDataUnit->GetValue(pMagicArray[getcount].btLevel);
			pDataUnit->GetValue(pMagicArray[getcount].wId);
			pDataUnit->GetValue(pMagicArray[getcount].dwCurTrain);
			getcount++;
		} while (pDataUnit->MoveNext() == SE_OK && getcount < count);
	}

	count = getcount;
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	else
		qsort(pMagicArray, count, sizeof(MAGICDB), (int(*)(const VOID*, const VOID*))qsort_cmpmagic);
	return ret;
}

SERVER_ERROR CAppDB::UpdateCommunity(DWORD dwOwner, const char* pszCommunity)
{
	std::string szText(pszCommunity);
	xStringsExpander<200> community(&szText[0], '/');

	char* name;
	char* wife;
	char* master;
	char* friends[32] = { 0 };
	char* students[3] = { 0 };

	name = community[0] != nullptr ? community[0] : "";
	wife = community[1] != nullptr ? community[1] : "";
	master = community[2] != nullptr ? community[2] : "";
	if (name[0] == 0)return SE_DB_NOMOREDATA;
	for (int i = 0; i < 3; i++)
	{
		students[i] = community[3 + i] != nullptr ? community[3 + i] : "";
	}
	for (int i = 0; i < 32; i++)
	{
		friends[i] = community[6 + i] != nullptr ? community[6 + i] : "";
	}

	SERVER_ERROR ret = SE_OK;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	// 转义所有字符串参数
	std::string sName = Escape(name);

	// 更新/插入主表 TBL_CHARACTER_COMMUNITY
	ret = pDataUnit->Operation("select * from TBL_CHARACTER_COMMUNITY where OWNERID = %u LIMIT 1", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	if (ret == SE_OK)
	{
		ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY set name = '%s' WHERE OWNERID = %d",
			sName.c_str(), dwOwner);
	}
	else
	{
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_COMMUNITY(OWNERID,name) VALUES(%d,'%s')",
			dwOwner, sName.c_str());
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK) return ret;

	// 先清除旧的好友关系，再批量插入
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_FRIEND where character_id = %d", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK && ret != SE_DB_NOMOREDATA) return ret;

	for (int i = 0; i < 32; i++)
	{
		if (friends[i] == nullptr || friends[i][0] == 0) continue;
		std::string sFriend = Escape(friends[i]);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_FRIEND(character_id,friend_name,friend_order) VALUES(%d,'%s',%d)",
			dwOwner, sFriend.c_str(), i);
		m_pDBConnection->DelDataUnit(pDataUnit);
		if (ret != SE_OK) return ret;
	}

	// 先清除旧的师徒关系，再批量插入
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_STUDENT where character_id = %d", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK && ret != SE_DB_NOMOREDATA) return ret;

	for (int i = 0; i < 3; i++)
	{
		if (students[i] == nullptr || students[i][0] == 0) continue;
		std::string sStudent = Escape(students[i]);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_STUDENT(character_id,student_name,student_order) VALUES(%d,'%s',%d)",
			dwOwner, sStudent.c_str(), i);
		m_pDBConnection->DelDataUnit(pDataUnit);
		if (ret != SE_OK) return ret;
	}

	// 先清除旧的婚姻关系，再插入
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_MARRIAGE where character_id = %d", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK && ret != SE_DB_NOMOREDATA) return ret;

	if (wife != nullptr && wife[0] != 0)
	{
		std::string sWife = Escape(wife);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_MARRIAGE(character_id,spouse_name) VALUES(%d,'%s')",
			dwOwner, sWife.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
		if (ret != SE_OK) return ret;
	}

	// 处理师傅（一人一个师傅）
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_MASTER where character_id = %d", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK && ret != SE_DB_NOMOREDATA) return ret;

	if (master != nullptr && master[0] != 0)
	{
		std::string sMaster = Escape(master);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_MASTER(character_id,master_name) VALUES(%d,'%s')",
			dwOwner, sMaster.c_str());
		m_pDBConnection->DelDataUnit(pDataUnit);
		if (ret != SE_OK) return ret;
	}

	return SE_OK;
}

typedef char SHORTNAME[64];
SERVER_ERROR CAppDB::QueryCommunity(DWORD dwOwner, char* pszCommunity, int& iBufferSize)
{
	xPacketPool::ScopedPacket packet(pszCommunity, iBufferSize);

	// 查主表确认角色存在
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select OWNERID FROM TBL_CHARACTER_COMMUNITY where OWNERID = %d", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret != SE_OK)
	{
		strcpy(pszCommunity, "//////////////////////////////////////");
		iBufferSize = static_cast<int>(strlen(pszCommunity));
		return SE_OK;
	}

	// 查婚姻关系
	SHORTNAME wife = { 0 };
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("select spouse_name from TBL_CHARACTER_MARRIAGE where character_id = %d", dwOwner);
	if (ret == SE_OK) { pDataUnit->GetValue(wife, 64); }
	m_pDBConnection->DelDataUnit(pDataUnit);
	packet->push((LPVOID)wife, static_cast<int>(strlen(wife)));
	packet->push((LPVOID)"/", 1);

	// 查师傅
	SHORTNAME master = { 0 };
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("select master_name from TBL_CHARACTER_MASTER where character_id = %d", dwOwner);
	if (ret == SE_OK) { pDataUnit->GetValue(master, 64); }
	m_pDBConnection->DelDataUnit(pDataUnit);
	packet->push((LPVOID)master, static_cast<int>(strlen(master)));
	packet->push((LPVOID)"/", 1);

	// 查徒弟（按排序取前3个）
	SHORTNAME students[3] = { 0 };
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("select student_name from TBL_CHARACTER_STUDENT where character_id = %d order by student_order limit 3", dwOwner);
	if (ret == SE_OK)
	{
		for (int i = 0; i < 3; i++)
		{
			pDataUnit->GetValue(students[i], 64);
			if (pDataUnit->MoveNext() != SE_OK) break;
		}
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	for (int i = 0; i < 3; i++)
	{
		packet->push((LPVOID)students[i], static_cast<int>(strlen(students[i])));
		packet->push((LPVOID)"/", 1);
	}

	// 查好友（按排序取前32个）
	SHORTNAME friends[32] = { 0 };
	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("select friend_name from TBL_CHARACTER_FRIEND where character_id = %d order by friend_order limit 32", dwOwner);
	if (ret == SE_OK)
	{
		for (int i = 0; i < 32; i++)
		{
			pDataUnit->GetValue(friends[i], 64);
			if (pDataUnit->MoveNext() != SE_OK) break;
		}
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	for (int i = 0; i < 32; i++)
	{
		packet->push((LPVOID)friends[i], static_cast<int>(strlen(friends[i])));
		packet->push((LPVOID)"/", 1);
	}

	packet->push((LPVOID)"\0", 1);
	iBufferSize = static_cast<int>(packet->getsize());
	return SE_OK;
}

SERVER_ERROR CAppDB::DeleteMarriage(const char* pszName, const char* pszMarriage)
{
	// 转义参数
	std::string sName = Escape(pszName);
	std::string sMarriage = Escape(pszMarriage);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select OWNERID from TBL_CHARACTER_COMMUNITY where Name = '%s'", sName.c_str());
	if (ret != SE_OK) { m_pDBConnection->DelDataUnit(pDataUnit); return ret; }
	DWORD dwOwner = 0;
	pDataUnit->GetValue(dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_MARRIAGE where character_id = %d and spouse_name = '%s'", dwOwner, sMarriage.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteTeacher(const char* pszName, const char* pszTeacher)
{
	// 转义参数
	std::string sName = Escape(pszName);
	std::string sTeacher = Escape(pszTeacher);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select OWNERID from TBL_CHARACTER_COMMUNITY where Name = '%s'", sName.c_str());
	if (ret != SE_OK) { m_pDBConnection->DelDataUnit(pDataUnit); return ret; }
	DWORD dwOwner = 0;
	pDataUnit->GetValue(dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_MASTER where character_id = %d and master_name = '%s'", dwOwner, sTeacher.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteStudent(const char* pszTeacher, const char* pszStudent)
{
	// 转义参数
	std::string sTeacher = Escape(pszTeacher);
	std::string sStudent = Escape(pszStudent);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select OWNERID from TBL_CHARACTER_COMMUNITY where Name = '%s'", sTeacher.c_str());
	if (ret != SE_OK) { m_pDBConnection->DelDataUnit(pDataUnit); return ret; }
	DWORD dwOwner = 0;
	pDataUnit->GetValue(dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_STUDENT where character_id = %d and student_name = '%s'", dwOwner, sStudent.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::BreakFriend(const char* friend1, const char* friend2)
{
	// 转义参数
	std::string sFriend1 = Escape(friend1);
	std::string sFriend2 = Escape(friend2);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select OWNERID from TBL_CHARACTER_COMMUNITY where Name = '%s'", sFriend1.c_str());
	if (ret != SE_OK) { m_pDBConnection->DelDataUnit(pDataUnit); return ret; }
	DWORD dwOwner = 0;
	pDataUnit->GetValue(dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("delete from TBL_CHARACTER_FRIEND where character_id = %d and friend_name = '%s'", dwOwner, sFriend2.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

VOID CAppDB::DoInit()
{
	if (m_pDBConnection == nullptr) return;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set DELFLAG = 1 "
		"where FLAG = %u", IDF_GROUND);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
		PRINT(SUCCESS_GREEN, "垃圾物品清理完毕!\n");
}

SERVER_ERROR CAppDB::RestoreGuild(const char* name, const char* guildname)
{
	// 转义参数
	std::string sName = Escape(name);
	std::string sGuildName = Escape(guildname);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update tbl_character_info set guildname = '%s' where name = '%s'", sGuildName.c_str(), sName.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::AddCredit(const char* pszName, UINT nCount)
{
	// 转义参数
	std::string sName = Escape(pszName);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select flag1 from tbl_character_info where name = '%s'", sName.c_str());
	if (ret == SE_OK)
	{
		DWORD dwCredit;
		pDataUnit->GetValue(dwCredit);
		m_pDBConnection->DelDataUnit(pDataUnit);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		if (nCount > static_cast<WORD>(0xffff) - (dwCredit & 0xffff))
			dwCredit = (dwCredit & 0xffff0000) | 0xffff;
		else
			dwCredit = (dwCredit & 0xffff0000) | ((dwCredit & 0xffff) + nCount);
		ret = pDataUnit->Operation("update tbl_character_info set flag1 = %d", dwCredit);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateCharacterGuildName(const char* pszName, const char* pszGuildName)
{
	// 转义参数
	std::string sName = Escape(pszName);
	std::string sGuildName = Escape(pszGuildName);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select GUILDNAME from tbl_character_info where name = '%s'", sName.c_str());
	if (ret == SE_OK)
	{
		m_pDBConnection->DelDataUnit(pDataUnit);
		pDataUnit = m_pDBConnection->GetDataUnit();
		ret = pDataUnit->Operation("update tbl_character_info set GUILDNAME = '%s'", sGuildName.c_str());
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::ExecSqlCommand(DWORD dwTransId, const char* pszCommand, EXECSQLRECORD* pRecordDef, xPacket& packet)
{
	std::vector<char> szBufferVec(65536);
	char* szBuffer = szBufferVec.data();
	packet.clear();
	packet.push(&dwTransId, sizeof(dwTransId));
	packet.push(sizeof(DWORD) * 2);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation(pszCommand);
	DWORD dwCount = 0;
	DWORD* pdwParam = (DWORD*)packet.getbuf();
	BOOL bContinue = TRUE;
	if (ret == SE_OK && pRecordDef->dwColCount > 0 && pRecordDef->dwCount > 0)
	{
		do
		{
			for (UINT i = 0; i < pRecordDef->dwColCount; i++)
			{
				switch (pRecordDef->coldef[i].type)
				{
				case CT_STRING:		//	字符串
				{
					szBuffer[0] = 0;
					pDataUnit->GetValue(szBuffer, 65536);
					if (!packet.push((LPVOID)szBuffer, pRecordDef->coldef[i].length))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_TINYINT:		//	8位整数
				{
					BYTE	bt = 0;
					pDataUnit->GetValue(bt);
					if (!packet.push((LPVOID)&bt, sizeof(BYTE)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_SMALLINT:		///	16位整数
				{
					WORD w = 0;
					pDataUnit->GetValue(w);
					if (!packet.push((LPVOID)&w, sizeof(WORD)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_INTEGER:		//	32位整数
				{
					DWORD dw = 0;
					pDataUnit->GetValue(dw);
					if (!packet.push((LPVOID)&dw, sizeof(DWORD)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_BIGINT:			//	64位整数
				{
					__int64 i64 = 0;
					//pDataUnit->GetValue( dw );
					if (!packet.push((LPVOID)&i64, sizeof(__int64)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_DATETIME:		//	时间
				{
					szBuffer[0] = 0;
					SYSTEMTIME	st;
					pDataUnit->GetValue(szBuffer, 65536);
					GetTimeFromString(st, szBuffer);
					if (!packet.push((LPVOID)&st, sizeof(st)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_CODEDARRAY:		//	编码存的数据	
				{
					szBuffer[0] = 0;
					if (packet.getfreesize() < (int)pRecordDef->coldef[i].length)
					{
						bContinue = FALSE;
						break;
					}
					pDataUnit->GetValue(szBuffer, 65536);
					int limitsize = ((pRecordDef->coldef[i].length + 2) / 3) * 4;
					if (szBuffer[0] != 0)
					{
						szBuffer[limitsize] = 0;
						_UnGameCode(szBuffer, (BYTE*)packet.getfreebuf());
					}
					else
					{
						memset((VOID*)packet.getfreebuf(), 0, pRecordDef->coldef[i].length);
					}
					packet.addsize(pRecordDef->coldef[i].length);
				}
				break;
				}
			}
			if (!bContinue)
				break;
			dwCount++;
		} while (pDataUnit->MoveNext() == SE_OK && dwCount < pRecordDef->dwCount);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	pdwParam[0] = ret;
	pdwParam[1] = dwCount;
	return ret;
}

SERVER_ERROR CAppDB::DeleteMagic(DWORD dwOwner, WORD wId)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("DELETE FROM TBL_CHARACTER_MAGIC "
		//"set DELFLAG = 1 "
		"where CHARID = %d AND MAGICID = %d", dwOwner, wId);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

static constexpr UINT TASK_BINARY_SIZE = sizeof(TASKNODE) * MAX_TASK;
static VOID SerializeTasksBinary(TASKINFO* pInfo, BYTE* pBuffer, UINT bufferSize)
{
	memset(pBuffer, 0, bufferSize);
	UINT offset = 0;
	for (int i = 0; i < MAX_TASK; i++)
	{
		memcpy(pBuffer + offset, &pInfo->tasks[i].wTaskId, sizeof(WORD));
		offset += sizeof(WORD);
		memcpy(pBuffer + offset, &pInfo->tasks[i].wStep, sizeof(WORD));
		offset += sizeof(WORD);
		if (offset + sizeof(DWORD) * 10 <= bufferSize)
		{
			memcpy(pBuffer + offset, pInfo->tasks[i].vParam, sizeof(DWORD) * 10);
			offset += sizeof(DWORD) * 10;
		}
		if (offset + sizeof(char) * 10 * 32 <= bufferSize)
		{
			memcpy(pBuffer + offset, pInfo->tasks[i].sParam, sizeof(char) * 10 * 32);
			offset += sizeof(char) * 10 * 32;
		}
	}
}

static VOID DeserializeTasksBinary(const BYTE* pData, UINT dataSize, TASKINFO* pInfo)
{
	UINT offset = 0;
	for (int i = 0; i < MAX_TASK; i++)
	{
		memcpy(&pInfo->tasks[i].wTaskId, pData + offset, sizeof(WORD));
		offset += sizeof(WORD);
		memcpy(&pInfo->tasks[i].wStep, pData + offset, sizeof(WORD));
		offset += sizeof(WORD);
		if (offset + sizeof(DWORD) * 10 <= dataSize)
		{
			memcpy(pInfo->tasks[i].vParam, pData + offset, sizeof(DWORD) * 10);
			offset += sizeof(DWORD) * 10;
		}
		if (offset + sizeof(char) * 10 * 32 <= dataSize)
		{
			memcpy(pInfo->tasks[i].sParam, pData + offset, sizeof(char) * 10 * 32);
			offset += sizeof(char) * 10 * 32;
		}
	}
}

static thread_local BYTE s_szTasksBinary[TASK_BINARY_SIZE];
SERVER_ERROR CAppDB::QueryTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation("select Achievement,TotalTaskCount,Tasks "
		"FROM TBL_CHARACTER_TASK "
		"where CharId = %d", dwOwner);
	memset(pInfo, 0, sizeof(*pInfo));
	if (ret == SE_OK)
	{
		pDataUnit->GetValue(pInfo->dwAchievement);
		pDataUnit->GetValue(pInfo->dwTotalTaskCount);
		memset(s_szTasksBinary, 0, sizeof(s_szTasksBinary));
		int binarySize = sizeof(s_szTasksBinary);
		pDataUnit->GetData(3, SQL_C_BINARY, s_szTasksBinary, binarySize);
		DeserializeTasksBinary(s_szTasksBinary, binarySize, pInfo);
	}
	else
	{
		pInfo->dwAchievement = 0;
		pInfo->dwTotalTaskCount = 0;
		ret = InstertTaskInfo(dwOwner, pInfo);
	}
	return ret;
}

static const char HEX_TABLE[] = "0123456789ABCDEF";
SERVER_ERROR CAppDB::InstertTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	CMySQLDBConnection* pMySQLConn = (CMySQLDBConnection*)m_pDBConnection;
	MYSQL* pMySQL = pMySQLConn->GetConnectionHandle();
	if (pMySQL == nullptr)
	{
		m_pDBConnection->DelDataUnit(pDataUnit);
		return SE_FAIL;
	}
	SerializeTasksBinary(pInfo, s_szTasksBinary, sizeof(s_szTasksBinary));
	std::string hexString;
	hexString.resize(TASK_BINARY_SIZE * 2);
	for (int i = 0; i < TASK_BINARY_SIZE; i++)
	{
		BYTE b = s_szTasksBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}
	std::string sql = "insert into tbl_character_task(CharId,Achievement,TotalTaskCount,Tasks) values(" 
	                  + std::to_string(dwOwner) + ", " 
	                  + std::to_string(pInfo->dwAchievement) +", "
					  + std::to_string(pInfo->dwTotalTaskCount) + ", "
					  + "UNHEX('" + hexString + "'))";
	int result = mysql_query(pMySQL, sql.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);

	return (result == 0) ? SE_OK : SE_FAIL;
}

SERVER_ERROR CAppDB::UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	CMySQLDBConnection* pMySQLConn = (CMySQLDBConnection*)m_pDBConnection;
	MYSQL* pMySQL = pMySQLConn->GetConnectionHandle();
	if (pMySQL == nullptr)
	{
		m_pDBConnection->DelDataUnit(pDataUnit);
		return SE_FAIL;
	}
	SerializeTasksBinary(pInfo, s_szTasksBinary, sizeof(s_szTasksBinary));
	std::string hexString;
	hexString.resize(TASK_BINARY_SIZE * 2);
	for (int i = 0; i < TASK_BINARY_SIZE; i++)
	{
		BYTE b = s_szTasksBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}
	std::string sql = "update tbl_character_task set Achievement = " + std::to_string(pInfo->dwAchievement) 
						+ ", TotalTaskCount = " + std::to_string(pInfo->dwTotalTaskCount)
						+ ", Tasks = UNHEX('" + hexString + "') where CharId = " + std::to_string(dwOwner);
	int result = mysql_query(pMySQL, sql.c_str());
	SERVER_ERROR ret = (result == 0) ? SE_OK : SE_FAIL;

	m_pDBConnection->DelDataUnit(pDataUnit);

	if (ret != SE_OK)
		return InstertTaskInfo(dwOwner, pInfo);
	return ret;
}

static VOID SerializeFengHaoGrowsBinary(FenghaoInfo* pInfo, BYTE* pBuffer, int bufferSize)
{
	memset(pBuffer, 0, bufferSize);
	int offset = 0;
	for (int i = 0; i < MAX_FENGHAO; i++)
	{
		memcpy(pBuffer + offset, &pInfo->mFengHaoRow[i].boActivation, sizeof(BOOL));
		offset += sizeof(BOOL);
		memcpy(pBuffer + offset, &pInfo->mFengHaoRow[i].dwLastDate, sizeof(DWORD));
		offset += sizeof(DWORD);
	}
}

static VOID DeserializeFengHaoGrowsBinary(const BYTE* pData, int dataSize, FenghaoInfo* pInfo)
{
	int offset = 0;
	for (int i = 0; i < MAX_FENGHAO; i++)
	{
		memcpy(&pInfo->mFengHaoRow[i].boActivation, pData + offset, sizeof(BOOL));
		offset += sizeof(BOOL);
		memcpy(&pInfo->mFengHaoRow[i].dwLastDate, pData + offset, sizeof(DWORD));
		offset += sizeof(DWORD);
	}
}

static constexpr int FENGHAOGROW_BINARY_SIZE = sizeof(FengHaoRow) * MAX_FENGHAO;
static thread_local BYTE s_szFengHaoGrowsBinary[FENGHAOGROW_BINARY_SIZE];
SERVER_ERROR CAppDB::QueryFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation("select Type1, Type2, Type3, FengHaoRow "
		"FROM TBL_CHARACTER_FENGHAO "
		"WHERE CharId = %u", dwOwner);

	if (ret == SE_OK)
	{
		pDataUnit->GetValue(pInfo->btType1);
		pDataUnit->GetValue(pInfo->btType2);
		pDataUnit->GetValue(pInfo->btType3);

		memset(s_szFengHaoGrowsBinary, 0, sizeof(s_szFengHaoGrowsBinary));
		int binarySize = sizeof(s_szFengHaoGrowsBinary);
		pDataUnit->GetData(4, SQL_C_BINARY, s_szFengHaoGrowsBinary, binarySize);
		DeserializeFengHaoGrowsBinary(s_szFengHaoGrowsBinary, binarySize, pInfo);
	}
	else if (ret == SE_DB_NOMOREDATA)
	{
		pInfo->btType1 = 0;
		pInfo->btType2 = 0;
		pInfo->btType3 = 0;
		ret = InstertFengHaoInfo(dwOwner, pInfo);
	}

	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::InstertFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SerializeFengHaoGrowsBinary(pInfo, s_szFengHaoGrowsBinary, sizeof(s_szFengHaoGrowsBinary));

	std::string hexString;
	hexString.resize(FENGHAOGROW_BINARY_SIZE * 2);
	for (int i = 0; i < FENGHAOGROW_BINARY_SIZE; i++)
	{
		BYTE b = s_szFengHaoGrowsBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}

	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_FENGHAO(CharId, Type1, Type2, Type3, FengHaoRow) values(%u, %u, %u, %u, UNHEX('%s'))",
		dwOwner, pInfo->btType1, pInfo->btType2, pInfo->btType3, hexString.c_str());

	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SerializeFengHaoGrowsBinary(pInfo, s_szFengHaoGrowsBinary, sizeof(s_szFengHaoGrowsBinary));

	std::string hexString;
	hexString.resize(FENGHAOGROW_BINARY_SIZE * 2);
	for (int i = 0; i < FENGHAOGROW_BINARY_SIZE; i++)
	{
		BYTE b = s_szFengHaoGrowsBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}

	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_FENGHAO set Type1 = %u, Type2 = %u, Type3 = %u, FengHaoRow = UNHEX('%s') where CharId = %u",
		pInfo->btType1, pInfo->btType2, pInfo->btType3, hexString.c_str(), dwOwner);

	m_pDBConnection->DelDataUnit(pDataUnit);

	if (ret != SE_OK)
		return InstertFengHaoInfo(dwOwner, pInfo);
	return ret;
}