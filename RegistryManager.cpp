// RegistrySettings.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "RegistryManager.h"
#include "afxdialogex.h"

#include <cjson/cJSON.h>


#define REG_HKEY				HKEY_LOCAL_MACHINE
#define REG_SUB_KEY				"Software\\Neuralog\\NeuraQService\\VIS"
#define REG_AOI_KEY_NAME		"ready"
#define REG_SNAPSHOT_KEY_NAME	"snapshot"
#define REG_SNAPSHOT_DIR_NAME	"snapshot-dir"
#define REG_BARCODE_KEY_NAME	"barcode"

// CRegistrySettings dialog

IMPLEMENT_DYNAMIC(CRegistryManager, CDialogEx)

CRegistryManager::CRegistryManager(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REG_SETTINGS, pParent)
	, m_bBarcodeMsgSent(FALSE)
{
	m_mapParentKeys["HKEY_CLASSES_ROOT"]	= HKEY_CLASSES_ROOT;
	m_mapParentKeys["HKEY_CURRENT_USER"]	= HKEY_CURRENT_USER;
	m_mapParentKeys["HKEY_LOCAL_MACHINE"]	= HKEY_LOCAL_MACHINE;
	m_mapParentKeys["HKEY_USERS"]			= HKEY_USERS;
	m_mapParentKeys["HKEY_CURRENT_CONFIG"]	= HKEY_CURRENT_CONFIG;

	m_strParentKey				= "HKEY_LOCAL_MACHINE";
	m_strSubKey					= REG_SUB_KEY;
	m_strAoiRegFlagName			= REG_AOI_KEY_NAME;
	m_strSnapshotRegFlagName	= REG_SNAPSHOT_KEY_NAME;
	m_strSnapshotDirRegKeyName	= REG_SNAPSHOT_DIR_NAME;
	m_strBarcodeRegFlagName		= REG_BARCODE_KEY_NAME;
}

CRegistryManager::~CRegistryManager()
{

}

void CRegistryManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_REG_SUB_KEY, m_strSubKey);
	DDX_Text(pDX, IDC_EDIT_REG_AOI_FLAG, m_strAoiRegFlagName);
	DDX_Text(pDX, IDC_EDIT_REG_SNAPSHOT_FLAG, m_strSnapshotRegFlagName);
	DDX_Text(pDX, IDC_EDIT_REG_SNAPSHOT_DIR, m_strSnapshotDirRegKeyName);
	DDX_Text(pDX, IDC_EDIT_REG_BARCODE_FLAG, m_strBarcodeRegFlagName);
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
		std::string minStr = cJSON_GetObjectItem(partition, "min")->valuestring;
		std::string maxStr = cJSON_GetObjectItem(partition, "max")->valuestring;
		std::string nonUniformityStr = cJSON_GetObjectItem(partition, "non-uniformity")->valuestring;

		std::string subKey = m_strSubKey.GetBuffer() + std::string("\\") + std::to_string(id);

		lResult = regKey.Open(GetParentKey(), subKey.c_str(), KEY_WRITE);

		if (lResult == ERROR_SUCCESS)
		{
			regKey.SetStringValue("total", totalStr.c_str());
			regKey.SetStringValue("average", averageStr.c_str());
			regKey.SetStringValue("min", minStr.c_str());
			regKey.SetStringValue("max", maxStr.c_str());
			regKey.SetStringValue("non-uniformity", nonUniformityStr.c_str());

			regKey.Close();
		}
	}

	// Clean up
	cJSON_Delete(pJson);
	SetRegFlag(m_strAoiRegFlagName.GetBuffer());
}

const char* CRegistryManager::ReadPrintPartitionsFromReg()
{
	char* pJsonStr = NULL;
	int idx = 0;
	BOOL bReady = TRUE;		// true means it's not ready
	LONG lResult = CheckRegFlag(m_strAoiRegFlagName.GetBuffer(), bReady);

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
		char pCenterStr[64], pDimensionsStr[64];
		ULONG ulCenter = sizeof(pCenterStr);
		ULONG ulDimensions = sizeof(pDimensionsStr);
		int x, y, width, height;
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

			regKey.Close();

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

	bMonitor = true;
	timerThread = std::thread(&CRegistryManager::Monitor, this);
}

void CRegistryManager::StopMonitoring()
{
	bMonitor = false;
	
	if (timerThread.joinable())
		timerThread.join();
}

void CRegistryManager::Monitor()
{
	while (bMonitor)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		// check for aoi partitions flag
		BOOL bFLAG = TRUE;		// true means it's not ready
		LONG lResult = CheckRegFlag(m_strAoiRegFlagName.GetBuffer(), bFLAG);

		if (lResult == ERROR_SUCCESS && !bFLAG)
		{
			// send message to parent
			m_pParent->PostMessage(WM_PRINT_ANALYSIS_AOI_PARTITIONS_READY);
		}

		// check for snapshot flag
		bFLAG = TRUE;		// true means it's not ready
		lResult = CheckRegFlag(m_strSnapshotRegFlagName.GetBuffer(), bFLAG);

		if (lResult == ERROR_SUCCESS && !bFLAG)
		{
			std::string strSnapshotDir;

			lResult = ReadString(m_strSnapshotDirRegKeyName.GetBuffer(), strSnapshotDir);

			if (lResult == ERROR_SUCCESS)
			{
				lResult = m_pParent->PostMessage(WM_TAKE_SNAPSHOT, 0, (LPARAM)strSnapshotDir.c_str());
			}
		}

		// check for barcode flag
		bFLAG = TRUE;		// true means it's not ready
		lResult = CheckRegFlag(m_strBarcodeRegFlagName.GetBuffer(), bFLAG);

		if (lResult == ERROR_SUCCESS && !bFLAG /* && !m_bBarcodeMsgSent*/)
		{
			m_pParent->PostMessage(WM_BARCODE_SCAN_REG, 0, 0);
			m_bBarcodeMsgSent = TRUE;
		}
	}
}

void CRegistryManager::SetSnapshotFlag()
{
	SetRegFlag(m_strSnapshotRegFlagName.GetBuffer());
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

			SetRegFlag(m_strBarcodeRegFlagName.GetBuffer());
		}
	}
}

BEGIN_MESSAGE_MAP(CRegistryManager, CDialogEx)
END_MESSAGE_MAP()


// CRegistrySettings message handlers
