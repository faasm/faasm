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
}