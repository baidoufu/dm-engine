#include "Global/Global.h"
#include "GameControl.h"
#include "GameData\MagicDefine.h"
#include "BaseClass/Control/Control.h"
#include "GameData/GameData.h"
#include "Global/Interface/AudioInterface.h"
#include "Global/Interface/StreamInterface.h"
#include "GameData/GameGlobal.h"
#include "GameData/MsgBoxMgr.h"
#include "GameAI/AIAutoPickMgr.h"
#include "GameAI/AIAutoEatMgr.h"
#include "GameData/WooolStoreData.h"
#include "GameData/TalkMgr.h"
#include "GameData/ConfigData.h"
#include "GameData/NpcData.h"
#include "BaseClass/Misc/Internet.h"
#include "GameData/OtherData.h"
#include "GameData/PetData.h"
#include "GameData/BoothData.h"
#include "GameData\MagicCtrlMgr.h"
#include "GameClient\WidgetManager.h"
#include "GameData/TaskData.h"
#include "BaseClass/Compress/Compr.h"
#include "BaseClass/Control/GoodGrid.h"
#include "WndClass/GameWnd/NpcQuickWnd.h"

void CGameControl::MSG_Exchange_Talk_End(const char * msg,int iLen)
{
	g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
    g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
	g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
	g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

	g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);

	g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
}

void CGameControl::MSG_Exchange_Buy_Menu(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen-12);

	BYTE byType = msg[10];//0：普通物品，1：用绑定金币购买的物品

	int nPos = 0;
	//int iNum = msg[6];
	BYTE     iNum = msg[6];

	g_NPC.GetVectorSellData().clear();

	for(int i = 0 ; i < iNum; i++)
	{
		string name = StringUtil::toStr(buf,nPos);
		int bNext = StringUtil::toInt(buf,nPos);
		int price = StringUtil::toInt(buf,nPos);
		int id    = StringUtil::toInt(buf,nPos);
		int nDura = StringUtil::toInt(buf,nPos);
		_SellData SellData(name.c_str(),bNext,price,id,byType);
		SellData.nDura = nDura;

		g_NPC.GetVectorSellData().push_back(SellData);
	}
	g_pControl->PopupWindow(MSG_CTRL_NPCBUYWND,OPER_CREATE,1);
}

void CGameControl::MSG_Exchange_PT_Buy_Menu(const char* msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen-12);

	int   nPos = 0;
	char  temp[256] = {0};	 //名称
	char  strId[256] = {0};  //ID
	int   price = 0;       //价格
	BYTE   iNum = msg[6];

	if(msg[8] == 0)
	{
		g_NPC.GetVectorPTData().clear();

		for(int i = 0 ; i < iNum && nPos < iLen-12; i++)
		{
			string strId = StringUtil::toStr(buf,nPos);
			string name = StringUtil::toStr(buf,nPos);
			int price = StringUtil::toInt(buf,nPos);

			_SellData SellData(name.c_str(),strId.c_str(),price);

			g_NPC.GetVectorPTData().push_back(SellData);
		}
	}
	else if(msg[8] == 1) //宠物
	{

	}
	else if(msg[8] == 2) //快捷消费2.0
	{
		g_WooolStoreMgr.GetQuickBuyItemMap().clear();

		for(int i = 0 ; i < iNum && nPos < iLen-12; i++)
		{
			string strId = StringUtil::toStr(buf,nPos);
			string name = StringUtil::toStr(buf,nPos);
			int price = StringUtil::toInt(buf,nPos);

			CQuickItem quickItem;
			quickItem.strName = name;
			quickItem.strItemID = strId;
			quickItem.iPrice = price;

			g_WooolStoreMgr.GetQuickBuyItemMap()[name] = quickItem;
		}
	}
}

void CGameControl::MSG_Exchange_Second_Buy_Menu(const char * msg,int iLen)
{
	if(iLen <= CMD_SIZE)
		return;

	BYTE byType = msg[10];//0：普通物品，1：用绑定金币购买的物品
	BYTE byGoodLen = BYTE(msg[7]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	string buf;
	buf.assign(msg+CMD_SIZE,iLen - CMD_SIZE);

	g_NPC.SetSecBuyStartPos(msg[8]);
	int nPos = 0;
	char num = msg[6];

	g_NPC.GetVectorSellData().clear();
	g_NPC.GetGoodList().clear();

	for(int i=0; i< num; i++)  
	{
		CGood tempGood;
		tempGood.FromBuffer(buf.c_str()+( i* byGoodLen), true,byGoodLen);
		g_NPC.GetGoodList().push_back(tempGood);

		int nDura = tempGood.GetDura();
		nDura = nDura/100;

		_SellData SellData(tempGood.GetName(),nDura,tempGood.GetPrice(),tempGood.GetID(),byType);
		g_NPC.GetVectorSellData().push_back(SellData);
	}
	g_pControl->Msg(MSG_CTRL_NPCBUYWND,4);
}

void CGameControl::MSG_Exchange_Sale_Object(const char * msg,int iLen)
{
	DWORD iMoney = 0;	
	iMoney = *(DWORD*)msg;

	g_NPC.SetSaleGold(iMoney);

	BYTE byBind = Conv_BYTE(msg+6);//0表示金币，1表示绑定金币
	if (byBind == 0)
		g_pControl->MsgToWnd("NpcSaleWnd",MSG_CTRL_NPCSALEWND,10);
	else
		g_pControl->MsgToWnd("NpcSaleWnd",MSG_CTRL_NPCSALEWND,11);
	

	if(g_NPC.IsExchangeFromQuickNpcWnd())//是通过图形化g_NPC窗口进行交易
	{
		g_NPC.SetExchangeFromQuickNpcWnd(false);

		if(iMoney <= 0)
		{
			g_TalkMgr.PushSysTalk("该物品不能在此出售");
			//把东西放回包裹
			if(g_NPC.GetGood().GetID() != 0)
			{
				g_NPC.SetExchangeFromQuickNpcWnd(false);
				CGood &temp = g_NPC.GetGood();
				if(temp.GetID() != 0)
				{
					for(int i = 0; i < MAX_PACKAGE_ELEMENT ; i++)
					{
						if(SELF.GetPackageGood(i).GetID() != 0)
							continue;

						SELF.GetPackageGood(i) = temp;
						g_NPC.GetGood().SetID(0);
						break;
					}
				}
			}		
		}
		else
		{
			string strTemp = StringUtil::format("您将要以<color=green %u>的价格卖出<color=green %s>，您确定要卖吗?",iMoney,g_NPC.GetGood().GetName());
			g_MsgBoxMgr.PopTagOkCancelBox(strTemp.c_str(),MSG_CTRL_QUICKNPC_SELL_OK,0,NULL,MSG_CTRL_QUICKNPC_SELL_CANCEL);
		}
	}

}


void  CGameControl::MSG_Exchange_Sale_Ok(const char * msg,int iLen)
{
	BYTE byBind = Conv_BYTE(msg+6);//0表示金币，1表示绑定金币

	if (byBind == 0)
	{
		SELF.SetGold(*(DWORD*)msg);
	}
	else if (byBind == 1)
	{
		SELF.SetBindGold(*(DWORD*)msg);
	}

	g_NPC.GetGood().SetID(0);
	g_NPC.SetSaleGold(0);
}

void  CGameControl::MSG_Exchange_Sale_Fail(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("你不能卖这个物品！");

	g_NPC.BackToPackage();
	g_NPC.SetSaleGold(0);
}

void  CGameControl::MSG_Exchange_Buy_Fail(const char * msg,int iLen)
{
	string str;

	switch(msg[0])
	{
	case 0x01:
		str = "物品被卖出。";
		break;
	case 0x02:
		str = "不能携带更多的物品了！";
		break;
	case 0x03:
		str = "你没有足够的钱来购买物品。";
		break;
	case 0x04:
		str = "你没有足够的元宝来购买物品。";
		break;
	case 0x05:
		str = "对方不能容纳更多金币。";
		break;
	case 0x06:
		str = "对方不能容纳更多元宝。";
		break;
	default:
		str = "购买物品失败！";
		break;
	}

	g_pControl->MsgToWnd("OtherBoothWnd",MSG_CTRL_OTHER_BOOTH,2);

	g_MsgBoxMgr.PopSimpleAlert(str.c_str());

	g_OtherBoothData.GetAlreadyBuy().clear();
}

void CGameControl::MSG_Exchange_Poison_Menu(const char * msg,int iLen)
{
	//扩充
	BYTE byType = msg[10];//0：普通物品，1：用绑定金币购买的物品

	string buf;
	buf.assign(msg+12,iLen-12);

	int iSize = (int)buf.size();
	int nPos = 0;

	g_NPC.GetVectorSellData().clear();

	while(nPos	< buf.size())
	{
		string name = StringUtil::toStr(buf,nPos);
		if(name.empty()) break;

		int bNext = StringUtil::toInt(buf,nPos);
		int price = StringUtil::toInt(buf,nPos);
		int id    = StringUtil::toInt(buf,nPos);

		_SellData SellData(name.c_str(),bNext,price,id,byType);

		g_NPC.GetVectorSellData().push_back(SellData);
	}
	g_pControl->PopupWindow(MSG_CTRL_NPCBUYWND,OPER_CREATE,3);
}


void  CGameControl::MSG_Exchange_Poison_Need(const char * msg,int iLen)
{
	if(iLen < CMD_SIZE)
		return;

	//string buf;
	//buf.assign(msg+CMD_SIZE,iLen-CMD_SIZE);

	const char *p = msg + CMD_SIZE;
	int nPos = 0;
	char temp[100];
	char num[100];
	DWORD dwGoodID;
	int size= msg[10];
	for(int i = 0; i<size;++i)
	{
		int j;
		for(j = nPos;j<iLen;++j)
			if(p[j] =='/')
				break;
		memcpy(temp, p+nPos,j-nPos);
		temp[j-nPos] = 0;
		nPos = j+1;
		for(j = nPos;j<iLen;++j)
			if(p[j] =='/')
				break;
		memcpy(num,p+nPos,j-nPos);
		num[j-nPos] = 0;
		nPos = j + 1;
		dwGoodID = atoi(num);
		if(dwGoodID==0)
			return ;

		int iPos = SELF.PackageGood().FindGoodPos(dwGoodID);
		if(iPos >= 0)
		{
			SELF.GetPackageGood(iPos).SetID(0);
		}
	}
}

void  CGameControl::MSG_Exchange_Poison_Money(const char * msg,int iLen)
{
	SELF.SetGold(*(DWORD*)msg);

	g_NPC.SetPoisonFlag(false);
	g_MsgBoxMgr.PopSimpleAlert("你需要的物品被成功打造！");
}


void CGameControl::MSG_Make_Poison_Fail(const char * msg,int iLen)
{
	g_NPC.SetPoisonFlag(false);
	g_MsgBoxMgr.PopSimpleAlert("你缺乏必需的物品！"); 
}


void CGameControl::MSG_Exchange_Buy_Ok(const char * msg,int iLen)
{
	if(g_NPC.GetSecondBuy())
	{
		vector<_SellData>::iterator itrSell = g_NPC.GetVectorSellData().begin();
		vector<CGood>::iterator  itrGood = g_NPC.GetGoodList().begin();
		DWORD dwID = *((DWORD*)(msg+6));
		for(;itrSell != g_NPC.GetVectorSellData().end() && itrGood != g_NPC.GetGoodList().end();itrSell++,itrGood++)
		{
			if(dwID == (*itrSell).id)
			{
				g_NPC.GetVectorSellData().erase(itrSell);
				g_NPC.GetGoodList().erase(itrGood);
				break;
			}
		}
		g_NPC.SetSecondBuy(false);
	}
	SELF.SetGold( *((DWORD*)msg));
}

void CGameControl::MSG_Exchange_Talk_Menu(const char * msg,int iLen)
{
	string buf;
	buf.assign(msg+12,iLen - 12);

	BYTE byType = msg[6];

	if (g_MapFinder.IsWalking())
	{
		DWORD dwID = Conv_DWORD(msg);
		VPJumpPoint::iterator it = g_MapFinder.GetPath().begin();
		
		if (it != g_MapFinder.GetPath().end())
		{
			JumpPoint* pJump = *it;
			if(pJump->bNpc && !pJump->bHaveSendNpcCom)
			{
				pJump->bHaveSendNpcCom = true;
				VString& vS = pJump->vNpcCom;
				VString::iterator its = vS.begin();
				//if (g_MapFinder.GetPath().size() > 1)
				{
					while(its != vS.end())
					{
						g_pGameControl->SEND_Exchange_Menu_Click(dwID,(*its).c_str());
						its++;
					}
				}
			}
		}
		else g_MapFinder.SetWalking(false);
	}

    if(byType == 0x00 || byType == 0x11 || byType == 0x21 || byType == 0x22 || byType == 0x23) //0x00传统方式,0x11公共板右侧方式,0x22:查看雕像界面,0x23:指定头像的npc对话框,
	{
        int nPos = buf.find_first_of("/");
		DWORD dwID = Conv_DWORD(msg);
		bool bChanged = (dwID == g_NPC.GetID())?false:true;
		g_NPC.SetID(dwID);

		g_NPC.SetName(buf.substr(0,nPos).c_str());
        g_TagTextMgr.GetNpcText().Clear();
        g_TagTextMgr.GetNpcText().Parse(buf,nPos + 1,"\\");

		WORD type = *((WORD*)(msg + 8));
        
        if(byType == 0x11)
        {
            g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
            g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
			g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);
			g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CREATE);
 			g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
			g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

			g_pControl->PopupWindow(MSG_CTRL_TASKWUSEDAILY_WND,OPER_CLOSE);

			g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
		}
        else
        {
			g_pControl->PopupWindow(MSG_CTRL_NPCBUYWND,OPER_CLOSE);
			g_pControl->PopupWindow(MSG_CTRL_NPC_QUICK_WND,OPER_CLOSE);

			g_pControl->PopupWindow(MSG_CTRL_TASKWUSEDAILY_WND,OPER_CLOSE);

			g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
            
            if(!g_NPC.IsRepairWindow())
            {
				g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CLOSE);
            }
			g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);

			if (byType == 0x23)
			{
				g_pControl->PopupWindow(MSG_CTRL_NPCWND,(bChanged?OPER_RECREATE:OPER_CREATE),type);
			}		
			else
			{
				g_pControl->PopupWindow(MSG_CTRL_NPCWND,(bChanged?OPER_RECREATE:OPER_CREATE));
			}		
		}
    }
	else if(byType== 0x10) //公共板左侧
	{
		g_TagTextMgr.GetBoard().Clear();
		int nPos = buf.find_first_of("/");
		g_TagTextMgr.GetBoard().Parse(buf,nPos+1,"\\");
		g_pControl->Msg(MSG_CTRL_NPCBOARDWND,MSG_CTRL_NPCBOARD_UPDATE);
	}
	//else if(byType== 0x20)
	//{
	//	g_TagTextMgr.Clear();

	//	string str = p+12;

	//	int nPos = str.find_first_of("/");
	//	g_TagTextMgr.Parse(str,nPos+1,"\\");;
	//	g_TagTextMgr.SetNpcType(0);
	//}
	else if(byType == 0x30)  //传世宝典,新手帮助
	{
		g_TagTextMgr.GetGuideData().Clear();
		int nPos = buf.find_first_of("/");
		g_NPC.SetID(Conv_DWORD(msg));
		g_TagTextMgr.GetGuideData().Parse(buf,nPos+1,"\\");
		g_pControl->PopupWindow(MSG_CTRL_NPCBOOK_WND,OPER_CREATE,2);
	}
	//else if(byType == 0x41)  //greeting npc
	//{
	//	g_pGameData->GetWebData().Clear();

	//	string str = p+12;

	//	int nPos = str.find_first_of("/");
	//	g_NPC.SetID(*((DWORD *)p));
	//	g_pGameData->GetWebData().Parse(str,nPos+1,"\\");
	//	g_pGameData->GetWebData().SetNpcType(0);
	//}
	else if(byType == 0x31) //传世要闻
	{
		g_TagTextMgr.GetBookData().Clear();
		int nPos = buf.find_first_of("/");
		g_TagTextMgr.GetBookData().Parse(buf,nPos+1,"\\");
		g_pControl->PopupWindow(MSG_CTRL_NPCBOOK_WND,OPER_CREATE,1);
	}
	else if(byType == 0x40)//每周活动
	{
		if(iLen <= 12)	return;
		iLen -= 12;		

		string Name;
		string Time;
		string Date;
		int iStart = 0;
		int iIndexStart = 0,iIndexEnd = 0;

		//时间
// 		struct tm *newtime = NULL;
// 		__time32_t t_time;
// 		_time32(&t_time);
// 		t_time += g_dwServerTime;
// 		newtime = _localtime32(&t_time);

		g_NPC.GetVectorActData().clear();
		TRY_BEGIN
			while(iIndexEnd < buf.size())
			{
				bool bIsGoing = false;
				iIndexStart = buf.find_first_of('<',iIndexEnd);
				if(iIndexStart == string::npos) break;
				iIndexEnd = buf.find_first_of('>',iIndexStart);
				if(iIndexEnd == string::npos) break;
				Name = buf.substr(iIndexStart + 1,iIndexEnd - iIndexStart - 1);

				iIndexStart = buf.find_first_of('<',iIndexEnd);
				if(iIndexStart == string::npos) break;
				iIndexEnd = buf.find_first_of('>',iIndexStart);
				if(iIndexEnd == string::npos) break;
				Time = buf.substr(iIndexStart + 1,iIndexEnd - iIndexStart - 1);

				iIndexStart = buf.find_first_of('<',iIndexEnd);
				if(iIndexStart == string::npos) break;
				iIndexEnd = buf.find_first_of('>',iIndexStart);
				if(iIndexEnd == string::npos) break;
				Date = buf.substr(iIndexStart + 1,iIndexEnd - iIndexStart - 1);

				_ACTIVITY ActivityData(Name.c_str(),Time.c_str(),Date.c_str());

				iIndexStart = buf.find_first_of('<',iIndexEnd);
				if(iIndexStart == string::npos) break;
				iIndexEnd = buf.find_first_of('>',iIndexStart);
				if(iIndexEnd == string::npos) break;
				string strNote = buf.substr(iIndexStart+1,iIndexEnd - iIndexStart - 1);
				StringUtil::replace_all(strNote,"/n","\\");//不知道为什么这个脚本中用"/n"表示换行,而不用npc脚本同样的"\"
				ActivityData.content = strNote;

				int iPrizeStart = buf.find("<Prize",iIndexEnd);//新版本的有<Prize,老的没有
				if(iPrizeStart != string::npos)
				{
					iIndexEnd = iPrizeStart + 6;
					int iPrizeEnd = buf.find("/>",iPrizeStart);
					if(iPrizeEnd != string::npos)
					{
						iIndexEnd = iPrizeEnd + 2;
						string strPrize = buf.substr(iPrizeStart + 6,iPrizeEnd - iPrizeStart - 6);
						StringUtil::replace_all(strPrize,"/n","\\");//不知道为什么这个脚本中用"/n"表示换行,而不用npc脚本同样的"\"
						ActivityData.prize = strPrize;
					}
				}

				g_NPC.GetVectorActData().push_back(ActivityData);
			}
		TRY_END		

		g_pControl->Msg(MSG_CTRL_WEEK_ACTIVITY_WND,OPER_CREATE);
	}
    else if(byType == 0x50) //豪华NPC界面
    {
		WORD type = *((WORD*)(msg + 8));

		int nPos = buf.find_first_of("/");
		g_NPC.SetID(Conv_DWORD(msg));
		g_NPC.SetName(buf.substr(0,nPos).c_str());
		g_TagTextMgr.GetNpcText().Clear();

		if (type == 24)//行会界面的神工坊页签
		{
			g_TagTextMgr.GetNpcText2().Clear();
			size_t iTopPos = buf.find("<endtop>");
			if (iTopPos != string::npos)
			{
				string szTemp = buf.substr(nPos + 1,iTopPos - nPos - 1);
				g_TagTextMgr.GetNpcText().Parse(szTemp,0,"\\");
				g_TagTextMgr.GetNpcText2().Parse(buf,iTopPos + 8,"\\");
			}
			else
			{
				g_TagTextMgr.GetNpcText().Parse(buf,nPos + 1,"\\");
			}
		}
		else
		{
			g_TagTextMgr.GetNpcText().Parse(buf,nPos + 1,"\\");
		}


		g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);	

		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);

		if (type == 60)
		{

			_WuSeTaskStruct wusetask;
			string strName;
			string tasktyle;
			string taskcolor;

			int iIndexEnd = 0;

			int data = 0;

			std::string::size_type iPosStart = buf.find("<taskcolornew= ",0);
			if(iPosStart != string::npos)
			{
				data = 11;
				iIndexEnd = iPosStart + 15;
				int iPosEnd = buf.find(">",iPosStart);
				if(iPosEnd != string::npos)
				{
					iIndexEnd = iPosEnd + 1;
					wusetask.color = atoi(buf.substr(iPosStart + 15,iPosEnd - iPosStart - 15).c_str());
				}
			}
			else
			{
				data = 10;
			}
			
			iPosStart = buf.find("<taskName = ",0);
			if(iPosStart != string::npos)
			{
				iIndexEnd = iPosStart + 12;
				int iPosEnd = buf.find(">",iPosStart);
				if(iPosEnd != string::npos)
				{
					iIndexEnd = iPosEnd + 1;
					wusetask.strName = buf.substr(iPosStart + 12,iPosEnd - iPosStart - 12);

					int iPosEnd2 = wusetask.strName.find("/@",0);
					if(iPosEnd2 != string::npos)
					{						
						wusetask.strCommand1 = wusetask.strName.substr(iPosEnd2+1);
						wusetask.strName = wusetask.strName.substr(0, iPosEnd2);
					}
				}
			}

			iPosStart = buf.find("<tasktype = ",0);
			if(iPosStart != string::npos)
			{
				iIndexEnd = iPosStart + 12;
				int iPosEnd = buf.find(">",iPosStart);
				if(iPosEnd != string::npos)
				{
					iIndexEnd = iPosEnd + 1;
					wusetask.type = buf.substr(iPosStart + 12,iPosEnd - iPosStart - 12);
				}
			}

			iPosStart = buf.find("<taskcolor= ",0);
			if(iPosStart != string::npos)
			{
				iIndexEnd = iPosStart + 12;
				int iPosEnd = buf.find(">",iPosStart);
				if(iPosEnd != string::npos)
				{
					iIndexEnd = iPosEnd + 1;
					wusetask.color = atoi(buf.substr(iPosStart + 12,iPosEnd - iPosStart - 12).c_str());
				}
			}

			iPosStart = buf.find("<colorfresh=",0);
			if(iPosStart != string::npos)
			{
				iIndexEnd = iPosStart + 12;
				int iPosEnd = buf.find(">",iPosStart);
				if(iPosEnd != string::npos)
				{
					iIndexEnd = iPosEnd + 1;
					wusetask.strCommand2 = buf.substr(iPosStart + 12,iPosEnd - iPosStart - 12);

					int iPosEnd2 = wusetask.strCommand2.find("/@",0);
					if(iPosEnd2 != string::npos)
					{						
						wusetask.fresh = atoi(wusetask.strCommand2.substr(0, iPosEnd2).c_str());
						wusetask.strCommand2 = wusetask.strCommand2.substr(iPosEnd2+1);
					}

					wusetask.strInfo = buf.substr(iIndexEnd);
				}
			}

			g_pControl->PopupWindow(MSG_CTRL_TASKWUSEDAILY_WND,OPER_CREATE);
			g_pControl->Msg(MSG_CTRL_TASKWUSEDAILY_WND,data,(CControl*)&wusetask);
			
		}

		if (type == 56 || type == 57)//宗族争夺战(56),纹配(57),gs有特殊用处,客户端不管,不能用这个编号
		{

		}
		if(type >= 50 && type < 56)//npc招募界面
		{
			g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_RECREATE,type - 50);
		}
		else if (type < 20)//豪华NPC,离火风魔阵
		{	
			g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);
			g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_RECREATE,type);
		}
		else if(type >= 20 && type < 50)//行会塔NPC窗口
		{
			//[2009-9-17]
			if (type == 27)//议事厅任务
			{
				CTaskData::VOldTaskList& vTaskList = g_TaskData.GetGuildTaskList();
				g_TaskData.ClearAllForGuild();//清除数据

				//接取任务描述
				int iCurPos = 0;
				std::string::size_type iPos = buf.find("<TaskName>",iCurPos);
				while (iPos != std::string::npos)
				{
					CTaskData::_OldTask* pTask = new CTaskData::_OldTask;
					int iBegin = iPos+10;
					pTask->strTaskName = StringUtil::toStr(buf,iBegin);
					pTask->wTaskID = StringUtil::toUInt(buf,iBegin);
					pTask->wTotalNum = StringUtil::toUInt(buf,iBegin);
					pTask->wRevNum = StringUtil::toUInt(buf,iBegin);
					iPos = buf.find("<TaskDes>",iBegin);

					if (iPos == std::string::npos) 
					{
						delete pTask;
						break;
					}
					iPos = iPos + 9;

					std::string::size_type iPos1 = buf.find("<TaskName>",iPos);
					string temp;
					if (iPos1 != std::string::npos)
					{							
						temp = buf.substr(iPos,iPos1 - iPos);
						pTask->strDesc.Parse(temp,0,"\\");
						vTaskList.push_back(pTask);
						iPos = iPos1;
					}
					else
					{//找不到，认为是最后一个							
						temp = buf.substr(iPos);	
						pTask->strDesc.Parse(temp,0,"\\");
						vTaskList.push_back(pTask);
						break;
					}					
				}
			}

			if (type != 100)
			{
				g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_RECREATE,MAKELONG(2,type - 19));
				g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
			}
		}        
    }
	//else if (byType == 0x51)//IEWnd窗口
	//{
	//	std::string strUrl;
	//	int nPos = buf.find_first_of("/");
	//	g_NPC.SetID(Conv_DWORD(msg));
	//	g_NPC.SetName(buf.substr(0,nPos).c_str());

	//	int nPosTmp = buf.find("!@");
	//	if (nPosTmp != string::npos)
	//	{
	//		nPos = buf.find(">",nPos);
	//		strUrl.assign(buf.c_str() + nPosTmp + 3,nPos - nPosTmp - 3);
	//	}

	//	g_TagTextMgr.GetNpcText().Clear();
	//	g_TagTextMgr.GetNpcText().Parse(buf,nPos + 1,"\\");

	//	g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
	//	g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
	//	g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
	//	g_pControl->PopupWindow(MSG_CTRL_IENPC_WND,OPER_RECREATE);
	//	g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
	//	g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);

	//	if (strUrl.length() > 0)
	//	{
	//		g_WidgetMgr.OpenNpcIEUrl(strUrl.c_str());
	//	}
	//}
	else if (byType == 0x52)//编辑参加跨服战成员的窗口
	{
		g_pControl->PopupWindow(MSG_CTRL_KFZ_EDIT_MEMBER_WND,OPER_CREATE);		
	}
	else if(byType== 0x70)//奇遇未完成
	{
		g_NPC.SetID(Conv_DWORD(msg));
		g_TagTextMgr.GetQiYuText().Clear();
		int nPos = buf.find_first_of("/");
		g_TagTextMgr.GetQiYuText().Parse(buf,nPos+1,"\\");

		g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);
		g_pControl->PopupWindow(MSG_CTRL_TASKWUSEDAILY_WND,OPER_CLOSE);
		

		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND,OPER_CREATE);
		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,22);
	}
	else if(byType== 0x71)//奇遇完成
	{
		g_NPC.SetID(Conv_DWORD(msg));
		g_TagTextMgr.GetQiYuText().Clear();
		int nPos = buf.find_first_of("/");
		g_TagTextMgr.GetQiYuText().Parse(buf,nPos+1,"\\");

		g_pControl->PopupWindow(MSG_CTRL_NPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_LUXNPCWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCBOARDWND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_NPCRECRUIT,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_YUANBAO_WND,OPER_CLOSE);
		g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM);
		g_pControl->PopupWindow(MSG_CTRL_TASKWUSEDAILY_WND,OPER_CLOSE);


		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND,OPER_CREATE);
		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,21);
	}
	
}


