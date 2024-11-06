
// titlewnd.h : interface of the splitterwnd class
//

#pragma once

class titlewnd : public CWnd
{
public:
	titlewnd()
	{
		LOGFONT lf;
		lf.lfHeight = 12;
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = false;
		lf.lfUnderline = false;
		lf.lfStrikeOut = false;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy( lf.lfFaceName, _T("MS Sans Serif") );
		m_Font.CreatePointFontIndirect(&lf);
		m_bMinimised=false;
		m_nMinimisedDim=6;
		m_nTitleGap=5;
		m_nMinimiseGap=10;
	}
	virtual ~titlewnd(){}
	bool Create(LPCTSTR lpszWindowName,CWnd* pParent,const CRect& rcClient,const int nID) { return CWnd::CreateEx(WS_EX_STATICEDGE,NULL,lpszWindowName,WS_CHILD|WS_VISIBLE,rcClient,pParent,nID); }
	void setminimised(const bool b){m_bMinimised=b;if(GetSafeHwnd())InvalidateRect(getminimisedrect());}
	bool getminimised(void)const {return m_bMinimised;}
protected:

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nHitTest,CPoint point);
	DECLARE_MESSAGE_MAP()

	CFont m_Font;
	bool m_bMinimised;
	int m_nMinimisedDim;
	int m_nTitleGap;
	int m_nMinimiseGap;
	
	CRect getminimisedrect(void)const
	{
		if(GetSafeHwnd())
		{
			CRect rcClientRect,rc(0,0,m_nMinimisedDim,m_nMinimisedDim);
			GetClientRect(rcClientRect);
			rc.OffsetRect((rcClientRect.right-m_nMinimiseGap)-rc.right,((rcClientRect.Height()-m_nMinimisedDim)/2)-rc.top);
			return rc;
		}
		return {0,0,0,0};
	}
};
