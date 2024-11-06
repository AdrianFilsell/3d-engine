
// Includes
#include "pch.h"
#include "splitterwnd.h"
#include "rasterizer.h"
#include "mainfrm.h"
#include <algorithm>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(splitterwnd, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CAPTURECHANGED()
	ON_MESSAGE(WM_TITLEWND_MINIMISE,OnTitleWndMinimise)
END_MESSAGE_MAP()

splitterwnd::splitterwnd()
{
	m_nHT=-1;
	m_nDraggingHT=-1;
	m_cpDragBegin={0,0};
	m_rcDragBeginClientRect.SetRectEmpty();
	m_dwInSizing=0;
	m_bInitialisingLayout=false;
}

void splitterwnd::OnSize(UINT uiFlags,int nCX,int nCY)
{
	CWnd::OnSize( uiFlags, nCX, nCY );

	if(theApp.GetMainFrame()->getinsizing())
	{
		if(m_dwInSizing != theApp.GetMainFrame()->getinsizing())
		{
			m_dwInSizing=theApp.GetMainFrame()->getinsizing();
			getdragbeginrepos();
		}
		
		recalclayout(true);
	}
	else
		recalclayout(false);
}

void splitterwnd::OnPaint(void)
{
	CPaintDC dc( this );

	auto i=m_vPanes.cbegin(),end=m_vPanes.cend();
	for(;i!=end;++i)
		dc.ExcludeClipRect((*i)->getrect());
	
	CRect rc;
	GetClientRect(rc);

	dc.FillSolidRect(rc,::GetSysColor(COLOR_3DFACE));
	
	POINT p;
	::GetCursorPos(&p);
	::MapWindowPoints(NULL,m_hWnd,&p,1);

	CRect rcTmp;
	const int nHT=m_nDraggingHT==-1?hittest(p):m_nDraggingHT;
	i=m_vPanes.cbegin();
	for(;(i+1)!=end;++i)
		drawborder(&dc,(*i)->getbarrect(false),std::distance(m_vPanes.cbegin(),i)==nHT);
}

BOOL splitterwnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void splitterwnd::OnMouseMove(UINT uiFlags,CPoint point)
{
	CWnd::OnMouseMove( uiFlags, point );

	if(m_nDraggingHT==-1)
	{
		const int nHT=hittest(point);
		if(m_nHT==nHT)
			return;
		
		if(nHT!=-1)
			invalidatebar(nHT);

		if(m_nHT!=-1)
			invalidatebar(m_nHT);

		if(nHT==-1 && GetCapture()==this)
			ReleaseCapture();
		else
		if(nHT!=-1 && GetCapture()!=this)
			SetCapture();
		
		m_nHT=nHT;
	}
	else
	{
		invalidatebar(m_nDraggingHT);
		const int nDY=point.y-m_cpDragBegin.y;
		recalclayout(m_nDraggingHT,m_vDragBeginRepos,nDY,m_rcDragBeginClientRect.Height());
		invalidatebar(m_nDraggingHT);
	}
}

void splitterwnd::OnLButtonDown(UINT uiFlags,CPoint point)
{
	CWnd::OnLButtonDown(uiFlags,point);

	const int nHT=hittest(point);
	if(m_nDraggingHT==-1 && nHT!=-1)
	{
		m_nDraggingHT=nHT;
		m_cpDragBegin=point;
		getdragbeginrepos();
		if(GetCapture()!=this)
			SetCapture();
	}
}

void splitterwnd::OnLButtonUp(UINT uiFlags,CPoint point)
{
	CWnd::OnLButtonUp(uiFlags,point);

	if(m_nDraggingHT!=-1)
	{
		enddrag();
		m_nHT=hittest(point);
		if(m_nHT!=-1 && GetCapture()!=this)
			SetCapture();
	}
}

