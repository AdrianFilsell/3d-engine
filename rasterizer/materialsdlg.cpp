
#include "pch.h"
#include "materialsdlg.h"
#include "hint.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "splitterwnd.h"
#include "jpeg.h"

materialsdlg::materialsdlg(CWnd* pParent /*=nullptr*/)
	: propertiesdlg(IDD, pParent)
{
	m_bInOnUpdate=false;
	m_bInENChange=false;
	m_pView=nullptr;
	m_pDoc=nullptr;

	m_nFaces=0;
	m_nVertexAtts=0;
	
	m_nRange=0;
	m_csRangeFrom=_T("0");
	m_csRangeInclusiveTo=_T("0");
	m_csShininess=_T("0.0");
	m_csImage=_T("");

	m_nColour=0;
	m_nImage=0;
	m_nUV=0;

	m_nBorder = 2;
	m_nRangeCheckGap=0;
	m_nRangeEditGap=0;
	m_nEditSpinGap=0;
	m_nColourGap=0;

	m_nColourEnable=BST_CHECKED;
	m_nImageEnable=BST_CHECKED;
}

materialsdlg::~materialsdlg()
{
}

void materialsdlg::DoDataExchange(CDataExchange* pDX)
{
	propertiesdlg::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_RANGE_COMBO,m_RangeCombo);
	DDX_Control(pDX,IDC_RANGE_FROM_SPIN,m_RangeFromSpin);
	DDX_Control(pDX,IDC_RANGE_INCLUSIVETO_SPIN,m_RangeInclusiveToSpin);
	DDX_Control(pDX,IDC_RANGE_FROM_EDIT,m_RangeFromEdit);
	DDX_Control(pDX,IDC_RANGE_INCLUSIVETO_EDIT,m_RangeInclusiveToEdit);
	DDX_Control(pDX,IDC_COLOUR_COMBO,m_ColourCombo);
	DDX_Control(pDX,IDC_SHININESS_EDIT,m_ShininessEdit);
	DDX_Control(pDX,IDC_IMAGE_COMBO,m_ImageCombo);
	DDX_Control(pDX,IDC_UV_COMBO,m_UVCombo);

	DDX_CBIndex(pDX,IDC_RANGE_COMBO,m_nRange);
	DDX_CBIndex(pDX,IDC_COLOUR_COMBO,m_nColour);
	DDX_CBIndex(pDX,IDC_IMAGE_COMBO,m_nImage);
	DDX_CBIndex(pDX,IDC_UV_COMBO,m_nUV);

	DDX_Text(pDX,IDC_SHININESS_EDIT,m_csShininess);
	DDX_Text(pDX,IDC_IMAGE_PATH,m_csImage);
	DDX_Text(pDX,IDC_RANGE_FROM_EDIT,m_csRangeFrom);
	DDX_Text(pDX,IDC_RANGE_INCLUSIVETO_EDIT,m_csRangeInclusiveTo);

	DDX_Check(pDX,IDC_COLOUR_CHECK,m_nColourEnable);
	DDX_Check(pDX,IDC_IMAGE_CHECK,m_nImageEnable);
}


