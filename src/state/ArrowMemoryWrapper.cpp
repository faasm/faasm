#include "ArrowMemoryWrapper.h"

#include <util/logging.h>

namespace state {
    ArrowMemoryWrapper::ArrowMemoryWrapper(size_t memSize) : size(memSize) {
        arrow::Result<std::unique_ptr<arrow::Buffer>> interimBuffer = arrow::AllocateBuffer(size);

        if (!interimBuffer.ok()) {
            throw std::runtime_error("Failed to allocate Arrow buffer");
        } else {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->debug("New Arrow mem {}b", size);
        }

        buffer = *std::move(interimBuffer);
    }

    uint8_t *ArrowMemoryWrapper::getMutablePointer() {
        return buffer->mutable_data();
    }

    void ArrowMemoryWrapper::flightMessageToMemory(
            std::unique_ptr<arrow::flight::FlightMessageReader> reader,
            std::unique_ptr<arrow::flight::FlightMetadataWriter> writer
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::vector<std::shared_ptr<arrow::RecordBatch>> *batchVec;
        CHECK_ARROW_OP("read", reader->ReadAll(batchVec))

        for(auto c : *batchVec) {
            const arrow::ArrayDataVector colData = c->column_data();

        }
    }
}