
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "camapp-gst.h"
#include "MainFrm.h"
#include "Utils.h"

#include <cstdlib>
#include <sstream>
#include <comdef.h>
#include <dshow.h>
#include <afxtoolbarimages.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_MENU_DEVICES					1
#define ID_MENU_SOURCES					2
#define ID_MENU_SINKS					3
#define ID_MENU_OPTIONS					4
#define ID_MENU_BARCODE					5


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(MENU_VDEVICE0, MENU_VDEVICE9, &CMainFrame::OnCameraSelect)
	ON_COMMAND_RANGE(ID_SOURCE_KERNELSTREAMING, ID_SOURCE_DIRECTSHOW, &CMainFrame::OnSourceSelect)
	ON_COMMAND_RANGE(ID_SINK_AUTO, ID_SINK_OPENGL, &CMainFrame::OnSinkSelect)
	ON_COMMAND(ID_OPTIONS_PREVIEW, &CMainFrame::OnPreview)
	ON_COMMAND(ID_OPTIONS_SNAPSHOT, &CMainFrame::OnSnapshot)
	ON_COMMAND(ID_OPTIONS_REGISTRYSETTINGS, &CMainFrame::OnRegistrySettings)
	ON_COMMAND(ID_BARCODE_SCAN, &CMainFrame::OnBarcodeScan)
	ON_MESSAGE(WM_BARCODE_SCAN_REG, &CMainFrame::OnBarcodeScanReg)
	ON_COMMAND(ID_BARCODE_SHOW_LOCATION, &CMainFrame::OnBarcodeShowLocation)
	ON_MESSAGE(WM_ON_RESIZE_WINDOW, &CMainFrame::OnResizeWindow)
	ON_MESSAGE(WM_ON_OPENGL_WINDOW_CLOSE, &CMainFrame::OnOpenGlWindowClose)
	ON_MESSAGE(WM_ON_PRINT_ANALYSIS_CHANGE_OPTS, &CMainFrame::OnPrintAnalysisChangeOpts)
	ON_COMMAND(ID_OPTIONS_CAMERAPROPERTIES, &CMainFrame::OnCameraOptions)
	ON_COMMAND(ID_OPTIONS_CAMERASETTINGS, &CMainFrame::OnCameraSettings)
	ON_COMMAND(ID_OPTIONS_SHOWFPS, &CMainFrame::OnShowFps)
	ON_COMMAND(ID_OPTIONS_AUTOSTART, &CMainFrame::OnAutoStart)
	ON_COMMAND(ID_MENU_PRINT_ANALYSIS_OPTS, &CMainFrame::OnPrintAnalysisOptions)
	ON_MESSAGE(WM_ON_PRINT_ANALYSIS_NOT_FOUND, &CMainFrame::OnPrintAnalysisFilterNotFound)
	ON_MESSAGE(WM_ON_BARCODE_READER_NOT_FOUND, &CMainFrame::OnBarcodeReaderFilterNotFound)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SNAPSHOT, &CMainFrame::OnUpdateSnapshotToolbarBtn)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_CAMERAPROPERTIES, &CMainFrame::OnUpdateCameraSettingsToolbarBtn)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SHOWFPS, &CMainFrame::OnUpdateFpsToolbarBtn)
	ON_UPDATE_COMMAND_UI(ID_BARCODE_SHOW_LOCATION, &CMainFrame::OnUpdateShowLocationToolbarBtn)
	ON_UPDATE_COMMAND_UI(ID_BARCODE_TYPES, &CMainFrame::OnUpdateBarcodeFormatsToolbarBtn)
	ON_UPDATE_COMMAND_UI(ID_BARCODE_SCAN, &CMainFrame::OnUpdateBarcodeScanToolbarBtn)
	ON_COMMAND(ID_BARCODE_TYPES, &CMainFrame::OnBarcodeTypesToScan)
	ON_MESSAGE(WM_ON_BARCODE_FOUND, &CMainFrame::OnBarcodeFound)
	ON_COMMAND(ID_OCR_RUN_OCR, &CMainFrame::OnRunOCR)
	ON_WM_SIZE()
	ON_MESSAGE(WM_ON_AOI_STATS_RECEIVED, &CMainFrame::OnAoiStatsReceived)
	ON_MESSAGE(WM_ON_CHILD_LBUTTONUP, &CMainFrame::OnChildLButtonUp)
	ON_MESSAGE(WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY, &CMainFrame::OnAoiPartitionsReady)
	ON_MESSAGE(WM_TAKE_SNAPSHOT, &CMainFrame::OnRegistrySnapshot)
	ON_MESSAGE(WM_START_STOP_PREVIEW_REG, &CMainFrame::OnStartStopPreviewReg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_PANE_STATUS,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
	: m_pAmVideoProcAmp(nullptr)
	, m_pAmCameraControl(nullptr)
	, m_bBarcodeScanEnabled(FALSE)
	, m_bBarcodeShowLocation(TRUE)
	, m_bBarcodeReaderAvailable(TRUE)
	, m_bRegBarcodeScanEnabled(FALSE)
	, m_bAutoStart(FALSE)
{
	m_bAutoMenuEnable = FALSE;
	m_bPreviewEnabled = FALSE;
	m_bShowFps = TRUE;
	m_iSelectedCam = 0;
	m_pDlgCamSettingsPropSheet = NULL;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
	}

	m_nToolbarBtnImageWidth = 25;
	m_nToolbarBtnImageHeight = 25;
}

