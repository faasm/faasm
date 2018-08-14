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

    size_t WasmModule::getChainCount() {
        return chainNames.size();
    }

    std::string WasmModule::getChainName(const size_t &idx) {
        return chainNames.at(idx);
    }

    std::vector<U8> WasmModule::getChainData(const size_t &idx) {
        return chainData.at(idx);
    }


    int WasmModule::execute(message::FunctionCall &call) {
        std::string filePath = infra::getFunctionFile(call);

        Module module;
        if (!loadModule(filePath.c_str(), module)) {
            std::cerr << "Could not load module at:  " << filePath << "\n";

            throw WasmException();
        }

        std::string input = call.inputdata();

        // Define input data segment
        DataSegment inputDataSegment;
        inputDataSegment.memoryIndex = (Uptr) 0;
        inputDataSegment.baseOffset = InitializerExpression((I32) INPUT_START);
        inputDataSegment.data = util::stringToBytes(input);

        // Define output data segment
        DataSegment outputDataSegment;
        outputDataSegment.memoryIndex = (Uptr) 0;
        outputDataSegment.baseOffset = InitializerExpression((I32) OUTPUT_START);

        std::vector<U8> outputPlaceholder;
        outputPlaceholder.reserve(MAX_OUTPUT_BYTES);
        outputDataSegment.data = outputPlaceholder;

        // Define chaining names segment
        DataSegment chainingNamesSegment;
        chainingNamesSegment.memoryIndex = (Uptr) 0;
        chainingNamesSegment.baseOffset = InitializerExpression((I32) CHAIN_NAMES_START);

        std::vector<U8> chainingNamesData;
        chainingNamesData.reserve(MAX_CHAIN_NAME_BYTES);
        chainingNamesSegment.data = chainingNamesData;

        // Define chaining segment
        DataSegment chainingDataSegment;
        chainingDataSegment.memoryIndex = (Uptr) 0;
        chainingDataSegment.baseOffset = InitializerExpression((I32) CHAIN_DATA_START);

        std::vector<U8> chainingData;
        chainingData.reserve(MAX_CHAIN_DATA_BYTES);
        chainingDataSegment.data = chainingData;

        // Add segments to module
        module.dataSegments.push_back(inputDataSegment);
        module.dataSegments.push_back(outputDataSegment);
        module.dataSegments.push_back(chainingNamesSegment);
        module.dataSegments.push_back(chainingDataSegment);

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

        // Set up regions
        std::vector<Value> invokeArgs;
        invokeArgs.emplace_back((I32) INPUT_START);
        invokeArgs.emplace_back((I32) OUTPUT_START);
        invokeArgs.emplace_back((I32) CHAIN_NAMES_START);
        invokeArgs.emplace_back((I32) CHAIN_DATA_START);

        // Make the call
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Get output data
        U8 *rawOutput = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) OUTPUT_START);
        std::vector<U8> outputData(rawOutput, rawOutput + MAX_OUTPUT_BYTES);
        util::trimTrailingZeros(outputData);
        call.set_outputdata(outputData.data(), outputData.size());

        // Check for chained calls. Note that we reserve chunks for each and can iterate
        // through them checking where the names are set
        U8 *rawChainNames = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_NAMES_START);
        U8 *rawChaininputs = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_DATA_START);

        // Extract the chaining requests
        for (int i = 0; i < MAX_CHAINS; i++) {
            // Get the function name
            int nameStart = (i * MAX_NAME_LENGTH);
            std::string thisName((char*)&rawChainNames[nameStart]);

            // Extract data without trailing zeros
            int dataStart = (i * MAX_INPUT_BYTES);
            std::vector<U8> thisData(&rawChaininputs[dataStart],
                                     &rawChaininputs[dataStart + MAX_INPUT_BYTES]);
            util::trimTrailingZeros(thisData);

            // Stop if we have an empty name
            if(thisName.empty()) {
                break;
            }

            chainNames.push_back(thisName);
            chainData.push_back(thisData);
        }

        return functionResults[0].u32;
    }
}