void CGameControl::MSG_Exchange_Storage_Window(const char * msg,int iLen)
{
	g_NPC.GetGood().SetID(0);

	g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,3);//3表示保管物品窗口
}

void CGameControl::MSG_Exchange_Storage_Ok(const char * msg,int iLen)
{
	g_NPC.GetGood().SetID(0);
}

void CGameControl::MSG_Exchange_Storage_Failed(const char * msg,int iLen)
{
	//把东西放回包裹
	if(g_NPC.IsExchangeFromQuickNpcWnd() && g_NPC.GetGood().GetID() != 0)//是通过图形化g_NPC窗口
	{
		g_NPC.SetExchangeFromQuickNpcWnd(false);
		CGood &temp = g_NPC.GetGood();
		if(temp.GetID() != 0)
		{
			for(int i = 0; i < MAX_PACKAGE_ELEMENT ; i++)
			{
				if(SELF.GetPackageGood(i).GetID() != 0)
					continue;

				SELF.GetPackageGood(i) = temp;
				g_NPC.GetGood().SetID(0);
				break;
			}
		}
	}
	g_MsgBoxMgr.PopSimpleAlert("您的仓库已满，不能再保管更多物品了。"); 
}

void CGameControl::MSG_Exchange_Getback_Ok(const char * msg,int iLen)
{
	int size = g_NPC.GetStorageData().size();

	vector<_StorageData>::iterator itr = g_NPC.GetStorageData().begin();
	vector<CGood>::iterator itr2 = g_NPC.GetGoodList().begin();
	for (;itr != g_NPC.GetStorageData().end();itr ++)
	{
		if(Conv_DWORD(msg) == itr->id)
		{
			g_NPC.GetStorageData().erase(itr);
			g_NPC.GetGoodList().erase(itr2);
			break;
		}
		itr2 ++;
	}
}

void CGameControl::MSG_Exchange_Getback_Fail(const char * msg,int iLen)
{
	g_MsgBoxMgr.PopSimpleAlert("您不能取回物品");
}

void CGameControl::MSG_Exchange_Getback_Window(const char * msg,int iLen)
{
	string buf;
	BYTE byGoodLen = BYTE(msg[7]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	if(msg[6] == 1)		//消息体进过压缩
	{
		DWORD size = 128 * 1024;
		Uncompress((byte *)g_cBuf,&size,(byte *)(msg + CMD_SIZE),iLen - CMD_SIZE);
		buf.assign(g_cBuf,size);		
	}
	else
	{
		buf.assign(msg + CMD_SIZE,iLen - CMD_SIZE);
	}		

	int nPos = 0;
	WORD wMaxCount = Conv_WORD(msg+ 8);//最大包裹个数
	WORD num = Conv_WORD(msg + 10);//实际物品个数
	g_NPC.GetStorageData().clear();
	g_NPC.GetGoodList().clear();

	for(int i= 0; i < num ; i++)
	{
		if((i + 1) * byGoodLen > buf.size() )
			return;

		CGood tempGood;

		tempGood.FromBuffer(buf.c_str() + i * byGoodLen,false,byGoodLen);

		g_NPC.GetGoodList().push_back(tempGood);

		int durSmall = tempGood.GetDura();
		durSmall = (durSmall+500)/1000;
		int durBig =  tempGood.GetDuraMax();
		durBig = (durBig+500)/1000;

		//1.908商城道具 常胜玄兵 盛宴蛋糕 黄榜大旗 由于服务器使用这两个量作为时间，所以写死
		if((tempGood.GetStdMode() == 52 && tempGood.GetShape() == 156)||
			(tempGood.GetStdMode() == 37 && (tempGood.GetShape() == 17 || tempGood.GetShape() == 40)) )
		{
			durSmall = 1;
			durBig = 1;
		}

		int flag = 0; 
		if((tempGood.GetFlag() & 0x000000ff) >0 && tempGood.GetFlag2() >0 ) //封+锁
			flag = 3;
		else if((tempGood.GetFlag() & 0x000000ff) >0) //封
			flag = 2;
		else if(tempGood.GetFlag2() > 0) //锁
			flag = 1;

		_StorageData StorageData(tempGood.GetName(),durSmall,durBig,tempGood.GetID(),flag);

		g_NPC.GetStorageData().push_back(StorageData);
	}
	g_pControl->PopupWindow(MSG_CTRL_NPCBUYWND,OPER_CREATE,2);
}


void CGameControl::MSG_Exchange_Sale_Window(const char * msg,int iLen)
{
	g_NPC.GetGood().SetID(0);
	g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,1);//1表示卖物品窗口
}

void CGameControl::MSG_Exchange_Repair_Window(const char * msg,int iLen)
{
	g_NPC.GetGood().SetID(0);

	char byType = msg[10];
	BYTE byBind = Conv_BYTE(msg+6);//0表示金币，1表示绑定金币

	if (byType == 0)
	{
		g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,2);//2表示修理物品窗口 
		if (byBind == 0)
		{
			g_pControl->MsgToWnd("NpcSaleWnd",MSG_CTRL_NPCSALEWND,10);
		}
		else if (byBind == 1)
		{
			g_pControl->MsgToWnd("NpcSaleWnd",MSG_CTRL_NPCSALEWND,11);
		}		
	}
	else if(byType == 1)
		g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,7);//7表示鉴定物品窗口 
}

void CGameControl::MSG_Exchange_Forge_Window(const char * msg,int iLen)
{//0265
	if(*(msg+6) == 0x01)
	{
		g_NPC.GetGood().SetID(0);
		g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,4);//4表示锻造物品窗口 
	}
	else if(*(msg+6)==0x02)
	{
		g_NPC.GetGood().SetID(0); 
		g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,5);//5祝福 
	}
}


void  CGameControl::MSG_Exchange_Repair_Money(const char * msg,int iLen)
{
	DWORD iMoney = 0;	
	iMoney = *(DWORD*)msg;
	

	BYTE byRepairType = *((BYTE *)(msg+6));
    g_NPC.SetSaleGold(iMoney);

	BYTE byBind = Conv_BYTE(msg+7);//0表示金币，1表示绑定金币

    if(byRepairType == 3)
    {
        CGood& temp = g_NPC.GetDamageGood();
        char str[256]={0};
        sprintf(str,"你的%s已经破碎了。运用%d点五色晶石的力量就可将其修复如新，\n你愿意修复吗？",temp.GetName(),iMoney);        
        g_MsgBoxMgr.PopSimpleComfirm(str,MSG_CTRL_REPAIR_DAMAGED_EQUIPMENT,0);
        return;
    }

	if(byRepairType == 11 || byRepairType == 12)//是通过图形化g_NPC窗口进行修理
	{
		g_NPC.SetExchangeFromQuickNpcWnd(false);
		if(iMoney == -1)
		{
			g_TalkMgr.PushSysTalk("该物品不能在此修理");			
			g_NPC.GetGood().SetID(0);
		}
		else if(iMoney == 0)
		{
			g_TalkMgr.PushSysTalk("该物品不需要修理");			
			g_NPC.GetGood().SetID(0);
		}
		else
		{
			char strTemp[256];
			if(byRepairType == 12)
				sprintf(strTemp,"您将要以<color=green %u>的价格特殊修理<color=green %s>，您确定要修吗？",iMoney,g_NPC.GetGood().GetName());
			else 
				sprintf(strTemp,"您将要以<color=green %u>的价格修理<color=green %s>，您确定要修吗？",iMoney,g_NPC.GetGood().GetName());

			g_MsgBoxMgr.PopTagOkCancelBox(strTemp,MSG_CTRL_QUICKNPC_REPAIR_OK,byBind,NULL,MSG_CTRL_QUICKNPC_REPAIR_CANCEL);
		}
	}
}

void  CGameControl::MSG_Exchange_Repair_Success(const char * msg,int iLen)
{
	WORD wDura = Conv_WORD(msg + 6);
	WORD wDuraMax = Conv_WORD(msg + 8);
	BYTE byBind = Conv_BYTE(msg+10);//0表示金币，1表示绑定金币

	CGood& tmp = g_NPC.GetGood();
	tmp.SetDura(wDura);
	tmp.SetDuraMax(wDuraMax);

	if(!g_NPC.IsExchangeFromQuickNpcWnd())
	{
		g_NPC.BackToPackage();
	}
	else
	{
		CGood* pGood = SELF.PackageGood().FindGood(tmp.GetID());
		if(pGood)
		{
			pGood->SetDura(wDura);
			pGood->SetDuraMax(wDuraMax);
		}
		g_NPC.SetExchangeFromQuickNpcWnd(false);
	}

	if (byBind == 0)
	{
		SELF.SetGold(*(DWORD*)msg);
	}
	else if (byBind == 1)
	{
		SELF.SetBindGold(*(DWORD*)msg);
	}
	
	tmp.SetID(0);
	g_NPC.SetSaleGold(0);
}

void CGameControl::MSG_Exchange_Repair_Duration(const char * msg,int iLen)
{
	DWORD dwGoodID = Conv_DWORD(msg);

	CGood* pGood = SELF.EquipGood().FindGood(dwGoodID);
	if(pGood)
	{
		pGood->SetDura(Conv_WORD(msg + 6));
		pGood->SetDuraMax(Conv_WORD(msg + 8));
	}
}
//byType = 0 修理； = 1 鉴定
void  CGameControl::MSG_Exchange_Repair_Fail(const char * msg,int iLen)
{
	char byType = msg[10];
	if (byType == 0 || byType == 1)
	{
		if(byType == 0)
			g_MsgBoxMgr.PopSimpleAlert("你不能修理这个物品！");
		else
			g_MsgBoxMgr.PopSimpleAlert("你不能鉴定这个物品！");

		if(g_NPC.IsExchangeFromQuickNpcWnd())
		{
			g_NPC.GetGood().SetID(0);
			g_NPC.SetExchangeFromQuickNpcWnd(false);
		}
	}

	g_NPC.BackToPackage();

	if(g_NPC.GetGood().GetID() != 0)
	{
		CGood *pGood = SELF.PackageGood().FindGood(0);
		if(pGood)
		{
			pGood->Assign(g_NPC.GetGood());
			g_NPC.GetGood().SetID(0);
			g_NPC.SetSaleGold(0);
		}
	}
}

void CGameControl::MSG_Object_Take(const char * msg,int iLen)
{
	DWORD id = *((DWORD *)(msg + 66));

	//托管包裹的物品的收回
	BYTE type = *(BYTE*)(msg + 8);
	CGood* pGood = 0;
	if (type > 0)
	{
// 		for (int i = 0; i < MAX_TRUSTEESHIP_NUM; ++i)
// 		{
// 			pGood = g_TrusteeshipData.GetMemberPackage(i).FindGood(id);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 
// 			pGood = SELF.GetUsingTemp().FindGood(id);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 
// 			if (id == CGoodGrid::GetDropGoodFrom().DropGood.GetID())
// 			{
// 				pGood = &(CGoodGrid::GetDropGoodFrom().DropGood);
// 			}
// 		}
// 		if (pGood == NULL)
// 		{
// 			return;
// 		}
// 		pGood->SetID(0);
// 		g_pGameData->DelExternString(id);
		return;
	}

	//////////////////////////看看是不是领养自别人的豹子被收回去了
	map<DWORD,PET_ADOPT> & MyAdopMap = g_PetData.GetMyAdoptMap();
	map<DWORD,PET_ADOPT>::iterator itr;
	if( ( itr = MyAdopMap.find(id) ) != MyAdopMap.end())
	{
		MyAdopMap.erase(itr);
	}

	g_BoothData.CleanItem(id);

	g_pGameData->TakeoutGoodByID(id);
	g_AutoPickMgr.RemoveObject(id);
}


void CGameControl::MSG_Bad_Good(const char * msg,int iLen)
{
	DWORD dwGoodID = Conv_DWORD(msg);

	//如果是群英包裹里的假物品
	BYTE type = *(BYTE*)(msg + 8);
	CGood* pGood = 0;
	if (type > 0)
	{
// 		for (int i = 0; i < MAX_TRUSTEESHIP_NUM; ++i)
// 		{
// 			pGood = g_TrusteeshipData.GetMemberPackage(i).FindGood(dwGoodID);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 			pGood = SELF.GetUsingTemp().FindGood(dwGoodID);
// 			if (pGood != NULL)
// 			{
// 				break;
// 			}
// 
// 			if (dwGoodID == CGoodGrid::GetDropGoodFrom().DropGood.GetID())
// 			{
// 				pGood = &(CGoodGrid::GetDropGoodFrom().DropGood);
// 			}
// 		}
// 		if (pGood == NULL)
// 		{
// 			return;
// 		}
// 		pGood->SetID(0);
// 		g_pGameData->DelExternString(dwGoodID);
		return;
	}

	if(msg[6] & 0x01) //清除托盘
	{
		if(dwGoodID == g_NPC.GetGood().GetID())
		{
			g_NPC.GetGood().SetID(0);
		}
	}

	if(dwGoodID == 0)
		return;

	g_pGameData->TakeoutGoodByID(dwGoodID); //清除物品
	g_AutoPickMgr.RemoveObject(dwGoodID);
    g_AutoEatMgr.BadBood(dwGoodID);
}


void CGameControl::MSG_Exchange_Forge_Percent(const char * msg,int iLen)
{
	g_NPC.SetSaleGold(0);
	g_NPC.SetStart(false);

	std::string str;

	switch(*(DWORD *)msg)
	{
	case 0xFFFFFFFF://
		str = "你放入的物品不匹配！";
		break;
	case 0xFFFFFFFE:
		str = "你的经验值不够！";
		break;
	case 0xFFFFFFFD:
		str = "你的武器持久力不够！";
		break;
	case 0xFFFFFFFC:
		str = "你还没有学会该技能！";
		break;
	default:
		g_NPC.SetForgeExp(Conv_DWORD(msg));
		return;
	}

	if(str.size() != 0)
	{
		g_MsgBoxMgr.PopSimpleAlert(str.c_str());
	}

	g_NPC.BackToPackage();
	g_NPC.SetSaleGold(0);
}

void CGameControl::MSG_Exchange_Forge_Result(const char * msg,int iLen)
{
	DWORD dwResult = Conv_DWORD(msg);

	if(g_NPC.GetGood().GetID() == 0)
		return;

	g_NPC.BackToPackage();
	g_NPC.SetSaleGold(0);

	string str;
	if(dwResult = 0)
		str = "您的武器锻造成功！";
	else
		str = "您的武器锻造失败！";

	g_MsgBoxMgr.PopSimpleAlert(str.c_str());
}

//////////祝福
void CGameControl::MSG_Exchange_Bless_Result(const char * msg,int iLen)
{//0025
	DWORD dwResult = Conv_DWORD(msg);

	if(g_NPC.GetGood().GetID() == 0)
		return;

	g_NPC.BackToPackage();
	g_NPC.SetSaleGold(0);

	string str;
	if(dwResult == 0)
		str = "您的盔甲祝福成功！";
	else
		str = "您的盔甲祝福失败！";

	g_MsgBoxMgr.PopSimpleAlert(str.c_str());
}


void CGameControl::MSG_Exchange_Forge_Experience(const char * msg,int iLen)
{
	UINT64 exp = Conv_INT64(msg + 12);
	UINT64 expup = Conv_INT64(msg + 20);

	if(SELF.GetLevelUpExp() > 0)
	{
		double fBefore,fFinish;
		fBefore = ((double)SELF.GetExp() / SELF.GetLevelUpExp());
		fFinish = (double)exp/SELF.GetLevelUpExp();

		if((int)(fFinish *100) - (int)(fBefore * 100) < 0)
			g_OtherData.SetExpUpTime(GetTickCount());
	}

	SELF.SetExp(exp);	
}



