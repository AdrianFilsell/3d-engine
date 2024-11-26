
#include "pch.h"
#include "lightdlg.h"
#include "hint.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "splitterwnd.h"

lightdlg::lightdlg(CWnd* pParent /*=nullptr*/)
	: propertiesdlg(IDD, pParent)
{
	m_bInOnUpdate=false;
	m_bInENChange=false;
	m_pView=nullptr;
	m_pDoc=nullptr;

	m_nAtten=0;
	m_csAtten=_T("0.0");

	m_csRange=_T("0.0");

	m_nColour=0;

	m_nSpot=0;
	m_csSpot=_T("0.0");

	m_nBorder = 2;
	m_nEditGap=0;
}

lightdlg::~lightdlg()
{
}

void lightdlg::DoDataExchange(CDataExchange* pDX)
{
	propertiesdlg::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_RANGE_EDIT,m_RangeEdit);
	DDX_Control(pDX,IDC_ATTEN_COMBO,m_AttenCombo);
	DDX_Control(pDX,IDC_ATTEN_EDIT,m_AttenEdit);
	DDX_Control(pDX,IDC_COLOUR_COMBO,m_ColourCombo);
	DDX_Control(pDX,IDC_SPOT_COMBO,m_SpotCombo);
	DDX_Control(pDX,IDC_SPOT_EDIT,m_SpotEdit);

	DDX_CBIndex(pDX,IDC_ATTEN_COMBO,m_nAtten);
	DDX_CBIndex(pDX,IDC_COLOUR_COMBO,m_nColour);
	DDX_CBIndex(pDX,IDC_SPOT_COMBO,m_nSpot);

	DDX_Text(pDX,IDC_RANGE_EDIT,m_csRange);
	DDX_Text(pDX,IDC_ATTEN_EDIT,m_csAtten);
	DDX_Text(pDX,IDC_SPOT_EDIT,m_csSpot);
}


