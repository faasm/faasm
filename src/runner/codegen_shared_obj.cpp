#include <boost/filesystem.hpp>

#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

#include <codegen/MachineCodeGenerator.h>
#include <storage/S3Wrapper.h>

using namespace boost::filesystem;

void codegenForDirectory(std::string& inputPath, bool clean)
{
    SPDLOG_INFO("Running codegen on directory {}", inputPath);

    // Iterate through the directory
    path inputFilePath(inputPath);
    recursive_directory_iterator iter(inputFilePath);
    recursive_directory_iterator end;
    std::mutex mx;

    // Run multiple threads to do codegen
    unsigned int nThreads = faabric::util::getUsableCores();
    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < nThreads; i++) {
        threads.emplace_back([&iter, &mx, &end, clean] {
            SPDLOG_INFO("Spawning codegen thread");
            codegen::MachineCodeGenerator& gen =
              codegen::getMachineCodeGenerator();

            while (true) {
                std::string thisPath;

                {
                    // Get lock
                    faabric::util::UniqueLock lock(mx);

                    // Check if we've got more to do
                    if (iter == end) {
                        SPDLOG_INFO("Codegen thread finished");
                        break;
                    }

                    // Get path and increment
                    thisPath = iter->path().string();
                    iter++;
                }

                directory_entry f(thisPath);
                const std::string fileName = f.path().filename().string();
                if (faabric::util::endsWith(fileName, ".so") ||
                    faabric::util::endsWith(fileName, ".wasm")) {
                    SPDLOG_INFO("Generating machine code for {}", thisPath);
                    gen.codegenForSharedObject(thisPath, clean);
                }
            }
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

int main(int argc, char* argv[])
{
    faabric::util::initLogging();
    storage::initFaasmS3();

    if (argc < 2) {
        SPDLOG_ERROR("Must provide path to shared object dir");
        return 1;
    }

    bool clean = false;
    if (argc == 3) {
        clean = std::string(argv[2]) == "--clean";
    }

    std::string inputPath = argv[1];
    if (is_directory(inputPath)) {
        codegenForDirectory(inputPath, clean);
    } else {
        codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();
        gen.codegenForSharedObject(inputPath, clean);
    }

    storage::shutdownFaasmS3();
}
