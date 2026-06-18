#pragma once
#include "gsClientObj.h"

constexpr UINT BLOCK_SIZE = 0x100; // 256 per block
constexpr UINT MAX_MSG_ID = 0xA000; //消息wCmd最大值
constexpr UINT BLOCK_COUNT = (MAX_MSG_ID + BLOCK_SIZE - 1) / BLOCK_SIZE; // 块数量

class HandlerTable
{
public:
	using MsgHandler = VOID(CClientObj::*)(PMIRMSG pMsg, int datasize);
private:
	struct Block
	{
		MsgHandler handlers[BLOCK_SIZE];
		Block() { std::fill(std::begin(handlers), std::end(handlers), nullptr); }
	};
	std::unique_ptr<Block> m_blocks[BLOCK_COUNT];
public:
	HandlerTable()
	{
		setHandler(0x0040, &CClientObj::HandleRideHorse);//上下马
		setHandler(0x0042, &CClientObj::HandleAddFriend);//添加好友
		setHandler(0x0043, &CClientObj::HandleDeleteFriend);//删除好友
		setHandler(0x0044, &CClientObj::HandleConfirmFriend);//回答好友请求
		setHandler(0x0045, &CClientObj::HandleSetItemPosition);//向物品端发送包裹中物品的位置
		setHandler(0x0052, &CClientObj::HandleViewEquipment);//察看其他玩家信息
		setHandler(0x006a, &CClientObj::HandleLeaveServer);//退出游戏
		setHandler(0x00c5, &CClientObj::HandleRecordHomeStone);//设置使用物品的参数 回城神石等
		setHandler(0x0310, &CClientObj::HandleMasterApprentice);//拜师与招徒相关请求
		setHandler(0x0312, &CClientObj::HandleTradeRequest);//服务器通过有玩家请求和自己交易
		setHandler(0x0331, &CClientObj::HandlePetFeed);//从界面上直接喂食
		setHandler(0x0345, &CClientObj::HandleGuildMemberList);//新的行会成员列表请求
		setHandler(0x03e8, &CClientObj::HandleDropItem);//把包裹中的物品丢掉
		setHandler(0x03e9, &CClientObj::HandlePickupItem);//从地上捡起物品放入包裹
		setHandler(0x03eb, &CClientObj::HandleTakeOnItem);//从包裹里的物品装备到身上
		setHandler(0x03ec, &CClientObj::HandleTakeOffItem);//把身上的物品放入包裹
		setHandler(0x03ee, &CClientObj::HandleUseItem);//使用包裹中的物品(药品等)
		setHandler(0x03ef, &CClientObj::HandleDigCorpse);//挖尸体
		setHandler(0x03f0, &CClientObj::HandleSkillShortcut);//技能快捷键切换
		setHandler(0x03f1, &CClientObj::HandleSoftClose);//退出游戏
		setHandler(0x03f2, &CClientObj::HandleClickNpcOrShop);//要求和NPC进入交互
		setHandler(0x03f3, &CClientObj::HandleNpcClick);//点击到交互菜单
		setHandler(0x03f4, &CClientObj::HandleGetItemPrice);//卖物品时把物品放入托盘
		setHandler(0x03f5, &CClientObj::HandleNpcSellItem);//卖物品时按确定
		setHandler(0x03f6, &CClientObj::HandleBuyShopItem);//买物品时按确认
		setHandler(0x03f7, &CClientObj::HandleNpcSellList);//买物品种类时按确认
		setHandler(0x03f8, &CClientObj::HandleDropGold);//扔下金钱
		setHandler(0x03fa, &CClientObj::HandleConfirmFirstDialog);//确认首次公告
		setHandler(0x03fb, &CClientObj::HandleChangeGroupMode);//开启，关闭组队
		setHandler(0x03fc, &CClientObj::HandleAddGroupMember);//创建队伍
		setHandler(0x03fd, &CClientObj::HandleAddGroupMember);//添加队员
		setHandler(0x03fe, &CClientObj::HandleDeleteGroupMember);//删除队员
		setHandler(0x03ff, &CClientObj::HandleRepairItem);//修理时点击修理
		setHandler(0x0400, &CClientObj::HandlePutRepairItem);//把武器放入托盘中修理
		setHandler(0x0401, &CClientObj::HandleQueryTrade);//要求和玩家进入交易
		setHandler(0x0402, &CClientObj::HandlePutTradeItem);//把物品放入托盘
		setHandler(0x0404, &CClientObj::HandleCancelTrade);//关闭交易窗口
		setHandler(0x0405, &CClientObj::HandlePutTradeGold);//把金钱放入托盘
		setHandler(0x0406, &CClientObj::HandleQueryTradeEnd);//点击交易确认
		setHandler(0x0407, &CClientObj::HandlePutToBank);//仓库-保管物品
		setHandler(0x0408, &CClientObj::HandleTakeFromBank);//仓库-找回物品
		setHandler(0x0409, &CClientObj::HandleWalkToMapLink);//打开小地图
		setHandler(0x040b, &CClientObj::HandleViewGuildInfo);//察看行会信息
		setHandler(0x040c, &CClientObj::HandleViewGuildInfo);//察看行会主页
		setHandler(0x040e, &CClientObj::HandleJoinGuild);//要求加入行会
		setHandler(0x040f, &CClientObj::HandleRemoveGuildMember);//删除行会成员
		setHandler(0x0410, &CClientObj::HandleEditGuildNotice);//编辑行会公告
		setHandler(0x0412, &CClientObj::HandleViewGuildExp);//察看行会经验值
		setHandler(0x041f, &CClientObj::HandleRequestGroupPos);//查询队员位置信息
		setHandler(0x0892, &CClientObj::HandlePetBackExp);//灵兽返回经验
		setHandler(0x0959, &CClientObj::HandleTimeAchieve);//成就系统
		setHandler(0x097a, &CClientObj::HandleShortcutKey);//自定义快捷键
		setHandler(0x09a2, &CClientObj::HandleNewMail);//新邮件系统
		setHandler(0x09a6, &CClientObj::HandleSocialInfo);//社交信息
		setHandler(0x09b0, &CClientObj::HandleFengHao);//时长封号系统
		setHandler(0x0a02, &CClientObj::HandleFuncCollection);//功能集合
		setHandler(0x0a06, &CClientObj::HandleAvatarFrame);//头像圈
		setHandler(0x0a11, &CClientObj::HandleGuildFengHao);//行会封号
		setHandler(0x0aaa, &CClientObj::HandleReplyGuild);//行会-回复邀请
		setHandler(0x0ba0, &CClientObj::HandleTrainHorse);//驯马
		setHandler(0x0bc2, &CClientObj::HandleTurn);//转方向
		setHandler(0x0bc3, &CClientObj::HandleWalk);//行走
		setHandler(0x0bc4, &CClientObj::HandleGetMeal);//蹲(Alt+鼠标左键)
		setHandler(0x0bc5, &CClientObj::HandleRun);//跑步
		setHandler(0x0bc6, &CClientObj::HandleAttack);//一般攻击(左键点击)
		setHandler(0x0bc7, &CClientObj::HandleMine);//挖矿
		setHandler(0x0bc9, &CClientObj::HandleSpellSkill);//魔法攻击
		setHandler(0x0bca, &CClientObj::HandleSpecialHit);//攻杀剑术
		setHandler(0x0bcb, &CClientObj::HandleSpecialHit);//刺杀剑术
		setHandler(0x0bcc, &CClientObj::HandleStop);//停止行走
		setHandler(0x0bcd, &CClientObj::HandleBowAction);//抱拳
		setHandler(0x0bd0, &CClientObj::HandleSpecialHit);//半月剑术
		setHandler(0x0bd1, &CClientObj::HandleSpecialHit);//烈火剑术
		setHandler(0x0bd2, &CClientObj::HandleSpecialHit);//新增加剑法
		setHandler(0x0bd6, &CClientObj::HandleSay);//发送消息(包括结盟请求)
		setHandler(0x0c00, &CClientObj::HandleClientTime);//请求服务器时间
		setHandler(0x0c01, &CClientObj::HandlePetPickup);//通知宠物捡物
		setHandler(0x0c02, &CClientObj::HandlePutToPetBag);//保存到宠物包裹
		setHandler(0x0c03, &CClientObj::HandleGetFromPetBag);//从宠物包裹取出东西
		setHandler(0x1000, &CClientObj::HandleOpenMarket);//游戏商城
		setHandler(0x5eb1, &CClientObj::HandlePrivateShop);//摆摊请求
		setHandler(0x5eb2, &CClientObj::HandleDeleteTask);//放弃任务
		setHandler(0x5eb3, &CClientObj::HandleRequestPrivateShop);//请求守摊/请求取回金币元宝/请求取回物品
		setHandler(0x6891, &CClientObj::HandleCreateGuild);//向服务端发送一段NPC信息
		setHandler(0x6893, &CClientObj::HandlePersonSetting);//个性化签名和临时封号
		setHandler(0x8810, &CClientObj::HandlePutItem);//提交某项物品
		setHandler(0x8897, &CClientObj::HandleShowPetInfo);//宠物状态
		setHandler(0x88a6, &CClientObj::HandlePetWindow);//请求宠物相关信息（例如血量信息）
		setHandler(0x9701, &CClientObj::HandleHeroRank);//排名报名或请求更新自己数据
		setHandler(0x9703, &CClientObj::HandleQueryHeroRank);//请求自己或玩家排名数据
	}

	VOID setHandler(UINT msgId, MsgHandler handler)
	{
		UINT blockIdx = msgId / BLOCK_SIZE;
		if (!m_blocks[blockIdx])
			m_blocks[blockIdx] = std::make_unique<Block>();
		UINT offset = msgId % BLOCK_SIZE;
		m_blocks[blockIdx]->handlers[offset] = handler;
	}

	MsgHandler GetHandler(UINT msgId) const
	{
		UINT blockIdx = msgId / BLOCK_SIZE;
		if (msgId >= MAX_MSG_ID || !m_blocks[blockIdx])
			return nullptr;
		UINT offset = msgId % BLOCK_SIZE;
		return m_blocks[blockIdx]->handlers[offset];
	}
};