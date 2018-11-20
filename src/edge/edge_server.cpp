#include <pistache/http.h>
#include <edge/edge.h>

int main() {
    Pistache::Port port(8001);

    int threadCount = 20;

    Pistache::Address addr(Ipv4::any(), port);

    edge::FunctionEndpoint endpoint(addr);

    endpoint.init(threadCount);
    endpoint.start();

    endpoint.shutdown();
}