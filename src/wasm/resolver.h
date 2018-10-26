#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
    struct RootResolver : Runtime::Resolver {
        HashMap<std::string, Runtime::ModuleInstance*> moduleNameToInstanceMap;

        bool resolve(const std::string& moduleName,
                     const std::string& exportName,
                     IR::ExternType type,
                     Runtime::Object*& outObject) override
        {
            auto namedInstance = moduleNameToInstanceMap.get(moduleName);

            if(namedInstance) {
                outObject = getInstanceExport(*namedInstance, exportName);

                if(outObject && isA(outObject, type)) {
                    return true;
                };
            }

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Missing import {}.{} {}", moduleName, exportName, asString(type).c_str());

            return false;
        }
    };
}
