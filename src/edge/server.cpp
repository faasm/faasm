#include <pistache/http.h>
#include <edge/edge.h>

using namespace Pistache;

int main() {
    Port port(8080);

    int threadCount = 4;

    Address addr(Ipv4::any(), port);

    edge::FunctionEndpoint endpoint(addr);

    endpoint.init(threadCount);
    endpoint.start();

    endpoint.shutdown();
}