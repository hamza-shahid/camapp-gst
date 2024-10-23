#include "pch.h"
#include "GstPlayer.h"
#include "resource.h"

#include <gst/video/videooverlay.h>
#include <functional>
#include <sstream>
#include <algorithm>



WNDPROC CGstPlayer::m_origOpenGlWndProc = NULL;

GstResource g_gstSources = {
	{ ID_SOURCE_KERNELSTREAMING, "ksvideosrc" },
	{ ID_SOURCE_MEDIAFOUNDATION, "mfvideosrc" },
	{ ID_SOURCE_DIRECTSHOW, "dshowvideosrc" }
};

GstResource g_gstSinks = {
	{ ID_SINK_AUTO, "autovideosink" },
	{ ID_SINK_DIRECT3D11, "d3d11videosink" },
	{ ID_SINK_DIRECT3D9, "d3dvideosink" },
	{ ID_SINK_OPENGL, "glimagesink" }
};

std::string GetExecutableFolderPath()
{
	char path[MAX_PATH];
	std::string fullPath;

	if (GetModuleFileName(NULL, path, MAX_PATH)) 
	{
		fullPath = path;
		size_t pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos) {
			return fullPath.substr(0, pos);
		}
	}
	return "";
}

CGstPlayer::CGstPlayer()
{
	int argc = 0;
	char** argv = NULL;

	gst_init(&argc, &argv);

	m_pDevices = NULL;
	m_pPipeline = NULL;
	m_pSource = NULL;
	m_pSink = NULL;
	m_pPrintAnalysis = NULL;
	m_bPrintAnalysisFilter = FALSE;
}

CGstPlayer::~CGstPlayer()
{
	gst_plugin_feature_list_free(m_pDevices);
	gst_deinit();
}

BOOL CGstPlayer::Init(HWND hParent)
{
	GstDeviceMonitor* pMonitor = gst_device_monitor_new();
	int iDeviceIndex = 0;

	m_hParent = hParent;

	GError* pPluginError = NULL;
	std::string strPrintAnalysisPluginPath = GetExecutableFolderPath() + "\\" + "printanalysis.dll";

	GstPlugin* pPlugin = gst_plugin_load_file(strPrintAnalysisPluginPath.c_str(), &pPluginError);
	if (!pPlugin)
	{
		m_bPrintAnalysisFilter = FALSE;
		PostMessage(m_hParent, WM_ON_PRINT_ANALYSIS_NOT_FOUND, 0L, 0L);
	}
	else
		m_bPrintAnalysisFilter = TRUE;

	gst_device_monitor_add_filter(pMonitor, "Video/Source", NULL);
	m_pDevices = gst_device_monitor_get_devices(pMonitor);

	for (GList* iter = m_pDevices; iter != NULL; iter = iter->next)
	{
		GstDevice* pDevice = GST_DEVICE(iter->data);
		const gchar* pcDeviceName = gst_device_get_display_name(pDevice);

		if (!CameraExists(pcDeviceName))
		{
			
			m_CameraList.push_back(pDevice);
			iDeviceIndex++;
		}
	}

	gst_device_monitor_stop(pMonitor);
	gst_object_unref(pMonitor);

	return TRUE;
}

void CGstPlayer::OnElementAddedToPipeline(GstBin* pBin, GstElement* pElement, gpointer pUserData)
{
	if (GST_IS_VIDEO_OVERLAY(pElement))
	{
		CGstPlayer* pGstPlayer = static_cast<CGstPlayer*>(pUserData);
		GstVideoOverlay* pOverlay = GST_VIDEO_OVERLAY(pElement);
		gst_video_overlay_set_window_handle(pOverlay, (guintptr) pGstPlayer->GetDisplayWindowHandle());
	}
}

