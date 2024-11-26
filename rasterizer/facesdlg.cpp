
#include "pch.h"
#include "facesdlg.h"
#include "hint.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "splitterwnd.h"

facesdlg::facesdlg(CWnd* pParent /*=nullptr*/)
	: propertiesdlg(IDD, pParent)
{
	m_bInOnUpdate=false;
	m_bInENChange=false;
	m_pView=nullptr;
	m_pDoc=nullptr;

	m_nVertex=0;

	m_csXYZ_X=_T("0.0");
	m_csXYZ_Y=_T("0.0");
	m_csXYZ_Z=_T("0.0");
	m_nXYZ=0;

	m_nUV=0;
	m_csUV_U=_T("0.0");
	m_csUV_V=_T("0.0");

	m_nVertexAtts=0;
	m_nFaces=0;

	m_nBorder = 2;
	m_nEditGap=0;
	m_nEditSpinGap=0;
}

facesdlg::~facesdlg()
{
}

void facesdlg::DoDataExchange(CDataExchange* pDX)
{
	propertiesdlg::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_VERTEX_COMBO,m_VertexCombo);
	DDX_Control(pDX,IDC_FACES_SPIN,m_FaceSpin);
	DDX_Control(pDX,IDC_FACES_EDIT,m_FaceEdit);

	DDX_Control(pDX,IDC_XYZ_X_EDIT,m_XYZ_X_Edit);
	DDX_Control(pDX,IDC_XYZ_Y_EDIT,m_XYZ_Y_Edit);
	DDX_Control(pDX,IDC_XYZ_Z_EDIT,m_XYZ_Z_Edit);
	DDX_Control(pDX,IDC_XYZ_COMBO,m_XYZ_Combo);

	DDX_Control(pDX,IDC_UV_U_EDIT,m_UV_U_Edit);
	DDX_Control(pDX,IDC_UV_V_EDIT,m_UV_V_Edit);
	DDX_Control(pDX,IDC_UV_COMBO,m_UV_Combo);

	DDX_CBIndex(pDX,IDC_VERTEX_COMBO,m_nVertex);

	DDX_CBIndex(pDX,IDC_XYZ_COMBO,m_nXYZ);
	DDX_Text(pDX,IDC_XYZ_X_EDIT,m_csXYZ_X);
	DDX_Text(pDX,IDC_XYZ_Y_EDIT,m_csXYZ_Y);
	DDX_Text(pDX,IDC_XYZ_Z_EDIT,m_csXYZ_Z);

	DDX_CBIndex(pDX,IDC_UV_COMBO,m_nUV);
	DDX_Text(pDX,IDC_UV_U_EDIT,m_csUV_U);
	DDX_Text(pDX,IDC_UV_V_EDIT,m_csUV_V);
}


