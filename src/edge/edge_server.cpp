#include <edge/FunctionEndpoint.h>

#include <util/logging.h>

#if AWS_SUPPORT == 1
#include <aws/aws.h>
#endif

int main() {
    util::initLogging();

#if AWS_SUPPORT == 1
    awswrapper::initSDK();
#endif

    edge::FunctionEndpoint endpoint;
    endpoint.start();

    return 0;
}