#pragma once

#include "afxdialogex.h"
#include "titlewnd.h"
#include "resource.h"
#include "hint.h"
#include "splitterwnd.h"

class CrasterizerView;

class propertiesdlg : public CDialogEx, public splitterwndpane
{
public:
	propertiesdlg(const UINT uiID,CWnd* pParent = nullptr);   // standard constructor
	virtual ~propertiesdlg();
	
	virtual bool getminimised(void)const override{return m_Title.getminimised();}
	virtual void setminimised(const bool b)override{m_Title.setminimised(b);}
	
	virtual void onupdate(hint *p){}
protected:
	bool m_bInitialised;
	int m_nMinMaxGap;
	titlewnd m_Title;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnDestroy(void);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg LPARAM OnTitleWndMinimise(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	void createtitle(const int nPosID,const int nTitleID,LPCTSTR lpszTitle,const bool bMinimised);

	union facebuffer
	{
		facebuffer(){}
		~facebuffer(){}

		af3d::facebuffer<af3d::face_pos3<>> *pPos3;
		af3d::facebuffer<af3d::face_pos3_norm<>> *pPos3Norm;
		af3d::facebuffer<af3d::face_pos3_norm_bump<>> *pPos3NormBump;
		af3d::facebuffer<af3d::face_pos3_norm_tex<>> *pPos3NormTex;
		af3d::facebuffer<af3d::face_pos3_norm_tex_bump<>> *pPos3NormTexBump;
		af3d::facebuffer<af3d::face_pos3_norm_col_bump<>> *pPos3NormColBump;
		af3d::facebuffer<af3d::face_pos3_norm_col<>> *pPos3NormCol;
		af3d::facebuffer<af3d::face_pos3_norm_col_tex<>> *pPos3NormColTex;
		af3d::facebuffer<af3d::face_pos3_norm_col_tex_bump<>> *pPos3NormColTexBump;
		af3d::facebuffer<af3d::face_pos3_col<>> *pPos3Col;
		af3d::facebuffer<af3d::face_pos3_col_tex<>> *pPos3ColTex;
		af3d::facebuffer<af3d::face_pos3_tex<>> *pPos3Tex;
	};
	static void getfacebuffer(CrasterizerView *pView,facebuffer& fb,int& nVertexAtts,int& nFaces);
};
