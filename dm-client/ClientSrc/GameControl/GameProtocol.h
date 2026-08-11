#pragma once

////////////////////////////////////////////////////////////////////
//服务器端发送到客户端消息
////////////////////////////////////////////////////////////////////

//小宝石合成消息
#define CS_SMALL_GEM_REQUEST               0x02B0       //查询宝石数量
#define CS_SMALLGEM_CONDENSE_REQUEST       0x03E1       //请求宝石合成
#define SC_SMALL_GEM_COUNT                 0x02B0       //答复宝石数量
//盛大积分
#define CS_SNDAMARK_REQUEST                0xE67A       //查询盛大积分
#define SC_SNDAMARK_ANSWER                 0xE67A       //答复盛大积分


//摆摊消息
#define SC_BOOTH_REQUEST_RESULT             0x80D7      //请求摆摊结果
#define SC_BOOTH_SHOW_OTHER                 0xFCA0      //显示其他人的摊位

#define SC_C2C_BOOTH_INFO                   0x80D0      //显示其它豹子的摊位拍卖行信息
#define SC_PET_GOODINFO                     0x88A7       //宠物的附加属性

//玩家和NPC进行交互消息
//      消息命令							消息值		消息注释
#define SC_EXCHANGE_TALK_MENU				0x0283		//弹出交互的菜单
#define SC_EXCHANGE_TALK_END				0x0284		//关闭交互菜单
#define SC_EXCHANGE_BUY_MENU				0x0285		//弹出购买列表
#define SC_EXCHANGE_BUY_FAIL				0x028B		//不能购买
#define SC_EXCHANGE_BUY_OK					0x028A		//购买成功后剩余的钱
#define SC_EXCHANGE_SALE_WINDOW				0x0286		//弹出出售列表
#define SC_EXCHANGE_SALE_OBJECT				0x0287		//把出售物品放入托盘
#define SC_EXCHANGE_SALE_OK					0x0288		//出售成功
#define SC_EXCHANGE_SALE_FAIL				0x0289		//不能出售此物品
#define SC_EXCHANGE_REPAIR_WINDOW			0x029C		//弹出修理窗口
#define SC_EXCHANGE_REPAIR_SUCCESS			0x029D		//修理成功
#define SC_EXCHANGE_REPAIR_FAIL				0x029E		//修理失败
#define SC_EXCHANGE_REPAIR_MONEY			0x029F		//修理所需的金钱
#define SC_EXCHANGE_REPAIR_DURATION         0x01FD      //服务器通知修改物品的持久
#define SC_EXCHANGE_STORAGE_WINDOW			0x02BC		//弹出保管窗口
#define SC_EXCHANGE_GETBACK_WINDOW			0x02C0		//弹出找回物品窗口
#define SC_EXCHANGE_STORAGE_OK				0x02BD		//保管成功
#define SC_EXCHANGE_STORAGE_FAILED			0x02BE		//保管失败
#define SC_EXCHANGE_GETBACK_OK				0x02C1		//从仓库找回物品成功
#define SC_EXCHANGE_GETBACK_FAIL			0x02C3		//取回物品失败
#define SC_EXCHANGE_POISON_MENU				0x02C8		//弹出毒药窗口
#define SC_EXCHANGE_POISON_NEED				0x02C5		//制造毒药所需物品
#define SC_EXCHANGE_POISON_FAIL				0x02CA		//制造毒药所需物品不全
#define SC_EXCHANGE_POISON_MONEY			0x02C9		//值造毒品后剩余金钱数
#define SC_EXCHANGE_SECOND_BUY_MENU			0x028C		//按ok后出现的二级购买列表
#define SC_BAD_GOOD							0x026A		//假物品
#define SC_EXCHANGE_FORGE_WINDOW			0x0265		//打开锻造窗口
#define SC_EXCHANGE_FORGE_PERCENT			0x0281		//锻造百分比and 出错提示
#define SC_OTHER_MESSAGES					0x22ED		// 关键情节提示
#define SC_EXCHANGE_FORGE_RESULT			0x0024		//锻造成功或者失败
#define SC_EXCHANGE_FORGE_EXPERIENCE		0x002B		//经验值信息
#define SC_EXCHANGE_BLESS_RESULT			0x0025		//祝福成功或者失败
#define SC_EXCHANGE_GEM_RESULT              0x0270      //宝石凝练/合成成功或者失败
#define SC_EXCHANGE_BOX_RESULT				0x0273      //开启宝箱/确定宝箱物品
#define CS_OPEN_BOX_REQUEST                 0x0273      //请求开启宝箱/请求获得宝箱物品
#define SC_EXCHANGE_LIST_MENU               0x9592      //英雄榜
#define CS_SUBMIT_PAIMAI_GOOD               0x0713      //提交拍卖物品
#define SC_SUBMIT_GOOD_INFO                 0x0713      //拍卖物品的具体信息
#define SC_SUBMIT_PAIMAI_RESULT             0x0714      //拍卖物品的结果
#define SC_PAIMAI_SALE_RESULT               0x80D1      //卖方的物品交易成功
#define SC_EXCHANGE_TRAY_WND				0x8850		//服务器要求客户端弹出一个托盘
#define SC_RETURN_RESULT					0x9590		//服务器端返回各种成功和失败消息
#define SC_PACKAGE_CAPACITY					0x9594		//包裹大小


//玩家和玩家进行的交易消息
//      消息命令							消息值		消息注释
#define SC_TRADE_ENABLE						0x02A1		//可以进行交易
#define SC_TRADE_DISABLE					0x02A2 		//不可以交易
#define SC_TRADE_FAILED						0x02A9 		//交易失败
#define SC_TRADE_OBJECT_CONFIRM				0x02A3 		//本方放入物品的确认
#define SC_TRADE_PLAYER_OBJECT_INFO			0x02AA 		//对方放入物品信息
#define SC_TRADE_PLAYER_OBJECT_TAKEOUT		0x02AB 		//对方放入物品信息
#define SC_TRADE_MONEY_INFO					0x02AC 		//本方放入的金钱的信息
#define SC_TRADE_PLAYER_MONEY_INFO			0x02AE 		//对方放入金钱的信息
#define SC_TRADE_SUCCESS					0x02AF 		//交易成功
#define SC_TRADE_TAKEIN_FAILED				0x02A4 		//放入物品失败

//玩家包裹中的相关操作消息
//      消息命令							消息值		消息注释
#define SC_PACKAGE_ADD_OBJECT				0x00C8 		//包裹中增加了一物品
#define SC_PACKAGE_UPDATE_OBJECT			0x00CB 		//更新物品属性
#define SC_PACKAGE_REJECT_FAIL				0x0259 		//扔掉一物品失败
#define SC_PACKAGE_ALL_INFO					0x00C9 		//包裹中所有物品的信息
#define SC_PACKAGE_REJECT_CONFIRM			0x0258 		//扔掉一物品成功

//物品相关消息
//      消息命令							消息值		 消息注释
#define SC_OBJECT_APPEARANCE				0x0262 		//一物品在地上的出现消息
#define SC_OBJECT_DISAPPEARANCE				0x0263 		//以物品在地上消失的消息
#define SC_OBJECT_USE_SUCCESS				0x027B 		//使用物品成功
#define SC_OBJECT_USE_FAILED				0x027C 		//使用物品失败
#define SC_OBJECT_DURA						0x0282 		//当前物品的持久力
#define SC_OBJECT_TAKE						0x00CA 		//服务器收走包裹里的物品
#define SC_OBJECT_DETAIL					0x2230		//物品的详细信息
#define CS_OBJECTS_POSITION					0x0045		//向物品端发送包裹中物品的位置
#define SC_OBJECT_EXTERN_INFO				0x00CE		//物品需要放特效及特殊字串属性
#define SC_YUANBAO_INFO						0xE679		//元宝信息
#define SC_BIND_YUANBAO_INFO                0xE680      //绑定的元宝信息
#define SC_CREDIT_INFO                      0xE681      //信用信息
#define SC_CREDIT_REQ					    0x033E		//购买道具元宝不足时询问玩家是否用信用购买请求，没有开通信用跳到igw提示通
#define SC_CREDIT_TOKEN					    0x033F		//信用购买数字签名串

#define CS_OBJECT_KEEPING					0x0407		//保管物品
#define CS_OBJECT_GETBACK					0x0408		//找回物品
#define CS_MAKE_POISON						0x040A		//选择你所要制造的毒药

