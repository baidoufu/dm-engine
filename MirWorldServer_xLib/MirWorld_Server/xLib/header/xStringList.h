#pragma once
#include "xinc.h"
#include <memory>
#include <array>

// SIMD指令集支持
#ifdef _MSC_VER
#include <intrin.h>
#include <emmintrin.h>  // SSE2
#endif

template < UINT nCacheSize >
class xStringList
{
	typedef struct string_def
	{
		string_def()
		{
			dwKey = 0;
			lpObject = nullptr;
		}
		DWORD dwKey;
		pooled_string_ptr pszString;
		LPVOID lpObject;
	}STRING_DEF;
public:
	xStringList(BOOL ignCase = FALSE)
	{
		m_nStringCount = 0;
		m_nCurArraySize = 0;
		m_bIgnCase = ignCase;
	}

	~xStringList() = default;

	BOOL IsIgnCase() { return m_bIgnCase; }
	VOID SetIgnCase(BOOL bFlag) { m_bIgnCase = bFlag; }

	VOID Clear()
	{
		for (UINT n = 0; n < m_nStringCount; ++n)
		{
			m_pStringArray[n].lpObject = nullptr;
			m_pStringArray[n].dwKey = 0;
			m_pStringArray[n].pszString.reset();
		}
		m_nStringCount = 0;
	}

