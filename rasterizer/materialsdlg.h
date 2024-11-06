#pragma once

#include "propertiesdlg.h"
#include "colourwnd.h"

class materialsdlg : public propertiesdlg
{
public:
	materialsdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~materialsdlg();

	virtual CWnd *getwnd(void)override{return this;}
	virtual UINT getid(void)const override{return IDC_MATERIALS_PANE;}
	virtual int getmax(void)const override;
	virtual int getmin(void)const override;

	virtual void onupdate(hint *p)override;

// Dialog Data
	enum { IDD = IDD_MATERIALS };
	CComboBox m_RangeCombo;
	int m_nRange;
	CSpinButtonCtrl m_RangeFromSpin;
	CSpinButtonCtrl m_RangeInclusiveToSpin;
	CEdit m_RangeFromEdit;
	CEdit m_RangeInclusiveToEdit;
	CString m_csRangeFrom;
	CString m_csRangeInclusiveTo;
	
	CComboBox m_ColourCombo;
	colourwnd m_ColWnd;
	int m_nColour;
	CEdit m_ShininessEdit;
	CString m_csShininess;
	int m_nColourEnable;
	
	CComboBox m_ImageCombo;
	int m_nImage;
	CString m_csImage;
	int m_nImageEnable;

	CComboBox m_UVCombo;
	int m_nUV;
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnRangeComboSelChanged(void);
	afx_msg void OnAdd(void);
	afx_msg void OnDel(void);
	afx_msg void OnAll(void);

	afx_msg void OnFromChange(void);
	afx_msg void OnFromKillFocus(void);
	afx_msg void OnToChange(void);
	afx_msg void OnToKillFocus(void);
	
	afx_msg void OnColourComboSelChanged(void);
	afx_msg void OnColourCheck(void);

	afx_msg void OnShininessChange(void);
	afx_msg void OnShininessKillFocus(void);

	afx_msg void OnImageComboSelChanged(void);
	afx_msg void OnImageCheck(void);
	afx_msg void OnImageBrowse(void);

	afx_msg void OnUVComboSelChanged(void);
	afx_msg void OnUVSet(void);
	DECLARE_MESSAGE_MAP()

	CrasterizerView *m_pView;
	CrasterizerDoc *m_pDoc;

	bool m_bInOnUpdate;
	bool m_bInENChange;

	int m_nFaces;
	int m_nVertexAtts;

	int m_nBorder;
	int m_nRangeCheckGap;
	int m_nRangeEditGap;
	int m_nEditSpinGap;
	int m_nColourGap;
	
	af3d::material<> *materialsdlg::getmaterial(void)const;
	const std::vector<std::shared_ptr<af3d::material<>>> *getmaterials(void)const;

	void populatematerials(void);
	void setfacerange(void);
	void getfacebuffer(void);
	void enabledisable(void);
	void clear(void);

	void addmaterial(af3d::material<> *p);
	void delmaterial(af3d::material<> *p);

	void getcol(af3d::vec3<>& c)const;
	void getcolon(int& n)const;
	void getrangefrom(CString& cs)const;
	void getrangeinclusiveto(CString& cs)const;
	void getshininess(CString& cs)const;
	void getimage(CString& cs)const;
	void getimageon(int& n)const;

	void setcol(const af3d::vec3<>& c);
	void setcolon(const bool b)const;
	void setrangefrom(const int n)const;
	void setrangeinclusiveto(const int n)const;
	void setrangefrominclusiveto(const int nFrom,const int nInclusiveTo)const;
	void setshininess(const double d)const;
	void setimage(const CString& cs)const;
	void setimageon(const bool b)const;

	void onfromtochange(const bool bFrom);
	void onfromtokillfocus(const bool bFrom);
	void onrangecomboselchanged(void);

	void colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview);
};