BEGIN_MESSAGE_MAP(materialsdlg,propertiesdlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()

	ON_CBN_SELCHANGE(IDC_RANGE_COMBO,OnRangeComboSelChanged)
	ON_BN_CLICKED(IDC_RANGE_ADD,OnAdd)
	ON_BN_CLICKED(IDC_RANGE_DEL,OnDel)
	ON_BN_CLICKED(IDC_RANGE_ALL,OnAll)

	ON_EN_CHANGE(IDC_RANGE_FROM_EDIT,OnFromChange)
	ON_EN_KILLFOCUS(IDC_RANGE_FROM_EDIT,OnFromKillFocus)
	ON_EN_CHANGE(IDC_RANGE_INCLUSIVETO_EDIT,OnToChange)
	ON_EN_KILLFOCUS(IDC_RANGE_INCLUSIVETO_EDIT,OnToKillFocus)

	ON_CBN_SELCHANGE(IDC_COLOUR_COMBO,OnColourComboSelChanged)
	ON_BN_CLICKED(IDC_COLOUR_CHECK,OnColourCheck)

	ON_EN_CHANGE(IDC_SHININESS_EDIT,OnShininessChange)
	ON_EN_KILLFOCUS(IDC_SHININESS_EDIT,OnShininessKillFocus)

	ON_CBN_SELCHANGE(IDC_IMAGE_COMBO,OnImageComboSelChanged)
	ON_BN_CLICKED(IDC_IMAGE_CHECK,OnImageCheck)
	ON_BN_CLICKED(IDC_IMAGE_BROWSE,OnImageBrowse)

	ON_CBN_SELCHANGE(IDC_UV_COMBO,OnUVComboSelChanged)
	ON_BN_CLICKED(IDC_UV_SET,OnUVSet)
END_MESSAGE_MAP()


// materialsdlg message handlers

BOOL materialsdlg::OnInitDialog()
{
	propertiesdlg::OnInitDialog();

	createtitle(IDC_TITLE,IDC_MATERIAL_TITLE,_T("Materials"),true);

	m_RangeFromSpin.SetRange32(0,0);
	m_RangeInclusiveToSpin.SetRange32(0,0);

	CRect rc;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_ColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,IDC_LIGHT_COLWND);
	m_ColWnd.setcol({0,0,0});
	auto fn = std::bind( &materialsdlg::colourcallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
	m_ColWnd.setfn(fn,true);

	CRect rcRangeAllBtn,rcRangeToSpin,rcRangeFromSpin,rcRangeToEdit;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{&m_RangeInclusiveToEdit,&rcRangeToEdit},{&m_RangeFromSpin,&rcRangeFromSpin},{&m_RangeInclusiveToSpin,&rcRangeToSpin},{GetDlgItem(IDC_RANGE_ALL),&rcRangeAllBtn}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}
	m_nRangeCheckGap=(rcRangeAllBtn.left-rcRangeToSpin.right);
	m_nRangeEditGap=rcRangeToEdit.left-rcRangeFromSpin.right;
	m_nEditSpinGap=rcRangeToSpin.left-rcRangeToEdit.right;

	CRect rcColourCombo,rcColWnd;
	vRects={{&m_ColourCombo,&rcColourCombo},{&m_ColWnd,&rcColWnd}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}
	m_nColourGap=rcColWnd.left-rcColourCombo.right;

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL materialsdlg::OnEraseBkgnd(CDC *pDC)
{
	std::vector<CWnd*> vErase;
	vErase.push_back(&m_Title);

	vErase.push_back(&m_RangeFromEdit);
	vErase.push_back(&m_RangeFromSpin);
	vErase.push_back(&m_RangeInclusiveToEdit);
	vErase.push_back(&m_RangeInclusiveToSpin);
	vErase.push_back(GetDlgItem(IDC_RANGE_ALL));
	vErase.push_back(&m_RangeCombo);
	vErase.push_back(GetDlgItem(IDC_RANGE_ADD));
	vErase.push_back(GetDlgItem(IDC_RANGE_DEL));

	vErase.push_back(GetDlgItem(IDC_COLOUR_EDGE));
	vErase.push_back(&m_ColourCombo);
	vErase.push_back(&m_ColWnd);
	vErase.push_back(GetDlgItem(IDC_COLOUR_CHECK));
	vErase.push_back(GetDlgItem(IDC_SHININESS_STATIC));
	vErase.push_back(&m_ShininessEdit);

	vErase.push_back(GetDlgItem(IDC_IMAGE_EDGE));
	vErase.push_back(GetDlgItem(IDC_IMAGE_BROWSE));
	vErase.push_back(&m_ImageCombo);
	vErase.push_back(GetDlgItem(IDC_IMAGE_CHECK));
	vErase.push_back(GetDlgItem(IDC_IMAGE_STATIC));
	vErase.push_back(GetDlgItem(IDC_IMAGE_PATH));
	vErase.push_back(&m_UVCombo);
	vErase.push_back(GetDlgItem(IDC_UV_SET));

	splitterwnd::excludecliprect(pDC,vErase);

	return propertiesdlg::OnEraseBkgnd(pDC);
}

void materialsdlg::OnSize(UINT nType,int cx,int cy)
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

	CRect rcRangeCombo,rcRangeAdd,rcRangeDel,rcRangeAllBtn,rcRangeToSpin,rcRangeFromSpin,rcRangeToEdit,rcRangeFromEdit;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{&m_RangeFromEdit,&rcRangeFromEdit},{&m_RangeInclusiveToEdit,&rcRangeToEdit},{&m_RangeFromSpin,&rcRangeFromSpin},{&m_RangeInclusiveToSpin,&rcRangeToSpin},{GetDlgItem(IDC_RANGE_ALL),&rcRangeAllBtn},{&m_RangeCombo,&rcRangeCombo},{GetDlgItem(IDC_RANGE_ADD),&rcRangeAdd},{GetDlgItem(IDC_RANGE_DEL),&rcRangeDel}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nGap=rcRangeDel.left-rcRangeAdd.right;
		rcRangeDel.OffsetRect((rcTitle.right-m_nBorder)-rcRangeDel.right,0);
		vRepos.push_back({GetDlgItem(IDC_RANGE_DEL),rcRangeDel});

		rcRangeAdd.OffsetRect((rcRangeDel.left-nGap)-rcRangeAdd.right,0);
		vRepos.push_back({GetDlgItem(IDC_RANGE_ADD),rcRangeAdd});
	}

	{
		rcRangeCombo.left=(rcTitle.left+m_nBorder);
		rcRangeCombo.right=rcRangeAdd.left-(rcRangeDel.left-rcRangeAdd.right);
		vRepos.push_back({&m_RangeCombo,rcRangeCombo});
	}

	{
		rcRangeAllBtn.OffsetRect((rcTitle.right-m_nBorder)-rcRangeAllBtn.right,0);
		vRepos.push_back({GetDlgItem(IDC_RANGE_ALL),rcRangeAllBtn});

		rcRangeToSpin.OffsetRect((rcRangeAllBtn.left-m_nRangeCheckGap)-rcRangeToSpin.right,0);
		vRepos.push_back({&m_RangeInclusiveToSpin,&rcRangeToSpin});

		const double dWidth=(rcRangeToSpin.right)-(rcTitle.left+m_nBorder);
		const double dEditWidth=dWidth-rcRangeToSpin.Width()-rcRangeFromSpin.Width()-m_nRangeEditGap;
		const int nEditWidth=static_cast<int>(dEditWidth/2.0 + 0.5);

		rcRangeToEdit.right=rcRangeToSpin.left-m_nEditSpinGap;
		rcRangeToEdit.left=rcRangeToEdit.right-nEditWidth;
		vRepos.push_back({&m_RangeInclusiveToEdit,&rcRangeToEdit});

		rcRangeFromEdit.left=rcTitle.left+m_nBorder;
		rcRangeFromEdit.right=rcRangeFromEdit.left+nEditWidth;
		vRepos.push_back({&m_RangeFromEdit,&rcRangeFromEdit});

		rcRangeFromSpin.OffsetRect((rcRangeFromEdit.right+m_nEditSpinGap)-rcRangeFromSpin.left,0);
		vRepos.push_back({&m_RangeFromSpin,&rcRangeFromSpin});
	}

	CRect rcColourEdge,rcColourCheck,rcColourCombo,rcColWnd,rcShininessStatic,rcShininessEdit;
	vRects={{&m_ShininessEdit,&rcShininessEdit},{GetDlgItem(IDC_SHININESS_STATIC),&rcShininessStatic},{GetDlgItem(IDC_COLOUR_EDGE),&rcColourEdge},{GetDlgItem(IDC_COLOUR_CHECK),&rcColourCheck},{&m_ColourCombo,&rcColourCombo},{&m_ColWnd,&rcColWnd}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		rcColourEdge.OffsetRect((rcTitle.left+m_nBorder)-rcColourEdge.left,0);
		rcColourEdge.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_COLOUR_EDGE),&rcColourEdge});
	
		rcColourCheck.OffsetRect((rcTitle.right-m_nBorder)-rcColourCheck.right,0);
		vRepos.push_back({GetDlgItem(IDC_COLOUR_CHECK),&rcColourCheck});

		rcColourCombo.OffsetRect((rcTitle.left+m_nBorder)-rcColourCombo.left,0);
		vRepos.push_back({&m_ColourCombo,&rcColourCombo});

		rcColWnd.left=rcColourCombo.right+m_nColourGap;
		rcColWnd.right=rcColourCheck.left-m_nColourGap;
		vRepos.push_back({&m_ColWnd,&rcColWnd});

		rcShininessStatic.OffsetRect((rcTitle.left+m_nBorder)-rcShininessStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_SHININESS_STATIC),&rcShininessStatic});

		rcShininessEdit.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({&m_ShininessEdit,&rcShininessEdit});
	}

	CRect rcUVCombo,rcUVSetBtn,rcImageStatic,rcImageEdit,rcImageEdge,rcImageCheck,rcImageBrowse,rcImageCombo;
	vRects={{&m_UVCombo,&rcUVCombo},{GetDlgItem(IDC_UV_SET),&rcUVSetBtn},{GetDlgItem(IDC_IMAGE_STATIC),&rcImageStatic},{GetDlgItem(IDC_IMAGE_PATH),&rcImageEdit},{GetDlgItem(IDC_IMAGE_EDGE),&rcImageEdge},{GetDlgItem(IDC_IMAGE_CHECK),&rcImageCheck},{GetDlgItem(IDC_IMAGE_BROWSE),&rcImageBrowse},{&m_ImageCombo,&rcImageCombo}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nUVGap=rcUVSetBtn.left-rcUVCombo.right;
		const int nStaticGap=rcImageEdit.left-rcImageStatic.right;
		const int nBrowseGap=rcImageBrowse.left-rcImageCombo.right;

		rcImageStatic.OffsetRect((rcTitle.left+m_nBorder)-rcImageStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_IMAGE_STATIC),&rcImageStatic});

		rcImageEdge.OffsetRect((rcTitle.left+m_nBorder)-rcImageEdge.left,0);
		rcImageEdge.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_IMAGE_EDGE),&rcImageEdge});

		rcImageCheck.OffsetRect((rcTitle.right-m_nBorder)-rcImageCheck.right,0);
		vRepos.push_back({GetDlgItem(IDC_IMAGE_CHECK),&rcImageCheck});

		rcImageCombo.OffsetRect((rcTitle.left+m_nBorder)-rcImageCombo.left,0);
		vRepos.push_back({&m_ImageCombo,&rcImageCombo});

		rcImageBrowse.OffsetRect((rcImageCombo.right+nBrowseGap)-rcImageBrowse.left,0);
		vRepos.push_back({GetDlgItem(IDC_IMAGE_BROWSE),&rcImageBrowse});

		rcImageEdit.left=(rcImageStatic.right+nStaticGap);
		rcImageEdit.right=(rcTitle.right-m_nBorder);
		vRepos.push_back({GetDlgItem(IDC_IMAGE_PATH),&rcImageEdit});

		rcUVCombo.OffsetRect((rcTitle.left+m_nBorder)-rcUVCombo.left,0);
		vRepos.push_back({&m_UVCombo,&rcUVCombo});

		rcUVSetBtn.OffsetRect((rcUVCombo.right+nUVGap)-rcUVSetBtn.left,0);
		vRepos.push_back({GetDlgItem(IDC_UV_SET),&rcUVSetBtn});
	}

	splitterwnd::repos(vRepos);
}

void materialsdlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	propertiesdlg::OnVScroll(nSBCode,nPos,pScrollBar);
	switch(nSBCode)
	{
		case SB_ENDSCROLL:
			if(pScrollBar &&
				((pScrollBar->GetDlgCtrlID()==IDC_RANGE_FROM_SPIN)||(pScrollBar->GetDlgCtrlID()==IDC_RANGE_INCLUSIVETO_SPIN)))
			{}
		break;
	}
}

void materialsdlg::OnRangeComboSelChanged(void)
{
	onrangecomboselchanged();
}

void materialsdlg::OnAll(void)
{
	if(m_nFaces<1)
		return;

	setrangefrominclusiveto(0,m_nFaces-1);
}

void materialsdlg::OnAdd(void)
{
	UpdateData();

	af3d::material<> *pSrc=getmaterial();
	const bool bSel=m_pView && m_pView->getselection();
	
	if(!bSel)
		return;

	CString cs;
	std::shared_ptr<af3d::material<>> sp;
	if(pSrc)
		sp=pSrc->clone();
	else
	{
		sp=std::shared_ptr<af3d::material<>>(new af3d::material<>(0,m_nFaces-1));
		sp->setcol(af3d::materialcol<>());
		sp->enable(af3d::face_vertex_att::t_col,true);
	}
	m_pView->getselection()->bind({sp});

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_add,m_pView->getselection(),sp.get()));
}