BEGIN_MESSAGE_MAP(facesdlg,propertiesdlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()

	ON_EN_CHANGE(IDC_FACES_EDIT,OnFaceChange)
	ON_EN_KILLFOCUS(IDC_FACES_EDIT,OnFaceKillFocus)
	ON_CBN_SELCHANGE(IDC_VERTEX_COMBO,OnVertexComboChange)

	ON_CBN_SELCHANGE(IDC_XYZ_COMBO,OnXYZComboChange)
	ON_EN_CHANGE(IDC_XYZ_X_EDIT,OnXEditChange)
	ON_EN_KILLFOCUS(IDC_XYZ_X_EDIT,OnXEditKillFocus)
	ON_EN_CHANGE(IDC_XYZ_Y_EDIT,OnYEditChange)
	ON_EN_KILLFOCUS(IDC_XYZ_Y_EDIT,OnYEditKillFocus)
	ON_EN_CHANGE(IDC_XYZ_Z_EDIT,OnZEditChange)
	ON_EN_KILLFOCUS(IDC_XYZ_Z_EDIT,OnZEditKillFocus)

	ON_CBN_SELCHANGE(IDC_UV_COMBO,OnUVComboChange)
	ON_EN_CHANGE(IDC_UV_U_EDIT,OnUEditChange)
	ON_EN_KILLFOCUS(IDC_UV_U_EDIT,OnUEditKillFocus)
	ON_EN_CHANGE(IDC_UV_V_EDIT,OnVEditChange)
	ON_EN_KILLFOCUS(IDC_UV_V_EDIT,OnVEditKillFocus)
END_MESSAGE_MAP()


// facesdlg message handlers

BOOL facesdlg::OnInitDialog()
{
	propertiesdlg::OnInitDialog();

	createtitle(IDC_TITLE,IDC_FACE_TITLE,_T("Faces"),true);

	m_FaceSpin.SetRange32(0,0);
		
	CRect rcXYZ_X,rcXYZ_Y,rcFacesEdit,rcFacesSpin;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{&m_FaceEdit,&rcFacesEdit},{&m_FaceSpin,&rcFacesSpin},{&m_XYZ_X_Edit,&rcXYZ_X},{&m_XYZ_Y_Edit,&rcXYZ_Y}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	m_nEditGap=rcXYZ_Y.left-rcXYZ_X.right;
	m_nEditSpinGap=rcFacesSpin.left-rcFacesEdit.right;

	m_XYZ_Combo.AddString(_T("Normal - (m\x207B\x00B9)\x1d40 space"));

	CRect rc;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_ColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,IDC_FACE_COLWND);
	m_ColWnd.setcol({0,0,0});
	auto fn = std::bind( &facesdlg::colourcallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
	m_ColWnd.setfn(fn,true);

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL facesdlg::OnEraseBkgnd(CDC *pDC)
{
	std::vector<CWnd*> vErase;
	vErase.push_back(&m_Title);
	vErase.push_back(&m_VertexCombo);
	vErase.push_back(GetDlgItem(IDC_VERTEX_STATIC));
	vErase.push_back(&m_FaceSpin);
	vErase.push_back(&m_FaceEdit);
	vErase.push_back(GetDlgItem(IDC_INDEX_STATIC));
	vErase.push_back(&m_XYZ_X_Edit);
	vErase.push_back(&m_XYZ_Y_Edit);
	vErase.push_back(&m_XYZ_Z_Edit);
	vErase.push_back(&m_XYZ_Combo);
	vErase.push_back(GetDlgItem(IDC_XYZ_EDGE));
	vErase.push_back(&m_UV_U_Edit);
	vErase.push_back(&m_UV_V_Edit);
	vErase.push_back(&m_UV_Combo);
	vErase.push_back(GetDlgItem(IDC_UV_EDGE));
	vErase.push_back(&m_ColWnd);
	vErase.push_back(GetDlgItem(IDC_COLOUR_STATIC));
	vErase.push_back(GetDlgItem(IDC_COLOUR_EDGE));

	splitterwnd::excludecliprect(pDC,vErase);

	return propertiesdlg::OnEraseBkgnd(pDC);
}

void facesdlg::OnSize(UINT nType,int cx,int cy)
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

	CRect rcVertexCombo,rcVertexStatic,rcFaceSpin,rcFaceEdit,rcIndexStatic;
	std::vector<std::pair<CWnd*,CRect*>> vRects={{&m_VertexCombo,&rcVertexCombo},{GetDlgItem(IDC_VERTEX_STATIC),&rcVertexStatic},{&m_FaceSpin,&rcFaceSpin},{&m_FaceEdit,&rcFaceEdit},{GetDlgItem(IDC_INDEX_STATIC),&rcIndexStatic}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nSpinGap = rcVertexStatic.left - rcFaceSpin.right;
		const int nStaticGap = rcFaceEdit.left-rcIndexStatic.right;

		{
			const int nGap = rcVertexCombo.left-rcVertexStatic.right;
			rcVertexCombo.OffsetRect((rcTitle.right-m_nBorder)-rcVertexCombo.right,0);
			vRepos.push_back({&m_VertexCombo,rcVertexCombo});

			rcVertexStatic.OffsetRect((rcVertexCombo.left-nGap)-rcVertexStatic.right,0);
			vRepos.push_back({GetDlgItem(IDC_VERTEX_STATIC),rcVertexStatic});
		}

		rcFaceSpin.OffsetRect((rcVertexStatic.left-nSpinGap)-rcFaceSpin.right,0);
		vRepos.push_back({&m_FaceSpin,rcFaceSpin});

		rcIndexStatic.OffsetRect((rcTitle.left+m_nBorder)-rcIndexStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_INDEX_STATIC),rcIndexStatic});

		{
			rcFaceEdit.left=rcIndexStatic.right+nStaticGap;
			rcFaceEdit.right=rcFaceSpin.left-m_nEditSpinGap;
			vRepos.push_back({&m_FaceEdit,rcFaceEdit});
		}
	}

	CRect rcXYZ_Combo,rcXYZ_X,rcXYZ_Y,rcXYZ_Z,rcXYZ_Edge;
	vRects={{&m_XYZ_X_Edit,&rcXYZ_X},{&m_XYZ_Y_Edit,&rcXYZ_Y},{&m_XYZ_Z_Edit,&rcXYZ_Z},{&m_XYZ_Combo,&rcXYZ_Combo},{GetDlgItem(IDC_XYZ_EDGE),&rcXYZ_Edge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nGap = rcXYZ_X.left-rcXYZ_Combo.right;
		rcXYZ_Combo.OffsetRect((rcTitle.left+m_nBorder)-rcXYZ_Combo.left,0);
		vRepos.push_back({&m_XYZ_Combo,rcXYZ_Combo});

		const int nEditGap = m_nEditGap;
		const double dWidth=((rcTitle.right-m_nBorder)-(rcTitle.left+m_nBorder))-(2*nEditGap);
		const int nWidth=static_cast<int>(dWidth/3.0 + 0.5);
			
		rcXYZ_X.OffsetRect((rcXYZ_Combo.right+nGap)-rcXYZ_X.left,0);
		rcXYZ_X.right=rcXYZ_X.left+nWidth;
		vRepos.push_back({&m_XYZ_X_Edit,rcXYZ_X});

		rcXYZ_Y.OffsetRect((rcXYZ_X.right+nEditGap)-rcXYZ_Y.left,0);
		rcXYZ_Y.right=rcXYZ_Y.left+nWidth;
		vRepos.push_back({&m_XYZ_Y_Edit,rcXYZ_Y});

		rcXYZ_Z.OffsetRect((rcTitle.right-m_nBorder)-rcXYZ_Z.right,0);
		rcXYZ_Z.left=rcXYZ_Z.right-nWidth;
		vRepos.push_back({&m_XYZ_Z_Edit,rcXYZ_Z});

		rcXYZ_Edge.OffsetRect((rcTitle.left+m_nBorder)-rcXYZ_Edge.left,0);
		rcXYZ_Edge.right=rcTitle.right-m_nBorder;
		vRepos.push_back({GetDlgItem(IDC_XYZ_EDGE),rcXYZ_Edge});
	}

	CRect rcUV_Combo,rcUV_U,rcUV_V,rcUV_Edge;
	vRects={{&m_UV_U_Edit,&rcUV_U},{&m_UV_V_Edit,&rcUV_V},{&m_UV_Combo,&rcUV_Combo},{GetDlgItem(IDC_UV_EDGE),&rcUV_Edge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nGap = rcUV_U.left-rcUV_Combo.right;
		rcUV_Combo.OffsetRect((rcTitle.left+m_nBorder)-rcUV_Combo.left,0);
		vRepos.push_back({&m_UV_Combo,rcUV_Combo});

		const int nEditGap = m_nEditGap;
		const double dWidth=((rcTitle.right-m_nBorder)-(rcTitle.left+m_nBorder))-(1*nEditGap);
		const int nWidth=static_cast<int>(dWidth/2.0 + 0.5);
			
		rcUV_U.OffsetRect((rcUV_Combo.right+nGap)-rcUV_U.left,0);
		rcUV_U.right=rcUV_U.left+nWidth;
		vRepos.push_back({&m_UV_U_Edit,rcUV_U});

		rcUV_V.OffsetRect((rcUV_U.right+nEditGap)-rcUV_V.left,0);
		rcUV_V.right=rcUV_V.left+nWidth;
		vRepos.push_back({&m_UV_V_Edit,rcUV_V});

		rcUV_Edge.OffsetRect((rcTitle.left+m_nBorder)-rcUV_Edge.left,0);
		rcUV_Edge.right=rcTitle.right-m_nBorder;
		vRepos.push_back({GetDlgItem(IDC_UV_EDGE),rcUV_Edge});
	}

	CRect rcColWnd,rcColStatic,rcColEdge;
	vRects={{&m_ColWnd,&rcColWnd},{GetDlgItem(IDC_COLOUR_STATIC),&rcColStatic},{GetDlgItem(IDC_COLOUR_EDGE),&rcColEdge}};
	for(std::pair<CWnd*,CRect*>& r : vRects) {if(r.first->GetSafeHwnd()){r.first->GetWindowRect(*r.second);ScreenToClient(*r.second);}}

	{
		const int nStaticGap=rcColWnd.left-rcColStatic.right;

		rcColStatic.OffsetRect((rcTitle.left+m_nBorder)-rcColStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_COLOUR_STATIC),rcColStatic});

		rcColWnd.OffsetRect((rcColStatic.right+nStaticGap)-rcColWnd.left,0);
		rcColWnd.right=rcTitle.right-m_nBorder;
		vRepos.push_back({&m_ColWnd,rcColWnd});

		rcColEdge.OffsetRect((rcTitle.left+m_nBorder)-rcColEdge.left,0);
		rcColEdge.right=rcTitle.right-m_nBorder;
		vRepos.push_back({GetDlgItem(IDC_COLOUR_EDGE),rcColEdge});
	}

	splitterwnd::repos(vRepos);
}

