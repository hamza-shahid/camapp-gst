#pragma once

#include <unordered_map>
#include "BarcodeFormat.h"


struct BarcodePair
{
	BarcodeFormat eBarcodeFormat;
	BOOL bChecked;
};

typedef std::shared_ptr<BarcodePair> BarcodePairPtr;
typedef std::unordered_map<UINT, BarcodePairPtr> BarcodeMap;

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

	void SetBarcodeFormats(UINT uBarcodeFormats);
	void SetBarcodeColStartX(UINT uBarcodeColStartX);
	void SetBarcodeColWidth(UINT uBarcodeColWidth);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BARCODE_TYPES };
#endif

private:
	void EnableLinearCodes(bool bEnable = TRUE);
	void EnableMatrixCodes(bool bEnable);

	void InitBarcodes();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

protected:
	UINT m_uBarcodeFormats;
	BarcodeMap m_barcodeMap;
	unsigned int m_uBarcodeColStartX;
	unsigned int m_uBarcodeColWidth;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckBarcodeAll();
	afx_msg void OnBnClickedCheckBarcodeLinear();
	afx_msg void OnBnClickedCheckBarcodeMatrix();
	afx_msg void OnBnClickedOk();
};
