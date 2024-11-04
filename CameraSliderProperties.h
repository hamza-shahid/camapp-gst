#pragma once

#include <strmif.h>
#include <unordered_map>
#include <memory>


struct CameraProperty
{
	long lProperty;
	long lMin;
	long lMax;
	long lSteppingDelta;
	long lDefault;
	long lCapsFlags;
	long lFlags;
	long lPropertyValue;
};

struct CameraSliderProperty
{
	CameraProperty property;
	BOOL bAutoFlag;
	CSliderCtrl slider;
	int nSliderCtrlId;
	int nEditCtrlId;
	int nCheckboxCtrlId;

	CString strLastValidText;
};

typedef std::shared_ptr< CameraSliderProperty > CameraSliderPropertyPtr;
typedef std::unordered_map<int, CameraSliderPropertyPtr> CameraSliderPropertyMap;


class CameraSliderProperties
{
public:
	CameraSliderProperties(CWnd* pParent);
	
	void AddCameraSliderProperty(long lProperty, int nSliderCtrlId, int nEditCtrlId, int nCheckboxCtrlId);
	void SetupCameraSliderProperty(long lProperty);

	CSliderCtrl& GetSliderCtrl(int nSliderCtrlId) { return m_mapProperties[nSliderCtrlId]->slider; }
	long& GetEditCtrlValue(int nEditCtrlId) { return m_mapProperties[nEditCtrlId]->property.lPropertyValue; }
	BOOL& GetCheckboxCtrlValue(int nCheckboxCtrlId) { return m_mapProperties[nCheckboxCtrlId]->bAutoFlag; }

	void SetAmVideoProcInterface(CComPtr<IAMVideoProcAmp> pAmVideoProcAmp) { m_pAmVideoProcAmp = pAmVideoProcAmp; }
	void SetAmCameraControlInterface(CComPtr<IAMCameraControl> pAmCameraControl) { m_pAmCameraControl = pAmCameraControl; }

	void SetProperty(long lProperty, long lPropertyValue, long lFlags);
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void OnAutoCheckbox(UINT id);
	void OnEditEnter(UINT nEditCtrlId);
	void OnEditChange(UINT nID);
	void SetDefault(int nPropertyIdx);

private:
	bool IsValidNumber(const CString& str);

private:
	CWnd*						m_pParent;
	CComPtr<IAMVideoProcAmp>	m_pAmVideoProcAmp;
	CComPtr<IAMCameraControl>	m_pAmCameraControl;
	CameraSliderPropertyMap		m_mapProperties;
};


