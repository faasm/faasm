#include "worker.h"

#include <Runtime/RuntimePrivate.h>
#include "Programs/CLI.h"
#include <Emscripten/Emscripten.h>
#include "resolver.h"

#include <util/util.h>


using namespace IR;
using namespace Runtime;

/**
 * NOTE: This is heavily based on the existing wavm application
 * https://github.com/AndrewScheidecker/WAVM/blob/master/Source/Programs/wavm.cpp
 */

namespace worker {
    // Create module for faasm-specific functions
    DEFINE_INTRINSIC_MODULE(faasm)

    /**
     * Allows functions to call others by specifying the function name and input data
     */
    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_faasmCall", void, faasmCall,
            I32 userPtr, I32 funcPtr, I32 inputPtr, I32 inputLength) {
        // Assume default memory
        MemoryInstance *memory = Runtime::getMemoryFromRuntimeData(contextRuntimeData, 0);

        U8 *user = &memoryRef<U8>(memory, (Uptr) userPtr);
        U8 *funcName = &memoryRef<U8>(memory, (Uptr) funcPtr);
        U8 *funcData = &memoryRef<U8>(memory, (Uptr) inputPtr);

        std::cout << user << " - " << funcName << " | " << funcData << " | " << asString(inputLength) << std::endl;
    }

    WasmModule::WasmModule() {

    }

    void WasmModule::clean() {
        // Clear up
        Runtime::collectGarbage();
    }

    int WasmModule::execute(message::FunctionCall &call) {
        std::string filePath = infra::getFunctionFile(call);

        Module module;
        if (!loadModule(filePath.c_str(), module)) {
            std::cerr << "Could not load module at:  " << filePath << "\n";

            throw WasmException();
        }

        std::string input = call.inputdata();

        // Define input and output data segments
        DataSegment inputDataSegment;
        inputDataSegment.memoryIndex = (Uptr) 0;
        inputDataSegment.baseOffset = InitializerExpression((I32) 0);
        inputDataSegment.data = util::stringToBytes(input);

        DataSegment outputDataSegment;
        outputDataSegment.memoryIndex = (Uptr) 0;
        outputDataSegment.baseOffset = InitializerExpression((I32) INPUT_MAX_BYTES);

        std::vector<U8> outputData;
        outputData.reserve(OUTPUT_MAX_BYTES);
        outputDataSegment.data = outputData;

        module.dataSegments.push_back(inputDataSegment);
        module.dataSegments.push_back(outputDataSegment);

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        Context *context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, module);

        // Faasm module set-up
        ModuleInstance *faasmModule = Intrinsics::instantiateModule(compartment, INTRINSIC_MODULE_REF(faasm), "faasm");

        if (emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("faasm", faasmModule);
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
                filePath.c_str()
        );
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

        // Set up output data on function
        U8 *rawOutput = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) outputStart);
        call.set_outputdata(rawOutput, OUTPUT_MAX_BYTES);

        return functionResults[0].u32;
    }
}