
// rasterizerView.cpp : implementation of the CrasterizerView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "rasterizer.h"
#endif

#include "rasterizerDoc.h"
#include "rasterizerView.h"
#include "mainfrm.h"
#include "hint.h"
#include "translate_scale_viewtool.h"
#include "depth_viewtool.h"
#include "rotate_viewtool.h"

#include "cameratranslate_viewtool.h"
#include "cameradepth_viewtool.h"
#include "camerarotate_viewtool.h"

#include "3d_rendercontext.h"

#include "vertexfmtdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CrasterizerView

IMPLEMENT_DYNCREATE(CrasterizerView, CView)

BEGIN_MESSAGE_MAP(CrasterizerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_CAMERA_FRONT,OnCameraFront)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_FRONT,OnUpdateCameraFront)
	ON_COMMAND(ID_CAMERA_BACK,OnCameraBack)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_BACK,OnUpdateCameraBack)
	ON_COMMAND(ID_CAMERA_LEFT,OnCameraLeft)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_LEFT,OnUpdateCameraLeft)
	ON_COMMAND(ID_CAMERA_RIGHT,OnCameraRight)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_RIGHT,OnUpdateCameraRight)
	ON_COMMAND(ID_CAMERA_ABOVE,OnCameraAbove)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ABOVE,OnUpdateCameraAbove)
	ON_COMMAND(ID_CAMERA_BELOW,OnCameraBelow)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_BELOW,OnUpdateCameraBelow)

	ON_COMMAND(ID_CAMERA_DUPLICATE_NEW,OnNewCameraDuplicate)

	ON_COMMAND(ID_LIGHT_RENDER,OnLightRender)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_RENDER,OnUpdateLightRender)
	ON_COMMAND(ID_MODEL_RENDER,OnModelRender)
	ON_UPDATE_COMMAND_UI(ID_MODEL_RENDER,OnUpdateModelRender)
	ON_COMMAND(ID_SHADOW_RENDER,OnShadowRender)
	ON_UPDATE_COMMAND_UI(ID_SHADOW_RENDER,OnUpdateShadowRender)

	ON_COMMAND(ID_TRANSLATE_SCALE_TOOL,OnTranslateScaleTool)
	ON_UPDATE_COMMAND_UI(ID_TRANSLATE_SCALE_TOOL,OnUpdateTranslateScaleTool)
	ON_COMMAND(ID_DEPTH_TOOL,OnDepthTool)
	ON_UPDATE_COMMAND_UI(ID_DEPTH_TOOL,OnUpdateDepthTool)
	ON_COMMAND(ID_ROTATE_TOOL,OnRotateTool)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_TOOL,OnUpdateRotateTool)

	ON_COMMAND(ID_OBJ_IMPORT,OnImportObj)
	ON_COMMAND(ID_SPOT_LIGHT_CREATE,OnCreateSpotLight)
	ON_COMMAND(ID_POINT_LIGHT_CREATE,OnCreatePointLight)
	ON_COMMAND(ID_DIRECTIONAL_LIGHT_CREATE,OnCreateDirectionalLight)

	ON_COMMAND(ID_DELETE,OnDelete)

	ON_COMMAND(ID_FPS,OnFPS)

	ON_COMMAND(ID_CAMERA_TRANSLATE_TOOL,OnCameraTranslateTool)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_TRANSLATE_TOOL,OnUpdateCameraTranslateTool)
	ON_COMMAND(ID_CAMERA_DEPTH_TOOL,OnCameraDepthTool)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_DEPTH_TOOL,OnUpdateCameraDepthTool)
	ON_COMMAND(ID_CAMERA_ROTATE_TOOL,OnCameraRotateTool)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ROTATE_TOOL,OnUpdateCameraRotateTool)

	ON_COMMAND(ID_CANCEL,OnCancel)

	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()

	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()

	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

// CrasterizerView construction/destruction

