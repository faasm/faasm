#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <storage/FileLoader.h>
#include <storage/S3Wrapper.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace boost::filesystem;
namespace po = boost::program_options;

po::variables_map parseCmdLine(int argc, char* argv[])
{
    // Define command line arguments
    po::options_description desc("Allowed options");
    desc.add_options()(
      "user", po::value<std::string>(), "function's user name (required)")(
      "func", po::value<std::string>(), "function's name")(
      "clean", "overwrite existing generated code");

    // Mark user and function as positional arguments
    po::positional_options_description p;
    p.add("user", 1);

    // Parse command line arguments
    po::variables_map vm;
    po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
    po::notify(vm);

    return vm;
}

void codegenForFunc(const std::string& user,
                    const std::string& func,
                    bool clean)
{
    codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();
    storage::FileLoader& loader = storage::getFileLoader();
    faabric::Message msg = faabric::util::messageFactory(user, func);

    std::string funcFile = loader.getFunctionFile(msg);
    if (!boost::filesystem::exists(funcFile)) {
        SPDLOG_WARN("Invalid function: {}/{}", user, func);
        return;
    }

    SPDLOG_INFO("Generating machine code for {}/{} (WASM VM: {})",
                user,
                func,
                conf::getFaasmConfig().wasmVm);

    gen.codegenForFunction(msg, clean);
}

int main(int argc, char* argv[])
{
    faabric::util::initLogging();
    storage::initFaasmS3();
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    auto vm = parseCmdLine(argc, argv);
    std::string user = vm["user"].as<std::string>();
    bool clean = vm.find("clean") != vm.end();

    if (vm.find("func") != vm.end()) {
        std::string func = vm["func"].as<std::string>();

        SPDLOG_INFO("Running codegen for function {}/{} (WASM VM: {})",
                    user,
                    func,
                    conf.wasmVm);
        codegenForFunc(user, func, clean);
    } else {
        SPDLOG_INFO(
          "Running codegen for user {} on dir {}", user, conf.functionDir);

        boost::filesystem::path path(conf.functionDir);
        path.append(user);

        if (!boost::filesystem::is_directory(path)) {
            SPDLOG_ERROR("Expected {} to be a directory", path.string());
            return 1;
        }

        boost::filesystem::directory_iterator iter(path);
        boost::filesystem::directory_iterator end;
        std::mutex mx;

        unsigned int nThreads = faabric::util::getUsableCores();
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < nThreads; i++) {
            threads.emplace_back([&iter, &mx, &end, &user, clean] {
                SPDLOG_INFO("Spawning codegen thread");

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

                    directory_entry subPath(thisPath);
                    std::string functionName =
                      subPath.path().filename().string();
                    codegenForFunc(user, functionName, clean);
                }
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    storage::shutdownFaasmS3();
}
