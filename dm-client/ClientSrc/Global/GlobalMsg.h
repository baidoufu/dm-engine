///////////////////////////////////////////////////////////////////////
//文件名：   GlobalMsg.h
//文件说明：保存游戏内所有控件和窗口之间传递的消息
///////////////////////////////////////////////////////////////////////

#pragma once

enum eMsgBox_Type
{
	MS_OK				= 0x01,
	MS_OK_CANCEL		= 0x02,
	MS_OK_EDIT			= 0x04,
	MS_OK_CANCEL_EDIT	= 0x08,
	MS_CANCEL			= 0x10,
	MS_URL              = 0x20,
	MS_URL_INFORM       = 0x30
};

// 游戏所有以及控件之间传递的消息
enum Game_Message_e
{
// 系统消息
	
	////////////////////////////////////////////////////////////////////////////
	// 系统输入响应消息								范围:0x00000100 - 0x00000200
	MSG_INPUT_BEGIN				= 0x00000100,
	MSG_INPUT_KEYDOWN,								// KeyDown，data高字为键盘状态，低字为键盘扫描码
	MSG_INPUT_KEYUP,								// KeyUp，data高字为键盘状态，低字为键盘扫描码
	MSG_INPUT_CHAR,									// 输入字符，data为字符的Ascii码
	MSG_INPUT_MOVE,									// 鼠标位置变动，data高字为当前鼠标y座标，低字为x座标
	MSG_INPUT_WHEEL,								// 滚轮消息，data为滚轮座标
	MSG_INPUT_LEFTBT_DOWN,							// 左键Down消息，data高字为当前鼠标y座标，低字为x座标
	MSG_INPUT_LEFTBT_UP,							// 左键Up消息，data高字为当前鼠标y座标，低字为x座标
	MSG_INPUT_LEFTBT_DCLICK,						// 左键DClick消息，data高字为当前鼠标y座标，低字为x座标。双击时第一个BUTTONDOWN发送BUTTON_DOWN消息，第二个BUTTONDOWN发送BUTTON_DCLICK消息
	MSG_INPUT_RIGHTBT_DOWN,							// 同左键
	MSG_INPUT_RIGHTBT_UP,							// 同左键
	MSG_INPUT_RIGHTBT_DCLICK,						// 同左键
	MSG_INPUT_MIDDLEBT_DOWN,						// 同左键
	MSG_INPUT_MIDDLEBT_UP,							// 同左键
	MSG_INPUT_CAPTURECHANGED,						// 鼠标在窗口之外，data为空
	// 在这前面添加新的系统响应消息
	MSG_INPUT_END,
	////////////////////////////////////////////////////////////////////////////

// 传递给控件消息

	////////////////////////////////////////////////////////////////////////////
	// 通用控件消息									范围:0x00000200 - 0x00000300
	MSG_CTRL_NORMAL_BEGIN		= 0x00000200,
	MSG_CTRL_BUTTON_CLICK,
	MSG_CTRL_EDIT_ENTRY,
	MSG_CTRL_EDIT_ESCAPE,
	MSG_CTRL_SCROLL_MOVE,
	MSG_CTRL_GOODICON_PICK,
	MSG_CTRL_EDIT_ONCHAR,

	MSG_CTRL_BUTTON_DOWN,
	MSG_CTRL_BUTTON_UP,
	MSG_POPEDIT_CHOICE_SET,
	MSG_CTRL_POPUP_WINDOW,
	MSG_POPUP_MSG_BOX, //新对话框
	MSG_CTRL_CLOSE_WINDOW, //关闭窗口

	MSG_CTRL_MENU_SELCHANGED,             //菜单被选中
	MSG_CTRL_MENU_SELCHANGING,            //菜单正改变
 	MSG_CTRL_MENU_RSELCHANGED,            //菜单被右键选中
	MSG_CTRL_GRID_SELECT_LINE_COL,        //CtrlGrid选中某行某列,dwData低word为行号,高word为列号
	MSG_CTRL_GRID_RCLICK_LINE_COL,        //CtrlGrid选中某行某列,dwData低word为行号,高word为列号
	MSG_CTRL_GRID_DBCLICK,                //CtrlGrid双击某行某列,dwData低word为行号,高word为列号
	MSG_CTRL_TALKVIEW_LCLICK,             //CtrlTalkView单击某行dwData为行号
	MSG_CTRL_TALKVIEW_RCLICK,             //CtrlTalkView右键单击某行dwData为行号
	MSG_GOOD_GRID_LCLICK,                 //GoodGrid左键单击某格,dwData为idx
	MSG_GOOD_GRID_RCLICK,                 //GoodGrid右键单击某格,dwData为idx
	MSG_GOOD_GRID_DBCLICK,                //GoodGrid双击某格,dwData为idx
	MSG_CTRL_ON_SET_FOCUS,                //告诉父窗口pControl自己获得了焦点
	MSG_CTRL_ON_LOSE_FOCUS,               //告诉父窗口pControl自己失去了焦点
	MSG_CTRL_TABPAGE_TABCHANGE,           //页签界面页签改变,dwData表示点击页签编号
	MSG_CTRL_LABEL_CLICK,                 //Label被点击
	MSG_ARROWTIP_CLICK_CLOSEBTN,          //告诉父窗口,点了arrowtip的关闭按钮,如果返回true表示父窗口已经处理了,arrowtip类则不处理,否则关闭
	MSG_REMOVE_ARROWTIP_CONTROL,          //删除某个arrowtip控件,根据显示的id来识别是哪一个arrowtip

