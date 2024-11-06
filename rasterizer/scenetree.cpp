
#include "pch.h"
#include "scenetree.h"
#include "viewtool.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(scenetree, CTreeCtrl)

BEGIN_MESSAGE_MAP(scenetree, CTreeCtrl)
	//{{AFX_MSG_MAP(scenetree)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_CAPTURECHANGED()
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, &scenetree::OnBeginLabelEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

scenetree::scenetree()
{
	m_bInDrag=false;
	m_bSeenLButtonDown=false;
	m_ptLButtonDown = {0,0};
	m_nDragMoveThreshold=3;
	m_hDragCandidate=nullptr;
	m_hDragHover=nullptr;
	m_DragReparentType=reparent::type::t_null;
	m_hDragHoverInterval=250;
	m_cpDragILOffset={0,0};
}

void scenetree::OnPaint(void)
{
	CTreeCtrl::OnPaint();

	if(!m_bInDrag)
		return;
	const reparent r=getreparent();
	if(!r.isvalid(this))
		return;
	
	CPoint cp;
	::GetCursorPos( &cp );
	::MapWindowPoints( NULL, GetSafeHwnd(), &cp, 1 );

	CRect rcText,rcItem;
	GetItemRect(r.hTo,&rcText,TRUE);
	GetItemRect(r.hTo,&rcItem,FALSE);
	
	af3d::vertexattsframe<> *pTo=reinterpret_cast<af3d::vertexattsframe<>*>(GetItemData(r.hTo));
	af3d::vertexattsframe<> *pFrom=reinterpret_cast<af3d::vertexattsframe<>*>(GetItemData(r.hFrom));
		
	CDC *pDC=GetDC();

	CRect rcBar=rcItem;
	switch(r.t)
	{
		case reparent::t_first_child:
		{
			rcBar.bottom+=2;
			rcBar.top=rcBar.bottom-4;
			const double dDescendant=std::min<>(25.0,rcText.Width()*(1/3.0));
			const int nDescendant=static_cast<int>(rcText.left+dDescendant+0.5);
			rcBar.left=nDescendant;

			pDC->FillSolidRect(rcBar,RGB(255,0,0));
			
			rcBar.right=rcBar.left+4;
			rcBar.top-=2;
			rcBar.bottom+=2;
			
			pDC->FillSolidRect(rcBar,RGB(255,0,0));
		}
		break;
		case reparent::t_sibling_above:
		{
			rcBar.bottom=rcBar.top+2;
			rcBar.top=rcBar.bottom-4;

			pDC->FillSolidRect(rcBar,RGB(255,0,0));
		}
		break;
		case reparent::t_sibling_below:
		{
			rcBar.bottom+=2;
			rcBar.top=rcBar.bottom-4;

			pDC->FillSolidRect(rcBar,RGB(255,0,0));
		}
		break;
	}

	ReleaseDC(pDC);
}

void scenetree::OnLButtonDown(UINT nFlags, CPoint point)
{
	ASSERT(!m_bInDrag);
	if(m_bInDrag)
		return;

	m_ptLButtonDown = point;
	m_bSeenLButtonDown = true;

	// ht
    UINT uFlags;
    HTREEITEM hItem = HitTest(point, &uFlags);
	if (hItem != NULL && !(uFlags & TVHT_ONITEMSTATEICON) && !(uFlags & TVHT_ONITEMBUTTON))
	{
		const viewtool::lbuttonwaittype wt = viewtool::lbuttondownwait(this,{m_ptLButtonDown.x,m_ptLButtonDown.y},m_nDragMoveThreshold);
		
		const bool bClickPending = viewtool::clickpending( wt );
		const bool bDragPending = viewtool::dragpending( wt );
		
		if( bClickPending )
		{
		}
		else
		{
			m_hDragCandidate=hItem;
			if( bDragPending )
			{
				move(point);
				return;
			}
		}
	}

	CTreeCtrl::OnLButtonDown(nFlags,point);
}

void scenetree::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( m_bInDrag )
	{
		enddrag();	
		return;
	}
	m_hDragCandidate=nullptr;
	
	CTreeCtrl::OnLButtonUp(nFlags,point);
}

void scenetree::OnMouseMove(UINT nFlags, CPoint point)
{
	CTreeCtrl::OnMouseMove(nFlags,point);
	move(point);
}

void scenetree::OnCaptureChanged(CWnd* pWnd)
{
	CTreeCtrl::OnCaptureChanged(pWnd);

	if(pWnd!=this && m_bInDrag)
		enddrag();	
}

void scenetree::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pDispInfo = reinterpret_cast<TV_DISPINFO*>(pNMHDR);
	HTREEITEM hItem = pDispInfo->item.hItem;

	if(m_bInDrag)
	{
		*pResult = 1; // dont continue
		return;
	}

	*pResult = 0;
}

