#include <faabric/util/func.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/host_interface_test.h>

namespace wasm {
void doHostInterfaceTest(int testNum)
{
    switch (testNum) {
        case ExceptionPropagationTest: {
            auto ex =
              faabric::util::FunctionMigratedException("Migrating MPI rank");
            getExecutingModule()->doThrowException(ex);
        }
        default: {
            SPDLOG_ERROR("Unrecognised host interface test: {}", testNum);
            throw std::runtime_error("Unrecognised host interface test!");
        }
    }
}
}