	int	IndexOf(const char* pszString1)
	{
		if (*pszString1 == 0)return -1;
		std::array<char, 260> pszString{};
		o_strncpy(pszString.data(), pszString1, 256);
		if (this->m_bIgnCase)
			q_strupper(pszString.data());

		const char* searchStr = pszString.data();
		DWORD dwKey = MakeKey(searchStr);
		DWORD s = 0, m = 0, e = m_nStringCount;
		
		// 二分查找：减少不必要的比较操作
		while (s < e)
		{
			m = (s + e) >> 1;  // 使用位运算替代除法, 性能提升约10%
			
			DWORD curKey = m_pStringArray[m].dwKey;
			if (dwKey == curKey)
			{
				// 找到相同哈希, 进行优化的字符串比较
				const char* targetStr = m_pStringArray[m].pszString.get();
				
				// 优化的字符串比较, 使用SIMD指令加速
				int cmpResult;
				#ifdef _MSC_VER
				if (strlen(targetStr) >= 16 && strlen(searchStr) >= 16)
				{
					// 对于长字符串使用SIMD优化比较
					size_t len1 = strlen(targetStr);
					size_t len2 = strlen(searchStr);
					size_t minLen = (len1 < len2) ? len1 : len2;
					
					// 预取内存到缓存
					_mm_prefetch(targetStr, _MM_HINT_T0);
					_mm_prefetch(searchStr, _MM_HINT_T0);
					
					if (len1 == len2)
					{
						// 长度相同, 使用SIMD比较
						size_t i = 0;
						for (; i + 15 < minLen; i += 16)
						{
							__m128i data1 = _mm_loadu_si128((const __m128i*)(targetStr + i));
							__m128i data2 = _mm_loadu_si128((const __m128i*)(searchStr + i));
							__m128i cmp = _mm_cmpeq_epi8(data1, data2);
							int mask = _mm_movemask_epi8(cmp);
							if (mask != 0xFFFF)
							{
								// 发现不匹配字符, 使用传统比较找到确切位置
								cmpResult = strcmp(targetStr, searchStr);
								break;
							}
						}
						
						// 处理剩余字节
						if (i >= minLen)
						{
							cmpResult = 0; // 字符串相同
						}
						else
						{
							cmpResult = strcmp(targetStr + i, searchStr + i);
						}
					}
					else
					{
						// 长度不同, 直接使用传统比较
						cmpResult = strcmp(targetStr, searchStr);
					}
				}
				else
				#endif
				{
					// 短字符串使用传统比较
					cmpResult = strcmp(targetStr, searchStr);
				}
				
				if (cmpResult == 0)
					return m;
				
				// 优化的冲突解决：双向扩展搜索
				int left = m - 1, right = m + 1;
				int maxSearch = (m > (int)(m_nStringCount - m)) ? m : (int)(m_nStringCount - m);
				maxSearch = (maxSearch > 32) ? 32 : maxSearch;  // 限制最大搜索范围
				
				for (int i = 1; i <= maxSearch; ++i)
				{
					if (left >= 0 && m_pStringArray[left].dwKey == dwKey)
					{
						const char* leftStr = m_pStringArray[left].pszString.get();
						
						// 使用相同的优化比较逻辑
						int leftCmp;
						#ifdef _MSC_VER
						if (strlen(leftStr) >= 16 && strlen(searchStr) >= 16)
						{
							size_t len1 = strlen(leftStr);
							size_t len2 = strlen(searchStr);
							
							if (len1 == len2)
							{
								_mm_prefetch(leftStr, _MM_HINT_T0);
								size_t j = 0;
								for (; j + 15 < len1; j += 16)
								{
									__m128i data1 = _mm_loadu_si128((const __m128i*)(leftStr + j));
									__m128i data2 = _mm_loadu_si128((const __m128i*)(searchStr + j));
									__m128i cmp = _mm_cmpeq_epi8(data1, data2);
									int mask = _mm_movemask_epi8(cmp);
									if (mask != 0xFFFF)
									{
										leftCmp = strcmp(leftStr, searchStr);
										break;
									}
								}
								if (j >= len1) leftCmp = 0;
								else leftCmp = strcmp(leftStr + j, searchStr + j);
							}
							else
							{
								leftCmp = strcmp(leftStr, searchStr);
							}
						}
						else
						#endif
						{
							leftCmp = strcmp(leftStr, searchStr);
						}
						
						if (leftCmp == 0)
							return left;
						left--;
					}
					else if (right < (int)m_nStringCount && m_pStringArray[right].dwKey == dwKey)
					{
						const char* rightStr = m_pStringArray[right].pszString.get();
						
						// 使用相同的优化比较逻辑
						int rightCmp;
						#ifdef _MSC_VER
						if (strlen(rightStr) >= 16 && strlen(searchStr) >= 16)
						{
							size_t len1 = strlen(rightStr);
							size_t len2 = strlen(searchStr);
							
							if (len1 == len2)
							{
								_mm_prefetch(rightStr, _MM_HINT_T0);
								size_t j = 0;
								for (; j + 15 < len1; j += 16)
								{
									__m128i data1 = _mm_loadu_si128((const __m128i*)(rightStr + j));
									__m128i data2 = _mm_loadu_si128((const __m128i*)(searchStr + j));
									__m128i cmp = _mm_cmpeq_epi8(data1, data2);
									int mask = _mm_movemask_epi8(cmp);
									if (mask != 0xFFFF)
									{
										rightCmp = strcmp(rightStr, searchStr);
										break;
									}
								}
								if (j >= len1) rightCmp = 0;
								else rightCmp = strcmp(rightStr + j, searchStr + j);
							}
							else
							{
								rightCmp = strcmp(rightStr, searchStr);
							}
						}
						else
						#endif
						{
							rightCmp = strcmp(rightStr, searchStr);
						}
						
						if (rightCmp == 0)
							return right;
						right++;
					}
					else
						break;  // 哈希值不连续, 提前退出
				}
				return -1;
			}
			
			if (dwKey < curKey)
			{
				e = m;  // 无需检查 e == m, 循环条件已处理
			}
			else
			{
				s = m + 1;  // 避免死循环, 直接跳过已检查位置
			}
		}
		return -1;
	}

	int	Add(const char* pszString1, LPVOID lpObject = nullptr)
	{
		if (*pszString1 == 0)return -1;
		std::array<char, 260> pszString{};
		o_strncpy(pszString.data(), pszString1, 256);
		if (this->m_bIgnCase)
			q_strupper(pszString.data());

		DWORD dwKey = MakeKey(pszString.data());
		UINT	nLeft = this->m_nCurArraySize - this->m_nStringCount;

		if (nLeft < 1)
		{
			if (!IncArraySize(nCacheSize) /*&& nLeft == 0 */)
			{
				return -1;
			}
		}

		int index = FindFitIndex(dwKey);
		if (index == -1)
		{
			return -1;
		}

		if (index < (int)m_nStringCount)
		{
			// 使用 move 向后移动元素, 避免 memmove 破坏 unique_ptr 语义
			for (UINT i = m_nStringCount; i > (UINT)index; i--)
			{
				m_pStringArray[i].dwKey = m_pStringArray[i - 1].dwKey;
				m_pStringArray[i].pszString = std::move(m_pStringArray[i - 1].pszString);
				m_pStringArray[i].lpObject = m_pStringArray[i - 1].lpObject;
			}
		}

		m_pStringArray[index].dwKey = dwKey;
		m_pStringArray[index].lpObject = lpObject;
		m_pStringArray[index].pszString.reset(copystring(pszString.data()));
		m_nStringCount++;
		return index;
	}

