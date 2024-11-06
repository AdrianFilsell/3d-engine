
#include "pch.h"
#include "titlewnd.h"
#include "viewtool.h"

//////////////////////////////////////////////////////////////////////
// Title Window

BEGIN_MESSAGE_MAP(titlewnd,CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void titlewnd::OnPaint()
{
	CPaintDC dc(this);
	
	CRect rcClient; GetClientRect( rcClient );
	dc.FillSolidRect( rcClient,GetSysColor(COLOR_ACTIVECAPTION));

	CFont* pOldFont=dc.SelectObject(&m_Font);
	COLORREF crOldFColour=dc.SetTextColor(GetSysColor(COLOR_WINDOW));
	
	CString cs;
	GetWindowText(cs);
	dc.DrawText(cs,CRect(rcClient.left+m_nTitleGap,rcClient.top,rcClient.right,rcClient.bottom),DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
	
	dc.SelectObject( pOldFont );
	dc.SetTextColor(crOldFColour);

	const CRect rc=getminimisedrect();
	const POINT pts[3]={{rc.left,m_bMinimised?rc.top:rc.bottom},{rc.left+rc.Width()/2,m_bMinimised?rc.bottom:rc.top},{rc.right,m_bMinimised?rc.top:rc.bottom}};
	CPen pen(PS_SOLID,2,GetSysColor(COLOR_WINDOW));
	HGDIOBJ hOldPen=dc.SelectObject(pen);
	dc.Polyline(pts,3);
	dc.SelectObject(hOldPen);
}

BOOL titlewnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

BOOL titlewnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(true)
	{
		HCURSOR h = LoadCursor(NULL,IDC_HAND);
		SetCursor(h);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void titlewnd::OnLButtonDown(UINT nHitTest,CPoint point)
{
	CWnd::OnLButtonDown(nHitTest,point);

	const int nDragMoveThreshold=3;
	if(viewtool::clickpending(viewtool::lbuttondownwait(this,{point.x,point.y},nDragMoveThreshold)))
	{
		if(m_bMinimised)
		{
			m_bMinimised=!m_bMinimised;
			GetParent()->SendMessage(WM_TITLEWND_MINIMISE,0,GetDlgCtrlID());
		}
		else
		{
			GetParent()->SendMessage(WM_TITLEWND_MINIMISE,1,GetDlgCtrlID());
			m_bMinimised=!m_bMinimised;
		}
		Invalidate();
	}
}
