/*
 * SIMD Optimized Implementation of RLE
 */

#include <stdint.h> // uint
#include <memory.h> // memcpy
#include <emmintrin.h> // SIMD
#include "bitmap.h"
#include "util.h"

uint32_t writeData(const uint8_t* inputData, uint8_t* rleData, uint8_t isDiff, uint8_t count) {

    if (isDiff && count > 2) {
        // write in absolute mode [00 count pixel1 pixel2]

        uint8_t diffUnaligned = count % 16;
        uint8_t diffAligned = count - diffUnaligned;
        *rleData++ = 0;
        *rleData++ = count;
        for (uint8_t i = 0; i < diffAligned; i += 16) {
            __m128i pixelsIn = _mm_loadu_si128((const __m128i_u*)(inputData + i));
            _mm_storeu_si128((__m128i_u*)(rleData + i), pixelsIn);
        }
        rleData += diffAligned;
        memcpy(rleData, inputData + diffAligned, diffUnaligned);
        rleData += diffUnaligned;

        // set 2-byte alignment
        if (count % 2 == 1) {
            *rleData++ = 0;
            return count + 3;
        }
        return count + 2;
    }
    else if (isDiff && count == 2) {
        // write in encoded mode [count pixel1] [count pixel2]
        *rleData++ = 1;
        *rleData++ = *inputData;
        *rleData++ = 1;
        *rleData++ = *(inputData + 1);
        return 4;
    }
    else {
        // write in encoded mode [count pixel1]
        *rleData++ = count;
        *rleData++ = *inputData;
        return 2;
    }
}

// Uses absolute and encoded mode
size_t bmpRle(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData) {

    uint32_t inPixelIndex = 0;
    uint32_t outPixelIndex = 0;

    // repetitions of pixels
    uint8_t reps = 0;
    // different pixels
    uint8_t diff = 0;

    const uint8_t bitmapPadding = getBitmapPaddingFromWidth(width);
    // to compare intervals of [0,15] with [1,16] of pixel data, we need to make sure that the '16'th index is available
    int32_t countPCMP = width / 16;
    if (width % 16 == 0 && countPCMP > 0) {
        countPCMP--;
    }
    // count of pixel data, that is not compared with SIMD
    const int32_t countRest = width - countPCMP * 16;

    for (uint32_t i = 1; i <= height; i++) {
        uint32_t currentLineIndex = inPixelIndex;

        // compress 16 byte aligned segments of pixels 
        for (int j = 0; j < countPCMP; j++) { // loops through one scan line
            // load and compare blocks of pixel data in intervals of [0,15] to [1,16] using 'cmpeq'
            __m128i pixels = _mm_loadu_si128((const __m128i_u*)&imgIn[currentLineIndex + j * 16]);
            __m128i pixels2 = _mm_loadu_si128((const __m128i_u*)&imgIn[currentLineIndex + j * 16 + 1]);
            __m128i mask = _mm_cmpeq_epi8(pixels, pixels2);

            // maskArr values:
            // '0xff' -> 2 pixel are equal
            // '0' -> 2 pixel are unequal
            __attribute__((aligned(16))) uint8_t maskArr[16];
            _mm_storeu_si128((__m128i_u*)maskArr, mask);


            // count diffs, then count reps, and write them when necessary
            for (int k = 0; k < 16; k++) {

                // write if maximum of reps and diffs will be exceeded
                if (reps == 254) {
                    reps++;
                    outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 0, reps);
                    inPixelIndex += reps;
                    reps = 0;
                }
                else if (diff == 255) {
                    outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                    inPixelIndex += diff;
                    diff = 0;
                }

                if (maskArr[k] == 0) {
                    // if 2 pixel unequal -> increase diff
                    diff++;

                    if (reps >= 2) {
                        // write previously counted reps, before counting diffs
                        // if reps equal 2 then three pixel were equal
                        reps++;
                        outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 0, reps);
                        inPixelIndex += reps;
                        reps = 0;
                        diff--;
                    }
                    else if (reps == 1) {
                        if (diff == 255) {
                            // write diff if necessary
                            outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                            inPixelIndex += diff;
                            diff = 0;
                        }
                        // if rep follows diff, only 2 pixel were equal, not three - use diff instead of rep
                        diff += reps;
                        reps = 0;
                    }
                    continue;
                }
                else if (maskArr[k] == 0xff) {
                    // increase reps if 2 pixel equal
                    reps++;
                    if (diff > 0 && reps >= 2) {
                        // write previously counted diffs, before counting reps
                        outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                        inPixelIndex += diff;
                        diff = 0;
                    }
                    continue;
                }
            }
        }

        for (int k = 1; k < countRest; k++) { // 1 1 2 2 /end of file 

            // write if maximum of reps and diffs will be exceeded
            if (reps == 254) {
                outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 0, reps + 1);
                inPixelIndex += reps + 1;
                reps = 0;
            }
            else if (diff == 255) {
                outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                inPixelIndex += diff;
                diff = 0;
            }
            if (imgIn[currentLineIndex + countPCMP * 16 + k] != imgIn[currentLineIndex + countPCMP * 16 + k - 1]) {
                // if 2 pixel unequal -> increase diff
                diff++;
                if (reps >= 2) {
                    // write previously counted reps, before counting diffs
                    // if reps equal 2 then three pixel were equal
                    outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 0, reps + 1);
                    inPixelIndex += reps + 1;
                    reps = 0;
                    diff--;
                }
                else if (reps == 1) {
                    if (diff == 255) {
                        // write diff if necessary
                        outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                        inPixelIndex += diff;
                        diff = 0;
                    }
                    // if rep follows diff, only 2 pixel were equal, not three - use diff instead of rep
                    diff += reps;
                    reps = 0;
                }
                continue;
            }
            else if (imgIn[currentLineIndex + countPCMP * 16 + k] == imgIn[currentLineIndex + countPCMP * 16 + k - 1]) {
                // increase reps if 2 pixel equal
                reps++;
                if (diff > 0 && reps >= 2) {
                    // write previously counted diffs, before counting reps
                    outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                    inPixelIndex += diff;
                    diff = 0;
                }
                continue;
            }
        }


        if (reps >= 2) {
            // if at least three pixel equal write reps

            // if reps would exceed end of line, don't increase
            reps += ((inPixelIndex + reps) % (width + bitmapPadding)) == width ? 0 : 1;
            outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 0, reps);
            inPixelIndex += reps;
            reps = 0;
        }
        else {
            if (reps == 1) {
                if (diff == 255) {
                    // write diff if necessary
                    outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff);
                    inPixelIndex += diff;
                    diff = 0;
                }
                diff += reps;
                reps = 0;
            }

            outPixelIndex += writeData(imgIn + inPixelIndex, rleData + outPixelIndex, 1, diff + 1);
            inPixelIndex += diff + 1;
            diff = 0;
        }

        if (height == i) {
            // end of file
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            rleData[outPixelIndex++] = END_OF_BITMAP_BYTE;
        }
        else {
            // end of line
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            rleData[outPixelIndex++] = END_OF_LINE_BYTE;
            inPixelIndex += bitmapPadding;

        }
    }
    return outPixelIndex;
}
