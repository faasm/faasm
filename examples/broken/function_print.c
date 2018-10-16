#include "emscripten.h"
#include <stdint.h>

/**
 * Prints the input as a string
 */
EMSCRIPTEN_KEEPALIVE
int run(uint8_t *input, int inputLength, uint8_t *output, int maxOutputLength) {

   char* strInput = (char*) input;
   printf("INPUT: %s", strInput);

   return 0;
}