	// 在这前面添加新的通用控件消息
	MSG_CTRL_NORMAL_END,


	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	// MainWnd管理子控件消息						范围:0x00000300 - 0x00000600
	MSG_CTRL_MAINWND_MANAGE_BEGIN	= 	0x00000300,
	MSG_CTRL_LOGINWND,
	MSG_CTRL_SELECTCHARWND,	
	MSG_CTRL_GAMEWND,				
	MSG_CTRL_QUIT_WND,
	MSG_CTRL_TALKVIEW_WND,                //聊天窗口
	MSG_CTRL_DISPLAY_MODE,                //改变了显示模式

	// CMainWnd结束
	MSG_CTRL_MAINWND_MANAGE_END,


	// CMainWnd的其他消息							范围:0x00000600 - 0x00000700
	MSG_CTRL_MAINWND_OTHER_BEGIN		= 0x00000600,

	// 在这前面添加CMainWnd的其他消息
	MSG_CTRL_MAINWND_OTHER_END,


	// 传递给CMessageBox消息						范围:0x00000700 - 0x00000900
	MSG_CTRL_MESSAGEBOX_BEGIN			= 0x00000700,

	MSG_CTRL_MESSAGEBOX,
	MSG_CTRL_EDITMESSAGEBOX,
	MSG_CTRL_CLOSEMSGBOX,
	MSG_CTRL_ENTER_GAME,
	MSG_CTRL_MSGBOX_SERVER,					          //返回给服务器一段信息的MessageBox
	MSG_CTRL_MSGBOX_SERVER_NOTNPC,                    //非NPC时弹出的MessageBox
	MSG_CTRL_ABANDON_TASK,                            //放弃任务的确认
	MSG_CTRL_BOOTH_ADD_CONFIRM,      	              //确认添加物品
	MSG_CTRL_BOOTH_ADD_CANCEL,       	              //添加物品取消
	MSG_CTRL_BOOTH_FIRSTTIME_CONFIRM,				  //租赁摊位首次添加物品
	MSG_CTRL_LEASEBOOTH_ADD_CONFIRM,				  //租赁摊位确认添加物品
	MSG_CTRL_LEASEBOOTH_LEASE_CONFIRM,				  //确认租赁摊位
	MSG_CTRL_TRADEYUANBAOBOX,		                  //交易时确认放下多少金币或元宝
	MSG_CTRL_DROPMONEY,		                          //主角往外丟钱		
	MSG_CTRL_TRADEGOLDBOX,			                  //交易黄金数量窗口
	MSG_CTRL_USE_QIANKUNSUO,		                  //确认使用乾坤锁
	MSG_RECONNECT_SERVER,				              //重新连接服务器
	MSG_CTRL_CHANGSHENGXUANBIN_WND,	                  //1.908 商城道具	常胜玄兵
	MSG_CTRL_RELEASE_EXP,                 	          //从宠物身上释放经验
	MSG_CTRL_UPLEVEL_ARM,                             //更换武器附加技能类型
    MSG_CTRL_RELIVE_WIDTH_YB_CONFIRM,                 //需要立即购买还魂神符并直接复活吗?
	MSG_CTRL_USE_MAGIC_BOOK,						  //学习技能书确认
	MSG_CTRL_USE_RED_PACKAGE,						  //打开红包确认
	MSG_CTRL_USE_AUTHENTICATE,                        //确认使用物品鉴定
	MSG_CTRL_CREATE_TEAM_VOICE_ROOM,                  //确认创建组队语聊房间
	MSG_CTRL_CLEARALL_LEAVEWORD,		              //清除所有留言
    MSG_CTRL_INCORPORATE_MEDAL_OK,                    //确认融合精英勋章
    MSG_CTRL_INCORPORATE_MEDAL_CANCEL,                //取消融合精英勋章
	MSG_CTRL_SPIRT_UPGRATE_WEAPON,                    //确认使用圣灵精华升级武器
	MSG_CTRL_QUICKNPC_SELL_OK,                        //图形化交易时确认卖物品
	MSG_CTRL_QUICKNPC_SELL_CANCEL,                    //图形化交易时取消卖物品
	MSG_CTRL_QUICKNPC_REPAIR_OK,                      //图形化交易时确认修理物品
	MSG_CTRL_QUICKNPC_REPAIR_CANCEL,                  //图形化交易时取消修理物品
    MSG_CTRL_REPAIR_DAMAGED_EQUIPMENT,                //修复破坏的装备
	MSG_CTRL_ADDLINGLI,								  //使用灵皓石加灵力,或者使用兽尊灵丹加宠物经验
	MSG_CTRL_EXIT_MAZE_OK,							  //离开迷仙阵OK
	MSG_CTRL_EXIT_MAZE_CANCEL,						  //离开迷仙阵CANCEL
	MSG_CTRL_START_USE_YSTPY, 						  //开启御兽开袍／衣 穿上并骑上豹子
	MSG_CTRL_PASSPORT,								  //GM输入passcode
	MSG_CTRL_RELOGIN_GAME,                            //是否用之前的帐号角色重新登录服务器
	MSG_CTRL_LOGIN_KEZ_SERVER,                        //确认是否登录跨服战服务器
	MSG_CTRL_BACK_FROM_KFZ,                           //确认是否从跨服战服务器返回
// 	MSG_CTRL_ENLARGE_PACKAGE,                         //确认是否扩包裹
	MSG_CTRL_ENLARGE_MONEYBAG,                        //确认是否扩钱袋
	MSG_CTRL_CONFIRM_KFZ_MEMBER_STATE,                //确认记录跨服战成员状态
	MSG_CTRL_ENTER_JIPINGE,							  //玩家进入极品阁
	MSG_CTRL_LEAVE_JIPINGE,						      //玩家离开极品阁
	MSG_CTRL_GET_USBMB_CODE,                          //获得usb密宝内容,获得后用这个密宝发送登录协议
	MSG_CTRL_OPEN_BOX,								  //开启至尊宝盒
	MSG_CTRL_GUILD_MOBILIZE,						  //行会动员令
	MSG_CTRL_JOIN_CONFIRM,							  //确认我来报到
	MSG_CTRL_ACCESSFGHVALUE_ANNWAY,					  //评定战斗值
	//MSG_CTRL_LOGIN_XL_SERVER,                         //确认是否登录仙灵大陆服务器
	//MSG_CTRL_BACK_FROM_XL,                            //确认是否从仙灵大陆返回
	MSG_CTRL_LEAVE_GUILD,							  //退出行会
	MSG_CTRL_DISMISS_PHYLE,							  //解散宗族
	MSG_CTRL_LEAVE_PHYLE,							  //离开宗族
	MSG_CTRL_TAN_BAO,								  //凤凰探宝	
	MSG_CTRL_ZHAO_HUAN,								  //凤凰召唤
	MSG_CTRL_USE_TREASURE_MAP,						  //使用藏宝图
	MSG_CTRL_QUIT_TO_LOGINGWND,						  //退出到登录界面
	MSG_CTRL_CLOSE_CLIENT,  						  //退出客户端
	MSG_CTRL_START_HUANHUA,							  //请求幻化
	MSG_CTRL_DEL_SKILL,								  //请求遗忘技能
	MSG_CTRL_DISASSEMBLE_WEAPON,					  //拆卸装备
	MSG_UPGRADE_YUANSHI,							  //升级原石
	MSG_RELIVE_DEMON,								  //复活心魔
    MSG_CTRL_USE_QIANKUNSUO_CHAREQ,                   //确认使用乾坤锁
	MSG_SUBMIT_GOOD_WND,                              //提交物品窗口
	MSG_CTRL_MINMAP_WND,                              //小地图窗口
	MSG_CTRL_PHRASE_WND,					          //短语窗口
	MSG_CTRL_LUCKQYZ_CLOSEOK,                         //确定关闭幸运珍宝阁
	MSG_CTRL_LUCKQYZ_CLOSECANCEL,                     //取消关闭幸运珍宝阁
	MSG_CTRL_LUCJQYZ_CONTINUE,                        //星期一继续幸运珍宝阁
	MSG_CTRL_BUY_YUANBAO,							  //购买元宝
	MSG_CTRL_SELL_YUANBAO,							  //寄售元宝
	MSG_CTRL_OPENDOOR,								  //开启石门
	MSG_CTRL_BOOTH,									  //开启摆摊空格
	MSG_CTRL_JIEJINGTI,								  //结晶体
	//MSG_CTRL_TRUSTEESHIP_RESPONSE,					  //托管响应
	//MSG_CTRL_TRUSTEESHIP_CONFIRM,					  //托管提示信息
	MSG_CTRL_REJECTOBJECT,							  //丢弃物品提示 [2/22/2010 dujun]
	MSG_CTRL_BREAKDOWN_EQUIP,                         //确定要分解装备吗?
	MSG_CTRL_CHAIFEN_NUM,                             //输入要拆分的数量
	MSG_CTRL_WENPEI_CHAIFEN,                          //纹佩拆分
	MSG_CTRL_USESUBWENPEI,							  //使用纹佩配件
	MSG_CTRL_AUTOKILL,							      //自动打怪
	MSG_CTRL_CHAIFEN_FABAO,							  //法宝拆分
	MSG_CTRL_SELL_GOOD_VIPWND,                        //从vip商行界面卖物品
	MSG_CTRL_RENZHU_FABAO,							  //法宝认主
	MSG_CTRL_BUY_VIPTRADE,                            //购买或升级vip商行会员
	MSG_CTRL_MORE_STORAGE_GRID,						  //开启仓库空格
	MSG_CTRL_FIREARTIFICE,							  //真火炼化
	MSG_CTRL_LOCK_EQUIK,                              //魂锁装备
	MSG_CTRL_YUSHOULEVELUP,                           //骑术非值守


