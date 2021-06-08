#include <fstream>
#include <iomanip>

#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

int main(int argc, char* argv[])
{

    if (argc < 3) {
        SPDLOG_ERROR("Must provide user and function name");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];

    faabric::Message call;
    call.set_user(user);
    call.set_function(function);

    logger->info("Disassembling function {}/{}", user, function);

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    std::string outPath = conf::getFunctionSymbolsFile(call);
    std::ofstream outfile;
    outfile.open(outPath, std::ios::out);

    for (auto const& p : disasMap) {
        // Separator needs to be something not found in the symbols themselves
        outfile << p.first << " = " << p.second << std::endl;
    }

    outfile.close();

    logger->info("Output at {}", outPath);

    return 0;
}
