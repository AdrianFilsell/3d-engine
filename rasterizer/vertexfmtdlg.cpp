// vertexfmtdlg.cpp : implementation file
//

#include "pch.h"
#include "rasterizer.h"
#include "afxdialogex.h"
#include "vertexfmtdlg.h"
#include "jpeg.h"


// vertexfmtdlg dialog

IMPLEMENT_DYNAMIC(vertexfmtdlg, CDialogEx)

vertexfmtdlg::vertexfmtdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VERTEX_FMT_DIALOG, pParent)
{
	m_nPosCheck=1;
	
	m_nColCheck=0;
	m_nColGenCheck=1;
	m_Col=af3d::materialcol<>::getdefdiffuse();
	
	m_nNormCheck=1;
	m_nNormGenCheck=0;
	m_nNormGenCombo=0;

	m_nTexCheck=0;
	m_nTexGenCheck=1;
	m_nTexGenCombo=0;

	m_nBumpCheck=0;
	m_nBumpGenCheck=1;
	m_nBumpGenCombo=0;

	m_nMatCubicEnvCheck=0;

	m_nMatTexCheck=0;
	
	m_nMatBumpCheck=0;

	m_nMatColCheck=0;

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
	m_csObjDir=csObjDir+_T("\\")+_T("3dobj");

	m_csTex=m_csObjDir+_T("\\")+_T("landmark.jpg");
	m_csBump=m_csObjDir+_T("\\")+_T("normal.jpg");
	m_csCubicEnv=m_csObjDir+_T("\\")+_T("canyon1.jpg");
	
	m_csMatShininess.Format(_T("%.2f"),m_Mat.getcol().getshininess().getexp());

	m_nOpacity=100;
}

vertexfmtdlg::~vertexfmtdlg()
{
}

void vertexfmtdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX,IDC_POS_CHECK,m_nPosCheck);

	DDX_Check(pDX,IDC_COL_CHECK,m_nColCheck);
	DDX_Check(pDX,IDC_COL_GEN_CHECK,m_nColGenCheck);

	DDX_Check(pDX,IDC_NORM_CHECK,m_nNormCheck);
	DDX_Check(pDX,IDC_NORM_GEN_CHECK,m_nNormGenCheck);
	DDX_CBIndex(pDX,IDC_NORM_COMBO,m_nNormGenCombo);

	DDX_Check(pDX,IDC_TEX_CHECK,m_nTexCheck);
	DDX_Check(pDX,IDC_TEX_GEN_CHECK,m_nTexGenCheck);
	DDX_CBIndex(pDX,IDC_TEX_COMBO,m_nTexGenCombo);

	DDX_Check(pDX,IDC_BUMP_CHECK,m_nBumpCheck);
	DDX_Check(pDX,IDC_BUMP_GEN_CHECK,m_nBumpGenCheck);
	DDX_CBIndex(pDX,IDC_BUMP_COMBO,m_nBumpGenCombo);

	DDX_Check(pDX,IDC_MAT_COL_CHECK,m_nMatColCheck);

	DDX_Check(pDX,IDC_MAT_CUBIC_CHECK,m_nMatCubicEnvCheck);
	DDX_Text(pDX,IDC_MAT_CUBIC_STATIC,m_csCubicEnv);

	DDX_Check(pDX,IDC_MAT_TEX_CHECK,m_nMatTexCheck);
	DDX_Text(pDX,IDC_MAT_TEXTURE_STATIC,m_csTex);

	DDX_Check(pDX,IDC_MAT_BUMP_CHECK,m_nMatBumpCheck);
	DDX_Text(pDX,IDC_MAT_BUMP_STATIC,m_csBump);

	DDX_Text(pDX,IDC_MAT_SHININESS_EDIT,m_csMatShininess);

	DDX_Text(pDX,IDC_OPACITY_EDIT,m_nOpacity);
}


