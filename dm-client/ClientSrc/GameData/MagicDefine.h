#pragma once

#include "Global/MemPool.h"
#include "ActionStruct.h"

////////////////////////////////////////////////////////////////////
// 魔法ID定义
////////////////////////////////////////////////////////////////////
enum E_MAGIC_ID
{
	MAGICID_FIREBALL		=	1,					// 小火球
	MAGICID_HEAL			=   2,					// 治愈术	
	MAGICID_ATTACK_BASE		=	3,					// 基本剑术
	MAGICID_ATTACK_SOUL		=	4,					// 精神战法
	MAGICID_ADV_FIREBALL	=	5,					// 火炎刀
	MAGICID_POISON_MAGIC	=	6,					// 施毒术
	MAGICID_ATTACK_KILL		=	7,					// 攻杀剑术
	MAGICID_DISPUTE_FIRE	=	8,					// 抗拒火环
	MAGICID_HELL_FIRE		=	9,					// 地狱火焰
	MAGICID_TRACE_THUNDER	=	10,					// 极光电影
	MAGICID_THUNDER			=	11,					// 雷电术
	MAGICID_ATTACK_STICK	=	12,					// 刺杀剑术
	MAGICID_PROTECT_SYMBOL  =	13,					// 灵魂道符
	MAGICID_PROTECT_GHOST	=	14,					// 幽灵盾
	MAGICID_PROTECT_ARMOR	=	15,					// 神圣战甲术
	MAGICID_LOCK_MONSTER	=	16,					// 困魔咒
	MAGICID_CALL_MONSTER	=	17,					// 骷髅召唤术
	MAGICID_HIDE_SKILL		=	18,					// 隐身术
	MAGICID_HIDE_ALL		=	19,					// 集体隐身术
	MAGICID_LURE_LIGHT		=	20,					// 诱惑之光
	MAGICID_MOVE_QUICK		=	21,					// 瞬间移动
	MAGICID_FIRE_WALL		=	22,					// 火墙
	MAGICID_BLOW_FIRE		=	23,					// 爆烈火焰
	MAGICID_HELL_THUNDER	=	24,					// 地狱雷光
	MAGICID_ATTACK_MOON		=	25,					// 半月弯刀
	MAGICID_ATTACK_FIRE		=	26,					// 烈火剑法
	MAGICID_WILD_COLLIDE	=	27,					// 野蛮冲撞
	MAGICID_SOUL_REVEAL		=	28,					// 心灵启示
	MAGICID_TEAM_HEAL		=	29,					// 群体治愈术
	MAGICID_CALL_JOSS		=	30,					// 召唤神兽
	MAGICID_MAGIC_PROTECT	=	31,					// 魔法盾
	MAGICID_GOD_SAY			=	32,					// 圣言术
	MAGICID_ICE_STORM		=	33,					// 冰咆哮
	MAGICID_FIGHTER_WILD	=	34,					// 战士兽化
	MAGICID_ENCHANTER_WILD  =	35,					// 化身蝙蝠
	MAGICID_TAOIST_WILD		=	36,					// 道士兽化
	MAGICID_LION_CALL		=	37,					// 狮子吼

	MAGICID_REMAIN_SHADE	=	40,					// 残影刀法
	MAGICID_BLOOD_SHADE		=	41,					// 血影刀法
	MAGICID_PROTECT_SKIN	=	42,					// 护身真气
	MAGICID_WHOLE_MOON		=	43,					// 园月弯刀
	MAGICID_THUNDER_FIRE	=	44,					// 雷烈剑法
	MAGICID_CURSE			=   45,					// 诅咒术
	MAGICID_CONTROL_CORPSE  =	46,					// 赶尸
	MAGICID_REPLACE_MAGIC	=	47,					// 替身法符
	MAGICID_SOUL_WALL		=	48,					// 灵魂墙
	MAGICID_ICE_ARROW		=	49,					// 冰箭术
	MAGICID_ROTATE_FIRE		=	50,					// 风火轮
	MAGICID_ICE_DRAG		=	51,					// 冰龙破
	MAGICID_ICE_ARRAY		=	52,					// 冰刃阵
	MAGICID_MULTI_THUDER	=	53,					// 五雷轰
	MAGICID_ICE_WHIRLWIND	=	54,					// 冰旋风
	MAGICID_DESTROY_WEAPON  =	55,					// 击破
	MAGICID_WARRIOR_KNOW	=	58,					// 战魂真悟
	MAGICID_DESTROY_SHELL   =	59,					// 破击剑法
	MAGICID_DESTROY_SHIELD  =	60,					// 破盾斩
	MAGICID_STEEL_PROTECT   =	61,					// 金刚护体
	MAGICID_SUDDEN_KILL     =   62,                  // 突斩
	MAGICID_TAOIST_KNOW     =	63,					// 道心清明 
	MAGICID_GODLIGHT_SKILL  =	64,					// 神光术
	MAGICID_DESTROY_POISON  =	65,					// 解毒术
	MAGICID_ENCALL_MONSTER  =  	66,					// 强化骷髅术
	MAGICID_FIRE_JUJU       =   67,                  // 幽冥火咒
	MAGICID_WIZ_KNOW     	=   68,					// 法之魄
	MAGICID_BEAST_SKILL 	=	69,	     			// 兽灵术
	MAGICID_DODGE_SKILL 	=   70,					// 风影盾
	MAGICID_DRAGON_LIGHT    =   71,                  // 狂龙紫电

	MAGICID_PET_CALL        =   72,                  // 道士心灵召唤
	MAGICID_FIRE_RAIN       =   73,                  // 法师流星火雨
	MAGICID_GRASP_DRAGON    =   74,                  // 战士擒龙手
	MAGICID_ESCAPE_QUICK    =   75,                  // 道士遁地
	MAGICID_SUCK_SOUL       =   76,                  // 法师魔魂术
	MAGICID_SHIFT_TRANSPORT =   77,                  // 战士移形换影

	MAGICID_FIGHTER_FIGHTER_KILL      = 78,          // 怒斩天下
	MAGICID_ENCHANTER_ENCHANTER_THUNDER=79,          // 天怒惊雷
	MAGICID_DAO_DAO_FLOWER            = 80,          // 天女散花咒起手
	MAGICID_FIGHTER_ENCHANTER_FIRE    = 81,          // 迷光烈焰
	MAGICID_DAO_FIGHTER_POISON        = 83,          // 火毒攻心剑
	MAGICID_ENCHANGER_DAO_BOSS        = 85,          // 神之召唤

	MAGICID_SUPER_FIREBALL	          = 87,         //强化小火球
	MAGICID_SUPER_ATTACK_KILL         = 88,			//强化攻杀剑法
	MAGICID_SUPER_POISON_MAGIC	      = 89,			//强化施毒术
	MAGICID_SUPER_CURSE			      = 90,			//强化诅咒术
	MAGICID_ZHENYUAN_ATTACK			  = 91,     	//真元攻击特效

	MAGICID_AOFENGCUT_ATTACK		  = 92,			//翱风斩
	MAGICID_ZHONGLEI_ATTACK			  = 93,			//纵雷诀
	MAGICID_DULINGBO_ATTACK			  = 94,			//毒凌波

	MAGICID_SWT		                  = 95,			//圣武天套装技能动作,战士
	MAGICID_LSJ			              = 96,			//雷神绝套装技能动作,法师
	MAGICID_JYL			              = 97,			//九幽灵套装技能动作,道士
	MAGICID_ZHI_YAN		              = 98,			//炙炎套装技能

  
    MAGICID_DIGMINE                   = 100,        //挖矿
    MAGICID_FELLWOOD                  = 101,        //伐木
    MAGICID_HUNTSKILL                 = 102,        //打猎
    MAGICID_DISASSEMBLE               = 103,        //拆卸
    MAGICID_FOUNDSKILL                = 104,        //铸造
    MAGICID_SMELTSKILL                = 105,        //炼制
	MAGICID_TRANSFER_HURT			  = 106,		//斗转星移    

	MAGICID_ANGRYWHIRLWIND			  =	107,			//狂怒旋风
	MAGICID_COLDSTROM				  = 108,			//寒冰风暴
	MAGICID_QUNMOTUNSHI				  = 109,			//心灵爆破(同传世群魔吞噬)
	MAGICID_SKULLCAGE				  = 110,			//骷髅囚笼
	MAGICID_HEROROAR				  = 111,			//英勇咆哮
	MAGICID_SOULATTACH				  = 112,			//电灵附体

	MAGICID_JUDUZHAOZE				  = 113,			//剧毒沼泽
	MAGICID_LIANJI					  = 114,			//连击技能

	MAGICID_SHADOWKILL8				  = 115,			//八方分影斩
	MAGICID_FIRE_SHIELD_ATTACK		  = 116,			//强袭烈焰盾攻击	
	MAGICID_DAOZUN_JIANGLIN			  = 117,			//无量五行诀
	//MAGICID_FAZHEN_CARRYON		  = 118,			//法阵启动

	MAGICID_GOLD_POWDER					 =1001,					// uu 金粉
	MAGICID_FIRE_1						 =1002,					// uu 焰火1
	MAGICID_FIRE_2						 =1003,					// uu 焰火2
	MAGICID_FIRE_3					     =1004,					// uu 焰火3
	MAGICID_FIRE_4						 =1005,					// uu 焰火4
	MAGICID_FIRE_5						 =1006,					// uu 焰火5
	MAGICID_FIRE_6						 =1007,					// uu 焰火6
	MAGICID_FIRE_7						 =1008,					// uu 焰火7
	MAGICID_FIRE_8						 =1009,					// uu 焰火8
	MAGICID_FIRE_9						 =1010,					// uu 焰火9
	MAGICID_FIRE_10						 =1011,					// uu 焰火10
	MAGICID_FIRE_11						 =1012,					// uu 焰火11
	MAGICID_FIRE_12						 =1013,					// uu 焰火12
	MAGICID_FIRE_13						 =1014,					// uu 焰火13
	MAGICID_FIRE_14						 =1015,					// uu 火热的心

	MAGICID_HAPPY_PASTE					 =1070,					// uu 喜帖
	MAGICID_HAPPY_CANDLE			     =1071,					// uu 花烛

