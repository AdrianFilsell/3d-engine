
// rasterizerDoc.cpp : implementation of the CrasterizerDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "rasterizer.h"
#endif

#include "mainfrm.h"
#include "rasterizerDoc.h"
#include "rasterizerView.h"
#include "hint.h"

#include <propkey.h>
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CrasterizerDoc

IMPLEMENT_DYNCREATE(CrasterizerDoc, CDocument)

BEGIN_MESSAGE_MAP(CrasterizerDoc, CDocument)
	ON_COMMAND(ID_CAMERA_FRONT_NEW,OnNewCameraFront)
	ON_COMMAND(ID_CAMERA_BACK_NEW,OnNewCameraBack)
	ON_COMMAND(ID_CAMERA_LEFT_NEW,OnNewCameraLeft)
	ON_COMMAND(ID_CAMERA_RIGHT_NEW,OnNewCameraRight)
	ON_COMMAND(ID_CAMERA_ABOVE_NEW,OnNewCameraAbove)
	ON_COMMAND(ID_CAMERA_BELOW_NEW,OnNewCameraBelow)
END_MESSAGE_MAP()


// CrasterizerDoc construction/destruction

CrasterizerDoc::CrasterizerDoc() noexcept
{
	// TODO: add one-time construction code here

}

CrasterizerDoc::~CrasterizerDoc()
{
}

BOOL CrasterizerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	start();

	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_null));
		
	return TRUE;
}

void CrasterizerDoc::DeleteContents()
{
	stop();

	CDocument::DeleteContents();
}

void CrasterizerDoc::start(void)
{
	std::shared_ptr<af3d::scene<>> spScene(new af3d::scene<>);
	spScene->settrns(*surface::s_spIdentity);
	spScene->setcomposite();
	m_spScene=spScene;

	std::shared_ptr<af3d::vertexattsframe<>> sp = createlight(af3d::light<>::t_directional,getscene());
	if(!sp)
		return;
	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_append,sp.get()));
}

void CrasterizerDoc::stop(void)
{
	tidyup();
}

void CrasterizerDoc::tidyup(void)
{
	m_spScene=nullptr;
	m_vViews.clear();
}

void CrasterizerDoc::push_back(CrasterizerView *pV)
{
	m_vViews.push_back( pV );
}

void CrasterizerDoc::erase(CrasterizerView *pV)
{
	auto i=m_vViews.cbegin(),end=m_vViews.cend();
	for(;i!=end;++i)
		if(*i==pV)
		{
			m_vViews.erase(i);
			break;
		}
}

// CrasterizerDoc serialization

void CrasterizerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CrasterizerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CrasterizerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CrasterizerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CrasterizerDoc diagnostics

#ifdef _DEBUG
void CrasterizerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CrasterizerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

template <typename F> std::shared_ptr<af3d::vertexattsframe<>> CrasterizerDoc::importobj(const af3d::obj<>& o,af3d::vertexattsframe<> *pParent,const af3d::objcfg<>& cfg,const CStringA& cs)
{
	using FB=af3d::facebuffer<F>;

	const af3d::facemodelbbox<F::template t_flt> bbox(o.getbbox());

	std::shared_ptr<FB> spVertexBuffer(new FB);
	o.push_back(spVertexBuffer.get(),cfg);
		
	std::shared_ptr<af3d::mesh<FB>> spMesh(new af3d::mesh<FB>);
	spMesh->setparent(pParent);
	spMesh->setvertexbuffer(spVertexBuffer);
	spMesh->setbbox(bbox);
	spMesh->setname(std::string("mesh - "+cs));
	
	spMesh->validatecompositebbox();

	af3d::mat4<> s;
	surface::s_spIdentity->mul(af3d::mat4<>(af3d::translate3<>(-spMesh->getbbox(true).getorigin())),s);
	spMesh->settrns(s);
	spMesh->setcomposite();
	
	pParent->validatecompositebbox();

	const RAS_FLTTYPE dDstExtent=50;
	setworld_aa_extents_translate(spMesh.get(),dDstExtent,af3d::facetrnsbbox<>(pParent->getbbox(true),pParent->getcompositetrns()).getcentre());

	if(cfg.getmat().getatts(true))
	{
		std::shared_ptr<af3d::material<>> sp(new af3d::material<>(cfg.getmat()));
		sp->setrange(0,static_cast<int>(spVertexBuffer->get().size()-1));
		spMesh->bind({sp});
	}

	spMesh->setopacity(cfg.getopacity());

	spMesh->getparent()->push_back(spMesh);
	spMesh->invalidatecompositebbox();
	getscene()->cache_lights();
	
	return spMesh;
}

