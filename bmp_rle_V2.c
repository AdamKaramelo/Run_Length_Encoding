#include <stdint.h>
#include <memory.h>
#include "bitmap.h"
#include "util.h"

// Uses absolute and encoded mode
size_t bmpRleV2(const uint8_t* imgIn, size_t width, size_t height, uint8_t* rleData)
{
    const uint8_t bitmapPadding = getBitmapPaddingFromWidth(width);
    const uint8_t* inPixelPointer = imgIn;
    const uint32_t inPixelDataSize = (width + bitmapPadding) * height;
    uint32_t inPixelIndex = 0;
    uint32_t outPixelIndex = 0;
    const uint32_t inPixelDataSizeNoLastPadd = inPixelDataSize - bitmapPadding;
    const uint32_t inPixelIndexOfLastPixel = inPixelDataSizeNoLastPadd - 1;
    uint8_t* outPixelPointer = rleData;
    uint8_t rep = 1;
    uint8_t diff = 1;
    uint8_t isEndOfLine;
    uint8_t isEndOfFile;
    if (outPixelPointer == NULL)
    {
        throwError("some error occured");
    }

    while (inPixelIndex < inPixelDataSizeNoLastPadd)
    {   //reset rep and diff
        rep = 1;
        diff = 1;
        /*
        * Loop and increase rep when we see two of the same pixel. Increase diff when different pixels
        * Also never compare with padding byte to avoid comparing pixel 0 with padding 0
        * Never go over last pixel
        */
        while (rep < 255 && diff < 255 && (inPixelIndex % (width + bitmapPadding)) < width - 1
            && inPixelIndex < inPixelIndexOfLastPixel)
        {
            if (inPixelPointer[inPixelIndex] == inPixelPointer[inPixelIndex + 1])
            {
                rep++;
                diff++; // diff also increase in case we have two same pixels and then one different (1 2 2 1)
                inPixelIndex++;
            }
            else if (rep >= 3)
            { // in case we reach rep threshhold and see diff pixels we write reps and diffs that stood in front of the reps
                break;
            }
            else
            {
                diff++;
                rep = 1; // always reset rep when 
                inPixelIndex++;
            }
        }

        isEndOfLine = (inPixelIndex % (width + bitmapPadding)) == width - 1;
        isEndOfFile = inPixelIndex == inPixelIndexOfLastPixel;

        //here we have diff and rep pixels; write diff pixels first then rep
        if (rep >= 3)
        { // we might or might not have some #diff pixels which we write first
            diff = diff - rep; // amount of #diff pixels update
            inPixelIndex -= (rep + diff) - 1; //fix input index to point to first diff pixel
            if (diff < 3 && diff > 0)
            {
                // write encoded #dif pixels
                for (;diff > 0; diff--)
                {
                    outPixelPointer[outPixelIndex++] = 1;
                    outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                }
            }
            else if (diff != 0 && diff >= 3)
            {
                // write absolute #diff pixels;
                uint8_t odd = diff % 2;
                outPixelPointer[outPixelIndex++] = 00;
                outPixelPointer[outPixelIndex++] = diff;
                for (; diff > 0; diff--)
                    outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                if (odd) outPixelPointer[outPixelIndex++] = 00; //padding byte for absolute mode in case its odd number of diff pixels
            }
            // write encoded #rep pixels;
            outPixelPointer[outPixelIndex++] = rep;
            outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex];
            inPixelIndex += rep - 1;
            if (isEndOfFile)
            { // end of file
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                outPixelPointer[outPixelIndex++] = END_OF_BITMAP_BYTE;
                break;
            }
            else if (isEndOfLine)
            { // end of line
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                inPixelIndex += (bitmapPadding + 1); // go to next scan line
            }
            else
            {
                inPixelIndex++; // here inPixelIndex is pointing to next pixel
            }
        }
        else if (diff >= 3) // here we can only have diff pixels
        {
            uint8_t odd = diff % 2;
            //if end of line comes next, we write the amount of diff accordingly
            if (isEndOfLine || isEndOfFile)
            {
                inPixelIndex -= diff - 1;
                outPixelPointer[outPixelIndex++] = 00;
                outPixelPointer[outPixelIndex++] = diff;
                for (; diff > 1; diff--)
                    outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex]; // in case inPixelIndex is last pixel we dont increment so we dont provoke seg fault
                if (odd) outPixelPointer[outPixelIndex++] = 00;
                if (isEndOfFile)
                {
                    outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                    outPixelPointer[outPixelIndex++] = END_OF_BITMAP_BYTE;
                    break;
                }
                else if (isEndOfLine)
                {
                    outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                    outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                    inPixelIndex += (bitmapPadding + 1); // go to next scan line
                }
            }
            // if end of line or file comes after next 2 pixels its worth to check for the upcoming pixels in case they are
            // the same as the last diff pixels we are about to write; so we can write them together in encoded mode in next loop
            else if ((inPixelIndex % (width + bitmapPadding)) == width - 2 || inPixelIndex == inPixelIndexOfLastPixel - 1)
            {
                if (rep == 2 && inPixelPointer[inPixelIndex] == inPixelPointer[inPixelIndex + 1])
                {
                    inPixelIndex -= diff - 1;
                    diff -= 2;
                    outPixelPointer[outPixelIndex++] = 00;
                    outPixelPointer[outPixelIndex++] = diff;
                    for (;diff > 0; diff--)
                        outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];

                    if (odd) outPixelPointer[outPixelIndex++] = 00;
                }
                else
                {
                    inPixelIndex -= diff - 1;
                    outPixelPointer[outPixelIndex++] = 00;
                    outPixelPointer[outPixelIndex++] = diff;
                    for (; diff > 0; diff--)
                        outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                    if (odd) outPixelPointer[outPixelIndex++] = 00;
                }
            }
            else
            { // not end of line or file after next 2 pixels
                if (rep == 2 && inPixelPointer[inPixelIndex] == inPixelPointer[inPixelIndex + 1])
                {
                    inPixelIndex -= diff - 1;
                    diff -= 2;
                    outPixelPointer[outPixelIndex++] = 00;
                    outPixelPointer[outPixelIndex++] = diff;
                    for (; diff > 0; diff--)
                        outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                    if (odd) outPixelPointer[outPixelIndex++] = 00;
                }
                else if (rep == 1 && inPixelPointer[inPixelIndex] == inPixelPointer[inPixelIndex + 1] && inPixelPointer[inPixelIndex] == inPixelPointer[inPixelIndex + 2])
                { // if next 2 pixels are equal to first
                    inPixelIndex -= diff - 1;
                    diff -= 1;
                    odd = diff % 2;
                    outPixelPointer[outPixelIndex++] = 00;
                    outPixelPointer[outPixelIndex++] = diff;
                    for (; diff > 0; diff--)
                        outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                    if (odd) outPixelPointer[outPixelIndex++] = 00;
                }
                else
                {
                    inPixelIndex -= diff - 1;
                    outPixelPointer[outPixelIndex++] = 00;
                    outPixelPointer[outPixelIndex++] = diff;
                    for (; diff > 0; diff--)
                        outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                    if (odd) outPixelPointer[outPixelIndex++] = 00;
                }
            }
        }
        else
        {   // here we only have 1 or 2 pixels before end of line or file
            // write on encoded mode those 1 or 2 pixels;
            if (rep == 2) {
                outPixelPointer[outPixelIndex++] = rep;
                outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex];
            }
            else {
                inPixelIndex -= diff - 1;
                for (;diff > 1; diff--)
                {
                    outPixelPointer[outPixelIndex++] = 1; // rep is one because pixels are different
                    outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex++];
                }
                outPixelPointer[outPixelIndex++] = 1;
                outPixelPointer[outPixelIndex++] = inPixelPointer[inPixelIndex];
            }
            if (isEndOfFile)
            { // end of file
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                outPixelPointer[outPixelIndex++] = END_OF_BITMAP_BYTE;
                break;
            }
            else if (isEndOfLine)
            { // end of line
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                outPixelPointer[outPixelIndex++] = END_OF_LINE_BYTE;
                inPixelIndex += (bitmapPadding + 1); // go to next scan line
            }
        }
    }
    return outPixelIndex;
}
