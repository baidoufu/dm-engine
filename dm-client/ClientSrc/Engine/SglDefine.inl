PACKAGE_BEGIN

//PACKAGE(SmTiles	 , 1)	// 小地表
//PACKAGE(Tiles	 , 2)
PACKAGE(INTERFACE, 3)
PACKAGE(objects1 , 5)
PACKAGE(objects2 , 6)
PACKAGE(objects3 , 7)
PACKAGE(objects4 , 8)

PACKAGE(cursor	 , 10)	// 光标
PACKAGE(mask	 , 11)
PACKAGE(mmap	 , 12)	// 小地图
PACKAGE(timap	 , 13)	// Tiny地图

PACKAGE(dnitems	 , 15)
PACKAGE(items    , 16)
PACKAGE(stateitem, 17)
PACKAGE(ptitems		, 18)	// 物品
PACKAGE(bgitems		, 19)	// 物品
PACKAGE(items2    , 21)
PACKAGE(stateitem2, 22)

PACKAGE(objects5	, 33)
PACKAGE(objects6	, 34)
PACKAGE(objects7	, 35)
PACKAGE(objects8	, 36)
PACKAGE(objects9	, 37)
PACKAGE(objects10	, 38)
PACKAGE(objects11	, 39)
PACKAGE(objects12	, 40)
PACKAGE(objects13	, 41)
PACKAGE(objects14	, 42)
PACKAGE(objects15	, 43)
PACKAGE(objects16	, 44)
PACKAGE(objects17	, 45)
PACKAGE(objects18	, 46)//这个图包里的物体用add模式,
PACKAGE(objects19	, 47)//这个图包里的物体用add2模式
//OBJECTS包的编号不可以大于 255,因为有些包是用一个word的低字节表示包的编号，如object,DrawObjectTexture(sx,y,pTile->uObject & OT_OBJECT_MASK);(OT_OBJECT_MASK     = 0x00FFFFFF,)

PACKAGE(effect		, 49)	// 地表特效
PACKAGE(others		, 50)	// 其他

PACKAGE(human1		, 51)
PACKAGE(human2	    , 52)
PACKAGE(human3		, 53)
PACKAGE(pattern1	, 56)	// 外套
PACKAGE(pattern2	, 57)	// 外套
PACKAGE(hair1		, 61)
PACKAGE(hair2       , 62)
PACKAGE(weapon1		, 66)
PACKAGE(weapon2		, 67)
PACKAGE(npc1		, 71)
PACKAGE(npc2		, 72)
PACKAGE(hairex1		, 75)
PACKAGE(hairex2		, 76)
PACKAGE(hairexn1	, 78)
PACKAGE(hairexn2	, 79)
PACKAGE(humanex1    , 81)
PACKAGE(humanex2    , 82)
PACKAGE(patternex1  , 84)
PACKAGE(patternex2  , 85)
PACKAGE(patternexn1	, 88)
PACKAGE(patternexn2	, 89)

PACKAGE(monster1	, 91)	// 怪物
PACKAGE(monster2	, 92)	// 怪物
PACKAGE(monster3	, 93)	// 怪物
PACKAGE(monster4	, 94)	// 怪物
PACKAGE(monster5	, 95)	// 怪物
PACKAGE(monster6	, 96)	// 怪物
PACKAGE(monster7	, 97)	// 怪物
PACKAGE(monster8	, 98)	// 怪物


PACKAGE(monsterex1	, 101)	// 怪物
PACKAGE(monsterex2	, 102)
PACKAGE(monsterex3  , 103)
PACKAGE(monsterex4  , 104)
PACKAGE(monsterex5  , 105)
PACKAGE(monsterex6  , 106)
PACKAGE(monsterex7  , 107)
PACKAGE(monsterex8  , 108)
PACKAGE(monsterex9  , 109)

PACKAGE(monsterexn1	, 111)
PACKAGE(monsterexn2 , 112)
PACKAGE(monsterexn3 , 113)
PACKAGE(monsterexn4 , 114)
PACKAGE(monsterexn5 , 115)
PACKAGE(monsterexn6 , 116)
PACKAGE(monsterexn7 , 117)
PACKAGE(monsterexn8 , 118)
PACKAGE(monsterexn9 , 119)