void materialsdlg::OnDel(void)
{
	UpdateData();

	af3d::material<> *pSrc=getmaterial();
	const bool bSel=m_pView && m_pView->getselection();
	
	if(!bSel)
		return;

	int nMat;
	if(!m_pView->getselection()->getmaterialindex(pSrc,nMat))
		return;
	std::shared_ptr<af3d::material<>> sp=(*getmaterials())[nMat];
	
	m_pView->getselection()->release(pSrc);
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_del,m_pView->getselection(),pSrc));
}

void materialsdlg::OnFromChange(void)
{
	onfromtochange(true);
}

void materialsdlg::OnFromKillFocus(void)
{
	onfromtokillfocus(true);
}

void materialsdlg::OnToChange(void)
{
	onfromtochange(false);
}

void materialsdlg::OnToKillFocus(void)
{
	onfromtokillfocus(false);
}

void materialsdlg::OnColourComboSelChanged(void)
{
	UpdateData();

	af3d::vec3<> c;
	getcol(c);
	m_ColWnd.setcol(c);

	getcolon(m_nColourEnable);

	enabledisable();
}

void materialsdlg::OnColourCheck(void)
{
	UpdateData();
	setcolon(m_nColourEnable==BST_CHECKED);
}

void materialsdlg::OnShininessChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	double d=_tcstod(m_csShininess,&pNull);
	if(d<0)
		d=0;
	
	setshininess(d);
	
	m_bInENChange=false;
}

void materialsdlg::OnShininessKillFocus(void)
{
	if(m_nFaces<1)
		return;

	CString csPre,csPost;
	m_ShininessEdit.GetWindowText(csPre);
	TCHAR* pNull=nullptr;
	double d=_tcstod(csPre,&pNull);
	if(d<0)
		d=0;
	csPost.Format(_T("%f"),d);
	if(csPre==csPost)
		return;

	setshininess(_tcstod(csPost,&pNull));
}

void materialsdlg::OnImageComboSelChanged(void)
{
	UpdateData();

	m_csImage=_T("");

	m_nImageEnable=BST_CHECKED;

	getimage(m_csImage);
	
	getimageon(m_nImageEnable);

	UpdateData(false);

	enabledisable();
}

void materialsdlg::OnImageCheck(void)
{
	UpdateData();
	setimageon(m_nImageEnable==BST_CHECKED);
}

void materialsdlg::OnImageBrowse(void)
{
	af3d::material<> *pSrc=getmaterial();
	if(!pSrc)
		return;

	CFileDialog dlg(true);
	if(dlg.DoModal()!=IDOK)
		return;
	const CString cs=dlg.GetPathName();
	
	bool bValid=false;
	switch(m_nImage)
	{
		case 0:
		{
			std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(cs);
			if(spDib)
			{
				bValid=true;
				std::shared_ptr<af3d::materialdib> sp(new af3d::materialdib(spDib));
				pSrc->settex(std::wstring(cs),sp);
			}
		}
		break;
		case 1:
		{
			std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(cs);
			if(spDib)
			{
				bValid=true;
				std::shared_ptr<af3d::materialnormalmap<>> sp(new af3d::materialnormalmap<>(spDib));
				pSrc->setbump(std::wstring(cs),sp);
			}
		}
		case 2:
		{
			std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(cs);
			if(spDib)
			{
				bValid=true;
				std::shared_ptr<af3d::materialcubicenvmap> sp(new af3d::materialcubicenvmap(af3d::rect({0,0},{spDib->getwidth()/4,spDib->getwidth()/4}),spDib));
				pSrc->setcubicenv(std::wstring(cs),sp);
			}
		}
		break;
	}

	if(bValid)
		theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_image,m_pView->getselection(),pSrc));
}