#define SC_PRENTICE_FLAG                    0x00C4     //消息队列中徒弟是否有玄铁令牌的标志
//怪物(包括NPC和其他玩家)显示想关消息
//      消息命令							消息值		消息注释
#define SC_MONSTER_APPEARANCE				0x000A 		//怪物出现的消息
#define SC_MONSTER_DISAPPEARANCE			0x001E 		//怪物消失的消息
#define SC_MONSTER_NAME						0x002A 		//怪物名称
#define SC_MONSTER_WALK						0x000B 		//怪物走动消息
#define SC_MONSTER_RUN						0x000D 		//怪物跑动消息
#define SC_MONSTER_HURTED					0x001F 		//怪物受攻击后伤害情况
#define SC_MONSTER_HURTED2					0x002F 		//怪物受攻击后伤害情况
#define SC_MONSTER_NAMECOLOR				0x0290 		//怪物名字颜色的改变
#define SC_MONSTER_STATE					0x0291 		//怪物状态(中毒,隐身等)
#define SC_MONSTER_REFRESH					0x0029 		//怪物外观重新更新(更换衣服,方向等)
#define SC_MONSTER_DEAD						0x0022 		//怪物死亡
#define SC_MONSTER_CORPUS					0x0020		//出现怪物尸体
#define SC_MONSTER_BONE						0x0021		//出现骨头
#define SC_MONSTER_ATTACK1					0x000E		//怪物单手攻击
#define SC_MONSTER_ATTACK2					0x000F		//怪物双手攻击
#define SC_MONSTER_ATTACK3					0x0010		//怪物跳砍攻击
#define SC_MONSTER_ATTACK_MAGIC				0x0011		//其他玩家或者怪物开始使用魔法攻击
#define SC_MONSTER_ATTACK_KILL				0x0012		//其他玩家使用攻杀
#define SC_MONSTER_ATTACK_STICK				0x0013		//其他玩家使用刺杀
#define SC_MONSTER_ATTACK_MOON				0x0018		//其他玩家使用半月
#define SC_MONSTER_ATTACK_FIRE				0x0008		//其他玩家使用烈火
#define SC_MONSTER_CUT						0x027D		//其他玩家挖肉动作
#define SC_MONSTER_BACK 					0x0009		//被其他玩家使用魔法弹开
#define SC_MONSTER_BACK1					0x0005		//被其他玩家用狮子吼推开
#define SC_MONSTER_BACK2					0x0BED		//毒凌波，被其他玩家用毒凌波推开
#define SC_OTHER_PLAYER_INFO				0x02EF		//其它玩家的信息
#define SC_MONSTER_ANIMATE_APPEAR			0x0014 		//怪物被召唤出来
#define SC_MONSTER_ANIMATE_DISAPPEAR		0x0015		//怪物动画消失
#define SC_CORPSE_RELIVE					0x001B		//僵尸复活
#define SC_MONSTER_MAGIC_FLY				0x00AA		//魔法师的变身飞行
#define SC_MONSTER_ATTACK_SWORD				0x0019		//其他玩家使用新加的剑法
#define SC_MONSTER_PROTECTOR_COLOR			0x001D		//护身真气的属性改变
#define SC_MONSTER_SOULWALL_STATE			0x0323		//灵魂之墙出现和消失
#define SC_MONSTER_EXTRASTATE               0x532C      //金刚护体,风行术的状态开关
#define SC_PLAY_ATTACKED_SOUND              0x9CBF      //请求播放破甲,破盾生效音效
#define SC_SPECIAL_MONSTER_EFFECT           0x044D      //player 特殊攻击效果
#define SC_MONSTER_MAGIC_EFFECT             0x0271      //怪物的特殊魔法效果（治疗，复活其他怪物）
#define SC_MONSTER_STOPGO					0x00CC      // 结束走路
#define SC_MONSTER_MOVEPOSITION				0x000C		// 移动位置的魔法


//玩家信息相关的消息
//      消息命令							消息值		消息注释
#define SC_PLAYER_ARM_INFO					0x026D 		//身上所有装备物品信息	SM_SENDUSEITEMS 621
#define SC_PLAYER_ARM_REFRESH				0x0267 		//装备成功刷新
#define SC_PLAYER_ARM_FAIL					0x0268		//装备失败
#define SC_PLAYER_UNARM_REFRESH				0x026B 		//卸装成功
#define SC_PLAYER_UNARM_FAIL				0x026C		//卸装失败
#define SC_PLAYER_INFO1						0x026E 		//当前角色信息(负重量,腕力等)		622
#define SC_PLAYER_INFO2						0x02F0 		//当前角色信息(精确度,敏捷度等)	752
#define SC_PLAYER_ATTRIBUTE					0x0034 		//角色信息			SM_ABILITY	52
#define SC_PLAYER_LIFE						0x0035 		//生命状态
#define SC_PLAYER_EXPERIENCE_UP				0x002C 		//经验值增加
#define SC_PLAYER_RANK_UP					0x002D 		//等级升高
#define SC_PLAYER_REFRESH					0x0032 		//人物外观更新
#define SC_PLAYER_POSITION					0x0033 		//人物在地图的位置
#define SC_PLAYER_MONEY						0x028D 		//身上金钱数量
#define SC_PLAYER_SKILL_ADD					0x00D2		//技能增加
#define SC_PLAYER_SKILL_DELETE				0x00D4		//技能删除
#define SC_PLAYER_AUTO_ARM					0x0023		//装备包裹中的物品
#define SC_PLAYER_SWORD_SWITCH				0x8080		// 剑术开关
#define SC_PLAYER_USE_SWORD					0x8858		// 随机的下一招剑术
#define SC_PLAYER_DIG_BREAK					0x4888		// 挖矿掉渣
#define SC_PLAYER_PET_STATE					0x6858 		// 宠物状态
#define SC_PLAYER_MAGIC_COLOR				0xB4A4		// 魔法颜色
#define SC_PLAYER_TASK_INFO                 0x9596      // 任务信息


// 组队相关操作消息
//      消息命令							消息值      消息注释
#define SC_GROUP_ENABLE						0x0293 		//是否允许组队
#define SC_GROUP_COMPANY_OK					0x0294 		//组队返回成功
#define SC_GROUP_COMPANY_FAILED				0x0295 		//组队返回失败
#define SC_GROUP_MEMBER_DELETED_NAME		0x0297 		//被删除队员名
#define SC_GROUP_MEMBER_DELETE_OK			0x029A 		//删除队员成功
#define SC_GROUP_MEMBER_DELETE_FAILED		0x0299 		//删除队员失败
#define SC_GROUP_MEMBER_ADD_FAILED			0x0298 		//加入队员失败
#define SC_GROUP_MEMBER_INFO				0x029B 		//返回所有队员信息
#define SC_GROUP_MEMBER_POS					0x02E0		//返回队员位置信息
#define SC_VOICECHAT_ENABLE					0x02E1		//是否允许语音聊天
#define SC_GROUP_REQUEST					0x02E2		//其他人发出组队邀请
#define CS_ANSWER_GROUP_REQUEST				0x02E2		//回应其他人发出组队邀请

//行会相关操作消息
//      消息命令							消息值		消息注释
#define SC_GUILD_RET_EXP_VAL				0x02CD		//返回行会经验值
#define SC_GUILD_MESSAGE					0x02F1 		//行会公告等信息
#define SC_GUILD_MESSAGE_FAIL				0x02F2 		//没有加入行会
#define SC_GUILD_MEMBER_LIST				0x02F4 		//行会成员信息
#define SC_GUILD_ADDIN_GUILD_SUCCESS		0x02F5 		//加入行会成功
#define SC_GUILD_ADDIN_GUILD_FAILED			0x02F6 		//加入行会失败
#define SC_GUILD_DELETE_MEMBER_SUCCESS		0x02F7 		//删除成员成功
#define SC_GUILD_DELETE_MEMBER_FAILED		0x02F8 		//删除成员失败
#define SC_GUILD_MEMBER_TITLE_EDIT_FAILED	0x02F9 		//成员头衔编辑失败
#define SC_GUILD_CREATE_SUCCESS				0x02FA 		//行会创建成功
#define SC_GUILD_CREATE_FAIL				0x02FB 		//行会创建失败
#define SC_GUILD_MEMBER_GET_TITLE			0x02EE 		//成员获得头衔
#define SC_GUILD_ALIGN_FAILED				0x0301 		//联盟失败
#define SC_GUILD_ALLY_SUCCESS				0x0302 		//行会间联盟或取消联盟成功
#define SC_GUILD_ALIGN_CANCEL_FAILED		0x0303 		//取消联盟失败
#define SC_GUILD_WAR_MESSAGE				0x02FF		
#define SC_GUILD_INVITED					0xC7FF
#define SC_GUILD_INI_INFO					0x0340		//行会初始化信息
#define CS_GUILD_OFF_APPDISS				0x0340		//行会官员任罢免
#define SC_GUILD_OFF_APPDISS_RESP			0x0341		//行会官员任罢免服务器端响应
#define CS_GUILD_OFF_ACCEPTORNOT			0x0341		//被邀请人是否接受该职位
#define SC_GUILD_CHANGE_TILE				0x0342		//行会成员改变封号协议
#define CS_GUILD_ASSIGN_TAEL				0x0344		//会长分配行会银两协议
#define SC_GUILD_ASSIGN_TAEL				0x0344		//会长分配行会银两协议
#define SC_GUILD_NEW_GUILD_LIST				0x0345		//新的行会成员列表
#define CS_GUILD_NEW_GUILD_LIST				0x0345		//新的行会成员列表请求
#define CS_GUILD_APPLY_TAEL					0x0346		//申请在线成员银两信息
#define SC_GUILD_APPLY_TAEL					0x0346		//申请在线成员银两信息
#define SC_GUILD_ONOROFF_LINE				0x0347		//行会成员上下线协议
#define SC_GUILD_RESOURCE_CONTRIBUTE		0x0348		//行会资源银两(行会)
#define CS_GUILD_RESOURCE_CONTRIBUTE		0x0348		//行会成员个人捐献资源
#define SC_GUILD_PERSON_TAEL				0x0349		//行会个人银两
#define CS_GUILD_TASK_INFO					0x0351		//行会任务(发布与接取)
#define SC_GUILD_TASK_INFO					0x0351		//行会任务(发布与接取)
#define SC_GUILD_STATUS_SWITCH				0x0354		//行会状态开关
#define SC_GUILD_DEVOTERES_RESULT			0x0355		//捐献资源结果
#define CS_SPIRT_INFO						0x0356		//至尊宝盒
#define SC_SPIRT_INFO						0x0356		//至尊宝盒


