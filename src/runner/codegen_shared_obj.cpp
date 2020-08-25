#include <faabric/util/logging.h>
#include <boost/filesystem.hpp>
#include <storage/FileLoader.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/string_tools.h>

using namespace boost::filesystem;


void codegenForDirectory(std::string &inputPath) {
    const std::shared_ptr<spdlog::logger> logger = faabric::util::getLogger();
    logger->info("Running codegen on directory {}", inputPath);
    storage::FileLoader &loader = storage::getFileLoader();

    // Iterate through the directory
    path inputFilePath(inputPath);
    recursive_directory_iterator iter(inputFilePath), end;
    std::mutex mx;

    // Run multiple threads to do codegen
    unsigned int nThreads = faabric::util::getUsableCores();
    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < nThreads; i++) {
        threads.emplace_back([&iter, &mx, &end, &logger, &loader] {
            logger->info("Spawning codegen thread");

            while (true) {
                std::string thisPath;

                {
                    // Get lock
                    faabric::util::UniqueLock lock(mx);

                    // Check if we've got more to do
                    if (iter == end) {
                        logger->info("Codegen thread finished");
                        break;
                    }

                    // Get path and increment
                    thisPath = iter->path().string();
                    iter++;
                }

                directory_entry f(thisPath);
                const std::string fileName = f.path().filename().string();
                if (faabric::util::endsWith(fileName, ".so") || faabric::util::endsWith(fileName, ".wasm")) {
                    logger->info("Generating machine code for {}", thisPath);
                    loader.codegenForSharedObject(thisPath);
                }
            }
        });
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

int main(int argc, char *argv[]) {
    faabric::util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = faabric::util::getLogger();

    if (argc < 2) {
        logger->error("Must provide path to shared object dir");
        return 1;
    }

    std::string inputPath = argv[1];
    if (is_directory(inputPath)) {
        codegenForDirectory(inputPath);
    } else {
        storage::FileLoader &loader = storage::getFileLoader();
        loader.codegenForSharedObject(inputPath);
    }
}
