#include "Global/GlobalErr.h"
#include "Global/GlobalMsg.h"
#include "Control.h"
#include "UiManager.h"
#include "CtrlWindowX.h"

#ifdef _DEBUG
#include "global/new.h"
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DTI_IMPLEMENT(CControlContainer, CControl)
CControlContainer::CControlContainer()
{
	m_iControlMode	= CTRL_MODE_CONTAINER;
	m_pControls		= NULL;
	m_pLastDrawControls = NULL;
	m_pDrawControl = NULL;

	m_ptFroneTexPos.x = m_ptFroneTexPos.y = 0;
}

// 此控件被摧毁时，会自动释放所有的子控件
CControlContainer::~CControlContainer()
{TRY_BEGIN
	CControl *p, *p2; 
	
	p = m_pControls;
	while( p )
	{
		//SET_CONTROL_NAME(p->GetDTIName());
		p2 = p;
		p = p->GetControlNext();
		SAFE_DELETE(p2);
	}
	m_pControls = NULL;

	p = m_pLastDrawControls;
	while( p )
	{
		//SET_CONTROL_NAME(p->GetDTIName());
		p2 = p;
		p = p->GetControlNext();
		SAFE_DELETE(p2);
	}
	m_pLastDrawControls = NULL;
TRY_END}


//////////////////////////////////////////////////////////////////////
// Message functions
//////////////////////////////////////////////////////////////////////

// 判断是否是子控件的消息
CControl * CControlContainer::IsSonsMsg(DWORD dwMsg,DWORD dwData,CControl * pControl)
{TRY_BEGIN

	CControl * pCtrl = m_pLastDrawControls;
	while( pCtrl )
	{
		if( pCtrl->IsEnable() && pCtrl->Msg(dwMsg,dwData,pControl) ) 
			return pCtrl;
		//SET_CONTROL_NAME(pCtrl->GetDTIName());
		pCtrl = pCtrl->GetControlNext();
	}

	pCtrl = m_pControls;
	while( pCtrl )
	{
		if( pCtrl->IsEnable() && pCtrl->Msg(dwMsg,dwData,pControl) ) 
			return pCtrl;
		//SET_CONTROL_NAME(pCtrl->GetDTIName());
		pCtrl = pCtrl->GetControlNext();
	}

TRY_END
return NULL;
}

// 判断是否是子控件
BOOL CControlContainer::IsSonCtrl(CControl *pControl)
{
	CControl * pCtrl = m_pControls;
	while(pCtrl)
	{
		if(pCtrl == pControl) 
			return TRUE;
		pCtrl = pCtrl->GetControlNext();
	}
	return FALSE;
}

// 容器被移动意味着子控件都要被移动
void CControlContainer::OnMove()
{
	CControl::OnMove();

	CControl * pCtrl = m_pControls;
	while( pCtrl )
	{
		pCtrl->OnMove();
		pCtrl = pCtrl->GetControlNext();
	}	

	pCtrl = m_pLastDrawControls;
	while( pCtrl )
	{
		pCtrl->OnMove();
		pCtrl = pCtrl->GetControlNext();
	}
}


//////////////////////////////////////////////////////////////////////
// Manange Controls functions
//////////////////////////////////////////////////////////////////////

// 增加控件
bool CControlContainer::AddControl(CControl * pCtrl,int iPos,bool bAddToLast)
{TRY_BEGIN

	CControl ** ppFirstControl = &m_pControls;

	if(bAddToLast)
	{
		ppFirstControl = &m_pLastDrawControls;
	}
    
	CControl *p, *prev;
	int i;

	p = *ppFirstControl;
	i = 0;
	prev = NULL;

	while( p && !p->IsNoChangeLevel() && iPos > 0) 
	{
		//SET_CONTROL_NAME(p->GetDTIName());
		if( i == iPos ) break;
		//如果该控件已经存在，那么把它移动最前，这样就能在最上面绘制
		if(pCtrl == p)
		{
			iPos = -1;
			CControl *pNext = p->GetControlNext();
			if( prev ) 
				prev->SetControlNext(pNext);
			if( pNext )	
				pNext->SetControlPrev(prev);

			if( *ppFirstControl == p ) 
				*ppFirstControl = p->GetControlNext();

			(*ppFirstControl)->SetControlPrev(pCtrl);
			pCtrl->SetControlNext(*ppFirstControl);
			pCtrl->SetControlPrev(NULL);
			*ppFirstControl = pCtrl;
			return true;
		}
		else
			prev = p;

		p = p->GetControlNext();
		i++;
	}

	//如果pCtrl不是模态的,则不应该放在模态的之上
	if(pCtrl && !pCtrl->GetMode())
	{
		while(p && p->GetMode()) 
		{
			prev = p;
			p = p->GetControlNext();
		}
	}

	if( prev ) prev->SetControlNext(pCtrl);
	if( p )	p->SetControlPrev(pCtrl);
	pCtrl->SetControlPrev(prev);
	pCtrl->SetControlNext(p);
	if( *ppFirstControl == p ) *ppFirstControl = pCtrl;

	if(pCtrl && IsFadeIn())
	{
		pCtrl->SetFadeIn(IsFadeIn());
	}
TRY_END
return true;
}