GstElement* CGstPlayer::AddElement(
	std::string strFactoryName, std::string name, 
	std::string strPropertyName, void* pvProperty, 
	GstElement** pElementOut, std::string& strError)
{
	strError = "";

	GstElement* pElement = gst_element_factory_make(strFactoryName.c_str(), name.c_str());
	if (!pElement)
	{
		strError = "Unable to create " + strFactoryName + " element";
		return pElement;
	}

	if (pvProperty)
		g_object_set(pElement, strPropertyName.c_str(), pvProperty, NULL);

	if (!gst_bin_add(GST_BIN(m_pPipeline), pElement))
	{
		strError = "Unable to add " + strFactoryName + " element to pipeline";
		gst_object_unref(pElement);
		return NULL;
	}

	if (m_elementList.size())
	{
		if (!gst_element_link(m_elementList.back(), pElement))
		{
			strError = "Unable to link " + strFactoryName + " element";
			return NULL;
		}
	}

	m_elementList.push_back(pElement);

	if (pElementOut)
		*pElementOut = pElement;

	return pElement;
}

#define ADD_ELEMENT_WITH_ERR_CHECK(strFactoryName, name, strPropertyName, pvProperty, pElemOut, strError)	\
	AddElement(strFactoryName, name, strPropertyName, pvProperty, pElemOut, strError);						\
	if (strError.size())																					\
	{																										\
		gst_object_unref(m_pPipeline);																		\
		m_pPipeline = NULL;																					\
		return FALSE;																						\
	}

BOOL CGstPlayer::StartPreview(std::string strSource, int iDeviceIndex, std::string strSink, std::string strMediaType, int iWidth, int iHeight, FractionPtr framerate, BOOL bShowFps, std::string& strError, HWND hVideoWindow)
{
	m_elementList.clear();
	m_hVideoWindow = hVideoWindow;
	m_strSink = strSink;
	m_pPipeline = gst_pipeline_new("video-display");

	ADD_ELEMENT_WITH_ERR_CHECK(strSource, "source", "device-name", gst_device_get_display_name(m_CameraList[iDeviceIndex]), &m_pSource, strError);

	GstCaps* pSourceCaps = gst_caps_new_simple(
		strMediaType.c_str(),
		"width", G_TYPE_INT, iWidth,
		"height", G_TYPE_INT, iHeight,
		"framerate", GST_TYPE_FRACTION, framerate->first, framerate->second,
		NULL);

	ADD_ELEMENT_WITH_ERR_CHECK("capsfilter", "source-capsfilter", "caps", pSourceCaps, NULL, strError);

	if (strMediaType == "image/jpeg")
	{
		ADD_ELEMENT_WITH_ERR_CHECK("jpegdec", "mjpeg-decoder", "", NULL, NULL, strError);
		ADD_ELEMENT_WITH_ERR_CHECK("videoconvert", "video-convert", "", NULL, NULL, strError);
		
		GstCaps* pVideoConvertCaps = gst_caps_new_simple(
			"video/x-raw",
			"format", G_TYPE_STRING, "BGRx",
			NULL);

		ADD_ELEMENT_WITH_ERR_CHECK("capsfilter", "video-convert-caps-filter", "caps", pVideoConvertCaps, NULL, strError);

		gst_caps_unref(pVideoConvertCaps);

		if (m_bPrintAnalysisFilter)
		{
			m_pPrintAnalysis = ADD_ELEMENT_WITH_ERR_CHECK("printanalysis", "print-analysis", "", NULL, NULL, strError);
			SetPrintAnalysisElementOpts();
		}
	}

	if (bShowFps)
	{
		m_pSink = gst_element_factory_make(strSink.c_str(), NULL);
		ADD_ELEMENT_WITH_ERR_CHECK("fpsdisplaysink", "sink", "video-sink", m_pSink, NULL, strError);
	}
	else
		m_pSink = ADD_ELEMENT_WITH_ERR_CHECK(strSink.c_str(), "sink", "", NULL, NULL, strError);
	
	if (strSink != g_gstSinks[ID_SINK_OPENGL])
	{
		if (strSink == g_gstSinks[ID_SINK_AUTO])
			g_signal_connect(GST_BIN(m_pSink), "element-added", G_CALLBACK(OnElementAddedToPipeline), this);
		else
		{
			if (GST_IS_VIDEO_OVERLAY(m_pSink))
			{
				GstVideoOverlay* overlay = GST_VIDEO_OVERLAY(m_pSink);
				gst_video_overlay_set_window_handle(overlay, (guintptr)hVideoWindow);
			}
		}
	}

	gst_element_set_state(m_pPipeline, GST_STATE_PLAYING);

	m_tGstMainLoopThread = std::thread(std::bind(&CGstPlayer::GstreamerPipelineRun, this));

	if (strSink == g_gstSinks[ID_SINK_OPENGL])
		m_tOpenGlMonitorThread = std::thread(std::bind(&CGstPlayer::MonitorOpenGlWindow, this));

	gst_caps_unref(pSourceCaps);
	return TRUE;
}

