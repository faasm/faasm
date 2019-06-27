#include <edge/FunctionEndpoint.h>

#include <util/logging.h>
#include <aws/aws.h>
#include <pistache/http.h>

int main() {
    util::initLogging();

    awswrapper::initSDK();

    edge::FunctionEndpoint endpoint;
    endpoint.start();

    return 0;
}