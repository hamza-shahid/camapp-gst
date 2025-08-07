#pragma once

#include <string>

#include "GstPlayer.h"

// CDeviceCapsDlg dialog

class CDeviceCapsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDeviceCapsDlg)

public:
	CDeviceCapsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDeviceCapsDlg();

	void UpdateDeviceCaps(std::string strSource, int iDeviceIdx, DeviceCapsListPtr deviceCaps);
	void SelectDeviceCaps(std::string strFormat, int iWidth, int iHeight, int iFramerateNum, int iFramerateDen);
	
	int GetFormatIdx() { return m_iFormatIdx; }
	int GetResolutionIdx() { return m_iResolutionIdx; }
	int GetFramerateIdx() { return m_iFramerateIdx; }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEVICE_CAPS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void PopulateFormatList();
	void PopulateResolutionList(int iFormatIdx);
	void PopulateFramerateList(int iFormatIdx, int iResolutionIdx);

	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_formatList;
	CComboBox m_resolutionList;
	CComboBox m_framerateList;

	int					m_iDeviceIndex;
	std::string			m_strSource;
	DeviceCapsListPtr	m_deviceCaps;
	int					m_iFormatIdx;
	int					m_iResolutionIdx;
	int					m_iFramerateIdx;

public:
	afx_msg void OnChangeFormat();
	afx_msg void OnChangeResolution();
	afx_msg void OnBnClickedOk();
};
