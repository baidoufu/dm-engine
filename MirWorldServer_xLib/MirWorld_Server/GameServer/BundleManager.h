#pragma once
#include <array>

typedef struct tag_bundleinfo
{
	tag_bundleinfo()
	{
		FILLSELF(0);
	}
	std::array<char, 20> szName{};
	std::array<char, 20> szExtractName{};
	int	count;
}BUNDLEINFO;

//捆绑物品管理
class CBundleManager : public xSingletonClass<CBundleManager>
{
public:
	CBundleManager(VOID);
	virtual ~CBundleManager(VOID);
	//加载捆绑物品配置BundleItem.csv
	VOID LoadBundle(const char* pszBundleFile);
	//获取捆绑物品信息
	BOOL GetBundleInfo(const char* pszName, char* pszExtractItemName, int& count);
private:
	CNameHash m_BundleNameHash;
};