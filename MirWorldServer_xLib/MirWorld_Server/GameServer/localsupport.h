#pragma	once

#include <array>
#include <charconv>
#include <string_view>
#include <vector>

#define	INDEXEDTABLE_SEGMENT_CACHE	2048
template<int MAXCOUNT>
class xIdAllocorEx
{
public:
	xIdAllocorEx() : m_nIdPtr(0), m_nCacheCount(0)
	{
		m_IdCache.fill(0);
	}
	UINT allocid()
	{
		if (m_nCacheCount > 0)
			return m_IdCache[--m_nCacheCount];
		return (++m_nIdPtr);
	}
	VOID freeid(UINT id)
	{
		if (id == m_nIdPtr)
			m_nIdPtr--;
		else if (m_nCacheCount < MAXCOUNT)
			m_IdCache[m_nCacheCount++] = id;
	}
	BOOL isused(UINT id)
	{
		return TRUE;
	}
private:
	std::array<UINT, MAXCOUNT> m_IdCache;
	UINT m_nCacheCount;
	UINT m_nIdPtr;
};

template<int MAXCOUNT>
class xIdAllocor
{
public:
	xIdAllocor() : m_iFree(0), m_iCount(0)
	{
		for (UINT i = 0; i < MAXCOUNT; i++)
		{
			m_NextFree[i] = i + 1;
		}
	}
	UINT allocid()
	{
		if (m_iFree >= MAXCOUNT) return 0;
		UINT id = m_iFree + 1;
		m_iFree = m_NextFree[m_iFree];
		m_NextFree[id - 1] = 0xffffffff;
		m_iCount++;
		return id;
	}
	VOID freeid(UINT id)
	{
		if (id == 0 || id > MAXCOUNT) return;
		id--;
		if (m_NextFree[id] != 0xffffffff) return;
		m_NextFree[id] = m_iFree;
		m_iFree = id;
		m_iCount--;
	}
	BOOL isused(UINT id)
	{
		return (id > 0 && id <= MAXCOUNT && m_NextFree[id - 1] == 0xffffffff);
	}
	UINT getcount() { return m_iCount; }
	BOOL isfull() { return (m_iCount >= MAXCOUNT); }
private:
	std::array<UINT, MAXCOUNT + 1> m_NextFree;
	UINT m_iFree;
	UINT m_iCount;
};

template<class T>
class xIndexedTable
{
	struct table_array
	{
		UINT nStartId{ 0 };
		xIdAllocor<INDEXEDTABLE_SEGMENT_CACHE> idallocor;
		std::array<T*, INDEXEDTABLE_SEGMENT_CACHE> array{};
		std::unique_ptr<table_array> pNext;
	};
public:
	xIndexedTable() : m_iCount(0) {}
	UINT add(T* pt)
	{
		table_array* pFreeArray = getfreearray();
		if (pFreeArray == nullptr) return 0;
		UINT id = pFreeArray->idallocor.allocid();
		if (id == 0) return 0;
		pFreeArray->array[id - 1] = pt;
		m_iCount++;
		return pFreeArray->nStartId + id;
	}
	T* get(UINT id)
	{
		table_array* p = getarray(id);
		if (p == nullptr) return nullptr;
		id -= p->nStartId;
		if (!p->idallocor.isused(id)) return nullptr;
		return p->array[id - 1];
	}
	VOID del(UINT id)
	{
		table_array* p = getarray(id);
		if (p == nullptr) return;
		id -= p->nStartId;
		if (!p->idallocor.isused(id)) return;
		p->array[id - 1] = nullptr;
		p->idallocor.freeid(id);
		m_iCount--;
	}
private:
	table_array* getarray(UINT id)
	{
		table_array* p = m_pArray.get();
		while (p)
		{
			if (id > p->nStartId)
			{
				return p;
			}
			p = p->pNext.get();
		}
		return nullptr;
	}
	table_array* getfreearray()
	{
		table_array* p = m_pArray.get();
		while (p)
		{
			if (!p->idallocor.isfull())
			{
				return p;
			}
			p = p->pNext.get();
		}
		auto newArray = std::make_unique<table_array>();
		if (m_pArray != nullptr)
			newArray->nStartId = m_pArray->nStartId + INDEXEDTABLE_SEGMENT_CACHE;
		else
			newArray->nStartId = 0;
		newArray->pNext = std::move(m_pArray);
		m_pArray = std::move(newArray);
		return m_pArray.get();
	}
	std::unique_ptr<table_array> m_pArray;
	int	m_iCount;
};


