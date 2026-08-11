///////////////////////////////////////////////////////////////////////
//文件名：Input.cpp
//文件说明：I/O键盘、鼠标
//暂时不使用dinput因为不方便处理DClick消息
///////////////////////////////////////////////////////////////////////

#include "Global/Global.h"
#include "Global/GlobalMsg.h"
#include "Input.h"
#include "GameClient/ReplayManager.h"
#include "Global/Interface/GraphicInterface.h"

#include <tchar.h>
#include <commctrl.h>
#include <basetsd.h>

#define TIMER_INPUTCHECK			1
#define INPUTCHECK_TIME				500

#define MAX_WINDOW_WIDTH            2560
#define MAX_WINDOW_HEIGHT           1440

//////////////////////////////////////////////////////////////////////
// 结构析构函数
//////////////////////////////////////////////////////////////////////

CInput::CInput()
{
	//默认的初始化
	m_bMove			= false;
	m_bWheel		= true;
	m_bDClick		= false;
	m_bButtonDown	= true;
	m_bButtonUp		= true;

	m_bChar			= false;
	m_bKeyDown		= true;
	m_bKeyUp		= true;

	m_bEnable		= false;

	m_bLButtonDown	= false;
	m_bRButtonDown	= false;
	m_bMButtonDown	= false;
	m_bLDClickDown	= false;
	m_bRDClickDown	= false;

	ZeroMemory(m_cKeyState,sizeof(BYTE)*256);   // 清空键盘状态表
	ClearMsgBuffer();                           // 清空消息队列缓冲
}

CInput::~CInput()
{
	Unacquire();
}


