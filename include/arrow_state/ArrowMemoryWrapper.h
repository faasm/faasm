#include <arrow/api.h>

namespace arrow_state {

    class ArrowMemoryWrapper {
    public:
        ArrowMemoryWrapper(size_t memSize);

        uint8_t *getMutablePointer();
    private:
        size_t size;
        std::shared_ptr<arrow::Buffer> buffer;
    };
}