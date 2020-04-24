#include <upload/UploadServer.h>


#include <util/logging.h>
#include <util/config.h>

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    edge::UploadServer server;
    server.listen(UPLOAD_PORT);

    return 0;
}