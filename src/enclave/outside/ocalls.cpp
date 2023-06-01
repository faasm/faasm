#include <faabric/scheduler/ExecutorContext.h>

#include <enclave/outside/EnclaveInterface.h>
#include <wasm/chaining.h>

#include <cstdio>
#include <cstring>

using namespace faabric::scheduler;

extern "C"
{
    int ocallFaasmReadInput(uint8_t* buffer, unsigned int bufferSize)
    {
        faabric::Message* msg = &ExecutorContext::get()->getMsg();

        unsigned long inputLen = msg->inputdata().size();

        if (bufferSize == 0) {
            return (int)inputLen;
        }

        if (inputLen > 0) {
            const std::string& _input = msg->inputdata();
            if (_input.size() > bufferSize) {
                memcpy(buffer, _input.data(), bufferSize);
                return (int)bufferSize;
            } else {
                memcpy(buffer, _input.data(), _input.size());
                return (int)inputLen;
            }
        }

        return 0;
    }

    void ocallFaasmWriteOutput(char* output, unsigned int outputSize)
    {
        ExecutorContext::get()->getMsg().set_outputdata(output, outputSize);
    }

    unsigned int ocallFaasmChainName(const char* name,
                                     const uint8_t* input,
                                     unsigned int inputSize)
    {
        std::vector<uint8_t> _input(input, input + inputSize);
        return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
    }

    unsigned int ocallFaasmChainPtr(int wasmFuncPtr,
                                    uint8_t* input,
                                    unsigned int inputSize)
    {
        const std::vector<uint8_t> _input(input, input + inputSize);
        return wasm::makeChainedCall(
          ExecutorContext::get()->getMsg().function(),
          wasmFuncPtr,
          nullptr,
          _input);
    }

    unsigned int ocallFaasmAwaitCall(unsigned int callId)
    {
        return wasm::awaitChainedCall(callId);
    }

    unsigned int ocallFaasmAwaitCallOutput(unsigned int callId,
                                           char* buffer,
                                           unsigned int bufferSize)
    {
        return wasm::awaitChainedCallOutput(callId, buffer, bufferSize);
    }

    int32_t ocallSbrk(int32_t increment)
    {
        SPDLOG_TRACE("S - __sbrk - {}", increment);
        SPDLOG_WARN("SGX-WAMR sbrk does not allocate more memory");
        return 0;
    }

    // ---------------------------------------
    // Logging
    // ---------------------------------------

    void ocallLogDebug(const char* msg) { SPDLOG_DEBUG("[enclave] {}", msg); }

    void ocallLogError(const char* msg) { SPDLOG_ERROR("[enclave] {}", msg); }
}
