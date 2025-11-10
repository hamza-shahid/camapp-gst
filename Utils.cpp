#include "pch.h"
#include "Utils.h"

#include <gdiplus.h>
#include <algorithm>
#include <vector>
#include <fstream>


using namespace Gdiplus;

ULONG_PTR CUtils::m_pGiplusToken = NULL;


void CUtils::InitGdiPlus()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&CUtils::m_pGiplusToken, &gdiplusStartupInput, nullptr);
}

void CUtils::DeInitGdiPlus()
{
    GdiplusShutdown(CUtils::m_pGiplusToken);
}

HBITMAP CUtils::LoadPNGToHBITMAP(UINT nResourceID, int targetWidth, int targetHeight)
{
    HINSTANCE hInst = AfxGetInstanceHandle();
    HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(nResourceID), _T("PNG"));
    if (!hResource) return nullptr;

    DWORD imageSize = ::SizeofResource(hInst, hResource);
    const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
    if (!pResourceData) return nullptr;

    HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hBuffer) return nullptr;
    void* pBuffer = ::GlobalLock(hBuffer);
    memcpy(pBuffer, pResourceData, imageSize);
    ::GlobalUnlock(hBuffer);

    IStream* pStream = nullptr;
    if (FAILED(::CreateStreamOnHGlobal(hBuffer, TRUE, &pStream))) 
    {
        ::GlobalFree(hBuffer);
        return nullptr;
    }

    // Load the original image
    Gdiplus::Bitmap* pOriginalBitmap = Gdiplus::Bitmap::FromStream(pStream);
    HBITMAP hBmp = nullptr;

    if (pOriginalBitmap)
    {
        // Create a new bitmap with the target size
        Gdiplus::Bitmap resizedBitmap(targetWidth, targetHeight, PixelFormat32bppARGB);
        Gdiplus::Graphics graphics(&resizedBitmap);

        // Set high-quality resizing mode
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

        // Draw the original image onto the resized bitmap, scaling it down
        graphics.DrawImage(pOriginalBitmap, Gdiplus::Rect(0, 0, targetWidth, targetHeight));

        // Convert the resized bitmap to HBITMAP
        resizedBitmap.GetHBITMAP(Color(0, 0, 0, 0), &hBmp);

        delete pOriginalBitmap;
    }

    pStream->Release();
    return hBmp;
}

BOOL CUtils::SaveBGRXToFile(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename, CLSID clsidEncoder)
{
    BOOL bRet = FALSE;

    // Step 1: Initialize GDI+ Bitmap with the BGRX data
    Bitmap bitmap(nWidth, nHeight, PixelFormat32bppRGB); // PixelFormat32bppRGB works for BGRX format

    // Lock the entire bitmap to copy BGRX data into it
    BitmapData bitmapData;
    Rect rect(0, 0, nWidth, nHeight);

    if (bitmap.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppRGB, &bitmapData) != Ok)
        return FALSE;

    // Step 2: Copy each row of the BGRX data to the bitmap
    if (bitmapData.Stride == nWidth * 4)
    {
        memcpy(bitmapData.Scan0, pBgrxData, nWidth * nHeight * 4);
    }
    else
    {
        for (int y = 0; y < nHeight; ++y)
        {
            BYTE* dest = (BYTE*)bitmapData.Scan0 + y * bitmapData.Stride;
            const BYTE* src = pBgrxData + y * nWidth * 4; // BGRX has 4 bytes per pixel
            memcpy(dest, src, nWidth * 4);
        }
    }

    // Unlock the bitmap after data is copied
    bitmap.UnlockBits(&bitmapData);

    IStream* pImgStream = nullptr;

    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pImgStream);
    if (SUCCEEDED(hr))
    {
        if (bitmap.Save(pImgStream, &clsidEncoder) == Ok)
        {
            // Rewind stream
            LARGE_INTEGER li = {};
            pImgStream->Seek(li, STREAM_SEEK_SET, NULL);

            // Write to file manually
            std::ofstream ofs(filename, std::ios::binary);
            if (ofs.is_open() && !ofs.fail())
            {
                STATSTG stat;
                pImgStream->Stat(&stat, STATFLAG_NONAME);

                ULONG ulBytesRead = 0;
                std::vector<BYTE> vBuffer(stat.cbSize.LowPart);

                hr = pImgStream->Read(vBuffer.data(), vBuffer.size(), &ulBytesRead);
                if (SUCCEEDED(hr))
                {
                    ofs.write(reinterpret_cast<const char*>(vBuffer.data()), ulBytesRead);
                    if (ofs.good())
                        bRet = TRUE;
                    ofs.flush();
                }
            }
        }
        pImgStream->Release();
    }

    return bRet;
}