	MAGICID_FIRE_SHIELD_HE				 = 1089,			//强袭烈焰盾
	MAGICID_FIRE_SHIELD					 = 1090,			//强袭烈焰盾loop
	MAGICID_DAOZUN_JIANGLIN_ATTACK		 = 1091,			//无量五行诀道符攻击
	MAGICID_SHADOWKILL8_DAOG			 = 1092,			//八方分影斩刀光
	MAGICID_SHADOWKILL8_HIT				 = 1093,			//八方分影斩击中

	MAGICID_FIRE_15						 =1151,					// uu 文字焰火杀
	MAGICID_FIRE_16						 =1152,					// uu 文字焰火攻
	MAGICID_FIRE_17					     =1153,					// uu 文字焰火回
	MAGICID_FIRE_18					     =1154,					// uu 文字焰火闪

	MAGICID_GUILD_FLY_ATTACK			   =1155,					// 护卫的远距离飞行攻击
	MAGICID_THROW_MONSTER_FLY_ATTACK	   =1156,					// 掷枪尸卫的远距离飞行攻击
	MAGICID_FLOORFIRE_SPEAR_FLY_ATTACK	   =1157,					// 地火兽投矛手的远距离飞行攻击
	MAGICID_CACTI_FLY_ATTACK			   =1158,					// 仙人掌的远距离飞行攻击
	MAGICID_BOW_MONSTER_FLY_ATTACK		   =1159,					// 逆魔弓箭手的远距离飞行攻击
	MAGICID_FLOORFIRE_CAVALIER_FLY_ATTACK  =1160,					// 地火兽弓骑士的远距离飞行攻击
	MAGICID_BLOOD_MONSTER_FLY_ATTACK	   =1161,					// 血魔的远距离飞行攻击
	MAGICID_COPPER_MONSTER_FLY_ATTACK	   =1162,				    // 铜人的远距离飞行攻击
	MAGICID_TONGTIAN_FLY_ATTACK            =1163,					// 通天教主飞行攻击
	MAGICID_IRON_WING_MONSTR_ATTACK        =1164,					// 铁翼魔攻击
	MAGICID_ARROW_TOWER_FLY_ATTACK         =1165,                   // 箭塔攻击飞行特效
	MAGICID_FIRE_BALL_HORSE_ATTACK         =1166,					//马怪攻击
	MAGICID_ICE_ATTACK1					   =1167,					//驭冰游神攻击1
	MAGICID_ICE_ATTACK2                    =1168,					//驭冰游神攻击2
	MAGICID_SHENGWUFU_ATTACK			   =1169,					// 圣武斧卫飞斧
	MAGICID_SHENGWUGONG_ATTACK			   =1170,					// 圣武连弩攻击
	MAGICID_BIANBING_ATTACK2			   =1171,					// 持鞭骑兵攻击2特效

	MAGICID_HEART_FLOWER_BLOSSOM         =2071,                 // uu心花怒放
	MAGICID_SNOWMAN						 =2072,                 // uu雪人
	MAGICID_SKELETON					 =2073,                 // uu骷髅
	MAGICID_HEAD_MONSTER				 =2074,                 // uu战将
	MAGICID_BROADSWORD_MONSTER			 =2075,                 // uu大刀
	MAGICID_BOWMAN						 =2076,                 // uu弓箭
	MAGICID_GREENTIGER_FLAG				 =2077,                 // uu青虎旗
	MAGICID_WHITETIGER_FLAG				 =2078,                 // uu白虎旗
	MAGICID_TOAD_FLAG				     =2079,                 // uu玄武旗
	MAGICID_REDSPARROW_FLAG				 =2080,                 // uu朱雀旗
	MAGICID_SUPER_HEART_FLOWER_BLOSSOM   =2081,                 // uu超级心花怒放
	MAGICID_TELESTHESIA_KNOT             =2082,                 // uu心心相映结

	MAGICID_REGALEMENT_CAKE              =2152,                 // uu盛宴蛋糕
	MAGICID_REGALEMENT_ICE_TEA           =2153,                 // 冰红茶
	MAGICID_REGALEMENT_ICE_TEA_END       =2154,                 // 冰红茶结束

	MAGICID_FIREWORK_BLAST               =2155,                 // uu鞭炮爆炸
	MAGICID_DARK_MONSTER_ATTACK          =2156,					// 暗域魔王攻击 

	MAGICID_BIG_STONE_ATTACKED			 =2158,					// 被投石巨魔的石头砸中
	//MAGICID_BIG_FIRE_ATTACKED			 =2159,					// 炎魔攻击附带得火焰
	MAGICID_BIG_SWORD_ATTACKED			 =2160,					// 噩梦铠魔攻击得刀光
	MAGICID_ICE_SWORD_ATTACKED			 =2161,					// 寒冰剑
	MAGICID_ICE_BLOCK_ATTACKED			 =2162,					// 千里冰封
	MAGICID_MON_BZ_ATTACK_EFFECT         =2163,                  // 冰锥攻击效果
	
	
	
	MAGICID_DARK_MONSTER_DEATH1          =2165,                  //暗域魔王前4方向死亡效果
	MAGICID_DARK_MONSTER_DEATH2          =2166,                  //暗域魔王后4方向死亡效果
	MAGICID_DARK_MONSTER_APPEAR1         =2167,                  //暗域魔王前4方向出现效果
	MAGICID_DARK_MONSTER_APPEAR2         =2168,                  //暗域魔王后4方向出现效果
	MAGICID_AXIULUO_MONSTER_DEATH        =2169,                  //阿修罗神变身死亡效果
	MAGICID_SHADOW_MONSTER_APPEAR        =2170,                  //幽影武士出现效果
	MAGICID_SHADOW_MONSTER2_APPEAR       =2171,                  //幽影武士2出现效果
	MAGICID_XIULUO_KING_MONSTER_APPEAR   =2172,                  //修罗王神出现效果
	MAGICID_SHADOW_MONSTER2_DIR3_APPEAR  =2173,                  //幽影武士2方向3出现效果
	MAGICID_YUANSHEN_MONSTER_APPEAR      =2174,                  //元神怪物出现效果
	MAGICID_DARK_MONSTER_ATTACK2         =2175,                  //暗域之王攻击2
	MAGICID_DARK_MONSTER_ATTACK3         =2176,                  //暗域之王攻击3
	MAGICID_DARK_MONSTER_ATTACK4         =2177,                  //暗域之王攻击4	
	MAGICID_DARK_MONSTER_ATTACK5_DIR4    =2178,                  //暗域之王攻击5前4方向	
	MAGICID_DARK_MONSTER_ATTACK5_DIR8    =2179,                  //暗域之王攻击5后4方向
	MAGICID_YAN_MONSTER_ATTACK_EFFECT    =2180,                  //炎魔攻击效果
	MAGICID_YAN_MONSTER2_DEATH_EFFECT    =2181,                  //炎魔2死亡效果
	MAGICID_FLOOR_MONSTER_STAND          =2182,                  //地火兽站立效果
	MAGICID_FLOOR_MONSTER_ATTACKED       =2183,                  //地火兽被攻击效果
	MAGICID_FLOOR_MONSTER_WALK           =2184,                  //地火兽行走击效果
	MAGICID_FLOOR_MONSTER_DEATH          =2185,                  //地火兽死亡效果
	MAGICID_FLOOR_MONSTER_ATTACK1        =2186,                  //地火兽攻击效果
	MAGICID_JINDI_MONSTER_APPEAR         =2187,                  //禁地魔王出现效果
	MAGICID_DARK_MONSTER_ATTACK6         =2188,                  //暗域之王攻击6	
	MAGICID_BIANSHEN_EFFECT              =2189,                  //变身效果，通天教主火圈	
	MAGICID_MON_TIANMO_ATTACK1           =2190,                  //无相天魔的攻击效果
	MAGICID_MON_TIANMO_ATTACK2           =2191,                  //无相天魔的攻击效果2	
	MAGICID_MON_OCEAN_APPEAR             =2192,                  //寒荒海兽出现	
	MAGICID_ROTATE_FIRE_ATTACKED		 =2193,					 //风火轮击中效果
	MAGICID_DAO_BLOOD_MONSTER_ATTACK     =2194,					 //道士套装技能召唤出来的碧血魔的攻击效果
	MAGICID_MOONCAKE_EFFECT				 =2195,					 //月饼特效
	

	//3000-4000为骑战技能特效,其它特效不要占用这些编号
	MAGICID_LEOPARD_ATTACK_KILL		  =	3007,		 //骑战攻杀剑术
	MAGICID_LEOPARD_ATTACK_STICK	  =	3012,		 //骑战刺杀剑术
	MAGICID_LEOPARD_ATTACK_MOON	      =	3025,		 //骑战半月弯刀
	MAGICID_LEOPARD_ATTACK_FIRE       = 3026,        //骑战烈火剑法
	MAGICID_LEOPARD_BLOOD_SHADE		  =	3041,		 //骑战血影刀法
	MAGICID_LEOPARD_WHOLE_MOON		  =	3043,		 //骑战园月弯刀
	MAGICID_LEOPARD_THUNDER_FIRE	  =	3044,		 //骑战雷烈剑法
	MAGICID_LEOPARD_FIRE_JUJU         = 3067,        //骑战幽冥火咒
	MAGICID_LEOPARD_DRAGON_LIGHT      = 3071,        //骑战狂龙紫电 
	MAGICID_LEOPARD_SWT		          = 3095,		 //骑战圣武天套装技能动作,战士
	MAGICID_LEOPARD_LSJ			      = 3096,		 //骑战雷神绝套装技能动作,法师
	//上面3000-4000为骑战技能特效,其它特效不要占用这些编号




	
	MAGICID_ONE_LOCK_MOSTER    		     =4000,                  //一个困魔咒
	MAGICID_FIREWALL_LAST                =4001,                  //火墙持续效果
	MAGICID_FIRE_RAIN_LAST               =4002,                  //流星火雨持续效果 
	MAGICID_PROTECT_SKIN_END             =4003,                  //护身真气结束

	MAGICID_DAO_FIGHTER_POISON_LEFT      =4005,					 //火毒攻心剑左边效果
	MAGICID_DAO_FIGHTER_POISON_RIGHT     =4006,					 //火毒攻心剑右边效果
	MAGICID_DAO_FIGHTER_POISON_MIDDLE    =4007,					 //火毒攻心剑中间效果

