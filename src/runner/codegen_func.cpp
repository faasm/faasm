#include <boost/filesystem.hpp>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <storage/FileLoader.h>

using namespace boost::filesystem;

void codegenForFunc(const std::string& user,
                    const std::string& func,
                    bool isSgx = false)
{

    faabric::Message msg = faabric::util::messageFactory(user, func);
    if (!conf::isValidFunction(msg)) {
        SPDLOG_WARN("Invalid function: {}/{}", user, func);
        return;
    }
    if (isSgx) {
        msg.set_issgx(true);
        SPDLOG_INFO("Generating SGX machine code for {}/{}", user, func);
    } else {
        SPDLOG_INFO("Generating machine code for {}/{}", user, func);
    }

    storage::FileLoader& loader = storage::getFileLoader();
    loader.codegenForFunction(msg);
}

int main(int argc, char* argv[])
{
    faabric::util::initLogging();

    if (argc == 3) {
        std::string user = argv[1];
        std::string func = argv[2];

        SPDLOG_INFO("Running codegen for function {}/{}", user, func);
        codegenForFunc(user, func);
    } else if (argc == 2) {
        std::string user = argv[1];

        conf::FaasmConfig& conf = conf::getFaasmConfig();
        SPDLOG_INFO(
          "Running codegen for user {} on dir {}", user, conf.functionDir);

        boost::filesystem::path path(conf.functionDir);
        path.append(user);

        if (!boost::filesystem::is_directory(path)) {
            SPDLOG_ERROR("Expected {} to be a directory", path.string());
            return 1;
        }

        boost::filesystem::directory_iterator iter(path), end;
        std::mutex mx;

        unsigned int nThreads = faabric::util::getUsableCores();
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < nThreads; i++) {
            threads.emplace_back([&iter, &mx, &end, &user] {
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
                    codegenForFunc(user, functionName);
                }
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else if (argc == 4 && std::string(argv[3]) == "--sgx") {
        std::string user = argv[1];
        std::string func = argv[2];

        SPDLOG_INFO("Running SGX codegen for function {}/{}", user, func);
        codegenForFunc(user, func, true);
    } else {
        SPDLOG_ERROR("Must provide function user and optional function name");
        return 0;
    }
}