CrasterizerView::CrasterizerView() noexcept
{
	// TODO: add construction code here
	m_spSurface=std::shared_ptr<surface>(new surface);
	m_bActive = false;
	m_bStopping = false;
	m_rComposeInflate={{5,5},{5,5}};
	m_pSel=nullptr;
	m_pSelectedTool=nullptr;
	m_nRender=af3d::vertexattsframe<>::t_all & ~(af3d::vertexattsframe<>::t_light_mesh|af3d::vertexattsframe<>::t_shadow);
}

CrasterizerView::~CrasterizerView()
{
}

BOOL CrasterizerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CrasterizerView drawing

void CrasterizerView::OnDraw(CDC* pDC)
{
	CrasterizerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	CRect rcClipRect;
	int n = pDC->GetClipBox( rcClipRect );

	if(m_spSurface && m_spOffscreenDib)
	{
		const af3d::rect r({{rcClipRect.left,rcClipRect.top},{rcClipRect.right,rcClipRect.bottom}});
		if(!m_pSelectedTool || !m_pSelectedTool->render())
			m_spSurface->flip(pDC,r);
		else
		{
			m_spSurface->flip(m_spOffscreenDib.get(),r);
			if(m_pSelectedTool)
				m_pSelectedTool->render(m_spOffscreenDib.get(),r,m_spSurface.get());
			m_spOffscreenDib->blt(pDC->GetSafeHdc(),r,r);
		}
		return;
	}
	pDC->FillSolidRect(rcClipRect,surface::s_grey);
}


// CrasterizerView printing

BOOL CrasterizerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CrasterizerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CrasterizerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CrasterizerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	
	start();
}

void CrasterizerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	hint *pH=static_cast<hint*>(pHint);
	bool bCompose=false,bComposeShadowMaps=false,bInvalidate=!m_bStopping;
	af3d::vertexattsframe<>* pSpecificLightMap=nullptr;
	if(pH)
		switch(pH->gettype())
		{
			case hint::t_null:
			case hint::t_initial_update:
			case hint::t_view_stop:break;
			case hint::t_view_active:
			{
				if(pH->getview()!=this)
					bInvalidate=false;
			}
			break;
			break;
			case hint::t_selection:
			{
				if(pH->getview()==this)
				{
					if(m_pSelectedTool)
						m_pSelectedTool->selectionchanged();
				}
				else
					bInvalidate=false;
			}
			break;
			case hint::t_drag:
			{
				switch(pH->getdragtype())
				{
					case viewtool::td_camera_transform:
					{
						bCompose=(pH->getview()==this);
						bComposeShadowMaps=bCompose;
						bInvalidate=bCompose;
					}
					break;
					case viewtool::td_frame_transform:
					{
						bCompose=bInvalidate;
						if(pH->getframe()->gettype() & af3d::vertexattsframe<>::t_light_mesh)
							pSpecificLightMap=pH->getframe();
						bComposeShadowMaps=bCompose;
					}
					break;
				}
			}
			break;
			case hint::t_frame_visible:
			{
				if(pH->getframe()->gettype() & af3d::vertexattsframe<>::t_light_mesh)
					pSpecificLightMap=pH->getframe();
			}
			case hint::t_frame_reparent:
			case hint::t_frame_append:
			case hint::t_facebuffer_pos:
			case hint::t_frame_opacity:bComposeShadowMaps=!m_bStopping;

			case hint::t_light_atten:
			case hint::t_light_range:
			case hint::t_spotlight_umbra_penumbra:
			case hint::t_light_col:
			
			case hint::t_material_add:
			case hint::t_material_del:
			case hint::t_material_range:
			case hint::t_material_col:
			case hint::t_material_enable:
			case hint::t_material_shininess:
			case hint::t_material_image:

			case hint::t_facebuffer_norm:
			case hint::t_facebuffer_tex:
			case hint::t_facebuffer_bump:
			case hint::t_facebuffer_col:bCompose=!m_bStopping;break;
			case hint::t_frame_erase:
			{
				if(pH->getframe()==getselection())
					setselection(nullptr);
				bCompose=bInvalidate;
				bComposeShadowMaps=bCompose;
			}
			break;
			case hint::t_frame_name:bInvalidate=false;break;
		}
	if(bComposeShadowMaps)
	{
		if(pSpecificLightMap)
			composeshadowmap(pSpecificLightMap,m_spSurface->getdstndc());
		else
			composeshadowmaps(m_spSurface->getdstndc());
	}
	if(bCompose)
		compose(m_spSurface->getdstndc());
	if(bInvalidate)
	{
		Invalidate();
		UpdateWindow();
	}
}

