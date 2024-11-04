#pragma once

#include <strmif.h>

#include "CameraSliderProperties.h"

// CCameraControlsPage dialog

class CCameraControlsPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CCameraControlsPage)

public:
	CCameraControlsPage();   // standard constructor
	virtual ~CCameraControlsPage();

	void SetCameraControlInterface(CComPtr<IAMCameraControl> pAmCameraControl);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_CAMERA_CONTROLS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	CComPtr<IAMCameraControl> m_pAmCameraControl;
	CameraSliderProperties* m_pSliderProperties;
	BOOL m_bLowLightCompensation;

protected:
	afx_msg void OnAutoCheckbox(UINT id);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditEnter(UINT nEditCtrlId);
	afx_msg void OnEditChange(UINT nID);
	afx_msg void OnBnClickedDefault();
	afx_msg void OnBnClickedCheckLowLightCompensation();
	DECLARE_MESSAGE_MAP()
};
