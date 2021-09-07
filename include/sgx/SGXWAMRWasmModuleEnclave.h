#pragma once

#include <sgx/enclave_types.h>
#include <sgx/error.h>

#include <vector>

// WAMR includes
#include <iwasm/aot/aot_runtime.h>
#include <iwasm/include/wasm_export.h>

namespace wasm {
// This class interacts with the WebAssembly runtime (WAMR) running inside an
// SGX enclave. Instances of this class live _inside_ the enclave, in the
// trusted region.
// Support for C++ inside the enclave is limited, for information check the SGX
// SDK reference manual for Linux, under "C++ Language Features":
// https://download.01.org/intel-sgx/linux-2.5/docs/Intel_SGX_Developer_Reference_Linux_2.5_Open_Source.pdf
class SGXWAMRWasmModuleEnclave
{
  public:
    explicit SGXWAMRWasmModuleEnclave();

    ~SGXWAMRWasmModuleEnclave();

    int initialiseWamrGlobally();

    void tearDownGlobally();

  private:
    uint8_t* enclaveHeap;

    size_t enclaveHeapSize;

    // Thread Control Structure
    // TODO - is this actually needed?
    _faasm_sgx_tcs_t* tcsSlots;
};

SGXWAMRWasmModuleEnclave* getModuleEnclave();
}
