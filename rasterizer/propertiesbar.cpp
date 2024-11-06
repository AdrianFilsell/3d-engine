
#include "pch.h"
#include "propertiesbar.h"

BEGIN_MESSAGE_MAP(propertiesbar, CControlBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CAPTURECHANGED()
	ON_WM_PAINT()
END_MESSAGE_MAP()

propertiesbar::propertiesbar()
{
	m_bCreateSplitter=false;
	m_nSplitterDeflate=6;
	m_bDragging=false;
	m_nDragBeginWidth=0;
	m_cpDragBegin={0,0};
}

propertiesbar::~propertiesbar()
{
}

CRect propertiesbar::getsplitterrect(const int nCX,const int nCY)const
{
	CRect rcSplitter(0,0,nCX,nCY);
	return rcSplitter;
}

CRect propertiesbar::getlayoutrect(void)const
{
	class dockbar : public CDockBar{friend class propertiesbar;};
	dockbar* pDockBar = static_cast<dockbar*>(m_pDockBar);
	
	CRect rcLayout = pDockBar->m_rectLayout;
	if(rcLayout.IsRectEmpty())
		m_pDockSite->GetClientRect(rcLayout);
	return rcLayout;
}

bool propertiesbar::hittest(const CPoint& cpScreen)const
{
	CRect rcClient,rcWindow,rcSplitter;
	GetWindowRect(rcWindow);
	GetClientRect(rcClient);
	rcSplitter=getsplitterrect(rcClient.Width(),rcClient.Height());
	::MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcSplitter,2);

	if(rcWindow.PtInRect(cpScreen) && cpScreen.x<rcSplitter.left)
		return true;
	return false;
}

void propertiesbar::enddrag(void)
{
	if(!m_bDragging)
		return;
	m_pDockSite->DelayRecalcLayout();
	m_bDragging=false;
	if(GetCapture()==this)
		ReleaseCapture();
}

void propertiesbar::onupdate(hint *p)
{
	if(m_spSceneDlg)
		m_spSceneDlg->onupdate(p);
	if(m_spLightDlg)
		m_spLightDlg->onupdate(p);
	if(m_spFacesDlg)
		m_spFacesDlg->onupdate(p);
	if(m_spMaterialsDlg)
		m_spMaterialsDlg->onupdate(p);
}

CSize propertiesbar::CalcDynamicLayout(int nLength, DWORD nMode)
{
	CRect rcWindow;
	GetWindowRect(rcWindow);
	
	const CRect rcLayout = getlayoutrect();
	
	if(!(nMode & LM_VERTDOCK))
		return CControlBar::CalcDynamicLayout(nLength,nMode);

	m_bCreateSplitter=true;

	CSize szDynamicLayout=CSize(nLength==-1?rcWindow.Width():nLength,rcLayout.Height());

	if(m_bDragging)
	{
		POINT p;
		::GetCursorPos(&p);
		
		const int nDX=m_cpDragBegin.x-p.x;
		szDynamicLayout.cx=nLength==-1?m_nDragBeginWidth+nDX:szDynamicLayout.cx;
	}
	return szDynamicLayout;
}

void propertiesbar::OnNcCalcSize(BOOL bCalcValidRects,NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CRect rcClient = lpncsp->rgrc[0];
	rcClient.DeflateRect(m_nSplitterDeflate,m_nSplitterDeflate,m_nSplitterDeflate,m_nSplitterDeflate);
	lpncsp->rgrc[0] = rcClient;
}

LRESULT propertiesbar::OnNcHitTest(CPoint point)
{
	if(hittest(point))
		return HTLEFT;
	return CControlBar::OnNcHitTest(point);
}

void propertiesbar::OnPaint(void) 
{
	CPaintDC dc(this);
}