//////////////////////////////////////////////////////////////////////
// 消息函数 uMsg:消息类型  wParam:WORD类型参数 lParam:long类型参数
//////////////////////////////////////////////////////////////////////
bool CInput::WndMessage(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	bool bHaveDeal = true;
	switch(uMsg) 
	{		
	// 鼠标消息
	case WM_MOUSEMOVE:										// 鼠标移动消息
		m_iX = LOWORD(lParam);								// 更新鼠标X位置
		m_iY = HIWORD(lParam);								// 更新鼠标Y位置
		if( m_bMove ) 
			AddMsgBuffer(MSG_INPUT_MOVE,(DWORD)lParam);		// 往消息队列加入MSG_INPUT_MOVE
		break;
	case WM_MOUSEWHEEL:										//鼠标滚轮消息
		if( m_bWheel ) 
			AddMsgBuffer(MSG_INPUT_WHEEL,(short)(HIWORD(wParam)));	//往消息队列加入MSG_INPUT_WHEEL
		break;
	case WM_CAPTURECHANGED:									//鼠标移出窗口区
		//AddMsgBuffer(MSG_INPUT_CAPTURECHANGED,0);			//往消息队列加入MSG_INPUT_CAPTURECHANGED
		break;
	case WM_LBUTTONDOWN:                         //鼠标左键按下
		if( m_bRButtonDown ) break;
		if( m_bButtonDown && !m_bLButtonDown ) 
			AddMsgBuffer(MSG_INPUT_LEFTBT_DOWN,(DWORD)lParam);       //往消息队列加入MSG_INPUT_BUTTON1_DOWN
		m_bLButtonDown = true;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			SetCapture(g_hWnd);
		}
		break;
	case WM_RBUTTONDOWN:                        //鼠标右键按下
		if( m_bLButtonDown ) break;
		if( m_bButtonDown && !m_bRButtonDown ) 
			AddMsgBuffer(MSG_INPUT_RIGHTBT_DOWN,(DWORD)lParam);       //往消息队列加入MSG_INPUT_BUTTON2_DOWN
		m_bRButtonDown = true;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			SetCapture(g_hWnd);
		}
		break;
	case WM_MBUTTONDOWN:                         //鼠标中键按下
		if( m_bButtonDown && !m_bMButtonDown  ) 
			AddMsgBuffer(MSG_INPUT_MIDDLEBT_DOWN,(DWORD)lParam);       //往消息队列加入MSG_INPUT_BUTTON3_DOWN
		m_bMButtonDown = true;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			SetCapture(g_hWnd);
		}
		break;
	case WM_LBUTTONUP:                           //鼠标左键弹起
		//if( m_bButtonUp && (m_bLButtonDown || m_bLDClickDown) ) 
		if( m_bButtonUp && m_bLButtonDown) //如果是双击没有必要发第二次MSG_INPUT_LEFTBT_UP的消息
			AddMsgBuffer(MSG_INPUT_LEFTBT_UP,(DWORD)lParam);         //往消息队列加入MSG_INPUT_BUTTON1_UP
		m_bLButtonDown = false;
		m_bLDClickDown = false;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			ReleaseCapture();
		}
		break;
	case WM_RBUTTONUP:                           //鼠标右键弹起
		if( m_bButtonUp && (m_bRButtonDown || m_bRDClickDown) )
			AddMsgBuffer(MSG_INPUT_RIGHTBT_UP,(DWORD)lParam);         //往消息队列加入MSG_INPUT_BUTTON2_UP
		m_bRButtonDown = false;
		m_bRDClickDown = false;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			ReleaseCapture();
		}
		break;
	case WM_MBUTTONUP:                           //鼠标中键弹起
		if( m_bButtonUp && m_bMButtonDown ) 
			AddMsgBuffer(MSG_INPUT_MIDDLEBT_UP,(DWORD)lParam);         //往消息队列加入MSG_INPUT_BUTTON3_UP
		m_bMButtonDown = false;
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			ReleaseCapture();
		}
		break;
	case WM_LBUTTONDBLCLK:                       //鼠标左键双击                  
		if( m_bLDClickDown ) break;
		if( m_bDClick ) 
			AddMsgBuffer(MSG_INPUT_LEFTBT_DCLICK,(DWORD)lParam);     //往消息队列加入MSG_INPUT_BUTTON1_DCLICK
		m_bLDClickDown = true;
		break;
	case WM_RBUTTONDBLCLK:                       //鼠标右键双击
		if( m_bRDClickDown ) break;
		if( m_bDClick ) 
			AddMsgBuffer(MSG_INPUT_RIGHTBT_DCLICK,(DWORD)lParam);     //往消息队列加入MSG_INPUT_BUTTON2_DCLICK
		m_bRDClickDown = true;
		break;

	// 键盘消息
	case WM_SYSKEYDOWN:
		AddMsgBuffer(MSG_INPUT_KEYDOWN,MAKELONG((WORD)wParam,0));
		break;

	case WM_KEYDOWN:                             //键盘按下
		// 假如键盘可以接收输入的话，就向消息队列添加消息.
		if( m_bKeyDown )
			AddMsgBuffer(MSG_INPUT_KEYDOWN,MAKELONG((WORD)wParam,0));
		break;

	case WM_SYSKEYUP:
		AddMsgBuffer(MSG_INPUT_KEYUP,MAKELONG((WORD)wParam,0));
		break;

	case WM_KEYUP:                            //按键弹起,减少引用计数                
		if( m_bKeyUp )                        //假如可以接收键盘弹起消息，就向消息队列添加消息
			AddMsgBuffer(MSG_INPUT_KEYUP,MAKELONG((WORD)wParam,0));
		break;

	//字符输入消息
	case WM_CHAR:                         
		if( m_bChar ) 
			AddMsgBuffer(MSG_INPUT_CHAR,MAKELONG((WORD)wParam,0));
		break;
		
	// 激活消息接收
	case WM_ACTIVATE:
		IsActive(LOWORD(wParam));
		break;
	// 设置最大窗口
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
			//pInfo->ptMaxTrackSize.x = 1050;
			//pInfo->ptMaxTrackSize.y = 800;
			//pInfo->ptMaxSize.x		= 1050;
			//pInfo->ptMaxSize.y		= 800;

			pInfo->ptMaxTrackSize.x = MAX_WINDOW_WIDTH + 40;
			pInfo->ptMaxTrackSize.y = MAX_WINDOW_HEIGHT + 40;
			pInfo->ptMaxSize.x		= MAX_WINDOW_WIDTH + 40;
			pInfo->ptMaxSize.y		= MAX_WINDOW_HEIGHT + 40;
		}
		break;

	// 定时检查
	case WM_TIMER:
		if( wParam == TIMER_INPUTCHECK)
		{
#ifdef ENABLE_REPLAY
			if (!g_ReplayManager.IsInReplay())
#endif
			{
				if(m_bKeyDown)
				{
					for(BYTE cVk = VK_F1; cVk <= VK_F8; cVk++)
					{
						if((::GetKeyState(cVk) & 0xFF80) == 0xFF80)
						{
							WndMessage(WM_KEYDOWN,cVk,0);
							break;
						}
					}
				}
			}
		}
		break;
	default:
		bHaveDeal = false;
		break;
	}

#ifdef ENABLE_REPLAY
	if(g_ReplayManager.IsInRecord() && bHaveDeal && g_pGfx)
	{
		g_ReplayManager.SaveInput(g_pGfx->GetFrameCount(),uMsg,(DWORD)wParam,(DWORD)lParam);
	}
