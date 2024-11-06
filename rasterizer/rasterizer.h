
// rasterizer.h : main header file for the rasterizer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "thread_taskscheduler.h"
#include "hint.h"

// CrasterizerApp:
// See rasterizer.cpp for the implementation of this class
//

class CMainFrame;

class CrasterizerApp : public CWinApp
{
public:
	CrasterizerApp() noexcept;

	static const afthread::taskscheduler *getsched(void)
	{
		#ifdef  _DEBUG
			return nullptr;
		#endif
		return &m_Sched;
	}

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	CMainFrame *GetMainFrame( void );
	void UpdateAllViews(hint *p);

	bool getinitialised(void)const{return m_bInitialised;}

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

protected:
	static const afthread::taskscheduler m_Sched;
	bool m_bInitialised;
};

extern CrasterizerApp theApp;