void CGameControl::SEND_Exchange_Repair(DWORD npcid,DWORD weaponID,const char * strName,BYTE byRepairType,BYTE byUseBind)
{
	SET_COMMAND(CS_EXCHANGE_REPAIR,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,weaponID);
	ADD_STR(strName);

	ASSIGN_BYTE(10,byRepairType);
	ASSIGN_BYTE(11,byUseBind);

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Sale_Takein(DWORD npcid,DWORD goodID,const char * strName)
{
	SET_COMMAND(CS_EXCHANGE_SALE_TAKEIN,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}
//bytype:0老版本，1新版图形化存仓
void CGameControl::SEND_Exchange_Storage_Ok(DWORD npcid,DWORD goodID,const char * strName,BYTE byType)
{
	SET_COMMAND(CS_EXCHANGE_STORAGE_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
	ASSIGN_BYTE(10,byType);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}
//bytype :0老版本，1：新的图形化仓库取物
void CGameControl::SEND_Exchange_Getback_Ok(DWORD npcid,DWORD goodID,const char * strName,BYTE byType)
{
	SET_COMMAND(CS_EXCHANGE_GETBACK_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
	ASSIGN_BYTE(10,byType);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Sale_Ok(DWORD npcid,DWORD goodID,const char * strName)
{
	SET_COMMAND(CS_EXCHANGE_SALE_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Repair_Ok(DWORD npcid,DWORD goodID,const char * strName,BYTE byType,BYTE byUseBind)
{
	SET_COMMAND(CS_EXCHANGE_REPAIR_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
    ASSIGN_BYTE(10,byType);
	ASSIGN_BYTE(11,byUseBind);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Buy_Ok(DWORD npcid,DWORD goodID,const char * strName,BYTE byType)
{
	SET_COMMAND(CS_EXCHANGE_BUY_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_DWORD(6,goodID);
	ASSIGN_BYTE(10,byType);
	ADD_STR(strName);
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_EXCHANGE_BUY_OK, npcid, 0, 0, 0);
	//g_pNet->SendPacket(&DefMsg, (char*)strName);
}

void CGameControl::SEND_Exchange_Poison_Ok(DWORD npcid,const char * strName)
{
	SET_COMMAND(CS_EXCHANGE_POISON_OK,256);
	ASSIGN_ID(npcid);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Buy_Kind_Ok(DWORD npcid,const char * strName,BYTE byType)
{
	SET_COMMAND(CS_EXCHANGE_BUY_KIND_OK,256);
	ASSIGN_ID(npcid);
	ASSIGN_BYTE(10,byType);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}


void CGameControl::SEND_SmallGem_Condense_Request(int i1,int i2,int i3)
{
	SET_COMMAND(CS_SMALLGEM_CONDENSE_REQUEST,256);
	ASSIGN_BYTE(6,i1);
	ASSIGN_BYTE(7,i2);
	ASSIGN_BYTE(8,i3);
	int iOreCount = 0;

	ADD_DWORD(g_NPC.GetOreGood(0).GetID());

	// 1--14  表示矿石 0 表示幸运符
	for(int i = 1;i<MAX_ORE_COUNT;i++)
	{
		DWORD id = g_NPC.GetOreGood(i).GetID();
		if(id > 0)
		{
			ADD_DWORD(id);
			iOreCount++;
		}
	}
	temp[10] = iOreCount;
	SEND_GAME_SERVER();
}

void CGameControl::SEND_SmallGem_Count_Request()
{
	SET_COMMAND(CS_SMALL_GEM_REQUEST,12);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_SmallGem_Count(const char* msg,int iLen)
{
	if(iLen >12)
		return;
	int iRed = 0;
	int iGreen = 0;
	int iBlue = 0;
	// 659 660 661
	for(int i=0;i<MAX_PACKAGE_ELEMENT;i++)
	{
		if(SELF.GetPackageGood(i).GetID() == 0)
			continue;

		int iLook = SELF.GetPackageGood(i).GetLooks();
		if(iLook == 659)
			iRed++;
		else if(iLook == 660)
			iGreen++;
		else if(iLook == 661)
			iBlue++;
	}
	SELF.SetReserveData(plyRedGemCount,(unsigned char)msg[6] + iRed);
	SELF.SetReserveData(plyBlueGemCount,(unsigned char)msg[7] + iBlue);
	SELF.SetReserveData(plyGreenGemCount,(unsigned char)msg[8] + iGreen);
}

void CGameControl::SEND_SndaMark_Request()
{
	SET_COMMAND(CS_SNDAMARK_REQUEST,12);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_SndaMark_Answer(const char* msg,int iLen)
{
	if(iLen >12)
		return;

	DWORD ul = *((DWORD *)msg) ;
	SELF.SetSndaMark(ul);

	//std::string str = StringUtil::format("您目前可用积分为%u，让您的积分给您带来更多的惊喜，请登陆http://jf.sdo.com了解详情",ul);
	//g_TalkMgr.PushSysTalk(str.c_str());
}

void CGameControl::Get_Guild_Logo(DWORD npcid,const char * act)
{
	//act 为logo  ID
	static char s_str[256]={0};
	int len = sizeof(act);
	DWORD dwTexID=0;
	char *pChar = (char *)act;

	if(len>8) *(pChar+8)=0;
	for(int i=0;i<8 &&*pChar;++i,++pChar )
	{
		if(*pChar>='0'&& *pChar<='9')
		{
			dwTexID=(dwTexID<<4 | ((*pChar-'0')&0x0F) );
		}
		else if(*pChar>='a'&& *pChar<='f') 
		{
			dwTexID=(dwTexID<<4 | (((*pChar-'a')&0x0F) + 10) );
		}
		else if(*pChar>='A'&& *pChar<='F')
		{
			dwTexID=(dwTexID<<4 | (((*pChar-'A')&0x0F) + 10 ) );
		}
		else//不是有效字符
		{
			g_NPC.SetChangeLogo(false);
			return ;
		}
	}
}

void CGameControl::SEND_Exchange_Menu_Click(DWORD npcid,const char * act,int type,DWORD texID ,WORD x,WORD y)
{
	SET_COMMAND(CS_EXCHANGE_MENU_CLICK,256);
	ASSIGN_ID(npcid);

	if(type == 0)
	{
		ADD_STR(act);
		SEND_GAME_SERVER();		

		//fnMakeDefMessage(&DefMsg, CS_EXCHANGE_MENU_CLICK, npcid, 0, 0, 0);
		//g_pNet->SendPacket(&DefMsg, (char*)act);
	}
	else if(type == 1)
	{
		ADD_STR(act);
		char flag[32] = "";
		sprintf(flag,"&%d",texID);
		ADD_STR(flag);
		
		SEND_GAME_SERVER();
	}
	else//图片坐标
	{
		temp[6]=0x01;
		ADD_DWORD(texID);
		ADD_WORD(x);
		ADD_WORD(y);

		SEND_GAME_SERVER();
	}
}

void CGameControl::SEND_Exchange_Goin(DWORD npcid)
{
	SET_COMMAND(CS_EXCHANGE_GOIN,12);
	ASSIGN_ID(npcid);
	SEND_GAME_SERVER();

	//fnMakeDefMessage(&DefMsg, CS_EXCHANGE_GOIN, npcid, 0, 0, 0);
	//g_pNet->SendPacket(&DefMsg, NULL);
}

void CGameControl::SEND_Exchange_Get_Tex(DWORD npcid,DWORD TexID)
{
	SET_COMMAND(CS_EXCHANGE_GET_TEX,16);
	ASSIGN_ID(npcid);
	ASSIGN_BYTE(10,0x01);
	ADD_DWORD(TexID);

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Forge_Ok(DWORD npcid,DWORD goodID,const char * strName,BYTE type)
{
	SET_COMMAND(CS_EXCHANGE_FORGE_OK,12);
	ASSIGN_ID(npcid);
	ASSIGN_BYTE(11,type);
	ASSIGN_DWORD(6,goodID);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Exchange_Forge(DWORD npcid,DWORD goodID,const char * strName,BYTE type)
{
	SET_COMMAND(CS_EXCHANGE_FORGE,256);
	ASSIGN_ID(npcid);

	temp[11]=type;
	memcpy(temp+6,&goodID,4);
	ADD_STR(strName);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Exchange_Tray_Wnd(const char * msg,int iLen)
{
	char byType = msg[10];

	g_NPC.GetGood().SetID(0);
	if(iLen > 12)
	{
		char str[512] = {0};
		memcpy(str,msg+12,iLen - 12);
		g_NPC.SetTrayTitle(str);
	}
	else
		g_NPC.SetTrayTitle("");

	char byType1 = msg[6];
	if (byType1 == 1)
	{
		//g_pControl->PopupWindow(MSG_CTRL_ZHENPUCUILIAN_WND,OPER_RECREATE);
	}
	else
	{
		if(byType == 0)
		{
			g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,6);//6表示弹出托盘
		}
		else
		{
			g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CREATE,8);//8不关闭的弹出托盘
		}
	}	
}
void CGameControl::MSG_Return_Result(const char * msg,int iLen)
{
	BYTE cType,cFlag;

	memcpy(&cType,msg,1);
	memcpy(&cFlag,msg + 6,1);

	string buf;
	buf.assign(msg+CMD_SIZE,iLen-12);

	switch(cType)
	{
	case 1:				// 圣诞节新加的
		{
			CGood& tmp = g_NPC.GetGood();

			if(cFlag == 0xFF)		// 提交物品失败
			{
				if(iLen > 12)
				{
					g_MsgBoxMgr.PopSimpleAlert(buf.c_str());
				}

				g_NPC.BackToPackage();
				g_NPC.SetSaleGold(0);
			}
			else
			{
				// 提交物品成功
				tmp.SetID(0);
			}
			break;
		}
	default:
		{
			if(cFlag == 0xFF)		// 失败提示
			{
				g_MsgBoxMgr.PopSimpleAlert(buf.c_str());
			}
		}
	}
}
void CGameControl::SEND_Exchange_Submit(DWORD dwNpcID,DWORD dwGoodID)
{
	SET_COMMAND(CS_EXCHANGE_SUBMIT,256);

	memcpy(temp,&dwNpcID,sizeof(DWORD));
	memcpy(temp + 6,&dwGoodID,sizeof(DWORD));

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Message_Submit(DWORD dwNpcID,BYTE cType,BYTE cFlag,char *sText)
{
	int iLen = strlen(sText);
	if( iLen >= 8192)
		return;

	SET_COMMAND(CS_MESSAGE_SUBMIT,8204);

	memcpy(temp,&dwNpcID,4);
	memcpy(temp+6,&cFlag,1);
	memcpy(temp+8,&cType,1);
	ADD_STR(sText);

	SEND_GAME_SERVER();
}

//12字节以后:提示内容+'\0'+输入框默认内容
void CGameControl::MSG_Npc_Msgbox(const char * msg,int iLen)
{
	if(iLen > 1024)
		return;
	char sTemp[1024] = {0};
	BYTE cDigital;
	BYTE cLen;
	BYTE cNonNpc;
	bool bModel = true;

	memcpy(sTemp,msg + 12,iLen - 12);
	memcpy(&cDigital,msg + 6,1);
	memcpy(&cLen,msg + 7,1);
	memcpy(&cNonNpc,msg + 10,1);
	if(msg[8])
		bModel = false;

	bool bForbidDirtyWord = false;
	if (msg[9] == 1)//要限制
	{
		bForbidDirtyWord = true;
	}

	DWORD dwCallbackMsg = MSG_CTRL_MSGBOX_SERVER;
	if(cNonNpc)
		dwCallbackMsg = MSG_CTRL_MSGBOX_SERVER_NOTNPC;

	if(iLen > 12 + strlen(sTemp) + 1)
	{
		g_MsgBoxMgr.PopEditBox(sTemp,dwCallbackMsg,0,0,cDigital==1,(int)cLen,0,0,NULL,bModel,sTemp + strlen(sTemp) + 1,bForbidDirtyWord);
	}
	else
	{
		g_MsgBoxMgr.PopEditBox(sTemp,dwCallbackMsg,0,0,cDigital==1,(int)cLen,0,0,NULL,bModel,NULL,bForbidDirtyWord);
	}

}

void CGameControl::SEND_Npc_Msgbox(DWORD npcid,char *sBody)
{
	SET_COMMAND(CS_NPC_MSGBOX,256);
	ASSIGN_ID(npcid);

	ADD_STR(sBody);

	SEND_GAME_SERVER();
}


//iType == 7 请求装备升级,9:合成衣服,10:升魂珠,11合原石
void CGameControl::SEND_Exchange_Gem_Ok(int iType,bool bTest)
{
	DWORD dwNpcId = g_NPC.GetID();

	SET_COMMAND(CS_EXCHANGE_GEM_OK,256);
	if(iType == 2 || iType == 5)
	{
		ASSIGN_ID(SELF.GetID());
	}
	else
	{
		ASSIGN_ID(dwNpcId);
	}

	if(iType == 7 || iType == 9 || iType == 10)
	{
		ASSIGN_WORD(6,1);
		ASSIGN_WORD(10,iType);
		ASSIGN_BYTE(8,bTest);
		ASSIGN_DWORD(12,g_NPC.GetEquLUpGood(6).GetID());

		int iGoodCount = 1;
		iBLen += 4;
		for (int i=0;i< 6 ;i++)
		{
			DWORD dwGoodID = g_NPC.GetEquLUpGood(i).GetID();
			if (dwGoodID == 0)
				continue;

			ADD_DWORD(dwGoodID);
			iGoodCount++;
		}

		ASSIGN_WORD(6,iGoodCount);	
		SEND_GAME_SERVER();
		return;
	}
	else
	{
		int iGemCount = 0;
		for (int i=0;i<MAX_COMMON_GOOD_COUNT;i++)
		{
			DWORD dwGoodID = g_NPC.GetGemGood(i).GetID();
			if (dwGoodID == 0)
				continue;

			ADD_DWORD(dwGoodID);
			iGemCount++;
		}

		if (iGemCount == 0 && iType>=0 && iType<=1)//没有宝石不发消息
			return;

		ASSIGN_WORD(6,iGemCount);
		ASSIGN_WORD(10,iType);
		ASSIGN_BYTE(8,bTest?1:0);

		if(iType == 8) *((BYTE*)(temp+9)) = (BYTE)g_pGameData->GetSuperArmCondenseType();
		SEND_GAME_SERVER();
	}
}

//cType =0 1 凝练 合成   
//cType =3 矿石合成武器  (宝石 + 高纯度矿石 + 幸运符)  
//ctype =4 超级武器合成  (宝石 + 战神/道神/魔神碎片 + 幸运符) 
//ctype =6 合成高一级装备(宝石 + 普通装备   + 幸运符)
//ctype =7 装备升级
//ctype =9 装备衣服
//ctype =10 升级魂珠
//wResult 0 合成失败 1合成成功
//cTest   0 测试阶段 1合成阶段
//cState  0 默认     1关闭窗口 2 延迟关闭窗口
void CGameControl::MSG_Exchange_Gem_Result(const char* msg,int iLen)
{

	BYTE cType = *(BYTE*)(msg+10);
	WORD wResult = *(WORD*)(msg+6);
	BYTE cTest = *(BYTE*)(msg+8);
	BYTE cState = *(BYTE*)(msg+9);

	if(cType == 3 && cTest == 0  && wResult == 0 && cState == 0)
	{
		if(g_NPC.IsCondensing())
		{
			g_MsgBoxMgr.PopSimpleAlert("\n\n对不起,系统正忙,请稍后再试。");
			return ;
		}

		g_pControl->PopupWindow(MSG_CTRL_COMMONCONDENSE_WND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_SMALL_GEM_CON_WND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_EQUIP_LEVEL_UP_WND,OPER_CLOSE);

		//双击炼制宝鼎
		//g_NPC.SetID(SELF.GetID());
		g_NPC.SetCondenseFrame(-1);
		g_NPC.SetIsCondensing(false);
		g_pControl->PopupWindow(MSG_CTRL_GEMCONDENSE_WND,OPER_RECREATE,3);
		return;
	}
	else if(cType == 6 && cTest == 0 && wResult == 0 && cState == 0 )
	{
		if(g_NPC.IsCondensing())
		{
			g_MsgBoxMgr.PopSimpleAlert("\n\n对不起,系统正忙,请稍后再试。");
			return ;
		}

		g_pControl->PopupWindow(MSG_CTRL_GEMCONDENSE_WND,OPER_CLOSE);		
		g_pControl->PopupWindow(MSG_CTRL_SMALL_GEM_CON_WND,OPER_CLOSE);
		g_pControl->PopupWindow(MSG_CTRL_EQUIP_LEVEL_UP_WND,OPER_CLOSE);

		//双击熔炼宝箱
		//g_NPC.SetID(SELF.GetID());
		g_NPC.SetIsLUpintEqu(false);
		g_NPC.SetCondenseFrame(-1);
		g_NPC.SetIsCondensing(false);
		g_pControl->PopupWindow(MSG_CTRL_COMMONCONDENSE_WND,OPER_CREATE,1);

		return;
	}

	//测试阶段物品的放置是否正确(第一阶段)
	if(cTest)
	{
		//测试物品放置成功播放特效
		if(wResult)
		{
			if(cType == 7 || cType == 9 || cType == 10)
				g_NPC.SetIsLUpintEqu(true);

			g_NPC.SetCondenseFrame(0);
			g_pAudio->Play(EAT_OTHER,200,g_pAudio->GetRand()++);
		}
		//测试物品放置失败
		else
		{
			g_NPC.SetCondenseFrame(-1);
			g_NPC.SetIsCondensing(false);
			g_NPC.SetIsLUpintEqu(false);
		}
	}
	//熔炼的结果(第二阶段)
	else
	{
		g_NPC.SetIsCondensing(false);
		g_NPC.SetIsLUpintEqu(false);
		g_NPC.SetCondenseFrame(-1);
		//熔炼或合成成功
		if(wResult)
		{
			if(cType == 6)
			{
				g_pControl->PopupWindow(MSG_CTRL_COMMONCONDENSE_WND,OPER_CLOSE);
			}
			else if(cType == 7 || cType == 9 || cType == 10)
			{
				if(wResult == 1)
				{
					g_pControl->Msg(MSG_CTRL_EQUIP_LEVEL_UP_WND,4);
				}
				else
				{
					g_pControl->Msg(MSG_CTRL_EQUIP_LEVEL_UP_WND,5);
				}
			}
			g_pAudio->Play(EAT_OTHER,201,g_pAudio->GetRand()++);
		}
		//熔炼或合成失败
		else
		{
			//延迟关闭窗口
			if(cType == 6 && cState == 2)
			{
				g_pControl->Msg(MSG_CTRL_COMMONCONDENSE_WND,6,NULL);
			}
			else if(cType == 7 || cType == 9 || cType == 10)
			{
				g_pControl->Msg(MSG_CTRL_EQUIP_LEVEL_UP_WND,6);
			}
			g_pAudio->Play(EAT_OTHER,202,g_pAudio->GetRand()++);	
		}
	}

}


//x=0 关闭窗口 x=1 真假物品 x=2 真物品
void CGameControl::MSG_Exchange_Box_Result(const char* msg,int iLen)
{
	BYTE cResult = msg[6];//开宝箱的阶段
	int  iCount  = *(WORD*)(msg+8);//物品数量
	int  iRealBoxType = *(WORD*)(msg+10);//箱子的类型 

	if(iRealBoxType && iRealBoxType>0 && iRealBoxType < 6)
	{
		g_NPC.SetBoxType(iRealBoxType);
	}

	int iBoxType = g_NPC.GetBoxType();

	switch(cResult) 
	{
	case 0:
		{
			//关闭窗口
			g_NPC.SetBoxOpenState(3);
			//g_pControl->MsgToWnd(MSG_CTRL_NEWTREASUREBOX_WND,MSG_CTRL_NEWTREASUREBOX_WND,999);
			//g_pControl->MsgToWnd(MSG_CTRL_ALLTREASUREBOX_WND,MSG_CTRL_ALLTREASUREBOX_WND,899);
		}
		break;
	case 255://强制关闭新宝箱
		{
			//g_pControl->PopupWindow(MSG_CTRL_NEWTREASUREBOX_WND,OPER_CLOSE);
			//g_pControl->PopupWindow(MSG_CTRL_ALLTREASUREBOX_WND,OPER_CLOSE);
		}
		break;
	case 1: //宝箱V1开启
	case 3: //新宝箱开启
	case 5: //新宝箱2开启
		{
			if(cResult == 1)
				g_NPC.SetBoxOpenState(0);
			else
				g_NPC.SetBoxOpenState(2);

			g_NPC.SetBoxOpenCycle(msg[7]);//表示第几轮

			if(iCount < 0 || (cResult == 1 && iCount > MAX_TREASUER_BOX_NUM) || (cResult != 1 && iCount > MAX_TOTAL_BOX_NUM))
				break;

			if(cResult == 1 || ((cResult == 3 || cResult == 5 )&& msg[7] == 1))//新宝箱如果不是第一回合，发过来的looks也不要，因为会冲掉物品的移动效果
			{
				for(int i=0;i < iCount;i++)
				{
					_TreasureData& data = g_NPC.GetBoxGood(i);

					if(cResult == 1)
					{
						const char* ch = msg+12+17*i;

						data.Clear();
						int iTempLooks = Conv_WORD(ch);
						BYTE cNameLength = ch[2];

						char szGoodName[20] = {0};
						if(cNameLength> 0 && cNameLength <=14)
						{
							memcpy(szGoodName,ch+3,cNameLength);
						}

						if(iTempLooks > 0)
						{
							data.Looks = iTempLooks;
							strcpy(data.strName,szGoodName);
							data.bTrue = false;
						}
					}
					else if(cResult == 3 || cResult == 5)
					{
						const char* ch = msg+12+18*i;

						data.Clear();

						int iTempLooks = *(WORD*)(ch);
						BYTE byGoodTyep = *(BYTE*)(ch+2);
						BYTE cNameLength = ch[3];

						char szGoodName[20] = {0};
						if(cNameLength> 0 && cNameLength <=14)
						{
							memcpy(szGoodName,ch+4,cNameLength);
						}

						if(iTempLooks > 0)
						{
							data.Looks = iTempLooks;
							data.byGoodTyep = byGoodTyep;
							strcpy(data.strName,szGoodName);
							data.bTrue = false;
						}
					}
				}
			}

			//打开宝箱窗口
			if(cResult == 3 )//新宝箱
			{
				//if(msg[7] == 1)//第一回合时开启界面
				//{
				//	g_pControl->PopupWindow(MSG_CTRL_ALLTREASUREBOX_WND,OPER_CREATE,iBoxType);
				//}
				//g_pControl->Msg(MSG_CTRL_ALLTREASUREBOX_WND,997);//设置本回合开始;
			}
			else if(cResult == 5 )//新宝箱2
			{
				//if(msg[7] == 1)//第一回合时开启界面
				//{
				//	g_pControl->PopupWindow(MSG_CTRL_ALLTREASUREBOX_WND,OPER_CREATE,iBoxType);
				//}
				//g_pControl->Msg(MSG_CTRL_ALLTREASUREBOX_WND,897);//设置本回合开始;
			}

			g_pAudio->Play(EAT_OTHER,220,g_pAudio->GetRand()++);

		}
		break;
	case 2://老宝箱开启完毕
		{
			g_NPC.SetBoxOpenState(1);

			if(iCount < 0 || iCount > MAX_TREASUER_BOX_NUM)
				break;

			for(int i=0;i<iCount;i++)
			{
				int iTempLooks = *(WORD*)(msg+12+2*i);

				for(int j=0;j< MAX_TREASUER_BOX_NUM;j++)
				{
					_TreasureData& data = g_NPC.GetBoxGood(j);
					if(data.Looks == iTempLooks && !data.bTrue)
					{
						data.bTrue = true;
						break;
					}
				}
			}

			g_pAudio->Play(EAT_OTHER,221,g_pAudio->GetRand()++);

			break;
		}
	case 4: //新宝箱子开启完毕
	case 6: //新宝箱2开启完毕
		{
			g_NPC.SetBoxOpenState(1);

			int iCycle = msg[7]; //表示第几轮

			if(iCycle < 0 || iCycle > 4)
				break;

			int iTempLooks = *(WORD*)(msg+12);
			string name;
			if(iLen > 14)
			{
				name.assign(msg + 14,iLen - 14);
			}

			for(int j=0;j< MAX_TREASUER_BOX_NUM;j++)
			{
				_TreasureData& data = g_NPC.GetBoxGood(j);

				if(data.Looks == iTempLooks && !data.bTrue)
				{ 
					if(name.size() > 0 && strcmp(name.c_str(),""))
					{
						if(strcmp(name.c_str(),data.strName) == 0)
						{
							data.bTrue = true;
							break;
						}						
					}
					else
					{
						data.bTrue = true;
						break;
					}					
				}
			}

			//if(cResult == 4)
			//	g_pControl->Msg(MSG_CTRL_NEWTREASUREBOX_WND,998);//设置本回合结束;
			//else if(cResult == 6)
			//	g_pControl->Msg(MSG_CTRL_ALLTREASUREBOX_WND,898);//设置本回合结束;
			break;
		}
	case 12://祈愿树
		{
			BYTE cType = msg[7];
			WORD wValue = Conv_WORD(msg+8);
			if(cType == 0)
			{
				//弹出界面
				g_NPC.GetGood().SetID(0);
				g_pControl->PopupWindow(MSG_CTRL_PRAYTREE_WND,OPER_CREATE);
			}
			else if(cType == 1)//祈愿树奖励等级，力量条信息
			{
				//力量条等级为 wValue//为0时表示不可放，其它表示力量条的等级
				g_NPC.SetPrayTreeLevel(msg[8]);				
			}
			else if(cType == 2)//获得的奖励
			{
				int iStrLen = msg[8];
				string& strPrize = g_NPC.GetPrayTreePrize();
				strPrize.clear();
				strPrize.assign(msg + 12,iStrLen);
				char temp[128] = {0};
				sprintf(temp,"恭喜你从祈愿树上获得了%s，",strPrize.c_str());
				strPrize = temp;
				g_NPC.GetGood().SetID(0);
				//g_NPC.SetPrayTreeLevel(0xFF);				
			}	
			else if(cType == 3)//祈愿树的奖励列表
			{
				int iDev = 12;
				int iStrLen = 0;
				//列表长度为wValue
				for(int i = 0;i < 3;i++)
				{
					VTreeGift* pVec = g_NPC.GetGiftsByTreeLevel(i + 1);
					if(pVec == NULL) break;
					pVec->clear();
					int iLen = msg[8+i];
					for(int j = 0;j<iLen;j++)
					{
						//装填数据
						_TreeGift treegift;
						treegift.wLooks = Conv_WORD(msg + iDev);
						iDev += 2;
						iStrLen = msg[iDev];
						iDev++;
						treegift.strName.assign(msg + iDev,iStrLen);   
						iDev += 14;
						pVec->push_back(treegift);
					}
				}				
			}
			else if(cType == 4)
			{
				int iStrLen = 0;
				BYTE cType = msg[8];
				int iDev = 12;
				if(cType >= 1 && cType <= 3)
				{
					//增加数据
					VTreeGift* pVec = g_NPC.GetGiftsByTreeLevel(cType);
					_TreeGift treegift;
					treegift.wLooks = *((WORD*)(msg + iDev));
					iDev += 2;
					iStrLen = msg[iDev];
					iDev++;
					treegift.strName.assign(msg + iDev,iStrLen); 
					iDev += 14;
					pVec->push_back(treegift);
				}
				else
				{
					return;
				}

				cType = msg[9];				
				if(cType >= 1 && cType <= 3)
				{
					//删除数据
					iDev += 2;
					iStrLen = msg[iDev];
					iDev++;
					string strName;
					strName.assign(msg + iDev,iStrLen);

					VTreeGift* pVec = g_NPC.GetGiftsByTreeLevel(cType);

					VTreeGift::iterator itr = pVec->begin();
					for(;itr != pVec->end();itr++)
					{
						if(itr->strName.compare(strName) == 0)
						{
							pVec->erase(itr);
							break;
						}
					}

				}
			}
		}
		break;
	case 13:
		{
			if(msg[7] == 4)
			{
				g_pControl->PopupWindow(MSG_CTRL_WUXING_PRIZE_WND,OPER_CLOSE);	//关闭发奖窗口
                g_pControl->PopupWindow(MSG_CTRL_WUXING_PRIZE_WND,OPER_RECREATE,2);	//打开询问窗口
				return;
			}

			g_NPC.GetWuXingResult().dwTime = Conv_WORD(msg);
			g_NPC.GetWuXingResult().bySuccess = msg[7];
			g_NPC.GetWuXingResult().dwMonster = Conv_WORD(msg + 8);
			if(g_NPC.GetWuXingResult().dwMonster > 20)
			{
				g_NPC.GetWuXingResult().dwMonster = 20;		//逃脱怪物最多为20个
			}
			g_NPC.GetWuXingResult().strPrize.clear();

			char temp[512] = {0};
			memcpy(temp,msg + 12,iLen - 12);
			int iStart = 0;
			
			while(iStart < iLen - 12)
			{		
				string strTmp = StringUtil::toStr(temp, iStart);
				if(!strTmp.empty())	//奖品名字
				{
					g_NPC.GetWuXingResult().strPrize.push_back(strTmp);	
				}
			}

			g_NPC.GetWuXingResult().bGameOver = true;
			g_NPC.GetWuXingResult().dwOverTime = GetTickCount();

            if(g_NPC.GetWuXingResult().bySuccess == 0)		//防守失败9个召唤点放紫烟特效
            {
                DWORD pos[9] ={0};
                pos[0] = MAKELONG(18,48-6);
                pos[1] = MAKELONG(23,48-6);
                pos[2] = MAKELONG(26,40-6);
                pos[3] = MAKELONG(23,32-6);
                pos[4] = MAKELONG(27,23-6);
                pos[5] = MAKELONG(35,23-6);
                pos[6] = MAKELONG(35,31-6);
                pos[7] = MAKELONG(27,31-6);
                pos[8] = MAKELONG(31,40-6);

                for(int i = 0;i < 9;i++)
                {
                    g_pMagicCtrl->CreateMagic(MAGICID_TMBZ_ZY,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,pos[i]);//固定坐标                    
                }
            }
		}
		break;

	default:
		break;
	}
}

void CGameControl::MSG_Exchange_List_Menu(const char* msg,int iLen)
{
TRY_BEGIN
	if(iLen<=12)
		return;

	char* p = NULL;
	p = new char[iLen-11];
	memset(p,0,iLen-11);
	memcpy(p,msg+12,iLen-12);	

	g_NPC.GetVectorStrData().clear();

	char* seps = "\n\r\\";
	char* temp = strtok(p, seps);

	if(temp)
	{
		g_NPC.SetTitleStrData(temp);
		temp = strtok(NULL,seps);
	}
	else
		g_NPC.SetTitleStrData("");

	while(temp != NULL )
	{
		if(strlen(temp)>0)
			g_NPC.GetVectorStrData().push_back(temp);

		temp = strtok(NULL, seps );
	}
	SAFE_DELETE_ARRAY(p);

	if(g_NPC.GetVectorStrCount()>0 && !g_NPC.GetTitleStrData().empty())
	{
		g_pControl->PopupWindow(MSG_CTRL_NPCLISTWND,OPER_CREATE,1);
	}
	else
	{
		g_pControl->PopupWindow(MSG_CTRL_NPCLISTWND,OPER_CLOSE);
	}
TRY_END
}

void CGameControl::MSG_Package_Capacity(const char * msg,int iLen)
{
	int iMaxBoothGoodNum = Conv_WORD(msg + 10);
	if(iMaxBoothGoodNum > g_BoothData.GetMaxGoodNum())
	{
		g_BoothData.SetMaxGoodNum(iMaxBoothGoodNum);
	}

	int iOld = MAX_PACKAGE_ELEMENT;
	MAX_PACKAGE_ELEMENT = *((WORD*)(msg + 8));
	MAX_PACKAGE_NUM = MAX_PACKAGE_ELEMENT - 6;

	bool bWndOpen = false;
	if(g_pControl->FindWindowByName("PackageWnd"))
	{
		bWndOpen = true;
	}
	
	g_pControl->Msg(MSG_CTRL_PACKAGEWND,OPER_CLOSE);

	if(MAX_PACKAGE_ELEMENT != iOld)
	{
		SELF.ResizePackageCapacity(iOld);
	}

	if(bWndOpen)
	{
		g_pControl->Msg(MSG_CTRL_PACKAGEWND,OPER_CREATE);
	}
}

void CGameControl::SEND_Person_Setting()
{
	SET_COMMAND(CS_PERSON_STATE,256);

	//第0位表示夫妻
	//第1位表示师徒
	//第2位表示沙城，3表示等级封号 4表示翅膀 5-表示临时封号,6表示第二元神闪光。
	WORD tt = 0;
	if(!g_TempConfig.GetHusbandAndWife())
		tt = tt | 0x0001;
	if(!g_TempConfig.GetShaMember())
		tt = tt | 0x0004;
	if(!g_TempConfig.GetOpenWing())
		tt = tt | 0x0010;
	if(!g_TempConfig.GetTempMask())
		tt = tt | 0x0020;
	if(!g_TempConfig.GetOpenShine())
		tt = tt | 0x0040;

	*(WORD*)(temp + 8) = tt;

	strncpy(temp+12,g_TempConfig.GetTempMaskStr(),20);
	strncpy(temp+32,g_TempConfig.GetTitleStr(),80);

	iBLen = 112;
	SEND_GAME_SERVER();
}

void CGameControl::MSG_Person_State(const char * msg,int iLen)
{
	if( iLen > 40960 )
		fatal_error("网络数据错误！");

	if(iLen < 12)
		return ;

	DWORD id = *(DWORD*)(msg);
	WORD wType = *(WORD*)(msg+ 6);
	WORD tt = *(WORD*)(msg+ 8);

	if(wType == 1) // 个人开关和个人签名
	{
		char  temp[1024] = {0};
		memcpy_s(temp,1023, msg+12,iLen-12 > 1023 ? 1023 : iLen-12);
		g_Config.SetTitleStr(temp);

		g_Config.SetHusbandAndWife((tt & 0x0001) == 0);  // 夫妻关系
		g_Config.SetShaMember((tt & 0x0004) == 0);  // 沙城关系
		g_Config.SetOpenWing((tt& 0x0010) == 0);  // 翅膀显示
		g_Config.SetTempMask((tt & 0x0020) == 0);  // 临时封号
		g_Config.SetOpenShine((tt & 0x0040) == 0);//第二元神闪光
	}
	else if(wType == 2) // 临时封号
	{
		char temp[1024]={0};
		memcpy_s(temp,1023, msg+12,iLen-12 > 1023 ? 1023 : iLen-12);
		g_Config.SetTempMaskStr(temp);
	}
	else if(wType == 3)
	{
		if(tt == 1)
		{
			g_Config.SetTitleStr(g_TempConfig.GetTitleStr());
			g_Config.SetHusbandAndWife(g_TempConfig.GetHusbandAndWife());
			g_Config.SetOpenWing(g_TempConfig.GetOpenWing());
			g_Config.SetTempMask(g_TempConfig.GetTempMask());
			g_Config.SetShaMember(g_TempConfig.GetShaMember());
			g_Config.SetOpenShine(g_TempConfig.GetOpenShine());

			if(g_Config.GetTempMask())
			{
				vector<string> vec;
				string str = g_Config.GetTempMaskStr();
				g_Config.ParseStrToken(str,vec);
				string strTemp = g_TempConfig.GetTempMaskStr();
				for(int i=0;i<vec.size();i++)
				{
					if(vec[i].compare(g_TempConfig.GetTempMaskStr()) != 0)
						strTemp += "/" + vec[i];
				}
				g_Config.SetTempMaskStr(strTemp.c_str());
			}
		}
	}
}

void CGameControl::MSG_Investigation(const char* msg,int iLen)//调查问卷
{
	if(iLen > 1024)
		return;

	char temp[1024] = {0}; //读取服务端发送过来的地址

	memcpy(temp,msg+12,iLen - 12);

	if(strcmp(temp,"http")>=0)
		g_pGameData->SetInvestURL(temp);

	g_pControl->PopupWindow(MSG_CTRL_HELPWND,OPER_CREATE,2);//打开调查问卷的窗口
}

//1.908 商城道具	黄榜大旗	豹语泡泡
void CGameControl::SEND_Input_YelloFlag(DWORD id,string& str,int x,int y,WORD type)
{
	if(str.size() > 2048)
		return;

	SET_COMMAND(CS_PT_YELLOW_FLAG,2048);
	ASSIGN_ID(id);
	ASSIGN_WORD(6,x);
	ASSIGN_WORD(8,y);
	ASSIGN_WORD(10,type);
	ADD_ARRAY(str.c_str(),str.size());
	SEND_GAME_SERVER();
}

////1.908 商城道具	黄榜大旗 提交留言信息
void CGameControl::SEND_LeavWords(DWORD id,const string& str,WORD type)
{
	if(str.size()>1024)
		return;

	SET_COMMAND(CS_PT_LEAVWORDS,1024);
	ASSIGN_ID(id);
	ASSIGN_WORD(6,type);
	ASSIGN_WORD(8,str.size());
	ADD_STR(str.c_str());
	SEND_GAME_SERVER();
}


//1.908 商城道具	
/*
type=1常胜玄兵 
2=开启传世宝盒或天书提示请求
3=解除绑定手机帐号提示请求
4=解除绑定豹魔石提示请求
*/
void CGameControl::MSG_ChangshengXuanbin(const char * msg,int iLen)
{
	DWORD id = *((DWORD*)(msg));
	WORD type = *((WORD*)(msg + 10));

	string buf;
	buf.assign(msg+12,iLen - 12);

	g_MsgBoxMgr.PopSimpleComfirm(buf.c_str(),MSG_CTRL_CHANGSHENGXUANBIN_WND,id,(CControl*)type);
}
//类型同上
void CGameControl::SEND_Message_ChangshengXuanbin(DWORD id,WORD type,char i)
{
	SET_COMMAND(CS_PT_CHANGSHENGXUANBIN,256);
	ASSIGN_ID(id);
	temp[6] = i;
	*((WORD*)(temp + 10)) = type;
	iBLen = 12;
	if(type == 5)
	{
		iBLen += 4;
		ASSIGN_DWORD(12,g_PetData.GetFeedID());
	}

	SEND_GAME_SERVER();
}

/*
1代表请求绑定手机帐号
2代表请求解除绑定手机帐号
3代表请求绑定豹子
4代表请求解除绑定豹子
*/
void CGameControl::MSG_CellPhoneUser_InputRequest(const char* msg,int iLen)
{
	char temp[1024] = {0};
	WORD type = *( (WORD*)(msg + 6) );
	if(type >= 1 && type <= 4)
	{
		g_pControl->PopupWindow(MSG_CTRL_CELLPHONE_USER_WND,OPER_CREATE);
	}

	return;
}

//提交手机绑定的帐号信息
void CGameControl::SEND_CellPhoneUser_Input(string& str,WORD type)
{
	SET_COMMAND(CS_CELLPHONE_USER_INPUT,256);
	ASSIGN_WORD(6,type);
	ADD_STR(str.c_str());
	SEND_GAME_SERVER();
}


//1.908 商城道具 查看怪物属性	i=1 常胜玄兵怪物
void CGameControl::SEND_Message_See_Mon(DWORD id,WORD i)
{
	SET_COMMAND(CS_SEE_MON_INFO,256);
	*((DWORD*)(temp)) = id;
	*((WORD*)(temp + 10)) = i;
	SEND_GAME_SERVER();
}

//type=1,表示黄榜大旗;type=2,表示常胜玄兵
//ID
//是否可以留言等参数
//[20060726]
void CGameControl::MSG_Extern_Messgebox_info(const char * msg,int iLen)
{
	if(iLen>40960)
		return;
	//临时保存当前点击的g_NPC的ID
	g_NPC.Clear();
	g_NPC.SetID( *((DWORD *)msg));

	char temp[2048] = {0};
	int iLength = *((WORD*)(msg + 6)) + 12;
	WORD type = *((WORD*)(msg + 10)) ;



	string strTemp;
	string strTemp2;
	strTemp.clear();
	strTemp2.clear();
	BYTE byCount = 0;
	int iPos = 0;
	if(iLength>12)
		strTemp.assign(msg+12,iLength-12);


	int iTempPos =strTemp.find('\0');
	if( iTempPos != string::npos )	
	{
		memcpy(temp,strTemp.c_str(),iTempPos);

		byCount = strTemp[iTempPos+1];
		if(byCount>=1)
		{
			iTempPos += 2;
			iPos += iTempPos;

			strTemp.assign(strTemp,iTempPos+1,iLength-12-iPos-1);

			iTempPos =strTemp.find('\0');
			for(int i=0;i<byCount;i++)
			{
				if(iTempPos != string::npos)
				{
					iPos += iTempPos;
					strTemp2.assign(strTemp,0,iTempPos);
					g_NPC.GetLeavWords().push_back(strTemp2);
					strTemp.assign(strTemp,iTempPos+1,iLength-12-iPos-1);

					iTempPos = strTemp.find('\0');
				}
				else
				{
					strTemp2.assign(strTemp);
					g_NPC.GetLeavWords().push_back(strTemp2);
				}
			}

		}
		else
		{
			memcpy(temp,strTemp.c_str(),strTemp.size());
		}


	}
	else
	{
		memcpy(temp,strTemp.c_str(),strTemp.size());
	}

	g_pControl->PopupWindow(MSG_CTRL_EXTERN_MESSAGEBOX_WND,OPER_RECREATE,type);
	g_pControl->Msg(MSG_CTRL_EXTERN_MESSAGEBOX_WND,0,(CControl*)temp);

}

void CGameControl::SEND_Good_Info_Require(const char *strGoodName)
{
    SET_COMMAND(CS_GOOD_INFO_REQUIRE,30);
    ASSIGN_WORD(6,strlen(strGoodName));
    ADD_STR(strGoodName);
	iBLen += 1;
    SEND_GAME_SERVER();
}

void CGameControl::MSG_Good_Info_Answer(const char* msg,int iLen)
{
    CGood good;
    good.FromBuffer(msg + 12,false,iLen - 12);
    if(strlen(good.GetName()) <= 0)
    {
        return;
    }

    CTagText::sm_GoodMap[good.GetName()] = good;
}

//g_NPC购物图形化，弹出快捷买卖、修理窗口
void CGameControl::MSG_Npc_Quick_Exchange(const char* msg,int iLen)
{
	DWORD dwID = Conv_DWORD(msg);//g_NPC指针,或者取回的物品指针
	WORD wCount = Conv_WORD(msg+6);// 物品数目
	BYTE byType = Conv_BYTE(msg+8);//0:为npc快捷卖物列表,1:npc快捷存取列表
	
	BYTE byGoodLen = BYTE(msg[9]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}

	WORD wStorageMaxCount = Conv_WORD(msg+10);//在wType=1时表示包裹容量

	if(byType == 0 || byType == 1)//快捷卖物列表及快捷存取列表用的都是g_NPC.GetNpcQuickVector()
	{
		g_NPC.SetID(dwID);
		g_NPC.GetNpcQuickVector().clear();
		if(byType == 1)
			g_NPC.SetStorageMaxCount(wStorageMaxCount);

		int iPos = 12;
		for(WORD i = 0; i < wCount && iPos < iLen; i++,iPos += byGoodLen)
		{
			CGood temp;
			if(byType == 0)
				temp.FromBuffer(msg+iPos,true,byGoodLen);
			else 
				temp.FromBuffer(msg+iPos,false,byGoodLen);

			g_NPC.GetNpcQuickVector().push_back(temp);
		}

		g_pControl->PopupWindow(MSG_CTRL_NPC_QUICK_WND,OPER_CREATE,byType + 1);
	}
	else if(byType == 2)//2存物品成功,添加新物品
	{
		CGood temp;
		temp.FromBuffer(msg+12,false,byGoodLen);
		g_NPC.GetNpcQuickVector().push_back(temp);

		g_NPC.GetGood().SetID(0);	
		g_pControl->MsgToWnd(MSG_CTRL_NPC_QUICK_WND,MSG_CTRL_NPC_QUICK_WND,201);
	}
	else if(byType == 3)//取仓库物品成功
	{
		std::vector<CGood> &npcquick = g_NPC.GetNpcQuickVector();
		std::vector<CGood>::iterator iter;
		for(iter = npcquick.begin(); iter != npcquick.end(); iter++)
		{
			if((*iter).GetID() == dwID)
			{
				npcquick.erase(iter);
				DWORD* posArray = CNpcQuickWnd::GetGoodPos();

				for (int ii = 0; ii < MAXNUMBERSTORAGE; ++ii)
				{
					if (posArray[ii] == dwID)
					{
						posArray[ii] = 0;
						break;
					}
				}
				break;
			}
		}

		g_NPC.GetGood().SetID(0);
		g_pControl->MsgToWnd(MSG_CTRL_NPC_QUICK_WND,MSG_CTRL_NPC_QUICK_WND,201);
	}
	else if (byType == 4)//从VIP交易行取回物品或者买物品成功
	{
		SVipTradGood &VTGood = g_NPC.GetVipTradeGood();
		bool bFind = false;

		for(MVipGood::iterator iter = VTGood.mMVipGood.begin();!bFind && iter != VTGood.mMVipGood.end(); iter++)
		{
			vector<CGood> &VOnePersonGoods = iter->second.VGoods;
			for (vector<CGood>::iterator itr3 = VOnePersonGoods.begin(); itr3 != VOnePersonGoods.end(); itr3 ++)
			{
				if (itr3->GetID() == dwID)
				{
					VOnePersonGoods.erase(itr3);
					bFind = true;
					break;
				}
			}
		}

		g_pControl->PopupWindow(MSG_CTRL_FIRM_WND,MSG_CTRL_FIRM_WND,1);
	}


	g_NPC.SetExchangeFromQuickNpcWnd(false);

}

//g_NPC图形化购物]
//byType 0:购买物品

void CGameControl::SEND_Npc_Quick_Exchange(BYTE byType,DWORD dwNpcID,DWORD dwGoodID,const char* strName)
{
	SET_COMMAND(SC_NPC_QUICK_EXCHANGE,256);
	ASSIGN_DWORD(0,dwNpcID);
	ASSIGN_DWORD(6,dwGoodID);
	ASSIGN_BYTE(10,byType);
	ADD_STR(strName);
	SEND_GAME_SERVER();
}

//弹出拆分符石框
void CGameControl::MSG_Split_Stone(const char * msg,int iLen)
{
	DWORD dwData = Conv_WORD(msg+6);
	g_pControl->PopupWindow(MSG_CTRL_SPLITSTONE_WND,OPER_CREATE,dwData);
}

void CGameControl::SEND_Stone_Split_Request(DWORD dwID,DWORD dwSplitNum,BYTE byType)
{
	SET_COMMAND(CS_SPLIT_STONE,12);
	ASSIGN_DWORD(0,dwID);
	ASSIGN_DWORD(6,dwSplitNum);
	ASSIGN_BYTE(10,byType);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_ADD_Internet_Msg(const char * msg,int iServer)
{
	string buf;
	buf.assign(msg,strlen(msg));

	switch(iServer)
	{
	case INET_ACTIVITY:
		{
			g_TagTextMgr.GetActData().Parse(buf);
			break;
		}
	case INET_MAIN:
		{
			g_TagTextMgr.GetHelpData().Parse(buf);
			break;
		}
	case INET_INVEST:
		{
			g_TagTextMgr.GetInvestData().Parse(buf);
			break;
		}
	case INET_RADIO:
		{
			g_TagTextMgr.GetRadioData().Parse(buf);
			break;
		}
	case INET_REPLY:
		{
			g_OtherData.ParseNote(buf);
			break;
		}
	case INET_LOGO:
		{
			g_OtherData.GetLogoTex(buf.c_str());
			break;
		}
	}
}

void CGameControl::MSG_Internet_Error(int nNum,DWORD dwParam)
{
	string str = "数据读取错误";
	switch(nNum)
	{
	case INET_ACTIVITY:
		{
			g_TagTextMgr.GetActData().setText(str);
			break;
		}
	case INET_MAIN:
		{
			g_TagTextMgr.GetHelpData().setText(str);
			break;
		}
	case INET_INVEST:
		{
			g_TagTextMgr.GetInvestData().setText(str);
			break;
		}
	case INET_RADIO:
		{
			g_TagTextMgr.GetRadioData().setText(str);
			break;
		}
	case INET_REPLY:
		{
			g_OtherData.GetNoteBody()["#000"] = str;
			break;
		}
	case INET_LOGO:
		{
			if( g_OtherData.IsChangeLogo())
			{
				g_OtherData.SetChangeLogo(false);
				g_MsgBoxMgr.PopSimpleAlert("设定行会会旗失败，请确认会旗编号是否正确。");			
			}

			if(dwParam == g_OtherData.OtherPlayer().GetGuildLogoID())
			{
				g_OtherData.OtherPlayer().SetLogo(true);
			}
			break;
		}
	}
}

void CGameControl::MSG_FengShengTakeObject(const char *strMsg,int iLen)
{
	BYTE bType = strMsg[6];
	BYTE bSuccess = strMsg[7];
	if(bType == 0)
	{
		if(bSuccess == 0)//成功
		{
			g_NPC.GetGood().SetID(0);
			g_pControl->PopupWindow(MSG_CTRL_NPCSALEWND,OPER_CLOSE);
		}
		else//失败
		{
			CGood &temp = g_NPC.GetGood();
			if(temp.GetID() != 0)
			{
				for(int i = 0; i < MAX_PACKAGE_ELEMENT ; i++)
				{
					if(SELF.GetPackageGood(i).GetID() != 0)
						continue;

					SELF.GetPackageGood(i) = temp;
					g_NPC.GetGood().SetID(0);
					break;
				}
			}
		}
	}
}

void CGameControl::SEND_Fill_LingFuBag(DWORD dwBagID,DWORD dwLingFuID)
{
    SET_COMMAND(CS_FILL_LINGFUBAG,32);
    ASSIGN_DWORD(0,dwBagID);
    ASSIGN_DWORD(6,dwLingFuID);
    SEND_GAME_SERVER();
}

//宠物幻化,客户端请求检查灵兽石type为0，客户端请求幻化灵兽石type为1
//宠物幻化　0X0BEC
//协议号
//C->S  第0～3位是　主灵兽的豹魔石ID
//		第6～9位是　副灵兽的豹魔石ID
//　　　第10位BYTE  表示这次是客户端请求检查灵兽石，还是客户端请求幻化灵兽石
//   　 第11位BYTE　备用
//	    第12～16位是　封元印的ID
void CGameControl::SEND_PET_MERGE(BYTE byType,DWORD dwFirstPetID,DWORD dwSecondPetID,DWORD dwSealID)
{
    SET_COMMAND(CS_PET_MERGE,32);
    ASSIGN_DWORD(0,dwFirstPetID);//主灵兽的豹魔石ID    
    ASSIGN_DWORD(6,dwSecondPetID);//副灵兽的豹魔石ID
    ASSIGN_BYTE(10,byType) ;   
    ADD_DWORD(dwSealID);
    
	g_NPC.GetPetMergeData().byTestResult = 0xff;
    g_NPC.GetPetMergeData().byMergeResult = 0xff;
    g_NPC.GetPetMergeData().iLingLi = 0;
    g_NPC.GetPetMergeData().iMoney = 0;
	g_NPC.GetPetMergeData().result.clear();

    SEND_GAME_SERVER();
}

//宠物幻化　0X0BEC
//S->C  第6位　测试结果　0 幻化结果　1
//　　　第7位　0　成功　>0　都是失败　失败原因后期详细指定
//   　 第8.9位　返回需要的玩家灵力值
//	  　第10.11位　返回需要的玩家金币（单位为万）
void CGameControl::MSG_PET_MERGE(const char* Msg,int iLen)
{
    byte byType = *(BYTE*)(Msg + 6);
    byte byResult = *(BYTE*)(Msg + 7);
    WORD wLingli = Conv_WORD(Msg + 8);
    WORD wMoney = Conv_WORD(Msg + 10);

    if(byType == 0)
    {
		g_NPC.GetPetMergeData().byTestResult = byResult;
		g_NPC.GetPetMergeData().iLingLi = wLingli;
		g_NPC.GetPetMergeData().iMoney = wMoney;
    }
    else
    {
        g_NPC.GetPetMergeData().byMergeResult = byResult;
        
    }

	g_NPC.GetPetMergeData().result.assign(Msg + 12,iLen - 12);

	if(byType == 0 && g_NPC.GetPetMergeData().byTestResult != 0 && g_NPC.GetPetMergeData().result.size() != 0)
	{
		g_MsgBoxMgr.PopSimpleAlert(g_NPC.GetPetMergeData().result.c_str());
	}
}

//协议格式
//协议号 0343
//客户端打开界面第一次请求所有打开格子的数据
//第7位  0
//玩家请求打开格子
//第7位  1
//第8位显示格子的位置0～71
//玩家请求进入极品阁
//第七位 2
//第8位  点击的门的位置
//玩家请求打开极品阁格子
//第7位  3
//第9位  原来珍宝阁所属的格子位置（服务器检查用）
//第8位 格子的位置0～35

void CGameControl::SEND_ZHENBAO_INFO(int iType,BYTE iNum)
{
	SET_COMMAND(CS_ZHENBAO_INFO,32);
	ASSIGN_BYTE(6,iType);	
	ASSIGN_BYTE(7,(BYTE)iNum); 
	if(iType == 3)
	{
		ASSIGN_BYTE(8,g_NPC.GetZhenBaoState().iOriginPos); 	
	} 
	SEND_GAME_SERVER();
}

//服务器发送1
//第7位 0  服务器发送初始状态
//第8位 表示 打开格子的个数
//第9位 表示 是否节点状态
//第10位 可否打开 0 可以打开 1 今天珍宝阁不开放
//如果打开格子的个数=0 则后面不发送
//如果打开格子的个数>0 12位后为已经打开的格子的信息
//格子类型(1)＋格子位置 (1位，0～71) +奖励图片(4)+奖励数量(4)+玩家名字长度+玩家名 + {(当格子奖励为1物品/3图腾时)+打开奖励名字长度(1)+打开奖励名}

//服务器发送2
//第7位 1  点击格子的结果 
//第8位    能否打开 0 1 （状态肯定是打开中）
//第9位    格子位置
//第10位   格子类型
//12位以后信息：
//奖励图片(4)+奖励数量(4)+玩家名字长度+玩家名 + {(当格子奖励为1物品/3图腾时)+打开奖励名字长度(1)+打开奖励名}

//服务器发送5
//12位后跟提示语句
void CGameControl::MSG_ZHENBAO_INFO(const char* msg,int iLen)
{
	_ZhenBaoState& zhenBaoState = g_NPC.GetZhenBaoState();
	byte byType = *(byte*)(msg + 6); 

	BYTE byGoodLen = BYTE(msg[11]);//单个物品的长度
	if (byGoodLen == 0)
	{
		byGoodLen = CGood::PKLength();
	}


	if(byType == 0)		//服务器第一次发送
	{
		if(*(BYTE*)(msg + 9) == 0)
		{
			g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_RECREATE,MAKELONG(2,3));
		}
		else
		{
			g_MsgBoxMgr.PopSimpleAlert("              行会宝塔资源不足被封闭，无法开启珍宝阁");
			return;
		}

		zhenBaoState.Clear();

		if(*(BYTE*)(msg + 8) != 0)
		{
			zhenBaoState.bSpecialNode = true;
		}		

		int iOpenedGridNum = *(BYTE*)(msg + 7);
		const char* start = msg + 12;

		for(int i = 0; i < iOpenedGridNum; i++)
		{
			byte byType = *(byte*)(start);
			WORD byPos = *(byte*)(start + 1);
			zhenBaoState.grid[byPos].iType = byType;
			if(byType == 3)
			{
				zhenBaoState.grid[byPos].iState = 1;
			}
			else
			{
				zhenBaoState.grid[byPos].iState = 2;
			}

			if(*(byte*)(start + 2) == 0)
			{
				zhenBaoState.grid[byPos].bShine = false;
			}
			else
			{
				zhenBaoState.grid[byPos].bShine = true;
			}

			zhenBaoState.grid[byPos].uLooks = *(DWORD*)(start + 3);
			if(zhenBaoState.grid[byPos].uLooks == 0)	//默认图片
			{
				zhenBaoState.grid[byPos].uLooks = 0x001003b7;	//默认经验值图片
			}

			zhenBaoState.grid[byPos].iCount = *(DWORD*)(start + 7);
			if(zhenBaoState.grid[byPos].iCount == 0)	//默认数量
			{
				zhenBaoState.grid[byPos].iCount = 1;
			}

			byte byNameLen = *(byte*)(start + 11);	//玩家名字长度
			if(byNameLen != 0)
			{
				zhenBaoState.grid[byPos].strOwnerName.assign(start + 12,byNameLen);
			}

			if(byType == 1 || byType == 2)
			{
				byte byPrizeLen = *(byte*)(start + 12 + byNameLen);		//奖励名字长度
				if(byPrizeLen != 0)
				{
					zhenBaoState.grid[byPos].strPrizeName.assign(start + 13 +byNameLen,byPrizeLen);
				}
				
				start = start + 13 + byNameLen + byPrizeLen;
			}
			else
			{
				start = start + 12 + byNameLen;
			}
		}
	}
	else if(byType == 1)	//点击格子的结果 
	{
		if(*(byte*)(msg + 7) == 0)  		//不能打开
		{
			
		}		
		else
		{
			g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);

			int byPos = *(byte*)(msg + 8);
			int byType = *(byte*)(msg + 9);

			if(*(byte*)(msg + 10) == 0)
			{
				zhenBaoState.grid[byPos].bShine = false;
			}
			else
			{
				zhenBaoState.grid[byPos].bShine = true;
			}
			
			zhenBaoState.grid[byPos].dwOpenTime = GetTickCount();
			
			zhenBaoState.grid[byPos].iType = byType;

			if(byType == 3)
			{				
				zhenBaoState.grid[byPos].iState = 1;
			}
			else
			{
				zhenBaoState.grid[byPos].iState = 2;
			}

			const char* start = msg + 12;

			zhenBaoState.grid[byPos].uLooks = *(DWORD*)(start);
			if(zhenBaoState.grid[byPos].uLooks == 0)	//默认图片
			{
				zhenBaoState.grid[byPos].uLooks = 0x001003b7;	//默认经验值图片
			}

			zhenBaoState.grid[byPos].iCount = *(DWORD*)(start + 4);
			if(zhenBaoState.grid[byPos].iCount == 0)	//默认数量
			{
				zhenBaoState.grid[byPos].iCount = 1;
			}

			byte byNameLen = *(byte*)(start + 8);	//玩家名字长度
			if(byNameLen != 0)
			{
				zhenBaoState.grid[byPos].strOwnerName.assign(start + 9,byNameLen);
			}

			if(byType != 0)
			{
				byte byPrizeLen = *(byte*)(start + 9 + byNameLen);		//奖励名字长度
				if(byPrizeLen != 0)
				{
					zhenBaoState.grid[byPos].strPrizeName.assign(start + 10 +byNameLen,byPrizeLen);
				}
			}	

			if(strcmp(SELF.GetName(),zhenBaoState.grid[byPos].strOwnerName.c_str()) == 0)	//自己点的格子
			{
				zhenBaoState.grid[byPos].dwOpenTime = 0;
				g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM + 3);

				if(byType == 3)		//自己点到了极品阁
				{
					g_MsgBoxMgr.PopSimpleComfirm("你发现了一个极品阁，阁内珍藏着极品装备和图腾。你要花费一个灵皓石进入极品阁吗？进入极品阁后无需花费任何行会银两就可获得阁内珍宝。",MSG_CTRL_ENTER_JIPINGE,byPos);
				}
			}			
		}
	}	
	else if(byType == 2)	//弹出极品阁界面
	{
		g_pControl->PopupWindow(MSG_CTRL_RELATION_WND,OPER_RECREATE,MAKELONG(3,4));	
		zhenBaoState.iOriginPos = *(BYTE*)(msg + 8);

		int iOpenedGridNum = *(BYTE*)(msg + 7);

		if(iOpenedGridNum > 36 || iLen <= 12)
		{
			return;
		}
		
		TRY_BEGIN
			const char* start = msg + 12;
			for(int i = 0; i < iOpenedGridNum; i++)
			{
				byte byType = *(byte*)(start);
				WORD byPos = *(byte*)(start + 1);
				
				if(*(byte*)(start + 2) == 0)
				{
					zhenBaoState.jipingrid[byPos].bShine = false;
				}
				else
				{
					zhenBaoState.jipingrid[byPos].bShine = true;
				}

				zhenBaoState.jipingrid[byPos].iType = byType;
				zhenBaoState.jipingrid[byPos].iState = 2;

				byte byNameLen = *(byte*)(start + 3);	//玩家名字长度
				if(byNameLen != 0)
				{
					if (iLen < (start + 4 + byNameLen - msg))
					{
						break;
					}

					zhenBaoState.jipingrid[byPos].strOwnerName.assign(start + 4,byNameLen);
				}

				if(byType != 1)		//1为骷髅，没有物品包
				{
					if (iLen < (start + 4 + byNameLen + byGoodLen - msg))
					{
						break;
					}
					zhenBaoState.jipingrid[byPos].prize.FromBuffer(start + 4 + byNameLen,false,byGoodLen);
					zhenBaoState.jipingrid[byPos].prize.AddItemState(InJiPinGrid);
					start = start + 4 + byNameLen + 68;
				}
				else
				{
					if (iLen < (start + 4 + byNameLen - msg))
					{
						break;
					}

					start = start + 4 + byNameLen;
				}
				
			}
	  TRY_END		
	}
	else if(byType == 3)	//点击极品阁格子的结果 
	{
		int byPos = *(byte*)(msg + 7);

		if(*(byte*)(msg + 10) == 0)
		{
			zhenBaoState.jipingrid[byPos].bShine = false;
		}
		else
		{
			zhenBaoState.jipingrid[byPos].bShine = true;
		}

		int result = *(byte*)(msg + 9);
		if(result == 1)			//点到了骷髅
		{
			zhenBaoState.jipingrid[byPos].iType = 1;
			zhenBaoState.jipingrid[byPos].iState = 2;
			byte byNameLen = *(byte*)(msg + 12);	//玩家名字长度
			zhenBaoState.jipingrid[byPos].strOwnerName.assign(msg + 13,byNameLen);

			if(strcmp(SELF.GetName(),zhenBaoState.jipingrid[byPos].strOwnerName.c_str()) == 0)	//自己点到了骷髅
			{
				g_pControl->Msg(MSG_CTRL_RELATION_WND,OPER_CUSTOM + 2);
				//g_MsgBoxMgr.PopSimpleAlert("碰到骷髅啦，请点击确定退出。",MSG_CTRL_LEAVE_JIPINGE);
			}
		}
		else if(result == 0)	//点击成功
		{
			g_pAudio->Play(EAT_OTHER,3,g_pAudio->GetRand()++);
			zhenBaoState.jipingrid[byPos].iType = 0;
			zhenBaoState.jipingrid[byPos].iState = 2;			
			byte byNameLen = *(byte*)(msg + 12);	//玩家名字长度
			zhenBaoState.jipingrid[byPos].strOwnerName.assign(msg + 13,byNameLen);
			zhenBaoState.jipingrid[byPos].prize.FromBuffer(msg + 13 + byNameLen,false,byGoodLen);
			if(strcmp(SELF.GetName(),zhenBaoState.jipingrid[byPos].strOwnerName.c_str()) != 0)	//不是自己点的格子放特效，自己点的已经放完了
			{
				zhenBaoState.jipingrid[byPos].dwOpenTime = GetTickCount();
			}
		}
		else if(result == 2)	//已经被点过了
		{

		}
	}
	else if(byType == 4)	//包裹不够或者行会银两不足
	{
		string buf;
		buf.assign(msg + 12,iLen - 12);
		g_MsgBoxMgr.PopSimpleAlert(buf.c_str());
	}
	else if(byType == 5)
	{
		zhenBaoState.strHint.assign(msg + 12,iLen - 12);
	}
}

void CGameControl::MSG_SHENGONGFANG_INFO(const char* msg,int iLen)
{
// 	g_NPC.GetShenGongFangGoods().bSend = false;
// 
// 	int result = msg[6];	//升级盾牌0成功1有假物品2留盾牌扣其它3全部物品扣光4护宝符保存物品
// 	//升级魂珠0成功1有假物品2失败
// 	//圣王升级0成功1有假物品2留盾牌扣其它3全部物品扣光4护宝符保存物品
// 	if(result == 0)
// 	{
// 		if(g_NPC.GetShenGongFangGoods().m_Shield.GetStdMode() == 12)	//盾牌升级成功物品更新
// 		{
// 			g_NPC.GetShenGongFangGoods().iResult = 1;
// 			g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 			if(g_NPC.GetShenGongFangGoods().m_Luck.GetDura() == 1)
// 			{
// 				g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 			}
// 		}
// 		else															//其他物品升级成功后删掉，服务器会发来新物品
// 		{
// 			g_NPC.GetShenGongFangGoods().iResult = 1;
// 			g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 			if(g_NPC.GetShenGongFangGoods().m_Luck.GetDura() == 1)
// 			{
// 				g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 			}
// 			g_NPC.GetShenGongFangGoods().m_Shield.SetID(0);	//升级魂珠成功都删了
// 		}
// 	}
// 	else if(result == 1)		//有假物品，将3件物品放回包裹，等待假物品协议将之删除
// 	{
// 		g_NPC.GetShenGongFangGoods().iResult = 3;
// 
// 		if(g_NPC.GetShenGongFangGoods().m_Shield.GetID() != 0)
// 		{
// 			SELF.PackageGood().BackToArray(g_NPC.GetShenGongFangGoods().m_Shield);
// 			g_NPC.GetShenGongFangGoods().m_Shield.SetID(0);
// 		}
// 
// 		if(g_NPC.GetShenGongFangGoods().m_Wood.GetID() != 0)
// 		{
// 			SELF.PackageGood().BackToArray(g_NPC.GetShenGongFangGoods().m_Wood);
// 			g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 		}
// 
// 		if(g_NPC.GetShenGongFangGoods().m_Luck.GetID() != 0)
// 		{
// 			SELF.PackageGood().BackToArray(g_NPC.GetShenGongFangGoods().m_Luck);
// 			g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 		}
// 	}
// 	else if(result == 2)
// 	{
// 		g_NPC.GetShenGongFangGoods().iResult = 0;
// 		g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 		if(g_NPC.GetShenGongFangGoods().m_Luck.GetDura() == 1)
// 		{
// 			g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 		}
// 	}
// 	else if(result == 3)
// 	{
// 		g_NPC.GetShenGongFangGoods().iResult = 2;
// 		g_NPC.GetShenGongFangGoods().m_Shield.SetID(0);
// 		g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 		if(g_NPC.GetShenGongFangGoods().m_Luck.GetDura() == 1)
// 		{
// 			g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 		}
// 	}
// 	else if(result == 4)
// 	{
// 		g_NPC.GetShenGongFangGoods().iResult = 4;
// 		g_NPC.GetShenGongFangGoods().m_Wood.SetID(0);
// 		if(g_NPC.GetShenGongFangGoods().m_Luck.GetDura() == 1)
// 		{
// 			g_NPC.GetShenGongFangGoods().m_Luck.SetID(0);
// 		}
// 	}
}

void CGameControl::SEND_SHENGONGFANG_INFO(DWORD dwShieldID,DWORD dwWoodID,DWORD dwLuckID,BYTE byType)
{
	g_NPC.GetShenGongFangGoods().iResult = -1;

	SET_COMMAND(CS_SHENGONG_FANG,32);
	ASSIGN_DWORD(0,dwShieldID);
	ASSIGN_DWORD(8,dwWoodID);

	//if(g_NPC.GetShenGongFangGoods().m_Shield.GetStdMode() == 12)
	//{
	//	ASSIGN_BYTE(6,0);
	//}
	//else if(g_NPC.GetShenGongFangGoods().m_Shield.GetStdMode() == 13)
	//{
	//	ASSIGN_BYTE(6,1);
	//}
	//else		//圣王炼制
	//{
	//	ASSIGN_BYTE(6,2);
	//}

	ASSIGN_BYTE(6,byType);

	if(dwLuckID != 0)
	{
		ADD_DWORD(dwLuckID);
	}

	SEND_GAME_SERVER();
}
void CGameControl::SEND_DISASSEMBLE_GOODS(DWORD id,bool btest)
{
	SET_COMMAND(CS_DISASSEMBLE_GOODS,32);

	ASSIGN_DWORD(0,id);

	if(btest)
	{
		ASSIGN_BYTE(6,1);
	}
	else
	{
		ASSIGN_BYTE(6,0);
	}

	SEND_GAME_SERVER();
}

void CGameControl::SEND_COMBINE_YUANSHI(DWORD id,bool btest)
{
	SET_COMMAND(CS_COMBINE_YUANSHI,32);

	ASSIGN_DWORD(0,id);

	if(btest)
	{
		ASSIGN_BYTE(6,1);
	}
	else
	{
		ASSIGN_BYTE(6,0);
	}

	SEND_GAME_SERVER();
}

void CGameControl::MSG_DISASSEMBLE_GOODS(const char * msg,int iLen)
{
	g_NPC.GetYuanShiResult().iResult = msg[0];

	if(msg[0] == 3)	//拆解成功
	{
		g_pControl->Msg(MSG_CTRL_EXTRACTUPGRADE_YUANSHI,OPER_CUSTOM);
		return;
	}
	else if(msg[0] == 4)	//拆解失败
	{
		g_pControl->Msg(MSG_CTRL_EXTRACTUPGRADE_YUANSHI,OPER_CUSTOM + 1);
		return;
	}	

	WORD wLen = Conv_WORD(msg + 6);

	if(msg[8] == 1)			//高级物品
	{
		g_NPC.GetYuanShiResult().bExpensive = true;
	}
	else
	{
		g_NPC.GetYuanShiResult().bExpensive = false;
	}

	string strResult;
	strResult.assign(msg + 12,wLen);
	g_NPC.GetYuanShiResult().tagText.Parse(strResult,0,"\\",strResult.length());
}

void CGameControl::MSG_COMBINE_YUANSHI(const char * msg,int iLen)
{
	g_NPC.GetYuanShiResult().iResult = msg[0];

	if(msg[0] == 3)	//合成成功
	{
		g_pControl->Msg(MSG_CTRL_EXTRACTUPGRADE_YUANSHI,OPER_CUSTOM);
		return;
	}
	else if(msg[0] == 4)	//合成失败
	{
		g_pControl->Msg(MSG_CTRL_EXTRACTUPGRADE_YUANSHI,OPER_CUSTOM + 1);
		return;
	}	

	WORD wLen = Conv_WORD(msg + 6);

	string strResult;
	strResult.assign(msg + 12,wLen);

	if(g_NPC.GetYuanShiResult().iResult == 1)
	{
		char str[256] = "";
		int iLianHuaLevel = 0;
		CMagicData* pMagic = SELF.ProduceMagicArray().FindMagic(MAGICID_SMELTSKILL);
		if (pMagic)
		{
			iLianHuaLevel = pMagic->GetSkillValue();
		}

		sprintf(str,"                  您当前的炼化等级为%d级，%s",iLianHuaLevel,strResult.c_str());
		strResult = str;
	}

	g_NPC.GetYuanShiResult().tagText.Parse(strResult,0,"\\",strResult.length());
}

void CGameControl::Send_Extract_Gem()
{
	SET_COMMAND(CS_EXTRACT_GEM,32);
	ASSIGN_DWORD(0,g_NPC.GetEquLUpGood(6).GetID());

	for(int i = 0;i < 5;i++)
	{
		if(g_NPC.GetEquLUpGood(i).GetID() != 0)
		{
			ADD_DWORD(g_NPC.GetEquLUpGood(i).GetID());
		}
	}

	SEND_GAME_SERVER();

}

void CGameControl::Send_Remove_Gem(DWORD dwGemID,BYTE byType)
{
	SET_COMMAND(CS_REMOVE_GEM,32);
	ASSIGN_DWORD(0,dwGemID);
	ASSIGN_BYTE(6,byType);
	SEND_GAME_SERVER();
}

void CGameControl::Msg_Relive_Demon(const char * msg,int iLen)
{
	string str;
	str.assign(msg + 12,Conv_WORD(msg + 6));
	g_MsgBoxMgr.PopSimpleComfirm(str.c_str(),MSG_RELIVE_DEMON,0);
}

//void CGameControl::Send_Relive_Demon()
//{
//	SET_COMMAND(CS_RELIVE_DEMON,32);
//	SEND_GAME_SERVER();
//}

void CGameControl::Send_Drill_Equip(DWORD dwEquipID,DWORD dwLuckID,bool bTest)
{
	SET_COMMAND(CS_DRILL_EQUIP,32);
	ASSIGN_DWORD(0,dwEquipID);

	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	ASSIGN_DWORD(8,dwLuckID);

	SEND_GAME_SERVER();
}

void CGameControl::Msg_Success_Probability(const char * msg,int iLen)
{
	g_NPC.GetDrillResult().iPercent = Conv_WORD(msg + 6);
}

void CGameControl::Send_Embed_Fushi(DWORD dwEquipID,DWORD dwFushiID,DWORD dwLuckID,bool bTest)
{
	SET_COMMAND(CS_EMBED_FUSHI,32);

	ASSIGN_DWORD(0,dwEquipID);
	
	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	ADD_DWORD(dwFushiID);

	if(dwLuckID)
	{
		ADD_DWORD(dwLuckID);
	}

	SEND_GAME_SERVER();
}

void CGameControl::Send_Remove_Fushi(DWORD dwEquipID,BYTE byHoleNum,DWORD dwLuckID,bool bTest)
{
	SET_COMMAND(CS_REMOVE_FUSHI,32);

	ASSIGN_DWORD(0,dwEquipID);

	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	ASSIGN_BYTE(7,byHoleNum);

	ASSIGN_DWORD(8,dwLuckID);	

	SEND_GAME_SERVER();
}

void CGameControl::Send_Upgrade_Fushi(DWORD dwFushiID,DWORD dwLuckID,bool bTest)
{
	SET_COMMAND(CS_UPGRADE_FUSHI,32);

	ASSIGN_DWORD(0,dwFushiID);

	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	ASSIGN_DWORD(8,dwLuckID);

	SEND_GAME_SERVER();
}

void CGameControl::Send_Degrade_Fushi(DWORD dwFushiID,DWORD dwLuckID)
{
	SET_COMMAND(CS_DEGRADE_FUSHI,32);

	ASSIGN_DWORD(0,dwFushiID);	

	ASSIGN_DWORD(6,dwLuckID);

	SEND_GAME_SERVER();
}

void CGameControl::Send_Combine_Fushi(DWORD dwLuckID,DWORD FushiID1,DWORD FushiID2,DWORD FushiID3,DWORD FushiID4,DWORD FushiID5,bool bTest)
{
	SET_COMMAND(CS_COMBINE_FUSHI,32);

	ASSIGN_DWORD(0,dwLuckID);

	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	if(FushiID1)
	{
		ADD_DWORD(FushiID1);
	}

	if(FushiID2)
	{
		ADD_DWORD(FushiID2);
	}

	if(FushiID3)
	{
		ADD_DWORD(FushiID3);
	}

	if(FushiID4)
	{
		ADD_DWORD(FushiID4);
	}

	if(FushiID5)
	{
		ADD_DWORD(FushiID5);
	}

	SEND_GAME_SERVER();
}

void CGameControl::Send_Change_Hole(DWORD dwEquipID,BYTE bySrcHole,BYTE byDestHole,DWORD dwLuckID,bool bTest)
{
	SET_COMMAND(CS_CHANGE_HOLE,32);

	ASSIGN_DWORD(0,dwEquipID);

	if(bTest)
	{
		ASSIGN_BYTE(6,1);
	}

	ASSIGN_BYTE(7,bySrcHole);
	ASSIGN_BYTE(8,byDestHole);

	if(dwLuckID)
	{
		ADD_DWORD(dwLuckID);
	}

	SEND_GAME_SERVER();
}

void CGameControl::Send_Upgrade_Equip(DWORD dwEquipID,DWORD dwFushiID,DWORD dwLuckID)
{
	SET_COMMAND(CS_UPGRADE_GOOD,32);

	ASSIGN_DWORD(0,dwEquipID);	

	ASSIGN_DWORD(8,dwFushiID);

	if(dwLuckID)
	{
		ADD_DWORD(dwLuckID);
	}

	SEND_GAME_SERVER();
}

void CGameControl::Send_LuckQYYZ_Select(BYTE pos)
{
	SET_COMMAND(CS_LUCKQYZ_SELECT ,12);

	ASSIGN_BYTE(6, pos);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Open_LuckQYZWnd(const char * msg,int iLen)
{
	g_pControl->PopupWindow(MSG_CTRL_LUCKQYZ_WND,OPER_RECREATE);
	BYTE LuckChance = Conv_BYTE(msg+6);
	if (LuckChance == 2)
	{
		g_pControl->MsgToWnd(MSG_CTRL_LUCKQYZ_WND, MSG_CTRL_LUCKQYZ_CHANCE, LuckChance, NULL);
	}
}

void CGameControl::MSG_LuckQYZ_Prize(const char *msg, int iLen)
{
	BYTE num = *(BYTE*)(msg+6);
	num/=2;

	DWORD dwPos, dwPrize;
	for(BYTE i=0; i<num; i++)
	{
		dwPos = *(DWORD*)(msg + i*8 + 12);
		dwPrize = *(DWORD*)(msg + i*8 + 16);
		g_pControl->MsgToWnd(MSG_CTRL_LUCKQYZ_WND, MSG_CTRL_LUCKQYZ_PRIZE, dwPos, (CControl*)&dwPrize);
	}
}

void CGameControl::Send_BlessZhongZhou_Change(DWORD dwCharID, DWORD dwChangeID, BYTE ChangeTo)
{
	SET_COMMAND(CS_BLESSZHONGZHOU_CHANGECHAR ,12);

	ASSIGN_DWORD(0, dwChangeID);

	ASSIGN_BYTE(6, ChangeTo);

	ASSIGN_DWORD(8, dwCharID);

	SEND_GAME_SERVER();
}

void CGameControl::Send_Change_Material(DWORD dwGoodID, DWORD dwChangeID, BYTE MtrlType, BYTE ChangeTo)
{
	SET_COMMAND(CS_CHANGE_MATERIAL ,12);

	ASSIGN_DWORD(0, dwChangeID);

	ASSIGN_BYTE(6, MtrlType);

	ASSIGN_BYTE(7, ChangeTo);

	ASSIGN_DWORD(8, dwGoodID);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_Open_BlessZhongZhouChangeDlg(const char * msg, int iLen)
{
	g_pControl->PopupWindow(MSG_CTRL_BLESSZHONGZHOU_WND,OPER_RECREATE, 0);
}

void CGameControl::SEND_SELECT_BAOJIAN(byte byRound)
{
	SET_COMMAND(CS_BAOJIAN_SELECT,12);	

	ASSIGN_BYTE(6,byRound);	

	SEND_GAME_SERVER();
}

std::string ClearNum(std::string s)
{
	int n = s.size();
	while (n > 0)
	{
		if ((s[n-1]>'9' || s[n-1]<'0')
			&& (s[n-1]<'A' || s[n-1]>'Z')
			&& (s[n-1] != 0))
			break;
		n--;
	}
	if (n != s.size())
	{
		return s.substr(0, n);
	}
	else return s;
}

void CGameControl::MSG_SELECT_BAOJIAN(const char * msg, int iLen)
{
	byte byType = Conv_BYTE(msg + 6);

	_BaoJianData & stBaoJianData = g_NPC.GetBaoJianData();

	if(byType != 255)
	{
		stBaoJianData.anger = Conv_DWORD(msg + 8);
	}

	if(byType == 255)	//宝鉴无法开启
	{
		if(Conv_BYTE(msg + 7) == 2 || Conv_BYTE(msg + 7) == 3)
		{
			g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 1);		//上次点击无效

			if(Conv_BYTE(msg + 7) == 2)
			{
				g_WooolStoreMgr.SetQuickBuyItem("群英令");
				CQuickBuyData &QuickBuyData = g_WooolStoreMgr.GetQuickBuyData();

				if(QuickBuyData.pItem)
				{
					char str[256] = "";
					sprintf(str,"您的群英令已经用完了，您需要马上购买吗？单价%d元宝",QuickBuyData.pItem->iPrice);
					QuickBuyData.strMsg = str;
					QuickBuyData.iType = 1;
					QuickBuyData.iUseAfterBuyType = 0;					
					g_pControl->PopupWindow(MSG_CTRL_QUICKBUY_WND,OPER_CREATE);						
				}
			}
		}		
	}
	else if(byType == 1)	//第一轮
	{
		stBaoJianData.byCurrentRound = 1;
		stBaoJianData.byFirstPrize = msg[7];
		g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 11);
	}
	else if(byType == 2)	//第二轮
	{
		stBaoJianData.byCurrentRound = 2;
		stBaoJianData.bySecondPrize = msg[7];
		g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 12);
	}
	else if(byType == 3)	//第三轮
	{
		stBaoJianData.byCurrentRound = 3;
		stBaoJianData.byThirdPrize = msg[7];
		stBaoJianData.dwCloseTime = GetTickCount() + 6000;
		g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 13);
	}
	else if(byType == 0)	//所有12个物品
	{
		const char * start = msg + 12;

		for(int i = 0;i < 12 && start - msg < iLen;i++)
		{
			BYTE pos = Conv_BYTE(start);
			start += 1;

			stBaoJianData.prize[pos].name.assign(start,24);
			stBaoJianData.prize[pos].name = ClearNum(stBaoJianData.prize[pos].name);
			start += 24;

			stBaoJianData.prize[pos].looks = Conv_WORD(start);
			start += 2;

			stBaoJianData.prize[pos].dwnum = Conv_DWORD(start);
			start += 4;

			stBaoJianData.prize[pos].wgrade = Conv_WORD(start);
			start += 2;
		}

		stBaoJianData.byCurrentRound = 0;
		stBaoJianData.byFirstPrize = -1;
		stBaoJianData.bySecondPrize = -1;
		stBaoJianData.byThirdPrize = -1;
		stBaoJianData.byAdditionalPrize = -1;

		g_pControl->PopupWindow(MSG_CTRL_BAOJIAN_WND, OPER_RECREATE);
	}
	else if(byType == 4)	//附加物品奖励
	{
		stBaoJianData.byAdditionalPrize = msg[7];
		g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 14);
	}
	else if(byType == 5)	//铁英之怒满了,100点
	{
		stBaoJianData.anger = 100;
		g_pControl->Msg(MSG_CTRL_BAOJIAN_WND,OPER_CUSTOM + 2);		
		stBaoJianData.dwCloseTime = GetTickCount() + 12000;
	}
}

void CGameControl::MSG_YuanbaoSell_Info(const char * msg,int iLen)
{
	g_NPC.GetYuanBaoSellInfo().vecYuanBaoSellInfo.clear();

	byte type = Conv_BYTE(msg + 6);	//1 点NPC 2 刷新
	
	if(type == 1)
	{
		g_pControl->PopupWindow(MSG_CTRL_YUANBAO_WND, OPER_RECREATE,1);
	}

	const char * pStart = msg + 12;

	for(;pStart - msg < iLen;)
	{
		_YuanBaoAllInfo::_YuanBaoSellInfo info;
		info.strUserId.assign(pStart,20);
		pStart += 20;
		info.strPlayerName.assign(pStart,14);
		pStart += 14;
		info.dwYuanBaoCount = Conv_DWORD(pStart);
		pStart += 4;
		info.dwPrice = Conv_DWORD(pStart);
		pStart += 4;

		g_NPC.GetYuanBaoSellInfo().vecYuanBaoSellInfo.push_back(info);
	}	
}

void CGameControl::Send_Buy_YuanBao(string& userId,string& userName,DWORD dwCount,DWORD dwPrice)	//买元宝
{
	SET_COMMAND(CS_YUANBAOBUY,32);
	//ASSIGN_DWORD(0,g_NPC.GetID());
	ADD_ARRAY(userId.c_str(),20);
	ADD_ARRAY(userName.c_str(),14);
	ADD_DWORD(dwCount);
	ADD_DWORD(dwPrice);

	SEND_GAME_SERVER();
}

void CGameControl::Send_MyYuanBao_Query(byte byType)//////////1查询寄售信息2查询可领取金币数量
{
	SET_COMMAND(CS_YUANBAOQUERY,32);

	ASSIGN_BYTE(6,byType);

	SEND_GAME_SERVER();
}

void CGameControl::Msg_MyYuanBao_Query(const char * msg, int iLen)
{
	byte byType = msg[6];

	if(byType == 1)
	{
		g_NPC.GetYuanBaoSellInfo().dwMySellNum = Conv_DWORD(msg);
		g_NPC.GetYuanBaoSellInfo().dwMySellPrice = Conv_DWORD(msg + 8);
		g_pControl->PopupWindow(MSG_CTRL_YUANBAO_WND, OPER_RECREATE,2);
	}
	else if(byType == 2)
	{
		g_NPC.GetYuanBaoSellInfo().dwMoneyToTake = Conv_DWORD(msg + 8);
		g_pControl->PopupWindow(MSG_CTRL_YUANBAO_WND, OPER_RECREATE,3);
	}
}

void CGameControl::Send_OtherYuanBao_Query()		//查询其他人寄售元宝信息
{
	SET_COMMAND(CS_YUANBAOSELL_INFO,32);	

	ASSIGN_BYTE(6,1);

	SEND_GAME_SERVER();
}

void CGameControl::Send_YuanBao_Cancel()		//取消寄售
{
	SET_COMMAND(CS_YUANBAOCANCEL,32);	

	//ASSIGN_DWORD(0,g_NPC.GetID());

	SEND_GAME_SERVER();
}

void CGameControl::Send_YuanBao_Sell(DWORD dwNum,DWORD dwPrice)	//元宝寄售
{
	SET_COMMAND(CS_YUANBAOSELL,32);	

	//ASSIGN_DWORD(0,g_NPC.GetID());
	ASSIGN_WORD(6,(WORD)dwNum);
	ASSIGN_DWORD(8,dwPrice);

	SEND_GAME_SERVER();
}

void CGameControl::Sned_Take_YuanBaoMoney()
{
	SET_COMMAND(CS_TAKEMONEY,32);	

	SEND_GAME_SERVER();
}
/*
0-3:被访入的字符物品指针ID
4.5：协议号
6-9:表示虎符物品指针ID
10:2代表是虎符相关
11:放入位置索引 0-15
*/
void CGameControl::SEND_RUNE_INTO_TIGER(DWORD tigerID, DWORD runeID, BYTE index)
{
	SET_COMMAND(CS_PACKAGE_OBJECT_USE,12);
	ASSIGN_DWORD(0, runeID);
	ASSIGN_DWORD(6, tigerID);
	ASSIGN_BYTE(10,2);
	ASSIGN_BYTE(11,index);
	SEND_GAME_SERVER();
}
/*
功勋值
045e 回复
0－3：升级所需功勋值
6－9：下一级每小时累计的上限
12－15：功勋兑换比例
16－19：声望兑换比例
*/
void CGameControl::Msg_Meritoriousness_Update(const char *msg, int iLen)
{
	g_OtherData.SetMeritoriousnessUpdate(Conv_DWORD(msg),Conv_DWORD(msg + 6),Conv_DWORD(msg + 12),Conv_DWORD(msg + 16));
	g_pControl->PopupWindow(MSG_CTRL_MERITORIOUS_WND,OPER_CREATE,msg[20] > 0?msg[20]:1);
}
/*
0－3：兑换数量  （最后预判一下）
6   ：0：请求数据，1：请求兑换 ,2: 请求升级
10：兑换类型 0：功勋值兑换 1：声望兑换
*/
void CGameControl::Send_Meritoriousness_Update(BYTE requestType,BYTE exchangeType,DWORD number)
{
	SET_COMMAND(CS_MERITORIOUSNESS_UPDATE,12);
	ASSIGN_DWORD(0,number);
	ASSIGN_BYTE(6,requestType);
	ASSIGN_BYTE(10,exchangeType);
	SEND_GAME_SERVER();
}
/*
045f
6   ：1：进攻士兵，2： 守护士兵3: 箭塔
*/
void CGameControl::Send_RTSInstance_Exchange_ByResource(BYTE type)
{
	SET_COMMAND(CS_RTSINSTANCE_EXCHANGE_BYRESOURCE,12);
	ASSIGN_BYTE(6,type);
	SEND_GAME_SERVER();
}

// void CGameControl::Send_Refresh_QunYing_Members()
// {
// 	SET_COMMAND(CS_QUNYING_MEMBER,12);
// 	SEND_GAME_SERVER();
// }

// void CGameControl::MSG_Refresh_QunYing_Members(const char* msg, int iLen)
// {
// 	if(msg[6] == 1)
// 	{
// 		g_NPC.GetSafeQunYingMember().clear();
// 
// 		int start = 12;
// 
// 		while(start < iLen)
// 		{
// 			_QunYingMember member;
// 
// 			member.strName.assign(msg + start,15);
// 			start += 15;
// 
// 			member.byJob = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.byLevel = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.bySex = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.byTimeLevel = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.wPosX = Conv_WORD(msg + start);
// 			start += 2;
// 
// 			member.wPosY = Conv_WORD(msg + start);
// 			start += 2;
// 
// 			g_NPC.GetSafeQunYingMember().push_back(member);
// 		}
// 	}
// 	else if(msg[6] == 2)
// 	{
// 		g_NPC.GetFreeQunYingMember().clear();
// 
// 		int start = 12;
// 
// 		while(start < iLen)
// 		{
// 			_QunYingMember member;
// 
// 			member.strName.assign(msg + start,15);
// 			start += 15;
// 
// 			member.byJob = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.byLevel = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.bySex = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.byTimeLevel = Conv_BYTE(msg + start);
// 			start++;
// 
// 			member.wPosX = Conv_WORD(msg + start);
// 			start += 2;
// 
// 			member.wPosY = Conv_WORD(msg + start);
// 			start += 2;
// 
// 			g_NPC.GetFreeQunYingMember().push_back(member);
// 		}
// 	}
// 
// 	g_pControl->PopupWindow(MSG_CTRL_QUICKQUNYING_WND,OPER_CREATE,1);
// }
// 
//1：觉醒，2：剥离 3：拆分 4:升级
void CGameControl::Send_EquipSoul(WORD type, DWORD dwEquipID,DWORD dwAddGoodID)
{
	SET_COMMAND(CS_EQUIPSOUL,32);

	ASSIGN_DWORD(0,dwEquipID);
	ASSIGN_WORD(6,type);
	
	if (type == 1)
	{
		ASSIGN_DWORD(8,dwAddGoodID);
	}	

	SEND_GAME_SERVER();
}

//1：觉醒，2：剥离 3：拆分 4:升级
void CGameControl::Send_EquipSoulLevelUp(BYTE byUseBind,DWORD dwEquipID,DWORD JingshiID1,DWORD JingshiID2,DWORD JingshiID3,DWORD JingshiID4,DWORD JingshiID5)
{
	SET_COMMAND(CS_EQUIPSOUL,32);

	ASSIGN_DWORD(0,dwEquipID);
	ASSIGN_WORD(6,4);
	ASSIGN_BYTE(8,byUseBind);

	if(JingshiID1)
	{
		ADD_DWORD(JingshiID1);
	}

	if(JingshiID2)
	{
		ADD_DWORD(JingshiID2);
	}

	if(JingshiID3)
	{
		ADD_DWORD(JingshiID3);
	}

	if(JingshiID4)
	{
		ADD_DWORD(JingshiID4);
	}

	if(JingshiID5)
	{
		ADD_DWORD(JingshiID5);
	}

	SEND_GAME_SERVER();
}


void CGameControl::Send_BlessCompound(DWORD dwGoodId1, DWORD dwGoodId2, DWORD dwFu)
{
	SET_COMMAND(CS_COMPOUND_BLESS,32);
	ASSIGN_DWORD(0, dwGoodId1);
	ASSIGN_DWORD(8, dwGoodId2);
	if (dwFu != 0)
	{
		ADD_DWORD(dwFu);
	}
	SEND_GAME_SERVER();
}

void CGameControl::Send_BlessExtend(DWORD dwGoodId, DWORD dwFu )
{
	SET_COMMAND(CS_EXTEND_BLESS,12);
	ASSIGN_DWORD(0, dwGoodId);
	ASSIGN_DWORD(8, dwFu);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_EnterShan()
{
	SET_COMMAND(CS_ENTERSJS,12);
	SEND_GAME_SERVER();
}


//1) 1表示奇遇被触发
//2) 2表示玩家有未完成奇遇，玩家上线时发送
//3) 3表示奇遇被放弃
//4) 4表示奇遇被完成，正常结束
//5) 5表示奇遇完成，等待滚动奖励
void CGameControl::MSG_Notify_Adventure_Status(const char * msg,int iLen)
{
	WORD x = Conv_WORD(msg + 6);
	if (x == 1)
	{
		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
		g_OtherData.SetHaveQiYu(true);
		g_OtherData.SetQiYuState(0);
	}
	else if (x == 2)
	{
		g_OtherData.SetHaveQiYu(true);
		g_OtherData.SetQiYuState(1);
	}
	else if (x == 3)
	{
		g_OtherData.SetHaveQiYu(false);
		g_pControl->PopupWindow(MSG_CTRL_ADVENTURE_WND, OPER_CLOSE);
	}
	else if (x == 4)
	{
		g_OtherData.SetHaveQiYu(false);
	}
	else if (x == 5)
	{
		g_OtherData.SetHaveQiYu(true);
		g_OtherData.SetQiYuState(2);
	}
}

void CGameControl::MSG_Notify_Adventure_Reward(const char * msg,int iLen)
{
	//1:金币,2:物品,3:经验,4:真火值
	WORD x = Conv_WORD(msg + 6);

	if (x == 1)
	{
		DWORD count = Conv_DWORD(msg+8);
		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,10,(CControl*)count);
	}
	else if (x == 3)
	{
		DWORD count = Conv_DWORD(msg+8);
		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,12,(CControl*)count);
	}
	else if (x == 2)
	{
		g_OtherData.GetQiYuRotItems().clear();

		WORD looks = Conv_WORD(msg + 8);
		DWORD itemcount = Conv_DWORD(msg + 12);

		QiYuRotItem item;
		for (int i = 0; i < itemcount && 16+26*i + 26 <= iLen; ++i)
		{
			memset(&item,0,sizeof(item));
			memcpy(&item,msg + 16+26*i,26);

			g_OtherData.GetQiYuRotItems().push_back(item);
		}

		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,11,(CControl*)looks);
	}
	else if (x == 4)
	{
		DWORD count = Conv_DWORD(msg+8);
		g_pControl->Msg(MSG_CTRL_ADVENTURE_WND,10,(CControl*)count);
	}
}

void CGameControl::SEND_Request_Adventrue()
{
	SET_COMMAND(CS_REQUEST_ADVENTURE,12);
	SEND_GAME_SERVER();
}

void CGameControl::SEND_Request_Give_Adventrue_Reward()
{
	SET_COMMAND(CS_REQUEST_GIVE_ADVENTURE_REWARD,12);
	SEND_GAME_SERVER();
}

// dwGoodId1：装备，dwGoodId2：圣战之魂
void CGameControl::SEND_Equip_Compose(DWORD dwGoodId1, DWORD dwGoodId2)
{
	SET_COMMAND(CS_EQUIP_COMPOSE,32);
	ASSIGN_DWORD(0, dwGoodId1);
	ASSIGN_DWORD(8, dwGoodId2);
	SEND_GAME_SERVER();
}

// dwGoodId1：圣战之魂，dwGoodId2：圣战精华
void CGameControl::SEND_Szzp_Compose(DWORD dwGoodId1, DWORD dwGoodId2)
{
	SET_COMMAND(CS_SZZP_COMPOSE,32);
	ASSIGN_DWORD(0, dwGoodId1);
	ASSIGN_DWORD(8, dwGoodId2);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_SANWEIZHENHUOINFO(const char * msg,int iLen)
{
	DWORD playerID = Conv_DWORD(msg);
	BYTE byFirst = Conv_BYTE(msg+6);//第一次领悟
	BYTE byFireLevel = Conv_BYTE(msg+7);

	if (playerID == SELF.GetID())
	{
		BYTE byOldFireLevel = g_OtherData.GetSanWeiZhenHuoInfo().byFireLevel;
		
		if (byFireLevel > 0)
		{
			if (iLen >= 6 + sizeof(SanWeiZhenHuoInfo))
			{
				memcpy(&g_OtherData.GetSanWeiZhenHuoInfo(), msg + 6, sizeof(SanWeiZhenHuoInfo));
			}


			if (byFirst != 0)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_LAN,EMP_MAGIC_NOTARGET,SELF.GetID());
			}
			else if (byOldFireLevel != 0 && byFireLevel > byOldFireLevel)
			{
				if (byFireLevel == 28)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_HONG,EMP_MAGIC_NOTARGET,SELF.GetID());
				}
				else if (byFireLevel == 55)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_JIN,EMP_MAGIC_NOTARGET,SELF.GetID());
				}
				else
				{
					g_pMagicCtrl->CreateMagic(MAGICID_FIRE_LEVELUP,EMP_MAGIC_NOTARGET,SELF.GetID());
				}
			}
		}
		else
		{
			ZeroMemory(&g_OtherData.GetSanWeiZhenHuoInfo(),sizeof(SanWeiZhenHuoInfo));		
		}
	}
	else
	{
		CSimpleCharacterNode *pChar = g_pGameData->FindSimpleCharacter(playerID);
		if(pChar)
		{
			if (byFirst != 0)
			{
				g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_LAN,EMP_MAGIC_NOTARGET,playerID);
			}
			else
			{
				if (byFireLevel == 28)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_HONG,EMP_MAGIC_NOTARGET,playerID);
				}
				else if (byFireLevel == 55)
				{
					g_pMagicCtrl->CreateMagic(MAGICID_JUEXING_ZHENHUO_JIN,EMP_MAGIC_NOTARGET,playerID);
				}
				else
				{
					g_pMagicCtrl->CreateMagic(MAGICID_FIRE_LEVELUP,EMP_MAGIC_NOTARGET,playerID);
				}
			}
		}		
	}
	
}