BEGIN_MESSAGE_MAP(vertexfmtdlg, CDialogEx)
	ON_BN_CLICKED(IDC_NORM_CHECK,OnNorm)
	ON_CBN_SELCHANGE(IDC_NORM_COMBO,OnUpdateData)
	ON_BN_CLICKED(IDC_NORM_GEN_CHECK,OnNormGen)

	ON_BN_CLICKED(IDC_COL_CHECK,OnCol)
	ON_BN_CLICKED(IDC_COL_GEN_CHECK,OnColGen)

	ON_BN_CLICKED(IDC_TEX_CHECK,OnTex)
	ON_CBN_SELCHANGE(IDC_TEX_COMBO,OnUpdateData)
	ON_BN_CLICKED(IDC_TEX_GEN_CHECK,OnTexGen)

	ON_BN_CLICKED(IDC_BUMP_CHECK,OnBump)
	ON_CBN_SELCHANGE(IDC_BUMP_COMBO,OnUpdateData)
	ON_BN_CLICKED(IDC_BUMP_GEN_CHECK,OnBumpGen)

	ON_BN_CLICKED(IDC_MAT_COL_CHECK,OnMatCol)
	ON_EN_KILLFOCUS(IDC_MAT_SHININESS_EDIT,OnShininessEdit)

	ON_BN_CLICKED(IDC_MAT_TEX_CHECK,OnMatTex)
	ON_BN_CLICKED(IDC_MAT_TEXTURE_BUTTON,OnMatTexBtn)

	ON_BN_CLICKED(IDC_MAT_BUMP_CHECK,OnMatBump)
	ON_BN_CLICKED(IDC_MAT_BUMP_BUTTON,OnMatBumpBtn)

	ON_BN_CLICKED(IDC_MAT_CUBIC_CHECK,OnMatCubicEnv)
	ON_BN_CLICKED(IDC_MAT_CUBIC_BUTTON,OnMatCubicEnvBtn)
END_MESSAGE_MAP()

BOOL vertexfmtdlg::OnInitDialog()
{
	BOOL b = CDialogEx::OnInitDialog();
	
	CRect rc;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_ColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,1024);
	m_ColWnd.setcol(m_Col);
	auto fn = std::bind( &vertexfmtdlg::colourcallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
	m_ColWnd.setfn(fn,true);

	GetDlgItem(IDC_MAT_DIFFUSE_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_MatDiffuseColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,1024+1);
	m_MatDiffuseColWnd.setcol(m_Mat.getcol().getdiffuse());
	m_MatDiffuseColWnd.setfn(fn,true);

	GetDlgItem(IDC_MAT_AMBIENT_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_MatAmbientColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,1024+2);
	m_MatAmbientColWnd.setcol(m_Mat.getcol().getambient());
	m_MatAmbientColWnd.setfn(fn,true);

	GetDlgItem(IDC_MAT_SPECULAR_COLOUR)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_MatSpecularColWnd.Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_HAND)),_T(""),WS_BORDER | WS_CHILD | WS_VISIBLE,rc,this,1024+3);
	m_MatSpecularColWnd.setcol(m_Mat.getcol().getspecular());
	m_MatSpecularColWnd.setfn(fn,true);

	reinterpret_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_OPACITY_SPIN))->SetRange(0,100);
	reinterpret_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_OPACITY_SPIN))->SetPos(m_nOpacity);

	EnablePos();
	EnableNorm();
	EnableCol();
	EnableTex();
	EnableBump();

	EnableMatCol();
	EnableMatTex();
	EnableMatBump();
	EnableMatCubicEnv();

	return b;
}

void vertexfmtdlg::OnOK(void)
{
	CDialogEx::OnOK();

	const bool bVertexCol=m_nColCheck==1;
	const bool bVertexTex=m_nTexCheck==1;
	const bool bVertexBump=m_nBumpCheck==1;

	const bool bMatCubic=m_nMatCubicEnvCheck==1;
	const bool bMatTex=m_nMatTexCheck==1;
	const bool bMatBump=m_nMatBumpCheck==1;
	const bool bMatCol=(!bVertexCol && !bVertexTex && !bMatCubic && !(m_nMatColCheck==1) && !(m_nMatTexCheck==1) && !(m_nMatCubicEnvCheck==1));
	
	m_Mat.clear(af3d::face_vertex_att::t_material);

	if(bMatCol)
		m_Mat.setcol(af3d::materialcol<>());
	if(bMatCubic)
	{
		std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(m_csCubicEnv);
		if(spDib)
		{
			std::shared_ptr<af3d::materialcubicenvmap> sp(new af3d::materialcubicenvmap(af3d::rect({0,0},{spDib->getwidth()/4,spDib->getwidth()/4}),spDib));
			m_Mat.setcubicenv(std::wstring(m_csCubicEnv),sp);
		}
	}
	if(bMatTex)
	{
		std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(m_csTex);
		if(spDib)
		{
			std::shared_ptr<af3d::materialdib> sp(new af3d::materialdib(spDib));
			m_Mat.settex(std::wstring(m_csTex),sp);
		}
	}
	if(bMatBump)
	{
		std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(m_csBump);
		if(spDib)
		{
			std::shared_ptr<af3d::materialnormalmap<>> sp(new af3d::materialnormalmap<>(spDib));
			m_Mat.setbump(std::wstring(m_csBump),sp);
		}
	}
}

