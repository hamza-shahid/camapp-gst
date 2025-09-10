// RegistrySettings.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "RegistryManager.h"
#include "afxdialogex.h"

#include <cjson/cJSON.h>
#include <chrono>


#define REG_HKEY				HKEY_LOCAL_MACHINE
#define REG_SUB_KEY				"Software\\Neuralog\\NeuraQService\\VIS"
#define REG_AOI_KEY_NAME		"ready"
#define REG_SNAPSHOT_KEY_NAME	"snapshot"
#define REG_SNAPSHOT_DIR_NAME	"snapshot-dir"
#define REG_START_STOP_PREVIEW	"start-stop"
#define REG_CAMERA_NAME_VALUE	"CameraName"
#define REG_SOURCE_VALUE		"Source"
#define REG_SINK_VALUE			"Sink"
#define REG_AUTO_START			"AutoStart"
#define REG_MEDIA_TYPE			"MediaType"
#define REG_FORMAT				"Format"
#define REG_WIDTH				"Width"
#define REG_HEIGHT				"Height"
#define REG_FRAMERATE_NUM		"FramerateNum"
#define REG_FRAMERATE_DEN		"FramerateDen"
#define REG_BARCODE_FORMATS		"BarcodeFormats"
#define REG_BARCODE_COL_START_X	"BarcodeColumnStartX"
#define REG_BARCODE_COL_WIDTH	"BarcodeColumnWidth"
#define REG_BARCODE_ENABLED		"BarcodeEnabled"


using namespace std::chrono_literals;

// CRegistrySettings dialog

IMPLEMENT_DYNAMIC(CRegistryManager, CDialogEx)

void CRegistryManager::NewRegFlagManager(
	CString strRegKeyValueName,
	int nEditBoxId,
	UINT uOnChangeMsg,
	DWORD dwDefaultVal,
	DWORD dwPrevVal,
	RegNotPolicy eNotifyPolicy,
	std::chrono::milliseconds waitInterval
)
{
	m_regFlags[nEditBoxId] = std::make_shared<RegFlagManager>();
	m_regFlags[nEditBoxId]->strRegFlagName = strRegKeyValueName;
	m_regFlags[nEditBoxId]->nEditBoxId = nEditBoxId;
	m_regFlags[nEditBoxId]->uiOnChangeMsg = uOnChangeMsg;
	m_regFlags[nEditBoxId]->dwDefaultValue = dwDefaultVal;
	m_regFlags[nEditBoxId]->dwPrevValue = dwPrevVal;
	m_regFlags[nEditBoxId]->eNotPolicy = eNotifyPolicy;
	m_regFlags[nEditBoxId]->waitInterval = waitInterval;
}

CRegistryManager::CRegistryManager(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REG_SETTINGS, pParent)
	, m_pParent(NULL)
{
	m_mapParentKeys["HKEY_CLASSES_ROOT"]	= HKEY_CLASSES_ROOT;
	m_mapParentKeys["HKEY_CURRENT_USER"]	= HKEY_CURRENT_USER;
	m_mapParentKeys["HKEY_LOCAL_MACHINE"]	= HKEY_LOCAL_MACHINE;
	m_mapParentKeys["HKEY_USERS"]			= HKEY_USERS;
	m_mapParentKeys["HKEY_CURRENT_CONFIG"]	= HKEY_CURRENT_CONFIG;

	m_strParentKey				= "HKEY_LOCAL_MACHINE";
	m_strSubKey					= REG_SUB_KEY;
	m_strSnapshotDirRegKeyName	= REG_SNAPSHOT_DIR_NAME;

	NewRegFlagManager(REG_AOI_KEY_NAME, IDC_EDIT_REG_AOI_FLAG, WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY, 1, 1, ON_INTERVAL, 1000ms);
	NewRegFlagManager(REG_SNAPSHOT_KEY_NAME, IDC_EDIT_REG_SNAPSHOT_FLAG, WM_TAKE_SNAPSHOT, 1, 1, ON_INTERVAL, 2000ms);
	NewRegFlagManager(REG_BARCODE_ENABLED, IDC_EDIT_REG_BARCODE_FLAG, WM_BARCODE_SCAN_REG, 0, 0, ON_UPDATE, 1000ms);
	NewRegFlagManager(REG_START_STOP_PREVIEW, IDC_EDIT_REG_START_STOP_FLAG, WM_START_STOP_PREVIEW_REG, 0, 0, ON_UPDATE, 1000ms);
}