	MAGICID_ATTACKED_BY_ATTACK_FIRE      =4009,                  //被烈火攻击
	MAGICID_ATTACKED_BY_WILD_ATTACK      =4010,                  //被战士兽化攻击
	MAGICID_ATTACKED_BY_DESTROY_SHELL    =4011,                  //被破甲破击攻击
	MAGICID_ATTACKED_BY_DESTROY_SHIELD   =4012,                  //被破盾攻击
	MAGICID_ATTACKED_BY_TONGTIAN         =4013,                  //被通天教主攻击的效果
	MAGICID_ARM_REFRESH_FAILED           =4014,                  //武器升级失败
	MAGICID_ATTACKED_BY_SUPER_ATTACK_KILL=4015,                  //被强化攻杀攻击


	MAGICID_LION_CALLED					=4224,					// 被别人狮子吼
	MAGICID_FIGHTER_WILD_SEND			=4225,					// 仅仅发送时使用
	MAGICID_ENCHANTER_WILD_SEND			=4226,					// 仅仅发送时使用
	MAGICID_TAOIST_WILD_SEND			=4227,				    // 仅仅发送时使用
	MAGICID_WILD_COLLIDE_SEND			=4228,					// 仅仅发送时使用
	MAGICID_BOWMAN_ATTACK				=4229,					// 守卫弓箭手的攻击
	MAGICID_ZUMA_BOWMAN_ATTACK			=4230,					// 祖玛弓箭手的攻击
	MAGICID_THUNDER_CORPSE_ATTACK		=4231,					// 电僵的攻击
	MAGICID_ZUMA_HEADER_ATTACK			=4232,					// 祖玛教主的攻击
	MAGICID_TAOIST_WILD_ATTACK			=4233,					// 道士兽化后的攻击
	MAGICID_SHENSHOU_ATTACK				=4234,					// 神兽的站立攻击
	MAGICID_JINDI_FLY_ATTACK			=4235,					// 禁地魔王的远距离飞行攻击
	MAGICID_ZHIQIANGSHIWEI_ATTACK		=4236,					// 掷枪尸卫
	MAGICID_RANKUP_EFFECTIVE			=4237,					// 升级特效
	MAGICID_DIG_SPARK					=4238,					// 挖矿冒的火花
	MAGICID_SPARKLE						=4240,					// 小伙球后面跟的火花
	MAGICID_ADVFIRE_SHADE				=4241,					// 大火球后面跟的重影
	MAGICID_THUNDER_SHADE				=4242,					// 疾光电影的影子
	MAGICID_THUNDER_SPARKLE				=4243,					// 疾光电影的电火花
	MAGICID_NIMO_BREAK					=4244,					// 逆魔蛋壳破裂
	MAGICID_SHIBA_DEATH					=4245,					// 尸霸死亡效果
	MAGICID_STONE_ATTACKED				=4246,					// 被机关巨兽石头砸中
	MAGICID_FALL_OBJECT					=4247,				    // 掉物品
	MAGICID_SKYROCKET					=4248,					// 焰火粒子
	MAGICID_FESTAL_SKYROCKET			=4249,			        // 节日焰火
	MAGICID_STREW_SKYROCKET				=4250,					// 散播的焰火
	MAGICID_GRID_EFFECT					=4251,					// 格子上的效果
	MAGICID_TONGTIAN_DEATH				=4252,					// 通天教主死亡效果
	MAGICID_ATTACKED_BY_TONGTIAN2       =4253,                  // 被通天教主第二招攻击
	MAGICID_NIMO_BREAK2					=4254,					// 逆魔蛋壳破裂2
	MAGICID_SHEN_PROTECT_SKIN_END       =4055,                  // 护身真气结束,神
	MAGICID_MO_PROTECT_SKIN_END         =4056,                  // 护身真气结束,魔
	MAGICID_HEIYANMO_REFRACTION         =4057,                  // 黑怨魔折射效果
	MAGICID_SHEN_FLY_SUCCESS            =4058,                  // 神-莲花下降
	MAGICID_MO_FLY_SUCCESS              =4059,                  // 魔-莲花下降
	MAGICID_AOFENGCUT_ATTACK_ONPHENIX	=4060,			        // 骑凤凰翱风斩
	MAGICID_ICE_ATTACKED	            =4061,			        // 玄冰柱被击
	MAGICID_FIRE_ATTACKED	            =4062,			        // 离火柱被击
	MAGICID_TOWER_ATTACKED	            =4061,			        // 护塔机关被击,后来改成了玄冰柱,所以这里也用玄冰柱被击


	MAGICID_SUDDEN_KILL_SEND        =    4264,                  // 突斩

	//附身
	MAGICID_PLAYER_MAGIC_EFFECT        =  4272,                  // 神佑
	MAGICID_MON_KILLED_EFFECT          =  4273,                  // 怪物被秒杀效果 100%
	MAGICID_MON_KILLED_EFFECT_50       =  4274,                  // 怪物被秒杀效果 50%
	MAGICID_MON_RECOVERY_EFFECT        =  4275,                  // 怪物攻击被防御效果 100%
	MAGICID_MON_RECOVERY_EFFECT_50     =  4276,                  // 怪物攻击被防御效果 50%
	MAGICID_MON_ATTACK_EFFECT          =  4277,                  // 怪物攻击效果包魔法
	MAGICID_MON_MDS_ATTACK_EFFECT      =  4278,                  // 魔斗士攻击效果
	MAGICID_MON_BZ_APPEAR_EFFECT       =  4279,                  // 冰锥出现效果
	MAGICID_MON_AYSD_ATTACK_EFFECT     =  4280,                  // 暗影闪电攻击效果
	MAGICID_ESCAPE_QUICK_SEND          =  4281,                  // 道士遁地发送
	MAGICID_SHIFT_TRANSPORT_SEND       =  4282,                  // 战士移形换位发送
	MAGICID_MON_ANYING_ATTACK_EFFECT   =  4283,                  // 暗影修罗攻击效果：扑出
	MAGICID_MON_ANYING_ATTACKED        =  4284,                  // 被暗影修罗击中
	MAGICID_MON_MOSHEN_ATTACK_EFFECT   =  4285,                  // 魔神修罗攻击效果：扑出
	MAGICID_MON_MOSHEN_ATTACKED        =  4286,                  // 被魔神修罗击中
	MAGICID_PLAYER_CALL_OTHER          =  4287,                  // 江湖告急，斧头帮
	MAGICID_MON_WEAPON_WATER           =  4288,                  // 冰泉圣水特效(自动特修武器的)
	MAGICID_MON_FREEZE_ICE             =  4289,                  // 北海玄冰特效
	MAGICID_MON_SILKWORM               =  4290,                  // 隐身蚕甲特效
	MAGICID_MON_GOLD_ACCOUNT           =  4291,                  // 升级为金牌帐号特效
	MAGICID_PAOPAO_BLAST               =  4292,                  // 泡泡爆炸
	MAGICID_BAOGU_BLAST                =  4293,                  // 爆骨爆炸
	MAGICID_MON_XIAOGUAI_ATTACK_EFFECT =  4294,                  // 寒荒小怪攻击效果
	MAGICID_MON_HAISHOU_ATTACK_EFFECT  =  4295,                  // 海兽攻击效果
	MAGICID_MON_HAISHOU_UNDER_EFFECT   =  4296,                  // 海兽地面效果
	MAGICID_MON_FOUND_TREASURE         =  4297,                  // 挖到了宝贝
	MAGICID_MON_HAISHOU_ATTACK2_EFFECT =  4298,                  // 海兽攻击效果2
	MAGICID_PRINCESS_SWEETHEART        =  4299,                  // 公主献芳心
	MAGICID_DOUBLE_EXP_EFFECT		   =  4300,                  // 双倍经验卡特效
	MAGICID_PLAYER_SWEETHEART          =  4301,                  // 玩家献芳心
	MAGICID_PLAYER_QUIT_YUANSHEN       =  4308,                  // 元神消散 元神消失
	MAGICID_RABBIT_SHENYOU             =  4310,                  // 玉兔神佑
	MAGICID_ATTACKED_BY_RABBIT         =  4311,                  // 被兔女郎攻击
	MAGICID_RABBIT_ATTACK              =  4312,                  // 兔女郎攻击特效

	MAGICID_PLAYER_BAOBAO_REEL		  = 4336,                     // PT商城的道具使用--使用宝宝卷轴特效
	MAGICID_PLAYER_HAOJIAO			  = 4337,                     // PT商城的道具使用--使用号角特效
	MAGICID_PLAYER_HUICHEN			  = 4338,                     // PT商城的道具使用--使用回城石特效
	MAGICID_PLAYER_HUANHUNSHI		  = 4339,                     // PT商城的道具使用--使用还魂石特效
	MAGICID_PLAYER_SHITUSHENZHUO	  = 4340,                     // PT商城的道具使用--使用师徒神镯特效
	MAGICID_PLAYER_AIQINGMOJIE	      = 4341,                     // PT商城的道具使用--使用爱情魔戒特效
	MAGICID_PLAYER_JINGANGREEL	      = 4342,                     // PT商城的道具使用--使用爱情魔戒特效
	MAGICID_PLAYER_ZHENQIREEL	      = 4343,                     // PT商城的道具使用--使用爱情魔戒特效
	MAGICID_PLAYER_JIEYISHENLIAN	  = 4344,                     // PT商城的道具使用--结义圣链
	MAGICID_PLAYER_GAIMING			  = 4345,					  // PT商城的道具使用--易名心咒特效
	MAGICID_PLAYER_YANHUATONG		  = 4346,					  // PT商城的道具使用--烟花筒特效
	MAGICID_PLAYER_ZHENSHISHUIJIN	  = 4347,					  // PT商城的道具使用--真视水晶特效
	MAGICID_PLAYER_LINGOUSHENYOU      = 4348,					  // PT商城的道具使用--灵狗神佑特效
	MAGICID_PLAYER_CHUANYUNJIAN1      = 4349,					  // PT商城的道具使用--穿云箭使用人特效
	MAGICID_PLAYER_CHUANYUNJIAN2      = 4350,					  // PT商城的道具使用--穿云箭救援人特效(小斧头)
	MAGICID_PLAYER_TANMOQIU           = 4351,					  // PT商城的道具使用--探魔球特效