// vertexfmtdlg message handlers
void vertexfmtdlg::OnUpdateData()
{
	UpdateData();
}

void vertexfmtdlg::OnNorm()
{
	UpdateData();
	EnableNorm();
}

void vertexfmtdlg::OnNormGen()
{
	UpdateData();
	EnableNorm();
}

void vertexfmtdlg::OnCol()
{
	UpdateData();
	EnableCol();
}

void vertexfmtdlg::colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview)
{
	switch(nID)
	{
		case 1024+0:m_ColWnd.setcol(c);m_Col={m_ColWnd.getcol()[0],m_ColWnd.getcol()[1],m_ColWnd.getcol()[2]};break;
		case 1024+1:
		{
			m_MatDiffuseColWnd.setcol(c);
			af3d::materialcol<> c=m_Mat.getcol();
			c.setdiffuse({m_MatDiffuseColWnd.getcol()[0],m_MatDiffuseColWnd.getcol()[1],m_MatDiffuseColWnd.getcol()[2]});
			m_Mat.setcol(c);
		}
		break;
		case 1024+2:
		{
			m_MatAmbientColWnd.setcol(c);
			af3d::materialcol<> c=m_Mat.getcol();
			c.setambient({m_MatAmbientColWnd.getcol()[0],m_MatAmbientColWnd.getcol()[1],m_MatAmbientColWnd.getcol()[2]});
			m_Mat.setcol(c);
		}
		break;
		case 1024+3:
		{
			m_MatSpecularColWnd.setcol(c);
			af3d::materialcol<> c=m_Mat.getcol();
			c.setspecular({m_MatSpecularColWnd.getcol()[0],m_MatSpecularColWnd.getcol()[1],m_MatSpecularColWnd.getcol()[2]});
			m_Mat.setcol(c);
		}
		break;
	}
}

void vertexfmtdlg::OnColGen()
{
	UpdateData();
	EnableCol();
}

void vertexfmtdlg::OnShininessEdit()
{
	UpdateData();
	const RAS_FLTTYPE d=_tstof((LPCTSTR)m_csMatShininess);
	af3d::materialcol<> c=m_Mat.getcol();
	c.setshininess(d);
	CString cs;
	cs.Format(_T("%.2f"),d);
	if(cs!=m_csMatShininess)
	{
		m_csMatShininess=cs;
		UpdateData(false);
	}
}

void vertexfmtdlg::OnTex()
{
	UpdateData();
	EnableTex();
}

void vertexfmtdlg::OnTexGen()
{
	UpdateData();
	EnableTex();
}

void vertexfmtdlg::OnBump()
{
	UpdateData();
	EnableBump();
}

void vertexfmtdlg::OnBumpGen()
{
	UpdateData();
	EnableBump();
}

void vertexfmtdlg::OnMatCol()
{
	UpdateData();
	EnableMatCol();
}

void vertexfmtdlg::OnMatTex()
{
	UpdateData();
	EnableMatTex();
}

void vertexfmtdlg::OnMatTexBtn()
{
	CFileDialog dlg(true);
	if(dlg.DoModal()==IDOK)
	{
		m_csTex=dlg.GetPathName();
		UpdateData(false);
	}
}

void vertexfmtdlg::OnMatBump()
{
	UpdateData();
	EnableMatBump();
}