CRegistryManager::~CRegistryManager()
{

}

void CRegistryManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	for (const auto& regFlagManager : m_regFlags)
		DDX_Text(pDX, regFlagManager.first, regFlagManager.second->strRegFlagName);

	DDX_Text(pDX, IDC_EDIT_REG_SUB_KEY, m_strSubKey);
	DDX_Text(pDX, IDC_EDIT_REG_SNAPSHOT_DIR, m_strSnapshotDirRegKeyName);
	DDX_Control(pDX, IDC_COMBO_REG_AOI_PARENT_KEY, m_comboParentKey);
}

BOOL CRegistryManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (const auto& pairParentKey : m_mapParentKeys)
	{
		int nIdx = m_comboParentKey.InsertString(-1, pairParentKey.first.c_str());
		m_comboParentKey.SetItemData(nIdx, (DWORD_PTR)pairParentKey.second);
	}

	m_comboParentKey.SetCurSel(2);

	return TRUE;
}

HKEY CRegistryManager::GetParentKey()
{
	return m_mapParentKeys[m_strParentKey.GetBuffer()];
}

LRESULT CRegistryManager::OpenRegFlag(std::string flagName, DWORD dwDefaultVal, CRegKey &regKey)
{
	LRESULT lResult;

	lResult = regKey.Create(GetParentKey(), m_strSubKey.GetBuffer());

	if (lResult == ERROR_SUCCESS)
	{
		DWORD dwValue;
		
		lResult = regKey.QueryDWORDValue(flagName.c_str(), dwValue);
		
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			lResult = regKey.SetDWORDValue(flagName.c_str(), dwDefaultVal);
		}
	}

	return lResult;
}

LRESULT CRegistryManager::GetSnapshotDir(std::string& strSnapshotDir)
{
	CRegKey regKey;

	LSTATUS lStatus = regKey.Open(GetParentKey(), m_strSubKey.GetBuffer());
	if (lStatus == ERROR_SUCCESS)
	{
		char pszDir[512];
		ULONG unDirSize = sizeof(pszDir);

		lStatus = regKey.QueryStringValue(m_strSnapshotDirRegKeyName.GetBuffer(), pszDir, &unDirSize);

		if (lStatus == ERROR_SUCCESS)
			strSnapshotDir = pszDir;

		regKey.Close();
	}

	return lStatus;
}

BOOL CRegistryManager::SetRegFlag(std::string flagName)
{
	CRegKey regKey;

	LSTATUS lStatus = regKey.Open(GetParentKey(), m_strSubKey.GetBuffer());
	if (lStatus == ERROR_SUCCESS)
	{
		lStatus = regKey.SetDWORDValue(flagName.c_str(), TRUE);
		regKey.Close();
	}

	return lStatus == ERROR_SUCCESS ? TRUE : FALSE;
}

LONG CRegistryManager::CheckRegFlag(std::string flagName, BOOL& bFlag)
{
	CRegKey regKey;

	LSTATUS lStatus = regKey.Open(GetParentKey(), m_strSubKey.GetBuffer());
	if (lStatus == ERROR_SUCCESS)
	{
		lStatus = regKey.QueryDWORDValue(flagName.c_str(), (DWORD&)bFlag);
		regKey.Close();
	}

	return lStatus;
}