	MAGICID_PLAYER_LEOPARD            = 4352,                     // PT商城的道具使用--放豹子
	MAGICID_HUODU_DILIE_EFF           = 4354,					  // 火毒攻心剑地裂效果
	MAGICID_MONSTER_DIZZY             = 4355,                     // 怪物晕眩效果
	MAGICID_MONSTER_DIZZY2            = 4356,                     // 豹子喷波效果
	

	MAGICID_MON_SELF_EFFECT           = 4362,                     // 阿修罗王攻击特效 
	MAGICID_MON_LIGHT_EFFECT          = 4363,                     // 阿修罗王变身攻击特效 

	MAGICID_SENCER_EFFECT             = 4364,                     // 香炉特效
	MAGICID_MONSTER_APPR_EFF          = 4366,                     // 怪物出现特效
	MAGICID_MON_LIGHT_EFFECT_BEGIN    = 4370,                     // 阿修罗王变身后攻击特效开始

	MAGICID_SUPER_POISON_CONTINUE     = 4622,                  //道士强化施毒术不同等级的持续特效
	MAGICID_SUPER_CURSE_CONTINUE      = 4623,                  //道士强化诅咒术不同等级的持续特效
	MAGICID_MON_TIANMO_POISON_EFFECT  = 4624,                  //无相天魔的施毒攻击效果
	MAGICID_MON_TIANMO_DIE_EFFECT     = 4625,                  //无相天魔的死亡效果
	MAGICID_SUPER_SKILL_ATTACKED      = 4629,                  //强化攻杀击中,不同等级有不同效果

	MAGICID_MON_DEMON_SURE_KILL_ROLE  = 4635,                  //心魔宝宝全屏特效
	MAGICID_MON_DEMON_CALL_UP         = 4636,                  //无相天魔的召唤效果
	MAGICID_MON_DEMON_LICK_UP1        = 4637,                  //无相天魔吞噬效果1
	MAGICID_MON_DEMON_LICK_UP2        = 4638,                  //无相天魔吞噬效果2
	MAGICID_MON_DEMON_GROUND          = 4639,                  //无相天魔地裂效果
	MAGICID_TMBZ_ZY                   = 4643,                  //天魔宝藏紫烟特效

    MAGICID_SCORIA                    = 4652,                  //矿渣地面持续魔法
    MAGICID_CORPSE_HOLE               = 4653,                  //僵尸2洞地面持续魔法
    MAGICID_NIMO_SHELL                = 4654,                  //逆魔系列地面持续魔法

	MAGICID_SUPER_FIREBALL_FLYING     = 4656,                  //强化火球飞行
	MAGICID_SUPER_POISON_SKULL1       = 4657,                  //强化施毒骷髅1
	MAGICID_SUPER_POISON_SKULL2       = 4658,                  //强化施毒骷髅2

	MAGICID_WUXING_FLAG_EFF           = 4659,                  //五行旗魔法特效
	MAGICID_CALL_MONSTER_WXXG         = 4660,                  //召唤怪物特效
	MAGICID_WUXING_FLAG_START          = 4663,                 //五行令旗起手效果
	MAGICID_WUXING_FALG_MONSTER_STATE  = 4664,                 //五行令旗怪物身上特效
	MAGICID_FENGYUAN_CALL_EFF          = 4666,                 //封元印召唤
	MAGICID_WXXG_MONSTER_HURT          = 4667,                 //五行玄关怪物受伤 
	MAGICID_SEAL_MONSTER_DISAPPEAR     = 4668,                 //封元印召唤怪物消失特效
	MAGICID_WXXG_MONSTER_SPIRIT        = 4669,                 //封元印召唤怪物精魄聚集
	MAGICID_WXXG_MONSTER_SPIRIT_FLY    = 4670,                 //封元印召唤怪物精魄聚集 

    MAGICID_TUTENG_EFFECT_TOWER        = 4677,                 //箭塔图腾效果蓝
	MAGICID_GOLDEN_OX_EFF		       = 4679,				   //金牛神佑特效
    MAGICID_TUTENG_EFFECT              = 4680,                 //图腾效果,不同的图腾用不同的方向表示




	MAGICID_ZHONGLEI_EFFECT_HIT		   = 4681,
	MAGICID_AOFENGCUT_EFFECT		   = 4682,

	MAGICID_DEATH_RABBI_ATTACK		   = 4687,					//骷髅法师攻击


    MAGICID_MIGONG_BGIDGE1             = 5001,               //迷仙阵天桥渐出上
	MAGICID_MIGONG_BGIDGE2             = 5002,               //迷仙阵天桥渐出右
	MAGICID_MIGONG_BGIDGE3             = 5003,               //迷仙阵天桥渐出下
	MAGICID_MIGONG_BGIDGE4             = 5004,               //迷仙阵天桥渐出左
	MAGICID_MIGONG_BGIDGE5             = 5005,               //迷仙阵天桥非渐隐渐出上
	MAGICID_MIGONG_BGIDGE6             = 5006,               //迷仙阵天桥非渐隐渐出右
	MAGICID_MIGONG_BGIDGE7             = 5007,               //迷仙阵天桥非渐隐渐出下
	MAGICID_MIGONG_BGIDGE8             = 5008,               //迷仙阵天桥非渐隐渐出左
	MAGICID_MIGONG_BGIDGE9             = 5009,               //迷仙阵天桥渐隐上
	MAGICID_MIGONG_BGIDGE10            = 5010,               //迷仙阵天桥渐隐右
	MAGICID_MIGONG_BGIDGE11            = 5011,               //迷仙阵天桥渐隐下
	MAGICID_MIGONG_BGIDGE12            = 5012,               //迷仙阵天桥渐隐左
    MAGICID_MIXIANLUYING               = 5013,               //迷仙路引飞行
	MAGICID_JIGUAN_EXPLODE             = 5014,               //迷仙阵机关爆炸启动
	
	MAGICID_THUNDER_HIT                = 5050,               //雷击特效
    MAGICID_THUNDER_HIT_BAOXIANG       = 5051,               //雷击特效，迷仙阵大宝箱地图专用
	MGGICID_MAZE_BAGUA                 = 5052,               //迷仙阵八卦地表
	MGGICID_MAZE_SHIBEI                = 5053,               //迷仙阵石碑怪物特效
	MAGICID_MAZE_SHIBEI_DEAD		   = 5054,				 //迷仙阵石碑怪物特效死亡(带延时)
	MAGICID_MAZE_SHIBEI_DEADNODELAY	   = 5055,				 //迷仙阵石碑怪物特效死亡(不带延时)
	MAGICID_SHIELD_REFLECT			   = 5058,				 //通灵盾反射
	MAGICID_TIANJUE_END_EFF			   = 5059,				 //天绝魔域结束后的特效
	MAGICID_ICE_SWORD				   = 5067,				 //怪物发射冰剑
	MAGICID_SPEAR					   = 5068,				 //怪物发射抢
	MAGICID_ICE_SWORD2				   = 5069,				 //怪物发射冰剑2

	MAGICID_ICE_BOSS_ATTACK1           = 5070,				 //冰BOSS动作ATTACK1
	MAGICID_ICE_BOSS_ATTACK2           = 5071,				 //冰BOSS动作ATTACK2
	MAGICID_ICE_BOSS_ATTACK3           = 5072,				 //冰BOSS动作ATTACK3

	MAGICID_FIRE_BOSS_ATTACK1           = 5073,				 //火BOSS动作ATTACK1
	MAGICID_FIRE_BOSS_ATTACK2           = 5074,				 //火BOSS动作ATTACK2
	MAGICID_FIRE_BOSS_ATTACK3           = 5075,				 //火BOSS动作ATTACK3
	MAGICID_MON_BZ7_EFFECT				= 5076,				 //冰锥7攻击


	MAGICID_BEAR_Claw_Light             = 5077,				 //熊爪光
	MAGICID_Sword_Fire					= 5078,				 //驭兽狂魔刀上的火
	MAGICID_Ground_Break				= 5079,				 //地裂效果
	MAGICID_MON_RY_EFFECT				= 5080,				 //熔岩攻击
	MAGICID_FENGHUANG_HOU				= 5081,				 //凤凰吼叫

	MAGICID_MON_BZ7_ATTACK				= 5084,              //冰锥7攻击
	MAGICID_MON_RY_ATTACK				= 5085,              //熔岩攻击
	MAGICID_ELECTRIC_1					= 5086,              //护塔机关1电效果
	MAGICID_ELECTRIC_2                  = 5087,              //护塔机关2电效果
	MAGICID_QUICK_TG_MAGIC              = 5090,              //快速救公主特效
	MAGICID_JYL_MONSTER                 = 5091,              //碧血魔出现
	MAGICID_TILE_CHANGE					= 5092,              //地表改变
	MAGICID_SIGNAL_FIRE					= 5093,              //风火台点燃
	MAGICID_DIGMONSTER_DEAD             = 5094,              //可挖的怪物死亡后特效
	MAGICID_POPUP_PAOPAO                = 5095,              //冒泡，人物头顶冒泡，地图资源点冒泡等
    MAGICID_MONSTER_CHANGEBALL          = 5096,              //托管模式下队长变灵珠特效	
    MAGICID_FOOTBALL_DISAPPEAR          = 5097,              //足球消失
	MAGICID_TIGER_SHENYOU               = 5104,              //灵虎神佑
	MAGICID_WENPEITASK					= 5105,              //纹佩任务特效

	MAGICID_HUMANTIGER_ROAR             = 5120,				 //人形老虎怪吼叫 
	MAGICID_TIGER_SPRAY                 = 5151,				 //老虎爆击全屏特效
	MAGICID_QUICK_SHIFT_TRANSPORT       = 5152,              //战士无cd时间的快速移形换影
	MAGICID_BOMB_BLAST                  = 5153,              //炸弹爆炸
	MAGICID_BOMB_MONSTER_FIRE           = 5154,              //怪物被炸弹炸死的火柱
	MAGICID_REGALEMENT_WITHOUT_CAKE		= 5155,				 //盛宴蛋糕烟火特效(无蛋糕模型)
	
