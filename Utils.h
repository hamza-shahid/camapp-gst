#pragma once

#include <string>

class CUtils
{
private:
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static void ConvertYUY2ToBGRX(const BYTE* pYuy22Data, BYTE* pRgbData, int nWidth, int nHight);

public:
	static void InitGdiPlus();
	static void DeInitGdiPlus();
	
	static BOOL SaveBGRXToJPEG(const BYTE* pBgrxData, int nWidth, int nHeight, std::wstring filename);
	static BOOL SaveYUY2ToJPEG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::wstring filename);
	static BOOL SaveFrameToFile(const BYTE* pFrameBuffer, int nWidth, int nHeight, std::string format, std::wstring filename);
	
	static HBITMAP LoadPNGToHBITMAP(UINT nResourceID, int targetWidth = 40, int targetHeight = 40);

	static std::wstring CStringToStdWString(CString& str);

	static std::string GetFormattedTime();

	static BOOL WriteToRegistry(std::string subKey, std::string regKey, std::string regValue);
	static BOOL DeleteRegistryEntryTree(std::string subKey);

private:
	static ULONG_PTR m_pGiplusToken;
};