void CGstPlayer::StopPreview()
{
	if (m_pPipeline)
	{
		gst_element_send_event(m_pPipeline, gst_event_new_eos());
		m_tGstMainLoopThread.join();
		
		if (m_strSink == g_gstSinks[ID_SINK_OPENGL])
			m_tOpenGlMonitorThread.join();

		// cleanup
		gst_element_set_state(m_pPipeline, GST_STATE_NULL);
		gst_object_unref(m_pPipeline);
		m_pPipeline = NULL;
		m_pPrintAnalysis = NULL;
	}
}

void CGstPlayer::GstreamerPipelineRun()
{
	if (m_pPipeline)
	{
		/* Wait until error or EOS */
		GstBus* pBus = gst_element_get_bus(m_pPipeline);
		bool done = false;

		while (!done)
		{
			GstMessage* msg = gst_bus_timed_pop_filtered(pBus, GST_CLOCK_TIME_NONE,
				(GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR | GST_MESSAGE_STATE_CHANGED));

			if (msg != NULL)
			{
				switch (GST_MESSAGE_TYPE(msg))
				{
				case GST_MESSAGE_EOS:
					// End of stream, clean up
					done = true;
					break;

				case GST_MESSAGE_ERROR:
				{
					// Error encountered, clean up
					GError* err;
					gchar* debug_info;
					gst_message_parse_error(msg, &err, &debug_info);
					g_printerr("Error received: %s\n", err->message);
					std::string strError = std::string("Error received: ") + err->message;
					AfxMessageBox(strError.c_str());
					
					if (debug_info)
					{
						g_printerr("Debug info: %s\n", debug_info ? debug_info : "none");
						strError = std::string("Debug info: ") + debug_info;
						AfxMessageBox(strError.c_str());
					}
					
					g_clear_error(&err);
					g_free(debug_info);
					done = true;
					break;
				}

				case GST_MESSAGE_STATE_CHANGED:
				{
					GstState state;
					GstStateChangeReturn ret = gst_element_get_state(m_pPipeline, &state, NULL, GST_CLOCK_TIME_NONE);

					if (state == GST_STATE_PLAYING && m_strSink != g_gstSinks[ID_SINK_OPENGL])
					{
						int iWidth = 0, iHeight = 0;
						LPARAM iResolution = NULL;
						
						GetCurrentResolution(iWidth, iHeight);

						iResolution = MAKELONG(iWidth, iHeight);
						SendMessage(m_hParent, WM_ON_RESIZE_WINDOW, NULL, iResolution);
					}
					break;
				}

				default:
					// Handle other messages if needed
					break;
				}
				gst_message_unref(msg);
			}
		}
		gst_object_unref(pBus);
	}
}

BOOL CGstPlayer::CameraExists(std::string strCameraName)
{
	for (GstDevice* pDevice : m_CameraList)
	{
		const gchar* pcDeviceName = gst_device_get_display_name(pDevice);
		if (strCameraName == pcDeviceName)
			return TRUE;
	}

	return FALSE;
}

std::string CGstPlayer::GetDeviceName(int iDeviceIdx)
{
	return gst_device_get_display_name(m_CameraList[iDeviceIdx]);
}

StringList CGstPlayer::GetDeviceNames()
{
	StringList deviceNames;

	for (GstDevice* pDevice : m_CameraList)
		deviceNames.push_back(gst_device_get_display_name(pDevice));

	return deviceNames;
}

void CGstPlayer::AddFramerateToResolution(ResolutionPtr pResolution, int iFramerateNum, int iFramerateDen)
{
	FramerateListPtr framerateList = pResolution->m_framerates;

	FramerateList::iterator framerateIt = find_if(
		framerateList->begin(),
		framerateList->end(),
		[iFramerateNum, iFramerateDen](FractionPtr pFramerate) {
			return (pFramerate->first == iFramerateNum && pFramerate->second == iFramerateDen);
		}
	);

	if (framerateIt == framerateList->end())
	{
		// new framerate
		FractionPtr pFramerate = std::make_shared<Fraction>();
		pFramerate->first = iFramerateNum;
		pFramerate->second = iFramerateDen;

		framerateList->push_back(pFramerate);
	}
}

