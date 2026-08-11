#include "StdAfx.h"
#include <unordered_map>
#include ".\magicmanager.h"
#include ".\stringlistmanager.h"
#include "fmttextfile.h"
#include "server.h"
#include <vector>
#include "tinyxml.h"
#include "humanplayermgr.h"

xObjectPool<xListHost<MAGICCLASS>::xListNode> CMagicManager::m_xMagicNodePool;
CMagicManager::CMagicManager(VOID)
{
	m_pMagicArray.fill(nullptr);
}

CMagicManager::~CMagicManager(VOID)
{
}

VOID CMagicManager::ClearMagicData()
{
	magics.clear();
	xListHost<MAGICCLASS>::xListNode* pNode = m_xMagicClassList.getHead();
	while (pNode != nullptr)
	{
		xListHost<MAGICCLASS>::xListNode* pNext = pNode->getNext();
		MAGICCLASS* pObject = pNode->getObject();
		if (pObject != nullptr)
		{
			delete pObject;
		}
		m_xMagicNodePool.deleteObject(pNode);
		pNode = pNext;
	}
	m_xMagicClassList.~xListHost<MAGICCLASS>();
	new (&m_xMagicClassList) xListHost<MAGICCLASS>();

	m_MagicClassHash.Clear();
	m_pMagicArray.fill(nullptr);
}

VOID CMagicManager::ReloadAllPlayerSkills()const
{
	CIndexListEx<CHumanPlayer>* pPlayerList = CHumanPlayerMgr::GetInstance()->GetPlayerList();
	if (pPlayerList == nullptr || pPlayerList->GetCount() == 0) return;
	CHumanPlayer* pPlayer = pPlayerList->First();
	while (pPlayer != nullptr)
	{
		for (auto& up : pPlayer->GetMagics())
		{
			if (up->magic.wId != 0)
			{
				MAGICCLASS* pNewClass = GetClassById(up->magic.wId);
				if (pNewClass != nullptr)
					up->pClass = pNewClass;
			}
		}
		pPlayer = pPlayerList->Next();
	}
}

VOID CMagicManager::LoadMaigc(const char* pszMagicFile)
{
	CStringFile sfMagic(pszMagicFile);
	for (int i = 0; i < sfMagic.GetLineCount(); i++)
	{
		if (*sfMagic[i] != '#')
		{
			if (!AddMagicClassString(sfMagic[i]))
			{
				PRINT(ERROR_RED, "无法解析的物品类信息: %s\n错误: %s\n", sfMagic[i], "nullptr");
			}
		}
	}
}

BOOL CMagicManager::AddMagicClassString(char* pszMagicClassDesc)
{
	char* Params[26];
	int nParam = SearchParam(pszMagicClassDesc, Params, 26, ",");
	//#name/id/job/effecttype/effectvalue/
	// needlv1/lv1exp/needlv2/lv2exp/needlv3/lv3exp/needlv4/lv4exp/needlv5/lv5exp/needlv6/lv6exp/needlv7/lv7exp/needlv8/lv8exp/
	// spl/dspl/delay/needmagic/mutexmagic
	if (nParam < 24) return FALSE;

	MAGICCLASS class1;
	MAGICCLASS* pClass = &class1;
	o_strncpy(pClass->szName, Params[0], 19);
	pClass->id = (DWORD)StringToInteger(Params[1]);
	pClass->btJob = (BYTE)StringToInteger(Params[2]);
	pClass->btEffectType = (BYTE)StringToInteger(Params[3]);
	pClass->btEffectValue = (BYTE)StringToInteger(Params[4]);
	pClass->btNeedLv[0] = (BYTE)StringToInteger(Params[5]);
	pClass->dwNeedExp[0] = (DWORD)StringToInteger(Params[6]);
	pClass->btNeedLv[1] = (BYTE)StringToInteger(Params[7]);
	pClass->dwNeedExp[1] = (DWORD)StringToInteger(Params[8]);
	pClass->btNeedLv[2] = (BYTE)StringToInteger(Params[9]);
	pClass->dwNeedExp[2] = (DWORD)StringToInteger(Params[10]);
	pClass->btNeedLv[3] = (BYTE)StringToInteger(Params[11]);
	pClass->dwNeedExp[3] = (DWORD)StringToInteger(Params[12]);

	pClass->btNeedLv[4] = (BYTE)StringToInteger(Params[13]);
	pClass->dwNeedExp[4] = (DWORD)StringToInteger(Params[14]);
	pClass->btNeedLv[5] = (BYTE)StringToInteger(Params[15]);
	pClass->dwNeedExp[5] = (DWORD)StringToInteger(Params[16]);

	pClass->btNeedLv[6] = (BYTE)StringToInteger(Params[17]);
	pClass->dwNeedExp[6] = (DWORD)StringToInteger(Params[18]);
	pClass->btNeedLv[7] = (BYTE)StringToInteger(Params[19]);
	pClass->dwNeedExp[7] = (DWORD)StringToInteger(Params[20]);
	pClass->btNeedLv[8] = pClass->btNeedLv[7];
	pClass->dwNeedExp[8] = pClass->dwNeedExp[7];

	pClass->sSpell = (WORD)StringToInteger(Params[21]);
	pClass->sDefSpell = (WORD)StringToInteger(Params[22]);
	pClass->wDelay = (WORD)StringToInteger(Params[23]);

	if (nParam > 24)
	{
		if (strchr(Params[24], '|') != nullptr)
		{
			xStringsExtracter<3> needmagic(Params[24], "|");
			for (UINT n = 0; n < needmagic.getCount(); n++)
			{
				pClass->wNeedMagic[n] = (WORD)StringToInteger(needmagic[n]);
			}
		}
		else 
			pClass->wNeedMagic[0] = (WORD)StringToInteger(Params[24]);
	}

	if (nParam > 25)
	{
		if (strchr(Params[25], '|') != nullptr)
		{
			xStringsExtracter<3>	mutexmagic(Params[25], "|");
			for (UINT n = 0; n < mutexmagic.getCount(); n++)
			{
				pClass->wMutexMagic[n] = (WORD)StringToInteger(mutexmagic[n]);
			}
		}
		else 
			pClass->wMutexMagic[0] = (WORD)StringToInteger(Params[25]);
	}
	if (!AddMagicClass(pClass))
	{
		delete pClass;
		return FALSE;
	}
	return TRUE;
}

