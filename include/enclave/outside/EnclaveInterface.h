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

    void reset(faabric::Message& msg, const std::string& snapshotkey) override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    int32_t executeFunction(faabric::Message& msg) override;

    size_t getMemorySizeBytes() override;

    size_t getMaxMemoryPages() override;

    uint8_t* getMemoryBase() override;

    sgx_enclave_id_t getEnclaveId() const { return enclaveId; }

    uint32_t interfaceId = 0;

  private:
    // ID of the enclave associated with this interface
    sgx_enclave_id_t enclaveId;
};

EnclaveInterface* getExecutingEnclaveInterface();
}
