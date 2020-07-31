#pragma once

#include <arrow/flight/api.h>
#include "InMemoryStateRegistry.h"
#include "State.h"

// --------------------------------------------
// Arrow Flight message format: http://arrow.apache.org/docs/format/Flight.html
// Arrow Flight CPP: https://arrow.apache.org/docs/cpp/flight.html
//
// Integration test and benchmark (good example implementations):
// https://github.com/apache/arrow/blob/master/cpp/src/arrow/flight/test_integration_server.cc
// https://github.com/apache/arrow/blob/master/cpp/src/arrow/flight/flight_benchmark.cc
// --------------------------------------------

#define ARROW_STATE_PORT 8004

namespace state {
    class ArrowStateServer : public arrow::flight::FlightServerBase {
    public:
        explicit ArrowStateServer(State &globalStateIn);

        void start();

        arrow::Status GetFlightInfo(
                const arrow::flight::ServerCallContext &context,
                const arrow::flight::FlightDescriptor &request,
                std::unique_ptr<arrow::flight::FlightInfo> *info
        ) override;

        arrow::Status DoGet(
                const arrow::flight::ServerCallContext &context,
                const arrow::flight::Ticket &request,
                std::unique_ptr<arrow::flight::FlightDataStream> *stream
        ) override;

        arrow::Status DoPut(
                const arrow::flight::ServerCallContext &context,
                std::unique_ptr<arrow::flight::FlightMessageReader> reader,
                std::unique_ptr<arrow::flight::FlightMetadataWriter> writer) override;

    private:
        State &globalState;
        InMemoryStateRegistry &stateRegistry;
    };
}