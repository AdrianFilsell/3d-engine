
// rasterizerView.h : interface of the CrasterizerView class
//

#pragma once

#include "surface.h"
#include "viewtool.h"
#include <map>

class CrasterizerDoc;

class CrasterizerView : public CView
{
protected: // create from serialization only
	CrasterizerView() noexcept;
	DECLARE_DYNCREATE(CrasterizerView)

// Attributes
public:
	CrasterizerDoc* GetDocument() const;

	void setactive(const bool b);
	bool getactive(void)const{return m_bActive;}

	bool captureinput(void);
	bool releasecapturedinput(void);

	af3d::vec2<int> getclientcursorpos( void ) const;

	void setselection(af3d::vertexattsframe<> *p);
	af3d::vertexattsframe<> *getselection(void)const{return m_pSel;}

	surface *getsurface(void)const{return m_spSurface.get();}

	int getrendertypes(void)const{return m_nRender;}

	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate()override;
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

// Implementation
public:
	virtual ~CrasterizerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
protected:
	std::shared_ptr<afdib::dib> m_spOffscreenDib;

	bool m_bActive;

	bool m_bStopping;
	
	af3d::vertexattsframe<> *m_pSel;

	int m_nRender;

	std::shared_ptr<surface> m_spSurface;
	af3d::rect m_rComposeInflate;
	
	std::map<const viewtool::type,std::shared_ptr<viewtool>> m_mTools;
	viewtool *m_pSelectedTool;

	void pushbacktool(std::shared_ptr<viewtool> sp);
	void setselectedtool(viewtool *p);
	viewtool *gettool(const viewtool::type t) const;

	void start(void);
	void stop(void);

	void compose(const af3d::rect& r);
	void composeshadowmaps(const af3d::rect& r);
	void composeshadowmap(const af3d::vertexattsframe<>* pLight,const af3d::rect& r);
	
	void createtools(void);

// Generated message map functions
protected:
	afx_msg void OnCameraFront(void);
	afx_msg void OnUpdateCameraFront(CCmdUI *pCmdUI);
	afx_msg void OnCameraBack(void);
	afx_msg void OnUpdateCameraBack(CCmdUI *pCmdUI);
	afx_msg void OnCameraLeft(void);
	afx_msg void OnUpdateCameraLeft(CCmdUI *pCmdUI);
	afx_msg void OnCameraRight(void);
	afx_msg void OnUpdateCameraRight(CCmdUI *pCmdUI);
	afx_msg void OnCameraAbove(void);
	afx_msg void OnUpdateCameraAbove(CCmdUI *pCmdUI);
	afx_msg void OnCameraBelow(void);
	afx_msg void OnUpdateCameraBelow(CCmdUI *pCmdUI);

	afx_msg void OnNewCameraDuplicate(void);

	afx_msg void OnLightRender(void);
	afx_msg void OnUpdateLightRender(CCmdUI *pCmdUI);
	afx_msg void OnModelRender(void);
	afx_msg void OnUpdateModelRender(CCmdUI *pCmdUI);
	afx_msg void OnShadowRender(void);
	afx_msg void OnUpdateShadowRender(CCmdUI *pCmdUI);

	afx_msg void OnImportObj(void);
	afx_msg void OnCreateSpotLight(void);
	afx_msg void OnCreatePointLight(void);
	afx_msg void OnCreateDirectionalLight(void);

	afx_msg void OnDelete(void);
	
	afx_msg void OnFPS(void);

	afx_msg void OnTranslateScaleTool(void);
	afx_msg void OnUpdateTranslateScaleTool(CCmdUI *pCmdUI);
	afx_msg void OnDepthTool(void);
	afx_msg void OnUpdateDepthTool(CCmdUI *pCmdUI);
	afx_msg void OnRotateTool(void);
	afx_msg void OnUpdateRotateTool(CCmdUI *pCmdUI);
	afx_msg void OnCameraTranslateTool(void);
	afx_msg void OnUpdateCameraTranslateTool(CCmdUI *pCmdUI);
	afx_msg void OnCameraDepthTool(void);
	afx_msg void OnUpdateCameraDepthTool(CCmdUI *pCmdUI);
	afx_msg void OnCameraRotateTool(void);
	afx_msg void OnUpdateCameraRotateTool(CCmdUI *pCmdUI);
	
	afx_msg void OnCancel(void);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnDestroy(void);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd* pGainWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()

	bool getvertexfacecount(const af3d::vertexattsframe<> *p,int& n)const;
	template <typename F> int getvertexfacecount(const af3d::vertexattsframe<> *p)const
	{
		const af3d::mesh<af3d::facebuffer<F>> *pMesh=static_cast<const af3d::mesh<af3d::facebuffer<F>>*>(p);
		if(pMesh->getvertexbuffer())
			return static_cast<int>(pMesh->getvertexbuffer()->get().size());
		return 0;
	}
};

#ifndef _DEBUG  // debug version in rasterizerView.cpp
inline CrasterizerDoc* CrasterizerView::GetDocument() const
   { return reinterpret_cast<CrasterizerDoc*>(m_pDocument); }
#endif

