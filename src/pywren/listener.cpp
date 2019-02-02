#include "pywren.h"

#include <util/config.h>
#include <util/logging.h>

#include <aws/aws.h>
#include <aws/S3Wrapper.h>


int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    awswrapper::initSDK();

    awswrapper::S3Wrapper s3;
    s3.listKeys("shillaker-pywren");

    awswrapper::cleanUpSDK();
}
