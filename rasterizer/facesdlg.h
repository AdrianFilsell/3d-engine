#pragma once

#include "propertiesdlg.h"
#include "colourwnd.h"

class facesdlg : public propertiesdlg
{
public:
	facesdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~facesdlg();

	virtual CWnd *getwnd(void)override{return this;}
	virtual UINT getid(void)const override{return IDC_FACES_PANE;}
	virtual int getmax(void)const override;
	virtual int getmin(void)const override;

	virtual void onupdate(hint *p)override;

// Dialog Data
	enum { IDD = IDD_FACES };
	CComboBox m_VertexCombo;
	CSpinButtonCtrl m_FaceSpin;
	CEdit m_FaceEdit;
	int m_nVertex;

	CEdit m_XYZ_X_Edit;
	CEdit m_XYZ_Y_Edit;
	CEdit m_XYZ_Z_Edit;
	CComboBox m_XYZ_Combo;
	int m_nXYZ;
	CString m_csXYZ_X;
	CString m_csXYZ_Y;
	CString m_csXYZ_Z;

	CEdit m_UV_U_Edit;
	CEdit m_UV_V_Edit;
	CComboBox m_UV_Combo;
	int m_nUV;
	CString m_csUV_U;
	CString m_csUV_V;

	colourwnd m_ColWnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnSize(UINT nType,int cx,int cy);

	afx_msg void OnFaceChange(void);
	afx_msg void OnFaceKillFocus(void);
	afx_msg void OnVertexComboChange(void);

	afx_msg void OnXYZComboChange(void);
	afx_msg void OnXEditChange(void);
	afx_msg void OnXEditKillFocus(void);
	afx_msg void OnYEditChange(void);
	afx_msg void OnYEditKillFocus(void);
	afx_msg void OnZEditChange(void);
	afx_msg void OnZEditKillFocus(void);

	afx_msg void OnUVComboChange(void);
	afx_msg void OnUEditChange(void);
	afx_msg void OnUEditKillFocus(void);
	afx_msg void OnVEditChange(void);
	afx_msg void OnVEditKillFocus(void);
	DECLARE_MESSAGE_MAP()
	
	CrasterizerView *m_pView;
	CrasterizerDoc *m_pDoc;

	facebuffer m_FB;
	int m_nVertexAtts;
	int m_nFaces;

	int m_nBorder;
	int m_nEditGap;
	int m_nEditSpinGap;

	bool m_bInOnUpdate;
	bool m_bInENChange;

	void enabledisable(void);
	void clear(void);

	void getfacebuffer(void);

	bool istexuv(void)const;
	bool isposxyz(void)const;
	bool ismodelspacexyz(void)const;
	void setfacerange(void);
	void settitle(void);

	void onxyzeditchange(const int nC);
	void onxyzeditkillfocus(const int nC);

	void onuveditchange(const int nC);
	void onuveditkillfocus(const int nC);

	af3d::facemodelbbox<> getbbox(void)const;
	af3d::face_pos_vertex_data<af3d::vec3<>> *getpos(void)const;
	af3d::face_norm_vertex_data<> *getnorm(void)const;
	af3d::face_tex_vertex_data<> *gettex(void)const;
	af3d::face_bump_vertex_data<> *getbump(void)const;
	af3d::face_col_vertex_data<> *getcol(void)const;
	
	void getxyz(const int nComponent,CString& cs)const;
	void getuv(const int nComponent,CString& cs)const;
	void getcol(af3d::vec3<>& c)const;

	void setxyz(const int nComponent,const RAS_FLTTYPE d);
	void setuv(const int nComponent,const RAS_FLTTYPE d);
	void setcol(const af3d::vec3<>& c);

	void colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview);
};
