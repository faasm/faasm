#include <iostream>
#include <signal.h>
#include <util/TCPClient.h>

using namespace util;

TCPClient tcp;

void sig_exit(int s) {
    tcp.exit();
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./client ip port message" << std::endl;
        return 0;
    }
    signal(SIGINT, sig_exit);

    tcp.setup(argv[1], atoi(argv[2]));
    while (1) {
        tcp.Send(argv[3]);
        std::string rec = tcp.receive();
        if (rec != "") {
            std::cout << rec << std::endl;
        }
        sleep(1);
    }
    return 0;
}