BEGIN_MESSAGE_MAP(lightdlg,propertiesdlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()

	ON_CBN_SELCHANGE(IDC_COLOUR_COMBO,OnColourComboChange)

	ON_EN_CHANGE(IDC_ATTEN_EDIT,OnAttenChange)
	ON_EN_KILLFOCUS(IDC_ATTEN_EDIT,OnAttenKillFocus)
	ON_CBN_SELCHANGE(IDC_ATTEN_COMBO,OnAttenComboChange)

	ON_EN_CHANGE(IDC_SPOT_EDIT,OnSpotChange)
	ON_EN_KILLFOCUS(IDC_SPOT_EDIT,OnSpotKillFocus)
	ON_CBN_SELCHANGE(IDC_SPOT_COMBO,OnSpotComboChange)
END_MESSAGE_MAP()


// lightdlg message handlers

BOOL lightdlg::OnInitDialog()
{
	propertiesdlg::OnInitDialog();

	createtitle(IDC_TITLE,IDC_LIGHT_TITLE,_T("Light"),false);

	CRect rcAttenCombo,rcAttenEdit;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{&m_AttenEdit,&rcAttenEdit},{&m_AttenCombo,&rcAttenCombo}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	m_nEditGap=rcAttenEdit.left-rcAttenCombo.right;

	CRect rc;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_ColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,IDC_LIGHT_COLWND);
	m_ColWnd.setcol({0,0,0});
	auto fn = std::bind( &lightdlg::colourcallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
	m_ColWnd.setfn(fn,true);

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL lightdlg::OnEraseBkgnd(CDC *pDC)
{
	std::vector<CWnd*> vErase;
	vErase.push_back(&m_Title);
	vErase.push_back(GetDlgItem(IDC_RANGE_STATIC));
	vErase.push_back(&m_RangeEdit);
	vErase.push_back(GetDlgItem(IDC_ATTEN_EDGE));
	vErase.push_back(&m_AttenCombo);
	vErase.push_back(&m_AttenEdit);
	vErase.push_back(GetDlgItem(IDC_COLOUR_EDGE));
	vErase.push_back(&m_ColourCombo);
	vErase.push_back(&m_ColWnd);
	vErase.push_back(GetDlgItem(IDC_SPOT_EDGE));
	vErase.push_back(&m_SpotCombo);
	vErase.push_back(&m_SpotEdit);
	
	splitterwnd::excludecliprect(pDC,vErase);

	return propertiesdlg::OnEraseBkgnd(pDC);
}

void lightdlg::OnSize(UINT nType,int cx,int cy)
{
	propertiesdlg::OnSize(nType,cx,cy);

	CRect rcClient;
	GetClientRect(rcClient);

	std::vector<std::pair<CWnd*,CRect>> vRepos;

	CRect rcTitle;
	if(m_Title.GetSafeHwnd())
	{
		m_Title.GetWindowRect(rcTitle);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcTitle,2);
		rcTitle.right=rcClient.right-(rcTitle.left-rcClient.left);
		vRepos.push_back({&m_Title,rcTitle});
	}

	CRect rcRangeStatic,rcRangEdit;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{GetDlgItem(IDC_RANGE_STATIC),&rcRangeStatic},{&m_RangeEdit,&rcRangEdit}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	const int nRangeGap=rcRangEdit.left-rcRangeStatic.right;

	if(GetDlgItem(IDC_RANGE_STATIC))
	{
		rcRangeStatic.OffsetRect((rcTitle.left+m_nBorder)-rcRangeStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_RANGE_STATIC),rcRangeStatic});
	}

	if(m_RangeEdit.GetSafeHwnd())
	{
		rcRangEdit.OffsetRect((rcRangeStatic.right+nRangeGap)-rcRangEdit.left,0);
		rcRangEdit.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({&m_RangeEdit,rcRangEdit});
	}

	CRect rcAttenEdge,rcAttenCombo,rcAttenEdit;
	vRects={{&m_AttenEdit,&rcAttenEdit},{&m_AttenCombo,&rcAttenCombo},{GetDlgItem(IDC_ATTEN_EDGE),&rcAttenEdge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	if(GetDlgItem(IDC_ATTEN_EDGE))
	{
		rcAttenEdge.OffsetRect((rcTitle.left+m_nBorder)-rcAttenEdge.left,0);
		rcAttenEdge.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_ATTEN_EDGE),rcAttenEdge});
	}

	if(m_AttenCombo.GetSafeHwnd())
	{
		rcAttenCombo.OffsetRect((rcTitle.left+m_nBorder)-rcAttenCombo.left,0);
		vRepos.push_back({&m_AttenCombo,rcAttenCombo});
	}

	if(m_AttenEdit.GetSafeHwnd())
	{
		rcAttenEdit.left = rcAttenCombo.right+m_nEditGap;
		rcAttenEdit.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({&m_AttenEdit,rcAttenEdit});
	}
	
	CRect rcColourEdge,rcColour,rcColourCombo;
	vRects={{&m_ColourCombo,&rcColourCombo},{&m_ColWnd,&rcColour},{GetDlgItem(IDC_COLOUR_EDGE),&rcColourEdge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	if(GetDlgItem(IDC_COLOUR_EDGE))
	{
		rcColourEdge.OffsetRect((rcTitle.left+m_nBorder)-rcColourEdge.left,0);
		rcColourEdge.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_COLOUR_EDGE),rcColourEdge});
	}

	if(m_ColourCombo.GetSafeHwnd())
	{
		rcColourCombo.OffsetRect((rcTitle.left+m_nBorder)-rcColourCombo.left,0);
		vRepos.push_back({&m_ColourCombo,rcColourCombo});
	}

	if(m_ColWnd.GetSafeHwnd())
	{
		rcColour.left = rcColourCombo.right+m_nEditGap;
		rcColour.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({&m_ColWnd,rcColour});
	}

	CRect rcSpotEdge,rcSpotEdit,rcSpotCombo;
	vRects={{&m_SpotCombo,&rcSpotCombo},{&m_SpotEdit,&rcSpotEdit},{GetDlgItem(IDC_SPOT_EDGE),&rcSpotEdge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	if(GetDlgItem(IDC_SPOT_EDGE))
	{
		rcSpotEdge.OffsetRect((rcTitle.left+m_nBorder)-rcSpotEdge.left,0);
		rcSpotEdge.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_SPOT_EDGE),rcSpotEdge});
	}

	if(m_SpotCombo.GetSafeHwnd())
	{
		rcSpotCombo.OffsetRect((rcTitle.left+m_nBorder)-rcSpotCombo.left,0);
		vRepos.push_back({&m_SpotCombo,rcSpotCombo});
	}

	if(m_SpotEdit.GetSafeHwnd())
	{
		rcSpotEdit.left = rcSpotCombo.right+m_nEditGap;
		rcSpotEdit.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({&m_SpotEdit,rcSpotEdit});
	}

	splitterwnd::repos(vRepos);
}

void lightdlg::OnAttenChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	setatten(_tcstod(m_csAtten,&pNull));

	m_bInENChange=false;
}