void CrasterizerView::compose(const af3d::rect& r)
{
	if(!m_spSurface)
		return;

	m_spSurface->compose(GetDocument()->getscene(),r,m_nRender);
}

void CrasterizerView::composeshadowmaps(const af3d::rect& r)
{
	if(!m_spSurface)
		return;

	if(m_nRender&af3d::vertexattsframe<>::t_shadow)
		m_spSurface->composeshadowmaps(GetDocument()->getscene(),nullptr,r,m_nRender);
}

void CrasterizerView::composeshadowmap(const af3d::vertexattsframe<>* pLight,const af3d::rect& r)
{
	if(!m_spSurface)
		return;

	if(m_nRender&af3d::vertexattsframe<>::t_shadow)
		m_spSurface->composeshadowmaps(GetDocument()->getscene(),pLight,r,m_nRender);
}

void CrasterizerView::start(void)
{
	GetDocument()->push_back(this);

	createtools();

	CRect rc;
	GetClientRect(rc);
	m_spSurface->start();
	m_spSurface->resize(GetDocument()->getscene(),rc.Width(),rc.Height());

	composeshadowmaps({{0,0},{rc.Width(),rc.Height()}});
	compose({{0,0},{rc.Width(),rc.Height()}});
	Invalidate();
}

void CrasterizerView::setselection(af3d::vertexattsframe<> *p)
{
	if(m_pSel==p)
		return;
	if(!p || (m_nRender&p->gettype() && p->gettype()!=af3d::vertexattsframe<>::t_scene && p->getvisible()))
	{
		m_pSel=p;
		theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_selection,p));
	}
}

void CrasterizerView::OnMouseMove( UINT nFlags, CPoint point )
{
	CView::OnMouseMove( nFlags, point );

	if(m_pSelectedTool)
		m_pSelectedTool->mousemove({point.x,point.y});
}

BOOL CrasterizerView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	const int nIts = ( zDelta > 0 ) ? ( zDelta / WHEEL_DELTA ) : ( -zDelta / WHEEL_DELTA );
	const bool bUp = ( zDelta > 0 );

	POINT p = pt;
	::MapWindowPoints( NULL, GetSafeHwnd(), &p, 1 );

	if(m_pSelectedTool && m_pSelectedTool->mousewheel(bUp,nIts,{p.x,p.y}))
		return TRUE;

	return CWnd::OnMouseWheel( nFlags, zDelta, pt );
}

void CrasterizerView::OnLButtonDown( UINT nFlags, CPoint point )
{
	CView::OnLButtonDown( nFlags, point );

	if(m_pSelectedTool)
		m_pSelectedTool->lbuttondown({point.x,point.y});
}

void CrasterizerView::OnLButtonUp( UINT nFlags, CPoint point )
{
	CView::OnLButtonUp( nFlags, point );

	if(m_pSelectedTool)
		m_pSelectedTool->lbuttonup({point.x,point.y});
}

void CrasterizerView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	CView::OnLButtonDblClk( nFlags, point );

	if(m_pSelectedTool)
		m_pSelectedTool->lbuttondblclk({point.x,point.y});
}