	//MAGICID_COLD_ICE_STORM				= 5162,				 //寒冰风暴
	MAGICID_KULOUQIULONG				= 5158,				 //骷髅囚笼循环结束
	MAGICID_KUANGNU_XUANFENG			= 5159,				 //狂怒旋风循环结束
	MAGICID_XINLINGHUTI_XUNHUAN_JIESHU	= 5156,				 //心灵护体循环结束
	MAGICID_HANBINGFENGBAO_3X3			= 5175,				 //寒冰风暴3x3
	MAGICID_HANBINGFENGBAO_5X5			= 5176,				 //寒冰风暴5X5
	MAGICID_DUNBING_ATTACK1				= 5163,				 //盾兵刀光1
	MAGICID_DUNBING_ATTACK2				= 5164,				 //盾兵刀光2
	MAGICID_SHUANGDAOBING_ATTACK1		= 5165,				 //双刀兵刀光1
	MAGICID_SHUANGDAOBING_ATTACK2		= 5166,				 //双刀兵刀光2
	MAGICID_GONGJIAN_HUANGSE_JIAQIANG	= 5167,				 //弓箭黄色加强
	MAGICID_GONGJIAN_HUANGSE_PUTONG		= 5168,				 //弓箭黄色普通
	MAGICID_GONGJIAN_LANSE_JIAQIANG		= 5169,				 //弓箭蓝色加强
	MAGICID_GONGJIAN_LANSE_PUTONG		= 5170,				 //弓箭蓝色普通
	MAGICID_QIBING_ATTACK1				= 5171,				 //骑兵刀光1
	MAGICID_QIBING_ATTACK2				= 5172,				 //骑兵刀光2
	MAGICID_CHANGQIANG_ATTACK1			= 5173,				 //长枪刀光1
	MAGICID_CHANGQIANG_ATTACK2			= 5174,				 //长枪刀光2
	MAGICID_JIANGJUNBOSS_PT_ATTACK		= 5177,				 //将军boss普通刀光
	MAGICID_JIANGJUNBOSS_PT_FLYINGBALL	= 5178,				 //将军boss魔法球
	MAGICID_JIANGJUNBOSS_BS_MAGIC		= 5179,				 //将军boss变身魔法
	MAGICID_JIANGJUNBOSS_BS_ATTACK		= 5180,				 //将军boss变身刀光
	MAGICID_JIANGJUNBOSS_BS_SPECIAL		= 5181,				 //将军boss变身特殊
	MAGICID_JIANGJUNBOSS_BS_AOE			= 5182,				 //将军boss变身AOE
	MAGICID_QIBING_APPEAR_LEFT			= 5183,				 //骑兵出现左侧特效
	MAGICID_QIBING_APPEAR_RIGHT			= 5184,				 //骑兵出现右侧特效
	MAGICID_LUOJIAN_BIG_LEFT			= 5185,				 //落剑特效
	MAGICID_LUOJIAN_BIG_RIGHT			= 5186,
	MAGICID_LUOJIAN_SMALL_LEFT			= 5187,
	MAGICID_LUOJIAN_SMALL_RIGHT			= 5188,
	MAGICID_DIAOXIANG_XIANSHEN			= 5189,				 //雕像现身
	MAGICID_HANBINGFENGBAO_QISHOUJIDA	= 5190,				 //寒冰风暴起手击打
	MAGICID_BAOZI_XINLINGBAOPO			= 5191,				 //心灵爆破"爆"字特效
	MAGICID_BAOZHA_XINLINGBAOPO			= 5192,				 //心灵爆破宠物爆炸
	MAGICID_JIZHONG_XINLINGBAOPAO		= 5193,				 //心灵爆破敌人击中
	MAGICID_JINGBAO             		= 5194,				 //警报
	MAGICID_JINGBAO2             		= 5195,				 //警报2
	MAGICID_JIANGJUN_ZAJIAN				= 5196,				 //将军砸剑normal
	MAGICID_LINGWU_CHENGYUN				= 5220,				 //领悟三味真火,成云
	MAGICID_JUEXING_ZHENHUO_HONG		= 5221,				 //觉醒真火_红色
	MAGICID_JUEXING_ZHENHUO_LAN			= 5222,				 //觉醒真火_蓝色
	MAGICID_JUEXING_ZHENHUO_JIN			= 5223,				 //觉醒真火_金色
	MAGICID_FABAOTRACK_SHUXING_WU		= 5224,				 //法宝轨迹,属性无
	MAGICID_FABAOTRACK_SHUXING_jin		= 5225,				 //法宝轨迹,属性金
	MAGICID_FABAOTRACK_SHUXING_mu		= 5226,				 //法宝轨迹,属性木
	MAGICID_FABAOTRACK_SHUXING_tu		= 5227,				 //法宝轨迹,属性土
	MAGICID_FABAOTRACK_SHUXING_shui		= 5228,				 //法宝轨迹,属性水
	MAGICID_FABAOTRACK_SHUXING_huo		= 5229,				 //法宝轨迹,属性火

	MAGICID_FABAOTRACK_SHUXING_WU_YUAN	= 5230,				 //法宝轨迹,属性无
	MAGICID_FABAOTRACK_SHUXING_jin_YUAN	= 5231,				 //法宝轨迹,属性金
	MAGICID_FABAOTRACK_SHUXING_mu_YUAN	= 5232,				 //法宝轨迹,属性木
	MAGICID_FABAOTRACK_SHUXING_tu_YUAN	= 5233,				 //法宝轨迹,属性土
	MAGICID_FABAOTRACK_SHUXING_shui_YUAN= 5234,				 //法宝轨迹,属性水
	MAGICID_FABAOTRACK_SHUXING_huo_YUAN	= 5235,				 //法宝轨迹,属性火

	MAGICID_MONSTER_JIN_DEAD            = 5240,              //金属性怪物死亡
	MAGICID_MONSTER_MU_DEAD             = 5241,              //木属性怪物死亡
	MAGICID_MONSTER_TU_DEAD             = 5242,              //土属性怪物死亡
	MAGICID_MONSTER_SHUI_DEAD           = 5243,              //水属性怪物死亡
	MAGICID_MONSTER_HUO_DEAD            = 5244,              //火属性怪物死亡

	MAGICID_FIRE_LEVELUP				= 5245,				 //真火升级

	MAGICID_FABAOATTACK_XILING_WU		= 5248,				 //无属性法宝杀怪物吸灵
	MAGICID_FABAOATTACK_XILING_jin		= 5249,				 //金属性法宝杀怪物吸灵
	MAGICID_FABAOATTACK_XILING_mu		= 5250,				 //木属性法宝杀怪物吸灵
	MAGICID_FABAOATTACK_XILING_tu		= 5251,				 //土属性法宝杀怪物吸灵
	MAGICID_FABAOATTACK_XILING_shui		= 5252,				 //水属性法宝杀怪物吸灵
	MAGICID_FABAOATTACK_XILING_huo		= 5253,				 //火属性法宝杀怪物吸灵

	MAGICID_SHOWTEXT_ONHEAD				= 5254,				 //使用魔法头顶冒字
	MAGICID_FAJUE_BURNNING				= 5255,				 //法诀击中燃烧
	MAGICID_FABAO_LIANJIE				= 5256,				 //三法宝连接特效

	MAGICID_JUDUZHAOZE_DA				= 5257,				 //剧毒沼泽大
	MAGICID_JUDUZHAOZE_XIAO				= 5258,				 //剧毒沼泽小
	MAGICID_LIANJI_XP					= 5259,				 //连击XP技能
	MAGICID_SHENSHOU_SHANXING			= 5260,				 //召唤神兽扇形喷火
	MAGICID_YUNSHI_DIAOLUO				= 5261,				 //陨石掉落
	MAGICID_LIANJI_FEIXING				= 5262,				 //连击飞行特效
	MAGICID_LIANJI_FEIXING_800			= 5263,				 //连击飞行特效800分辨率

	MAGICID_LEOPARD_EFFECT				= 5264,					 //豹子出现
	MAGICID_LEOPARD_ATTACK				= 5265,					 //豹子攻击
	MAGICID_LEOPARD_ATTACK2				= 5266,					 //豹子攻击2
	MAGICID_QUEEN_EFFECT				= 5267,					 //混沌女王出现
	MAGICID_QUEEN_ATTACK				= 5268,					 //混沌女王攻击
	MAGICID_FIREMONSTER_ATTACK			= 5269,					 //火怪攻击
	MAGICID_APE_ATTACK					= 5270,					 //巨猿攻击
	MAGICID_FIREMONSTER_THROWATTACK     = 5271,					 //扔斧子火怪特效		

	MAGICID_WOLF_ATTACK = 5274,		//狼怪人形爪子攻击
	MAGICID_WOLF_FLYINGBALL = 5275,	//狼怪人形魔法球攻击
	MAGICID_Tiger_FLYINGBALL = 5276,//木灵虎魔法球攻击
	MAGICID_Tiger_ATTACK = 5277,	//木灵虎爪子攻击
	MAGICID_Tiger_COOLATTACK = 5278,//木灵虎大招?
	MAGICID_Tiger_SELF_EFFECT = 5279,//木灵虎自身特效
	MAGICID_QILIN_EARTH_ICE = 5280,	//麒麟地面冰
	MAGICID_QILIN_FLYINGBALL = 5281,//麒麟魔法球攻击
	MAGICID_QILIN_ATTACK = 5282,	//麒麟爪子攻击
	MAGICID_QILIN_SELF_EFFECT = 5283,//麒麟自身特效
	MAGICID_TULION_EARTH_CI = 5284,	//狮子地刺
	MAGICID_TULION_ATTACKED_TARGET = 5285,//狮子击中
	MAGICID_TULION_SELF_EFFECT = 5286,	//狮子自身特效
	MAGICID_WOLF_ANIMAL_ATTACK = 5287,	//狼怪兽型爪子攻击
	MAGICID_PHOENIX_FLYINGBALL = 5288,	//凤凰魔法球攻击
	MAGICID_MEINVSNAKE_FLYINGBALL = 5289,	//美女蛇魔法球攻击
	MAGICID_PHOENIX_COOLATTACK_START = 5290,	//凤凰大招起手
	MAGICID_PHOENIX_COOLATTACK = 5291,	//凤凰大招
	MAGICID_PHOENIX_ATTACK = 5292,	//凤凰爪攻击
	MAGICID_PHOENIX_SELF_EFFECT = 5293,	//凤凰自身特效
	MAGICID_SHUYAO_FLYINGBALL = 5294,	//树妖魔法球攻击
	MAGICID_DUJIAOSHOU_TOUDING = 5295,	//独角兽头顶(attack1)攻击
	MAGICID_DUJIAOSHOU_ZHUAZI = 5296,	//独角兽爪子(attack2)攻击
	MAGICID_JINGANGBAO_ZHUAZI = 5297,	//金刚豹爪子攻击
	MAGICID_TULION_ZHUAZI = 5298,		//土岩狮爪子攻击
	MAGICID_XINIU_ZHENDI = 5299,		//犀牛震地特效


