#pragma once

#define WM_ON_PRINT_ANALYSIS_CHANGE_OPTS (WM_USER + 11)

// CPrintAnalysisOptsDlg dialog

class CPrintAnalysisOptsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPrintAnalysisOptsDlg)

public:
	CPrintAnalysisOptsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPrintAnalysisOptsDlg();

	void SetPrintAnalysisOpts(int nAnalysisType, int nGrayscaleType, int nBlackoutType, BOOL bConnectValues, int nAoiHeight, int nAoiPartitions);
	void SavePrintAnalysisOpts();

	int GetAnalysisType();
	int GetGrayscaleType();
	int GetBlackoutType();
	BOOL GetConnectValues();
	int GetAoiHeight();
	int GetAoiPartitions();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PRINT_ANALYSIS_OPTS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

protected:
	int m_nAnalysisType;
	int m_nGrayscaleType;
	int m_nBlackoutType;
	BOOL m_bConnectValues;
	int m_nAoiHeight;
	int m_nAoiPartitions;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonApply();
};
