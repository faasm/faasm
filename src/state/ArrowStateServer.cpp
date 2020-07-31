#include "ArrowStateServer.h"
#include "ArrowMemoryWrapper.h"

#include <arrow/flight/api.h>
#include <arrow/api.h>
#include <csignal>
#include <memory>

#include <util/logging.h>
#include <util/string_tools.h>

namespace state {
    std::pair<std::string, std::string> getUserKeyFromPath(const std::string &path) {
        std::vector<std::string> tokens = util::splitString(path, '/');
        if (tokens.size() != 2) {
            throw std::runtime_error("Invalid state path " + path);
        }
        std::string stateUser = tokens.at(0);
        std::string stateKey = tokens.at(1);

        return {stateUser, stateKey};
    }

    std::pair<std::string, std::string> getUserKeyFromDescriptor(
            const arrow::flight::FlightDescriptor& descriptor
    ) {
        if (descriptor.type != arrow::flight::FlightDescriptor::DescriptorType::PATH) {
            throw std::runtime_error("Unexpected request type");
        }
        // Extract state key from request
        std::vector<std::string> pathStrings = descriptor.path;
        if (pathStrings.size() != 1) {
            throw std::runtime_error("Expected one path string. Got " + std::to_string(pathStrings.size()));
        }
        return getUserKeyFromPath(pathStrings[0]);
    }
    
    ArrowStateServer::ArrowStateServer(State &globalStateIn) : globalState(globalStateIn),
                                                               stateRegistry(getInMemoryStateRegistry()) {

    }

    void ArrowStateServer::start() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Initialize server
        std::unique_ptr<arrow::flight::FlightServerBase> server;
        arrow::flight::Location location;

        // Listen to all interfaces on a free port
        CHECK_ARROW_OP("connect", arrow::flight::Location::ForGrpcTcp(
                "0.0.0.0",
                ARROW_STATE_PORT,
                &location
        ))

        // Start the server
        arrow::flight::FlightServerOptions options(location);
        CHECK_ARROW_OP("listen", server->Init(options))

        // Set up signals for shutdown
        CHECK_ARROW_OP("signals", server->SetShutdownOnSignals({SIGTERM}))

        // Start serving
        logger->debug("Arrow server listening on {}:{}", location.ToString(), server->port());
        CHECK_ARROW_OP("serve", server->Serve())
    }

    arrow::Status ArrowStateServer::GetFlightInfo(
            const arrow::flight::ServerCallContext &context,
            const arrow::flight::FlightDescriptor &request,
            std::unique_ptr<arrow::flight::FlightInfo> *info
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::pair<std::string, std::string> stateUserKey = getUserKeyFromDescriptor(request);

        // Get master IP for this state value
        const std::string &stateMaster = stateRegistry.getMasterIP(
                stateUserKey.first, stateUserKey.second,
                globalState.getThisIP(),
                false
        );

        // Create a location for the master
        arrow::flight::Location stateMasterLocation;
        CHECK_ARROW_OP("master_location", arrow::flight::Location::ForGrpcTcp(
                stateMaster,
                ARROW_STATE_PORT,
                &stateMasterLocation
        ))

        // Create associated endpoint
        arrow::flight::FlightEndpoint endpoint({{request.path[0]},
                                                {stateMasterLocation}});
        arrow::flight::FlightInfo::Data flightData;
        flightData.schema = stateSchema.ToString();
        flightData.descriptor = request;
        flightData.endpoints = {endpoint};
        flightData.total_records = 1;
        flightData.total_bytes = -1;

        // Write to the response
        arrow::flight::FlightInfo value(flightData);
        *info = std::make_unique<arrow::flight::FlightInfo>(value);

        return arrow::Status::OK();
    }

    arrow::Status ArrowStateServer::DoGet(
            const arrow::flight::ServerCallContext &context,
            const arrow::flight::Ticket &request,
            std::unique_ptr<arrow::flight::FlightDataStream> *stream
    ) {
        auto stateUserKey = getUserKeyFromPath(request.ticket);
        const std::shared_ptr<StateKeyValue> &kv = globalState.getKV(stateUserKey.first, stateUserKey.second);

        *stream = std::unique_ptr<arrow::flight::FlightDataStream>(

                        );


        return arrow::Status::OK();

    }

    arrow::Status ArrowStateServer::DoPut(
            const arrow::flight::ServerCallContext &context,
            std::unique_ptr<arrow::flight::FlightMessageReader> reader,
            std::unique_ptr<arrow::flight::FlightMetadataWriter> writer) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::pair<std::string, std::string> stateUserKey = getUserKeyFromDescriptor(reader->descriptor());
        std::shared_ptr<StateKeyValue> kv = globalState.getKV(stateUserKey.first, stateUserKey.second);


        return arrow::Status::OK();
    }
}