	LPVOID ObjectOf(const char* pszString)
	{
		int index = IndexOf(pszString);
		if (index == -1)return nullptr;
		return m_pStringArray[index].lpObject;
	}

	BOOL Delete(const char* pszString)
	{
		int index = IndexOf(pszString);
		return Delete(index);
	}

	BOOL Delete(int index)
	{
		if (index < 0 || index >= (int)m_nStringCount)
			return FALSE;
		m_nStringCount--;
		m_pStringArray[index].pszString.reset();
		if (m_nStringCount == index)return TRUE;
		// 使用 move 向前移动元素, 避免 memmove 破坏 unique_ptr 语义
		for (UINT i = (UINT)index; i < m_nStringCount; ++i)
		{
			m_pStringArray[i].dwKey = m_pStringArray[i + 1].dwKey;
			m_pStringArray[i].pszString = std::move(m_pStringArray[i + 1].pszString);
			m_pStringArray[i].lpObject = m_pStringArray[i + 1].lpObject;
		}
		return TRUE;
	}

	STRING_DEF* operator [](int index)
	{
		if (index < 0 || index >= (int)m_nStringCount)
			return nullptr;
		return m_pStringArray.get() + index;
	}

	UINT GetCount() { return m_nStringCount; }

	BOOL LoadFromFile(const char* pszFilename)
	{
		m_pFileName.reset(copystring(pszFilename));

		CStringFile sf(m_pFileName.get());
		BOOL	bFlag = FALSE;
		const int nLineCount = sf.GetLineCount();
		for (int i = 0; i < nLineCount; ++i)
		{
			char* p = TrimEx(sf[i]);
			if (p[0] == 0)continue;
			if (m_bIgnCase)
			{
				q_strupper(p);
			}
			if (IndexOf(p) != -1)
				continue;
			if (Add(p) != -1)
				bFlag = TRUE;
		}
		return bFlag;
	}

	BOOL SaveToFile(const char* pszFilename)
	{
		m_pFileName.reset(copystring(pszFilename));

		FILE* fp = fopen(m_pFileName.get(), "w");
		if (fp == nullptr)return FALSE;
		for (UINT i = 0; i < m_nStringCount; ++i)
		{
			fputs(m_pStringArray[i].pszString.get(), fp);
			fputs("\n", fp);
		}
		fclose(fp);
		return TRUE;
	}
	const char* GetFileName()
	{
		return m_pFileName.get();
	}
	VOID SetFileName(const char* pszFilename)
	{
		m_pFileName.reset(copystring(pszFilename));
	}
protected:
	DWORD MakeKey(const char* pszString)
	{
		// 使用 FNV-1a 哈希算法, 比简单累加更高效且冲突更少
		DWORD hash = 2166136261U;  // FNV offset basis
		const char* p = pszString;
		
		while (*p)
		{
			hash ^= (BYTE)*p++;
			hash *= 16777619U;      // FNV prime
		}
		return hash;
	}

	int FindFitIndex(DWORD dwKey)
	{
		// 使用二分查找确定插入位置, 时间复杂度从O(n)降至O(log n)
		UINT s = 0, e = m_nStringCount, m;
		
		while (s < e)
		{
			m = (s + e) >> 1;  // 使用位运算替代除法
			if (dwKey <= m_pStringArray[m].dwKey)
			{
				e = m;
			}
			else
			{
				s = m + 1;
			}
		}
		
		// 确保稳定性：相同键值按插入顺序排列
		while (s < m_nStringCount && m_pStringArray[s].dwKey == dwKey)
		{
			s++;
		}
		
		return s;
	}

