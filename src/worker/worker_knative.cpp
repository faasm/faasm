#include "KnativeEndpoint.h"

#include <util/logging.h>

using namespace worker;

int main() {
    util::initLogging();

    KnativeEndpoint endpoint;
    endpoint.start();

    return 0;
}