VOID CMagicManager::LoadMaigcskill(const char* pszMagicFile)
{
	TiXmlDocument doc(pszMagicFile);
	if (doc.LoadFile(TIXML_ENCODING_UNKNOWN))
	{
		TiXmlElement* root = doc.FirstChildElement("Skill");
		if (root)
		{
			for (TiXmlElement* magicElem = root->FirstChildElement("Magic"); magicElem; magicElem = magicElem->NextSiblingElement("Magic"))
			{
				Magic magic;
				magicElem->QueryIntAttribute("id", &magic.id);
				for (TiXmlElement* skillElem = magicElem->FirstChildElement("Skill"); skillElem; skillElem = skillElem->NextSiblingElement("Skill"))
				{
					Skill skill;
					std::memset(&skill, 0, sizeof(skill));
					switch (magic.id)
					{
					case 16://只有1个Value
					case 18:
					case 19:
					case 21:
					case 77:
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
					}
					break;
					case 1://只有2个Value
					case 5:
					case 8:
					case 11:
					case 13:
					case 14:
					case 15:
					case 32:
					case 35:
					case 47:
					case 48:
					case 49:
					case 67:
					case 71:
					case 72:
					case 75:
					case 76:
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
					}
					break;
					case 2://只有3个Value
					case 3:
					case 4:
					case 6:
					case 7:
					case 9:
					case 10:
					case 12:
					case 23:
					case 24:
					case 25:
					case 26:
					case 33:
					case 42:
					case 43:
					case 44:
					case 45:
					case 50:
					case 51:
					case 52:
					case 53:
					case 59:
					case 60:
					case 107:
					case 115:
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
						skillElem->QueryIntAttribute("Value3", &skill.value3);
					}
					break;
					case 22://只有4个Value
					case 29:
					case 37:
					case 40:
					case 41:
					case 65:
					case 74:
					case 111:
					case 114:
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
						skillElem->QueryIntAttribute("Value3", &skill.value3);
						skillElem->QueryIntAttribute("Value4", &skill.value4);
					}
					break;
					case 20:
					case 27://只有5个Value
					case 31:
					case 54:
					case 62:
					case 64:
					case 73:
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
						skillElem->QueryIntAttribute("Value3", &skill.value3);
						skillElem->QueryIntAttribute("Value4", &skill.value4);
						skillElem->QueryIntAttribute("Value5", &skill.value5);
					}
					break;
					case 61://金刚护体
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
						skillElem->QueryIntAttribute("Value3", &skill.value3);
						skillElem->QueryIntAttribute("Value4", &skill.value4);
						skillElem->QueryIntAttribute("Value5", &skill.value5);
						skillElem->QueryIntAttribute("Value6", &skill.value6);
						skillElem->QueryIntAttribute("Value7", &skill.value7);
						skillElem->QueryIntAttribute("Value8", &skill.value8);
					}
					break;
					case 70://风影盾
					{
						skillElem->QueryIntAttribute("Lv", &skill.level);
						skillElem->QueryIntAttribute("Value1", &skill.value1);
						skillElem->QueryIntAttribute("Value2", &skill.value2);
						skillElem->QueryIntAttribute("Value3", &skill.value3);
						skillElem->QueryIntAttribute("Value4", &skill.value4);
						skillElem->QueryIntAttribute("Value5", &skill.value5);
						skillElem->QueryIntAttribute("Value6", &skill.value6);
						skillElem->QueryIntAttribute("Value7", &skill.value7);
					}
					break;
					default:
						break;
					}
					magic.skills.push_back(skill);
				}
				magics[magic.id] = magic;
			}
		}
	}
}