// 删除控件
bool CControlContainer::RemoveControl(CControl** pCtrl,bool bRemoveFromLastDraw)
{TRY_BEGIN
   
   if(pCtrl == NULL || *pCtrl == NULL)
	   return false;

	CControl ** ppFirstControl = &m_pControls;

	if(bRemoveFromLastDraw)
	{
		ppFirstControl = &m_pLastDrawControls;
	}

	CControl* p = *ppFirstControl;
	while( p ) 
	{
		//SET_CONTROL_NAME(p->GetDTIName());
		if( p == *pCtrl ) 
		{
			if( p->GetControlPrev() ) (p->GetControlPrev())->SetControlNext(p->GetControlNext());
			if( p->GetControlNext() ) (p->GetControlNext())->SetControlPrev(p->GetControlPrev());
			if( p == *ppFirstControl ) *ppFirstControl = p->GetControlNext();
			//在Draw()里被删除应该对m_pDrawControl重新赋值,否则就出现野指针
			if( p == m_pDrawControl)
			{
				m_pDrawControl = p->GetControlPrev();
			}

			SAFE_DELETE(p);
			*pCtrl = NULL;
			return true;
		}
		p = p->GetControlNext();
	}
TRY_END
return false;
}

// 为位置为索引删除控件
bool CControlContainer::RemoveControl(int iPos)
{TRY_BEGIN

	CControl * p = FindControl(iPos);
	if(p)
	{
		//SET_CONTROL_NAME(p->GetDTIName());
		if( p->GetControlPrev() ) (p->GetControlPrev())->SetControlNext(p->GetControlNext());
		if( p->GetControlNext() ) (p->GetControlNext())->SetControlPrev(p->GetControlPrev());
		if( p == m_pControls ) m_pControls = p->GetControlNext();
		//在Draw()里被删除应该对m_pDrawControl重新赋值,否则就出现野指针
		if( p == m_pDrawControl) 
		{
			m_pDrawControl = p->GetControlPrev();
		}

		SAFE_DELETE(p);
	} 
TRY_END
return false;
}

// 查找控件
CControl* CControlContainer::FindControl(int iPos)
{
	CControl * p = m_pControls;
	int i = 0;
	while( p )
	{
		if( i == iPos )
			return p;
		p = p->GetControlNext();
		i++;
	}
	return NULL;
}

// 根据名字查找控件
CControl* CControlContainer::FindControlByName(const char* name,eCtrlMode filter)
{TRY_BEGIN
	if(!name || strlen(name) == 0)
		return NULL;

	CControl * p = m_pControls;
	while( p )
	{
		//SET_CONTROL_NAME(p->GetDTIName());

		if(filter == CTRL_MODE_ALL || filter == p->GetControlMode())
		{
			if(stricmp(p->GetName(),name) == 0)
				return p;
		}
		p = p->GetControlNext();
	}
TRY_END
return NULL;
}

//////////////////////////////////////////////////////////////////////
// Draw functions
//////////////////////////////////////////////////////////////////////