//地图相关消息
//      消息命令							消息值      消息注释
#define SC_MAP_OUTDOOR_NUMBER				0x02C6 		//返回地图编号
#define SC_MAP_NO_SMALL_MAP					0x02C7 		//在室内返回地图
#define SC_MAP_NAME							0x0036 		//地图编号
#define SC_GS_JUMP_BEGIN					0x021F		// 开始跳转GS
#define SC_MAP_JUMP_SUCCESS					0x02C4 		// 地图跳转成功。 
#define SC_MAP_CHANGE						0x0279 		// 地图跳转。 
#define SC_MAP_JUMP							0x0322		// 地图跳转。 
#define SC_JUMP_REFRESH						0x0321		// 由于地图跳转产生的更新外观指令.
#define SC_MAP_DESCRIPTION					0x0054		// 地图介绍, 暂时无用

//魔法相关消息
//      消息命令							消息值      消息注释
#define SC_MAGIC_EFFECTIVED					0x027E 		//有动画魔法特效包
#define SC_MAGIC_NOEFFECTIVED				0x027F 		//无动画魔法特效包
#define SC_MAGIC_POSITION_SUCCESS			0x0006 		//使对方位置发生改变的魔法使用成功
#define SC_MAGIC_POSITION_FAILED			0x0007 		//使对方位置发生改变的魔法使用失败
#define SC_MAGIC_CONTINUE_BEGIN				0x0324 		//连续魔法开始
#define SC_MAGIC_CONTINUE_END				0x0325 		//连续魔法结束
#define SC_MAGIC_QUICK_MOVE_SUCCESS			0x0326 		//瞬移魔法使用成功
#define SC_MAGIC_QUICK_MOVE					0x0327 		//瞬移魔法
#define SC_MAGIC_QUICK_MOVE_POSITION		0x027A 		//瞬移后人物所在的位置
#define SC_MAGIC_ABILITY					0x00D3		//魔法能力
#define SC_MAGIC_SKILL_VALUE				0x0280		//魔法值变更
#define SC_MAGIC_MONSTER1_ATTACK			0x0016		//掷斧骷髅的掷斧，祖玛弓箭手的弓箭攻击
#define SC_MAGIC_MONSTER2_ATTACK			0x0017		//僵尸的闪电攻击
#define SC_SPECIAL_EFFECT					0x0152		//放特效

//其他相关消息
//      消息命令							消息值      消息注释
#define SC_WEAPON_BREAK						0x044E		// 修炼的武器破碎
#define SC_MESSAGE_TALK						0x0028 		// 其他玩家谈话信息
#define SC_MESSAGE_SYSTEM					0x0064 		// 系统广播信息
#define SC_MESSAGE_PRIVATE					0x0067		// 私聊
#define SC_MESSAGE_GUILD					0x0068		// 行会聊天
#define SC_MAKE_POISON_REMAIN_MONEY			0x02C9		// 制造毒药后所剩money

#define SC_SESSION_ID						0x0000
#define SC_FRIEND_LIST						0x01C1		// 返回好友列表
#define SC_ADD_FRIEND_SUCCESS				0x01C2		// 增加了一个好友
#define SC_DEL_FRIEND_SUCCESS				0x01C3		// 删除了一个好友
#define SC_FRIEND_ERROR						0x01C4		// 增加删除的出错信息
#define SC_FRIEND_NOTICE					0x01C5		// 在线通知
#define SC_FRIEND_REQUEST					0x01C6		// 收到加入好友请求
#define SC_FRIEND_REQUEST_IM				0x01C7		// 收到加入 IM 好友请求
#define SC_FRIEND_MOQI_CHANGE				0x0469		// 好友默契度更新

#define SC_UNREAD_LEAVE_WORD				0x0026		// 别人给你的留言
#define SC_CHAT_NOT_ONLINE					0x0027		// 私聊但对象不在线时，提示改为短信或留言
#define SC_SYSTEM_SKYROCKET					0x8581		// 焰火
#define SC_SYSTEM_PLAY_MIDI					0x3192		// 播放Midi歌曲
#define SC_SERVER_VERSION					0x9591      // 服务器版本编号
#define SC_ITEM_EFFECT_SWITH                0x01FE      // 使用道具后特效开关
#define SC_SELF_CHARACTER_ACTION            0x9555      // 主角动作
#define CS_USE_RELIVE_ITEM                  0x9555      // 使用还魂符

#define SC_TRAIN_SUCCESS					0x003E      // 训马成功
#define SC_RIDE_END							0x00CD      // 结束骑宠
#define SC_PLAYER_SALUTE					0x0BCD      // 敬礼
#define SC_WEATHER							0x002E		// 天气时间
#define SC_NPC_MSGBOX						0x1213		// 服务器端发送一段NPC信息
#define SC_NORMAL_MSGBOX					0x0AFA		// 弹出一个MessageBox
#define SC_WEB_MESSAGEBOX                   0x0BFA      // 弹出一个支持Web连接的消息框

#define SC_ISVALID_ACTION					0x0180		// 动作是否合法
#define SC_LOGIN_VALIDATE					0x0700


#define SC_PERSON_STATE						0x9593		//1.80 得个性化签名和临时封号
#define CS_PERSON_STATE                     0x6893      //个性化签名和临时封号


// 登陆，角色和RunGate相关消息
#define SC_DELETED_CHAR_INFO			    0x0218		// 列出已删除的角色信息
#define SC_RESTORE_CHAR_ERROR				0x0214		// 角色恢复失败
#define SC_RESTORE_CHAR_OK					0x0215		// 角色恢复成功
#define SC_USERNAME_INFO						0x01FC		// 账户信息
#define SC_RUNGATE_ANSWER					0x03D4		// Rungate的响应消息
#define SC_CHARACTER_INFO						0x0209		// 查询此帐号有几个角色
#define SC_CHARACTER_INFO1					0x020B		// 查询此帐号有几个角色
#define SC_CREATE_CHARACTER_FAIL			0x020A		// 创建角色失败
#define SC_PASSWORD_CHANGE_OK				0x01FA		// 修改密码成功
#define SC_PASSWORD_CHANGE_FAIL				0x01FB		// 修改密码失败
#define SC_LOGIN_USERPWDOK					0x0211      // 动态密码正确
#define SC_LOGIN_USERPWDFAIL				0x01F7      // 动态密码不对
#define SC_OBJECTS_POSITION					0x0046		// 服务器返回包裹中物品位置
#define SC_KICK_OUT									0x0210		// 被踢下线
#define SC_LOGIN_CHARSERVER					0x0212      // 角色服务器地址
#define SC_SELCHAR_CHARINFO				    0x0213		// 角色信息新
#define SC_SELCHAR_RUNSERVER				0x020D      // Rungate地址
#define SC_GAME_GREETING						0x0292      // 欢迎进入游戏
#define SC_CREATE_ACCOUNT_FAIL				0x01F9      // 创建帐号失败
#define SC_CREATE_ACCOUNT_SUCCESS			0x01F8      // 创建帐号成功
#define SC_INPUT_DPWD							0x01FF		// 要输入动态密码
#define CS_INPUT_DPWD							0x07FE      // 输入动态密码
#define SC_RUNGATE_DPWD						0x01C8      // Rungate要输入动态密码
#define CS_RUNGATE_DPWD						0x0049      // 向Rungate输入动态密码
#define SC_LOGIN_TUIGUANG					0x3333 		// 推广员编号
#define SC_INPUT_MIBAO_LATER                0x0051      // 延时2分钟输入密宝，有倒计时
#define SC_FLASH_VOICE_BTN					0x0053      // 语聊按钮闪烁

