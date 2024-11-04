// VideoProcAmpPage.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "VideoProcAmpPage.h"
#include "afxdialogex.h"

#include <ks.h>
#include <ksmedia.h>
#include <string>


#define POWERLINE_FREQ_50_HZ 1
#define POWERLINE_FREQ_60_HZ 2


// CVideoProcAmpPage dialog

IMPLEMENT_DYNAMIC(CVideoProcAmpPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CVideoProcAmpPage, CMFCPropertyPage)
	ON_COMMAND_RANGE(IDC_CHECK_BRIGHTNESS, IDC_CHECK_GAIN, &CVideoProcAmpPage::OnAutoCheckbox)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_BRIGHTNESS, IDC_EDIT_GAIN, &CVideoProcAmpPage::OnEditChange)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_VID_PROC_AMP_DEFAULT, &CVideoProcAmpPage::OnBnClickedDefault)
	ON_BN_CLICKED(IDC_CHECK_COLOR_ENB, &CVideoProcAmpPage::OnBnClickedCheckColorEnb)
	ON_CBN_SELCHANGE(IDC_COMBO_PL_FREQ, &CVideoProcAmpPage::OnChangePowerLineFreq)
END_MESSAGE_MAP()


CVideoProcAmpPage::CVideoProcAmpPage()
	: CMFCPropertyPage(IDD_PROPPAGE_VIDEO_PROC)
	, m_pAmVideoProcAmp(nullptr)
{
	m_pSliderProperties = new CameraSliderProperties(this);

	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Brightness, IDC_SLIDER_BRIGHTNESS, IDC_EDIT_BRIGHTNESS, IDC_CHECK_BRIGHTNESS);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Contrast, IDC_SLIDER_CONTRAST, IDC_EDIT_CONTRAST, IDC_CHECK_CONTRAST);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Hue, IDC_SLIDER_HUE, IDC_EDIT_HUE, IDC_CHECK_HUE);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Saturation, IDC_SLIDER_SATURATION, IDC_EDIT_SATURATION, IDC_CHECK_SATURATION);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Sharpness, IDC_SLIDER_SHARPNESS, IDC_EDIT_SHARPNESS, IDC_CHECK_SHARPNESS);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Gamma, IDC_SLIDER_GAMMA, IDC_EDIT_GAMMA, IDC_CHECK_GAMMA);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_WhiteBalance, IDC_SLIDER_WHITE_BAL, IDC_EDIT_WHITE_BAL, IDC_CHECK_WHITE_BAL);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_BacklightCompensation, IDC_SLIDER_BACKLIGHT_COMP, IDC_EDIT_BACKLIGHT_COMP, IDC_CHECK_BACKLIGHT_COMP);
	m_pSliderProperties->AddCameraSliderProperty(VideoProcAmp_Gain, IDC_SLIDER_GAIN, IDC_EDIT_GAIN, IDC_CHECK_GAIN);
}

CVideoProcAmpPage::~CVideoProcAmpPage()
{
	delete m_pSliderProperties;
}

