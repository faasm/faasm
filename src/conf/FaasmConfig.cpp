#include <conf/FaasmConfig.h>
#include <faabric/util/environment.h>
#include <faabric/util/logging.h>

using namespace faabric::util;

namespace conf {
FaasmConfig& getFaasmConfig()
{
    static FaasmConfig conf;
    return conf;
}

FaasmConfig::FaasmConfig()
{
    this->initialise();
}

void FaasmConfig::initialise()
{
    hostType = getEnvVar("HOST_TYPE", "default");
    functionStorage = getEnvVar("FUNCTION_STORAGE", "local");
    fileserverUrl = getEnvVar("FILESERVER_URL", "");

    cgroupMode = getEnvVar("CGROUP_MODE", "on");
    netNsMode = getEnvVar("NETNS_MODE", "off");

    pythonPreload = getEnvVar("PYTHON_PRELOAD", "off");
    captureStdout = getEnvVar("CAPTURE_STDOUT", "off");

    wasmVm = getEnvVar("WASM_VM", "wavm");
    chainedCallTimeout = this->getIntParam("CHAINED_CALL_TIMEOUT", "300000");

    std::string faasmLocalDir =
      getEnvVar("FAASM_LOCAL_DIR", "/usr/local/faasm");
    functionDir = fmt::format("{}/{}", faasmLocalDir, "wasm");
    objectFileDir = fmt::format("{}/{}", faasmLocalDir, "object");
    runtimeFilesDir = fmt::format("{}/{}", faasmLocalDir, "runtime_root");
    sharedFilesDir = fmt::format("{}/{}", faasmLocalDir, "shared");
    sharedFilesStorageDir = fmt::format("{}/{}", faasmLocalDir, "shared_store");
}

int FaasmConfig::getIntParam(const char* name, const char* defaultValue)
{
    int value = stoi(faabric::util::getEnvVar(name, defaultValue));

    return value;
};

void FaasmConfig::reset()
{
    this->initialise();
}

void FaasmConfig::print()
{
    SPDLOG_INFO("--- HOST ---");
    SPDLOG_INFO("Cgroup mode:          {}", cgroupMode);
    SPDLOG_INFO("Host type:            {}", hostType);
    SPDLOG_INFO("Network ns mode:      {}", netNsMode);

    SPDLOG_INFO("--- MISC ---");
    SPDLOG_INFO("Capture stdout:       {}", captureStdout);
    SPDLOG_INFO("Chained call timeout: {}", chainedCallTimeout);
    SPDLOG_INFO("Python preload:       {}", pythonPreload);
    SPDLOG_INFO("Wasm VM:              {}", wasmVm);

    SPDLOG_INFO("--- STORAGE ---");
    SPDLOG_INFO("Fileserver:           {}", fileserverUrl);
    SPDLOG_INFO("Function storage:     {}", functionStorage);
    SPDLOG_INFO("Function dir:         {}", functionDir);
    SPDLOG_INFO("Object file dir:      {}", objectFileDir);
    SPDLOG_INFO("Runtime files dir:    {}", runtimeFilesDir);
    SPDLOG_INFO("Shared files dir:     {}", sharedFilesDir);
    SPDLOG_INFO("Shared storage dir:   {}", sharedFilesStorageDir);
}
}