void materialsdlg::OnUVComboSelChanged(void)
{
	UpdateData();
}

void materialsdlg::OnUVSet(void)
{
	UpdateData();

	facebuffer fb;
	int nVertexAtts,nFaces;
	propertiesdlg::getfacebuffer(m_pView,fb,nVertexAtts,nFaces);
	
	const af3d::facemodelbbox<>& bbox=m_pView->getselection()->getbbox(false);

	TCHAR* pNull=nullptr;
	int nFrom=_tcstol(m_csRangeFrom,&pNull,10);
	int nInclusiveTo=_tcstol(m_csRangeInclusiveTo,&pNull,10);
	nFrom=std::max<>(0,nFrom);
	nInclusiveTo=std::min<>(nInclusiveTo,m_nFaces-1);

	const af3d::face_tex_vertex_data<>::type t=static_cast<af3d::face_tex_vertex_data<>::type>(m_nUV+1);
	switch(nVertexAtts)
	{
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormTex->get()[nF].gettex().gettex()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormTexBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormTexBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormTexBump->get()[nF].getbump().getbump()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormColTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormColTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormColTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormColTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormColTex->get()[nF].getpos().getpos()[nV],fb.pPos3NormColTex->get()[nF].gettex().gettex()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormColTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormColTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormColTexBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormColTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormColTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormColTexBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormColTexBump->get()[nF].getpos().getpos()[nV],m_nImage==0?fb.pPos3NormColTexBump->get()[nF].gettex().gettex()[nV]:fb.pPos3NormColTexBump->get()[nF].getbump().getbump()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3ColTex->get()[nF].getpos().getpos()[nV],fb.pPos3ColTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3ColTex->get()[nF].getpos().getpos()[nV],fb.pPos3ColTex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3ColTex->get()[nF].getpos().getpos()[nV],fb.pPos3ColTex->get()[nF].gettex().gettex()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3Tex->get()[nF].getpos().getpos()[nV],fb.pPos3Tex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3Tex->get()[nF].getpos().getpos()[nV],fb.pPos3Tex->get()[nF].gettex().gettex()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3Tex->get()[nF].getpos().getpos()[nV],fb.pPos3Tex->get()[nF].gettex().gettex()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormBump->get()[nF].getbump().getbump()[nV]);break;
			}
		break;
		case af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump:
			switch(t)
			{
				case af3d::face_tex_vertex_data<>::t_flat:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_flat>(bbox,fb.pPos3NormColBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormColBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_cylindrical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_cylindrical>(bbox,fb.pPos3NormColBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormColBump->get()[nF].getbump().getbump()[nV]);break;
				case af3d::face_tex_vertex_data<>::t_spherical:for(int nF=nFrom;nF<=nInclusiveTo;++nF) for(int nV=0;nV<3;++nV) af3d::face_tex_vertex_data<>::getuv<af3d::face_tex_vertex_data<>::t_spherical>(bbox,fb.pPos3NormColBump->get()[nF].getpos().getpos()[nV],fb.pPos3NormColBump->get()[nF].getbump().getbump()[nV]);break;
			}
		break;
		default:return;
	}

	switch(m_nImage)
	{
		case 0:theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_facebuffer_tex,m_pView->getselection()));break;
		case 1:theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_facebuffer_bump,m_pView->getselection()));break;
	}
}

void materialsdlg::onfromtochange(const bool bFrom)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	int n=_tcstol(bFrom?m_csRangeFrom:m_csRangeInclusiveTo,&pNull,10);
	if(n<0)
		n=0;
	else
	if(n>=m_nFaces)
		n=m_nFaces-1;

	if(bFrom)
		setrangefrom(n);
	else
		setrangeinclusiveto(n);
	
	m_bInENChange=false;
}

void materialsdlg::onfromtokillfocus(const bool bFrom)
{
	if(m_nFaces<1)
		return;

	CString csPre,csPost;
	if(bFrom)
		m_RangeFromEdit.GetWindowText(csPre);
	else
		m_RangeInclusiveToEdit.GetWindowText(csPre);
	TCHAR* pNull=nullptr;
	int n=_tcstol(csPre,&pNull,10);
	if(n<0)
		n=0;
	else
	if(n>=m_nFaces)
		n=m_nFaces-1;
	csPost.Format(_T("%li"),n);
	if(csPre==csPost)
		return;

	if(bFrom)
	{
		m_csRangeFrom=csPost;
		setrangefrom(_tcstol(m_csRangeFrom,&pNull,10));
	}
	else
	{
		m_csRangeInclusiveTo=csPost;
		setrangeinclusiveto(_tcstol(m_csRangeFrom,&pNull,10));
	}
}

