#include "Logging/Logging.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Inline/Serialization.h"
#include "IR/Operators.h"
#include "IR/Validate.h"

#include <boost/filesystem.hpp>
#include <Runtime/RuntimePrivate.h>

namespace wasm {

    struct RootResolver : Runtime::Resolver {
        Runtime::Compartment *compartment;
        HashMap<std::string, Runtime::ModuleInstance *> moduleNameToInstanceMap;
        
        RootResolver(Runtime::Compartment *inCompartment) : compartment(inCompartment) {
        }

        /**
         * Returns the path to the file where generated stub object files will be stored 
         */
        std::string getStubObjectFilePath(IR::FunctionType const &funcType) const {
            // Build the function signature as
            // stub_<inputs>_<outputs>
            // i.e. for functions that take in two i32s and return one i32, it would be
            // stub_i32i32_i32
            std::string funcSignature = "stub_";
            for(int i = 0; i < funcType.params().size(); i++) {
                IR::ValueType resultType = funcType.params()[i];
                funcSignature.append(asString(resultType));
            }

            funcSignature.append("_");
            if(funcType.results().size() == 0) {
                funcSignature.append("void");
            }
            else {
                for(int i = 0; i < funcType.results().size(); i++) {
                    IR::ValueType resultType = funcType.results()[i];
                    funcSignature.append(asString(resultType));
                }
            }

            std::string dirPath = infra::getFunctionStubDir();
            std::string stubPath = dirPath + "/" + funcSignature + ".o";

            return stubPath;
        }

        std::vector<uint8_t> getStubObjectBytes(const IR::FunctionType &funcType) const {
            std::string stubPath = getStubObjectFilePath(funcType);

            if(boost::filesystem::exists(stubPath)) {
                // Load bytes if they exist
                std::cout << "Using existing stub at " << stubPath << std::endl;
                return util::readFileToBytes(stubPath);
            }
            else {
                std::vector<uint8_t> stubBytes;
                return stubBytes;
            }
        }

        void persistStubObjectBytes(const IR::FunctionType &funcType, std::vector<uint8_t> bytes) const {
            std::string stubPath = getStubObjectFilePath(funcType);

            std::cout << "Creating new stub file at " << stubPath << std::endl;

            util::writeBytesToFile(stubPath, bytes);
        }

        bool resolve(const std::string &moduleName, const std::string &exportName, IR::ObjectType type,
                     Runtime::Object *&outObject) override {

            // Attempt lookup in specified module
            auto moduleInstance = moduleNameToInstanceMap.get(moduleName);
            if (moduleInstance) {
                outObject = getInstanceExport(*moduleInstance, exportName);

                if (outObject && isA(outObject, type)) {
                    return true;
                }
            }

            // Try looking up function in faasm module
            auto faasmModule = moduleNameToInstanceMap.get("faasm");
            outObject = getInstanceExport(*faasmModule, exportName);

            if(outObject && isA(outObject, type)) {
                Log::printf(Log::Category::debug, "Using Faasm version of %s \n", exportName.c_str());
                return true;
            }

            // Lookup totally failed, use stub
            Log::printf(Log::Category::error, "Stubbing missing import %s.%s : %s\n", moduleName.c_str(),
                        exportName.c_str(), asString(type).c_str());

            outObject = getStubObject(exportName, type);
            return true;
        }

        Runtime::Object* getStubObject(const std::string &exportName, IR::ObjectType type) const {
            switch (type.kind) {
                case IR::ObjectKind::function: {
                    const IR::FunctionType &funcType = asFunctionType(type);

                    // Generate a module for the stub function.
                    IR::Module stubModule;
                    stubModule.types.push_back(funcType);
                    stubModule.exports.push_back({"importStub", IR::ObjectKind::function, 0});

                    // Try and load existing object bytes for this stub
                    std::vector<uint8_t> stubBytes = getStubObjectBytes(funcType);

                    Runtime::Module *compiledModule;
                    if(stubBytes.empty()) {
                        // Add function body to the module (just calling "unreachable"
                        Serialization::ArrayOutputStream codeStream;
                        IR::OperatorEncoderStream encoder(codeStream);
                        encoder.unreachable();
                        encoder.end();
                        const std::vector<U8> &stubCodeBytes = codeStream.getBytes();

                        stubModule.functions.defs.push_back({{0}, {}, std::move(stubCodeBytes), {}});

                        // Compile stub from scratch and persist the compiled bytes
                        compiledModule = Runtime::compileModule(stubModule);
                        std::vector<U8> objectFileBytes = Runtime::getObjectCode(compiledModule);
                        persistStubObjectBytes(funcType, objectFileBytes);
                    }
                    else {
                        // Use existing bytes
                        std::vector<U8> emptyBytes;
                        stubModule.functions.defs.push_back({{0}, {}, emptyBytes, {}});

                        compiledModule = Runtime::loadPrecompiledModule(stubModule, stubBytes);
                    }

                    // Instantiate the module within the compartment
                    auto stubModuleInstance = instantiateModule(compartment, compiledModule, {}, "importStub");
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