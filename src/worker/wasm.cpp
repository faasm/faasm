#include <stdio.h>

#include <Runtime/RuntimePrivate.h>
#include "Programs/CLI.h"
#include <Emscripten/Emscripten.h>

#include "worker.h"
#include "resolver.h"


using namespace IR;
using namespace Runtime;

/**
 * NOTE: This is heavily based on the existing wavm application
 * https://github.com/AndrewScheidecker/WAVM/blob/master/Source/Programs/wavm.cpp
 */

namespace worker {
    const std::string ENTRYPOINT_FUNC = "run";

    WasmModule::WasmModule() {

    }

    void WasmModule::clean() {
        // Clear up
        Runtime::collectGarbage();
    }

    void WasmModule::execute(message::FunctionCall &call) {
        std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";

        std::string filePath = infra::getFunctionFile(call);
        std::cout << "Executing function at:  " << filePath << "\n";

        Module module;
        if (!loadModule(filePath.c_str(), module)) {
            std::cerr << "Could not load module at:  " << filePath << "\n";

            throw WasmException();
        }

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        Context *context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, module);

        if (emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
            rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
            rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
        }

        // Linking
        LinkResult linkResult = linkModule(module, rootResolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            for (auto &missingImport : linkResult.missingImports) {
                std::cerr << "Missing import: module=\"" << missingImport.moduleName
                          << "\" export=\"" << missingImport.exportName
                          << "\" type=\"" << asString(missingImport.type) << "\"" << std::endl;
            }
            throw WasmException();
        }

        // Instantiate the module.
        moduleInstance = instantiateModule(
                compartment,
                module,
                std::move(linkResult.resolvedImports),
                filePath.c_str());
        if (!moduleInstance) {
            throw WasmException();
        }

        // Call the module start function, if it has one.
        FunctionInstance *startFunction = getStartFunction(moduleInstance);
        if (startFunction) {
            invokeFunctionChecked(context, startFunction, {});
        }

        // Call the Emscripten global initalizers.
        Emscripten::initializeGlobals(context, module, moduleInstance);

        // Extract the module's exported function
        // Note that emscripten can add an underscore before the function name
        FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_" + ENTRYPOINT_FUNC));
        }

        // Construct the arguments
        std::vector<Value> invokeArgs;

        // Inject args
        std::vector<const char*> argStrings;
        Emscripten::injectCommandArgs(emscriptenInstance,argStrings,invokeArgs);

        // Make the call
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);
    }

    char* WasmModule::resultToCharPtr() {
        // Assume return value is a pointer from the function
        // i.e. an offset from the base of the default memory
        I32 offset = functionResults[0].i32;

        // Get the address held at this offset in the default memory
        MemoryInstance *mem = moduleInstance->defaultMemory;
        U8 *valueAddr = mem->baseAddress + offset;

        // Check this belongs to the module
        U8 *memoryEnd = mem->baseAddress + mem->endOffset;
        U8 *maxValueAddr = valueAddr + sizeof(char*);
        if (maxValueAddr >= memoryEnd) {
            throw InvalidResultException();
        }

        // Cast to a char ptr
        char* value = (char*) valueAddr;

        return value;
    }
}