LSTATUS CRegistryManager::ReadString(std::string strKeyName, std::string &strKeyValue)
{
	CRegKey regKey;

	LSTATUS lStatus = regKey.Open(GetParentKey(), m_strSubKey.GetBuffer());
	if (lStatus == ERROR_SUCCESS)
	{
		char pKeyValue[512];
		ULONG nSize = sizeof(pKeyValue);

		lStatus = regKey.QueryStringValue(strKeyName.c_str(), pKeyValue, &nSize);

		if (lStatus == ERROR_SUCCESS)
			strKeyValue = pKeyValue;

		regKey.Close();
	}

	return lStatus;
}

void CRegistryManager::WritePrintPartitionsResultsToReg(const char* pJsonStr)
{
	cJSON* pJson = cJSON_Parse(pJsonStr);
	if (pJson == NULL)
		return;

	// Get "partitions" array
	cJSON* pPartitions = cJSON_GetObjectItem(pJson, "partitions");
	if (!cJSON_IsArray(pPartitions))
	{
		cJSON_Delete(pJson);
		return;
	}

	// Iterate through the array
	int nPartitions = cJSON_GetArraySize(pPartitions);

	for (int i = 0; i < nPartitions; i++)
	{
		LSTATUS lResult;
		CRegKey regKey;
		cJSON* partition = cJSON_GetArrayItem(pPartitions, i);

		// Extract values from each partition
		int id = cJSON_GetObjectItem(partition, "id")->valueint;
		std::string totalStr = cJSON_GetObjectItem(partition, "total")->valuestring;
		std::string averageStr = cJSON_GetObjectItem(partition, "average")->valuestring;
		std::string averageColStr = cJSON_GetObjectItem(partition, "average_col")->valuestring;
		std::string minStr = cJSON_GetObjectItem(partition, "min")->valuestring;
		std::string maxStr = cJSON_GetObjectItem(partition, "max")->valuestring;
		std::string nonUniformityStr = cJSON_GetObjectItem(partition, "non-uniformity")->valuestring;
		std::string minSatStr = cJSON_GetObjectItem(partition, "saturation_min")->valuestring;
		std::string maxSatStr = cJSON_GetObjectItem(partition, "saturation_max")->valuestring;
		std::string avgSatStr = cJSON_GetObjectItem(partition, "saturation_avg")->valuestring;

		std::string subKey = m_strSubKey.GetBuffer() + std::string("\\") + std::to_string(id);

		lResult = regKey.Open(GetParentKey(), subKey.c_str(), KEY_WRITE);

		if (lResult == ERROR_SUCCESS)
		{
			regKey.SetStringValue("total", totalStr.c_str());
			regKey.SetStringValue("average", averageStr.c_str());
			regKey.SetStringValue("average_col", averageColStr.c_str());
			regKey.SetStringValue("min", minStr.c_str());
			regKey.SetStringValue("max", maxStr.c_str());
			regKey.SetStringValue("non-uniformity", nonUniformityStr.c_str());
			regKey.SetStringValue("saturation_min", minSatStr.c_str());
			regKey.SetStringValue("saturation_max", maxSatStr.c_str());
			regKey.SetStringValue("saturation_avg", avgSatStr.c_str());

			regKey.Close();
		}
	}

	// Clean up
	cJSON_Delete(pJson);
	SetRegFlag(m_regFlags[IDC_EDIT_REG_AOI_FLAG]->strRegFlagName.GetBuffer());
}

