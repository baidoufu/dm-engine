#pragma once
#include <memory>
#include <vector>

enum fmt_type
{
	FT_STRING,
	FT_BYTE,
	FT_WORD,
	FT_DWORD,
};

struct fmt_struct_element
{
	fmt_struct_element() : wType(0), wLength(0) {}
	WORD wType;
	WORD wLength;
	std::unique_ptr<fmt_struct_element> pNext;
};

class CFmtTextFile
{
public:
	CFmtTextFile(const char* pszStruct, const char* pszFile = nullptr, BOOL bCSV = FALSE);
	virtual ~CFmtTextFile(VOID) = default;
	BOOL Load(const char* pszFile, BOOL bCSV = FALSE);
	int	GetCount() { return m_sfFile.GetLineCount(); }
	BOOL GetStruct(int line, LPVOID lpStruct);
protected:
	std::array<char, 1024> m_szStruct{};
	BOOL m_bLoaded{ FALSE };
	CStringFile m_sfFile;
	BOOL m_bCSV{ FALSE };
	std::unique_ptr<fmt_struct_element> m_pStructDesc;
	int	m_iStructElements{ 0 };
	int	m_iMaxParams{ 0 };
	std::vector<char*> Params;
	int	nParam{ 0 };
};