#include "func.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <faabric/util/environment.h>
#include <faabric/util/random.h>
#include <faabric/util/gids.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/clock.h>


namespace faabric::util {
    const static std::string pyFile = "function.py";
    const static std::string funcFile = "function.wasm";
    const static std::string symFile = "function.symbols";
    const static std::string objFile = "function.wasm.o";
    const static std::string wamrAotFile = "function.aot";
    const static std::string confFile = "conf.json";
    
    std::string getRootUrl() {
        std::string rootUrl = faabric::utilgetEnvVar("FILESERVER_URL", "");
        if (rootUrl.empty()) {
            throw std::runtime_error("Fileserver URL not set");
        }

        return rootUrl;
    }

    std::string getUrl(const faabric::Message &msg, const std::string &urlPart) {
        std::string rootUrl = getRootUrl();

        std::string funcUrl = rootUrl + "/" + urlPart + "/" + msg.user() + "/" + msg.function();

        return funcUrl;
    }

    std::string getSharedObjectUrl() {
        std::string rootUrl = getRootUrl();
        return rootUrl + "/sobjwasm";
    }

    std::string getSharedObjectObjectUrl() {
        std::string rootUrl = getRootUrl();
        return rootUrl + "/sobjobj";
    }

    std::string getSharedFileUrl() {
        std::string rootUrl = getRootUrl();
        return rootUrl + "/file";
    }

    std::string getFunctionUrl(const faabric::Message &msg) {
        return getUrl(msg, "f");
    }

    std::string getFunctionObjectUrl(const faabric::Message &msg) {
        return getUrl(msg, "fo");
    }

    std::string getPythonFunctionUrl(const faabric::Message &msg) {
        return getUrl(msg, "p");
    }

    std::string _doGetPythonFunctionFile(const faabric::Message &msg, const std::string &parentDir, bool createDirs) {
        if (!msg.ispython()) {
            throw std::runtime_error(
                    "Getting python function file for non-Python function " + funcToString(msg, false)
            );
        }

        if(msg.pythonuser().empty() || msg.pythonfunction().empty()) {
            throw std::runtime_error(
                    "Invalid Python call: user=" + msg.pythonuser() + " func=" + msg.pythonfunction()
            );
        }
        boost::filesystem::path path(parentDir);

        path.append(PYTHON_FUNC_DIR);
        path.append(msg.pythonuser());
        path.append(msg.pythonfunction());

        if (createDirs) {
            boost::filesystem::create_directories(path);
        }

        path.append(pyFile);
        return path.string();
    }

    boost::filesystem::path _doGetDir(std::string baseDir, const faabric::Message &msg, bool create) {
        boost::filesystem::path path(baseDir);
        path.append(msg.user());
        path.append(msg.function());

        // Create directory if doesn't exist
        if (create) {
            boost::filesystem::create_directories(path);
        }

        return path;
    }

    boost::filesystem::path getFunctionDir(const faabric::Message &msg, bool create = true) {
        SystemConfig &conf = faabric::utilgetSystemConfig();
        return _doGetDir(conf.functionDir, msg, create);
    }

    boost::filesystem::path getObjectDir(const faabric::Message &msg, bool create = true) {
        SystemConfig &conf = faabric::utilgetSystemConfig();
        return _doGetDir(conf.objectFileDir, msg, create);
    }

    bool isValidFunction(const faabric::Message &msg) {
        if (msg.user().empty() || msg.function().empty()) {
            return false;
        }

        auto path = getFunctionDir(msg, false);
        path.append("function.wasm");

        bool isValid = boost::filesystem::exists(path);
        return isValid;
    }

    std::string getFunctionKey(const faabric::Message &msg) {
        std::string key = "wasm";

        key += "/";
        key += msg.user();
        key += "/";
        key += msg.function();
        key += "/";
        key += funcFile;

        return key;
    }

    std::string getFunctionObjectKey(const faabric::Message &msg) {
        std::string key = "wasm";

        key += "/";
        key += msg.user();
        key += "/";
        key += msg.function();
        key += "/";
        key += objFile;

        return key;
    }

    std::string getPythonFunctionFile(const faabric::Message &msg) {
        // Python functions are stored as shared files to make it easier to
        // share them through the system
        return _doGetPythonFunctionFile(msg, faabric::utilgetSystemConfig().sharedFilesStorageDir, true);
    }

    std::string getPythonFunctionFileSharedPath(const faabric::Message &msg) {
        // This is the shared path of the form faasm:// used to access the Python file
        return _doGetPythonFunctionFile(msg, SHARED_FILE_PREFIX, false);
    }

