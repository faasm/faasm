#include <sgx_urts.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/sgx_system.h>

#include <faabric/util/func.h>
#include <cstdio>


int main(int argc, char **argv) {
    if (argc < 3) {
        printf("[Error] Too few arguments. Please enter user and function\n");
        return -1;
    }

    // Check for SGX support
    int threadNumber = 1;
    if(argc > 3) {
        sgx::checkSgxSetup(argv[3], threadNumber);
    } else {
        sgx::checkSgxSetup(SGX_WAMR_ENCLAVE_PATH, threadNumber);
    }

    // Set up the module
    sgx_enclave_id_t enclaveId = sgx::getGlobalEnclaveId();
    wasm::SGXWAMRWasmModule module(enclaveId);

    // Execute the function
    faabric::Message msg = faabric::util::messageFactory(argv[1], argv[2]);
    msg.set_issgx(true);
    module.bindToFunction(msg);
    module.execute(msg);

    // Tidy up
    sgx::tearDownEnclave();
}