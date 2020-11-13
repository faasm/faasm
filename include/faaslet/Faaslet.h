#pragma once

#include <system/NetworkNamespace.h>

#include <faabric/executor/FaabricExecutor.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>

#include <wasm/WasmModule.h>

#if(FAASM_SGX)
#include <sgx/SGXWAMRWasmModule.h>
#if(FAASM_SGX_ATTESTATION)
#include <sgx/faasm_sgx_attestation.h>
#endif
#endif


#include <string>

namespace faaslet {
class Faaslet final : public faabric::executor::FaabricExecutor
{
  public:
    explicit Faaslet(int threadIdx);

#if(FAASM_SGX)
        ~Faaslet(void);
#endif

    std::unique_ptr<wasm::WasmModule> module;

    void flush() override;

#if(FAASM_SGX)
        std::unique_ptr<wasm::SGXWAMRWasmModule> module_sgx_wamr;
#if(FAASM_SGX_ATTESTATION)
        faaslet_sgx_msg_buffer_t sgx_wamr_msg_response;
        faaslet_sgx_gp_buffer_t sgx_wamr_attestation_output, sgx_wamr_attestation_result;
#endif
#endif


  protected:
    void postBind(const faabric::Message& msg, bool force) override;

    bool doExecute(faabric::Message& call) override;

    void preFinishCall(faabric::Message& call,
                       bool success,
                       const std::string& errorMsg) override;

    void postFinish() override;

  private:
    int isolationIdx;
    std::unique_ptr<isolation::NetworkNamespace> ns;
};

void preloadPythonRuntime();

}