void CrasterizerView::OnCaptureChanged(CWnd* pGainWnd)
{
	CView::OnCaptureChanged(pGainWnd);

	if(m_pSelectedTool)
		m_pSelectedTool->capturechanged();
}

void CrasterizerView::createtools(void)
{
	pushbacktool(std::shared_ptr<translate_scale_viewtool>(new translate_scale_viewtool(this)));
	pushbacktool(std::shared_ptr<depth_viewtool>(new depth_viewtool(this)));
	pushbacktool(std::shared_ptr<rotate_viewtool>(new rotate_viewtool(this)));
	
	pushbacktool(std::shared_ptr<cameratranslate_viewtool>(new cameratranslate_viewtool(this)));
	pushbacktool(std::shared_ptr<cameradepth_viewtool>(new cameradepth_viewtool(this)));
	pushbacktool(std::shared_ptr<camerarotate_viewtool>(new camerarotate_viewtool(this)));

	setselectedtool(gettool(viewtool::t_translate_scale));
}

void CrasterizerView::pushbacktool(std::shared_ptr<viewtool> sp)
{
	if(!sp)
		return;
	m_mTools.insert({sp->gettype(),sp});
}

viewtool *CrasterizerView::gettool(const viewtool::type t) const
{
	auto i = m_mTools.find( t );
	if( i == m_mTools.cend() )
		return nullptr;
	return ( *i ).second.get();
}

void CrasterizerView::setselectedtool(viewtool *p)
{
	if(m_pSelectedTool == p)
		return;
	
	const bool bCurrentTool = !!m_pSelectedTool;
	if(bCurrentTool)
		m_pSelectedTool->stop();

	m_pSelectedTool = p;

	Invalidate();
}

af3d::vec2<int> CrasterizerView::getclientcursorpos( void ) const
{
	POINT p;
	::GetCursorPos( &p );
	::MapWindowPoints( NULL, GetSafeHwnd(), &p, 1 );
	return {p.x,p.y};
}

void CrasterizerView::stop(void)
{
	m_bStopping=true;

	setactive(false);

	if(m_spSurface)
	{
		m_spSurface->stop();
		m_spSurface=nullptr;
	}

	m_pSel = nullptr;

	{
		auto i = m_mTools.cbegin(), end = m_mTools.cend();
		for( ; i != end ; ++i )
			(*i).second->stop();
	}
	m_mTools.clear();

	theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_view_stop));

	if(GetDocument())
		GetDocument()->erase( this );
}

// CrasterizerView diagnostics

void CrasterizerView::setactive(const bool b)
{
	if( m_bActive == b )
		return;
	
	if( m_pSelectedTool && m_pSelectedTool->getindrag() != viewtool::td_null )
	{
		ASSERT( m_bActive );
		m_pSelectedTool->cancel();
	}

	m_bActive = b;

	if(m_pSelectedTool)
		m_pSelectedTool->setactive(b);
	
	if(b)
		theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_view_active));
}

bool CrasterizerView::captureinput( void )
{
	if( GetSafeHwnd() != ::GetCapture() )
		::SetCapture( GetSafeHwnd() );
	return true;
}

bool CrasterizerView::releasecapturedinput( void )
{
	if( GetSafeHwnd() == ::GetCapture() )
		::ReleaseCapture();
	return true;
}

#ifdef _DEBUG
void CrasterizerView::AssertValid() const
{
	CView::AssertValid();
}

void CrasterizerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CrasterizerDoc* CrasterizerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CrasterizerDoc)));
	return (CrasterizerDoc*)m_pDocument;
}
#endif //_DEBUG