CMainFrame::~CMainFrame()
{
	SaveAppSettings();

	if (m_pDlgCamSettingsPropSheet)
		delete m_pDlgCamSettingsPropSheet;

	if (m_bPreviewEnabled)
		m_gstPlayer.StopPreview();

	CoUninitialize();
}

void CMainFrame::SaveAppSettings()
{
	DeviceCapsPtr pDeviceCaps = (*m_deviceCapsList)[m_deviceCapsDlg.GetFormatIdx()];
	ResolutionPtr pResolution = (*(pDeviceCaps->m_resolutions))[m_deviceCapsDlg.GetResolutionIdx()];
	FractionPtr pFramerate = (*(pResolution->m_framerates))[m_deviceCapsDlg.GetFramerateIdx()];
	RegAppSettings appSettings;

	appSettings.bAutoStart = m_bAutoStart;
	appSettings.strCameraName = m_gstPlayer.GetDeviceName(m_iSelectedCam);
	appSettings.strSource = m_strSource;
	appSettings.strSink = m_strSink;
	appSettings.strMediaType = pDeviceCaps->m_strMediaType;
	appSettings.strFormat = pDeviceCaps->m_strFormat;
	appSettings.iWidth = pResolution->m_iWidth;
	appSettings.iHeight = pResolution->m_iHeight;
	appSettings.iFramerateNum = pFramerate->first;
	appSettings.iFramerateDen = pFramerate->second;

	m_registryManager.SaveAppSettings(appSettings);
}