BOOL CUtils::SaveBGRXToBMP(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename)
{
    // BMP rows must be aligned to 4 bytes. Since BGRx is already 4 bytes per pixel, no padding needed.
    int nBytesPerPixel = 4;
    int nImageSize = nWidth * nHeight * nBytesPerPixel;

    // File header
    BITMAPFILEHEADER bmfHeader{};
    bmfHeader.bfType = 0x4D42; // "BM"
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + nImageSize;

    // Info header
    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = nWidth;
    bi.biHeight = -nHeight; // negative = top-down bitmap (so image isn't upside down)
    bi.biPlanes = 1;
    bi.biBitCount = 32;    // BGRx is 32 bits per pixel
    bi.biCompression = BI_RGB; // no compression
    bi.biSizeImage = nImageSize;

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file)
        return false;

    file.write(reinterpret_cast<const char*>(&bmfHeader), sizeof(bmfHeader));
    file.write(reinterpret_cast<const char*>(&bi), sizeof(bi));
    file.write(reinterpret_cast<const char*>(pBgrxData), nImageSize);

    return true;
}

BOOL CUtils::SaveBGRXToJPEG(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename)
{
    CLSID clsid;
    if (GetEncoderClsid(L"image/jpeg", &clsid) == -1)
        return FALSE;

    return SaveBGRXToFile(pBgrxData, nWidth, nHeight, filename, clsid);
}

BOOL CUtils::SaveBGRXToPNG(const BYTE* pBgrxData, int nWidth, int nHeight, std::string filename)
{
    CLSID clsid;
    if (GetEncoderClsid(L"image/png", &clsid) == -1)
        return FALSE;

    return SaveBGRXToFile(pBgrxData, nWidth, nHeight, filename, clsid);
}

// Helper function to get CLSID of encoder (e.g., JPEG encoder)
int CUtils::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;           // Number of image encoders
    UINT size = 0;          // Size of the image encoder array in bytes

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1; // No encoders available

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (!pImageCodecInfo) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT i = 0; i < num; ++i)
    {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return i;
        }
    }
    free(pImageCodecInfo);
    return -1; // Encoder not found
}

void CUtils::ConvertYUY2ToBGRX(const BYTE* pYuy22Data, BYTE* pRgbData, int nWidth, int nHight)
{
    int nRgbIndex = 0;
    
    for (int i = 0; i < nWidth * nHight * 2; i += 4)
    {
        int y0 = pYuy22Data[i];
        int u = pYuy22Data[i + 1] - 128;
        int y1 = pYuy22Data[i + 2];
        int v = pYuy22Data[i + 3] - 128;

        int r0 = y0 + (1.370705 * v);
        int g0 = y0 - (0.698001 * v) - (0.337633 * u);
        int b0 = y0 + (1.732446 * u);

        int r1 = y1 + (1.370705 * v);
        int g1 = y1 - (0.698001 * v) - (0.337633 * u);
        int b1 = y1 + (1.732446 * u);

        pRgbData[nRgbIndex++] = std::clamp(b0, 0, 255);
        pRgbData[nRgbIndex++] = std::clamp(g0, 0, 255);
        pRgbData[nRgbIndex++] = std::clamp(r0, 0, 255);
        pRgbData[nRgbIndex++] = 0;

        pRgbData[nRgbIndex++] = std::clamp(b1, 0, 255);
        pRgbData[nRgbIndex++] = std::clamp(g1, 0, 255);
        pRgbData[nRgbIndex++] = std::clamp(r1, 0, 255);
        pRgbData[nRgbIndex++] = 0;
    }
}