bool CrasterizerView::getvertexfacecount(const af3d::vertexattsframe<> *p,int& n)const
{
	switch(p->getvertexatts())
	{
		case (af3d::face_vertex_att::t_pos):n=getvertexfacecount<af3d::face_pos3<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):n=getvertexfacecount<af3d::face_pos3_norm<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):n=getvertexfacecount<af3d::face_pos3_norm_bump<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):n=getvertexfacecount<af3d::face_pos3_norm_tex<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):n=getvertexfacecount<af3d::face_pos3_norm_tex_bump<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):n=getvertexfacecount<af3d::face_pos3_norm_col_bump<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):n=getvertexfacecount<af3d::face_pos3_norm_col<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):n=getvertexfacecount<af3d::face_pos3_norm_col_tex<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):n=getvertexfacecount<af3d::face_pos3_norm_col_tex_bump<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):n=getvertexfacecount<af3d::face_pos3_col<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):n=getvertexfacecount<af3d::face_pos3_col_tex<>>(p);break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):n=getvertexfacecount<af3d::face_pos3_tex<>>(p);break;
		default:return false;
	}
	return true;
}

// CrasterizerView message handlers
BOOL CrasterizerView::OnEraseBkgnd(CDC* pDC)
{
	// Dont erase the background
	if( m_spSurface )
		return TRUE;
	else
		return CView::OnEraseBkgnd( pDC );
}

void CrasterizerView::OnDestroy()
{
	stop();
	
	CView::OnDestroy();
}

void CrasterizerView::OnSize( UINT nType, int cx, int cy )
{
	CrasterizerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CView::OnSize( nType, cx, cy );

	std::shared_ptr<afdib::dib> spDib(new afdib::dib);
	spDib->create(cx,cy,afdib::dib::pt_b8g8r8);
	m_spOffscreenDib=spDib;
	
	if(m_spSurface)
	{
		m_spSurface->resize(GetDocument()->getscene(),cx,cy);

		composeshadowmaps({{0,0},{cx,cy}});
		compose({{0,0},{cx,cy}});
		Invalidate();
		UpdateWindow();
	}
}

BOOL CrasterizerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(m_pSelectedTool && m_pSelectedTool->setcursor())
		return TRUE;
	
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CrasterizerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	theApp.GetMainFrame()->setactiveview();
}

void CrasterizerView::OnLightRender(void)
{
	if(m_nRender&af3d::vertexattsframe<>::t_light_mesh)
		m_nRender&=~af3d::vertexattsframe<>::t_light_mesh;
	else
		m_nRender|=af3d::vertexattsframe<>::t_light_mesh;

	if(m_pSel && !(m_pSel->gettype()&m_nRender))
		setselection(nullptr);

	composeshadowmaps(m_spSurface->getdstndc());
	compose(m_spSurface->getdstndc());
	Invalidate();
}

void CrasterizerView::OnUpdateLightRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nRender&af3d::vertexattsframe<>::t_light_mesh?1:0);
}

void CrasterizerView::OnModelRender(void)
{
	if(m_nRender&af3d::vertexattsframe<>::t_mesh)
		m_nRender&=~af3d::vertexattsframe<>::t_mesh;
	else
		m_nRender|=af3d::vertexattsframe<>::t_mesh;

	if(m_pSel && !(m_pSel->gettype()&m_nRender))
		setselection(nullptr);

	composeshadowmaps(m_spSurface->getdstndc());
	compose(m_spSurface->getdstndc());
	Invalidate();
}

void CrasterizerView::OnUpdateModelRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nRender&af3d::vertexattsframe<>::t_mesh?1:0);
}

void CrasterizerView::OnShadowRender(void)
{
	if(m_nRender&af3d::vertexattsframe<>::t_shadow)
	{
		m_nRender&=~af3d::vertexattsframe<>::t_shadow;
		if(m_spSurface)
			m_spSurface->clearshadowmaps();
	}
	else
		m_nRender|=af3d::vertexattsframe<>::t_shadow;

	composeshadowmaps(m_spSurface->getdstndc());
	compose(m_spSurface->getdstndc());
	Invalidate();
}

void CrasterizerView::OnUpdateShadowRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nRender&af3d::vertexattsframe<>::t_shadow?1:0);
}

