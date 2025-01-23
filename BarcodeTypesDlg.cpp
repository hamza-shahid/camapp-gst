// BarcodeTypesDlg.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "BarcodeTypesDlg.h"
#include "afxdialogex.h"
#include "BarcodeFormat.h"


#define IsChecked(id) ((BOOL)(DWORD)GetDlgItem(id)->SendMessage(BM_GETCHECK, 0L, 0L))

// CBarcodeTypesDlg dialog

IMPLEMENT_DYNAMIC(CBarcodeTypesDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CBarcodeTypesDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_BARCODE_ALL, &CBarcodeTypesDlg::OnBnClickedCheckBarcodeAll)
	ON_BN_CLICKED(IDC_CHECK_BARCODE_LINEAR, &CBarcodeTypesDlg::OnBnClickedCheckBarcodeLinear)
	ON_BN_CLICKED(IDC_CHECK_BARCODE_MATRIX, &CBarcodeTypesDlg::OnBnClickedCheckBarcodeMatrix)
	ON_BN_CLICKED(IDOK, &CBarcodeTypesDlg::OnBnClickedOk)
END_MESSAGE_MAP()


CBarcodeTypesDlg::CBarcodeTypesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_BARCODE_TYPES, pParent)
	, m_uBarcodeFormats(0)
	, m_bAllCodesEnabled(TRUE)
	, m_bLinearCodeEnabled(TRUE)
	, m_bMatrixCodeEnabled(TRUE)
{
	m_barcodeMap[IDC_CHECK_BARCODE_AZTEC] = BarcodeFormat_Aztec;
	m_barcodeMap[IDC_CHECK_BARCODE_CODABAR] = BarcodeFormat_Codabar;
	m_barcodeMap[IDC_CHECK_BARCODE_CODE39] = BarcodeFormat_Code39;
	m_barcodeMap[IDC_CHECK_BARCODE_CODE93] = BarcodeFormat_Code93;
	m_barcodeMap[IDC_CHECK_BARCODE_CODE128] = BarcodeFormat_Code128;
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR] = BarcodeFormat_DataBar;
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR_EXPANDED] = BarcodeFormat_DataBarExpanded;
	m_barcodeMap[IDC_CHECK_BARCODE_DATA_MATRIX] = BarcodeFormat_DataMatrix;
	m_barcodeMap[IDC_CHECK_BARCODE_EAN8] = BarcodeFormat_EAN8;
	m_barcodeMap[IDC_CHECK_BARCODE_EAN13] = BarcodeFormat_EAN13;
	m_barcodeMap[IDC_CHECK_BARCODE_ITF] = BarcodeFormat_ITF;
	m_barcodeMap[IDC_CHECK_BARCODE_MAXI_CODE] = BarcodeFormat_MaxiCode;
	m_barcodeMap[IDC_CHECK_BARCODE_PDF417] = BarcodeFormat_PDF417;
	m_barcodeMap[IDC_CHECK_BARCODE_QR_CODE] = BarcodeFormat_QRCode;
	m_barcodeMap[IDC_CHECK_BARCODE_UPCA] = BarcodeFormat_UPCA;
	m_barcodeMap[IDC_CHECK_BARCODE_UPCE] = BarcodeFormat_UPCE;
	m_barcodeMap[IDC_CHECK_BARCODE_MICRO_QR_CODE] = BarcodeFormat_MicroQRCode;
	m_barcodeMap[IDC_CHECK_BARCODE_RM_QR_CODE] = BarcodeFormat_RMQRCode;
	m_barcodeMap[IDC_CHECK_BARCODE_DX_FILM_EDGE] = BarcodeFormat_DXFilmEdge;
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR_LIMITED] = BarcodeFormat_DataBarLimited;
	m_barcodeMap[IDC_CHECK_BARCODE_LINEAR] = BarcodeFormat_LinearCodes;
	m_barcodeMap[IDC_CHECK_BARCODE_MATRIX] = BarcodeFormat_MatrixCodes;
	m_barcodeMap[IDC_CHECK_BARCODE_ALL] = BarcodeFormat_Any;
}

CBarcodeTypesDlg::~CBarcodeTypesDlg()
{
}

void CBarcodeTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Check(pDX, IDC_CHECK_BARCODE_ALL, m_bAllCodesEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_LINEAR, m_bLinearCodeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_MATRIX, m_bMatrixCodeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_AZTEC, m_bAztecEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_CODABAR, m_bCodabarEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_CODE39, m_bCode39Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_CODE93, m_bCode93Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_CODE128, m_bCode128Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_DATABAR, m_bDataBarEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_DATABAR_EXPANDED, m_bDataBarExpandedEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_DATA_MATRIX, m_bDataMatrixEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_EAN8, m_bEAN8Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_EAN13, m_bEAN13Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_ITF, m_bITFEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_MAXI_CODE, m_bMaxiCodeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_PDF417, m_bPDF417Enabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_QR_CODE, m_bQRCode);
	DDX_Check(pDX, IDC_CHECK_BARCODE_UPCA, m_bUPCAEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_UPCE, m_bUPCEEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_MICRO_QR_CODE, m_bMicroQRCodeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_RM_QR_CODE, m_bRMQRCodeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_DX_FILM_EDGE, m_bDXFilmEdgeEnabled);
	DDX_Check(pDX, IDC_CHECK_BARCODE_DATABAR_LIMITED, m_bDataBarLimited);
}

BOOL CBarcodeTypesDlg::OnInitDialog()
{
	EnableLinearCodes(!m_bLinearCodeEnabled);
	EnableMatrixCodes(!m_bMatrixCodeEnabled);

	GetDlgItem(IDC_CHECK_BARCODE_LINEAR)->EnableWindow(!m_bAllCodesEnabled);
	GetDlgItem(IDC_CHECK_BARCODE_MATRIX)->EnableWindow(!m_bAllCodesEnabled);

	return CDialogEx::OnInitDialog();
}

void CBarcodeTypesDlg::EnableLinearCodes(bool bEnable)
{
	for (UINT lc = IDC_CHECK_BARCODE_CODABAR; lc <= IDC_CHECK_BARCODE_UPCE; lc++)
		GetDlgItem(lc)->EnableWindow(bEnable);
}

void CBarcodeTypesDlg::EnableMatrixCodes(bool bEnable)
{
	for (UINT mc = IDC_CHECK_BARCODE_AZTEC; mc <= IDC_CHECK_BARCODE_RM_QR_CODE; mc++)
		GetDlgItem(mc)->EnableWindow(bEnable);
}

UINT CBarcodeTypesDlg::GetBarcodeFormats()
{
	return m_uBarcodeFormats;
}

// CBarcodeTypesDlg message handlers

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeAll()
{
	UpdateData(TRUE);
	OnBnClickedCheckBarcodeLinear();
	OnBnClickedCheckBarcodeMatrix();

	GetDlgItem(IDC_CHECK_BARCODE_LINEAR)->EnableWindow(!m_bAllCodesEnabled);
	GetDlgItem(IDC_CHECK_BARCODE_MATRIX)->EnableWindow(!m_bAllCodesEnabled);
}

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeLinear()
{
	UpdateData(TRUE);
	EnableLinearCodes(m_bAllCodesEnabled ? FALSE : !m_bLinearCodeEnabled);
}

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeMatrix()
{
	UpdateData(TRUE);
	EnableMatrixCodes(m_bAllCodesEnabled ? FALSE : !m_bMatrixCodeEnabled);
}

void CBarcodeTypesDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	m_uBarcodeFormats = 0;

	if (m_bAllCodesEnabled)
		m_uBarcodeFormats = m_barcodeMap[IDC_CHECK_BARCODE_ALL];

	if (m_bLinearCodeEnabled)
		m_uBarcodeFormats |= m_barcodeMap[IDC_CHECK_BARCODE_LINEAR];
	else
	{
		for (UINT lc = IDC_CHECK_BARCODE_CODABAR; lc <= IDC_CHECK_BARCODE_UPCE; lc++)
		{
			m_uBarcodeFormats |= IsChecked(lc) ? m_barcodeMap[lc] : 0;
		}
	}

	if (m_bMatrixCodeEnabled)
		m_uBarcodeFormats |= m_barcodeMap[IDC_CHECK_BARCODE_MATRIX];
	else
	{
		for (UINT mc = IDC_CHECK_BARCODE_AZTEC; mc <= IDC_CHECK_BARCODE_RM_QR_CODE; mc++)
			m_uBarcodeFormats |= IsChecked(mc) ? m_barcodeMap[mc] : 0;
	}

	CDialogEx::OnOK();
}
