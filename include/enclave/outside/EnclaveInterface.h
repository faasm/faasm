#pragma once

#include <enclave/error.h>

#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

// Non-faasm SGX includes
#include <sgx.h>
#include <sgx_urts.h>

namespace wasm {
/*
 * This class interfaces between an untrusted Faasm runtime running outside any
 * enclave, and a WebAssembly runtime (WAMR) running inside.
 * This class lives _outside_ the enclave, in an untrusted region, but is the
 * single entrypoint to the enclave. It is _not_ a WebAssembly module, but it
 * implements the stubs to operate with Faaslets.
 */
class EnclaveInterface final : public WasmModule
{
  public:
    explicit EnclaveInterface();

    ~EnclaveInterface() override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    bool unbindFunction();

    int32_t executeFunction(faabric::Message& msg) override;

    uint32_t growMemory(size_t nBytes) override;

    uint32_t shrinkMemory(size_t nBytes) override;

    size_t getMemorySizeBytes() override;

    uint8_t* getMemoryBase() override;

  private:
    uint32_t threadId = 0;
};
}
