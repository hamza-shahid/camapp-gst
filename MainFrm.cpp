
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "camapp-gst.h"
#include "MainFrm.h"

#include <cstdlib>
#include <sstream>
#include <comdef.h>
#include <dshow.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_MENU_DEVICES					1
#define ID_MENU_SOURCES					2
#define ID_MENU_SINKS					3
#define ID_MENU_OPTIONS					4


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(MENU_VDEVICE0, MENU_VDEVICE9, &CMainFrame::OnCameraSelect)
	ON_COMMAND_RANGE(ID_SOURCE_KERNELSTREAMING, ID_SOURCE_DIRECTSHOW, &CMainFrame::OnSourceSelect)
	ON_COMMAND_RANGE(ID_SINK_AUTO, ID_SINK_OPENGL, &CMainFrame::OnSinkSelect)
	ON_COMMAND(ID_OPTIONS_PREVIEW, &CMainFrame::OnPreview)
	ON_MESSAGE(WM_ON_RESIZE_WINDOW, &CMainFrame::OnResizeWindow)
	ON_MESSAGE(WM_ON_OPENGL_WINDOW_CLOSE, &CMainFrame::OnOpenGlWindowClose)
	ON_MESSAGE(WM_ON_PRINT_ANALYSIS_CHANGE_OPTS, &CMainFrame::OnPrintAnalysisChangeOpts)
	ON_COMMAND(ID_OPTIONS_CAMERAPROPERTIES, &CMainFrame::OnCameraOptions)
	ON_COMMAND(ID_OPTIONS_CAMERASETTINGS, &CMainFrame::OnCameraSettings)
	ON_COMMAND(ID_OPTIONS_SHOWFPS, &CMainFrame::OnShowFps)
	ON_COMMAND(ID_MENU_PRINT_ANALYSIS_OPTS, &CMainFrame::OnPrintAnalysisOptions)
	ON_MESSAGE(WM_ON_PRINT_ANALYSIS_NOT_FOUND, &CMainFrame::OnPrintAnalysisFilterNotFound)
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
{
	m_bAutoMenuEnable = FALSE;
	m_bPreviewEnabled = FALSE;
	m_bShowFps = TRUE;
	m_iSelectedCam = 0;
	m_pDlgCamSettingsPropSheet = NULL;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
	}

}

CMainFrame::~CMainFrame()
{
	if (m_pDlgCamSettingsPropSheet)
		delete m_pDlgCamSettingsPropSheet;

	if (m_bPreviewEnabled)
		m_gstPlayer.StopPreview();

	CoUninitialize();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndView.SetGstPlayer(&m_gstPlayer);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	CMenu* pMenuSub = GetMenu()->GetSubMenu(ID_MENU_DEVICES);
	int iMenuItems = pMenuSub->GetMenuItemCount();

	if (iMenuItems > 0)
	{
		for (int i = 0; i < iMenuItems; i++)
			pMenuSub->RemoveMenu(0, MF_BYPOSITION);
	}

	m_gstPlayer.Init(GetSafeHwnd());

	StringList deviceNames = m_gstPlayer.GetDeviceNames();
	int iDeviceIndex = 0;

	for (std::string deviceName : deviceNames)
	{
		pMenuSub->AppendMenu(MF_STRING, MENU_VDEVICE0 + iDeviceIndex, deviceName.c_str());
		iDeviceIndex++;
	}
	
	if (deviceNames.size())
		pMenuSub->CheckMenuItem(MENU_VDEVICE0, MF_CHECKED);

	// By default we use media foundation
	m_strSource = g_gstSources[ID_SOURCE_MEDIAFOUNDATION];
	GetMenu()->GetSubMenu(ID_MENU_SOURCES)->CheckMenuItem(ID_SOURCE_MEDIAFOUNDATION, MF_CHECKED);

	m_strSink = g_gstSinks[ID_SINK_AUTO];
	GetMenu()->GetSubMenu(ID_MENU_SINKS)->CheckMenuItem(ID_SINK_AUTO, MF_CHECKED);

	m_deviceCapsList = m_gstPlayer.GetDeviceCaps(m_strSource, 0);
	m_deviceCapsDlg.UpdateDeviceCaps(m_strSource, 0, m_deviceCapsList);

	m_pDlgCamSettingsPropSheet = new CMFCPropertySheet("Camera Settings", this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
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
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
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
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
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
		}
	}
	else
	{
		m_gstPlayer.StopPreview();
		m_bPreviewEnabled = FALSE;
		m_wndStatusBar.SetPaneText(ID_PANE_STATUS, "Ready");
	}

	pOptionsMenu->CheckMenuItem(ID_OPTIONS_PREVIEW, m_bPreviewEnabled ? MF_CHECKED : MF_UNCHECKED);
	pOptionsMenu->EnableMenuItem(ID_OPTIONS_CAMERAPROPERTIES, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
	pOptionsMenu->EnableMenuItem(ID_OPTIONS_SHOWFPS, m_bPreviewEnabled ? MF_DISABLED : MF_ENABLED);
	
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
			// Subtract the height of the status bar from the total height
			desiredClientRect.bottom += statusBarRect.Height();
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

	hr = pCapFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&m_pAmVideoProcAmp);
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
		m_dlgVidProcAmp.SetVideoProcAmpInterface(m_pAmVideoProcAmp);

	m_pDlgCamSettingsPropSheet->DoModal();
}

void CMainFrame::OnShowFps()
{
	m_bShowFps = !m_bShowFps;

	CMenu* pOptionsMenu = GetMenu()->GetSubMenu(ID_MENU_OPTIONS);
	pOptionsMenu->CheckMenuItem(ID_OPTIONS_SHOWFPS, m_bShowFps ? MF_CHECKED : MF_UNCHECKED);
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