void CMainFrame::AddToolbarButton(int nCommandId, int nResourceIdDefault, int nResourceIdPressed, CString strBtnText)
{
	CMFCToolBarImages* toolbarImages = CMFCToolBar::GetImages();
	ToolbarBtnPtr pToolbarBtn = std::make_shared<ToolbarBtn>();
	HBITMAP hBmpDefault = CUtils::LoadPNGToHBITMAP(nResourceIdDefault, m_nToolbarBtnImageWidth, m_nToolbarBtnImageHeight);

	pToolbarBtn->nCmdId = nCommandId;
	pToolbarBtn->nImgIdxDefault = toolbarImages->AddImage(hBmpDefault, TRUE);

	if (nResourceIdPressed != -1)
	{
		HBITMAP hBmpPressed = CUtils::LoadPNGToHBITMAP(nResourceIdPressed, m_nToolbarBtnImageWidth, m_nToolbarBtnImageHeight);
		pToolbarBtn->nImgIdxPressed = toolbarImages->AddImage(hBmpPressed, TRUE);
	}

	m_toolbarBtnList[nCommandId] = pToolbarBtn;
	
	CMFCToolBarButton mfctbButton;
	mfctbButton.m_nID = nCommandId;
	
	mfctbButton.m_bText = TRUE; // No text for simplicity (set TRUE if you need text)
	mfctbButton.m_bTextBelow = TRUE;
	mfctbButton.m_bWrap = TRUE;
	mfctbButton.m_bWholeText = FALSE;
	CMFCToolBarButton::m_bWrapText = TRUE;
	mfctbButton.m_strText = "";
	
	mfctbButton.m_nStyle = TBSTATE_ENABLED | TBSTYLE_BUTTON;
	mfctbButton.SetImage(pToolbarBtn->nImgIdxDefault);

	m_wndToolBar.InsertButton(mfctbButton);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableToolTips(TRUE);
	m_wndToolBar.EnableTextLabels();
	
	CMFCToolBar::GetImages()->SetImageSize(CSize(m_nToolbarBtnImageWidth, m_nToolbarBtnImageHeight));
	m_wndToolBar.SetSizes(CSize(m_nToolbarBtnImageWidth + 12, m_nToolbarBtnImageHeight + 10), CSize(m_nToolbarBtnImageWidth, m_nToolbarBtnImageHeight));
	
	AddToolbarButton(ID_OPTIONS_PREVIEW, IDB_PNG_PREVIEW, IDB_PNG_PREVIEW_STOP, "Preview");
	AddToolbarButton(ID_OPTIONS_CAMERASETTINGS, IDB_PNG_VIDEO_SETTINGS, -1, "Video Settings");
	AddToolbarButton(ID_OPTIONS_CAMERAPROPERTIES, IDB_PNG_CAMERA_SETTINGS, -1, "Camera Settings");
	AddToolbarButton(ID_MENU_PRINT_ANALYSIS_OPTS, IDB_PNG_PRINT_ANALYSIS, -1, "Print Analysis");
	AddToolbarButton(ID_OPTIONS_SNAPSHOT, IDB_PNG_SNAPSHOT, -1, "Snapshot");
	AddToolbarButton(ID_OPTIONS_SHOWFPS, IDB_PNG_FPS, IDB_PNG_FPS_OFF, "Show FPS");
	AddToolbarButton(ID_BARCODE_SCAN, IDB_PNG_BARCODE, IDB_PNG_BARCODE_OFF, "Barcode Scanning");
	AddToolbarButton(ID_BARCODE_SHOW_LOCATION, IDB_PNG_LOCATION, IDB_PNG_LOCATION_OFF, "Barcode Location");
	AddToolbarButton(ID_BARCODE_TYPES, IDB_PNG_BARCODE_FORMATS, -1, "Barcode Formats");
	AddToolbarButton(ID_OCR_RUN_OCR, IDB_PNG_OCR, -1, "Run OCR");

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);
	
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Create the output pane
	if (!m_wndOutputPane.CreateEx(WS_EX_CLIENTEDGE, _T("Output"), this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_BARCODEWND,
		WS_CHILD | WS_VISIBLE | CBRS_BOTTOM | CBRS_HIDE_INPLACE | CBRS_SIZE_DYNAMIC))
	{
		TRACE("Failed to create output pane\n");
		return -1;
	}

	m_wndOutputPane.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutputPane);

	m_gstPlayer.Init(GetSafeHwnd());

	CMenu* pMenuSub = GetMenu()->GetSubMenu(ID_MENU_DEVICES);
	int iMenuItems = pMenuSub->GetMenuItemCount();

	if (iMenuItems > 0)
	{
		for (int i = 0; i < iMenuItems; i++)
			pMenuSub->RemoveMenu(0, MF_BYPOSITION);
	}

	StringList deviceNames = m_gstPlayer.GetDeviceNames();
	int iDeviceIndex = 0;
	BOOL bAutoStart = FALSE;

	RegAppSettings appSettings;
	BOOL bAppSettingsRet = m_registryManager.GetAppSettings(appSettings);

	m_bAutoStart = appSettings.bAutoStart;
	GetMenu()->GetSubMenu(ID_MENU_OPTIONS)->CheckMenuItem(ID_OPTIONS_AUTOSTART, m_bAutoStart ? MF_CHECKED : MF_UNCHECKED);

	for (std::string deviceName : deviceNames)
	{
		pMenuSub->AppendMenu(MF_STRING, MENU_VDEVICE0 + iDeviceIndex, deviceName.c_str());

		if (bAppSettingsRet && deviceName == appSettings.strCameraName)
		{
			m_iSelectedCam = iDeviceIndex;

			/*
			* We found the camera set in the registry so if auto start
			* was set in the registry we set it here to start auto preview
			*/
			bAutoStart = m_bAutoStart;
		}
		iDeviceIndex++;
	}
	
	if (deviceNames.size())
		pMenuSub->CheckMenuItem(MENU_VDEVICE0 + m_iSelectedCam, MF_CHECKED);

	// first try to use the source from app settings
	if (bAppSettingsRet && appSettings.strSource.size())
	{
		for (auto& [iSrcIdx, strSource] : g_gstSources)
		{
			if (strSource == appSettings.strSource)
			{
				m_strSource = strSource;
				GetMenu()->GetSubMenu(ID_MENU_SOURCES)->CheckMenuItem(iSrcIdx, MF_CHECKED);
				break;
			}
		}
	}
	else
	{
		// By default we use media foundation
		m_strSource = g_gstSources[ID_SOURCE_MEDIAFOUNDATION];
		GetMenu()->GetSubMenu(ID_MENU_SOURCES)->CheckMenuItem(ID_SOURCE_MEDIAFOUNDATION, MF_CHECKED);
	}

	// first try to use the sink from app settings
	if (bAppSettingsRet && appSettings.strSink.size())
	{
		for (auto& [iSinkIdx, strSink] : g_gstSinks)
		{
			if (strSink == appSettings.strSink)
			{
				m_strSink = strSink;
				GetMenu()->GetSubMenu(ID_MENU_SINKS)->CheckMenuItem(iSinkIdx, MF_CHECKED);
				break;
			}
		}
	}
	else
	{
		m_strSink = g_gstSinks[ID_SINK_AUTO];
		GetMenu()->GetSubMenu(ID_MENU_SINKS)->CheckMenuItem(ID_SINK_AUTO, MF_CHECKED);
	}

	GetMenu()->GetSubMenu(ID_MENU_OPTIONS)->EnableMenuItem(ID_OPTIONS_SNAPSHOT, MF_DISABLED);
	GetMenu()->GetSubMenu(ID_MENU_BARCODE)->CheckMenuItem(ID_BARCODE_SCAN, MF_UNCHECKED);
	GetMenu()->GetSubMenu(ID_MENU_BARCODE)->CheckMenuItem(ID_BARCODE_SHOW_LOCATION, MF_CHECKED);
	GetMenu()->GetSubMenu(ID_MENU_BARCODE)->EnableMenuItem(ID_BARCODE_SHOW_LOCATION, MF_DISABLED);
	GetMenu()->GetSubMenu(ID_MENU_BARCODE)->EnableMenuItem(ID_BARCODE_TYPES, MF_DISABLED);
	ToggleToolbarButton(ID_BARCODE_SCAN);

	m_deviceCapsList = m_gstPlayer.GetDeviceCaps(m_strSource, m_iSelectedCam);
	m_deviceCapsDlg.UpdateDeviceCaps(m_strSource, m_iSelectedCam, m_deviceCapsList);
	m_deviceCapsDlg.SelectDeviceCaps(appSettings.strFormat, appSettings.iWidth, appSettings.iHeight, appSettings.iFramerateNum, appSettings.iFramerateDen);

	m_pDlgCamSettingsPropSheet = new CPropertySheet("Camera Settings", this);

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
	CDockingManager::SetDockingMode(DT_SMART);
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	m_registryManager.StartMonitoring(this);

	if (bAutoStart)
		OnPreview();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

	cs.cx = 800;  // Width
	cs.cy = 600;  // Height

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnCameraSelect(UINT id)
{
	CMenu* pMenuSub = GetMenu()->GetSubMenu(ID_MENU_DEVICES);

	for (int i = 0; i < pMenuSub->GetMenuItemCount(); i++)
		pMenuSub->CheckMenuItem(MENU_VDEVICE0 + i, (id == MENU_VDEVICE0 + i) ? MF_CHECKED : MF_UNCHECKED);

	m_iSelectedCam = id - MENU_VDEVICE0;

	m_deviceCapsList = m_gstPlayer.GetDeviceCaps(m_strSource, m_iSelectedCam);
	m_deviceCapsDlg.UpdateDeviceCaps(m_strSource, m_iSelectedCam, m_deviceCapsList);
}