void materialsdlg::onrangecomboselchanged(void)
{
	UpdateData();

	m_nColourEnable=BST_CHECKED;
	m_nImageEnable=BST_CHECKED;

	m_csRangeFrom=_T("0");
	m_csRangeInclusiveTo=_T("0");

	m_csImage=_T("");

	setfacerange();

	getcolon(m_nColourEnable);
	getimageon(m_nImageEnable);
	
	enabledisable();

	getrangefrom(m_csRangeFrom);
	getrangeinclusiveto(m_csRangeInclusiveTo);

	af3d::vec3<> c;
	getcol(c);
	m_ColWnd.setcol(c);

	getimage(m_csImage);

	m_nRange=m_RangeCombo.GetCurSel();

	UpdateData(false);
}

void materialsdlg::colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview)
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

void materialsdlg::clear(void)
{
	m_pView=nullptr;
	m_pDoc=nullptr;
}

const std::vector<std::shared_ptr<af3d::material<>>> *materialsdlg::getmaterials(void)const
{
	if(m_pView && m_pView->getselection())
		return m_pView->getselection()->getmaterials();
	return nullptr;
}

void materialsdlg::populatematerials(void)
{
	m_RangeCombo.ResetContent();
	const std::vector<std::shared_ptr<af3d::material<>>> *p=getmaterials();
	if(p && p->size())
	{
		auto i=p->cbegin(),end=p->cend();
		for(int n=0;i!=end;++i,++n)
		{
			CString cs;
			cs.Format(_T("Material - %li"),n);
			m_RangeCombo.SetItemDataPtr(m_RangeCombo.AddString(cs),(*i).get());
		}
		m_RangeCombo.SetCurSel(0);
	}
}

af3d::material<> *materialsdlg::getmaterial(void)const
{
	const int n=m_RangeCombo.GetCurSel();
	if(n==CB_ERR || n<0)
		return nullptr;
	const std::vector<std::shared_ptr<af3d::material<>>> *p=getmaterials();
	if(!p)
		return nullptr;
	if(n>=static_cast<int>(p->size()))
		return nullptr;
	return reinterpret_cast<af3d::material<>*>(m_RangeCombo.GetItemDataPtr(n));
}

void materialsdlg::getfacebuffer(void)
{
	propertiesdlg::getfacebuffer(m_pView,facebuffer(),m_nVertexAtts,m_nFaces);
}

void materialsdlg::enabledisable(void)
{
	af3d::material<> *p=getmaterial();
	const bool bSel=m_pView && m_pView->getselection();
	const bool bMats=!!getmaterials();
	const bool bMat=!!p;
	bool bImageVertexAtts=false;
	switch(m_nImage)
	{
		case 0:bImageVertexAtts=m_nVertexAtts&af3d::face_vertex_att::t_tex;break;
		case 1:bImageVertexAtts=m_nVertexAtts&af3d::face_vertex_att::t_bump;break;
		case 2:bImageVertexAtts=true;break;
	}

	m_RangeCombo.EnableWindow(bMats);

	GetDlgItem(IDC_RANGE_ADD)->EnableWindow(bSel);
	GetDlgItem(IDC_RANGE_DEL)->EnableWindow(bMat);

	m_RangeFromEdit.EnableWindow(bMat);
	m_RangeFromSpin.EnableWindow(bMat);
	m_RangeInclusiveToEdit.EnableWindow(bMat);
	m_RangeInclusiveToSpin.EnableWindow(bMat);
	GetDlgItem(IDC_RANGE_ALL)->EnableWindow(bMat);

	m_ColourCombo.EnableWindow(bMat && m_nColourEnable);
	m_ColWnd.EnableWindow(bMat && m_nColourEnable);
	m_ColWnd.Invalidate();
	GetDlgItem(IDC_COLOUR_CHECK)->EnableWindow(bMat);
	GetDlgItem(IDC_SHININESS_STATIC)->EnableWindow(bMat && m_nColourEnable);
	m_ShininessEdit.EnableWindow(bMat && m_nColourEnable);

	m_ImageCombo.EnableWindow(bMat);
	GetDlgItem(IDC_IMAGE_BROWSE)->EnableWindow(bImageVertexAtts && bMat && m_nImageEnable);
	GetDlgItem(IDC_IMAGE_CHECK)->EnableWindow(bImageVertexAtts && bMat);
	GetDlgItem(IDC_IMAGE_STATIC)->EnableWindow(bImageVertexAtts && bMat && m_nImageEnable);
	GetDlgItem(IDC_IMAGE_PATH)->EnableWindow(bImageVertexAtts && bMat && m_nImageEnable);
	m_UVCombo.EnableWindow(m_nImage==2?false:bImageVertexAtts && bMat && m_nImageEnable);
	GetDlgItem(IDC_UV_SET)->EnableWindow(m_nImage==2?false:bImageVertexAtts && bMat && m_nImageEnable);
}

