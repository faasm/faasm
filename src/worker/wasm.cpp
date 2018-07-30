#include "worker.h"
#include "resolver.h"
#include <infra/infra.h>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {
    WasmModule::WasmModule() {

    }

    int WasmModule::execute(message::FunctionCall &call) {
        std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";

        std::string filePath = infra::getFunctionFile(call);

        Module module;
        loadModule(filePath.c_str(), module);

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        Context *context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, module);

        if(emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
            rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
            rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
        }

        // Linking
        LinkResult linkResult = linkModule(module, rootResolver);

        // Instantiate the module.
        ModuleInstance *moduleInstance = instantiateModule(
                compartment,
                module,
                std::move(linkResult.resolvedImports),
                filePath.c_str());

        // Call the module start function, if it has one.
        FunctionInstance *startFunction = getStartFunction(moduleInstance);
        if (startFunction) {
            invokeFunctionChecked(context, startFunction, {});
        }

        // Call the Emscripten global initalizers.
        Emscripten::initializeGlobals(context, module, moduleInstance);

        // Call the module's main function
        FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "main"));

        std::vector<Value> invokeArgs;
        IR::ValueTuple functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Clear up
        Runtime::collectGarbage();

        return 0;
    }
}