#include <util/logging.h>
#include <boost/filesystem.hpp>
#include <util/func.h>
#include <storage/FunctionLoader.h>

using namespace boost::filesystem;

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide function user and name");
        return 0;
    }

    std::string user = argv[1];
    std::string func = argv[2];

    message::Message msg = util::messageFactory(user, func);
    if(!util::isValidFunction(msg)) {
        logger->error("Invalid function: {}/{}", user, func);
    }

    logger->info("Generating machine code for {}/{}", user, func);
    storage::FunctionLoader &loader = storage::getFunctionLoader();
    loader.codegenForFunction(msg);
}
