#include <memory.h> // memcpy
#include <stdlib.h> // malloc
#include "bitmap.h"
#include "util.h"

/*
 * BITMAP GETTERS
 */

uint16_t getFileType(const uint8_t* imgIn) {
    uint16_t fileType;
    memcpy(&fileType, imgIn + BITMAP_INDEX_FILE_TYPE, 2);
    return fileType;
}
uint32_t getFileSize(const uint8_t* imgIn) {
    uint32_t fileSize;
    memcpy(&fileSize, imgIn + BITMAP_INDEX_FILE_SIZE, 4);
    return fileSize;
}
uint32_t getInfoHeaderSize(const uint8_t* imgIn) {
    uint32_t infoHeaderSize;
    memcpy(&infoHeaderSize, imgIn + BITMAP_INDEX_INFO_SIZE, 4);
    return infoHeaderSize;
}
int32_t getHeight(const uint8_t* imgIn) {
    uint32_t height = 0;
    isBitmapCoreHeader(imgIn) ? memcpy(&height, imgIn + BITMAP_INDEX_CORE_HEIGHT, 2) : memcpy(&height, imgIn + BITMAP_INDEX_HEIGHT, 4);
    return height;
}
int32_t getWidth(const uint8_t* imgIn) {
    uint32_t width = 0;
    isBitmapCoreHeader(imgIn) ? memcpy(&width, imgIn + BITMAP_INDEX_CORE_WIDTH, 2) : memcpy(&width, imgIn + BITMAP_INDEX_WIDTH, 4);
    return width;
}
uint16_t getPlanes(const uint8_t* imgIn) {
    uint16_t planes;
    memcpy(&planes, imgIn + (isBitmapCoreHeader(imgIn) ? BITMAP_INDEX_CORE_PLANES : BITMAP_INDEX_PLANES), 2);
    return planes;
}
uint32_t getCompression(const uint8_t* imgIn) {
    uint32_t compression;
    memcpy(&compression, imgIn + BITMAP_INDEX_COMPRESSION, 4);
    return compression;
}
uint32_t getOffBits(const uint8_t* imgIn) {
    uint32_t offBits;
    memcpy(&offBits, imgIn + BITMAP_INDEX_OFF_BITS, 4);
    return offBits;
}
uint16_t getBitCount(const uint8_t* imgIn) {
    uint16_t bitCount = 0;
    memcpy(&bitCount, imgIn + (isBitmapCoreHeader(imgIn) ? BITMAP_INDEX_CORE_BIT_COUNT : BITMAP_INDEX_BIT_COUNT), 2);
    return bitCount;
}
uint32_t getClrUsed(const uint8_t* imgIn) {
    uint32_t clrUsed;
    memcpy(&clrUsed, imgIn + BITMAP_INDEX_CLR_USED, 4);
    return clrUsed;
}
uint32_t getClrImportant(const uint8_t* imgIn) {
    uint32_t clrImportant;
    memcpy(&clrImportant, imgIn + BITMAP_INDEX_CLR_IMPORTANT, 4);
    return clrImportant;
}

/*
 * BITMAP FUNCTIONS
 */

 /*
  * Check if 'imgIn's InformationHeader size equals BitmapCoreHeader size
  */
uint8_t isBitmapCoreHeader(const uint8_t* imgIn) {
    return getInfoHeaderSize(imgIn) == BITMAPCOREHEADER_SIZE;
}

/*
* Get size of color palette in bytes
*/
uint32_t getColorPaletteSize(const uint8_t* imgIn) {
    return getOffBits(imgIn) - getInfoHeaderSize(imgIn) - BITMAPFILEHEADER_SIZE;
}

/*
 * Get padding from bitmap width in bytes (all bitmaps are 32 Bit padded)
 */
uint8_t getBitmapPaddingFromWidth(const uint8_t width) {
    return width % 4 == 0 ? 0 : 4 - (width % 4);
}

/*
 * Check if information header size corresponds to either BitmapCoreHeader, BitmapInfoHeader, BitmapV4Header or BitmapV5Header size
 */
uint8_t isInfoHeaderSizeValid(const uint32_t infoHeaderSize) {
    return infoHeaderSize == BITMAPCOREHEADER_SIZE || infoHeaderSize == BITMAPINFOHEADER_SIZE || infoHeaderSize == BITMAPV4HEADER_SIZE || infoHeaderSize == BITMAPV5HEADER_SIZE;
}

/*
 * Validates BitmapCoreHeader specifics
 * returns 'SUCCESS_BITMAP_VALIDATION' if success or 'ERROR_*' if not valid
 */
