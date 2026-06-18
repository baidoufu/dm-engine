#include "StdAfx.h"
#include "mysqldatabase.h"
#include "server.h"
#include <cstring>
#include "dbserver.h"
#include <vector>

CMySQLDatabase::CMySQLDatabase(VOID) = default;

CMySQLDatabase::~CMySQLDatabase(VOID)
{
	UnInit();
}

SERVER_ERROR CMySQLDatabase::Init()
{
	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;

	return SE_OK;
}

SERVER_ERROR CMySQLDatabase::UnInit()
{
	if (m_pMySQL)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
		return SE_OK;
	}
	return SE_DB_NOTINITED;
}

CVirtualDBConnection* CMySQLDatabase::GetConnection()
{
	CMySQLDBConnection* pConnection = nullptr;
	UINT id = m_DbConnections.New(&pConnection);
	if (id == 0 || pConnection == nullptr)
		return nullptr;
	pConnection->SetId(id);
	pConnection->SetDatabase(this);
	pConnection->Init(m_pMySQL);
	return pConnection;
}

SERVER_ERROR CMySQLDatabase::DelConnection(CVirtualDBConnection* pConnection)
{
	if (pConnection == nullptr)
		return SE_INVALIDPARAM;
	CMySQLDBConnection* pConnectionT = static_cast<CMySQLDBConnection*>(pConnection);
	pConnectionT->UnInit();
	m_DbConnections.Del(pConnectionT->GetId());
	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::Init(VOID* pMySQL)
{
	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;
	// 启用自动重连：mysql_ping 检测到断开时自动重连，减少一次网络往返
	my_bool bReconnect = 1;
	mysql_options(m_pMySQL, MYSQL_OPT_RECONNECT, &bReconnect);
	// 设置连接超时和读写超时（秒），避免网络异常时无限阻塞
	unsigned int nConnectTimeout = 10;
	mysql_options(m_pMySQL, MYSQL_OPT_CONNECT_TIMEOUT, &nConnectTimeout);
	unsigned int nReadTimeout = 30;
	unsigned int nWriteTimeout = 30;
	mysql_options(m_pMySQL, MYSQL_OPT_READ_TIMEOUT, &nReadTimeout);
	mysql_options(m_pMySQL, MYSQL_OPT_WRITE_TIMEOUT, &nWriteTimeout);

	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::UnInit()
{
	if (m_pMySQL != nullptr)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
	}
	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::Connect(const char* pServerName, const char* pPort, const char* pdbname, const char* pId, const char* pPassword)
{
	strncpy_s(m_szServerName.data(), m_szServerName.size(), pServerName, _TRUNCATE);
	strncpy_s(m_szPort.data(), m_szPort.size(), pPort, _TRUNCATE);
	strncpy_s(m_szDBName.data(), m_szDBName.size(), pdbname, _TRUNCATE);
	strncpy_s(m_szId.data(), m_szId.size(), pId, _TRUNCATE);
	strncpy_s(m_szPassword.data(), m_szPassword.size(), pPassword, _TRUNCATE);

	UINT port = atoi(pPort);

	if (!mysql_real_connect(m_pMySQL, pServerName, pId, pPassword, pdbname, port, nullptr, 0))
	{
		return SE_ODBC_SQLCONNECTFAIL;
	}

	std::array<char, 512> sql{};
	sprintf_s(sql.data(), sql.size(), "SET SESSION wait_timeout=28800,interactive_timeout=28800,net_read_timeout=3600,net_write_timeout=3600,tx_isolation='REPEATABLE-READ'");
	mysql_query(m_pMySQL, sql.data());

	IsstartDB = TRUE;
	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::Disconnect()
{
	if (m_pMySQL != nullptr)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
	}
	IsstartDB = FALSE;

	return SE_OK;
}

BOOL CMySQLDBConnection::IsConnectionValid()
{
	if (m_pMySQL == nullptr || !IsstartDB) return FALSE;

	if (mysql_ping(m_pMySQL) != 0)
		return FALSE;

	return TRUE;
}

std::string CMySQLDBConnection::EscapeString(const char* pszInput)
{
	if (pszInput == nullptr) return "";
	if (m_pMySQL == nullptr) return pszInput;

	size_t nLen = strlen(pszInput);
	std::string escaped;
	escaped.resize(nLen * 2 + 1);
	unsigned long result_len = mysql_real_escape_string(m_pMySQL, &escaped[0], pszInput, (unsigned long)nLen);
	escaped.resize(result_len);
	return escaped;
}

VOID CMySQLDBConnection::ResetForPoolReuse()
{
	// 清理所有 DataUnit，但保留 MySQL 连接
	CMySQLDataUnit* pUnit = m_DataUnits.First();
	while (pUnit)
	{
		pUnit->UnInit();
		pUnit = m_DataUnits.Next();
	}
	// 释放 DataUnit 池中的对象
	while (m_DataUnits.GetCount() > 0)
	{
		CMySQLDataUnit* pTemp = nullptr;
		UINT id = m_DataUnits.New(&pTemp);
		if (id > 0 && pTemp)
		{
			pTemp->UnInit();
			m_DataUnits.Del(id);
		}
		else
			break;
	}
}

SERVER_ERROR CMySQLDBConnection::Reconnect()
{
	if (m_pMySQL != nullptr && IsstartDB)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
		IsstartDB = FALSE;
	}

	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;

	// 重连时同样设置自动重连和超时
	my_bool bReconnect = 1;
	mysql_options(m_pMySQL, MYSQL_OPT_RECONNECT, &bReconnect);
	unsigned int nConnectTimeout = 10;
	mysql_options(m_pMySQL, MYSQL_OPT_CONNECT_TIMEOUT, &nConnectTimeout);
	unsigned int nReadTimeout = 30;
	unsigned int nWriteTimeout = 30;
	mysql_options(m_pMySQL, MYSQL_OPT_READ_TIMEOUT, &nReadTimeout);
	mysql_options(m_pMySQL, MYSQL_OPT_WRITE_TIMEOUT, &nWriteTimeout);

	SERVER_ERROR ret = Connect(m_szServerName.data(), m_szPort.data(), m_szDBName.data(), m_szId.data(), m_szPassword.data());
	if (ret != SE_OK)
	{
		CServer* pServer = CServer::GetInstance();
		PRINT(ERROR_RED, "数据库重连失败!\n");
	}
	else
	{
		CServer* pServer = CServer::GetInstance();
		PRINT(SUCCESS_GREEN, "数据库重连成功!\n");
	}
	return ret;
}

CVirtualDataUnit* CMySQLDBConnection::GetDataUnit()
{
	if (!IsConnectionValid())
	{
		CServer* pServer = CServer::GetInstance();
		PRINT(ERROR_RED, "GetDataUnit: 检测到连接断开, 尝试重连...\n");
		if (Reconnect() != SE_OK)
			return nullptr;
	}

	CMySQLDataUnit* pUnit = nullptr;
	UINT id = m_DataUnits.New(&pUnit);
	if (id == 0 || pUnit == nullptr)
		return nullptr;

	pUnit->SetId(id);
	pUnit->SetDBConnection(this);
	pUnit->Init(m_pMySQL);
	return pUnit;
}

SERVER_ERROR CMySQLDBConnection::DelDataUnit(CVirtualDataUnit* pDataUnit)
{
	if (pDataUnit == nullptr)
		return SE_INVALIDPARAM;
	CMySQLDataUnit* pUnit = static_cast<CMySQLDataUnit*>(pDataUnit);
	pUnit->UnInit();

	UINT id = pUnit->GetId();
	if (id > 0)
		m_DataUnits.Del(id);

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::Init(VOID* pMySQL)
{
	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	m_pCurrentRow = nullptr;
	m_nRowCount = 0;
	m_nCols = 0;
	m_nCurCols = 0;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::UnInit()
{
	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	m_pCurrentRow = nullptr;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::Operation(const char* desc, ...)
{
	std::array<char, 8192> szBuff{};
	va_list vl;
	va_start(vl, desc);
	vsprintf_s(szBuff.data(), szBuff.size(), desc, vl);
	va_end(vl);

	m_nCurCols = 0;

	CMySQLDBConnection* pConnection = static_cast<CMySQLDBConnection*>(m_pConnection);
	if (pConnection == nullptr)
		return SE_FAIL;

	MYSQL* pMySQL = pConnection->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;

	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	m_pCurrentRow = nullptr;

	int nResult = mysql_query(pMySQL, szBuff.data());

	if (nResult != 0)
	{
		UINT error_code = mysql_errno(pMySQL);
		CServer* pServer = CServer::GetInstance();
		if (error_code == 2006 || error_code == CR_SERVER_LOST)
		{
			PRINT(ERROR_RED, "检测到 MySQL server has gone away, 尝试重连...\n");

			if (pConnection->Reconnect() == SE_OK)
			{
				pMySQL = pConnection->GetConnectionHandle();
				nResult = mysql_query(pMySQL, szBuff.data());
				if (nResult != 0)
				{
					PRINT(ERROR_RED, "MySQL Error: %s\n", mysql_error(pMySQL));
					return SE_ODBC_SQLEXECDIRECTFAIL;
				}
			}
			else
			{
				return SE_ODBC_SQLEXECDIRECTFAIL;
			}
		}
		else
		{
			PRINT(ERROR_RED, "MySQL Error: %s\n", mysql_error(pMySQL));
			return SE_ODBC_SQLEXECDIRECTFAIL;
		}
	}

	m_pResult = mysql_store_result(pMySQL);
	if (m_pResult == nullptr)
	{
		if (mysql_field_count(pMySQL) == 0)
		{
			return SE_OK;
		}
		return SE_FAIL;
	}

	while (mysql_next_result(pMySQL) == 0)
	{
		MYSQL_RES* pNextResult = mysql_store_result(pMySQL);
		if (pNextResult)
			mysql_free_result(pNextResult);
	}

	nResult = PrepareColAndRow();
	if (nResult != SE_OK)
		return SE_FAIL;
	if (m_nCols == 0)
		return SE_OK;
	return MoveNext();
}

SERVER_ERROR CMySQLDataUnit::GetData(int nCol, int type, LPVOID lpBuffer, int& size)
{
	if (m_pResult == nullptr || m_pCurrentRow == nullptr)
		return SE_DB_NOTINITED;

	int colIndex = nCol - 1;
	if (colIndex < 0 || colIndex >= m_nCols)
		return SE_FAIL;

	unsigned long* lengths = mysql_fetch_lengths(m_pResult);
	if (lengths == nullptr)
		return SE_FAIL;

	if (size == 0)
	{
		if (m_pCurrentRow[colIndex] == nullptr)
		{
			// NULL 列：根据类型设置默认 size，让后续逻辑填充 0 值
			switch (type)
			{
			case SQL_C_CHAR:   size = 1; break;
			case SQL_INTEGER:  size = sizeof(int); break;
			case SQL_SMALLINT: size = sizeof(short); break;
			case SQL_TINYINT:  size = sizeof(BYTE); break;
			case SQL_C_BINARY: size = 0; break;
			default: return SE_FAIL;
			}
			// size 已确定，跳到 NULL 列填充逻辑
			if (lpBuffer && size > 0)
				memset(lpBuffer, 0, size);
			return SE_OK;
		}

		switch (type)
		{
		case SQL_C_CHAR:
			size = static_cast<int>(lengths[colIndex]) + 1;
			break;
		case SQL_INTEGER:
			size = sizeof(int);
			break;
		case SQL_SMALLINT:
			size = sizeof(short);
			break;
		case SQL_TINYINT:
			size = sizeof(BYTE);
			break;
		case SQL_C_BINARY:
			size = static_cast<int>(lengths[colIndex]);
			break;
		default:
			return SE_FAIL;
		}
	}

	if (lpBuffer == nullptr || size < 1) return SE_FAIL;

	if (m_pCurrentRow[colIndex] == nullptr)
	{
		memset(lpBuffer, 0, size);
		return SE_OK;
	}

	switch (type)
	{
	case SQL_C_CHAR:
		strncpy_s(static_cast<char*>(lpBuffer), size, m_pCurrentRow[colIndex], _TRUNCATE);
		size = static_cast<int>(lengths[colIndex]);
		break;
	case SQL_INTEGER:
		if (sizeof(int) <= static_cast<size_t>(size))
			*static_cast<int*>(lpBuffer) = atoi(m_pCurrentRow[colIndex]);
		size = sizeof(int);
		break;
	case SQL_SMALLINT:
		if (sizeof(short) <= static_cast<size_t>(size))
			*static_cast<short*>(lpBuffer) = static_cast<short>(atoi(m_pCurrentRow[colIndex]));
		size = sizeof(short);
		break;
	case SQL_TINYINT:
		if (sizeof(BYTE) <= static_cast<size_t>(size))
			*static_cast<BYTE*>(lpBuffer) = static_cast<BYTE>(atoi(m_pCurrentRow[colIndex]));
		size = sizeof(BYTE);
		break;
	case SQL_C_BINARY:
		if (static_cast<size_t>(size) >= lengths[colIndex])
			memcpy(lpBuffer, m_pCurrentRow[colIndex], lengths[colIndex]);
		size = static_cast<int>(lengths[colIndex]);
		break;
	default:
		return SE_FAIL;
	}
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::PrepareColAndRow()
{
	if (m_pResult == nullptr)
		return SE_FAIL;

	m_nCols = mysql_num_fields(m_pResult);
	if (m_nCols == 0)
	{
		m_nRowCount = 0;
		return SE_OK;
	}

	m_nRowCount = static_cast<int>(mysql_num_rows(m_pResult));

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::MoveNext()
{
	if (m_pResult == nullptr)
		return SE_DB_NOTINITED;

	m_pCurrentRow = mysql_fetch_row(m_pResult);
	if (m_pCurrentRow == nullptr)
		return SE_DB_NOMOREDATA;

	m_nCurCols = 0;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::GetColDesc(int nCol, ColumnInfo* pInfo)
{
	if (nCol < 0 || nCol >= m_nCols || m_pResult == nullptr)
		return SE_FAIL;

	MYSQL_FIELD* field = mysql_fetch_field_direct(m_pResult, nCol);
	if (field == nullptr)
		return SE_FAIL;

	strncpy_s(pInfo->szColName, field->name, _TRUNCATE);
	pInfo->nColNameSize = static_cast<int>(strlen(pInfo->szColName));
	pInfo->nColType = field->type;
	pInfo->nColSize = field->length;
	pInfo->nAllowDecimalDigit = field->decimals;
	pInfo->nAllowNull = (field->flags & NOT_NULL_FLAG) ? 0 : 1;

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::UpdateImage(const char* pszTableName, const char* pszColName, const char* pszCondition, LPVOID lpData, int datasize)
{
	CMySQLDBConnection* pConnection = static_cast<CMySQLDBConnection*>(m_pConnection);
	if (pConnection == nullptr)
		return SE_FAIL;

	MYSQL* pMySQL = pConnection->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;

	std::vector<char> escapedData(static_cast<size_t>(datasize) * 2 + 1);
	mysql_real_escape_string(pMySQL, escapedData.data(), static_cast<char*>(lpData), datasize);

	size_t sqlSize = strlen(pszTableName) + strlen(pszColName) + strlen(pszCondition) +
		static_cast<size_t>(datasize) * 2 + 100;
	std::vector<char> szSQLStr(sqlSize);
	sprintf_s(szSQLStr.data(), szSQLStr.size(),
		"UPDATE %s SET %s = '%s' WHERE %s", pszTableName, pszColName, escapedData.data(), pszCondition);

	int result = mysql_query(pMySQL, szSQLStr.data());

	if (result != 0)
		return SE_FAIL;

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::GetValue(DWORD& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_INTEGER, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(WORD& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_SMALLINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(BYTE& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_TINYINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(int& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_INTEGER, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(short& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_SMALLINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(char& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_TINYINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(char* pszString, int size)
{
	if (pszString == nullptr || size < 1) return SE_FAIL;
	return GetData(++m_nCurCols, SQL_C_CHAR, pszString, size);
}