#include <stdint.h>
#include <memory.h>
#include "bitmap.h"
#include "util.h"

//-------Uncompressed-----------//
// 5x2 Example with padding bytes
// 01 03 03 03 04 00 00 00 -> 3 padding bytes for 4 byte alignment
// 01 03 03 03 04 00 00 00

//-------Compressed-----------// only in encoded mode
// 01 01 03 03 01 04 00 00 -> 00 00 is end of line 
// 01 01 03 03 01 04 00 01 -> 00 01 is end of file

// Uses only encoded mode
size_t bmpRleEncodeV3(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData) {

    if (rleData == NULL) {
        throwError("some error occured");
    }

    uint32_t inPixelIndex = 0;
    uint32_t outPixelIndex = 0;
    const uint8_t bitmapPadding = getBitmapPaddingFromWidth(width);
    const uint32_t inPixelDataSizeNoLastPadd = (width + bitmapPadding) * height - bitmapPadding;
    const uint32_t inPixelIndexOfLastPixel = inPixelDataSizeNoLastPadd - 1;
    /*
    * Compare the pixels one by one in each scan line while skipping padding bytes and
    * not going over last pixel of file(otherwise seg fault)
    */
    for (;inPixelIndex < inPixelDataSizeNoLastPadd; inPixelIndex++) {
        uint8_t rep = 1;
        for (;rep < 255 && (inPixelIndex % (width + bitmapPadding)) < width - 1 &&
            inPixelIndex < inPixelIndexOfLastPixel &&
            imgIn[inPixelIndex] == imgIn[inPixelIndex + 1]; rep++, inPixelIndex++) {
        }
        //write the amount of same pixels
        rleData[outPixelIndex++] = rep;
        rleData[outPixelIndex++] = imgIn[inPixelIndex];

        if (inPixelIndex == inPixelIndexOfLastPixel) {
            // end of file
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            rleData[outPixelIndex++] = END_OF_BITMAP_BYTE;
        }
        else if ((inPixelIndex % (width + bitmapPadding)) == width - 1) {
            // end of line
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            inPixelIndex += bitmapPadding; // skip padding bytes and go to next scan line
        }
    }
    return outPixelIndex;
}
