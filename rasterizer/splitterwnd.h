
// splitterwnd.h : interface of the splitterwnd class
//

#pragma once

#include <vector>
#include <memory>

class splitterwndpane
{
public:
	splitterwndpane(){m_nMaximise=0;m_rc.SetRectEmpty();}
	~splitterwndpane(){}
	virtual CWnd *getwnd(void)=0;
	virtual UINT getid(void)const=0;
	virtual int getmax(void)const=0;
	virtual int getmin(void)const=0;
	virtual bool getminimised(void)const=0;
	int getmaximise(void)const{return m_nMaximise;}
	const CRect& getrect(void)const{return m_rc;}
	CRect getbarrect(const bool bIncludeGap)const{CRect r=m_rc;r.top=r.bottom;r.bottom=r.top+m_nBarHeight;if(bIncludeGap)return r;r.DeflateRect(0,m_nBarGap,0,m_nBarGap);return r;}

	void setrect(const CRect& r){m_rc=r;}
	void setmaximise(const int n){m_nMaximise=n;}
	virtual void setminimised(const bool b)=0;

	splitterwndpane& operator=(const splitterwndpane& o){m_nMaximise=o.m_nMaximise;m_rc=o.m_rc;return *this;}
	static const int m_nBarGripper=5;
	static const int m_nBarGap=4;
	static const int m_nBarHeight=m_nBarGripper+(2*m_nBarGap);
protected:
	CRect m_rc;
	int m_nMaximise;
};

class splitterwnd : public CWnd
{
public:
	splitterwnd();
	virtual ~splitterwnd(){}

	splitterwndpane *getpane(const int nID);
	bool getindex(const splitterwndpane *p,int& n)const;

	void pushback(std::shared_ptr<splitterwndpane> sp){m_vPanes.push_back(sp);}

	void initlayout(void);
	void setminimise(splitterwndpane *p,const bool b);

	static void repos(const std::vector<std::pair<CWnd*,CRect>>& vRepos);
	static void excludecliprect(CDC *pDC,const std::vector<CWnd*>& vErase);
protected:
	afx_msg void OnSize(UINT uiFlags, int nCX, int nCY); 
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT, CPoint);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg LPARAM OnTitleWndMinimise(WPARAM,LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	int m_nHT;
	int m_nDraggingHT;
	CPoint m_cpDragBegin;
	CRect m_rcDragBeginClientRect;
	DWORD m_dwInSizing;
	bool m_bInitialisingLayout;
	std::vector<std::pair<int,int>> m_vDragBeginRepos;
	std::vector<std::shared_ptr<splitterwndpane>> m_vPanes;
	
	int hittest(const CPoint& p) const;
	
	void recalclayout(const bool bMainFrm);
	void recalclayout(const int nPane,const std::vector<std::pair<int,int>>& vDragBeginRepos,const int nDY,const int nBeginClientRect);
	void reposlayout(const std::vector<std::pair<int,int>>& p);
	int gettopheight(std::vector<std::pair<int,int>>& v)const;
	void settop(std::vector<std::pair<int,int>>& vRepos)const;

	void getdragbeginrepos(void);
	void enddrag(void);

	void invalidatebar(const int n);
	void drawborder(CDC* const pDC,const CRect& rc,const bool bUp) const;
};
