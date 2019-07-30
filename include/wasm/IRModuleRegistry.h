#pragma once

#include <WAVM/Runtime/Intrinsics.h>
#include <proto/faasm.pb.h>

// Note that page size in wasm is 64kiB
// Note also that this initial memory must be big enough to include all data, stack and dynamic
// memory that the module will need.
#define ONE_MB_PAGES 16
#define ONE_GB_PAGES 1024 * ONE_MB_PAGES
#define MAX_MEMORY_PAGES ONE_GB_PAGES
#define MAX_TABLE_SIZE 500000

using namespace WAVM;

namespace wasm {
    class IRModuleRegistry {
    public:
        IR::Module &getMainModule(const std::string &user, const std::string &func);

        IR::Module &getMainModule(const message::Message &msg);

        Runtime::ModuleRef &getCompiledModule(const std::string &user, const std::string &func);

        Runtime::ModuleRef &getCompiledModule(const message::Message &msg);

        IR::Module &getSharedModule(const std::string &path);

        Runtime::ModuleRef &getCompiledSharedModule(const std::string &path);

    private:
        void initialiseIRModule(
                IR::Module &module,
                Runtime::ModuleRef &compiledModule,
                const std::vector<uint8_t> &wasmBytes,
                const std::vector<uint8_t> &objBytes,
                bool isMainModule
        );

    };

    IRModuleRegistry &getIRModuleRegistry();
}
