#pragma once

#include "propertiesdlg.h"
#include "scenetree.h"
#include "3d_frame.h"
#include <map>

class scenedlg : public propertiesdlg
{
public:
	scenedlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~scenedlg();

	virtual CWnd *getwnd(void)override{return this;}
	virtual UINT getid(void)const override{return IDC_FRAMES_PANE;}
	virtual int getmax(void)const override;
	virtual int getmin(void)const override;

	virtual void onupdate(hint *p)override;

	virtual BOOL PreTranslateMessage(MSG* pMsg)override;

// Dialog Data
	enum { IDD = IDD_SCENE };
	scenetree m_Tree;
	CSliderCtrl m_OpacitySlider;
	int m_nEffectCombo;
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	afx_msg void OnTreeSelChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeItemEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTreeItemReparent(WPARAM wParam,LPARAM lParam);
	afx_msg void OnEffectComboSelChanged(void);
	DECLARE_MESSAGE_MAP()

	CrasterizerView *m_pView;
	CrasterizerDoc *m_pDoc;
	std::map<af3d::vertexattsframe<>*,HTREEITEM> m_Map;
	int m_nSliderComboGap;
	bool m_bInOnUpdate;
	
	void clear(void);
	void populate(af3d::vertexattsframe<> *p);
	void expandframe(af3d::vertexattsframe<> *p);
	void eraseframe(af3d::vertexattsframe<> *p);
	void selectframe(af3d::vertexattsframe<> *p);
	void setframecheck(af3d::vertexattsframe<> *p);
	void setframename(af3d::vertexattsframe<> *p);
	void setframeopacity(af3d::vertexattsframe<> *p);
	void setframeeffect(af3d::vertexattsframe<> *p);
	HTREEITEM appendframe(af3d::vertexattsframe<> *p);
	HTREEITEM insertframe(af3d::vertexattsframe<> *p);

	void oneffectcomboselchanged(void);
};