	MAGICID_HDNW_LOCK_JIN              = 5300,                 //混沌女王锁链_金
	MAGICID_HDNW_LOCK_MU               = 5301,                 //混沌女王锁链_木
	MAGICID_HDNW_LOCK_TU               = 5302,                 //混沌女王锁链_土
	MAGICID_HDNW_LOCK_SHUI             = 5303,                 //混沌女王锁链_水
	MAGICID_HDNW_LOCK_HUO              = 5304,                 //混沌女王锁链_火
	MAGICID_HDNW_ATTACK				   = 5305,				   //混沌女王爪子攻击特效
	//下面的内容，必须跟服务器一致的！
	MAGICID_MON_HEIPAO_ATTACK_EFFECT  =  8192,                 //黑袍修罗攻击特效
	MAGICID_MON_HEIPAO_HEAL_EFFECT    =  8193,                 //黑袍修罗治疗特效
	MAGICID_MON_HEIPAO_RELIVE_EFFECT  =  8194,                 //黑袍修罗复活特效 

	//9001以后为法宝使用
	MAGICID_FABAO_START					 = 9000,				//法宝魔法开始
	
	//9200法阵放出特效
	MAGICID_FAZHEN_START				 = 9200,				//法阵魔法开始

	//9301以后为法宝特效
	MAGICID_FABAO_ATTACK_EFFECT_START	 = 9300,				//法宝特效开始

	//9800法阵攻击特效
	MAGICID_FAZHEN_ATTACK_START			 = 9800,				//法阵攻击特效开始

	//10001-11000为大师级-神技能特效,其它特效不要占用这些编号
	MAGICID_SHEN_FIRE_BALL		         = 10001,//小火球神	
	MAGICID_SHEN_CURE		             = 10002,//治愈术神
	MAGICID_SHEN_POISON                  = 10006,//施毒术神
	MAGICID_SHEN_ATTACK_KILL             = 10007,//攻杀剑法神
	MAGICID_SHEN_DISPUTE_FIRE            = 10008,//抗拒火环神
	MAGICID_SHEN_HELL_FIRE				 = 10009,//地狱火焰神
	MAGICID_SHEN_THUNDER				 = 10011,//雷电术神
	MAGICID_SHEN_PROTECT_SYMBOL			 = 10013,//灵魂道符神
	MAGICID_SHEN_LURE_LIGHT              = 10020,//诱惑之光神
	MAGICID_SHEN_BLOOD_SHADE		     = 10041,//血影刀法神
	MAGICID_SHEN_PROTECT_SKIN            = 10042,//护身真气神
	MAGICID_SHEN_CURSE					 = 10045,//诅咒术神
	MAGICID_SHEN_ICE_ARROW				 = 10049,//冰剑术神
	MAGICID_SHEN_STEEL_PROTECT           = 10061,//金刚护体神

	MAGICID_SHEN_WHOLE_MOON				 = 10043,// 抱月刀
	MAGICID_SHEN_ADV_FIREBALL			 = 10005,// 火炎刀
	MAGICID_SHEN_PROTECT_ARMOR			 = 10015,// 神圣战甲术
	MAGICID_SHEN_WILD_COLLIDE			 = 10027,// 野蛮冲撞
	MAGICID_SHEN_PROTECT_GHOST			 = 10014,// 幽灵盾

	MAGICID_SHEN_TRACE_THUNDER			 = 10010,// 极光电影
	MAGICID_SHEN_MAGIC_PROTECT			 = 10031,// 魔法盾
	MAGICID_SHEN_REPLACE_MAGIC			 = 10047,// 替身法符
	MAGICID_SHEN_ATTACK_MOON			 = 10025,// 半月弯刀
	MAGICID_SHEN_HELL_THUNDER			 = 10024,// 地狱雷光
	MAGICID_SHEN_BLOW_FIRE				 = 10023,// 爆烈火焰
	MAGICID_SHEN_ATTACK_STICK			 = 10012,// 刺杀剑术
	MAGICID_SHEN_ROTATE_FIRE			 = 10050,// 风火轮
	MAGICID_SHEN_FIRE_WALL				 = 10022,// 火墙
	MAGICID_SHEN_HIDE_ALL				 = 10019,// 集体隐身术
	MAGICID_SHEN_CONTROL_CORPSE			 = 10046,// 赶尸
	MAGICID_SHEN_ICE_DRAG				 = 10051,// 冰龙破
	MAGICID_SHEN_ICE_ARRAY				 = 10052,// 冰刃阵
	MAGICID_SHEN_CALL_MONSTER			 = 10017,// 骷髅召唤术

	//11001-12000为大师级-魔技能特效,其它特效不要占用这些编号
	MAGICID_MO_FIRE_BALL		         = 11001,//小火球魔
	MAGICID_MO_CURE					     = 11002,//治愈术魔 
	MAGICID_MO_POISON					 = 11006,//施毒术魔
	MAGICID_MO_ATTACK_KILL               = 11007,//攻杀剑法魔
	MAGICID_MO_DISPUTE_FIRE				 = 11008,//抗拒火环魔
	MAGICID_MO_HELL_FIRE                 = 11009,//地狱火焰魔
	MAGICID_MO_THUNDER                   = 11011,//雷电术魔
	MAGICID_MO_PROTECT_SYMBOL			 = 11013,//灵魂道符魔
	MAGICID_MO_LURE_LIGHT                = 11020,//诱惑之光魔
	MAGICID_MO_BLOOD_SHADE   	         = 11041,//血影刀法魔
	MAGICID_MO_PROTECT_SKIN				 = 11042,//护身真气魔
	MAGICID_MO_CURSE					 = 11045,//诅咒术魔
	MAGICID_MO_ICE_ARROW				 = 11049,//冰剑术魔
	MAGICID_MO_STEEL_PROTECT             = 11061,//金刚护体魔

	MAGICID_MO_WHOLE_MOON				 = 11043,// 抱月刀
	MAGICID_MO_ADV_FIREBALL				 = 11005,// 火炎刀
	MAGICID_MO_PROTECT_ARMOR			 = 11015,// 神圣战甲术
	MAGICID_MO_WILD_COLLIDE				 = 11027,// 野蛮冲撞
	MAGICID_MO_PROTECT_GHOST			 = 11014,// 幽灵盾

	MAGICID_MO_TRACE_THUNDER			 = 11010,// 极光电影
	MAGICID_MO_MAGIC_PROTECT			 = 11031,// 魔法盾
	MAGICID_MO_REPLACE_MAGIC			 = 11047,// 替身法符
	MAGICID_MO_ATTACK_MOON				 = 11025,// 半月弯刀
	MAGICID_MO_HELL_THUNDER				 = 11024,// 地狱雷光
	MAGICID_MO_BLOW_FIRE				 = 11023,// 爆烈火焰
	MAGICID_MO_ATTACK_STICK				 = 11012,// 刺杀剑术
	MAGICID_MO_ROTATE_FIRE				 = 11050,// 风火轮
	MAGICID_MO_FIRE_WALL				 = 11022,// 火墙
	MAGICID_MO_HIDE_ALL					 = 11019,// 集体隐身术
	MAGICID_MO_CONTROL_CORPSE			 = 11046,// 赶尸
	MAGICID_MO_ICE_DRAG					 = 11051,// 冰龙破
	MAGICID_MO_ICE_ARRAY				 = 11052,// 冰刃阵
	MAGICID_MO_CALL_MONSTER				 = 11017,// 骷髅召唤术


	MAGICID_SHEN_FIREWALL_LAST           = 15001,//火墙持续效果
	MAGICID_MO_FIREWALL_LAST             = 15002,//火墙持续效果

	//13001-14000为大师级-神-骑战 技能特效,其它特效不要占用这些编号
	MAGICID_SHEN_LEOPARD_FIRE_BALL		         = 13001,//小火球神	
	MAGICID_SHEN_LEOPARD_CURE		             = 13002,//治愈术神
	MAGICID_SHEN_LEOPARD_POISON                  = 13006,//施毒术神
	MAGICID_SHEN_LEOPARD_ATTACK_KILL             = 13007,//攻杀剑法神
	MAGICID_SHEN_LEOPARD_DISPUTE_FIRE            = 13008,//抗拒火环神
	MAGICID_SHEN_LEOPARD_THUNDER				 = 13011,//雷电术神
	MAGICID_SHEN_LEOPARD_PROTECT_SYMBOL			 = 13013,//灵魂道符神
	MAGICID_SHEN_LEOPARD_BLOOD_SHADE		     = 13041,//血影刀法神
	MAGICID_SHEN_LEOPARD_PROTECT_SKIN            = 13042,//护身真气神
	MAGICID_SHEN_LEOPARD_CURSE					 = 13045,//诅咒术神
	MAGICID_SHEN_LEOPARD_ICE_ARROW				 = 13049,//冰剑术神
	MAGICID_SHEN_LEOPARD_STEEL_PROTECT           = 13061,//金刚护体神
	MAGICID_SHEN_LEOPARD_ATTACK_STICK			 = 13012,// 刺杀剑术
	MAGICID_SHEN_LEOPARD_ATTACK_MOON			 = 13025,// 半月弯刀
	MAGICID_SHEN_LEOPARD_WHOLE_MOON				 = 13043,// 园月弯刀