	// 在这前面添加CMessageBox的消息
	MSG_CTRL_MESSAGEBOX_END,



	// 传递给CTalkViewWnd消息						范围:0x00000900 - 0x00000A00
	MSG_CTRL_TALKVIEWWND_BEGIN			= 0x00000B00,
	MSG_CTRL_SWITCH_TALKVIEWER,               //切换频道
	MSG_CTRL_INSERT_TEXT,                     //在聊天框中插入文本,Msg()参数dwData != 0时把焦点给聊天框,否则只插入文本
	MSG_CTRL_INSERT_OBJECTLINK,               //插入物品链接
	MSG_CTRL_RELEASE_FAKEFOCUS,               //清除聊天框中的假焦点
	MSG_CTRL_CHANGE_TALKVIEWSIZE,             //缩放聊天框的大小
	// 在这前面添加CTalkViewWnd的消息
	MSG_CTRL_TALKVIEWWND_END,

	// 在LoginWnd内的消息							范围:0x00000A00 - 0x00000B00
	MSG_CTRL_LOGINWND_BEGIN			= 0x00000C00,
	MSG_CTRL_USERLOGINWND,		
	MSG_CTRL_SELECTSERVERWND,
	MSG_CTRL_FOCUSLOGIN,			
	MSG_CTRL_ENABLELOGINWND,
	MSG_CTRL_CLEARID,
	MSG_CTRL_LOGIN_IN_NOUSE_OA,                //oa登录三次失败后改用原来的登录方式
	MSG_CTRL_ENABLEOPENLOGIN,			//ap登陆
	// LoginWnd结束
	MSG_CTRL_LOGINWND_END,


