#pragma once

#include <string>
#include <unordered_map>
#include <thread>

#include "BarcodeFormat.h"


#define WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY	(WM_USER + 21)
#define WM_TAKE_SNAPSHOT						(WM_USER + 22)
#define WM_BARCODE_SCAN_REG						(WM_USER + 23)


// CRegistrySettings dialog

class CRegistryManager : public CDialogEx
{
	DECLARE_DYNAMIC(CRegistryManager)

public:
	CRegistryManager(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRegistryManager();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REG_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	HKEY GetParentKey();
	BOOL SetRegFlag(std::string flagName);
	LONG CheckRegFlag(std::string flagName, BOOL& bFlag);
	LSTATUS ReadString(std::string strKeyName, std::string& strKeyValue);

	void Monitor();

public:
	void WritePrintPartitionsResultsToReg(const char* pJsonStr);
	const char* ReadPrintPartitionsFromReg();

	void SetSnapshotFlag();

	void WriteBarcodesToReg(BarcodeList* pBarcodeList);

	void StartMonitoring(CFrameWndEx* pParent);
	void StopMonitoring();

protected:
	std::unordered_map<std::string, HKEY> m_mapParentKeys;

	CComboBox	m_comboParentKey;
	CString		m_strParentKey;
	CString		m_strSubKey;
	CString		m_strAoiRegFlagName;
	CString		m_strSnapshotRegFlagName;
	CString		m_strSnapshotDirRegKeyName;

	CString		m_strBarcodeRegFlagName;
	BOOL		m_bBarcodeMsgSent;

	CFrameWndEx*	m_pParent;

	std::thread			timerThread;
	std::atomic<bool>	bMonitor;

	DECLARE_MESSAGE_MAP()
};