#define SC_NEED_UPTO_UUID						0x01F1		// 需要升级到UUID
#define SC_SERVER_TEMP_UUID					0x01F2		// 服务器暂定的UUID
#define SC_CHANGE_UUID_RESULT				0x01F3		// 修改UUID的结果
#define SC_DYNAMIC_PASSWORD_RESULT			0x01F4		// 动态密码绑定结果
#define SC_UNDYNAMIC_PASSWORD_RESULT		0x01F5		// 动态密码解邦结果
#define SC_ASSERT_BIND									0x01F6		// 确认动态绑定
#define SC_INPUT_RESERVE_PASSWORD				0x0AF8		// 提示客户端输入预留口令
#define SC_CARD_RESULT									0x0AF9		// 令牌剩余时间
#define SC_DYNAMIC_PASSWORD_CHANGE			0x0AFB		// 令牌漂移

#define SC_USER_FULL							0x020E		// 服务器人数已满
#define SC_ERROR_LOGIN						0x020F		// 服务器认证失败
#define SC_LEVEL_CHECK						0x9595		// 7级小号检查
#define CS_LEVEL_CHECK						0x6892		// 客户端使用7级小号检查

#define SC_VERSION_NOMATCH					0xFFDD		// 版本不匹配
#define SC_RUNGATE_ENCRYPT					0x1050
#define CS_RUNGATE_ENCRYPT					0x1050

//发送。。
#define CS_DELETED_CHAR_INFO				0x0068		// 列出已删除角色
#define CS_RESTORE_CHAR						0x0069		// 恢复角色。
#define CS_QUERY_USERNAME					0x0076		// 查询账户是否存在
#define CS_SENDTO_RUNGATE					0x03D3		// 向Rungate发送应答消息
#define CS_LOGIN_USERPWD						0x07D1
#define CS_LOGIN_USERPWD_CHANGE		0x07D3		// 修改密码
#define CS_LOGIN_SERVERNAME				0x0068
#define CS_GAME_ENTERGAME					0x03FA
#define CS_SELCHAR_CHARSERVER				0x0064
#define CS_SELCHAR_CHAR							0x0067
#define CS_GAME_CONNECT						0x2A2A
#define CS_DELETE_CHARACTER					0x0066
#define CS_CREATE_CHARACTER					0x0065
#define CS_CREATE_ACCOUNT					0x07D2

#define CS_LOGIN_VALIDATE						0x0700
#define CS_SEND_WARRANTOR					0x0777		// 推销员姓名

#define CS_UPTO_UUID									0x07D5		// 升级到UUID
#define CS_CHANGE_UUID								0x07D7		// 修改UUID
#define CS_UUID_LOGIN									0x07D8		// UUID登陆
#define CS_BIND_DYNAMIC_PASSWORD			0x07D9		// 动态绑定密码
#define CS_UNBIND_DYNAMIC_PASSWORD		0x07DA		// 解绑动态密码
#define CS_ASSERT_BIND								0x07DB		// 确认绑定
#define CS_ASSERT_RESERVE							0x07DC		// 确认预留口令
#define CS_ASSERT_CARD								0x07DD		// 令牌已停用或已到期的确认
#define CS_SEND_NEXT_DYNAMIC_PASSWORD		0x07DF		// 下一个令牌
#define CS_MESSAGEBOX_REPLY						0x07DE		// 消息框返回信息
#define CS_NEW_VERSION								0x0788		// 新版本
#define CS_LOGIN_REGISTER_INFO					0x00E1     //修改身份信息

//~~end of 登陆

//服务器传入的是+开始的字符串的协议，客户端自己定义

#define SC_PLAYER_SUCCESS					0xFFFF      // 动作成功
#define SC_PLAYER_FAIL						0xFFFE      // 动作失败
#define SC_OPEN_ATTACK_KILL					0xFFFD		// 攻杀剑术开启
#define SC_CLOSE_ATTACK_KILL				0xFFFC		// 攻杀剑术关闭
#define SC_OPEN_ATTACK_STICK				0xFFFB		// 刺杀剑术开启
#define SC_CLOSE_ATTACK_STICK				0xFFFA		// 刺杀剑术关闭
#define SC_OPEN_ATTACK_MOON					0xFFF9		// 半月开启
#define SC_CLOSE_ATTACK_MOON				0xFFF8		// 半月关闭
#define SC_OPEN_ATTACK_FIRE					0xFFF7		// 烈火开启
#define SC_CLOSE_ATTACK_FIRE				0xFFF6		// 烈火消失
#define SC_VALID_DIG						0xFFF5		// 挖矿有效，出矿渣.
#define SC_INTERNET_GOOD					0xFFF4		// internet 消息
#define SC_INTERNET_FAIL					0xFFF3
#define SC_OPEN_REMAIN_SHADE				0xFFF2		// 残影开启
#define SC_CLOSE_REMAIN_SHADE				0xFFF1		// 残影关闭(保留)
#define SC_OPEN_BLOOD_SHADE					0xFFF0		// 血影开启
#define SC_CLOSE_BLOOD_SHADE				0xFFEF		// 血影关闭(保留)
#define SC_OPEN_WHOLE_MOON					0xFFEE		// 抱月开启
#define SC_CLOSE_WHOLE_MOON					0xFFED		// 抱月关闭
#define SC_OPEN_THUNDER_FIRE				0xFFEC		// 雷烈开启
#define SC_CLOSE_THUNDER_FIRE				0xFFEB		// 雷烈关闭
#define SC_OPEN_DESTROY_WEAPON				0xFFEA		// 击破开启
#define SC_CLOSE_DESTROY_WEAPON				0xFFE9		// 击破关闭
#define SC_OPEN_DESTROY_SHELL               0xFFE8      // 破甲开启
#define SC_CLOSE_DESTROY_SHELL              0xFFE7      // 破甲关闭 
#define SC_OPEN_DESTROY_SHIELD              0xFFE6      // 破盾开启
#define SC_CLOSE_DESTROY_SHIELD             0xFFE5      // 破盾关闭  
#define SC_FU_SHENG_TIME                    0xFFD0      // 附身的时间设置
#define CONNECT_ERROR						0xFFFD      // 连接断开了


////////////////////////////////////////////////////////////////////
//客户端发送到服务端消息
////////////////////////////////////////////////////////////////////
//摆摊消息
#define CS_BOOTH_REQUEST_STALL              0x5EB1
#define CS_USER_REAL_INFO                   0x07FD      //发送玩家真实信息

//豹子摆摊相关协议[20060616]
#define CS_BOOTH_REQUEST                    0x5EB3      //请求守摊/请求取回金币元宝/请求取回物品
#define CS_BOOTH_REFRESH_GOOD               0x5EB4      //请求更新摊位上的物品 取回物品或者放上物品
#define CS_BOOTH_REQUEST_FIRST              0x5EB5      //请求摆摊 (初次验证)

//玩家和NPC进行交互消息
//      消息命令							消息值		消息注释
#define CS_EXCHANGE_GOIN					0x03F2 		//要求和NPC进入交互
#define CS_EXCHANGE_MENU_CLICK				0x03F3 		//点击到交互菜单
#define CS_EXCHANGE_SALE_TAKEIN				0x03F4 		//卖物品时把物品放入托盘
#define CS_EXCHANGE_SALE_OK					0x03F5 		//卖物品时按确定
#define CS_EXCHANGE_BUY_OK					0x03F6 		//买物品时按确认
#define CS_EXCHANGE_BUY_KIND_OK				0x03F7 		//买物品种类时按确认
#define CS_EXCHANGE_REPAIR_OK				0x03FF		//修理时点击修理
#define CS_EXCHANGE_STORAGE_OK				0X0407		//保管物品时点击保管
#define CS_EXCHANGE_GETBACK_OK				0x0408		//取回物品点击OK
#define CS_EXCHANGE_REPAIR					0x0400		//把武器放入托盘中修理
#define CS_EXCHANGE_POISON_OK				0x040A		//作毒药按ok
#define CS_EXCHANGE_FORGE					0x041E		//武器放入托盘中锻造
#define CS_EXCHANGE_FORGE_OK				0x03F9		//锻造按OK
#define CS_EXCHANGE_GET_TEX					0x03D1		//获得交易界面图片
#define CS_EXCHANGE_SUBMIT					0x8810		//提交某项物品
#define CS_EXCHANGE_GEM_OK                  0x03E0      //请求宝石凝练/合成

//玩家和玩家进行的交易消息
//      消息命令							消息值      消息注释
#define CS_TRADE_GOIN						0x0401 		//要求和玩家进入交易
#define CS_TRADE_OBJECT_TAKCEIN				0x0402 		//把物品放入托盘
#define CS_TRADE_OBJECT_TAKCEOUT			0x0403 		//把物品从托盘取出
#define CS_TRADE_MONEY_TAKCEIN				0x0405 		//把金钱放入托盘
#define CS_TRADE_OK							0x0406 		//点击交易确认
#define CS_TRADE_END						0x0404		// 关闭交易窗口	                       
#define CS_DROP_GOLD						0x03F8		//扔下金钱
//包裹中的相关操作消息
//      消息命令							消息值      消息注释
#define CS_PACKAGE_OBJECT_PICKUP			0x03E9 		//从地上捡起物品放入包裹
#define CS_PACKAGE_OBJECT_REJECT			0x03E8 		//把包裹中的物品丢掉
#define CS_PACKAGE_OBJECT_ARM				0x03EB 		//从包裹里的物品装备到身上
#define CS_PACKAGE_OBJECT_UNARM				0x03EC 		//把身上的物品放入包裹
#define CS_PACKAGE_OBJECT_USE				0x03EE 		//使用包裹中的物品(药品等)
#define CS_PETPACKAGE_OBJECT_USE			0x03ED		//宠物包裹使用物品（宠物食品)

