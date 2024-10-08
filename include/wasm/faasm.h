#pragma once

#include <cstdint>

namespace wasm {
void doFaasmSmCriticalLocal();

void doFaasmSmCriticalLocalEnd();

void doFaasmSmReduce(int32_t varPtr,
                     int32_t varType,
                     int32_t reduceOp,
                     int32_t currentBatch);

int32_t doFaasmReadInput(char* inBuff, int32_t inBuffLen);
}
