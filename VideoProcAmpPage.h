#pragma once

#include <strmif.h>
#include <unordered_map>

#include "CameraSliderProperties.h"

// CVideoProcAmpPage dialog

class CVideoProcAmpPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CVideoProcAmpPage)

public:
	CVideoProcAmpPage();   // standard constructor
	virtual ~CVideoProcAmpPage();

	void SetVideoProcAmpInterface(CComPtr<IAMVideoProcAmp> pAmVideoProcAmp);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_VIDEO_PROC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	CComPtr<IAMVideoProcAmp> m_pAmVideoProcAmp;
	CameraSliderProperties* m_pSliderProperties;
	BOOL m_bColorEnable;
	CComboBox m_comboPowerlineFreq;
	CameraProperty m_propPowerlineFreq;

protected:
	afx_msg void OnAutoCheckbox(UINT id);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditEnter(UINT nEditCtrlId);
	afx_msg void OnEditChange(UINT nID);
	afx_msg void OnChangePowerLineFreq();
	afx_msg void OnBnClickedDefault();
	afx_msg void OnBnClickedCheckColorEnb();
	DECLARE_MESSAGE_MAP()
};
