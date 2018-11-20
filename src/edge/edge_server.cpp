#include <edge/edge.h>
#include <pistache/http.h>

int main() {
    util::initLogging();

    edge::FunctionEndpoint endpoint;
    endpoint.start();

    endpoint.shutdown();

    return 0;
}