const char* CRegistryManager::ReadPrintPartitionsFromReg()
{
	char* pJsonStr = NULL;
	int idx = 0;
	BOOL bReady = TRUE;		// true means it's not ready
	LONG lResult = CheckRegFlag(m_regFlags[IDC_EDIT_REG_AOI_FLAG]->strRegFlagName.GetBuffer(), bReady);

	cJSON* root = cJSON_CreateObject();

	if (!root)
		return NULL;

	// Create an array for partitions
	cJSON* pPartitions = cJSON_CreateArray();
	if (!pPartitions)
	{
		cJSON_Delete(root);
		return NULL;
	}

	// Add partitions array to the root object
	cJSON_AddItemToObject(root, "partitions", pPartitions);

	do
	{
		std::string subKey = m_strSubKey.GetBuffer() + std::string("\\") + std::to_string(idx);
		char pCenterStr[64], pDimensionsStr[64], pBackgroundStr[64];
		ULONG ulCenter = sizeof(pCenterStr);
		ULONG ulDimensions = sizeof(pDimensionsStr);
		ULONG ulBackground = sizeof(pBackgroundStr);
		int x, y, width, height, bg_r, bg_g, bg_b;
		DWORD dwId;
		CRegKey regKey;

		lResult = regKey.Open(GetParentKey(), subKey.c_str(), KEY_READ);

		if (lResult == ERROR_SUCCESS)
		{
			lResult = regKey.QueryDWORDValue("id", dwId);
			if (lResult != ERROR_SUCCESS)
				break;

			lResult = regKey.QueryStringValue("center", pCenterStr, &ulCenter);
			if (lResult != ERROR_SUCCESS)
				break;

			lResult = regKey.QueryStringValue("dimensions", pDimensionsStr, &ulDimensions);
			if (lResult != ERROR_SUCCESS)
				break;

			lResult = regKey.QueryStringValue("bg", pBackgroundStr, &ulBackground);
			if (lResult == ERROR_SUCCESS)
				sscanf_s(pBackgroundStr, "%d,%d,%d", &bg_r, &bg_g, &bg_b);
			else
			{
				bg_r = bg_g = bg_b = 255;
			}
			
			lResult = regKey.Close();

			sscanf_s(pCenterStr, "%d,%d", &x, &y);
			sscanf_s(pDimensionsStr, "%dx%d", &width, &height);

			cJSON* pPartition = cJSON_CreateObject();
			if (!pPartition)
				continue;

			// Add fields to the partition object
			cJSON_AddNumberToObject(pPartition, "id", dwId);
			cJSON_AddNumberToObject(pPartition, "center_x", x);
			cJSON_AddNumberToObject(pPartition, "center_y", y);
			cJSON_AddNumberToObject(pPartition, "width", width);
			cJSON_AddNumberToObject(pPartition, "height", height);
			cJSON_AddNumberToObject(pPartition, "bg_r", bg_r);
			cJSON_AddNumberToObject(pPartition, "bg_g", bg_g);
			cJSON_AddNumberToObject(pPartition, "bg_b", bg_b);

			// Add the partition to the array
			cJSON_AddItemToArray(pPartitions, pPartition);
		}

		idx++;
	} while (lResult == ERROR_SUCCESS);

	// Convert the root JSON object to a string
	pJsonStr = cJSON_Print(root);
	cJSON_Delete(root);

	return pJsonStr;
}

void CRegistryManager::StartMonitoring(CFrameWndEx* pParent)
{
	m_pParent = pParent;

	m_bMonitor = true;

	for (const auto& regFlagEntry : m_regFlags)
		regFlagEntry.second->regFlagMonitorThread = std::thread(&CRegistryManager::Monitor, this, regFlagEntry.second);
}

void CRegistryManager::StopMonitoring()
{
	m_bMonitor = false;
	
	for (const auto& regFlagEntry : m_regFlags)
	{
		if (regFlagEntry.second->regFlagMonitorThread.joinable())
			regFlagEntry.second->regFlagMonitorThread.join();
	}
}