PACKAGE(magic1		, 121)	// 魔法1
PACKAGE(magic2		, 122)	// 魔法2
PACKAGE(magic3		, 123)
PACKAGE(magic4		, 124)


PACKAGE(weaponex1	, 130)
PACKAGE(weaponex2	, 131)
PACKAGE(weaponexn1	, 135)
PACKAGE(weaponexn2	, 136)

PACKAGE(weaponeff1	, 140)  // 武器weapon1特效
PACKAGE(weaponeff2  , 141)  // 武器weapon2特效
PACKAGE(weaponeffex1, 144)  //武器特效EX
PACKAGE(weaponeffex2, 145)  //武器特效EX
PACKAGE(weaponeffexn1,148)
PACKAGE(weaponeffexn2,149)


PACKAGE(weaponeffexo1,152)
PACKAGE(weaponeffexo2,153)

PACKAGE(humanexn1	, 160)
PACKAGE(humanexn2	, 161)


PACKAGE(weaponexo1	, 180)
PACKAGE(weaponexo2	, 181)
PACKAGE(weaponexo3	, 182)
PACKAGE(weaponexo4	, 183)
PACKAGE(weaponexo5	, 184)

PACKAGE(monsterexo1	, 190)
PACKAGE(monsterexo2 , 191)
PACKAGE(monsterexo3 , 192)
PACKAGE(monsterexo4 , 193)
PACKAGE(monsterexo5 , 194)
PACKAGE(monsterexo6 , 195)
PACKAGE(monsterexo7 , 196)
PACKAGE(monsterexo8 , 197)


PACKAGE(OBJECTS20	, 210)
PACKAGE(OBJECTS21	, 211)
PACKAGE(OBJECTS22	, 212)
PACKAGE(OBJECTS23	, 213)
PACKAGE(OBJECTS24	, 214)
PACKAGE(OBJECTS25	, 215)

//255之前的图包请不要用做其它用途,objects图包不能大于255,给他用

PACKAGE(weapon8      , 300)
PACKAGE(weaponex8    , 364)
PACKAGE(weaponexn8   , 369)
PACKAGE(weaponexo8   , 414)

PACKAGE(weaponeff8   , 374)
PACKAGE(weaponeffex8 , 378)
PACKAGE(weaponeffexn8, 382)
PACKAGE(weaponeffexo8, 386)

PACKAGE(weapon9      , 301)
PACKAGE(weaponex9    , 365)
PACKAGE(weaponexn9   , 370)
PACKAGE(weaponexo9   , 415)

PACKAGE(weaponeff9   , 375)
PACKAGE(weaponeffex9 , 379)
PACKAGE(weaponeffexn9, 383)
PACKAGE(weaponeffexo9, 387)

PACKAGE(human8       , 451)
PACKAGE(humanex8     , 481)
PACKAGE(humanexn8    , 560)

PACKAGE(humanexo1    , 601)
PACKAGE(humanexo2    , 602)
PACKAGE(humanexo3    , 603)
PACKAGE(humanexo4    , 604)

PACKAGE(humanexo8    , 608)
PACKAGE(humanexo9    , 609)


PACKAGE(patternexo1	,  621)
PACKAGE(patternexo2	,  622)
PACKAGE(patternexo3	,  623)

PACKAGE(patternexo8	,  628)
PACKAGE(patternexo9	,  629)

PACKAGE(hairexo1	, 651)
PACKAGE(hairexo2	, 652)



PACKAGE(monstereff1	, 1091)	// 怪物
PACKAGE(monstereff2	, 1092)	// 怪物
PACKAGE(monstereff3	, 1093)	// 怪物
PACKAGE(monstereff4	, 1094)	// 怪物
PACKAGE(monstereff5	, 1095)	// 怪物
PACKAGE(monstereff6	, 1096)	// 怪物
PACKAGE(monstereff7	, 1097)	// 怪物
PACKAGE(monstereff8	, 1098)	// 怪物