	// 传递给SelectCharWnd消息						范围:0x00000B00 - 0x00000C00
	MSG_CTRL_SELECTCHAR_BEGIN		= 0x00000D00,
	MSG_CTRL_CREATECHARWND,        //创建角色窗口
	MSG_CTRL_RECOVERCHARWND,  
	MSG_RECOVER_CHAR_OK,
	MSG_CTRL_CREATECHAR_UPDATE,
	MSG_CTRL_RECOVER_CHAR,			
	MSG_CTRL_REGISTERINFO_BTN_HIDE,      //防沉迷系统，补登身份按钮隐藏
	MSG_CTRL_SELECTCHAR_JUMP_GS_SUCCESS, //正在跨gs的时候快速小退,此时请求角色列表会失败,如果后面收到跳gs成功的消息应该再次退重新请求角色列表

	// SelectCharWnd结束
	MSG_CTRL_SELECTCHAR_END,

	// 传递给GameWnd消息							范围:0x00000900 - 0x00000A00
	MSG_CTRL_GAMEWND_BEGIN			= 0x00000E00,
	MSG_CTRL_UPDATE_ATTACKMODE,               //刷新攻击模式
	//MSG_CTRL_UPDATE_HIDESELFMSG_BTN,          //刷新隐藏系统消息的按钮状态
	MSG_CTRL_LOCATE_AREA,	                  //通知GameWnd地图属性(安全,战斗,攻城)
	MSG_CTRL_UPDATE_INSTANCE_INFO,			  //通知GameWnd更新提示信息
	MSG_CTRL_UPDATE_TRACK_INFO,				  //通知GameWnd更新追踪信息
	MSG_WND_TIMER,							  //定时器消息
	MSG_CTRL_AI_START,						  //开启AI功能
	MSG_CTRL_RUNHORSELAMP_WND,				  //跑马灯窗口
	MSG_CTRL_DEFEND12GONGBTN,				  //12宫按钮
	MSG_CTRL_AUTOKILLBTN,					//自动打怪按钮更新


	// GameWnd结束
	MSG_CTRL_GAMEWND_END,


