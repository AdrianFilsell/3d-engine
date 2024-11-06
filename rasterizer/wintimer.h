
#pragma once

#include <set>
#include <functional>

class wintimer : public CWnd
{
public:
	wintimer(){ CreateEx( 0, AfxRegisterWndClass( 0 ), _T( "" ), WS_POPUP, 0, 0, 0, 0, NULL, NULL ); }
	void setfn( std::function<void (wintimer *,const int)> fn ) { m_Fn = fn; }
	virtual void destroy( void ) {if(GetSafeHwnd()) DestroyWindow();}
	virtual bool settimer( const int nID, const int nMilliSecInterval ) {bool b = UINT_PTR(nID) == CWnd::SetTimer(UINT_PTR(nID),nMilliSecInterval,NULL);if(b)m_IDs.insert(nID); return b;}
	virtual bool killtimer( const int nID ) {bool b = CWnd::KillTimer(UINT_PTR(nID)); m_IDs.erase(nID); return b;}
protected:
	std::set<int> m_IDs;
	std::function<void (wintimer *,const int)> m_Fn;
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent)
	{
		if(m_IDs.find((int)nIDEvent)==m_IDs.cend())
			return;
		if( m_Fn )
			m_Fn(this, (int)nIDEvent);
	}
};