/*
2. 法宝攻击
offset = 4, 2字节表示消息号0x0A41
offset = 0, 4字节表示玩家ID
offset = 6, 1字节表示法宝WeaponLooks
offset = 7, 1字节表示法宝五行
offset = 8, 1字节表示法宝强化等级
offset = 9, 1字节表示法宝飞行轨迹
offset = 12, 4字节表示目标ID
*/
void CGameControl::MSG_FABAOATTACK(const char * msg,int iLen)
{
	if (iLen < 16)
	{
		return;
	}
	DWORD dwOwnerID = Conv_DWORD(msg);
	BYTE  wFaBaoType = Conv_BYTE(msg + 6);
	BYTE wWuxing = Conv_BYTE(msg + 7);
	DWORD dwTargetID = Conv_DWORD(msg + 12);
	BYTE  byQianghuaLevel = Conv_BYTE(msg + 8);	
	BYTE  byFlyingTrack = Conv_BYTE(msg + 9);	

	CSimpleCharacter * pOwner = g_pGameData->FindCharacterByID(dwOwnerID);

	CSimpleCharacter * pTarget = g_pGameData->FindCharacterByID(dwTargetID);

	if (pOwner == NULL || pTarget == NULL)
	{
		return;
	}
	int ownX,ownY;
	int tarX,tarY;

	pOwner->GetXY(ownX,ownY);
	pTarget->GetXY(tarX,tarY);

	int distant = max(abs(ownX - tarX),abs(ownY - tarY));
	
	if (wFaBaoType < 1)
	{
		return;
	}

	//强化等级0~5:白色 6~8:蓝色 9:金
	int EffColor = byQianghuaLevel == 9?3:(byQianghuaLevel >= 6?2:1);

	if ( distant > 4 /*|| (rand()%10) < 1*/)//一定用远距离的
	{
		g_pMagicCtrl->CreateMagic(wFaBaoType*2 + MAGICID_FABAO_START,0,dwOwnerID,dwTargetID);
		g_pMagicCtrl->CreateMagic(wWuxing + MAGICID_FABAOTRACK_SHUXING_WU_YUAN,0,dwOwnerID,dwTargetID);
		g_pMagicCtrl->CreateMagic((wFaBaoType - 1)*6 + MAGICID_FABAO_ATTACK_EFFECT_START + EffColor + 3,0,dwOwnerID,dwTargetID);
	}
	else 
	{
		g_pMagicCtrl->CreateMagic(wFaBaoType*2 - 1 + MAGICID_FABAO_START,0,dwOwnerID,dwTargetID);
		g_pMagicCtrl->CreateMagic(wWuxing + MAGICID_FABAOTRACK_SHUXING_WU,0,dwOwnerID,dwTargetID);
		g_pMagicCtrl->CreateMagic((wFaBaoType - 1)*6 + MAGICID_FABAO_ATTACK_EFFECT_START + EffColor,0,dwOwnerID,dwTargetID);
	}

}
/*
offset = 4, 2字节表示消息号0x0A43
offset = 0, 4字节表示玩家ID
offset = 6, 2字节表示怪物坐标X
offset = 8, 2字节表示怪物坐标Y
offset = 10, 1字节表示魔灵类别，1～5分别代表金木水火土
*/

