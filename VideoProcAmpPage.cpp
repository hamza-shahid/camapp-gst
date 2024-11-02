// VideoProcAmpPage.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "VideoProcAmpPage.h"
#include "afxdialogex.h"

#include <ks.h>
#include <ksmedia.h>
#include <string>
#include <regex>


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

}

CVideoProcAmpPage::~CVideoProcAmpPage()
{
}

#define ADD_DDX_FOR_PROPERTY(ctrlIdPostfix, name)								\
DDX_Control(pDX, IDC_SLIDER_ ## ctrlIdPostfix, m_prop ## name ##.slider);		\
DDX_Text(pDX, IDC_EDIT_ ## ctrlIdPostfix, m_prop ## name ##.lPropertyValue);	\
DDX_Check(pDX, IDC_CHECK_ ## ctrlIdPostfix, m_prop ## name ##.bAutoFlag)

void CVideoProcAmpPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
	
	ADD_DDX_FOR_PROPERTY(BRIGHTNESS, Brightness);
	ADD_DDX_FOR_PROPERTY(CONTRAST, Contrast);
	ADD_DDX_FOR_PROPERTY(HUE, Hue);
	ADD_DDX_FOR_PROPERTY(SATURATION, Saturation);
	ADD_DDX_FOR_PROPERTY(SHARPNESS, Sharpness);
	ADD_DDX_FOR_PROPERTY(GAMMA, Gamma);
	ADD_DDX_FOR_PROPERTY(WHITE_BAL, WhiteBalance);
	ADD_DDX_FOR_PROPERTY(BACKLIGHT_COMP, BacklightCompensation);
	ADD_DDX_FOR_PROPERTY(GAIN, Gain);

	DDX_Check(pDX, IDC_CHECK_COLOR_ENB, m_bColorEnable);
	DDX_Control(pDX, IDC_COMBO_PL_FREQ, m_comboPowerlineFreq);
}

void CVideoProcAmpPage::SetupVideoProcAmpProperty(long lProperty, VideoProcAmpPropertyGroup& propGroup, int nSliderCtrlId, int nEditCtrlId, int nCheckboxId)
{
	HRESULT hr = m_pAmVideoProcAmp->GetRange(
		lProperty, 
		&propGroup.lMin, 
		&propGroup.lMax, 
		&propGroup.lSteppingDelta, 
		&propGroup.lDefault, 
		&propGroup.lCapsFlags
	);
	
	propGroup.lProperty = lProperty;
	propGroup.nSliderCtrlId = nSliderCtrlId;
	propGroup.nEditCtrlId = nEditCtrlId;
	propGroup.nCheckboxId = nCheckboxId;

	if (FAILED(hr))
	{
		propGroup.slider.SetRange(0, 10);
		propGroup.slider.SetPos(0);
		propGroup.lPropertyValue = 0;
		propGroup.lDefault = 0;
		GetDlgItem(nSliderCtrlId)->EnableWindow(FALSE);
		GetDlgItem(nEditCtrlId)->EnableWindow(FALSE);
		GetDlgItem(nEditCtrlId)->SetWindowTextA("");
		GetDlgItem(nCheckboxId)->EnableWindow(FALSE);
	}
	else
	{
		m_pAmVideoProcAmp->Get(lProperty, &propGroup.lPropertyValue, &propGroup.lFlags);
		propGroup.slider.SetRange(propGroup.lMin, propGroup.lMax);
		propGroup.slider.SetPos(propGroup.lPropertyValue);
		propGroup.slider.SetLineSize(propGroup.lSteppingDelta);
		propGroup.slider.SetPageSize(propGroup.lSteppingDelta);

		if (propGroup.lCapsFlags == VideoProcAmp_Flags_Auto)
		{
			propGroup.bAutoFlag = TRUE;
			GetDlgItem(nCheckboxId)->EnableWindow(FALSE);
		}
		else if (propGroup.lCapsFlags == VideoProcAmp_Flags_Manual)
		{
			propGroup.bAutoFlag = (propGroup.lFlags == VideoProcAmp_Flags_Auto ? TRUE : FALSE);
			GetDlgItem(nCheckboxId)->EnableWindow(FALSE);
		}
		else if (propGroup.lCapsFlags == (VideoProcAmp_Flags_Auto | VideoProcAmp_Flags_Manual))
		{
			propGroup.bAutoFlag = (propGroup.lFlags == VideoProcAmp_Flags_Auto ? TRUE : FALSE);
			GetDlgItem(nCheckboxId)->EnableWindow(TRUE);
		}

		GetDlgItem(nSliderCtrlId)->EnableWindow(!propGroup.bAutoFlag);
		GetDlgItem(nEditCtrlId)->EnableWindow(!propGroup.bAutoFlag);
	}

	m_mapProperties[nSliderCtrlId] = &propGroup;
	m_mapProperties[nEditCtrlId] = &propGroup;
	m_mapProperties[nCheckboxId] = &propGroup;
}

BOOL CVideoProcAmpPage::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	m_mapProperties.clear();

	if (m_pAmVideoProcAmp)
	{
		SetupVideoProcAmpProperty(VideoProcAmp_Brightness, m_propBrightness, IDC_SLIDER_BRIGHTNESS, IDC_EDIT_BRIGHTNESS, IDC_CHECK_BRIGHTNESS);
		SetupVideoProcAmpProperty(VideoProcAmp_Contrast, m_propContrast, IDC_SLIDER_CONTRAST, IDC_EDIT_CONTRAST, IDC_CHECK_CONTRAST);
		SetupVideoProcAmpProperty(VideoProcAmp_Hue, m_propHue, IDC_SLIDER_HUE, IDC_EDIT_HUE, IDC_CHECK_HUE);
		SetupVideoProcAmpProperty(VideoProcAmp_Saturation, m_propSaturation, IDC_SLIDER_SATURATION, IDC_EDIT_SATURATION, IDC_CHECK_SATURATION);
		SetupVideoProcAmpProperty(VideoProcAmp_Sharpness, m_propSharpness, IDC_SLIDER_SHARPNESS, IDC_EDIT_SHARPNESS, IDC_CHECK_SHARPNESS);
		SetupVideoProcAmpProperty(VideoProcAmp_Gamma, m_propGamma, IDC_SLIDER_GAMMA, IDC_EDIT_GAMMA, IDC_CHECK_GAMMA);
		SetupVideoProcAmpProperty(VideoProcAmp_WhiteBalance, m_propWhiteBalance, IDC_SLIDER_WHITE_BAL, IDC_EDIT_WHITE_BAL, IDC_CHECK_WHITE_BAL);
		SetupVideoProcAmpProperty(VideoProcAmp_BacklightCompensation, m_propBacklightCompensation, IDC_SLIDER_BACKLIGHT_COMP, IDC_EDIT_BACKLIGHT_COMP, IDC_CHECK_BACKLIGHT_COMP);
		SetupVideoProcAmpProperty(VideoProcAmp_Gain, m_propGain, IDC_SLIDER_GAIN, IDC_EDIT_GAIN, IDC_CHECK_GAIN);

		long lFlags;
		HRESULT hr = m_pAmVideoProcAmp->Get(VideoProcAmp_ColorEnable, (long*)&m_bColorEnable, &lFlags);
		if (!SUCCEEDED(hr))
		{
			GetDlgItem(IDC_CHECK_COLOR_ENB)->EnableWindow(FALSE);
		}

		long lMin, lMax, lSteppingDelta, lDefault, lCapsFlags, lValue;
		hr = m_pAmVideoProcAmp->GetRange(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &lMin, &lMax, &lSteppingDelta, &lDefault, &lCapsFlags);
		if (!SUCCEEDED(hr))
		{
			GetDlgItem(IDC_COMBO_PL_FREQ)->EnableWindow(FALSE);
		}
		else
		{
			hr = m_pAmVideoProcAmp->Get(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &lValue, &lFlags);
			if (!SUCCEEDED(hr) || lValue == 0)
			{
				GetDlgItem(IDC_COMBO_PL_FREQ)->EnableWindow(FALSE);
			}
			else
			{
				if (POWERLINE_FREQ_50_HZ >= lMin && POWERLINE_FREQ_50_HZ <= lMax)
				{
					m_comboPowerlineFreq.AddString("50 Hz");
					m_comboPowerlineFreq.SetItemData(0, POWERLINE_FREQ_50_HZ);

					if (lValue == POWERLINE_FREQ_50_HZ)
						m_comboPowerlineFreq.SetCurSel(0);
				}

				if (POWERLINE_FREQ_60_HZ >= lMin && POWERLINE_FREQ_60_HZ <= lMax)
				{
					m_comboPowerlineFreq.AddString("60 Hz");
					m_comboPowerlineFreq.SetItemData(1, POWERLINE_FREQ_60_HZ);

					if (lValue == POWERLINE_FREQ_60_HZ)
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
}

void CVideoProcAmpPage::SetVidroProcAmpProperty(VideoProcAmpPropertyGroup* pPropGroup)
{
	HRESULT hr = m_pAmVideoProcAmp->Set(pPropGroup->lProperty, pPropGroup->lPropertyValue, pPropGroup->lFlags);
}

// CVideoProcAmpPage message handlers

void CVideoProcAmpPage::OnAutoCheckbox(UINT id)
{
	UpdateData(TRUE);
	GetDlgItem(m_mapProperties[id]->nSliderCtrlId)->EnableWindow(!m_mapProperties[id]->bAutoFlag);
	GetDlgItem(m_mapProperties[id]->nEditCtrlId)->EnableWindow(!m_mapProperties[id]->bAutoFlag);
	
	m_mapProperties[id]->lFlags = m_mapProperties[id]->bAutoFlag ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual;

	SetVidroProcAmpProperty(m_mapProperties[id]);
}

void CVideoProcAmpPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != nullptr && pScrollBar->IsKindOf(RUNTIME_CLASS(CSliderCtrl)))
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		int nSliderId = pSlider->GetDlgCtrlID();

		switch (nSBCode)
		{
		case SB_THUMBTRACK:
			m_mapProperties[nSliderId]->lPropertyValue = nPos;
			break;

		case SB_LINELEFT:
		case SB_LINERIGHT:
		case SB_PAGELEFT:
		case SB_PAGERIGHT:
			m_mapProperties[nSliderId]->lPropertyValue = pSlider->GetPos();
			break;

		default:
			break;
		}

		m_mapProperties[nSliderId]->strLastValidText = std::to_string(m_mapProperties[nSliderId]->lPropertyValue).c_str();
		SetVidroProcAmpProperty(m_mapProperties[nSliderId]);
	}

	UpdateData(FALSE);
	CMFCPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVideoProcAmpPage::OnEditEnter(UINT nEditCtrlId)
{
	UpdateData(TRUE);

	if (m_mapProperties[nEditCtrlId]->lPropertyValue < m_mapProperties[nEditCtrlId]->lMin)
		m_mapProperties[nEditCtrlId]->lPropertyValue = m_mapProperties[nEditCtrlId]->lMin;
	else if (m_mapProperties[nEditCtrlId]->lPropertyValue > m_mapProperties[nEditCtrlId]->lMax)
		m_mapProperties[nEditCtrlId]->lPropertyValue = m_mapProperties[nEditCtrlId]->lMax;

	m_mapProperties[nEditCtrlId]->slider.SetPos(m_mapProperties[nEditCtrlId]->lPropertyValue);
	m_mapProperties[nEditCtrlId]->strLastValidText = std::to_string(m_mapProperties[nEditCtrlId]->lPropertyValue).c_str();
	UpdateData(FALSE);
	SetVidroProcAmpProperty(m_mapProperties[nEditCtrlId]);
}

void CVideoProcAmpPage::OnEditChange(UINT nID)
{
	CString strText;
	CEdit* pEditCtrl = (CEdit*) GetDlgItem(nID);
	
	pEditCtrl->GetWindowText(strText);

	// Allow empty string for the user to delete input
	if (strText.IsEmpty()) return;

	// Check if the text is a valid number
	if (!IsValidNumber(strText))
	{
		// If not valid, revert the last change
		MessageBeep(MB_ICONEXCLAMATION);
		strText = m_mapProperties[nID]->strLastValidText;
		pEditCtrl->SetWindowText(strText);
		pEditCtrl->SetSel(strText.GetLength(), strText.GetLength());  // Place cursor at the end
	}
	else
	{
		m_mapProperties[nID]->strLastValidText = strText;  // Update last valid text
	}
}

bool CVideoProcAmpPage::IsValidNumber(const CString& str)
{
	std::regex regexPattern("^-?\\d*$");
	return std::regex_match((LPCTSTR)str, regexPattern);
}


void CVideoProcAmpPage::OnBnClickedDefault()
{
	for (int i = IDC_EDIT_BRIGHTNESS; i <= IDC_EDIT_GAIN; i++)
	{
		m_mapProperties[i]->lPropertyValue = m_mapProperties[i]->lDefault;
		m_mapProperties[i]->slider.SetPos(m_mapProperties[i]->lPropertyValue);
		m_mapProperties[i]->strLastValidText = std::to_string(m_mapProperties[i]->lPropertyValue).c_str();
		SetVidroProcAmpProperty(m_mapProperties[i]);
	}

	UpdateData(FALSE);
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
