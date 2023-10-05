#include <stdint.h> // uint
#include <memory.h> // memcpy
#include "bitmap.h"
#include "util.h"

// Uses absolute and encoded mode
size_t bmpRleV1(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData) {

    if (rleData == NULL) {
        throwError("some error occured");
    }

    uint32_t inPixelIndex = 0;
    uint32_t outPixelIndex = 0;
    const uint8_t bitmapPadding = getBitmapPaddingFromWidth(width);
    const uint32_t inPixelDataSize = (width + bitmapPadding) * height;
    const uint32_t inPixelDataSizeNoLastPadd = inPixelDataSize - bitmapPadding;
    const uint32_t inPixelIndexOfLastPixel = inPixelDataSizeNoLastPadd - 1;

    // loop over every pixel
    for (;inPixelIndex < inPixelDataSizeNoLastPadd; inPixelIndex++) {

        // count diffs first
        // is pixel1 and pixel2 different or is pixel1 and pixel3 different, then increase diff
        uint8_t diff = 0;
        while (diff < 255 && inPixelIndex < inPixelIndexOfLastPixel &&
            ((inPixelIndex % (width + bitmapPadding)) < width - 2 &&
                (imgIn[inPixelIndex + 0] != imgIn[inPixelIndex + 2] ||
                    imgIn[inPixelIndex + 0] != imgIn[inPixelIndex + 1])))
        {
            inPixelIndex++;
            diff++;
        }

        // if end of line set diff and inPixelIndex correctly
        char isEndOfLine = inPixelIndex % (width + bitmapPadding) == width - 2;
        if (isEndOfLine && diff > 0 && diff != 255) {
            inPixelIndex += diff == 254 ? 0 : 1;
            diff += diff == 254 ? 1 : 2;
        }
        else if (diff > 0) {
            inPixelIndex--;
        }

        if (diff > 2) {
            // absolute - write as much as 256
            rleData[outPixelIndex++] = 0;
            rleData[outPixelIndex++] = diff;
            memcpy(rleData + outPixelIndex, imgIn + inPixelIndex - (diff - 1), diff);
            outPixelIndex += diff;

            // 2 byte alignment
            if (diff % 2 == 1) {
                rleData[outPixelIndex++] = 0;
            }
        }
        else if (diff == 2 || width == 2) {
            // encode mode
            inPixelIndex -= width == 2 ? 0 : 1;
            rleData[outPixelIndex++] = 1;
            rleData[outPixelIndex++] = imgIn[inPixelIndex++];
            rleData[outPixelIndex++] = 1;
            rleData[outPixelIndex++] = imgIn[inPixelIndex];
        }
        else if (diff == 1 || width == 1) {
            // encode mode
            rleData[outPixelIndex++] = 1;
            rleData[outPixelIndex++] = imgIn[inPixelIndex];
        }
        else {
            // encode mode

            // count reps, if no diffs found
            uint8_t rep = 1;
            for (; rep < 255 && (inPixelIndex % (width + bitmapPadding)) < width - 1 &&
                inPixelIndex < inPixelIndexOfLastPixel &&
                imgIn[inPixelIndex] == imgIn[inPixelIndex + 1]; rep++, inPixelIndex++) {
            }
            rleData[outPixelIndex++] = rep;
            rleData[outPixelIndex++] = imgIn[inPixelIndex];
        }

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
