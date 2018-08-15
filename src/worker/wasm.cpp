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
     * Dummy function to allow printing a message from a wasm function
     */
    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_faasmPrint", void, faasmPrint, I32 messagePtr) {
        // Assume default memory
        MemoryInstance *memory = Runtime::getMemoryFromRuntimeData(contextRuntimeData, 0);

        U8 *message = &memoryRef<U8>(memory, (Uptr) messagePtr);

        std::cout << "Message - " << message << std::endl;
    }

    WasmModule::WasmModule() {

    }

    int WasmModule::execute(message::FunctionCall &call) {
        this->load(call);

        // Make the call
        std::vector<Value> invokeArgs = buildInvokeArgs();
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Retrieve output data
        this->setOutputData(call);

        // Retrieve chaining data
        this->setUpChainingData();

        return functionResults[0].u32;
    }


    void WasmModule::load(message::FunctionCall &call) {
        std::string filePath = infra::getFunctionFile(call);

        module = new Module();

        if (!loadModule(filePath.c_str(), *module)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;

            throw WasmException();
        }

        // Construct the module
        this->buildModule(call);
    }

    void WasmModule::buildModule(message::FunctionCall &call) {
        // Define input data segment
        const std::string &inputStr = call.inputdata();
        std::vector<U8> inputBytes = util::stringToBytes(inputStr);
        this->addDataSegment(INPUT_START, inputBytes);

        // Define output data segment
        this->addDataSegment(OUTPUT_START);

        // Define chaining segments
        this->addDataSegment(CHAIN_NAMES_START);
        this->addDataSegment(CHAIN_DATA_START);

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, *module);

        // Faasm module set-up
        ModuleInstance *faasmModule = Intrinsics::instantiateModule(compartment, INTRINSIC_MODULE_REF(faasm), "faasm");

        if (emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("faasm", faasmModule);
            rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
            rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
            rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
        }

        // Linking
        LinkResult linkResult = linkModule(*module, rootResolver);
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
                *module,
                std::move(linkResult.resolvedImports),
                filePath.c_str()
        );
        if (!moduleInstance) {
            throw WasmException();
        }

        // Call the Emscripten global initalizers.
        Emscripten::initializeGlobals(context, *module, moduleInstance);

        // Extract the module's exported function
        // Note that emscripten can add an underscore before the function name
        functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_" + ENTRYPOINT_FUNC));
        }
    }

    void WasmModule::addDataSegment(int offset) {
        // Define the data segment
        DataSegment segment;
        segment.memoryIndex = (Uptr) 0;
        segment.baseOffset = InitializerExpression((I32) offset);

        module->dataSegments.push_back(segment);
    }

    std::vector<Value> WasmModule::buildInvokeArgs() {
        // Set up regions
        std::vector<Value> invokeArgs;
        invokeArgs.emplace_back((I32) INPUT_START);
        invokeArgs.emplace_back((I32) OUTPUT_START);
        invokeArgs.emplace_back((I32) CHAIN_NAMES_START);
        invokeArgs.emplace_back((I32) CHAIN_DATA_START);

        return invokeArgs;
    }

    void WasmModule::addDataSegment(int offset, std::vector<U8> &initialData) {
        this->addDataSegment(offset);

        // Set the initial data
        module->dataSegments.back().data = initialData;
    }

    void WasmModule::setOutputData(message::FunctionCall &call) {
        // Get output data
        U8 *rawOutput = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) OUTPUT_START);
        std::vector<U8> outputData(rawOutput, rawOutput + MAX_OUTPUT_BYTES);
        util::trimTrailingZeros(outputData);
        call.set_outputdata(outputData.data(), outputData.size());
    }

    void WasmModule::setUpChainingData() {
        // Check for chained calls. Note that we reserve chunks for each and can iterate
        // through them checking where the names are set
        U8 *rawChainNames = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_NAMES_START);
        U8 *rawChaininputs = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_DATA_START);

        // Extract the chaining requests
        for (int i = 0; i < MAX_CHAINS; i++) {
            // Get the function name
            int nameStart = (i * MAX_NAME_LENGTH);
            std::string thisName((char *) &rawChainNames[nameStart]);

            // Extract data without trailing zeros
            int dataStart = (i * MAX_INPUT_BYTES);
            std::vector<U8> thisData(&rawChaininputs[dataStart],
                                     &rawChaininputs[dataStart + MAX_INPUT_BYTES]);
            util::trimTrailingZeros(thisData);

            // Stop if we have an empty name
            if (thisName.empty()) {
                break;
            }

            chainNames.push_back(thisName);
            chainData.push_back(thisData);
        }
    }

    void WasmModule::clean() {
        // Clear up
        Runtime::collectGarbage();

        delete module;
        delete moduleInstance;
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


}