
// rasterizerDoc.h : interface of the CrasterizerDoc class
//


#pragma once

#include "3d_scene.h"
#include "3d_obj.h"
#include "surface.h"
#include "vertexfmtdlg.h"

class CrasterizerView;

class CrasterizerDoc : public CDocument
{
protected: // create from serialization only
	CrasterizerDoc() noexcept;
	DECLARE_DYNCREATE(CrasterizerDoc)

// Attributes
public:
	af3d::scene<> *getscene(void)const{return m_spScene.get();}

// Operations
public:

	CFrameWnd *createnewwindow(void);
	CFrameWnd *createnewwindow(const cameraorient& o);

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CrasterizerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void push_back(CrasterizerView *pV);
	void erase(CrasterizerView *pV);

	std::shared_ptr<af3d::vertexattsframe<>> importobj(const std::wstring& s,af3d::vertexattsframe<> *pParent,const af3d::objcfg<>& cfg);
	std::shared_ptr<af3d::vertexattsframe<>> createlight(const af3d::light<>::type t,af3d::vertexattsframe<> *pParent);
	void setname(af3d::vertexattsframe<> *p,const std::string& s);
	void setvisible(af3d::vertexattsframe<> *p,const bool b);
	void setopacity(af3d::vertexattsframe<> *p,const RAS_FLTTYPE d);
	void reparent(af3d::vertexattsframe<> *p,af3d::vertexattsframe<> *pParent,af3d::vertexattsframe<> *pTarget,const bool bAbove);
	void erase(af3d::vertexattsframe<> *p);
protected:
	std::shared_ptr<af3d::scene<>> m_spScene;
	std::vector<CrasterizerView*> m_vViews;

	void start(void);
	void stop(void);
	void tidyup(void);

// Generated message map functions
protected:
	afx_msg void OnNewCameraFront(void);
	afx_msg void OnNewCameraBack(void);
	afx_msg void OnNewCameraLeft(void);
	afx_msg void OnNewCameraRight(void);
	afx_msg void OnNewCameraAbove(void);
	afx_msg void OnNewCameraBelow(void);
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

	void setworld_aa_extents_translate(af3d::vertexattsframe<> *p,const RAS_FLTTYPE dS,const af3d::vec3<>& o) const;
	template <typename F> std::shared_ptr<af3d::vertexattsframe<>> importobj(const af3d::obj<>& o,af3d::vertexattsframe<> *pParent,const af3d::objcfg<>& cfg,const CStringA& cs);
};