inline int getintegerinside(char* buffer, int begin, int end)
{
	char* p = buffer;

	char* pret = nullptr;
	int iret = 0;
	while (*p)
	{
		if (*p == begin)
			pret = p + 1;
		if (pret != nullptr && *p == end)
		{
			*p = 0;
			iret = StringToInteger(pret);
			*p = end;
			return iret;
		}
		p++;
	}
	return 0;
}

inline char* getstringinside(char* buffer, int begin, int end)
{
	char* p = buffer;
	char* pret = nullptr;
	while (*p)
	{
		if (*p == begin)
			pret = p + 1;
		if (pret != nullptr && *p == end)
		{
			*p = 0;
			return pret;
		}
		p++;
	}
	return nullptr;
}

inline char* gettoedge(char* buffer, int begin, int end)
{
	char* p = buffer;
	int level = 0;
	while (*p)
	{
		if (*p == begin)
		{
			level++;
		}
		else if (*p == end)
		{
			level--;
			if (level == 0)
				return (p + 1);
		}
		p++;
	}
	return nullptr;
}

inline int StringToStruct(const char* pszString, LPVOID lpStruct)
{
	std::array<char*, 200> Params{};
	std::array<char*, 100> subParams{};
	int nSubParam = 0;
	std::array<char, 2048> szBuffer{};
	char* pBinbuffer = static_cast<char*>(lpStruct);
	int binPtr = 0;
	o_strncpy(szBuffer.data(), pszString, 2047);
	int nParam = SearchParam(szBuffer.data(), Params.data(), 200, "|");

	bool bArray = false;
	int	arraysize = 0;

	for (int i = 0; i < nParam; i++)
	{
		bArray = false;
		arraysize = 1;
		char c = *Params[i]++;
		c = tolower(c);

		if (*Params[i] == '[')
		{
			bArray = true;
			arraysize = getintegerinside(Params[i], '[', ']');
			if (arraysize == 0) return 0;
			Params[i] = gettoedge(Params[i], '[', ']');
			if (Params[i] == nullptr) return 0;
		}
		if (*Params[i] == ':')
		{
			(*Params[i])++;
		}
		else
			return 0;

		if (c == 's')
		{
			strcpy(pBinbuffer + binPtr, Params[i]);
			if (bArray)
				binPtr += arraysize;
			else
				binPtr += static_cast<int>(strlen(Params[i]));
			continue;
		}

		int blocksize = 0;

		switch (c)
		{
		case 'w':
			blocksize = 2;
			break;
		case 'd':
			blocksize = 4;
			break;
		case 'b':
			blocksize = 1;
			break;
		default:
			blocksize = 4;
		}
		int value;

		if (bArray)
		{
			Params[i] = getstringinside(Params[i], '{', '}');
			if (Params[i] == nullptr)
				return 0;
			nSubParam = SearchParam(Params[i], subParams.data(), 100, ",");
			if (nSubParam != arraysize)
			{
				if (nSubParam == 0) return 0;
				arraysize = nSubParam;
			}
			for (int block = 0; block < arraysize; block++)
			{
				value = StringToInteger(subParams[block]);
				memcpy(pBinbuffer + binPtr, &value, blocksize);
				binPtr += blocksize;
			}
		}
		else
		{
			value = StringToInteger(Params[i]);
			memcpy(pBinbuffer + binPtr, &value, blocksize);
			binPtr += blocksize;
		}
	}
	return binPtr;
}

