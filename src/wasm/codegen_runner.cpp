#include <wasm/codegen.h>

#include <util/logging.h>
#include <boost/filesystem.hpp>


using namespace boost::filesystem;

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->warn("No input provided to codegen");
        return 0;
    }

    std::string inputFile = argv[1];
    path inputFilePath(inputFile);

    if (is_directory(inputFilePath)) {
        wasm::codeGenForDir(inputFile);
    } else {
        wasm::codeGenForFile(inputFile);
    }
}
