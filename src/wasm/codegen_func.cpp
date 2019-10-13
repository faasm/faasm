#include <util/logging.h>
#include <boost/filesystem.hpp>
#include <util/func.h>
#include <storage/FileLoader.h>
#include <util/config.h>
#include <util/environment.h>
#include <util/locks.h>

using namespace boost::filesystem;

void codegenForFunc(const std::string &user, const std::string &func) {
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    message::Message msg = util::messageFactory(user, func);
    if (!util::isValidFunction(msg)) {
        logger->error("Invalid function: {}/{}", user, func);
    }

    logger->info("Generating machine code for {}/{}", user, func);
    storage::FileLoader &loader = storage::getFileLoader();
    loader.codegenForFunction(msg);
}

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc == 3) {
        std::string user = argv[1];
        std::string func = argv[2];

        logger->info("Running codegen for function {}/{}", user, func);
        codegenForFunc(user, func);
    } else if (argc == 2) {
        std::string user = argv[1];

        util::SystemConfig &conf = util::getSystemConfig();
        logger->info("Running codegen for user {} on dir {}", user, conf.functionDir);

        boost::filesystem::path path(conf.functionDir);
        path.append(user);

        if (!boost::filesystem::is_directory(path)) {
            logger->error("Expected {} to be a directory", path.string());
            return 1;
        }

        boost::filesystem::directory_iterator iter(path), end;
        std::mutex mx;

        unsigned int nThreads = util::getUsableCores();
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < nThreads; i++) {
            threads.emplace_back([&iter, &mx, &end, &logger, &user] {
                logger->info("Spawning codegen thread");

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

                    directory_entry subPath(thisPath);
                    std::string functionName = subPath.path().filename().string();
                    codegenForFunc(user, functionName);
                }
            });
        }

        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else {
        logger->error("Must provide function user and optional function name");
        return 0;
    }
}