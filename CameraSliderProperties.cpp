#include "pch.h"
#include "CameraSliderProperties.h"

#include <string>
#include <regex>


CameraSliderProperties::CameraSliderProperties(CWnd* pParent)
{
	m_pParent = pParent;
	m_pAmVideoProcAmp = nullptr;
	m_pAmCameraControl = nullptr;
}

void CameraSliderProperties::AddCameraSliderProperty(long lProperty, int nSliderCtrlId, int nEditCtrlId, int nCheckboxCtrlId)
{
	CameraSliderPropertyPtr pCamSiderProp = std::make_shared< CameraSliderProperty >();

	pCamSiderProp->nSliderCtrlId = nSliderCtrlId;
	pCamSiderProp->nEditCtrlId = nEditCtrlId;
	pCamSiderProp->nCheckboxCtrlId = nCheckboxCtrlId;

	m_mapProperties[lProperty] = pCamSiderProp;
	m_mapProperties[nSliderCtrlId] = pCamSiderProp;
	m_mapProperties[nEditCtrlId] = pCamSiderProp;
	m_mapProperties[nCheckboxCtrlId] = pCamSiderProp;
}

void CameraSliderProperties::SetupCameraSliderProperty(long lProperty)
{
	HRESULT hr;
	CameraSliderPropertyPtr pCamSiderProp = m_mapProperties[lProperty];
	
	if (m_pAmVideoProcAmp)
	{
		hr = m_pAmVideoProcAmp->GetRange(
			lProperty,
			&pCamSiderProp->property.lMin,
			&pCamSiderProp->property.lMax,
			&pCamSiderProp->property.lSteppingDelta,
			&pCamSiderProp->property.lDefault,
			&pCamSiderProp->property.lCapsFlags
		);

		hr = m_pAmVideoProcAmp->Get(lProperty, &pCamSiderProp->property.lPropertyValue, &pCamSiderProp->property.lFlags);
	}
	else
	{
		hr = m_pAmCameraControl->GetRange(
			lProperty,
			&pCamSiderProp->property.lMin,
			&pCamSiderProp->property.lMax,
			&pCamSiderProp->property.lSteppingDelta,
			&pCamSiderProp->property.lDefault,
			&pCamSiderProp->property.lCapsFlags
		);

		hr = m_pAmCameraControl->Get(lProperty, &pCamSiderProp->property.lPropertyValue, &pCamSiderProp->property.lFlags);
	}

	pCamSiderProp->property.lProperty = lProperty;
	
	if (FAILED(hr))
	{
		pCamSiderProp->slider.SetRange(0, 10);
		pCamSiderProp->slider.SetPos(0);
		pCamSiderProp->property.lPropertyValue = 0;
		pCamSiderProp->property.lDefault = 0;
		m_pParent->GetDlgItem(pCamSiderProp->nSliderCtrlId)->EnableWindow(FALSE);
		m_pParent->GetDlgItem(pCamSiderProp->nEditCtrlId)->EnableWindow(FALSE);
		m_pParent->GetDlgItem(pCamSiderProp->nEditCtrlId)->SetWindowTextA("");
		m_pParent->GetDlgItem(pCamSiderProp->nCheckboxCtrlId)->EnableWindow(FALSE);
	}
	else
	{
		pCamSiderProp->slider.SetRange(pCamSiderProp->property.lMin, pCamSiderProp->property.lMax);
		pCamSiderProp->slider.SetPos(pCamSiderProp->property.lPropertyValue);
		pCamSiderProp->slider.SetLineSize(pCamSiderProp->property.lSteppingDelta);
		pCamSiderProp->slider.SetPageSize(pCamSiderProp->property.lSteppingDelta);

		if (pCamSiderProp->property.lCapsFlags == VideoProcAmp_Flags_Auto)
		{
			pCamSiderProp->bAutoFlag = TRUE;
			m_pParent->GetDlgItem(pCamSiderProp->nCheckboxCtrlId)->EnableWindow(FALSE);
		}
		else if (pCamSiderProp->property.lCapsFlags == VideoProcAmp_Flags_Manual)
		{
			pCamSiderProp->bAutoFlag = (pCamSiderProp->property.lFlags == VideoProcAmp_Flags_Auto ? TRUE : FALSE);
			m_pParent->GetDlgItem(pCamSiderProp->nCheckboxCtrlId)->EnableWindow(FALSE);
		}
		else if (pCamSiderProp->property.lCapsFlags == (VideoProcAmp_Flags_Auto | VideoProcAmp_Flags_Manual))
		{
			pCamSiderProp->bAutoFlag = (pCamSiderProp->property.lFlags == VideoProcAmp_Flags_Auto ? TRUE : FALSE);
			m_pParent->GetDlgItem(pCamSiderProp->nCheckboxCtrlId)->EnableWindow(TRUE);
		}

		m_pParent->GetDlgItem(pCamSiderProp->nSliderCtrlId)->EnableWindow(!pCamSiderProp->bAutoFlag);
		m_pParent->GetDlgItem(pCamSiderProp->nEditCtrlId)->EnableWindow(!pCamSiderProp->bAutoFlag);
	}
}

