#include <edge/edge.h>

int main() {
    std::string port = "8080";

    util::initLogging();

    edge::RestServer server;
    server.listen(port);

    return 0;
}