BOOL splitterwnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	POINT p;
	::GetCursorPos(&p);
	::MapWindowPoints(NULL,m_hWnd,&p,1);

	const int nHT=hittest(p);
	if(m_nDraggingHT!=-1 || nHT!=-1)
	{
		HCURSOR h = LoadCursor(NULL,IDC_SIZENS);
		SetCursor(h);
		return TRUE;
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void splitterwnd::OnCaptureChanged(CWnd* pWnd)
{
	CWnd::OnCaptureChanged(pWnd);

	if(pWnd!=this)
		enddrag();
}

LPARAM splitterwnd::OnTitleWndMinimise(WPARAM wParam,LPARAM lParam)
{
	int nPaneID=0;
	switch(lParam)
	{
		case IDC_SCENE_TITLE:nPaneID=IDC_FRAMES_PANE;break;
		case IDC_FACE_TITLE:nPaneID=IDC_FACES_PANE;break;	
		case IDC_LIGHT_TITLE:nPaneID=IDC_LIGHT_PANE;break;	
		case IDC_MATERIAL_TITLE:nPaneID=IDC_MATERIALS_PANE;break;	
	}
	splitterwndpane *p=getpane(nPaneID);
	if(p)
		setminimise(p,!(wParam==0));

	return 0;
}

bool splitterwnd::getindex(const splitterwndpane *p,int& n)const
{
	auto i=m_vPanes.begin(),end=m_vPanes.end();
	for(n=0;i!=end;++i,++n)
		if((*i).get()==p)
			return true;
	return false;
}

splitterwndpane *splitterwnd::getpane(const int nID)
{
	auto i=m_vPanes.begin(),end=m_vPanes.end();
	for(;i!=end;++i)
		if((*i)->getid()==nID)
			return (*i).get();
	return nullptr;
}

void splitterwnd::initlayout(void)
{
	if(!m_vPanes.size())
		return;

	CRect rc;
	GetClientRect(rc);
	const int nAvailable=rc.Height()-(static_cast<int>(m_vPanes.size())-1)*splitterwndpane::m_nBarHeight;

	int nMinHeight=0,nTop=0;
	auto i=m_vPanes.cbegin(),end=m_vPanes.cend();
	std::vector<std::pair<int,int>> vBeginRepos;
	vBeginRepos.resize(m_vPanes.size());
	auto j=vBeginRepos.begin();
	for(int n=0;i!=end;++i,++j,++n)
	{
		(*j)={nTop,(*i)->getmin()};
		nMinHeight+=(*j).second;
		nTop+=(*j).second+splitterwndpane::m_nBarHeight;
	}
	
	m_bInitialisingLayout=true;
	const int nPanes = static_cast<int>(m_vPanes.size());
	recalclayout(nPanes-1,vBeginRepos,nAvailable-nMinHeight,rc.Height());

	m_bInitialisingLayout=false;
	m_dwInSizing=theApp.GetMainFrame()->getinsizing();
}

void splitterwnd::setminimise(splitterwndpane *pPane,const bool b)
{
	int nPane;
	if(!pPane || !getindex(pPane,nPane))
		return;

	CRect rcClientRect;
	GetClientRect(rcClientRect);

	const int nPanes = static_cast<int>(m_vPanes.size());
	std::vector<std::pair<int,int>> vRepos;
	gettopheight(vRepos);

	if(b)
	{
		pPane->setmaximise(pPane->getrect().Height());
		const int nMin=pPane->getmin()==-1?0:pPane->getmin();
		
		int nGrow=vRepos[nPane].second-nMin;
		for(int n=nPane+1;n<nPanes && nGrow>0;++n)
		{
			const int nCanGrow=m_vPanes[n]->getminimised()?0:m_vPanes[n]->getmax()==-1?nGrow:m_vPanes[n]->getmax()-vRepos[n].second;
			const int nDelta=std::max<>(0,std::min<>(nCanGrow,nGrow));
			if(nDelta)
			{
				nGrow-=nDelta;
				vRepos[n].second+=nDelta;
			}
		}
		
		vRepos[nPane].second=nMin;

		settop(vRepos);
		reposlayout(vRepos);
	}
	else
	{
		const int nClientAvailable=rcClientRect.Height()-(vRepos[vRepos.size()-1].first+vRepos[vRepos.size()-1].second);

		const int nDelta=std::max<>(0,std::min<>(pPane->getmaximise()-vRepos[nPane].second,nClientAvailable));
		vRepos[nPane].second+=nDelta;
		
		int nShrink=pPane->getmaximise()-vRepos[nPane].second;
		for(int n=nPane+1;n<nPanes && nShrink>0;++n)
		{
			const int nCanShrink=m_vPanes[n]->getminimised()?0:m_vPanes[n]->getmin()==-1?vRepos[n].second:vRepos[n].second-m_vPanes[n]->getmin();
			const int nDelta=std::max<>(0,std::min<>(nCanShrink,nShrink));
			if(nDelta)
			{
				const int nMin=m_vPanes[n]->getmin()==-1?0:m_vPanes[n]->getmin();
				if(vRepos[n].second-nDelta==nMin && m_vPanes[n]->getmaximise())
				{
					// auto rollup/minimise
					m_vPanes[n]->setmaximise(vRepos[n].second);
					m_vPanes[n]->setminimised(true);
				}

				nShrink-=nDelta;
				vRepos[n].second-=nDelta;
				vRepos[nPane].second+=nDelta;
			}
		}
		for(int n=nPane-1;n>=0 && nShrink>0;--n)
		{
			const int nCanShrink=m_vPanes[n]->getminimised()?0:m_vPanes[n]->getmin()==-1?vRepos[n].second:vRepos[n].second-m_vPanes[n]->getmin();
			const int nDelta=std::max<>(0,std::min<>(nCanShrink,nShrink));
			if(nDelta)
			{
				const int nMin=m_vPanes[n]->getmin()==-1?0:m_vPanes[n]->getmin();
				if(vRepos[n].second-nDelta==nMin && m_vPanes[n]->getmaximise())
				{
					// auto rollup/minimise
					m_vPanes[n]->setmaximise(vRepos[n].second);
					m_vPanes[n]->setminimised(true);
				}

				nShrink-=nDelta;
				vRepos[n].second-=nDelta;
				vRepos[nPane].second+=nDelta;
			}
		}

		settop(vRepos);
		reposlayout(vRepos);
	}
}

int splitterwnd::hittest(const CPoint& p) const
{
	auto i=m_vPanes.cbegin(),end=m_vPanes.cend();
	for(;i!=end;++i)
		if((*i)->getbarrect(true).PtInRect(p))
			return static_cast<int>(std::distance(m_vPanes.cbegin(),i));
	return -1;
}

void splitterwnd::recalclayout(const bool bMainFrm)
{
	const int nPanes = static_cast<int>(m_vPanes.size());
	if(nPanes==0)
		return;

	if(bMainFrm)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		const int nDY=rcClient.Height()-m_rcDragBeginClientRect.Height();
		recalclayout(nPanes-1,m_vDragBeginRepos,nDY,m_rcDragBeginClientRect.Height());
	}
	else
	{
		std::vector<std::pair<int,int>> vRepos;
		gettopheight(vRepos);
		
		reposlayout(vRepos);
	}
}