void scenetree::OnTimer(UINT_PTR nIDEvent)
{
	CTreeCtrl::OnTimer(nIDEvent);

	if(m_uiDragHoverID)
	{
		KillTimer(m_uiDragHoverID);
		m_uiDragHoverID=0;
	}

	switch(nIDEvent)
	{
		case IDC_DRAG_HOVER:
		{
			POINT p;
			::GetCursorPos( &p );
			::MapWindowPoints( NULL, GetSafeHwnd(), &p, 1 );
			
			UINT uFlags;
			HTREEITEM hItem = HitTest(p, &uFlags);
			if(hItem)
			{
				const UINT uiState = GetItemState(hItem, TVIS_EXPANDED);
				const bool bExpanded = (uiState & TVIS_EXPANDED) != 0;
				if(!bExpanded)
					Expand(hItem,TVE_EXPAND);
			}
		}
		break;
	}
}

void scenetree::move(const CPoint& pt)
{
	if( !CTreeCtrl::GetEditControl() && !m_bInDrag && m_bSeenLButtonDown && viewtool::getmousecaps_lButtondown() && m_hDragCandidate )
	{
		// ht
		if(canbegindrag(m_hDragCandidate) && viewtool::beindragmovedelta({m_ptLButtonDown.x,m_ptLButtonDown.y},{pt.x,pt.y},m_nDragMoveThreshold))
			begindrag(m_ptLButtonDown,pt);
	}

	if(m_bInDrag)
		movedrag(pt);
}

void scenetree::begindrag(const CPoint& ptStart,const CPoint& pt)
{
	std::shared_ptr<CImageList> spDragIL=std::shared_ptr<CImageList>(new CImageList);
	CRect rect;
	{
		GetItemRect(m_hDragCandidate,&rect,TRUE);
		CRect normrect(0,0,rect.Width(),rect.Height());

		CWnd *pCompWnd=this;//GetDesktopWindow();
		CDC *pDC=pCompWnd->GetDC();		

		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());

		pCompWnd->ReleaseDC(pDC);

		const COLORREF crMask=RGB(255,0,255);

		CBitmap *pOldBitmap = memDC.SelectObject(&bitmap);
		memDC.FillSolidRect(&normrect, crMask);
		
		CRgn clipRegion;
		clipRegion.CreateRectRgn(normrect.left,normrect.top,normrect.right,normrect.bottom);
		memDC.SelectClipRgn(&clipRegion);
		const CPoint cpOld=memDC.OffsetViewportOrg(-rect.left,-rect.top);
		
		SendMessage(WM_PRINTCLIENT, (WPARAM)memDC.GetSafeHdc(), (LPARAM)PRF_CLIENT | PRF_CHILDREN | PRF_ERASEBKGND);

		memDC.OffsetViewportOrg(-cpOld.x,cpOld.y);
		memDC.SelectClipRgn(nullptr);
		
//		::BitBlt(::GetDC(NULL),0,0,rect.Width(),rect.Height(),memDC,0,0,SRCCOPY);

		memDC.SelectObject(pOldBitmap);

		spDragIL->Create(rect.Width(),rect.Height(),ILC_COLOR32 | ILC_MASK,1,1);
		spDragIL->Add(&bitmap, crMask);
	}
	m_spDragIL=spDragIL;
	m_cpDragILOffset=ptStart-rect.TopLeft();
	
	UINT uFlags;
	HTREEITEM hItem = HitTest(pt, &uFlags);
	
	m_hDragHover=hItem;

	af3d::vertexattsframe<> *pTo=m_hDragHover?reinterpret_cast<af3d::vertexattsframe<>*>(GetItemData(m_hDragHover)):nullptr;
	m_DragReparentType=reparent::t_null;
	if(pTo)
		m_DragReparentType=(!pTo->getchildren() || pTo->getchildren()->size()==0) && ischild(m_hDragHover,pt-m_cpDragILOffset) ?
						   reparent::t_first_child:
						   ( isbelow(m_hDragHover,pt) ? reparent::t_sibling_below : reparent::t_sibling_above );

	BOOL b = m_spDragIL->BeginDrag(0, m_cpDragILOffset);

	CPoint cp=rect.TopLeft();
	ClientToScreen(&cp); // relative to the lock window
	b = m_spDragIL->DragEnter(GetDesktopWindow(),cp);

	m_bInDrag=true;

	setmovedragcursor();

	Invalidate();
	if(GetCapture()!=this)
		SetCapture();
}

