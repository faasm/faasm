//
// Created by Joshua Heinemann on 15.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/SGXWAMRWasmModule.h>

extern "C"{
extern sgx_status_t enclave_call_function(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const char* wasm_function_name, uint32_t wasm_function_argc, uint32_t* wasm_function_argv);
extern sgx_status_t enclave_load_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const void* wasm_opcode_ptr, uint32_t wasm_opcode_size);
extern sgx_status_t enclave_init_wamr(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val);
void ocall_printf(const char* msg){
    printf(msg);
}
}

namespace wasm{
     SGXWAMRWasmModule::SGXWAMRWasmModule(sgx_enclave_id_t* enclave_id)
    :enclave_id_ptr{enclave_id}{

    }
    void SGXWAMRWasmModule::bindToFunction(const message::Message& msg) {
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        storage::FileSystem fs;
        storage::FileLoader& fl = storage::getFileLoader();
        fs.prepareFilesystem();
        wasm_opcode = fl.loadFunctionWasm(msg);
        if((sgx_ret_val = enclave_init_wamr(*enclave_id_ptr,&ret_val)) != SGX_SUCCESS){
            UNABLE_TO_ENTER_ENCLAVE:
            printf("[Error] Unable to enter enclave (%d)\n",sgx_ret_val);
            return;
        }
        if((ret_val != FAASM_SGX_SUCCESS)){
            printf("[Error] Unable to initialize WAMR-RTE (%d)\n",ret_val);
            return;
        }
        if((sgx_ret_val = enclave_load_module(*enclave_id_ptr,&ret_val,(void*)wasm_opcode.data(),(uint32_t)wasm_opcode.size())) != SGX_SUCCESS)
            goto UNABLE_TO_ENTER_ENCLAVE;
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to load WASM module (%d)\n",ret_val);
        }
        _is_bound = true;
    }
    void SGXWAMRWasmModule::bindToFunctionNoZygote(const message::Message &msg) {
        bindToFunction(msg); //See src/wamr/WAMRWasmModule.cpp:48
    }
    bool SGXWAMRWasmModule::execute(message::Message &msg, bool force_noop) {
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        uint32_t dummy_argv[] = {
                0x0,0x0
        };//TODO: Change main to _start if stdlib support is available
        if((sgx_ret_val = enclave_call_function(*enclave_id_ptr,&ret_val,"main",2,dummy_argv)) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%d)\n",sgx_ret_val);
            return false;
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to call desired function (%d)\n",ret_val);
            return false;
        }
        return true;
    }
    const bool SGXWAMRWasmModule::isBound() {
        return _is_bound;
    }
    void SGXWAMRWasmModule::writeWasmEnvToMemory(uint32_t env_ptr, uint32_t env_buffer) {

    }
    void SGXWAMRWasmModule::writeMemoryToFd(int fd) {

    }
    void SGXWAMRWasmModule::mapMemoryFromFd() {

    }
    void SGXWAMRWasmModule::writeArgvToMemory(uint32_t wasm_argv_ptr, uint32_t wasm_argv_buffer) {

    }
    void SGXWAMRWasmModule::doSnapshot(std::ostream &out_stream) {

    }
    void SGXWAMRWasmModule::doRestore(std::istream &in_stream) {

    }
}