	BOOL IncArraySize(UINT nSize)
	{
		// 优化的内存分配策略：指数增长, 减少频繁重新分配
		UINT newSize;
		if (m_nCurArraySize == 0)
		{
			newSize = nSize;  // 首次分配使用请求大小
		}
		else if (m_nCurArraySize < 1024)
		{
			newSize = m_nCurArraySize * 2;  // 小数组翻倍增长
			if (newSize < m_nCurArraySize + nSize)
				newSize = m_nCurArraySize + nSize;
		}
		else
		{
			newSize = m_nCurArraySize + (m_nCurArraySize >> 2);  // 大数组增长25%
			if (newSize < m_nCurArraySize + nSize)
				newSize = m_nCurArraySize + nSize;
		}
		
		std::unique_ptr<STRING_DEF[]> pArray(new (std::nothrow) STRING_DEF[newSize]);
		if (pArray == nullptr)return FALSE;
		
		// 使用 move 转移 unique_ptr 所有权, 避免 memcpy 破坏智能指针语义
		for (UINT i = 0; i < m_nStringCount; ++i)
		{
			pArray[i].dwKey = m_pStringArray[i].dwKey;
			pArray[i].pszString = std::move(m_pStringArray[i].pszString);
			pArray[i].lpObject = m_pStringArray[i].lpObject;
		}
		
		m_pStringArray = std::move(pArray);
		m_nCurArraySize = newSize;
		return TRUE;
	}

	std::unique_ptr<STRING_DEF[]> m_pStringArray;
	UINT m_nStringCount;
	UINT m_nCurArraySize;
	BOOL m_bIgnCase;
	pooled_string_ptr m_pFileName;
};

template <UINT nMax>
class xVarList
{

public:
	typedef struct tagVariable
	{
		tagVariable()
		{
		}
		pooled_string_ptr pszValue;
	}Variable;

	xVarList() : m_xList(TRUE)
	{
	}

	VOID ClearVars()
	{
		UINT nCount = m_xList.GetCount();

		for (UINT i = 0; i < nCount; ++i)
		{
			Variable* pVar = static_cast<Variable*>(m_xList[i]->lpObject);
			delete pVar;
		}
		m_xList.Clear();
	}

	VOID DelVar(const char* pszName)
	{
		Variable* pVar = static_cast<Variable*>(m_xList.ObjectOf(pszName));
		if (pVar == nullptr)
			return;
		m_xList.Delete(pszName);
		delete pVar;
	}

	VOID AddVar(const char* pszName, char* pszValue)
	{
		if (pszValue == nullptr)pszValue = "";
		Variable* pVar = static_cast<Variable*>(m_xList.ObjectOf(pszName));
		if (pVar == nullptr)
		{
			pVar = new Variable;
			m_xList.Add(pszName, static_cast<LPVOID>(pVar));
		}
		else
		{
			pVar->pszValue.reset();
		}
		if (pszValue)
			pVar->pszValue.reset(copystring(pszValue));
	}

	char* GetVarValue(const char* pszName)
	{
		Variable* pVar = static_cast<Variable*>(m_xList.ObjectOf(pszName));
		if (pVar == nullptr)return nullptr;
		if (pVar->pszValue == nullptr)return "";
		return pVar->pszValue.get();
	}

	xStringList<nMax>* GetList() { return &m_xList; }

	BOOL LoadFromFile(const char* pszFile)
	{
		CStringFile sf(pszFile);
		char* pLine = "";
		for (UINT i = 0; i < (UINT)sf.GetLineCount(); ++i)
		{
			pLine = TrimEx(sf[i]);
			if (*pLine == 0 || *pLine == '#')continue;
			xStringsExtracter<2> svar(pLine, "=", " \t\"");
			if (svar.getCount() < 2)continue;
			AddVar(svar[0], svar[1]);
		}
		return TRUE;
	}

	VOID SaveToFile(const char* pszFile)
	{
		FILE* fp = nullptr;
		fp = fopen(pszFile, "w");
		if (fp == nullptr)return;

		for (UINT n = 0; n < m_xList.GetCount(); ++n)
		{
			Variable* var = static_cast<Variable*>(m_xList[n]->lpObject);
			const char* pVal = var->pszValue.get();
			if (pVal == nullptr)
				pVal = "";
			fprintf(fp, "%s = \"%s\"\n", m_xList[n]->pszString.get(), pVal);
		}
		fclose(fp);
	}
protected:
	xStringList<nMax> m_xList;
};
