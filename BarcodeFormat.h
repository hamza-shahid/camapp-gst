#pragma once

#include <string>
#include <vector>
#include <memory>

typedef enum
{
	BarcodeFormat_None = 0,
	BarcodeFormat_Aztec = (1 << 0),
	BarcodeFormat_Codabar = (1 << 1),
	BarcodeFormat_Code39 = (1 << 2),
	BarcodeFormat_Code93 = (1 << 3),
	BarcodeFormat_Code128 = (1 << 4),
	BarcodeFormat_DataBar = (1 << 5),
	BarcodeFormat_DataBarExpanded = (1 << 6),
	BarcodeFormat_DataMatrix = (1 << 7),
	BarcodeFormat_EAN8 = (1 << 8),
	BarcodeFormat_EAN13 = (1 << 9),
	BarcodeFormat_ITF = (1 << 10),
	BarcodeFormat_MaxiCode = (1 << 11),
	BarcodeFormat_PDF417 = (1 << 12),
	BarcodeFormat_QRCode = (1 << 13),
	BarcodeFormat_UPCA = (1 << 14),
	BarcodeFormat_UPCE = (1 << 15),
	BarcodeFormat_MicroQRCode = (1 << 16),
	BarcodeFormat_RMQRCode = (1 << 17),
	BarcodeFormat_DXFilmEdge = (1 << 18),
	BarcodeFormat_DataBarLimited = (1 << 19),

	BarcodeFormat_LinearCodes = BarcodeFormat_Codabar | BarcodeFormat_Code39 | BarcodeFormat_Code93
	| BarcodeFormat_Code128 | BarcodeFormat_EAN8 | BarcodeFormat_EAN13
	| BarcodeFormat_ITF | BarcodeFormat_DataBar | BarcodeFormat_DataBarExpanded
	| BarcodeFormat_DataBarLimited | BarcodeFormat_DXFilmEdge | BarcodeFormat_UPCA
	| BarcodeFormat_UPCE,
	BarcodeFormat_MatrixCodes = BarcodeFormat_Aztec | BarcodeFormat_DataMatrix | BarcodeFormat_MaxiCode
	| BarcodeFormat_PDF417 | BarcodeFormat_QRCode | BarcodeFormat_MicroQRCode
	| BarcodeFormat_RMQRCode,
	BarcodeFormat_Any = BarcodeFormat_LinearCodes | BarcodeFormat_MatrixCodes,

	BarcodeFormat_Invalid = 0xFFFFFFFFu
} BarcodeFormat;

struct BarcodeInfo
{
	std::string barcode;
	std::string format;
};

typedef std::shared_ptr< BarcodeInfo > BarcodeInfoPtr;
typedef std::vector< BarcodeInfoPtr > BarcodeList;
