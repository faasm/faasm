#pragma once

#include <wasm_export.h>
#include <wasm_native.h>

uint32_t getFaasmApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmWasiApi(NativeSymbol** nativeSymbols);

void initialiseSGXWAMRNatives();
