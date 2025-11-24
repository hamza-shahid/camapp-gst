#pragma once

#include <string>
#include <unordered_map>
#include <thread>

#include "BarcodeFormat.h"


#define WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY	(WM_USER + 21)
#define WM_TAKE_SNAPSHOT						(WM_USER + 22)
#define WM_BARCODE_SCAN_REG						(WM_USER + 23)
#define WM_START_STOP_PREVIEW_REG				(WM_USER + 24)
#define WM_EXPOSURE_REG							(WM_USER + 25)

enum RegNotPolicy
{
	ON_UPDATE,
	ON_INTERVAL
};

struct RegFlagManager
{
	std::string		strRegFlagName;
	std::thread		regFlagMonitorThread;
	UINT			uiOnChangeMsg;
	DWORD			dwDefaultValue;
	DWORD			dwPrevValue;
	RegNotPolicy	eNotPolicy;

	std::chrono::milliseconds waitInterval;
};

typedef std::shared_ptr<RegFlagManager>	RegFlagManagerPtr;

struct RegAppSettings
{
	BOOL			bAutoStart;
	BOOL			bShowFps;
	BOOL			bAutoExposure;
	std::string		strCameraName;
	std::string		strSource;
	std::string		strSink;
	std::string		strMediaType;
	std::string		strFormat;
	int				iWidth;
	int				iHeight;
	int				iFramerateNum;
	int				iFramerateDen;
	unsigned int	uBarcodeFormats;
	unsigned int	uBarcodeColStartX;
	unsigned int	uBarcodeColWidth;
	BOOL			bBarcodeEnabled;
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

	void NewRegFlagManager(
		std::string strRegKeyValueName,
		UINT uOnChangeMsg,
		DWORD dwDefaultVal,
		DWORD dwPrevVal,
		RegNotPolicy eNotifyPolicy,
		std::chrono::milliseconds waitInterval
	);

	static LSTATUS RegQueryDWORDValue(CRegKey& regKey, LPCTSTR pszValueName, DWORD& dwValue, DWORD dwDefaultValue);
	static LSTATUS RegQueryStringValue(CRegKey& regKey, LPCTSTR pszValueName, std::string& strValue);

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

	BOOL StartMinimized();

protected:
	std::unordered_map<std::string, HKEY>		m_mapParentKeys;
	std::unordered_map<std::string, RegFlagManagerPtr>	m_regFlags;

	CComboBox	m_comboParentKey;
	CString		m_strParentKey;
	CString		m_strSubKey;

	CFrameWndEx*		m_pParent;
	std::atomic<bool>	m_bMonitor;

	DECLARE_MESSAGE_MAP()
};
