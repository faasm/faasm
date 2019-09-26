#include <wasm/WasmModule.h>
#include <storage/FunctionLoader.h>

#include <util/logging.h>
#include <util/strings.h>
#include <util/files.h>

#include <WAVM/WASM/WASM.h>
#include <boost/filesystem.hpp>
#include <util/config.h>
#include <util/locks.h>
#include <util/environment.h>

using namespace boost::filesystem;

namespace wasm {
    void codeGenForFile(const std::string &inputFile) {
        directory_entry f(inputFile);
        const std::string fileName = f.path().filename().string();

        if (!util::endsWith(fileName, ".so") && !util::endsWith(fileName, ".wasm")) {
            // Not a file we can generate code for
            return;
        }

        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        logger->info("Generating machine code for file {}", inputFile);

        storage::FunctionLoader &functionLoader = storage::getFunctionLoader();
        functionLoader.compileToObjectFile(inputFile);
    }

    void codeGenForDir(const std::string &dirPath) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        logger->info("Generating machine code for dir {}", dirPath);

        // Output dir will be original prefixed with object path
        util::SystemConfig &conf = util::getSystemConfig();
        logger->info("Output will live at {}", conf.objectFileDir);

        path inputFilePath(dirPath);
        recursive_directory_iterator iter(inputFilePath), end;
        std::mutex mx;

        // Run multiple threads to do codegen
        unsigned int nThreads = util::getUsableCores();
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < nThreads; i++) {
            threads.emplace_back([&iter, &mx, &end, &logger] {
                logger->info("Spawning codgen thread");

                while (true) {
                    std::string thisPath;

                    {
                        // Get lock
                        util::UniqueLock lock(mx);

                        // Check if we've got more to do
                        if (iter == end) {
                            logger->info("Codegen thread finished");
                            break;
                        }

                        // Get path and increment
                        thisPath = iter->path().string();
                        iter++;
                    }

                    // Run codegen
                    codeGenForFile(thisPath);
                }
            });
        }

        for(auto &t : threads) {
            if(t.joinable()) {
                t.join();
            }
        }
    }
}