void splitterwnd::recalclayout(const int nPane,const std::vector<std::pair<int,int>>& vRepos,const int nDY,const int nBeginClientRect)
{
	const int nPanes = static_cast<int>(m_vPanes.size());
	if(vRepos.size()==0 || nPane<0 || nPane>=nPanes || nDY==0)
		return;
	
	std::function<void(const int nPane,const int nDY,std::vector<std::pair<int,int>>& vRepos)> recalc=[nPanes,nBeginClientRect,this](const int nPane,const int nDY,std::vector<std::pair<int,int>>& vRepos) -> void
	{
		std::function<int(const int nPane,const std::vector<std::pair<int,int>>& vRepos,int n)> mutatableheight=[nPanes,nBeginClientRect,this](const int nPane,const std::vector<std::pair<int,int>>& vRepos,int n) -> int
		{
			const splitterwndpane *p=m_vPanes[nPane].get();
			if(p->getminimised())
				return 0;
			if(n<0)
			{
				const int nMin=(p->getmin()==-1)?0:p->getmin();
				const int nCanShrink=vRepos[nPane].second>nMin?vRepos[nPane].second-nMin:0;
				return nCanShrink;
			}
			if(p->getmax()==-1)
				return n;
			const int nMax=p->getmax();
			const int nCanGrow=vRepos[nPane].second<nMax?nMax-vRepos[nPane].second:0;
			return nCanGrow;
		};
		
		if(vRepos.size()==0 || nDY==0)
			return;

		const bool bFaux=nPane==nPanes-1;
		const bool bShrink=(nDY<0);

		if(bFaux)
		{
			if(bShrink)
			{
				// paradigm:reduce 'spare' client height
				//			reduce drag pane height
				//			reduce above panes height
				int nRemaining=-nDY;
				
				// reduce 'spare' client height
				const int nClientAvailable=nBeginClientRect-(vRepos[vRepos.size()-1].first+vRepos[vRepos.size()-1].second);
				const int nDelta=std::max<>(0,std::min<>(nRemaining,nClientAvailable));
				if(nDelta>0)
					nRemaining-=nDelta;

				// reduce drag pane height
				// reduce above panes height
				for(int n=nPane;nRemaining>0 && n>=0;--n)
				{
					const int nDelta=std::min<>(mutatableheight(n,vRepos,-nRemaining),nRemaining);
					vRepos[n].second-=nDelta;
					nRemaining-=nDelta;
				}
			}
			else
			{
				// paradigm:increase drag pane height
				//			increase above panes height
				int nRemaining=nDY;

				const int nClientAvailable=nBeginClientRect-(vRepos[vRepos.size()-1].first+vRepos[vRepos.size()-1].second);
				if(!m_bInitialisingLayout && nClientAvailable)
					return; // we could snap the bottom of the last pane to the bottom of the splitter wnd but think that would not look good

				// increase drag pane height
				// increase above panes height
				for(int n=nPane;nRemaining>0 && n>=0;--n)
				{
					const int nDelta=std::min<>(mutatableheight(n,vRepos,nRemaining),nRemaining);
					vRepos[n].second+=nDelta;
					nRemaining-=nDelta;
				}
			}
			settop(vRepos);
		}
		else
		if(bShrink)
		{
			// paradigm:reduce drag pane height
			//			reduce above panes height
			// ie last thing we want to do is reduce the height of panes
			int nRemaining=-nDY,nFrom;

			// decrease drag pane height
			// reduce above panes height
			nFrom=nRemaining;
			for(int n=nPane;nRemaining>0 && n>=0;--n)
			{
				const int nDelta=std::min<>(mutatableheight(n,vRepos,-nRemaining),nRemaining);
				vRepos[n].second-=nDelta;
				nRemaining-=nDelta;
			}
			settop(vRepos);

			// use the 'spare' client space
			if(!m_vPanes[nPanes-1]->getminimised())
			{
				const int nClientAvailable=nBeginClientRect-(vRepos[vRepos.size()-1].first+vRepos[vRepos.size()-1].second);
				const int nCanGrow=m_vPanes[nPanes-1]->getmax()==-1?nClientAvailable:m_vPanes[nPanes-1]->getmax()-vRepos[nPanes-1].second;
				const int nDelta=std::max<>(0,std::min<>(nClientAvailable,nCanGrow));
				if(nDelta)
				{
					vRepos[nPanes-1].second+=nDelta;
					settop(vRepos);
				}
			}
		}
		else
		{
			// paradigm:increase drag pane height
			//			increase above panes height
			// aim:		try to not mutate the top of the pane
			int nRemaining=nDY,nFrom;

			// above available
			nFrom=nRemaining;
			for(int n=nPane;nRemaining>0 && n>=0;--n)
			{
				const int nDelta=std::min<>(mutatableheight(n,vRepos,nRemaining),nRemaining);
				nRemaining-=nDelta;
			}
			const int nAbove=nFrom-nRemaining;

			// below available
			nRemaining=nFrom;
			for(int n=nPane+1;nRemaining>0 && n<nPanes;++n)
			{
				const int nDelta=std::min<>(mutatableheight(n,vRepos,-nRemaining),nRemaining);
				nRemaining-=nDelta;
			}
			const int nClientAvailable=nBeginClientRect-(vRepos[vRepos.size()-1].first+vRepos[vRepos.size()-1].second);
			const int nBelow=(nFrom-nRemaining)+std::max<>(0,nClientAvailable);

			// increase drag pane height
			// increase above panes height
			nRemaining=std::min<>(nAbove,nBelow);
			nFrom=nRemaining;
			for(int n=nPane;nRemaining>0 && n>=0;--n)
			{
				const int nDelta=std::min<>(mutatableheight(n,vRepos,nRemaining),nRemaining);
				vRepos[n].second+=nDelta;
				nRemaining-=nDelta;
			}

			// reduce below panes height
			if(nFrom>nRemaining)
			{
				// use the 'spare' client space
				int nShrink=nFrom-nRemaining;

				if(nClientAvailable>0)
				{
					const int nDelta=std::min<>(nShrink,nClientAvailable);
					nShrink-=nDelta;
				}

				for(int n=nPane+1;nShrink>0 && n<nPanes;++n)
				{
					const int nDelta=std::min<>(mutatableheight(n,vRepos,-nShrink),nShrink);
					vRepos[n].second-=nDelta;
					nShrink-=nDelta;
				}
			}
			settop(vRepos);
		}
	};

	std::vector<std::pair<int,int>> vMutableRepos=vRepos;

	recalc(nPane,nDY,vMutableRepos);

	reposlayout(vMutableRepos);
}