//组队相关操作消息
//      消息命令							消息值      消息注释
#define CS_GROUP_OPEN_TEAM					0x03FB 		//开启，关闭组队
#define CS_GROUP_OPEN_VOICECHAT				0x04F2
#define CS_GROUP_CREATE						0x03FC 		//创建队伍
#define CS_GROUP_ADD						0x03FD 		//添加队员
#define CS_GROUP_DELETE						0x03FE 		//删除队员
#define CS_GROUP_QUERY						0x041F		//查询队员位置信息

//行会相关操作消息
//      消息命令							消息值      消息注释
#define CS_GUILD_REQUEST					0x03F3 		//行会相关命令(创建行会,申请攻城,申请行会战等)
#define CS_GUILD_MESSAGE_EDIT				0x0410 		//编辑行会公告
#define CS_GUILD_MEMBER_TITLE_EDIT			0x0411 		//编辑行会成员封号
#define CS_GUILD_LOOK_EXP_VAL				0x0412		//察看行会经验值
#define CS_GUILD_LOOK_MESSAGE				0x040B 		//察看行会信息
#define CS_GUILD_LOOK_WEB					0x040C 		//察看行会主页
#define CS_GUILD_ADDIN_GUILD				0x040E 		//要求加入行会
#define CS_GUILD_DELETE_MEMBER				0x040F 		//删除行会成员
#define CS_GUILD_ALLY						0x0BD6		// 与其它行会联盟
#define CS_GUILD_REPLY_INVITE				0x0AAA		// 
//发送消息给其他玩家相关
//      消息命令							消息值      消息注释
#define CS_MESSAGE_SEND						0x0BD6 		//发送消息(包括结盟请求)
#define CS_MESSAGE_CONFIRM					0x0051 		//消息接收确认
#define CS_MESSAGE_PRIVATE					0x0BD6		//私聊	
#define CS_MESSAGE_SUBMIT					0x6871		//客户端提交某种文字信息
#define CS_MIDI_FOR_OTHER					0x5372		//为其他人点播MIDI

//玩家自身动作相关消息
//      消息命令							消息值      消息注释
#define CS_PLAYER_TURN						0x0BC2 		//转方向
#define CS_PLAYER_WALK						0x0BC3 		//走(鼠标左键)
#define CS_PLAYER_RUN						0x0BC5 		//跑(鼠标右键)
#define CS_PLAYER_SQUAT						0x0BC4 		//蹲(Alt+鼠标左键)
#define CS_PLAYER_NORMAL_ATTACK				0x0BC6 		//一般攻击(左键点击)
#define CS_PLAYER_FORCE_ATTACK				0x0BC7 		//强行攻击(shift+左健)
#define CS_PLAYER_DIG						0x0BC8 		//挖矿(左健，有挖矿工具的时候)
#define CS_PLAYER_ATTACK_KILL				0x0BCA 		//攻杀剑术
#define CS_PLAYER_ATTACK_STICK				0x0BCB 		//刺杀剑术
#define CS_PLAYER_ATTACK_MOON				0x0BD0 		//半月剑术
#define CS_PLAYER_ATTACK_FIRE				0x0BD1 		//烈火剑术
#define CS_PLAYER_ATTACK_NEW_FENCE			0x0BD2		//新增加剑法
#define CS_PLAYER_ATTACK_MAGIC				0x0BC9 		//魔法攻击
#define CS_PLAYER_DIG_CORPSE				0x03EF 		//挖尸体
#define CS_PLAYER_ATTACK_POSTION			0x8878 		//击破攻击的位置
#define CS_PLAYER_PET_STATE					0x8897 		//宠物状态
#define CS_PLAYER_USE_MAGIC_REEL			0xCCAE 		//使用魔法卷轴
#define CS_PLAYER_ABANDON_TASK              0x5EB2      //放弃任务

#define CS_QUERY_STORYBOARD_URL             0x8d00      // 故事板URL请求
#define SC_STORYBOARD_URL                   0x518C      // 故事板响应

#define SC_MONSTERATTACK_EFFECT             0x2BDC      // 怪物攻击得效果包
#define SC_INVESTIGATION                    0x9597      // 调查问卷

#define CS_NPC_MSGBOX						0x6891		//向服务器端发送一段NPC信息

#define CS_SHORTCUT_KEY_CHANGE				0x03F0

#define CS_CHECK_USER						0x9598
#define CS_DYN_CRYPT_OK						0x0712
#define CS_RE_REQUEST_DYN_CRYPT             0x0715     //重新请求动态加解密
#define SC_CHECK_USER						0x9598
#define SC_NEED_DYN_CRYPT					0x0328
//其他命令消息
//      消息命令							消息值      消息注释
#define CS_OPEN_MAPVIEW						0x0409 		// 打开小地图
#define CS_LOOK_PLAYER_INFO					0x0052 		// 察看其他玩家信息
#define CS_QUIT_GAME						0x03F1		// 小退出游戏
#define CS_REAL_QUIT_GAME					0x006A		// 退出游戏
#define CS_ADD_FRIEND						0x0042		// 添加好友
#define CS_DEL_FRIEND						0x0043		// 删除好友
#define CS_ANSWER_FRIEND_REQUEST			0x0044		// 回答好友请求

#define CS_READY_QUIT						0x0050
#define CS_RIDE_HORSE						0x0040		//上下马
#define CS_TRAIN_HORSE						0x0BA0		//驯马
#define CS_CHANGE_HAIR						0x0041		//头发
#define CS_PLAYER_STOPGO					0x0BCC		//停止行走
#define CS_PLAYER_SALUTE					0x0BCD		//抱拳
#define CS_CHANGE_BODYCOLOR					0x0BCE		//换衣服颜色
#define CS_REFLASH_ALL						0x03D2		//刷新数据
#define CS_LEPOARD_ACTION                   0x0C09      //豹子动作


#define CS_IM_ADDFRIEND                     0x0048      // for IM

//游戏商城
#define SC_PT_STORE_INFO					0x1000		//游戏商城
#define CS_PT_STORE_INFO					0x1000		//游戏商城
#define CS_LEAVE_MESSAGE_INFO				0x03EA		//爱情宣言 留言
#define CS_PT_YELLOW_FLAG					0x00C0   //黄榜大旗
#define CS_PT_CHANGSHENGXUANBIN				0x00C1      //常胜玄兵,也可应用确认回复消息
#define SC_PT_CHANGSHENGXUANBIN				0x00C1      //常胜玄兵,也可应用确认回复消息
#define CS_SEE_MON_INFO 					0x00C2		//右键查看怪物属性
#define SC_PT_MESSAGE_INFO					0x028F		//服务器弹出特殊对话框
#define CS_PT_LEAVWORDS						0x028E		//黄榜大旗常胜玄兵 提交留言信息

////////////////////////////////////////////////////////////////////////////////////////////

#define CS_REQUEST_SERVER_TIME              0x0C00      //请求服务器时间
#define SC_ANSWER_SERVER_TIME               0x9600      //服务器实际时间

//宠物包裹
#define CS_PET_OBJECT_STORAGE			    0x0C02		//保存到宠物包裹
#define CS_PET_OBJECT_GETBACK               0x0C03      //从宠物包裹取出东西
#define CS_PET_OBJECT_PICKUP                0x0C01      //通知宠物捡物
#define SC_PET_OBJECT_ADD                   0x9603      //宠物包裹增加一个东西
#define SC_PET_OBJECT_STORAGE_RESULT        0x9604      //保存到宠物包裹成功
#define SC_PET_OBJECT_GETBACK_RESULT        0x9605      //宠物包裹拿走一个东西成功
#define SC_PET_OBJECT_INFO                  0x9602      //返回宠物包裹内所有物品信息
#define SC_PET_PACKAGE_CAPACITY             0x9601      //宠物包裹格子数目
#define SC_PET_OBJECT_PICKUP_FAIL           0x9606      //宠物捡物失败
#define SC_PET_STATE_CHANGE                 0x9610      //宠物状态改变

#define SC_PLAYER_SKILLPOWER                0x9611      //元气值

#define SC_PLAYER_CONNECTED					0x9609		// 确认玩家上线

#define CS_WOOLSTORE_GUID                   0x0710      // 发送GUID
#define SC_WOOLSTORE_TOKEN                  0x0710      // 接受TOKEN

#define SC_SERVER_SWITCH					0x0330      //服务器可以使用的功能开关

