#pragma once

#include <iostream>
#include <unistd.h>

#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace util {

    class TCPClient {
    private:
        int sock;
        std::string address;
        int port;
        struct sockaddr_in server;

    public:
        TCPClient();

        bool setup(std::string address, int port);

        bool Send(std::string data);

        std::string receive(int size = 4096);

        std::string read();

        void exit();
    };
}