void CMainFrame::OnSourceSelect(UINT id)
{
	CMenu* pMenuSub = GetMenu()->GetSubMenu(ID_MENU_SOURCES);

	for (int i = 0; i < pMenuSub->GetMenuItemCount(); i++)
		pMenuSub->CheckMenuItem(ID_SOURCE_KERNELSTREAMING + i, (id == ID_SOURCE_KERNELSTREAMING + i) ? MF_CHECKED : MF_UNCHECKED);

	if (id >= ID_SOURCE_KERNELSTREAMING && id <= ID_SOURCE_DIRECTSHOW)
		m_strSource = g_gstSources[id];
	else
		m_strSource = g_gstSources[ID_SOURCE_MEDIAFOUNDATION];

	m_deviceCapsList = m_gstPlayer.GetDeviceCaps(m_strSource, m_iSelectedCam);
	m_deviceCapsDlg.UpdateDeviceCaps(m_strSource, m_iSelectedCam, m_deviceCapsList);
}

void CMainFrame::ToggleToolbarButton(int nBtnCommandId)
{
	ToolbarBtnPtr pToolbarBtn = m_toolbarBtnList[nBtnCommandId];
	
	int index = m_wndToolBar.CommandToIndex(nBtnCommandId);
	if (index < 0)
		return;
	
	CMFCToolBarButton* pButton = m_wndToolBar.GetButton(index);
	if (pButton == nullptr)
		return;
	
	pButton->SetImage(pToolbarBtn->bToggleFlag ? pToolbarBtn->nImgIdxDefault : pToolbarBtn->nImgIdxPressed);
	pToolbarBtn->bToggleFlag = !pToolbarBtn->bToggleFlag;
}

void CMainFrame::OnSinkSelect(UINT id)
{
	CMenu* pMenuSub = GetMenu()->GetSubMenu(ID_MENU_SINKS);

	for (int i = 0; i < pMenuSub->GetMenuItemCount(); i++)
		pMenuSub->CheckMenuItem(ID_SINK_AUTO + i, (id == ID_SINK_AUTO + i) ? MF_CHECKED : MF_UNCHECKED);

	if (id >= ID_SINK_AUTO && id <= ID_SINK_OPENGL)
		m_strSink = g_gstSinks[id];
	else
		m_strSink = g_gstSinks[ID_SINK_AUTO];
}

