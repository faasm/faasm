#include <grpcpp/server.h>

#include <thread>

using namespace grpc;

namespace rpc {
    class RPCServer {
    public:
        RPCServer(const std::string &hostIn, int portIn);

        void start(bool background = true);

        void stop();
    protected:
        const std::string host;
        const int port;

        bool _started = false;
        bool _isBackground = false;

        std::unique_ptr<Server> server;
        std::thread servingThread;

        virtual void doStart(const std::string &serverAddr) = 0;
    };
}