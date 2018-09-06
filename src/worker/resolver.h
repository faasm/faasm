#include "Logging/Logging.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Inline/Serialization.h"
#include "IR/Operators.h"
#include "IR/Validate.h"


namespace worker {

    struct RootResolver : Runtime::Resolver {
        Runtime::Compartment *compartment;
        HashMap<std::string, Runtime::ModuleInstance *> moduleNameToInstanceMap;

        RootResolver(Runtime::Compartment *inCompartment) : compartment(inCompartment) {}

        bool resolveFaasm(const std::string &exportName, IR::ObjectType type, Runtime::Object *&outObject) {

            auto faasmModule = moduleNameToInstanceMap.get("faasm");
            outObject = getInstanceExport(*faasmModule, exportName);

            // Unsuccessful lookup
            if (!outObject) {
                return false;
            }

            // Successful lookup of correct type
            if (isA(outObject, type)) {
                Log::printf(Log::Category::debug, "Using Faasm version of %s \n", exportName.c_str());
                return true;
            }

            Log::printf(Log::Category::error, "Faasm version of import %s wrong type (%s), was expecting %s\n",
                        exportName.c_str(),
                        asString(getObjectType(outObject)).c_str(),
                        asString(type).c_str());

            return false;
        }

        bool resolve(const std::string &moduleName, const std::string &exportName, IR::ObjectType type,
                     Runtime::Object *&outObject) override {
            auto namedInstance = moduleNameToInstanceMap.get(moduleName);

            if (namedInstance) {
                // Try looking up normally
                outObject = getInstanceExport(*namedInstance, exportName);

                if (outObject && isA(outObject, type)) {
                    // Successful lookup of correct type
                    return true;
                } else if (outObject) {
                    // Successful lookup of wrong type
                    Log::printf(Log::Category::error, "Resolved import %s.%s to a %s, but was expecting %s\n",
                                moduleName.c_str(),
                                exportName.c_str(),
                                asString(getObjectType(outObject)).c_str(),
                                asString(type).c_str());

                    return false;
                } else if (resolveFaasm(exportName, type, outObject)) {
                    // Found alternative in faasm module
                    return true;
                }
            }

            // Lookup totally failed, generate stub
            Log::printf(Log::Category::error, "Generated stub for missing import %s.%s : %s\n", moduleName.c_str(),
                        exportName.c_str(), asString(type).c_str());
            outObject = getStubObject(exportName, type);
            return true;
        }

        Runtime::Object *getStubObject(const std::string &exportName, IR::ObjectType type) const {
            // If the import couldn't be resolved, stub it in.
            switch (type.kind) {
                case IR::ObjectKind::function: {
                    // Generate a function body that just uses the unreachable op to fault if called.
                    Serialization::ArrayOutputStream codeStream;
                    IR::OperatorEncoderStream encoder(codeStream);
                    encoder.unreachable();
                    encoder.end();

                    // Generate a module for the stub function.
                    IR::Module stubModule;
                    IR::DisassemblyNames stubModuleNames;
                    stubModule.types.push_back(asFunctionType(type));
                    stubModule.functions.defs.push_back({{0}, {}, std::move(codeStream.getBytes()), {}});
                    stubModule.exports.push_back({"importStub", IR::ObjectKind::function, 0});
                    stubModuleNames.functions.push_back({"importStub: " + exportName, {}, {}});

                    IR::setDisassemblyNames(stubModule, stubModuleNames);
                    IR::validateDefinitions(stubModule);

                    // Instantiate the module and return the stub function instance.
                    auto stubModuleInstance = instantiateModule(compartment, Runtime::compileModule(stubModule), {}, "importStub");
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
}