CFrameWnd *CrasterizerDoc::createnewwindow(void)
{
	CMainFrame *pMainFrame = theApp.GetMainFrame();
	CMDIChildWnd *pActiveChild = pMainFrame->MDIGetActive();
	if(!pActiveChild)
		return nullptr;
	CDocTemplate* pTemplate = GetDocTemplate();
	CFrameWnd* pNewFrame = pTemplate->CreateNewFrame(this,pActiveChild);
	if(!pNewFrame)
		return nullptr;
	pTemplate->InitialUpdateFrame(pNewFrame,this);
	return pNewFrame;
}

CFrameWnd *CrasterizerDoc::createnewwindow(const cameraorient& o)
{
	CFrameWnd *pFrame = createnewwindow();
	CrasterizerView *pView = static_cast<CrasterizerView*>(pFrame->GetActiveView());

	pView->GetDocument()->UpdateFrameCounts();
	pFrame->OnUpdateFrameTitle(TRUE);

	pView->getsurface()->setcameraorient(o);
	theApp.UpdateAllViews(&hint(pView,this,viewtool::td_camera_transform,pView->getsurface()->getcamera()));

	return pFrame;
}

std::shared_ptr<af3d::vertexattsframe<>> CrasterizerDoc::importobj(const std::wstring& s,af3d::vertexattsframe<> *pParent,const af3d::objcfg<>& cfg)
{
	af3d::obj<> o;
	if(!o.read(s))
		return nullptr;

	CHAR drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
	_splitpath_s(CStringA(CString(s.c_str())),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	const CStringA cs=CStringA(fname)+CStringA(ext);

	if(cfg.getvertexatts() & af3d::face_vertex_att::t_tex)
	{
		if(cfg.getvertexatts() & af3d::face_vertex_att::t_col)
		{
			if(cfg.getvertexatts() & af3d::face_vertex_att::t_norm)
			{
				if(cfg.getvertexatts() & af3d::face_vertex_att::t_bump)
				{
					// pos + tex + col + norm + bump
					return importobj<af3d::face_pos3_norm_col_tex_bump<>>(o,pParent,cfg,cs);
				}
				else
				{
					// pos + tex + col + norm
					return importobj<af3d::face_pos3_norm_col_tex<>>(o,pParent,cfg,cs);
				}
			}
			else
			{
				// pos + tex + col
				return importobj<af3d::face_pos3_col_tex<>>(o,pParent,cfg,cs);
			}
		}
		else
		if(cfg.getvertexatts() & af3d::face_vertex_att::t_norm)
		{
			if(cfg.getvertexatts() & af3d::face_vertex_att::t_bump)
			{
				// pos + tex + norm + bump
				return importobj<af3d::face_pos3_norm_tex_bump<>>(o,pParent,cfg,cs);
			}
			else
			{
				// pos + tex + norm
				return importobj<af3d::face_pos3_norm_tex<>>(o,pParent,cfg,cs);
			}
		}
		else
		{
			// pos + tex
			return importobj<af3d::face_pos3_tex<>>(o,pParent,cfg,cs);
		}
	}
	else
	if(cfg.getvertexatts() & af3d::face_vertex_att::t_col)
	{
		if(cfg.getvertexatts() & af3d::face_vertex_att::t_norm)
		{
			if(cfg.getvertexatts() & af3d::face_vertex_att::t_bump)
			{
				// pos + col + norm + bump
				return importobj<af3d::face_pos3_norm_col_bump<>>(o,pParent,cfg,cs);
			}
			else
			{
				// pos + col + norm
				return importobj<af3d::face_pos3_norm_col<>>(o,pParent,cfg,cs);
			}
		}
		else
		{
			// pos + col
			return importobj<af3d::face_pos3_col<>>(o,pParent,cfg,cs);
		}
	}
	else
	if(cfg.getvertexatts() & af3d::face_vertex_att::t_norm)
	{
		if(cfg.getvertexatts() & af3d::face_vertex_att::t_bump)
		{
			// pos + norm + bump
			return importobj<af3d::face_pos3_norm_bump<>>(o,pParent,cfg,cs);
		}
		else
		{
			// pos + norm
			return importobj<af3d::face_pos3_norm<>>(o,pParent,cfg,cs);
		}
	}
	else
	{
		// pos
		return importobj<af3d::face_pos3<>>(o,pParent,cfg,cs);
	}

	return nullptr;
}

std::shared_ptr<af3d::vertexattsframe<>> CrasterizerDoc::createlight(const af3d::light<>::type t,af3d::vertexattsframe<> *pParent)
{
	std::shared_ptr<af3d::light<>> spLight(new af3d::light<>);
	spLight->settype(t);

	af3d::obj<>::primitivetype pt;
	switch(t)
	{
		case af3d::light<>::t_spot:pt=af3d::obj<>::pt_unit_spot_light;break;
		case af3d::light<>::t_point:pt=af3d::obj<>::pt_unit_point_light;break;
		case af3d::light<>::t_directional:pt=af3d::obj<>::pt_unit_directional_light;break;
		default:return nullptr;
	}

	af3d::obj<> o;
	if(!o.create(pt))
		return nullptr;

	af3d::mat4<> worldtoparent = pParent->getcompositetrns().inverse();

	using F=af3d::face_pos3_norm<>;
	using FB=af3d::facebuffer<F>;

	const af3d::facemodelbbox<> bbox(o.getbbox());

	af3d::material<> m;
	const RAS_FLTTYPE dIntensity=GetRValue(surface::s_grey)*afdib::b8g8r8::getmaxrecip<RAS_FLTTYPE>(),dShininess=0;
	af3d::vec3<> diffuse;
	std::string sName;
	switch(t)
	{
		case af3d::light<>::t_spot:diffuse={0,dIntensity,dIntensity};sName="light - spot";break;
		case af3d::light<>::t_point:diffuse={dIntensity,dIntensity,0};sName="light - point";break;
		case af3d::light<>::t_directional:diffuse={dIntensity,0,dIntensity};sName="light - directional";break;
		default:return nullptr;
	}
	m.setcol(af3d::materialcol<>(diffuse,
								 af3d::materialcol<>::getdefambient(),
								 af3d::materialcol<>::getdefspecular(),
								 dShininess));
	const af3d::objcfg<> cfg(af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm,{false,{}},af3d::face_norm_vertex_data<>::t_geometric,af3d::face_tex_vertex_data<>::t_null,af3d::face_tex_vertex_data<>::t_null,m,1.0);
	
	std::shared_ptr<FB> spVertexBuffer(new FB);
	o.push_back(spVertexBuffer.get(),cfg);
		
	std::shared_ptr<af3d::lightmesh<FB>> spMesh(new af3d::lightmesh<FB>);
	spMesh->setparent(pParent);
	spMesh->setvertexbuffer(spVertexBuffer);
	spMesh->setbbox(bbox);
	spMesh->setlight(spLight);
	spMesh->setname(sName);
	
	spMesh->validatecompositebbox();

	af3d::mat4<> s;
	surface::s_spIdentity->mul(af3d::mat4<>(af3d::translate3<>(-spMesh->getbbox(true).getorigin())),s);
	spMesh->settrns(s);
	spMesh->setcomposite();
	
	const RAS_FLTTYPE dDstExtent=8;
	pParent->validatecompositebbox();
	setworld_aa_extents_translate(spMesh.get(),dDstExtent,af3d::facetrnsbbox<>(pParent->getbbox(true),pParent->getcompositetrns()).getcentre());

	if(cfg.getmat().getatts(true))
	{
		std::shared_ptr<af3d::material<>> sp(new af3d::material<>(cfg.getmat()));
		sp->setrange(0,static_cast<int>(spVertexBuffer->get().size()-1));
		spMesh->bind({sp});
	}

	spMesh->setopacity(cfg.getopacity());
	
	spMesh->getparent()->push_back(spMesh);
	spMesh->invalidatecompositebbox();
	getscene()->cache_lights();
	
	spMesh->sync();

	return spMesh;
}

void CrasterizerDoc::erase(af3d::vertexattsframe<> *p)
{
	if(!p || p->gettype()&af3d::vertexattsframe<>::t_scene)
		return;
	
	int n;
	p->getindex(n);
	std::shared_ptr<af3d::vertexattsframe<>> sp = p->getparent()->getchild(n); // hold a ref for hint

	getscene()->erase_lights(p);
	p->getparent()->erase(p);
	p->getparent()->invalidatecompositebbox();
	
	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_erase,p));
}

