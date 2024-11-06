
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "propertiesbar.h"

class CrasterizerView;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

	CrasterizerView *getactiveview( void ) { return m_pActiveView; }
	void setactiveview( void );

	DWORD getinsizing(void)const{return m_dwInSizing;}

	void onupdate(hint *p);
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar			m_wndToolBar;
	CToolBar			m_wnd3DToolBar;
	CStatusBar			m_wndStatusBar;
	propertiesbar		m_wndProperties;
	CrasterizerView*	m_pActiveView;
	std::shared_ptr<CImageList> m_sp3DToolBarIL;

	DWORD m_dwInSizing;
	
	void setactiveviewex( CrasterizerView *pView );

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivateApp( BOOL bActive, DWORD dwThreadID );
	afx_msg void OnEnterSizeMove(void);
	afx_msg void OnExitSizeMove(void);
	DECLARE_MESSAGE_MAP()
};
