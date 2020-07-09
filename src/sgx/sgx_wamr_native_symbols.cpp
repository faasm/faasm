//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <string.h>
#include <sgx/SGXWAMRWasmModule.h>


namespace wasm{
    extern int awaitChainedCall(unsigned int messageId);
    extern int awaitChainedCallOutput(unsigned int messageId, uint8_t *buffer, int bufferLen);
    extern int makeChainedCall(const std::string &functionName, int idx, const char *pyFuncName, const std::vector<uint8_t> &inputData);
}
extern "C"{
    unsigned int ocall_faasm_get_input_size(void){
        message::Message* bounded_message = wasm::getExecutingCall();
        return bounded_message->inputdata().size();
    }
    void ocall_faasm_get_input(uint8_t* buffer, unsigned int buffer_size){
        message::Message* bounded_message = wasm::getExecutingCall();
        if(bounded_message->has_inputdata()){
            const std::string& _input = bounded_message->inputdata();
            if(_input.size() > buffer_size){
                memcpy(buffer,_input.data(),buffer_size);
            }else{
                memcpy(buffer,_input.data(), _input.size());
            }
        }
        return;
    }
    void ocall_faasm_set_output(uint8_t* output, unsigned int output_size){
        message::Message* bounded_message = wasm::getExecutingCall();
        bounded_message->set_outputdata((void*)output,output_size);
        return;
    }
    unsigned int ocall_faasm_chain_function_input(const char* name, const uint8_t* input, unsigned int input_size){
        std::vector<uint8_t>_input(input, input+input_size);
        return wasm::makeChainedCall(std::string(name),0,NULL,_input);
    }
    unsigned int ocall_faasm_chain_this_input(const int idx, uint8_t* input, unsigned int input_size){
        message::Message* bounded_message = wasm::getExecutingCall();
        const std::vector<uint8_t>_input(input, input+input_size);
        return wasm::makeChainedCall(bounded_message->function(),idx,NULL,_input);
    }
    unsigned int ocall_faasm_await_call(unsigned int call_id){
        return wasm::awaitChainedCall(call_id);
    }
    unsigned int ocall_faasm_await_call_output(unsigned int call_id, uint8_t* buffer, unsigned int buffer_size){
        return wasm::awaitChainedCallOutput(call_id,buffer,buffer_size);
    }
};