	// 其他GameWnds消息								范围:0x00000C00 - 0x00000D00
	MSG_GAMEWNDS_MESSAGE_BEGIN			= 0x00001000,
	MSG_CTRL_PANEL_WND,                               //PANEL窗口
	MSG_CTRL_CHECK_QUICK_WND,                         //快捷消息窗口
	MSG_CTRL_CHARWND,                                 //子娃娃界面
	MSG_CTRL_PACKAGEWND,			                  //包裹界面
	MSG_CTRL_POP_MENU,	                              //弹出菜单窗口
	MSG_CTRL_SKILLWND,                                //技能界面
	MSG_CTRL_WEEK_ACTIVITY_WND,                       //每周任务
	MSG_CTRL_HANDYMAP_WND,                            //右上角小地图
	MSG_CTRL_PERSONS_GUIDE_WND,                       //人物指南窗口
	MSG_CTRL_WOOOL_STORY_WND,                         //传世残卷
	MSG_CTRL_WOOOL_STORE_WND,                         //商城			
	MSG_CTRL_NPCWND,			                      //NPC窗口	
	MSG_CTRL_NPCBUYWND,				                  //NPC　1买，2，仓库取回物品窗口 3毒药窗口	4/二级购买窗口
	MSG_CTRL_NPCSALEWND,		                      //NPC  //1:卖，2:修理	3表示保管物品窗口4表示锻造物品窗口7表示鉴定物品窗口5祝福6表示弹出托盘8不关闭的弹出托盘
	MSG_CTRL_NPCLISTWND,                              //NPC列表窗口
    MSG_CTRL_NPC_QUICK_WND,                           //NPC图形化窗口
	MSG_CTRL_NPCBOOK_WND,                             //传世宝典
	MSG_CTRL_NPCBOARDWND,                             //NPC公告板窗口
	MSG_CTRL_LEAVEWORD_WND,				              //留言窗口
	MSG_CTRL_QUICKBUY_WND,	             			  //快捷消费2.0
	MSG_CTRL_BOOTH_WND,              	              //摊位窗口
	MSG_CTRL_OTHER_BOOTH,            	              //他人摊位
	MSG_CTRL_BOOTH_ADD_GOOD,         	              //添加商品，弹出输入价格对话框
	MSG_CTRL_LEASEBOOTH_FIRSTTIME,					  //租赁摊位被购买后首次打开
	MSG_CTRL_LEASEBOOTH_ADD_GOOD,					  //租赁摊位添加商品,弹出输入价格对话框
	MSG_CTRL_BOOTH_NAME,             	              //设置摊位名对话框
	MSG_CTRL_SELECT_BOOTH_CONFIRM,                    //豹子摆摊-选择豹子摆摊还是玩家摆摊 
	MSG_CTRL_TRADEWND,	                              //交易窗口		
	MSG_CTRL_ASSCONFIG_WND,                           //辅助工具设置
	MSG_CTRL_AUTOLEAVEWORD_WND,				          //不在线，自动留言窗口
	MSG_CTRL_AUTO_SAY_WND,                            //自动发言窗口
	MSG_CTRL_CELLPHONE_USER_WND,           	          //手机帐号绑定用户输入窗口
	MSG_CTRL_COMMAND_WND,                             //指令窗口				 
	MSG_CTRL_COMMONCONDENSE_WND,           	          //消耗装备 熔炼装备的界面
    MSG_CTRL_COMMON_CONFIRM_WND,                      //通用确认窗口，用于拜师，收徒，交易等请求的确认
	MSG_CTRL_EQUIP_LEVEL_UP_WND,                      //装备升级界面
	MSG_CTRL_GEMCONDENSE_WND,                         //凝练物品窗口
    MSG_CTRL_GREETING_MSG_WND,                        //GREETING_MSG_WND，进游戏时第一屏欢迎界面
	MSG_GUILDADDMEMBER_WND,			                  //新加行会成员
	MSG_CTRL_INPUT_MIBAO_LATER_WND,                	  //延迟输入密宝信息提示窗口
	MSG_CTRL_INPUT_MIBAO_WND,                         //输入密宝窗口				
    MSG_CTRL_GUILD_ALLYWND,                           //行会联盟窗口
	MSG_GUILDDELMEMBER_WND,                           //删除行会成员窗口
	MSG_GUILD_EDIT,		                              //行会编辑窗口，公告、封号等			
	MSG_CTRL_TASK_WND,                                //任务显示面板
	MSG_CTRL_TEAMWND,	                              //组队界面			
	MSG_CTRL_YELLOW_FLAG_WND,                         //1.908 商城道具	黄榜大旗
	MSG_CTRL_LEAV_MESSAGE_WND,            	          //爱情宣言 留言窗口
	MSG_CTRL_LEAVE_NOTE_WND,                          //给离线玩家留言
	MSG_CTRL_OTHER_PLAYER_INFO,                       //查看其它玩家子娃娃界面	
	MSG_CTRL_NAMEWND,				                  //快速选择历史聊天对象名字
	MSG_CTRL_NEWBIEWND,			                      //给新角色的提示信息
	MSG_CTRL_NOTICE_HELP_WND,                         //通知窗口
    MSG_CTRL_PERSON_PANEL_WND,                        //选中人物的头像窗口
	MSG_CTRL_PET_PACKAGE_WND,                         //宠物包裹
	MSG_CTRL_PET_WND,                                 //宠物界面
	MSG_CTRL_QUICK_STORE_WND,		                  //快捷商城窗口,包裹左边
	MSG_CTRL_READ_QUIT_WND,                           //读秒退出窗口
	MSG_CTRL_SMALL_GEM_CON_WND,		                  //宝石合成窗口
    MSG_CTRL_SPLITSTONE_WND,                  	      //拆分符石窗口
	MSG_CTRL_USERINFO_WND,                            //用户信息收集
    MSG_CTRL_USE_OBJECT_WND,                          //使用或设置物品如:飞来神石，师徒神镯，结义项链，回城神石、回城神石（大）等
	MSG_CTRL_HELPWND,				                  //帮助窗口，
    MSG_CTRL_SYSCONFIG_WND,                           //系统设置窗口
	MSG_CTRL_RELATION_WND,                            //社会关系
	MSG_CTRL_SHORTCUT_WND,                            //设置技能快捷键
	MSG_CTRL_RANGE_WND,				                  //排名系统
	//MSG_CTRL_UPLOAD_WND,                 			  //C2C上传页面
	MSG_CTRL_SOFT_KEYBOARD_CREATE,	                  //软键盘窗口
	MSG_CTRL_OPEN_CHANGEPASSWORDWND,                  //更改密码窗口
	MSG_CTRL_EXTERN_MESSAGEBOX_WND,	                  //弹出特殊对话框
	MSG_CTRL_EX_LEAVWORDS_WND,                        //黄榜大旗等留言窗口    
	MSG_CTRL_BOOTH_BUY_CONFIRM,      	              //确认了购买物品窗口
	MSG_CTRL_VOICE_WND,                               //语聊窗口
	MSG_CTRL_VOICE_ADD_WND,                           //语聊创建频道窗口
	MSG_CTRL_VOICE_CHAT_WND,                          //语聊聊天窗口
	MSG_CTRL_VOICE_CONFIG_WND,                        //语聊设置界面
	MSG_CTRL_VOICE_SEARCH_WND,                        //语聊搜索界面
	MSG_CTRL_VOICE_PASSWORD_WND,                      //语聊密码输入窗口
	MSG_CTRL_QUICKCONSUMESET_WND,		              //快速设置窗口
	MSG_CTRL_WOOOLSTORE_BUYCOUNT_WND,	              //商城确认填写购买道具,并提交购买道具的数量的窗口
	MSG_CTRL_WOOOLSTORE_SEND_WND,		              //商城赠送的留言窗口
	MSG_CTRL_WOOOLSTORE_SEND_OK_WND,	              //商城赠送的确认窗口
	MSG_CTRL_WOOOLSTORE_RECV_INFO_WND,	              //商城通知清包裹收赠品的窗口
	MSG_CTRL_WOOOLSTORE_RECV_OK_WND,	              //商城包裹收赠品的确定窗口
	MSG_CTRL_PRAYTREE_WND,                            //祈愿树窗口
	MSG_CTRL_REGISTER_INFO_WND,                       //防沉迷系统，补登身份信息
	MSG_CTRL_RADIO_WND,                               //电台，留言窗口
	MSG_CTRL_REGISTER_WND,                            //内嵌IE注册窗口
	MSG_CTRL_WUXING_PRIZE_WND = 4209,                 //五行天关发奖窗口
	MSG_CTRL_HEARTKNACK_WND,                          //灵力心诀窗口
	MSG_CTRL_SEAL_WND,					 	          //封元印
    MSG_CTRL_SEALBOOK_WND,				              //封元册
	MSG_CTRL_RESERVEPASSWORD_WND,                     //秘宝预留口令窗口
    MSG_CTRL_LUXNPCWND,                               //豪华 NPC 窗口
    MSG_CTRL_INPUT_NEW_PASWORD,			              //语聊改密码
    MSG_CTRL_CHECK_OPEN_WND,                          //语聊弹出框
    MSG_CTRL_HUANHUA_WND,                             //幻化界面
	MSG_CTRL_LINGXIBOX_WND,							  //至尊宝盒界面
	MSG_CTRL_MAZE_WND,								  //迷仙阵界面
	MSG_CTRL_RECEIVE_MSG_WND,						  //接收消息窗口
	MSG_CTRL_KFZ_MEMBER_LIST_WND,                     //查看本服参加跨服战成员窗口
	MSG_CTRL_KFZ_EDIT_MEMBER_WND,                     //编辑参加跨服战成员的窗口
	MSG_CTRL_KFZ_FLOW_WND,                            //跨服战流程窗口
	MSG_CTRL_KFZ_IE_WND,                              //跨服战内嵌IE窗口,用来查看排名及对阵图
	MSG_CTRL_KFZ_RESULT,                              //跨服结果窗口
	MSG_CTRL_KFZ_MEMBER_STATE_WND,                    //查看跨服战成员状态的窗口
	MSG_CTRL_KFZ_ARENA_WND,                           //查看跨服战擂台窗口	
	MSG_CTRL_GUILD_CHANGE_INFO,						  //行会信息改变
	MSG_CTRL_GUILD_FLAG_SEL_WND,					  //行会旗帜选择界面
	MSG_CTRL_COMMON_SELECT_WND,						  //通用选择窗口
	MSG_CTRL_TREASUREMAP_WND,						  //藏宝图窗口
	MSG_CTRL_NPCRECRUIT,                              //NPC招募窗口
	MSG_CTRL_LICENSEWND_WND,                          //用户协议窗口
	MSG_CTRL_VIPSTORE_WND,				              //VIP秘市窗口
	MSG_CTRL_EXTRACTUPGRADE_YUANSHI,				  //萃取升级原石
	MSG_CTRL_DEL_SKILL_WND,							  //删除技能界面
	MSG_CTRL_ELIXIR_WND,							  //仙灵丹界面
	MSG_CTRL_RELIVE_WND,                              //复活窗口          
	MSG_CTRL_RELIVECONFIRM_WND,                       //复活确认窗口
    MSG_CTRL_RELIVEWITHYUANBAO_WND,                   //用元宝复活窗口
	MSG_CTRL_PET_INFO_WND,                            //宠物信息界面
	MSG_CTRL_REPLY_WND,				                  //回复留言
	MSG_CTRL_FORWARD_WND,			                  //转发留言
	MSG_FUSHI_WND,									  //符石界面	
	MSG_CTRL_NOTE_MSG,						          //短语读取信息
	MSG_CTRL_NOTE_CHANGED,					          //短语改变了
	MSG_CTRL_FAIRYGUIDE_WND,						  //即时提示窗口
	MSG_CTRL_MEMOIRE_WND,							  //新手提示备忘录
	MSG_CTRL_PRODUCE_WND,							  //生产窗口
	MSG_CTRL_LUCKQYZ_WND,							  //幸运珍宝阁
	MSG_CTRL_BLESSZHONGZHOU_WND,                      //天佑中州窗口
	MSG_CTRL_InstanceZonePopUp_Wnd,					  //进副本弹出框
	MSG_CTRL_ACTIVITY_LOG_WND,						  //活动日志窗口
	MSG_CTRL_BAOJIAN_WND,							  //群英宝鉴窗口
	MSG_CTRL_TASKTREEDES_WND,						  //任务描述窗口
	MSG_CTRL_YUANBAO_WND,							  //元宝交易窗口
	MSG_CTRL_TIGERRUNE_WND,							  //点击虎符弹出界面
	//MSG_CTRL_TNEUPCHART_WND,                          //群英阵谱托管角色列表窗口
	//MSG_CTRL_TNEUPCTRLPANEL_WND,                      //群英阵谱队长角色控制窗口
	//MSG_CTRL_TNEUP_WND,                               //群英阵谱窗口
	//MSG_CTRL_TRUSTEESHIPCONFIG_WND,                   //托管队员设置窗口
	//MSG_CTRL_QUITTNEUP_WND,                           //被托管队员退出托管窗口
	MSG_CTRL_MERITORIOUS_WND,						  //功勋值界面
	MSG_CTRL_RTSINSTANCE_WND,						  //RTS副本界面(召唤进攻,守护,箭塔)
	//MSG_CTRL_RTSGUIDE_WND,						      //RTS教程界面
	MSG_CTRL_SHORTCUTKEY_WND,					      //界面快捷栏
	MSG_CTRL_TODAYACTIVITY_WND,                       //今日活动
	MSG_CTRL_TODAYACTIVITY_IE_WND,                    //今日活动内嵌ie界面
	MSG_CTRL_TODAYACTIVITY_LOCAL_WND,                 //今日活动_本地界面
	MSG_CTRL_TASKWUSEDAILY_WND,                       //五色日常任务界面
	MSG_CTRL_LEASEBOOTH_WND,						  //租赁摆摊窗口,未租赁状态或者自己租赁的
	MSG_CTRL_LEASEOTHERBOOTH_WND,					  //租赁摆摊窗口,别人租赁的
	MSG_CTRL_FIRM_WND,								  //商行窗口
	//MSG_CTRL_QUICKQUNYING_WND,						  //群英模式快速组队
	//MSG_CTRL_QUNYING_REPORT_WND,					  //群英战报
	//MSG_CTRL_ZHENPUCUILIAN_WND,					      //阵谱淬炼
	//MSG_CTRL_OFFLINETRUSTEESHIP_WND,				  //离线托管界面
	MSG_CTRL_FIRMDETAIL_WND,						  //收购商行详细信息窗口
	MSG_CTRL_EQUIPSOUL_WND,							  //器魂窗口
	MSG_CTRL_DIVINITYWING_WND,						  //神翼窗口
	MSG_CTRL_LICENSE2WND_WND,                         //PK协议窗口
	MSG_CTRL_QUICKCONSUME_FASHIONAL_WND,		      //快速设置窗口(新)
	MSG_CTRL_CHATPANEL_WND,							  //聊天(新)
	MSG_CTRL_AUTHEN_WND,							  //强制补全防沉迷
	MSG_CTRL_BLESSCOMPOUND_WND,						  //神佑融合
	MSG_CTRL_WENPEI_WND,							  //纹佩
	MSG_CTRL_ENTERSHAN_WND,							  //加入圣殿山
	MSG_CTRL_ADVENTURE_WND,							  //奇遇
	MSG_CTRL_COMPOSEEQUIP_WND,						  //装备合成
	MSG_CTRL_BOOTHGUILD_WND,                          //查看身边所有摊位
	MSG_CTRL_SANWEIFIRE_WND,						  //三味真火界面
	MSG_CTRL_LINGWUSANWEIFIRE_WND,					  //领悟三味真火界面
	MSG_CTRL_FABAOSHOW_WND,							  //法宝展示界面
	MSG_CTRL_FABAOLEVELUP_WND,						  //法宝升级强化
	MSG_CTRL_FIREARTIFICE_WND,						  //真火炼化
	MSG_CTRL_NOTICEIE_WND,							  //游戏IE公告
	MSG_CTRL_OTHERPLAYER_FABAOSHOW_WND,				  //其他玩家法宝展示界面
	MSG_CTRL_VIPCARD_WND,				              //Vip界面
	MSG_CTRL_DEFEND12GONG_WND,						  //12宫布阵界面
	MSG_CTRL_DEFEND12CTRL_WND,						  //12宫兵种控制界面
	MSG_CTRL_LOGIN_CHECK_MASK_WND,					  //登录检验码窗口
	MSG_CTRL_YIHUOZHANMSG_WND,						  //异火战消息
	MSG_CTRL_PLAYER_PANEL_WND,						  //时尚界面的人物信息面版
	MSG_CTRL_EQUIPCHAIFEN_WND,							  //装备拆分窗口
	MSG_CTRL_QISHOU_WND,								//骑术界面
	MSG_CTRL_HUFUFLUSH_WND,							  //赤金护符刷新
	MSG_CTRL_CAIHONGHELP_WND,						  //彩虹帮助
	MSG_CTRL_YUSHOULEVELUP_WND,						  //驭兽修炼
	MSG_CTRL_BINDPTWND_WND,							  //绑定盛大通行证
	MSG_CTRL_BINDPTMSGWND_WND,						  //退出绑定盛大通行证提示框















