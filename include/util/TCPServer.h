#pragma once

#include <vector>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace util {
    class TCPServer {
    public:
        explicit TCPServer(int port);

        void start();

        void accepted();

        void closed();

    private:
        int serverSocket;
        struct sockaddr_in serverAddress{};

        std::atomic<int> clientCount;

        std::vector<std::thread> threads;
    };

}