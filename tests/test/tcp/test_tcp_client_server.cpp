#include <catch/catch.hpp>

#include "utils.h"

#include <tcp/EchoServer.h>
#include <tcp/TCPClient.h>

using namespace tcp;

namespace tests {
    TEST_CASE("Test send and receive on echo server", "[tcp]") {
        cleanSystem();

        // Use different ports to make sure there's no conflicts
        int port;
        std::string host = "127.0.0.1";

        std::vector<uint8_t> requestBytes;

        TCPMessage m;
        m.type = 1;

        SECTION("Some data") {
            port = 8006;

            requestBytes = {0, 1, 2, 3, 4};
            m.len = requestBytes.size();
            m.buffer = requestBytes.data();
        }

        SECTION("No data") {
            port = 8007;

            // Don't set other fields on message to check defaults
        }

        // Run an echo server in a thread
        std::thread serverThread([port] {
            util::getLogger()->debug("Starting echo server thread");
            EchoServer server(port);

            int nMsgs = 0;
            while (nMsgs == 0) {
                nMsgs += server.poll();
            }

            server.close();
        });

        // Let the server start
        usleep(1000 * 100);

        // Send request with client
        TCPClient client(host, port);
        client.sendMessage(&m);

        // Receive response
        TCPMessage *r = client.recvMessage();
        std::vector<uint8_t> actualRecv(r->buffer, r->buffer + r->len);
        REQUIRE(actualRecv == requestBytes);

        // Let server thread finish
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }
}