BOOL CMagicManager::AddMagicClass(MAGICCLASS* pMagicclass)
{
	xListHost<MAGICCLASS>::xListNode* pNode = nullptr;
	pNode = (xListHost<MAGICCLASS>::xListNode*)m_MagicClassHash.HGet(pMagicclass->szName);
	if (pNode)
	{
		memcpy(pNode->getObject(), pMagicclass, sizeof(MAGICCLASS));
		return TRUE;
	}
	MAGICCLASS* pTemp = new MAGICCLASS;
	if (pTemp == nullptr)return FALSE;
	*pTemp = *pMagicclass;
	pNode = m_xMagicNodePool.newObject();
	if (pNode == nullptr)
	{
		m_strErrorMsg = "分配node失败!";
		delete pTemp;
		return FALSE;
	}
	pNode->setObject(pTemp);
	if (!m_xMagicClassList.addNode(pNode))
	{
		m_strErrorMsg = "添加node失败!";
		delete pTemp;
		m_xMagicNodePool.deleteObject(pNode);
		return FALSE;
	}
	if (!m_MagicClassHash.HAdd(pTemp->szName, (LPVOID)pNode))
	{
		m_strErrorMsg = "加入到名字表中失败!";
		m_xMagicClassList.removeNode(pNode);
		delete pTemp;
		m_xMagicNodePool.deleteObject(pNode);
		return FALSE;
	}
	m_pMagicArray[pTemp->id] = pTemp;
	return TRUE;
}

BOOL CMagicManager::CreateMagic(const char* pszName, MAGIC& magic)
{
	xListHost<MAGICCLASS>::xListNode* pNode = (xListHost<MAGICCLASS>::xListNode*)m_MagicClassHash.HGet(pszName);
	if (pNode == nullptr)return FALSE;
	MAGICCLASS* pClass = pNode->getObject();
	if (pClass == nullptr)return FALSE;

	o_strncpy(magic.szName, pClass->szName, 12);
	magic.btNameLength = static_cast<BYTE>(strlen(magic.szName));
	magic.btLevel = 0;
	magic.wDelayTime = pClass->wDelay;
	magic.btNeedLevel[0] = pClass->btNeedLv[0];
	magic.btNeedLevel[1] = pClass->btNeedLv[1];
	magic.btNeedLevel[2] = pClass->btNeedLv[2];
	magic.btNeedLevel[3] = pClass->btNeedLv[3];
	magic.iLevelupExp[0] = pClass->dwNeedExp[0];
	magic.iLevelupExp[1] = pClass->dwNeedExp[1];
	magic.iLevelupExp[2] = pClass->dwNeedExp[2];
	magic.iLevelupExp[3] = pClass->dwNeedExp[3];
	magic.btEffectType = pClass->btEffectType;
	magic.btEffect = pClass->btEffectValue;

	magic.wSpell = pClass->GetSpellPoint(0);

	magic.cKey = 0;
	magic.job = pClass->btJob;

	magic.wId = (WORD)pClass->id;

	return TRUE;
}

BOOL CMagicManager::CreateMagic(UINT id, MAGIC& magic)
{
	MAGICCLASS* pClass = GetClassById(id);
	if (pClass == nullptr)return FALSE;
	GetMagicFromClass(pClass, magic);
	return TRUE;
}

