#include <sgx/SGXWAMRWasmModuleEnclave.h>
#include <sgx/enclave_config.h>
#include <sgx/native_symbols_wrapper.h>

#include <mutex>

namespace wasm {
static bool enclaveWamrInitialised = false;
std::mutex enclaveInitMx;

SGXWAMRWasmModuleEnclave* getModuleEnclave()
{
    static SGXWAMRWasmModuleEnclave moduleEnclave;

    if (moduleEnclave.initialiseWamrGlobally() != FAASM_SGX_SUCCESS) {
        return nullptr;
    }

    return &moduleEnclave;
}

SGXWAMRWasmModuleEnclave::SGXWAMRWasmModuleEnclave()
  : enclaveHeapSize(FAASM_SGX_WAMR_HEAP_SIZE)
{
    enclaveHeap = new uint8_t[enclaveHeapSize];
    tcsSlots = new _faasm_sgx_tcs_t[FAASM_SGX_INIT_TCS_SLOTS];
}

SGXWAMRWasmModuleEnclave::~SGXWAMRWasmModuleEnclave()
{
    this->tearDownGlobally();
}

void SGXWAMRWasmModuleEnclave::tearDownGlobally()
{
    delete[] enclaveHeap;
    delete[] tcsSlots;
}

int SGXWAMRWasmModuleEnclave::initialiseWamrGlobally()
{
    if (enclaveWamrInitialised) {
        return FAASM_SGX_SUCCESS;
    } else {
        enclaveInitMx.lock();

        if (enclaveWamrInitialised) {
            return FAASM_SGX_SUCCESS;
        }

        // Initialise WAMR runtime
        RuntimeInitArgs wamr_rte_args;
        memset(&wamr_rte_args, 0x0, sizeof(wamr_rte_args));
        wamr_rte_args.mem_alloc_type = Alloc_With_Pool;
        wamr_rte_args.mem_alloc_option.pool.heap_buf =
          reinterpret_cast<void*>(this->enclaveHeap);
        wamr_rte_args.mem_alloc_option.pool.heap_size = this->enclaveHeapSize;

        if (!wasm_runtime_full_init(&wamr_rte_args)) {
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Initialise native symbols
        // TODO - fix how we do this
        /*
        wasm_native_register_natives(
          "env", faasm_sgx_native_symbols, FAASM_SGX_NATIVE_SYMBOLS_LEN);

        wasm_native_register_natives("wasi_snapshot_preview1",
                                     faasm_sgx_wasi_symbols,
                                     FAASM_SGX_WASI_SYMBOLS_LEN);
        */
        initialiseWAMRNatives();

#if (FAASM_SGX_ATTESTATION)
        // Initialize necessary attestation stuff if FAASM-SGX Attestation
        // extension is enabled
        sgx_status_t sgx_return_value;
        faasm_sgx_status_t return_value;

        if ((sgx_return_value = ocall_init_crt(&return_value)) != SGX_SUCCESS) {
            return FAASM_SGX_OCALL_ERROR(sgx_return_value);
        }

        if (return_value != FAASM_SGX_SUCCESS) {
            return return_value;
        }
#endif

        enclaveWamrInitialised = true;
        enclaveInitMx.unlock();
    }

    return FAASM_SGX_SUCCESS;
}
}