void propertiesbar::OnNcPaint(void) 
{
	CWindowDC dc(this);

	CRect rcClient, rcWindow;
	GetClientRect(rcClient);
	::MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcClient,2);
	GetWindowRect(rcWindow);
	rcClient.OffsetRect(-rcWindow.TopLeft());
	rcWindow.OffsetRect(-rcWindow.TopLeft());

	dc.ExcludeClipRect(rcClient);

	DrawBorders(&dc,rcWindow);

	HBRUSH hbr = reinterpret_cast<HBRUSH>(GetClassLongPtr(m_hWnd,GCLP_HBRBACKGROUND));
	::FillRect(dc.m_hDC,rcWindow,hbr);
}

BOOL propertiesbar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void propertiesbar::OnSize(UINT nType,int cx,int cy)
{
	CControlBar::OnSize(nType,cx,cy);

	const CRect rcSplitter=getsplitterrect(cx,cy);

	if(m_splitter.GetSafeHwnd())
		m_splitter.MoveWindow(rcSplitter);
	else
	if(m_bCreateSplitter)
	{
		m_splitter.Create( NULL,_T("splitterwnd"),WS_CHILD|WS_VISIBLE,rcSplitter,this,IDC_PROPERTIES_SPLITTER);

		m_spSceneDlg=std::shared_ptr<scenedlg>(new scenedlg(&m_splitter));
		m_spSceneDlg->Create(scenedlg::IDD,&m_splitter);
		m_splitter.pushback(m_spSceneDlg);

		m_spLightDlg=std::shared_ptr<lightdlg>(new lightdlg(&m_splitter));
		m_spLightDlg->Create(lightdlg::IDD,&m_splitter);
		m_splitter.pushback(m_spLightDlg);

		m_spFacesDlg=std::shared_ptr<facesdlg>(new facesdlg(&m_splitter));
		m_spFacesDlg->Create(facesdlg::IDD,&m_splitter);
		m_splitter.pushback(m_spFacesDlg);

		m_spMaterialsDlg=std::shared_ptr<materialsdlg>(new materialsdlg(&m_splitter));
		m_spMaterialsDlg->Create(materialsdlg::IDD,&m_splitter);
		m_splitter.pushback(m_spMaterialsDlg);

		m_spSceneDlg->setmaximise(m_spSceneDlg->getmax()==-1?250:m_spSceneDlg->getmax());
		m_spLightDlg->setmaximise(m_spLightDlg->getmax()==-1?250:m_spLightDlg->getmax());
		m_spFacesDlg->setmaximise(m_spFacesDlg->getmax()==-1?250:m_spFacesDlg->getmax());
		m_spMaterialsDlg->setmaximise(m_spMaterialsDlg->getmax()==-1?250:m_spMaterialsDlg->getmax());

		m_splitter.initlayout();
	}
}

void propertiesbar::OnMouseMove(UINT uiFlags,CPoint point)
{
	CWnd::OnMouseMove( uiFlags, point );

	if(m_bDragging)
		m_pDockSite->DelayRecalcLayout();
}

void propertiesbar::OnNcLButtonDown(UINT nHitTest,CPoint point)
{
	if(m_bDragging)
		return;
	if(nHitTest==HTLEFT)
	{
		CRect rcDragMax;
		RedrawWindow(NULL,NULL,RDW_ALLCHILDREN|RDW_UPDATENOW); // process all pending
		m_pDockSite->RepositionBars(0,0xFFFF,AFX_IDW_PANE_FIRST,reposQuery,&rcDragMax,NULL,TRUE);

		CRect rcWindow;
		GetWindowRect(rcWindow);

		m_bDragging=true;
		m_cpDragBegin=point;
		m_nDragBeginWidth=rcWindow.Width();
		if(GetCapture()!=this)
			SetCapture();
	}
}

void propertiesbar::OnLButtonUp(UINT nFlags,CPoint point)
{
	CControlBar::OnLButtonUp(nFlags,point);

	enddrag();
}

void propertiesbar::OnCaptureChanged(CWnd* pWnd)
{
	CControlBar::OnCaptureChanged(pWnd);

	if(pWnd!=this)
		enddrag();
}
