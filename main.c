#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include "bitmap.h"
#include "util.h"

size_t(*bmpCompressionFunctionPointer[])(const uint8_t*, size_t, size_t, uint8_t*) = { bmpRle, bmpRleV1, bmpRleV2, bmpRleEncodeV3 };
int amountOfVersions = 4;

static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}  // for array termination
};

int main(int argc, char** argv) {
    long versionNumber = 0; // -V <argument>
    char isBenchmark = 0; // true if -B option set
    long repetitions = 0; // -B <argument>
    char* outputFile = "out.bmp"; // -o <argument>
    char opt = -1;
    do {
        int option_index = 0;
        opt = getopt_long(argc, argv, "V:B:o:h", long_options, &option_index);
        switch (opt) {
        case 'V':
            versionNumber = getNumberAsLong(optarg);
            break;
        case 'B':
            isBenchmark = 1;
            repetitions = getNumberAsLong(optarg);
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'h':
            printUsage();
            exit(0);
        case '?': // invalid option
            printUsage();
            throwError("\nAn unrecognized option was used!");
        }
    } while (opt != -1); // 'opt' equals -1 (if all options are read)

    // printUsage when no arguments specified
    if (argc == 1) {
        printUsage();
        return 0;
    }
    if (versionNumber < 0 || versionNumber >= amountOfVersions) throwError("Wrong version number");
    if (repetitions < 0) throwError("Benchmark(-B) argument should be at least 0");
    if (optind >= argc) throwError("No input file found");
    if (argc > optind + 1) throwError("Too many input files");

    char* inputFile = argv[optind];

    FILE* ptrIn = fopen(inputFile, "rb"); // input
    FILE* ptrOut = fopen(outputFile, "w"); // output

    if (ptrIn == NULL) throwSystemError("Error while opening input file");
    if (ptrOut == NULL) throwSystemError("Error while opening output file");

    // get size of input file
    fseek(ptrIn, 0, SEEK_END);
    long inputSize = ftell(ptrIn);
    rewind(ptrIn);

    if (inputSize == -1) throwSystemError("Can't read size of input file");

    // allocate input buffer
    uint8_t* inputBuffer = malloc(sizeof(char) * inputSize);
    if (inputBuffer == NULL) throwSystemError("Error while allocating memory");

    size_t read = fread(inputBuffer, 1, inputSize, ptrIn);
    if (read != inputSize) throwError("Read failed");

    // close ptrIn as input is read into 'inputBuffer'
    fclose(ptrIn);

    // validate if input is bitmap
    const uint8_t code = validateBitmap(inputBuffer, inputSize);
    if (code != SUCCESS_BITMAP_VALIDATION) throwValidationError(code);

    // get output buffer to write bitmap into
    uint8_t* outputBuffer = createOutputBufferForRle(inputBuffer);
    if (outputBuffer == NULL) {
        throwSystemError("Error while allocating memory");
    }

    const uint32_t offBits = writeBitmapMetadataForRle(inputBuffer, outputBuffer);
    const uint32_t width = getWidth(outputBuffer);
    const uint32_t height = getHeight(outputBuffer);
    const uint8_t* inPixelPointer = moveToPixelData(inputBuffer);
    uint8_t* outPixelPointer = moveToPixelData(outputBuffer);
    size_t rleSize;
    size_t(*bmpRle)(const uint8_t*, size_t, size_t, uint8_t*) = bmpCompressionFunctionPointer[versionNumber];
    if (isBenchmark) {
        double totalTime = 0.0;
        struct timespec start;
        struct timespec end;

        for (int i = 1; i <= repetitions + 1; i++) {
            // measure time and execute compression function
            clock_gettime(CLOCK_MONOTONIC, &start);
            rleSize = bmpRle(inPixelPointer, width, height, outPixelPointer);
            clock_gettime(CLOCK_MONOTONIC, &end);

            double time = start.tv_sec - end.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
            totalTime += time;

            printf("%d. Run: %f\n", i, time);

        }
        printf("Average time per Run: %f\nTotal time: %f\n", totalTime / (repetitions + 1), totalTime);
    }
    else {
        // execute compression function
        rleSize = bmpRle(inPixelPointer, width, height, outPixelPointer);
    }

    uint32_t size = writeBitmapSizesForRle(outputBuffer, offBits, rleSize);

    // write compressed output
    fwrite(outputBuffer, size, 1, ptrOut);

    printf("%s", "Bitmap succesfully written\n");

    // close pointer & free buffer
    fclose(ptrOut);
    free(inputBuffer);
    free(outputBuffer);

    return 0;
}

