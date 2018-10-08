#include <edge/edge.h>


/**
 * Script to upload functions without needing the full server up and running
 */
int main(int argc, char** argv) {
    if(argc != 4) {
        std::cerr << "Must enter a .wasm file, user and function name to upload" << std::endl;
        return 1;
    }

    char* wasmFile = argv[1];
    char* user = argv[2];
    char* function = argv[3];
    
    message::FunctionCall call;
    call.set_user(user);
    call.set_function(function);

    const std::vector<uint8_t> fileBytes = util::readFileToBytes(wasmFile);
    call.set_inputdata((char*) fileBytes.data());

    wasm::WasmModule::compileToObjectFile(call);
    
    return 0;
}