//排名系统协议
#define CS_RANGE_SIGN_UP					0x9701		//排名报名或请求更新自己数据
#define SC_RANGE_SIGN_UP_RESULT				0x9702		//排名报名返回

#define CS_RANGE_REQUEST					0x9703		//请求自己或玩家排名数据
#define SC_RANGE_REQUEST_RESULT				0x9704		//请求自己或玩家排名数据返回

#define CS_RANGE_FINDOTHERPLACE				0x9705		//请求其他玩家排名位置
#define SC_RANGE_FINDPLACE_RESULT			0x9706		//返回其他玩家排名位置

#define CS_RANGE_FRIEND						0x9707		//请求好友排名趋势
#define SC_RANGE_FRIEND_RESULT				0x9708		//通知好友排名趋势

#define SC_CELLPHONE_USER_INPUT_REQUEST     0x9800     //弹出窗口输入手机绑定信息
#define CS_CELLPHONE_USER_INPUT             0x9801     //输入手机绑定信息

#define SC_EXCHANGE_PT_BUY_MENU             0x028E     //PT物品的购买信息
#define CS_SPECIALITEMACT				    0x001C	   //特殊物品处理协议,如使用豹神水复活豹
#define SC_PET_ADOPT_INFO				    0x88A0	   //豹子寄养相关信息
#define CS_PET_ADOPT_REQUEST			    0x88A0	   //请求豹子寄养相关信息
#define SC_SENDOUT_PET_INFO				    0x88A1	   //自己放出来的豹子的相关信息
#define SC_MYADOPT_PET_INFO				    0x88A2	   //自己所有领养回来的豹子的相关信息
#define SC_PET_BOOTH_INFO		            0x88A3	   //自己的豹子的摆摊信息

#define CS_LOGIN_REGISTER_INFO              0x00E1     //修改身份信息
#define SC_MASTER_PRENTICE_MSG              0x0310     //拜师与招徒相关信息
#define CS_MASTER_PRENTICE_REQUIRE          0x0310     //拜师与招徒相关请求
#define SC_PLAYER_GUILD                     0x0311     //通知玩家所在行会
#define SC_PLAYER_TRADE                     0x0312     //服务器通过有玩家请求和自己交易
#define SC_RELATION_BASICINFO				0x01C0	   //收到服务器玩家的关系信息
#define CS_GOOD_INFO_REQUIRE   				0x00C7	   //客户端请求一个物品名称对应的数据，为了在npctext时显示图片对应的tips
#define SC_GOOD_INFO_ANSWER    				0x00C7	   //服务器应答物品请求数据

#define CS_VOICE_REQUST_INFO                0x0BD9     //[v1.930][[语音聊天][队长告知/队员查询组队频道信息]
#define CS_VOICE_REQUEST_SERVER_ADDR        0x0BD8     //[v1.930][[语音聊天][客户端请求语音聊天服务器的地址信息]
#define SC_VOICE_INFO                       0x0BD9     //语音聊天服务器组队频道信息[v1.930]
#define SC_VOICE_SERVER_ADDR                0x0BD8     //[v1.930][[语音聊天][服务器告知语音聊天服务器的地址信息]

#define SC_SWYTMBZ_BUBBLEUP_RULE            0x0BDB     //神威狱通道及神威狱天魔宝藏地图里冒经验的规则
#define SC_PET_ADOPT_END					0x88A5	   //每周活动
#define CS_SET_USE_OBJECT_PARA			    0x00C5     ////[v1.932][[易用 性][设置使用物品的参数　回城神石等]
#define SC_SET_USE_OBJECT_PARA              0x00C5     //易用性
#define SC_NPC_QUICK_EXCHANGE   		    0x0BDF     //NPC购物图形化，弹出快捷买卖、修理窗口
#define SC_SPLIT_STONE						0x00C6     //拆分符石
#define CS_SPLIT_STONE						0x00C6	   //拆分符石
#define CS_PET_INFO_REQ						0x88A6	   //请求宠物相关信息（例如血量信息）
#define SC_PET_FEED_FROM_INTERFACE			0x0331		//从界面上直接喂食

#define SC_UPDATE_MEDAL_ATTRIBUTE			0x0BE3		//更新精英勋章
#define SC_UPDATE_MEDAL_EXP					0x0BE4		//升级勋章所需要经验数
#define CS_UPDATE_MEDAL_EXP					0x0BE4		//升级勋章所需要经验数

#define CS_SEAL_TAKE_PUT					0x0BE5	   //存取封元印
#define SC_SEAL_TAKE_PUT					0x0BE5	   //存取封元印
#define SC_FLAG_WUXING						0x0BE6		//五行旗
#define CS_FLAG_WUXING						0x0BE6		//五行旗
#define CS_WUXING_APPEAR_POINT				0x0BE7		//五行玄关召唤点
#define SC_FENSHEN_TAKE_OBJECT				0x0272		//分神时收走道具协议
#define CS_FILL_LINGFUBAG					0x0BEB		//灵符加入灵符袋
#define CS_PET_MERGE                        0X0BEC		//宠物幻化
#define SC_PET_MERGE                        0X0BEC		//宠物幻化
#define CS_SESSIONID_TO_SELGATE             0x01CC		//把sessionid发给selgate
#define CS_LINGXIBOX_INFO					0x0332		//至尊宝盒存取物品
#define SC_LINGXIBOX_INFO					0x0332		//至尊宝盒物品信息
#define SC_MAZE_INFO						0x0333		//迷仙阵宝藏位置
#define CS_EXIT_MAZE						0x0334		//离开迷仙阵
#define CS_KFZ_CONFIRM_MEMBER				0x0335		//跨服战报名参赛人员
#define CS_KFZ_ACTIVATE  			  	    0x0336		//跨服战参赛人员激活
#define SC_KFZ_ACTIVATE_RESULT		  	    0x0336		//跨服战参赛人员激活结果
#define CS_KFZ_JOIN_MEMBER 			  	    0x0337		//请求本服参加跨服战的成员名单
#define SC_KFZ_JOIN_MEMBER 			  	    0x0337		//gs考试客户端本服参加跨服战的行会成员名单
#define SC_KFZ_VISTOR_JOIN		  	        0x0338		//观众报名观看跨服战
#define SC_SECNOND_TIME_OUT 		  	    0x0339		//读秒倒计时
#define SC_KFZ_RESULT     		  	        0x033A		//跨服战结果
#define SC_ENLARGE_PACKAGE		  	        0x033B		//下标为6的那个字节表示类型,1为扩包裹,2为扩钱袋
#define SC_KFZ_MEMBER_STATE		  	        0x033C		//gs告诉client参加跨服战人员是否在本地图的状态
#define CS_KFZ_MEMBER_CONFIRM_STATE		    0x033C		//client告诉gs参加跨服战人员是否在本地图的状态
#define SC_KFZ_ARENA_LIST		  	        0x033D		//gs告诉client跨服战擂台列表
#define CS_KFZ_ARENA_LIST		  	        0x033D		//client请求跨服战擂台列表
#define CS_KFZ_ARENA_JOIN		  	        0x033F		//跨服战请求攻擂
#define SC_ZHENBAO_INFO                     0x0343      //行会珍宝阁系统
#define CS_ZHENBAO_INFO                     0x0343      //行会珍宝阁系统
#define SC_SHENGONG_FANG					0x0352		//神工坊协议
#define CS_SHENGONG_FANG					0x0352		//神工坊协议
#define SC_TUTENG_STATE					    0x0353		//各种行会图腾效果
#define CS_STELE_HERO						0x0357		//挑战精英战将
#define SC_STELE_HERO						0x0357		//挑战精英战将
#define CS_STELE_BANI						0x0358		//动员令
#define SC_STELE_BANI						0x0358		//动员令
#define SC_ASSESS_STELE_LEVEL				0x0359		//评定仙级
#define SC_STELE_PART_LIST					0x0360		//参与者列表
#define CS_STELE_REGISTER					0x0361		//报到
#define SC_STELE_REGISTER					0x0361		//报到回应
#define SC_MSGBOX_COMMAND				    0x0394		//弹出对话框,按确认后根据不同类型做不同的事
#define CS_PET_EXPLORE						0x88A8      //凤凰探宝
#define SC_PYHLE_MODIFY						0x0370		//行会修神修魔状态修改
#define SC_GUILD_PHYLE_LIST					0x0371		//行会宗派列表
#define CS_GUILD_PHYLE_LIST					0x0371		//行会宗派列表
#define CS_GUILD_PHYLE_STATE_MODIFY			0x0372		//行会状态修改
#define SC_GUILD_PHYLE_STATE_MODIFY			0x0372		//行会状态修改
#define CS_PHYLE_RELATION					0x0373		//社会关系宗派
#define SC_PHYLE_RELATION					0x0373		//宗派加入或退出(社会关系与行会共用)
#define SC_GENERAL_REPLY_CONFIRM			0x0374		//通用的响应其它玩家的请求
#define CS_GENERAL_REPLY_CONFIRM			0x0374		//通用的响应其它玩家的请求
#define SC_GUILD_PHYLE_STATE				0x0375		//行会修神还是修魔
#define SC_GENERAL_CHAT_MSG					0x0376		//通用聊天消息（宗派。。。）	
#define SC_GENERAL_OPERATE_RESULT			0x0390		//通用操作结果
#define SC_HEAVEN_CITY_STATUS				0x0391		//锦罗天城状态
#define CS_HEAVEN_CITY_STATUS				0x0391		//锦罗天城状态
#define SC_DYNOBJ_APPEAR				    0x0392		//动态物体出现
#define SC_DYNOBJ_DISAPPEAR				    0x0393		//动态物体消失
#define SC_MSGBOX_COMMAND				    0x0394		//弹出对话框,按确认后根据不同类型做不同的事
#define SC_HEIYANMO_REFRACTION			    0x0395		//黑怨魔折射效果
#define CS_DISASSEMBLE_GOODS				0x0397		//拆卸物品
#define SC_DISASSEMBLE_GOODS				0x0397		//拆卸物品
#define CS_COMBINE_YUANSHI					0x0398		//合成原石
#define SC_COMBINE_YUANSHI					0x0398		//合成原石
#define SC_JINGLI_VALUE						0x0399		//精力值
#define SC_LIANHUA_VALUE					0x0400		//炼化值
#define SC_LIANHUA_LEVEL					0x0401		//炼化等级
#define CS_EXTRACT_GEM						0x0413		//淬炼宝石
#define CS_REMOVE_GEM						0x0414		//摘取原石
#define CS_DEL_SKILL						0x0415      //遗忘技能
#define SC_QUICK_TG_MONSTER_LIST            0x0420      //快速救公主,gs发过来的死亡怪物列表
#define SC_VIP_STORE_INFO                   0x0421      //vip商城信息
#define SC_VIP_STORE_PERSONAL_INFO          0x0422      //vip商城个人信息
#define SC_ELIXIR_INFO						0x0423		//仙灵丹
#define CS_ELIXIR_APPLY_BUY					0x0424		//仙灵丹购买
#define CS_DEGRADE_FUSHI					0x0425		//还原符石
#define SC_SUCCESS_PROBABILTY               0x0426		//成功概率
#define CS_DRILL_EQUIP                      0x0426	    //装备打孔
#define SC_EMBED_FUSHI						0x0427		//符石嵌装备
#define CS_EMBED_FUSHI                      0x0427		//符石嵌装备
#define SC_REMOVE_FUSHI						0x0428		//符石摘除
#define CS_REMOVE_FUSHI						0x0428		//符石摘除
#define SC_UPGRADE_FUSHI					0x0429		//符石雕琢
#define CS_UPGRADE_FUSHI					0x0429		//符石雕琢
#define SC_COMBINE_FUSHI					0x0430		//符石合成
#define CS_COMBINE_FUSHI					0x0430		//符石合成
#define SC_CHANGE_HOLE						0x0431		//转变孔属性
#define CS_CHANGE_HOLE						0x0431		//转变孔属性
#define SC_UPGRADE_GOOD						0x0432		//升级物品
#define CS_UPGRADE_GOOD						0x0432		//升级物品
#define SC_WARM_HEART_PROMPT_INFO			0x0433		//温心提示
#define CS_WARM_HEART_PROMPT_INFO			0x0433		//温心提示
#define SC_NPC_LOOK_CHANGE					0x0434		//npc改变look
#define CS_PRODUCE_INFO						0x0435		//生产
#define SC_LUCKYDIALOG	                    0x043A      //抽奖对话框
#define CS_LUCKQYZ_SELECT                   0x0436      //抽奖
#define SC_LUCKQYZ_PRIZE                    0x0439      //抽中的经验
#define CS_BLESSZHONGZHOU_CHANGECHAR        0x0437      //天佑中州字符变换
#define SC_BLESSZHONGZHOU_CHANGE_DIALOG     0x043B      //天佑中州转换字符对话框
#define SC_INSTANCEZONE_POPUP               0x043C		//副本弹出界面
#define CS_INSTANCEZONE_POPUP               0x043C		//副本弹出界面
#define SC_BAOJIAN_SELECT					0x0440		//群英宝鉴
#define CS_BAOJIAN_SELECT					0x0440		//群英宝鉴

