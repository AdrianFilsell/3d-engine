#pragma once

#include "Resource.h"

#include "afxdialogex.h"
#include "colourwnd.h"
#include "3d_obj.h"
#include "3d_material.h"


// vertexfmtdlg dialog

class vertexfmtdlg : public CDialogEx
{
	DECLARE_DYNAMIC(vertexfmtdlg)

public:
	vertexfmtdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~vertexfmtdlg();

// Dialog Data
	enum { IDD = IDD_VERTEX_FMT_DIALOG };
	int m_nPosCheck;
	int m_nColCheck;
	int m_nColGenCheck;
	af3d::vec3<> m_Col;
	int m_nNormCheck;
	int m_nNormGenCheck;
	int m_nNormGenCombo;
	int m_nTexCheck;
	int m_nTexGenCheck;
	int m_nTexGenCombo;
	int m_nBumpCheck;
	int m_nBumpGenCheck;
	int m_nBumpGenCombo;
	int m_nMatCubicEnvCheck;
	CString m_csCubicEnv;
	int m_nMatTexCheck;
	CString m_csTex;
	int m_nMatBumpCheck;
	CString m_csBump;
	int m_nMatColCheck;
	CString m_csMatShininess;
	int m_nOpacity;
	
	colourwnd m_ColWnd;
	colourwnd m_MatDiffuseColWnd;
	colourwnd m_MatAmbientColWnd;
	colourwnd m_MatSpecularColWnd;

	af3d::material<> m_Mat;
	CString m_csObjDir;

	virtual BOOL OnInitDialog();

	af3d::objcfg<> getobjcfg(void)const
	{
		const std::pair<bool,af3d::vec3<>> c=std::make_pair(m_nColGenCheck==1,m_Col);
		const af3d::face_norm_vertex_data<>::type n=m_nNormGenCheck==0?af3d::face_norm_vertex_data<>::t_null:(m_nNormGenCombo==0?af3d::face_norm_vertex_data<>::t_average:af3d::face_norm_vertex_data<>::t_geometric);
		const af3d::face_tex_vertex_data<>::type tex=m_nTexGenCheck==0?af3d::face_tex_vertex_data<>::t_null:(m_nTexGenCombo==0?af3d::face_tex_vertex_data<>::t_flat:(m_nTexGenCombo==1?af3d::face_tex_vertex_data<>::t_cylindrical:af3d::face_tex_vertex_data<>::t_spherical));
		const af3d::face_tex_vertex_data<>::type bump=m_nBumpGenCheck==0?af3d::face_tex_vertex_data<>::t_null:(m_nBumpGenCombo==0?af3d::face_tex_vertex_data<>::t_flat:(m_nBumpGenCombo==1?af3d::face_tex_vertex_data<>::t_cylindrical:af3d::face_tex_vertex_data<>::t_spherical));

		int nVertexAtts=af3d::face_vertex_att::t_pos;
		if(m_nTexCheck)nVertexAtts|=af3d::face_vertex_att::t_tex;
		if(m_nColCheck)nVertexAtts|=af3d::face_vertex_att::t_col;
		if(m_nNormCheck)nVertexAtts|=af3d::face_vertex_att::t_norm;
		if(m_nBumpCheck)nVertexAtts|=af3d::face_vertex_att::t_bump;

		return {nVertexAtts,c,n,tex,bump,m_Mat,m_nOpacity/100.0};
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(void);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateData();

	afx_msg void OnNorm();
	afx_msg void OnNormGen();

	afx_msg void OnCol();
	afx_msg void OnColGen();

	afx_msg void OnTex();
	afx_msg void OnTexGen();

	afx_msg void OnBump();
	afx_msg void OnBumpGen();

	afx_msg void OnMatCol();
	afx_msg void OnShininessEdit();

	afx_msg void OnMatTex();
	afx_msg void OnMatTexBtn();

	afx_msg void OnMatBump();
	afx_msg void OnMatBumpBtn();

	afx_msg void OnMatCubicEnv();
	afx_msg void OnMatCubicEnvBtn();

	void EnablePos();
	void EnableNorm();
	void EnableCol();
	void EnableTex();
	void EnableBump();

	void EnableMatCol();
	void EnableMatBump();
	void EnableMatTex();
	void EnableMatCubicEnv();

	CString getfnameext(const CString& cs)const;

	void colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview);
};
