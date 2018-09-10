#include "worker.h"
#include "intrinsic.h"
#include "resolver.h"

#include <syscall.h>

#include "Emscripten/Emscripten.h"
#include "Inline/CLI.h"

#include "Runtime/RuntimePrivate.h"


namespace worker {
    WasmModule::WasmModule() = default;

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call) {
        Runtime::ModuleInstance *moduleInstance = this->load(call);

        // Call the Emscripten global initializers.
        Runtime::Context* context = Runtime::createContext(moduleInstance->compartment);
        Emscripten::initializeGlobals(context, module, moduleInstance);

        // Extract the module's exported function
        // Note that emscripten can add an underscore before the function name
        Runtime::FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_" + ENTRYPOINT_FUNC));
        }

        // Set up input data in module memory
        this->addInputData(moduleInstance, call);

        // Make the call
        std::vector<IR::Value> invokeArgs = buildInvokeArgs();
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Retrieve output data
        this->extractOutputData(moduleInstance, call);

        // Retrieve chaining data
        this->extractChainingData(moduleInstance, call);

        return functionResults[0].u32;
    }

    Runtime::ModuleInstance* WasmModule::load(message::FunctionCall &call) {
        // Load the wasm file
        this->loadWasm(call);

        // Set up module's memory
        this->setUpMemory(call);

        // Link
        Runtime::Compartment *compartment = Runtime::createCompartment();
        Runtime::LinkResult linkResult = this->link(compartment);

        // Compile the module
        Runtime::Module *compiledModule = Runtime::compileModule(module);

        // Instantiate the module, i.e. create memory, tables etc.
        std::string moduleName = call.user() + " - " + call.function();
        Runtime::ModuleInstance *moduleInstance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                moduleName.c_str()
        );

        return moduleInstance;
    }

    /**
     * Load in the wasm binary file
     */
    void WasmModule::loadWasm(message::FunctionCall &call) {
        std::vector<U8> fileBytes;
        std::string filePath = infra::getFunctionFile(call);
        if (!loadFile(filePath.c_str(), fileBytes)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;
        }

        loadBinaryModule(fileBytes.data(), fileBytes.size(), module);
    }

    /**
     * Generic module set-up
     */
    void WasmModule::setUpMemory(message::FunctionCall &call) {
        // Define input data segment
        this->addDataSegment(INPUT_START);

        // Define output data segment
        this->addDataSegment(OUTPUT_START);

        // Define chaining segments
        this->addDataSegment(CHAIN_NAMES_START);
        this->addDataSegment(CHAIN_DATA_START);
    }

    /**
     * Link the module with the environment
     */
    Runtime::LinkResult WasmModule::link(Runtime::Compartment *compartment) {
        RootResolver resolver(compartment);

        // Set up the Emscripten module
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, module);

        // Set up the Faasm module
        Runtime::ModuleInstance *faasmModule = Intrinsics::instantiateModule(compartment, INTRINSIC_MODULE_REF(faasm),
                                                                             "faasm");
        // Prepare name resolution
        resolver.moduleNameToInstanceMap.set("faasm", faasmModule);
        resolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
        resolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
        resolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);

        // Linking
        Runtime::LinkResult linkResult = linkModule(module, resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            for (auto &missingImport : linkResult.missingImports) {
                std::cerr << "Missing import: module=\"" << missingImport.moduleName
                          << "\" export=\"" << missingImport.exportName
                          << "\" type=\"" << asString(missingImport.type) << "\"" << std::endl;
            }
            throw WasmException();
        }

        return linkResult;
    }

    /**
     * Adds a data segment to the module
     */
    void WasmModule::addDataSegment(int offset) {
        IR::DataSegment segment;

        // Note - using default memory
        segment.memoryIndex = (Uptr) 0;
        segment.baseOffset = IR::InitializerExpression((I32) offset);

        module.dataSegments.push_back(segment);
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

    void WasmModule::addInputData(Runtime::ModuleInstance *moduleInstance, message::FunctionCall &call) {
        const std::string &inputString = call.inputdata();
        std::cout << "Received input: " << inputString << std::endl;

        if(inputString.size() > MAX_INPUT_BYTES) {
            throw(WasmException());
        }

        const std::vector<uint8_t> &inputBytes = util::stringToBytes(inputString);

        // Copy input data into place
        U8 *inputStart = &Runtime::memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) INPUT_START);
        std::copy(inputBytes.begin(), inputBytes.end(), inputStart);
    }

    /**
     * Extracts output data from module and sets it on the function call
     */
    void WasmModule::extractOutputData(Runtime::ModuleInstance *moduleInstance, message::FunctionCall &call) {
        U8 *rawOutput = &Runtime::memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) OUTPUT_START);
        std::vector<U8> outputData(rawOutput, rawOutput + MAX_OUTPUT_BYTES);
        util::trimTrailingZeros(outputData);

        call.set_outputdata(outputData.data(), outputData.size());
    }

    /**
     * Extracts chaining data form module and performs the necessary chained calls
     */
    void WasmModule::extractChainingData(Runtime::ModuleInstance *moduleInstance, const message::FunctionCall &originalCall) {
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