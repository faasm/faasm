#include <arrow/api.h>
#include <arrow/flight/api.h>

#define CHECK_ARROW_OP(label, op)  { \
    arrow::Status status = op; \
    if (!status.ok()) {  \
        logger->error("ARROW ERROR ({}): {}", label, status.message()); \
        throw std::runtime_error("Arrow failed. See logs"); \
    } \
    }

namespace state {
    arrow::Schema stateSchema({
        field("memory", arrow::list(arrow::uint8()))
    });

    class ArrowMemoryWrapper {
    public:
        ArrowMemoryWrapper(size_t memSize);

        uint8_t *getMutablePointer();

        void flightMessageToMemory(
                std::unique_ptr<arrow::flight::FlightMessageReader> reader,
                std::unique_ptr<arrow::flight::FlightMetadataWriter> writer
        );

    private:
        size_t size;
        std::shared_ptr<arrow::Buffer> buffer;
    };
}