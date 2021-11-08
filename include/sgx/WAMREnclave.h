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

    void initSgxEnclave();

    void tearDownSgxEnclave();

    bool isSgxEnclaveSetUp();

    sgx_enclave_id_t getId();

    void loadWasmModule(const std::string& funcStr, std::vector<uint8_t>& data);

    void callMainFunction(const std::string& funcStr);

  private:
    sgx_enclave_id_t enclaveId = 0;
};

std::shared_ptr<WAMREnclave> acquireGlobalWAMREnclaveLock();

void releaseGlobalWAMREnclaveLock();
}
