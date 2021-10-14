#pragma once

#include <sgx/ecalls.h>
#include <sgx/error.h>

#include <sgx_urts.h>

#include <memory>
#include <vector>

namespace sgx {
// This class encapsulates the logic to interact with an SGX enclave.
// 11/10/2021 - Currently we only spawn one enclave per Faasm instance. Thus,
// there will only be one instance of this class.
class WAMREnclave
{
  public:
    WAMREnclave();

    // Delete the copy constructor to prevent not getting the object by
    // reference through the getter
    WAMREnclave(const WAMREnclave&) = delete;

    void init();

    // As enclaves are persited across function executions, we must ensure we
    // clean everything up without relying on the constructor.
    void tearDown();

    bool isSetUp();

    bool isWasmLoaded();

    bool isWasmLoaded(const std::vector<uint8_t>& dataToLoad);

    sgx_enclave_id_t getId();

    void loadWasmModule(std::vector<uint8_t>& data, uint32_t* slot);

    void unloadWasmModule(const uint32_t slot);

    void callMainFunction(const uint32_t slot);

  private:
    sgx_enclave_id_t enclaveId = 0;

    std::vector<uint8_t> loadedBytes;
};

std::shared_ptr<WAMREnclave> acquireGlobalWAMREnclave();

void releaseGlobalWAMREnclave();
}