VOID CMagicManager::GetMagicFromClass(MAGICCLASS* pClass, MAGIC& magic)
{
	o_strncpy(magic.szName, pClass->szName, 12);
	magic.btNameLength = static_cast<BYTE>(strlen(magic.szName));

	magic.btLevel = 0;
	magic.wDelayTime = pClass->wDelay;
	magic.btNeedLevel[0] = pClass->btNeedLv[0];
	magic.btNeedLevel[1] = pClass->btNeedLv[1];
	magic.btNeedLevel[2] = pClass->btNeedLv[2];
	magic.btNeedLevel[3] = pClass->btNeedLv[3];
	magic.iLevelupExp[0] = pClass->dwNeedExp[0];
	magic.iLevelupExp[1] = pClass->dwNeedExp[1];
	magic.iLevelupExp[2] = pClass->dwNeedExp[2];
	magic.iLevelupExp[3] = pClass->dwNeedExp[3];

	magic.btEffectType = pClass->btEffectType;
	magic.btEffect = pClass->btEffectValue;

	magic.wSpell = pClass->GetSpellPoint(0);

	magic.cKey = 0;
	magic.job = pClass->btJob;

	magic.wId = (WORD)pClass->id;
}

MAGICCLASS* CMagicManager::GetClassByName(const char* pszMagic)
{
	xListHost<MAGICCLASS>::xListNode* pNode = (xListHost<MAGICCLASS>::xListNode*)m_MagicClassHash.HGet(pszMagic);
	if (pNode == nullptr)return nullptr;
	return pNode->getObject();
}

Magic& CMagicManager::GetMagic(WORD wMagicId)
{
	auto it = magics.find(wMagicId);
	if (it != magics.end())
		return it->second;
	static Magic s_emptyMagic{0, {}};
	if (!magics.empty())
		return magics.begin()->second;
	return s_emptyMagic;
}

VOID CMagicManager::LoadMagicExt(const char* pszMagicExtFile)
{
	struct tagMagicExt
	{
		char	szMagicName[20]; // 技能名
		BOOL	bNoEffect; // 无效果
		BOOL	bActive; // 开关式
		DWORD	bForced; // 被动类型(0非被动1攻击2经验值)
		WORD	wCharmCount; // 耗道符数量
		WORD	wRedPoisonCount; // 耗红毒
		WORD	wGreenPoisonCount; // 耗绿毒
		WORD	wStrawManCount; // 耗雄稻草人(男)
		WORD	wStrawWomanCount; // 耗雌稻草人(女)
		char	szSpecial[256]; // 附加参数
	}MagicExt{};
	CFmtTextFile ftfMagicExt("s20d3w5s256", pszMagicExtFile, TRUE);

	for (int i = 0; i < ftfMagicExt.GetCount(); i++)
	{
		if (ftfMagicExt.GetStruct(i, &MagicExt))
		{
			MAGICCLASS* pMagicClass = GetClassByName(MagicExt.szMagicName);
			if (pMagicClass)
			{
				o_strncpy(pMagicClass->szSpecial, MagicExt.szSpecial, 255);
				if (MagicExt.bNoEffect)
					pMagicClass->dwFlag = MAGICFLAG_NOEFFECT;
				if (MagicExt.bActive)
					pMagicClass->dwFlag |= MAGICFLAG_ACTIVED;
				if (MagicExt.bForced == 2)
					pMagicClass->dwFlag |= MAGICFLAG_FORCED_EXP;
				else if (MagicExt.bForced != 0)
					pMagicClass->dwFlag |= MAGICFLAG_FORCED;
				pMagicClass->wCharmCount = MagicExt.wCharmCount;
				if (pMagicClass->wCharmCount > 0)
					pMagicClass->dwFlag |= MAGICFLAG_USECHARM;
				pMagicClass->wRedPoisonCount = MagicExt.wRedPoisonCount;
				if (pMagicClass->wRedPoisonCount > 0)
					pMagicClass->dwFlag |= MAGICFLAG_USEREDPOISON;
				pMagicClass->wGreenPoisonCount = MagicExt.wGreenPoisonCount;
				if (pMagicClass->wGreenPoisonCount > 0)
					pMagicClass->dwFlag |= MAGICFLAG_USEGREENPOISON;
				pMagicClass->wStrawManCount = MagicExt.wStrawManCount;
				if (pMagicClass->wStrawManCount > 0)
					pMagicClass->dwFlag |= MAGICFLAG_USESTRAWMAN;
				pMagicClass->wStrawWomanCount = MagicExt.wStrawWomanCount;
				if (pMagicClass->wStrawWomanCount > 0)
					pMagicClass->dwFlag |= MAGICFLAG_USESTRAWWOMAN;
			}
		}
	}
}

BOOL CMagicManager::DeleteMagicFromDB(DWORD dwOwner, WORD wMagicId)
{
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(0);
	if (pObj == nullptr)return FALSE;
	pObj->SendDeleteMagic(dwOwner, wMagicId);
	return TRUE;
}