void lightdlg::OnAttenKillFocus(void)
{
	af3d::light<> *pL=getlightcache();
	if(!pL)
		return;

	CString csPre,csPost;
	m_AttenEdit.GetWindowText(csPre);
	TCHAR* pNull=nullptr;
	const RAS_FLTTYPE d=_tcstod(csPre,&pNull);
	csPost.Format(_T("%f"),d);
	if(csPre==csPost)
		return;

	setatten(_tcstod(csPost,&pNull));
}

void lightdlg::OnAttenComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	m_csAtten=_T("0.0");

	getatten(m_csAtten);
	
	enabledisable();

	UpdateData(false);
}

void lightdlg::OnSpotChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	setspot(_tcstod(m_csSpot,&pNull));

	m_bInENChange=false;
}

void lightdlg::OnSpotKillFocus(void)
{
	af3d::light<> *pL=getlightcache();
	if(!pL || !(pL->gettype()&af3d::light<>::t_spot))
		return;

	CString csPre,csPost;
	m_SpotEdit.GetWindowText(csPre);
	TCHAR* pNull=nullptr;
	const RAS_FLTTYPE d=_tcstod(csPre,&pNull);
	csPost.Format(_T("%f"),d);
	if(csPre==csPost)
		return;

	setspot(_tcstod(csPost,&pNull));
}

void lightdlg::OnSpotComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	m_csSpot=_T("0.0");

	getspot(m_csSpot);
	
	enabledisable();

	UpdateData(false);
}

void lightdlg::OnColourComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	af3d::vec3<> c;
	getcol(c);
	m_ColWnd.setcol(c);
}

void lightdlg::colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview)
{
	switch(nID)
	{
		case IDC_LIGHT_COLWND:
		{
			m_ColWnd.setcol(c);
			setcol(c);
		}
		break;
	}
}

void lightdlg::clear(void)
{
	m_pView=nullptr;
	m_pDoc=nullptr;
}

void lightdlg::enabledisable(void)
{
	af3d::light<> *pL=getlightcache();
	const bool bEnable=!!pL;

	GetDlgItem(IDC_RANGE_STATIC)->EnableWindow(bEnable);
	m_RangeEdit.EnableWindow(bEnable);

	m_ColourCombo.EnableWindow(bEnable);
	m_ColWnd.EnableWindow(bEnable);
	m_ColWnd.Invalidate();

	const bool bEnableSpot=bEnable && pL->gettype()==af3d::light<>::t_spot;
	m_SpotCombo.EnableWindow(bEnableSpot);
	m_SpotEdit.EnableWindow(bEnableSpot);

	const bool bEnablePoint=bEnable && pL->gettype()==af3d::light<>::t_point;
	m_AttenCombo.EnableWindow(bEnablePoint || bEnableSpot);
	m_AttenEdit.EnableWindow(bEnablePoint || bEnableSpot);
}

af3d::light<> *lightdlg::getlightcache(void)const
{
	if(!m_pView || !m_pView->getselection() || !(m_pView->getselection()->gettype()&af3d::vertexattsframe<>::t_light_mesh))
		return nullptr;
	af3d::lightmeshcache<> *pC=dynamic_cast<af3d::lightmeshcache<>*>(m_pView->getselection());
	return pC?pC->getlight():nullptr;
}

void lightdlg::getrange(CString& cs)const
{
	af3d::light<> *pL=getlightcache();
	if(!pL)
		return;
	cs.Format(_T("%f"),pL->getrange());
}

void lightdlg::getatten(CString& cs)const
{
	af3d::light<> *pL=getlightcache();
	if(!pL || !(pL->gettype()&(af3d::light<>::t_spot|af3d::light<>::t_point)))
		return;
	switch(m_nAtten)
	{
		case 0:cs.Format(_T("%f"),pL->getconstattenuation());break;
		case 1:cs.Format(_T("%f"),pL->getlinearattenuation());break;
		case 2:cs.Format(_T("%f"),pL->getquadraticattenuation());break;
	}
}

void lightdlg::getcol(af3d::vec3<>& c)const
{
	af3d::light<> *pL=getlightcache();
	if(!pL)
		return;
	switch(m_nColour)
	{
		case 0:c=pL->getdiffuse();break;
		case 1:c=pL->getspecular();break;
		case 2:c=pL->getambient();break;
	}
}