#define CS_YUANBAOSELL						0x0441 //元宝寄售
#define CS_YUANBAOBUY						0x0442 //购买元宝
#define CS_YUANBAOCANCEL					0x0443 //取消寄售
#define CS_YUANBAOQUERY						0x0444 //查询元宝，点击寄售元宝及领取金币页签发
#define SC_YUANBAOQUERY						0x0444 //查询元宝，点击寄售元宝及领取金币页签发
#define CS_TAKEMONEY						0x0445 //领取金币

#define CS_YUANBAOSELL_INFO					0x0446	//寄售元宝信息
#define SC_YUANBAOSELL_INFO					0x0446  //寄售元宝信息

#define CS_CHANGE_MATERIAL                  0x0447  //转换材料
#define SC_GIVEONLINEPRIZE                  0x0448  //在线领道具提醒
#define SC_MSG_TXZG                         0x0455  //铁血战歌副本通知
#define CS_MSG_TXZG                         0x0455  //领取铁血战歌副本奖励

#define SC_LEASEBOOTH_STATE					0x0465	//当前租赁摊位状态
#define CS_LEASEBOOTH_REQUEST				0x0466	//申请租赁摆摊
#define SC_LEASEBOOTH_RESPONSE				0x0466	//申请结果
#define CS_LEASEBOOTH_START					0x0467	//开始摆摊,发送商品列表
#define SC_LEASEBOOTH_STARTRESPONSE			0x0467	//发送结果
#define CS_LEASEBOOTH_BUYOTHERS				0x0468	//购买别人租赁摊位的货物
#define SC_LEASEBOOTH_BUYOTHERSRESPONSE		0x0468	//购买结果
#define CS_LEASEBOOTH_TAKEBACKMONEY			0x0470	//拿回离线时出售物品的货币
#define SC_LEASEBOOTH_TAKEBACKMONEYRESPONSE	0x0470	//拿钱结果
#define SC_LEASEBOOTH_OVERDUETIME			0x0471	//租赁商铺过期
#define CS_LEASEBOOTH_LEAVEMESSAGE			0x0478	//买家向卖家留言

#define SC_FIRM_SHOWPURCHASEITEM			0x0472	//查看收购信息
#define CS_FIRM_SHOWPURCHASEITEM			0x0472
#define SC_FIRM_CANPURCHASEITEM				0x0473  //可收购物品
#define CS_FIRM_SENDPURCHASEITEMINFO		0x0474  //发布自己的收购信息
#define SC_FIRM_SENDPURCHASEITEMINFO		0x0474
#define CS_FIRM_SELLITEM					0x0475  //在商行出售物品
#define CS_FIRM_TAKEBACKGOODS				0x0476	//领取收购的物品
#define CS_FIRM_CANCELPURCHASE				0x0480	//取消收购信息
#define CS_FIRM_GETMONEYOVER				0x0481  // 领取收购预付金额
#define SC_CUSTOM_PIAOHONG					0x048a	//自定义飘红
////////////////////////////////托管//////////////////////////////////////////
#define CS_TRUSTEESHIP_SWITCH				0x0449	//托管开关
#define SC_TRUSTEESHIP_SWITCH				0x0449	//托管开关

#define CS_TRUSTEESHIP_MODE					0x0450	//托管模式设置
#define SC_TRUSTEESHIP_MODE					0x0450	//托管模式设置

#define CS_TRUSTEESHIP_HEAD_TARGET			0x0451	//队长攻击目标
#define SC_TRUSTEESHIP_HEAD_TARGET			0x0451	//队长攻击目标

#define SC_TRUSTEESHIP_HEAD_POS				0x0453	//队长位置
#define CS_TRUSTEESHIP_INVITE_ASK			0x0454	//寻问响应
#define SC_TRUSTEESHIP_INVITE_ASK			0x0454	//响应答复
#define SC_MSG_TXZG                         0x0455  //铁血战歌副本通知
#define CS_MSG_TXZG                         0x0455  //领取铁血战歌副本奖励