void CMainFrame::OnPreview()
{
	CMenu* pOptionsMenu = GetMenu()->GetSubMenu(ID_MENU_OPTIONS);
	
	if (!m_bPreviewEnabled)
	{
		std::string strError;
		int iFormatIdx = m_deviceCapsDlg.GetFormatIdx();
		int iResolutionIdx = m_deviceCapsDlg.GetResolutionIdx();
		int iFramerateIdx = m_deviceCapsDlg.GetFramerateIdx();

		DeviceCapsPtr pDeviceCaps = (*m_deviceCapsList)[iFormatIdx];
		ResolutionPtr pResolution = (*(pDeviceCaps->m_resolutions))[iResolutionIdx];
		FractionPtr pFramerate = (*(pResolution->m_framerates))[iFramerateIdx];

		m_gstPlayer.SetPrintAnalysisOpts(
			m_printAnalysisOptsDlg.GetAnalysisType(),
			m_printAnalysisOptsDlg.GetGrayscaleType(),
			m_printAnalysisOptsDlg.GetBlackoutType(),
			m_printAnalysisOptsDlg.GetConnectValues(),
			m_printAnalysisOptsDlg.GetAoiHeight(),
			m_printAnalysisOptsDlg.GetAoiPartitions()
		);

		BOOL bRet = m_gstPlayer.StartPreview(
			m_strSource, 
			m_iSelectedCam, 
			m_strSink,
			pDeviceCaps->m_strMediaType,
			pDeviceCaps->m_strFormat,
			pResolution->m_iWidth, 
			pResolution->m_iHeight, 
			pFramerate,
			m_bShowFps,
			strError, 
			m_wndView.GetSafeHwnd());

		if (!bRet)
			MessageBox(strError.c_str());
		else
		{
			m_bPreviewEnabled = TRUE;

			std::string strStatus = "Playing: (Device = " + m_gstPlayer.GetDeviceName(m_iSelectedCam) + ", Source = " + m_strSource + ", Sink = " + m_strSink +
				", Format = " + pDeviceCaps->m_strFormat + ", Resolution = " + std::to_string(pResolution->m_iWidth) + "x" + std::to_string(pResolution->m_iHeight) +
				", FPS = " + std::to_string(pFramerate->first) + "/" + std::to_string(pFramerate->second) + ")";
			m_wndStatusBar.SetPaneText(ID_PANE_STATUS, strStatus.c_str());

			ToggleToolbarButton(ID_OPTIONS_PREVIEW);
		}
	}
	else
	{
		m_gstPlayer.StopPreview();
		m_bPreviewEnabled = FALSE;
		m_wndStatusBar.SetPaneText(ID_PANE_STATUS, "Ready");
		ToggleToolbarButton(ID_OPTIONS_PREVIEW);
	}

	pOptionsMenu->CheckMenuItem(ID_OPTIONS_PREVIEW, m_bPreviewEnabled ? MF_CHECKED : MF_UNCHECKED);
	pOptionsMenu->EnableMenuItem(ID_OPTIONS_CAMERAPROPERTIES, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
	pOptionsMenu->EnableMenuItem(ID_OPTIONS_SHOWFPS, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
	pOptionsMenu->EnableMenuItem(ID_OPTIONS_SNAPSHOT, m_bPreviewEnabled ? MF_ENABLED : MF_DISABLED);
	
	EnableDisableMenuDevices();
	EnableDisableMenuSources();
	EnableDisableMenuSinks();
}

void CMainFrame::EnableDisableMenuDevices()
{
	CMenu* pDevicesMenu = GetMenu()->GetSubMenu(ID_MENU_DEVICES);

	for (int i = 0; i < pDevicesMenu->GetMenuItemCount(); i++)
		pDevicesMenu->EnableMenuItem(MENU_VDEVICE0 + i, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
}

void CMainFrame::EnableDisableMenuSources()
{
	CMenu* pSourcesMenu = GetMenu()->GetSubMenu(ID_MENU_SOURCES);

	for (int i = 0; i < pSourcesMenu->GetMenuItemCount(); i++)
		pSourcesMenu->EnableMenuItem(ID_SOURCE_KERNELSTREAMING + i, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
}

void CMainFrame::EnableDisableMenuSinks()
{
	CMenu* pSinksMenu = GetMenu()->GetSubMenu(ID_MENU_SINKS);

	for (int i = 0; i < pSinksMenu->GetMenuItemCount(); i++)
		pSinksMenu->EnableMenuItem(ID_SINK_AUTO + i, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
}

LRESULT CMainFrame::OnResizeWindow(WPARAM wParam, LPARAM lParam)
{
	int iVideoWidth = GET_X_LPARAM(lParam);
	int iVideoHeight = GET_Y_LPARAM(lParam);
	
	if (iVideoWidth && iVideoHeight)
	{
		CRect windowRect;

		// Resize the child window to match the video resolution
		m_wndView.Resize(iVideoWidth, iVideoHeight);

		CRect desiredClientRect(0, 0, iVideoWidth, iVideoHeight);

		// Adjust the window size to account for the non-client areas
		::AdjustWindowRectEx(&desiredClientRect, GetStyle(), GetMenu() != nullptr, GetExStyle());

		if (m_wndStatusBar && m_wndStatusBar.IsWindowVisible())
		{
			CRect statusBarRect;
			m_wndStatusBar.GetWindowRect(&statusBarRect);
			desiredClientRect.bottom += statusBarRect.Height();
		}

		if (m_wndToolBar && m_wndToolBar.IsWindowVisible())
		{
			CRect toolBarRect;
			m_wndToolBar.GetWindowRect(&toolBarRect);
			desiredClientRect.bottom += toolBarRect.Height();
		}

		if (m_wndOutputPane && m_wndOutputPane.IsWindowVisible())
		{
			CRect outputPaneRect;
			m_wndOutputPane.GetWindowRect(&outputPaneRect);
			desiredClientRect.bottom += outputPaneRect.Height();
		}

		// Resize the frame window so that its client area is exactly 640x360
		SetWindowPos(nullptr, 0, 0, desiredClientRect.Width(), desiredClientRect.Height(), SWP_NOMOVE | SWP_NOZORDER);

		Invalidate();
		UpdateWindow();
	}

	return 0;
}

LRESULT CMainFrame::OnOpenGlWindowClose(WPARAM wParam, LPARAM lParam)
{
	OnPreview();
	return 0;
}

LRESULT CMainFrame::OnPrintAnalysisChangeOpts(WPARAM wParam, LPARAM lParam)
{
	m_gstPlayer.SetPrintAnalysisOpts(
		m_printAnalysisOptsDlg.GetAnalysisType(),
		m_printAnalysisOptsDlg.GetGrayscaleType(),
		m_printAnalysisOptsDlg.GetBlackoutType(),
		m_printAnalysisOptsDlg.GetConnectValues(),
		m_printAnalysisOptsDlg.GetAoiHeight(),
		m_printAnalysisOptsDlg.GetAoiPartitions()
	);

	return 0;
}

void CMainFrame::OnCameraOptions()
{
	m_deviceCapsDlg.DoModal();
}

HRESULT CMainFrame::GetCameraSettingsInterfaces()
{
	CComPtr<ICreateDevEnum> pDevEnum;
	CComPtr<IEnumMoniker> pEnum;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
	if (SUCCEEDED(hr)) {
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr == S_FALSE) {
			hr = VFW_E_NOT_FOUND;  // No video capture devices found
		}
	}

	CComPtr<IMoniker> pMoniker;
	CComPtr<IBaseFilter> pCapFilter;
	
	if (SUCCEEDED(hr)) {
		while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
			CComPtr<IPropertyBag> pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
			if (FAILED(hr)) {
				continue;
			}

			VARIANT varName;
			VariantInit(&varName);
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if (SUCCEEDED(hr) && varName.vt == VT_BSTR) {
				// Convert BSTR to char*
				_bstr_t bstrName(varName.bstrVal);
				const char* deviceName = bstrName;

				if (strcmp(deviceName, m_gstPlayer.GetDeviceName(m_iSelectedCam).c_str()) == 0) {
					// Found the device
					VariantClear(&varName);
					break;
				}
			}
			VariantClear(&varName);
			pMoniker = nullptr;
		}

		if (pMoniker != NULL) {
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCapFilter);
			if (FAILED(hr)) {
				return hr;
			}
		}
		else {
			// Device not found
			hr = E_FAIL;
		}
	}

	m_pAmVideoProcAmp = nullptr;
	m_pAmCameraControl = nullptr;

	hr = pCapFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&m_pAmVideoProcAmp);
	hr = pCapFilter->QueryInterface(IID_IAMCameraControl, (void**)&m_pAmCameraControl);

	return hr;
}

void CMainFrame::OnCameraSettings()
{
	int pageCount = m_pDlgCamSettingsPropSheet->GetPageCount();
	for (int i = pageCount - 1; i >= 0; --i)
		m_pDlgCamSettingsPropSheet->RemovePage(i);
	
	m_pDlgCamSettingsPropSheet->AddPage(&m_dlgCamCtrl);
	m_pDlgCamSettingsPropSheet->AddPage(&m_dlgVidProcAmp);

	HRESULT hr = GetCameraSettingsInterfaces();
	if (SUCCEEDED(hr))
	{
		m_dlgVidProcAmp.SetVideoProcAmpInterface(m_pAmVideoProcAmp);
		m_dlgCamCtrl.SetCameraControlInterface(m_pAmCameraControl);
	}

	m_pDlgCamSettingsPropSheet->DoModal();
}

void CMainFrame::OnShowFps()
{
	m_bShowFps = !m_bShowFps;

	CMenu* pOptionsMenu = GetMenu()->GetSubMenu(ID_MENU_OPTIONS);
	pOptionsMenu->CheckMenuItem(ID_OPTIONS_SHOWFPS, m_bShowFps ? MF_CHECKED : MF_UNCHECKED);
	ToggleToolbarButton(ID_OPTIONS_SHOWFPS);
}

void CMainFrame::OnAutoStart()
{
	m_bAutoStart = !m_bAutoStart;

	CMenu* pOptionsMenu = GetMenu()->GetSubMenu(ID_MENU_OPTIONS);
	pOptionsMenu->CheckMenuItem(ID_OPTIONS_AUTOSTART, m_bAutoStart ? MF_CHECKED : MF_UNCHECKED);
}

void CMainFrame::OnPrintAnalysisOptions()
{
	m_printAnalysisOptsDlg.DoModal();
}

LRESULT CMainFrame::OnPrintAnalysisFilterNotFound(WPARAM wParam, LPARAM lParam)
{
	CMenu* pOptionsMenu = GetMenu()->GetSubMenu(ID_MENU_OPTIONS);
	pOptionsMenu->EnableMenuItem(ID_MENU_PRINT_ANALYSIS_OPTS, MF_DISABLED);

	return 0;
}

LRESULT CMainFrame::OnBarcodeReaderFilterNotFound(WPARAM wParam, LPARAM lParam)
{
	/*
	CMenu* pBarcodeMenu = GetMenu()->GetSubMenu(ID_MENU_BARCODE);
	pBarcodeMenu->EnableMenuItem(ID_BARCODE_SCAN, MF_DISABLED);
	pBarcodeMenu->EnableMenuItem(ID_BARCODE_SHOW_LOCATION, MF_DISABLED);
	pBarcodeMenu->EnableMenuItem(ID_BARCODE_TYPES, MF_DISABLED);
	*/
	m_bBarcodeReaderAvailable = FALSE;
	return 0;
}

void CMainFrame::OnSnapshot()
{
	CFileDialog dlg(FALSE, _T("jpg"), _T("snapshot"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
		_T("JPEG Files (*.jpeg;*.jpg)|*.jpeg;*.jpg|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString filePath = dlg.GetPathName();
		BYTE* pBuffer = NULL;
		int nSize, nWidth, nHeight;
		std::string format;

		if (m_gstPlayer.GetSnapshot(&pBuffer, nSize, nWidth, nHeight, format))
		{
			CUtils::SaveFrameToFile(pBuffer, nWidth, nHeight, format, filePath.GetBuffer());
		}
		else
		{
			MessageBox("Failed to get snapshot");
		}
	}
}

void CMainFrame::OnRegistrySettings()
{
	m_registryManager.DoModal();
}

void CMainFrame::OnUpdateSnapshotToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bPreviewEnabled);
}

void CMainFrame::OnUpdateCameraSettingsToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bPreviewEnabled);
}

