#include <arrow_state/ArrowMemoryWrapper.h>
#include <util/logging.h>


int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    arrow_state::ArrowMemoryWrapper memoryWrapper(4096);
    uint8_t *rawPtr = memoryWrapper.getMutablePointer();

    std::memcpy(rawPtr, "hello world", 11);

    auto bufferStr = reinterpret_cast<char *>(rawPtr);
    logger->info("Buffer: {}", bufferStr);
}