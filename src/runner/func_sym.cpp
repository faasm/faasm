#include <fstream>
#include <iomanip>

#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

int main(int argc, char* argv[])
{
    faabric::util::initLogging();

    if (argc < 3) {
        SPDLOG_ERROR("Must provide user and function name");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];

    faabric::Message call;
    call.set_user(user);
    call.set_function(function);

    SPDLOG_INFO("Disassembling function {}/{}", user, function);

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    storage::FileLoader& loader = storage::getFileLoader();
    std::string outPath = loader.getFunctionSymbolsFile(call);
    std::ofstream outfile;
    outfile.open(outPath, std::ios::out);

    for (auto const& p : disasMap) {
        // Separator needs to be something not found in the symbols themselves
        outfile << p.first << " = " << p.second << std::endl;
    }

    outfile.close();

    SPDLOG_INFO("Output at {}", outPath);

    return 0;
}