void CrasterizerDoc::reparent(af3d::vertexattsframe<> *p,af3d::vertexattsframe<> *pParent,af3d::vertexattsframe<> *pTarget,const bool bAbove)
{
	if(!getscene()->reparent(p,pParent,pTarget,bAbove))
		return;

	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_reparent,p));
}

void CrasterizerDoc::setopacity(af3d::vertexattsframe<> *p,const RAS_FLTTYPE d)
{
	if(!p || p->getopacity()==d)
		return;

	p->setopacity(d);

	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_opacity,p));
}

void CrasterizerDoc::setvisible(af3d::vertexattsframe<> *p,const bool b)
{
	if(!p || p->getvisible()==b)
		return;

	p->setvisible(b);
	getscene()->cache_visible_lights();

	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_visible,p));
}

void CrasterizerDoc::setname(af3d::vertexattsframe<> *p,const std::string& s)
{
	if(!p || p->getname()==s)
		return;

	p->setname(s);

	theApp.UpdateAllViews(&hint(nullptr,this,hint::t_frame_name,p));
}

void CrasterizerDoc::setworld_aa_extents_translate(af3d::vertexattsframe<> *p,const RAS_FLTTYPE dExtent,const af3d::vec3<>& o) const
{
	const RAS_FLTTYPE dDstExtent=dExtent;

	af3d::mat4<> mModelToWorld=p->getcompositetrns();
	mModelToWorld.mul(p->getparent()->getcompositetrns().inverse()); // knock out the ancestors

	p->validatecompositebbox();
	const af3d::facetrnsbbox<> worldbbox(p->getbbox(true),mModelToWorld);

	const af3d::vec3<> fromY=worldbbox.getup(af3d::facetrnsbbox<>::p_front,true);
	const af3d::vec3<> fromZ=worldbbox.getdir(af3d::facetrnsbbox<>::p_front,true,true);
	#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
		const af3d::vec3<> fromX=fromZ.cross(fromY);
	#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
		const af3d::vec3<> fromX=fromY.cross(fromZ);
	#endif

	const af3d::vec3<> toY(0,1,0);
	const af3d::vec3<> toZ(0,0,af3d::getfwd());
	const af3d::vec3<> toX(1,0,0);
	
	mModelToWorld.mul(af3d::translate3<>(-worldbbox.getcentre()));

	af3d::mat4<> mAA;
	af3d::vertexattsframe<>::getaatrns(fromX,fromY,fromZ,toX,toY,toZ,mAA);
	mModelToWorld.mul(mAA);

	const af3d::facetrnsbbox<> aaworldbbox(p->getbbox(true),mModelToWorld);
	const RAS_FLTTYPE d=dDstExtent/aaworldbbox.getmaxextent();
	mModelToWorld.mul(af3d::mat4<>(af3d::scale3<>({d,d,d})));

	mModelToWorld.mul(af3d::translate3<>(o));

	af3d::mat4<> mTrns;
	mModelToWorld.mul(p->getparent()->getcompositetrns().inverse(),mTrns);

	p->settrns(mTrns);
	p->setcomposite();
}


// CrasterizerDoc commands

void CrasterizerDoc::OnNewCameraFront(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefFront);
}

void CrasterizerDoc::OnNewCameraBack(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefBack);
}

void CrasterizerDoc::OnNewCameraLeft(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefLeft);
}

void CrasterizerDoc::OnNewCameraRight(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefRight);
}

void CrasterizerDoc::OnNewCameraAbove(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefAbove);
}

void CrasterizerDoc::OnNewCameraBelow(void)
{
	CFrameWnd *pFrame = createnewwindow(surface::s_DefBelow);
}
