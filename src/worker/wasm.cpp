#include "worker.h"

#include <Runtime/RuntimePrivate.h>
#include "Programs/CLI.h"
#include <Emscripten/Emscripten.h>
#include "resolver.h"

#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include <util/util.h>


using namespace IR;
using namespace Runtime;

/**
 * NOTE: This was derived from the existing wavm CLI at
 * https://github.com/AndrewScheidecker/WAVM/blob/master/Source/Programs/wavm.cpp
 */

namespace worker {
    // Create module for faasm-specific functions
    DEFINE_INTRINSIC_MODULE(faasm)

    /**
     * socketcall
     */
    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "___syscall102", I32, ___syscall102, I32 syscallNo, I32 argsPtr) {
        MemoryInstance* memory = getMemoryFromRuntimeData(contextRuntimeData,defaultMemoryId.id);
        U32* args = memoryArrayPtr<U32>(memory, argsPtr, 2);
        U32 call = args[0];
        U32 callArgsPtr = args[1];

        // See mapping in Emscripten repo to see socketcall codes
        // https://github.com/kripken/emscripten/blob/master/system/lib/libc/musl/src/internal/syscall.h#L227
        switch(call) {
            case(1): { // socket
                U32* subCallArgs = memoryArrayPtr<U32>(memory, callArgsPtr, 3);
                U32 domain = subCallArgs[0];
                U32 type = subCallArgs[1];
                U32 protocol = subCallArgs[2];

                printf("SYSCALL - socket %i %i %i\n", domain, type, protocol);
                long sock = syscall(SYS_socket, domain, type, protocol);
                printf("Opened system socket %li\n", sock);

                return (I32) sock;
            }
            default: {
                printf("Unrecognised socketcall %i\n", call);
            }
        }

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall122", I32, ___syscall122, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "___syscall142", I32, ___syscall142, I32 syscallNo, I32 argsPtr) {
//        MemoryInstance* memory = getMemoryFromRuntimeData(contextRuntimeData,defaultMemoryId.id);
//        U32* args = memoryArrayPtr<U32>(memory, argsPtr, 2);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_gethostbyname", I32, _gethostbyname, I32 hostnamePtr) {
        MemoryInstance* memory = getMemoryFromRuntimeData(contextRuntimeData,defaultMemoryId.id);
        auto hostname = &memoryRef<char>(memory, (Uptr) hostnamePtr);

        printf("INTRINSIC - gethostbyname %s\n", hostname);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall195", I32, ___syscall195, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall197", I32, ___syscall197, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall201", I32, ___syscall201, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall221", I32, ___syscall221, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall3", I32, ___syscall3, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall33", I32, ___syscall33, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall4", I32, ___syscall4, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall5", I32, ___syscall5, I32 a, I32 b) {
        return 0;
    }

    /** Struct to fake 32-bit time in wasm modules */
    struct wasm_timespec
    {
        I32 tv_sec;
        I32 tv_nsec;
    };

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_clock_gettime", I32, _clock_gettime, I32 clockId, I32 resultAddress) {
        printf("INTRINSIC - _clock_gettime\n");

        // Get module's default memory
        MemoryInstance* memory = getMemoryFromRuntimeData(contextRuntimeData,defaultMemoryId.id);
        auto result = memoryRef<wasm_timespec>(memory, (Uptr) resultAddress);

        // Fake a clock incrementing by 1 with each call
        static std::atomic<U64> fakeClock;
        const U64 currentClock = fakeClock;

        result.tv_sec = I32(currentClock / 1000000000);
        result.tv_nsec = I32(currentClock % 1000000000);
        ++fakeClock;

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_getaddrinfo", I32, _getaddrinfo, I32 a, I32 b, I32 c, I32 d) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_getpwuid", I32, _getpwuid, I32 a) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_gettimeofday", I32, _gettimeofday, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_gmtime_r", I32, _gmtime_r, I32 a, I32 b) {
        return 0;
    }

    WasmModule::WasmModule() = default;

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call) {
        this->load(call);

        // Make the call
        std::vector<Value> invokeArgs = buildInvokeArgs();
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

        module = new Module();

        if (!loadModule(filePath.c_str(), *module)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;

            throw WasmException();
        }

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

    /**
     * Adds a data segment to the module
     */
    void WasmModule::addDataSegment(int offset) {
        DataSegment segment;

        // Note - using default memory
        segment.memoryIndex = (Uptr) 0;
        segment.baseOffset = InitializerExpression((I32) offset);

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
    std::vector<Value> WasmModule::buildInvokeArgs() {
        // Set up regions
        std::vector<Value> invokeArgs;
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
        U8 *rawOutput = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) OUTPUT_START);
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
        U8 *rawChainNames = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_NAMES_START);
        U8 *rawChaininputs = &memoryRef<U8>(moduleInstance->defaultMemory, (Uptr) CHAIN_DATA_START);

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