#pragma once

namespace wasm {
void doMigrationPoint(int32_t entrypointFuncWasmOffset,
                      const std::string& entrypointFuncArg);
}