void CMainFrame::OnUpdateFpsToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bPreviewEnabled);
}

void CMainFrame::OnBarcodeScan()
{
	m_bBarcodeScanEnabled = !m_bBarcodeScanEnabled;

	CMenu* pBarcodeMenu = GetMenu()->GetSubMenu(ID_MENU_BARCODE);
	pBarcodeMenu->CheckMenuItem(ID_BARCODE_SCAN, m_bBarcodeScanEnabled ? MF_CHECKED : MF_UNCHECKED);
	pBarcodeMenu->EnableMenuItem(ID_BARCODE_SHOW_LOCATION, m_bBarcodeScanEnabled ? MF_ENABLED : MF_DISABLED);
	ToggleToolbarButton(ID_BARCODE_SCAN);

	m_wndToolBar.Invalidate();
	m_wndToolBar.UpdateWindow();

	m_gstPlayer.EnableBarcodeScan(m_bBarcodeScanEnabled);
}

LRESULT CMainFrame::OnBarcodeScanReg(WPARAM wParam, LPARAM lParam)
{
	if (!m_bBarcodeScanEnabled && (DWORD)lParam == 0)
	{
		OnBarcodeScan();
		m_bRegBarcodeScanEnabled = TRUE;
	}

	return 0;
}

void CMainFrame::OnUpdateShowLocationToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBarcodeScanEnabled);
}

