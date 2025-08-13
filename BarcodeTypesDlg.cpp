// BarcodeTypesDlg.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "BarcodeTypesDlg.h"
#include "afxdialogex.h"


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
	, m_uBarcodeFormats(BarcodeFormat_None)
	, m_uBarcodeColStartX(0)
	, m_uBarcodeColWidth(0)
{
	m_barcodeMap[IDC_CHECK_BARCODE_AZTEC] = std::make_shared<BarcodePair>(BarcodeFormat_Aztec, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_CODABAR] = std::make_shared<BarcodePair>(BarcodeFormat_Codabar, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_CODE39] = std::make_shared<BarcodePair>(BarcodeFormat_Code39, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_CODE93] = std::make_shared<BarcodePair>(BarcodeFormat_Code93, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_CODE128] = std::make_shared<BarcodePair>(BarcodeFormat_Code128, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR] = std::make_shared<BarcodePair>(BarcodeFormat_DataBar, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR_EXPANDED] = std::make_shared<BarcodePair>(BarcodeFormat_DataBarExpanded, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_DATA_MATRIX] = std::make_shared<BarcodePair>(BarcodeFormat_DataMatrix, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_EAN8] = std::make_shared<BarcodePair>(BarcodeFormat_EAN8, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_EAN13] = std::make_shared<BarcodePair>(BarcodeFormat_EAN13, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_ITF] = std::make_shared<BarcodePair>(BarcodeFormat_ITF, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_MAXI_CODE] = std::make_shared<BarcodePair>(BarcodeFormat_MaxiCode, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_PDF417] = std::make_shared<BarcodePair>(BarcodeFormat_PDF417, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_QR_CODE] = std::make_shared<BarcodePair>(BarcodeFormat_QRCode, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_UPCA] = std::make_shared<BarcodePair>(BarcodeFormat_UPCA, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_UPCE] = std::make_shared<BarcodePair>(BarcodeFormat_UPCE, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_MICRO_QR_CODE] = std::make_shared<BarcodePair>(BarcodeFormat_MicroQRCode, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_RM_QR_CODE] = std::make_shared<BarcodePair>(BarcodeFormat_RMQRCode, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_DX_FILM_EDGE] = std::make_shared<BarcodePair>(BarcodeFormat_DXFilmEdge, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_DATABAR_LIMITED] = std::make_shared<BarcodePair>(BarcodeFormat_DataBarLimited, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_LINEAR] = std::make_shared<BarcodePair>(BarcodeFormat_LinearCodes, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_MATRIX] = std::make_shared<BarcodePair>(BarcodeFormat_MatrixCodes, 0);
	m_barcodeMap[IDC_CHECK_BARCODE_ALL] = std::make_shared<BarcodePair>(BarcodeFormat_Any, 0);
}

CBarcodeTypesDlg::~CBarcodeTypesDlg()
{
}

void CBarcodeTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	for (int nIDC = IDC_CHECK_BARCODE_ALL; nIDC <= IDC_CHECK_BARCODE_RM_QR_CODE; nIDC++)
		DDX_Check(pDX, nIDC, m_barcodeMap[nIDC]->bChecked);
	
	DDX_Text(pDX, IDC_EDIT_COLUMN_START_X, m_uBarcodeColStartX);
	DDV_MinMaxUInt(pDX, m_uBarcodeColStartX, 0, UINT_MAX);
	DDX_Text(pDX, IDC_EDIT_COLUMN_WIDTH, m_uBarcodeColWidth);
	DDV_MinMaxUInt(pDX, m_uBarcodeColWidth, 0, UINT_MAX);
}

BOOL CBarcodeTypesDlg::OnInitDialog()
{
	InitBarcodes();

	EnableLinearCodes(!m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->bChecked);
	EnableMatrixCodes(!m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->bChecked);

	GetDlgItem(IDC_CHECK_BARCODE_LINEAR)->EnableWindow(!m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked);
	GetDlgItem(IDC_CHECK_BARCODE_MATRIX)->EnableWindow(!m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked);

	return CDialogEx::OnInitDialog();
}

