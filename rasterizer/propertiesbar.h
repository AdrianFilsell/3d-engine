#include <afxext.h>  // For CControlBar
#include "splitterwnd.h"
#include "scenedlg.h"
#include "facesdlg.h"
#include "materialsdlg.h"
#include "lightdlg.h"
#include <memory>

class hint;

class propertiesbar : public CControlBar
{
public:
	propertiesbar();
	virtual ~propertiesbar();

	virtual CSize CalcDynamicLayout(int nLength,DWORD nMode) override;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget,BOOL bDisableIfNoHndler) override{}

	void onupdate(hint *p);
protected:
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects,NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest,CPoint point);
	afx_msg void OnPaint(void); 
	afx_msg void OnNcPaint(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT uiFlags,CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags,CPoint point);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	DECLARE_MESSAGE_MAP()

	CPoint m_cpDragBegin;
	int m_nDragBeginWidth;
	splitterwnd m_splitter;
	bool m_bCreateSplitter;
	int m_nSplitterDeflate;
	bool m_bDragging;
	std::shared_ptr<scenedlg> m_spSceneDlg;
	std::shared_ptr<lightdlg> m_spLightDlg;
	std::shared_ptr<facesdlg> m_spFacesDlg;
	std::shared_ptr<materialsdlg> m_spMaterialsDlg;

	CRect getlayoutrect(void)const;
	CRect getsplitterrect(const int nCX,const int nCY)const;
	bool hittest(const CPoint& p)const;
	void enddrag(void);
};
