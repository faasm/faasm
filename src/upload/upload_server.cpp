#include <upload/UploadServer.h>

#if AWS_SUPPORT == 1
#include <aws/aws.h>
#endif

#include <util/logging.h>
#include <util/config.h>

int main() {
    std::string port = "8002";
    
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

#if AWS_SUPPORT == 1
    awswrapper::initSDK();
#endif

    edge::UploadServer server;
    server.listen(port);

    return 0;
}