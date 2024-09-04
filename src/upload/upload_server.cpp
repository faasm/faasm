#include <storage/S3Wrapper.h>
#include <upload/UploadServer.h>
#include <upload/UploadEndpointHandler.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/state/StateServer.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>

int main()
{
    storage::initFaasmS3();

    faabric::util::initLogging();

    // Print the Faasm config
    conf::getFaasmConfig().print();

    SPDLOG_INFO("Starting upload's state server");
    faabric::state::StateServer stateServer(
      faabric::state::getGlobalState());
    stateServer.start();

    // Start the upload server in the main thread
    SPDLOG_INFO("Starting upload endpoint");
    faabric::endpoint::FaabricEndpoint endpoint(
      UPLOAD_PORT,
      2, // TODO
      std::make_shared<upload::UploadEndpoingHandler>());
    endpoint.start(faabric::endpoint::EndpointMode::SIGNAL);

    // Stop the state server
    stateServer.stop();

    storage::shutdownFaasmS3();

    return 0;
}