void CMainFrame::OnUpdateBarcodeFormatsToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBarcodeScanEnabled);
}

void CMainFrame::OnBarcodeShowLocation()
{
	m_bBarcodeShowLocation = !m_bBarcodeShowLocation;
	m_gstPlayer.EnableBarcodeLocation(m_bBarcodeShowLocation);
	ToggleToolbarButton(ID_BARCODE_SHOW_LOCATION);
}

void CMainFrame::OnBarcodeTypesToScan()
{
	if (m_dlgBarcodeTypes.DoModal() == IDOK)
	{
		m_gstPlayer.SetBarcodeFormats(m_dlgBarcodeTypes.GetBarcodeFormats());
		m_gstPlayer.SetBarcodeColumnStartX(m_dlgBarcodeTypes.GetBarcodeColumnStartX());
		m_gstPlayer.SetBarcodeColumnWidth(m_dlgBarcodeTypes.GetBarcodeColumnWidth());
	}
}

LRESULT CMainFrame::OnBarcodeFound(WPARAM wParam, LPARAM lParam)
{
	BarcodeList* pBarcodeList = (BarcodeList*) lParam;

	m_registryManager.WriteBarcodesToReg(pBarcodeList);

	// append barcodes in output pane
	for (BarcodeInfoPtr pBarcodeInfo : *pBarcodeList)
		m_wndOutputPane.AppendOutput(pBarcodeInfo->barcode, pBarcodeInfo->format);

	delete pBarcodeList;

	if (m_bRegBarcodeScanEnabled)
	{
		m_bRegBarcodeScanEnabled = FALSE;
		OnBarcodeScan();
	}

	return 0;
}

