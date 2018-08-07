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

    void WasmModule::initModule(message::FunctionCall &call) {
        std::string filePath = infra::getFunctionFile(call);
        std::cout << "Executing function at:  " << filePath << "\n";

        Module module;
        if (!loadModule(filePath.c_str(), module)) {
            std::cerr << "Could not load module at:  " << filePath << "\n";

            throw WasmException();
        }

        // Add input data
        std::vector<U8> inputData = {1, 2, 3, 4};
        addDataSegment(module, inputData);

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        context = Runtime::createContext(compartment);
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

    }

    void WasmModule::execute(message::FunctionCall &call) {
        std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";

        this->initModule(call);

        // Extract the module's exported function
        // Note that emscripten can add an underscore before the function name
        FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_" + ENTRYPOINT_FUNC));
        }

        // Construct the arguments
        std::vector<Value> invokeArgs;

        // Make the call
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);
    }

    void WasmModule::addDataSegment(Module module, std::vector<U8> &inputData) {
        DataSegment dataSegment;
        InitializerExpression baseOffset((I32) 0);
        dataSegment.memoryIndex = (Uptr) 0;
        dataSegment.baseOffset = baseOffset;
        dataSegment.data = inputData;

        module.dataSegments.push_back(dataSegment);
    }

    char* WasmModule::resultToCharPtr() {
        // Assume return value is a pointer from the function
        // i.e. an offset from the base of the default memory
        Uptr offset = functionResults[0].u32;
        MemoryInstance *mem = moduleInstance->defaultMemory;

        char* value = &memoryRef<char>(mem, offset);

        return value;
    }
}