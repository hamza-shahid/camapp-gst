#pragma once

#include <string>

class CUtils
{
private:
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static void ConvertYUY2ToBGRX(const BYTE* pYuy22Data, BYTE* pRgbData, int nWidth, int nHight);

	static std::wstring StdStringToStdWString(std::string str);

	static BYTE* LoadImageAsBGRx(const std::string filename, int& nWidth, int& nHeight);
	static BOOL SubtractBgrxImages(const BYTE* pImg1, const BYTE* pImg2, BYTE* result, int nWidth, int nHeight);

	static BOOL SaveBGRXToBMP(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename);
	static BOOL SaveBGRXToJPEG(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename);
	static BOOL SaveYUY2ToJPEG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::string filename);
	static BOOL SaveBGRXToPNG(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename);
	static BOOL SaveYUY2ToPNG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::string filename);
	static BOOL SaveBGRXToFile(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename, CLSID clsidEncoder);

public:
	static std::string GetExecutableFolderPath();

	static void InitGdiPlus();
	static void DeInitGdiPlus();
	
	static BOOL CompareImages(std::string strImg1Filename, std::string strImg2Filename);
	static BOOL SaveFrameToFile(const BYTE* pFrameBuffer, int nWidth, int nHeight, std::string format, std::string filename, std::string ext);
	
	static int GetNextFileNumberInSeq(const char* pFileDir, const char* pFileNamePrefix, const char* pExt);
	static CString GetFilePathWithoutExt(const CString strFilePath);
	
	static HBITMAP LoadPNGToHBITMAP(UINT nResourceID, int targetWidth = 40, int targetHeight = 40);

	static std::string GetFormattedTime();

	static void LogToFile(const std::string& message);

private:
	static ULONG_PTR m_pGiplusToken;
};
