#include <util/logging.h>
#include <util/func.h>
#include <util/chaining.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 6) {
        logger->error("Must provide host, port, user, function and function idx");
        return 1;
    }

    std::string host = argv[1];
    std::string portStr = argv[2];
    std::string user = argv[3];
    std::string function = argv[4];
    std::string functionIdxStr = argv[5];

    int port = std::stoi(portStr);
    int functionIdx = std::stoi(functionIdxStr);

    message::Message msg = util::messageFactory(user, function);
    msg.set_idx(functionIdx);

    logger->info("Posting {} to {}:{}", util::funcToString(msg, false), host, port);
    const std::string responseStr = util::postJsonFunctionCall(host, port, msg);
    logger->info("Response: {}", responseStr);
    
    return 0;
}