BOOL CUtils::SaveYUY2ToJPEG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::string filename)
{
    std::vector<BYTE> rgbData(nWidth * nHeight * 4);
   
    ConvertYUY2ToBGRX(pYuy2Data, rgbData.data(), nWidth, nHeight);

    return SaveBGRXToJPEG(rgbData.data(), nWidth, nHeight, filename);
}

BOOL CUtils::SaveYUY2ToPNG(const BYTE* pYuy2Data, int nWidth, int nHeight, std::string filename)
{
    std::vector<BYTE> rgbData(nWidth * nHeight * 4);

    ConvertYUY2ToBGRX(pYuy2Data, rgbData.data(), nWidth, nHeight);

    return SaveBGRXToPNG(rgbData.data(), nWidth, nHeight, filename);
}

BYTE* CUtils::LoadImageAsBGRx(const std::string filename, int& nWidth, int& nHeight)
{
    Gdiplus::Bitmap bmp(StdStringToStdWString(filename).c_str());
    if (bmp.GetLastStatus() != Gdiplus::Ok) return nullptr;

    nWidth = bmp.GetWidth();
    nHeight = bmp.GetHeight();

    Gdiplus::Rect rect(0, 0, nWidth, nHeight);
    BitmapData bmpData{};

    if (bmp.LockBits(&rect, ImageLockModeRead, PixelFormat32bppRGB, &bmpData) != Gdiplus::Ok)
        return nullptr;

    UINT uBytesPerPixel = GetPixelFormatSize(bmpData.PixelFormat) / 8;
    int nStride = nWidth * uBytesPerPixel;
    BYTE* pBgrxData = new BYTE[nStride * nHeight];

    if (nStride == nWidth * uBytesPerPixel)
    {
        memcpy(pBgrxData, bmpData.Scan0, nWidth * nHeight * uBytesPerPixel);
    }
    else
    {
        for (int y = 0; y < nHeight; y++)
        {
            memcpy(pBgrxData + y * nStride, (BYTE*)bmpData.Scan0 + y * bmpData.Stride, nStride);
        }
    }

    bmp.UnlockBits(&bmpData);
    return pBgrxData;
}

BOOL CUtils::SubtractBgrxImages(const BYTE* pImg1, const BYTE* pImg2, BYTE* pResult, int nWidth, int nHeight)
{
    int nDiff = 0;
    int stride = nWidth * 4;

    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
        {
            int idx = y * stride + x * 4;

            pResult[idx + 0] = (BYTE)std::abs(pImg1[idx + 0] - pImg2[idx + 0]); // B
            pResult[idx + 1] = (BYTE)std::abs(pImg1[idx + 1] - pImg2[idx + 1]); // G
            pResult[idx + 2] = (BYTE)std::abs(pImg1[idx + 2] - pImg2[idx + 2]); // R
            pResult[idx + 3] = 0xFF;

            if (pResult[idx + 0] || pResult[idx + 1] || pResult[idx + 2])
                nDiff++;
        }
    }


    return nDiff == 0 ? TRUE : FALSE;
}

BOOL CUtils::CompareImages(std::string strImg1Filename, std::string strImg2Filename)
{
    int w1, h1, w2, h2;

    BYTE* pImg1 = LoadImageAsBGRx(strImg1Filename, w1, h1);
    BYTE* pImg2 = LoadImageAsBGRx(strImg2Filename, w2, h2);
    BOOL bSame = FALSE;

    if (!pImg1 || !pImg2 || w1 != w2 || h1 != h2)
    {
        MessageBox(NULL, "Image load failed or sizes don't match", "Error", MB_ICONERROR);
    }
    else
    {
        BYTE* pResult = new BYTE[w1 * h1 * 4];
        std::string strDiffFilename = GetFilePathWithoutExt(strImg1Filename.c_str()).GetBuffer() + std::string("_diff");

        bSame = SubtractBgrxImages(pImg1, pImg2, pResult, w1, h1);
        if (bSame)
        {
            MessageBox(NULL, "Files are a match", "Match", MB_ICONINFORMATION);
        }
        else
        {
            std::string strErrMsg = "Files are not a match. Saving diff in file: " + strDiffFilename + ".bmp";
            MessageBox(NULL, strErrMsg.c_str(), "Warning", MB_ICONWARNING);

            BOOL bDiffSaved = SaveFrameToFile(pResult, w1, h1, "BGRx", strDiffFilename, "bmp");
            if (!bDiffSaved)
            {
                std::string strErrMsg = "Unable to save diff file: " + strDiffFilename;
                MessageBox(NULL, strErrMsg.c_str(), "Error", MB_ICONERROR);
            }
        }

        if (pResult) delete[] pResult;
    }

    if (pImg1) delete[] pImg1;
    if (pImg2) delete[] pImg2;

    return TRUE;
}