	//下面是传到窗口内部处理的消息
	MSG_CTRL_LEFT_FLOW_INFO,				          //左飘消息
	MSG_CTRL_VOICE_BTN_FLASH,                         //语聊闪烁
	MSG_CTRL_SETALARMSTATE,				              //更新指示灯
	MSG_CTRL_VOICE_CHAT_BEGIN,                        //开始语聊
	MSG_CTRL_FLASH_BTN,                               //新手帮助相关的特定按钮闪烁

	MSG_DELETE_CHAR_OK,			                      //确认删除角色,传到SelectCharWnd处理	
	MSG_CTRL_ENTER_GAME_BEGIN,                        //开始连接服务器
	MSG_CTRL_ENTER_GAME_STOP,                         //连接服务器失败
	MSG_CTRL_BOOTH_PRICE_FOCUS,      	              //摆摊设置价格框获得焦点
	MSG_CTRL_BOOTH_NAME_FOCUS,       	              //设置摊位名称获得焦点
	MSG_CTRL_WOOOLSTORE_BUYCONFIRM,		              //购买确认商城道具
	MSG_CTRL_EQUIPMENT_EFFECT,                        //播放圣灵精华特效消息
	MSG_CTRL_REQUIRE_GUID,							  //实物商城页面 重新请求更新一次GUID 和 token
	MSG_CTRL_RANGE_GOTO_PLACE,						  //排名窗口调用GotoPlace函数
	MSG_CTRL_CHANGE_AREA_BUTTON_STATUS,               //语聊搜索窗口区信息刷新
	MSG_CTRL_CHANGE_GROUP_BUTTON_STATUS,              //语聊搜索窗口组信息刷新
	MSG_CTRL_SEND_YUANSHEN,                           //分神选择确认
	MSG_CTRL_SEND_PASSWORD,							  //用户登录框发送密码
	MSG_CTRL_GUILD_MOBILIZE_CONFIRM,				  //动员令确认
	MSG_CTRL_REGISTER_INFO_WND_FOCUS,                 //防沉迷系统，补登身份信息
	MSG_CTRL_RENAMECHARNAME,	                      //重命名	
	MSG_CTRL_RELIVE_USEYANBAO_OK,                     //确认用元宝复活
	MSG_CTRL_RELIVE_USEYANBAO_CANCEL,                 //取消用元宝复活
    MSG_CTRL_LEVEL_CHECK,                             //上号检查
    MSG_CHANGE_GRAPHMODE,                             //更改显示模式后要求退出
    MSG_CTRL_SOFT_KEYBOARD_INPUT,                     //小键盘
	MSG_CTRL_QUICKBUY_OBJ_ARRIVE,        			  //快捷消费的物品到达，告诉触发的地方
    MSG_CTRL_RECORD_ADDPRIVATEMSG,                    //记录私聊
	MSG_CTRL_UPLOAD_ADD_GOOD,         		          //C2C弹出输入价格对话框
	MSG_CTRL_UPDATE_WND,							  //更新父窗口信息
	MSG_CTRL_NPCBOARD_UPDATE,                         //NPC公告板左侧数据更新
	MSG_CTRL_UPDATE_SKILLWND,                         //更新技能窗口
	MSG_CTRL_LEAV_MESSAGE_FOCUS,                      //爱情宣言 窗口焦点
	MSG_CTRL_DELETE_TEAMMEMBER,	                      //删除组队队员
	MSG_CTRL_USERINFO_USERNAME,                       //用户名
	MSG_CTRL_WOOOLSTORE_USERINFO_WND,                 //商城用户信息收集窗口
	MSG_CTRL_WOOOLSTORE_SEND_FOUCUS,	              //商城赠送的留言窗口焦点设置
	MSG_CTRL_DOUBLEMAGIC,                             //平滑模式
	MSG_CTRL_UPDATE_PROMPT_INFO,					  //新手提示信息更新
	MSG_CTRL_CLEAN_PROCUDE_GOOD_INFO,
	MSG_CTRL_LUCKQYZ_PRIZE,							  //幸运珍宝阁抽奖结果
	MSG_CTRL_LUCKQYZ_CHANCE,                          //幸运珍宝阁抽奖次数
	MSG_CTRL_WUXING_PRIZE_WND_CLOSE,                  //五行玄关窗口关闭
	MSG_CTRL_MINIMAP_WND_REFRESH,                     //跨地图寻路跳转地图后刷新小地图显示
	//MSG_CTRL_NOTIFY_TRUSTEESHIP_MODE,				  //托管模式改变
	//MSG_CTRL_QUNYING_EXP_WND,						  //确认群英模式是否开启经验加成
	MSG_CTRL_FLASH_BTN_WND,                           //新手帮助相关的特定窗口特定按钮闪烁
	MSG_CTRL_POPUP_ARROWTIP,                          //通知某个窗口冒泡
	MSG_CTRL_CUSTOM_PIAOHONG,				          //自定义飘红初始化消息	
	MSG_CTRL_LOGIN_CHECK_MASK_UPDATE,				  // 登录检验码更新
	//MSG_CTRL_FABAOSHOW_UPDATE,						  //法宝界面更新

