
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ChildView.h"
#include "GstPlayer.h"
#include "DeviceCapsDlg.h"
#include "PrintAnalysisOptsDlg.h"
#include "VideoProcAmpPage.h"
#include "CameraControlsPage.h"

#include <vector>
#include <thread>
#include <string>
#include <unordered_map>


struct ToolbarBtn
{
	int nCmdId;
	int nImgIdxDefault = -1;
	int nImgIdxPressed = -1;
	BOOL bToggleFlag = FALSE;
};

typedef std::shared_ptr< ToolbarBtn > ToolbarBtnPtr;
typedef std::unordered_map<int, ToolbarBtnPtr> ToolbarBtnList;

class CMainFrame : public CFrameWnd
{
	
protected: 
	DECLARE_DYNAMIC(CMainFrame)

	void EnableDisableMenuDevices();
	void EnableDisableMenuSources();
	void EnableDisableMenuSinks();

	HRESULT GetCameraSettingsInterfaces();
	void AddToolbarButton(int nCommandId, int nResourceIdDefault, int nResourceIdPressed = -1);
	void ToggleToolbarButton(int nBtnCommandId);

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
	CStatusBar				m_wndStatusBar;
	CDialogBar				m_wndDialogBar;
	CToolBar				m_wndToolBar;
	CToolTipCtrl			m_tooltips;
	CImageList				m_imgList;
	int						m_nToolbarBtnImageWidth;
	int						m_nToolbarBtnImageHeight;
	CChildView				m_wndView;
	CDeviceCapsDlg			m_deviceCapsDlg;
	CPrintAnalysisOptsDlg	m_printAnalysisOptsDlg;
	ToolbarBtnList			m_toolbarBtnList;

	int			m_iSelectedCam;
	BOOL		m_bPreviewEnabled;
	BOOL		m_bShowFps;
	std::string	m_strSource;
	std::string m_strSink;
	CGstPlayer	m_gstPlayer;

	CPropertySheet* m_pDlgCamSettingsPropSheet;
	CCameraControlsPage m_dlgCamCtrl;
	CVideoProcAmpPage m_dlgVidProcAmp;

	DeviceCapsListPtr m_deviceCapsList;

	CComPtr<IAMVideoProcAmp> m_pAmVideoProcAmp;
	CComPtr<IAMCameraControl> m_pAmCameraControl;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnPreview();
	afx_msg void OnSnapshot();
	afx_msg void OnCameraSelect(UINT id);
	afx_msg void OnSourceSelect(UINT id);
	afx_msg void OnSinkSelect(UINT id);
	afx_msg LRESULT OnResizeWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenGlWindowClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintAnalysisChangeOpts(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCameraOptions();
	afx_msg void OnShowFps();
	afx_msg void OnPrintAnalysisOptions();
	afx_msg LRESULT OnPrintAnalysisFilterNotFound(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCameraSettings();
	afx_msg void OnUpdateSnapshotToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCameraSettingsToolbarBtn(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};
