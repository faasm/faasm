#include "KnativeEndpoint.h"

#include <util/logging.h>

using namespace knative;

int main() {
    util::initLogging();

    KnativeEndpoint endpoint;
    endpoint.start();

    return 0;
}