	// 其他GameWnds结束
	MSG_GAMEWNDS_MESSAGE_END,
	////////////////////////////////////////////////////////////////////////////
};

#define WM_ADDMSG						WM_USER + 50
#define WM_INTERNET_ERROR				WM_USER + 51 
#define WM_SHOWFPS						WM_USER + 52
#define WM_ADDPAYMSG					WM_USER + 56
#define WM_SHOWPERF						WM_USER + 57
#define WM_CHATWND						WM_USER + 58
#define WM_FLASH_WND					WM_USER + 59
#define MSG_SOCKET                      WM_USER + 102
#define MSG_SOCKET_DOWNLOADFILE         WM_USER + 103
#define MSG_SOCKET_BGDOWNLOADFILE         WM_USER + 104
#define MSG_SOCKET_DOWNLOADFILE_LIMITSPEED         WM_USER + 105
#define WM_TEXTGAMEMODE_CHANGE			WM_USER + 500
#define MSG_IGW_APP                     WM_USER + 0xA200
#define MSG_IGW_WIDGET                  WM_USER + 0xA201
#define HOSTAPP_OPEN   1
#define HOSTAPP_CLOSE  2

#define MUSIC_PAN						500		// 800 
#define MAX_NOTE_BYTES					80		// 短信的最大留言字节数

//关于UuId Msg中 dwData 的定义
#define UUID_RECOMMEND					0		// 建议id
#define UUID_SUCCESS					1		// 申请成功
#define UUID_FAIL						2		// id已存在， 申请失败
#define UUID_ERROR						3		// 系统错
