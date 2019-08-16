#include "pistache/endpoint.h"

using namespace Pistache;

class HelloHandler : public Http::Handler {
public:
    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request& request, Http::ResponseWriter response) override{
        response.send(Pistache::Http::Code::Ok, "Hello World\n");
    }
};

int main() {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
    auto opts = Pistache::Http::Endpoint::options()
            .threads(5);

    Http::Endpoint server(addr);
    server.init(opts);
    server.setHandler(Http::make_handler<HelloHandler>());
    server.serve();
}