void CRegistryManager::Monitor(RegFlagManagerPtr pRegFlagManager)
{
	CRegKey regKey;
	BOOL bExiting = FALSE;

	LSTATUS lStatus = OpenRegFlag(pRegFlagManager->strRegFlagName.GetBuffer(), pRegFlagManager->dwDefaultValue, regKey);
	if (lStatus == ERROR_SUCCESS)
	{
		while (m_bMonitor)
		{
			DWORD dwValue;

			lStatus = regKey.QueryDWORDValue(pRegFlagManager->strRegFlagName.GetBuffer(), dwValue);

			if (lStatus == ERROR_SUCCESS)
			{
				if ((pRegFlagManager->eNotPolicy == ON_UPDATE && dwValue != pRegFlagManager->dwPrevValue) || pRegFlagManager->eNotPolicy == ON_INTERVAL)
				{
					m_pParent->PostMessage(pRegFlagManager->uiOnChangeMsg, 0, dwValue);
					pRegFlagManager->dwPrevValue = dwValue;
				}
			}

			std::this_thread::sleep_for(pRegFlagManager->waitInterval);
		}
		regKey.Close();
	}
	else
	{
		bExiting = TRUE;
	}
}

void CRegistryManager::SetSnapshotFlag()
{
	SetRegFlag(m_regFlags[IDC_EDIT_REG_SNAPSHOT_FLAG]->strRegFlagName.GetBuffer());
}

void CRegistryManager::WriteBarcodesToReg(BarcodeList* pBarcodeList)
{
	CRegKey regKey;
	LSTATUS lStatus = regKey.Open(GetParentKey(), m_strSubKey.GetBuffer());
	
	if (lStatus == ERROR_SUCCESS)
	{
		lStatus = regKey.RecurseDeleteKey("Barcodes");
		
		if (lStatus == ERROR_SUCCESS || lStatus == ERROR_FILE_NOT_FOUND)
		{
			// delete Barcodes subkey
			regKey.Close();

			int nBarcodeIdx = 0;
			std::string subKey = m_strSubKey.GetBuffer() + std::string("\\Barcodes\\");

			for (BarcodeInfoPtr pBarcodeInfo : *pBarcodeList)
			{
				std::string subSubKey = subKey + std::to_string(nBarcodeIdx);

				lStatus = regKey.Create(GetParentKey(), subSubKey.c_str());

				if (lStatus == ERROR_SUCCESS)
				{
					regKey.SetStringValue("Barcode", pBarcodeInfo->barcode.c_str());
					regKey.SetStringValue("Format", pBarcodeInfo->format.c_str());
				}
				nBarcodeIdx++;
			}

			SetRegFlag(m_regFlags[IDC_EDIT_REG_BARCODE_FLAG]->strRegFlagName.GetBuffer());
		}
	}
}

LSTATUS CRegistryManager::RegQueryDWORDValue(CRegKey& regKey, LPCTSTR pszValueName, DWORD& dwValue, DWORD dwDefaultValue)
{
	LSTATUS lStatus = regKey.QueryDWORDValue(pszValueName, dwValue);
	if (lStatus != ERROR_SUCCESS)
		dwValue = dwDefaultValue;

	return lStatus;
}

LSTATUS CRegistryManager::RegQueryStringValue(CRegKey& regKey, LPCTSTR pszValueName, std::string& strValue)
{
	char pszRegStr[512];
	ULONG unRegStrSize = sizeof(pszRegStr);

	LSTATUS lStatus = regKey.QueryStringValue(pszValueName, pszRegStr, &unRegStrSize);
	if (lStatus == ERROR_SUCCESS)
		strValue = pszRegStr;

	return lStatus;
}

