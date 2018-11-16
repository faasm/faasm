#include "wasm.h"

#include <prof/prof.h>

#include <syscall.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>


using namespace WAVM;

namespace wasm {
    static thread_local WasmModule *executingModule;
    static thread_local message::FunctionCall *executingCall;
    static thread_local CallChain *executingCallChain;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    message::FunctionCall *getExecutingCall() {
        return executingCall;
    }

    CallChain *getExecutingCallChain() {
        return executingCallChain;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() {
//        delete compartment;
//        delete context;
    }

    void WasmModule::initialise() {
        const auto &t = prof::startTimer();

        // Treat any unhandled exception (e.g. in a thread) as a fatal error.
        Runtime::setUnhandledExceptionHandler([](Runtime::Exception &&exception) {
            Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
        });

        compartment = Runtime::createCompartment();
        context = Runtime::createContext(compartment);
        resolver = new RootResolver();

        // Link with intrinsics (independent of module)
        Intrinsics::Module &moduleRef = INTRINSIC_MODULE_REF(env);

        Runtime::ModuleInstance *envModule = Intrinsics::instantiateModule(
                compartment,
                moduleRef,
                "env"
        );

        // Prepare name resolution
        resolver->moduleNameToInstanceMap.set("env", envModule);
        prof::logEndTimer("pre-init", t);
    }

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::FunctionCall &call, CallChain &callChain) {
        if(compartment == nullptr) {
            throw std::runtime_error("Must initialise module before executing");
        }

        //TODO this referencing could be done better
        // Set reference to self
        executingModule = this;
        executingCall = &call;
        executingCallChain = &callChain;

        Runtime::ModuleInstance *moduleInstance = this->load(call);

        // Extract the module's exported function
        // Note that an underscore may be added before the function name by the compiler
        Runtime::Function *functionInstance = asFunctionNullable(
                getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));

        if (!functionInstance) {
            std::string errorMsg = "No exported function \"" + ENTRYPOINT_FUNC + "\"";
            throw std::runtime_error(errorMsg);
        }

        // Set up public properties
        this->defaultMemory = getDefaultMemory(moduleInstance);

        // Make the call
        const auto &t = prof::startTimer();
        int exitCode = 0;
        std::vector<IR::Value> invokeArgs;
        try {
            invokeFunctionChecked(context, functionInstance, invokeArgs);
        }
        catch (wasm::WasmExitException &e) {
            exitCode = e.exitCode;
        }
        prof::logEndTimer("exec", t);

        // Tidy up
        Runtime::collectCompartmentGarbage(compartment);

        return exitCode;
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

    Runtime::ModuleInstance *WasmModule::load(message::FunctionCall &call) {
        // Parse the wasm file to work out imports, function signatures etc.
        this->parseWasm(call);

        // Set up minimum memory size
        this->module.memories.defs[0].type.size.min = (U64) MIN_MEMORY_PAGES;

        // Linking
        const auto &t1 = prof::startTimer();
        Runtime::LinkResult linkResult = linkModule(module, *resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            throw std::runtime_error("Failed linking module");
        }

        prof::logEndTimer("link", t1);

        // Load the object file
        const auto &t2 = prof::startTimer();
        std::vector<uint8_t> objectFileBytes = infra::getFunctionObjectBytes(call);
        prof::logEndTimer("load-bytes", t2);

        const auto &t3 = prof::startTimer();
        Runtime::ModuleRef compiledModule = Runtime::loadPrecompiledModule(module, objectFileBytes);
        prof::logEndTimer("load-obj", t3);

        // Instantiate the module, i.e. create memory, tables etc.
        const auto &t4 = prof::startTimer();
        std::string moduleName = call.user() + " - " + call.function();
        Runtime::ModuleInstance *moduleInstance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                moduleName.c_str()
        );
        prof::logEndTimer("instantiate", t4);

        return moduleInstance;
    }

    /**
     * Parse the WASM file to work out functions, exports, imports etc.
     */
    void WasmModule::parseWasm(message::FunctionCall &call) {
        const auto &t = prof::startTimer();

        std::vector<U8> fileBytes;
        std::string filePath = infra::getFunctionFile(call);
        if (!loadFile(filePath.c_str(), fileBytes)) {
            std::cerr << "Could not load module at:  " << filePath << std::endl;
        }

        WASM::loadBinaryModule(fileBytes.data(), fileBytes.size(), this->module);

        prof::logEndTimer("parse-wasm", t);
    }

}