PACKAGE(monstereffex1  , 1101)	// 怪物特效图包=怪物图包加1000
PACKAGE(monstereffex2  , 1102)
PACKAGE(monstereffex3  , 1103)
PACKAGE(monstereffex4  , 1104)
PACKAGE(monstereffex5  , 1105)
PACKAGE(monstereffex6  , 1106)
PACKAGE(monstereffex7  , 1107)
PACKAGE(monstereffex8  , 1108)
PACKAGE(monstereffex9  , 1109)

PACKAGE(monstereffexn1 , 1111)
PACKAGE(monstereffexn2 , 1112)
PACKAGE(monstereffexn3 , 1113)
PACKAGE(monstereffexn4 , 1114)
PACKAGE(monstereffexn5 , 1115)
PACKAGE(monstereffexn6 , 1116)
PACKAGE(monstereffexn7 , 1117)
PACKAGE(monstereffexn8 , 1118)
PACKAGE(monstereffexn9 , 1119)

PACKAGE(monsterffeexo1 , 1190)
PACKAGE(monstereffexo2 , 1191)
PACKAGE(monstereffexo3 , 1192)
PACKAGE(monstereffexo4 , 1193)
PACKAGE(monstereffexo5 , 1194)
PACKAGE(monstereffexo6 , 1195)
PACKAGE(monstereffexo7 , 1196)
PACKAGE(monstereffexo8 , 1197)

PACKAGE(humaneff1		,2051)// 衣服特效图包=衣服图包加2000
PACKAGE(humaneff2	    ,2052)

PACKAGE(humaneffex1		,2081)
PACKAGE(humaneffex2	    ,2082)

PACKAGE(humaneffexn1	,2160)
PACKAGE(humaneffexn2    ,2161)

PACKAGE(humaneffexo1    ,2601)
PACKAGE(humaneffexo2    ,2602)
PACKAGE(humaneffexo3    ,2603)
PACKAGE(humaneffexo4    ,2604)



PACKAGE(SmTiles	, 3001)//小地表
PACKAGE(SmTiles2, 3002)//小地表
PACKAGE(SmTiles3, 3003)//小地表
PACKAGE(SmTiles4, 3004)//小地表
PACKAGE(SmTiles5, 3005)//小地表
PACKAGE(SmTiles6, 3006)//小地表
PACKAGE(SmTiles7, 3007)//小地表
PACKAGE(SmTiles8, 3008)//小地表
PACKAGE(SmTiles9, 3009)//小地表
PACKAGE(SmTiles10,3010)//小地表

PACKAGE(Tiles	 ,3051)//大地表
PACKAGE(Tiles2	 ,3052)//大地表
PACKAGE(Tiles3	 ,3053)//大地表
PACKAGE(Tiles4	 ,3054)//大地表
PACKAGE(Tiles5	 ,3055)//大地表
PACKAGE(Tiles6	 ,3056)//大地表
PACKAGE(Tiles7	 ,3057)//大地表
PACKAGE(Tiles8	 ,3058)//大地表
PACKAGE(Tiles9	 ,3059)//大地表
PACKAGE(Tiles10	 ,3060)//大地表


PACKAGE(FaBao1	 ,4000)//法宝
PACKAGE(FaBaoEff1	 ,4100)//法宝

PACKAGE(monsterarm1 , 4300)	//骑兽盔甲--1转
PACKAGE(monsterarmex1 , 4301)
PACKAGE(monsterarmexn1 , 4302)
PACKAGE(monsterarmexo1 , 4303)

//objects包的编号不可以大于 255,因为有些包是用一个word的低字节表示包的编号，如object,DrawObjectTexture(sx,y,pTile->uObject & OT_OBJECT_MASK);(OT_OBJECT_MASK     = 0x00FFFFFF,)

PACKAGE_END

#undef	PACKAGE_BEGIN
#undef	PACKAGE
#undef	PACKAGE_END