void splitterwnd::excludecliprect(CDC *pDC,const std::vector<CWnd*>& vErase)
{
	std::vector<CWnd*> vFiltered;
	auto i=vErase.begin(),end=vErase.end();
	for(;i!=end;++i)
	{
		if(!(*i) || !(*i)->GetSafeHwnd())
			continue;
		vFiltered.push_back(*i);
	}	

	const int nBars=static_cast<int>(vFiltered.size());
	if(nBars==0)
		return;

	i=vFiltered.begin(),end=vFiltered.end();
	for(;i!=end;++i)
	{
		CRect rc;
		(*i)->GetWindowRect(rc);
		::MapWindowPoints(NULL,(*i)->GetParent()->GetSafeHwnd(),(LPPOINT)&rc,2);
		pDC->ExcludeClipRect(rc);
	}
}

void splitterwnd::repos(const std::vector<std::pair<CWnd*,CRect>>& vRepos)
{
	std::vector<std::pair<CWnd*,CRect>> vFiltered;
	auto i=vRepos.begin(),end=vRepos.end();
	for(;i!=end;++i)
	{
		if(!(*i).first || !(*i).first->GetSafeHwnd())
			continue;
		CRect r;
		(*i).first->GetWindowRect(r);
		::ScreenToClient(::GetParent((*i).first->GetSafeHwnd()),&r.TopLeft());
		::ScreenToClient(::GetParent((*i).first->GetSafeHwnd()),&r.BottomRight());
		if(r==(*i).second)
			continue;
		vFiltered.push_back(*i);
	}	

	const int nBars=static_cast<int>(vFiltered.size());
	if(nBars==0)
		return;
	
	HDWP hDWP = ::BeginDeferWindowPos(nBars);
	HDWP hDefer = hDWP;
	i=vFiltered.begin(),end=vFiltered.end();
	for(;i!=end;++i)
		hDefer=::DeferWindowPos(hDefer,(*i).first->GetSafeHwnd(),NULL,(*i).second.left,(*i).second.top,(*i).second.Width(),(*i).second.Height(),SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);	
	::EndDeferWindowPos(hDWP);
}