    std::string getPythonRuntimeFunctionFile(const faabric::Message &msg) {
        // This is the path where the file is placed at runtime to be
        // accessible to the function
        return _doGetPythonFunctionFile(msg, faabric::utilgetSystemConfig().runtimeFilesDir, true);
    }

    std::string getFunctionFile(const faabric::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionSymbolsFile(const faabric::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(symFile);

        return path.string();
    }

    std::string getFunctionObjectFile(const faabric::Message &msg) {
        auto path = getObjectDir(msg);
        path.append(objFile);

        return path.string();
    }

    std::string getFunctionAotFile(const faabric::Message &msg) {
        auto path = getObjectDir(msg);
        path.append(wamrAotFile);

        return path.string();
    }

    std::string getSharedObjectObjectFile(const std::string &realPath) {
        boost::filesystem::directory_entry f(realPath);
        const std::string directory = f.path().parent_path().string();
        const std::string fileName = f.path().filename().string();

        // Work out the final destination for the object file. This will be the
        // object path with the directory of the original file appended
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();
        boost::filesystem::path objPath(conf.objectFileDir);
        objPath.append(directory);

        // Create directory (if necessary)
        create_directories(objPath);

        // Add the filename
        std::string outputFile = objPath.append(fileName).string();
        outputFile += SHARED_OBJ_EXT;

        return outputFile;
    }

    std::string getSharedFileFile(const std::string &path) {
        SystemConfig &conf = faabric::utilgetSystemConfig();

        boost::filesystem::path p(conf.sharedFilesStorageDir);
        p.append(path);

        if (!boost::filesystem::exists(p.parent_path())) {
            boost::filesystem::create_directories(p.parent_path());
        }

        return p.string();
    }

    std::vector<uint8_t> messageToBytes(const faabric::Message &msg) {
        size_t byteSize = msg.ByteSizeLong();
        uint8_t buffer[byteSize];
        msg.SerializeToArray(buffer, (int) byteSize);

        std::vector<uint8_t> inputData(buffer, buffer + byteSize);

        return inputData;
    }

    std::string funcToString(const faabric::Message &msg, bool includeId) {
        std::string str = msg.user() + "/" + msg.function();

        if (includeId) {
            str += ":" + std::to_string(msg.id());
        }
        return str;
    }

    std::string buildAsyncResponse(const faabric::Message &msg) {
        if (msg.id() == 0) {
            throw std::runtime_error("Message must have id to build async response");
        }

        return std::to_string(msg.id());
    }

    faabric::Message messageFactory(const std::string &user, const std::string &function) {
        faabric::Message msg;
        msg.set_user(user);
        msg.set_function(function);
        setMessageId(msg);

        return msg;
    }

    void convertMessageToPython(faabric::Message &msg) {
        msg.set_ispython(true);
        msg.set_pythonfunction(msg.function());
        msg.set_pythonuser(msg.user());

        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);
    }

    unsigned int setMessageId(faabric::Message &msg) {
        // If message already has an ID, just make sure the keys are set up
        unsigned int messageId;
        if (msg.id() > 0) {
            messageId = msg.id();
        } else {
            // Generate a random ID
            messageId = faabric::utilgenerateGid();
            msg.set_id(messageId);
        }

        // Set the timestamp if it doesn't have one
        if(msg.timestamp() <= 0) {
            Clock &clock = faabric::utilgetGlobalClock();
            msg.set_timestamp(clock.epochMillis());
        }

        std::string resultKey = resultKeyFromMessageId(messageId);
        msg.set_resultkey(resultKey);

        std::string statusKey = statusKeyFromMessageId(messageId);
        msg.set_statuskey(statusKey);

        return messageId;
    }

    std::string resultKeyFromMessageId(unsigned int mid) {
        std::string k = "result_";
        k += std::to_string(mid);
        return k;
    }

    std::string statusKeyFromMessageId(unsigned int mid) {
        std::string k = "status_";
        k += std::to_string(mid);
        return k;
    }

    std::vector<std::string> getArgvForMessage(const faabric::Message &msg) {
        // We always have some arbitrary script name as argv[0]
        std::vector<std::string> argv = {"function.wasm"};

        std::string cmdlineArgs = msg.cmdline();
        if (cmdlineArgs.empty()) {
            return argv;
        }

        // Split the extra args
        std::vector<std::string> extraArgs;
        std::string copy(msg.cmdline());
        boost::split(extraArgs, copy, [](char c) { return c == ' '; });

        // Build the final list
        argv.insert(argv.end(), extraArgs.begin(), extraArgs.end());

        return argv;
    }
}

