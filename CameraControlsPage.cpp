// CameraControlsPage.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "CameraControlsPage.h"
#include "afxdialogex.h"

#include <ks.h>
#include <ksmedia.h>

// CCameraControlsPage dialog

IMPLEMENT_DYNAMIC(CCameraControlsPage, CMFCPropertyPage)


BEGIN_MESSAGE_MAP(CCameraControlsPage, CMFCPropertyPage)
	ON_COMMAND_RANGE(IDC_CHECK_ZOOM, IDC_CHECK_ROLL, &CCameraControlsPage::OnAutoCheckbox)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_ZOOM, IDC_EDIT_ROLL, &CCameraControlsPage::OnEditChange)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_CAM_CTRL_DEFAULT, &CCameraControlsPage::OnBnClickedDefault)
	ON_BN_CLICKED(IDC_CHECK_LLC, &CCameraControlsPage::OnBnClickedCheckLowLightCompensation)
END_MESSAGE_MAP()


CCameraControlsPage::CCameraControlsPage()
	: CMFCPropertyPage(IDD_PROPPAGE_CAMERA_CONTROLS)
	, m_pAmCameraControl(nullptr)
{
	m_pSliderProperties = new CameraSliderProperties(this);

	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Zoom, IDC_SLIDER_ZOOM, IDC_EDIT_ZOOM, IDC_CHECK_ZOOM);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Focus, IDC_SLIDER_FOCUS, IDC_EDIT_FOCUS, IDC_CHECK_FOCUS);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Exposure, IDC_SLIDER_EXPOSURE, IDC_EDIT_EXPOSURE, IDC_CHECK_EXPOSURE);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Iris, IDC_SLIDER_APERTURE, IDC_EDIT_APERTURE, IDC_CHECK_APERTURE);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Pan, IDC_SLIDER_PAN, IDC_EDIT_PAN, IDC_CHECK_PAN);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Tilt, IDC_SLIDER_TILT, IDC_EDIT_TILT, IDC_CHECK_TILT);
	m_pSliderProperties->AddCameraSliderProperty(CameraControl_Roll, IDC_SLIDER_ROLL, IDC_EDIT_ROLL, IDC_CHECK_ROLL);
}

CCameraControlsPage::~CCameraControlsPage()
{
	delete m_pSliderProperties;
}

#define ADD_DDX_FOR_PROPERTY(ctrlIdPostfix)																				\
DDX_Control(pDX, IDC_SLIDER_ ## ctrlIdPostfix, m_pSliderProperties->GetSliderCtrl(IDC_SLIDER_ ## ctrlIdPostfix));		\
DDX_Text(pDX, IDC_EDIT_ ## ctrlIdPostfix, m_pSliderProperties->GetEditCtrlValue(IDC_EDIT_ ## ctrlIdPostfix));			\
DDX_Check(pDX, IDC_CHECK_ ## ctrlIdPostfix, m_pSliderProperties->GetCheckboxCtrlValue(IDC_CHECK_ ## ctrlIdPostfix))

void CCameraControlsPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);

	ADD_DDX_FOR_PROPERTY(ZOOM);
	ADD_DDX_FOR_PROPERTY(FOCUS);
	ADD_DDX_FOR_PROPERTY(EXPOSURE);
	ADD_DDX_FOR_PROPERTY(APERTURE);
	ADD_DDX_FOR_PROPERTY(PAN);
	ADD_DDX_FOR_PROPERTY(TILT);
	ADD_DDX_FOR_PROPERTY(ROLL);

	DDX_Check(pDX, IDC_CHECK_LLC, m_bLowLightCompensation);
}

BOOL CCameraControlsPage::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	if (m_pAmCameraControl)
	{
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Zoom);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Focus);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Exposure);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Iris);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Pan);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Tilt);
		m_pSliderProperties->SetupCameraSliderProperty(CameraControl_Roll);
	}

	long lFlags;
	HRESULT hr = m_pAmCameraControl->Get(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, (long*)&m_bLowLightCompensation, &lFlags);
	if (!SUCCEEDED(hr))
	{
		GetDlgItem(IDC_CHECK_LLC)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
	return TRUE;
}

BOOL CCameraControlsPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CWnd* pFocusWnd = GetFocus();
		if (pFocusWnd != nullptr)
		{
			int ctrlID = pFocusWnd->GetDlgCtrlID();
			if (ctrlID >= IDC_EDIT_ZOOM && ctrlID <= IDC_EDIT_ROLL)
			{
				OnEditEnter(ctrlID);
				return TRUE;
			}
		}
	}
	return CMFCPropertyPage::PreTranslateMessage(pMsg);
}

void CCameraControlsPage::SetCameraControlInterface(CComPtr<IAMCameraControl> pAmCameraControl)
{
	m_pAmCameraControl = pAmCameraControl;
	m_pSliderProperties->SetAmCameraControlInterface(pAmCameraControl);
}

void CCameraControlsPage::SetExposure(BOOL bAuto, long lValue)
{
	if (m_pAmCameraControl)
	{
		HRESULT hr;

		if (bAuto)
			hr = m_pAmCameraControl->Set(CameraControl_Exposure, lValue, CameraControl_Flags_Auto);
		else
			hr = m_pAmCameraControl->Set(CameraControl_Exposure, lValue, CameraControl_Flags_Manual);
	}
}

long CCameraControlsPage::GetExposure()
{
	long lExposure = 0;

	if (m_pAmCameraControl)
	{
		long lFlags = -1;
		HRESULT hr = m_pAmCameraControl->Get(CameraControl_Exposure, &lExposure, &lFlags);
	}

	return lExposure;
}

// CCameraControlsPage message handlers

void CCameraControlsPage::OnEditEnter(UINT nEditCtrlId)
{
	UpdateData(TRUE);
	m_pSliderProperties->OnEditEnter(nEditCtrlId);
	UpdateData(FALSE);
}

void CCameraControlsPage::OnEditChange(UINT nID)
{
	m_pSliderProperties->OnEditChange(nID);
}

void CCameraControlsPage::OnBnClickedDefault()
{
	for (long i = IDC_SLIDER_ZOOM; i <= IDC_SLIDER_ROLL; i++)
		m_pSliderProperties->SetDefault(i);

	UpdateData(FALSE);
}

void CCameraControlsPage::OnAutoCheckbox(UINT id)
{
	UpdateData(TRUE);
	m_pSliderProperties->OnAutoCheckbox(id);
}

void CCameraControlsPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_pSliderProperties->OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateData(FALSE);

	CMFCPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCameraControlsPage::OnBnClickedCheckLowLightCompensation()
{
	UpdateData(TRUE);
	HRESULT hr = m_pAmCameraControl->Set(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, m_bLowLightCompensation, 0);
}