void facesdlg::OnFaceChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	getxyz(0,m_csXYZ_X);
	getxyz(1,m_csXYZ_Y);
	getxyz(2,m_csXYZ_Z);

	getuv(0,m_csUV_U);
	getuv(1,m_csUV_V);

	af3d::vec3<> c;
	getcol(c);
	m_ColWnd.setcol(c);

	UpdateData(false);
}

void facesdlg::OnFaceKillFocus(void)
{
	if(m_nFaces<1)
		return;

	CString csPre,csPost;
	m_FaceEdit.GetWindowText(csPre);
	const int nFace=_tstol(csPre);
	csPost.Format(_T("%li"),nFace);
	if(csPre==csPost && nFace>=0 && nFace<m_nFaces)
		return;

	if(nFace<0)
		m_FaceSpin.SetPos(0);
	else
	if(nFace>=m_nFaces)
		m_FaceSpin.SetPos(m_nFaces-1);
	else
		m_FaceSpin.SetPos(nFace);
}

void facesdlg::OnVertexComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	getxyz(0,m_csXYZ_X);
	getxyz(1,m_csXYZ_Y);
	getxyz(2,m_csXYZ_Z);

	getuv(0,m_csUV_U);
	getuv(1,m_csUV_V);

	af3d::vec3<> c;
	getcol(c);
	m_ColWnd.setcol(c);

	enabledisable();

	UpdateData(false);
}