void CGameControl::MSG_FABAOAXILING(const char * msg,int iLen)
{
	DWORD dwOwnerID = Conv_DWORD(msg);
	WORD wx = Conv_WORD(msg + 6);
	WORD wy = Conv_WORD(msg + 8);
	BYTE type = Conv_BYTE(msg + 10);

	g_pMagicCtrl->CreateMagic(MAGICID_FABAOATTACK_XILING_WU+type,EMP_MAGIC_NOOWNER,MAKELONG(wx,wy),dwOwnerID);
}
/*
offset = 4, 2字节表示消息号0x0A45
offset = 0, 4字节表示玩家ID
offset = 6, 4字节表示法宝的ID
offset = 10, 1字节表示装备还是卸下，1为装备，2为卸下
offset = 11, 1字节表示结果，0表示成功，其余表示失败
offset = 12, 2字节表示法宝WeaponLooks
offset = 14, 1字节表示法宝强化等级
offset = 15, 一个字节表示位置
0a45要改，最后添加一个字节表示位置
0a45  11为增加一种返还结果  5  表示已装备过此类法宝
*/

void CGameControl::MSG_ZHUANGBEIFABAO(const char * msg,int iLen)
{

	if (iLen < 15)
	{
		return;
	}

	DWORD playerID = Conv_DWORD(msg);
	DWORD fabaoID = Conv_DWORD(msg + 6);
	BYTE type = Conv_BYTE(msg + 10);
	BYTE result = Conv_BYTE(msg + 11);
	DWORD weaponLooks = Conv_WORD(msg + 12);
	BYTE qianghuaLevel = Conv_BYTE(msg + 14);
	BYTE EquipPos = Conv_BYTE(msg + 15);

	//CGood* goods = NULL;

	if (result == 0)
	{
		CSimpleCharacter * pOwner = g_pGameData->FindCharacterByID(playerID);
		if (type == 1 && pOwner)//装备成功
		{
			 
			//goods  = &(SELF.GetEquipTemp());
			if (EquipPos == 0)
			{
				pOwner->SetFaBaoType(weaponLooks,qianghuaLevel,1);
			}
			else if (EquipPos == 1)
			{
				pOwner->SetLeftFaBaoType(weaponLooks);
			}
			else
			{
				pOwner->SetRightFaBaoType(weaponLooks);
			}
			
			if (SELF.GetID() == playerID)
			{
				CGood& tmp = SELF.GetEquipTemp();

				if (tmp.GetID() != fabaoID)
				{
					return;
				}

				g_BoothData.CleanItem(tmp.GetID()); //清除掉

				if(tmp.GetID() != 0)    //判断装备是否存在
				{
					int iPos = SELF.GetEquipTempPos();
					if(iPos >= 0)
					{
						SELF.GetEquipGood(iPos).Assign(tmp);
						SELF.GetEquipTemp().SetID(0);
					}
				}
			}
			
		} 
		else if(type == 2 && pOwner)//卸下成功
		{
			if (EquipPos == 0)
			{
				pOwner->SetFaBaoType(0);
			}
			else if (EquipPos == 1)
			{
				pOwner->SetLeftFaBaoType(0);
			}
			else
			{
				pOwner->SetRightFaBaoType(0);
			}
		
			if (SELF.GetID() == playerID && fabaoID == SELF.GetEquipTemp().GetID())
			{
				SELF.GetEquipTemp().SetID(0);
			}
		}
	}
	//else if(result == 5)
	//{
	//	//..装备过此类法宝
	//} 
	else
	{
		if (type == 1 && SELF.GetID() == playerID)//装备失败
		{
			CGood& tmp = SELF.GetEquipTemp();
			if(tmp.GetID() == 0 || tmp.GetID() != fabaoID) //没有装备物品
				return;

			int pos = tmp.GetPos();
			CGood* pGood = SELF.PackageGood().GetPtr(pos);
			if(pGood && pGood->GetID() == 0)
			{
				pGood->Assign(tmp);
			}
			else
			{
				pGood = SELF.PackageGood().FindGood(0); //找到空位
				if(pGood)
				{
					pGood->Assign(tmp);
				}
			}

			tmp.SetID(0);
		}
		else if(type == 2 && SELF.GetID() == playerID)//卸下失败
		{
			int iEquipPos = SELF.GetEquipTempPos();
			if(iEquipPos < 0 || iEquipPos >= MAX_EQUIPMENT || SELF.GetEquipTemp().GetID() != fabaoID)
				return;

			SELF.GetEquipGood(iEquipPos) = SELF.GetEquipTemp();
			SELF.GetEquipTemp().SetID(0);
		}
	}

	/*if (g_pControl->FindWindowByName("FaBaoShowWnd"))
		g_pControl->MsgToWnd("FaBaoShowWnd",MSG_CTRL_FABAOSHOW_UPDATE);*/
}