void lightdlg::getspot(CString& cs)const
{
	af3d::light<> *pL=getlightcache();
	if(!pL || !(pL->gettype()&af3d::light<>::t_spot))
		return;
	switch(m_nSpot)
	{
		case 0:cs.Format(_T("%f"),pL->getumbra());break;
		case 1:cs.Format(_T("%f"),pL->getpenumbra());break;
	}
}

void lightdlg::setatten(const RAS_FLTTYPE d)
{
	af3d::light<> *pL=getlightcache();
	if(!pL || !(pL->gettype()&(af3d::light<>::t_spot|af3d::light<>::t_point)))
		return;
	switch(m_nAtten)
	{
		case 0:pL->setconstattenuation(d);break;
		case 1:pL->setlinearattenuation(d);break;
		case 2:pL->setquadraticattenuation(d);break;
	}
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_light_atten,m_pView->getselection()));
}

void lightdlg::setrange(const RAS_FLTTYPE d)
{
	af3d::light<> *pL=getlightcache();
	if(!pL)
		return;
	pL->setrange(d);
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_light_range,m_pView->getselection()));
}

void lightdlg::setspot(const RAS_FLTTYPE d)
{
	af3d::light<> *pL=getlightcache();
	if(!pL || !(pL->gettype()&af3d::light<>::t_spot))
		return;
	switch(m_nSpot)
	{
		case 0:
		{
			// inner
			pL->setumbra(d);
			if(d>pL->getpenumbra())
				pL->setpenumbra(d);
		}
		break;
		case 1:
		{
			// outer
			pL->setpenumbra(d);
			if(d<pL->getumbra())
				pL->setumbra(d);
		}
		break;
	}
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_spotlight_umbra_penumbra,m_pView->getselection()));
}

void lightdlg::setcol(const af3d::vec3<>& c)
{
	af3d::light<> *pL=getlightcache();
	if(!pL)
		return;
	switch(m_nColour)
	{
		case 0:pL->setdiffuse(c);break;
		case 1:pL->setspecular(c);break;
		case 2:pL->setambient(c);break;
	}

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_light_col,m_pView->getselection()));
}

int lightdlg::getmin(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_TITLE)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

int lightdlg::getmax(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_SPOT_COMBO)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

void lightdlg::onupdate(hint *p)
{
	propertiesdlg::onupdate(p);

	m_bInOnUpdate=true;
	if(p)
		switch(p->gettype())
		{
			case hint::t_initial_update:
			case hint::t_view_active:
			{
				if(theApp.getinitialised() && (p->getdoc()!=m_pDoc || p->getview()!=m_pView))
				{
					clear();
					m_pView=p->getview();
					m_pDoc=m_pView?static_cast<CrasterizerDoc*>(m_pView->GetDocument()):nullptr;
					
					enabledisable();

					getrange(m_csRange);
					getatten(m_csAtten);
					af3d::vec3<> c;
					getcol(c);
					m_ColWnd.setcol(c);
					getspot(m_csSpot);

					UpdateData(false);
				}
			}
			break;
			case hint::t_view_stop:
			{
				if(p->getdoc()==m_pDoc)
				{
					clear();
			
					enabledisable();

					m_csAtten=_T("0.0");
					m_csRange=_T("0.0");
					m_csSpot=_T("0.0");
					af3d::vec3<> c(0,0,0);
					
					getrange(m_csRange);
					getatten(m_csAtten);
					getcol(c);
					m_ColWnd.setcol(c);
					getspot(m_csSpot);

					UpdateData(false);
				}
			}
			break;
			case hint::t_selection:
			{
				enabledisable();

				m_csAtten=_T("0.0");
				m_csRange=_T("0.0");
				m_csSpot=_T("0.0");
				af3d::vec3<> c(0,0,0);
				
				getrange(m_csRange);
				getatten(m_csAtten);
				getcol(c);
				m_ColWnd.setcol(c);
				getspot(m_csSpot);

				UpdateData(false);
			}
			break;
			case hint::t_light_atten:
			{
				if(!m_bInENChange)
				{
					getatten(m_csAtten);
				
					UpdateData(false);
				}
			}
			break;
			case hint::t_light_range:
			{
				if(!m_bInENChange)
				{
					getrange(m_csRange);
					
					UpdateData(false);
				}
			}
			break;
			case hint::t_spotlight_umbra_penumbra:
			{
				if(!m_bInENChange)
				{
					getspot(m_csSpot);
				
					UpdateData(false);
				}
			}
			break;
			case hint::t_light_col:
			{
				af3d::vec3<> c;
				getcol(c);
				m_ColWnd.setcol(c);
			}
			break;
		}
	m_bInOnUpdate=false;
}