void facesdlg::OnXYZComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	m_csXYZ_X=_T("0.0");
	m_csXYZ_Y=_T("0.0");
	m_csXYZ_Z=_T("0.0");

	getxyz(0,m_csXYZ_X);
	getxyz(1,m_csXYZ_Y);
	getxyz(2,m_csXYZ_Z);

	enabledisable();

	UpdateData(false);
}

void facesdlg::OnXEditChange(void)
{
	onxyzeditchange(0);
}

void facesdlg::OnXEditKillFocus( void )
{
	onxyzeditkillfocus(0);
}

void facesdlg::OnYEditChange(void)
{
	onxyzeditchange(1);
}

void facesdlg::OnYEditKillFocus( void )
{
	onxyzeditkillfocus(1);
}

void facesdlg::OnZEditChange(void)
{
	onxyzeditchange(2);
}

void facesdlg::OnZEditKillFocus( void )
{
	onxyzeditkillfocus(2);
}

void facesdlg::OnUVComboChange(void)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;
	
	UpdateData();

	m_csUV_U=_T("0.0");
	m_csUV_V=_T("0.0");

	getuv(0,m_csUV_U);
	getuv(1,m_csUV_V);

	enabledisable();

	UpdateData(false);
}

void facesdlg::OnUEditChange(void)
{
	onuveditchange(0);
}

void facesdlg::OnUEditKillFocus( void )
{
	onuveditkillfocus(0);
}

void facesdlg::OnVEditChange(void)
{
	onuveditchange(1);
}

void facesdlg::OnVEditKillFocus( void )
{
	onuveditkillfocus(1);
}

void facesdlg::colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview)
{
	switch(nID)
	{
		case IDC_FACE_COLWND:
		{
			m_ColWnd.setcol(c);
			setcol(c);
		}
		break;
	}
}

void facesdlg::onxyzeditchange(const int nC)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	switch(nC)
	{
		case 0:setxyz(nC,_tcstod(m_csXYZ_X,&pNull));break;
		case 1:setxyz(nC,_tcstod(m_csXYZ_Y,&pNull));break;
		case 2:setxyz(nC,_tcstod(m_csXYZ_Z,&pNull));break;
	}

	m_bInENChange=false;
}

void facesdlg::onxyzeditkillfocus(const int nC)
{
	if(m_nFaces<1)
		return;

	CString csPre,csPost;
	switch(nC)
	{
		case 0:m_XYZ_X_Edit.GetWindowText(csPre);break;
		case 1:m_XYZ_Y_Edit.GetWindowText(csPre);break;
		case 2:m_XYZ_Z_Edit.GetWindowText(csPre);break;
	}
	TCHAR* pNull=nullptr;
	const RAS_FLTTYPE d=_tcstod(csPre,&pNull);
	csPost.Format(_T("%f"),d);
	if(csPre==csPost)
		return;

	switch(nC)
	{
		case 0:setxyz(nC,_tcstod(csPost,&pNull));break;
		case 1:setxyz(nC,_tcstod(csPost,&pNull));break;
		case 2:setxyz(nC,_tcstod(csPost,&pNull));break;
	}
}