static int GetMsgFromString(std::string_view sv, char* pMsgBuffer)
{
	if (sv.empty()) return 0;

	MIRMSG header{};
	char* pMsg = pMsgBuffer;
	std::array<char, 1024> szBinBuffer{};
	int binPtr = 0;

	// 使用 string_view 按 '|' 分割，不修改原始字符串
	std::array<std::string_view, 200> tokens{};
	int nParam = 0;
	size_t pos = 0;
	while (pos <= sv.size() && nParam < 200)
	{
		auto next = sv.find('|', pos);
		auto token = (next == std::string_view::npos) ? sv.substr(pos) : sv.substr(pos, next - pos);
		// 去除首尾空白
		while (!token.empty() && (token.front() == ' ' || token.front() == '\t'))
			token.remove_prefix(1);
		while (!token.empty() && (token.back() == ' ' || token.back() == '\t'))
			token.remove_suffix(1);
		tokens[nParam++] = token;
		if (next == std::string_view::npos) break;
		pos = next + 1;
	}
	if (nParam < 5) return 0;

	auto svToInt = [](std::string_view sv) -> int {
		int result = 0;
		std::from_chars(sv.data(), sv.data() + sv.size(), result);
		return result;
	};

	header.dwFlag = static_cast<DWORD>(svToInt(tokens[0]));
	header.wCmd = static_cast<WORD>(svToInt(tokens[1]));
	header.wParam[0] = static_cast<WORD>(svToInt(tokens[2]));
	header.wParam[1] = static_cast<WORD>(svToInt(tokens[3]));
	header.wParam[2] = static_cast<WORD>(svToInt(tokens[4]));
	*pMsg++ = '#';
	pMsg += _CodeGameCode(reinterpret_cast<BYTE*>(&header), MSGHEADERSIZE, reinterpret_cast<BYTE*>(pMsg));

	for (int i = 5; i < nParam; i++)
	{
		auto token = tokens[i];
		if (token.size() >= 2 && token[1] == ':')
		{
			char typeChar = token[0];
			auto value = token.substr(2);
			switch (typeChar)
			{
			case 'w': case 'W':
				*reinterpret_cast<WORD*>(szBinBuffer.data() + binPtr) = static_cast<WORD>(svToInt(value));
				binPtr += 2;
				break;
			case 'D': case 'd':
				*reinterpret_cast<DWORD*>(szBinBuffer.data() + binPtr) = static_cast<DWORD>(svToInt(value));
				binPtr += 4;
				break;
			case 'B': case 'b':
				*reinterpret_cast<BYTE*>(szBinBuffer.data() + binPtr) = static_cast<BYTE>(svToInt(value));
				binPtr += 1;
				break;
			case 's': case 'S':
				memcpy(szBinBuffer.data() + binPtr, value.data(), value.size());
				binPtr += static_cast<int>(value.size());
				break;
			default:
				break;
			}
			continue;
		}
		// 无类型前缀，直接作为字符串
		memcpy(szBinBuffer.data() + binPtr, token.data(), token.size());
		binPtr += static_cast<int>(token.size());
	}
	if (binPtr > 0)
	{
		*(szBinBuffer.data() + binPtr) = 0;
		pMsg += _CodeGameCode(reinterpret_cast<BYTE*>(szBinBuffer.data()), binPtr, reinterpret_cast<BYTE*>(pMsg));
	}
	*pMsg++ = '!';
	return static_cast<int>(pMsg - pMsgBuffer);
}

template <class PT, UINT nIncreaseMeter>
class xDynamicPtrArray
{
public:
	xDynamicPtrArray() = default;
	~xDynamicPtrArray() = default;

	BOOL add(PT pt, UINT& nIndex)
	{
		nIndex = static_cast<UINT>(m_Array.size());
		m_Array.push_back(pt);
		return TRUE;
	}

	BOOL add(PT pt)
	{
		m_Array.push_back(pt);
		return TRUE;
	}

	BOOL insert(UINT nIndex, PT pt)
	{
		if (nIndex >= m_Array.size())
			nIndex = static_cast<UINT>(m_Array.size());
		m_Array.insert(m_Array.begin() + nIndex, pt);
		return TRUE;
	}

	VOID remove(UINT nIndex)
	{
		if (nIndex >= m_Array.size()) return;
		m_Array.erase(m_Array.begin() + nIndex);
	}