uint8_t validateCoreInfoHeader(const uint8_t* imgIn) {
    // no need to check file size as already checked in function 'validate'
    if (getOffBits(imgIn) < MIN_CORE_OFF_BITS || getOffBits(imgIn) > MAX_CORE_OFF_BITS) return ERROR_WRONG_OFF_BITS;
    if (getColorPaletteSize(imgIn) % 3 != 0 || getColorPaletteSize(imgIn) < MIN_CORE_COLOR_PALETTE_SIZE || getColorPaletteSize(imgIn) > MAX_CORE_COLOR_PALETTE_SIZE) return ERROR_INVALID_COLOR_PALETTE_SIZE;
    return SUCCESS_BITMAP_VALIDATION;
}

/*
 * Validates BitmapInfoHeader specifics
 * returns 'SUCCESS_BITMAP_VALIDATION' if success or 'ERROR_*' if not valid
 */
uint8_t validateInfoHeader(const uint8_t* imgIn) {
    uint32_t infoHeaderSize = getInfoHeaderSize(imgIn);
    if (getFileSize(imgIn) < MIN_INFO_BITMAP_SIZE) return ERROR_TOO_SMALL;
    if (getOffBits(imgIn) < MIN_INFO_OFF_BITS || getOffBits(imgIn) > MAX_INFO_OFF_BITS) return ERROR_WRONG_OFF_BITS;
    if (!isInfoHeaderSizeValid(infoHeaderSize)) return ERROR_INVALID_INFO_HEADER_SIZE; // check header size
    if (getCompression(imgIn) != BI_RGB) return ERROR_ALREADY_COMPRESSED; // check if uncompressed
    if (getClrUsed(imgIn) > 256) return ERROR_CLR_USED;
    if (getClrImportant(imgIn) > 256) return ERROR_CLR_IMPORTANT;
    if (getColorPaletteSize(imgIn) % 4 != 0 || getColorPaletteSize(imgIn) < MIN_INFO_COLOR_PALETTE_SIZE || getColorPaletteSize(imgIn) > MAX_INFO_COLOR_PALETTE_SIZE) return ERROR_INVALID_COLOR_PALETTE_SIZE;
    return SUCCESS_BITMAP_VALIDATION;
}

/*
 * Validates if bitmap is valid for being compressed
 * returns 'SUCCESS_BITMAP_VALIDATION' if success or 'ERROR_*' if not valid
 */
uint8_t validateBitmap(const uint8_t* imgIn, const long size) {

    if (size < MIN_BITMAP_SIZE) return ERROR_TOO_SMALL; // is at least min size
    if (getFileType(imgIn) != BITMAP_FILE_TYPE) return ERROR_WRONG_FILE_TYPE; // file type equals bitmap spec
    if ((long)getFileSize(imgIn) != size) return ERROR_INVALID_FILE_SIZE; // specified file size equals real file size

    if (getWidth(imgIn) == 0 || getWidth(imgIn) > 7680) return ERROR_WRONG_WIDTH; // check width 8K resolution
    if (getHeight(imgIn) == 0 || getHeight(imgIn) > 7680) return ERROR_WRONG_HEIGHT; // check height 8K resolution
    if (getHeight(imgIn) < 0) return ERROR_NO_TOP_DOWN; // check if top down bitmap
    if (getPlanes(imgIn) != 1) return ERROR_WRONG_PLANES; // check if planes is 1
    if (getBitCount(imgIn) != BITS_PER_PIXEL) return ERROR_BITS_PER_PIXEL; // is 8bpp bitmap 

    // validate further based on BitmapCoreHeader or different header
    return isBitmapCoreHeader(imgIn) ? validateCoreInfoHeader(imgIn) : validateInfoHeader(imgIn);
}

/*
 * Calculates new off bits
 * as BitmapCoreHeader does not support compression, offBits differ after compression
 */
uint32_t calcOffBitsForRle(const uint8_t* imgIn) {
    if (isBitmapCoreHeader(imgIn)) {
        return BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE + (getColorPaletteSize(imgIn) / 3) * 4;
    }
    return getOffBits(imgIn);
}

/*
 * Creates a Buffer to write a compressed bitmap into
 */
uint8_t* createOutputBufferForRle(const uint8_t* imgIn) {
    const uint32_t width = getWidth(imgIn);
    const uint32_t height = getHeight(imgIn);
    const uint32_t offBits = calcOffBitsForRle(imgIn);
    const uint8_t bitmapPadding = getBitmapPaddingFromWidth(width);
    const uint32_t pixelDataSize = getFileSize(imgIn) - getOffBits(imgIn);
    // (inPixelDataSize - bitmapPadding * height) * 2 + height * 2
    const uint32_t maxSize = offBits + 2 * (pixelDataSize - bitmapPadding * height + height);
    return malloc(maxSize);
}

