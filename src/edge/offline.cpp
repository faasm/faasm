#include <edge/edge.h>

#include <boost/filesystem.hpp>

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

    // Remove target file if it exists
    std::string functionFile = infra::getFunctionFile(call);
    if(boost::filesystem::exists(functionFile)) {
        boost::filesystem::remove(functionFile);
    }

    // Put wasm file in place
    boost::filesystem::copy(wasmFile, functionFile);

    // Compile to object file
    wasm::WasmModule::compileToObjectFile(call);
    
    return 0;
}