void facesdlg::onuveditchange(const int nC)
{
	if(!m_bInitialised)
		return;
	if(m_bInOnUpdate)
		return;

	UpdateData();

	m_bInENChange=true;

	TCHAR* pNull=nullptr;
	switch(nC)
	{
		case 0:setuv(nC,_tcstod(m_csUV_U,&pNull));break;
		case 1:setuv(nC,_tcstod(m_csUV_V,&pNull));break;
	}

	m_bInENChange=false;
}

void facesdlg::onuveditkillfocus(const int nC)
{
	if(m_nFaces<1)
		return;

	CString csPre,csPost;
	switch(nC)
	{
		case 0:m_UV_U_Edit.GetWindowText(csPre);break;
		case 1:m_UV_V_Edit.GetWindowText(csPre);break;
	}
	TCHAR* pNull=nullptr;
	const RAS_FLTTYPE d=_tcstod(csPre,&pNull);
	csPost.Format(_T("%f"),d);
	if(csPre==csPost)
		return;

	switch(nC)
	{
		case 0:setuv(nC,_tcstod(csPost,&pNull));break;
		case 1:setuv(nC,_tcstod(csPost,&pNull));break;
	}
}

void facesdlg::enabledisable(void)
{
	const bool bEnable=m_nFaces>0;
	
	GetDlgItem(IDC_VERTEX_STATIC)->EnableWindow(bEnable);
	GetDlgItem(IDC_INDEX_STATIC)->EnableWindow(bEnable);
	m_FaceEdit.EnableWindow(bEnable);
	m_FaceSpin.EnableWindow(bEnable);
	m_VertexCombo.EnableWindow(bEnable);

	const bool bPosEnable=bEnable && (m_nVertexAtts & af3d::face_vertex_att::t_pos);
	const bool bNormEnable=bEnable && (m_nVertexAtts & af3d::face_vertex_att::t_norm);
	m_XYZ_Combo.EnableWindow(bPosEnable || bNormEnable);
	switch(m_nXYZ)
	{
		case 0:
		case 1:
		{
			m_XYZ_X_Edit.EnableWindow(bPosEnable);
			m_XYZ_Y_Edit.EnableWindow(bPosEnable);
			m_XYZ_Z_Edit.EnableWindow(bPosEnable);
		}
		break;
		case 2:
		case 3:
		{
			m_XYZ_X_Edit.EnableWindow(bNormEnable);
			m_XYZ_Y_Edit.EnableWindow(bNormEnable);
			m_XYZ_Z_Edit.EnableWindow(bNormEnable);
		}
		break;
	}

	const bool bTexEnable=bEnable && (m_nVertexAtts & af3d::face_vertex_att::t_tex);
	const bool bBumpEnable=bEnable && (m_nVertexAtts & af3d::face_vertex_att::t_bump);
	m_UV_Combo.EnableWindow(bTexEnable || bBumpEnable);
	switch(m_nUV)
	{
		case 0:
		{
			m_UV_U_Edit.EnableWindow(bTexEnable);
			m_UV_V_Edit.EnableWindow(bTexEnable);
		}
		break;
		case 1:
		{
			m_UV_U_Edit.EnableWindow(bBumpEnable);
			m_UV_V_Edit.EnableWindow(bBumpEnable);
		}
		break;
	}

	const bool bColEnable=bEnable && (m_nVertexAtts & af3d::face_vertex_att::t_col);
	m_ColWnd.EnableWindow(bColEnable);
	m_ColWnd.Invalidate();
	GetDlgItem(IDC_COLOUR_STATIC)->EnableWindow(bEnable);
}

void facesdlg::setfacerange(void)
{
	if(m_nFaces>0)
		m_FaceSpin.SetRange32(0,m_nFaces-1);
	else
		m_FaceSpin.SetRange32(0,0);
	m_FaceSpin.SetPos(0);
}

void facesdlg::settitle(void)
{
	CString cs;
	if(m_nFaces>1)
		cs.Format(_T("Faces - %li"),m_nFaces);
	else
		cs=_T("Faces");

	if(m_Title.GetSafeHwnd())
	{
		m_Title.SetWindowText(cs);
		m_Title.Invalidate();
	}
}

