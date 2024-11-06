
#include "pch.h"
#include "colourwnd.h"
#include "Resource.h"

IMPLEMENT_DYNAMIC(colourwnd, CWnd)

BEGIN_MESSAGE_MAP(colourwnd, CWnd)
	//{{AFX_MSG_MAP(colourwnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

colourwnd::colourwnd()
{
	m_col={1,1,1};
	m_nTimerInterval=50;
	m_nTimer=0;
	m_pDlg=nullptr;
	m_bPreview=false;
}

void colourwnd::OnPaint(void)
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect(rc);
	if(IsWindowEnabled())
		dc.FillSolidRect(rc,RGB(int(0.5+(m_col[2]*255.0)),int(0.5+(m_col[1]*255.0)),int(0.5+(m_col[0]*255.0))));
	else
		dc.FillSolidRect(rc,GetSysColor(COLOR_BTNFACE));
}

BOOL colourwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void colourwnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	CWnd::OnLButtonUp( nFlags, point );

	COLORREF c=RGB(int(0.5+(m_col[2]*255.0)),int(0.5+(m_col[1]*255.0)),int(0.5+(m_col[0]*255.0)));

	CMFCColorDialog dlg(c);
	m_pDlg=&dlg;

	if(m_bPreview && m_fn)
		m_nTimer=SetTimer(IDC_COLOURWND_TIMER,m_nTimerInterval,nullptr);

	af3d::vec3<> precol=m_col;
	
	const INT_PTR nRet=dlg.DoModal();
	
	if(m_nTimer)
	{
		KillTimer(m_nTimer);
		m_nTimer=0;
	}
	m_pDlg=nullptr;
	
	if(nRet==IDOK)
	{
		c=dlg.GetColor();
		m_col[0]=GetBValue(c)/255.0;
		m_col[1]=GetGValue(c)/255.0;
		m_col[2]=GetRValue(c)/255.0;
		if(m_fn)
			m_fn(GetDlgCtrlID(),m_col,false);
	}
	else
	{
		if(m_bPreview && m_fn)
			m_fn(GetDlgCtrlID(),precol,true);
		m_col=precol;
	}
	Invalidate();
}

void colourwnd::OnTimer(UINT_PTR nIDEvent)
{
	CWnd::OnTimer(nIDEvent);
	switch(nIDEvent)
	{
		case IDC_COLOURWND_TIMER:
		{
			if(m_pDlg && m_fn)
			{
				const COLORREF c=m_pDlg->GetColor();
				af3d::vec3<> col;
				col[0]=GetBValue(c)/255.0;
				col[1]=GetGValue(c)/255.0;
				col[2]=GetRValue(c)/255.0;
				if(!(m_col==col))
				{
					m_fn(GetDlgCtrlID(),col,true);
					m_col=col;
				}
			}
		}
		break;
	}
}