#define ADD_DDX_FOR_PROPERTY(ctrlIdPostfix)																				\
DDX_Control(pDX, IDC_SLIDER_ ## ctrlIdPostfix, m_pSliderProperties->GetSliderCtrl(IDC_SLIDER_ ## ctrlIdPostfix));		\
DDX_Text(pDX, IDC_EDIT_ ## ctrlIdPostfix, m_pSliderProperties->GetEditCtrlValue(IDC_EDIT_ ## ctrlIdPostfix));			\
DDX_Check(pDX, IDC_CHECK_ ## ctrlIdPostfix, m_pSliderProperties->GetCheckboxCtrlValue(IDC_CHECK_ ## ctrlIdPostfix))

void CVideoProcAmpPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
	
	ADD_DDX_FOR_PROPERTY(BRIGHTNESS);
	ADD_DDX_FOR_PROPERTY(CONTRAST);
	ADD_DDX_FOR_PROPERTY(HUE);
	ADD_DDX_FOR_PROPERTY(SATURATION);
	ADD_DDX_FOR_PROPERTY(SHARPNESS);
	ADD_DDX_FOR_PROPERTY(GAMMA);
	ADD_DDX_FOR_PROPERTY(WHITE_BAL);
	ADD_DDX_FOR_PROPERTY(BACKLIGHT_COMP);
	ADD_DDX_FOR_PROPERTY(GAIN);

	DDX_Check(pDX, IDC_CHECK_COLOR_ENB, m_bColorEnable);
	DDX_Control(pDX, IDC_COMBO_PL_FREQ, m_comboPowerlineFreq);
}

BOOL CVideoProcAmpPage::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	if (m_pAmVideoProcAmp)
	{
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Brightness);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Contrast);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Hue);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Saturation);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Sharpness);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Gamma);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_WhiteBalance);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_BacklightCompensation);
		m_pSliderProperties->SetupCameraSliderProperty(VideoProcAmp_Gain);

		long lFlags;
		HRESULT hr = m_pAmVideoProcAmp->Get(VideoProcAmp_ColorEnable, (long*)&m_bColorEnable, &lFlags);
		if (!SUCCEEDED(hr))
		{
			GetDlgItem(IDC_CHECK_COLOR_ENB)->EnableWindow(FALSE);
		}

		//long lMin, lMax, lSteppingDelta, lDefault, lCapsFlags, lValue;
		hr = m_pAmVideoProcAmp->GetRange(
			KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, 
			&m_propPowerlineFreq.lMin, 
			&m_propPowerlineFreq.lMax, 
			&m_propPowerlineFreq.lSteppingDelta, 
			&m_propPowerlineFreq.lDefault, 
			&m_propPowerlineFreq.lCapsFlags
		);
		
		if (!SUCCEEDED(hr))
		{
			GetDlgItem(IDC_COMBO_PL_FREQ)->EnableWindow(FALSE);
		}
		else
		{
			hr = m_pAmVideoProcAmp->Get(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &m_propPowerlineFreq.lPropertyValue, &lFlags);
			if (!SUCCEEDED(hr) || m_propPowerlineFreq.lPropertyValue == 0)
			{
				GetDlgItem(IDC_COMBO_PL_FREQ)->EnableWindow(FALSE);
			}
			else
			{
				if (POWERLINE_FREQ_50_HZ >= m_propPowerlineFreq.lMin && POWERLINE_FREQ_50_HZ <= m_propPowerlineFreq.lMax)
				{
					m_comboPowerlineFreq.AddString("50 Hz");
					m_comboPowerlineFreq.SetItemData(0, POWERLINE_FREQ_50_HZ);

					if (m_propPowerlineFreq.lPropertyValue == POWERLINE_FREQ_50_HZ)
						m_comboPowerlineFreq.SetCurSel(0);
				}

				if (POWERLINE_FREQ_60_HZ >= m_propPowerlineFreq.lMin && POWERLINE_FREQ_60_HZ <= m_propPowerlineFreq.lMax)
				{
					m_comboPowerlineFreq.AddString("60 Hz");
					m_comboPowerlineFreq.SetItemData(1, POWERLINE_FREQ_60_HZ);

					if (m_propPowerlineFreq.lPropertyValue == POWERLINE_FREQ_60_HZ)
						m_comboPowerlineFreq.SetCurSel(1);
				}
			}
		}
	}

	UpdateData(FALSE);
	return TRUE;
}

BOOL CVideoProcAmpPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CWnd* pFocusWnd = GetFocus();
		if (pFocusWnd != nullptr)
		{
			int ctrlID = pFocusWnd->GetDlgCtrlID();
			if (ctrlID >= IDC_EDIT_BRIGHTNESS && ctrlID <= IDC_EDIT_GAIN)
			{
				OnEditEnter(ctrlID);
				return TRUE;
			}
		}
	}
	return CMFCPropertyPage::PreTranslateMessage(pMsg);
}

void CVideoProcAmpPage::SetVideoProcAmpInterface(CComPtr<IAMVideoProcAmp> pAmVideoProcAmp)
{
	m_pAmVideoProcAmp = pAmVideoProcAmp;
	m_pSliderProperties->SetAmVideoProcInterface(pAmVideoProcAmp);
}

// CVideoProcAmpPage message handlers

void CVideoProcAmpPage::OnEditEnter(UINT nEditCtrlId)
{
	UpdateData(TRUE);
	m_pSliderProperties->OnEditEnter(nEditCtrlId);
	UpdateData(FALSE);
}

void CVideoProcAmpPage::OnEditChange(UINT nID)
{
	m_pSliderProperties->OnEditChange(nID);
}

void CVideoProcAmpPage::OnBnClickedDefault()
{
	for (long i = IDC_SLIDER_BRIGHTNESS; i <= IDC_SLIDER_GAIN; i++)
		m_pSliderProperties->SetDefault(i);

	HRESULT hr = m_pAmVideoProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, m_propPowerlineFreq.lDefault, 0);

	for (long i = 0; i < m_comboPowerlineFreq.GetCount(); i++)
	{
		if (m_comboPowerlineFreq.GetItemData(i) == m_propPowerlineFreq.lDefault)
			m_comboPowerlineFreq.SetCurSel(i);
	}

	UpdateData(FALSE);
}

void CVideoProcAmpPage::OnAutoCheckbox(UINT id)
{
	UpdateData(TRUE);
	m_pSliderProperties->OnAutoCheckbox(id);
}

void CVideoProcAmpPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_pSliderProperties->OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateData(FALSE);
	
	CMFCPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVideoProcAmpPage::OnBnClickedCheckColorEnb()
{
	UpdateData(TRUE);
	HRESULT hr = m_pAmVideoProcAmp->Set(VideoProcAmp_ColorEnable, m_bColorEnable, 0);
}

void CVideoProcAmpPage::OnChangePowerLineFreq()
{
	long lSlectedPlFreq = m_comboPowerlineFreq.GetItemData(m_comboPowerlineFreq.GetCurSel());
	HRESULT hr = m_pAmVideoProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, lSlectedPlFreq, 0);
}