void facesdlg::setxyz(const int nComponent,const RAS_FLTTYPE d)
{
	if(m_nFaces<1 || nComponent<0 || nComponent>2)
		return;

	af3d::facemodelbbox<> bbox=m_pView->getselection()->getbbox(false);

	hint::type h=hint::t_facebuffer_pos;
	bool bInvalidateBBox=false;
	switch(m_nXYZ)
	{
		case 0:
		{
			af3d::face_pos_vertex_data<af3d::vec3<>> *p=getpos();
			if(!p)
				return;
			p->getpos()[m_nVertex][nComponent]=d;
			bbox=getbbox();
			bInvalidateBBox=true;
		}
		break;
		case 1:
		{
			af3d::face_pos_vertex_data<af3d::vec3<>> *p=getpos();
			if(!p)
				return;
			af3d::vec3<> w,m;
			m_pView->getselection()->getcompositetrns().mul(p->getpos()[m_nVertex],w);
			w[nComponent]=d;
			af3d::mat4<>::mul(m_pView->getselection()->getcompositetrns().inverse(),w,m);
			p->getpos()[m_nVertex]=m;
			bbox=getbbox();
			bInvalidateBBox=true;
		}
		break;
		case 2:
		{
			af3d::face_norm_vertex_data<> *p=getnorm();
			if(!p)
				return;
			p->getnorm()[m_nVertex][nComponent]=d;
			p->setsingular();
			h=hint::t_facebuffer_norm;
		}
		break;
		case 3:
		{
			af3d::face_norm_vertex_data<> *p=getnorm();
			if(!p)
				return;
			af3d::vec3<> w,m;
			af3d::mat4<>::mul(m_pView->getselection()->getcompositetrns().inverse().transpose(),p->getnorm()[m_nVertex],w);
			w[nComponent]=d;
			af3d::mat4<>::mul(m_pView->getselection()->getcompositetrns().inverse().transpose().inverse(),w,m);
			p->getnorm()[m_nVertex]=m;
			p->setsingular();
			h=hint::t_facebuffer_norm;
		}
		break;
		default:return;
	}
	m_pView->getselection()->setbbox(bbox);
	if(bInvalidateBBox)
		m_pView->getselection()->invalidatecompositebbox();

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,h,m_pView->getselection()));
}

void facesdlg::setcol(const af3d::vec3<>& c)
{
	if(m_nFaces<1)
		return;
	hint::type h=hint::t_facebuffer_col;
	af3d::face_col_vertex_data<> *p=getcol();
	p->getcol()[m_nVertex]=c;
	p->setsingular();
	
	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,h,m_pView->getselection()));
}

void facesdlg::setuv(const int nComponent,const RAS_FLTTYPE d)
{
	if(m_nFaces<1 || nComponent<0 || nComponent>1)
		return;
	hint::type h=hint::t_facebuffer_tex;
	switch(m_nXYZ)
	{
		case 0:
		{
			af3d::face_tex_vertex_data<> *p=gettex();
			if(!p)
				return;
			p->gettex()[m_nVertex][nComponent]=d;
		}
		break;
		case 1:
		{
			af3d::face_bump_vertex_data<> *p=getbump();
			if(!p)
				return;
			p->getbump()[m_nVertex][nComponent]=d;
			h=hint::t_facebuffer_bump;
		}
		break;
		default:return;
	}

	theApp.UpdateAllViews(&hint(m_pView,m_pDoc,h,m_pView->getselection()));
}

void facesdlg::getxyz(const int nComponent,CString& cs)const
{
	if(m_nFaces<1 || nComponent<0 || nComponent>2)
		return;
	switch(m_nXYZ)
	{
		case 0:
		{
			const af3d::face_pos_vertex_data<af3d::vec3<>> *p=getpos();
			if(!p)
				return;
			cs.Format(_T("%f"),p->getpos()[m_nVertex][nComponent]);
		}
		break;
		case 1:
		{
			const af3d::face_pos_vertex_data<af3d::vec3<>> *p=getpos();
			if(!p)
				return;
			af3d::vec3<> v;
			m_pView->getselection()->getcompositetrns().mul(p->getpos()[m_nVertex],v);
			cs.Format(_T("%f"),v[nComponent]);
		}
		break;
		case 2:
		{
			const af3d::face_norm_vertex_data<> *p=getnorm();
			if(!p)
				return;
			cs.Format(_T("%f"),p->getnorm()[m_nVertex][nComponent]);
		}
		break;
		case 3:
		{
			const af3d::face_norm_vertex_data<> *p=getnorm();
			if(!p)
				return;
			af3d::vec3<> v;
			af3d::mat4<>::mul(m_pView->getselection()->getcompositetrns().inverse().transpose(),p->getnorm()[m_nVertex],v);
			cs.Format(_T("%f"),v[nComponent]);
		}
		break;
	}
}

void facesdlg::getuv(const int nComponent,CString& cs)const
{
	if(nComponent<0 || nComponent>1)
		return;
	switch(m_nUV)
	{
		case 0:
		{
			const af3d::face_tex_vertex_data<> *p=gettex();
			if(!p)
				return;
			cs.Format(_T("%f"),p->gettex()[m_nVertex][nComponent]);
		}
		break;
		case 1:
		{
			const af3d::face_bump_vertex_data<> *p=getbump();
			if(!p)
				return;
			cs.Format(_T("%f"),p->getbump()[m_nVertex][nComponent]);
		}
		break;
	}
}

