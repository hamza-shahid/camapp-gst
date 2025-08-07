#pragma once

#include <string>
#include <unordered_map>
#include <thread>

#include "BarcodeFormat.h"


#define WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY	(WM_USER + 21)
#define WM_TAKE_SNAPSHOT						(WM_USER + 22)
#define WM_BARCODE_SCAN_REG						(WM_USER + 23)
#define WM_START_STOP_PREVIEW_REG				(WM_USER + 24)

enum RegNotPolicy
{
	ON_UPDATE,
	ON_INTERVAL
};

struct RegFlagManager
{
	CString			strRegFlagName;
	std::thread		regFlagMonitorThread;
	int				nEditBoxId;
	UINT			uiOnChangeMsg;
	DWORD			dwDefaultValue;
	DWORD			dwPrevValue;
	RegNotPolicy	eNotPolicy;
};

typedef std::shared_ptr<RegFlagManager>	RegFlagManagerPtr;

struct RegAppSettings
{
	BOOL			bAutoStart;
	std::string		strCameraName;
	std::string		strSource;
	std::string		strSink;
	std::string		strMediaType;
	std::string		strFormat;
	int				iWidth;
	int				iHeight;
	int				iFramerateNum;
	int				iFramerateDen;
};

typedef std::shared_ptr<RegAppSettings>	RegAppSettingsPtr;

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
	LRESULT OpenRegFlag(std::string flagName, DWORD dwDefaultVal, CRegKey& regKey);
	BOOL SetRegFlag(std::string flagName);
	LONG CheckRegFlag(std::string flagName, BOOL& bFlag);
	LSTATUS ReadString(std::string strKeyName, std::string& strKeyValue);

	void Monitor(RegFlagManagerPtr pRegFlagManager);
	//void Monitor();

public:
	void WritePrintPartitionsResultsToReg(const char* pJsonStr);
	const char* ReadPrintPartitionsFromReg();

	void SetSnapshotFlag();
	LRESULT GetSnapshotDir(std::string& strSnapshotDir);

	BOOL GetAppSettings(RegAppSettings& appSettings);
	BOOL SaveAppSettings(RegAppSettings appSettings);

	void WriteBarcodesToReg(BarcodeList* pBarcodeList);

	void StartMonitoring(CFrameWndEx* pParent);
	void StopMonitoring();

protected:
	std::unordered_map<std::string, HKEY>		m_mapParentKeys;
	std::unordered_map<int, RegFlagManagerPtr>	m_regFlags;

	CComboBox	m_comboParentKey;
	CString		m_strParentKey;
	CString		m_strSubKey;
	//CString		m_strAoiRegFlagName;
	//CString		m_strSnapshotRegFlagName;
	CString		m_strSnapshotDirRegKeyName;

	//CString		m_strStartStopRegFlagName;

	//CString		m_strBarcodeRegFlagName;
	BOOL		m_bBarcodeMsgSent;

	CFrameWndEx*	m_pParent;

	//std::thread			timerThread;
	std::atomic<bool>	bMonitor;

	std::thread	m_aoiRegFlagMonitorThread;
	std::thread	m_snapshotRegFlagMonitorThread;
	std::thread	m_barcodeRegFlagMonitorThread;
	std::thread	m_startStopRegFlagMonitorThread;

	DECLARE_MESSAGE_MAP()
};
