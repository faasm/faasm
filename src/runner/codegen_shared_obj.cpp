#include <codegen/MachineCodeGenerator.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>
#include <storage/S3Wrapper.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace boost::filesystem;
namespace po = boost::program_options;

po::variables_map parseCmdLine(int argc, char* argv[])
{
    // Define command line arguments
    po::options_description desc("Allowed options");
    desc.add_options()("input-path",
                       po::value<std::string>(),
                       "directory of shared objects (required)")(
      "clean", "overwrite existing generated code");

    // Mark user and function as positional arguments
    po::positional_options_description p;
    p.add("input-path", 1);

    // Parse command line arguments
    po::variables_map vm;
    po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
    po::notify(vm);

    return vm;
}

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

    auto vm = parseCmdLine(argc, argv);
    std::string inputPath = vm["input-path"].as<std::string>();
    bool clean = vm.find("clean") != vm.end();

    if (is_directory(inputPath)) {
        codegenForDirectory(inputPath, clean);
    } else {
        codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();
        gen.codegenForSharedObject(inputPath, clean);
    }

    storage::shutdownFaasmS3();
}
