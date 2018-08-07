#include "emscripten.h"

EMSCRIPTEN_KEEPALIVE
int run(int* input, int inputLength, int* output, int outputLength) {

   for(int i=0; i < 5; i++) {
      output[i] = i;
   }

   return 0;
}