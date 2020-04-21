#pragma once

#include <vector>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace tcp {
    class TCPServer {
    public:
        explicit TCPServer(int portIn);

        void start(int messageLimit = -1);

        void accepted();

        void closed();

        void setEchoMode(bool echoModeIn);

    private:
        int port;
        int serverSocket;
        struct sockaddr_in serverAddress{};

        bool echoMode;

        std::vector<std::thread> threads;
    };

}