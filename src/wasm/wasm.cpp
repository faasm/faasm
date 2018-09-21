#include "wasm.h"
#include "resolver.h"

#include <syscall.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>


using namespace WAVM;

namespace wasm {
    WasmModule::WasmModule() = default;

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call) {
        Runtime::Compartment *compartment = Runtime::createCompartment();
        Runtime::Context *context = Runtime::createContext(compartment);

        Runtime::ModuleInstance *moduleInstance = this->load(call, compartment);

        // Extract the module's exported function
        // Note that an underscore may be added before the function name by the compiler
        Runtime::FunctionInstance *functionInstance = asFunctionNullable(
                getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));
        if (!functionInstance) {
            std::string errorMsg = "No function named " + ENTRYPOINT_FUNC + " found";
            throw std::runtime_error(errorMsg);
        }

        // Set up input data in module memory
        this->addInputData(call);

        moduleMemory = getDefaultMemory(moduleInstance);

        // Make the call
        std::vector<IR::Value> invokeArgs = buildInvokeArgs();
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Retrieve output data
        this->extractOutputData(call);

        // Retrieve chaining data
        this->extractChainingData(call);

        return functionResults[0].u32;
    }

    std::vector<uint8_t> WasmModule::compile(message::FunctionCall &call) {
        // Parse the wasm file to work out imports, function signatures etc.
        WasmModule tempModule;
        tempModule.parseWasm(call);

        // Compile the module to object code
        Runtime::Module *module = Runtime::compileModule(tempModule.module);
        return Runtime::getObjectCode(module);
    }

    Runtime::ModuleInstance *WasmModule::load(message::FunctionCall &call, Runtime::Compartment *compartment) {
        // Parse the wasm file to work out imports, function signatures etc.
        this->parseWasm(call);

        // Define module's memory segments
        this->setUpMemory(call);

        // Link with intrinsics
        Runtime::LinkResult linkResult = this->link(compartment);

        // Load the object file
        std::vector<uint8_t> objectFileBytes = infra::getFunctionObjectBytes(call);
        Runtime::Module *compiledModule = Runtime::loadPrecompiledModule(module, objectFileBytes);

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
     * Parse the WASM file to work out functions, exports, imports etc.
     */
    void WasmModule::parseWasm(message::FunctionCall &call) {
        std::vector<U8> fileBytes;
        std::string filePath = infra::getFunctionFile(call);
        if (!loadFile(filePath.c_str(), fileBytes)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;
        }

        WASM::loadBinaryModule(fileBytes.data(), fileBytes.size(), this->module);
    }

    /**
     * Generic module set-up
     */
    void WasmModule::setUpMemory(message::FunctionCall &call) {
        // Make sure we have a big enough minimum memory size
        //this->module.memories.defs[0].type.size.min = MIN_MEMORY_SIZE;

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

        Runtime::ModuleInstance *envModule = Intrinsics::instantiateModule(
                compartment,
                INTRINSIC_MODULE_REF(env),
                "env"
        );

        // Prepare name resolution
        resolver.moduleNameToInstanceMap.set("env", envModule);

        // Linking
        Runtime::LinkResult linkResult = linkModule(module, resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            throw std::runtime_error("Failed linking module");
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

    void WasmModule::addInputData(message::FunctionCall &call) {
        const std::string &inputString = call.inputdata();
        std::cout << "Received input: " << inputString << std::endl;

        if (inputString.size() > MAX_INPUT_BYTES) {
            throw (std::runtime_error("Input data too large"));
        }

        const std::vector<uint8_t> &inputBytes = util::stringToBytes(inputString);

        // Copy input data into place
        U8 *inputStart = &Runtime::memoryRef<U8>(moduleMemory, (Uptr) INPUT_START);
        std::copy(inputBytes.begin(), inputBytes.end(), inputStart);
    }

    /**
     * Extracts output data from module and sets it on the function call
     */
    void WasmModule::extractOutputData(message::FunctionCall &call) {
        U8 *rawOutput = &Runtime::memoryRef<U8>(moduleMemory, (Uptr) OUTPUT_START);
        std::vector<U8> outputData(rawOutput, rawOutput + MAX_OUTPUT_BYTES);
        util::trimTrailingZeros(outputData);

        call.set_outputdata(outputData.data(), outputData.size());
    }

    /**
     * Extracts chaining data form module and performs the necessary chained calls
     */
    void WasmModule::extractChainingData(const message::FunctionCall &originalCall) {
        // Check for chained calls. Note that we reserve chunks for each and can iterate
        // through them checking where the names are set
        U8 *rawChainNames = &Runtime::memoryRef<U8>(moduleMemory, (Uptr) CHAIN_NAMES_START);
        U8 *rawChaininputs = &Runtime::memoryRef<U8>(moduleMemory, (Uptr) CHAIN_DATA_START);

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