void scenetree::movedrag(const CPoint& pt)
{
	if(m_spDragIL)
	{
		CPoint cp=pt;
		ClientToScreen(&cp); // relative to the lock window
		m_spDragIL->DragMove(cp);

		// ht
		UINT uFlags;
		HTREEITEM hItem = HitTest(pt, &uFlags);
		if(m_hDragHover!=hItem)
			movedragautoexpand(hItem,uFlags);

		const reparent rOld=getreparent();

		m_hDragHover=hItem;

		af3d::vertexattsframe<> *pTo=m_hDragHover?reinterpret_cast<af3d::vertexattsframe<>*>(GetItemData(m_hDragHover)):nullptr;
		m_DragReparentType=reparent::t_null;
		if(pTo)
			m_DragReparentType=(!pTo->getchildren() || pTo->getchildren()->size()==0) && ischild(m_hDragHover,pt-m_cpDragILOffset) ?
							   reparent::t_first_child:
							   ( isbelow(m_hDragHover,pt) ? reparent::t_sibling_below : reparent::t_sibling_above );
				
		const reparent rNew=getreparent();

		const bool bDelta=!(rOld==rNew);
		if(bDelta)
		{
			setmovedragcursor();
			Invalidate();
		}
	}
}

void scenetree::enddrag(void)
{
	const reparent r=getreparent();
	
	if(m_spDragIL)
	{
		m_spDragIL->DragLeave(GetDesktopWindow());
		m_spDragIL->EndDrag();
	}
	m_bInDrag=false;
	m_bSeenLButtonDown=false;
	m_hDragCandidate=nullptr;
	m_hDragHover=nullptr;
	m_DragReparentType=reparent::t_null;
	m_spDragIL=nullptr;
	if(m_uiDragHoverID)
	{
		KillTimer(m_uiDragHoverID);
		m_uiDragHoverID=0;
	}
	m_cpDragILOffset={0,0};
	if(GetCapture()==this)
		ReleaseCapture();
	Invalidate();

	if(r.isvalid(this) && GetParent())
		GetParent()->SendMessage(WM_SCENETREE_REPARENT,0,reinterpret_cast<LPARAM>(&r));
}

bool scenetree::canbegindrag(HTREEITEM h)const
{
	af3d::vertexattsframe<> *p=reinterpret_cast<af3d::vertexattsframe<>*>(GetItemData(h));
	return !(!p || p->gettype()&af3d::vertexattsframe<>::t_scene);
}

void scenetree::movedragautoexpand(HTREEITEM h,const UINT uFlags)
{
	if(m_uiDragHoverID)
	{
		KillTimer(m_uiDragHoverID);
		m_uiDragHoverID=0;
	}
	if(!h)
		return;

	const UINT uiState = GetItemState(h, TVIS_EXPANDED);
	const bool bExpanded = (uiState & TVIS_EXPANDED) != 0;
	if(bExpanded)
		return;

	m_uiDragHoverID=SetTimer(IDC_DRAG_HOVER,m_hDragHoverInterval,nullptr);
}

void scenetree::setmovedragcursor(void)
{
	HCURSOR h = LoadCursor(NULL,(getreparent().isvalid(this))?IDC_HAND:IDC_NO);
	SetCursor(h);
}

bool scenetree::isdescendant(HTREEITEM hA,HTREEITEM hB)const
{
	// a is a descendant of b
	HTREEITEM h=hA?GetParentItem(hA):nullptr;
	while(h && hB)
		if(h == hB)
			return true;
		else
			h=GetParentItem(h);
	return false;
}

bool scenetree::reparent::isvalid(scenetree *pTree)const
{
	if(!hFrom || !hTo || hFrom==hTo || pTree->isdescendant(hTo,hFrom))
		return false; // no valid src or dst
	
	af3d::vertexattsframe<> *pTo=reinterpret_cast<af3d::vertexattsframe<>*>(pTree->GetItemData(hTo));
	if(pTo->gettype()&af3d::vertexattsframe<>::t_scene)
		return false; // no valid dst

	af3d::vertexattsframe<> *pFrom=reinterpret_cast<af3d::vertexattsframe<>*>(pTree->GetItemData(hFrom));

	return true;
}

bool scenetree::ischild(HTREEITEM h,const CPoint& pt)const
{
	// approx 1/3 into item means descendant
	CRect rect;
	GetItemRect(h,&rect,TRUE);
	const double dDescendant=std::min<>(25.0,rect.Width()*(1/3.0));
	if(static_cast<double>(pt.x)>=rect.left+dDescendant)
		return true;
	return false;
}

bool scenetree::isbelow(HTREEITEM h,const CPoint& pt)const
{
	// approx 1/2 into item means below
	CRect rect;
	GetItemRect(h,&rect,TRUE);
	const double dBelow=rect.Height()*0.5;
	if(static_cast<double>(pt.y)>=rect.top+dBelow)
		return true;
	return false;
}