void vertexfmtdlg::OnMatBumpBtn()
{
	CFileDialog dlg(true);
	if(dlg.DoModal()==IDOK)
	{
		m_csBump=dlg.GetPathName();
		UpdateData(false);
	}
}

void vertexfmtdlg::OnMatCubicEnv()
{
	UpdateData();
	EnableMatCubicEnv();
}

void vertexfmtdlg::OnMatCubicEnvBtn()
{
	CFileDialog dlg(true);
	if(dlg.DoModal()==IDOK)
	{
		m_csCubicEnv=dlg.GetPathName();
		UpdateData(false);
	}
}

void vertexfmtdlg::EnablePos()
{
	GetDlgItem(IDC_POS_CHECK)->EnableWindow(false);
}

void vertexfmtdlg::EnableNorm()
{
	const bool bVertex = m_nNormCheck==1;
	const bool bGen = m_nNormGenCheck==1;

	GetDlgItem(IDC_NORM_GEN_CHECK)->EnableWindow(bVertex);
	GetDlgItem(IDC_NORM_COMBO)->EnableWindow(bVertex && bGen);
}

void vertexfmtdlg::EnableCol()
{
	const bool bVertex = m_nColCheck==1;
	const bool bGen = m_nColGenCheck==1;

	GetDlgItem(IDC_COL_GEN_CHECK)->EnableWindow(bVertex);
	if(m_ColWnd.GetSafeHwnd())
	{
		m_ColWnd.EnableWindow(bVertex && bGen);
		m_ColWnd.Invalidate();
	}
}

void vertexfmtdlg::EnableTex()
{
	const bool bVertex = m_nTexCheck==1;
	const bool bGen = m_nTexGenCheck==1;

	GetDlgItem(IDC_TEX_GEN_CHECK)->EnableWindow(bVertex);
	GetDlgItem(IDC_TEX_COMBO)->EnableWindow(bVertex && bGen);
}

void vertexfmtdlg::EnableBump()
{
	const bool bVertex = m_nBumpCheck==1;
	const bool bGen = m_nBumpGenCheck==1;

	GetDlgItem(IDC_BUMP_GEN_CHECK)->EnableWindow(bVertex);
	GetDlgItem(IDC_BUMP_COMBO)->EnableWindow(bVertex && bGen);
}

void vertexfmtdlg::EnableMatCol()
{
	const bool bMat = m_nMatColCheck==1;

	if(m_MatDiffuseColWnd.GetSafeHwnd())
	{
		m_MatDiffuseColWnd.EnableWindow(bMat);
		m_MatDiffuseColWnd.Invalidate();
	}
	if(m_MatSpecularColWnd.GetSafeHwnd())
	{
		m_MatSpecularColWnd.EnableWindow(bMat);
		m_MatSpecularColWnd.Invalidate();
	}
	if(m_MatAmbientColWnd.GetSafeHwnd())
	{
		m_MatAmbientColWnd.EnableWindow(bMat);
		m_MatAmbientColWnd.Invalidate();
	}
	GetDlgItem(IDC_MAT_DIFFUSE)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_AMBIENT)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_SPECULAR)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_SHININESS)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_SHININESS_EDIT)->EnableWindow(bMat);
}

void vertexfmtdlg::EnableMatTex()
{
	const bool bMat = m_nMatTexCheck==1;

	GetDlgItem(IDC_MAT_TEXTURE_STATIC)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_TEXTURE_BUTTON)->EnableWindow(bMat);
}

void vertexfmtdlg::EnableMatBump()
{
	const bool bMat = m_nMatBumpCheck==1;

	GetDlgItem(IDC_MAT_BUMP_STATIC)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_BUMP_BUTTON)->EnableWindow(bMat);
}

void vertexfmtdlg::EnableMatCubicEnv()
{
	const bool bMat = m_nMatCubicEnvCheck==1;

	GetDlgItem(IDC_MAT_CUBIC_STATIC)->EnableWindow(bMat);
	GetDlgItem(IDC_MAT_CUBIC_BUTTON)->EnableWindow(bMat);
}

CString vertexfmtdlg::getfnameext(const CString& cs)const
{
	TCHAR drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
	_tsplitpath_s((LPCTSTR)cs,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	return CString(fname)+CString(ext);
}