void CMainFrame::OnUpdateBarcodeScanToolbarBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBarcodeReaderAvailable);
}

VOID CMainFrame::OnRunOCR()
{

}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWndEx::OnSize(nType, cx, cy);

	if (nType == SIZE_MINIMIZED)
	{
		m_gstPlayer.PausePreview(TRUE);
	}
	else if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		m_gstPlayer.PausePreview(FALSE);
	}
}

LRESULT CMainFrame::OnAoiStatsReceived(WPARAM wParam, LPARAM lParam)
{
	char* pAoiStatsStr = (char*) lParam;

	m_registryManager.WritePrintPartitionsResultsToReg(pAoiStatsStr);
	
	free(pAoiStatsStr);
	return 0;
}

LRESULT CMainFrame::OnChildLButtonUp(WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	CString msg;
	msg.Format(_T("Mouse Up in ChildView at: (%d, %d)"), x, y);
	AfxMessageBox(msg);

	return 0;
}

LRESULT CMainFrame::OnAoiPartitionsReady(WPARAM wParam, LPARAM lParam)
{
	if (m_bPreviewEnabled && (DWORD)lParam == 0)
	{
		const char* pPrintPartitionsJson = m_registryManager.ReadPrintPartitionsFromReg();
		if (pPrintPartitionsJson)
			m_gstPlayer.ProcessPrintPartitions(pPrintPartitionsJson);
	}

	return 0;
}

LRESULT CMainFrame::OnRegistrySnapshot(WPARAM wParam, LPARAM lParam)
{
	if (m_bPreviewEnabled && (DWORD)lParam == 0)
	{
		BYTE* pBuffer = NULL;
		int nSize, nWidth, nHeight;
		std::string format, snapshotDir;
		//const char* pSnapshotDir = (const char*)lParam;

		if (m_registryManager.GetSnapshotDir(snapshotDir) == ERROR_SUCCESS)
		{
			if (m_gstPlayer.GetSnapshot(&pBuffer, nSize, nWidth, nHeight, format))
			{
				std::string snapshotPrefix = "snap_";
				std::string snapshotExt = "jpg";
				int iSnapshotNo = CUtils::GetNextFileNumberInSeq(snapshotDir.c_str(), snapshotPrefix.c_str(), snapshotExt.c_str());
				std::string snapshotFile = snapshotDir + "\\" + snapshotPrefix + std::to_string(iSnapshotNo) + "." + snapshotExt;

				if (CUtils::SaveFrameToFile(pBuffer, nWidth, nHeight, format, snapshotFile.c_str()))
				{
					m_registryManager.SetSnapshotFlag();
				}
			}
		}
	}

	return 0;
}

LRESULT CMainFrame::OnStartStopPreviewReg(WPARAM wParam, LPARAM lParam)
{
	BOOL bStartStopFlag = (BOOL)lParam;

	if ((m_bPreviewEnabled && bStartStopFlag == FALSE) || (!m_bPreviewEnabled && bStartStopFlag == TRUE))
	{
		OnPreview();
	}

	return 0;
}

void CMainFrame::OnClose()
{
	m_registryManager.StopMonitoring();
	CFrameWndEx::OnClose();
}
