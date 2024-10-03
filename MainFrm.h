
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ChildView.h"
#include "GstPlayer.h"
#include "DeviceCapsDlg.h"

#include <vector>
#include <thread>
#include <string>


class CMainFrame : public CFrameWnd
{
	
protected: 
	DECLARE_DYNAMIC(CMainFrame)

	void EnableDisableMenuDevices();
	void EnableDisableMenuSources();
	void EnableDisableMenuSinks();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	CMainFrame() noexcept;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar    m_wndStatusBar;
	CChildView    m_wndView;
	CDeviceCapsDlg m_deviceCapsDlg;

	int			m_iSelectedCam;
	BOOL		m_bPreviewEnabled;
	BOOL		m_bShowFps;
	std::string	m_strSource;
	std::string m_strSink;
	CGstPlayer	m_gstPlayer;

	DeviceCapsListPtr m_deviceCapsList;


// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnPreview();
	afx_msg void OnCameraSelect(UINT id);
	afx_msg void OnSourceSelect(UINT id);
	afx_msg void OnSinkSelect(UINT id);
	afx_msg LRESULT OnResizeWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenGlWindowClose(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCameraOptions();
	afx_msg void OnShowFps();
	DECLARE_MESSAGE_MAP()

};