	PT& operator[](UINT nIndex)
	{
		static PT empty{};
		if (nIndex >= m_Array.size()) return empty;
		return m_Array[nIndex];
	}

	UINT getCount() const { return static_cast<UINT>(m_Array.size()); }

private:
	std::vector<PT> m_Array;
};

template <class T>
class xKeyPtrList
{
	struct KeyObj
	{
		KeyObj() = default;
		DWORD dwKey{ 0 };
		T* pObject{ nullptr };
	};
public:
	xKeyPtrList() = default;
	~xKeyPtrList() = default;

	BOOL addObject(DWORD dwKey, T* pObject)
	{
		UINT index = getFitableIndex(dwKey);
		KeyObj obj{};
		obj.pObject = pObject;
		obj.dwKey = dwKey;
		return m_xObjectArray.insert(index, obj);
	}

	T* getObject(DWORD dwKey)
	{
		UINT index = getFitableIndex(dwKey);
		if (index < m_xObjectArray.getCount() && m_xObjectArray[index].dwKey == dwKey)
			return m_xObjectArray[index].pObject;
		return nullptr;
	}

	DWORD getObjectKey(T* pObject)
	{
		UINT index = getObjectIndex(pObject);
		if (index == static_cast<UINT>(-1)) return 0;
		return m_xObjectArray[index].dwKey;
	}

	BOOL removeObject(DWORD dwKey)
	{
		UINT index = getFitableIndex(dwKey);
		if (index >= m_xObjectArray.getCount() || m_xObjectArray[index].dwKey != dwKey) return FALSE;
		m_xObjectArray.remove(index);
		return TRUE;
	}

	BOOL removeObject(T* pObject)
	{
		UINT index = getObjectIndex(pObject);
		if (index == static_cast<UINT>(-1)) return FALSE;
		m_xObjectArray.remove(index);
		return TRUE;
	}

protected:
	UINT getObjectIndex(T* pObject)
	{
		for (UINT i = 0; i < m_xObjectArray.getCount(); i++)
		{
			if (pObject == m_xObjectArray[i].pObject)
				return i;
		}
		return static_cast<UINT>(-1);
	}

	UINT getFitableIndex(DWORD dwKey)
	{
		UINT s = 0, e = m_xObjectArray.getCount();
		if (e == 0) return 0;
		while (s < e)
		{
			UINT m = (s + e) / 2;
			if (dwKey > m_xObjectArray[m].dwKey)
				s = m + 1;
			else
				e = m;
		}
		return s;
	}

	xDynamicPtrArray<KeyObj, 512> m_xObjectArray;
};

inline int GetFlyDirection(int sx, int sy, int ttx, int tty)
{
	int dx = ttx - sx;
	int dy = tty - sy;

	if (dx == 0) return (dy > 0) ? 4 : 0;
	if (dy == 0) return (dx > 0) ? 2 : 6;

	int absDx = abs(dx);
	int absDy = abs(dy);

	if (absDx > absDy * 3) return (dx > 0) ? 2 : 6;
	if (absDy > absDx * 3) return (dy > 0) ? 4 : 0;

	if (dx > 0) return (dy > 0) ? 3 : 1;
	else return (dy > 0) ? 5 : 7;
}

inline int countStringChar(const char* pstring, int ichar)
{
	char* p = const_cast<char*>(pstring);
	int c = 0;
	while (*p)
	{
		if (*p == ichar) c++;
		p++;
	}
	return c;
}

inline VOID replaceOutPair(char* pszString, int PairL, int PairR, int ReplaceTo)
{
	char* p = pszString;
	int	PairLevel = 0;
	int tLevel = 0;
	BOOL	bErase = FALSE;
	while (*p)
	{
		if (*p == ' ' || *p == '\t')
		{
			if (tLevel == 0)
				tLevel = 1;
		}
		else if (*p == PairL)
		{
			PairLevel++;
			if (PairLevel == 1 && tLevel != 2)
			{
				*p = ReplaceTo;
				bErase = TRUE;
			}
		}
		else if (*p == PairR)
		{
			if (PairLevel == 1 && bErase)
				*p = ReplaceTo;
			PairLevel--;
		}
		else
		{
			if (tLevel == 1)
				tLevel = 2;
		}
		p++;
	}
}