void CameraSliderProperties::SetProperty(long lProperty, long lPropertyValue, long lFlags)
{
	HRESULT hr;
	if (m_pAmVideoProcAmp)
		hr = m_pAmVideoProcAmp->Set(lProperty, lPropertyValue, lFlags);
	else if (m_pAmCameraControl)
		hr = m_pAmCameraControl->Set(lProperty, lPropertyValue, lFlags);
}

void CameraSliderProperties::OnAutoCheckbox(UINT id)
{
	m_pParent->GetDlgItem(m_mapProperties[id]->nSliderCtrlId)->EnableWindow(!m_mapProperties[id]->bAutoFlag);
	m_pParent->GetDlgItem(m_mapProperties[id]->nEditCtrlId)->EnableWindow(!m_mapProperties[id]->bAutoFlag);
	m_mapProperties[id]->property.lFlags = m_mapProperties[id]->bAutoFlag ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual;

	SetProperty(m_mapProperties[id]->property.lProperty, m_mapProperties[id]->property.lPropertyValue, m_mapProperties[id]->property.lFlags);
}

void CameraSliderProperties::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != nullptr && pScrollBar->IsKindOf(RUNTIME_CLASS(CSliderCtrl)))
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		int nSliderId = pSlider->GetDlgCtrlID();

		switch (nSBCode)
		{
		case SB_THUMBTRACK:
			m_mapProperties[nSliderId]->property.lPropertyValue = nPos;
			break;

		case SB_LINELEFT:
		case SB_LINERIGHT:
		case SB_PAGELEFT:
		case SB_PAGERIGHT:
			m_mapProperties[nSliderId]->property.lPropertyValue = pSlider->GetPos();
			break;

		default:
			break;
		}

		m_mapProperties[nSliderId]->strLastValidText = std::to_string(m_mapProperties[nSliderId]->property.lPropertyValue).c_str();
		SetProperty(m_mapProperties[nSliderId]->property.lProperty, m_mapProperties[nSliderId]->property.lPropertyValue, m_mapProperties[nSliderId]->property.lFlags);
	}
}

void CameraSliderProperties::OnEditEnter(UINT nEditCtrlId)
{
	if (m_mapProperties[nEditCtrlId]->property.lPropertyValue < m_mapProperties[nEditCtrlId]->property.lMin)
		m_mapProperties[nEditCtrlId]->property.lPropertyValue = m_mapProperties[nEditCtrlId]->property.lMin;
	else if (m_mapProperties[nEditCtrlId]->property.lPropertyValue > m_mapProperties[nEditCtrlId]->property.lMax)
		m_mapProperties[nEditCtrlId]->property.lPropertyValue = m_mapProperties[nEditCtrlId]->property.lMax;

	m_mapProperties[nEditCtrlId]->slider.SetPos(m_mapProperties[nEditCtrlId]->property.lPropertyValue);
	m_mapProperties[nEditCtrlId]->strLastValidText = std::to_string(m_mapProperties[nEditCtrlId]->property.lPropertyValue).c_str();
	
	SetProperty(m_mapProperties[nEditCtrlId]->property.lProperty, m_mapProperties[nEditCtrlId]->property.lPropertyValue, m_mapProperties[nEditCtrlId]->property.lFlags);
}

void CameraSliderProperties::OnEditChange(UINT nID)
{
	CString strText;
	CEdit* pEditCtrl = (CEdit*)m_pParent->GetDlgItem(nID);

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

bool CameraSliderProperties::IsValidNumber(const CString& str)
{
	std::regex regexPattern("^-?\\d*$");
	return std::regex_match((LPCTSTR)str, regexPattern);
}

void CameraSliderProperties::SetDefault(int nPropertyIdx)
{
	m_mapProperties[nPropertyIdx]->property.lPropertyValue = m_mapProperties[nPropertyIdx]->property.lDefault;
	m_mapProperties[nPropertyIdx]->slider.SetPos(m_mapProperties[nPropertyIdx]->property.lPropertyValue);
	m_mapProperties[nPropertyIdx]->strLastValidText = std::to_string(m_mapProperties[nPropertyIdx]->property.lPropertyValue).c_str();
	SetProperty(m_mapProperties[nPropertyIdx]->property.lProperty, m_mapProperties[nPropertyIdx]->property.lPropertyValue, m_mapProperties[nPropertyIdx]->property.lFlags);
}
