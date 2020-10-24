#include <sgx_urts.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/faasm_sgx_system.h>

#include <faabric/util/func.h>


int main(int argc, char **argv) {
    auto logger = faabric::util::getLogger();
    if (argc < 3) {
        logger->error("Too few arguments ({}). Please enter user and function", argc);
        return -1;
    }

    std::string user(argv[1]);
    std::string function(argv[2]);
    logger->info("Running {}/{} in SGX", user, function);

    // Check for SGX support
    int threadNumber = 1;
    std::string enclavePath = SGX_WAMR_ENCLAVE_PATH;
    if(argc > 3) {
        enclavePath = argv[3];
    }
    sgx::checkSgxSetup(enclavePath, threadNumber);

    // Set up the module
    sgx_enclave_id_t enclaveId = sgx::getGlobalEnclaveId();
    wasm::SGXWAMRWasmModule module(enclaveId);

    // Execute the function
    faabric::Message msg = faabric::util::messageFactory(argv[1], argv[2]);
    msg.set_issgx(true);

    module.bindToFunction(msg);
    module.execute(msg);

    logger->info("SGX run successful");
    if(msg.outputdata().empty()) {
        printf("No function output data");
    } else {
        printf("Function output data: \n%s\n", msg.outputdata().c_str());
    }
}