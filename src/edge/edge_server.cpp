#include <edge/FunctionEndpoint.h>

#include <util/logging.h>
#include <aws/aws.h>
#include <pistache/http.h>

int main() {
    util::initLogging();

    edge::FunctionEndpoint endpoint;
    endpoint.start();

    awswrapper::initSDK();

    endpoint.shutdown();

    return 0;
}