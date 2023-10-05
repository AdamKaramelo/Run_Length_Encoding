#include <stdio.h> // perror
#include <stdlib.h> // stdlib
#include <errno.h>
#include "bitmap.h"
#include "util.h"

void throwError(char* errorMessage) {
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}

void throwSystemError(char* errorMessage) {
    perror(errorMessage);
    exit(1);
}

long getNumberAsLong(char* numberPtr) {
    char* numberEndPtr;
    long number = strtol(numberPtr, &numberEndPtr, 10);
    if (*numberEndPtr != '\0' || numberEndPtr == numberPtr || errno == ERANGE) {
        return -1;
    }
    return number;
}

void throwValidationError(const uint8_t code) {
    switch (code) {
    case ERROR_TOO_SMALL:
        throwError("Bitmap file is too small");
    case ERROR_INVALID_FILE_SIZE:
        throwError("The bitmap declares a wrong file size");
    case ERROR_WRONG_FILE_TYPE:
        throwError("The file is not a bitmap");
    case ERROR_WRONG_WIDTH:
        throwError("Something is wrong with the specified width of the bitmap");
    case ERROR_WRONG_HEIGHT:
        throwError("Something is wrong with the specified height of the bitmap");
    case ERROR_ALREADY_COMPRESSED:
        throwError("The Bitmap was already compressed, please use an uncompressed bitmap for compression");
    case ERROR_BITS_PER_PIXEL:
        throwError("Bits per pixel should be 8");
    case ERROR_WRONG_PLANES:
        throwError("Invalid plane number in bitmap");
    case ERROR_INVALID_INFO_HEADER_SIZE:
        throwError("Invalid information header size");
    case ERROR_CLR_USED:
        throwError("Invalid number of colors used");
    case ERROR_CLR_IMPORTANT:
        throwError("Invalid number of important colors used");
    case ERROR_WRONG_OFF_BITS:
        throwError("Invalid off bits number");
    case ERROR_NO_TOP_DOWN:
        throwError("Top Down Bitmaps are not supported");
    case ERROR_INVALID_COLOR_PALETTE_SIZE:
        throwError("Check your Bitmap, something is wrong with the size of the color palette");
    default:
        throwError("Something unexpected happened");
    }
}

void printUsage() {

    char* help =
        "bmpRle\n\n"
        "\033[1mNAME\033[0m\n"
        "\tbmpRle - compress an 8bpp bitmap file using RLE_8 compression\n\n"
        "\033[1mSYNOPSIS\033[0m\n"
        "\tbmpRle [-V=<USED_VERSION>] [-B=<AMOUNT_OF_REPETITIONS>] [-o=<OUTPUT_FILE_PATH>] [-h] <INPUT_FILE_PATH>\n\n"
        "\033[1mOPTIONS\033[0m\n"
        "\t-V\tUsed version\n\n"
        "\t-B\tAmount of repetitions\n\n"
        "\t-o\tPath to output file (default ./out.bmp)\n\n"
        "\t-h, --help\n\t\t Show help\n"
        "\033[1mINSTALLATION\033[0m\n\n"
        "\tmake\tCreate an exectuable main\n\n"
        "\033[1mSAMPLE EXECUTIONS\033[0m\n\n"
        "\t./bmpRle input.bmp\n"
        "\t./bmpRle -V0 -B0 input.bmp\n"
        "\t./bmpRle -V1 -B10 -o out.bmp input.bmp\n\n";

    fprintf(stdout, "%s", help);
}

void printUsage() {

    char* help =
        "salsa20\n\n"
        "NAME\n\n"
        "\tsalsa20 - stream sypher algorithm used to encrypt/decrypt a message\n\n"
        "SYNOPSIS\n\n"
        "\tsalsa20 [-V=<DEFINED_VERSION>] [-B=<NUMBER_OF_FUNCTION_REPETITIONS>] [-o=<OUTPUT_FILE>] [-k=<KEY>] [-iv=<NONCE>] <INPUT_FILE> [-h]\n\n"
        "OPTIONS\n\n"
        "\t-V\tUsed version, default version is 0\n\n"
        "\t-B\tAmount of repetitions of salsa20_crypt function\n\n"
        "\t-o\tPath to output file (default ./out.txt)\n\n"
        "\t-h, --help\n\t\t Display help\n"
        "EXECUTION\n\n"
        "\tmake - Compiles and creates Executable\n\n"
        "EXAMPLES\n\n"
        "\t./salsa20 -k 123 -iv 12345 input.txt\n"
        "\t./salsa20 -V0 -B10 -k 123 -iv 12345 -o output.txt input.txt\n"

    fprintf(stdout, "%s", help);
}

