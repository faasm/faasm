#include "worker.h"
#include <infra/infra.h>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {

    struct RootResolver : Resolver {
        Compartment *compartment;
        HashMap<std::string, ModuleInstance *> moduleNameToInstanceMap;

        RootResolver(Compartment *inCompartment) : compartment(inCompartment) {}

        bool resolve(const std::string &moduleName, const std::string &exportName, ObjectType type,
                     Object *&outObject) override {
            auto namedInstance = moduleNameToInstanceMap.get(moduleName);
            if (namedInstance) {
                outObject = getInstanceExport(*namedInstance, exportName);
                if (outObject) {
                    if (isA(outObject, type)) { return true; }
                    else {
                        Log::printf(Log::Category::error, "Resolved import %s.%s to a %s, but was expecting %s\n",
                                    moduleName.c_str(),
                                    exportName.c_str(),
                                    asString(getObjectType(outObject)).c_str(),
                                    asString(type).c_str());
                        return false;
                    }
                }
            }

            Log::printf(Log::Category::error, "Generated stub for missing import %s.%s : %s\n", moduleName.c_str(),
                        exportName.c_str(), asString(type).c_str());
            outObject = getStubObject(exportName, type);
            return true;
        }

        Object *getStubObject(const std::string &exportName, ObjectType type) const {
            // If the import couldn't be resolved, stub it in.
            switch (type.kind) {
                case IR::ObjectKind::function: {
                    // Generate a function body that just uses the unreachable op to fault if called.
                    Serialization::ArrayOutputStream codeStream;
                    OperatorEncoderStream encoder(codeStream);
                    encoder.unreachable();
                    encoder.end();

                    // Generate a module for the stub function.
                    Module stubModule;
                    DisassemblyNames stubModuleNames;
                    stubModule.types.push_back(asFunctionType(type));
                    stubModule.functions.defs.push_back({{0}, {}, std::move(codeStream.getBytes()), {}});
                    stubModule.exports.push_back({"importStub", IR::ObjectKind::function, 0});
                    stubModuleNames.functions.push_back({"importStub: " + exportName, {}, {}});

                    IR::setDisassemblyNames(stubModule, stubModuleNames);
                    IR::validateDefinitions(stubModule);

                    // Instantiate the module and return the stub function instance.
                    auto stubModuleInstance = instantiateModule(compartment, stubModule, {}, "importStub");
                    return getInstanceExport(stubModuleInstance, "importStub");
                }
                case IR::ObjectKind::memory: {
                    return asObject(Runtime::createMemory(compartment, asMemoryType(type)));
                }
                case IR::ObjectKind::table: {
                    return asObject(Runtime::createTable(compartment, asTableType(type)));
                }
                case IR::ObjectKind::global: {
                    return asObject(Runtime::createGlobal(
                            compartment,
                            asGlobalType(type),
                            IR::Value(asGlobalType(type).valueType, IR::UntaggedValue())));
                }
                case IR::ObjectKind::exceptionType: {
                    return asObject(Runtime::createExceptionTypeInstance(asExceptionType(type), "importStub"));
                }
                default:
                    Errors::unreachable();
            };
        }
    };

    WasmModule::WasmModule() {

    }

    int WasmModule::execute(message::FunctionCall &call) {
        std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";

        std::string filePath = infra::getFunctionFile(call);

        Module module;
        loadModule(filePath.c_str(), module);

        // Link the module with the intrinsic modules.
        Compartment *compartment = Runtime::createCompartment();
        Context *context = Runtime::createContext(compartment);
        RootResolver rootResolver(compartment);

        // Emscripten set-up
        Emscripten::Instance *emscriptenInstance = Emscripten::instantiate(compartment, module);

        if(emscriptenInstance) {
            rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
            rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
            rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
        }

        // Linking
        LinkResult linkResult = linkModule(module, rootResolver);

        // Instantiate the module.
        ModuleInstance *moduleInstance = instantiateModule(
                compartment,
                module,
                std::move(linkResult.resolvedImports),
                filePath.c_str());

        // Call the module start function, if it has one.
        FunctionInstance *startFunction = getStartFunction(moduleInstance);
        if (startFunction) {
            invokeFunctionChecked(context, startFunction, {});
        }

        // Call the Emscripten global initalizers.
        Emscripten::initializeGlobals(context, module, moduleInstance);

        // Call the module's main function
        FunctionInstance *functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "main"));

        std::vector<Value> invokeArgs;
        IR::ValueTuple functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);

        // Clear up
        Runtime::collectGarbage();

        return 0;
    }
}