// 自绘后让子控件也显示出来
void CControlContainer::Draw()
{
TRY_BEGIN
	CControl::Draw();
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}

	//DrawTexture(GetFrontTexX(), GetFrontTexY(), m_pTex);

	if( !m_pControls ) return;

	m_pDrawControl = m_pControls;
	while(true) 
	{
		if(!m_pDrawControl->GetControlNext()) 
			break;

		//szLastControlName	= p->GetDTIName()?p->GetDTIName():"CHILD_NOTYPE";
		m_pDrawControl = m_pDrawControl->GetControlNext();
	}

	CControl *pOld = m_pDrawControl;
	while(m_pDrawControl) 
	{
		//m_pDrawControl->Draw()中可能把m_pDrawControl析构了,删除的时候会自动赋值成m_pDrawControl->GetControlPrev()
		pOld = m_pDrawControl;
		if(m_pDrawControl->IsShow())
		{
		  TRY_BEGIN
				m_pDrawControl->Draw();
		        if (pOld == m_pDrawControl)//m_pDrawControl->Draw()有可能把自己析构掉了
		        {
					m_pDrawControl->AfterDraw();
		        }

		  TRY_END_DO_JUMP(
			{
				if(m_pDrawControl)
				{
					char szTemp[512]={0};
					sprintf(szTemp,"DrawControlName:%s,DrawControlDTIName:%s,ThisDTIName:%s",m_pDrawControl->GetName(),m_pDrawControl->GetDTIName(),this->GetDTIName());
					strAddMsg += szTemp;
					SaveException(__FILE__,__FUNCTION__,__LINE__,pSec,strAddMsg.c_str());

					if(m_pDrawControl->GetControlMode() == CTRL_MODE_GAMEWND)
					{
						DetailOutput("GameWnd::Draw() error!");
					}
					else if(m_pDrawControl == pOld)//在m_pDrawControl->Draw里m_pDrawControl发生变化，如m_pDrawControl被关闭删除了
					{
						CCtrlWindowX * pXmlWnd = dynamic_cast<CCtrlWindowX *>(m_pDrawControl);
						if(pXmlWnd && strlen(pXmlWnd->GetName()) > 0)
						{
							g_pControl->DELETECONTROL(&m_pDrawControl);
						}
						else
						{
							RemoveControl(&m_pDrawControl);
						}
					}
				}
				return;
			}
		  )
		}
		//在m_pDrawControl->Draw里m_pDrawControl没有发生变化，如果发生变化了就是原来的m_pDrawControl被删除了，删除的时候会自动赋值成m_pDrawControl->GetControlPrev()
		//这里就不用再取前一个，否则就会漏掉一下没有绘制
		if(m_pDrawControl == pOld)
		{
			m_pDrawControl = m_pDrawControl->GetControlPrev();
		}
	}

	TRY_END_DO(
		char szTemp[256]={0};
	    sprintf(szTemp,"DrawControlName:%s,DrawControlDTIName:%s,ThisDTIName:%s",m_pDrawControl?m_pDrawControl->GetName():"",m_pDrawControl?m_pDrawControl->GetDTIName():"",this->GetDTIName());
		strAddMsg = szTemp;
	)
}

void CControlContainer::AfterDraw()
{
	//背景图片还没有下载下来,其它的东西不画
	if (m_iOriginalWidth <= 0 || m_iOriginalHeight <= 0 || m_iWidth <= 0 || m_iHeight <= 0)
	{
		return;
	}

	if(m_pLastDrawControls)
	{
		m_pDrawControl = m_pLastDrawControls;

		while(true) 
		{
			if(!m_pDrawControl->GetControlNext()) 
				break;

			m_pDrawControl = m_pDrawControl->GetControlNext();
		}

		CControl *pOld = m_pDrawControl;
		while(m_pDrawControl) 
		{
			//m_pDrawControl->Draw()中可能把m_pDrawControl析构了,删除的时候会自动赋值成m_pDrawControl->GetControlPrev()
			pOld = m_pDrawControl;
			if(m_pDrawControl->IsShow())
			{				
				m_pDrawControl->Draw();				
			}

			//在m_pDrawControl->Draw里m_pDrawControl没有发生变化，如果发生变化了就是原来的m_pDrawControl被删除了，删除的时候会自动赋值成m_pDrawControl->GetControlPrev()
			//这里就不用再取前一个，否则就会漏掉一下没有绘制
			if(m_pDrawControl == pOld)
			{
				m_pDrawControl = m_pDrawControl->GetControlPrev();
			}
		}
	}
}

void CControlContainer::ResetControlPos()
{
	//先改变自己的位置
	CControl::ResetControlPos();

	////如果该控件允许缩放,再改变子控件的位置
	//if(!m_bScale)
	//	return;

	CControl * p = m_pControls;
	while(p) 
	{
		p->ResetControlPos();
		p = p->GetControlNext(); 
	}
}

bool CControlContainer::SwitchToTop(CControl * pCtrl,bool bNeedCallOnSwitchToTop)
{
	if( pCtrl && pCtrl != m_pControls ) 
	{
		CControl * p = m_pControls;

		while( p )
		{
			if( p == pCtrl )
			{
				if( p->GetControlPrev() ) (p->GetControlPrev())->SetControlNext(p->GetControlNext());
				if( p->GetControlNext() ) (p->GetControlNext())->SetControlPrev(p->GetControlPrev());
				break;
			}
			p = p->GetControlNext();
		}

		if( p )
		{	
			pCtrl->SetControlPrev(NULL);
			pCtrl->SetControlNext(m_pControls);
			m_pControls->SetControlPrev(pCtrl);
			m_pControls = pCtrl;
			if (bNeedCallOnSwitchToTop)
			{
				m_pControls->OnSwitchToTop();
			}
			return true;
		}		
	}

	return false;
}