void CrasterizerView::OnImportObj(void)
{
	DWORD dw = ::GetCurrentDirectory(0,nullptr);
	CString cs;
	dw = ::GetCurrentDirectory(dw,cs.GetBuffer(dw-1));
	cs.ReleaseBuffer();

	TCHAR drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
	_tsplitpath_s((LPCTSTR)cs,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	CString csObjDir=CString(drive)+CString(dir);
	csObjDir.TrimRight();
	if(csObjDir.Right(1)==_T("\\") || csObjDir.Right(1)==_T("/"))
		csObjDir=csObjDir.Left(csObjDir.GetLength()-1);
	const CString csInitialDir=csObjDir+_T("\\")+_T("3dobj");

	CFileDialog dlg(true);
	dlg.m_ofn.lpstrInitialDir=csInitialDir;
	if(dlg.DoModal()!=IDOK)
		return;
	
	vertexfmtdlg fmt;
	if(fmt.DoModal()!=IDOK)
		return;

	std::shared_ptr<af3d::vertexattsframe<>> sp = GetDocument()->importobj(std::wstring(dlg.GetPathName()),m_pSel?m_pSel:GetDocument()->getscene(),fmt.getobjcfg());
	if(!sp)
		return;

	theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_frame_append,sp.get()));

	setselection(sp.get());
}

void CrasterizerView::OnCreateSpotLight(void)
{
	std::shared_ptr<af3d::vertexattsframe<>> sp = GetDocument()->createlight(af3d::light<>::t_spot,m_pSel?m_pSel:GetDocument()->getscene());
	if(!sp)
		return;

	theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_frame_append,sp.get()));

	if(m_nRender&sp->gettype())
		setselection(sp.get());
}

void CrasterizerView::OnCreatePointLight(void)
{
	std::shared_ptr<af3d::vertexattsframe<>> sp = GetDocument()->createlight(af3d::light<>::t_point,m_pSel?m_pSel:GetDocument()->getscene());
	if(!sp)
		return;

	theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_frame_append,sp.get()));

	if(m_nRender&sp->gettype())
		setselection(sp.get());
}

void CrasterizerView::OnCreateDirectionalLight(void)
{
	std::shared_ptr<af3d::vertexattsframe<>> sp = GetDocument()->createlight(af3d::light<>::t_directional,m_pSel?m_pSel:GetDocument()->getscene());
	if(!sp)
		return;

	theApp.UpdateAllViews(&hint(this,GetDocument(),hint::t_frame_append,sp.get()));

	if(m_nRender&sp->gettype())
		setselection(sp.get());
}

void CrasterizerView::OnDelete(void)
{
	GetDocument()->erase(getselection());
}

void CrasterizerView::OnFPS(void)
{
	DWORD dw=GetTickCount();
	int nCompose=100;
	for(int n=0;n<nCompose;++n)
	{
		composeshadowmaps(m_spSurface->getdstndc());
		compose(m_spSurface->getdstndc());
	}
	dw=GetTickCount()-dw;
	CString cs;
	std::function<void(const af3d::vertexattsframe<> *p,int& nM,int& nF)> meshface=[&](const af3d::vertexattsframe<> *p,int& nM,int& nF) -> void
	{
		int n=0;
		if(p && p->gettype()&m_nRender && getvertexfacecount(p,n)){++nM;nF+=n;}
		if(p && p->getchildren()){auto i=p->getchildren()->cbegin(),end=p->getchildren()->cend();for(;i!=end;++i)meshface((*i).get(),nM,nF);}
	};
	int nMeshes=0,nMeshFaces=0;
	meshface(GetDocument()->getscene(),nMeshes,nMeshFaces);
	const RAS_FLTTYPE dAvg=dw/static_cast<double>(nCompose);
	if(dAvg)
		cs.Format(_T("Mesh count:\t\t%li\r\nMesh face count:\t\t%li\r\n\r\nCompositions:\t\t%li\r\n\r\nTotal time:\t\t%f seconds\r\nAvg. time:\t\t%f milli\r\n\r\nFPS:\t\t\t%f"),nMeshes,nMeshFaces,nCompose,dw/1000.0,dAvg,1000.0/dAvg);
	else
		cs.Format(_T("Mesh count:\t\t%li\r\nMesh face count:\t\t%li\r\n\r\nCompositions:\t\t%li\r\n\r\nTotal time:\t\t%f seconds\r\nAvg. time:\t\t%f milli\r\n\r\nFPS:\t\t\tinf"),nMeshes,nMeshFaces,nCompose,dw/1000.0,dAvg);
	AfxMessageBox(cs);
}

