#pragma once

#include "propertiesdlg.h"
#include "colourwnd.h"

class lightdlg : public propertiesdlg
{
public:
	lightdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~lightdlg();

	virtual CWnd *getwnd(void)override{return this;}
	virtual UINT getid(void)const override{return IDC_LIGHT_PANE;}
	virtual int getmax(void)const override;
	virtual int getmin(void)const override;

	virtual void onupdate(hint *p)override;

// Dialog Data
	enum { IDD = IDD_LIGHT };
	CEdit m_RangeEdit;
	CString m_csRange;
	CComboBox m_AttenCombo;
	CEdit m_AttenEdit;
	int m_nAtten;
	CString m_csAtten;
	colourwnd m_ColWnd;
	CComboBox m_ColourCombo;
	int m_nColour;
	CComboBox m_SpotCombo;
	CEdit m_SpotEdit;
	int m_nSpot;
	CString m_csSpot;
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnSize(UINT nType,int cx,int cy);

	afx_msg void OnColourComboChange(void);

	afx_msg void OnAttenChange(void);
	afx_msg void OnAttenKillFocus(void);
	afx_msg void OnAttenComboChange(void);

	afx_msg void OnSpotChange(void);
	afx_msg void OnSpotKillFocus(void);
	afx_msg void OnSpotComboChange(void);
	DECLARE_MESSAGE_MAP()

	CrasterizerView *m_pView;
	CrasterizerDoc *m_pDoc;

	bool m_bInOnUpdate;
	bool m_bInENChange;
	
	int m_nBorder;
	int m_nEditGap;

	af3d::light<> *getlightcache(void)const;

	void enabledisable(void);
	void clear(void);

	void getrange(CString& cs)const;
	void getatten(CString& cs)const;
	void getcol(af3d::vec3<>& c)const;
	void getspot(CString& cs)const;

	void setcol(const af3d::vec3<>& c);
	void setatten(const RAS_FLTTYPE d);
	void setrange(const RAS_FLTTYPE d);
	void setspot(const RAS_FLTTYPE d);

	void colourcallback(const int nID,const af3d::vec3<>& c,const bool bPreview);
};
