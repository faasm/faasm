#include <cstdio>

#include <sgx/SGXWAMRWasmModule.h>

extern "C" {
void ocall_printf(const char *msg) {
    printf("%s", msg);
}
}

namespace wasm {
    SGXWAMRWasmModule::SGXWAMRWasmModule(sgx_enclave_id_t *enclaveId) : enclaveIdPtr(enclaveId) {

    }

    SGXWAMRWasmModule::~SGXWAMRWasmModule() {
        if (!unbindFunction()) {
            printf("[Error] SGX Faaslet destruction failed\n");
        }
    }

    void SGXWAMRWasmModule::bindToFunction(const faabric::Message &msg) {
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        storage::FileSystem fs;
        storage::FileLoader &fl = storage::getFileLoader();

        fs.prepareFilesystem();
        wasmOpcode = fl.loadFunctionWasm(msg);

        // Set up the enclave
        sgx_status_t status = sgx_ret_val = sgx_wamr_enclave_load_module(
                *enclaveIdPtr,
                &ret_val,
                (void *) wasmOpcode.data(),
                (uint32_t) wasmOpcode.size(),
                &threadId
        );

        if (status != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgx_ret_val);
            return;
        }
        if (ret_val != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to load WASM module (%#010x)\n", ret_val);
            return;
        }

        _isBound = true;
        wasm::setExecutingCall(const_cast<faabric::Message *>(&msg));
    }

    void SGXWAMRWasmModule::bindToFunctionNoZygote(const faabric::Message &msg) {
        bindToFunction(msg); //See src/wamr/WAMRWasmModule.cpp:48
    }

    const bool SGXWAMRWasmModule::unbindFunction() {
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        if ((sgx_ret_val = sgx_wamr_enclave_unload_module(*enclaveIdPtr, &ret_val, threadId)) != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgx_ret_val);
            return false;
        }
        if (ret_val != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to unbind function (%#010x)\n", ret_val);
            return false;
        }
        return true;
    }

    bool SGXWAMRWasmModule::execute(faabric::Message &msg, bool forceNoop) {
        if (!_isBound) {
            printf("[Error] Unable to call desired function (%#010x)\n", FAASM_SGX_WAMR_MODULE_NOT_BOUNDED);
            msg.set_returnvalue(FAASM_SGX_WAMR_MODULE_NOT_BOUNDED);
            return false;
        }

        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        uint32_t dummy_argv[] = {
                0x0, 0x0
        };

        if ((sgx_ret_val = sgx_wamr_enclave_call_function(*enclaveIdPtr, &ret_val, threadId, msg.idx())) !=
            SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgx_ret_val);
            msg.set_returnvalue(FAASM_SGX_UNABLE_TO_ENTER_ENCLAVE);
            return false;
        }
        if (ret_val != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to call desired function (%#010x)\n", ret_val);
            msg.set_returnvalue(ret_val);
            return false;
        }
        return true;
    }

    const bool SGXWAMRWasmModule::isBound() {
        return _isBound;
    }
}
