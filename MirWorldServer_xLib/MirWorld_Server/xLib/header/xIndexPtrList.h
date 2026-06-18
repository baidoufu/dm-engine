#pragma once
#include <memory>
#include <vector>
#include <stack>
#include <unordered_map>

// ======================================================================================
// xIndexPtrList — 基于 STL 的高性能版本
// 使用 std::vector + std::unordered_map + std::stack 替换原始数组+线性查找
// ======================================================================================
template<class T>
class xIndexPtrList
{
public:
	xIndexPtrList(VOID)
	{
		m_iMax = 0;
		m_iArrayPtr = 0;
		m_nCount = 0;
	}
	virtual ~xIndexPtrList(VOID) { destroy(); }
	BOOL create(int nSize)
	{
		if (nSize > 0)
		{
			destroy();
			m_varArray.resize(nSize, nullptr);
			m_iMax = nSize;
			// 预留 hash map 空间以减少 rehash
			m_ptrToId.reserve(nSize);
			return TRUE;
		}
		return FALSE;
	}
	VOID destroy()
	{
		m_varArray.clear();
		m_varArray.shrink_to_fit();
		while (!m_freeIds.empty())
			m_freeIds.pop();
		m_ptrToId.clear();
		m_iMax = 0;
		m_iArrayPtr = 0;
		m_nCount = 0;
	}
	UINT addObject(T* pt)
	{
		if (pt == nullptr)return 0;
		auto it = m_ptrToId.find(pt);
		if (it != m_ptrToId.end())
			return it->second;

		UINT id = 0;
		if (m_iArrayPtr >= (UINT)m_iMax)
		{
			// 数组已满，从空闲栈中取一个回收的 ID
			if (m_freeIds.empty())
				return 0;
			id = m_freeIds.top();
			m_freeIds.pop();
		}
		else
		{
			id = ++m_iArrayPtr;
		}

		m_varArray[id - 1] = pt;
		m_ptrToId[pt] = id;
		m_nCount++;
		return id;
	}
	BOOL delObject(UINT id)
	{
		T* pt = getObject(id);
		if (pt == nullptr)return FALSE;

		m_varArray[id - 1] = nullptr;
		m_ptrToId.erase(pt);

		if (id == m_iArrayPtr)
			m_iArrayPtr--;
		else
			m_freeIds.push(id);

		m_nCount--;
		return TRUE;
	}
	BOOL delObject(T* pt)
	{
		auto it = m_ptrToId.find(pt);
		if (it == m_ptrToId.end())
			return FALSE;
		return delObject(it->second);
	}
	T* getObject(UINT id)
	{
		if (id == 0 || id > m_iMax)return nullptr;
		return m_varArray[id - 1];
	}
	UINT findObject(T* pt)
	{
		auto it = m_ptrToId.find(pt);
		if (it != m_ptrToId.end())
			return it->second;
		return 0;
	}
	UINT getCount() { return m_nCount; }
public:
	UINT getMax() { return m_iMax; }
	UINT getCurPtr() { return m_iArrayPtr; }
	const std::vector<T*>& getArray() const { return m_varArray; }
private:
	std::vector<T*> m_varArray;                   // 主存储：ID-1 为索引，O(1) 随机访问
	std::stack<UINT> m_freeIds;                   // 空闲 ID 栈，O(1) 回收复用
	std::unordered_map<T*, UINT> m_ptrToId;       // 反向索引：T* -> ID，O(1) 查找
	UINT m_iMax;
	UINT m_iArrayPtr;
	UINT m_nCount;
};


template<class T>
class xIndexPtrListHelper
{
public:
	xIndexPtrListHelper(xIndexPtrList<T>& list)
	{
		m_iPtr = 0;
		m_pIndexPtrList = &list;
	}
	xIndexPtrListHelper()
	{
		m_iPtr = 0;
		m_pIndexPtrList = nullptr;
	}
	~xIndexPtrListHelper()
	{
		m_iPtr = 0;
		m_pIndexPtrList = nullptr;
	}

	VOID setPtrList(xIndexPtrList<T>& list)
	{
		m_pIndexPtrList = &list;
	}

	T* first()
	{
		if (m_pIndexPtrList == nullptr) return nullptr;
		T* pt = nullptr;
		for (m_iPtr = 1; m_iPtr <= m_pIndexPtrList->getCurPtr(); m_iPtr++)
		{
			pt = m_pIndexPtrList->getObject(m_iPtr);
			if (pt)
				return pt;
		}
		return nullptr;
	}
	T* next()
	{
		if (m_pIndexPtrList == nullptr) return nullptr;
		T* pt = nullptr;
		m_iPtr++;
		for (; m_iPtr <= m_pIndexPtrList->getCurPtr(); m_iPtr++)
		{
			pt = m_pIndexPtrList->getObject(m_iPtr);
			if (pt)
				return pt;
		}
		return nullptr;
	}
protected:
	UINT m_iPtr;
	xIndexPtrList<T>* m_pIndexPtrList;
};