#endif

	return false;
}


//////////////////////////////////////////////////////////////////////
// 属性函数
//////////////////////////////////////////////////////////////////////

//激活键盘字符输入
void CInput::EnableCharMsg(bool bChar)
{ 
	m_bChar = bChar;
}
//激活键盘按键接收功能
void CInput::EnableKeyDownMsg(bool bMsg)
{
	m_bKeyDown = bMsg;
}
//激活键盘按键弹起接收功能
void CInput::EnableKeyUpMsg(bool bMsg)
{
	m_bKeyUp = bMsg;
}
//激活接收鼠标移动功能
void CInput::EnableMoveMsg(bool bMsg)
{
	m_bMove = bMsg;
}
//激活接收鼠标滑轮功能
void CInput::EnableWheelMsg(bool bMsg)
{
	m_bWheel = bMsg;
}
//激活鼠标双击接收功能
void CInput::EnableDClickMsg(bool bMsg)
{
	m_bDClick = bMsg;
}
//激活接收鼠标按键功能
void CInput::EnableButtonDownMsg(bool bMsg)
{
	m_bButtonDown = bMsg;
}
//激活接收鼠标键弹起功能
void CInput::EnableButtonUpMsg(bool bMsg)
{
	m_bButtonUp = bMsg;
}


//////////////////////////////////////////////////////////////////////
// 输入状态函数
//////////////////////////////////////////////////////////////////////

bool CInput::Acquire()
{
#ifdef ENABLE_REPLAY
	if (!g_ReplayManager.IsInReplay())
#endif
	{
		// 获得键盘状态
		UpdateKeyState();

		// 获得鼠标状态
		//UpdateMouseState(0,0);

		// 设置定时检查
		SetTimer(g_hWnd,TIMER_INPUTCHECK,INPUTCHECK_TIME,NULL);
	}

	m_bEnable = true;
	return true;
}

bool CInput::Unacquire()
{
#ifdef ENABLE_REPLAY
	if (!g_ReplayManager.IsInReplay())
#endif
	{
		KillTimer(g_hWnd,TIMER_INPUTCHECK);
		m_bEnable = false;
	}

	return true;
}

// 判断是否需要释放控制权
void CInput::IsActive(WORD af)
{
#ifdef ENABLE_REPLAY
	if (!g_ReplayManager.IsInReplay())
#endif
	{
		if(af == WA_INACTIVE) 
			Unacquire();
		else
			Acquire();
	}
}


//////////////////////////////////////////////////////////////////////
//Input状态函数
//////////////////////////////////////////////////////////////////////

// 更新键盘即时状态
void CInput::UpdateKeyState()
{
	if( m_bEnable ) GetKeyboardState(m_cKeyState);
}

// 判断键是否被按下，参数为机器扫描码
bool CInput::IsKeyDown(BYTE vkey)
{
	return ((::GetKeyState(vkey) & 0x0100) != 0);
}

