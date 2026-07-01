#pragma once
#include <array>

/**
 *  MonsterStateComponent — 怪物运行时状态数据（纯数据组件）
 *
 *  替代 CMonsterEx 中的散落状态变量，连续存储，缓存友好。
 *  所有字段均为值类型，无指针/引用，生命周期与怪物对象一致。
 */
struct MonsterStateComponent
{
	WORD  wCurHp = 0;               // 当前生命值
	WORD  wCurMp = 0;               // 当前魔法值
	DWORD dwExp = 0;                // 经验值(宠物升级用)
	DWORD dwKillCount = 0;          // 击杀计数(变身条件)
	BYTE  btRevivalCount = 0;       // 被复活次数(影响掉落)
	BOOL  fCuted = FALSE;           // 是否已被挖肉
	BOOL  bFirstEnterMap = FALSE;   // 是否首次进入地图
	BOOL  boSpecialGen = FALSE;     // 是否是特殊刷怪
	BOOL  bIsShow = FALSE;          // 是否已播放出现动画
	BOOL  bNoAiDelayAttack = TRUE;  // 是否无延时AI攻击
	std::array<char, 20> szOriginalName{}; // 原始显示名字(变身恢复用)

	MonsterStateComponent() = default;
};
