#include "wasm.h"
#include "resolver.h"

#include <syscall.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>


using namespace WAVM;

namespace wasm {
    WasmModule::WasmModule() = default;

    thread_local static Runtime::Memory *moduleMemory = nullptr;
    thread_local message::FunctionCall *moduleCall;

    Runtime::Memory *getModuleMemory() {
        return moduleMemory;
    }

    message::FunctionCall* getModuleCall() {
        return moduleCall;
    }

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call) {
        // Treat any unhandled exception (e.g. in a thread) as a fatal error.
        Runtime::setUnhandledExceptionHandler([](Runtime::Exception&& exception) {
            Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
        });

        Runtime::Compartment *compartment = Runtime::createCompartment();
        Runtime::Context *context = Runtime::createContext(compartment);

        Runtime::ModuleInstance *moduleInstance = this->load(call, compartment);

        // Extract the module's exported function
        // Note that an underscore may be added before the function name by the compiler
        Runtime::Function *functionInstance = asFunctionNullable(
                getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));

        if (!functionInstance) {
            // Try CPP compiler name instead
            functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, CPP_ENTRYPOINT_FUNC));

            if (!functionInstance) {
                std::string errorMsg = "No exported function \"" + ENTRYPOINT_FUNC + "\"";
                errorMsg += " (or \"" + CPP_ENTRYPOINT_FUNC + "\") found";
                throw std::runtime_error(errorMsg);
            }
        }

        // Set up module-specific values
        moduleMemory = getDefaultMemory(moduleInstance);
        moduleCall = &call;

        // Set up input data in module memory
        this->addInputData(call);

        // Make the call
        std::vector<IR::Value> invokeArgs = buildInvokeArgs();
        functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Retrieve output data
        this->extractOutputData(call);

        // Retrieve chaining data
        this->extractChainingData(call);

        // Tidy up
        Runtime::collectCompartmentGarbage(compartment);

        return functionResults[0].u32;
    }

    std::vector<uint8_t> WasmModule::compile(message::FunctionCall &call) {
        // Parse the wasm file to work out imports, function signatures etc.
        WasmModule tempModule;
        tempModule.parseWasm(call);

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(tempModule.module);
        return Runtime::getObjectCode(module);
    }

    void WasmModule::compileToObjectFile(message::FunctionCall &call) {
        std::vector<uint8_t> objBytes = wasm::WasmModule::compile(call);
        std::string objFilePath = infra::getFunctionObjectFile(call);
        util::writeBytesToFile(objFilePath, objBytes);
    }

    Runtime::ModuleInstance *WasmModule::load(message::FunctionCall &call, Runtime::Compartment *compartment) {
        // Parse the wasm file to work out imports, function signatures etc.
        this->parseWasm(call);

        // Define module's memory segments
        this->setUpMemoryDefinitions(call);

        // Link with intrinsics
        Runtime::LinkResult linkResult = this->link(compartment);

        // Load the object file
        std::vector<uint8_t> objectFileBytes = infra::getFunctionObjectBytes(call);
        Runtime::ModuleRef compiledModule = Runtime::loadPrecompiledModule(module, objectFileBytes);

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
    void WasmModule::setUpMemoryDefinitions(message::FunctionCall &call) {
        // Make sure we have a big enough minimum memory size
        this->module.memories.defs[0].type.size.min = (U64) MIN_MEMORY_PAGES;

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
        RootResolver resolver;

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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string &inputString = call.inputdata();

        if (inputString.size() > MAX_INPUT_BYTES) {
            logger->error("Input data too large");
            throw (std::runtime_error("Input data too large"));
        }

        logger->debug("Received input {}", call.inputdata());

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
        U8 *rawChainNames = Runtime::memoryArrayPtr<U8>(moduleMemory, (Uptr) CHAIN_NAMES_START,
                                                        (Uptr) MAX_CHAIN_NAME_BYTES);
        U8 *rawChainInputs = Runtime::memoryArrayPtr<U8>(moduleMemory, (Uptr) CHAIN_DATA_START,
                                                         (Uptr) MAX_CHAIN_DATA_BYTES);

        // Extract the chaining requests
        for (int i = 0; i < MAX_CHAINS; i++) {
            int nameStart = i * MAX_NAME_LENGTH;
            int nameEnd = nameStart + MAX_NAME_LENGTH;

            int inputStart = i * MAX_INPUT_BYTES;
            int inputEnd = inputStart + MAX_INPUT_BYTES;

            std::vector<U8> thisChainName(rawChainNames + nameStart, rawChainNames + nameEnd);
            std::vector<U8> thisChainData(rawChainInputs + inputStart, rawChainInputs + inputEnd);

            // Trim zeroes
            util::trimTrailingZeros(thisChainName);

            // Stop if we have an empty name
            if (thisChainName.empty()) {
                break;
            }

            util::trimTrailingZeros(thisChainData);

            // Create call and add to list
            message::FunctionCall chainedCall;
            chainedCall.set_user(originalCall.user());
            chainedCall.set_function((char *) thisChainName.data());
            chainedCall.set_inputdata(thisChainData.data(), thisChainData.size());

            chainedCalls.push_back(chainedCall);
        }
    }
}