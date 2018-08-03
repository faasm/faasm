#include "emscripten.h"

char* EMSCRIPTEN_KEEPALIVE run() {
   char* data = "THIS IS MY DATA";

   return data;
}