BOOL CRegistryManager::GetAppSettings(RegAppSettings& appSettings)
{
	CRegKey regKey;
	LSTATUS lStatus = regKey.Create(GetParentKey(), m_strSubKey.GetBuffer());

	if (lStatus != ERROR_SUCCESS)
		return FALSE;

	/* General Settings */
	lStatus = RegQueryDWORDValue(regKey, REG_AUTO_START, (DWORD&)appSettings.bAutoStart, FALSE);
	lStatus = RegQueryStringValue(regKey, REG_CAMERA_NAME_VALUE, appSettings.strCameraName);
	lStatus = RegQueryStringValue(regKey, REG_SOURCE_VALUE, appSettings.strSource);
	lStatus = RegQueryStringValue(regKey, REG_SINK_VALUE, appSettings.strSink);

	/* Media format Settings */
	lStatus = RegQueryStringValue(regKey, REG_MEDIA_TYPE, appSettings.strMediaType);
	lStatus = RegQueryStringValue(regKey, REG_FORMAT, appSettings.strFormat);
	lStatus = RegQueryDWORDValue(regKey, REG_WIDTH, (DWORD&)appSettings.iWidth, -1);
	lStatus = RegQueryDWORDValue(regKey, REG_HEIGHT, (DWORD&)appSettings.iHeight, -1);
	lStatus = RegQueryDWORDValue(regKey, REG_FRAMERATE_NUM, (DWORD&)appSettings.iFramerateNum, -1);
	lStatus = RegQueryDWORDValue(regKey, REG_FRAMERATE_DEN, (DWORD&)appSettings.iFramerateDen, -1);
	
	/* Barcode Settings */
	lStatus = RegQueryDWORDValue(regKey, REG_BARCODE_FORMATS, (DWORD&)appSettings.uBarcodeFormats, 0xFFFFFFFFu);
	lStatus = RegQueryDWORDValue(regKey, REG_BARCODE_COL_START_X, (DWORD&)appSettings.uBarcodeColStartX, 0);
	lStatus = RegQueryDWORDValue(regKey, REG_BARCODE_COL_WIDTH, (DWORD&)appSettings.uBarcodeColWidth, 0);
	lStatus = RegQueryDWORDValue(regKey, REG_BARCODE_ENABLED, (DWORD&)appSettings.bBarcodeEnabled, FALSE);

	regKey.Close();

	return TRUE;
}

BOOL CRegistryManager::SaveAppSettings(RegAppSettings appSettings)
{
	CRegKey regKey;
	LSTATUS lStatus = regKey.Create(GetParentKey(), m_strSubKey.GetBuffer());

	if (lStatus != ERROR_SUCCESS)
		return FALSE;

	lStatus = regKey.SetDWORDValue(REG_AUTO_START, appSettings.bAutoStart);
	lStatus = regKey.SetStringValue(REG_CAMERA_NAME_VALUE, appSettings.strCameraName.c_str());
	lStatus = regKey.SetStringValue(REG_SOURCE_VALUE, appSettings.strSource.c_str());
	lStatus = regKey.SetStringValue(REG_SINK_VALUE, appSettings.strSink.c_str());
	lStatus = regKey.SetStringValue(REG_MEDIA_TYPE, appSettings.strMediaType.c_str());
	lStatus = regKey.SetStringValue(REG_FORMAT, appSettings.strFormat.c_str());
	lStatus = regKey.SetDWORDValue(REG_WIDTH, appSettings.iWidth);
	lStatus = regKey.SetDWORDValue(REG_HEIGHT, appSettings.iHeight);
	lStatus = regKey.SetDWORDValue(REG_FRAMERATE_NUM, appSettings.iFramerateNum);
	lStatus = regKey.SetDWORDValue(REG_FRAMERATE_DEN, appSettings.iFramerateDen);
	lStatus = regKey.SetDWORDValue(REG_BARCODE_FORMATS, appSettings.uBarcodeFormats);
	lStatus = regKey.SetDWORDValue(REG_BARCODE_COL_START_X, appSettings.uBarcodeColStartX);
	lStatus = regKey.SetDWORDValue(REG_BARCODE_COL_WIDTH, appSettings.uBarcodeColWidth);
	lStatus = regKey.SetDWORDValue(REG_BARCODE_ENABLED, appSettings.bBarcodeEnabled);

	regKey.Close();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CRegistryManager, CDialogEx)
END_MESSAGE_MAP()


// CRegistrySettings message handlers
