/*
 * Header file for bitmap.c and bmpRle implementations
 * For information on format of Bitmaps
 * https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-structures
 * and https://www.digicamsoft.com/bmp/bmp.html
 */

#ifndef TEAM121_BITMAP_H
#define TEAM121_BITMAP_H

 // Includes
#include <stdint.h>

// Bitmap File Header
#define BITMAPFILEHEADER_SIZE 14
#define BITMAP_FILE_TYPE 0x4d42

// Bitmap Information Header
#define BITMAPCOREHEADER_SIZE 12
#define BITMAPINFOHEADER_SIZE 40
#define BITMAPV2INFOHEADER_SIZE 52
#define BITMAPV3INFOHEADER_SIZE 56
#define BITMAPV4HEADER_SIZE 108
#define BITMAPV5HEADER_SIZE 124

// Color Palette
#define MAX_INFO_COLOR_PALETTE_SIZE 1024 // 256 max # of el, where every element of color palette is 4 bytes
#define MIN_INFO_COLOR_PALETTE_SIZE 4 // 1 pixel is 4 byte (r, g, b, reserved)
#define MAX_CORE_COLOR_PALETTE_SIZE 768
#define MIN_CORE_COLOR_PALETTE_SIZE 3

// Compression Mode
#define BI_RGB 0
#define BI_RLE8 1

// Escape Bytes
#define END_OF_LINE_BYTE 0
#define END_OF_BITMAP_BYTE 1

// PixelData
#define MIN_PIXEL_DATA_SIZE 1 // 1 equals one pixel index = 1 byte
#define BITS_PER_PIXEL 8

// Off Bits
#define MIN_INFO_OFF_BITS (BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE + MIN_INFO_COLOR_PALETTE_SIZE)
#define MIN_CORE_OFF_BITS (BITMAPFILEHEADER_SIZE + BITMAPCOREHEADER_SIZE + MIN_CORE_COLOR_PALETTE_SIZE)
#define MAX_INFO_OFF_BITS (BITMAPFILEHEADER_SIZE + BITMAPV5HEADER_SIZE + MAX_INFO_COLOR_PALETTE_SIZE)
#define MAX_CORE_OFF_BITS (BITMAPFILEHEADER_SIZE + BITMAPCOREHEADER_SIZE + MAX_CORE_COLOR_PALETTE_SIZE)

// Min Bitmap Sizes
#define MIN_INFO_BITMAP_SIZE (MIN_INFO_OFF_BITS + MIN_PIXEL_DATA_SIZE)
#define MIN_CORE_BITMAP_SIZE (MIN_CORE_OFF_BITS + MIN_PIXEL_DATA_SIZE)
#define MIN_BITMAP_SIZE (MIN_CORE_BITMAP_SIZE)

// Indices
#define BITMAP_INDEX_FILE_TYPE 0
#define BITMAP_INDEX_FILE_SIZE 2
#define BITMAP_INDEX_OFF_BITS 10
#define BITMAP_INDEX_INFO_SIZE 14
#define BITMAP_INDEX_WIDTH 18
#define BITMAP_INDEX_HEIGHT 22
#define BITMAP_INDEX_PLANES 26
#define BITMAP_INDEX_BIT_COUNT 28
#define BITMAP_INDEX_COMPRESSION 30
#define BITMAP_INDEX_SIZE_IMAGE 34
#define BITMAP_INDEX_X_PELS_PER_METER 38
#define BITMAP_INDEX_CLR_USED 46
#define BITMAP_INDEX_CLR_IMPORTANT 50
#define BITMAP_INDEX_CORE_WIDTH 18
#define BITMAP_INDEX_CORE_HEIGHT 20
#define BITMAP_INDEX_CORE_PLANES 22
#define BITMAP_INDEX_CORE_BIT_COUNT 24

// Validation
#define SUCCESS_BITMAP_VALIDATION 0
#define ERROR_TOO_SMALL 1
#define ERROR_WRONG_FILE_TYPE 2
#define ERROR_WRONG_WIDTH 3
#define ERROR_WRONG_HEIGHT 4
#define ERROR_ALREADY_COMPRESSED 5
#define ERROR_BITS_PER_PIXEL 6
#define ERROR_WRONG_PLANES 7
#define ERROR_INVALID_FILE_SIZE 8
#define ERROR_INVALID_INFO_HEADER_SIZE 9
#define ERROR_CLR_USED 10
#define ERROR_CLR_IMPORTANT 11
#define ERROR_WRONG_OFF_BITS 12
#define ERROR_NO_TOP_DOWN 13
#define ERROR_INVALID_COLOR_PALETTE_SIZE 14

// Bitmap Getter
int32_t getWidth(const uint8_t* imgIn);
int32_t getHeight(const uint8_t* imgIn);
uint32_t getOffBits(const uint8_t* imgIn);


// Functions
uint32_t getColorPaletteSize(const uint8_t* imgIn);
uint8_t getBitmapPaddingFromWidth(const uint8_t width);
uint8_t validateBitmap(const uint8_t* imgIn, const long size);
uint8_t* createOutputBufferForRle(const uint8_t* imgIn);
uint32_t writeBitmapMetadataForRle(const uint8_t* imgIn, uint8_t* imgOut);
uint32_t writeBitmapSizesForRle(uint8_t* imgOut, const uint32_t offBits, const uint32_t pixelDataSize);
uint8_t* moveToPixelData(uint8_t* imgIn);
uint8_t isBitmapCoreHeader(const uint8_t* imgIn);

// Bitmap Compression Functions
size_t bmpRle(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData);
size_t bmpRleV1(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData);
size_t bmpRleV2(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData);
size_t bmpRleEncodeV3(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData);

#endif //TEAM121_BITMAP_H
