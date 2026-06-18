#pragma once
class CScriptFile
{
public:
	CScriptFile(VOID);
	virtual ~CScriptFile(VOID);
	BOOL Load(const char* pszFileName);
	VOID Close();
	char* FirstLine();
	char* NextLine();
	char* PrevLine();
	char* CurrentLine();
	char* CurrentLineRaw();
	UINT GetCurrentLineNumber();
	VOID SetLineIndex(UINT index)
	{
		m_nLineIndex = index;
	}
	const char* GetFileName() { return m_szFileName.data(); }
protected:
	BOOL ProtectedLoad();
	std::array<char, 1024> m_szFileName;
	UINT m_nLineIndex;
	CStringFile m_fScript;
	std::unique_ptr<char[]> m_pData;
};