inline static BOOL InRect(int x, int y, RECT& rc)
{
	if (x < rc.left || x > rc.right) return FALSE;
	if (y < rc.top || y > rc.bottom) return FALSE;
	return TRUE;
}

inline int FilterItemShowName(char* sName)
{
	if (!sName) return 0;
	int len = 0;
	while (len < 256 && sName[len] != '\0')
		len++;
	if (len == 0) return 0;
	int pos = len - 1;
	while (pos >= 0)
	{
		if (sName[pos] < '0' || sName[pos] > '9')
			break;
		if (pos == 0)
		{
			sName[0] = '\0';
			return 0;
		}
		pos--;
	}
	if (pos >= 0)
	{
		sName[pos + 1] = '\0';
		return pos + 1;
	}
	return 0;
}

inline VOID CharToBytes(const char* str, BYTE* outBytes, int& outCount)
{
	if (str == nullptr || outBytes == nullptr)
	{
		outCount = 0;
		return;
	}
	int len = static_cast<int>(strlen(str));
	outCount = MIN(len, 256);
	for (int i = 0; i < outCount; i++)
	{
		outBytes[i] = static_cast<BYTE>(str[i] & 0xff);
	}
}

inline VOID CharToBytesAndPrint(const char* str, BYTE* outBytes, int& outCount)
{
	CharToBytes(str, outBytes, outCount);
	LG1("[");
	for (int i = 0; i < outCount; i++)
	{
		LG1("%d,", outBytes[i]);
	}
	LG1("]\n\n");
}

inline WORD GetGoldImageIndex(DWORD dwCount)
{
	static const std::array<DWORD, 5> thresholds{ 100, 300, 500, 1000, UINT_MAX };
	static const std::array<WORD, 5> images{ 0xe1, 0xe2, 0xe3, 0xe4, 0xe5 };
	auto it = std::lower_bound(thresholds.begin(), thresholds.end(), dwCount);
	return images[std::distance(thresholds.begin(), it)];
}

inline bool IsIntegerNumber(const char* str)
{
	if (str == nullptr || *str == '\0') return false;
	int i = 0;
	if (str[i] == '-' || str[i] == '+')
	{
		i++;
		if (str[i] == '\0') return false;
	}
	while (str[i] != '\0')
	{
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (!isdigit(c)) return false;
		i++;
	}
	return true;
}

inline int IsNextDay(time_t timestamp1, time_t timestamp2)
{
	if (timestamp1 == 0 || timestamp2 == 0)
		return 1;
	time_t days1 = timestamp1 / ONE_DAY_SECONDS;
	time_t days2 = timestamp2 / ONE_DAY_SECONDS;
	return static_cast<int>(abs(days2 - days1));
}

struct SquareArea
{
	int centerX = 0, centerY = 0;
	int sideLength = 0;
	std::vector<std::pair<int, int>> outerSquare;
	std::vector<std::pair<int, int>> innerSquare;
};

inline SquareArea CalculateSquareArea(int centerX, int centerY)
{
	SquareArea area;
	area.centerX = centerX;
	area.centerY = centerY;
	area.sideLength = 5;

	area.outerSquare.reserve(16);
	for (int x = centerX - 2; x <= centerX + 2; x++)
		area.outerSquare.emplace_back(x, centerY - 2);
	for (int y = centerY - 1; y <= centerY + 2; y++)
		area.outerSquare.emplace_back(centerX + 2, y);
	for (int x = centerX + 1; x >= centerX - 2; x--)
		area.outerSquare.emplace_back(x, centerY + 2);
	for (int y = centerY + 1; y >= centerY - 1; y--)
		area.outerSquare.emplace_back(centerX - 2, y);

	area.innerSquare.reserve(9);
	for (int y = centerY - 1; y <= centerY + 1; y++)
	{
		for (int x = centerX - 1; x <= centerX + 1; x++)
		{
			area.innerSquare.emplace_back(x, y);
		}
	}

	return area;
}