	//14001-15000为大师级-魔-骑战 技能特效,其它特效不要占用这些编号
	MAGICID_MO_LEOPARD_FIRE_BALL		         = 14001,//小火球魔
	MAGICID_MO_LEOPARD_CURE					     = 14002,//治愈术魔 
	MAGICID_MO_LEOPARD_POISON					 = 14006,//施毒术魔
	MAGICID_MO_LEOPARD_ATTACK_KILL               = 14007,//攻杀剑法魔
	MAGICID_MO_LEOPARD_DISPUTE_FIRE				 = 14008,//抗拒火环魔
	MAGICID_MO_LEOPARD_THUNDER                   = 14011,//雷电术魔
	MAGICID_MO_LEOPARD_PROTECT_SYMBOL			 = 14013,//灵魂道符魔
	MAGICID_MO_LEOPARD_BLOOD_SHADE   	         = 14041,//血影刀法魔
	MAGICID_MO_LEOPARD_PROTECT_SKIN				 = 14042,//护身真气魔
	MAGICID_MO_LEOPARD_CURSE					 = 14045,//诅咒术魔
	MAGICID_MO_LEOPARD_ICE_ARROW				 = 14049,//冰剑术魔
	MAGICID_MO_LEOPARD_STEEL_PROTECT             = 14061,//金刚护体魔
	MAGICID_MO_LEOPARD_ATTACK_STICK				 = 14012,// 刺杀剑术
	MAGICID_MO_LEOPARD_ATTACK_MOON				 = 14025,// 半月弯刀
	MAGICID_MO_LEOPARD_WHOLE_MOON				 = 14043,// 园月弯刀



	//特殊魔法或魔法单元
	MAGICID_TIME_DELAY                = 50001,                 //延时空帧,Cycles表示帧数,(如果要延时一段时间,可以使用EMP_MAGIC_END_TIME节点属性：指示本节点根据时间结束,Cycles表示该魔法结点生存时间,单位100毫秒,配合EMP_MAGIC_LOOP一起使用)
	MAGICID_SHAKE_SCREEN              = 50002,                 //屏幕抖动的魔法单元，在用到这个魔法单元的魔法中用Cycles表示抖动的时间,单位为100毫秒
	MAGICID_GRADUAL_APPEAR            = 50003,                 //渐隐处理魔法单元，在用到这个魔法单元的魔法中用Cycles表示渐隐速度
	MAGICID_CREATE_NEW_MAGIC          = 50004,                 //创建新的魔法，在用到这个魔法单元的魔法中用Cycles表示要创建的魔法ID,创建完了结束掉当前魔法,如果还要求播放该魔法的子结点,可以把它配成子结点的兄弟结点
	MAGICID_OPEN_NEW_WND              = 50005,                  //弹出新的窗口
	MAGICID_TARGET_OWNER_CHANGE       = 50006,                  //TargetOwner互换ID
	MAGICID_HIDE                      = 50007,                  //owner或target隐身
	MAGICID_SHOW                      = 50008,                  //owner或target现身
	MAGICID_SET_MAGIC_STATE           = 50009,                  //设置magic owner或magei taget的魔法状态,有些地方要根据这个状态来判断能否移动,做其它动作等,结点的sound值表示状态值
	MAGICID_CREATE_MAGIC_WHEN_END     = 50010,                  //魔法结点结束时创建新魔法,iRev1表示要创建的魔法id
//	MAGICID_FINISH_MAGIC              = 50010,                  //结束指定的魔法，在用到这个魔法单元的魔法中用Cycles表示要创建的魔法ID,创建完了结束掉当前魔法,如果还要求播放该魔法的子结点,可以把它配成子结点的兄弟结点
	MAGICID_ALTERNATIVE				  = 50011,					//多魔法状态选一种播放
	MAGICID_CREATE_NEW_MAGIC_FABAO    = 50012,                  //创建新的魔法，针对法宝射出后,在适当帧数播放击中特效
	MAGICID_HIDE_FABAO				  = 50013,                  //使用法宝时,隐藏法宝,结束后显示法宝
	MAGICID_SEND_MESSAGE			  = 50014,                  //向服务器发消息
	//MAGICID_TEXTONHEAD				  = 50016,					//头顶冒字
	MAGICID_LIANJI_ZHUANGJI			  = 50017,					//连击撞击事件	

	//其它特殊用处的编号
	//MAGICID_SHOW_IGA                  = 60001,                 //控制是否显示IGA视频广告,如果配置文件里没有这个编号的魔法根结点则不显示IGALIVE,如有问题要暂停IGALIVE只要更新magicfile.dat文件
	MAGICID_SHOW_IGW                  = 60002,                 //控制是否载入IGW,如果配置文件里没有这个编号的魔法根结点则不载入IGW,如有问题要暂停IGW只要更新magicfile.dat文件
	MAGICID_ENABLE_REQUIT             = 60003,                 //正在跨gs的时候快速小退,此时请求角色列表会失败,如果后面收到跳gs成功的消息应该再次退重新请求角色列表,配置了这个选项后开启此功能
	MAGICID_ENABLE_CREDIT             = 60004,                 //是否开启信用功能
	MAGICID_ENABLE_SDOA               = 60005,                 //是否开启OA登录功能
	MAGICID_ENABLE_SAMPLER            = 60006,                 //是否开启客户信息收集功能
	MAGICID_ENABLE_SDERROR            = 60007,                 //是否广告监测功能

};

//[1]魔法文件采用2进制描述
//[2]record结构；标识字为FFID

#include <vector>

#define NAME_LEN 64

//	由服务端返回的魔法，id {1-100};
//	客户端特效，id{>100};

enum E_MAGIC_NODE_ATTR
{
	//图片显示分层属性	[bits = 1- 4]
	EMP_MAGIC_FLOOR			= (1<<0),			//节点属性：地表魔法
	EMP_MAGIC_BACK			= (1<<1),			//节点属性：人物后魔法
	EMP_MAGIC_FRONT			= (1<<2),			//节点属性：人物前魔法
	EMP_MAGIC_AIR 			= (1<<3),			//节点属性：空中魔法

	//位置判定属性		[bits = 5- 8]
	EMP_MAGIC_STATIC		= (1<<4),			//节点属性；未知
	EMP_MAGIC_MYSELF		= (1<<5),			//节点属性：从属于self id的魔法和特效
	EMP_MAGIC_OBJECT		= (1<<6),			//节点属性：从属于target id的魔法和特效[如果不存在，那么取目的点tile坐标]的魔法和特效
	EMP_MAGIC_FLYING		= (1<<7),			//节点属性：具有位置迁移特性，飞行速度计算？？
    
    //播放循环控制和结束;状态魔法和飞行魔法用    [bits = 9- 12]
    EMP_MAGIC_LOOP          = (1<<8),           //节点属性：允许循环播放魔法
    EMP_MAGIC_HIT_TARGET	= (1<<9),			//节点属性：用于触发target击中/受伤效果；如果没有目标不产生爆炸效果，如果没有目标也要爆炸效果请用EMP_MAGIC_HIT_MOUSE属性，需要进行hit判断，从而判定何时结束自身并启动子节点；请注意：本算法采用即时创建，暂不支持延迟创建（可能会造成和原效果有一帧的出入）
	EMP_MAGIC_OFFSCREEN		= (1<<10),			//节点属性：节点必须离开屏幕才结束
    EMP_MAGIC_END_TIME	    = (1<<11),          //节点属性：指示本节点根据时间结束,Cycles表示该魔法结点生存时间,单位100毫秒,配合EMP_MAGIC_LOOP一起使用

	//伤害触发属性		[bits = 13- 16]
	EMP_MAGIC_ATTACKED		= (1<<12),			//节点属性：首帧触发物标的被攻击行为
	EMP_MAGIC_POST_ATTACKED	= (1<<13),			//节点属性：尾帧触发物标的被攻击行为
	EMP_MAGIC_MUSIC_LOOP	= (1<<14),			//节点属性：触发循环音效
	EMP_MAGIC_SCALE			= (1<<15),			//节点属性：根据所有者比例放大
	
	EMP_MAGIC_WAIT_SERVER   = (1<<16),          //节点属性：该结点的所有子结点要求服务器回应了以后才创建,用在MagicShow中表示该魔法结点正在等待服务器回应
	EMP_MAGIC_HIT_MOUSE		= (1<<17),          //节点属性:点中鼠标位置结束,不管这个位置有没有人物目标
	EMP_MAGIC_OWNER_FLY		= (1<<18),			//节点属性：当该结点有EMP_MAGIC_FLYING加EMP_MAGIC_OWNER_FLY时那么该魔法的拥有者将随着魔法一起飞行
	EMP_MAGIC_STATE		    = (1<<19),			//节点属性：是否状态魔法，像护身真气就属于状态魔法，有时查找指定ownerid的魔法要排除这种魔法
	EMP_MAGIC_CAN_JUMPMAP   = (1<<20),          //节点属性：该魔法是否允许跨地图,否则跨地图时会结束该魔法
	EMP_MAGIC_SCREEN        = (1<<21),          //节点属性：有该属性的魔法相对于屏幕左上角坐标为零点,在屏幕上，不会随着地图移动
	EMP_MAGIC_NOTARGET      = (1<<22),          //魔法属性：没有目标的魔法,TargetID为坐标
	EMP_MAGIC_NOOWNER       = (1<<23),          //魔法属性：没有目标的魔法,OwnerID为坐标
	EMP_MAGIC_RANDOM_SONS	= (1<<24),			//节点属性：在子节点中随机创建1个，如果需要在3个中创建2个?
	EMP_MAGIC_ADJUST_OBJECT = (1<<25),          //节点属性：在EMP_MAGIC_OBJECT时有效，自动根据目标怪物的大小调整位置
	EMP_MAGIC_NO_HIDE    	= (1<<26),			//节点属性：即使魔法的owner或target隐藏也要一直显示；
	EMP_MAGIC_ADJUST_DIR 	= (1<<27),			//节点属性：飞行魔法空中方向自动调整；
	//27-29被E_MAGIC_REF_ATTR使用了
};

//------魔法show参考属性[MagicRef_t property]
enum E_MAGIC_REF_ATTR
{
	//[bits = 26-32]
	EMP_MAGIC_SERVER_ADVISE = (1<<27),			//魔法属性：本魔法仅由服务端请求创建，用于实现由服务端控制的节日等相关特效,主体ID不存在队列中！
	EMP_MAGIC_EXIT			= (1<<28),			//魔法属性：强制结束魔法；服务器返回失败；任何节点判断都会导致整个魔法树的lock down,从而frosen后继节点
	EMP_MAGIC_SERVER_SUCCESS= (1<<29),			//魔法属性：on = 服务器返回成功(此时，结束起首动作序列，启动子节点；此时，只需要从显示队列头部开始遍历；查找同create_id的魔法，删除；因为总是从头部开始创建，所以，不会存在漏过的问题。) | off = 正常进行
};


