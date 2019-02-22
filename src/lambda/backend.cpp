#include "backend.h"

#include <aws/aws.h>
#include <string>

namespace faasm {
    void initialiseLambdaBackend() {
        // Set up lambda SDK
        awswrapper::initSDK();
    }

    void tearDownLambdaBackend() {
        // Shut down SDK
        awswrapper::cleanUpSDK();
    }
}
