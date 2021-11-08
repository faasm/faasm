#pragma once

#include <sgx/WAMREnclave.h>
#include <sgx/attestation.h>
#include <sgx/error.h>
#include <sgx/system.h>
#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

// Non-faasm SGX includes
#include <sgx.h>
#include <sgx_urts.h>

namespace wasm {
// This class interfaces between an untrusted Faasm runtime running outside any
// enclave, and a WebAssembly runtime (WAMR) running inside.
// This class lives _outside_ the enclave, in an untrusted region, but is the
// single entrypoint to the enclave.
class SGXWAMRWasmModule final : public WasmModule
{
  public:
    explicit SGXWAMRWasmModule();

    ~SGXWAMRWasmModule() override;

    static void clearCaches();

    // ----- Module lifecycle -----
    void doFunctionLoad(std::shared_ptr<sgx::WAMREnclave> wamrEnclave,
                        const faabric::Message& msg);

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    void reset(faabric::Message& msg, const std::string& snapshotKey) override;

    int32_t executeFunction(faabric::Message& msg) override;

    // ----- Memory management -----
    uint32_t growMemory(uint32_t nBytes) override;

    uint32_t shrinkMemory(uint32_t nBytes) override;

  private:
    std::shared_ptr<sgx::WAMREnclave> wamrEnclave = nullptr;

    std::string boundFuncStr;
};
}