void CGstPlayer::AddResolutionToFormat(DeviceCapsPtr pDeviceCaps, int iWidth, int iHeight, int iFramerateNum, int iFramerateDen)
{
	// format already exists, check if resolution exists
	ResolutionListPtr resolutionList = pDeviceCaps->m_resolutions;
	ResolutionPtr pResolution;

	ResolutionList::iterator resIt = find_if(
		resolutionList->begin(),
		resolutionList->end(),
		[iWidth, iHeight](ResolutionPtr pResolution) {
			return (pResolution->m_iWidth == iWidth && pResolution->m_iHeight == iHeight);
		}
	);

	if (resIt != resolutionList->end())
	{
		// resolution found
		pResolution = *resIt;
	}
	else
	{
		// new resolution
		pResolution = std::make_shared<Resolution>();
		
		pResolution->m_iWidth		= iWidth;
		pResolution->m_iHeight		= iHeight;
		pResolution->m_framerates	= std::make_shared< FramerateList >();

		resolutionList->push_back(pResolution);
	}

	AddFramerateToResolution(pResolution, iFramerateNum, iFramerateDen);
}

DeviceCapsListPtr CGstPlayer::GetDeviceCaps(std::string strSource, int iDeviceIndex)
{
	DeviceCapsListPtr deviceCapsList = std::make_shared< DeviceCapsList >();

	GstElement* pSource = gst_element_factory_make(strSource.c_str(), "source");
	if (!pSource) 
		return deviceCapsList;

	std::string strDeviceName = GetDeviceName(iDeviceIndex);
	
	g_object_set(pSource, "device-name", strDeviceName.c_str(), NULL);
	
	GstElement* pPipeline = gst_pipeline_new("pipeline");
	gst_bin_add(GST_BIN(pPipeline), pSource);

	// Set the element to the PLAYING state otherwise we do not get actual caps
	gst_element_set_state(pSource, GST_STATE_PLAYING);
	
	// Get the source pad and query the capabilities
	GstPad* pSrcPad = gst_element_get_static_pad(pSource, "src");
	if (!pSrcPad) 
		return deviceCapsList;

	// Query the capabilities from the source pad
	GstCaps* pCaps = gst_pad_query_caps(pSrcPad, NULL);

	if (pCaps)
	{
		guint iTotalCaps = gst_caps_get_size(pCaps);

		for (guint i = 0; i < iTotalCaps; i++) 
		{
			const GstStructure* pStructure = gst_caps_get_structure(pCaps, i);

			gint iWidth, iHeight;
			gint iFramerateNum, iFramerateDen;
			const gchar* pcMediaType = gst_structure_get_name(pStructure);
			gboolean bWidthFound = gst_structure_get_int(pStructure, "width", &iWidth);
			gboolean bHeightFound = gst_structure_get_int(pStructure, "height", &iHeight);
			gboolean bFramerateFound = gst_structure_get_fraction(pStructure, "framerate", &iFramerateNum, &iFramerateDen);
			
			if (pcMediaType && bWidthFound && bHeightFound && bFramerateFound)
			{
				std::string strMediaType = pcMediaType;
				std::string strFormat;

				if (strMediaType == "video/x-raw")
					strFormat = gst_structure_get_string(pStructure, "format");
				else if (strMediaType == "image/jpeg")
					strFormat = "MJPEG";
				else
					continue;

				DeviceCapsList::iterator devCapIt = find_if(deviceCapsList->begin(), deviceCapsList->end(), [strFormat](DeviceCapsPtr pDevCaps) { return pDevCaps->m_strFormat == strFormat; });
				DeviceCapsPtr pDeviceCaps;

				if (devCapIt != deviceCapsList->end())
				{
					// fromat exists
					pDeviceCaps = *devCapIt;
				}
				else
				{
					// new format
					pDeviceCaps = std::make_shared<DeviceCaps>();

					pDeviceCaps->m_strFormat	= strFormat;
					pDeviceCaps->m_strMediaType	= strMediaType;
					pDeviceCaps->m_resolutions	= std::make_shared< ResolutionList >();

					deviceCapsList->push_back(pDeviceCaps);
				}

				AddResolutionToFormat(pDeviceCaps, iWidth, iHeight, iFramerateNum, iFramerateDen);
			}
		}
		gst_caps_unref(pCaps);
	}

	// Clean up
	gst_object_unref(pSrcPad);
	gst_element_set_state(pSource, GST_STATE_NULL);
	gst_object_unref(pPipeline);

	return deviceCapsList;
}

