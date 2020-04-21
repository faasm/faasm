#include <util/TCPServer.h>
#include <util/logging.h>

using namespace util;

#define PORT 8005

int main() {
    util::initLogging();

    TCPServer server(PORT);
    server.start();

    server.closed();
    return 0;
}
