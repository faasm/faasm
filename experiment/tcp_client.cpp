#include <iostream>
#include <util/TCPClient.h>
#include <util/logging.h>

using namespace util;

#define N_REQUESTS 3
#define PORT 8005
#define HOST "127.0.0.1"

int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    TCPClient client(HOST, PORT);

    for (int i = 0; i < N_REQUESTS; i++) {
        // Send some data
        std::vector<uint8_t> bytesToSend = {0, 1, 2, 3, 4, 5, 6, 7};
        client.sendBytes(bytesToSend.data(), bytesToSend.size());

        // Expect to receive it back
        std::vector<uint8_t> receivedBytes;
        receivedBytes.reserve(bytesToSend.size());
        size_t receivedSize = client.receiveBytes(receivedBytes.data(), bytesToSend.size());

        if (receivedSize != bytesToSend.size()) {
            logger->error("Bytes unexpected size (got {}, expected {})", receivedSize, bytesToSend.size());
        } else if (receivedBytes != bytesToSend) {
            logger->error("Sent and received do not match up");
        } else {
            logger->debug("Bytes sent and received as expected");
        }
    }
    return 0;
}