void facesdlg::getcol(af3d::vec3<>& c)const
{
	const af3d::face_col_vertex_data<> *p=getcol();
	if(!p)
		return;
	c=p->getcol()[m_nVertex];
}

bool facesdlg::ismodelspacexyz(void)const
{
	return m_nXYZ==0 || m_nXYZ==2;
}

bool facesdlg::isposxyz(void)const
{
	return m_nXYZ==0 || m_nXYZ==1;
}

bool facesdlg::istexuv(void)const
{
	return m_nUV==0;
}

af3d::facemodelbbox<> facesdlg::getbbox(void)const
{
	if(m_nFaces<1)
		return af3d::facemodelbbox<>();
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos):return af3d::facemodelbbox<>(m_FB.pPos3->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):return af3d::facemodelbbox<>(m_FB.pPos3Norm->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):return af3d::facemodelbbox<>(m_FB.pPos3NormBump->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):return af3d::facemodelbbox<>(m_FB.pPos3NormTex->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return af3d::facemodelbbox<>(m_FB.pPos3NormTexBump->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):return af3d::facemodelbbox<>(m_FB.pPos3NormColBump->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):return af3d::facemodelbbox<>(m_FB.pPos3NormCol->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return af3d::facemodelbbox<>(m_FB.pPos3NormColTex->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return af3d::facemodelbbox<>(m_FB.pPos3NormColTexBump->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):return af3d::facemodelbbox<>(m_FB.pPos3Col->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return af3d::facemodelbbox<>(m_FB.pPos3ColTex->get());
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):return af3d::facemodelbbox<>(m_FB.pPos3Tex->get());
				
		default:return af3d::facemodelbbox<>();
	}
}

af3d::face_pos_vertex_data<af3d::vec3<>> *facesdlg::getpos(void)const
{
	if(m_nFaces<1 || m_nVertex<0 || m_nVertex>2)
		return nullptr;
	const int nFace=m_FaceSpin.GetPos();
	if(nFace<0 || nFace>=m_nFaces)
		return nullptr;
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos):return &m_FB.pPos3->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):return &m_FB.pPos3Norm->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormBump->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormTex->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormTexBump->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColBump->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):return &m_FB.pPos3NormCol->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormColTex->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColTexBump->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):return &m_FB.pPos3Col->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3ColTex->get()[nFace].getpos();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):return &m_FB.pPos3Tex->get()[nFace].getpos();
				
		default:return nullptr;
	}
}

af3d::face_col_vertex_data<> *facesdlg::getcol(void)const
{
	if(m_nFaces<1 || m_nVertex<0 || m_nVertex>2)
		return nullptr;
	const int nFace=m_FaceSpin.GetPos();
	if(nFace<0 || nFace>=m_nFaces)
		return nullptr;
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColBump->get()[nFace].getcol();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):return &m_FB.pPos3NormCol->get()[nFace].getcol();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormColTex->get()[nFace].getcol();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColTexBump->get()[nFace].getcol();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):return &m_FB.pPos3Col->get()[nFace].getcol();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3ColTex->get()[nFace].getcol();
						
		default:return nullptr;
	}
}

af3d::face_norm_vertex_data<> *facesdlg::getnorm(void)const
{
	if(m_nFaces<1 || m_nVertex<0 || m_nVertex>2)
		return nullptr;
	const int nFace=m_FaceSpin.GetPos();
	if(nFace<0 || nFace>=m_nFaces)
		return nullptr;
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):return &m_FB.pPos3Norm->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormBump->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormTex->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormTexBump->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColBump->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):return &m_FB.pPos3NormCol->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormColTex->get()[nFace].getnorm();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColTexBump->get()[nFace].getnorm();
				
		default:return nullptr;
	}
}

af3d::face_tex_vertex_data<> *facesdlg::gettex(void)const
{
	if(m_nFaces<1 || m_nVertex<0 || m_nVertex>2)
		return nullptr;
	const int nFace=m_FaceSpin.GetPos();
	if(nFace<0 || nFace>=m_nFaces)
		return nullptr;
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormTex->get()[nFace].gettex();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormTexBump->get()[nFace].gettex();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3NormColTex->get()[nFace].gettex();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColTexBump->get()[nFace].gettex();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):return &m_FB.pPos3ColTex->get()[nFace].gettex();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):return &m_FB.pPos3Tex->get()[nFace].gettex();
				
		default:return nullptr;
	}
}

