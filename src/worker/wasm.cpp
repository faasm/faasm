#include "worker.h"
#include "intrinsic.h"
#include "resolver.h"

#include <syscall.h>

#include "Emscripten/Emscripten.h"
#include "Inline/CLI.h"
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"
#include "Runtime/RuntimePrivate.h"


/**
 * NOTE: This was derived from the existing wavm CLI at
 * https://github.com/AndrewScheidecker/WAVM/blob/master/Source/Programs/wavm.cpp
 */

namespace worker {
    WasmModule::WasmModule() = default;

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call) {
        this->load(call);

        // Make the call
        std::vector<IR::Value> invokeArgs = buildInvokeArgs();
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Retrieve output data
        this->setOutputData(call);

        // Retrieve chaining data
        this->setUpChainingData(call);

        return functionResults[0].u32;
    }

    /**
     * Sets up the module ready for execution
     */
    void WasmModule::load(message::FunctionCall &call) {
        std::string filePath = infra::getFunctionFile(call);

        module = new IR::Module();
        std::vector<U8> fileBytes;
        if(!loadFile(filePath.c_str(), fileBytes)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;
        }

        loadBinaryModule(fileBytes.data(), fileBytes.size(), *module);

        // Define input data segment
        const std::string &inputStr = call.inputdata();
        std::cout << "Received input: " << call.inputdata() << std::endl;
        std::vector<U8> inputBytes = util::stringToBytes(inputStr);
        this->addDataSegment(INPUT_START, inputBytes);

        // Define output data segment
        this->addDataSegment(OUTPUT_START);

        // Define chaining segments
        this->addDataSegment(CHAIN_NAMES_START);
        this->addDataSegment(CHAIN_DATA_START);

        // Link the module with the intrinsic modules.
        Runtime::Compartment *compartment = Runtime::createCompartment();
        context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, *module);

        // Faasm module set-up
        Runtime::ModuleInstance *faasmModule = Intrinsics::instantiateModule(compartment, INTRINSIC_MODULE_REF(faasm), "faasm");

        if (emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("faasm", faasmModule);
            rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
            rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
            rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
        }

        // Linking
        Runtime::LinkResult linkResult = linkModule(*module, rootResolver);
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
                Runtime::compileModule(*module),
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

    /**
     * Adds a data segment to the module
     */
    void WasmModule::addDataSegment(int offset) {
        IR::DataSegment segment;

        // Note - using default memory
        segment.memoryIndex = (Uptr) 0;
        segment.baseOffset = IR::InitializerExpression((I32) offset);

        module->dataSegments.push_back(segment);
    }

    /**
     * Adds a data segment with initial data
     */
    void WasmModule::addDataSegment(int offset, std::vector<U8> &initialData) {
        this->addDataSegment(offset);

        // Set the initial data
        module->dataSegments.back().data = initialData;
    }

    /**
     * Sets up arguments to be passed to the invocation of the function
     */
    std::vector<IR::Value> WasmModule::buildInvokeArgs() {
        // Set up regions
        std::vector<IR::Value> invokeArgs;
        invokeArgs.emplace_back((I32) INPUT_START);
        invokeArgs.emplace_back((I32) OUTPUT_START);
        invokeArgs.emplace_back((I32) CHAIN_NAMES_START);
        invokeArgs.emplace_back((I32) CHAIN_DATA_START);

        return invokeArgs;
    }

    /**
     * Extracts output data from module and sets it on the function call
     */
    void WasmModule::setOutputData(message::FunctionCall &call) {
        U8 *rawOutput = &Runtime::memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) OUTPUT_START);
        std::vector<U8> outputData(rawOutput, rawOutput + MAX_OUTPUT_BYTES);
        util::trimTrailingZeros(outputData);

        call.set_outputdata(outputData.data(), outputData.size());
    }

    /**
     * Extracts chaining data form module and performs the necessary chained calls
     */
    void WasmModule::setUpChainingData(const message::FunctionCall &originalCall) {
        // Check for chained calls. Note that we reserve chunks for each and can iterate
        // through them checking where the names are set
        U8 *rawChainNames = &Runtime::memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_NAMES_START);
        U8 *rawChaininputs = &Runtime::memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_DATA_START);

        // Extract the chaining requests
        for (int i = 0; i < MAX_CHAINS; i++) {
            // Get the function name
            int nameStart = (i * MAX_NAME_LENGTH);
            std::string thisName((char *) &rawChainNames[nameStart]);

            // Stop if we have an empty name
            if (thisName.empty()) {
                break;
            }

            // Extract data without trailing zeros
            int dataStart = (i * MAX_INPUT_BYTES);
            std::vector<U8> thisData(&rawChaininputs[dataStart],
                                     &rawChaininputs[dataStart + MAX_INPUT_BYTES]);
            util::trimTrailingZeros(thisData);

            // Create call and add to list
            message::FunctionCall chainedCall;
            chainedCall.set_user(originalCall.user());
            chainedCall.set_function(thisName);
            chainedCall.set_inputdata(thisData.data(), thisData.size());

            chainedCalls.push_back(chainedCall);
        }
    }

    void WasmModule::clean() {
        // Clear up
        Runtime::collectGarbage();
    }
}