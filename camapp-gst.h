
// camapp-gst.h : main header file for the camapp-gst application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CCamGstApp:
// See camapp-gst.cpp for the implementation of this class
//

class CCamGstApp : public CWinApp
{
public:
	CCamGstApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCamGstApp theApp;
