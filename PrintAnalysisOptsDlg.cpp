// PrintAnalysisOptsDlg.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "PrintAnalysisOptsDlg.h"
#include "afxdialogex.h"

#include <string>

// CPrintAnalysisOptsDlg dialog

IMPLEMENT_DYNAMIC(CPrintAnalysisOptsDlg, CDialogEx)

CPrintAnalysisOptsDlg::CPrintAnalysisOptsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PRINT_ANALYSIS_OPTS, pParent)
{
	m_nAnalysisType = IDC_RADIO_ANALYSIS_NONE;
	m_nGrayscaleType = IDC_RADIO_GRAY_NONE;
	m_nBlackoutType = IDC_RADIO_BLACK_NONE;
	m_bConnectValues = FALSE;
	m_nAoiHeight = 100;
	m_nAoiPartitions = 1;
}

CPrintAnalysisOptsDlg::~CPrintAnalysisOptsDlg()
{
}

void CPrintAnalysisOptsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CPrintAnalysisOptsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CheckRadioButton(IDC_RADIO_INTENSITY, IDC_RADIO_ANALYSIS_NONE, m_nAnalysisType);
	CheckRadioButton(IDC_RADIO_BLACK_WHOLE, IDC_RADIO_BLACK_NONE, m_nBlackoutType);
	CheckRadioButton(IDC_RADIO_GRAY_WHOLE, IDC_RADIO_GRAY_NONE, m_nGrayscaleType);

	GetDlgItem(IDC_CHECK_CONNECT_VALUES)->SendMessage(BM_SETCHECK, (WPARAM)(int)(m_bConnectValues), 0L);
	GetDlgItem(IDC_EDIT_AOI_HEIGHT)->SetWindowTextA(std::to_string(m_nAoiHeight).c_str());
	GetDlgItem(IDC_EDIT_AOI_PARTITIONS)->SetWindowTextA(std::to_string(m_nAoiPartitions).c_str());

	return TRUE;
}

void CPrintAnalysisOptsDlg::SetPrintAnalysisOpts(int nAnalysisType, int nGrayscaleType, int nBlackoutType, BOOL bConnectValues, int nAoiHeight, int nAoiPartitions)
{
	m_nAnalysisType = nAnalysisType;
	m_nGrayscaleType = nGrayscaleType;
	m_nBlackoutType = nBlackoutType;
	m_bConnectValues = bConnectValues;
	m_nAoiHeight = nAoiHeight;
	m_nAoiPartitions = nAoiPartitions;
}


int CPrintAnalysisOptsDlg::GetAnalysisType()
{
	return m_nAnalysisType;
}

int CPrintAnalysisOptsDlg::GetGrayscaleType()
{
	return m_nGrayscaleType;
}

int CPrintAnalysisOptsDlg::GetBlackoutType()
{
	return m_nBlackoutType;
}

BOOL CPrintAnalysisOptsDlg::GetConnectValues()
{
	return m_bConnectValues;
}

int CPrintAnalysisOptsDlg::GetAoiHeight()
{
	return m_nAoiHeight;
}

int CPrintAnalysisOptsDlg::GetAoiPartitions()
{
	return m_nAoiPartitions;
}

void CPrintAnalysisOptsDlg::SavePrintAnalysisOpts()
{
	m_nAnalysisType = GetCheckedRadioButton(IDC_RADIO_INTENSITY, IDC_RADIO_ANALYSIS_NONE);
	m_nGrayscaleType = GetCheckedRadioButton(IDC_RADIO_GRAY_WHOLE, IDC_RADIO_GRAY_NONE);
	m_nBlackoutType = GetCheckedRadioButton(IDC_RADIO_BLACK_WHOLE, IDC_RADIO_BLACK_NONE);
	m_bConnectValues = (BOOL)GetDlgItem(IDC_CHECK_CONNECT_VALUES)->SendMessage(BM_GETCHECK, 0L, 0L);

	char sz[256];

	GetDlgItem(IDC_EDIT_AOI_HEIGHT)->GetWindowTextA(sz, sizeof(sz));
	m_nAoiHeight = atoi(sz);

	GetDlgItem(IDC_EDIT_AOI_PARTITIONS)->GetWindowTextA(sz, sizeof(sz));
	m_nAoiPartitions = atoi(sz);
}

BEGIN_MESSAGE_MAP(CPrintAnalysisOptsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPrintAnalysisOptsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CPrintAnalysisOptsDlg::OnBnClickedButtonApply)
END_MESSAGE_MAP()


// CPrintAnalysisOptsDlg message handlers

void CPrintAnalysisOptsDlg::OnBnClickedOk()
{
	SavePrintAnalysisOpts();
	GetParent()->PostMessageA(WM_ON_PRINT_ANALYSIS_CHANGE_OPTS, 0L, 0L);
	CDialogEx::OnOK();
}


void CPrintAnalysisOptsDlg::OnBnClickedButtonApply()
{
	SavePrintAnalysisOpts();
	GetParent()->PostMessageA(WM_ON_PRINT_ANALYSIS_CHANGE_OPTS, 0L, 0L);
}
