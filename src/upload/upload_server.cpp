#include <upload/UploadServer.h>

#include <aws/aws.h>

#include <util/logging.h>
#include <util/config.h>

int main() {
    std::string port = "8002";
    
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    awswrapper::initSDK();

    edge::UploadServer server;
    server.listen(port);

    return 0;
}