void splitterwnd::reposlayout(const std::vector<std::pair<int,int>>& v)
{
	if(v.size()==0)
		return;
	
	CRect rc;
	GetClientRect(rc);

	std::vector<std::pair<CWnd*,CRect>> vRepos;
	auto i=m_vPanes.begin(),end=m_vPanes.end();
	auto j=v.cbegin();
	for(;i!=end;++i,++j)
	{
		(*i)->setrect({rc.left,(*j).first,rc.right,(*j).first+(*j).second});
		vRepos.push_back({(*i)->getwnd(),(*i)->getrect()});
	}
	
	repos(vRepos);

	Invalidate();
}

void splitterwnd::drawborder(CDC* const pDC,const CRect& rc,const bool bUp) const
{
	pDC->FillSolidRect( rc, ::GetSysColor(COLOR_3DFACE));

	if(bUp)
	{
		pDC->FillSolidRect(CRect(rc.left,rc.bottom-1,rc.right,rc.bottom),0);
		pDC->FillSolidRect(CRect(rc.right-1,rc.top,rc.right,rc.bottom),0);

		pDC->FillSolidRect(CRect(rc.left+1,rc.bottom-2,rc.right-1,rc.bottom-1),GetSysColor(COLOR_3DSHADOW));
		pDC->FillSolidRect(CRect(rc.right-2,rc.top+1,rc.right-1,rc.bottom-1),GetSysColor(COLOR_3DSHADOW));

		pDC->FillSolidRect(CRect(rc.left+1,rc.top+1,rc.left+2,rc.bottom-2),GetSysColor(COLOR_3DHILIGHT));
		pDC->FillSolidRect(CRect(rc.left+1,rc.top+1,rc.right-2,rc.top+2),GetSysColor(COLOR_3DHILIGHT));
	}
	else
	{
		pDC->FillSolidRect(CRect(rc.left,rc.top+2,rc.right,rc.top+3),GetSysColor(COLOR_3DSHADOW));
		pDC->FillSolidRect(CRect(rc.left,rc.top+3,rc.right,rc.top+4),GetSysColor(COLOR_3DHILIGHT));
	}
}