void CrasterizerView::OnCameraFront(void)
{
	m_spSurface->setcameraorient(surface::s_DefFront);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraFront(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefFront?1:0);
}

void CrasterizerView::OnCameraBack(void)
{
	m_spSurface->setcameraorient(surface::s_DefBack);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraBack(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefBack?1:0);
}

void CrasterizerView::OnCameraLeft(void)
{
	m_spSurface->setcameraorient(surface::s_DefLeft);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraLeft(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefLeft?1:0);
}

void CrasterizerView::OnCameraRight(void)
{
	m_spSurface->setcameraorient(surface::s_DefRight);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraRight(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefRight?1:0);
}

void CrasterizerView::OnCameraAbove(void)
{
	m_spSurface->setcameraorient(surface::s_DefAbove);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraAbove(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefAbove?1:0);
}

void CrasterizerView::OnCameraBelow(void)
{
	m_spSurface->setcameraorient(surface::s_DefBelow);
	theApp.UpdateAllViews(&hint(this,GetDocument(),viewtool::td_camera_transform,m_spSurface->getcamera()));
}

void CrasterizerView::OnUpdateCameraBelow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_spSurface->getcameraorient()==surface::s_DefBelow?1:0);
}

void CrasterizerView::OnNewCameraDuplicate(void)
{
	cameraorient o;
	o.up=getsurface()->getcamera()->getup();
	o.origin=getsurface()->getcamera()->getorigin();
	o.dir=getsurface()->getcamera()->getdir();

	GetDocument()->createnewwindow(o);
}

void CrasterizerView::OnTranslateScaleTool(void)
{
	setselectedtool(gettool(viewtool::t_translate_scale));
}

void CrasterizerView::OnUpdateTranslateScaleTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_translate_scale ? 1 : 0);
}

void CrasterizerView::OnDepthTool(void)
{
	setselectedtool(gettool(viewtool::t_depth));
}

void CrasterizerView::OnUpdateDepthTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_depth ? 1 : 0);
}

void CrasterizerView::OnRotateTool(void)
{
	setselectedtool(gettool(viewtool::t_rotate));
}

void CrasterizerView::OnUpdateRotateTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_rotate ? 1 : 0);
}

void CrasterizerView::OnCameraTranslateTool(void)
{
	setselectedtool(gettool(viewtool::t_cameratranslate));
}

void CrasterizerView::OnUpdateCameraTranslateTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_cameratranslate ? 1 : 0);
}

void CrasterizerView::OnCameraDepthTool(void)
{
	setselectedtool(gettool(viewtool::t_cameradepth));
}

void CrasterizerView::OnUpdateCameraDepthTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_cameradepth ? 1 : 0);
}

void CrasterizerView::OnCameraRotateTool(void)
{
	setselectedtool(gettool(viewtool::t_camerarotate));
}

void CrasterizerView::OnUpdateCameraRotateTool(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pSelectedTool && m_pSelectedTool->gettype()==viewtool::t_camerarotate ? 1 : 0);
}

void CrasterizerView::OnCancel(void)
{
	if(m_pSelectedTool)
		m_pSelectedTool->cancel();
}
