// DeviceCapsDlg.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "DeviceCapsDlg.h"
#include "afxdialogex.h"


// CDeviceCapsDlg dialog

IMPLEMENT_DYNAMIC(CDeviceCapsDlg, CDialogEx)

CDeviceCapsDlg::CDeviceCapsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEVICE_CAPS, pParent)
{
	m_iFormatIdx = 0;
	m_iResolutionIdx = 0;
	m_iFramerateIdx = 0;
}

CDeviceCapsDlg::~CDeviceCapsDlg()
{
}

void CDeviceCapsDlg::PopulateFormatList()
{
	for (DeviceCapsPtr pDeviceCaps : *m_deviceCaps)
		m_formatList.AddString(pDeviceCaps->m_strFormat.c_str());
}

void CDeviceCapsDlg::PopulateResolutionList(int iFormatIdx)
{
	ResolutionListPtr resolutionList = (*m_deviceCaps)[iFormatIdx]->m_resolutions;

	for (ResolutionPtr pResolution : *resolutionList)
	{
		std::string strResolution = std::to_string(pResolution->m_iWidth) + " x " + std::to_string(pResolution->m_iHeight);
		m_resolutionList.AddString(strResolution.c_str());
	}
}

void CDeviceCapsDlg::PopulateFramerateList(int iFormatIdx, int iResolutionIdx)
{
	FramerateListPtr framerateList = (*(*m_deviceCaps)[iFormatIdx]->m_resolutions)[iResolutionIdx]->m_framerates;
	char pcFramerate[16];

	for (FractionPtr pFramerate : *framerateList)
	{
		sprintf_s(pcFramerate, sizeof(pcFramerate), "%.2f", (float)pFramerate->first / pFramerate->second);
		m_framerateList.AddString(pcFramerate);
	}
}

void CDeviceCapsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FORMAT, m_formatList);
	DDX_Control(pDX, IDC_COMBO_FRAME_SIZE, m_resolutionList);
	DDX_Control(pDX, IDC_COMBO_FRAMERATE, m_framerateList);
}

BOOL CDeviceCapsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	PopulateFormatList();
	PopulateResolutionList(m_iFormatIdx);
	PopulateFramerateList(m_iFormatIdx, m_iResolutionIdx);

	m_formatList.SetCurSel(m_iFormatIdx);
	m_resolutionList.SetCurSel(m_iResolutionIdx);
	m_framerateList.SetCurSel(m_iFramerateIdx);

	return TRUE;
}

void CDeviceCapsDlg::UpdateDeviceCaps(std::string strSource, int iDeviceIdx, DeviceCapsListPtr deviceCaps)
{
	if (m_iDeviceIndex != iDeviceIdx || m_strSource != strSource)
	{
		m_deviceCaps		= deviceCaps;
		m_iDeviceIndex		= iDeviceIdx;
		m_strSource			= strSource;
		m_iFormatIdx		= 0;
		m_iResolutionIdx	= 0;
		m_iFramerateIdx		= 0;
	}
}


BEGIN_MESSAGE_MAP(CDeviceCapsDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_FORMAT, &CDeviceCapsDlg::OnChangeFormat)
	ON_CBN_SELCHANGE(IDC_COMBO_FRAME_SIZE, &CDeviceCapsDlg::OnChangeResolution)
	ON_BN_CLICKED(IDOK, &CDeviceCapsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CDeviceCapsDlg message handlers


void CDeviceCapsDlg::OnChangeFormat()
{
	int iFormatIdx = m_formatList.GetCurSel();
	
	m_resolutionList.ResetContent();

	PopulateResolutionList(iFormatIdx);

	m_resolutionList.SetCurSel(0);
	OnChangeResolution();
}


void CDeviceCapsDlg::OnChangeResolution()
{
	int iFormatIndex = m_formatList.GetCurSel();
	int iResolutionIndex = m_resolutionList.GetCurSel();
	FramerateListPtr framerateList = (*(*m_deviceCaps)[iFormatIndex]->m_resolutions)[iResolutionIndex]->m_framerates;
	char pcFramerate[16];

	m_framerateList.ResetContent();

	for (FractionPtr pFramerate : *framerateList)
	{
		sprintf_s(pcFramerate, sizeof(pcFramerate), "%.2f", (float)pFramerate->first / pFramerate->second);
		m_framerateList.AddString(pcFramerate);
	}

	m_framerateList.SetCurSel(0);
}


void CDeviceCapsDlg::OnBnClickedOk()
{
	m_iFormatIdx = m_formatList.GetCurSel();
	m_iResolutionIdx = m_resolutionList.GetCurSel();
	m_iFramerateIdx = m_framerateList.GetCurSel();

	CDialogEx::OnOK();
}