af3d::face_bump_vertex_data<> *facesdlg::getbump(void)const
{
	if(m_nFaces<1 || m_nVertex<0 || m_nVertex>2)
		return nullptr;
	const int nFace=m_FaceSpin.GetPos();
	if(nFace<0 || nFace>=m_nFaces)
		return nullptr;
	switch(m_nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormBump->get()[nFace].getbump();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormTexBump->get()[nFace].getbump();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColBump->get()[nFace].getbump();
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):return &m_FB.pPos3NormColTexBump->get()[nFace].getbump();
				
		default:return nullptr;
	}
}

void facesdlg::getfacebuffer(void)
{
	propertiesdlg::getfacebuffer(m_pView,m_FB,m_nVertexAtts,m_nFaces);
}

void facesdlg::clear(void)
{
	m_pView=nullptr;
	m_pDoc=nullptr;
}

int facesdlg::getmin(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_TITLE)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

int facesdlg::getmax(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

void facesdlg::onupdate(hint *p)
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
					enabledisable();

					setfacerange();
					settitle();

					getxyz(0,m_csXYZ_X);
					getxyz(1,m_csXYZ_Y);
					getxyz(2,m_csXYZ_Z);

					getuv(0,m_csUV_U);
					getuv(1,m_csUV_V);

					af3d::vec3<> c;
					getcol(c);
					m_ColWnd.setcol(c);

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
					enabledisable();

					setfacerange();
					settitle();

					m_csXYZ_X=_T("0.0");
					m_csXYZ_Y=_T("0.0");
					m_csXYZ_Z=_T("0.0");
				
					m_csUV_U=_T("0.0");
					m_csUV_V=_T("0.0");

					af3d::vec3<> c(0,0,0);
					
					getxyz(0,m_csXYZ_X);
					getxyz(1,m_csXYZ_Y);
					getxyz(2,m_csXYZ_Z);

					getuv(0,m_csUV_U);
					getuv(1,m_csUV_V);

					getcol(c);
					m_ColWnd.setcol(c);

					UpdateData(false);
				}
			}
			break;
			case hint::t_selection:
			{
				getfacebuffer();
				enabledisable();

				setfacerange();
				settitle();
				
				m_csXYZ_X=_T("0.0");
				m_csXYZ_Y=_T("0.0");
				m_csXYZ_Z=_T("0.0");
				
				m_csUV_U=_T("0.0");
				m_csUV_V=_T("0.0");

				af3d::vec3<> c(0,0,0);
				
				getxyz(0,m_csXYZ_X);
				getxyz(1,m_csXYZ_Y);
				getxyz(2,m_csXYZ_Z);

				getuv(0,m_csUV_U);
				getuv(1,m_csUV_V);

				getcol(c);
				m_ColWnd.setcol(c);

				UpdateData(false);
			}
			break;
			case hint::t_frame_reparent:
			{
				getxyz(0,m_csXYZ_X);
				getxyz(1,m_csXYZ_Y);
				getxyz(2,m_csXYZ_Z);

				UpdateData(false);
			}
			break;
			case hint::t_drag:
			{
				if(!ismodelspacexyz())
				{
					getxyz(0,m_csXYZ_X);
					getxyz(1,m_csXYZ_Y);
					getxyz(2,m_csXYZ_Z);

					UpdateData(false);
				}
			}
			break;
			case hint::t_facebuffer_pos:
			{
				if(isposxyz() && !m_bInENChange)
				{
					getxyz(0,m_csXYZ_X);
					getxyz(1,m_csXYZ_Y);
					getxyz(2,m_csXYZ_Z);

					UpdateData(false);
				}
			}
			break;
			case hint::t_facebuffer_norm:
			{
				if(!isposxyz() && !m_bInENChange)
				{
					getxyz(0,m_csXYZ_X);
					getxyz(1,m_csXYZ_Y);
					getxyz(2,m_csXYZ_Z);

					UpdateData(false);
				}
			}
			break;
			case hint::t_facebuffer_tex:
			{
				if(istexuv() && !m_bInENChange)
				{
					getuv(0,m_csUV_U);
					getuv(1,m_csUV_V);

					UpdateData(false);
				}
			}
			break;
			case hint::t_facebuffer_bump:
			{
				if(!istexuv() && !m_bInENChange)
				{
					getuv(0,m_csUV_U);
					getuv(1,m_csUV_V);

					UpdateData(false);
				}
			}
			break;
			case hint::t_facebuffer_col:
			{
				af3d::vec3<> c;
				getcol(c);
				m_ColWnd.setcol(c);
			}
			break;
		}
	m_bInOnUpdate=false;
}
