//
// Created by Joshua Heinemann on 15.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/SGXWAMRWasmModule.h>

extern "C"{
void ocall_printf(const char* msg){
    printf("%s",msg);
}
}

namespace wasm{
     SGXWAMRWasmModule::SGXWAMRWasmModule(sgx_enclave_id_t* enclave_id)
    :enclave_id_ptr{enclave_id}{

    }
    void SGXWAMRWasmModule::bindToFunction(const message::Message& msg) {
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        unsigned int thread_id;
        storage::FileSystem fs;
        storage::FileLoader& fl = storage::getFileLoader();
        fs.prepareFilesystem();
        wasm_opcode = fl.loadFunctionWasm(msg);
        /*if((sgx_ret_val = sgx_wamr_enclave_init_wamr(*enclave_id_ptr,&ret_val)) != SGX_SUCCESS){
            UNABLE_TO_ENTER_ENCLAVE:
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            return;
        }
        if((ret_val != FAASM_SGX_SUCCESS)){
            printf("[Error] Unable to initialize WAMR-RTE (%#010x)\n",ret_val);
            return;
        }*/
        if((sgx_ret_val = sgx_wamr_enclave_load_module(*enclave_id_ptr,&ret_val,(void*)wasm_opcode.data(),(uint32_t)wasm_opcode.size(),&thread_id)) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            return;
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to load WASM module (%#010x)\n",ret_val);
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
        if((sgx_ret_val = sgx_wamr_enclave_call_function(*enclave_id_ptr,&ret_val,"main",2,dummy_argv,0)) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            return false;
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to call desired function (%#010x)\n",ret_val);
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