#include "Global/Global.h"

//魔法子单元结构
//采用配置表格描述，在数据处理的时候需要修正该表格
struct Magic_Unit_s
{
	Magic_Unit_s()
	{
		id = 0;
		tex = 0;
		frames = 0;
		framereal = 0;
		strcpy(name,"\0");
		dirs = 1;
		reserved2 = 0;
		package = 0;
		alpha = 0;
	}
	WORD	id;						    // 索引标识
	WORD	tex;						// 第一帧所在的位置,//	新图片格式,一组图的一个方向都默认都放在一个frame中
	WORD	frames;						// 帧数
	WORD    framereal;                  // 实际祯数
	char	name[NAME_LEN];				// 魔法单元名称

	//保留字更新到新的名字
	BYTE	dirs;
	int     package;                    // 图包编号,以PACKAGE_magic1为参考点PACKAGE_magic1为0,MONSTEREX为-1
	BYTE    alpha;                      // 混合方式
	BYTE    reserved2;                  //

	//对应的音效编码
	WORD	attr;						// 属性标签,2004-8-30
};

//----------------------------------生成树-----------------------------------//

//魔法树节点,内存结构,[固定]；

//[1]魔法根节点unit_idx为空
//[2]用于描述一个魔法的所有组成部分;
//[3]用于控制显示结构的自动生成;显示结构在显示完成当前的节点才考虑下一级；显示结构必须包含一个当前节点指针
//[4]文件存储;脚本??二进制

struct Magic_Node_s
{
	Magic_Node_s()
	{
		unit_idx = 0;
		sibling = NULL;
		child = NULL;
		parent = NULL;
		attr = 0;
		cycle = 0;
		speed = 1;
		flyspeed = 1;
		offx = offy = 0;
		sound = 0;
		byRev1 = 0;
		iRev1 = 0;
	}
	WORD   unit_idx ;			// 当前索引--来自magic unit table
	WORD   cycle;               // 有限循环次数
	DWORD  attr;     			// 属性标签--是否允许循环；是否地表；是否为静态（创建属性）
	WORD   speed;				// 速度-采用循环总帧数表示，实际显示的时候，通过计算来获得实际显示的帧。
	WORD   flyspeed;            // 飞行移动的速度
	short  offx,offy;			// 帧偏移
	WORD   sound;				// 音效
	BYTE   byRev1;              // 保留字段
	int    iRev1;               // 保留字段

	Magic_Node_s* parent;	    // 父亲节点，根节点 == NULL
	Magic_Node_s* sibling;	    // 兄弟节点单向列表，指向第一个兄弟，根节点 == NULL
	Magic_Node_s* child;		// 子节点列表，指向第一个儿子，第2个儿子为第一个儿子的兄弟，二叉树的形式
};
//显示节点的创建：
//根据Magic_Node_s 根节点来创建时，直接遍历所有的child

//如果参考该节点的魔法show结束；那么将根据该

//显示的时候，对于每个命名的魔法，必须维护一个活动列表，用来直观的表示该魔法的显示状态
//为了查找方便，该列表采用数组索引的方法

//魔法结构；
//由配置文件读取
//基本单位是魔法单元（一个帧序列）
//每个魔法由一个十字链表串接魔法单元而成。但是要怎样表示同时播放的魔法？
//一种结构是，多叉树结构所有高度为n的节点先放；
//build一棵魔法树；
//魔法树需要动态创建？
//显示结构维护属性指针 ----指向根魔法标志位；这样的帧循环的时候，如果标志位被置ending，
//那么；所有参考该标志位的魔法（后继显示）都将自动停止
//清空操作将在帧循环的过程中自动删除；

//魔法将以id为序；
struct Magic_Root_s
{
	Magic_Root_s()
	{
		id = 0;
		attr = 0;
		strcpy(name,"\0");
		d1 = d2 = d3 = d4 = 0;
		m = NULL;
	}
	WORD   id;						// 标识
	DWORD  attr;					// 属性标签；控制显示，如果为静态，
	char   name[NAME_LEN];			// 魔法名称

	Magic_Node_s*  m;				// 子树

	int	 d1,d2,d3,d4;				// 附加属性
};

//[1]为了有效控制每个显示结构；在总的活动列表中，应该增加一个引用计数结构Magic_Ref_s
//特效显示公共属性
#define NEW_MAGIC_REF()		Magic_Ref_s::NEW_Magic_Ref_s()
#define DEL_MAGIC_REF(p)	Magic_Ref_s::DEL_Magic_Ref_s(p)

struct Magic_Show_s;

struct Magic_Ref_s
{
	DECLARE_POOL(Magic_Ref_s);	// 缓冲

	Magic_Ref_s()
	{
		create_id = 0;
		attr = 0;
		create_time = 0;
		life_time = 0;
		ref_count = 0;
		wReserved = 0;
		wMagicID  = 0;
		wExtraAttr = 0;
		uOwnerID  = 0;
		wOriginTileX = 0;
		wOriginTileY = 0;
		uTargetID = 0;
		wTargetTileX = 0;
		wTargetTileY = 0;
		iOffX = 0;
		iOffY = 0;
		iDir = 0;
		pTarget = NULL;
		ppShowList = NULL;
		pDynData = NULL;
	}

	DWORD	create_id;					// 可以根据该标识来查找所有的效果部分
	DWORD	attr;					    // 属性控制;服务器返回状态;

	DWORD	create_time;				// 创建时间;(GetTickCount：单位，ms)
	DWORD	life_time;					// 生命期;(非过程魔法适用)

	WORD	ref_count;					// 引用计数;用于控制自删除;
	WORD	wReserved;					// 保留字;(1.sice 用于位置控制)

	WORD	wMagicID;					// 魔法ID
	WORD	wExtraAttr;					// 魔法附加类型|属性
	UINT	uOwnerID;					// 使用者ID
	UINT	uTargetID;					// 目标ID
	WORD	wTargetTileX,wTargetTileY;	// 目标格的坐标：创建时，必须填充。
	WORD	wOriginTileX,wOriginTileY;	// 发出位置坐标：
	int     iOffX,iOffY;                // 整个魔法的偏移,所有子结点都有效
	int     iDir;                       // 方向


	STargetData *pTarget;				// 攻击目标链表
	Magic_Show_s** ppShowList;          // 这个魔法的根结点,所有从这个魔法创建的子结点都会加入到pShowList
	char *pDynData;                     //动态生成的数据
	WORD	wData[4];					// 附加数据[结构补齐16*4 = 64字节]
};

//魔法显示控制	104bytes
//仍然根据id来识别本魔法；所有的显示状态参数将根据show_node来自动获取；
//Magic_Show_s运行结束时，将创建show_node所有子女的Magic_Show_s
#define NEW_MAGIC_SHOW()	Magic_Show_s::NEW_Magic_Show_s()
#define DEL_MAGIC_SHOW(p)	Magic_Show_s::DEL_Magic_Show_s(p)

struct Magic_Show_s
{
	DECLARE_POOL(Magic_Show_s);			// 缓冲

	Magic_Show_s()
	{
		show_node	= NULL;
		ref			= NULL;
		pUnit       = NULL;

		attr        = 0;
		fx = fy	= 0.0f;
		fData1 = fData2 = 0;
		tFrameStart		= 0;
		iFrameDir       = 0;
		iFrameCount		= 0;
		iFrameReal      = 0;
		iDir            = 0;
		iFrameSpeed     = 0;
		iCycles         = 0;
		iCurCycle       = 0;
		byAlpha         = 0; //RM_ALPHA

		iData1 = iData2 = iData3 = iData4 = 0;
		iOffX = iOffY = 0;
		wSound = 0;
		byRev1 = 0;
		iRev1 = 0;

		dwLastTex	= 0;
		nSoundRand	= 0;
		dwColor = 0xFFFFFFFF;
		pPrev = NULL;
		pNext = NULL;
	}

	//参数控制；一个魔法同时有多个显示效果
	Magic_Node_s* show_node;	        // 指向图量节点;不可能指向root;
	Magic_Ref_s * ref;				    // 参考计数；任何一个show，都可以直接控制本棵显示树的删除；后继show节点将自动不再显示；
	Magic_Unit_s *pUnit;                //对应的魔法单元

	DWORD	attr;					    // 属性控制;服务器返回状态;

	//飞行魔法的参数
	float fx,fy;						// 一般用来纪录累计的坐标偏移
	float fData1,fData2;				// 计算偏移

	//帧数控制的参数
	DWORD tFrameStart;				    // 开始魔法当前循环的开始时间,开始一个新的循环后会重置
	DWORD dwStartTime;                  // 该魔法的开始时间,开始一个新的循环也不会重置

	int   iFrameDir;                    // 两个方向之间的图片间隔数
	int   iFrameCount;				    // 当前show的总帧数,iFrameReal到iFrameCount之间的帧不显示
	int   iFrameReal;                   // 当前show的有图片的实际帧数
	int   iDir;                         // 当前show的方向
	int	  iFrameSpeed;				    // 变帧速度，以ms为单位
	int	  iFlySpeed;				    // 变帧速度，以ms为单位
	WORD  iCycles;                      // 有限循环次数
	WORD  iCurCycle;                    // 当前处于第几次循环
	BYTE  byAlpha;                      // 混合方式

	//附加属性的参数
	int	 iData1,iData2,iData3,iData4;	// 用于记录其它魔法信息的变量

	DWORD dwLastTex;                    // 上次使用的纹理帧
	DWORD dwColor;                      // 绘制魔法的颜色
	int iOffX,iOffY;                    // 偏移量
	WORD wSound;                        // 音效

	UINT  nSoundRand;					// 声音播放索引；用于控制音效结束
	BYTE   byRev1;              // 保留字段
	int    iRev1;               // 保留字段

	Magic_Show_s* pPrev;
	Magic_Show_s* pNext;

	bool IsEnd(DWORD tTime);
	int  GetFrame(DWORD tTime);			//计算当前帧
};

#include <map>

typedef std::map<WORD,Magic_Root_s>   MMagic_Root_s;
typedef std::map<WORD,Magic_Unit_s>   MMagic_Unit_s;
