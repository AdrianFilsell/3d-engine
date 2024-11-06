
#include "pch.h"
#include "propertiesdlg.h"
#include "rasterizerView.h"

propertiesdlg::propertiesdlg(const UINT uiID,CWnd* pParent /*=nullptr*/)
	: CDialogEx(uiID, pParent)
{
	m_nMinMaxGap=2;
	m_bInitialised = false;
}

propertiesdlg::~propertiesdlg()
{
}

void propertiesdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(propertiesdlg, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(WM_TITLEWND_MINIMISE,OnTitleWndMinimise)
END_MESSAGE_MAP()

void propertiesdlg::createtitle(const int nPosID,const int nTitleID,LPCTSTR lpszTitle,const bool bMinimised)
{
	CRect rc;
	GetDlgItem(nPosID)->GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	m_Title.Create(lpszTitle,this,rc,nTitleID);
	m_Title.setminimised(bMinimised);
}

// propertiesdlg message handlers

BOOL propertiesdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_bInitialised = true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void propertiesdlg::OnDestroy(void)
{
	CDialogEx::OnDestroy();
}

void propertiesdlg::OnSize(UINT nType,int cx, int cy)
{
	CDialogEx::OnSize(nType,cx,cy);
}

LPARAM propertiesdlg::OnTitleWndMinimise(WPARAM wParam,LPARAM lParam)
{
	GetParent()->SendMessage(WM_TITLEWND_MINIMISE,wParam,lParam);
	return 0;
}

void propertiesdlg::getfacebuffer(CrasterizerView *pView,facebuffer& fb,int& nVertexAtts,int& nFaces)
{
	nVertexAtts=0;
	nFaces=0;
	fb.pPos3=nullptr;
	if(!pView || !pView->getselection())
		return;
	nVertexAtts=pView->getselection()->getvertexatts();
	switch(nVertexAtts)
	{
		case (af3d::face_vertex_att::t_pos):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3<>>>*>(pView->getselection());
			fb.pPos3=pM->getvertexbuffer();
			if(fb.pPos3)
				nFaces=static_cast<int>(fb.pPos3->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm<>>>*>(pView->getselection());
			fb.pPos3Norm=pM->getvertexbuffer();
			if(fb.pPos3Norm)
				nFaces=static_cast<int>(fb.pPos3Norm->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_bump<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_bump<>>>*>(pView->getselection());
			fb.pPos3NormBump=pM->getvertexbuffer();
			if(fb.pPos3NormBump)
				nFaces=static_cast<int>(fb.pPos3NormBump->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_tex<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_tex<>>>*>(pView->getselection());
			fb.pPos3NormTex=pM->getvertexbuffer();
			if(fb.pPos3NormTex)
				nFaces=static_cast<int>(fb.pPos3NormTex->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_tex_bump<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_tex_bump<>>>*>(pView->getselection());
			fb.pPos3NormTexBump=pM->getvertexbuffer();
			if(fb.pPos3NormTexBump)
				nFaces=static_cast<int>(fb.pPos3NormTexBump->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_bump<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_bump<>>>*>(pView->getselection());
			fb.pPos3NormColBump=pM->getvertexbuffer();
			if(fb.pPos3NormColBump)
				nFaces=static_cast<int>(fb.pPos3NormColBump->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col<>>>*>(pView->getselection());
			fb.pPos3NormCol=pM->getvertexbuffer();
			if(fb.pPos3NormCol)
				nFaces=static_cast<int>(fb.pPos3NormCol->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_tex<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_tex<>>>*>(pView->getselection());
			fb.pPos3NormColTex=pM->getvertexbuffer();
			if(fb.pPos3NormColTex)
				nFaces=static_cast<int>(fb.pPos3NormColTex->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_tex_bump<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_norm_col_tex_bump<>>>*>(pView->getselection());
			fb.pPos3NormColTexBump=pM->getvertexbuffer();
			if(fb.pPos3NormColTexBump)
				nFaces=static_cast<int>(fb.pPos3NormColTexBump->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_col<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_col<>>>*>(pView->getselection());
			fb.pPos3Col=pM->getvertexbuffer();
			if(fb.pPos3Col)
				nFaces=static_cast<int>(fb.pPos3Col->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_col_tex<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_col_tex<>>>*>(pView->getselection());
			fb.pPos3ColTex=pM->getvertexbuffer();
			if(fb.pPos3ColTex)
				nFaces=static_cast<int>(pM->getvertexbuffer()->get().size());
		}
		break;
		case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):
		{
			af3d::mesh<af3d::facebuffer<af3d::face_pos3_tex<>>> *pM=dynamic_cast<af3d::mesh<af3d::facebuffer<af3d::face_pos3_tex<>>>*>(pView->getselection());
			fb.pPos3Tex=pM->getvertexbuffer();
			if(fb.pPos3Tex)
				nFaces=static_cast<int>(fb.pPos3Tex->get().size());
		}
		break;
				
		default:ASSERT(false);break;
	}
}