void CGameControl::SEND_GoLingWu()
{
	SET_COMMAND(CS_GOLINGWU,12);
	SEND_GAME_SERVER();
}

void CGameControl::MSG_NPC_LINGWUSANWEIZHENHUO(const char * msg,int iLen)
{
	DWORD playerID = Conv_DWORD(msg);
	WORD result = Conv_WORD(msg+6);
	if (0 == result)
	{
		if (playerID == SELF.GetID())
		{
			g_pMagicCtrl->CreateMagic(MAGICID_LINGWU_CHENGYUN,EMP_MAGIC_NOTARGET,SELF.GetID());
		}
		else
		{
			g_pMagicCtrl->CreateMagic(MAGICID_LINGWU_CHENGYUN,EMP_MAGIC_NOTARGET,playerID);
		}
		
	}
}

void CGameControl::Send_FabaoChaifen(DWORD dwGoodId)
{
	SET_COMMAND(CS_FABAOCHAIFEN,12);

	ASSIGN_DWORD(0,dwGoodId);

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Request_FaBaoRenZhu(DWORD dwGoodId)
{
	SET_COMMAND(CS_FABAO_RENZHU,12);

	ASSIGN_DWORD(0,dwGoodId);

	SEND_GAME_SERVER();
}

void CGameControl::SEND_Request_FaBaoZhuangBei(CGood & m ,int nPos)
{
	if(m.GetID() == 0)	return;

	SET_COMMAND(CS_FABAO_ZHUANGBEI,12);

	ASSIGN_DWORD(6,m.GetID());
	
	ASSIGN_BYTE(10,nPos == ITEM_POS_FABAO/*?0:nPos == ITEM_POS_FABAOLEFT?1:2*/);

	int i = SEND_GAME_SERVER();

	if (i > 0)
	{
		SELF.SetEquipTempPos(nPos);
		SELF.GetEquipTemp().Assign(m);
		m.SetID(0);
	}

}
//6-7 行数
//8-9 列数
//10-11 是否是测试 1:测试
void CGameControl::SEND_FireArtifice(int iGridCount, WORD test)
{	
	if (iGridCount < 3 || iGridCount > 6)
		return;


	SET_COMMAND(CS_FIREARTIFICE,256);
	
	ASSIGN_WORD(6,iGridCount);
	ASSIGN_WORD(8,iGridCount);

	ASSIGN_WORD(10,test);

	for (int j = 0; j < iGridCount; ++j)
	{
		for (int i = 0; i < iGridCount; ++i)
		{
			DWORD dwID = g_NPC.GetFireArtGood(i,j).GetID();
			ADD_DWORD(dwID);
		}
	}

	SEND_GAME_SERVER();

}

//0-4 物品外观
//6-7 测试结果(0:成功, 1:错误的物品指针，2:系统错误)
//8-11 所需金币数目
//协议体是 物品名称
void CGameControl::MSG_FireArtifice(const char * msg,int iLen)
{	
	WORD result = Conv_WORD(msg+6);

	if (result == 0)
	{
		FireArtificeResult ret;

		ret.wLooks = Conv_WORD(msg);
		ret.iPercent = Conv_WORD(msg+2);
		ret.dwNeedMoney = Conv_DWORD(msg+8);
		ret.strGoodName.assign(msg+12,iLen-12);

		g_pControl->MsgToWnd("FireArtificeWnd",MSG_CTRL_FIREARTIFICE_WND,1000,(CControl*)&ret);
	}
}

void CGameControl::MSG_FireArtificeTime(const char * msg,int iLen)
{
	DWORD time = Conv_DWORD(msg);

	g_OtherData.SetFireArtTime(time);
	g_OtherData.SetFireArtStartTime(GetTickCount());
	g_pControl->MsgToWnd("FireArtificeWnd", MSG_CTRL_FIREARTIFICE_WND, 1001);
}

void CGameControl::Send_Upgrade_FaBao(DWORD dwGoodMainFaBao, DWORD dwGoodSecondFaBao1, DWORD dwGoodSecondFaBao2, 
									  DWORD dwGoodSecondFaBao3, DWORD dwGoodFu)
{
	SET_COMMAND(CS_UPGRADE_GOOD,32);

	ASSIGN_DWORD(0,dwGoodMainFaBao);

	ASSIGN_DWORD(8,dwGoodFu);

	if(dwGoodSecondFaBao1)
	{
		ADD_DWORD(dwGoodSecondFaBao1);
	}

	if(dwGoodSecondFaBao2)
	{
		ADD_DWORD(dwGoodSecondFaBao2);
	}

	if(dwGoodSecondFaBao3)
	{
		ADD_DWORD(dwGoodSecondFaBao3);
	}

	SEND_GAME_SERVER();
}


////g_NPC购物图形化，弹出快捷买卖、修理窗口
//void CGameControl::MSG_Npc_Quick_Exchange(const char* msg,int iLen)
//{
//	DWORD dwID = Conv_DWORD(msg);//g_NPC指针,或者取回的物品指针
//	WORD wCount = Conv_WORD(msg+6);// 物品数目
//	BYTE byType = Conv_BYTE(msg+8);//0:为npc快捷卖物列表,1:npc快捷存取列表
//
//	BYTE byGoodLen = BYTE(msg[9]);//单个物品的长度
//	if (byGoodLen == 0)
//	{
//		byGoodLen = CGood::PKLength();
//	}
//
//	WORD wStorageMaxCount = Conv_WORD(msg+10);//在wType=1时表示包裹容量
//
//	if(byType == 0 || byType == 1)//快捷卖物列表及快捷存取列表用的都是g_NPC.GetNpcQuickVector()
//	{
//		g_NPC.SetID(dwID);
//		g_NPC.GetNpcQuickVector().clear();
//		if(byType == 1)
//			g_NPC.SetStorageMaxCount(wStorageMaxCount);
//
//		int iPos = 12;
//		for(WORD i = 0; i < wCount && iPos < iLen; i++,iPos += byGoodLen)
//		{
//			CGood temp;
//			if(byType == 0)
//				temp.FromBuffer(msg+iPos,true,byGoodLen);
//			else 
//				temp.FromBuffer(msg+iPos,false,byGoodLen);
//
//			g_NPC.GetNpcQuickVector().push_back(temp);
//		}
//
//		g_pControl->PopupWindow(MSG_CTRL_NPC_QUICK_WND,OPER_CREATE,byType + 1);
//	}
//	else if(byType == 2)//2存物品成功,添加新物品
//	{
//		CGood temp;
//		temp.FromBuffer(msg+12,false,byGoodLen);
//		g_NPC.GetNpcQuickVector().push_back(temp);
//
//		g_NPC.GetGood().SetID(0);	
//		g_pControl->MsgToWnd(MSG_CTRL_NPC_QUICK_WND,MSG_CTRL_NPC_QUICK_WND,201);
//	}
//	else if(byType == 3)//取仓库物品成功
//	{
//		std::vector<CGood> &npcquick = g_NPC.GetNpcQuickVector();
//		std::vector<CGood>::iterator iter;
//		for(iter = npcquick.begin(); iter != npcquick.end(); iter++)
//		{
//			if((*iter).GetID() == dwID)
//			{
//				npcquick.erase(iter);
//				break;
//			}
//		}
//
//		g_NPC.GetGood().SetID(0);
//		g_pControl->MsgToWnd(MSG_CTRL_NPC_QUICK_WND,MSG_CTRL_NPC_QUICK_WND,201);
//	}
//
//	g_NPC.SetExchangeFromQuickNpcWnd(false);
//
//}

/**
S->C 047f  仓库个数以及位置信息
6－7：类型 0：表示仓库容量 1：表示物品位置
8－9: 仓库最大容量
10：11：仓库物品数量
类型为1是：
物品位置属性（同0046协议，5byte表示一个物品）
*/
void CGameControl::MSG_New_Storage_Info(const char* msg,int iLen)
{
	WORD wType = Conv_WORD(msg+6);// 物品数目

	WORD wStorageMaxCount = Conv_WORD(msg+8);//仓库容量
	g_NPC.SetStorageMaxCount(wStorageMaxCount);
	WORD wStorageGoodsCount = Conv_WORD(msg+10);//仓库物品数量

	if (wType == 1)
	{
		int iCount = min(wStorageGoodsCount,MAXNUMBERSTORAGE);

		std::vector<DWORD> illegalGoodId;
						
		DWORD* posArray = CNpcQuickWnd::GetGoodPos();
		memset(posArray, 0, sizeof(DWORD)*(MAXNUMBERSTORAGE));
		for(int iNum = 0; iNum < iCount; iNum++)						// 接受服务器端的位置信息		
		{
			BYTE pos = Conv_BYTE(msg+16+iNum*5);
			DWORD id = Conv_DWORD(msg+12+iNum*5);
			
			if (pos < 0 || pos >= MAXNUMBERSTORAGE || posArray[pos] != 0)
			{
				illegalGoodId.push_back(id);
			}
			else
				posArray[pos] = id;
		}
		
		//pos不正确的物品,从前往后找空位放入
		for (std::vector<DWORD>::iterator itrId = illegalGoodId.begin(); itrId != illegalGoodId.end(); ++itrId)
		{
			for (int ii = 0; ii < MAXNUMBERSTORAGE; ++ii)
			{
				if (posArray[ii] == 0)
				{
					posArray[ii] = *itrId;
					break;
				}
			}
		}


		//保持原有数据结构
		//对仓库物品增加位置信息
		//for(int iNum = 0; iNum < iCount; iNum++)						// 接受服务器端的位置信息		
		//{
		//	std::vector<CGood>& vGood = g_NPC.GetNpcQuickVector();
		//	for (std::vector<CGood>::iterator itr = vGood.begin(); itr != vGood.end(); ++itr)
		//	{
		//		if (PosArray[iNum].dwID == (*itr).GetID())
		//		{
		//			(*itr).SetPos(PosArray[iNum].iPos);
		//			break;
		//		}
		//	}
		//}

		//SAFE_DELETE_ARRAY(PosArray);
	}
	//g_pControl->PopupWindow(MSG_CTRL_NPC_QUICK_WND,OPER_CREATE,2);
}

//0x04B6
void CGameControl::MSG_YiHuoZhanMsg(const char * msg,int iLen)
{
	S_YiHuoZhanMsg& yihuomsg = g_OtherData.GetYiHuoZhanMsg();
	yihuomsg.iShowTime = Conv_WORD(msg + 6);
	yihuomsg.iShowTime *= 1000;
	yihuomsg.iStartTime = GetTickCount();

	memset(yihuomsg.szType,0,sizeof(yihuomsg.szType));
	memset(yihuomsg.szBtn1,0,sizeof(yihuomsg.szBtn1));
	memset(yihuomsg.szBtn2,0,sizeof(yihuomsg.szBtn2));

	memcpy(yihuomsg.szType, msg + 12, 10);
	yihuomsg.szType[9] = '\0';

	memcpy(yihuomsg.szBtn1, msg + 22, 10);
	yihuomsg.szBtn1[9] = '\0';

	memcpy(yihuomsg.szBtn2, msg + 32, 10);
	yihuomsg.szBtn2[9] = '\0';

	yihuomsg.strMsg.assign(msg+42,iLen-42);

	g_pControl->PopupWindow(MSG_CTRL_YIHUOZHANMSG_WND,OPER_CREATE);
	g_pControl->MsgToWnd("YiHuoZhanMsgWnd",MSG_CTRL_YIHUOZHANMSG_WND,10);
}

void CGameControl::SEND_YiHuoZhanMsg(BYTE btn)
{
	SET_COMMAND(CS_OPEN_YIHUOZHENGDUOWND,32);

	ASSIGN_BYTE(6,btn);	

	S_YiHuoZhanMsg& yihuomsg = g_OtherData.GetYiHuoZhanMsg();
	ADD_STR(yihuomsg.szType);	

	SEND_GAME_SERVER();
}

/*
GS->Client
1.玩家连击次数通知
offset = 0, 4字节表示玩家ID
offset = 4, 2字节表示消息号0x0a55
offset = 6, 4字节表示当前连击杀怪计数
offset = 10, 1字节表示XP是否启动，1表示启动，其他表示未启动
offset = 11, 1字节表示间隔时间
*/
void CGameControl::MSG_LianJi_Show(const char * msg,int iLen)
{
	DWORD uID = Conv_DWORD(msg);
	DWORD LianJiNumber = Conv_DWORD(msg+6);
	BYTE  XPStart = Conv_BYTE(msg + 10);
	BYTE  interTime = Conv_BYTE(msg + 11);

	DWORD addTime = interTime*1000 + 500;

	int x = 0;
	int y = 0;

	UINT oid = 0;

	if(uID == SELF.GetID())
	{
		SELF.GetXY(x,y);

		if(g_pGfx->GetWidth() == 800)
		{
			oid = MAKELONG(x+5,y+1);
		}
		else if(g_pGfx->GetWidth() == 1024)
		{
			oid = MAKELONG(x+6,y-2);
		}
		else
		{
			oid = MAKELONG(x+8,y-2);
		}

		//if (SELF.GetCareer() == 0)//战士15秒,其他12秒
		//{
		//	addTime = 15500;
		//} 

		DWORD curTime = GetTickCount();
		//SELF.SetLianJiNumber(LianJiNumber);
		if (LianJiNumber > SELF.GetRealLianJiNumber())
		{
			DWORD startFrame = 0;
			
			if (curTime < SELF.GetLastLianJiStartTime())
			{
				startFrame = (SELF.GetLastLianJiStartTime() - curTime)/30;
			}

			DWORD CurLianJiNumber = LianJiNumber - SELF.GetRealLianJiNumber();

			for (int i = 0; i < CurLianJiNumber; ++i)
			{
				Magic_Show_s* ms = NULL;
				if(g_pGfx->GetWidth() == 800)
				{
					ms = g_pMagicCtrl->CreateMagic(MAGICID_LIANJI_FEIXING_800,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,uID,oid);
				}
				else
				{
					ms = g_pMagicCtrl->CreateMagic(MAGICID_LIANJI_FEIXING,EMP_MAGIC_NOTARGET|EMP_MAGIC_SERVER_ADVISE,uID,oid);
				}
				if (ms)
				{
					ms->iCycles = (WORD)(startFrame + i * 2);
					ms->ref->wData[0] = (WORD)SELF.GetRealLianJiNumber() + 1 + i;
				}
			}

			SELF.SetLianJiShowEndTime(curTime + addTime);
			SELF.SetRealLianJiNumber(LianJiNumber);
			SELF.SetLastLianJiStartTime(curTime + (CurLianJiNumber * 2 + startFrame) * 30);
		}
		else//跳转gs
		{
			SELF.SetLianJiShowEndTime(curTime + addTime);
			SELF.SetRealLianJiNumber(LianJiNumber);
			SELF.SetLianJiNumber(LianJiNumber);
			SELF.SetLastLianJiStartTime(curTime);
		}
		
	}
	else 
	{
		CSimpleCharacterNode * p = g_pGameData->FindSimpleCharacter(uID);

		//if (p && p->GetCareer() == 0)//战士15秒,其他12秒
		//{
		//	addTime = 15000;
		//} 

		if(p) 
		{
			//p->SetLianJiNumber(LianJiNumber);
			//p->SetLianJiShowEndTime(GetTickCount() + addTime);
			p->GetXY(x,y);
		}
	}

	if (XPStart == 1)
	{
		for (int dir = 0; dir != 8; ++dir)
		{
			g_pMagicCtrl->CreateMagic(MAGICID_LIANJI_XP,EMP_MAGIC_NOTARGET,uID,0,dir*2);
		}

		//随机掉落陨石5-8
		//水平 +-8 垂直 +-12
		
		int number = 8 + (rand()%4);

		for (int i = 0; i != number; ++i)
		{
			Magic_Show_s* ms = g_pMagicCtrl->CreateMagic(MAGICID_YUNSHI_DIAOLUO,EMP_MAGIC_NOOWNER|EMP_MAGIC_NOTARGET,0,MAKELONG(x + rand()%16 - 8,y + rand()%24 - 12));
			ms->iCycles = rand()%16;
		}

	}
}


void CGameControl::SEND_Equip_Chaifen(DWORD dwGoodId, BYTE type)
{
	SET_COMMAND(CS_EQUIP_CHAIFEN,CMD_SIZE);
	ASSIGN_DWORD(0,dwGoodId);
	ASSIGN_BYTE(6,type);
	SEND_GAME_SERVER();
}

/*
下标6为测试结果
0:成功，并会带信息
2:非法的物品
3:系统忙
4:不能分解成魂
5:系统忙
*/
void CGameControl::MSG_Equip_Chaifen(const char * msg,int iLen)
{
	BYTE  res = Conv_BYTE(msg + 6);

	string buf;
	switch (res)
	{
	case 0:
		buf.assign(msg+12,iLen-12);
		break;
	case 2:
		buf = "<color=graygolden 非法物品>";
		break;
	case 3:
		buf = "<color=graygolden 系统忙，请稍后再试！>";
		break;
	case 4:
		buf = "<color=graygolden 该物品不能分解成魂>";
		break;
	case 5:
		buf = "<color=graygolden 系统忙，请稍后再试！>";
		break;
	default:
		buf = "<color=graygolden 系统忙，请稍后再试！>";
		break;
	}

	g_pControl->MsgToWnd("EquipChaifenWnd",MSG_CTRL_EQUIPCHAIFEN_WND,100,(CControl*)(buf.c_str()));
}

/*
0-3 装备ID
6-7 操作类型
1:刷新属性
2:保留属性)
8-11 附加物品
04b7
*/
void CGameControl::SEND_HufuFlush(BYTE type, DWORD dwGoodId, DWORD dwAddGoodId)
{
	SET_COMMAND(CS_HUFU_FLUSH,CMD_SIZE);

	ASSIGN_DWORD(0,dwGoodId);
	ASSIGN_BYTE(6,type);
	ASSIGN_DWORD(8,dwAddGoodId);

	SEND_GAME_SERVER();
}

//0x04B7
//DC1 DC2 MC1 MC2 SC1 SC2 AC1 AC2 MAC1 MAC2 暴击 反弹 吸血 绝对防御 破防
void CGameControl::MSG_YHufuFlush(const char * msg,int iLen)
{
	SChiJinRetPacket lChiJinRet;
	memset(&lChiJinRet, 0, sizeof(SChiJinRetPacket));

	int retlen = iLen - 12;
	retlen = min(retlen,sizeof(SChiJinRetPacket));

	memcpy(&lChiJinRet,msg+12,retlen);

	g_pControl->MsgToWnd("HuFuFlushWnd", MSG_CTRL_HUFUFLUSH_WND, 10, (CControl*)(&lChiJinRet));
}



void CGameControl::MSG_PLAYER_HORSEMANSHIP_INFO(const char * msg,int iLen)
{
	HorsemanshipProtocalInfo& HorseManShipInfo = g_OtherData.GetHorseManShipInfo();

	HorseManShipInfo.clear();


	DWORD uID = Conv_DWORD(msg);
	
	int count = 0;
	int CurrPos = 12;
	bool bRecreate = false;

	if (iLen >= 12 + 16 + sizeof(HorsemanshipTotalEffect))
	{
		if (HorseManShipInfo.Grade == 0)
		{
			bRecreate = true;
		}

		HorseManShipInfo.Grade = Conv_INT(msg + CurrPos);
		CurrPos += 4;
		HorseManShipInfo.Exp = Conv_DWORD(msg+CurrPos);
		CurrPos += 4;
		HorseManShipInfo.MaxExp = Conv_DWORD(msg+CurrPos);
		CurrPos += 4;

		memcpy(&(HorseManShipInfo.effect),msg+CurrPos,sizeof(HorsemanshipTotalEffect));
		CurrPos += sizeof(HorsemanshipTotalEffect);
		
		count = Conv_INT(msg + CurrPos);
		CurrPos += 4;
	}

	if (count > 0)
	{
		for (int i = 0; i != count; ++i)
		{
			if (CurrPos + sizeof(HorsemanshipProtocalSkill) <= iLen)
			{
				HorsemanshipProtocalSkill hps;
				memcpy(&hps,msg + CurrPos,sizeof(HorsemanshipProtocalSkill));
				HorseManShipInfo.vecSkill.push_back(hps);
				CurrPos += sizeof(HorsemanshipProtocalSkill);
			}
		}
	}

	if (bRecreate)
	{
		if (g_pControl->FindWindowByName("AvatarWnd"))
		{
			g_pControl->PopupWindow("AvatarWnd",OPER_RECREATE);
		}
		else if (g_pControl->FindWindowByName("FaBaoShowWnd"))
		{
			g_pControl->PopupWindow(MSG_CTRL_FABAOSHOW_WND,OPER_RECREATE);
		}

		g_pControl->MsgToWnd("SkillWnd",MSG_CTRL_SKILLWND,10);
	}

}


/*
0,训练
1,取经验	
2,提纯
3,搀杂
4,查看
*/
void CGameControl::Send_HorseManShip_Watch_Req(BYTE type, const char * strName, DWORD dwGoodId)
{
	SET_COMMAND(CS_PLAYER_HORSEMANSHIP_WATCH_REQ,64);

	ASSIGN_DWORD(0,dwGoodId);
	ASSIGN_BYTE(6,type);

	ADD_STR(strName);

	SEND_GAME_SERVER();
}

void CGameControl::MSG_HorseManShip_Watch(const char * msg,int iLen)
{	
	if (iLen < 12 + 11)
		return;

	char szName[16] = {0};
	memcpy(szName, msg+25, 16);

	
	WORD  bOpenWnd = Conv_WORD(msg + 6);

	HorsemanshipWatchInfo* pInfo = NULL;
	if (strcmp(szName, SELF.GetName()) == 0)
		pInfo = &g_NPC.GetHorsemanshipWatchInfo();
	else
		pInfo = &g_NPC.GetOtherHorsemanshipWatchInfo();

	pInfo->clear();
	strcpy(pInfo->m_szPlayerName, szName);
	pInfo->m_bFlag = Conv_BYTE(msg+12);
	pInfo->m_wBasePure = Conv_WORD(msg+13);
	pInfo->m_wPure = Conv_WORD(msg+15);
	pInfo->m_dwTrainTime = Conv_DWORD(msg+17);
	g_NPC.GetHorsemanshipWatchInfo().m_dwActionPoint = Conv_DWORD(msg+21);//行动点都是自己的

	pInfo->m_dwTrainClientStartTime = GetTickCount();

	int count = Conv_INT(msg+41);

	if (count > 0)
	{
		pInfo->m_event.reserve(count);

		int curpos = 45;
		for (int i = 0; i < count && curpos < iLen; ++i)
		{
			HmsWatchEventProtocal levent;
			memcpy(&levent, msg+curpos, sizeof(HmsWatchEventProtocal));
			curpos += sizeof(HmsWatchEventProtocal);

			pInfo->m_event.push_back(levent);
		}
	}
	

	if (bOpenWnd)
		g_pControl->PopupWindow(MSG_CTRL_YUSHOULEVELUP_WND,OPER_CREATE);

	g_pControl->MsgToWnd("YuShouLevelUpWnd",MSG_CTRL_YUSHOULEVELUP_WND,11);
	g_pControl->MsgToWnd("YuShouLevelUpWnd",MSG_CTRL_YUSHOULEVELUP_WND,12);
}

void CGameControl::MSG_HorseManShip_ActionPoint(const char * msg,int iLen)
{
	HorsemanshipWatchInfo& hInfo = g_NPC.GetHorsemanshipWatchInfo();
	hInfo.m_dwActionPoint = Conv_DWORD(msg + 6);
}

