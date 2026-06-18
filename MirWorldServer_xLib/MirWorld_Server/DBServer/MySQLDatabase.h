#pragma once
#include "virtualdatabase.h"
#include <mysql.h>
#include <errmsg.h>
#include <array>
#include <memory>

#ifndef SQL_C_CHAR
#define SQL_C_CHAR           1
#define SQL_C_LONG           4
#define SQL_C_SHORT          5
#define SQL_C_TINYINT        6
#define SQL_C_BINARY         7
#endif

#ifndef SQL_INTEGER
#define SQL_INTEGER          4
#endif

#ifndef SQL_SMALLINT
#define SQL_SMALLINT         5
#endif

#ifndef SQL_TINYINT
#define SQL_TINYINT          6
#endif

constexpr UINT MAX_CONNECTION = 255;
constexpr UINT MAX_DATAUNIT = 32;
constexpr UINT MAXBUFLEN = 1024;

class CMySQLDataUnit :
	public CVirtualDataUnit
{
	int m_nRowCount{ 0 };
	int m_nCols{ 0 };
	int m_nCurCols{ 0 };
	MYSQL_ROW m_pCurrentRow{ nullptr };
	int m_nCurrentRowIndex{ -1 };
public:
	CMySQLDataUnit() = default;
	~CMySQLDataUnit() = default;
	VOID SetId(UINT id) { m_Id = id; }
	UINT GetId()const { return m_Id; }
	VOID SetDBConnection(CVirtualDBConnection* pConnection) { m_pConnection = pConnection; }
	CVirtualDBConnection* GetDBConnection() { return m_pConnection; }
public:
	SERVER_ERROR Init(VOID*);
	SERVER_ERROR UnInit();

	int	GetColCount() { return m_nCols; }
	int GetRowCount() { return m_nRowCount; }

	SERVER_ERROR GetData(int nCol, int type, LPVOID lpBuffer, int& size);
	SERVER_ERROR GetValue(DWORD& val);
	SERVER_ERROR GetValue(WORD& val);
	SERVER_ERROR GetValue(BYTE& val);
	SERVER_ERROR GetValue(int& val);
	SERVER_ERROR GetValue(short& val);
	SERVER_ERROR GetValue(char& val);
	SERVER_ERROR GetValue(char* pszString, int size);

	SERVER_ERROR UpdateImage(const char* pszTableName, const char* pszColName, const char* pszCondition, LPVOID lpData, int datasize);

	SERVER_ERROR Operation(const char* desc, ...);
	SERVER_ERROR MoveNext();
	SERVER_ERROR GetColDesc(int nCol, ColumnInfo* pInfo);
private:
	SERVER_ERROR PrepareColAndRow();
	MYSQL_RES* m_pResult{ nullptr };
	UINT m_Id{ 0 };
	CVirtualDBConnection* m_pConnection{ nullptr };
};

class CMySQLDBConnection :
	public CVirtualDBConnection
{
public:
	CMySQLDBConnection() = default;
	~CMySQLDBConnection() = default;
	VOID SetId(UINT id) { m_Id = id; }
	UINT GetId()const { return m_Id; }
	VOID SetDatabase(CVirtualDatabase* pDatabase) { m_pDatabase = pDatabase; }
	CVirtualDatabase* GetDatabase() { return m_pDatabase; }
public:
	SERVER_ERROR Init(VOID*);
	SERVER_ERROR UnInit();
	SERVER_ERROR Connect(const char* pServerName, const char* pPort, const char* pdbname, const char* pId, const char* pPassword);
	SERVER_ERROR Disconnect();
	SERVER_ERROR Reconnect() override;
	CVirtualDataUnit* GetDataUnit();
	SERVER_ERROR DelDataUnit(CVirtualDataUnit* pDataUnit);
	UINT GetFreeUnitCount() { return m_DataUnits.GetCount(); }
	BOOL GetIsstart() { return IsstartDB; }
	BOOL IsConnectionValid();
	MYSQL* GetConnectionHandle()const { return m_pMySQL; }
	std::string EscapeString(const char* pszInput) override;
	VOID ResetForPoolReuse();
private:
	CIndexList<CMySQLDataUnit, MAX_DATAUNIT > m_DataUnits;
	MYSQL* m_pMySQL{ nullptr };
	UINT m_Id{ 0 };
	BOOL IsstartDB{ FALSE };
	CVirtualDatabase* m_pDatabase{ nullptr };
	std::array<char, 256> m_szServerName{};
	std::array<char, 32> m_szPort{};
	std::array<char, 256> m_szDBName{};
	std::array<char, 256> m_szId{};
	std::array<char, 256> m_szPassword{};
};

class CMySQLDatabase :
	public CVirtualDatabase
{
public:
	CMySQLDatabase(VOID);
	virtual ~CMySQLDatabase(VOID);
	SERVER_ERROR Init();
	SERVER_ERROR UnInit();
	CVirtualDBConnection* GetConnection();
	SERVER_ERROR DelConnection(CVirtualDBConnection* pConnection);
private:
	CIndexList<CMySQLDBConnection, MAX_CONNECTION > m_DbConnections;
	MYSQL* m_pMySQL{ nullptr };
};