void materialsdlg::setfacerange(void)
{
	if(m_nFaces>0)
	{
		m_RangeFromSpin.SetRange32(0,m_nFaces-1);
		m_RangeInclusiveToSpin.SetRange32(0,m_nFaces-1);
	}
	else
	{
		m_RangeFromSpin.SetRange32(0,0);
		m_RangeInclusiveToSpin.SetRange32(0,0);
	}
}

void materialsdlg::setcolon(const bool b)const
{
	af3d::material<> *pSrc=getmaterial();
	
	if(!pSrc)
		return;

	pSrc->enable(af3d::face_vertex_att::t_col,b);
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_enable,m_pView->getselection(),pSrc));
}

void materialsdlg::setimageon(const bool b)const
{
	af3d::material<> *pSrc=getmaterial();
	
	if(!pSrc)
		return;

	switch(m_nImage)
	{
		case 0:pSrc->enable(af3d::face_vertex_att::t_tex,b);break;
		case 1:pSrc->enable(af3d::face_vertex_att::t_bump,b);break;
		case 2:pSrc->enable(af3d::face_vertex_att::t_env_cubic,b);break;
	}
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_enable,m_pView->getselection(),pSrc));
}

void materialsdlg::getcol(af3d::vec3<>& c)const
{
	af3d::material<> *p=getmaterial();
	if(p && p->getatts(true)&af3d::face_vertex_att::t_col)
		switch(m_nColour)
		{
			case 0:c=p->getcol().getdiffuse();break;
			case 1:c=p->getcol().getspecular();break;
			case 2:c=p->getcol().getambient();break;
		}
}

void materialsdlg::getshininess(CString& cs)const
{
	af3d::material<> *p=getmaterial();
	if(p)
		cs.Format(_T("%f"),p->getcol().getshininess().getexp());
}

void materialsdlg::getcolon(int& n)const
{
	af3d::material<> *p=getmaterial();
	if(p)
		n=p->getatts(true)&af3d::face_vertex_att::t_col?BST_CHECKED:BST_UNCHECKED;
}

void materialsdlg::getimageon(int& n)const
{
	af3d::material<> *p=getmaterial();
	if(p)
		switch(m_nImage)
		{
			case 0:n=p->getatts(true)&af3d::face_vertex_att::t_tex ? BST_CHECKED : BST_UNCHECKED;break;
			case 1:n=p->getatts(true)&af3d::face_vertex_att::t_bump ? BST_CHECKED : BST_UNCHECKED;break;
			case 2:n=p->getatts(true)&af3d::face_vertex_att::t_env_cubic ? BST_CHECKED : BST_UNCHECKED;break;
		}
}

void materialsdlg::getrangefrom(CString& cs)const
{
	af3d::material<> *p=getmaterial();
	if(p)
		cs.Format(_T("%li"),p->getfrom());
}

void materialsdlg::getrangeinclusiveto(CString& cs)const
{
	af3d::material<> *p=getmaterial();
	if(p)
		cs.Format(_T("%li"),p->getinclusiveto());
}

void materialsdlg::getimage(CString& cs)const
{
	af3d::material<> *p=getmaterial();
	switch(m_nImage)
	{
		case 0:
		{
			if(p && p->getatts(true)&af3d::face_vertex_att::t_tex)
				cs=p->gettexpath().c_str();
		}
		break;
		case 1:
		{
			if(p && p->getatts(true)&af3d::face_vertex_att::t_bump)
				cs=p->getbumppath().c_str();
		}
		break;
		case 2:
		{
			if(p && p->getatts(true)&af3d::face_vertex_att::t_env_cubic)
				cs=p->getcubicenvpath().c_str();
		}
		break;
	}
}

void materialsdlg::setcol(const af3d::vec3<>& c)
{
	af3d::material<> *p=getmaterial();
	if(p)
	{
		af3d::materialcol<> matcol=p->getcol();
		switch(m_nColour)
		{
			case 0:matcol.setdiffuse(c);break;
			case 1:matcol.setspecular(c);break;
			case 2:matcol.setambient(c);break;
		}
		p->setcol(matcol);

		theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_col,m_pView->getselection()));
	}
}

void materialsdlg::setrangefrominclusiveto(const int nFrom,const int nInclusiveTo)const
{
	af3d::material<> *p=getmaterial();
	if(!p)
		return;
	p->setrange(nFrom,nInclusiveTo);
	m_pView->getselection()->sortmaterials();

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_range,m_pView->getselection()));
}

void materialsdlg::setrangefrom(const int n)const
{
	af3d::material<> *p=getmaterial();
	if(!p)
		return;
	p->setrange(n,p->getinclusiveto());
	m_pView->getselection()->sortmaterials();

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_range,m_pView->getselection()));
}

void materialsdlg::setrangeinclusiveto(const int n)const
{
	af3d::material<> *p=getmaterial();
	if(!p)
		return;
	p->setrange(p->getfrom(),n);
	m_pView->getselection()->sortmaterials();

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_range,m_pView->getselection()));
}

void materialsdlg::setshininess(const double d)const
{
	af3d::material<> *p=getmaterial();
	if(!p)
		return;
	af3d::materialcol<> c=p->getcol();
	c.setshininess(d);
	p->setcol(c);
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,hint::t_material_shininess,m_pView->getselection()));
}

