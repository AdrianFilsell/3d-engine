#pragma once

#include "3d.h"
#include <functional>

class colourwnd : public CWnd
{
DECLARE_DYNAMIC(colourwnd)

public:
	colourwnd();
	virtual ~colourwnd(){}
	const af3d::vec3<>& getcol(void)const{return m_col;}
	void setcol(const af3d::vec3<>& c){if(m_col==c)return;m_col=c;if(GetSafeHwnd()){Invalidate();}}
	void setfn(std::function<void(const int nID,const af3d::vec3<>& c,const bool bPreview)> fn,const bool bPreview){m_fn=fn;m_bPreview=bPreview;}
protected:
	afx_msg void OnPaint(void);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	bool m_bPreview;
	af3d::vec3<> m_col;
	UINT_PTR m_nTimer;
	int m_nTimerInterval;
	CMFCColorDialog *m_pDlg;
	std::function<void(const int nID,const af3d::vec3<>& c,const bool bPreview)> m_fn;
};
