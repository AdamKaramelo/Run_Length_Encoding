/*
 * Header file for util.c
 */

void throwError(char* errorMessage);
void throwSystemError(char* errorMessage);
void throwValidationError(const uint8_t code);
void printUsage();
long getNumberAsLong(char* numberPtr);
