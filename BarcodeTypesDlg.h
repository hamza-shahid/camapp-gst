#pragma once

#include <unordered_map>

typedef std::unordered_map<UINT, UINT> BarcodeMap;

// CBarcodeTypesDlg dialog

class CBarcodeTypesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBarcodeTypesDlg)

public:
	CBarcodeTypesDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBarcodeTypesDlg();

	UINT GetBarcodeFormats();
	UINT GetBarcodeColumnStartX();
	UINT GetBarcodeColumnWidth();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BARCODE_TYPES };
#endif

private:
	void EnableLinearCodes(bool bEnable = TRUE);
	void EnableMatrixCodes(bool bEnable);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

protected:
	UINT m_uBarcodeFormats;
	BOOL m_bAllCodesEnabled;
	BOOL m_bLinearCodeEnabled;
	BOOL m_bMatrixCodeEnabled;
	BOOL m_bAztecEnabled;
	BOOL m_bCodabarEnabled;
	BOOL m_bCode39Enabled;
	BOOL m_bCode93Enabled;
	BOOL m_bCode128Enabled;
	BOOL m_bDataBarEnabled;
	BOOL m_bDataBarExpandedEnabled;
	BOOL m_bDataMatrixEnabled;
	BOOL m_bEAN8Enabled;
	BOOL m_bEAN13Enabled;
	BOOL m_bITFEnabled;
	BOOL m_bMaxiCodeEnabled;
	BOOL m_bPDF417Enabled;
	BOOL m_bQRCode;
	BOOL m_bUPCAEnabled;
	BOOL m_bUPCEEnabled;
	BOOL m_bMicroQRCodeEnabled;
	BOOL m_bRMQRCodeEnabled;
	BOOL m_bDXFilmEdgeEnabled;
	BOOL m_bDataBarLimited;

	BarcodeMap m_barcodeMap;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckBarcodeAll();
	afx_msg void OnBnClickedCheckBarcodeLinear();
	afx_msg void OnBnClickedCheckBarcodeMatrix();
	afx_msg void OnBnClickedOk();
protected:
	unsigned int m_barcodeColStartX;
	unsigned int m_barcodeColumnWidth;
};