#define SC_TRUSTEESHIP_ARRAY				0x0456	//阵谱
#define SC_TRUSTEESHIP_DATA_EXCHANGE	    0x0457	//托管成员间数据转发请求
#define CS_TRUSTEESHIP_DATA_EXCHANGE	    0x0457	//托管成员间数据转发回应
#define SC_TRUSTEESHIP_GOOD_EXCHANGE	    0x0458	//托管成员间物品交换请求
#define CS_TRUSTEESHIP_GOOD_EXCHANGE	    0x0458	//托管成员间物品交换回应
#define SC_TRUSTEESHIP_NEEDJOB              0x0459  //接受阵谱各位置职业数据
#define CS_TRUSTEESHIP_NEEDJOB              0x0459  //请求阵谱各位置职业数据
#define CS_TRUSTEESHIP_HPMP                 0x045A  //请求广播托管队员及队长红蓝数据
#define SC_TRUSTEESHIP_HPMP                 0x045A  //接受托管队员及队长红蓝数据
#define SC_MICROCONTROL_SELECT              0x045B  //选择微操控制对象成功
#define CS_MICROCONTROL_SELECT              0x045B  //选择微操控制对象
#define SC_MICROCONTROL_RECEIVEPROTOCOL     0x045C  //被微操控制角色的相关协议，协议体为封装原始协议
#define CS_MICROCONTROL_SENDPROTOCOL        0x045C  //被微操控制角色的相关协议，协议体为封装原始协议
#define SC_MERITORIOUSNESS_UPDATE			0x045E	//功勋值的协议
#define CS_MERITORIOUSNESS_UPDATE			0x045E	//功勋值的协议
#define CS_RTSINSTANCE_EXCHANGE_BYRESOURCE	0x045F	//在RTS副本中用资源换取
#define SC_MONSTER_CHANGE_BALL              0x0460  //队长死亡后变成灵珠
#define SC_TRUSTEESHIP_HEAD_MODE			0x0461	//队长战斗模式
#define CS_TRUSTEESHIP						0x0462	//托管
#define SC_TRUSTEESHIP						0x0462	//托管响应

#define CS_QUNYING_MEMBER                   0x0463	//群英招募
#define SC_QUNYING_MEMBER                   0x0463	//群英招募

#define CS_QUNYING_REPORT                   0x0479	//群英战报
#define SC_QUNYING_REPORT                   0x0479	//群英战报

#define SC_TRUSTEESHIP_MEMBER_IS_OFFLINE    0x0477	//告诉队长某个队员是否离线托管
#define SC_TRUSTEPSHIP_ONLY_FRIEND          0x047A  //只允许好友把我加入群英模式
#define CS_TRUSTEPSHIP_ONLY_FRIEND          0x047A  //只允许好友把我加入群英模式

////////////////////////////////END 托管//////////////////////////////////////////

#define CS_EQUIPSOUL						0x047D	//器魂
#define CS_LEVELUPWING						0x047E	//升级翅膀
#define SC_FULL_ZHENBAO_LEFT_TIME           0x0483  //满珍宝值剩余时间(秒)
#define SC_ZHENBAO_VALUE                    0x0484  //gs给客户端通知珍宝值
#define SC_HONOR_VALUE	                    0x0485  //服务器同步荣誉值

#define CS_COMPOUND_BLESS					0x0486   //合成神佑
#define CS_EXTEND_BLESS						0x0487   //延长神佑期限
#define SC_MONSTER_HDNW_LOCK       			0x048F	//混沌女王锁链

#define SC_SWEAR_LIST						0x0378     //结义列表
#define SC_PLAYERJOINORQUIT_SWEAR			0x0379     //加入或退出结义
#define CS_ADDORDEL_SWEARMEM				0x0489     //增加或删除结义成员
#define CS_ADD_SWEAR_CONFIRM				0x0490     //同意添加
#define SC_SWEAR_ADD_CONFIRM				0x0490	   //添加确认
#define SC_SWEAR_ERROR						0x0491     //结义错误提示

#define SC_WENPEI_LIST						0x04A0     //纹佩配件列表
#define CS_WENPEI_COMPOUND					0x04A0     //纹佩合成
#define CS_WENPEISUB_DECOMPOUND				0x04A2     //纹佩配件分解0，纹佩提取1
#define CS_WENPEI_DECOMPOUND				0x04A1     //纹佩拆分
#define CS_WENPEI_STRENG					0x04A3     //纹佩强化
#define CS_WENPEI_RONGHE					0x04B5	   //纹佩融合


#define CS_ENTERSJS							0x04A4     //进入圣殿山
#define CS_HEART_BEAT						0x04A5     //心跳包

#define SC_HASWAIGUA						0x04A6     //是否有运行外挂
#define CS_HASWAIGUA						0x04A6     //是否有运行外挂

#define CS_MAC       						0x0380     //告诉本机MAC给gs




#define SC_CONSKILLBUFF       				0x0492		//连击技能buff

#define CS_EQUIP_COMPOSE					0x04A7      //装备合成
#define CS_SZZP_COMPOSE						0x04A8      //圣战之魂升级

#define SC_NOTIFY_ADVENTURE_STATUS			0x0A00		//奇遇状态通知
#define CS_REQUEST_ADVENTURE				0x0A04		//玩家点击太极玄镜
#define SC_NOTIFY_ADVENTURE_REWARD			0x0A01		//奇遇奖励
#define CS_REQUEST_GIVE_ADVENTURE_REWARD	0x0A03		//奇遇点击停止按钮

#define SC_NOTIFY_SANWEIZHENHUOINFO			0x0A21		//三味真火详细信息
#define SC_NPC_LINGWUSANWEIZHENHUO			0x0A10		//NPC处判断领悟三味真火条件满足
#define CS_GOLINGWU							0x0A14		//传送到领悟的地图

#define CS_FABAO_RENZHU						0x0A42		//法宝认主
#define CS_FABAO_ZHUANGBEI					0x0A44		//装备法宝/卸下法宝
#define SC_FABAO_ATTACK						0x0A41		//法宝攻击
#define SC_FABAO_ZHUANGBEI					0x0A45		//装备/卸下法宝结果
#define SC_FABAO_XILING						0x0A43		//法宝吸灵

#define CS_FIREARTIFICE						0x04A9		// 真火炼化
#define SC_FIREARTIFICE						0x04A9		// 真火炼化

#define SC_NOTICEIE							0x04AA		// IE广告
#define SC_FIREARTIFICETIME					0x04AB		// 真火炼化冷却时间
#define CS_FABAOCHAIFEN						0x04AC		// 法宝拆分

#define SC_NEW_STORAGE_INFO					0x047f  //新仓库协议
#define CS_OBJECT_STORAGE_POSITION			0x047f	//向服务器端发送仓库中物品的位置


#define CS_PLAYER_VIP_OPTION                0x04B0    //vip相关操作  包括开启/续费VIP，勾选自动续费，卖物品，取回所得收入，取回Vip商城物品 买vip商城物品
#define SC_PLAYER_VIPINFO                   0x04B0    //vip信息
#define SC_VIPSTORE_INFO                    0x04B1    //vip商城物品信息

#define SC_YIHUO_REQUEST					0x04B2	  //异火
#define CS_YIHUO_SET_NAME_SHORTCUT			0x04B2	  //异火名字,快捷键设置

#define CS_EDIT_BING						0x04B3	//12宫布兵
#define SC_BINGINFO							0x04B3
#define CS_CONTROL_BING						0x04B4	//12宫布兵 6-7类型0跟随1攻2自由 0-3攻击目标
#define SC_OPEN_BINGCONTROLWND				0x04B4  //打开兵种控制界面

#define SC_OPEN_YIHUOZHENGDUOWND			0x04B6  //打开异火争夺系统提示界面
#define CS_OPEN_YIHUOZHENGDUOWND			0x04B6

#define SC_HUFU_FLUSH						0x04B7  //赤金护符
#define CS_HUFU_FLUSH						0x04B7

#define SC_ACTIVERATE_PRIZE                 0x04BE  //活跃度奖励相关协议
#define CS_ACTIVERATE_PRIZE                 0x04BE  //活跃度奖励相关协议

#define CS_EQUIP_CHAIFEN					0x04BF	// 装备拆分
#define SC_EQUIP_CHAIFEN					0x04BF	// 装备拆分

#define SC_FIRE_SHIELD						0x0A46	//八卦盾状态通知
#define SC_DAOZUN_JIANGLIN					0x0A47	//无量五行诀
#define SC_SHADOWKILL8						0x0A48	//八方分影斩

#define SC_LIANJI_SHOW						0x0A55  //连击显示
#define CS_ENLARGE_PACKAGE_INFO_REQ			0x0A60  //请求扩金币上限信息
#define SC_ENLARGE_PACKAGE_INFO_ACK			0x0A61  //请求扩金币上限信息的回复

#define SC_PLAYER_HORSEMANSHIP_WATCH		0x0A56  //骑术非值守信息
#define CS_PLAYER_HORSEMANSHIP_WATCH_REQ	0x0A57	//非值守操作请求
#define SC_PLAYER_HORSEMANSHIP_ACTIONPOINT	0x0A57  //行动点改变

#define SC_PLAYER_HORSEMANSHIP_INFO			0x0482  //骑术信息(界面)

//安全登陆
#define CS_LOGIN_REQ_PUBLIC_KEY             0x01C9     //请求公钥
#define SC_LOGIN_PUBLIC_KEY                 0x01CA     //收到公钥
#define CS_LOGIN_KEY_USERPWD                0x01CB     //安全登陆
#define SC_LOGIN_SDOA						0x01CD	   //SDOA登录

#define CS_LOGIN_CHECK_MASK					0x0AFD		//PT登录校验码