/*
 * Convert RGBTriple (3 byte) to RGBQuad (4 byte, last byte is '0' it is reserved)
 * (Bitmaps using BitmapCoreInfo use RGBTriple instead of RGBQuad)
 * returns new size of RGBQuad in bytes
 */
uint32_t rgbTripleToRgbQuadColorPalette(const uint8_t* colorPaletteIn, uint8_t* colorPaletteOut, const uint32_t colorPaletteSize) {
    int colors = colorPaletteSize / 3;
    for (int i = 0; i < colors; i++) {
        memcpy(colorPaletteOut + i * 4, colorPaletteIn + i * 3, 3);
        memset(colorPaletteOut + i * 4 + 3, 0, 1);
    }
    return colors * 4;
}

/*
 * Returns a pointer pointing to the start of the pixel data in the Bitmap
 */
uint8_t* moveToPixelData(uint8_t* imgIn) {
    return imgIn + getOffBits(imgIn);
}

/*
 * Write Bitmap Metadata
 * - copy BitmapFileHeader, BitmapInfoHeader and ColorPalette into imgOut
 * - if BitmapCoreHeader used convert first to BitmapInfoHeader and set default values
 * - set compression to RLE_8
 * - set offBits
 */
uint32_t writeBitmapMetadataForRle(const uint8_t* imgIn, uint8_t* imgOut) {
    const uint32_t inInfoHeaderSize = getInfoHeaderSize(imgIn);
    const uint32_t colorPaletteSize = getColorPaletteSize(imgIn);
    const uint8_t isCoreHeader = isBitmapCoreHeader(imgIn);
    const uint32_t inHeaderSize = BITMAPFILEHEADER_SIZE + inInfoHeaderSize;
    const uint32_t inTopSize = inHeaderSize + colorPaletteSize;
    const uint32_t outInfoHeaderSize = isCoreHeader ? BITMAPINFOHEADER_SIZE : inInfoHeaderSize;
    const uint32_t outHeaderSize = BITMAPFILEHEADER_SIZE + outInfoHeaderSize;
    const uint32_t outTopSize = calcOffBitsForRle(imgIn);

    // transform BitmapCoreHeader to BitmapInfoHeader
    // see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapcoreheader
    // compared to https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    if (isCoreHeader) {
        // copy File Header, and parts of Information Header inclusive 'width', exclusive 'height'
        memcpy(imgOut, imgIn, BITMAP_INDEX_CORE_HEIGHT);
        // set InformationHeader size to '40'
        memset(imgOut + BITMAP_INDEX_INFO_SIZE, BITMAPINFOHEADER_SIZE, 1);
        memset(imgOut + BITMAP_INDEX_INFO_SIZE + 1, 0, 3);
        // Note: BitmapCoreHeader stores width & height each as 2 bytes,
        // BitmapInformationHeader „ each as 4 bytes
        // set missing bytes of width to '0'
        memset(imgOut + BITMAP_INDEX_WIDTH + 2, 0, 2);
        // copy height
        memcpy(imgOut + BITMAP_INDEX_HEIGHT, imgIn + BITMAP_INDEX_CORE_HEIGHT, 2);
        memset(imgOut + BITMAP_INDEX_HEIGHT + 2, 0, 2);
        // copy planes (2 byte) and bitCount (2 byte)
        memcpy(imgOut + BITMAP_INDEX_PLANES, imgIn + BITMAP_INDEX_CORE_PLANES, 4);
        // convert RGBTriple (3 Byte rows) to RGBQuad (4 Byte rows) color palette
        rgbTripleToRgbQuadColorPalette(imgIn + inHeaderSize, imgOut + outHeaderSize, colorPaletteSize);
        // set missing fields to 0
        memset(imgOut + BITMAP_INDEX_X_PELS_PER_METER, 0, 16);
    }
    else {
        // Copy File Header, Information Header and Color Palette
        memcpy(imgOut, imgIn, inTopSize);
    }

    // write offBits
    memcpy(imgOut + BITMAP_INDEX_OFF_BITS, &outTopSize, 4);
    // write compression RLE8
    memset(imgOut + BITMAP_INDEX_COMPRESSION, BI_RLE8, 1);
    memset(imgOut + BITMAP_INDEX_COMPRESSION + 1, 0, 3);

    return outTopSize;
}

/*
 * Writes sizes after run-length-encoding
 * - file size
 * - size image
 */
uint32_t writeBitmapSizesForRle(uint8_t* imgOut, const uint32_t offBits, const uint32_t pixelDataSize) {
    const uint32_t outSize = offBits + pixelDataSize;

    memcpy(imgOut + BITMAP_INDEX_FILE_SIZE, &outSize, 4);
    memcpy(imgOut + BITMAP_INDEX_SIZE_IMAGE, &pixelDataSize, 4);

    return outSize;
}
