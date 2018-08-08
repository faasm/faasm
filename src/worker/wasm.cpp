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
    WasmModule::WasmModule() {

    }

    void WasmModule::clean() {
        // Clear up
        Runtime::collectGarbage();
    }

    int WasmModule::execute(message::FunctionCall &call, std::vector<U8> &input) {
        std::string filePath = infra::getFunctionFile(call);

        Module module;
        if (!loadModule(filePath.c_str(), module)) {
            std::cerr << "Could not load module at:  " << filePath << "\n";

            throw WasmException();
        }

        // Define input and output data segments
        DataSegment inputDataSegment;
        inputDataSegment.memoryIndex = (Uptr) 0;
        inputDataSegment.baseOffset = InitializerExpression((I32) 0);
        inputData = input;
        inputDataSegment.data = inputData;

        DataSegment outputDataSegment;
        outputDataSegment.memoryIndex = (Uptr) 0;
        outputDataSegment.baseOffset = InitializerExpression((I32) INPUT_MAX_BYTES);
        outputData.reserve(OUTPUT_MAX_BYTES);
        outputDataSegment.data = outputData;

        module.dataSegments.push_back(inputDataSegment);
        module.dataSegments.push_back(outputDataSegment);

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

        // Extract the module's exported function
        // Note that emscripten can add an underscore before the function name
        FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_" + ENTRYPOINT_FUNC));
        }

        // Construct the arguments
        std::vector<Value> invokeArgs;
        inputStart = (I32) 0;
        inputLength = (I32) INPUT_MAX_BYTES;
        outputStart = (I32) INPUT_MAX_BYTES;
        outputLength = (I32) OUTPUT_MAX_BYTES;

        invokeArgs.emplace_back(inputStart);
        invokeArgs.emplace_back(inputLength);
        invokeArgs.emplace_back(outputStart);
        invokeArgs.emplace_back(outputLength);

        // Make the call
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Set up output data
        U8 *rawOutput = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) outputStart);
        std::vector<U8> output(rawOutput, rawOutput + OUTPUT_MAX_BYTES);
        outputData = output;

        return functionResults[0].u32;
    }
}