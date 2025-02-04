#pragma once

#include <string>

class CUtils
{
private:
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static void ConvertYUY2ToBGRX(const BYTE* pYuy22Data, BYTE* pRgbData, int nWidth, int nHight);

	static std::wstring StdStringToStdWString(std::string& str);

	static BOOL SaveBGRXToJPEG(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename);
	static BOOL SaveYUY2ToJPEG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::string filename);

public:
	static void InitGdiPlus();
	static void DeInitGdiPlus();
	
	static BOOL SaveFrameToFile(const BYTE* pFrameBuffer, int nWidth, int nHeight, std::string format, std::string filename);
	
	static int GetNextFileNumberInSeq(const char* pFileDir, const char* pFileNamePrefix, const char* pExt);
	
	static HBITMAP LoadPNGToHBITMAP(UINT nResourceID, int targetWidth = 40, int targetHeight = 40);

	static std::string GetFormattedTime();

private:
	static ULONG_PTR m_pGiplusToken;
};