BOOL CUtils::SaveFrameToFile(const BYTE* pFrameBuffer, int nWidth, int nHeight, std::string format, std::string filename, std::string ext)
{
    BOOL ret = FALSE;

    if (format == "BGRx")
    {
        if (ext == "jpg" || ext == "jpeg")
            ret = CUtils::SaveBGRXToJPEG(pFrameBuffer, nWidth, nHeight, filename + "." + ext);
        else if (ext == "png")
            ret = CUtils::SaveBGRXToPNG(pFrameBuffer, nWidth, nHeight, filename + "." + ext);
        else if (ext == "bmp")
            ret = SaveBGRXToBMP(pFrameBuffer, nWidth, nHeight, filename + "." + ext);
    }
    else if (format == "YUY2")
        ret = CUtils::SaveYUY2ToJPEG(pFrameBuffer, nWidth, nHeight, filename + "." + ext);
    else
        AfxMessageBox("Unsupported video format");

    if (!ret)
        AfxMessageBox("Unable to save snapshot");

    return ret;
}

std::wstring CUtils::StdStringToStdWString(std::string str)
{
    size_t len = str.size() + 1;
    wchar_t* wcharStr = new wchar_t[len];

    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wcharStr, len);

    std::wstring wstr(wcharStr);
    delete[] wcharStr;

    return wstr;
}

std::string CUtils::GetFormattedTime()
{
    SYSTEMTIME st;
    GetLocalTime(&st); // Use GetSystemTime for UTC time

    CString formattedTime;
    formattedTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d.%03d"),
        st.wYear,
        st.wMonth,
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond,
        st.wMilliseconds);

    return formattedTime.GetBuffer();
}

std::string CUtils::GetExecutableFolderPath()
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

void CUtils::LogToFile(const std::string& message)
{
    std::string filename = GetExecutableFolderPath() + "\\" + std::string("camapp_") + CTime::GetCurrentTime().Format("%Y-%m-%d").GetBuffer() + std::string(".log");
    std::ofstream file(filename, std::ios::app);
    
    if (file)
    {
        file << "[" << GetFormattedTime() << "] " << message << std::endl;
    }
}

CString CUtils::GetFilePathWithoutExt(const CString strFilePath)
{
    int dotPos = strFilePath.ReverseFind('.');
    if (dotPos > 0)
        return strFilePath.Left(dotPos);
    return strFilePath;
}

int CUtils::GetNextFileNumberInSeq(const char* pFileDir, const char* pFileNamePrefix, const char* pExt)
{
    CString searchPath;
    searchPath.Format("%s\\%s*.%s", pFileDir, pFileNamePrefix, pExt);
    int iSeqNo = -1;

    CFileFind finder;
    BOOL bFound = finder.FindFile(searchPath);

    while (bFound)
    {
        bFound = finder.FindNextFile();
        if (finder.IsDots() || finder.IsDirectory())
            continue;

        CString fileName = finder.GetFileName();
        int num = 0;
        std::string filenameFormat = pFileNamePrefix + std::string("%d.") + pExt;

        if (sscanf_s(fileName, filenameFormat.c_str(), &num) == 1)
        {
            if (num > iSeqNo)
                iSeqNo = num;
        }
    }

    return iSeqNo + 1;
}

std::string CUtils::GetErrorAsString(HRESULT error)
{
    LPSTR buffer = nullptr;

    DWORD size = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0,
        NULL);

    std::string message;
    if (size > 0 && buffer != nullptr)
    {
        message = buffer;
        LocalFree(buffer);
    }
    else
    {
        message = "Unknown error code: " + std::to_string(error);
    }

    return message;
}