DeviceCapsPtr CGstPlayer::ParseCapsStr(std::string strCaps)
{
	DeviceCapsPtr pDeviceCaps = std::make_shared<DeviceCaps>();
	return pDeviceCaps;
}

void CGstPlayer::GetCurrentResolution(int& iWidth, int& iHeight)
{
	GstPad* pSrcPad = gst_element_get_static_pad(m_pSource, "src");
	GstCaps* pCaps = gst_pad_get_current_caps(pSrcPad);

	iWidth = iHeight = 0;

	if (pCaps)
	{
		GstStructure* pCapsStructure = gst_caps_get_structure(pCaps, 0);

		gst_structure_get_int(pCapsStructure, "width", &iWidth);
		gst_structure_get_int(pCapsStructure, "height", &iHeight);

		gst_caps_unref(pCaps);
	}

	gst_object_unref(pSrcPad);
}

void CGstPlayer::Redraw()
{
	if (m_pSink && GST_IS_VIDEO_OVERLAY(m_pSink))
	{
		GstVideoOverlay* pOverlay = GST_VIDEO_OVERLAY(m_pSink);
		gst_video_overlay_expose(pOverlay);
	}
}



LRESULT CALLBACK CGstPlayer::NewOpenGlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CGstPlayer* pGstPlayer = (CGstPlayer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_CLOSE:
		PostMessage(pGstPlayer->GetParentHwnd(), WM_ON_OPENGL_WINDOW_CLOSE, NULL, NULL);
		break;
	default:
		return CallWindowProc(m_origOpenGlWndProc, hwnd, msg, wParam, lParam);
	}
	return CallWindowProc(m_origOpenGlWndProc, hwnd, msg, wParam, lParam);
}

void CGstPlayer::MonitorOpenGlWindow()
{
	while (true) 
	{
		HWND hwnd = FindWindow(NULL, "OpenGL renderer");

		if (hwnd)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
			m_origOpenGlWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)NewOpenGlWndProc);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Check every 100ms
	}
}

void CGstPlayer::SetPrintAnalysisOpts(int nAnalysisType, int nGrayscaleType, int nBlackoutType, BOOL bConnectValues, int nAoiHeight, int nAoiPartitions)
{
	m_nAnalysisType = nAnalysisType;
	m_nGrayscaleType = nGrayscaleType;
	m_nBlackoutType = nBlackoutType;
	m_bConnectValues = bConnectValues;
	m_nAoiHeight = nAoiHeight;
	m_nAoiPartitions = nAoiPartitions;

	SetPrintAnalysisElementOpts();
}

void CGstPlayer::SetPrintAnalysisElementOpts()
{
	if (m_pPrintAnalysis)
	{
		g_object_set(G_OBJECT(m_pPrintAnalysis), "analysis-type", m_nAnalysisType - IDC_RADIO_INTENSITY, NULL);
		g_object_set(G_OBJECT(m_pPrintAnalysis), "blackout-type", m_nBlackoutType - IDC_RADIO_BLACK_WHOLE, NULL);
		g_object_set(G_OBJECT(m_pPrintAnalysis), "grayscale-type", m_nGrayscaleType - IDC_RADIO_GRAY_WHOLE, NULL);
		g_object_set(G_OBJECT(m_pPrintAnalysis), "connect-values", m_bConnectValues, NULL);
		g_object_set(G_OBJECT(m_pPrintAnalysis), "aoi-height", m_nAoiHeight, NULL);
		g_object_set(G_OBJECT(m_pPrintAnalysis), "aoi-partitions", m_nAoiPartitions, NULL);
	}
}
