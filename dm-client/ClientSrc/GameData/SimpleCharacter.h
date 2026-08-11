///////////////////////////////////////////////////////////////////////
//文件名：  SimpleCharacter.h
//文件说明：本文件是类CSimpleCharacter的头文件（包含实现）
//			CSimpleCharacter用来记录游戏中在地图上的所有角色的显示数据，
//			包括玩家，NPC，尸体。
//			由于每个角色有一个唯一ID，所以CSimpleCharacter
//			重载了==，用ID来判断是否相等。
//
//注：		函数SetName在内存不足的时候会抛出异常。
///////////////////////////////////////////////////////////////////////
#pragma once

#include "Global/Global.h"
#include "Global/Template.h"
#include "CharacterAttr.h"

class CSimpleCharacter : public CCharacterAttr
{
public:
	CSimpleCharacter();
	CSimpleCharacter::~CSimpleCharacter();

	void		SetMonsterArmLevel(int i);
	int			GetMonsterArmLevel();

	//豹子留言
	string&		GetPetLeaveWords();
	void		SetPetLeaveWords(const char * str);
	DWORD		GetPetLeaveWordsTime();
	void		SetPetLeaveWordsTime(DWORD dw);

	int			GetSilkwormFrame();
	void		SetSilkwormFrame(int i);

	bool        IsLeopard();
	bool		IsCanRideLeopard();
	bool		IsHorse();
	bool		IsPet();            //根据名字判断是否是宠物(首先必须是怪物)
	bool		IsMyPet();
	bool        IsHuman();//判断是否人
	bool        IsNpc();//是不是NPC
	bool        IsMonster();//是不是怪物

	bool        IsYourPet(const char * strName);
	bool        IsNoNormalAttack();
	bool        IsPetOfHeartMonster();

	int			GetOtherLevel();
	void		SetOtherLevel(int level);

	void        SetGuildName(const char * strName);
	const char * GetGuildName();

	//void		StartFaZhen(BYTE flag,BYTE leftfabao,BYTE rightfabao);

	virtual bool FromBuffer(const char* buf,int iLen); 
private:

	string		m_strLeaveWords;//宠物喊话 豹语泡泡
	DWORD       m_dwPetLeaveWordsTime;//豹子说话的间隔时间记录
	int         m_iMonstArmLevel;//豹子的装备等级 

	int         m_iSilkwormFrame;
	string      m_strGuildName;//角色所在行会名

};

//角色列表结点
typedef ListNode<CSimpleCharacter> CSimpleCharacterNode;

#include "SimpleCharacter.inl"
