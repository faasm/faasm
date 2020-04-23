#include <catch/catch.hpp>

#include <tcp/TCPServer.h>
#include <tcp/TCPMessage.h>
#include <tcp/TCPClient.h>

using namespace tcp;

namespace tests {
    TEST_CASE("Test send and receive", "[tcp]") {
        int port = 8006;
        std::string host = "127.0.0.1";

        std::vector<uint8_t> requestBytes = {0, 1, 2, 3, 4};
        std::vector<uint8_t> responseBytes = {5, 6, 7};

        // Run an echo server in a thread
        std::thread serverThread([port, &requestBytes, &responseBytes] {
            TCPServer server(port);

            TCPMessage *m = server.next(200);
            std::vector<uint8_t> actualRecv(m->buffer, m->buffer + m->len);
            
            REQUIRE(actualRecv == requestBytes);
            
            TCPMessage r;
            r.type=TCPMessageType::STANDARD;
            r.len = responseBytes.size();
            r.buffer = responseBytes.data();

            server.respond(m, &r);
        });

        // Let the server start
        usleep(1000*100);

        // Send request with client
        TCPClient client(host, port);
        TCPMessage m;
        m.type=TCPMessageType::STANDARD;
        m.len = requestBytes.size();
        m.buffer = requestBytes.data();
        client.sendMessage(&m);

        // Receive response
        TCPMessage *r = client.recvMessage();
        std::vector<uint8_t> actualRecv(r->buffer, r->buffer + r->len);
        REQUIRE(actualRecv == responseBytes);

        // Let server thread finish
        if(serverThread.joinable()) {
            serverThread.join();
        }
    }
}