void CBarcodeTypesDlg::InitBarcodes()
{
	if ((m_uBarcodeFormats & BarcodeFormat_Any) == BarcodeFormat_Any)
	{
		m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked = TRUE;
		m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->bChecked = TRUE;
		m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->bChecked = TRUE;
	}
	else
	{
		if ((m_uBarcodeFormats & BarcodeFormat_LinearCodes) == BarcodeFormat_LinearCodes)
		{
			m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->bChecked = TRUE;
		}
		else
		{
			for (UINT mc = IDC_CHECK_BARCODE_CODABAR; mc <= IDC_CHECK_BARCODE_UPCE; mc++)
			{
				m_barcodeMap[mc]->bChecked = m_uBarcodeFormats & m_barcodeMap[mc]->eBarcodeFormat ? TRUE : FALSE;
			}
		}

		if ((m_uBarcodeFormats & BarcodeFormat_MatrixCodes) == BarcodeFormat_MatrixCodes)
		{
			m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->bChecked = TRUE;
		}
		else
		{
			for (UINT mc = IDC_CHECK_BARCODE_AZTEC; mc <= IDC_CHECK_BARCODE_RM_QR_CODE; mc++)
			{
				m_barcodeMap[mc]->bChecked = m_uBarcodeFormats & m_barcodeMap[mc]->eBarcodeFormat ? TRUE : FALSE;
			}
		}
	}
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

void CBarcodeTypesDlg::SetBarcodeFormats(UINT uBarcodeFormats)
{
	m_uBarcodeFormats = uBarcodeFormats;
}

void CBarcodeTypesDlg::SetBarcodeColStartX(UINT uBarcodeColStartX)
{
	m_uBarcodeColStartX = uBarcodeColStartX;
}

void CBarcodeTypesDlg::SetBarcodeColWidth(UINT uBarcodeColWidth)
{
	m_uBarcodeColWidth = uBarcodeColWidth;
}

UINT CBarcodeTypesDlg::GetBarcodeFormats()
{
	return m_uBarcodeFormats;
}

UINT CBarcodeTypesDlg::GetBarcodeColumnStartX()
{
	return m_uBarcodeColStartX;
}

UINT CBarcodeTypesDlg::GetBarcodeColumnWidth()
{
	return m_uBarcodeColWidth;
}

// CBarcodeTypesDlg message handlers

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeAll()
{
	UpdateData(TRUE);
	OnBnClickedCheckBarcodeLinear();
	OnBnClickedCheckBarcodeMatrix();

	GetDlgItem(IDC_CHECK_BARCODE_LINEAR)->EnableWindow(!m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked);
	GetDlgItem(IDC_CHECK_BARCODE_MATRIX)->EnableWindow(!m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked);
}

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeLinear()
{
	UpdateData(TRUE);
	EnableLinearCodes(m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked ? FALSE : !m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->bChecked);
}

void CBarcodeTypesDlg::OnBnClickedCheckBarcodeMatrix()
{
	UpdateData(TRUE);
	EnableMatrixCodes(m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked ? FALSE : !m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->bChecked);
}

void CBarcodeTypesDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	m_uBarcodeFormats = 0;

	if (m_barcodeMap[IDC_CHECK_BARCODE_ALL]->bChecked)
		m_uBarcodeFormats = m_barcodeMap[IDC_CHECK_BARCODE_ALL]->eBarcodeFormat;

	if (m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->bChecked)
		m_uBarcodeFormats |= m_barcodeMap[IDC_CHECK_BARCODE_LINEAR]->eBarcodeFormat;
	else
	{
		for (UINT lc = IDC_CHECK_BARCODE_CODABAR; lc <= IDC_CHECK_BARCODE_UPCE; lc++)
		{
			m_uBarcodeFormats |= IsChecked(lc) ? m_barcodeMap[lc]->eBarcodeFormat : 0;
		}
	}

	if (m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->bChecked)
		m_uBarcodeFormats |= m_barcodeMap[IDC_CHECK_BARCODE_MATRIX]->eBarcodeFormat;
	else
	{
		for (UINT mc = IDC_CHECK_BARCODE_AZTEC; mc <= IDC_CHECK_BARCODE_RM_QR_CODE; mc++)
			m_uBarcodeFormats |= IsChecked(mc) ? m_barcodeMap[mc]->eBarcodeFormat : 0;
	}

	CDialogEx::OnOK();
}
