#pragma once

#include <arrow/flight/api.h>


// See Arrow Flight docs here: http://arrow.apache.org/docs/format/Flight.html

namespace state {
    class ArrowStateServer : public arrow::flight::FlightServerBase {
    public:
        ArrowStateServer();

        arrow::Status GetFlightInfo(
                const arrow::flight::ServerCallContext &context,
                const arrow::flight::FlightDescriptor &request,
                std::unique_ptr<arrow::flight::FlightInfo> *info
        ) override {
            if (request.type == arrow::flight::FlightDescriptor::DescriptorType::PATH) {
                std::vector<std::string> pathStrings = request.path;
                if (pathStrings.size() != 1) {
                    throw std::runtime_error("Expected one path string. Got " + std::to_string(pathStrings.size()));
                }

                std::string stateKey = pathStrings.at(0);

            } else {
                throw std::runtime_error("Unexpected request type");
            }
        }

        arrow::Status DoGet(
                const arrow::flight::ServerCallContext &context,
                const arrow::flight::Ticket &request,
                std::unique_ptr<arrow::flight::FlightDataStream> *stream
        ) override {
            std::string stateKey = request.ticket;

        }

        arrow::Status DoPut(
                const arrow::flight::ServerCallContext &context,
                std::unique_ptr<arrow::flight::FlightMessageReader> reader,
                std::unique_ptr<arrow::flight::FlightMetadataWriter> writer) override {

        }

    };
}