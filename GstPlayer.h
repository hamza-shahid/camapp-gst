#pragma once

#include <gst/gst.h>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>


#define WM_ON_RESIZE_WINDOW (WM_USER + 1)
#define WM_ON_OPENGL_WINDOW_CLOSE (WM_USER + 2)
#define WM_ON_PRINT_ANALYSIS_NOT_FOUND (WM_USER + 3)

typedef std::vector< std::string > StringList;
typedef std::pair< int, int > Fraction;
typedef std::shared_ptr< Fraction > FractionPtr;
typedef std::vector< FractionPtr > FramerateList;
typedef std::shared_ptr< FramerateList > FramerateListPtr;
typedef std::unordered_map<int, std::string> GstResource;

struct Resolution
{
	int m_iWidth;
	int m_iHeight;

	FramerateListPtr m_framerates;
};

typedef std::shared_ptr< Resolution > ResolutionPtr;
typedef std::vector< ResolutionPtr > ResolutionList;
typedef std::shared_ptr< ResolutionList > ResolutionListPtr;

struct DeviceCaps
{
	std::string			m_strMediaType;
	std::string			m_strFormat;
	ResolutionListPtr	m_resolutions;
};

typedef std::shared_ptr< DeviceCaps > DeviceCapsPtr;
typedef std::vector< DeviceCapsPtr > DeviceCapsList;
typedef std::shared_ptr< DeviceCapsList > DeviceCapsListPtr;

typedef std::vector< GstElement* > GstElementList;


class CGstPlayer
{
public:
	CGstPlayer();
	~CGstPlayer();

	BOOL Init(HWND hParent);
	BOOL StartPreview(std::string strSource, int iDeviceIndex, std::string strSink, std::string strMediaType, int iWidth, int iHeight, FractionPtr framerate, BOOL bShowFps, std::string& strError, HWND hVideoWindow);
	void StopPreview();
	void GetCurrentResolution(int &iWidth, int &iHeight);
	void Redraw();

	HANDLE GetDisplayWindowHandle() { return m_hVideoWindow; }

	std::string GetDeviceName(int iDeviceIdx);
	StringList GetDeviceNames();
	DeviceCapsListPtr GetDeviceCaps(std::string strSource, int iDeviceIndex);
	HWND GetParentHwnd() { return m_hParent; }
	void SetPrintAnalysisOpts(int nAnalysisType, int nGrayscaleType, int nBlackoutType, BOOL bConnectValues, int nAoiHeight, int nAoiPartitions);

private:
	GstElement* AddElement(std::string strFactoryName, std::string name, std::string strPropertyName, void* pvProperty, GstElement** pElementOut, std::string& strError);
	BOOL CameraExists(std::string strCameraName);
	void GstreamerPipelineRun();
	void MonitorOpenGlWindow();
	void AddResolutionToFormat(DeviceCapsPtr pDeviceCaps, int iWidth, int iHeight, int iFramerateNum, int iFramerateDen);
	void AddFramerateToResolution(ResolutionPtr pResolution, int iFramerateNum, int iFramerateDen);
	DeviceCapsPtr ParseCapsStr(std::string strCaps);
	void SetPrintAnalysisElementOpts();

	static void OnElementAddedToPipeline(GstBin* pBin, GstElement* pElement, gpointer pUserData);
	static LRESULT CALLBACK NewOpenGlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hParent;
	GList* m_pDevices;
	GstElement* m_pPipeline;
	GstElement* m_pSource;
	GstElement* m_pSink;
	GstElement* m_pPrintAnalysis;
	std::string m_strSink;
	HWND		m_hVideoWindow;
	BOOL		m_bPrintAnalysisFilter;
	
	static WNDPROC	m_origOpenGlWndProc;
	
	std::vector< GstDevice* >	m_CameraList;
	GstElementList				m_elementList;

	std::thread m_tGstMainLoopThread;
	std::thread m_tOpenGlMonitorThread;

	int m_nAnalysisType;
	int m_nGrayscaleType;
	int m_nBlackoutType;
	BOOL m_bConnectValues;
	int m_nAoiHeight;
	int m_nAoiPartitions;
};

extern GstResource g_gstSources;
extern GstResource g_gstSinks;
