#include <wasm/WasmModule.h>
#include <storage/FunctionLoader.h>

#include <util/logging.h>
#include <util/strings.h>
#include <util/files.h>

#include <WAVM/WASM/WASM.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace wasm {
    void codeGenForFile(const std::string &inputFile) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        logger->info("Generating machine code for file {}", inputFile);

        storage::FunctionLoader &functionLoader = storage::getFunctionLoader();
        std::string outputFile = inputFile + SHARED_OBJ_EXT;
        functionLoader.compileToObjectFile(inputFile, outputFile);
    }

    void codeGenForDir(const std::string &dirPath) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        logger->info("Generating machine code for dir {}", dirPath);

        path inputFilePath(dirPath);
        recursive_directory_iterator iter(inputFilePath);

        for (auto &d : iter) {
            const std::string fileName = d.path().filename().string();
            if (util::endsWith(fileName, ".so") || util::endsWith(fileName, ".wasm")) {
                codeGenForFile(d.path().string());
            }
        }
    }
}
