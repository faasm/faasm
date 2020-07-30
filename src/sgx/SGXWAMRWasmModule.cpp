//
// Created by Joshua Heinemann on 15.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/SGXWAMRWasmModule.h>
#if(FAASM_SGX_ATTESTATION)
#include <faaslet/Faaslet.h>
#include <sgx/sgx_wamr_attestation.h>
#endif

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
        storage::FileSystem fs;
        std::vector<uint8_t> wasm_opcode;
        storage::FileLoader& fl = storage::getFileLoader();
        fs.prepareFilesystem();
        wasm_opcode = fl.loadFunctionWasm(msg);
#if(FAASM_SGX_ATTESTATION)
        if((sgx_ret_val = sgx_wamr_enclave_load_module(*enclave_id_ptr,&ret_val,(void*)wasm_opcode.data(),(uint32_t)wasm_opcode.size(),&thread_id, &get_sgx_msg_buffer()->buffer_ptr)) != SGX_SUCCESS){
#else
        if((sgx_ret_val = sgx_wamr_enclave_load_module(*enclave_id_ptr,&ret_val,(void*)wasm_opcode.data(),(uint32_t)wasm_opcode.size(),&thread_id)) != SGX_SUCCESS){
#endif
                printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
                return;
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to load WASM module (%#010x)\n",ret_val);
            return;
        }
        _is_bound = true;
    }
    void SGXWAMRWasmModule::bindToFunctionNoZygote(const message::Message &msg) {
        bindToFunction(msg); //See src/wamr/WAMRWasmModule.cpp:48
    }
    const bool SGXWAMRWasmModule::unbindFunction(void){
            if(_is_bound){
                sgx_status_t sgx_ret_val;
                faasm_sgx_status_t ret_val;
                if((sgx_ret_val = sgx_wamr_enclave_unload_module(*enclave_id_ptr,&ret_val,thread_id)) != SGX_SUCCESS){
                    printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
                    return false;
                }
                if(ret_val != FAASM_SGX_SUCCESS){
                    printf("[Error] Unable to unbind function (%#010x)\n",ret_val);
                    return false;
                }
                return true;
            }
            return true;
     }
    bool SGXWAMRWasmModule::execute(message::Message &msg, bool force_noop) {
         if(!_is_bound){
             printf("[Error] Unable to call desired function (%#010x)\n",FAASM_SGX_WAMR_MODULE_NOT_BOUNDED);
             msg.set_returnvalue(FAASM_SGX_WAMR_MODULE_NOT_BOUNDED);
             return false;
         }
        sgx_status_t sgx_ret_val;
        faasm_sgx_status_t ret_val;
        wasm::setExecutingCall(const_cast<message::Message*>(&msg));
        if((sgx_ret_val = sgx_wamr_enclave_call_function(*enclave_id_ptr,&ret_val,thread_id, msg.idx())) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            msg.set_returnvalue(FAASM_SGX_UNABLE_TO_ENTER_ENCLAVE);
            return false;
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to call desired function (%#010x)\n",ret_val);
            msg.set_returnvalue(ret_val);
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