void splitterwnd::enddrag(void)
{
	if(m_nDraggingHT!=-1)
	{
		POINT p;
		::GetCursorPos(&p);
		::MapWindowPoints(NULL,m_hWnd,&p,1);

		invalidatebar(m_nDraggingHT);

		m_nDraggingHT=-1;
		m_nHT=-1;
		m_vDragBeginRepos.clear();
		m_rcDragBeginClientRect.SetRectEmpty();
	
		if(GetCapture()==this)
			ReleaseCapture();
	}
}

void splitterwnd::getdragbeginrepos(void)
{
	GetClientRect(m_rcDragBeginClientRect);
	m_vDragBeginRepos.resize(m_vPanes.size());
	gettopheight(m_vDragBeginRepos);
}

int splitterwnd::gettopheight(std::vector<std::pair<int,int>>& v)const
{
	int nSum=0;
	v.resize(m_vPanes.size());
	auto i=m_vPanes.cbegin(),end=m_vPanes.cend();
	auto j=v.begin();
	for(;i!=end;++i,++j)
	{
		const CRect rc=(*i)->getrect();
		(*j)={rc.top,rc.Height()};
		nSum+=rc.Height();
	}
	return nSum;
}

void splitterwnd::settop(std::vector<std::pair<int,int>>& vRepos)const
{
	int nTop=0;
	auto i=vRepos.begin(),end=vRepos.end();
	for(;i!=end;++i)
	{
		(*i).first=nTop;
		nTop+=(*i).second+splitterwndpane::m_nBarHeight;
	}
}

void splitterwnd::invalidatebar(const int n)
{
	if(n<0 || n>=static_cast<int>(m_vPanes.size()))
		return;
	InvalidateRect(m_vPanes[n]->getbarrect(true));
}
