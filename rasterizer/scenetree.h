#pragma once

#include <memory>
#include "3d_frame.h"

class scenetree : public CTreeCtrl
{
DECLARE_DYNAMIC(scenetree)

public:
	scenetree();
	virtual ~scenetree(){}

	struct reparent
	{
		enum type {t_null,t_first_child,t_sibling_above,t_sibling_below};
		HTREEITEM hFrom;
		HTREEITEM hTo;
		type t;
		bool isvalid(scenetree *pTree)const;
		bool operator ==(const reparent& o)const{return hFrom==o.hFrom && hTo==o.hTo && t==o.t;}
	};

	HTREEITEM append(HTREEITEM hParent,af3d::vertexattsframe<> *p)
	{
		HTREEITEM h=InsertItem(_T(" ")+CString(p->getname().c_str()),hParent,TVI_LAST);
		SetItemData(h,reinterpret_cast<DWORD_PTR>(p));
		SetCheck(h,p->getvisible());
		return h;
	}
	HTREEITEM insert(HTREEITEM hParent,af3d::vertexattsframe<> *p,const int n)
	{
		HTREEITEM hAfter=n==0?TVI_FIRST:getitem(hParent,n-1);
		HTREEITEM h=InsertItem(_T(" ")+CString(p->getname().c_str()),hParent,hAfter);
		SetItemData(h,reinterpret_cast<DWORD_PTR>(p));
		SetCheck(h,p->getvisible());
		return h;
	}
protected:
	afx_msg void OnPaint(void);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	bool m_bInDrag;
	bool m_bSeenLButtonDown;
	CPoint m_ptLButtonDown;
	int m_nDragMoveThreshold;
	CPoint m_cpDragILOffset;
	std::shared_ptr<CImageList> m_spDragIL;
	HTREEITEM m_hDragCandidate;
	HTREEITEM m_hDragHover;
	reparent::type m_DragReparentType;
	UINT_PTR m_uiDragHoverID;
	int m_hDragHoverInterval;
	
	void begindrag(const CPoint& ptStart,const CPoint& pt);
	void move(const CPoint& pt);
	void movedrag(const CPoint& pt);
	void enddrag(void);
	bool canbegindrag(HTREEITEM h)const;
	void movedragautoexpand(HTREEITEM h,const UINT uFlags);
	void setmovedragcursor(void);

	bool isdescendant(HTREEITEM hA,HTREEITEM hB)const;
	HTREEITEM getitem(HTREEITEM hParent,const int n)
	{
		HTREEITEM h=GetChildItem(hParent);
		for(int nChild=0;nChild<n;++nChild)
			h=GetNextSiblingItem(h);
		return h;
	}
	bool getindex(HTREEITEM h,int& nChild)
	{
		HTREEITEM hParent=GetParentItem(h);
		HTREEITEM hChild=GetChildItem(hParent);
		for(nChild=0;hChild;++nChild)
			if(h==hChild)
				return true;
			else
				hChild=GetNextSiblingItem(hChild);
		return false;
	}
	bool ischild(HTREEITEM h,const CPoint& pt)const;
	bool isbelow(HTREEITEM h,const CPoint& pt)const;
	reparent getreparent(void)const{return {m_hDragCandidate,m_hDragHover,m_DragReparentType};}
};
