
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ChildView.h"
#include "GstPlayer.h"
#include "DeviceCapsDlg.h"
#include "PrintAnalysisOptsDlg.h"
#include "RegistryManager.h"
#include "VideoProcAmpPage.h"
#include "CameraControlsPage.h"
#include "BarcodeTypesDlg.h"
#include "OutputWnd.h"

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

class CMainFrame : public CFrameWndEx
{
	
protected: 
	DECLARE_DYNAMIC(CMainFrame)

	void EnableDisableMenuDevices();
	void EnableDisableMenuSources();
	void EnableDisableMenuSinks();

	HRESULT GetCameraSettingsInterfaces();
	void AddToolbarButton(int nCommandId, int nResourceIdDefault, int nResourceIdPressed = -1, CString strBtnText = "");
	void ToggleToolbarButton(int nBtnCommandId);

	void EnableBarcodeScan(BOOL bEnable);

	void SaveAppSettings();

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
	CMFCStatusBar			m_wndStatusBar;
	CMFCToolBar				m_wndToolBar;
	COutputPane				m_wndOutputPane;
	int						m_nToolbarBtnImageWidth;
	int						m_nToolbarBtnImageHeight;
	CChildView				m_wndView;
	CDeviceCapsDlg			m_deviceCapsDlg;
	CPrintAnalysisOptsDlg	m_printAnalysisOptsDlg;
	CRegistryManager		m_registryManager;
	ToolbarBtnList			m_toolbarBtnList;
	BarcodeList				m_barcodeList;

	int			m_iSelectedCam;
	BOOL		m_bPreviewEnabled;
	BOOL		m_bBarcodeScanEnabled;
	BOOL		m_bRegBarcodeScanEnabled;
	BOOL		m_bBarcodeShowLocation;
	BOOL		m_bBarcodeReaderAvailable;
	BOOL		m_bShowFps;
	BOOL		m_bAutoStart;
	BOOL		m_bSnapshotAsBMP;
	std::string m_strLastSnapshotFilename;
	std::string	m_strSource;
	std::string m_strSink;
	CGstPlayer	m_gstPlayer;

	CPropertySheet* m_pDlgCamSettingsPropSheet;
	CCameraControlsPage m_dlgCamCtrl;
	CVideoProcAmpPage m_dlgVidProcAmp;
	CBarcodeTypesDlg m_dlgBarcodeTypes;

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
	afx_msg void OnAutoStart();
	afx_msg void OnSnapshotAsBMP();
	afx_msg void OnPrintAnalysisOptions();
	afx_msg LRESULT OnPrintAnalysisFilterNotFound(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBarcodeReaderFilterNotFound(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCameraSettings();
	afx_msg void OnUpdateSnapshotToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCameraSettingsToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFpsToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnBarcodeScan();
	afx_msg void OnUpdateShowLocationToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarcodeFormatsToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarcodeScanToolbarBtn(CCmdUI* pCmdUI);
	afx_msg void OnBarcodeShowLocation();
	afx_msg void OnBarcodeTypesToScan();
	afx_msg LRESULT OnBarcodeFound(WPARAM wParam, LPARAM lParam);
	afx_msg VOID OnRunOCR();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAoiStatsReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChildLButtonUp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRegistrySettings();
	afx_msg LRESULT OnAoiPartitionsReady(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRegistrySnapshot(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBarcodeScanReg(WPARAM wParam, LPARAM lParam);
	LRESULT OnStartStopPreviewReg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnCompareSnapshot();

	DECLARE_MESSAGE_MAP()
};
