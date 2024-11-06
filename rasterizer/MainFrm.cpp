
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "rasterizer.h"

#include "MainFrm.h"
#include "rasterizerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_ACTIVATEAPP()
    ON_WM_ENTERSIZEMOVE()
    ON_WM_EXITSIZEMOVE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
{
	// TODO: add member initialization code here
	m_pActiveView = nullptr;
	m_dwInSizing=0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	std::function<std::shared_ptr<CImageList>(CToolBar *pTB,const UINT nRes)> set32bppIL=[](CToolBar *pTB,const UINT nRes) -> std::shared_ptr<CImageList>
	{
		if(!pTB)return nullptr;
		HBITMAP hBitmap = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(nRes),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADTRANSPARENT);
		if(hBitmap==NULL) return nullptr;

		std::shared_ptr<CImageList> sp;
		sp=std::shared_ptr<CImageList>(new CImageList);
		sp->Create(16,15,ILC_COLOR32|ILC_MASK,0,1);
		const int n = sp->Add(CBitmap::FromHandle(hBitmap),COLORREF(0));
		::DeleteObject(hBitmap);

		CImageList* pOldIL=pTB->GetToolBarCtrl().SetImageList(sp.get());
		return sp;
	};

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wnd3DToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wnd3DToolBar.LoadToolBar(IDR_3D_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_sp3DToolBarIL=set32bppIL(&m_wnd3DToolBar,IDR_3D_TOOLBAR_32BPP);
	
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	if (!m_wndProperties.Create(::AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),::GetSysColorBrush(COLOR_BTNFACE),0),_T("Properties"),CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | WS_CHILD | WS_VISIBLE,CRect(CPoint(0,0),CSize(275,100)),this,IDC_PROPERTIES_BAR))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY & ~CBRS_ALIGN_RIGHT);
	m_wnd3DToolBar.EnableDocking(CBRS_ALIGN_ANY & ~CBRS_ALIGN_RIGHT);
	m_wndProperties.EnableDocking(CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wnd3DToolBar);
	DockControlBar(&m_wndProperties,AFX_IDW_DOCKBAR_RIGHT);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

void CMainFrame::OnActivateApp( BOOL bActive, DWORD dwThreadID )
{
	// Call the base class
	CMDIFrameWnd::OnActivateApp( bActive, dwThreadID );

	if( bActive )
		setactiveview();
	else
		setactiveviewex(nullptr);
}

void CMainFrame::OnEnterSizeMove()
{
    CFrameWnd::OnEnterSizeMove();
	m_dwInSizing++;
	if(m_dwInSizing==0)
		m_dwInSizing++;
}

void CMainFrame::OnExitSizeMove()
{
    CFrameWnd::OnExitSizeMove();
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}
#endif //_DEBUG


void CMainFrame::setactiveview( void )
{
	CFrameWnd *pFrameWnd = GetActiveFrame();
	CrasterizerView *pView = pFrameWnd ? static_cast<CrasterizerView*>( pFrameWnd->GetActiveView() ) : NULL;
	setactiveviewex( pView );
}

void CMainFrame::setactiveviewex( CrasterizerView *pView )
{
	if( pView == m_pActiveView )
		return;

	if( m_pActiveView )
		m_pActiveView->setactive( false );
	
	m_pActiveView = pView;

	if( m_pActiveView )
		m_pActiveView->setactive( true );
}

void CMainFrame::onupdate(hint *p)
{
	m_wndProperties.onupdate(p);
}

// CMainFrame message handlers