void materialsdlg::setimage(const CString& cs)const
{
	int y= 0;
}

void materialsdlg::addmaterial(af3d::material<> *p)
{
	if(!p)
		return;
	int n=m_RangeCombo.GetCount();
	CString cs;
	cs.Format(_T("Material - %li"),n);
	while(m_RangeCombo.FindString(-1,cs)!=CB_ERR)
		cs.Format(_T("Material - %li"),++n);
	const int nItem=m_RangeCombo.AddString(cs);
	m_RangeCombo.SetItemDataPtr(nItem,p);
	m_RangeCombo.SetCurSel(nItem);
	
	onrangecomboselchanged();
}

void materialsdlg::delmaterial(af3d::material<> *p)
{
	if(!p)
		return;
	
	int nItem=CB_ERR;
	for(int n=0;n<m_RangeCombo.GetCount() && nItem==CB_ERR;++n)
		if(m_RangeCombo.GetItemDataPtr(n)==p)
			nItem=n;
	if(nItem==CB_ERR)
		return;
	
	m_RangeCombo.DeleteString(nItem);
	if(m_RangeCombo.GetCount())
	{
		if(nItem>=m_RangeCombo.GetCount())
			m_RangeCombo.SetCurSel(m_RangeCombo.GetCount()-1);
		else
			m_RangeCombo.SetCurSel(nItem);
	}

	onrangecomboselchanged();
}

int materialsdlg::getmin(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_TITLE)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

int materialsdlg::getmax(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_UV_COMBO)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

void materialsdlg::onupdate(hint *p)
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
					getfacebuffer();
					populatematerials();
					setfacerange();

					getcolon(m_nColourEnable);
					getimageon(m_nImageEnable);
	
					enabledisable();

					getrangefrom(m_csRangeFrom);
					getrangeinclusiveto(m_csRangeInclusiveTo);

					getshininess(m_csShininess);

					af3d::vec3<> c;
					getcol(c);
					m_ColWnd.setcol(c);

					getimage(m_csImage);

					m_nRange=m_RangeCombo.GetCurSel();

					UpdateData(false);
				}
			}
			break;
			case hint::t_view_stop:
			{
				if(p->getdoc()==m_pDoc)
				{
					clear();
					getfacebuffer();
					populatematerials();
					setfacerange();
					
					m_nColourEnable=BST_CHECKED;
					m_nImageEnable=BST_CHECKED;

					m_csRangeFrom=_T("0");
					m_csRangeInclusiveTo=_T("0");

					m_csImage=_T("");

					m_csShininess=_T("0.0");

					getcolon(m_nColourEnable);
					getimageon(m_nImageEnable);
	
					enabledisable();

					getrangefrom(m_csRangeFrom);
					getrangeinclusiveto(m_csRangeInclusiveTo);

					getshininess(m_csShininess);

					af3d::vec3<> c;
					getcol(c);
					m_ColWnd.setcol(c);

					getimage(m_csImage);

					m_nRange=m_RangeCombo.GetCurSel();

					UpdateData(false);
				}
			}
			break;
			case hint::t_selection:
			{
				getfacebuffer();
				populatematerials();
				setfacerange();

				m_nColourEnable=BST_CHECKED;
				m_nImageEnable=BST_CHECKED;

				m_csRangeFrom=_T("0");
				m_csRangeInclusiveTo=_T("0");

				m_csShininess=_T("0.0");

				m_csImage=_T("");

				getcolon(m_nColourEnable);
				getimageon(m_nImageEnable);
	
				enabledisable();

				getrangefrom(m_csRangeFrom);
				getrangeinclusiveto(m_csRangeInclusiveTo);

				getshininess(m_csShininess);

				af3d::vec3<> c;
				getcol(c);
				m_ColWnd.setcol(c);

				getimage(m_csImage);

				m_nRange=m_RangeCombo.GetCurSel();

				UpdateData(false);
			}
			break;
			case hint::t_material_add:addmaterial(p->getmaterial());break;
			case hint::t_material_del:delmaterial(p->getmaterial());break;
			case hint::t_material_shininess:
			{
				if(!m_bInENChange)
				{
					m_csShininess=_T("0.0");

					getshininess(m_csRangeFrom);

					UpdateData(false);
				}
			}
			break;
			case hint::t_material_range:
			{
				if(!m_bInENChange)
				{
					m_csRangeFrom=_T("0");
					m_csRangeInclusiveTo=_T("0");

					getrangefrom(m_csRangeFrom);
					getrangeinclusiveto(m_csRangeInclusiveTo);

					UpdateData(false);
				}
			}
			break;
			case hint::t_material_col:
			{
				af3d::vec3<> c;
				getcol(c);
				m_ColWnd.setcol(c);
			}
			break;
			case hint::t_material_enable:
			{
				getcolon(m_nColourEnable);
				getimageon(m_nImageEnable);
				enabledisable();

				UpdateData(false);
			}
			break;
			case hint::t_material_image:
			{
				m_csImage=_T("");

				getimage(m_csImage);

				UpdateData(false);
			}
			break;
		}
	m_bInOnUpdate=false;
}
