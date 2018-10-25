#include <WAVM/Logging/Logging.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Inline/Serialization.h>
#include <WAVM/IR/Operators.h>
#include <WAVM/IR/Validate.h>

#include <boost/filesystem.hpp>

using namespace WAVM;

namespace wasm {
    struct RootResolver : Runtime::Resolver {
        HashMap<std::string, Runtime::ModuleInstance*> moduleNameToInstanceMap;
        Runtime::Compartment *compartment;

        RootResolver(Runtime::Compartment *inCompartment) : compartment(inCompartment) {
        }

        bool resolve(const std::string& moduleName,
                     const std::string& exportName,
                     IR::ExternType type,
                     Runtime::Object*& outObject) override
        {
            auto namedInstance = moduleNameToInstanceMap.get(moduleName);

            if(namedInstance) {
                outObject = getInstanceExport(*namedInstance, exportName);

                if(outObject && isA(outObject, type)) {
                    return true;
                };
            }

            std::cout << "Missing import " << moduleName << "." << exportName << " - " << asString(type).c_str() << std::endl;

            return false;
        }
    };

//    struct RootResolver : Runtime::Resolver {
//        Runtime::Compartment *compartment;
//        HashMap<std::string, Runtime::ModuleInstance *> moduleNameToInstanceMap;
//
//        RootResolver(Runtime::Compartment *inCompartment) : compartment(inCompartment) {
//        }
//
//        /**
//         * Returns the path to the file where generated stub object files will be stored
//         */
//        std::string getStubObjectFilePath(IR::FunctionType const &funcType) const {
//            // Build the function signature as
//            // stub_<inputs>_<outputs>
//            // i.e. for functions that take in two i32s and return one i32, it would be
//            // stub_i32i32_i32
//            std::string funcSignature = "stub_";
//            for (int i = 0; i < funcType.params().size(); i++) {
//                IR::ValueType resultType = funcType.params()[i];
//                funcSignature.append(asString(resultType));
//            }
//
//            funcSignature.append("_");
//            if (funcType.results().size() == 0) {
//                funcSignature.append("void");
//            } else {
//                for (int i = 0; i < funcType.results().size(); i++) {
//                    IR::ValueType resultType = funcType.results()[i];
//                    funcSignature.append(asString(resultType));
//                }
//            }
//
//            std::string dirPath = infra::getFunctionStubDir();
//            std::string stubPath = dirPath + "/" + funcSignature + ".o";
//
//            return stubPath;
//        }
//
//        std::vector<uint8_t> getStubObjectBytes(const IR::FunctionType &funcType) const {
//            std::string stubPath = getStubObjectFilePath(funcType);
//
//            if (boost::filesystem::exists(stubPath)) {
//                // Load bytes if they exist
//                return util::readFileToBytes(stubPath);
//            } else {
//                std::vector<uint8_t> stubBytes;
//                return stubBytes;
//            }
//        }
//
//        void persistStubObjectBytes(const IR::FunctionType &funcType, std::vector<uint8_t> bytes) const {
//            std::string stubPath = getStubObjectFilePath(funcType);
//
//            std::cout << "Creating new stub file at " << stubPath << std::endl;
//
//            util::writeBytesToFile(stubPath, bytes);
//        }
//
//        bool resolve(const std::string &moduleName, const std::string &exportName, IR::ExternType type,
//                     Runtime::Object *&outObject) override {
//
//            // Attempt lookup in specified module
//            auto moduleInstance = moduleNameToInstanceMap.get(moduleName);
//            if (moduleInstance) {
//                outObject = getInstanceExport(*moduleInstance, exportName);
//
//                if (outObject && isA(outObject, type)) {
//                    return true;
//                }
//            }
//
//            // Lookup totally failed, use stub
//            Log::printf(Log::Category::error, "Stubbing missing import %s.%s : %s\n", moduleName.c_str(),
//                        exportName.c_str(), asString(type).c_str());
//
//            outObject = getStubObject(exportName, type);
//            return true;
//        }
//
//        Runtime::Object *getStubObject(const std::string &exportName, IR::ExternType type) const {
//            switch (type.kind) {
//                case IR::ExternKind::function: {
//                    // Generate a module for the stub function.
//                    IR::Module stubModule;
//
////                    IR::DisassemblyNames stubModuleNames;
////                    stubModuleNames.functions.push_back({"importStub: " + exportName, {}, {}});
////                    IR::setDisassemblyNames(stubModule, stubModuleNames);
//
//                    IR::FunctionType funcType = asFunctionType(type);
//                    std::vector<U8> emptyBytes;
//                    stubModule.types.push_back(funcType);
//                    stubModule.functions.defs.push_back({{0}, {}, emptyBytes, {}});
//                    stubModule.exports.push_back({"importStub", IR::ExternKind::function, 0});
//
//                    // Validation
//                    IR::validatePreCodeSections(stubModule);
//                    IR::DeferredCodeValidationState deferredCodeValidationState;
//                    IR::validatePostCodeSections(stubModule, deferredCodeValidationState);
//
//                    // Try and load existing object bytes for this stub
//                    std::vector<uint8_t> stubBytes = getStubObjectBytes(funcType);
//
//                    Runtime::ModuleRef compiledModule;
//                    if (true) {
//                        // Add function body to the module just calling "unreachable"
//                        Serialization::ArrayOutputStream codeStream;
//                        IR::OperatorEncoderStream encoder(codeStream);
//                        encoder.unreachable();
//                        encoder.end();
//
//                        stubModule.functions.defs.push_back({{0}, {}, std::move(codeStream.getBytes()), {}});
//
//                        // Compile stub from scratch and persist the compiled bytes
//                        compiledModule = Runtime::compileModule(stubModule);
//                        std::vector<U8> objectFileBytes = Runtime::getObjectCode(compiledModule);
//                        persistStubObjectBytes(funcType, objectFileBytes);
//
//                    } else {
//                        // Use existing bytes
//                        std::cout << "Using precompiled stub " << exportName << std::endl;
//
//                        compiledModule = Runtime::loadPrecompiledModule(stubModule, stubBytes);
//                    }
//
//                    // Instantiate the module within the compartment
//                    auto stubModuleInstance = instantiateModule(compartment, compiledModule, {}, "importStub");
//                    return getInstanceExport(stubModuleInstance, "importStub");
//                }
//                case IR::ExternKind::memory: {
//                    return asObject(
//                            Runtime::createMemory(compartment, asMemoryType(type), std::string(exportName)));
//                }
//                case IR::ExternKind::table: {
//                    return asObject(
//                            Runtime::createTable(compartment, asTableType(type), std::string(exportName)));
//                }
//                case IR::ExternKind::global: {
//                    return asObject(Runtime::createGlobal(
//                            compartment,
//                            asGlobalType(type),
//                            IR::Value(asGlobalType(type).valueType, IR::UntaggedValue()))
//                    );
//                }
//                case IR::ExternKind::exceptionType: {
//                    return asObject(
//                            Runtime::createExceptionType(compartment, asExceptionType(type), "importStub"));
//                }
//                default: {
//                    Errors::unreachable();
//                }
//            };
//        }
//    };
}