// 更新鼠标即时状态
void CInput::UpdateMouseState(int cltx,int clty)
{
	if(m_bEnable)					//假如处于可接收状态就接收鼠标位置
	{
#ifdef ENABLE_REPLAY
		if (!g_ReplayManager.IsInReplay())
#endif
		{
			POINT p;
			this->GetCursorPos(&p);			// 取得系统鼠标位置

			m_iX = p.x - cltx;		//更新成员数据
			m_iY = p.y - clty;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// 数据获取函数
//////////////////////////////////////////////////////////////////////

bool CInput::IsCapsLock()
{
	return ( (::GetKeyState(VK_CAPITAL) & 0x0100) != 0 );
}

bool CInput::IsNumLock()
{ 
	return ( (::GetKeyState(VK_NUMLOCK) & 0x0100) != 0 );
}

bool CInput::IsShift()
{
#ifdef ENABLE_REPLAY
	if(!g_ReplayManager.IsInReplay())
#endif
	{
		bool bShift = ( (::GetKeyState(VK_SHIFT) & 0x0100) != 0 );

#ifdef ENABLE_REPLAY
		if(g_ReplayManager.IsInRecord() && g_ReplayManager.IsReplayShiftKeyDown() != bShift && g_pGfx)
		{
			g_ReplayManager.SaveKeyState(g_pGfx->GetFrameCount(),1,bShift);			
		}
#endif

		return bShift;
	}
#ifdef ENABLE_REPLAY
	else
	{
		return g_ReplayManager.IsReplayShiftKeyDown();
	}
#endif

}

bool CInput::IsCtrl()
{ 
#ifdef ENABLE_REPLAY
	if(!g_ReplayManager.IsInReplay())
#endif
	{
		bool bCtrl = ( (::GetKeyState(VK_CONTROL) & 0x0100) != 0 );

#ifdef ENABLE_REPLAY
		if(g_ReplayManager.IsInRecord() && g_ReplayManager.IsReplayCtrlKeyDown() != bCtrl && g_pGfx)
		{
			g_ReplayManager.SaveKeyState(g_pGfx->GetFrameCount(),2,bCtrl);
		}
#endif

		return bCtrl;
	}
#ifdef ENABLE_REPLAY
	else
	{
		return g_ReplayManager.IsReplayCtrlKeyDown();
	}	
#endif
}

bool CInput::IsAlt()
{
#ifdef ENABLE_REPLAY
	if(!g_ReplayManager.IsInReplay())
#endif
	{
		bool bAlt = ( (::GetKeyState(VK_MENU) & 0x0100) != 0 );

#ifdef ENABLE_REPLAY
		if(g_ReplayManager.IsInRecord() && g_ReplayManager.IsReplayAltKeyDown() != bAlt && g_pGfx)
		{
			g_ReplayManager.SaveKeyState(g_pGfx->GetFrameCount(),3,bAlt);
		}
#endif

		return bAlt;
	}
#ifdef ENABLE_REPLAY
	else
	{
		return g_ReplayManager.IsReplayAltKeyDown();
	}
#endif
}

bool CInput::IsEscape()
{
#ifdef ENABLE_REPLAY
	if(!g_ReplayManager.IsInReplay())
#endif
	{
		bool bEscape = ( (::GetKeyState(VK_ESCAPE) & 0x0100) != 0 );

#ifdef ENABLE_REPLAY
		if(g_ReplayManager.IsInRecord() && g_ReplayManager.IsReplayEscapeKeyDown() != bEscape && g_pGfx)
		{
			g_ReplayManager.SaveKeyState(g_pGfx->GetFrameCount(),4,bEscape);
		}
#endif

		return bEscape;
	}
#ifdef ENABLE_REPLAY
	else
	{
		return g_ReplayManager.IsReplayEscapeKeyDown();
	}	
#endif
}

bool CInput::IsLeftButton()
{
	return m_bLButtonDown;
}

bool CInput::IsRightButton()
{
	return m_bRButtonDown;
}

bool CInput::IsMiddleButton()
{
	return m_bMButtonDown;
}

//////////////////////////////////////////////////////////////////////
// 消息缓冲函数
//////////////////////////////////////////////////////////////////////

// 添加新消息到消息循环
bool CInput::AddMsgBuffer(DWORD msg,DWORD data)
{
	int i;

	i = m_iEnd + 1;                 //移动到下一位置
	if( i >= INPUT_MSG_BUFFERSIZE ) //假如填满了就移开头位置
		i = 0;
	if( i == m_iStart )             //队列里面没有消息就直接返回
		return false;

	m_sBuffer[m_iEnd].msg	= msg;  //保存消息类型
	m_sBuffer[m_iEnd].data	= data; //保存消息数据
	m_iEnd = i;                     //更新消息存放索引值
	return true;             
}

// 从消息循环里取出一个消息
bool CInput::GetMsgBuffer(DWORD *msg,DWORD *data)
{
	if( m_iStart == m_iEnd ) 
		return false;                        //消息队列为空，直接返回

	*msg	= m_sBuffer[m_iStart].msg;       //取出消息类型
	*data	= m_sBuffer[m_iStart].data;      //取出消息数据
	m_sBuffer[m_iStart].msg  = 0;            //清空
	m_sBuffer[m_iStart].data = 0;

	m_iStart++;                              //丢弃该消息
	if( m_iStart >= INPUT_MSG_BUFFERSIZE )   //如果索引值大于最大数值就重新定位
		m_iStart = 0;
	return true;
}

// 清除键盘缓冲区
void CInput::ClearMsgBuffer()
{
	m_iStart	= 0;	//设置开始索引为0
	m_iEnd		= 0;	//设置结束索引为0
	ZeroMemory(m_sBuffer,sizeof(SInputBuffer)*INPUT_MSG_BUFFERSIZE);    //清空内存
}

BOOL CInput::GetCursorPos( LPPOINT lpPoint )
{
#ifdef ENABLE_REPLAY
	if(g_ReplayManager.IsInReplay())
	{
		lpPoint->x = m_iX + g_iMainWndX;
		lpPoint->y = m_iY + g_iMainWndY;
		return true;
	}
	else
#endif
	{
		return ::GetCursorPos(lpPoint);
	}
}