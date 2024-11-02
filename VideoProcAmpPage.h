#pragma once

#include <strmif.h>
#include <unordered_map>

struct VideoProcAmpPropertyGroup
{
	long lProperty;
	long lMin;
	long lMax;
	long lSteppingDelta;
	long lDefault;
	long lCapsFlags;
	long lFlags;

	CSliderCtrl slider;
	long lPropertyValue;
	BOOL bAutoFlag;

	int nSliderCtrlId;
	int nEditCtrlId;
	int nCheckboxId;

	CString strLastValidText;
};

typedef std::unordered_map<int, VideoProcAmpPropertyGroup*> VideoPorcAmpPropMap;

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

	void SetupVideoProcAmpProperty(long lProperty, VideoProcAmpPropertyGroup& propGroup, int nSliderId, int nEditId, int nCheckboxId);
	void SetVidroProcAmpProperty(VideoProcAmpPropertyGroup* pPropGroup);
	bool IsValidNumber(const CString& str);

private:
	CComPtr<IAMVideoProcAmp> m_pAmVideoProcAmp;
	VideoPorcAmpPropMap m_mapProperties;

protected:
	afx_msg void OnAutoCheckbox(UINT id);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditEnter(UINT nEditCtrlId);
	afx_msg void OnEditChange(UINT nID);
	afx_msg void OnChangePowerLineFreq();
	DECLARE_MESSAGE_MAP()
	
	VideoProcAmpPropertyGroup m_propBrightness;
	VideoProcAmpPropertyGroup m_propContrast;
	VideoProcAmpPropertyGroup m_propHue;
	VideoProcAmpPropertyGroup m_propSaturation;
	VideoProcAmpPropertyGroup m_propSharpness;
	VideoProcAmpPropertyGroup m_propGamma;
	VideoProcAmpPropertyGroup m_propWhiteBalance;
	VideoProcAmpPropertyGroup m_propBacklightCompensation;
	VideoProcAmpPropertyGroup m_propGain;

	BOOL m_bColorEnable;
	CComboBox m_comboPowerlineFreq;

public:
	afx_msg void OnBnClickedDefault();
	afx_msg void OnBnClickedCheckColorEnb();
};
